/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/io.h>
#include <asm/mach-types.h>
#include <mach/iomux.h>
//#include <plat/gpio-cfg.h>

#include "bt_hwctl.h"

extern int mt6622_suspend_flag;


#ifdef CONFIG_ARCH_RK29
    #define rk_mux_api_set(name,mode)      rk29_mux_api_set(name,mode)
#elif defined (CONFIG_ARCH_RK30)
    #define rk_mux_api_set(name,mode)      rk30_mux_api_set(name,mode)
#else
    #define rk_mux_api_set(name,mode)      rk30_mux_api_set(name,mode)
#endif

/****************************************************************************
 *                           C O N S T A N T S                              *
*****************************************************************************/
#define MODULE_TAG         "[BT-PLAT] "

/* SMDK platform PIN definition */
#define GPIO_BT_PWR_EN_PIN	46
#define GPIO_BT_RESET_PIN	46 //if pwr_en & reset not connect can use this pin
#define GPIO_BT_EINT_PIN	49
//#define GPIO_BT_URXD_PIN ( EXYNOS4_GPA0(0) )  //wenhui: due to panda has configed uart interface
//#define GPIO_BT_UTXD_PIN ( EXYNOS4_GPA0(1) )

//wenhui:should add bt uart cts/rts gpio config
//#define GPIO_BT_HOST_UART_CTS		123


static struct gpio_set bt_gpio_set[2] = {// wenhui modify
    { GPIO_BT_PWR_EN_PIN, 0, "BT_PWR_EN" },
//    { GPIO_BT_RESET_PIN,  0, "BT_RESET"  },
    { GPIO_BT_EINT_PIN,   0, "BT_EINT"   }//,
//    { GPIO_BT_URXD_PIN,   0, "BT_URXD"   },
//    { GPIO_BT_UTXD_PIN,   0, "BT_UTXD"   }
};

static unsigned char irq_requested = 0;//wenhui add 
static int irq_num = -1;
// to avoid irq enable and disable not match
static unsigned int irq_mask;
//static spinlock_t bt_irq_lock;
extern void omap_mux_set_gpio(u16 val, int gpio);
/****************************************************************************
 *                       I R Q   F U N C T I O N S                          *
*****************************************************************************/
static int mt_bt_request_irq(void)
{
    int iRet;
    int trigger = IRQF_TRIGGER_RISING;
    struct mt6622_platform_data *pdata = (struct mt6622_platform_data *)mt_bt_get_platform_data();
		
    irq_mask = 0;
    if(pdata->irq_gpio.enable == GPIO_LOW)
    	trigger = IRQF_TRIGGER_FALLING;
    
    iRet = request_irq(irq_num, mt_bt_eirq_handler, 
        trigger, "BT_INT_B", NULL);
    if (iRet){
        printk(KERN_ALERT MODULE_TAG "request_irq IRQ%d fails, errno %d\n", irq_num, iRet);
    }
    else{
        printk(KERN_INFO MODULE_TAG "request_irq IRQ%d success\n", irq_num);
        mt_bt_disable_irq();
        /* enable irq when driver init complete, at hci_uart_open */
    }
    irq_requested = 1; //wenhui add
    return iRet;
}

static void mt_bt_free_irq(void)
{
    if(irq_requested) {
	free_irq(irq_num, NULL);
	irq_mask = 0;
	irq_requested = 0;
    }
}

void mt_bt_enable_irq(void)
{
    printk(KERN_INFO MODULE_TAG "enable_irq, mask %d\n", irq_mask);
    if (irq_mask){
        irq_mask = 0;
        enable_irq(irq_num);
    }
}
EXPORT_SYMBOL(mt_bt_enable_irq);

int mt6622_suspend(struct platform_device *pdev, pm_message_t state)
{
    if(irq_num != -1) {
        printk(KERN_INFO MODULE_TAG "mt6622_suspend\n");
        mt6622_suspend_flag = 1;
        enable_irq_wake(irq_num);
    }
	return 0;
}

int mt6622_resume(struct platform_device *pdev)
{
    if(irq_num != -1) {
        printk(KERN_INFO MODULE_TAG "mt6622_resume\n");
        disable_irq_wake(irq_num);
    }
	return 0;
}

void mt_bt_disable_irq(void)
{
    printk(KERN_INFO MODULE_TAG "disable_irq_nosync, mask %d\n", irq_mask);
    if (!irq_mask){
        irq_mask = 1;
        disable_irq_nosync(irq_num);
    }
}
EXPORT_SYMBOL(mt_bt_disable_irq);

/****************************************************************************
 *                      P O W E R   C O N T R O L                           *
*****************************************************************************/

int mt_bt_power_init(void)
{
    struct mt6622_platform_data *pdata;
    
    printk(KERN_INFO MODULE_TAG "mt_bt_power_init\n");
    
    pdata = (struct mt6622_platform_data *)mt_bt_get_platform_data();
    
    if(pdata) {
	    
	    // PWR_EN and RESET
	    /* PWR_EN set to gpio output low */
	    if(pdata->power_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->power_gpio.io, 0);
	    /* RESET set to gpio output low */
	    if(pdata->reset_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->reset_gpio.io, 0);
	    msleep(200);
	    
	    /* PWR_EN pull up */
	    //if(pdata->power_gpio.io != INVALID_GPIO)
	    //	gpio_direction_output(pdata->power_gpio.io, 0);
	    //msleep(200);
	    /* RESET pull up */
	    if(pdata->reset_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->reset_gpio.io, 1);
	    msleep(1000);
	    
	    //pdata->power_gpio.io = INVALID_GPIO;
	    pdata->reset_gpio.io = INVALID_GPIO;
	}
    
    return 0;
}
EXPORT_SYMBOL(mt_bt_power_init);

int mt_bt_power_on(void)
{
    int error;
    struct mt6622_platform_data *pdata;
    
    printk(KERN_INFO MODULE_TAG "mt_bt_power_on ++\n");
    
    pdata = (struct mt6622_platform_data *)mt_bt_get_platform_data();
    
    if(pdata) {
	    // UART TX/RX
	    
	    // PCMIN, PCMOUT, PCMCLK, PCMSYNC
	    
	    // EINT
	    //--s3c_gpio_cfgpin(GPIO_BT_EINT_PIN, S3C_GPIO_SFN(0));
	    //--s3c_gpio_setpull(GPIO_BT_EINT_PIN, S3C_GPIO_PULL_DOWN);
	    if(pdata->irq_gpio.io != INVALID_GPIO)
	    	gpio_direction_input(pdata->irq_gpio.io);
	    //gpio_pull_updown(pdata->irq_gpio->io, GPIOPullDown);
	    /* set to EINT mode */
	    //--s3c_gpio_cfgpin(GPIO_BT_EINT_PIN, S3C_GPIO_SFN(0xF));
	    /* get irq number */
	    if(pdata->irq_gpio.io != INVALID_GPIO)
	    	irq_num = gpio_to_irq(pdata->irq_gpio.io);
	    //mt_set_gpio_mode(GPIO_BT_EINT_PIN, GPIO_BT_EINT_PIN_M_GPIO);
	    //mt_set_gpio_pull_enable(GPIO_BT_EINT_PIN, 1);
	    //mt_set_gpio_pull_select(GPIO_BT_EINT_PIN, GPIO_PULL_DOWN);
	    //mt_set_gpio_mode(GPIO_BT_EINT_PIN, GPIO_BT_EINT_PIN_M_EINT);
	    
	    // 32k CLK
	    //mt_set_gpio_mode(GPIO_BT_CLK_PIN , GPIO_BT_CLK_PIN_M_CLK);
	    //mt_set_clock_output(GPIO_BT_CLK_PIN_CLK, CLK_SRC_F32K, 1);
	   
         if(gpio_is_valid(pdata->rts_gpio.io)) {
             printk(KERN_INFO MODULE_TAG "mt_bt_power_on rts iomux\n");
             rk_mux_api_set(pdata->rts_gpio.iomux.name, pdata->rts_gpio.iomux.fgpio);
             gpio_direction_output(pdata->rts_gpio.io, 0);
         }	   
	    
	    // PWR_EN and RESET
	    /* PWR_EN set to gpio output low */
	    if(pdata->power_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->power_gpio.io, 0);
	    /* RESET set to gpio output low */
	    if(pdata->reset_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->reset_gpio.io, 0);
	    msleep(200);
	    
	    /* PWR_EN pull up */
	    if(pdata->power_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->power_gpio.io, 1);
	    msleep(200);
	    /* RESET pull up */
	    if(pdata->reset_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->reset_gpio.io, 1);
	    msleep(1000);

        if(gpio_is_valid(pdata->rts_gpio.io)) {
            rk_mux_api_set(pdata->rts_gpio.iomux.name, pdata->rts_gpio.iomux.fmux);
        }
	    
	    error = mt_bt_request_irq();
	    if (error){
	        if(pdata->power_gpio.io != INVALID_GPIO)
	        	gpio_direction_output(pdata->power_gpio.io, 0);
	        if(pdata->reset_gpio.io != INVALID_GPIO)	
	        	gpio_direction_output(pdata->reset_gpio.io, 0);
	        //--s3c_gpio_cfgpin(GPIO_BT_EINT_PIN, S3C_GPIO_SFN(1));
	        if(pdata->irq_gpio.io != INVALID_GPIO)
	        	gpio_direction_output(pdata->irq_gpio.io, 0);
	        return error;
	    }
    }
    
    printk(KERN_INFO MODULE_TAG "mt_bt_power_on --\n");
    
    return 0;
}

EXPORT_SYMBOL(mt_bt_power_on);


void mt_bt_power_off(void)
{
    struct mt6622_platform_data *pdata;
    pdata = (struct mt6622_platform_data *)mt_bt_get_platform_data();	
	
    printk(KERN_INFO MODULE_TAG "mt_bt_power_off ++\n");
    
    if(pdata) {
	    // PWR_EN and RESET
	    if(pdata->power_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->power_gpio.io, 0);
	    if(pdata->reset_gpio.io != INVALID_GPIO)	
	    	gpio_direction_output(pdata->reset_gpio.io, 0);
	    
	    // EINT
	    //--s3c_gpio_cfgpin(GPIO_BT_EINT_PIN, S3C_GPIO_SFN(1));
	    if(pdata->irq_gpio.io != INVALID_GPIO)
	    	gpio_direction_output(pdata->irq_gpio.io, 0);
	    
	    mt_bt_free_irq();
    }
    
    printk(KERN_INFO MODULE_TAG "mt_bt_power_off --\n");
}

EXPORT_SYMBOL(mt_bt_power_off);
