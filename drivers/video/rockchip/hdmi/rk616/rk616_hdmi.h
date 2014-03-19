#ifndef __RK616_HDMI_H__
#define __RK616_HDMI_H__

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/mfd/rk616.h>
#include "../rk_hdmi.h"

#ifdef DEBUG
#define RK616DBG(format, ...) \
		printk(KERN_INFO "RK616 HDMI: " format "\n", ## __VA_ARGS__)
#else
#define RK616DBG(format, ...)
#endif

struct rk616_hdmi {
	struct mfd_rk616	*rk616_drv;
	struct hdmi			*hdmi;
	struct dentry		*debugfs_dir;
	int irq;
	int io_irq_pin;
	int io_pwr_pin;
	int io_rst_pin;
	int			pwr_mode;
	unsigned long tmdsclk;
	struct work_struct	irq_work;
	struct delayed_work delay_work;
	struct workqueue_struct *workqueue;
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
	int enable;
};

extern int rk616_hdmi_read(int addr);
extern void rk616_hdmi_write(int addr, int val);
extern void rk616_hdmi_write_msk(u16 offset, u32 msk, u32 val);
#endif //__RK616_HDMI_H__
