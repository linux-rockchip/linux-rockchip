#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#if defined(CONFIG_DEBUG_FS)
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#include "rk616_hdmi.h"
#include "rk616_hdmi_hw.h"

static struct rk616_hdmi *rk616_hdmi = NULL;

int rk616_hdmi_read(int addr)
{
	int value = 0;
	
	rk616_hdmi->rk616_drv->read_dev(rk616_hdmi->rk616_drv, (RK616_HDMI_BASE + ((addr)<<2)), &value);
	return value;
}

void rk616_hdmi_write(int addr, int val)
{
	rk616_hdmi->rk616_drv->write_dev(rk616_hdmi->rk616_drv, (RK616_HDMI_BASE + ((addr)<<2)), &val);
}

void rk616_hdmi_write_msk(u16 offset, u32 msk, u32 val)
{
    rk616_hdmi->rk616_drv->write_dev_bits(rk616_hdmi->rk616_drv, (RK616_HDMI_BASE + ((offset)<<2)), msk, &val);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void rk616_hdmi_early_suspend(struct early_suspend *h)
{
	struct rk616_hdmi *rk616_hdmi = container_of(h, struct rk616_hdmi, early_suspend);
	struct hdmi *hdmi = rk616_hdmi->hdmi;
	
	RK616DBG("hdmi enter early suspend \n");	
	hdmi_submit_work(hdmi, HDMI_SUSPEND_CTL, 0, NULL);
	return;
}

static void rk616_hdmi_early_resume(struct early_suspend *h)
{
	struct rk616_hdmi *rk616_hdmi = container_of(h, struct rk616_hdmi, early_suspend);
	struct hdmi *hdmi = rk616_hdmi->hdmi;
	
	RK616DBG("hdmi exit early resume\n");
	
	if(hdmi->ops->hdcp_power_on_cb)
		hdmi->ops->hdcp_power_on_cb();
	hdmi_submit_work(hdmi, HDMI_RESUME_CTL, 0, NULL);
}
#endif

static void rk616_hdmi_irq_work_func(struct work_struct *work)
{
	struct hdmi *hdmi = rk616_hdmi->hdmi;

	if(rk616_hdmi->enable) {
		rk616_hdmi_interrupt(hdmi);
		queue_delayed_work(rk616_hdmi->workqueue, &(rk616_hdmi->delay_work), msecs_to_jiffies(50));
	}
}


#if defined(CONFIG_DEBUG_FS)
static int rk616_hdmi_reg_show(struct seq_file *s, void *v)
{
	int i = 0;
	u32 val = 0;
	seq_printf(s, "\n>>>rk616_ctl reg");
	for (i = 0; i < 16; i++) {
		seq_printf(s, " %2x", i);
	}
	seq_printf(s, "\n-----------------------------------------------------------------");
	
	for(i=0; i<= PHY_PRE_DIV_RATIO; i++) {
                val = rk616_hdmi_read(i);
		if(i%16==0)
			seq_printf(s,"\n>>>rk616_ctl %2x:", i);
		seq_printf(s," %02x",val);

	}
	seq_printf(s, "\n-----------------------------------------------------------------\n");
	
	return 0;
}

static ssize_t rk616_hdmi_reg_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{ 
	u32 reg;
	u32 val;
	char kbuf[25];
	if (copy_from_user(kbuf, buf, count))
		return -EFAULT;
	sscanf(kbuf, "%x%x", &reg, &val);
    if ((reg < 0) || (reg > 0xed)) {
            dev_info(rk616_hdmi->hdmi->dev, "it is no hdmi reg\n");
            return count;
    }
	dev_info(rk616_hdmi->hdmi->dev, "/**********rk616 reg config******/");
	dev_info(rk616_hdmi->hdmi->dev, "\n reg=%x val=%x\n", reg, val);
    rk616_hdmi_write(reg, val);

	return count;
}

static int rk616_hdmi_reg_open(struct inode *inode, struct file *file)
{
	struct mfd_rk616 *rk616 = inode->i_private;
	return single_open(file,rk616_hdmi_reg_show,rk616);
}

static const struct file_operations rk616_hdmi_reg_fops = {
	.owner		= THIS_MODULE,
	.open		= rk616_hdmi_reg_open,
	.read		= seq_read,
	.write          = rk616_hdmi_reg_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static struct hdmi_property rk616_hdmi_property = {
	.videosrc = DISPLAY_SOURCE_LCDC1,
	.display = DISPLAY_MAIN,
};

static struct hdmi_ops rk616_hdmi_ops = {
	.enable = rk616_hdmi_enable,
	.disable = rk616_hdmi_disable,
	.getStatus = rk616_hdmi_detect_hotplug,
	.insert = rk616_hdmi_insert,
	.remove = rk616_hdmi_removed,
	.getEdid = rk616_hdmi_read_edid,
	.setVideo = rk616_hdmi_config_video,
	.setAudio = rk616_hdmi_config_audio,
	.setMute = rk616_hdmi_control_output,
//	.setCEC = rk616_hdmi_set_cec,
};

static int __devinit rk616_hdmi_probe (struct platform_device *pdev)
{
	int ret = -1;
	struct rkdisplay_platform_data *hdmi_data;
		
	RK616DBG("%s\n", __FUNCTION__);
	
	rk616_hdmi = kmalloc(sizeof(struct rk616_hdmi), GFP_KERNEL);
	if(!rk616_hdmi)
	{
    	dev_err(&pdev->dev, ">>rk30 hdmi kmalloc fail!");
    	return -ENOMEM;
	}
	memset(rk616_hdmi, 0, sizeof(struct rk616_hdmi));
	platform_set_drvdata(pdev, rk616_hdmi);
	
	rk616_hdmi->pwr_mode = NORMAL;
	
	rk616_hdmi->rk616_drv = dev_get_drvdata(pdev->dev.parent);
	if(rk616_hdmi->rk616_drv == NULL) {
		goto failed;
	}
	hdmi_data = rk616_hdmi->rk616_drv->pdata->pdata;
	if(hdmi_data == NULL) {
		goto failed;
	}
	// Register HDMI device
	if(hdmi_data) {
		rk616_hdmi_property.videosrc = hdmi_data->video_source;
		rk616_hdmi_property.display = hdmi_data->property;
	}
	
	rk616_hdmi_property.name = (char*)pdev->name;
	rk616_hdmi_property.priv = rk616_hdmi;
	rk616_hdmi->hdmi = hdmi_register(&rk616_hdmi_property, &rk616_hdmi_ops);
	if(rk616_hdmi->hdmi == NULL) {
		dev_err(&pdev->dev, "register hdmi device failed\n");
		ret = -ENOMEM;
		goto failed;
	}
		
	rk616_hdmi->hdmi->dev = &pdev->dev;
	rk616_hdmi->hdmi->xscale = 95;
	rk616_hdmi->hdmi->yscale = 95;
	rk616_hdmi->enable = 1;
	rk616_hdmi_initial(rk616_hdmi);
	
	#ifdef CONFIG_HAS_EARLYSUSPEND
	rk616_hdmi->early_suspend.suspend = rk616_hdmi_early_suspend;
	rk616_hdmi->early_suspend.resume = rk616_hdmi_early_resume;
	rk616_hdmi->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 10;
	register_early_suspend(&rk616_hdmi->early_suspend);
	#endif
	
	#if defined(CONFIG_DEBUG_FS)
	if(rk616_hdmi->rk616_drv && rk616_hdmi->rk616_drv->debugfs_dir) {
		debugfs_create_file("hdmi", S_IRUSR, rk616_hdmi->rk616_drv->debugfs_dir, rk616_hdmi->rk616_drv, &rk616_hdmi_reg_fops);
	} else {
	    rk616_hdmi->debugfs_dir = debugfs_create_dir("rk616", NULL);
	    if (IS_ERR(rk616_hdmi->debugfs_dir)) {
	            dev_err(rk616_hdmi->hdmi->dev,"failed to create debugfs dir for rk616!\n");
	    } else {
	            debugfs_create_file("hdmi", S_IRUSR, rk616_hdmi->debugfs_dir, rk616_hdmi, &rk616_hdmi_reg_fops);
	    }
	}
#endif
	{
		rk616_hdmi->workqueue = create_singlethread_workqueue("rk616 irq");
		INIT_DELAYED_WORK(&(rk616_hdmi->delay_work), rk616_hdmi_irq_work_func);
		rk616_hdmi_irq_work_func(NULL);
	}
	dev_info(&pdev->dev, "rk616 hdmi probe sucess.\n");
	return 0;
	
failed:
	if(rk616_hdmi) {
		kfree(rk616_hdmi);
		rk616_hdmi = NULL;
	}
	dev_err(&pdev->dev, "rk30 hdmi probe error.\n");
	return ret;
}

static int __devexit rk616_hdmi_remove(struct platform_device *pdev)
{
	
	printk(KERN_INFO "rk616 hdmi driver removed.\n");
	return 0;
}

static void rk616_hdmi_shutdown(struct platform_device *pdev)
{
	
}

static struct platform_driver rk616_hdmi_driver = {
	.probe		= rk616_hdmi_probe,
	.remove		= __devexit_p(rk616_hdmi_remove),
	.driver		= {
		.name	= "rk616-hdmi",
		.owner	= THIS_MODULE,
	},
	.shutdown   = rk616_hdmi_shutdown,
};

static int __init rk616_hdmi_init(void)
{
	return platform_driver_register(&rk616_hdmi_driver);
}

static void __exit rk616_hdmi_exit(void)
{
	platform_driver_unregister(&rk616_hdmi_driver);
}

fs_initcall(rk616_hdmi_init);
module_exit(rk616_hdmi_exit);