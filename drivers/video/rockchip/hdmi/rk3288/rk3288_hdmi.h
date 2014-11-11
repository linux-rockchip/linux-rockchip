#ifndef __RK32_HDMI_H__
#define __RK32_HDMI_H__

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include "../rk_hdmi.h"

#ifdef DEBUG
#define HDMIDBG(format, ...) \
		printk(KERN_INFO "RK32 HDMI: " format "\n", ## __VA_ARGS__)
#else
#define HDMIDBG(format, ...)
#endif

enum {
	INPUTIIS = 0,
	INPUTSPIDF
};

struct hdmi_dev {
	void __iomem		*regbase;
	int			regbase_phy;
	int			regsize_phy;
	struct clk		*pd;
	struct clk		*pclk;
	struct clk		*hdcp_clk;
	struct clk		*cec_clk;
	struct hdmi		*hdmi;
	struct device		*dev;
	struct dentry		*debugfs_dir;
	int			irq;

	struct work_struct	irq_work;
	struct delayed_work	delay_work;
	struct workqueue_struct *workqueue;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
	int			audiosrc;
	int			enable;
	unsigned char		clk_disable;
	unsigned char		clk_on;

	unsigned long		pixelclk;
	unsigned int		tmdsclk;
	unsigned int		pixelrepeat;
	unsigned char		colordepth;

	bool			tmdsclk_ratio_change;
	struct mutex		ddc_lock;
};
#endif /*__RK32_HDMI_H__*/
