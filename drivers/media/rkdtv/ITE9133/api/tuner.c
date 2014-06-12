/**
 * @(#)Afatech_OMEGA.cpp
 *
 * ==========================================================
 * Version: 2.0
 * Date:    2009.06.15
 * ==========================================================
 *
 * ==========================================================
 * History:
 *
 * Date         Author      Description
 * ----------------------------------------------------------
 *
 * 2009.06.15   M.-C. Ho    new tuner
 * ==========================================================
 *
 * Copyright 2009 Afatech, Inc. All rights reserved.
 *
 */


#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/i2c.h>
#include <linux/kthread.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>

#include <asm/system.h>
#include "error.h"
#include "user.h"
#include "register.h"
#define __OMAGADEMOD_H__
#include "standard.h"
#include "tuner.h"
#include "Omega.h"
//#include "Firmware_Afa_Omega_Script.h"
//#include "Firmware_Afa_Omega_LNA_Config_1_Script.h"
#include "Firmware_Afa_Omega_LNA_Config_2_Script.h"

Demodulator* Afatech_OMEGA_demodulator;

Dword OMEGA_open (
	IN  Demodulator*	demodulator
) {
	Dword error = Error_NO_ERROR;
	Afatech_OMEGA_demodulator = demodulator;
	error = omega_init();

	return (error);
}

Dword OMEGA_close (
	IN  Demodulator*	demodulator
) {
	return (Error_NO_ERROR);
}

Dword OMEGA_set (
	IN  Demodulator*	demodulator,
	IN  Word			bandwidth,
	IN  Dword			frequency
) {
	Dword error = Error_NO_ERROR;
	Afatech_OMEGA_demodulator = demodulator;
	error = omega_setfreq((unsigned int)bandwidth, (unsigned int)frequency);
	
	return (error);
}


TunerDescription tunerDescription= {
    OMEGA_open,
    OMEGA_close,
    OMEGA_set,
    NULL,
    NULL,
    OMEGA_ADDRESS,			/** tuner i2c address */
    2,						/** length of tuner register address */
    0,						/** tuner if */
    False,					/** spectrum inverse */
    0x38,					/** tuner id */
};

Dword OMEGA_supportLNA (
    IN  Demodulator*    demodulator,
    IN  Byte            supporttype
 ) {
     Dword error = Error_INVALID_DEV_TYPE;

	 switch (supporttype){
		case OMEGA_NORMAL:
			tunerDescription.tunerScriptTable = OMEGA_LNA_Config_2_scripts;
            tunerDescription.tunerScriptSetsTable = OMEGA_LNA_Config_2_scriptSets;
    printk("TSTV:ITE9133: OMEGA_NORMAL  ------ \t%s[%8d]\n",  __FUNCTION__, __LINE__);            
            tunerDescription.tunerId = 0x38;
            error = Error_NO_ERROR;
			break;
		case OMEGA_LNA_Config_1:
			tunerDescription.tunerScriptTable = OMEGA_LNA_Config_2_scripts;
            tunerDescription.tunerScriptSetsTable = OMEGA_LNA_Config_2_scriptSets;
    printk("TSTV:ITE9133:  OMEGA_LNA_Config_1  ------ \t%s[%8d]\n",  __FUNCTION__, __LINE__);        
            tunerDescription.tunerId = 0x51;
            error = Error_NO_ERROR;
			break;
		case OMEGA_LNA_Config_2:
    printk("TSTV:ITE9133: OMEGA_LNA_Config_2  ------ \t%s[%8d]\n",  __FUNCTION__, __LINE__);  			
			tunerDescription.tunerScriptTable = OMEGA_LNA_Config_2_scripts;
            tunerDescription.tunerScriptSetsTable = OMEGA_LNA_Config_2_scriptSets;
            tunerDescription.tunerId = 0x52;
            error = Error_NO_ERROR;
			break;
		default:

			break;

	 }
	 
	 return error;
  }
