/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

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
#include <linux/rk_baseband.h>
#include <linux/firmware.h>

#include "rk_ipc.h"
#include "rk_ipcmsg_queue.h"



#define DRV_NAME				"rk_baseband"

#undef NV_USED_CMD_FS

/* IPCMSG_CMD_HARDWARE sub command */
#define IPC_HW_GPIO_SET_OUTPUT		__IPCMSG_SUBCMD_ID_HW_GPIO_SET_OUTPUT
#define IPC_HW_GPIO_SET_INPUT		__IPCMSG_SUBCMD_ID_HW_GPIO_SET_INPUT
#define IPC_HW_GPIO_SET_VALUE		__IPCMSG_SUBCMD_ID_HW_GPIO_SET_VALUE
#define IPC_HW_GPIO_GET_VALUE		__IPCMSG_SUBCMD_ID_HW_GPIO_GET_VALUE
#define IPC_HW_GPIO_SET_DEBOUNCE	__IPCMSG_SUBCMD_ID_HW_GPIO_SET_DEBOUNCE

#define IPC_HW_SIM_VCC_SELECT		__IPCMSG_SUBCMD_ID_HW_SIM_VCC_SELECT

#define IPC_HW_BB_SYSTEM_HALT		__IPCMSG_SUBCMD_ID_HW_BB_SYSTEM_HALT

/* IPCMSG_CMD_FS sub command */
#define IPC_FS_READFILE				__IPCMSG_SUBCMD_ID_FS_READFILE
#define IPC_FS_WRITEFILE			__IPCMSG_SUBCMD_ID_FS_WRITEFILE

enum {
	BB_STATUS_OFF = 0,
	BB_STATUS_ON,
};


struct rk_baseband {
	struct device *dev;

	struct rk_baseband_platform_data *pdata;

	void __iomem* fw_start;
	size_t		fw_max_sz;
	unsigned long status;

	char nv_path[PATH_MAX];
};


struct bb_boot_struct {
	u32 instr[4];
	u32 valid_flags;
#define BOOT_MODE_VALID_BIT		0x01
#define BB_MEM_VALID_BIT		0x02
#define IPC_MEM_VALID_BIT		0x04

	u32 boot_mode;
#define NORMAL_MODE				0xAB
#define CALI_MODE				0xCC

	u32 bb_mem_start;
	u32 bb_mem_length;
	u32 ipc_mem_start;
	u32 ipc_mem_length;
};

static int rk_baseband_startup(struct rk_baseband *bb, int mode)
{
	void __iomem* bootram;
	struct bb_boot_struct *boot_code;

	if (mode != NORMAL_MODE && mode != CALI_MODE) {
		mode = NORMAL_MODE;
	}

	bootram = ioremap(0xFDFF1000, PAGE_SIZE);
	if (bootram == NULL) {
		return -ENOMEM;
	}

	boot_code = (struct bb_boot_struct*)bootram;	
	boot_code->instr[0] = 0xe59ff010;	// LDR PC, [PC, #0x10] --> jump to bb_mem_start.
	boot_code->valid_flags = 0x7;
	boot_code->boot_mode = mode;
	boot_code->bb_mem_start = bb->pdata->bb_memphys;
	boot_code->bb_mem_length = bb->pdata->bb_memsz;
	boot_code->ipc_mem_start = bb->pdata->ipc_memphys;
	boot_code->ipc_mem_length = bb->pdata->ipc_memsz;

	iounmap(bootram);

	// TODO
	// turn baseband power and clock on
	// De-assert baseband reset
	writel(0x00200020,RK319X_BB_CRU_BASE+0x0110);
	mdelay(10);
	writel(0x00200000,RK319X_BB_CRU_BASE+0x0110);

	return 0;
}

static int rk_baseband_shutdown(struct rk_baseband *bb)
{
	// TODO
	// assert baseband reset
	// shutdown baseband power domain
	rk_ipc_reset();	  
	writel(0x00200020,RK319X_BB_CRU_BASE+0x0110);
	
	return 0;
}
static ssize_t firmware_loader_show(struct device *dev,
								struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev); 
	struct rk_baseband *bb = platform_get_drvdata(pdev);
	

	
	return sprintf(buf, "%d\n", bb->status == BB_STATUS_ON);
}

static ssize_t firmware_loader_store(struct device *dev,
								struct device_attribute *attr,
								const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev); 
	struct rk_baseband *bb = platform_get_drvdata(pdev);
	char fw_name[20];
	strcpy(fw_name,buf);
	fw_name[strcspn(buf, "\r\n")]='\0';
	if (fw_name != NULL)	
	{
		const struct firmware	*fw;
		int ret_val ;
		if(request_firmware(&fw, fw_name, dev)!=0)//"fw_rk3190_bb.bin"
			printk("BB fw name is err!!!\n");
		else{
			memcpy(bb->fw_start, fw->data, fw->size);
			printk("BB firmware fw_name=%s size=%d \n",fw_name,fw->size);
		}
		release_firmware(fw);
	}	
	else
	{
		printk("file name is null!!\n");
	}
	return count;
}

static DEVICE_ATTR(loader, 0644, firmware_loader_show, firmware_loader_store);

static ssize_t firmware_boot_show(struct device *dev,
								struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev); 
	struct rk_baseband *bb = platform_get_drvdata(pdev);

	return sprintf(buf, "%d\n", bb->status == BB_STATUS_ON);
}

static ssize_t firmware_boot_store(struct device *dev,
								struct device_attribute *attr,
								const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev); 
	struct rk_baseband *bb = platform_get_drvdata(pdev);
	int boot = simple_strtol(buf, NULL, 0);
	int ret;
	printk("%s:boot=0x%x\n",__func__,boot);
	/* 
	 * boot value :
	 *		0x0		- shutdown BB
	 *		0xAB 	- startup BB with normal mode.
	 *		0xCC	- startup BB with calibration mode.
	 */
	switch (boot) {
	case 0:
		ret = rk_baseband_shutdown(bb);
		bb->status = BB_STATUS_OFF;
		break;
	default:
		ret = rk_baseband_startup(bb, boot);
		bb->status = BB_STATUS_ON;
		break;
	}

	return count;
}

static DEVICE_ATTR(boot, 0644, firmware_boot_show, firmware_boot_store);

static ssize_t baseband_nvroot_show(struct device *dev,
								struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev); 
	struct rk_baseband *bb = platform_get_drvdata(pdev);

	return sprintf(buf, "%s\n", bb->nv_path);
}

static ssize_t baseband_nvroot_store(struct device *dev,
								struct device_attribute *attr,
								const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev); 
	struct rk_baseband *bb = platform_get_drvdata(pdev);

	strncpy(bb->nv_path, buf, PATH_MAX);

	return count;
}

static DEVICE_ATTR(nvroot, 0644, baseband_nvroot_show, baseband_nvroot_store);

static int handle_sim_vccsel_req(struct rk_baseband *bb, struct ipc_msg *pmsg)
{
	int ret = -1;

	if (bb->pdata && bb->pdata->sim_vcc_sel) {
		int simid;
		int sel;

	
		simid = (int)IPCMSG_GET_ARG(pmsg, 0);
		sel = (int)IPCMSG_GET_ARG(pmsg, 1);
		bb->pdata->sim_vcc_sel(simid, sel);

		return 0;
	}

	return ret;
}

static int handle_bb_system_halt(struct rk_baseband *bb, struct ipc_msg *pmsg)
{
	//TODO

	rk_ipc_reset();

	return 0;
}

static int handle_gpio_req(struct rk_baseband *bb, int req, struct ipc_msg *pmsg)
{
	int ret = -1;
	unsigned gpio;
	int value;

	
	if (IPCMSG_GET_ARGC(pmsg) < 1)
		return ret;

	/* argv[0] : gpio number. */
	gpio = (unsigned)IPCMSG_GET_ARG(pmsg, 0);

	switch (req) {
	case IPC_HW_GPIO_SET_OUTPUT:

		if (IPCMSG_GET_ARGC(pmsg) < 2)
			break;

		/* argv[1] : output value. */
		value = (int)IPCMSG_GET_ARG(pmsg, 1);
		ret = gpio_direction_output(gpio, value);
		break;

	case IPC_HW_GPIO_SET_INPUT:
		ret = gpio_direction_input(gpio);
		break;

	case IPC_HW_GPIO_SET_VALUE:
		if (IPCMSG_GET_ARGC(pmsg) < 2)
			break;

		/* argv[1] : output value. */
		value = (int)IPCMSG_GET_ARG(pmsg, 1);

		gpio_set_value(gpio, value);

		ret = 0;

		break;

	case IPC_HW_GPIO_GET_VALUE:
		value = gpio_get_value(gpio);

		ret = value;

		break;

	case IPC_HW_GPIO_SET_DEBOUNCE:
		if (IPCMSG_GET_ARGC(pmsg) < 2)
			break;

		/* argv[1] : debounce value. */
		value = (int)IPCMSG_GET_ARG(pmsg, 1);

		ret = gpio_set_debounce(gpio, value);
		break;
	}

	return ret;
}

static int hwreq_ipcmsg_handler(struct ipc_msg *pmsg, void *data)
{
	struct rk_baseband *bb;
	int cmd;
	int ret;

	bb = (struct rk_baseband *)data;

	cmd = pmsg->subcmd;
	IPC_DBG(DEBUG_INFO,"%s:cmd=%d",__func__,cmd);
	switch (cmd) {
	case IPC_HW_SIM_VCC_SELECT:
		ret = handle_sim_vccsel_req(bb, pmsg);
		break;

	case IPC_HW_GPIO_SET_OUTPUT ... IPC_HW_GPIO_SET_DEBOUNCE:
		ret = handle_gpio_req(bb, cmd, pmsg);
		break;

	case IPC_HW_BB_SYSTEM_HALT:
		ret = handle_bb_system_halt(bb, pmsg);
		break;
	default:
		break; 
	}
	
	return 0;
}

#ifdef NV_USED_CMD_FS
static int handle_file_read_req(struct rk_baseband *bb, struct ipc_msg *pmsg)
{
	int ret;
	const char* fname;
	loff_t offset;
	void *buf;
	size_t sz;
	char *path = __getname();
	struct file *file;

	/* argv[0] : relative file path */
	/* argv[1] : file offset */
	/* argv[2] : buffer pointer */
	/* argv[3] : length */

	if (IPCMSG_GET_ARGC(pmsg) != 4)
		return -1;

	fname = (const char*)IPCMSG_GET_ARG(pmsg, 0);
	offset = (loff_t)IPCMSG_GET_ARG(pmsg, 1);
	buf = (void*)IPCMSG_GET_ARG(pmsg, 2);
	sz = (void*)IPCMSG_GET_ARG(pmsg, 3);

	snprintf(path, PATH_MAX, "%s/%s", bb->nv_path, fname);
	file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file)) {
		return -1;
	}

	ret = kernel_read(file, offset, buf, sz);

	fput(file);
	__putname(path);

	return ret;
}

static int handle_file_write_req(struct rk_baseband *bb, struct ipc_msg *pmsg)
{
	int ret;
	const char* fname;
	loff_t offset;
	void *buf;
	size_t sz;
	char *path = __getname();
	struct file *file;

	/* argv[0] : relative file path */
	/* argv[1] : file offset */
	/* argv[2] : buffer pointer */
	/* argv[3] : length */

	if (IPCMSG_GET_ARGC(pmsg) != 4)
		return -1;

	fname = (const char*)IPCMSG_GET_ARG(pmsg, 0);
	offset = (loff_t)IPCMSG_GET_ARG(pmsg, 1);
	buf = (void*)IPCMSG_GET_ARG(pmsg, 2);
	sz = (size_t)IPCMSG_GET_ARG(pmsg, 3);

	snprintf(path, PATH_MAX, "%s/%s", bb->nv_path, fname);
	file = filp_open(path, O_RDWR, 0);
    if (IS_ERR(file)) {
		return -1;
	}

	ret = vfs_write(file, buf, sz, &offset);

	fput(file);
	__putname(path);

	return ret;
}

static int fsreq_ipcmsg_handler(struct ipc_msg *pmsg, void *data)
{
	struct rk_baseband *bb;
	int cmd;
	int ret = -1;

	bb = (struct rk_baseband *)data;

	cmd = pmsg->subcmd;

	switch (cmd) {
	case IPC_FS_READFILE:
		ret = handle_file_read_req(bb, pmsg);
		break;

	case IPC_FS_WRITEFILE:
		ret = handle_file_write_req(bb, pmsg);
		break;
	default:
		break; 
	}
	
	return ret;
}
#endif

static int create_ipc_serial(int nr_ports)
{
	int i;

	for (i=0; i<nr_ports; i++) {
		struct platform_device *pdev;

		pdev = platform_device_alloc("rk_ipc-serial", i);
		platform_device_add(pdev);
	}

	return 0;
}

static int rk_baseband_probe(struct platform_device *pdev)
{
	struct rk_baseband *bb;
	struct rk_baseband_platform_data *pdata = pdev->dev.platform_data;
	int nr_ipc_serial_ports;
	int ret;

	if (pdata == NULL) {
		dev_err(&pdev->dev, "No provide platform data\n");
		return -EINVAL;
	}

	// basic check
	if (pdata->bb_memsz <= 0 || pdata->ipc_memsz <= 0) {
		return -EINVAL;
	}

	bb = kzalloc(sizeof(struct rk_baseband), GFP_KERNEL);
	if (unlikely(bb == NULL)) {
		dev_err(&pdev->dev, "Unable to malloc rk_baseband\n");
		return -ENOMEM;
	}

	bb->dev = &pdev->dev;
	bb->pdata = pdata;
	
	platform_set_drvdata(pdev, bb);
	
	bb->fw_start = ioremap(pdata->bb_memphys, pdata->bb_memsz);
	if (bb->fw_start == NULL) {
		return -ENOMEM;
	}

	bb->fw_max_sz = pdata->bb_memsz;

	// init IPC system
	ret = rk_ipc_init(pdata->ipc_memphys, pdata->ipc_memsz);
	if (ret) {
		dev_err(&pdev->dev, "Init rk-ipc failed\n");
		iounmap(bb->fw_start);
		kfree(bb);
		return -ENODEV;
	}

	// add ipc serial port
	nr_ipc_serial_ports = min_t(int, pdata->nr_serial_ports, __IPC_PIPE_NUM_MAX);
	if (nr_ipc_serial_ports > 0) 
		create_ipc_serial(nr_ipc_serial_ports);

	ipc_msg_register_handler(IPCMSG_CMD_HARDWARE, hwreq_ipcmsg_handler, bb);
#ifdef NV_USED_CMD_FS
	ipc_msg_register_handler(IPCMSG_CMD_FS, fsreq_ipcmsg_handler, bb);
#endif

	device_create_file(&pdev->dev, &dev_attr_loader);
	device_create_file(&pdev->dev, &dev_attr_boot);
	device_create_file(&pdev->dev, &dev_attr_nvroot);

	return 0;
}

static struct platform_driver rk_baseband_driver = {
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};


static int __init rk_baseband_init(void)
{
	return platform_driver_probe(&rk_baseband_driver, rk_baseband_probe);
}


static void __exit rk_baseband_exit(void)
{
	platform_driver_unregister(&rk_baseband_driver);

}

module_init(rk_baseband_init);
module_exit(rk_baseband_exit);

MODULE_AUTHOR("rock-chips");
MODULE_DESCRIPTION("rock-chips Baseband driver");
MODULE_LICENSE("GPL");

