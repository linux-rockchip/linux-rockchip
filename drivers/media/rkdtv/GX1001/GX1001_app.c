/**
 * CopyRight: all rights reserved by rockchips fuzhou inc. 
 * 10.30.2010
 */


#include <TSTV.h>
#include "GX1001_V8.2.5.h"

#include <mach/hardware.h>
#include <mach/iomux.h>
#include <mach/gpio.h>
#include <mach/irqs.h>

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

int  gGX1001_lock_status = 0;

/********************************************************************************************/
//
//
//    Source Code
//
//
/********************************************************************************************/
char GX1001_init(void)
{
    bool  ret;
	
    gGX1001_lock_status = 0;
    GX1001_TDA18250_Init_Chip();

    DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);

    return 1; //success
}

void GX1001_deinit(void)
{
    /* deinit */
    return;
}

void GX1001_tune(unsigned int frequency_KHz, unsigned int bandwidth_KHz)
{
/*
Input:
Symbol_Rate_1   --  Used first symbol rate value (range: 450 -- 9000)     (Unit: kHz)
Symbol_Rate_2	--  Used second symbol rate value. Please set 0 if no use	(Unit: kHz)

Spec_Mode	    --	0£ºsearch only positive spectrum
1£ºsearch only negative spectrum
2£ºfirst positive and then negative spectrum
3£ºfirst negative and then positive spectrum

Qam_Size	    --  0-2 = reserved;
3 = 16QAM;
4 = 32QAM; 
5 = 64QAM; 
6 = 128QAM;
7 = 256QAM.

RF_Freq		    --  The RF frequency (KHz)

Wait_OK_X_ms    --  The waiting time before give up one search ( Unit: ms )
(Range: 250ms -- 2000ms, Recommend: 700ms)
*/
    //extern S32 GX1001_Search_Signal(U32 Symbol_1,U32 Symbol_2,U8 Spec_Mode,U8 Qam_Size,U32 RF_Freq,U32 Wait_OK_X_ms,fe_tunertype_t  tunertype);
    U32 Symbol_1 = 6875;  
    U32 Symbol_2 = 0; 
    U8 Spec_Mode = 0;
    U8 Qam_Size = 5; 
    U32 RF_Freq = 482000;
    U32 Wait_OK_X_ms= 2000;
    fe_tunertype_t  tunertype = TUNER_USER1;

    DBG("TSTV:GX1001:   ------ \t%s[%8d]: Symbol_1 = %d, Symbol_2 = %d, Spec_Mode = %d, Qam_Size = %d, RF_Freq = %d, Wait_OK_X_ms = %d, tunertype = %d,\n",  __FUNCTION__, __LINE__, Symbol_1, Symbol_2, Spec_Mode, Qam_Size, RF_Freq, Wait_OK_X_ms, tunertype);   
     
    if(SUCCESS == GX1001_Search_Signal(Symbol_1, Symbol_2, Spec_Mode, Qam_Size, RF_Freq, Wait_OK_X_ms, tunertype))
    {
    	gGX1001_lock_status = 1;
    }
}
void GX1001_signal_strength_quality(char* signal_quality, char* signal_strength);
char GX1001_lockstatus(void)
{   
    char lock_status;
	
    DBG("TSTV:GX1001:   ------ \t%s[%8d]: gGX1001_lock_status = %d. \n",  __FUNCTION__, __LINE__, gGX1001_lock_status);
 
    if(gGX1001_lock_status == SUCCESS)
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
int  GX1001_pid_filter_ctrl(uint8_t onoff)
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
int GX1001_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
    return 1;
}

void GX1001_signal_strength_quality(char* signal_quality, char* signal_strength)
{
//u8 GX1001_Get_Signal_Strength(void)
//u8 GX1001_Get_Signal_Quality(void)
	*signal_strength = GX1001_Get_TDA18250_Signal_Strength();
	*signal_quality = GX1001_Get_Signal_Quality();

    return;	
}

//! i2c chip address
/**
 * MTV chip ID is two as 0x86 and 0xDO corresponding with TMODEx pin configuration
 * TMODE(TMOD2 = 0, TMODE1 = 0, TMODE0 = 0), i2c address: 0x86
 * TMODE(TMOD2 = 0, TMODE1 = 1, TMODE0 = 1), i2c address: 0xd0
 */
int GX1001_i2c_addr(void)
{
    return 0xC0;
}

//! i2c speed
/**
 * maximum 400khz in fast-mode
 * 100 khz in standard-mode
 */
unsigned short GX1001_i2c_speed(void)
{
    return 100;
}

char GX1001_null(void)
{
    return 1;
}


/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "GX1001_CONTROL",
    GX1001_init,
    GX1001_deinit,
    GX1001_tune,
    GX1001_lockstatus,
    GX1001_pid_filter,
    GX1001_pid_filter_ctrl,
    GX1001_signal_strength_quality,
    GX1001_i2c_addr,
    GX1001_i2c_speed,
    GX1001_init 
};


/********************************************************************************************/
//
//
//    OVER
//
//
/********************************************************************************************/

