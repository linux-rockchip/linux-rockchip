/********************************************************************************************/
//dibcom 9080 需要的程序
//
//
//dibcom 的头文件
//
//
/********************************************************************************************/
#include<linux/string.h>
#include <linux/module.h>

#include "../TSTV.h"
#include "Auvitek_Dev.h"
#include "AuvitekTunerInterfaceTypedef.h"

//typedef int INT32U;
//typedef  unsigned int UINT32;

static int AU8524_signal_state = 0;

char AU8524_init()
{
    UINT32 ret = -1;
	
    ret = AU8524_INIT();

    return 1;	
}

void AU8524_tune(unsigned int frequency, unsigned int bandwidth)
{
    UINT32 ret = -1;
	
    ret = AU8524_Set_Status(frequency);
    AU8524_signal_state = ret;		
//AU8524_signal_state = 1;	

    return ret;	
}

extern UINT32 AU8524_Close(void);
void AU8524_Deinit()
{
    UINT32 ret = -1;
	
    ret =AU8524_Close();

    return ret;		
}


extern G_APPCTHandle m_GAPPCTHandle;
extern TUNER_NTSTATUS  AU_GetChipLock(UINT32 ChipUnit,UINT32 * nLock);
char AU8524_lockstatus()
{
    UINT32 Status = 0;
    AU_GetChipLock(m_GAPPCTHandle.m_AUChipHandle, &Status);
     if(Status == 0)
     {
        AU8524_signal_state  = 1;
     }
//AU8524_signal_state = 1;
	printk("AU Get chip Lock , the status is %d\n",AU8524_signal_state);
    return AU8524_signal_state;
}

void AU8524_signal_strenth_quality(char *signal_quality, char *signal_strength)
{

}

int  AU8524_demod_pid_filter_ctrl(uint8_t onoff)
{

}

int AU8524_demod_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
	
}

int AU8524_i2c_addr(void)
{
    return 0x8e;
}

unsigned short AU8524_i2c_speed(void)
{
    return 100;
}

char AU8524_NULL(void)
{
    return 1;
}


/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "AU8524_CONTROL",
    AU8524_NULL,
    AU8524_Deinit,
    AU8524_tune,
    AU8524_lockstatus,
    AU8524_demod_pid_filter,
    AU8524_demod_pid_filter_ctrl,
    AU8524_signal_strenth_quality,
    AU8524_i2c_addr,
    AU8524_i2c_speed,
    AU8524_init
};




