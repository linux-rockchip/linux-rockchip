/**
 * CopyRight: all rights reserved by rockchips fuzhou inc. 
 * 10.30.2010
 */


#include <TSTV.h>
#include "raontv.h"
#include "raontv_types.h"

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

#define MTV818_LIST_CHANNEL_INFO

/********************************************************************************************/
//
//
//    Source Code
//
//
/********************************************************************************************/

/*TMCC Information*/
char* rtv_isdbt_seg[] = {"1seg", "3seg"};

char* rtv_isdbt_tvmode[] = {"2048", "4096", "8192 fft"};

char* rtv_isdbt_guard[] = {"1/32", "1/16", "1/8", "1/4"};

char* rtv_isdbt_modulation[] = {"DQPSK", "QPSK", "16QAM", "64QAM"};

char* rtv_isdbt_coderate[] = {"1/2", "2/3", "3/4", "5/6", "7/8"};

char* rtv_isdbt_interlave[] = {"0", "1", "2", "4", "8", "16", "32"};


/*channel 7 to channel 69*/
unsigned int isdbt_freq_tbl[] = {
    //ch7
    177143, 183143, 189143, 195143, 201143, 207143, 213143, //vhf
    473143, 479143, 485143, 491143, 497143, 503143, 509143, //uhf
    515143, 521143, 527143, 533143, 539143, 545143, 551143,
    557143, 563143, 569143, 575143, 581143, 587143, 593143,
    599143, 605143, 611143, 617143, 623143, 629143, 635143,
    641143, 647143, 653143, 659143, 665143, 671143, 677143,
    683143, 689143, 695143, 701143, 707143, 713143, 719143,
    725143, 731143, 737143, 743143, 749143, 755143, 761143,
    767143, 773143, 779143, 785143, 791143, 797143, 803143
}; //khz

#ifdef MTV818_LIST_CHANNEL_INFO
extern void mtv818_signal_strength_quality(char* signal_quality, char* signal_strength);
#endif

unsigned int isdbtFreq_to_Ch(unsigned int frequency)//khz
{
    unsigned int ch;
    
    for(ch = 0; ch < 63; ch++)
    {
        if(frequency == isdbt_freq_tbl[ch])
        {
            return (ch + 7);
        }
    }

    /*not found*/
    return 0;
}

char mtv818_init(void)
{
    int ret;

    /* Initialize the ISDB-T 1seg service with 8MHz ADC sampling clock. */
    ret = rtvISDBT_Initialize(RTV_ADC_CLK_FREQ_8_MHz);
    if(ret != RTV_SUCCESS)
    {
        /* Error */
        return -1;
    }

    return 1;
}

void mtv818_deinit(void)
{
    /* deinit */
    return;
}

void mtv818_scanfrequency(unsigned int frequency, char bandwidth)
{
    int ret;
    unsigned int ch;
#ifdef MTV818_LIST_CHANNEL_INFO
    char signal_quality = 0;
    char signal_strength = 0;
#endif

    ch = isdbtFreq_to_Ch(frequency);
    if(ch == 0)
    {
        DBG("MTV818: inllegal frequency found [%s]\n", __FUNCTION__);
        return;
    }

    ret = rtvISDBT_ScanFrequency(ch);
    if(ret == RTV_SUCCESS)
    {
        /* Add a scanned channel to channel-list array */
        DBG("MTV818: scan frequency success!\n");

        /* show channel information such as tmcc */
#ifdef MTV818_LIST_CHANNEL_INFO
        mtv818_signal_strength_quality(&signal_quality, &signal_strength);
#endif
    }
    else
    {
        /* Error or Not detected */
        DBG("MTV818: scan frequency failed!\n");
    }
}

void mtv818_setfrequency(unsigned int frequency, char bandwidth)
{
    int ret;
    unsigned int ch;
#ifdef MTV818_LIST_CHANNEL_INFO
    char signal_quality = 0;
    char signal_strength = 0;
#endif

    ch = isdbtFreq_to_Ch(frequency / 1000);
    if(ch == 0)
    {
        DBG("MTV818: inllegal frequency found [%s]\n", __FUNCTION__);
        return;
    }

    /* PLAY a selected channel from user. */
    ret = rtvISDBT_SetFrequency(ch);
    if(ret == RTV_SUCCESS)
    {
        /* GET the informations of scanned channel */
        DBG("MTV818: set frequency success!\n");

        /* show channel information such as tmcc */
#ifdef MTV818_LIST_CHANNEL_INFO
        mtv818_signal_strength_quality(&signal_quality, &signal_strength);
#endif
    }
    else
    {
        DBG("MTV818: set frequency failed!\n");
    }
}

void mtv818_tune(unsigned int frequency_KHz, unsigned int  bandwidth_KHz)
{
    mtv818_scanfrequency(frequency_KHz, bandwidth_KHz);
    //mtv818_setfrequency(frequency, bandwidth);
}

char mtv818_lockstatus(void)
{
    char block = 0;

    if(rtvISDBT_GetLockStatus() == RTV_ISDBT_CHANNEL_LOCK_OK)
    {
        DBG("MTV818: isdbt channel lock ok!\n");
        block = 1;
    }

    return block;
}

//! PID filter control
/**
 * Enables or disabled the internal PID filter of a demod. If it is disabled,
 * the complete MPEG2 Transport is put out otherwise only selected PIDs are
 * passed through (see demod_pid_filter).
 *
 * \param onoff 0 = disable PID filter (default), 1 = enable
 */
int  mtv818_pid_filter_ctrl(uint8_t onoff)
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
int mtv818_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
    return 1;
}

void mtv818_signal_strength_quality(char* signal_quality, char* signal_strength)
{
    int rssi;
    unsigned int per, ber, cnr;
    RTV_ISDBT_TMCC_INFO tmcc;

    return 0;	

    /* GET the informations of scanned channel */
    per = rtvISDBT_GetPER();
    rssi = rtvISDBT_GetRSSI() / (int)RTV_ISDBT_RSSI_DIVIDER;
    ber = rtvISDBT_GetBER() / (unsigned int)RTV_ISDBT_BER_DIVIDER;
    cnr = rtvISDBT_GetCNR() / (unsigned int)RTV_ISDBT_CNR_DIVIDER;
    rtvISDBT_GetTMCC(&tmcc);

    *signal_quality = (char)(((long)(2097151 - ber)/2097151)*255);
    *signal_strength = (char)rssi;

    DBG("MTV818: informations of the channel:\n");
    DBG("        per: %d, rssi: %d, ber: %d, cnr: %d\n", per, rssi, ber, cnr);
    DBG("        %s, tvmode: %s, guard: %s, modulation: %s, coderate: %s, interlave: %s\n", rtv_isdbt_seg[tmcc.eSeg], rtv_isdbt_tvmode[tmcc.eTvMode], 
        rtv_isdbt_guard[tmcc.eGuard], rtv_isdbt_modulation[tmcc.eModulation], rtv_isdbt_coderate[tmcc.eCodeRate], rtv_isdbt_interlave[tmcc.eInterlv]);
}

//! i2c chip address
/**
 * MTV chip ID is two as 0x86 and 0xDO corresponding with TMODEx pin configuration
 * TMODE(TMOD2 = 0, TMODE1 = 0, TMODE0 = 0), i2c address: 0x86
 * TMODE(TMOD2 = 0, TMODE1 = 1, TMODE0 = 1), i2c address: 0xd0
 */
int mtv818_i2c_addr(void)
{
    return 0x86;
}

//! i2c speed
/**
 * maximum 400khz in fast-mode
 * 100 khz in standard-mode
 */
unsigned short mtv818_i2c_speed(void)
{
    return 100;
}

char mtv818_null(void)
{
    return 1;
}


/*tstv func*/
#if 1 //init demod only in ts_wakeup
struct TSTV_Module_t gTSTVModule = {
    "MTV818_CONTROL",
    mtv818_init,
    mtv818_deinit,
    mtv818_tune,
    mtv818_lockstatus,
    mtv818_pid_filter,
    mtv818_pid_filter_ctrl,
    mtv818_signal_strength_quality,
    mtv818_i2c_addr,
    mtv818_i2c_speed,
    mtv818_null 
};
#else //init demod in both ts_wakeup and ts_set_frontend
struct TSTV_Module_t gTSTVModule = {
    "MTV818_CONTROL",
    mtv818_null,
    mtv818_deinit,
    mtv818_tune,
    mtv818_lockstatus,
    mtv818_pid_filter,
    mtv818_pid_filter_ctrl,
    mtv818_signal_strength_quality,
    mtv818_i2c_addr,
    mtv818_i2c_speed,
    mtv818_init 
};
#endif

/********************************************************************************************/
//
//
//    OVER
//
//
/********************************************************************************************/

