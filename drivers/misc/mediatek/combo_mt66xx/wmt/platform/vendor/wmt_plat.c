/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

/*******************************************************************************
* Copyright (c) 2009 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#include <asm/mach-types.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <mach/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/combo_mt66xx.h>

#if CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#define CFG_WMT_WAKELOCK_SUPPORT 1
#endif


#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG         "[WMT-PLAT]"


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*header files*/
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/spinlock.h>

/* MTK_WCN_COMBO header files */
#include "wmt_plat.h"
#include "wmt_exp.h"
#include "mtk_wcn_cmb_hw.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

static VOID wmt_plat_bgf_eirq_cb (VOID);

static INT32 wmt_plat_ldo_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_pmu_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_rtc_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_rst_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_bgf_eint_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_wifi_eint_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_all_eint_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_uart_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_pcm_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_i2s_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_sdio_pin_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_sync_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_lna_ctrl (ENUM_PIN_STATE state);

static INT32 wmt_plat_dump_pin_conf (VOID);


extern int rk29sdk_wifi_power(int on);
extern int rk29sdk_wifi_set_carddetect(int val);

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
UINT32 gWmtDbgLvl = WMT_LOG_ERR;//WMT_LOG_DBG;//xbw

unsigned int g_bgf_irq = -1;//bgf eint number
unsigned int g_sdio_irq = -1;//bgf eint number
static struct mtk_wmt_platform_data wmt_pdata;
/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

#if CFG_WMT_WAKELOCK_SUPPORT
static OSAL_SLEEPABLE_LOCK gOsSLock;
static struct wake_lock wmtWakeLock;
#endif

irq_cb wmt_plat_bgf_irq_cb = NULL;
device_audio_if_cb wmt_plat_audio_if_cb = NULL;

const static fp_set_pin gfp_set_pin_table[] =
{
    [PIN_LDO] = wmt_plat_ldo_ctrl,
    [PIN_PMU] = wmt_plat_pmu_ctrl,
    [PIN_RTC] = wmt_plat_rtc_ctrl,
    [PIN_RST] = wmt_plat_rst_ctrl,
    [PIN_BGF_EINT] = wmt_plat_bgf_eint_ctrl,
    [PIN_WIFI_EINT] = wmt_plat_wifi_eint_ctrl,
    [PIN_ALL_EINT] = wmt_plat_all_eint_ctrl,
    [PIN_UART_GRP] = wmt_plat_uart_ctrl,
    [PIN_PCM_GRP] = wmt_plat_pcm_ctrl,
    [PIN_I2S_GRP] = wmt_plat_i2s_ctrl,
    [PIN_SDIO_GRP] = wmt_plat_sdio_pin_ctrl,
    [PIN_GPS_SYNC] = wmt_plat_gps_sync_ctrl,
    [PIN_GPS_LNA] = wmt_plat_gps_lna_ctrl,

};

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*!
 * \brief audio control callback function for CMB_STUB on ALPS
 *
 * A platform function required for dynamic binding with CMB_STUB on ALPS.
 *
 * \param state desired audio interface state to use
 * \param flag audio interface control options
 *
 * \retval 0 operation success
 * \retval -1 invalid parameters
 * \retval < 0 error for operation fail
 */
INT32 wmt_plat_audio_ctrl (CMB_STUB_AIF_X state, CMB_STUB_AIF_CTRL ctrl)
{
    INT32 iRet = 0;
    UINT32 pinShare;

    /* input sanity check */
    if ( (CMB_STUB_AIF_MAX <= state)
        || (CMB_STUB_AIF_CTRL_MAX <= ctrl) ) {
        iRet = -1;
        WMT_ERR_FUNC("WMT-PLAT: invalid para, state(%d), ctrl(%d),iRet(%d) \n", state, ctrl, iRet);
        return iRet;
    }
    if (0/*I2S/PCM share pin*/) {
        // TODO: [FixMe][GeorgeKuo] how about MT6575? The following is applied to MT6573E1 only!!
        pinShare = 1;
        WMT_INFO_FUNC( "PCM/I2S pin share\n");
    }
    else{ //E1 later
        pinShare = 0;
        WMT_INFO_FUNC( "PCM/I2S pin seperate\n");
    }

    iRet = 0;

    /* set host side first */
    switch (state) {
    case CMB_STUB_AIF_0:
        /* BT_PCM_OFF & FM line in/out */
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_DEINIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
        break;

    case CMB_STUB_AIF_1:
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_INIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
        break;

    case CMB_STUB_AIF_2:
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_DEINIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
        break;

    case CMB_STUB_AIF_3:
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_INIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
        break;

    default:
        /* FIXME: move to cust folder? */
        WMT_ERR_FUNC("invalid state [%d]\n", state);
        return -1;
        break;
    }

    if (CMB_STUB_AIF_CTRL_EN == ctrl) {
        WMT_INFO_FUNC("call chip aif setting \n");
        /* need to control chip side GPIO */
        //iRet += wmt_lib_set_aif(state, (pinShare) ? MTK_WCN_BOOL_TRUE : MTK_WCN_BOOL_FALSE);
		if (NULL != wmt_plat_audio_if_cb)
		{
		    iRet += (*wmt_plat_audio_if_cb)(state, (pinShare) ? MTK_WCN_BOOL_TRUE : MTK_WCN_BOOL_FALSE);
		}
		else
		{
		    WMT_WARN_FUNC("wmt_plat_audio_if_cb is not registered \n");
		    iRet -= 1;
		}
    }
    else {
        WMT_INFO_FUNC("skip chip aif setting \n");
    }
    return iRet;
}

#if CFG_WMT_PS_SUPPORT
irqreturn_t bgf_irq_handler(int i, void *arg)
{
    wmt_plat_bgf_eirq_cb();
    return IRQ_HANDLED;
}
#endif

static VOID
wmt_plat_bgf_eirq_cb (VOID)
{
	
#if CFG_WMT_PS_SUPPORT
	//#error "need to disable EINT here"
		//wmt_lib_ps_irq_cb();
		if (NULL != wmt_plat_bgf_irq_cb)
		{
			(*(wmt_plat_bgf_irq_cb))();
		}
		else
		{
			WMT_WARN_FUNC("WMT-PLAT: wmt_plat_bgf_irq_cb not registered\n");
		}
#else
		return;
#endif

}

VOID wmt_lib_plat_irq_cb_reg (irq_cb bgf_irq_cb)
{
    wmt_plat_bgf_irq_cb = bgf_irq_cb;
}

VOID wmt_lib_plat_aif_cb_reg (device_audio_if_cb aif_ctrl_cb)
{
    wmt_plat_audio_if_cb = aif_ctrl_cb;
}

void wmt_plat_gpio_init(struct platform_device *pdev)
{
	struct mtk_wmt_platform_data *p = pdev->dev.platform_data;

	wmt_pdata.pmu = p->pmu;
	wmt_pdata.rst = p->rst;
	wmt_pdata.bgf_int = p->bgf_int;
	wmt_pdata.urt_cts = p->urt_cts;
	wmt_pdata.rtc = p->rtc;
	wmt_pdata.gps_sync = p->gps_sync;
	wmt_pdata.gps_lna = p->gps_lna;
	
	wmt_plat_dump_pin_conf();
}

INT32
wmt_plat_init (P_PWR_SEQ_TIME pPwrSeqTime)
{
    //CMB_STUB_CB stub_cb;
    /*PWR_SEQ_TIME pwr_seq_time;*/
    INT32 iret = 0;

    //stub_cb.aif_ctrl_cb = wmt_plat_audio_ctrl;
    //stub_cb.func_ctrl_cb = wmt_plat_func_ctrl;
    //stub_cb.size = sizeof(stub_cb);

    /* register to cmb_stub */
    //iret = mtk_wcn_cmb_stub_reg(&stub_cb);

    /* init cmb_hw */
    iret += mtk_wcn_cmb_hw_init(pPwrSeqTime);

    /*init wmt function ctrl wakelock if wake lock is supported by host platform*/
    #ifdef CFG_WMT_WAKELOCK_SUPPORT
    wake_lock_init(&wmtWakeLock, WAKE_LOCK_SUSPEND, "wmtFuncCtrl");
    osal_sleepable_lock_init(&gOsSLock);
    #endif

    WMT_DBG_FUNC("WMT-PLAT: ALPS platform init (%d)\n", iret);

    return iret;
}

INT32
wmt_plat_deinit (VOID)
{
    INT32 iret;

    /* 1. de-init cmb_hw */
    iret = mtk_wcn_cmb_hw_deinit();
    /* 2. unreg to cmb_stub */
    iret += mtk_wcn_cmb_stub_unreg();
    /*3. wmt wakelock deinit*/
    #ifdef CFG_WMT_WAKELOCK_SUPPORT
    wake_lock_destroy(&wmtWakeLock);
    osal_sleepable_lock_deinit(&gOsSLock);
    WMT_DBG_FUNC("destroy wmtWakeLock\n");
    #endif
    WMT_DBG_FUNC("WMT-PLAT: ALPS platform init (%d)\n", iret);

    return 0;
}

INT32 wmt_plat_sdio_ctrl (WMT_SDIO_SLOT_NUM sdioPortType, ENUM_FUNC_STATE on)
{
	int ret = 0;

    if (FUNC_OFF == on)  {
        /* add control logic here to generate SDIO CARD REMOVAL event to mmc/sd
         * controller. SDIO card removal operation and remove success messages
         * are expected.
         */
	//rk29sdk_wifi_power(0);
	rk29sdk_wifi_set_carddetect(0);
    }
    else {
        /* add control logic here to generate SDIO CARD INSERTION event to mmc/sd
         * controller. SDIO card detection operation and detect success messages
         * are expected.
         */
	//rk29sdk_wifi_power(1);
	rk29sdk_wifi_set_carddetect(1);
    }
	//extern int omap_mmc_update_mtk_card_status(int state);
//	ret = omap_mmc_update_mtk_card_status((FUNC_OFF == on)? 0: 1);
	WMT_INFO_FUNC(KERN_INFO "%s, on=%d, ret=%d\n", __FUNCTION__, on, ret);
	return ret;
}

#if 0
INT32
wmt_plat_irq_ctrl (
    ENUM_FUNC_STATE state
    )
{
    return -1;
}
#endif

static INT32
wmt_plat_dump_pin_conf (VOID)
{
    WMT_INFO_FUNC( "WMT pin configuration [pmu: %d, rst: %d, bgf: %d, cts: %d, rtc: %d, gps_lna:%d, gps_sync:%d]\n", 
				wmt_pdata.pmu, 
				wmt_pdata.rst, 
				wmt_pdata.bgf_int,
				wmt_pdata.urt_cts, 
				wmt_pdata.rtc,
				wmt_pdata.gps_lna,
				wmt_pdata.gps_sync );
    return 0;
}


INT32 wmt_plat_pwr_ctrl (
    ENUM_FUNC_STATE state
    )
{
    INT32 ret = -1;

    switch (state) {
    case FUNC_ON:
        // TODO:[ChangeFeature][George] always output this or by request throuth /proc or sysfs?
        ret = mtk_wcn_cmb_hw_pwr_on();
        break;

    case FUNC_OFF:
        ret = mtk_wcn_cmb_hw_pwr_off();
        break;

    case FUNC_RST:
        ret = mtk_wcn_cmb_hw_rst();
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) in pwr_ctrl\n", state);
        break;
    }

    return ret;
}

INT32 wmt_plat_ps_ctrl (ENUM_FUNC_STATE state)
{
    return -1;
}

INT32
wmt_plat_eirq_ctrl (
    ENUM_PIN_ID id,
    ENUM_PIN_STATE state
    )
{
    INT32 iRet = 0;
    int gpio_value=0;

    // TODO: [ChangeFeature][GeorgeKuo]: use another function to handle this, as done in gpio_ctrls

    if ( (PIN_STA_INIT != state )
        && (PIN_STA_DEINIT != state )
        && (PIN_STA_EINT_EN != state )
        && (PIN_STA_EINT_DIS != state ) ) {
        iRet = -1;
        WMT_WARN_FUNC("WMT-PLAT:invalid PIN_STATE(%d) in eirq_ctrl for PIN(%d), ret(%d) \n", state, id, iRet);
        return iRet;
    }

    switch (id) {
    case PIN_BGF_EINT:
        if (PIN_STA_INIT == state) {
			//iRet = request_irq(g_bgf_irq, bgf_irq_handler,  IRQF_TRIGGER_LOW | IRQF_DISABLED, "MTK6620_BT", NULL);
			gpio_value = gpio_get_value(g_bgf_irq);
                        printk("%d..%s:  BGF_INT gpio_init value = %d  ====mt6620===\n", __LINE__, __FUNCTION__, gpio_value );
                        
                        iRet = request_irq(g_bgf_irq, bgf_irq_handler, 
					    //(gpio_get_value(MT6620_GPIO_BGF_INT_B))?IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING,
					    ( (gpio_value)?IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH ) |IRQF_DISABLED,
					    "MTK6620_BT", NULL);
 
                        if (iRet) {
				WMT_INFO_FUNC("WMT-PLAT: request IRQ fail for BGF IRQ : %d\n", g_bgf_irq);
			} else {
           		enable_irq_wake(g_bgf_irq);
			}
			WMT_INFO_FUNC("WMT-PLAT:BGF Int requested...%s, iRet=%d\n", iRet?"fail":"success.", iRet);
        } else if (PIN_STA_EINT_EN == state) {
		   	enable_irq(g_bgf_irq);
        } else if (PIN_STA_EINT_DIS == state) {
		   	disable_irq_nosync(g_bgf_irq);
        } else {
           	disable_irq_wake(g_bgf_irq);
            free_irq(g_bgf_irq,NULL);
            WMT_INFO_FUNC("WMT-PLAT:BGFInt (deinit) \n");
        }
        break;

    case PIN_ALL_EINT:
		break;
	case PIN_WIFI_EINT:
		break;
	default:
        WMT_WARN_FUNC("WMT-PLAT:unsupported EIRQ(PIN_ID:%d) in eirq_ctrl\n", id);
        break;
    }

    return iRet;
}

INT32 wmt_plat_gpio_ctrl (
    ENUM_PIN_ID id,
    ENUM_PIN_STATE state
    )
{
    if ( (PIN_ID_MAX > id)
        && (PIN_STA_MAX > state) ) {

        // TODO: [FixMe][GeorgeKuo] do sanity check to const function table when init and skip checking here
        if (gfp_set_pin_table[id]) {
            return (*(gfp_set_pin_table[id]))(state); /* .handler */
        }
        else {
            WMT_WARN_FUNC("WMT-PLAT: null fp for gpio_ctrl(%d)\n", id);
            return -2;
        }
    }
	WMT_ERR_FUNC("WMT-PLAT:[out of range] id(%d), state (%d)\n", id, state);
    return -1;
}

INT32
wmt_plat_ldo_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch(state)
    {
    case PIN_STA_INIT:
        /*set to gpio output low, disable pull*/
        WMT_DBG_FUNC("WMT-PLAT:LDO init (out 0) \n");
        break;

    case PIN_STA_OUT_H:
        WMT_DBG_FUNC("WMT-PLAT:LDO (out 1) \n");
        break;

    case PIN_STA_OUT_L:
        WMT_DBG_FUNC("WMT-PLAT:LDO (out 0) \n");
        break;

    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
        /*set to gpio input low, pull down enable*/
        WMT_DBG_FUNC("WMT-PLAT:LDO deinit (in pd) \n");
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on LDO\n", state);
        break;
    }
    return 0;
}

INT32
wmt_plat_pmu_ctrl (
    ENUM_PIN_STATE state
    )
{
	INT32 ret = 0;
    switch(state)
    {
    case PIN_STA_INIT:
		if (gpio_is_valid(wmt_pdata.pmu)) {
			ret = gpio_request(wmt_pdata.pmu, "MT66XX PMUEN");
			if (ret) {
				WMT_INFO_FUNC("PMU gpio_request %d failed", ret);
			} else {
				gpio_direction_output(wmt_pdata.pmu, 0);
           		WMT_INFO_FUNC("WMT-PLAT:PMU init (out 0) \n");
			}
		} else {
       		WMT_INFO_FUNC("WMT-PLAT:invalid GPIo num for PMU EN, gpio:%d\n", wmt_pdata.pmu);
		}
        break;

    case PIN_STA_OUT_H:
		if (gpio_is_valid(wmt_pdata.pmu)) {
			gpio_direction_output(wmt_pdata.pmu, 1);
        	WMT_INFO_FUNC("WMT-PLAT:PMU (out_h %d) \n", gpio_get_value(wmt_pdata.pmu));
		} else {
			WMT_INFO_FUNC("WMT-PLAT:PMU H invalid GPIO number: %d\n", wmt_pdata.pmu);
		}

        break;

    case PIN_STA_OUT_L:
		if (gpio_is_valid(wmt_pdata.pmu)) {
			gpio_direction_output(wmt_pdata.pmu, 0);
        	WMT_INFO_FUNC("WMT-PLAT:PMU (out_h %d) \n", gpio_get_value(wmt_pdata.pmu));
		} else {
			WMT_INFO_FUNC("WMT-PLAT:PMU L invalid GPIO number: %d\n", wmt_pdata.pmu);
		}

        break;

    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
		if (gpio_is_valid(wmt_pdata.pmu)) {
			gpio_direction_output(wmt_pdata.pmu, 0);
			gpio_free(wmt_pdata.pmu);
           	WMT_INFO_FUNC("WMT-PLAT:PMU deinit (in pd) \n");
		} else {
           	WMT_INFO_FUNC("WMT-PLAT:PMU deinit invalid GPIO: %d \n", wmt_pdata.pmu);
		}
        break;

    default:
        WMT_INFO_FUNC("WMT-PLAT:Warnning, invalid state(%d) on PMU\n", state);
        break;
    }

    return ret;
}

INT32
wmt_plat_rtc_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch(state)
    {
    case PIN_STA_INIT:
        WMT_DBG_FUNC("WMT-PLAT:RTC init \n");
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on RTC\n", state);
        break;
    }
    return 0;
}


INT32
wmt_plat_rst_ctrl (
    ENUM_PIN_STATE state
    )
{
	int ret = 0;
    switch(state)
    {
        case PIN_STA_INIT:
			if (gpio_is_valid(wmt_pdata.rst)) {
				ret = gpio_request(wmt_pdata.rst, "MT66XX SYSRST");
				if (ret) {
					WMT_INFO_FUNC("WMT-PLAT: request GPIO for RST failed. gpio:%d, ret:%d\n",wmt_pdata.rst, ret);
				} else {
					gpio_direction_output(wmt_pdata.rst, 0);
            		WMT_INFO_FUNC("WMT-PLAT:RST init (out 0) \n");
				}
			} else {
				WMT_INFO_FUNC("WMT-PLAT: invalid GPIO for RST: %d\n", wmt_pdata.rst);
			}
            break;

        case PIN_STA_OUT_H:
			if (gpio_is_valid(wmt_pdata.rst)) {
				gpio_direction_output(wmt_pdata.rst, 1);
        		WMT_INFO_FUNC("WMT-PLAT:RST (out_h %d) \n", gpio_get_value(wmt_pdata.rst));
			} else {
				WMT_INFO_FUNC("WMT-PLAT:RST H invalid GPIO number: %d\n", wmt_pdata.pmu);
			}

	        break;

        case PIN_STA_OUT_L:
			if (gpio_is_valid(wmt_pdata.rst)) {
				gpio_direction_output(wmt_pdata.rst, 0);
        		WMT_INFO_FUNC("WMT-PLAT:RST (out_h %d) \n", gpio_get_value(wmt_pdata.rst));
			} else {
				WMT_INFO_FUNC("WMT-PLAT:RST L invalid GPIO number: %d\n", wmt_pdata.pmu);
			}

	        break;

        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
            /*set to gpio input low, pull down enable*/
			if (gpio_is_valid(wmt_pdata.rst)) {
				gpio_direction_output(wmt_pdata.rst, 0);
				gpio_free(wmt_pdata.rst);
            	WMT_INFO_FUNC("WMT-PLAT:RST deinit (in pd) \n");
			} else {
            	WMT_INFO_FUNC("WMT-PLAT:RST deinit invalid GPIO: %d \n", wmt_pdata.rst);
			}
            break;

        default:
            WMT_INFO_FUNC("WMT-PLAT:Warnning, invalid state(%d) on RST\n", state);
            break;
    }

    return ret;
}

INT32
wmt_plat_bgf_eint_ctrl (
    ENUM_PIN_STATE state
    )
{
	int ret = 0;
    switch(state)
    {
        case PIN_STA_INIT:
			if (gpio_is_valid(wmt_pdata.bgf_int)) {
				ret = gpio_request(wmt_pdata.bgf_int, "MT66XX BGF EINT");
				if (ret) {
					WMT_INFO_FUNC("BGF_EINT gpio request fail, ret = %d\n", ret);
				}
			} else {
					WMT_INFO_FUNC("invalid BGF_EINT gpio: %d\n", wmt_pdata.bgf_int);
			}
            break;

        case PIN_STA_MUX:
			g_bgf_irq = gpio_to_irq(wmt_pdata.bgf_int);
            WMT_INFO_FUNC("WMT-PLAT: bgf irq num is: %d\n", g_bgf_irq);
            break;

        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
			if (gpio_is_valid(wmt_pdata.bgf_int)) {
				gpio_free(wmt_pdata.bgf_int);
			}
            break;

        default:
            WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on BGF EINT\n", state);
            break;
    }

    return ret;
}


INT32 wmt_plat_wifi_eint_ctrl(ENUM_PIN_STATE state)
{
    WMT_INFO_FUNC("WMT-PLAT:WIFI EINT is controlled by MSDC driver or SDIO EINT.\n");
    return 0;
}


INT32
wmt_plat_all_eint_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch(state)
    {
        case PIN_STA_INIT:
        	  /*set to gpio input low, pull down eanble*/
            WMT_DBG_FUNC("WMT-PLAT:ALLInt init(in pd) \n");
            break;

        case PIN_STA_MUX:
        	  /*set to gpio EINT mode, pull down enable*/
            break;

        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
            /*set to gpio input low, pull down enable*/
            break;

        default:
            WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on ALL EINT\n", state);
            break;
    }
    return 0;
}

/*
in this function, we will ctrl host UART CTS pin to output mode, and out high /low to control the 662x chip to COMMON sdio mod
step
1. control the CTS to gpio mode
2. contorl the CTS to output mode
3. control the CTS to output high/low
*/

INT32 wmt_plat_uart_cts_ctrl(INT32 high)
{
	INT32 ret;

	if (wmt_pdata.urt_cts == -EINVAL)
		return 0;

	if (high) {
		ret = gpio_request(wmt_pdata.urt_cts, "MT66XX UART CTS");
		if(ret<0) {
			WMT_INFO_FUNC( "UART CTS request fail:%d\n",ret);
		}
		ret = gpio_direction_output(wmt_pdata.urt_cts, 1);
		WMT_INFO_FUNC( "UART CTS out high --> common sdio mode,ret:%d \n",ret);
	} else {
		ret = gpio_direction_input(wmt_pdata.urt_cts);
		WMT_INFO_FUNC( "UART CTS out high --> common sdio mode \n");
		gpio_free(wmt_pdata.urt_cts);
	}
	return 0;
}

INT32 wmt_plat_uart_ctrl(ENUM_PIN_STATE state)
{
    switch(state)
    {
    case PIN_STA_MUX:
    case PIN_STA_INIT:
        WMT_INFO_FUNC("WMT-PLAT:UART init (mode_01, uart) \n");
        break;
    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
        WMT_INFO_FUNC("WMT-PLAT:UART deinit (out 0) \n");
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on UART Group\n", state);
        break;
    }

    return 0;
}


INT32 wmt_plat_pcm_ctrl(ENUM_PIN_STATE state)
{
    switch(state)
    {
    case PIN_STA_MUX:
    case PIN_STA_INIT:
    	  /*set to PCM function*/
        WMT_DBG_FUNC("WMT-PLAT:PCM init (pcm) \n");
        break;

    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
        WMT_DBG_FUNC("WMT-PLAT:PCM deinit (out 0) \n");
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on PCM Group\n", state);
        break;
    }
    return 0;
}


INT32 wmt_plat_i2s_ctrl(ENUM_PIN_STATE state)
{
#ifndef FM_ANALOG_INPUT
    switch(state)
    {
    case PIN_STA_INIT:
    case PIN_STA_MUX:
    /*set to I2S function*/
        WMT_DBG_FUNC("WMT-PLAT:I2S init \n");
        break;
    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
    /*set to gpio input low, pull down enable*/
        WMT_DBG_FUNC("WMT-PLAT:I2S deinit (out 0) \n");
        break;
    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on I2S Group\n", state);
        break;
    }
#else
        WMT_INFO_FUNC( "[MT6620]warnning:FM analog mode is set, no I2S GPIO settings should be modified by combo driver\n");
#endif

    return 0;
}

INT32
wmt_plat_sdio_pin_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_MUX:
        break;
    case PIN_STA_DEINIT:
        break;
    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on SDIO Group\n", state);
        break;
    }
    return 0;
}

static INT32
wmt_plat_gps_sync_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_DEINIT:
    /*set GPS_SYNC GPIO to GPIO mode, pull disable,output low*/
        break;

    case PIN_STA_MUX:
    /*set GPS_SYNC GPIO to GPS_SYNC function*/
        break;

    default:
        break;
    }
    return 0;
}


static INT32
wmt_plat_gps_lna_ctrl (
        ENUM_PIN_STATE state
        )
{
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_DEINIT:
    /*set GPS_LNA GPIO to GPIO mode, pull disable,output low*/
        break;
    case PIN_STA_OUT_H:
    /*set GPS_LNA GPIO to GPIO mode, pull disable,output high*/
        break;
    case PIN_STA_OUT_L:
    /*set GPS_LNA GPIO to GPIO mode, pull disable,output low*/
        break;

    default:
        WMT_WARN_FUNC("%d mode not defined for  gps lna pin !!!\n", state);
        break;
    }
    return 0;

}



INT32 wmt_plat_wake_lock_ctrl(ENUM_WL_OP opId)
{
#ifdef CFG_WMT_WAKELOCK_SUPPORT
    static INT32 counter = 0;


    osal_lock_sleepable_lock( &gOsSLock);
    if (WL_OP_GET == opId)
    {
        ++counter;
    }else if (WL_OP_PUT == opId)
    {
        --counter;
    }
    osal_unlock_sleepable_lock( &gOsSLock);
    if (WL_OP_GET == opId && counter == 1)
    {
        wake_lock(&wmtWakeLock);
        WMT_DBG_FUNC("WMT-PLAT: after wake_lock(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter);

    }
    else if (WL_OP_PUT == opId && counter == 0)
    {
        wake_unlock(&wmtWakeLock);
        WMT_DBG_FUNC("WMT-PLAT: after wake_unlock(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter);
    }
    else
    {
        WMT_WARN_FUNC("WMT-PLAT: wakelock status(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter);
    }
    return 0;
#else
    WMT_WARN_FUNC("WMT-PLAT: host awake function is not supported.");
    return 0;

#endif
}

