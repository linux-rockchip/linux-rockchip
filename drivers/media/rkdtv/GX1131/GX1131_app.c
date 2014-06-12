/**
 * CopyRight: all rights reserved by rockchips fuzhou inc. 
 * 10.30.2010
 */


#include <TSTV.h>
#include "GX1131.h"

#include <mach/hardware.h>
#include <mach/iomux.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/rk29_iomap.h>

/********************************************************************************************/
//
//
//    DEBUG Options
//
//
/********************************************************************************************/
#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif


/********************************************************************************************/
//
//
//    Source Code
//
//
/********************************************************************************************/
char GX1131_init(void)
{
    bool  ret;
	
    ret = GX1131_Init_Chip();

    DBG("TSTV:GX1131:  init retval = %d ------ \t%s[%8d]\n",  ret, __FUNCTION__, __LINE__);	

    return 1; //success
}

void GX1131_deinit(void)
{
    /* deinit */
    return;
}

void GX1131_tune(unsigned int frequency_KHz, unsigned int bandwidth_KHz)
{
//bool GX1131_lock_TP_BS(GX1131_TP tp,  TunerDriver tunerDriver)

//GX1131_STATE GX_Set_AV2011(u32 RFfreq, u32 Symbol_Rate)
//GX1131_STATE GX1131_set_tuner(u32 RFfreq,u32 Symbol_Rate)
	GX1131_TP tp;
	TunerDriver tunerDriver;

	//tp.Freq_KHz = 1000000;
	//tp.SymRate_KSps = 25162;

	//tp.Freq_KHz = 1172000;
	//tp.SymRate_KSps = 33500;
	
	tp.V_polar = true;
	tp._22K_on = 	false;
//------------------------------------------------	
	tp.Freq_KHz = frequency_KHz;
	tp.SymRate_KSps = bandwidth_KHz;

	  if((bandwidth_KHz & 0x01) == 0) 
	  {
	  	tp.V_polar = false;  //V     
	  }
	  else
	 {
		tp.V_polar = true;  //H 			  	   
	  }	

	    if((bandwidth_KHz  & 0x02) == 0) 
	  {
	      tp._22K_on = true;// 22K ON
	  }
	  else
	  {
	      tp._22K_on = false;// 22K ON
	  }	
	
       DBG("TSTV:GX1131:   ------ \t%s[%8d]: Freq_KHz: %d, SymRate_KSps: %d, V_polar: %d, _22K_on: %d,\n",  __FUNCTION__, __LINE__, tp.Freq_KHz, tp.SymRate_KSps, tp.V_polar, tp._22K_on);
	GX1131_lock_TP_BS(tp,  GX1131_set_tuner);
	//GX1131_Search_Signal(tp,  GX1131_set_tuner);
}
void GX1131_signal_strength_quality(char* signal_quality, char* signal_strength);
char GX1131_lockstatus(void)
{
    GX1131_LOCK_STATUS lock_status = UNLOCKED;
char temp = 0;    
    
//GX1131_LOCK_STATUS GX1131_Read_Lock_Status(void)
/*
typedef enum _GX1131_LOCK_STATUS
{
	UNLOCKED,
	AGC_LOCKED,
	BCS_LOCKED,
	TIM_LOCKED,
	CRL_LOCKED,
	EQU_LOCKED,
	FEC_LOCKED
}GX1131_LOCK_STATUS;
*/

    DBG("TSTV:GX1131:   ------ \t%s[%8d]\n",  __FUNCTION__, __LINE__);
    //msleep(10);
	
    lock_status = GX1131_Read_Lock_Status();
    DBG("TSTV:GX1131:   ------ \t%s[%8d]£ºlock_status£º %d \n",  __FUNCTION__, __LINE__, lock_status);    
GX1131_signal_strength_quality(&temp, &temp);
if(lock_status >= FEC_LOCKED)
{
lock_status = 1;
}
else
{
	lock_status = 0;
}

lock_status  = 1;

    return lock_status;
}

//! PID filter control
/**
 * Enables or disabled the internal PID filter of a demod. If it is disabled,
 * the complete MPEG2 Transport is put out otherwise only selected PIDs are
 * passed through (see demod_pid_filter).
 *
 * \param onoff 0 = disable PID filter (default), 1 = enable
 */
int  GX1131_pid_filter_ctrl(uint8_t onoff)
{
    return 1;
}

//! PID filter table
/**
 * Enable a PID to be filtered. Disable the consideration of a position of the internal table.
 *
 * \param id          index of the internal table
 * \param pid        PID to be filtered
 * \param onoff     activate de-activate the given table index
 */
int GX1131_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
    return 1;
}

void GX1131_signal_strength_quality(char* signal_quality, char* signal_strength)
{
//u8 GX1131_Get_Signal_Strength(void)
//u8 GX1131_Get_Signal_Quality(void)
	*signal_strength = GX1131_Get_Signal_Strength();
	*signal_quality = GX1131_Get_Signal_Quality();

    return;	
}

//! i2c chip address
/**
 * MTV chip ID is two as 0x86 and 0xDO corresponding with TMODEx pin configuration
 * TMODE(TMOD2 = 0, TMODE1 = 0, TMODE0 = 0), i2c address: 0x86
 * TMODE(TMOD2 = 0, TMODE1 = 1, TMODE0 = 1), i2c address: 0xd0
 */
int GX1131_i2c_addr(void)
{
    return 0x86;
}

//! i2c speed
/**
 * maximum 400khz in fast-mode
 * 100 khz in standard-mode
 */
unsigned short GX1131_i2c_speed(void)
{
    return 100;
}

char GX1131_null(void)
{
    return 1;
}


/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "GX1131_CONTROL",
    GX1131_init,
    GX1131_deinit,
    GX1131_tune,
    GX1131_lockstatus,
    GX1131_pid_filter,
    GX1131_pid_filter_ctrl,
    GX1131_signal_strength_quality,
    GX1131_i2c_addr,
    GX1131_i2c_speed,
    GX1131_init 
};


/********************************************************************************************/
//
//
//    OVER
//
//
/********************************************************************************************/

