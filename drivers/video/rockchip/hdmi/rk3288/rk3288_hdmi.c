#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/rockchip/grf.h>
#include <linux/rockchip/iomap.h>
#if defined(CONFIG_DEBUG_FS)
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#include "rk3288_hdmi.h"
#include "rk3288_hdmi_hw.h"

#define HDMI_SEL_LCDC(x)	((((x)&1)<<4)|(1<<20))
#define grf_writel(v, offset)	do { \
		writel_relaxed(v, RK_GRF_VIRT + offset); \
		dsb(); \
		} while (0)

static struct hdmi_dev *hdmi_dev;

#if defined(CONFIG_DEBUG_FS)
static const struct rk3288_hdmi_reg_table hdmi_reg_table[] = {
	{IDENTIFICATION_BASE, CONFIG3_ID},
	{INTERRUPT_BASE, IH_MUTE},
	{VIDEO_SAMPLER_BASE, TX_BCBDATA1},
	{VIDEO_PACKETIZER_BASE, VP_MASK},
	{FRAME_COMPOSER_BASE, FC_DBGTMDS2},
	{HDMI_SOURCE_PHY_BASE, PHY_PLLCFGFREQ2},
	{I2C_MASTER_PHY_BASE, PHY_I2CM_SDA_HOLD},
	{AUDIO_SAMPLER_BASE, AHB_DMA_STPADDR_SET1_0},
	{MAIN_CONTROLLER_BASE, MC_SWRSTZREQ_2},
	{COLOR_SPACE_CONVERTER_BASE, CSC_SPARE_2},
	{HDCP_ENCRYPTION_ENGINE_BASE, HDCP_REVOC_LIST},
	{HDCP_BKSV_BASE, HDCPREG_BKSV4},
	{HDCP_AN_BASE, HDCPREG_AN7},
	{ENCRYPTED_DPK_EMBEDDED_BASE, HDCPREG_DPK6},
	{CEC_ENGINE_BASE, CEC_WKUPCTRL},
	{I2C_MASTER_BASE, I2CM_SCDC_UPDATE1},
};

static int rk3288_hdmi_reg_show(struct seq_file *s, void *v)
{
	int i = 0, j = 0;
	u32 val = 0;

	seq_printf(s, "\n>>>hdmi_ctl reg ");
	for (i = 0; i < 16; i++)
		seq_printf(s, " %2x", i);
	seq_printf(s, "\n-----------------------------------------------------------------");

	for (i = 0; i < ARRAY_SIZE(hdmi_reg_table); i++) {
		for (j = hdmi_reg_table[i].reg_base;
		     j <= hdmi_reg_table[i].reg_end; j++) {
			val = hdmi_readl(hdmi_dev, j);
			if ((j - hdmi_reg_table[i].reg_base)%16 == 0)
				seq_printf(s, "\n>>>hdmi_ctl %04x:", j);
			seq_printf(s, " %02x", val);

		}
	}
	seq_printf(s, "\n-----------------------------------------------------------------\n");

	return 0;
}

static ssize_t rk3288_hdmi_reg_write(struct file *file,
			const char __user *buf, size_t count, loff_t *ppos)
{
	u32 reg;
	u32 val;
	char kbuf[25];

	if (copy_from_user(kbuf, buf, count))
		return -EFAULT;
	sscanf(kbuf, "%x%x", &reg, &val);
	if ((reg < 0) || (reg > I2CM_SCDC_UPDATE1)) {
		dev_info(hdmi_dev->hdmi->dev, "it is no hdmi reg\n");
		return count;
	}
	dev_info(hdmi_dev->hdmi->dev,
		"/**********rk3288 hdmi reg config******/");
	dev_info(hdmi_dev->hdmi->dev, "\n reg=%x val=%x\n", reg, val);
	hdmi_writel(hdmi_dev, reg, val);

	return count;
}

static int rk3288_hdmi_reg_open(struct inode *inode, struct file *file)
{
	struct hdmi_dev *hdmi_dev = inode->i_private;

	return single_open(file, rk3288_hdmi_reg_show, hdmi_dev);
}

static const struct file_operations rk3288_hdmi_reg_fops = {
	.owner		= THIS_MODULE,
	.open		= rk3288_hdmi_reg_open,
	.read		= seq_read,
	.write		= rk3288_hdmi_reg_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

#if defined(CONFIG_OF)
static const struct of_device_id rk3288_hdmi_dt_ids[] = {
	{.compatible = "rockchip,rk3288-hdmi",},
	{}
};
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void rk3288_hdmi_early_suspend(struct early_suspend *h)
{
	struct hdmi *hdmi = hdmi_dev->hdmi;
	struct delay_work *delay_work;
	struct pinctrl_state *gpio_state;

	HDMIDBG("hdmi enter early suspend \n");
	delay_work = hdmi_submit_work(hdmi, HDMI_SUSPEND_CTL, 0, NULL);
	if (delay_work)
		flush_delayed_work_sync(delay_work);
	/* iomux to gpio and pull down when suspend */
	gpio_state = pinctrl_lookup_state(hdmi_dev->dev->pins->p, "gpio");
	pinctrl_select_state(hdmi_dev->dev->pins->p, gpio_state);
	rk3288_hdmi_clk_disable(hdmi_dev);
}

static void rk3288_hdmi_early_resume(struct early_suspend *h)
{
	struct hdmi *hdmi = hdmi_dev->hdmi;

	HDMIDBG("hdmi exit early resume\n");
	/* iomux to default state for hdmi use when resume */
	pinctrl_select_state(hdmi_dev->dev->pins->p,
			     hdmi_dev->dev->pins->default_state);
	rk3288_hdmi_clk_enable(hdmi_dev);
	hdmi_dev_initial(hdmi_dev);
	if (hdmi->ops->hdcp_power_on_cb)
		hdmi->ops->hdcp_power_on_cb();
	hdmi_submit_work(hdmi, HDMI_RESUME_CTL, 0, NULL);
}
#endif

#define HDMI_PD_ON			(1 << 0)
#define HDMI_PCLK_ON		(1 << 1)
#define HDMI_HDCPCLK_ON		(1 << 2)
#define HDMI_CECCLK_ON		(1 << 3)

static int rk3288_hdmi_clk_enable(struct hdmi_dev *hdmi_dev)
{
	if ((hdmi_dev->clk_on & HDMI_PD_ON) &&
	    (hdmi_dev->clk_on & HDMI_PCLK_ON)
		&& (hdmi_dev->clk_on & HDMI_HDCPCLK_ON))
		return 0;

	if ((hdmi_dev->clk_on & HDMI_PD_ON) == 0) {
		if (hdmi_dev->pd == NULL) {
			hdmi_dev->pd = devm_clk_get(hdmi_dev->dev, "pd_hdmi");
			if (IS_ERR(hdmi_dev->pd)) {
				dev_err(hdmi_dev->dev,
					"Unable to get hdmi pd\n");
				return -1;
			}
		}
		clk_prepare_enable(hdmi_dev->pd);
		hdmi_dev->clk_on |= HDMI_PD_ON;
	}

	if ((hdmi_dev->clk_on & HDMI_PCLK_ON) == 0) {
		if (hdmi_dev->pclk == NULL) {
			hdmi_dev->pclk =
				devm_clk_get(hdmi_dev->dev, "pclk_hdmi");
			if (IS_ERR(hdmi_dev->pclk)) {
				dev_err(hdmi_dev->dev,
					"Unable to get hdmi pclk\n");
				return -1;
			}
		}
		clk_prepare_enable(hdmi_dev->pclk);
		hdmi_dev->clk_on |= HDMI_PCLK_ON;
	}

	if ((hdmi_dev->clk_on & HDMI_HDCPCLK_ON) == 0) {
		if (hdmi_dev->hdcp_clk == NULL) {
			hdmi_dev->hdcp_clk =
				devm_clk_get(hdmi_dev->dev, "hdcp_clk_hdmi");
			if (IS_ERR(hdmi_dev->hdcp_clk)) {
				dev_err(hdmi_dev->dev,
					"Unable to get hdmi hdcp_clk\n");
				return -1;
			}
		}
		clk_prepare_enable(hdmi_dev->hdcp_clk);
		hdmi_dev->clk_on |= HDMI_HDCPCLK_ON;
	}

	if ((hdmi_dev->clk_on & HDMI_CECCLK_ON) == 0) {
		if (hdmi_dev->cec_clk == NULL) {
			hdmi_dev->cec_clk =
				devm_clk_get(hdmi_dev->dev, "cec_clk_hdmi");
			if (IS_ERR(hdmi_dev->cec_clk)) {
				dev_err(hdmi_dev->dev,
					"Unable to get hdmi cec_clk\n");
				return -1;
			}
		}
		clk_prepare_enable(hdmi_dev->cec_clk);
		hdmi_dev->clk_on |= HDMI_CECCLK_ON;
	}
	return 0;
}

static int rk3288_hdmi_clk_disable(struct hdmi_dev *hdmi_dev)
{
	if (hdmi_dev->clk_on == 0)
		return 0;

	if ((hdmi_dev->clk_on & HDMI_PD_ON) && (hdmi_dev->pd != NULL)) {
		clk_disable_unprepare(hdmi_dev->pd);
		hdmi_dev->clk_on &= ~HDMI_PD_ON;
	}
/*
	if ((hdmi_dev->clk_on & HDMI_PCLK_ON) &&
	    (hdmi_dev->pclk != NULL)) {
		clk_disable_unprepare(hdmi_dev->pclk);
		hdmi_dev->clk_on &= ~HDMI_PCLK_ON;
	}

	if ((hdmi_dev->clk_on & HDMI_HDCPCLK_ON) &&
	    (hdmi_dev->hdcp_clk != NULL)) {
		clk_disable_unprepare(hdmi_dev->hdcp_clk);
		hdmi_dev->clk_on &= ~HDMI_HDCPCLK_ON;
	}
*/
	return 0;
}

static int rk3288_hdmi_fb_event_notify(struct notifier_block *self,
				unsigned long action, void *data)
{
	struct fb_event *event = data;
	int blank_mode = *((int *)event->data);
	struct hdmi *hdmi = hdmi_dev->hdmi;
	struct delayed_work *delay_work;

	if (action == FB_EARLY_EVENT_BLANK) {
		switch (blank_mode) {
		case FB_BLANK_UNBLANK:
			break;
		default:
			HDMIDBG("suspend hdmi\n");
			if (!hdmi->sleep) {
				delay_work =
					hdmi_submit_work(hdmi,
							 HDMI_SUSPEND_CTL,
							 0, NULL);
				if (delay_work)
					flush_delayed_work(delay_work);
				rk3288_hdmi_clk_disable(hdmi_dev);
			}
			break;
		}
	} else if (action == FB_EVENT_BLANK) {
		switch (blank_mode) {
		case FB_BLANK_UNBLANK:
			HDMIDBG("resume hdmi\n");
			if (hdmi->sleep) {
				rk3288_hdmi_clk_enable(hdmi_dev);
				rk3288_hdmi_dev_initial(hdmi_dev);
				if (hdmi->ops->hdcp_power_on_cb)
					hdmi->ops->hdcp_power_on_cb();
				hdmi_submit_work(hdmi, HDMI_RESUME_CTL,
						 0, NULL);
			}
			break;
		default:
			break;
		}
	}
	return NOTIFY_OK;
}

static struct notifier_block rk3288_hdmi_fb_notifier = {
	.notifier_call = rk3288_hdmi_fb_event_notify,
};
#ifdef HDMI_INT_USE_POLL
static void rk3288_hdmi_irq_work_func(struct work_struct *work)
{
	if (hdmi_dev->enable) {
		rk3288_hdmi_dev_irq(0, hdmi_dev);
		queue_delayed_work(hdmi_dev->workqueue,
				   &(hdmi_dev->delay_work),
				   msecs_to_jiffies(50));
	}
}
#endif
static struct hdmi_property rk3288_hdmi_property = {
	.videosrc = DISPLAY_SOURCE_LCDC0,
	.display = DISPLAY_MAIN,
};

static struct hdmi_ops rk3288_hdmi_ops;

static int rk3288_hdmi_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct resource *res;
	struct delayed_work *delaywork;
	struct device_node *np = pdev->dev.of_node;
	const struct of_device_id *match;
	int val = 0;

	HDMIDBG("%s\n", __func__);
	hdmi_dev = kmalloc(sizeof(struct hdmi_dev), GFP_KERNEL);
	if (!hdmi_dev) {
		dev_err(&pdev->dev, ">>rk3288 hdmi kmalloc fail!");
		return -ENOMEM;
	}
	memset(hdmi_dev, 0, sizeof(struct hdmi_dev));
	platform_set_drvdata(pdev, hdmi_dev);
	hdmi_dev->dev = &pdev->dev;
	/*request and remap iomem*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Unable to get register resource\n");
		ret = -ENXIO;
		goto failed;
	}
	hdmi_dev->regbase_phy = res->start;
	hdmi_dev->regsize_phy = resource_size(res);
	hdmi_dev->regbase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(hdmi_dev->regbase)) {
		ret = PTR_ERR(hdmi_dev->regbase);
		dev_err(&pdev->dev,
			"cannot ioremap registers,err=%d\n", ret);
		goto failed;
	}

	/*enable pd and pclk and hdcp_clk*/
	if (rk3288_hdmi_clk_enable(hdmi_dev) < 0) {
		ret = -ENXIO;
		goto failed1;
	}

	rk3288_hdmi_dev_init_ops(&rk3288_hdmi_ops);
	/* Register HDMI device */
	rk3288_hdmi_property.name = (char *)pdev->name;
	rk3288_hdmi_property.priv = hdmi_dev;
	hdmi_dev->hdmi =
		hdmi_register(&rk3288_hdmi_property, &rk3288_hdmi_ops);
	if (hdmi_dev->hdmi == NULL) {
		dev_err(&pdev->dev, "register hdmi device failed\n");
		ret = -ENOMEM;
		goto failed1;
	}
	mutex_init(&hdmi_dev->ddc_lock);
	hdmi_dev->hdmi->dev = &pdev->dev;
	fb_register_client(&rk3288_hdmi_fb_notifier);
	/*lcdc source select*/
	grf_writel(HDMI_SEL_LCDC(rk3288_hdmi_property.videosrc),
		   RK3288_GRF_SOC_CON6);
	/* select GPIO7_C0 as cec pin */
	grf_writel(((1 << 12) | (1 << 28)), RK3288_GRF_SOC_CON8);
	match = of_match_node(rk3288_hdmi_dt_ids, np);
	if (!match)
		return PTR_ERR(match);
	if (of_property_read_u32(np, "hdmi_cec", &val))
		hdmi_dev->hdmi->cecenable= 0;
	else
		hdmi_dev->hdmi->cecenable = val;
	if (of_property_read_u32(np, "hdcp_enable", &val))
		hdmi_dev->hdcp_enable = 0;
	else
		hdmi_dev->hdcp_enable = val;
	rk3288_hdmi_dev_initial(hdmi_dev);
	pinctrl_select_state(hdmi_dev->dev->pins->p,
			     hdmi_dev->dev->pins->default_state);
#if defined(CONFIG_DEBUG_FS)
	hdmi_dev->debugfs_dir = debugfs_create_dir("rk3288-hdmi", NULL);
	if (IS_ERR(hdmi_dev->debugfs_dir))
		dev_err(hdmi_dev->hdmi->dev,
			"failed to create debugfs dir for rk616!\n");
	else
		debugfs_create_file("hdmi", S_IRUSR,
				    hdmi_dev->debugfs_dir,
				    hdmi_dev, &rk3288_hdmi_reg_fops);
#endif
	delaywork =
		hdmi_submit_work(hdmi_dev->hdmi, HDMI_HPD_CHANGE, 0, NULL);
	if (delaywork)
		flush_delayed_work(delaywork);
#ifndef HDMI_INT_USE_POLL
	/* get and request the IRQ */
	hdmi_dev->irq = platform_get_irq(pdev, 0);
	if (hdmi_dev->irq <= 0) {
		dev_err(hdmi_dev->dev,
			"failed to get hdmi irq resource (%d).\n",
			hdmi_dev->irq);
		ret = -ENXIO;
		goto failed1;
	}

	ret =
	    devm_request_irq(hdmi_dev->dev, hdmi_dev->irq,
			     rk3288_hdmi_dev_irq, IRQF_TRIGGER_HIGH,
			     dev_name(hdmi_dev->dev), hdmi_dev);
	if (ret) {
		dev_err(hdmi_dev->dev, "hdmi request_irq failed (%d).\n", ret);
		goto failed1;
	}
#else
	hdmi_dev->workqueue = create_singlethread_workqueue("rk3288 hdmi irq");
	INIT_DELAYED_WORK(&(hdmi_dev->delay_work), rk3288_hdmi_irq_work_func);
	rk3288_hdmi_irq_work_func(NULL);

#endif
	dev_info(&pdev->dev, "rk3288 hdmi probe sucess.\n");
	return 0;

failed1:
	hdmi_unregister(hdmi_dev->hdmi);
failed:
	kfree(hdmi_dev);
	hdmi_dev = NULL;
	dev_err(&pdev->dev, "rk3288 hdmi probe error.\n");
	return ret;
}

static int rk3288_hdmi_remove(struct platform_device *pdev)
{

	dev_info(&pdev->dev, "rk3288 hdmi driver removed.\n");
	return 0;
}

static void rk3288_hdmi_shutdown(struct platform_device *pdev)
{
	if (hdmi_dev) {
		#ifdef CONFIG_HAS_EARLYSUSPEND
		unregister_early_suspend(&hdmi_dev->early_suspend);
		#endif
	}
}

static struct platform_driver rk3288_hdmi_driver = {
	.probe		= rk3288_hdmi_probe,
	.remove		= rk3288_hdmi_remove,
	.driver		= {
		.name	= "rk3288-hdmi",
		.owner	= THIS_MODULE,
		#if defined(CONFIG_OF)
		.of_match_table = of_match_ptr(rk3288_hdmi_dt_ids),
		#endif
	},
	.shutdown   = rk3288_hdmi_shutdown,
};

static int __init rk3288_hdmi_init(void)
{
	return platform_driver_register(&rk3288_hdmi_driver);
}

static void __exit rk3288_hdmi_exit(void)
{
	platform_driver_unregister(&rk3288_hdmi_driver);
}

/* fs_initcall(rk3288_hdmi_init); */
device_initcall_sync(rk3288_hdmi_init);
module_exit(rk3288_hdmi_exit);