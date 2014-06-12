//
//
/********************************************************************************************/
#include <include/interface/host.h>

#include <include/adapter/frontend_tune.h>
#include <include/adapter/sip.h>
#include <include/sip/dibx090p.h>
#include <include/demod/dib7000p.h>
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
static int dib7790p_update_lna(struct dibFrontend *fe, uint16_t agc_global);

static const struct dibx090p_config tfe7790p_layout_config = {
    1,
    dib7790p_update_lna, // update_lna

    DIBX090P_GPIO_DEFAULT_DIRECTIONS,
    DIBX090P_GPIO_DEFAULT_VALUES,
    DIBX090P_GPIO_DEFAULT_PWM_POS,

    0,     // dib0090_freq_offset_khz_uhf
    0,     // dib0090_freq_offset_khz_vhf

    12000, // clock_khz
    0,      // diversity delay
    NULL,   // dib0090_wbd_table
    0,
    4,
    0,
    0,
    0,
    1,      // crystal mode
    0x2d98, // dib8k_drives
    0,
    1, //clkouttobamse
    0,
    0,
    0,
};

static int dib7790p_update_lna(struct dibFrontend *fe, uint16_t agc_global)
{
    uint16_t agc1, agc2,wbd, wbd_target,wbd_offset,threshold_agc1;
    int16_t  wbd_delta;

    if ((fe->current_rf)<400000)
        threshold_agc1 = 25000;
    else
        threshold_agc1 = 30000;

    wbd_target = (dib0090_get_wbd_target(fe, fe->current_rf)*8+1)/2;
    wbd_offset = dib0090_get_wbd_offset(fe);
    dib7000p_get_agc_values(fe, NULL, &agc1, &agc2, &wbd);
    wbd_delta =(int16_t)wbd - (((int16_t)wbd_offset+10)*4) ;

    //dbgpl(NULL, "update lna, agc_global=%d agc1=%d agc2=%d\n", agc_global, agc1, agc2);
    //dbgpl(NULL, "update lna, wbd=%d wbd target=%d wbd offset=%d wbd delta=%d\n", wbd, wbd_target, wbd_offset, wbd_delta);

    if ((agc1 < threshold_agc1) && (wbd_delta > 0)) {
        //dbgpl(NULL, "agc global low (%d) and wbd high (%d) ==> set the switch to 0 \n", agc_global, wbd);
        dib0090_set_switch(fe, 1, 1, 1);
        dib0090_set_vga(fe, 0);
        dib0090_update_rframp_7090(fe, 0);
        dib0090_update_tuning_table_7090(fe, 0);
    } else {
        //dbgpl(NULL, "wbd low (%d) ==> set the vga to 1\n ", wbd);
        dib0090_set_vga(fe, 1);
        dib0090_update_rframp_7090(fe, 1);
        dib0090_update_tuning_table_7090(fe, 1);
        dib0090_set_switch(fe, 0, 0, 0);
    }

    if ((tfe7790p_layout_config.use_high_level_adjustment & 1) && (agc_global < 16000))
        demod_set_gpio(fe, 8, 0, 0);

    return 0;
}

char DVB_Module_Init()
{
	struct dibDataBusHost *i2c = (struct dibDataBusHost *)rkdroid_open_i2c();
	DibZeroMemory(mon, sizeof(struct dibDemodMonitor)*NUM_OF_DEMOD);
    frontend_init(&fe[0]); /* initializing the frontend-structure */
    frontend_set_id(&fe[0], 0); /* assign an absolute ID to the frontend */
    frontend_set_description(&fe[0], "ISDB-T #0 Master");
	
	if ( dib7090e_sip_register(&fe, i2c, 0x10, &tfe7790p_layout_config) == NULL)
			return DIB_RETURN_ERROR;
	
	if (dib7000p_i2c_enumeration(i2c, 1, 0x10, 0x10) != DIB_RETURN_SUCCESS) {
		DibDbgPrint("-E-  Can not find dib7090\n");
		return DIB_RETURN_ERROR;
		}

   if(DIB_RETURN_SUCCESS != frontend_reset(&fe[0]))
        return DIB_RETURN_ERROR;
   
   //dib7000p_set_gpio(&fe[0], 8, 0, 1);;
        
	DBG("%s: OK!", __FUNCTION__);
	return 1;


}

void DVB_Module_Tune(unsigned int frequency,char bandwidth)
{
	
	static struct dibChannel ch ;
	DBG("%s: frequency: %d, bandwidth: %d!\n", __FUNCTION__, frequency, bandwidth);
	INIT_CHANNEL(&ch, STANDARD_DVBT);

	ch.RF_kHz = frequency;

	ch.bandwidth_kHz     = 8 * 1000;  //INDEX_BW_6_0_MHZ;

	tune_diversity(&fe[0], NUM_OF_DEMOD, &ch);
	//set_streaming_dib0700(&fe[0], &ch, 0, 1);

}


void DVB_Module_Deinit()
{
    frontend_unregister_components(&fe[0]);
	//set_streaming_dib0700(&fe[0], &ch, 1, 0);
    rkdroid_close_i2c();
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
    "DIBCOM7090_CONTROL",
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

