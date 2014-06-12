//
//
/********************************************************************************************/
#include <include/interface/host.h>

#include <include/adapter/frontend_tune.h>
#include <include/adapter/sip.h>
#include <include/sip/dibx090p.h>
#include <include/demod/dib8000p.h>
#include <include/monitor/monitor.h>
#include "DiBcom_i2c_app.h"



#include <TSTV.h>

#if 0
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif  
 //使用的demod的数量
#define NUM_OF_DEMOD 1
struct dibFrontend fe[1];
struct dibDemodMonitor mon[1];


typedef struct
{
      uint32_t   bUsed;
      uint32_t iPid;
}DVBPIDFilterInfo;

DVBPIDFilterInfo g_iPidFilter[32];

static int tfe8096p_update_lna(struct dibFrontend *fe, uint16_t agc_global);

static const struct dibx090p_config tfe8096p_layout_config = {
    1,
    tfe8096p_update_lna, // update_lna
    DIBX090P_GPIO_DEFAULT_DIRECTIONS,
    DIBX090P_GPIO_DEFAULT_VALUES,
    DIBX090P_GPIO_DEFAULT_PWM_POS,
    -143, //freq_offset_khz_uhf;
    -143, //freq_offset_khz_vhf;
    12000,  // clock_khz
    48,   //diversity_delay
    NULL,   // dib0090_wbd_table
    0, 0, 0, 0,
    0, // use high level adjustment
};
static int tfe8096p_update_lna(struct dibFrontend *fe, uint16_t agc_global)
{
    dbgpl(NULL, "update lna, agc_global=%d", agc_global);
    if ((tfe8096p_layout_config.use_high_level_adjustment & 1) && (agc_global < 16000))
        demod_set_gpio(fe, 5, 0, 0);

    return 0;
}

char DVB_Module_Init()
{
	struct dibDataBusHost *i2c = (struct dibDataBusHost *)rkdroid_open_i2c();
	DibZeroMemory(mon, sizeof(struct dibDemodMonitor)*NUM_OF_DEMOD);
    frontend_init(&fe[0]); /* initializing the frontend-structure */
    frontend_set_id(&fe[0], 0); /* assign an absolute ID to the frontend */
    frontend_set_description(&fe[0], "ISDB-T #0 Master");
	if (dib8090p_sip_register(&fe[0], i2c, 0x10, &tfe8096p_layout_config) == NULL)
		   return DIB_RETURN_ERROR;
	
   if (dib8000p_i2c_enumeration(i2c, 1, DIBX090P_DEFAULT_I2C_ADDRESS, 0x10) != DIB_RETURN_SUCCESS) {
	   DibDbgPrint("-E-  Can not find dib8096p\n");
	   return DIB_RETURN_ERROR;
   }
   if(DIB_RETURN_SUCCESS != frontend_reset(&fe[0]))
        return DIB_RETURN_ERROR;
   
   //demod_set_gpio(&fe, 8, 0, 1);
        
	DBG("%s: OK!", __FUNCTION__);
	return 1;

}

void DVB_Module_Tune(unsigned int frequency,char bandwidth)
{
	local_irq_disable();
	static struct dibChannel ch ;
	DBG("%s: frequency: %d, bandwidth: %d!\n", __FUNCTION__, frequency, bandwidth);
	INIT_CHANNEL(&ch, STANDARD_ISDBT);

	ch.RF_kHz = frequency;

	ch.bandwidth_kHz     = 6 * 1000;  //INDEX_BW_6_0_MHZ;

	tune_diversity(&fe[0], NUM_OF_DEMOD, &ch);
	//set_streaming_dib0700(&fe[0], &ch, 0, 1);

}


void DVB_Module_Deinit()
{
    frontend_unregister_components(&fe[0]);
    rkdroid_close_i2c();
	//set_streaming_dib0700(&fe[0], &ch, 1, 0);
	DBG("%s: OK!", __FUNCTION__);
}


char DVB_Module_Lockstatus()
{ int j;
    uint8_t bLock =0, lock;
    struct dibDVBSignalStatus status1;

	
	mdelay(10);
	
	if (fe[0].demod->demod_info->ops.get_signal_status(fe[0].demod, &status1) != 0)
	{
			DBG("-E-  Get signal status failed\n");
	}
	 bLock =status1.mpeg_data_lock;
	 lock = status1.tps_data_lock;
	 //printk("MASTER DIBCOM bLOCK : %d lock: %d\n		  ",bLock, lock);
	if(bLock)
	{
		DBG("MASTER DIBCOM LOCK OK %d......\n		 ");
	}
	//return 1;

	return (bLock);

}

void DVB_Module_signal_strenth_quality(unsigned char *signal_quality, unsigned char *signal_strength)
{ 
	   struct dibDVBSignalStatus status;
	 //   static struct dibDemodMonitor demodmon;
	
		if (fe[0].demod->demod_info->ops.get_signal_status(fe[0].demod, &status) != 0)
		{
				DibDbgPrint("-E-  Get signal status failed\n");
		}
	
		//*signal_strength = (unsigned char)(153 + (unsigned char)((255- 153)* (((float)status.signal_strength)/255.0)));
		*signal_quality = (unsigned char)(((long)(2097151 - status.bit_error_rate)/2097151)*255);
		*signal_strength = (unsigned char)(153 + (unsigned char)((255 - 153)*(((long)status.signal_strength)/255)));
	
		if(*signal_quality < 1)
			*signal_quality = 1;
		else if (*signal_quality >255)
			*signal_quality =255;
	
		if(*signal_strength < 1)
			*signal_strength = 1;
		else if (*signal_strength >255)
			*signal_strength =255;
	
	//	  printf("quality = %d strength = %d\n", *signal_quality, *signal_strength);

}


//! PID filter control
/**
 * Enables or disabled the internal PID filter of a demod. If it is disabled,
 * the complete MPEG2 Transport is put out otherwise only selected PIDs are
 * passed through (see demod_pid_filter).
 *
 * \param onoff 0 = disable PID filter (default), 1 = enable
 */
int  DVB_demod_pid_filter_ctrl(uint8_t onoff)
{ 
	if(demod_pid_filter_ctrl(&fe[0],onoff) ==DIB_RETURN_ERROR)
		return 0;
	if(onoff ==0)
	{
		uint32_t i;
		for(i=0;i<32;i++)
		{
			if(g_iPidFilter[i].bUsed ==1)
			{
				demod_pid_filter(&fe[0], i, g_iPidFilter[i].iPid,onoff);
				g_iPidFilter[i].bUsed =0;
				g_iPidFilter[i].iPid  =0x1fff;
			}
		}
	}	
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
int DVB_demod_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
	 
	if(demod_pid_filter(&fe[0], id, pid,onoff)==DIB_RETURN_ERROR)
		return 0;
	if(onoff )
	{
		uint32_t i;
		for(i=0;i<32;i++)
		{
			if(g_iPidFilter[i].bUsed ==0)
			{
				g_iPidFilter[i].bUsed =1;
				g_iPidFilter[i].iPid =pid;
				break;
			}
		}		
	}
	else
	{
		uint32_t i;
		for(i=0;i<32;i++)
		{
			if((g_iPidFilter[i].bUsed ==1)&&(g_iPidFilter[i].iPid ==pid))
			{
				g_iPidFilter[i].bUsed =0;
				g_iPidFilter[i].iPid  =0x1fff;
				break;
			}
		}
	}
	return 1;
}

void DVB_demod_turn_with_no_delay(unsigned int frequency,char bandwidth)
{
}

void  DVB_demod_refresh_signal_status(unsigned int frequency,char bandwidth)
{ 

}

int DVB_Module_i2c_addr(void)
{
    return 0x12;
    //return 0x90;
}

unsigned short DVB_Module_i2c_speed(void)
{
    return 120;//80;//100;//500;
}

char DVB_Module_NULL(void)
{
    return 1;
}


/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "DIBCOM8096_CONTROL",
    DVB_Module_Init,
    DVB_Module_Deinit,
    DVB_Module_Tune,
    DVB_Module_Lockstatus,
    DVB_demod_pid_filter,
    DVB_demod_pid_filter_ctrl,
    DVB_Module_signal_strenth_quality,
    DVB_Module_i2c_addr,
    DVB_Module_i2c_speed,
    DVB_Module_NULL
};

/********************************************************************************************/
//
//
//    OVER
//
//
/********************************************************************************************/

