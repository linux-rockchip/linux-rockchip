#ifndef __RK3028A_HDMI_H__
#define __RK3028A_HDMI_H__

#include <linux/clk.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include "../rk_hdmi.h"

#ifdef DEBUG
#define RK3028ADBG(format, ...) \
		printk(KERN_INFO "RK3028A HDMI: " format "\n", ## __VA_ARGS__)
#else
#define RK3028ADBG(format, ...)
#endif

extern struct hdmi* rk3028a_hdmi_register_hdcp_callbacks(
					 void (*hdcp_cb)(void),
					 void (*hdcp_irq_cb)(int status),
					 int  (*hdcp_power_on_cb)(void),
					 void (*hdcp_power_off_cb)(void));
					 
struct rk3028a_hdmi {
	struct clk		*hclk;				//HDMI AHP clk
	int 			regbase;
	int				irq;
	int				regbase_phy;
	int				regsize_phy;
	struct hdmi		*hdmi;
	
	int				pwr_mode;
	
	int				tmdsclk;
	int				enable;
	spinlock_t		irq_lock;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
};

#endif /* __RK3028A_HDMI_H__ */
