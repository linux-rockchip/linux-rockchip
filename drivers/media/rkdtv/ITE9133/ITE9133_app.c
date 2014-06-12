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
#include <linux/timer.h>
#include <linux/timex.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>
#include <asm/system.h>

#include <TSTV.h>

#include "api/IT9133.h"
#include "api/tuner.h"

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

#define ITE9133_TRUE  1
#define ITE9133_FALSE 0 

StreamType streamType = StreamType_DVBT_PARALLEL;
extern Handle I2c_handle;


DefaultDemodulator Global_demod = {
	NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    20480,
    20480000,
    StreamType_DVBT_PARALLEL,
    8000,
    642000,
    0x00000000,
	{False, False, 0, 0},
    0,
    False,
    False,
	0,
	User_I2C_ADDRESS,
	False
};

unsigned long GetTickCount(void)
{
    unsigned long time_ms = 0;
    struct timeval tp; 

    do_gettimeofday(&tp); 

    time_ms = (1000000*tp.tv_sec + tp.tv_usec)/1000;     
    
    return time_ms;	
}

char Example_initialize (Demodulator *demod) {
    Dword error = Error_NO_ERROR;
    Dword start = 0, end = 0;

    //Initialize with StreamType (StreamType_DVBT_PARALLEL or StreamType_DVBT_SERIAL)
    start = GetTickCount();
    error = OMEGA_supportLNA(demod, 0x02); // with External LNA Config. 2 design (IT9133FN_NIM_TS_ V15)
    if (error) return;	
    error = Demodulator_initialize (demod, streamType);
    end = GetTickCount();
	if (error){
		printk("Initialize failed.0x%08x\n", error);
		return ITE9133_FALSE;
	}
    printk("Initialize Time: %lld msec\n", ((long)(end-start)));
    printk("Initialize successful.\n");
    
    return ITE9133_TRUE;  
}

char Example_finalize (Demodulator *demod) {
    Dword error = Error_NO_ERROR;

    //Finialize demodulator and Tuner when device is off
    error = Demodulator_finalize (demod);
	if (error){
		printk("Finalize failed.0x%08x\n", error);
		return ITE9133_FALSE;
	}

    printk("Finalize successful.\n");
    return ITE9133_TRUE;      
}
void Example_OnOffPowerControl (Demodulator *demod, Byte bOn) {
    Dword error = Error_NO_ERROR;

    //Enable Power control function
    error = Demodulator_enableControlPowerSaving(demod, bOn);
    if (error) goto exit;        

exit:
    if (error) printk("Error = 0x%08x\n", error);
    else printk("Power Control is %s Successfully! \n", bOn?"On":"Off");
}

void Example_PowerControl (Demodulator *demod, Byte enable) {
    Dword error = Error_NO_ERROR;

    error = Demodulator_controlPowerSaving(demod, enable);
    if (error) goto exit;        

exit:
    if (error) printk("Error = 0x%08x\n", error);
    else printk("Power Control %s Successfully! \n", enable?"Enable":"Disable");
}

void Example_acquireChannel (Demodulator *demod, Word bandwidth, Dword freq) {
    Dword error = Error_NO_ERROR;
    Bool locked;
    Bool found = False;

    //Set Channel
    error = Demodulator_acquireChannel (demod, bandwidth, freq);
    if (error) goto exit;
    
    //Wait demodulator ready and Check if Locked
    //error = Demodulator_isLocked (demod, &locked);
    //if (error) goto exit;

    //printk("Channel is %slocked!\n", locked?"":"not ");

exit:
    if (error) printk("Error = 0x%08x\n", error);
}

void Example_monitorVersion (Demodulator *demod) {
    Dword error = Error_NO_ERROR;
	Dword version = 0;

	printk("API Version = %04x.%08x.%02x\n", Version_NUMBER, Version_DATE, Version_BUILD);

	error = Demodulator_getFirmwareVersion (demod, Processor_LINK, &version);
	if (error) {
		printk("Demodulator_getFirmwareVersion(LINK) failed! Error = 0x%08x\n", error);
		return;
	} else {
		printk("LINK FW Version = 0x%08x\n", version);
	}
	error = Demodulator_getFirmwareVersion (demod, Processor_OFDM, &version);
	if (error) {
		printk("Demodulator_getFirmwareVersion(OFDM) failed! Error = 0x%08x\n", error);
		return;
	} else {
		printk("OFDM FW Version = 0x%08x\n", version);
	}
}

void Example_monitorStatistic (Demodulator *demod) {
    
    Dword error = Error_NO_ERROR;
    Statistic stat;
    Dword postErrorCount=0;
    Dword postBitCount=0;
    Word  abortCount=0;
	Word  cnt=0;
    Long dbm = 0;
    
	do{
		error = Demodulator_getStatistic(demod, &stat);
		if (error) {
			printk("Demodulator_getStatistic failed!Error = 0x%08x\n", error);
			break;
		} else {
			printk("Signal Present = %s, Locked = %s\n", stat.signalPresented?"TRUE":"FALSE", stat.signalLocked?"TRUE":"FALSE");
			printk("Signal Quality = %d, Strength = %d\n", stat.signalQuality, stat.signalStrength);
		}

		error = Demodulator_getSignalStrengthDbm(demod, 18, 18, &dbm);
		if (error) {
			printk("Demodulator_getSignalStrengthDbm failed!Error = 0x%08x\n", error);
			break;
		} else {
			printk("Signal Strength = %d(DBm)\n", dbm);
		}   

		error = Demodulator_getPostVitBer (demod, &postErrorCount, &postBitCount, &abortCount);
		if (error) {
			printk("Demodulator_getPostVitBer failed!Error = 0x%08x\n", error);
			break;
		} else {
			if(postErrorCount){ 
				while(postErrorCount<postBitCount){
					postErrorCount = postErrorCount*10;
					cnt++;
				}
			}
			//printk("BER = %.03f+E%d, Abort Cnt  = %d\n", (double)postErrorCount/(double)postBitCount,cnt, abortCount);
		}

	}while(0);
}

/********************************************************************************************/
//
//
//    Source Code
//
//
/********************************************************************************************/
char ITE9133_init(void)
{
    char  ret;
    
    ret = Example_initialize(&Global_demod);
    Demodulator_resetPidFilter(&Global_demod);

    return ret; //success
}

void ITE9133_deinit(void)
{
    /* deinit */
    char  ret;
    
    ret = Example_finalize(&Global_demod);

    return;    
}

void ITE9133_tune(unsigned int frequency_KHz, unsigned int  bandwidth_KHz)
{
    Example_acquireChannel(&Global_demod, bandwidth_KHz, frequency_KHz);

}

char ITE9133_lockstatus(void)
{
    bool locked;
    Dword error = Error_NO_ERROR;    
    
    error = Demodulator_isLocked (&Global_demod, &locked);
    if (error)
    {
        DBG("TSTV:ITE9133:   Demodulator_isLocked error ------ \t%s[%8d]\n",  __FUNCTION__, __LINE__);    	
    } 
    printk("Channel is %slocked!\n", locked?"":"not ");
    DBG("TSTV:ITE9133:   ------ \t%s[%8d]\n",  __FUNCTION__, __LINE__);
    //msleep(10);

    return (char)locked;
}

//! PID filter control
/**
 * Enables or disabled the internal PID filter of a demod. If it is disabled,
 * the complete MPEG2 Transport is put out otherwise only selected PIDs are
 * passed through (see demod_pid_filter).
 *
 * \param onoff 0 = disable PID filter (default), 1 = enable
 */
int  ITE9133_pid_filter_ctrl(uint8_t onoff)
{
    Demodulator_controlPidFilter (&Global_demod, onoff);

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
int ITE9133_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
    if(onoff)
    {
        Demodulator_addPidToFilter (&Global_demod,  id,  pid);
    }

    return 1;
}

#if 1
void ITE9133_signal_strength_quality(char *quality, char *strength)
{
	Dword error = Error_NO_ERROR;
	Statistic stat;
    	
	error = Demodulator_getStatistic(&Global_demod, &stat);
	if (error) {
		printk("Demodulator_getStatistic failed!Error = 0x%08x\n", error);
		*quality = 100;
	       *strength = 100;
		goto exit;
	} else {
		printk("Signal Present = %s, Locked = %s\n", stat.signalPresented?"TRUE":"FALSE", stat.signalLocked?"TRUE":"FALSE");
		printk("Signal Quality = %d, Strength = %d\n", stat.signalQuality, stat.signalStrength);
	}
	
	*quality = stat.signalQuality;
	*strength = stat.signalStrength;
exit:
	
    return;	
}
#endif

int ITE9133_i2c_addr(void)
{
    return 0x86;
}

unsigned short ITE9133_i2c_speed(void)
{
    return 100;
}

char ITE9133_null(void)
{
    return 1;
}


/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "ITE9133_CONTROL",
    ITE9133_init,
    ITE9133_deinit,
    ITE9133_tune,
    ITE9133_lockstatus,
    ITE9133_pid_filter,
    ITE9133_pid_filter_ctrl,
    ITE9133_signal_strength_quality,
    ITE9133_i2c_addr,
    ITE9133_i2c_speed,
    ITE9133_null 
};
