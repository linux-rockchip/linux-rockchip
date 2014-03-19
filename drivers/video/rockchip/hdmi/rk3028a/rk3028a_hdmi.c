#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <mach/board.h>
#include <mach/io.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include "rk3028a_hdmi.h"
#include "rk3028a_hdmi_hw.h"
#include "rk3028a_hdmi_cec.h"

static struct rk3028a_hdmi *rk3028a_hdmi = NULL;

extern irqreturn_t hdmi_irq(int irq, void *priv);

struct hdmi* rk3028a_hdmi_register_hdcp_callbacks(void (*hdcp_cb)(void),
					 void (*hdcp_irq_cb)(int status),
					 int (*hdcp_power_on_cb)(void),
					 void (*hdcp_power_off_cb)(void))
{
	struct hdmi *hdmi = rk3028a_hdmi->hdmi;
	
	hdmi->ops->hdcp_cb = hdcp_cb;
	hdmi->ops->hdcp_irq_cb = hdcp_irq_cb;
	hdmi->ops->hdcp_power_on_cb = hdcp_power_on_cb;
	hdmi->ops->hdcp_power_off_cb = hdcp_power_off_cb;
	return hdmi;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void rk3028a_hdmi_early_suspend(struct early_suspend *h)
{
	struct rk3028a_hdmi *rk3028a_hdmi = container_of(h, struct rk3028a_hdmi, early_suspend);
	struct hdmi *hdmi = rk3028a_hdmi->hdmi;
	
	RK3028ADBG("hdmi enter early suspend \n");
	
//	rk30_mux_api_set(GPIO0A7_I2C3_SDA_HDMI_DDCSDA_NAME, GPIO0A_GPIO0A7);
//	rk30_mux_api_set(GPIO0A6_I2C3_SCL_HDMI_DDCSCL_NAME, GPIO0A_GPIO0A6);
	
	hdmi_submit_work(hdmi, HDMI_SUSPEND_CTL, 0, NULL);
	return;
}

static void rk3028a_hdmi_early_resume(struct early_suspend *h)
{
	struct rk3028a_hdmi *rk3028a_hdmi = container_of(h, struct rk3028a_hdmi, early_suspend);
	struct hdmi *hdmi = rk3028a_hdmi->hdmi;
	
	RK3028ADBG("hdmi exit early resume\n");

//	rk30_mux_api_set(GPIO0A7_I2C3_SDA_HDMI_DDCSDA_NAME, GPIO0A_HDMI_DDCSDA);
//	rk30_mux_api_set(GPIO0A6_I2C3_SCL_HDMI_DDCSCL_NAME, GPIO0A_HDMI_DDCSCL);
	
	clk_enable(rk3028a_hdmi->hclk);

	if(hdmi->ops->hdcp_power_on_cb)
		hdmi->ops->hdcp_power_on_cb();
	hdmi_submit_work(hdmi, HDMI_RESUME_CTL, 0, NULL);
}
#endif

int rk3028a_hdmi_set_cec(struct hdmi *hdmi)
{
#ifdef CONFIG_CEC_RK3028A
	CecSetDevicePA(hdmi->edid.cecaddress);
	CecEnumerate();
#endif
	return HDMI_ERROR_SUCESS;
}

static struct hdmi_property rk3028a_hdmi_property = {
	.videosrc = DISPLAY_SOURCE_LCDC1,
	.display = DISPLAY_MAIN,
};

static struct hdmi_ops rk3028a_hdmi_ops = {
	.enable = rk3028a_hdmi_enable,
	.disable = rk3028a_hdmi_disable,
	.getStatus = rk3028a_hdmi_detect_hotplug,
	.insert = rk3028a_hdmi_insert,
	.remove = rk3028a_hdmi_removed,
	.getEdid = rk3028a_hdmi_read_edid,
	.setVideo = rk3028a_hdmi_config_video,
	.setAudio = rk3028a_hdmi_config_audio,
	.setMute = rk3028a_hdmi_control_output,
	.setCEC = rk3028a_hdmi_set_cec,
};

static int __devinit rk3028a_hdmi_probe (struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct resource *mem;
	
	rk3028a_hdmi = kmalloc(sizeof(struct rk3028a_hdmi), GFP_KERNEL);
	if(!rk3028a_hdmi)
	{
    	dev_err(&pdev->dev, ">>rk30 hdmi kmalloc fail!");
    	return -ENOMEM;
	}
	memset(rk3028a_hdmi, 0, sizeof(struct rk3028a_hdmi));
	platform_set_drvdata(pdev, rk3028a_hdmi);
	
	rk3028a_hdmi->pwr_mode = NORMAL;
	
	/* get the IRQ */
	rk3028a_hdmi->irq = platform_get_irq(pdev, 0);
	if(rk3028a_hdmi->irq <= 0) {
		dev_err(&pdev->dev, "failed to get hdmi irq resource (%d).\n", rk3028a_hdmi->irq);
		ret = -ENXIO;
		goto err0;
	}
	
	rk3028a_hdmi->hclk = clk_get(NULL,"pclk_hdmi");
	if(IS_ERR(rk3028a_hdmi->hclk))
	{
		dev_err(&pdev->dev, "Unable to get hdmi hclk\n");
		ret = -ENXIO;
		goto err0;
	}
	clk_enable(rk3028a_hdmi->hclk);
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Unable to get register resource\n");
		ret = -ENXIO;
		goto err0;
	}
	rk3028a_hdmi->regbase_phy = res->start;
	rk3028a_hdmi->regsize_phy = (res->end - res->start) + 1;
	mem = request_mem_region(res->start, (res->end - res->start) + 1, pdev->name);
	if (!mem)
	{
    	dev_err(&pdev->dev, "failed to request mem region for hdmi\n");
    	ret = -ENOENT;
    	goto err0;
	}
	
	rk3028a_hdmi->regbase = (int)ioremap(res->start, (res->end - res->start) + 1);
	if (!rk3028a_hdmi->regbase) {
		dev_err(&pdev->dev, "cannot ioremap registers\n");
		ret = -ENXIO;
		goto err1;
	}
	
	#ifdef CONFIG_HAS_EARLYSUSPEND
	rk3028a_hdmi->early_suspend.suspend = rk3028a_hdmi_early_suspend;
	rk3028a_hdmi->early_suspend.resume = rk3028a_hdmi_early_resume;
	rk3028a_hdmi->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 10;
	register_early_suspend(&rk3028a_hdmi->early_suspend);
	#endif
	
	rk3028a_hdmi_property.name = (char*)pdev->name;
	rk3028a_hdmi_property.priv = rk3028a_hdmi;
	rk3028a_hdmi->hdmi = hdmi_register(&rk3028a_hdmi_property, &rk3028a_hdmi_ops);
	if(rk3028a_hdmi->hdmi == NULL) {
		dev_err(&pdev->dev, "register hdmi device failed\n");
		ret = -ENOMEM;
		goto err2;
	}
		
	rk3028a_hdmi->hdmi->dev = &pdev->dev;
	rk3028a_hdmi->hdmi->xscale = 95;
	rk3028a_hdmi->hdmi->yscale = 95;
	rk3028a_hdmi->enable = 1;
	rk3028a_hdmi_initial(rk3028a_hdmi);

	iomux_set(HDMI_DDC_SDA);
    iomux_set(HDMI_DDC_SCL);
    iomux_set(HDMI_HOTPLUGIN);
    
	#ifdef CONFIG_CEC_RK3028A		
	iomux_set(HDMI_CECSDA);	
	CecInit(rk3028a_hdmi->hdmi);
	#endif
	
	spin_lock_init(&rk3028a_hdmi->irq_lock);
	
	/* request the IRQ */
	ret = request_irq(rk3028a_hdmi->irq, hdmi_irq, 0, dev_name(&pdev->dev), rk3028a_hdmi);
	if (ret)
	{
		dev_err(&pdev->dev, "rk30 hdmi request_irq failed (%d).\n", ret);
		goto err3;
	}

	dev_info(&pdev->dev, "rk30 hdmi probe sucess.\n");
	return 0;
	
err3:
	hdmi_unregister(rk3028a_hdmi->hdmi);
err2:
	#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&rk3028a_hdmi->early_suspend);
	#endif
	iounmap((void*)rk3028a_hdmi->regbase);
err1:
	release_mem_region(res->start,(res->end - res->start) + 1);
	clk_disable(rk3028a_hdmi->hclk);
err0:
	kfree(rk3028a_hdmi);
	rk3028a_hdmi = NULL;
	dev_err(&pdev->dev, "rk30 hdmi probe error.\n");
	return ret;
}

static int __devexit rk3028a_hdmi_remove(struct platform_device *pdev)
{
	
	printk(KERN_INFO "rk3028a hdmi removed.\n");
	return 0;
}

static void rk3028a_hdmi_shutdown(struct platform_device *pdev)
{
	struct rk3028a_hdmi *rk3028a_hdmi = platform_get_drvdata(pdev);
	struct hdmi *hdmi = rk3028a_hdmi->hdmi;
	if(hdmi) {
		#ifdef CONFIG_HAS_EARLYSUSPEND
		unregister_early_suspend(&rk3028a_hdmi->early_suspend);
		#endif
		if(!hdmi->sleep && hdmi->enable)
			disable_irq(rk3028a_hdmi->irq);
	}
	printk(KERN_INFO "rk3028a hdmi shut down.\n");
}

static struct platform_driver rk3028a_hdmi_driver = {
	.probe		= rk3028a_hdmi_probe,
	.remove		= __devexit_p(rk3028a_hdmi_remove),
	.driver		= {
		.name	= "rk3028a-hdmi",
		.owner	= THIS_MODULE,
	},
	.shutdown   = rk3028a_hdmi_shutdown,
};

static int __init rk3028a_hdmi_init(void)
{
    return platform_driver_register(&rk3028a_hdmi_driver);
}

static void __exit rk3028a_hdmi_exit(void)
{
    platform_driver_unregister(&rk3028a_hdmi_driver);
}


fs_initcall(rk3028a_hdmi_init);
//module_init(rk3028a_hdmi_init);
module_exit(rk3028a_hdmi_exit);
