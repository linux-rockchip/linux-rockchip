#ifndef _RK_IPC_H
#define _RK_IPC_H

#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/gpio.h>

struct rk_baseband_platform_data {
	void (*sim_vcc_sel)(int sim_id, int vccsel);		/* vccsel - 0 : off, 1 : 1.8v, 2 : 3.0v */
	phys_addr_t fw_phys_start;	/* BB firmware running start address */
	size_t		fw_max_sz;	/* BB firmware running space size */
};

#endif
