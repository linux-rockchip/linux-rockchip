/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <plat/efuse.h>

#if defined(CONFIG_ARCH_RK3188) || defined(CONFIG_SOC_RK3028)
#define efuse_readl(offset)		readl_relaxed(RK30_EFUSE_BASE + offset)
#define efuse_writel(val, offset)	writel_relaxed(val, RK30_EFUSE_BASE + offset)
#endif

u8 efuse_buf[32 + 1] = {0, 0};

static int efuse_readregs(u32 addr, u32 length, u8 *buf)
{
#ifndef efuse_readl
	return 0;
#else
	unsigned long flags;
	static DEFINE_SPINLOCK(efuse_lock);
	int ret = length;

	if (!length)
		return 0;

	spin_lock_irqsave(&efuse_lock, flags);

	efuse_writel(EFUSE_CSB, REG_EFUSE_CTRL);
	efuse_writel(EFUSE_LOAD | EFUSE_PGENB, REG_EFUSE_CTRL);
	udelay(2);
	do {
		efuse_writel(efuse_readl(REG_EFUSE_CTRL) & (~(EFUSE_A_MASK << EFUSE_A_SHIFT)), REG_EFUSE_CTRL);
		efuse_writel(efuse_readl(REG_EFUSE_CTRL) | ((addr & EFUSE_A_MASK) << EFUSE_A_SHIFT), REG_EFUSE_CTRL);
		udelay(2);
		efuse_writel(efuse_readl(REG_EFUSE_CTRL) | EFUSE_STROBE, REG_EFUSE_CTRL);
		udelay(2);
		*buf = efuse_readl(REG_EFUSE_DOUT);
		efuse_writel(efuse_readl(REG_EFUSE_CTRL) & (~EFUSE_STROBE), REG_EFUSE_CTRL);
		udelay(2);
		buf++;
		addr++;
	} while(--length);
	udelay(2);
	efuse_writel(efuse_readl(REG_EFUSE_CTRL) | EFUSE_CSB, REG_EFUSE_CTRL);
	udelay(1);

	spin_unlock_irqrestore(&efuse_lock, flags);
	return ret;
#endif
}

void rk_efuse_init(void)
{
	efuse_readregs(0x0, 32, efuse_buf);
}

int rk_pll_flag(void)
{
	return (efuse_buf[22] & 0x3) | (efuse_buf[22] & (0x1 << 3));	
}
int rk_tflag(void)
{
	return efuse_buf[22] & (0x1 << 3);
}

int rk_leakage_val(void)
{
	/*
	 * efuse_buf[22]
	 * bit[3]:
	 * 	0:enable leakage level auto voltage scale
	 * 	1:disalbe leakage level avs
	 */
	if ((efuse_buf[22] >> 2) & 0x1)
		return 0;
	else
		return  (efuse_buf[22] >> 4) & 0x0f;
}

int rk3028_version_val(void)
{
	return efuse_buf[5];
}

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#define EFUSE_IOCTL_MAGIC                         'M'

#define EFUSE_DECRYPT                         _IOW(EFUSE_IOCTL_MAGIC, 0x00, int)
#define EFUSE_ECRYPT                          _IOW(EFUSE_IOCTL_MAGIC, 0x01, int)

static long proc_efuse_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
       int ret = 0,i;
       u8 *num = (u8 *)arg;
       switch (cmd) {
               case EFUSE_DECRYPT: {
                       for(i=0; i<16; i++){
                       num[i] =  efuse_buf[i+6];
       }
                       
               } break;
               case EFUSE_ECRYPT: {
                       
               } break;
               
               default: {
               
               } break;
       }

       return ret;
} 
static const struct file_operations proc_efuse_fops = {
       .unlocked_ioctl = proc_efuse_ioctl,
};
 
static int __init codec_proc_init (void)
{
       proc_create ("efuse", 0, NULL, &proc_efuse_fops);
       return 0;
}
late_initcall (codec_proc_init);
#endif /* CONFIG_PROC_FS */
