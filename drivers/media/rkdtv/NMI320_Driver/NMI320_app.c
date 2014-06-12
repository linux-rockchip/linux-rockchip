/********************************************************************************************/
//dibcom 9080 需要的程序
//
//
//dibcom 的头文件
//
//
/********************************************************************************************/
#include "NMI320_isdbt.h"
#include "NMI320_nmidrv.h"
#include "NMI320_i2c_app.h"

#include<linux/string.h>
#include"NMI320_isdbt.h"
#include<linux/types.h>
#include <linux/module.h>

#include "../TSTV.h"

typedef int INT32U;


typedef struct {
	uint32_t freq;
	uint32_t subchannel;
	uint32_t segmentType;
	uint32_t cfgOk;
} ISDBTCFG;




NMICMN nmi;
NMICMN *pn = &nmi;
NMICHIP chip;
NMICHIP *pchp = &chip;
NMIBUSVTBL bus;
NMIBUSVTBL *pbus = &bus;
NMIDTVVTBL dtv;
NMIDTVVTBL *ptv = &dtv;
ISDBTCFG gISDBTCfg;

static int load_bus_RKPlatform(void)
{
	pbus->nmi_bus_read	= NPM_bus_read;//(NMI_BUS_READ)GetProcAddress(hLib, "nmi_bus_read");
	pbus->nmi_bus_write 	= NPM_bus_write;//(NMI_BUS_WRITE)GetProcAddress(hLib, "nmi_bus_write");
}


int npm_isdbt_init(void)
{
	printk("TSTV:NMI320: \t%s[%d]\n", __FUNCTION__, __LINE__);
	
	memset(&gISDBTCfg, 0, sizeof(gISDBTCfg));

	load_bus_RKPlatform();
	memset(pn, 0, sizeof(NMICMN));
	pn->dtvtype = ISDBT;
	pn->bustype = _I2C_;
	pn->crystalindex = _26_MHz;
	pn->boardtype = _USB_DONGLE_;//_EVAL_BOARD_;//p->boardType;//edited by hyl 开环无效，闭环有效的原因修改
	pn->dbgflag = _ERR_|_INFO_;
	
	pn->tbl.nmi_write_reg = NPM_bus_write;//nm_write_reg;	
	pn->tbl.nmi_read_reg  = NPM_bus_read;//nm_bus_read;
	pn->tbl.nmi_delay = nmi_delay;
	pn->tbl.nmi_get_tick = nmi_get_tick;
	pn->tbl.nmi_log = nmi_log;
	pn->tstype = TS_PARALLEL;
	printk("TSTV:NMI320: \t%s[%d]\n", __FUNCTION__, __LINE__);	
	if (nmi_common_init(pn,ptv) < 0)
        {
                printk("TSTV:NMI320: nmi_common_init failed \t%s[%d]\n", __FUNCTION__, __LINE__);
		return -1;
        }
        
        printk("TSTV:NMI320: nmi_common_init success \t%s[%d]\n", __FUNCTION__, __LINE__);
	return 1;
}


int npm_isdbt_scan(ISDBTCFG *p)
{
	int lock = 0;
	if ((p != NULL))
	{
		if ((ptv->nmi_scan_frequency && 
			ptv->nmi_set_segment))
		{
			ISDBTTUNE tune;
			//tune.freq = p->freq * 1000;//1000000;
                        tune.freq = p->freq; // Hz
			tune.subchannel = p->subchannel;
			ptv->nmi_set_segment(p->segmentType);
                        printk("TSTV:NMI320: frequency: %d\n\t\t\t subchannel: %d\n\t\t\t segmentType: %d\n", p->freq, p->subchannel, p->segmentType);
			lock = ptv->nmi_scan_frequency((void *)&tune);
                        printk("TSTV:NMI320: lock status %d \n", lock);
		}
	}
	if(p->freq == 767143)
	{
		ptv->nmi_check_DRAM();
	}
	if(lock)
        {
                printk("TSTV:NMI320: npm_isdbt_scan success \t%s[%d]\n", __FUNCTION__, __LINE__);
		return 1;
        }
	else 
        {
                printk("TSTV:NMI320: npm_isdbt_scan failed \t%s[%d]\n", __FUNCTION__, __LINE__);
		return -1;
        }
}

int npm_isdbt_run(ISDBTCFG *p)
{
	unsigned long delaytime;
	int result;
	int retry, lock = 0;
	uint8_t symbollock,delay,agcdelay;
	uint8_t agclock;
	uint32_t starttime;
	//uint32_t iWrval,iaddr;
	result		= 0;
	delay		= 0;
	agcdelay	= 0;
	agclock		= 0;
	symbollock	= 0;
	if ((p != NULL) ) {
		if ((ptv->nmi_config_tuner) &&
			 (ptv->nmi_config_demod) &&
			 (ptv->nmi_config_mac) &&
			 (ptv->nmi_tmc_lock) &&
			 (ptv->nmi_soft_reset)) {
			ISDBTTUNE tune;
			//tune.freq = p->freq * 1000;
                        tune.freq = p->freq;
                        //printk("TSTV:NMI320: ---------------------------------------->the frequency we tune %d<-----------------------------------------\n", p->freq);
			tune.subchannel = p->subchannel;
			ptv->nmi_set_segment(p->segmentType);
			starttime = nmi_get_tick();
			//iWrval = 0xff;
			//iaddr = 0x6430;
			//npm_isdbt_write_register((unsigned char *)&iWrval,4,iaddr);
			//iWrval = 0x0;
			//iaddr = 0x6434;
			//npm_isdbt_write_register((unsigned char *)&iWrval,4,iaddr);
			result = ptv->nmi_config_tuner((void *)&tune);
			if(result == 0)
				goto isdbt_run_exit;
			//iWrval = 0x3fff;
			//iaddr = 0x6430;
			//npm_isdbt_write_register((unsigned char *)&iWrval,4,iaddr);
			//iWrval = 0xffffffff;
			//iaddr = 0x6434;
			//npm_isdbt_write_register((unsigned char *)&iWrval,4,iaddr);
			result = ptv->nmi_config_demod((void *)&tune);
			if(result == 0)
				goto isdbt_run_exit;
			retry = 2;
			nmi_delay(50);
			do {
				agclock = ptv->nmi_agc_lock();
				if(agclock) {
					agcdelay = 0;
					do {
						symbollock = ptv->nmi_symbol_lock();
						nmi_delay(20);
						delay += 5;
						if(delay == 100) {
							delay = 0;
							break;
						}
					}while(!symbollock);
				}
				else {
					agcdelay += 5;
					if(agcdelay < 60) {
						nmi_delay(20);
						continue;
					}
					else {
						agcdelay = 0;
						lock = 0;
						break;
					}
				}
				if(symbollock) {
					//nmi_delay(100);
					for(delaytime = 0;delaytime <= 600; delaytime += 15) {
						if (ptv->nmi_tmc_lock()) {
							lock = 1;
							break;
						}
						nmi_delay(25);
					}
					if(lock)
						break;
					nmi_delay(300);
					if (ptv->nmi_tmc_lock()) {
						lock = 1;
						break;
					}
					ptv->nmi_soft_reset();
				}
			} while (--retry);

			if (lock) {
				/* run decoder */
				starttime = nmi_get_tick();
				ptv->nmi_config_mac(NULL);
				p->cfgOk = 1;
			} else {
				p->cfgOk = 0;
			}
		}
	}
	result = 1;
isdbt_run_exit:
	return result;
}

void npm_isdbt_get_status(void)//(ISDBTSTATUS *p)
{
	unsigned int		starttime;
#if 0	
	if ((p != NULL) ) {
		if (ptv->nmi_fec_lock &&
			ptv->nmi_get_snr &&
			ptv->nmi_get_ber &&
			ptv->nmi_get_per &&
			ptv->nmi_track_agc) {
			ISDBTSNR snr;
			ISDBTBER ber;
			//ISDBTBER_B4VITERBI ber_b4viterbi;
			ISDBTPER per;
			ISDBTAGC agc;
			ISDBTFEC fec;
			ISDBTCHLEN chn_len;
#if defined _HAVE_FLOAT_
			double  freq_offset;
			double  time_offset;
#else
			int32_t freq_offset;
			int32_t time_offset;
#endif
			starttime = nmi_get_tick();
			ptv->nmi_get_snr(&snr);
			p->dSnr_a			= snr.snr_a;
			p->dSnr_b			= snr.snr_b;
			ptv->nmi_get_ber(&ber);
			p->dBer_a			= ber.ber_a;
			p->dBer_b			= ber.ber_b;
	
			/*
			// viswa on Dec17 2007 to test the BER before viterbi
			ptv->nmi_get_ber_b4viterbi(&ber_b4viterbi);
			p->dBer_a			= ber_b4viterbi.ber_a;
			p->dBer_b			= ber_b4viterbi.ber_b;
			*/

			ptv->nmi_get_per(&per);
			p->dPer_a			= per.per_a;
			p->dPer_b			= per.per_b;
			ptv->nmi_fec_lock(&fec);
			p->bFecLock_a = fec.fec_lock_a;
			p->bFecLock_b = fec.fec_lock_b;

			ptv->nmi_track_agc(&agc);
			p->extlna_mode		= agc.ExtLNA;
			p->rssi				= agc.RSSI;
			p->rfagc_current	= agc.RfAgcCurrent;
			p->vga_gain			= agc.IfVGAGain;

			ptv->nmi_get_channel_length(&chn_len);
			p->channel_length	= chn_len.channelLength;
			ptv->nmi_get_freq_offset(&freq_offset);
			p->frequency_offset	= (double)freq_offset;
			ptv->nmi_get_time_offset(&time_offset);
			p->time_offset = (double)time_offset;
			ptv->nmi_check_soft_reset();	
			//dbg(_INFO_,"GetStatus[%d]\n",(int)(GetTickCount() - starttime));
		}
	}
#endif	
}



void npm_isdbt_rst_cnt(void)
{
	if (ptv->nmi_rst_ber &&
		ptv->nmi_rst_per) 
		{
			ptv->nmi_rst_ber();
			ptv->nmi_rst_per();
		}
}


char NMI320_init()
{
	if(npm_isdbt_init() > 0)
		return 1;
	else
		//return -1;
        return 0;	
}

void NMI320_tune(unsigned int frequency_KHz, unsigned int  bandwidth_KHz)
{
	ISDBTTUNE SISBTTuner;
    struct timespec cur_time_1, cur_time_2;  
    long time_used = 0;

    getnstimeofday(&cur_time_1);
	
	gISDBTCfg.freq = frequency_KHz*1000;
	gISDBTCfg.segmentType =1;
	gISDBTCfg.cfgOk =0;
	SISBTTuner.freq = frequency;
	SISBTTuner.subchannel =0;
	isdbt_get_subchannel(&SISBTTuner);
	gISDBTCfg.subchannel =SISBTTuner.subchannel;
	if(npm_isdbt_scan(&gISDBTCfg) >= 0)
	{
		npm_isdbt_run(&gISDBTCfg);
	}

    getnstimeofday(&cur_time_2);
	time_used = 1000*(cur_time_2.tv_sec - cur_time_1.tv_sec) + ((cur_time_2.tv_nsec - cur_time_1.tv_nsec)/1000)/1000; 
    printk("TSTV:NMI320: used time length: %ld ms!!!! \t%s[%d]\n", time_used, __FUNCTION__, __LINE__);	
}


void NMI320_Deinit()
{
	NMI_WriteDibcomLogClose();
}


char NMI320_lockstatus()
{
	return gISDBTCfg.cfgOk;
}

void NMI320_signal_strenth_quality(char *signal_quality, char *signal_strength)
{
	ISDBTSNR spv;
	uint8_t rfgain;

    return 100;
	
	ptv->nmi_get_snr(&spv);
    printk("NMI320_signal_strenth_quality spv.snr_a: == %d !!!!\n", spv.snr_a);		
	*signal_strength =(unsigned char )(spv.snr_a/1000);
	*signal_strength = (*signal_strength);
	*signal_quality = *signal_strength;	
#if 0	
	ptv->nmi_get_rfagcgain(&rfgain);
	if (rfgain < 0x2f)
		ptv->nmi_config_extLNA_GPIO(_UHF_LOW_GAIN_);
	else
		ptv->nmi_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
#endif
}

int  NMI320_demod_pid_filter_ctrl(uint8_t onoff)
{
//	ptv->nmi_enable_pid_filter(onoff);
}

int NMI320_demod_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
	
}

int NMI320_i2c_addr(void)
{
    return 0xc0;
}

unsigned short NMI320_i2c_speed(void)
{
    return 150;
}

char NMI320_NULL(void)
{
    return 1;
}


/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "NMI320_CONTROL",
    NMI320_NULL,
    NMI320_Deinit,
    NMI320_tune,
    NMI320_lockstatus,
    NMI320_demod_pid_filter,
    NMI320_demod_pid_filter_ctrl,
    NMI320_signal_strenth_quality,
    NMI320_i2c_addr,
    NMI320_i2c_speed,
    NMI320_init
};




