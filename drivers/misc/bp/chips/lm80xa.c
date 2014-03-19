/* drivers/misc/bp/chips/lm80xa.c
 *
 * Copyright (C) 2012-2015 ROCKCHIP.
 * Author: luowei <lw@rock-chips.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/circ_buf.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <mach/iomux.h>
#include <mach/gpio.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>

#include <linux/bp-auto.h>
	 
	 
#if 1
#define DBG(x...)  printk(x)

#else
#define DBG(x...)
#endif
//struct class *modem_class = NULL; 
//static int modem_status;
/*
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
static ssize_t modem_status_read(struct class *cls, struct class_attribute *attr, char *_buf)
#else
static ssize_t modem_status_read(struct class *cls, char *_buf)
#endif
{

	return sprintf(_buf, "%d\n", modem_status);
	
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
static ssize_t modem_status_write(struct class *cls, struct class_attribute *attr, const char *_buf, size_t _count)
#else
static ssize_t modem_status_write(struct class *cls, const char *_buf, size_t _count)
#endif
{
    int new_state = simple_strtoul(_buf, NULL, 16);
   if(new_state == modem_status) return _count;
   if (new_state == 1){
     printk("%s, c(%d), open modem \n", __FUNCTION__, new_state);
	 //modem_poweron_off(1);
	 gpio_set_value(RK30_PIN6_PB2, GPIO_HIGH);
   }else if(new_state == 0){
     printk("%s, c(%d), close modem \n", __FUNCTION__, new_state);
	 //modem_poweron_off(0);
	 gpio_set_value(RK30_PIN6_PB2, GPIO_LOW);
   }else{
     printk("%s, invalid parameter \n", __FUNCTION__);
   }
	modem_status = new_state;
    return _count; 
}
static CLASS_ATTR(modem_status, 0777, modem_status_read, modem_status_write);
*/

/****************operate according to bp chip:start************/
static int bp_active(struct bp_private_data *bp, int enable)
{		
	printk("<-----lm80xa bp_active-------->\n");
	if(enable)
	{
		printk("<-----lm80xa bp_active---enable---HIGH-->\n");
        //gpio_set_value(bp->ops->bp_power, GPIO_HIGH);
        gpio_set_value(bp->ops->bp_power, GPIO_HIGH);
        gpio_set_value(bp->ops->ap_wakeup_bp, GPIO_HIGH);
	}
	else
	{
		printk("<-----lm80xa bp_active---disenable----->\n");
       // gpio_set_value(bp->ops->bp_power, GPIO_LOW);
       gpio_set_value(bp->ops->bp_power, GPIO_LOW);
       gpio_set_value(bp->ops->ap_wakeup_bp, GPIO_LOW);
	}
	
	return 0;
}

static int ap_wake_bp(struct bp_private_data *bp, int wake)
{
	return 0;

}

static void  ap_wake_bp_work(struct work_struct *work)
{
}


static int bp_init(struct bp_private_data *bp)
{
	printk("<-----lm80xa bp_init-------->\n");
	gpio_direction_output(bp->ops->bp_power, GPIO_LOW);
//	gpio_set_value(bp->ops->bp_power, GPIO_HIGH);
//	msleep(500);
	return 0;
}

static int bp_reset(struct bp_private_data *bp)
{
	return 0;
}

static int bp_wake_ap(struct bp_private_data *bp)
{
    printk("-------------------bp_wakeup_ap---------------\n");
    wake_lock_timeout(&bp->bp_wakelock, 10* HZ);
	return 0;
}


static int bp_shutdown(struct bp_private_data *bp)
{
	int result = 0;
	gpio_set_value(bp->ops->bp_power, GPIO_LOW);
	return result;
}



static int bp_suspend(struct bp_private_data *bp)
{

    printk("----------------bp_suspend-----------\n");
    gpio_set_value(bp->ops->ap_wakeup_bp,GPIO_LOW);
	return 0;
}




static int bp_resume(struct bp_private_data *bp)
{
    printk("----------------bp_resume-----------\n");
    gpio_set_value(bp->ops->ap_wakeup_bp,GPIO_HIGH);
	return 0;
}


struct bp_operate bp_lm80xa_ops = {
#if defined(CONFIG_ARCH_RK2928)
	.name			= "lm80xa",
	.bp_id			= BP_ID_LM80xA,
	.bp_bus			= BP_BUS_TYPE_USB_UART,		
	.bp_pid			= 0,	
	.bp_vid			= 0,	
	.bp_power		= RK30_PIN0_PC3,//RK2928_PIN3_PC2, 	// 3g_power
	.bp_en			= BP_UNKNOW_DATA,	// 3g_en
	.bp_reset			= RK2928_PIN1_PA3,
	.ap_ready		= BP_UNKNOW_DATA,	//
	.bp_ready		= BP_UNKNOW_DATA,
	.ap_wakeup_bp		= RK30_PIN0_PC4,
	.bp_wakeup_ap		= RK30_PIN0_PC5,	//
	.bp_uart_en		= BP_UNKNOW_DATA, 	//EINT9
	.bp_usb_en		= BP_UNKNOW_DATA, 	//W_disable
	.trig			= IRQF_TRIGGER_RISING,// IRQF_TRIGGER_FALLING,

	.active			= bp_active,
	.init			= bp_init,
	.reset			= bp_reset,
	.ap_wake_bp		= ap_wake_bp,
	.bp_wake_ap		= bp_wake_ap,
	.shutdown		= bp_shutdown,
	.read_status		= NULL,
	.write_status		= NULL,
	.suspend 		= bp_suspend,
	.resume			= bp_resume,
	.misc_name		= NULL,
	.private_miscdev	= NULL,
#elif defined(CONFIG_ARCH_RK30)
	.name			= "lm80xa",
	.bp_id			= BP_ID_LM80xA,
	.bp_bus			= BP_BUS_TYPE_USB_UART,		
	.bp_pid			= 0,	
	.bp_vid			= 0,	
	.bp_power		= RK30_PIN6_PB2, 	// 3g_power
	.bp_en			= BP_UNKNOW_DATA,	// 3g_en
	.bp_reset			= BP_UNKNOW_DATA,
	.ap_ready		= BP_UNKNOW_DATA,	//
	.bp_ready		= BP_UNKNOW_DATA,
	.ap_wakeup_bp		= BP_UNKNOW_DATA,
	.bp_wakeup_ap		= BP_UNKNOW_DATA,	//
	.bp_uart_en		= BP_UNKNOW_DATA, 	//EINT9
	.bp_usb_en		= BP_UNKNOW_DATA, 	//W_disable
	.trig			= IRQF_TRIGGER_RISING,

	.active			= bp_active,
	.init			= bp_init,
	.reset			= bp_reset,
	.ap_wake_bp		= ap_wake_bp,
	.bp_wake_ap		= bp_wake_ap,
	.shutdown		= bp_shutdown,
	.read_status		= NULL,
	.write_status		= NULL,
	.suspend 		= bp_suspend,
	.resume			= bp_resume,
	.misc_name		= NULL,
	.private_miscdev	= NULL,
#elif defined(CONFIG_ARCH_RK3188)
	.name			= "lm80xa",
	.bp_id			= BP_ID_LM80xA,
	.bp_bus			= BP_BUS_TYPE_USB_UART,		
	.bp_pid			= 0,	
	.bp_vid			= 0,	
	.bp_power		= RK30_PIN0_PC6,//RK2928_PIN3_PC2, 	// 3g_power // lgh -- 0c6
	.bp_en			= BP_UNKNOW_DATA,	// 3g_en
	.bp_reset			= RK30_PIN0_PC3,//RK2928_PIN1_PA3, // lgh -- 0c3
	.ap_ready		= BP_UNKNOW_DATA,	//
	.bp_ready		= BP_UNKNOW_DATA,
	.ap_wakeup_bp		= RK30_PIN0_PC4,//
	.bp_wakeup_ap		= RK30_PIN0_PC5,//
	.bp_uart_en		= BP_UNKNOW_DATA, 	//EINT9
	.bp_usb_en		= BP_UNKNOW_DATA, 	//W_disable
	.trig			= IRQF_TRIGGER_RISING,

	.active			= bp_active,
	.init			= bp_init,
	.reset			= bp_reset,
	.ap_wake_bp		= ap_wake_bp,
	.bp_wake_ap		= bp_wake_ap,
	.shutdown		= bp_shutdown,
	.read_status		= NULL,
	.write_status		= NULL,
	.suspend 		= bp_suspend,
	.resume			= bp_resume,
	.misc_name		= NULL,
	.private_miscdev	= NULL,
#else
	.name			= "lm80xa",
	.bp_id			= BP_ID_LM80xA,
	.bp_bus			= BP_BUS_TYPE_USB_UART,		
	.bp_pid			= 0,	
	.bp_vid			= 0,	
	.bp_power		= BP_UNKNOW_DATA,//RK2928_PIN3_PC2, 	// 3g_power
	.bp_en			= BP_UNKNOW_DATA,	// 3g_en
	.bp_reset			= BP_UNKNOW_DATA,//RK2928_PIN1_PA3,
	.ap_ready		= BP_UNKNOW_DATA,	//
	.bp_ready		= BP_UNKNOW_DATA,
	.ap_wakeup_bp		= BP_UNKNOW_DATA,//RK2928_PIN3_PC4,
	.bp_wakeup_ap		= BP_UNKNOW_DATA,//RK2928_PIN3_PC3,	//
	.bp_uart_en		= BP_UNKNOW_DATA, 	//EINT9
	.bp_usb_en		= BP_UNKNOW_DATA, 	//W_disable
	.trig			= IRQF_TRIGGER_FALLING,

	.active			= bp_active,
	.init			= bp_init,
	.reset			= bp_reset,
	.ap_wake_bp		= ap_wake_bp,
	.bp_wake_ap		= bp_wake_ap,
	.shutdown		= bp_shutdown,
	.read_status		= NULL,
	.write_status		= NULL,
	.suspend 		= bp_suspend,
	.resume			= bp_resume,
	.misc_name		= NULL,
	.private_miscdev	= NULL,
#endif
};

/****************operate according to bp chip:end************/

//function name should not be changed
static struct bp_operate *bp_get_ops(void)
{
	return &bp_lm80xa_ops;
}

static int __init bp_lm80xa_init(void)
{
	struct bp_operate *ops = bp_get_ops();
	int result = 0;
	result = bp_register_slave(NULL, NULL, bp_get_ops);
	if(result)
	{	
		return result;
	}
	
	if(ops->private_miscdev)
	{
		result = misc_register(ops->private_miscdev);
		if (result < 0) {
			printk("%s:misc_register err\n",__func__);
			return result;
		}
	}

	/*modem_class = class_create(THIS_MODULE, "rk291x_modem");
	result =  class_create_file(modem_class, &class_attr_modem_status);
	if (result)
	{
		printk("Fail to class rk291x_modem.\n");
	}

	modem_status = 1;
*/
	
	
	DBG("%s\n",__func__);
	return result;
}

static void __exit bp_lm80xa_exit(void)
{
	//struct bp_operate *ops = bp_get_ops();
	bp_unregister_slave(NULL, NULL, bp_get_ops);
	//class_remove_file(modem_class, &class_attr_modem_status);
}


subsys_initcall(bp_lm80xa_init);
module_exit(bp_lm80xa_exit);

