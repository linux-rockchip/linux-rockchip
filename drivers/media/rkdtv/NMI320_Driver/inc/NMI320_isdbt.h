


/******************************************************************/
/*	 Copyright (C) 2008 ROCK-CHIPS FUZHOU . All Rights Reserved.  */
/*******************************************************************
File	:  
Desc	:  
Author	:  
Date	:  
Notes	: 
$Log    :
*********************************************************************/

#ifndef _ISDBT_NMI320_GLOBAL_H
#define _ISDBT_NMI320_GLOBAL_H
#include<linux/types.h> 

uint8_t rReg8(uint32_t adr);
void wReg8(uint32_t adr, uint8_t val);

uint32_t rReg32(uint32_t adr);

void wReg32(uint32_t adr, uint32_t val);
void nmi_delay(uint32_t msec);
void nmi_log(char *str);
uint32_t nmi_get_tick(void);

void isdbtRfburstWrite(uint32_t startadr, uint8_t *val,uint8_t cnt);
void isdbtRfWrite(uint32_t adr, uint8_t val);
uint8_t isdbtRfRead(uint32_t adr);

unsigned char isdbt_get_channel_number(long freq,unsigned char channel_select);

void freq_conversion_fp(uint32_t dFreq,uint32_t * IntegerPart,uint32_t * FractionalPart);

void LUT(uint32_t mode,uint32_t guardtype,uint32_t numsegments,uint32_t modulationA,uint32_t ratetypeA,uint32_t modulationB,uint32_t ratetypeB);

int isdbt_config_tuner(void *pv);

void isdbt_set_segment(int seg);

void isdbt_software_reset(void);

void isdbt_get_snr(void *pv);

void isdbt_get_ber(void *pv);

void isdbt_get_ber_b4viterbi(void *pv);

void isdbt_set_new_bertimer(void *pv);

void isdbt_get_per(void *pv);

void isdbt_get_channel_length(void *p);

int isdbt_check_tmc_lock(void);

void isdbt_check_fec_lock(void *pv);

void isdbt_check_soft_reset(void);

int isdbt_config_demod(void *pv) ;

void isdbt_config_decoder(void *p);

void isdbt_config_extLNA_GPIO(uint8_t lna_gain);

uint32_t isdbt_agc_gain(void);

void isdbt_track_agc(void *p);

uint32_t isdbt_get_chipid(void) ;

void isdbt_rst_ber(void);

void isdbt_rst_per(void);


void isdbt_get_time_offset(void *timeoffset);

void isdbt_get_frequency_offset(void *freqoffset);

int isdbt_agc_lock(void);

int isdbt_symbol_lock(void);

void isdbt_get_rfagcgain(void *rfgain);

void isdbt_set_rfagcgain(void *rfgain);

int isdbt_add_new_pid(void *pidconfig);

void isdbt_enable_pid_filter(uint8_t enable);

void isdbt_enable_encrypt(void *encrypt);

void isdbt_demod_enable(uint8_t enable);

uint8_t isdbt_get_subchannel(void *pv);

void isdbt_get_coarsefreqoffset(void *p);

void isdbt_RF_powerdown(void);

void isdbt_RF_powerup(void);

void isdbt_enable_dma(void *pv);

void isdbt_config_TS_output(void *ts);

void isdbt_check_DRAM(void);

int isdbt_scan_frequency(void *p);

void isdbt_master_reset(void);

void isdbt_start(void);

void isdbt_stop(void);

void isdbt_preempt_track_agc(void);

void isdbt_get_signal_rssi(void *p);

void isdbt_handle_spi_intr(void *pv);

void isdbt_handle_sdio_intr(void *pv);

void isdbt_handle_intr(void *pv);

int isdbt_chip_init(void);


int isdbt_init(void);
void nmi_dma_read(void *pv);
void nmi_bus_mode(uint8_t enable);
int nmi_common_init(void *pdrv,void *dtvvtbl);
void NMI_WriteDibcomLogClose(void);
void NMI_WriteDibcomLog(unsigned char * szRow,int iLen);



#endif

