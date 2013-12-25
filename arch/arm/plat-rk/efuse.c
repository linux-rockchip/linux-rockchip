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
#include <linux/string.h>

#if defined(CONFIG_ARCH_RK3188) || defined(CONFIG_SOC_RK3028) || defined(CONFIG_ARCH_RK3066B)
#define efuse_readl(offset)		readl_relaxed(RK30_EFUSE_BASE + offset)
#define efuse_writel(val, offset)	writel_relaxed(val, RK30_EFUSE_BASE + offset)
#endif

#if defined(CONFIG_ARCH_RK3026)
#define efuse_readl(offset)             readl_relaxed(RK2928_EFUSE_BASE + offset)
#define efuse_writel(val, offset)       writel_relaxed(val, RK2928_EFUSE_BASE + offset)
#endif

u8 efuse_buf[32 + 1] = {0, 0};
static char efuse_val[65];

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
	int i=0;
	char temp[3];
#if defined(CONFIG_ARCH_RK3026)
	u8 tmp_buf[32];
	int j, err = 0;

	efuse_readregs(0x0, 32, efuse_buf);

	/*
	 * i = 10,	need time = 2,860,875	ns
	 * i = 100,	need time = 27,327,000	ns
	 */
	for (i = 0; i < 10; i++){
		efuse_readregs(0x0, 32, tmp_buf);
		for (j = 0; j < 32; j++){
			if (efuse_buf[j] != tmp_buf[j]){
				printk(KERN_WARNING ":%s:rk3026 efuse bit err\n", __func__);
				efuse_readregs(0x0, 32, efuse_buf);
				i = 0;
				err++;
				break;
			}
		}

		if (err >= 500) {
			printk(KERN_ERR "%s:rk3026 get efuse err\n", __func__);
			efuse_buf[5] = 0x00;	/* set default SOC version */
			efuse_buf[22] = 0x00;	/* clean msg about leakage */
			break;
		}
	}
#else
	efuse_readregs(0x0, 32, efuse_buf);
#endif
	for(i=0;i<32;i++){
		sprintf(temp,"%02x",efuse_buf[i]);
		strcat(efuse_val,temp);
	}

}

int rk_pll_flag(void)
{
	return efuse_buf[22] & 0x3;
}
int rk_tflag(void)
{
	return efuse_buf[22] & (0x1 << 3);
}

int efuse_version_val(void)
{
	int ret = efuse_buf[4] & (~(0x1 << 3));
	printk("%s: efuse version = %02x\n", __func__, ret);
	return ret;
}

int rk_leakage_val(void)
{
	/*
	 * efuse_buf[22]
	 * bit[2]:
	 * 	0:enable leakage level auto voltage scale
	 * 	1:disalbe leakage level avs
	 */
	
	int leakage_level = 0;
	int leakage_val = 0;
	int efuse_version = efuse_version_val();

	if ((efuse_buf[22] >> 2) & 0x1){
		return 0;
	} else {
#if defined(CONFIG_ARCH_RK3066B) || defined(CONFIG_ARCH_RK3026)
		if (efuse_version == 0x31){
			leakage_level = ((efuse_buf[22] >> 4) & 0x0f);
			if (leakage_level)
				leakage_val = 1500 * (leakage_level - 1);
			else
				return 0;
		} else if (efuse_version == 0x41){
			leakage_level = ((efuse_buf[22] >> 4) & 0x0f) + ((efuse_buf[23] & 0x03) << 4);
			if (leakage_level)
				leakage_val = 1000 * (leakage_level - 1);
			else
				return 0;
		} else {
			return 0;
		}

		leakage_val = leakage_val < 1000 ? 1000 : leakage_val;
#elif defined(CONFIG_ARCH_RK3188)
		leakage_level = (efuse_buf[22] >> 4) & 0x0f;
		if (leakage_level){
			if (leakage_level <= 10)
				leakage_val = 3000 * (leakage_level - 1);
			else {
				leakage_val = 30000 + 5000 * (leakage_level - 1 - 10);
			}
			if (efuse_version == 0x51){
				leakage_val += 1000 * ((efuse_buf[23] & 0x3) + 1);
			}
		} else {
			return 0;
		}

		leakage_val = leakage_val < 1000 ? 1000 : leakage_val;
#endif
		return leakage_val;
	}
}

int rk3028_version_val(void)
{
	return efuse_buf[5];
}

int rk3026_version_val(void)
{
	if (efuse_buf[24]){
		return efuse_buf[24];
	} else {
		return efuse_buf[5];
	}
}

char *rk_efuse_value(void)
{
	return efuse_val;
}
