/********************************************************************************************/
//New port Media  nmi310 需要的程序
//
//
//nmi310 的头文件
//
//
/********************************************************************************************/

#include "NMI310_i2c_app.h"
#include"nmicmn.h"
#include"nmidvb.h"
#include<linux/string.h>
#include"nmi310_dvb.h"
#include<linux/types.h>
#include <linux/module.h>

#include "../TSTV.h"

#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
typedef int INT32U;


typedef struct {
	uint32_t frequency;
	uint32_t bandwidth;
	uint32_t cfgOk;
} DVBTCFG;

NMICMN nmi;
NMICMN *pn = &nmi;
NMIDTVVTBL dtv;
NMIDTVVTBL *ptv = &dtv;
NMIBUSVTBL bus;
NMIBUSVTBL *pbus = &bus;
DVBTCFG gDVBTCfg;

typedef struct
{
      INT32U   bUsed;
      INT32U iPid;
}DVBPIDFilterInfo;

DVBPIDFilterInfo g_iPidFilter[16];



#ifdef NMI_Bool_Write_log

MY_FILE *g_FileHwd =NULL;
char g_szlog[100*1024];
int g_szlogLen =0;

#endif


void NMI_WriteDibcomLog(unsigned char * szRow,int iLen)
{	

	
}


void NMI_WriteDibcomLogClose(void)
{
   		
}

void nmi_log(char *str)
{
	uint32_t ilen;
	ilen =strlen(str);
	NMI_WriteDibcomLog((unsigned char *)str,ilen);
}


int npm_dvb_init()
{
	//unsigned char base = 0xc0;
    memset(&gDVBTCfg, 0, sizeof(gDVBTCfg));

	/* common init */
	memset(pn, 0, sizeof(NMICMN));
	pn->dtvtype = DVB;
	pn->bustype = _I2C_;
	pn->dbgflag = _ERR_|_INFO_;
	pn->crystalindex = 5;
	pn->tbl.nmi_write_reg = NPM_bus_write;
	pn->tbl.nmi_read_reg = NPM_bus_read;
	pn->tbl.nmi_delay = nmi_delay;
	pn->tbl.nmi_get_tick = nmi_get_tick;
	pn->tbl.nmi_log = nmi_log;
	
	ptv = nmi_common_init(pn);
	if (ptv == NULL)
		return FALSE;
	return TRUE;
}



void npm_dvb_setup_tuner(DVBTCFG *p)
{
	DVBTUNE tune;
	tune.frequency = p->frequency;
	tune.bandwidth = p->bandwidth;
	if (ptv->nmi_config_tuner)
		ptv->nmi_config_tuner((void *)&tune);
}

void npm_dvb_setup_demod(DVBRUN* p)
{
	DVBRUN run;
	if (ptv->nmi_config_demod)
	{
		ptv->nmi_config_demod((void *)&run);
		p->lock = run.lock;
	}
}

void npm_dvbh_setup_link()//(DVBLNKCTL *p)
{
	DVBLNKCTL lnk;
	int i;
#if 0
	memset((void *)&lnk, 0, sizeof(DVBLNKCTL));
		
	if (ptv->nmi_config_mac && ptv->nmi_dvb_rst_lnk_cnt) {
		ptv->nmi_dvb_rst_lnk_cnt();
		if (p->mode) {	/* dvb-t */
			lnk.mode = 1;
			lnk.tsmode = 1;
			ptv->nmi_config_mac((void *)&lnk);
		} 	
		else {	/* dvb-h */
			lnk.mode = 0;
			lnk.tsmode = 0;
			lnk.npid = p->numPID;
			for (i = 0; i < p->numPID; i++) {
				lnk.pid[i] = p->pid[i];
				lnk.fec[i] = p->uFec[i];
				lnk.type[i] = 1;
			}		
			ptv->nmi_config_mac((void *)&lnk);
		}		
	}
#else
	memset((void *)&lnk, 0, sizeof(DVBLNKCTL));
	if (ptv->nmi_config_mac && ptv->nmi_dvb_rst_lnk_cnt) 
	{
		ptv->nmi_dvb_rst_lnk_cnt();
		/* dvb-t */
			lnk.mode = 1;
			lnk.tsmode = 1;
			lnk.tsvalpol = 0;
			ptv->nmi_config_mac((void *)&lnk);
	}
#endif
}


void npm_dvb_get_demod_status(DVBSTA *p)
{
	if (ptv->nmi_get_dvb_sta) 
	{
		DVBSTA sts;
		memset((void *)&sts, 0, sizeof(DVBSTA));
		ptv->nmi_get_dvb_sta((void *)&sts);
		p->bAGCLock 		= sts.bAGCLock;
		p->bSyrLock 			= sts.bSyrLock;
		p->bTpsLock 			= sts.bTpsLock;
		p->bChcTrkLock 	= sts.bChcTrkLock;
		p->bFecLock 			= sts.bFecLock;
#ifdef _HAVE_FLOAT_		
		p->dSnr 					= sts.dSnr;
		p->dBer 					= sts.dBer;
		p->dPer 					= sts.dPer;
		p->dFreqOffset 		= sts.dFreqOffset;
		p->dTimeOffset 		= sts.dTimeOffset;
#endif		
		p->agc 					= sts.agc;
		p->uRFAGC 			= (uint8_t)sts.uRFAGC;
		p->rssi					= (int)sts.rssi;
		p->lnaOn				= sts.lnaOn;
		p->alg						= sts.alg;
		p->chcdelay			= sts.chcdelay;

		if (!p->bFecLock) 
		{
			ptv->nmi_soft_reset();
			//Sleep(300);
		}

	} 
}

char npm_fec_lock()
{
	char bLock;
	if (ptv->nmi_fec_lock) 
	{
		ptv->nmi_fec_lock((void*)&bLock);
		if(bLock==1)
			return 1;
		else
			return 0;
	}
}

int npm_dvbt_scan(DVBTCFG *p)
{
	bool bLock =FALSE;
	DVBRUN core;
	npm_dvb_setup_tuner(p);
	npm_dvb_setup_demod(&core);
	if(core.lock)
		bLock = TRUE;
	return bLock;
}

int npm_dvbt_run(DVBTCFG *p)
{
	unsigned long delaytime;
	int result;
	int retry, lock = 0;
	uint8_t symbollock,delay,agcdelay;
	uint8_t agclock;
	uint32_t starttime;
//	DVBSTA sDVBState;

	npm_dvbh_setup_link();
	retry =20;	
	do {
//			npm_dvb_get_demod_status(&sDVBState);
			if(npm_fec_lock() ==1)
			{
				lock = 1;
				break;
			}
			nmi_delay(100);
		}while (--retry);

	if (lock) {
		p->cfgOk = 1;
	} else {
		p->cfgOk = 0;
	}
}


char NMI310_init()
{
	if(npm_dvb_init() ==TRUE)
		return 1;
	else
		return 0;
}



void NMI310_tune(unsigned int frequency_KHz, unsigned int  bandwidth_KHz)
{
	gDVBTCfg.frequency =frequency_KHz*1000;
	gDVBTCfg.bandwidth =bandwidth_KHz/1000;

	printk(KERN_INFO"freq =%d,band =%d",gDVBTCfg.frequency,gDVBTCfg.bandwidth);
	if(npm_dvbt_scan(&gDVBTCfg)==TRUE)
		npm_dvbt_run(&gDVBTCfg);
}


void NMI310_turn_with_no_delay(unsigned int frequency,unsigned int bandwidth)
{
	NMI310_tune(frequency, bandwidth);
}


void NMI310_refresh_signal_status(unsigned int frequency,char bandwidth)
{ 
	DVBSTA sDVBState;

    npm_dvb_get_demod_status(&sDVBState);
}



void NMI310_Deinit()
{
	NMI_WriteDibcomLogClose();
}



char NMI310_lockstatus()
{
	return gDVBTCfg.cfgOk;
}



void NMI310_signal_strenth_quality(unsigned char *signal_quality, unsigned char *signal_strength)
{
	DVBSNR spv;
	
	ptv->nmi_get_snr(&spv);
	*signal_strength =(unsigned char )(spv.snr/1000);	/*  0  --->  25*/
    *signal_strength = (*signal_strength*11);
	*signal_quality = *signal_strength;
}



int NMI310_demod_pid_filter_ctrl(unsigned char onoff)
{
	static uint32_t adr, val;
	uint32_t i;
	if(onoff ==FALSE)
	{
#if 0
		val = rReg32(0xf2c0);
		val &=~(1<<30); 
		wReg32(0xf2c0,val);		
		dvb_t_filter_reset();
		{
			for (i = 0; i < 8; i++) 
			{
				adr = 0xf204 + i * 4;
				val = rReg32(adr);
			}
		}
#else
		val = rReg32(0xf2c0);
		val &=~(1<<30); 
		wReg32(0xf2c0,val);	
		for(i=0;i<16;i++)
		{
			if(g_iPidFilter[i].bUsed ==1)
			{
				dvb_t_filter_remove_pid(g_iPidFilter[i].iPid);
				g_iPidFilter[i].bUsed =0;
				g_iPidFilter[i].iPid =0x1fff;
			}
		}
#endif
	}
	else
	{
		val = rReg32(0xf2c0);
		val |=(1<<30); 
		wReg32(0xf2c0,val);
	}
	return TRUE;
}


int NMI310_demod_pid_filter(unsigned char id, unsigned short pid, unsigned char onoff)
{
	uint32_t i;
	if(onoff)
	{
		dvb_t_filter_add_pid(pid);
		for(i=0;i<16;i++)
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
		dvb_t_filter_remove_pid(pid);
		for(i=0;i<16;i++)
		{
			if((g_iPidFilter[i].bUsed ==1)&&(g_iPidFilter[i].iPid ==pid))
			{
				g_iPidFilter[i].bUsed =0;
				g_iPidFilter[i].iPid =0x1fff;
				break;
			}
		}
	}
	return TRUE;
}

int NMI310_i2c_addr(void)
{
    return 0xc0;
}

unsigned short NMI310_i2c_speed(void)
{
    return 150;
}

char NMI310_NULL(void)
{
    return 1;
}


/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "NMI310_CONTROL",
    NMI310_NULL,
    NMI310_Deinit,
    NMI310_tune,
    NMI310_lockstatus,
    NMI310_demod_pid_filter,
    NMI310_demod_pid_filter_ctrl,
    NMI310_signal_strenth_quality,
    NMI310_i2c_addr,
    NMI310_i2c_speed,
    NMI310_init
};

