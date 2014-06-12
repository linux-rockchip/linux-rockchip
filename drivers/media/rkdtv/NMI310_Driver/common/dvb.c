////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Newport Media Inc.  All rights reserved.
//
// Module Name:  dvb.c
//
// Author : K.Yu
//
// Date : 6th June. 2007
//
//////////////////////////////////////////////////////////////////////////////

#include <nmicmn.h>
#include <linux/unistd.h>

#include <linux/delay.h>
#include <linux/time.h>
#include <linux/string.h>
#include "nmi310_dvb.h"

//extern UINT32 SysOsTime;

static void dvb_soft_rst(void);


typedef struct {
	uint32_t 	chipid;
	uint32_t	bandwidth;
	int			fdvbt;				/* flag for DVB-T */

#ifdef _HAVE_FLOAT_
	double		frequency;
	double 	aber;
	int			nber;
	double 	aper;
	int			nper;
#else
	long			frequency;
#endif

	uint32_t 	maxVga;
	uint32_t 	minVga;
	int 			algCnt;

	int 			nframe;
	int 			nfecNeed;
	int 			nframeErr;

	uint32_t 	prefifoptr;

	uint32_t 	csinoise;
	int			constellation;
	int			corerate;

	uint32_t 	mode;
	uint32_t	guard;
	
} DVBPRIV;

#ifdef _HAVE_FLOAT_
typedef struct {
	double 	dCrystalFreq;
	uint32_t	regb414; 
} NMIDVBCRYSTALTBL;

static NMIDVBCRYSTALTBL dvbcrystaltbl[] = {
{12, 0x10540c30/*0x10540848*/},
{13, 0x116c2c2c/*0x116C1542*/},
{19.2, 0x10540f1e/*0x1054022C*/},
{19.68, 0x1d74721c/*0x1D765A2C*/},
{19.8, 0x1ad49a1c/*0x1AD6422C*/},
{26, 0x216C2c2c/*0x216C1542*/},
{27, 0x22f43d2a/*0x22F45C40*/}
};
#else
typedef struct {
	uint32_t	regb414; 
} NMIDVBCRYSTALTBL;

static NMIDVBCRYSTALTBL dvbcrystaltbl[] = {
{0x10540c30/*0x10540848*/},
{0x116c2c2c/*0x116C1542*/},
{0x10540f1e/*0x1054022C*/},
{0x1d74721c/*0x1D765A2C*/},
{0x1ad49a1c/*0x1AD6422C*/},
{0x216C2c2c/*0x216C1542*/},
{0x22f43d2a/*0x22F45C40*/}
};
#endif

typedef struct {
	NMITV				dtvtype;
	NMIBUSTYPE 	bustype;
	uint32_t			dbg;
	int					crystalindex;
	NMIHLPVTBL 	hlp;
	NMIDTVVTBL 	dtv;
	int					chipver;
} NMICHIP;


NMICHIP chip;
NMICHIP *pchp = &chip;

DVBPRIV dvbpriv;
DVBPRIV *pdvb = &dvbpriv; 

#define ISNM201 ((((pdvb->chipid) & 0xfff00) == 0x20100) ? 1 : 0)
#define ISNM311 ((((pdvb->chipid) & 0xfff00) == 0x31100) ? 1 : 0)
#define ISNM312 ((((pdvb->chipid) & 0xfff00) == 0x31200) ? 1 : 0)
#define ISNM312A0 ((((pdvb->chipid) & 0xfffff) == 0x312a0) ? 1 : 0)
#define ISNM312C2 ((((pdvb->chipid) & 0xfffff) == 0x312c2) ? 1 : 0)
#define ISNM312D0 ((((pdvb->chipid) & 0xfffff) == 0x312d0) ? 1 : 0)

#ifdef _HAVE_FLOAT_
#ifdef _USE_SNR_TBL_
static uint32_t log10_snr[] = {
 	60000, 50000, 43979,       40458,       37959,       36021,       34437,       33098,
  31938,       30915,       30000,       29172,       28416,       27721,       27077,       26478,
  25918,       25391,       24895,       24425,       23979,       23556,       23152,       22765,
  22396,       22041,       21701,       21373,       21057,       20752,       20458,       20173,
  19897,       19630,       19370,       19119,       18874,       18636,       18404,       18179,
  17959,       17744,       17535,       17331,       17131,       16936,       16745,       16558,
  16375,       16196,       16021,       15849,       15680,       15514,       15352,       15193,
  15036,       14883,       14731,       14583,       14437,       14293,       14152,       14013,
  13876,       13742,       13609,       13479,       13350,       13223,       13098,       12975,
  12853,       12734,       12615,       12499,       12384,       12270,       12158,       12047,
  11938,       11830,       11724,       11618,       11514,       11412,       11310,       11210,
  11110,       11012,       10915,       10819,       10724,       10630,       10537,       10446,
  10355,       10265,       10175,       10087,       10000,        9914,        9828,        9743,
  9659,        9576,        9494,        9412,        9332,        9251,        9172,        9094,
  9016,        8938,        8862,        8786,        8711,        8636,        8562,        8489,
  8416,        8344,        8273,        8202,        8132,        8062,        7993,        7924,
  7856,        7788,        7721,        7655,        7589,        7523,        7458,        7393,
  7329,        7266,        7202,        7140,        7077,        7016,        6954,        6893,
  6833,        6773,        6713,        6654,        6595,        6536,        6478,        6420,
  6363,        6306,        6250,        6193,        6138,        6082,        6027,        5972,
  5918,        5863,        5810,        5756,        5703,        5650,        5598,        5546,
  5494,        5442,        5391,        5340,        5289,        5239,        5189,        5139,
  5090,        5041,        4992,        4943,        4895,        4846,        4799,        4751,
  4704,        4657,        4610,        4563,        4517,        4471,        4425,        4379,
  4334,        4289,        4244,        4199,        4155,        4111,       4067,        4023,
  3979,        3936,        3893,        3850,        3807,        3765,        3723,        3681,
  3639,        3597,        3556,        3514,        3473,        3432,        3392,        3351,
  3311,        3271,        3231,        3191,        3152,        3112,        3073,        3034,
  2995,        2956,        2918,        2879,        2841,        2803,        2765,        2728,
  2690,        2653,        2616,        2579,        2542,        2505,        2468,        2432,
  2396,        2360,        2324,        2288,        2252,        2217,        2181,        2146,
  2111,        2076,        2041,        2007,        1972,        1938,        1903,        1869};
#endif
#else
static uint32_t log10_snr[] = {
 	60000, 50000, 43979,       40458,       37959,       36021,       34437,       33098,
  31938,       30915,       30000,       29172,       28416,       27721,       27077,       26478,
  25918,       25391,       24895,       24425,       23979,       23556,       23152,       22765,
  22396,       22041,       21701,       21373,       21057,       20752,       20458,       20173,
  19897,       19630,       19370,       19119,       18874,       18636,       18404,       18179,
  17959,       17744,       17535,       17331,       17131,       16936,       16745,       16558,
  16375,       16196,       16021,       15849,       15680,       15514,       15352,       15193,
  15036,       14883,       14731,       14583,       14437,       14293,       14152,       14013,
  13876,       13742,       13609,       13479,       13350,       13223,       13098,       12975,
  12853,       12734,       12615,       12499,       12384,       12270,       12158,       12047,
  11938,       11830,       11724,       11618,       11514,       11412,       11310,       11210,
  11110,       11012,       10915,       10819,       10724,       10630,       10537,       10446,
  10355,       10265,       10175,       10087,       10000,        9914,        9828,        9743,
  9659,        9576,        9494,        9412,        9332,        9251,        9172,        9094,
  9016,        8938,        8862,        8786,        8711,        8636,        8562,        8489,
  8416,        8344,        8273,        8202,        8132,        8062,        7993,        7924,
  7856,        7788,        7721,        7655,        7589,        7523,        7458,        7393,
  7329,        7266,        7202,        7140,        7077,        7016,        6954,        6893,
  6833,        6773,        6713,        6654,        6595,        6536,        6478,        6420,
  6363,        6306,        6250,        6193,        6138,        6082,        6027,        5972,
  5918,        5863,        5810,        5756,        5703,        5650,        5598,        5546,
  5494,        5442,        5391,        5340,        5289,        5239,        5189,        5139,
  5090,        5041,        4992,        4943,        4895,        4846,        4799,        4751,
  4704,        4657,        4610,        4563,        4517,        4471,        4425,        4379,
  4334,        4289,        4244,        4199,        4155,        4111,       4067,        4023,
  3979,        3936,        3893,        3850,        3807,        3765,        3723,        3681,
  3639,        3597,        3556,        3514,        3473,        3432,        3392,        3351,
  3311,        3271,        3231,        3191,        3152,        3112,        3073,        3034,
  2995,        2956,        2918,        2879,        2841,        2803,        2765,        2728,
  2690,        2653,        2616,        2579,        2542,        2505,        2468,        2432,
  2396,        2360,        2324,        2288,        2252,        2217,        2181,        2146,
  2111,        2076,        2041,        2007,        1972,        1938,        1903,        1869};
#endif

#ifdef NORMALZIF
static const uint32_t LUT_BB[] ={ 
140, 147, 154, 161, 168, 175, 182, 189,
196, 203, 210, 217, 224, 231, 238, 245,
252, 259, 266, 273, 280, 287, 294, 301,
308, 315, 322, 329, 336, 343, 350, 357,
364, 371, 378, 385, 392, 399, 406, 413,
420, 427, 434, 441, 448, 455, 462, 469,
476, 483, 490, 497, 504, 511, 518, 525,
532, 539, 546, 553, 560, 567, 574, 581,
588, 595, 602, 609, 616, 623, 630, 637,
644, 651, 658, 665, 672, 679, 686, 693,
700, 707, 714, 721, 728, 735, 742, 749,
756, 763, 770, 777, 784, 791, 798, 805,
812, 819, 826, 833, 840, 847, 853, 853,
853, 853, 853, 853, 853, 853, 853, 853,
853, 853, 853, 853, 853, 853, 853, 853,
853, 853, 853, 853, 853, 853, 853, 853,
}; 
#endif



/********************************************
	Debug Functions
********************************************/

static void nmi_debug(uint32_t flag, char *fmt,...)
{
	char buf[256];
	va_list args;
	int len;

	return ;

	if (flag & pchp->dbg) { 
		va_start(args, fmt);
#ifdef _WINXP_KERNEL_
		RtlStringCbVPrintfA(buf, 256, fmt, args);
#else
		len = vsprintf(buf, fmt, args);
#endif
		va_end(args);

		printk(KERN_ERR,buf);

#if 0
		if (pchp->hlp.nmi_log) {
			pchp->hlp.nmi_log(buf);
		}
#endif
	}

	return;
}

/********************************************
	Helper Functions
********************************************/
void nmi_delay(uint32_t msec)
{
#if 0
	if (pchp->hlp.nmi_delay) {
		pchp->hlp.nmi_delay(msec);
	}
#else
	msleep(msec);
#endif
}

EXPORT_SYMBOL(nmi_delay);
unsigned int nmi_get_tick(void)
{
#if 0
	uint32_t tick = 0;

	if (pchp->hlp.nmi_get_tick) {
		tick = pchp->hlp.nmi_get_tick();
	}

	return tick;
#else
    uint32_t     ticks = 0;
    struct timespec cur_time;  
    long time_ms = 0;

    do_gettimeofday(&cur_time);
	time_ms = cur_time.tv_sec*1000 + ((cur_time.tv_nsec / 1000)/1000);
	ticks = (uint32_t)time_ms;	
	return ticks ;
#endif
}

static void nmi_dma_read(void *pv)
{
	if (pchp->hlp.nmi_dma_read) {
		pchp->hlp.nmi_dma_read(pv);
	}
}

static void nmi_tbl_read(void *pv)
{
	if (pchp->hlp.nmi_dvb_read_tbl) {
		pchp->hlp.nmi_dvb_read_tbl(pv);
	}
}

/********************************************
	Bus Read/Write Functions
********************************************/
static uint8_t rReg8(uint32_t adr)
{
	uint8_t val;

	if (pchp->hlp.nmi_read_reg != NULL) {
		pchp->hlp.nmi_read_reg(adr, (uint8_t *)&val, 1);
		return val;
	}

	return 0; 
}

static void wReg8(uint32_t adr, uint8_t val)
{
	if (pchp->hlp.nmi_write_reg != NULL) {
		pchp->hlp.nmi_write_reg(adr, (uint8_t *)&val, 1);
	}

	return; 
}

uint32_t rReg32(uint32_t adr)
{
	uint32_t val;

	if (pchp->hlp.nmi_read_reg != NULL) {
		pchp->hlp.nmi_read_reg(adr, (uint8_t *)&val, 4);
		return val;
	}

	return 0; 
}

EXPORT_SYMBOL(rReg32);
void wReg32(uint32_t adr, uint32_t val)
{
	if (pchp->hlp.nmi_write_reg != NULL) {
		pchp->hlp.nmi_write_reg(adr, (uint8_t *)&val, 4);
	}
	return; 
}
EXPORT_SYMBOL(wReg32);
/********************************************
	DVB RF Functions
********************************************/
static void dvbRfWrite(uint32_t adr, uint8_t val)
{
	int retry;
	uint32_t reg;
	
	wReg32(0xb500, 2);
	wReg32(0xb504, adr);
	wReg32(0xb504, val);
	wReg32(0xb508, 0x1);

	retry = 100;
	do {
		reg = rReg32(0xb508);
		if ((reg & 0x1) == 0)
			break;
		nmi_delay(1);
	} while (retry--);
	
	reg = rReg32(0xb50c);
	if (!(reg & 0x1)) {
		nmi_debug(_ERR_, "RF write reg: Fail, addr (%02x), val (%02x)\n", adr, val);
	}

	return;
}

static uint8_t dvbRfRead(uint32_t adr)
{
	int retry;
	uint32_t reg;
	
	/* write register address */
	wReg32(0xb310, adr);

	retry = 100;
	/* wait for STOP_DET_INTR */
	do {
		reg = rReg32(0xb32c);
		nmi_delay(1);
	} while (!(reg & 0x200) && retry--);

	if (retry <= 0) {
		nmi_debug(_ERR_, "RF Read reg: Fail, wait STOP DET intr, (%08x)\r\n", reg);
		return 0;
	}

	/* clear interrupt */
	rReg32(0xb340);

	/* write 0x100 */
	wReg32(0xb310, 0x100);

	retry = 100;
	/* wait for STOP_DET_INTR */
	do {
		reg = rReg32(0xb32c);		
		nmi_delay(1);
	} while (!(reg & 0x200) && retry--);

	if (retry <= 0) {
		nmi_debug(_ERR_, "RF Read reg: Fail, wait STOP DET intr, (%08x)\r\n", reg);
		return 0;
	}

	/* read data */
	reg = rReg32(0xb310);

	/* clear interrupt */
	rReg32(0xb340);		

	return (uint8_t)reg;
}

static void dvb_tuner_init(void)
{
	dvbRfWrite(0x6, 0xe7);
	dvbRfWrite(0xb, 0xbd);	/* ykk */
	dvbRfWrite(0xf, 0xd);	
	dvbRfWrite(0x14, 0xff);
	dvbRfWrite(0x17, 0xff);
	dvbRfWrite(0x18, 0xcb);
	dvbRfWrite(0x1c, 0x00);	/* ykk */
	dvbRfWrite(0x10, 0x08);

	if (ISNM312A0) {
		if (pchp->chipver == 1)
			dvbRfWrite(0x08, 0x80);		
	} else {
		dvbRfWrite(0x08, 0xa0);		/* RFLI steady with temperature changes */	
	}

	dvbRfWrite(0xd, 0x48);
	dvbRfWrite(0xe, 0x48);

	dvbRfWrite(0x19, 0x83);

	if (ISNM312) 
		dvbRfWrite(0xc, 0x79); 

	return;
}

#ifdef _HAVE_FLOAT_
static void dvb_set_band(int band)
{
	uint8_t val, val1, val2;
	int ix = pchp->crystalindex;
	double refclk = dvbcrystaltbl[ix].dCrystalFreq;

   switch (band) {
	case 5:
      val = 0x00;
      val1= 0x50;
      val2 = 0xfc/*0xbc*/;
      break;
	case 6:
      val= 0x00;
      val1= 0x60;
      val2 = 0xfd/*0xbd*/;
      break;	
  	case 7:
      val= 0x00;
      val1= 0x70;
      val2 = 0xfe/*0xbe*/;
      break;	
  	case 8:
      val = 0x00;
      val1= 0x80;
      val2 = 0xff/*0xbf*/;
      break;
  	default:
      val= 0x00;
      val1= 0x80;
      val2 = 0xff/*0xbf*/;
      break;
	}

	/* for different crystal */
	if (refclk <= 13) {
		val2 &= ~(0x7 << 2);
	} else if ((refclk > 13) && (refclk <= 19.2)) {
		val2 &= ~(0x7 << 2);
		val2 |= (0x1 << 2);
	} else if ((refclk > 19.2) && (refclk <= 19.68)) {
		val2 &= ~(0x7 << 2);
		val2 |= (0x2 << 2);
	} else if ((refclk > 19.68) && (refclk <= 19.8)) {
		val2 &= ~(0x7 << 2);
		val2 |= (0x3 << 2);	
	} 
	dvbRfWrite(0x05, val2);
   wReg8(0xf01b, val);		/* TRL_NOMINALRATE_1 */
   wReg8(0xf01c, val1);		/* TRL_NOMINALRATE_2 */

	if (ISNM311) {
		if (band == 8) {
			dvbRfWrite(0x1b, 0x0);
			dvbRfWrite(0x1a, 0xe);
		} else if ((band == 5) || (band == 6)) {
			dvbRfWrite(0x1b, 0x3);
			dvbRfWrite(0x1a, 0xfe);
		} else if (band == 7) {
			dvbRfWrite(0x1b, 0x3);
			dvbRfWrite(0x1a, 0x99);
		}
	} else if (ISNM312) {
		dvbRfWrite(0x1b, 0x0);
	}
}
#else
static void dvb_set_band(int band)
{
	uint8_t val, val1, val2;

   switch (band) {
	case 5:
      val = 0x00;
      val1= 0x50;
      val2 = 0xfc/*0xbc*/;
      break;
	case 6:
      val= 0x00;
      val1= 0x60;
      val2 = 0xfd/*0xbd*/;
      break;	
  	case 7:
      val= 0x00;
      val1= 0x70;
      val2 = 0xfe/*0xbe*/;
      break;	
  	case 8:
      val = 0x00;
      val1= 0x80;
      val2 = 0xff/*0xbf*/;
      break;
  	default:
      val= 0x00;
      val1= 0x80;
      val2 = 0xff/*0xbf*/;
      break;
	}
	dvbRfWrite(0x05, val2);
   wReg8(0xf01b, val);		/* TRL_NOMINALRATE_1 */
   wReg8(0xf01c, val1);		/* TRL_NOMINALRATE_2 */

	if (ISNM311) {
		if (band == 8) {
			dvbRfWrite(0x1b, 0x0);
			dvbRfWrite(0x1a, 0xe);
		} else if ((band == 5) || (band == 6)) {
			dvbRfWrite(0x1b, 0x3);
			dvbRfWrite(0x1a, 0xfe);
		} else if (band == 7) {
			dvbRfWrite(0x1b, 0x3);
			dvbRfWrite(0x1a, 0x99);
		}
	} else if (ISNM312) {
		dvbRfWrite(0x1b, 0x0);
	}
}

#endif

#ifdef _HAVE_FLOAT_
static void dvb_set_freq(double dfreq)
{
	double 	target;
	long 		n0, alpha;
	uint8_t 	val;
	int ix = pchp->crystalindex;
	double refclk = dvbcrystaltbl[ix].dCrystalFreq;
#ifdef _EXT_LNA_
	uint32_t val32;
#endif

	if (refclk < 19) {
		refclk *= 2;
	}

#ifdef _EXT_LNA_
	val32 = rReg32(0xb000);
	if (dfreq <= 300) {
		if ((((val32 >> 12) & 0x1) != 0) ||
			(((val32 >> 13) & 0x1) != 1)) {
			/* switch to VHF */
			val32 &= ~(1 << 12);
			val32 |= (1 << 13);
			wReg32(0xb000, val32);
		}  
	} else {
		if ((((val32 >> 12) & 0x1) != 1) ||
			(((val32 >> 13) & 0x1) != 0)) {
			/* switch to UHF */
			val32 |= (1 << 12);
			val32 &= ~(1 << 13);
			wReg32(0xb000, val32);
		}  
	}
#endif

	if (ISNM312) {
		val = rReg8(0xf100);
		if (dfreq <= 300/*250*/) {
			val &= ~0x4;
		} else {
			val |= 0x4;
		}
		wReg8(0xf100, val);

		/* temperature for 480MHz */
		if (dfreq < 500)
			dvbRfWrite(0x15, 0x88);		
		else
			dvbRfWrite(0x15, 0x8);		
	}

	if (dfreq <= 219) {
		if (ISNM312) {
   			target = dfreq * 12/refclk/*26.0*/;
		} else if (ISNM311) {
   			target = dfreq * 16/refclk/*26.0*/;
	      	dvbRfWrite(0x05, 0x9f);
		}
		dvbRfWrite(0x00, 0x5f);
	} else if (dfreq <= 300/*250*/) {
		if (ISNM312) {
   			target = dfreq * 12/refclk/*26.0*/;
		} else if (ISNM311) {
   			target = dfreq * 8/refclk/*26.0*/;
		}
		dvbRfWrite(0x00, 0x5f);
	} else if (dfreq <= 900) {
		target = dfreq * 4/refclk/*26.0*/;
   		dvbRfWrite(0x00, 0x9f);
   } else if ((dfreq >= 1400) && (dfreq <= 1700)){
   		target = dfreq *2/refclk/*26.0*/;
      	dvbRfWrite(0x00, 0xdf);
	} else {
		target = dfreq * 4/refclk/*26.0*/;
   		dvbRfWrite(0x00, 0x9f);
	}

   	n0 = (long)(target); 
   	val = (uint8_t)n0;
	
	dvbRfWrite(0x01, val);
   	alpha = (long)((target - n0)*(0x1000000));
   	val = (uint8_t)(alpha & 0xff);
   	dvbRfWrite(0x02, val);
   	val = (uint8_t)((alpha >> 8) & 0xff);
   	dvbRfWrite(0x03, val);
   	val = (uint8_t)((alpha >> 16) & 0xff);
   	dvbRfWrite(0x04, val);

	
#ifdef _HW_TIME_SLICE_
	if (dfreq <= 300) {	/* VHF */
		wReg32(0xb510, 0x5f);		
	} else {
		wReg32(0xb510, 0x9f);
	}
	
	wReg32(0xb514, 0x05);
	wReg32(0xb51c, 0xff);

	nmi_debug(_INFO_, "[DVB]: HW Time Slice, (%08x)(%08x)(%08x)\n",
		rReg32(0xb510), rReg32(0xb514), rReg32(0xb51c));
#endif

	return;
}
#else
static void dvb_set_freq(long dfreq)
{
	uint64_t 	target;
	long 		n0, alpha;
	uint8_t 	val;
#ifdef _EXT_LNA_
	uint32_t val32;
#endif

	/* Comment:
			Pass in frequency in khz unit
	*/

	nmi_debug(_INFO_, "[DVB]: freq (%d)\n", dfreq);

#ifdef _EXT_LNA_
	val32 = rReg32(0xb000);
	if (dfreq/1000 <= 300) {
		if ((((val32 >> 12) & 0x1) != 0) ||
			(((val32 >> 13) & 0x1) != 1)) {
			/* switch to VHF */
			val32 &= ~(1 << 12);
			val32 |= (1 << 13);
			wReg32(0xb000, val32);
		}  
	} else {
		if ((((val32 >> 12) & 0x1) != 1) ||
			(((val32 >> 13) & 0x1) != 0)) {
			/* switch to UHF */
			val32 |= (1 << 12);
			val32 &= ~(1 << 13);
			wReg32(0xb000, val32);
		}  
	}
#endif

	if (ISNM312) {
		val = rReg8(0xf100);
		if (dfreq/1000 <= 300/*250*/) {
			val &= ~0x4;
		} else {
			val |= 0x4;
		}
		wReg8(0xf100, val);

		/* temperature for 480MHz */
		if (dfreq/1000 < 500)
			dvbRfWrite(0x15, 0x88);		
		else
			dvbRfWrite(0x15, 0x8);		

	}

	if (dfreq/1000 <= 219) {
		if (ISNM312) {
   			target = (dfreq * 129911527712)		/* 12/26 */;
		} else if (ISNM311) {
   			target = (dfreq * 173215370283)		/* 16/26 */;
	      	dvbRfWrite(0x05, 0x9f);
		}
		target >>= 24;
		dvbRfWrite(0x00, 0x5f);
	} else if (dfreq/1000 <= 300/*250*/) {
		if (ISNM312) {
   			target = (dfreq * 129911527712)		/* 12/26 */;
		} else if (ISNM311) {
   			target = (dfreq * 86607685142)			/* 8/26 */;
		}
		target >>= 24;
		dvbRfWrite(0x00, 0x5f);
	} else if (dfreq/1000 <= 900) {
		target = (dfreq * 43303842571); 			/* 4/26 */;
		target >>= 24;
   		dvbRfWrite(0x00, 0x9f);
   } else if ((dfreq/1000 >= 1400) && (dfreq/1000 <= 1700)){
   		target = (dfreq * 21651921285);			/* 2/26 */;
		target >>= 24;
      	dvbRfWrite(0x00, 0xdf);
	} else {
		target = (dfreq * 43303842571); 			/* 4/26 */;
		target >>= 24;
   		dvbRfWrite(0x00, 0x9f);
	}

   	n0 = (long)(target)  & 0xff000000; 
   	val = (uint8_t)(n0 >> 24);
	nmi_debug(_VERB_, "val1 (%02x)\n", val);
	dvbRfWrite(0x01, val);
   	alpha = (long)(target) & 0x00ffffff;
   	val = (uint8_t)(alpha & 0xff);
	nmi_debug(_VERB_, "val2 (%02x)\n", val);
   	dvbRfWrite(0x02, val);
   	val = (uint8_t)((alpha >> 8) & 0xff);
	nmi_debug(_VERB_, "val3 (%02x)\n", val);
   	dvbRfWrite(0x03, val);
   	val = (uint8_t)((alpha >> 16) & 0xff);
	nmi_debug(_VERB_, "val4 (%02x)\n", val);
   	dvbRfWrite(0x04, val);

#ifdef _HW_TIME_SLICE_
	if (dfreq/1000 <= 300) {	/* VHF */
		wReg32(0xb510, 0x5f);		
	} else {
		wReg32(0xb510, 0x9f);
	}
	
	wReg32(0xb514, 0x05);
	wReg32(0xb51c, 0xff);

	nmi_debug(_INFO_, "[DVB]: HW Time Slice, (%08x)(%08x)(%08x)\n",
		rReg32(0xb510), rReg32(0xb514), rReg32(0xb51c));
#endif

	return;
}
#endif

static int  dvb_run_tuner(void *pv)
{
	DVBTUNE *p = (DVBTUNE *)pv;

#ifdef _HAVE_FLOAT_
	nmi_debug(_INFO_, "[DVB]: tuner, frequency (%f), bandwidth (%d)\n", p->frequency, p->bandwidth);
#else
	nmi_debug(_INFO_, "[DVB]: tuner, frequency (%d), bandwidth (%d)\n", p->frequency, p->bandwidth);
#endif
	printk(KERN_INFO"dvb_run_tuner [DVB]: tuner, frequency (%d), bandwidth (%d)\n", p->frequency, p->bandwidth);

	dvb_set_band(p->bandwidth);
#ifdef _HAVE_FLOAT_
	dvb_set_freq(p->frequency/1000);
#else
	dvb_set_freq((p->frequency/1000));
#endif

	pdvb->bandwidth = p->bandwidth;
	pdvb->frequency = p->frequency;

	return 1;
}


/********************************************
	DVB Demod Functions
********************************************/
static void dvb_soft_rst(void)
{
#ifdef _HW_TIME_SLICE_
	wReg8(0xf000, 0x00);	/* COR_CTL */
	wReg8(0xf000, 0x60);
#else
	wReg8(0xf000, 0x00);	/* COR_CTL */
	wReg8(0xf000, 0x20);
#endif

	return;
}

static uint8_t dvb_get_lock_sts(void)
{
	uint8_t ulock = 0;
	ulock = rReg8(0xf1c1);		
   	return ulock;
}

static uint8_t dvb_get_cor_sts(void)
{
	uint8_t status;

	status = rReg8(0xf001);		/* COR_STAT */
	return status;
}

static uint8_t dvb_get_syr_guard(void)
{
	uint8_t guard;

	guard = rReg8(0xf02a);	/* TPS_RCVD_4 */
	
	return ((guard >> 4) & 0x3);
}

static uint8_t dvb_get_syr_mode(void)
{
	uint8_t mode;

	mode = rReg8(0xf02a);	/* TPS_RCVD_4 */

	return (mode & 0x3);   
}

static uint8_t dvb_get_corerate(void)
{
	uint8_t coderate;

	coderate = rReg8(0xf029);

	return (coderate & 0x07);   
}

static uint8_t dvb_get_constellation(void)
{
	uint8_t constellation;

	constellation = rReg8(0xf028);

	return (constellation & 0x3);   
}

static uint8_t dvb_get_hier(void)
{
	uint8_t alpha;

	alpha = rReg8(0xf028);

	return ((alpha >> 4)  & 0x3);   
}

static uint8_t csinoisetbl[] ={255, 169, 134, 107, 89, 0x90, 0x70, 0x40, 23, 21, 0x38, 0x18, 0x1a, 0xa, 9}; /* table since 1108 version */
//static uint8_t csinoisetbl[] ={255, 169, 134, 107, 89, 0x90, 0x70, 0x60, 23, 21, 0x38, 0x24, 0x1a, 6, 5}; /* table since 1107 version */
//static uint8_t csinoisetbl[] ={ 255, 169, 134, 107, 89, 69,41, 29,23, 21, 19,12, 8,6,5}; 	/* table since start */

static void dvb_csi_est(void)
{
	uint8_t constel, corerate, ix, csinoise;

	constel = dvb_get_constellation();
	corerate = dvb_get_corerate();
	ix = (constel * 5) + corerate;

	if (ix < sizeof(csinoisetbl)) {
		csinoise = csinoisetbl[ix];
		if (pdvb->csinoise != csinoise) {
			pdvb->csinoise = csinoise;
			wReg8(0xf049, csinoise);	/* CHC_CSI_NOISE_1 */
		}
	} else {
		nmi_debug(_ERR_, "[DVB]: Error: csi estimate, out of bound, (%d)(%d)\n", constel, corerate);
	}

	return;
}

static void dvb_set_per_window(void)
{
	uint8_t constel;
	uint32_t misc;

	constel= dvb_get_constellation();
	/* modify PER windows, based on modulation */
	misc = rReg32(0xf2c0);
	misc &= ~(0x7 << 13);
	if (constel == 0) {		/* QPSK */
		misc |= (0x2 << 13);
	} else if (constel == 1) {	/* 16 QAM */
		misc |= (0x3 << 13);
	} else {	/* 64 QAM */
		misc |= (0x4 << 13);
	}
	wReg32(0xf2c0, misc);

	return;
}

static void dvb_set_dst(void )
{
	/* set the delay spread threshold */
	uint8_t constel, corerate, val8;

	constel 	= dvb_get_constellation();
	corerate 	= dvb_get_corerate();
	val8 = rReg8(0xf096);
	nmi_debug(_INFO_, "[DVB]: delay threshold, modulation (%d), corerate (%d), (%02x)\n", constel, corerate, val8);
	if (constel == 2) {	/* 64 QAM */
		val8 &= ~0x1f;
		switch (corerate) {
		case 0:				/* 1/2 */
			val8 |= 4;
			break;
		case 1:				/* 2/3 */
			val8 |= 3;
			break;
		case 2:				/* 3/4 */
		case 3:				/* 5/6 */
		case 4:				/* 7/8 */
			val8 |= 2;
			break;
		}
	}
	if (constel == 1) {	/* 16 QAM */
		val8 &= ~0x1f;
		switch (corerate) {
		case 0:				/* 1/2 */
			val8 |= 6;
			break;
		case 1:				/* 2/3 */
			val8 |= 5;
			break;
		case 2:				/* 3/4 */
		case 3:				/* 5/6 */
		case 4:				/* 7/8 */
			val8 |= 4;
			break;
		}
	}
	wReg8(0xf096, val8);
	return;	
}

#ifdef _HAVE_FLOAT_
static void dvb_get_snr(void *pv)
{
	DVBSNR *p = (DVBSNR *)pv;
	uint8_t usnr, alg;
	double dsnr;

	alg = rReg8(0xf048);	/* CHC_CTL_2 */
	if (alg == 1) {
		usnr = rReg8(0xf024);
		dsnr = (double)usnr / 8;
	} else {
		usnr = rReg8(0xf07a);
#ifndef _USE_SNR_TBL_
		dsnr = log10(usnr) * (-20.) + (50.);
#else
		dsnr = (double)log10_snr(usnr)/1000;
#endif
	}

	p->snr = dsnr;

	return;
}
#else
static void dvb_get_snr(void *pv)
{
	DVBSNR *p = (DVBSNR *)pv;
	uint8_t usnr, alg;
	long dsnr;

	alg = rReg8(0xf048);	/* CHC_CTL_2 */
	if (alg == 1) {
		usnr = rReg8(0xf024);
		dsnr = usnr / 8;
	} else {
		usnr = rReg8(0xf07a);
		dsnr = log10_snr[usnr];
	}

	p->snr = dsnr;
	return;
}
#endif

static void dvb_get_ber(void *pv) 
{
#ifdef _HAVE_FLOAT_
	DVBBER *p = (DVBBER *)pv;
	uint32_t 	uber;
	uint32_t 	umax;
	double 	dber;
	double		aber = pdvb->aber;
	int 			cnt = pdvb->nber;

	uber = (rReg8(0xf1cc) << 16);			/* REVIEW */	
	uber |= (rReg8(0xf1cb) << 8);
	uber |= rReg8(0xf1ca);
	//nmi_debug(_INFO_, "BER (%d)\n", uber);

	umax = rReg8(0xf1c9);	/* REVIEW */
	umax &= 0x7;
	umax = 1 << (3 + 10 + umax);		/* 8 * 1024 * 2^ber_max_bytes */
	//nmi_debug(_INFO_, "BER MAX (%d)\n", umax);

	dber = (double)uber/umax;
	aber = (aber * cnt + dber)/(cnt + 1);

	pdvb->aber = aber;
	pdvb->nber++;

	p->ber = aber;
	p->beri = dber;
	return;
#endif
}

static uint32_t dvb_get_ber_count(void) 
{
	uint32_t 	uber;

	uber = (rReg8(0xf1cc) << 16);				/* REVIEW */	
	uber |= (rReg8(0xf1cb) << 8);
	uber |= rReg8(0xf1ca);
	return uber;
}

static uint32_t dvb_get_ber_period(void)
{
	uint32_t 	umax;

	umax = rReg8(0xf1c9);	/* REVIEW */
	umax &= 0x7;
	umax = 1 << (3 + 10 + umax);			/* 8 * 1024 * 2^ber_max_bytes */
	return umax;	
} 

static uint32_t dvb_get_per_count(void) 
{
	uint32_t 	uPer;

	uPer = rReg32(0xf2d8);
	nmi_debug(_INFO_, "[PER]: .1 (%08x)\n", uPer); 
	uPer &= 0x7fff;
	nmi_debug(_INFO_, "[PER]: .2 (%d)\n", (int)uPer); 
	
	return uPer;
}

static uint32_t dvb_get_per_period(void)
{
	uint32_t 	umax;

	umax = rReg32(0xf2c0);
	umax = (umax >> 13) & 0x7;
	umax = (1 << (umax + 8));

	return umax;	
} 


static void dvb_get_per(void *pv)
{
#ifdef _HAVE_FLOAT_
	DVBPER *p = (DVBPER *)pv;
	uint32_t misc, tspk, upd;
	double ts_err_pkt, denom, dper;

	misc = rReg32(0xf2c0);
	tspk = rReg32(0xf2d8);
	nmi_debug(_VERB_, "[DVB] tspk (%08x)\n", tspk);
	
	upd = (misc >> 13) & 0x7;
	denom = (double)(1 << (upd + 8));
	ts_err_pkt = (double)(tspk & 0x7fff);
	dper = ts_err_pkt/denom;
	pdvb->aper = (pdvb->aper * pdvb->nper + (ts_err_pkt/denom))/(pdvb->nper + 1);
	pdvb->nper++;

	p->per = pdvb->aper;
	p->peri = dper;
	return;
#endif
}

static uint32_t dvb_get_agc_gain(void)
{
	uint32_t  agcGain;
	uint32_t val;
	uint8_t val8;

#ifndef NORMALZIF
	/* freez */
	wReg8(0xf034, 1);

	/* AGC gain */
	val = rReg8(0xf009);
	agcGain = val & 0xff;
	val = rReg8(0xf00a);
	val = ((val + 2) & 0x3) << 8;
	agcGain |= val;

	/* unfreez */
	wReg8(0xf034, 0);
#else
	/* freez */
	val8 = rReg8(0xf142);
	val8 |= 0x1;
	wReg8(0xf142, val8);

	/* AGC gain */
	val = rReg8(0xf140);
	agcGain = val & 0xff;
	val = rReg8(0xf141);
#if 0
	val = ((val + 2) & 0x3) << 8;
#else
	val = (val & 0x3) << 8;
#endif
	agcGain |= val;

	/* unfreez */
	val8 = rReg8(0xf142);
	val8 &= ~0x1;
	wReg8(0xf142, val8);

#endif

   	return agcGain;
}

#ifdef _HAVE_FLOAT_
static double dvb_get_freq_offset(int bandwidth)
{
	uint8_t val;
   	uint32_t uFreqOffset = 0;
   	int32_t iFreqOffset;
   	double dFreqOffset;
	val = 1;
    
	wReg8(0xf034, 1);	/* PIR_CTL */

	uFreqOffset = (rReg8(0xf022) << 16);	/* CRL_FREQ_3 */
	uFreqOffset |= (rReg8(0xf021) << 8);	/* CRL_FREQ_2 */
	uFreqOffset |= rReg8(0xf020);				/* CRL_FREQ_1 */
	
  	iFreqOffset  = ((uFreqOffset & (1<<23)) == (1<<23)) ? -1 : 0;
  	iFreqOffset &= ~((1<<24)-1);
  	iFreqOffset |= (uFreqOffset & ((1<<24)-1));
	
  	dFreqOffset  = (double)iFreqOffset * 64000000 / (double)(1<<25);
  	dFreqOffset /= 7.0;

	val = rReg8(0xf02a);
   	switch ((val & 0x3)) {     // Scale according DVB-T transmission mode
	case 2:		/* 4K */
		dFreqOffset /= 2.0; 
		break;
	case 1:		/* 8K */
		dFreqOffset /= 4.0; 
		break;
	default:
        break;
  	}
  	dFreqOffset *= (bandwidth / 8.0);

	wReg8(0xf034, 0);	/* PIR_CTL */

   	return (-dFreqOffset);
}
#else
static int dvb_get_freq_offset(int bandwidth)
{
	uint8_t val;
   	uint32_t uFreqOffset = 0;
   	int32_t iFreqOffset;
   	int dFreqOffset;

	val = 1;
	wReg8(0xf034, 1);	/* PIR_CTL */

	uFreqOffset = (rReg8(0xf022) << 16);		/* CRL_FREQ_3 */
	uFreqOffset |= (rReg8(0xf021) << 8);		/* CRL_FREQ_2 */
	uFreqOffset |= rReg8(0xf020);					/* CRL_FREQ_1 */
	
  	iFreqOffset  = ((uFreqOffset & (1<<23)) == (1<<23)) ? -1 : 0;
  	iFreqOffset &= ~((1<<24)-1);
  	iFreqOffset |= (uFreqOffset & ((1<<24)-1));
	
  	dFreqOffset  = (iFreqOffset * 190)/100;  //64000000 / (double)(1<<25)
  	dFreqOffset /= 7;

	val = rReg8(0xf02a);
   	switch ((val & 0x3)) {     // Scale according DVB-T transmission mode
	case 2:		/* 4K */
		dFreqOffset /= 2; 
		break;
	case 1:		/* 8K */
		dFreqOffset /= 4; 
		break;
	default:
        break;
  	}
  	dFreqOffset *= (bandwidth/8);

	wReg8(0xf034, 0);	/* PIR_CTL */

   	return (-dFreqOffset);
}
#endif

#ifdef _HAVE_FLOAT_
static double dvb_get_time_offset(int bandwidth)
{
  	uint32_t uTimeOffset;
  	double dNominalRate;
  	double dTimeOffset;

	wReg8(0xf034, 1);	/* PIR_CTL */

	uTimeOffset = (rReg8(0xf01e) << 8);		/* TRL_TIME_2 */
	uTimeOffset |= rReg8(0xf01d);					/* TRL_TIME_1 */

  	dNominalRate = (64.0/7.0) * bandwidth * 65536.0 / (8.0 * 20.48);

  	dTimeOffset  = (int16_t) (uTimeOffset) / 268.4;
  	dTimeOffset /= (dNominalRate / 65536.0);

	wReg8(0xf034, 0);	/* PIR_CTL */

  	return dTimeOffset;
}
#endif

static void dvb_rst_ber(void)
{
#ifdef _HAVE_FLOAT_
	pdvb->aber = 0;
	pdvb->nber = 0;
#endif
	return;
}

static void dvb_rst_per(void)
{
#ifdef _HAVE_FLOAT_
	pdvb->aper = 0;
	pdvb->nper = 0;
#endif
	return;
}

static void dvb_get_mode(void *pv)
{
	DVBMODE *pMode= (DVBMODE *)pv;
	pMode->Guard 	= dvb_get_syr_guard();
	pMode->Mode		= dvb_get_syr_mode();
	pMode->Rate		= dvb_get_corerate();
	pMode->Conste	= dvb_get_constellation();
	pMode->Alpha	= dvb_get_hier();
}

static void dvb_fec_lock(void *pv)
{
	int *p = (int *)pv;
	uint8_t lock;

	lock = dvb_get_lock_sts();
	if ((lock & 0x3) == 0x3)
		*p = 1;
	else 
		*p = 0;

	return;
}

/* review: need to tune for each board. don't just ust it! */
static int dvb_calc_rssi(void)
{
#ifdef _EXT_LNA_
	uint32_t gio;
#endif
	uint32_t agc;
	uint8_t rfagc;
	int rssi, rssia;
	int offset;

	agc = dvb_get_agc_gain();
	rfagc = rReg8(0xf145);

	if (rfagc == 0xFF) {
		offset = 0; //45;
	}else if((rfagc & 0x80) == 0x80 ) {
		if (rfagc >= 0xBF) {
			offset = 46;
		} else if (rfagc >= 0x9F) {
			offset = 121;
		} else if (rfagc >= 0x8F) {
			offset = 176;
		} else if (rfagc >= 0x87) {
			offset = 222;
		} else if (rfagc >= 0x83) {
			offset = 277;
		}else{
			offset = 306;
		}
	}else{
		if (rfagc >= 0x7f) {
			offset = 130;
		} else if (rfagc >= 0x3F){
			offset = 170/*130*/;
		} else if (rfagc >= 0x1F){
			offset = 241 /*211*/;
		} else if (rfagc >= 0x0F){
			offset = 301 /*271*/;
		} else if (rfagc >= 0x07){
			offset = 367 /*327*/;
		} else if (rfagc >= 0x03){
			offset = 413 /*373*/;
		}else{
			offset = 413 /*406*/;
		}
	}	

	rssi = ((offset - (int)agc) / 10);

#ifdef _EXT_LNA_	
	gio = rReg32(0xb000);
	if ((gio >> 15) & 0x1) {		/* LNA Off */
		rssi -= 14;
	} else {								/* LNA On */
		rssi -= 29;								
	}
#endif

	if(rssi >= -12) {
		rssi = (rssi / 2) - 6;
	}

	rssia = (int)rReg8(0xf147);

	nmi_debug(_INFO_, "RSSI (%d)(%d), AGC (%d), RF AGC (%02x)\n", rssia, rssi, agc, rfagc);
	return rssi;
}

static int dvb_get_sq(void)
{
#ifdef _HAVE_FLOAT_
	uint32_t 	uber;
	uint32_t 	umax;
	double 		dber;
	int			sq;
	uint8_t  	core, lock;

	core	= dvb_get_cor_sts();
	lock 	= dvb_get_lock_sts();

	if (core != 0xf6) {
		sq = 0;
	} else {
		if ((lock & 0x3) != 0x3) {
			sq = 1;
		} else {
			uber = (rReg8(0xf1cc) << 16);				
			uber |= (rReg8(0xf1cb) << 8);
			uber |= rReg8(0xf1ca);
			umax = rReg8(0xf1c9);	
			umax &= 0x7;
			umax = 1 << (3 + 10 + umax);		/* 8 * 1024 * 2^ber_max_bytes */
			dber = (double)uber/umax;

			if (dber >= 0.0002) {
				sq = 2;
			} else if ((dber < 0.0002) && (dber >= 0.00019)) {
				sq = 3;
			} else if ((dber < 0.00019) && (dber > 0.0)) {
				sq = 4; 
			} else {
				sq = 5;
			}
		}
	}

	return sq;
#else
	uint32_t 	uber;
	uint32_t 	umax;
	int			sq;
	uint8_t  	core, lock;

	core	= dvb_get_cor_sts();
	lock 	= dvb_get_lock_sts();

	if (core != 0xf6) {
		sq = 0;
	} else {
		if ((lock & 0x3) != 0x3) {
			sq = 1;
		} else {
			uber = (rReg8(0xf1cc) << 16);				
			uber |= (rReg8(0xf1cb) << 8);
			uber |= rReg8(0xf1ca);
			umax = rReg8(0xf1c9);	
			umax &= 0x7;
			umax = 1 << (3 + 10 + umax);		/* 8 * 1024 * 2^ber_max_bytes */

			if ((uber*10000) >= (umax * 2)) {
				sq = 2;
			} else if (((uber * 10000) < (umax * 2)) && ((uber * 100000) >= (umax * 19))) {
				sq = 3;
			} else if (((uber * 100000) < (umax * 19)) && ((uber > 0))) {
				sq = 4; 
			} else {
				sq = 5;
			}
		}
	}

	return sq;

#endif

}

static void dvb_get_sta(void *pv)
{
#ifdef _HAVE_FLOAT_
	DVBSTA *pSts = (DVBSTA *)pv;
	uint8_t core; 
	uint8_t lock;
	uint32_t val=0;
	uint32_t agcGain=0;
	DVBSNR snr;
	DVBBER ber;
	DVBPER per;
	
	core	= dvb_get_cor_sts();
	lock 	= dvb_get_lock_sts();
	nmi_debug(_VERB_, "core (%02x), lock (%02x)\n", core, lock);
		
	if ((core >> 4) & 0x1)
		pSts->bAGCLock = 1;
	else
		pSts->bAGCLock = 0;

	if ((core >> 5) & 0x1)
		pSts->bSyrLock = 1;
	else
		pSts->bSyrLock = 0;

	if ((core >> 7) & 0x1)
		pSts->bChcTrkLock = 1;
	else
		pSts->bChcTrkLock = 0;

	if ((core >> 6) & 0x1) {
		int constel, corerate;
		constel = (int)dvb_get_constellation();
		corerate = (int)dvb_get_corerate();
		if ((pdvb->constellation != constel) ||
			 (pdvb->corerate != corerate)) {
			dvb_csi_est();
			dvb_set_dst();
			dvb_set_per_window();			
			pdvb->constellation = constel;
			pdvb->corerate = corerate;
		}
		pSts->bTpsLock = 1;
	} else
		pSts->bTpsLock = 0;

	if ((lock & 0x03) == 0x3) 
		pSts->bFecLock = 1;
	else
		pSts->bFecLock = 0; 

	dvb_get_snr((void *)&snr);
	pSts->dSnr = snr.snr;
	dvb_get_ber((void *)&ber);
	pSts->dBer = ber.ber;
	pSts->dBeri = ber.beri;
	dvb_get_per((void *)&per);
	pSts->dPer = per.per;
	pSts->dPeri = per.peri;
	pSts->dFreqOffset	= 	dvb_get_freq_offset(pdvb->bandwidth);
	pSts->dTimeOffset =	dvb_get_time_offset(pdvb->bandwidth);

	agcGain = dvb_get_agc_gain();
	pSts->agc = agcGain/*(double)((agcGain/1023.0)*2.5)*/;

	val = dvbRfRead(0x0c);
	nmi_debug(_VERB_, "[DVB]: open loop (%02x)\n", val);

	if ((val >> 3) & 0x1) {
		pSts->uRFAGC = rReg8(0xf145);
	} else {
		pSts->uRFAGC = dvbRfRead(0x07);
	}

	pSts->rssi = (int)rReg8(0xf147);
	pSts->alg = rReg8(0xf048);
	pSts->chcdelay = rReg8(0xf09a);
	val = rReg8(0xf097);
	pSts->freqfilter = ((val >> 4) & 0x7);
	pSts->vgalower = (int)(((rReg8(0xf157) << 8) & 0x3) | rReg8(0xf156));
	pSts->vgaupper = (int)(((rReg8(0xf155) << 8) & 0x3) | rReg8(0xf154));
	pSts->rssilower = (int)rReg8(0xf159);
	pSts->rssiupper = (int)rReg8(0xf158);

	/* review: this is to track LNA. may need to adjust it for each board.
			don't just use it */
#ifdef _EXT_LNA_
	{
		uint32_t val32;
		uint8_t val8;
		uint8_t rssi1;	
		val32 = rReg32(0xb000);
		nmi_debug(_INFO_, "*** GPIO (%08x) ***\n", val32);

		if (!((val32 >> 15) & 0x1)) {		/* LNA is On */
			val8 = rReg8(0xf145);
			rssi1 = rReg8(0xf147);
			agcGain = dvb_get_agc_gain(); /* this is for debug */

			nmi_debug(_INFO_, "*** 1. rfagc (%02x), rssi (%d), agc (%d) ***\n", val8, (int)rssi1, agcGain);

			val8 &= ~0x80;

			if ((val8 <= 0xf)) {
				val32 |= (1 << 15);
				wReg32(0xb000, val32);
				nmi_debug(_INFO_, "*** LNA OFF (%02x) ***\n", val8);
			}
		} else {											/* LNA is Off */
			val8 = rReg8(0xf145);
			rssi1 = rReg8(0xf147);
			agcGain = dvb_get_agc_gain();	/* this is for debug */

			nmi_debug(_INFO_, "*** 2. rfagc (%02x), rssi (%d), agc (%d) ***\n", val8, (int)rssi1, agcGain);

			val8 &= ~0x80;
			if ((val8 >= 0x7f) && (rssi1 < 4)) {
				val32 &= ~(1 << 15);
				wReg32(0xb000, val32);
				nmi_debug(_INFO_, "*** LNA ON (%02x) ***\n", val8);
			}
		}

		val32 = rReg32(0xb000);
		if ((val32 >> 15) & 0x1)
			pSts->lnaOn = 0;
		else
			pSts->lnaOn = 1;
  
	}
#endif

	nmi_debug(_VERB_, "status: (%d), (%d), (%d), (%d), (%d), snr (%f), AGC (%d) RFAGC (%02x), FOft (%f), RSSI(%d)\n",
		pSts->bAGCLock, pSts->bSyrLock, pSts->bChcTrkLock, pSts->bTpsLock, pSts->bFecLock,
		pSts->dSnr, pSts->agc, pSts->uRFAGC, pSts->dFreqOffset, pSts->rssi);
		
	return;
#endif

}

static void dvb_track_alg(void)
{
	uint32_t maxVga, minVga, agcVar, agcGain;
	uint8_t curAlg;

	agcGain = dvb_get_agc_gain();

	maxVga = pdvb->maxVga;
	minVga = pdvb->minVga;

	//nmi_debug(_INFO_, "[%d], agcGain(%d), max (%d), min (%d)\n", (int)nmi_get_tick(), agcGain, maxVga, minVga);
		 
 	if(pdvb->algCnt == 50) {

  		pdvb->algCnt =0;
  		agcVar = maxVga - minVga;
  		pdvb->maxVga =0;
  		pdvb->minVga = 1024;
		
		curAlg = rReg8(0xf048);

		nmi_debug(_INFO_, "[%d], max (%d), min (%d), agcVar(%d), (%d)\n", (int)nmi_get_tick(), maxVga, minVga, agcVar, curAlg);

	  	if((agcVar > 60/* 12 */) && (curAlg == 1)) {
			wReg8(0xf048, 3);
			nmi_debug(_INFO_, "*** switch to 3 ***\n");
			dvb_soft_rst(); 

		} else if ((agcVar< 55/*5*/ ) && (curAlg == 3)) {
			wReg8(0xf048, 1);
			nmi_debug(_INFO_, "*** switch to 1 ***\n");

			dvb_soft_rst(); 
	  	} // agc
	} else {
		/* ignore the first value */
		if (pdvb->algCnt != 0) {
			pdvb->maxVga = (maxVga > agcGain) ? maxVga : agcGain;
			pdvb->minVga = (minVga < agcGain) ? minVga : agcGain;
		}
		pdvb->algCnt ++;
 
	}

	return;
} 

static uint32_t dvb_lock_time(int bandwidth, uint32_t time)
{
	uint32_t locktime;

	switch (bandwidth) {
	case 5:	/* TODO */
		locktime = (time * 8)/5 + 1;
		break;
	case 6:
		locktime = (time * 8)/6 + 1;
		break;
	case 7:
		locktime = (time * 8)/7 + 1;
		break;
	case 8:
		locktime = time;
		break;
	}
	
	return locktime;
}

static void dvb_band_scan(void *pv)
{
	DVBBANDSCAN *p = (DVBBANDSCAN *)pv;
	uint8_t val8;
	uint32_t maxtime, time = 0;

	dvb_set_band(p->bandwidth);
#ifdef _HAVE_FLOAT_
	dvb_set_freq(p->frequency);
#else
	dvb_set_freq((p->frequency/1000));
#endif

	maxtime = dvb_lock_time(p->bandwidth, 250);
	dvb_soft_rst();
	p->tpslock = 0;
	do {
		nmi_delay(50);
		/* look for TPS lock */
		val8 = rReg8(0xf001);
		if ((val8 >> 6) & 0x1) {
			p->tpslock = 1;
			break;
		}
		time += 50;
	} while (time < maxtime);

	return;
}

/* review: this is obsolete. should use the internal tracking */
static void dvb_track_agc(void)
{
	uint8_t 	val, rfGain, rfGain1, l_lna, u_lna, UppthrdCnt, LowthrdCnt;
	uint32_t 	rssi = 0, agcGain, i, agcUpperThrd, agcLowerThrd;
	int 			pma_on = 0;

	UppthrdCnt = 0;
	LowthrdCnt = 0;
	l_lna = 2;
	u_lna = 50;
	agcUpperThrd = VGA_UPPER_THRD;
	agcLowerThrd = VGA_LOWER_THRD; 

	rfGain1 = rfGain = dvbRfRead(0x7);
	agcGain = dvb_get_agc_gain();

	for (i = 0; i < 10; i++) {
		if (agcGain > agcUpperThrd)
			UppthrdCnt++;
		if (agcGain < agcLowerThrd )
			LowthrdCnt++;
		agcGain = dvb_get_agc_gain();
		nmi_delay(7);
	}

	dvb_track_alg();

	for(i=0; i<(1 << _RSSI_SFT_); i++){
		val = rReg8(0xf147);
		rssi += val;
	}
	rssi >>= _RSSI_SFT_;

	if (LowthrdCnt > 5) {
		pma_on = 0;
		rfGain = rfGain & 0x7f;
	}

	if (UppthrdCnt > 5) {
		pma_on = 1;
		rfGain = 0x80 | rfGain;
	}

	if (pma_on == 0) {
		if (rssi > u_lna) {
			switch (rfGain){
			case 0xff:
				rfGain1 = 0x7f;
				break;
			case 0x7f:
				rfGain1 = 0x3f;
				break;
			case 0x3f:
				rfGain1 = 0x1f;
				break;
			case 0x1f:
				rfGain1 = 0xf;
				break;
			case 0xf:
				rfGain1 = 0x7;
				break;
			case 0x7:
				rfGain1 = 0x3;
				break;
			case 0x3:
				rfGain1 = 0x1;
				break;
			}
		}	else if(rssi < l_lna) {
			switch (rfGain){
			case 0x3f:
				rfGain1 = 0x7f;
				break;
			case 0x1f:
				rfGain1 = 0x3f;
				break;
			case 0xf:
				rfGain1 = 0x1f;
				break;
			case 0x7:
				rfGain1 = 0xf;
				break;
			case 0x3:
				rfGain1 = 0x7;
				break;
			case 0x1:
				rfGain1 = 0x3;
				break;
			}
		}
	}

	if (pma_on == 1) {
		if(rssi > u_lna) {
			switch (rfGain){	
			case 0xff:
				rfGain1 = 0xbf;
				break;
			case 0xbf:
				rfGain1 = 0x9f;
				break;
			case 0x9f:
				rfGain1 = 0x8f;
				break;
			case 0x8f:
				rfGain1 = 0x87;
				break;
			case 0x87:
				rfGain1 = 0x83;
				break;
			case 0x83:
				rfGain1 = 0x81;
				break;
			}
		}
		else if (rssi < l_lna) {
			switch (rfGain){
			case 0xbf:
				rfGain1 = 0xff;
				break;
			case 0x9f:
				rfGain1 = 0xbf;
				break;
			case 0x8f:
				rfGain1 = 0x9f;
				break;
			case 0x87:
				rfGain1 = 0x8f;
				break;
			case 0x83:
				rfGain1 = 0x87;
				break;
			case 0x81:
				rfGain1 = 0x83;
				break;
			}
		}
	}

	if (rfGain1 != rfGain) {
		dvbRfWrite(0x7, rfGain1);
	}
}

static void dvb_fast_acq(int fastacq)
{
	uint8_t 	val8, mode, guard;

	if(fastacq) {

		val8 = rReg8(0xf083);
		val8 &= ~(1 << 6);
		wReg8(0xf083, val8);

		guard = dvb_get_syr_guard(); 
		mode = dvb_get_syr_mode();

		val8 = (mode << 2) | guard | 0x10;
		wReg8(0xf004, val8);
		nmi_debug(_INFO_, "fast acq: 0xf004 (%02x)\n", rReg8(0xf004));
		val8 = (uint8_t)(mode | (guard << 4));
		wReg8(0xf133, val8);		/* TPS_SET3 */ 
		nmi_debug(_INFO_, "fast acq: 0xf133 (%02x)\n", rReg8(0xf133));

		/* Yongru recommends */
		if (mode == 0){ /* 2K */
			val8 =0x00; 
		} else if (mode == 2) { /* 4K */
			val8 =0x10;
		} 	else { /* 8K */
			val8 =0x20;
		}			
		wReg8(0xf019, val8);
		wReg8(0xf0f4, 0x12);
	} else {

		wReg8(0xf004, 0);
		wReg8(0xf133, 0);
		wReg8(0xf019, 0x30);
		wReg8(0xf0f4, 0x00);

		/* enable mode/guard detection method */
		val8 = rReg8(0xf083);
		val8 |= (1 << 6);
		wReg8(0xf083, val8);
	}
}

static void dvb_demod_init(void) 
{
#ifdef NORMALZIF
	int ix;
#endif

	wReg8(0xf000, 0x0);			/* CORE_CTRL */

	wReg8(0xf004, 0x0);			/* CORE_MODEGUARD */
	wReg8(0xf008, 0x3e);			/* AGC_TARGET */
	wReg8(0xf1c4, 0x3f);			/* FEC_SYNC_PARAM */ 
	
#ifdef NORMALZIF
	wReg8(0xf053, 0x49);			/* RIF_CTL_1 */
	wReg8(0xf11e, 0x20);			/* SIGNAL_FMT */
#else
	wReg8(0xf053, 0xc8);
#endif

#ifdef NORMALZIF
	for (ix = 0; ix < 128; ix++) {
		uint8_t val;
	
		val = 1;
		wReg8(0xf069, val);		/* RIF_RAM_CTL_1 */
		val = (uint8_t)(LUT_BB[ix]);
		wReg8(0xf06c, val);		/* RIF_RAM_DATA_1 */
		val = (uint8_t)(LUT_BB[ix] >> 8);
		wReg8(0xf06d, val);		/* RIF_RAM_DATA_2 */
		val = (uint8_t)(ix);
		wReg8(0xf06a, val);		/* RIF_RAM_ADDRESS_1 */
		val = (uint8_t)(ix >> 8);
		wReg8(0xf06b, val);		/* RIF_RAM_ADDRESS_2 */
	}

	wReg8(0xf06e, 0x00);			/* ITB_IIS_BLANK_THRESH_1 */
	wReg8(0xf06f, 0x00);			/* ITB_IIS_BLANK_THRESH_2 */
	wReg8(0xf076, (uint8_t)(LUT_BB[0] & 0xff));	/* RIF_BB_LEVEL_MIN_1 */
	wReg8(0xf077, (uint8_t)((LUT_BB[0] >> 8) & 0xff));	/* RIF_BB_LEVEL_MIN_2 */
	wReg8(0xf074, (uint8_t)(LUT_BB[127] & 0xff));	/* RIF_BB_LEVEL_MAX_1 */
	wReg8(0xf075, (uint8_t)((LUT_BB[127] >> 8) & 0xff));	/* RIF_BB_LEVEL_MAX_2 */
	wReg8(0xf072, 0x22);			/* RIF_RF_LEVEL_MIN_1 */
	wReg8(0xf073, 0x00);			/* RIF_RF_LEVEL_MIN_2 */
	wReg8(0xf070, 0x5a);			/* RIF_RF_LEVEL_MAX_1 */
	wReg8(0xf071, 0x02);			/* RIF_RF_LEVEL_MAX_2 */
	wReg8(0xf069, 0x06);			/* RIF_RAM_CTL_1 */
#endif

	wReg8(0xf180, 0x2);			/* REVIEW */
	wReg8(0xf188, 0x1);			/* REVIEW */
	wReg8(0xf189, 0x50);			/* REVIEW */
	wReg8(0xf18a, 0x0);			/* REVIEW */
	wReg8(0xf018, 0x32);			/* SCR_CTL */
	wReg8(0xf048, 3);			/* CHC_CTL_2 */

	wReg8(0xf096, 0x66);		/* CHC_ADAPT_FREQ_ITP_CTL_1 */

	wReg8(0xf051, 0x07);  		/* AGC_CTL_2 */
	wReg8(0xf01f, 0x14);			/* CRL_CTL */
#if 0
	wReg8(0xf1c9, 0xb);			/* BER_CTL */
#else
	wReg8(0xf1c9, 0x1f);			/* BER_CTL */
	wReg8(0xf1cd, 0x10);			/* BER_CTL */
#endif
	wReg8(0xf019, 0x30);			/* PPM_CTL_1 */
	wReg8(0xf00e, 0x5c);			/* CAS_CTL */

	wReg8(0xf128, 0x01);			/* REVIEW */
	wReg8(0xf078, 0x0);			/* REVIEW */
	wReg8(0xf079, 0x01);			/* REVIEW */

	wReg8(0xf0f7, 0x2);			/* SYR_MA_SEARCH_RANGE */		

	/* AGC tracking by chip */
	if (ISNM312) {

#ifdef NORMALZIF
#ifndef _EXT_LNA_
		wReg8(0xf154, 0x70);		/* VGA upper threshold */
		wReg8(0xf155, 0x02);
#else
		wReg8(0xf154, 0x26);
		wReg8(0xf155, 0x02);
#endif

		wReg8(0xf156, 0xb0);		/* VGA lower threshold */
		wReg8(0xf157, 0x01);

		/* LGMC modification, change back */
		wReg8(0xf158, 0x39);		/* RSSI upper threshold */
		/*wReg8(0xf158, 0x45);*//* LGMC threshold modification */

#ifndef _EXT_LNA_
		wReg8(0xf159, 0x04);		/* RSSI lower threshold */
#else
		wReg8(0xf159, 0x07);		/* this is for Europe */
#endif

		wReg8(0xf15f, 0x1);
		wReg8(0xf152, 0x77);
		wReg8(0xf15b, 0x59);

#else
		wReg8(0xf154, 0x88);
		wReg8(0xf155, 0x02);
		wReg8(0xf156, 0xe8);
		wReg8(0xf157, 0x01);
		wReg8(0xf158, 0x39);
		wReg8(0xf159, 0x04);

		wReg8(0xf15f, 0x1);
		wReg8(0xf152, 0x77);
		wReg8(0xf15b, 0x55);
#endif

	}

	pdvb->maxVga = 0;
	pdvb->minVga = 1024;

#ifdef _HW_TIME_SLICE_
	if (ISNM312) {
		wReg8(0xf04e, 0xff);
		wReg8(0xf04f, 0xff);
		wReg8(0xf050, 0xff);
		wReg8(0xf100, 0x9);
		wReg8(0xf101, 0x3c);
	}
#endif

	/* review: this is to enable the acquistion timer */
	wReg8(0xf05f, 0x4f);
	wReg8(0xf060, 0x00);
	wReg8(0xf061, 0x00);
	wReg8(0xf062, 0x00);
	wReg8(0xf064, 0x00);
	wReg8(0xf142, 0x10);		/* enable acquistion timer */

	return;
}

static int dvb_agc_lock(void)
{
	uint8_t val8;
	uint32_t total = 0;

	dvb_soft_rst();
	do {
		nmi_delay(20);
		val8 = rReg8(0xf001);
		if ((val8 >> 4) & 0x1)
			break;
		total += 20;
	} while (total < 100);

	if(total >=100)
		return 0;
	else 
		return 1;
	return;
}

static void dvb_syr_lock(void)
{
	uint8_t val8;
	uint32_t total = 0;

	dvb_soft_rst();
	do {
		nmi_delay(20);
		val8 = rReg8(0xf001);
		if ((val8 >> 5) & 0x1)
			break;
		total += 20;
	} while (total < 140);

	return;
}

static void dvb_tps_lock(void)
{
	uint8_t val8;
	uint32_t total = 0;
	uint32_t maxtime;

	maxtime = dvb_lock_time(pdvb->bandwidth, 300);
	dvb_soft_rst();
	do {
		nmi_delay(20);
		/* look for TPS lock */
		val8 = rReg8(0xf001);
		if ((val8 >> 6) & 0x1) {
			break;
		}
		total += 20;
	} while (total < maxtime);

	return;
}

static void dvb_chc_lock(void)
{
	uint8_t val8;
	uint32_t total = 0;

	dvb_soft_rst();
	do {
		nmi_delay(20);
		val8 = rReg8(0xf001);
		if ((val8 >> 7) & 0x1)
			break;
		total += 20;
	} while (total < 300);

	return;
}

#ifdef _HAVE_FLOAT_
static int dvb_run_demod311(void) 
{
	int i, lock = 0;
	uint8_t prevRfGain, curRfGain, val8;
	
	/* find the correct gain */
	curRfGain = dvbRfRead(0x7);		
	prevRfGain = curRfGain;
	do {
		dvb_track_agc();
		curRfGain = dvbRfRead(0x7);
		if (prevRfGain == curRfGain)
			break;
		else
			prevRfGain = curRfGain;			
	} while (1);

	/* try to lock */
	val8 = rReg8(0xf083);
	if (!((val8 >> 6) & 0x1)) {
		val8 |= (1 << 6);
		wReg8(0xf083, val8);
	}

	i = 2;
	do {
		dvb_soft_rst();
		nmi_delay(300);
		val8 = rReg8(0xf001);
		if (val8 == 0xf6) {
			lock = 1; 
			break;
		}
	} while (i--);

	if (lock) {
		double dFreq = pdvb->frequency;
		int band = pdvb->bandwidth;
		double dFreqOff = 0;

		for (i = 0; i < 5; i++) {
			dFreqOff += dvb_get_freq_offset(band);
		}
		dFreqOff /= 5.;

		dFreq += (dFreqOff/1000000.0);
		dvb_set_freq(dFreq);
		dvb_soft_rst();
		nmi_delay(300);
		val8 = rReg8(0xf001);
		if (val8 != 0xf6)
			lock = 0;
	}

	return lock;
}

#ifdef _HAVE_FLOAT_
static double dvb_get_acq_time()
{
	uint8_t val8;
	uint32_t val32;
	double acq;

	val8 = rReg8(0xf13f);
	val32 = val8 << 24;
	val8 = rReg8(0xf13e);
	val32 |= (val8 << 16);
	val8 = rReg8(0xf13d);
	val32 |= (val8 << 8);
	val8 = rReg8(0xf13c);
	val32 |= val8;

	acq = (double)val32/36670.;
	nmi_debug(_INFO_, "PHY ACQ Time, t = %f\n", acq);

	return acq;
}
#endif

static int dvb_run_demod312(void) 
{
	int lock = 0;
	uint8_t val8, i;
	uint32_t maxtim, total = 0;

#ifndef _TIME_SLICE_
	/* enable mode/guard detection method */
	val8 = rReg8(0xf083);
	if (!((val8 >> 6) & 0x1)) {
		val8 |= (1 << 6);
		wReg8(0xf083, val8);
	}
#endif

	/* wait for demod lock, depends on the bandwidth */
	maxtim = dvb_lock_time(pdvb->bandwidth, 300);
	dvb_soft_rst();
	do {
		nmi_delay(50);
		val8 = rReg8(0xf001);
		if (val8 == 0xf6) {
			pdvb->constellation = dvb_get_constellation();
			pdvb->corerate = dvb_get_corerate();
			dvb_csi_est();
			dvb_set_dst();
			dvb_set_per_window();
			val8 = dvb_get_syr_guard();
			if (val8 == 3) {		/* Echo, Guard is 1/4 */
				wReg8(0xf011, 1);
			} else {
				wReg8(0xf011, 0);
			}
			
			lock = 1;
			nmi_debug(_INFO_, "[DVB]: *** Demod Lock, time (%d) ***\n", (int)total);
 			break;
		}
		total += 50;
	} while (total < maxtim);

	if (lock) {
		double dFreq 		= pdvb->frequency;
		int band 				= pdvb->bandwidth;
		double dFreqOff = 0;

		for (i = 0; i < 5; i++) {
			dFreqOff += dvb_get_freq_offset(band);
		}
		dFreqOff /= 5.;

		dFreq += (dFreqOff/1000000.0);
		nmi_debug(_INFO_, "[DVB]: frequency offset (%f)\n", dFreqOff);

		dvb_set_freq(dFreq);
		dvb_soft_rst();
		total = 0;
		do {
			nmi_delay(50);
			val8 = rReg8(0xf001);
			if (val8 == 0xf6) {
	 			break;
			}
			total += 50;
		} while (total < maxtim);
		if (val8 != 0xf6) {
			nmi_debug(_ERR_, "[DVB]: !!! Demod Lost Lock at frequency (%f), status (%02x) !!!\n", dFreq, val8);
		}
	}

	return lock;
}
#else
static int dvb_run_demod311(void) 
{
	int i, lock = 0;
	uint8_t prevRfGain, curRfGain, val8;
	
	/* find the correct gain */
	curRfGain = dvbRfRead(0x7);		
	prevRfGain = curRfGain;
	do {
		dvb_track_agc();
		curRfGain = dvbRfRead(0x7);
		if (prevRfGain == curRfGain)
			break;
		else
			prevRfGain = curRfGain;			
	} while (1);

	/* try to lock */
	val8 = rReg8(0xf083);
	if (!((val8 >> 6) & 0x1)) {
		val8 |= (1 << 6);
		wReg8(0xf083, val8);
	}

	i = 2;
	do {
		dvb_soft_rst();
		nmi_delay(300);
		val8 = rReg8(0xf001);
		if (val8 == 0xf6) {
			lock = 1; 
			break;
		}
	} while (i--);

	if (lock) {
		long dFreq = pdvb->frequency;
		int band = pdvb->bandwidth;
		int dFreqOff = 0;

		for (i = 0; i < 5; i++) {
			dFreqOff += dvb_get_freq_offset(band);
		}
		dFreqOff /= 5;

		dFreq += dFreqOff;
		dvb_set_freq((dFreq/1000));
		dvb_soft_rst();
		nmi_delay(300);
		val8 = rReg8(0xf001);
		if (val8 != 0xf6)
			lock = 0;
	}

	return lock;
}

static int dvb_run_demod312(void) 
{
	int lock = 0;
	uint8_t val8, i;
	uint32_t maxtim, total = 0;

	/* enable mode/guard detection method */
	val8 = rReg8(0xf083);
	if (!((val8 >> 6) & 0x1)) {
		val8 |= (1 << 6);
		wReg8(0xf083, val8);
	}

	/* wait for demod lock, depends on the bandwidth */
	maxtim = dvb_lock_time(pdvb->bandwidth, 300);
	dvb_soft_rst();
	do {
		nmi_delay(50);
		val8 = rReg8(0xf001);
		if (val8 == 0xf6) {
			pdvb->constellation = dvb_get_constellation();
			pdvb->corerate = dvb_get_corerate(); 
			dvb_csi_est();
			dvb_set_dst();
			dvb_set_per_window();
			val8 = dvb_get_syr_guard();
			if (val8 == 3) {		/* Echo, Guard is 1/4 */
				wReg8(0xf011, 1);
			} else {
				wReg8(0xf011, 0);
			}
			
			lock = 1;
			nmi_debug(_INFO_, "[DVB]: *** Demod Lock, time (%d) ***\n", (int)total);
 			break;
		}
		total += 50;
	} while (total < maxtim);

	if (lock) {
		long dFreq 		= pdvb->frequency;
		int 	band 		= pdvb->bandwidth;
		int 	dFreqOff 	= 0;

		for (i = 0; i < 5; i++) {
			dFreqOff += dvb_get_freq_offset(band);
		}
		dFreqOff /= 5;
		nmi_debug(_INFO_, "[DVB]: frequency offset (%d)\n", dFreqOff);

		dFreq += dFreqOff;
		dvb_set_freq((dFreq/1000));
		dvb_soft_rst();
		total = 0;
		do {
			nmi_delay(50);
			val8 = rReg8(0xf001);
			if (val8 == 0xf6) {
	 			break;
			}
			total += 50;
		} while (total < maxtim);
		if (val8 != 0xf6) {
			nmi_debug(_ERR_, "[DVB]: !!! Demod Lost Lock at frequency (%d), status (%02x) !!!\n", dFreq, val8);
		}
	}

	return lock;
}
#endif

static int  dvb_run_demod(void *pv)
{
	DVBRUN *p = (DVBRUN *)pv;
	int lock;

	if (ISNM311) 
		lock = dvb_run_demod311();
	
	if (ISNM312) {
#ifdef _TIME_SLICE_
		dvb_fast_acq(0);
#endif	
		lock = dvb_run_demod312();
#ifdef _TIME_SLICE_
		dvb_fast_acq(1);
#endif
	}

	p->lock = lock;
	
	return lock;		
}

/********************************************
	DVB Mac Functions
********************************************/

static void dvb_t_filter_reset(void)
{
	int i;
	uint32_t adr, val;

	for (i = 0; i < 8; i++) {
		adr = 0xf204 + i * 4;
		val = rReg32(adr);
		val &= ~(0x3 << 13);
		wReg32(adr, val);
	}

	return;
}

void dvb_t_filter_add_pid(uint32_t pid)
{
	int i, found = 0;
	uint32_t adr, val;

	nmi_debug(_INFO_, "[DVB]: T filter add, pid (%08x)\n", pid); 

	for (i = 0; i < 8; i++) {
		adr = 0xf204 + i * 4;
		val = rReg32(adr);
		if (!((val >> 13) & 0x1)) {
			val &= ~0x1fff;
			val |= (pid & 0x1fff);
			val |= (1 << 13);
			found = 1;
		} else if (!((val >> 14) & 0x1)) {
			val &= ~(0x1fff << 16);
			val |= ((pid & 0x1fff) << 16);
			val |= (1 << 14);
			found = 1;
		}

		if (found) {
			wReg32(adr, val);
			nmi_debug(_INFO_, "[DVB]: T filter add, adr (%08x),(%08x)\n", adr, rReg32(adr)); 
			break;
		}
	}

	return;
}

void dvb_t_filter_remove_pid(uint32_t pid)
{
	int i, found = 0;
	uint32_t adr, val;

	nmi_debug(_INFO_, "[DVB]: T filter remove, pid (%08x)\n", pid); 

	for (i = 0; i < 8; i++) {
		adr = 0xf204 + i * 4;
		val = rReg32(adr);
		if (((val >> 13) & 0x1) && ((val & 0x1fff) == pid)) {
			val &= ~(1 << 13);
			found = 1;
		} else if (((val >> 14) & 0x1) && (((val >> 16) & 0x1fff) == pid)) {
			val &= ~(1 << 14);
			found = 1;
		}

		if (found) {
			wReg32(adr, val);
			nmi_debug(_INFO_, "[DVB]: T filter remove, adr (%08x),(%08x)\n", adr, rReg32(adr)); 
			break;
		}
	}

	return;
}

static void dvb_t_set_pid(void *pv)
{
	DVBSETPID *p = (DVBSETPID *)pv;
	int npid = p->npid;
	int i, ix, ix1;
	uint32_t adr, val;

	if (!pdvb->fdvbt)
		return;

	/* disable all the pid filter */
	for (i = 0; i < 8; i++) {
		adr = 0xf204 + i * 4;
		val = rReg32(adr);
		val &= ~(0x3 << 13);
		wReg32(adr, val);
	}

	/* enable the pid filter */
	for (i = 0; i < npid; i++) {
		ix = i / 2;
		ix1 = i % 2;
		nmi_debug(_INFO_, "[DVB]: T filter ix (%d), ix1 (%d), (%08x)\n", ix, ix1, p->pid[i]); 

		adr = 0xf204 + ix * 4;
		val = rReg32(adr);
		if (ix1 == 0) {
			val &= ~(0x1fff);
			val |= p->pid[i] | (0x1 << 13);
		} else {
			val &= ~(0x1fff << 16);
			val |= (p->pid[i] << 16) | (0x1 << 14);
		}
		wReg32(adr, val);			
		nmi_debug(_INFO_, "[DVB]: T filter enable (%08x)(%08x)\n", adr, rReg32(adr)); 
	}

	return;
}

static void dvb_t_pid_filter(DVBLNKCTL *p)
{
	int npid = p->ntfilt;
	int i, ix, ix1;
	uint32_t adr, val;
	uint16_t pid;

	/* enable it */
	for (i = 0; i < npid; i++) {
		ix = i / 2;
		ix1 = i % 2;

		adr = 0xf204 + ix * 4;
		val = rReg32(adr);
		if (ix1 == 0) {
			val &= ~(0x1fff);
			val &= ~(0x1 << 13);
			pid = p->tfilt[i];
			val |= pid | (0x1 << 13);
		} else {
			val &= ~(0x1fff << 16);
			val &= ~(0x1 << 14);
			pid = p->tfilt[i];
			val |= (pid << 16) | (0x1 << 14);
		}
		wReg32(adr, val);			
		nmi_debug(_INFO_, "[DVB]: T filter enable (%08x)(%08x)\n", adr, rReg32(adr)); 
	}

	/* disable it */
	for (i = npid; i < 16; i++) {
		ix = i / 2;
		ix1 = i % 2;
		adr = 0xf204 + ix * 4;
		val = rReg32(adr);
		if (ix1 == 0) {
			val &= ~(0x1 << 13);
		} else {
			val &= ~(0x1 << 14); 
		}
		wReg32(adr, val);
		nmi_debug(_INFO_, "[DVB]: T filter disable (%08x)(%08x)\n", adr, rReg32(adr)); 
	}

	return;
}

static void dvb_h_ip_filter(DVBLNKCTL *p)
{
	uint32_t val;
	int i;
	uint32_t adr;
	int nip;
	uint32_t ipadr;

	/* make sure global enable ip filtering */
	val = rReg32(0xf200);		/* GLB_CTRL */
	if (!((val >> 3) & 0x1)) {
		val |= (0x1 << 3);
	}

	/* for ipv4 */
	nip = p->nip4;
	/* ipv4 filter address */
	for (i = 0; i < nip; i++) {
		adr = 0xf228 + i * 4;		/* IPV4_ADDRx */
		ipadr = p->ip4filtadr[i];
		wReg32(adr, ipadr);
	}

	/* for ipv6 */
	nip = p->nip6;
	for (i = 0; i < (nip * 4); i++) {
		adr = 0xf258 + i * 4;
		ipadr = p->ip6filtadr[i];
		wReg32(adr, ipadr);
	}

	/* enable it */		
	val = 0;
	wReg32(0xf224, val);		/* IP_CTRL */
	nip = p->nip4;
	for (i = 0; i < nip; i++) {
		val |= (0x1 << i);
	}
	nip = p->nip6;
	for (i = 0; i < nip; i++) {
		val |= (0x1 << (i + 12));
	}
	wReg32(0xf224, val);
	
	return;
}

static void dvb_get_fer(void *pv)
{
#ifdef _HAVE_FLOAT_
	DVBFER *p = (DVBFER *)pv;
	double dTotal = (double)pdvb->nframe;
	double dErr = (double)pdvb->nfecNeed;

	if (dErr == 0.) {
		p->fer = 0.;
	} else {
		p->fer = (dErr/dTotal); 
	}
	return;
#endif
}

static void dvb_get_mfer(void *pv)
{
#ifdef _HAVE_FLOAT_
	DVBMFER *p = (DVBMFER *)pv;
	double dTotal = (double)pdvb->nframe;
	double dErr = (double)pdvb->nframeErr;
	if (dErr == 0.) {
		p->mfer = 0.;
	} else {
		p->mfer = (dErr/dTotal); 
	}
	return; 
#endif
}

static void dvb_h_pid(DVBLNKCTL *p)
{
	int i, npid = p->npid;
	uint32_t val;
	uint32_t adr;
	uint16_t pid;
	uint8_t fec, type;
	int filt = 0;

	for (i = 0; i < npid; i++) {
		adr = 0xf204 + i * 4;			/* PIDx */
		val = rReg32(adr);
		pid = p->pid[i];
		val &= ~(0x1fff);
		val |= pid;
		fec = p->fec[i];
		val &= ~(0x3 << 18);
		val |= (fec << 18);
		type = p->type[i];
		val &= ~(0x3 << 16);
		val |= (type << 16);
		if (p->enfilt[i]) {
			val |= (0x1 << 22);
			if (!filt)
				filt = 1;
		} else {
			val &= ~(0x1 << 22);
		}
		val |= (0x1 << 20);		/* fec_en */
		val |= (0x1 << 21);		/* fec_mode */
		wReg32(adr, val);
		nmi_debug(_INFO_, "[DVB]: PID(%08x)(%08x)\n", adr, rReg32(adr));
	
	} 

	for (i = npid; i < 8; i++) {
		adr = 0xf204 + i * 4;
		val = rReg32(adr);
		val &= ~(0x3 << 16);
		wReg32(adr, val);
	}

	if (filt) {
		dvb_h_ip_filter(p);
	}

	return;
}

static void dvb_run_lnk(void *pv)
{
	DVBLNKCTL *plnk = (DVBLNKCTL *)pv;
	uint32_t val32;

	/* disable link layer */
	val32 = rReg32(0xf200);
	val32 |= 0x1;
	wReg32(0xf200, val32);

	nmi_debug(_INFO_, "GLB CTRL 1 (%08x)\n", rReg32(0xf200));

	if (plnk->mode == 1) {
		pdvb->fdvbt = 1;
	} else {
		pdvb->fdvbt = 0;
	}

	/* gpio ctrl */
	if (plnk->tsmode) {
		wReg32(0xb400, 0x12492492);
		wReg32(0xb43c, 0x12);
	}

	/* 
		comment: if it is using TS output, then we can stop
			DRAM refresh to save power
	*/
	val32 = rReg32(0xf2c4);
	if (plnk->tsmode) {
		if (val32 & 0x1) {
			val32 &= ~0x1;
			wReg32(0xf2c4, val32);
		}
	} else {
		if (!(val32 & 0x1)) {
			val32 |= 0x1;
			wReg32(0xf2c4, val32);
		}
	}

	/* TS Ouptut */
	val32 = rReg32(0xf2f4);					/* TS_OUTPUT */
	if (plnk->tsmode) {
		val32 |= 0x1;									/* ts_out_en */
		if (plnk->mode == 1)					
			val32 |= (0x1 << 1);					/* ts source, dvb-t */
		else
			val32 &= ~(0x1 << 1);				/* ts source, dvb-h */
		if (plnk->tsmode == 1)					/* ts_mode, parallel */
			val32 &= ~(0x1 << 2);
		else													/* ts_mode, serial */
			val32 |= (0x1 << 2);
		val32 &= ~(0x3 << 3);					/* ts_clk_rate */
		val32 |= (plnk->tsclk << 3);

		if (plnk->tsgatclk == 1)					
			val32 |= (0x1 << 6);					/* ts gated clock */
		else
			val32 &= ~(0x1 << 6);				
		if (plnk->tsdatpol == 1)					
			val32 |= (0x1 << 8);					/* ts data polarity */
		else
			val32 &= ~(0x1 << 8);				
		if (plnk->tsvalpol == 1)					
			val32 |= (0x1 << 9);					/* ts valid polarity */
		else
			val32 &= ~(0x1 << 9);				
		if (plnk->tssynpol == 1)					
			val32 |= (0x1 << 10);				/* ts sync polarity */
		else
			val32 &= ~(0x1 << 10);				
	} else {
		val32 &= ~0x1;
	}
	val32 |=0x40;
	val32 &=~0x20;
	wReg32(0xf2f4, val32);
	nmi_debug(_INFO_, "[DVB]: TS OUTPUT (%08x)\n", rReg32(0xf2f4));

	/* MISC CTRL */
	val32 = rReg32(0xf2c0);
	if (plnk->mode == 1) {
		val32 |= (0x1 << 29);
		if (plnk->entfilt) {
			val32 |= (0x1 << 30);
			if (!plnk->tskip)
				dvb_t_pid_filter(plnk);
		} else {
			val32 &= ~(0x1 << 30);
		}
	} else {
		val32 &= ~(0x1 << 29);
	}
	wReg32(0xf2c0, val32);
	nmi_debug(_INFO_, "[DVB]: MISC CTRL (%08x)\n", rReg32(0xf2c0));

	/* DUMP CTRL */
	val32 = rReg32(0xf2b0);
	if (plnk->mode == 1) {								/* dvb-t */
		if (!plnk->tsmode) {
			val32 |= 0x1;
			if (pchp->bustype == _SDIO_ || pchp->bustype == _EBI_)
				val32 &= ~(0x3 << 2);					/* host_id, sdio host */
			else if (pchp->bustype == _SPI_)
				val32 |= (0x3 << 2);

			val32 |= (0x1 << 13);						/* dvbt_dma_en */
			val32 &= ~(0xfff << 14);
			val32 |= ((plnk->tsz/*348/*1742/2*/) << 14);						/* dvbt_dma_size */
		} else {
			val32 &= ~(0x1 << 13);					
		}
	} else {														/* dvb-h */
		if (!plnk->tsmode) { 
			val32 |= 0x1;										/* dump_mode, dma */
			if (pchp->bustype == _SDIO_ || pchp->bustype == _EBI_)
				val32 &= ~(0x3 << 2);					/* host_id, sdio host */
			else if (pchp->bustype == _SPI_)
				val32 |= (0x3 << 2);
			val32 |= (0x1 << 25);						/* mult_dma_wait */
		} else {
			val32 &= ~0x1;									/* dump mode, ts */
		}
		val32 |= (0x1 << 11);							/* whole_dg */
	}
	val32 |= (0x1 << 1); 							/* whole frame */
	wReg32(0xf2b0, val32);
	nmi_debug(_INFO_, "[DVB]: DUMP CTRL (%08x)\n", rReg32(0xf2b0));

	if (plnk->mode == 0) {
		/* Program the PID */
		dvb_h_pid(plnk);
	}

	if (!plnk->tsmode) {
		/* enable interrupt */
		wReg32(0xf2bc, 0xffffffff);		/* clear interrupt */
		nmi_debug(_INFO_, "[DVB]: interrupt status (%08x)\n", rReg32(0xf2bc));

		if (pchp->bustype == _SDIO_) {
			val32 = 0x8;
			wReg32(0xf2f8, val32);
			nmi_debug(_INFO_, "[DVB]: IRQ_EN_SDIO (%08x)\n", rReg32(0xf2f8));

			/* this is for debug */
#if 0
			val32 = rReg32(0xd030);
			nmi_debug(_INFO_, "[DVB]: SDIO intr enable 1 (%08x)\n", val32);
			//val32 &= ~(0x1 << 2);
			val32 = 0;
			wReg32(0xd030, val32);
			nmi_debug(_INFO_, "[DVB]: SDIO intr enable 2 (%08x)\n", rReg32(0xd030));
#endif
		} else if (pchp->bustype == _SPI_) {
			val32 = 0x8;
			wReg32(0xf2b8, val32);

		} else if (pchp->bustype == _EBI_) {
			val32 = 0x8;
			wReg32(0xf2f8, val32);
			val32 = 0x9; /* load done & DMA ready */
			wReg32(0xf2b8, val32);
		}
	}

	/* enable link layer */
	val32 = rReg32(0xf200);
	val32 &= ~0x1;
	wReg32(0xf200, val32);
	nmi_debug(_INFO_, "GLB CTRL 2 (%08x)\n", rReg32(0xf200));
}

static void dvb_lnk_init()
{
	wReg32(0xf200, 0x60a40006);			/* GLB_CTRL */ 
	wReg32(0xf2c0, 0x00d8e021);			/* MISC_CTL */		
	wReg32(0xf2b4, 0x145949);			/* DELTA_T */

	/* reset the pid filter */
	dvb_t_filter_reset();
	
}

static void dvb_rst_lnk(void)
{
	uint32_t val32;

	/* GLB control */
	val32 = rReg32(0xf200);
	val32 |= 0x1;		/* disable link layer */
	wReg32(0xf200, val32);

	/* disable the interrupt */
	if (pchp->bustype == _SDIO_ || pchp->bustype == _EBI_) {
		wReg32(0xf2f8, 0);
	} else {
		wReg32(0xf2b8, 0);
	}

	/* disbale TS output, if enable */
	val32 = rReg32(0xf2f4);
	if (val32 & 0x1) {
		val32 &= ~0x1;
		wReg32(0xf2f4, val32);
	}
	
	/* reenable GLB control */
	val32 = rReg32(0xf200);
	val32 &= ~0x1;		/* enable link layer */
	wReg32(0xf200, val32);

	return;	
}

static void dvb_rst_lnk_cnt(void)
{
	pdvb->nframe = 1;
	pdvb->nframeErr = 0;
	pdvb->nfecNeed = 0;
	pdvb->prefifoptr = 0xffffffff;
}

static void dvb_get_lnk_sta(void *pv)
{
#ifdef _HAVE_FLOAT_
	DVBLNKSTA *p = (DVBLNKSTA *)pv;
	DVBFER fer;
	DVBMFER mfer;

	dvb_get_fer((void *)&fer);
	p->dFer = fer.fer;
	dvb_get_mfer((void *)&mfer);
	p->dMFer = mfer.mfer;
	p->nFrame = pdvb->nframe;
#endif

	return;
}

static void dvb_get_psisi_tbl(void *pv)
{
	DVBPSISITBL *p = (DVBPSISITBL *)pv; 
	uint32_t val32;
	uint32_t pid = p->pid;
	uint32_t tid = p->tid;
	uint32_t sadr = 0x0;

	pdvb->fdvbt = 0;

	/* disable linke layer */
	val32 = rReg32(0xf200);
	val32 |= 0x1;
	wReg32(0xf200, val32);

	/* turn on the 8051 clock */
	val32 = rReg32(0xb410);
	if (!(val32 & 0x3))
		val32 |= 0x3;
	wReg32(0xb410, val32);
	nmi_debug(_VERB_, "[DVB]: table, 0xb410 (%08x)\n", rReg32(0xb410));

	/* let link layer access 8051 memory */
	val32 = rReg32(0xa808);
	if (!((val32 >> 1) & 0x1))
		val32 |= 0x2;										/* turn on 8051 */
	val32 &= ~((1 << 4) | (1 << 2));		/* link layer access memory */
	wReg32(0xa808, val32);
	nmi_debug(_VERB_, "[DVB]: table, 0xa808 (%08x)\n", rReg32(0xa808));

	val32 = (pid & 0x1fff) | ((tid & 0xff) << 16);
	if (p->fulltbl) 
		val32 |= (0x1 << 24);
	else
		val32 &= ~(0x1 << 24);
	if (p->ignorecrc) 
		val32 |= (0x1 << 25);
	else
		val32 &= ~(0x1 << 25);
	if (p->notidmatch) 
		val32 |= (0x1 << 26);
	else
		val32 &= ~(0x1 << 26);
	wReg32(0xf2ac, val32);				/* SP_CTRL2 */
	nmi_debug(_INFO_, "[DVB]: SP_CTRL2 (%08x)\n", rReg32(0xf2ac));

	val32 = ((sadr & 0x7fff) << 1) | 0x1;
	wReg32(0xf2a8, val32);				/* SP_CTRL1 */
	nmi_debug(_INFO_, "[DVB]: SP_CTRL1 (%08x)\n", rReg32(0xf2a8));

	/* enable PSI/SI table interrupt */
	if (pchp->bustype == _SDIO_) {
		wReg32(0xd014, sadr);
		val32 = rReg32(0xf2f8);
		val32 |= (1 << 9);
		wReg32(0xf2f8, val32);
		nmi_debug(_INFO_, "[DVB]: SD IRQ Enable (%08x)\n", rReg32(0xf2f8));

		/* clear the status */
		wReg32(0xf2bc, 0xffffffff);		/* clear interrupt */
	} else if (pchp->bustype == _SPI_) {
		val32 = rReg32(0xf2b8);
		val32 |= (0x1 << 9);
		wReg32(0xf2b8, val32);
		nmi_debug(_INFO_, "[DVB]: IRQ Enable (%08x)\n", rReg32(0xf2b8));
	} else if (pchp->bustype == _EBI_) {
		val32 = rReg32(0xf2b8);
		val32 |= (0x1 << 9);
		wReg32(0xf2b8, val32);
		nmi_debug(_INFO_, "[DVB]: IRQ Enable (%08x)\n", rReg32(0xf2b8));
	}

	val32 = rReg32(0xf200);			/* GLB_CTRL */
	val32 |= (0x1 << 1);
	val32 &= ~(0x3 << 4);
	val32 |= ((p->pidmatch & 0x3) << 4);
	val32 &= ~(0x3 << 6);
	val32 |= ((p->tidmatch & 0x3) << 6);
	//val32 |= (1 << 31);					/* PID lock */
	if (val32 & 0x1)
		val32 &= ~0x1;
	wReg32(0xf200, val32);
	nmi_debug(_INFO_, "[DVB]: GLB_CTRL (%08x)\n", rReg32(0xf200));
}

/********************************************
	DVB Intr Functions
********************************************/
static void dvb_poll_intr(void *pv)
{
	uint32_t fifoptr, fifolptr, fifosadr, fifosta;

	fifoptr = rReg32(0xf300);
	fifolptr = (fifoptr >> 4) & 0xf;
	fifosadr = (0xf304 + fifolptr * 6 * 4);
	fifosta = rReg32(fifosadr);

	if ((fifosta & 0x100) && (fifoptr != pdvb->prefifoptr)) {	/* data ready */
		pdvb->prefifoptr = fifoptr;

		pdvb->nframe++;
		if ((fifosta >> 4) & 0x1) {
			pdvb->nfecNeed++;
		}

		if ((fifosta >> 6) & 0x1) {
			pdvb->nframeErr++;
		}
	}
}

static void dvb_handle_ebi_intr(void *pv)
{
	uint32_t val32, status, enable,enable_sdio;
	uint32_t tcnt, tadr, tail;
	DVBEBIDMA dma;	
	uint32_t fifoptr;
	uint32_t wrap;

	if (pv != NULL)
		*((uint32_t *)pv) = 0;	/* clear the return status */

	enable_sdio = rReg32(0xf2f8); // irq_en_sdio
	enable = rReg32(0xf2b8); // irq_en
	status = rReg32(0xf2bc);
	nmi_debug(_INFO_, "[DVB]: intr, status (%08x), enable (%08x), enable_sdio (%08x)\n", status, enable, enable_sdio);
	/* clear the intr status */
	wReg32(0xf2bc, status);

	if ((status >> 8) & 0x1) {
		nmi_debug(_ERR_, "[DVB]: *** DMA Overflow ***\n");
		if (pv != NULL)
			*((uint32_t *)pv) |= DVB_ASIC_BUF_OVERFLOW;
		if (!pdvb->fdvbt) {	/* for DVB-H, we need to clear the overflow */
			tcnt = rReg32(0xd01c);		
			tadr = rReg32(0xd014);

			/* try to clear the overflow condition */
			tadr += tcnt - 4;
			wReg32(0xd01c, 4);		
			wReg32(0xd014, tadr);

			/* just do a read, will this work??? */
			rReg32(tadr);
			/* check the status, should see the DMA done */
			val32 = rReg32(0xf2bc);
			if ((val32 >> 4) & 0x1) {
				nmi_debug(_INFO_, "[DVB]: *** Clear the Overflow ***\n");
			} else {
				nmi_debug(_ERR_, "[DVB]: *** Can't Clear the Overflow ***\n");
			}
		}
		return;
	}

	if (!pdvb->fdvbt) {
		if ((status & 0x1) && (enable & 0x1)) {
			/* review: what do I need to do??? */
			nmi_debug(_INFO_, "[DVB]: *** load done ***\n");
		}
	}

	if (!pdvb->fdvbt) {
		if (((status >> 9) & 0x1) && ((enable >> 9) & 0x1)) {
			DVBTBLREAD tbl;
			nmi_debug(_INFO_, "[DVB]: *** PSI/SI tbl ***\n");
		
			if (pv != NULL)
				*((uint32_t *)pv) |= DVB_ASIC_TBL_READY;

			val32 = rReg32(0xf2a8);		/* SP_CTRL1 */
			nmi_debug(_VERB_, "[DVB]: SP_CTRL1 (%08x)\n", val32);

			tbl.sadr = (val32 >> 1) & 0x7fff;
			tbl.sz = (val32 >> 16) & 0x1fff;
			nmi_debug(_INFO_, "[DVB]: PSI/SI table adr (%08x), size (%d)\n", tbl.sadr, tbl.sz);

			/* let host access the 8051 memory */
			val32 = rReg32(0xa808);
			val32 |= (1 << 4);
			wReg32(0xa808, val32);
			nmi_debug(_VERB_, "[DVB]: PSI/SI, 0xa808 (%08x)\n", rReg32(0xa808));

			if (tbl.sz > 0) {
				nmi_tbl_read((void *)&tbl);
			}
			/* disable psi/si interrupt */
			val32 = rReg32(0xf2f8);
			val32 &= ~(1 << 9);
			wReg32(0xf2f8, val32);

			/* review: disable 8051 clock??? */
			val32 = rReg32(0xb410);
			if (val32 & 0x3)
				val32 &= ~0x3;
			wReg32(0xb410, val32);
		}
	}

	if (((status >> 3) & 0x1) && ((enable >> 3) & 0x1)) {
		int loopcount = 0;

		nmi_debug(_INFO_, "[DVB]: *** dma ready ***\n");

		if (pv != NULL)
			*((uint32_t *)pv) |= DVB_ASIC_DATA_READY;

		if (!pdvb->fdvbt) {
			pdvb->nframe++;
			if ((status >> 2) & 0x1)
				pdvb->nframeErr++;
			val32 = rReg32(0xf300);		/* FIFO_PTR */
			fifoptr = ((val32 >> 8) & 0xf);
			val32 = rReg32((0xf300 + fifoptr * 24 + 0x4));
			if ((val32 >> 4) & 0x1)
				pdvb->nfecNeed++;
		} else {
			/* review: this is to work around link layer's memory cache issue.  Do a
					read to sync up */
			val32 = rReg32(0xd014);
			val32 ^= (0x1 << 3);
			rReg32(val32);
		}

		/* handle wrapped around, if it happen */
		do {
#if 0 // EBI does not have to use multiwait DMA			
			if (!pdvb->fdvbt) {
				/* dma to go */
				val32 = rReg32(0xf2b0);
				val32 |= (0x1 << 26);
				wReg32(0xf2b0, val32);

				/* wait for it is ready */
				retry = 100;
				do {
					val32 = rReg32(0xf2b0);
					if ((val32 >> 8) & 0x1)
						break;
				} while (retry--);	

				if (retry <= 0) {
					nmi_debug(_INFO_, "[DVB]: Error, no DMA Go not ready...\n");
				}
			} 
#endif
			/* read the count */
			tcnt = rReg32(0xd01c);		
			tadr = rReg32(0xd014);
			if (!pdvb->fdvbt) {
				val32 = rReg32(0xf2b0);
				tail = (val32 >> 4) & 0x3;
			} else {
				tail = 0;
			}

			val32 = rReg32(0xf2b0);
			wrap=(val32>>6) & 0x3;
			
			nmi_debug(_INFO_, "[DVB]: intr, count(%d), address(%08x), tail(%08x)\n", tcnt, tadr, tail,wrap);

			dma.tcnt=tcnt-tail;
			dma.tadr=tadr;
			/* move the data */
			nmi_dma_read((void *)&dma);

			val32 = rReg32(0xf2bc);
			if ((val32 >> 4) & 0x1) {
				nmi_debug(_INFO_, "[DVB]: *** DMA Done ***\n");
				wReg32(0xf2bc, val32);
				break;
			}
			loopcount++;
			if (loopcount > 1) {
				nmi_debug(_INFO_, "[DVB]: Error, DMA never done...\n");
				break;
			}

		} while (1);
	}

	nmi_debug(_INFO_, "[DVB]: intr clear status (%08x)\n", rReg32(0xf2bc));
	return;	

}

static void dvb_handle_spi_intr(void *pv)
{
	int retry;
	uint32_t status, val32, fifoptr, enable;
	uint32_t tcnt, tadr, tail;

#ifdef _HW_TIME_SLICE_
	wReg32(0xb464, 0x09ff0000);
#endif

	if (pv != NULL)
		*((uint32_t *)pv) = 0;	/* clear the return status */

	enable = rReg32(0xf2b8);
	status = rReg32(0xf2bc);
	nmi_debug(_INFO_, "[DVB]: intr, status (%08x), enable (%08x)\n", status, enable);
	/* clear the intr status */
	wReg32(0xf2bc, status);

	if ((status >> 8) & 0x1) {
		nmi_debug(_ERR_, "[DVB]: *** DMA Overflow ***\n");
		if (pv != NULL)
			*((uint32_t *)pv) |= DVB_ASIC_BUF_OVERFLOW;
		if (!pdvb->fdvbt) {	/* for DVB-H, we need to clear the overflow */
			tcnt = rReg32(0xe068);		
			tadr = rReg32(0xe064);

			/* try to clear the overflow condition */
			tadr += tcnt - 4;
			wReg32(0xe068, 4);		
			wReg32(0xe064, tadr);

			/* just do a read, will this work??? */
			rReg32(tadr);
			/* check the status, should see the DMA done */
			val32 = rReg32(0xf2bc);
			if ((val32 >> 4) & 0x1) {
				nmi_debug(_INFO_, "[DVB]: *** Clear the Overflow ***\n");
			} else {
				nmi_debug(_ERR_, "[DVB]: *** Can't Clear the Overflow ***\n");
			}
		}
		return;
	}

	if (!pdvb->fdvbt) {
		if ((status & 0x1) && (enable & 0x1)) {
			/* review: what do I need to do??? */
			nmi_debug(_INFO_, "[DVB]: load done...\n");
		}
	}

	if (!pdvb->fdvbt) {
		if (((status >> 9) & 0x1) && ((enable >> 9) & 0x1)) {
			DVBTBLREAD tbl;
			nmi_debug(_INFO_, "[DVB]: PSI/SI tbl...\n");
		
			if (pv != NULL)
				*((uint32_t *)pv) |= DVB_ASIC_TBL_READY;
			val32 = rReg32(0xf2a8);		/* SP_CTRL1 */
			nmi_debug(_VERB_, "[DVB]: SP_CTRL1 (%08x)\n", val32);
			tbl.sadr = (val32 >> 1) & 0x7fff;
			tbl.sz = (val32 >> 16) & 0x1fff;
			nmi_debug(_INFO_, "[DVB]: PSI/SI table adr (%08x), size (%d)\n", tbl.sadr, tbl.sz);
			/* let host access the 8051 memory */
			val32 = rReg32(0xa808);
			val32 |= (1 << 4);
			wReg32(0xa808, val32);
			nmi_debug(_VERB_, "[DVB]: PSI/SI, 0xa808 (%08x)\n", rReg32(0xa808));
			if (tbl.sz > 0) {
				nmi_tbl_read((void *)&tbl);
			}
			/* disable psi/si interrupt */
			val32 = rReg32(0xf2f8);
			val32 &= ~(1 << 9);
			wReg32(0xf2f8, val32);

			/* review: disable 8051 clock??? */
			val32 = rReg32(0xb410);
			if (val32 & 0x3)
				val32 &= ~0x3;
			wReg32(0xb410, val32);
		}
	}

	if (((status >> 3) & 0x1) && ((enable >> 3) & 0x1)) {
		int loopcount = 0;

		nmi_debug(_INFO_, "[DVB]: dma ready...\n");

		if (pv != NULL)
			*((uint32_t *)pv) |= DVB_ASIC_DATA_READY;

		if (!pdvb->fdvbt) {
			pdvb->nframe++;
			if ((status >> 2) & 0x1)
				pdvb->nframeErr++;
			val32 = rReg32(0xf300);		/* FIFO_PTR */
			fifoptr = ((val32 >> 8) & 0xf);
			val32 = rReg32((0xf300 + fifoptr * 24 + 0x4));
			if ((val32 >> 4) & 0x1)
				pdvb->nfecNeed++;
		} else 

		/* review: this is to work around link layer's memory cache issue.  Do a
				read to sync up */
		val32 = rReg32(0xe064);
		val32 ^= (0x1 << 3);
		rReg32(val32);

		/* handle wrapped around, if it happen */
		do {
			if (!pdvb->fdvbt) {
				/* dma to go */
				val32 = rReg32(0xf2b0);
				val32 |= (0x1 << 26);
				wReg32(0xf2b0, val32);

				/* wait for it is ready */
				retry = 100;
				do {
					val32 = rReg32(0xf2b0);
					if ((val32 >> 8) & 0x1)
						break;
				} while (retry--);	

				if (retry <= 0) {
					nmi_debug(_INFO_, "[DVB]: Error, no DMA Go not ready...\n");
				}
			} 

			if (!pdvb->fdvbt) {
				val32 = rReg32(0xf2b0);
				tail = (val32 >> 4) & 0x3;
			} else {
				tail = 0;
			}

			/* move the data */
			nmi_dma_read((void *)&tail);

			val32 = rReg32(0xf2bc);
			if ((val32 >> 4) & 0x1) {
				nmi_debug(_INFO_, "[DVB]: *** DMA Done ***\n");
				wReg32(0xf2bc, val32);
				break;
			}
			loopcount++;
			if (loopcount > 1) {
				nmi_debug(_INFO_, "[DVB]: Error, DMA never done...\n");
				break;
			}
		} while (1);
	} 

	/* clear the intr status */
	//wReg32(0xf2bc, status);
	nmi_debug(_INFO_, "[DVB]: intr clear status (%08x)\n", rReg32(0xf2bc));

#ifdef _HW_TIME_SLICE_
	wReg32(0xb464, 0x09ff0fff);
#endif

	return;
}

static void dvb_handle_sdio_intr(void *pv)
{
	uint32_t val32, status, enable;
	uint32_t tcnt, tadr, tail;
	int i, dummy, retry;
	uint32_t nblk, nleft;
	DVBSDIODMA dma;	
	uint32_t fifoptr;

	if (pv != NULL)
		*((uint32_t *)pv) = 0;	/* clear the return status */

	enable = rReg32(0xf2f8);
	status = rReg32(0xf2bc);
	nmi_debug(_INFO_, "[DVB]: intr, status (%08x), enable (%08x)\n", status, enable);
	/* clear the intr status */
	wReg32(0xf2bc, status);

	if ((status >> 8) & 0x1) {
		nmi_debug(_ERR_, "[DVB]: *** DMA Overflow ***\n");
		if (pv != NULL)
			*((uint32_t *)pv) |= DVB_ASIC_BUF_OVERFLOW;
		if (!pdvb->fdvbt) {	/* for DVB-H, we need to clear the overflow */
			tcnt = rReg32(0xd01c);		
			tadr = rReg32(0xd014);

			/* try to clear the overflow condition */
			tadr += tcnt - 4;
			wReg32(0xd01c, 4);		
			wReg32(0xd014, tadr);

			/* just do a read, will this work??? */
			rReg32(tadr);
			/* check the status, should see the DMA done */
			val32 = rReg32(0xf2bc);
			if ((val32 >> 4) & 0x1) {
				nmi_debug(_INFO_, "[DVB]: *** Clear the Overflow ***\n");
			} else {
				nmi_debug(_ERR_, "[DVB]: *** Can't Clear the Overflow ***\n");
			}
		}
		return;
	}

	if (!pdvb->fdvbt) {
		if ((status & 0x1) && (enable & 0x1)) {
			/* review: what do I need to do??? */
			nmi_debug(_INFO_, "[DVB]: *** load done ***\n");
		}
	}

	if (!pdvb->fdvbt) {
		if (((status >> 9) & 0x1) && ((enable >> 9) & 0x1)) {
			DVBTBLREAD tbl;
			nmi_debug(_INFO_, "[DVB]: *** PSI/SI tbl ***\n");
		
			if (pv != NULL)
				*((uint32_t *)pv) |= DVB_ASIC_TBL_READY;

			val32 = rReg32(0xf2a8);		/* SP_CTRL1 */
			nmi_debug(_VERB_, "[DVB]: SP_CTRL1 (%08x)\n", val32);

			tbl.sadr = (val32 >> 1) & 0x7fff;
			tbl.sz = (val32 >> 16) & 0x1fff;
			nmi_debug(_INFO_, "[DVB]: PSI/SI table adr (%08x), size (%d)\n", tbl.sadr, tbl.sz);

			/* let host access the 8051 memory */
			val32 = rReg32(0xa808);
			val32 |= (1 << 4);
			wReg32(0xa808, val32);
			nmi_debug(_VERB_, "[DVB]: PSI/SI, 0xa808 (%08x)\n", rReg32(0xa808));

			if (tbl.sz > 0) {
				nmi_tbl_read((void *)&tbl);
			}
			/* disable psi/si interrupt */
			val32 = rReg32(0xf2f8);
			val32 &= ~(1 << 9);
			wReg32(0xf2f8, val32);

			/* review: disable 8051 clock??? */
			val32 = rReg32(0xb410);
			if (val32 & 0x3)
				val32 &= ~0x3;
			wReg32(0xb410, val32);
		}
	}

	if (((status >> 3) & 0x1) && ((enable >> 3) & 0x1)) {
		int loopcount = 0;

		nmi_debug(_INFO_, "[DVB]: *** dma ready ***\n");

		if (pv != NULL)
			*((uint32_t *)pv) |= DVB_ASIC_DATA_READY;


		if (!pdvb->fdvbt) {
			pdvb->nframe++;
			if ((status >> 2) & 0x1)
				pdvb->nframeErr++;
			val32 = rReg32(0xf300);		/* FIFO_PTR */
			fifoptr = ((val32 >> 8) & 0xf);
			val32 = rReg32((0xf304 + fifoptr * 24));
			nmi_debug(_INFO_, "[DVB]: frame needed, (%08x)\n", val32);

			if ((val32 >> 4) & 0x1)
				pdvb->nfecNeed++;
		} 

		/* review: this is to work around link layer's memory cache issue.  Do a
				read to sync up */
		val32 = rReg32(0xd014);
		val32 ^= (0x1 << 3);
		rReg32(val32);

		/* handle wrapped around, if it happen */
		do {
			if (!pdvb->fdvbt) {
				/* dma to go */
				val32 = rReg32(0xf2b0);
				val32 |= (0x1 << 26);
				wReg32(0xf2b0, val32);

				/* wait for it is ready */
				retry = 100;
				do {
					val32 = rReg32(0xf2b0);
					if ((val32 >> 8) & 0x1)
						break;
				} while (retry--);	

				if (retry <= 0) {
					nmi_debug(_INFO_, "[DVB]: Error, no DMA Go not ready...\n");
				}
			}	

			/* read the count */
			tcnt = rReg32(0xd01c);		
			tadr = rReg32(0xd014);
			if (!pdvb->fdvbt) {
				val32 = rReg32(0xf2b0);
				tail = (val32 >> 4) & 0x3;
			} else {
				tail = 0;
			}
			nmi_debug(_INFO_, "[DVB]: intr, count(%d), address(%08x), tail(%08x)\n", tcnt, tadr, tail);

			nblk = tcnt/1024;
			nleft = tcnt%1024;

			if (nblk > 0) {
				if (nleft == 0) {
					nblk = tcnt/1023;
					nleft = tcnt%1023;
					dma.nblk = nblk;
					dma.szblk = 1023;
					dma.tail = 0;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
					/* delay loop */
					for (i=0; i< 10000;i++){ 
						if (i == 1000) 
							dummy = i;
					}
					tadr += nblk*1023;
					wReg32(0xd014, tadr);
					val32 = rReg32(0xd01c);
					val32 = rReg32(0xd014);
					//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", rReg32(0xd01c), rReg32(0xd014));
					dma.nblk = 1;
					dma.szblk = nleft;
					dma.tail = tail;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
				} else {
					dma.nblk = nblk;
					dma.szblk = 1024;
					dma.tail = 0;
					dma.last = 0;
					//wReg32(0xd01c, (nblk * 1024));					
					nmi_dma_read((void *)&dma);
					/* delay loop */
					for (i=0; i< 10000;i++){ 
						if ( i == 1000) 
							dummy = i;
					}

					//wReg32(0xd01c, nleft);
					tadr += nblk*1024;
					wReg32(0xd014, tadr);					
					val32 = rReg32(0xd01c);
					val32 = rReg32(0xd014);
					//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", rReg32(0xd01c), rReg32(0xd014));
					dma.nblk = 1;
					dma.szblk = nleft;
					dma.tail = tail;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
				} 
			}else {
				int blksz = nleft/2 + 1;

				dma.nblk = 1;
				dma.szblk = blksz;
				dma.tail = 0;
				dma.last = 0;
				nmi_dma_read((void *)&dma);

				/* delay loop */
				for (i=0; i< 10000;i++){ 
					if ( i == 1000) 
						dummy = i;
				}

				tadr += blksz;
				wReg32(0xd014, tadr);					
				val32 = rReg32(0xd01c);
				val32 = rReg32(0xd014);

				//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", rReg32(0xd01c), rReg32(0xd014));
				dma.nblk = 1;
				dma.szblk = nleft - blksz;				
				dma.tail = tail;
				dma.last = 0;
				nmi_dma_read((void *)&dma);
			}

#if 0
			if (nblk > 0) {
				if (nleft == 0) {
					if (nblk > 1) {
						dma.nblk = nblk-1;
						dma.szblk = 1024;
						dma.last = 0;
						dma.tail = 0;
						nmi_dma_read((void *)&dma);
						/* delay loop */
						for (i=0; i< 10000;i++){ 
							if ( i == 1000) 
								dummy = i;
						}
						tadr += (nblk-1)*1024;
						wReg32(0xd014, tadr);					
						tcnt = rReg32(0xd01c);
						tadr = rReg32(0xd014);
						//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", tcnt, tadr);
						dma.nblk = 1;
						dma.szblk = 1024;
						dma.last = 0;
						dma.tail = tail;
						nmi_dma_read((void *)&dma);
					} else {
						dma.nblk = 1;
						dma.szblk = 1020;
						dma.last = 0;
						dma.tail = 0;
						nmi_dma_read((void *)&dma);
						/* delay loop */
						for (i=0; i< 10000;i++){ 
							if ( i == 1000) 
								dummy = i;
						}
						tadr += 1020;
						wReg32(0xd014, tadr);					
						tcnt = rReg32(0xd01c);
						tadr = rReg32(0xd014);
						//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", tcnt, tadr);
						dma.nblk = 1;
						dma.szblk = 4;
						dma.last = 0;
						dma.adr = tadr;
						dma.tail = tail;
						nmi_dma_read((void *)&dma);
					}
				} else {
					dma.nblk = nblk;
					dma.szblk = 1024;
					dma.last = 0;
					dma.tail = 0;
					nmi_dma_read((void *)&dma);

					for (i=0; i< 10000;i++){ 
						if ( i == 1000) 
							dummy = i;
					}

					tadr += nblk*1024;
					wReg32(0xd014, tadr);					
					tcnt = rReg32(0xd01c);
					tadr = rReg32(0xd014);
					nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", tcnt, tadr);
					dma.nblk = 1;
					dma.szblk = nleft;
					dma.last = 0;
					dma.tail = tail;
					nmi_dma_read((void *)&dma);
				} 
			} else {
				//int blksz = nleft/2 + 1;
				if (nleft > 4) {
					int blksz = nleft - 4;

					//nmi_debug(_INFO_, "[DVB]: nleft (%d), blksz (%d)\n", nleft, blksz);
					dma.nblk = 1;
					dma.szblk = blksz;
					dma.last = 0;
					dma.tail = 0;
					nmi_dma_read((void *)&dma);
					/* delay loop */
					for (i=0; i< 10000;i++){ 
						if ( i == 1000) 
							dummy = i;
					}
					tadr += blksz;
					wReg32(0xd014, tadr);					
					tcnt = rReg32(0xd01c);
					tadr = rReg32(0xd014);
					nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", tcnt, tadr);
					dma.nblk = 1;
					dma.szblk = 4;
					dma.last = 0;
					dma.adr = tadr;
					dma.tail = tail;
					nmi_dma_read((void *)&dma);
				} else {
					dma.nblk = 1;
					dma.szblk = 4;
					dma.last = 0;
					dma.adr = tadr;
					dma.tail = tail;
					nmi_dma_read((void *)&dma);
				}
			}
#endif

			val32 = rReg32(0xf2bc);
			if ((val32 >> 4) & 0x1) {
				nmi_debug(_INFO_, "[DVB]: *** DMA Done ***\n");
				wReg32(0xf2bc, val32);
				break;
			}
			loopcount++;
			if (loopcount > 1) {
				nmi_debug(_INFO_, "[DVB]: Error, DMA never done...\n");
				break;
			}
		} while (1);
	}

	nmi_debug(_INFO_, "[DVB]: intr clear status (%08x)\n", rReg32(0xf2bc));
	return;	
}

static void dvb_handle_intr(void *pv)
{
	if (pchp->bustype == _SDIO_)
		dvb_handle_sdio_intr(pv);
	else if (pchp->bustype == _SPI_)
		dvb_handle_spi_intr(pv);
	else if (pchp->bustype == _EBI_)
		dvb_handle_ebi_intr(pv);
	else 
		dvb_poll_intr(pv);
}

static void dvb_intr_init(void)
{
	uint32_t val32;
	/* enable interrupt */
	if (pchp->bustype == _SPI_) {

		val32 = rReg32(0xe094);
		val32 |= 0x1;
		wReg32(0xe094, val32);

		val32 = rReg32(0xb100);
		val32 |= (0x1 << 1);
		wReg32(0xb100, val32);
	} else if (pchp->bustype == _EBI_) {
		val32 = rReg32(0xb100);
		val32 |= (0x1 << 1);
		wReg32(0xb100, val32);
	} 
}

/********************************************
	DVB Time Slice Functions
********************************************/
static uint32_t dvb_ts_delta_t(int iPid)
{
	uint32_t val32;

	if (iPid >= 8) {
		nmi_debug(_ERR_, "[DVB]: delta t, bad index (%d)\n", iPid);
		return 0;
	}

	val32 = rReg32((0xf3c4 + iPid * 4));
	val32 &= 0x1fff;	
	val32 *= 10;

	return val32;
}

static void dvb_ts_link_off(void *pv)
{
	uint32_t val32;

	//nmi_debug(_INFO_, "[DVB]: link off...start\n"); 

	val32 = rReg32(0xf200);
	val32 |= 0x1;
	wReg32(0xf200, val32);
	
	//Turn off the clocks to link layer
	val32 = rReg32(0xb410);
	nmi_debug(_INFO_, "0xb410 .1 (%08x)\n", val32); 
	val32 &= ~(0x1 << 17);
	wReg32(0xb410, val32);
	nmi_debug(_INFO_, "0xb410 .2 (%08x)\n", rReg32(0xb410)); 

	val32 = rReg32(0xb434);
	nmi_debug(_INFO_, "0xb434 .1 (%08x)\n", val32); 
	val32 ^= 0x1800000;
	wReg32(0xb434, val32); 
	//nmi_debug(_INFO_, "0xb434 .2 (%08x)\n", rReg32(0xb434)); 

	// Turn PLL OFF
#ifdef _CLOCKLESS_
	 val = 0x01;
	 clockless_i2c_write_reg(0xc000, (uint8_t *)&val, 1);
#endif

	//nmi_debug(_INFO_, "[DVB]: link off...end\n"); 
}

static void dvb_ts_link_on(void *pv)
{
	uint32_t val32;

	//nmi_debug(_INFO_, "[DVB]: link on...start\n"); 

	val32 = rReg32(0xb410);
	val32 |= (0x1 << 17);
	wReg32(0xb410, val32);
	nmi_debug(_INFO_, "0xb410 (%08x)\n", rReg32(0xb410)); 

	val32 = rReg32(0xb434);
	val32 |= 0x1800000;
	wReg32(0xb434, val32);
	//nmi_debug(_INFO_, "0xb434 (%08x)\n", rReg32(0xb434)); 

	//Turn on the clocks to link layer, XORs to be cleaned
	val32 = rReg32(0xf200);
	val32 &= ~0x1;
	wReg32(0xf200, val32);

	// clear old intr status
	wReg32(0xf2bc, 0xffffffff);
	//nmi_debug(_INFO_, "[DVB]: link on...end\n"); 
}

static void dvb_ts_wakeup(void *pv)
{
	uint8_t val8;
	uint32_t val32;
	int retry;

#ifdef _CLOCKLESS_
	// Turn PLL back ON
	val8 = 0x03;
	clockless_i2c_write_reg(0xc000, (uint8_t *)&val8, 1);
	Sleep(1);
#endif

	nmi_debug(_INFO_, "[DVB]: wake up --> (%d)\n", (int)nmi_get_tick()); 

	// Enable  periph clocks
	val32 = rReg32(0xb410); 		/* periph_clock_enables */
	val32 |= 0x2eb;
	wReg32(0xb410, val32);
	//nmi_debug(_INFO_, "0xb410 (%08x)\n", rReg32(0xb410)); 

	val32 = rReg32(0xb410);
	val32 |= (0x1 << 2);
	wReg32(0xb410, val32);
	//nmi_debug(_INFO_, "0xb410 (%08x)\n", rReg32(0xb410)); 

	// Turn ADC on
	val32 = rReg32(0xb404);
	val32 &= ~0x7e;
	val32 |= (0x1 << 7);
	val32 |= (0x1);
	wReg32(0xb404, val32); 
	//nmi_debug(_INFO_, "0xb404 (%08x)\n", rReg32(0xb404)); 

	//Enable Tuner
	dvbRfWrite(0x14, 0xff);
	dvbRfWrite(0x06, 0xe7);
	if (pdvb->frequency <= 300) 
		val8 = 0x5f;
	else if (pdvb->frequency <= 900) 
		val8 = 0x9f;
	else
		val8 = 0xdf;
	dvbRfWrite(0x00, val8);
	nmi_delay(4);
	dvbRfWrite(0x00, val8);
	nmi_delay(2);
	//nmi_debug(_INFO_, "[RF] 0x0 (%02x)\n", dvbRfRead(0x0)); 

	// Turn AGC back on 
	val32 = rReg32(0xb430);
	//nmi_debug(_INFO_, "0xb430 .1 (%08x)\n", rReg32(0xb430)); 
	val32 ^= 0x1;
	wReg32(0xb430, val32);
	//nmi_debug(_INFO_, "0xb430 .2 (%08x)\n", rReg32(0xb430)); 

	// Turn Raptor and jack on
	val32 = rReg32(0xb410);
	//val32 &= ~((0x1) | (0x1 << 1) | (0x1 << 3) | (0x1 << 4) | (0x1 << 5) | (0x1 << 8) | (0x1 << 9) | (0x1 << 10) | (0x1 << 11) | (0x1f << 12)); 
	val32 |= ((0x1) | (0x1 << 1) | (0x1 << 3) | (0x1 << 4) | (0x1 << 5) | (0x1 << 8) | (0x1 << 9) | (0x1 << 10) | (0x1 << 11) | (0x1f << 12)); 
	wReg32(0xb410, val32);
	//nmi_debug(_INFO_, "0xb410 (%08x)\n", rReg32(0xb410)); 

	val32 = rReg32(0xb434);
	val32 |= 0xE7FF7FB;
	wReg32(0xb434, val32);
	//nmi_debug(_INFO_, "0xb434 (%08x)\n", rReg32(0xb434)); 

	/* Reset Reed-Solomon Decoder */
	val8 = 0x0f;
	wReg8(0xf1c0, val8);
	val8 = 0x1f;
	wReg8(0xf1c0, val8);
	//nmi_debug(_INFO_, "0xf1c0 (%02x)\n", rReg8(0xf1c0)); 

	/*Re-enable raptor core*/
	val8 = 0x0;
	wReg8(0xf000, val8);
	val8 = 0x20;
	wReg8(0xf000, val8);

	//Resync Viterbi Decoder
	val8 = 0x82;
	wReg8(0xf180, val8);
	//nmi_debug(_INFO_, "0xf180 (%02x)\n", rReg8(0xf180)); 

	dvb_ts_link_on(NULL);

	val8 = 1;
	wReg8(0xf15f, val8);
	//nmi_debug(_INFO_, "0xf15f (%02x)\n", rReg8(0xf15f)); 

	nmi_delay(150);
	retry = 3;
	while(retry--) {		
		val8 = rReg8(0xf1c1);
		nmi_debug(_VERB_, "0xf1c1 (%02x), 0xf142(%02x)\n", val8, rReg8(0xf142)); 

		if ((val8 & 0x1) == 0x1) { /* Break if RS is locked  */
#if 0
			val8 = rReg8(0xf13f);
			val32 = val8 << 24;
			val8 = rReg8(0xf13e);
			val32 |= (val8 << 16);
			val8 = rReg8(0xf13d);
			val32 |= (val8 << 8);
			val8 = rReg8(0xf13c);
			val32 |= val8;
			nmi_debug(_INFO_, "PHY ACQ Time, t = %f\n", (((double)val32)/36670.));
			//nmi_debug(_INFO_, "0xf019 (%02x)\n", rReg8(0xf019));
#endif
			break; 			
		} else {
			nmi_debug(_INFO_, "*** Can't Lock ***\n"); 
			//dvb_soft_rst();			
			//nmi_delay(100);
			//val8 = rReg8(0xf001);
			//nmi_debug(_INFO_, "0xf001 (%02x)\n", rReg8(0xf001)); 
		}
	}
}

static void dvb_ts_shutdown(void *pv)
{
	uint8_t val8;
	uint32_t val32;

	/* store the mode and guard */
	pdvb->guard = dvb_get_syr_guard(); 
	pdvb->mode = dvb_get_syr_mode();

	//nmi_debug(_INFO_, "(rfagc) 0xf145 (%02x)\n", rReg8(0xf145)); 

	//Disable RFAGC.
	//nmi_debug(_INFO_, "0xf15f .1 (%02x)\n", rReg8(0xf15f));
	val8 = 0;
	wReg8(0xf15f, val8);
	//nmi_debug(_INFO_, "0xf15f .2 (%02x)\n", rReg8(0xf15f)); 

	//Return Viterbi Decoder to its initial state 
	val8 = rReg8(0xf180);
	//nmi_debug(_INFO_, "0xf180 .1 (%02x)\n", val8); 
	val8 ^= 0x4;
	wReg8(0xf180, val8);
	//nmi_debug(_INFO_, "0xf180 .2 (%02x)\n", rReg8(0xf180)); 

	val8 = 0x1f;
	wReg8(0xf184, val8);

	/* Reset Reed-Solomon Decoder */
	val8 = rReg8(0xf1c0);
	//nmi_debug(_INFO_, "0xf1c0 .1 (%02x)\n", val8); 
	val8 |= 0x0f;
	wReg8(0xf1c0, val8);
	//nmi_debug(_INFO_, "0xf1c0 .2 (%02x)\n", rReg8(0xf1c0)); 

	// Disable Raptor Core
	val8 = 0x00;
	wReg8(0xf000, val8);
		
	// Turn Raptor and jack off 
	val32 = rReg32(0xb410);
	//nmi_debug(_INFO_, "0xb410 .1 (%08x)\n", val32); 
	val32 &= ~((0x1) | (0x1 << 1) | (0x1 << 3) | (0x1 << 4) | (0x1 << 5) | (0x1 << 8) | (0x1 << 9) | (0x1 << 10) | (0x1 << 11) | (0x1f << 12)); 
	wReg32(0xb410, val32);  
	nmi_debug(_VERB_, "0xb410 .2 (%08x)\n", rReg32(0xb410)); 

	val32 = rReg32(0xb434);
	nmi_debug(_VERB_, "0xb434 .1 (%08x)\n", val32); 
	val32 ^= 0xE7FF7FB;
	wReg32(0xb434, val32);
	nmi_debug(_VERB_, "0xb434 .2 (%08x)\n", rReg32(0xb434)); 

	// Turn AGC off
	val32 = rReg32(0xb430);
	//nmi_debug(_INFO_, "0xb430 .1 (%08x)\n", val32); 
	val32 ^= 0x1;
	wReg32(0xb430, val32);
	//nmi_debug(_INFO_, "0xb430 .2 (%08x)\n", rReg32(0xb430)); 

	// Turn ADC off
	val32 = rReg32(0xb404);
	nmi_debug(_VERB_, "0xb404 .1 (%08x)\n", val32); 
	val32 &= ~(0x1 << 7);
	val32 &= ~(0x1);
	val32 |= 0x7e;
	wReg32(0xb404, val32); 
	nmi_debug(_VERB_, "0xb404 .2 (%08x)\n", rReg32(0xb404)); 

	dvbRfWrite(0x14, 0x7f);
	dvbRfWrite(0x6, 0xe8);
	dvbRfWrite(0x0, 0x5);
	//nmi_debug(_INFO_, "[RF] 0x0, (%02x)\n", dvbRfRead(0)); 

	// Disable clock to RFI2C
	val32 = rReg32(0xb410);
	//nmi_debug(_INFO_, "0xb410 .1 (%08x)\n", val32); 
	val32 &= ~(0x1 << 2);
	wReg32(0xb410, val32);
	//nmi_debug(_INFO_, "0xb410 .2 (%08x)\n", rReg32(0xb410)); 

	/* turn off LL layer */
	dvb_ts_link_off(NULL);

	return;
}

/********************************************
	DVB Init Functions
********************************************/
static void dvb_set_gio(void *pv)
{
	DVBGIOCTL *p = (DVBGIOCTL *)pv;
	uint32_t val32;

	if (p->bit > 15) {
		nmi_debug(_ERR_, "[DVB]: Error, only 15 gio pins, (%d)\n", p->bit);
		return;
	}

	/* set the function to normal mode */
	if (p->bit < 10) {
		val32 = rReg32(0xb400);
		if (((val32 >> (p->bit * 3)) & 0x7) != 0) {
			val32 &= ~(0x7 >> (p->bit * 3));
			wReg32(0xb400, val32); 
		}
	} else {
		val32 = rReg32(0xb43c);
		if (((val32 >> (p->bit * 3)) & 0x7) != 0) {
			val32 &= ~(0x7 >> (p->bit * 3));
			wReg32(0xb43c, val32); 
		}
	}
	
	/* set the direction */
	val32 = rReg32(0xb004);
	if (p->dir) {	/* output */
		if (!((val32 >> p->bit) & 0x1)) {
			val32 |= (1 << p->bit);
			wReg32(0xb004, val32);
		}

		/* write the value */
		val32 = rReg32(0xb000);
		if (p->level > 0)	
			val32 |= (1 << p->bit);
		else
			val32 &= ~(1 << p->bit);
		wReg32(0xb000, val32);

	} else {	/* input */
		if ((val32 >> p->bit) & 0x1) {
			val32 &= ~(1 << p->bit);
			wReg32(0xb004, val32);
		}

		/* read the value */
		val32 = rReg32(0xb050);
		p->level = (val32 >> p->bit) & 0x1;
	
	}

	return;
}

static uint32_t dvb_get_chipid(void) 
{
	uint32_t chipid, val32;
	/* get the chip id */
	chipid = rReg32(0xb468);
	chipid &= 0x00ffffff;

	val32 = rReg32(0xb474);
	val32 >>= 21;
	val32 &= 0x3ff;
	if (val32 == 0x3ff) {
		chipid &= 0xffffff00;
		chipid |= 0xc2;
	} else if (val32 == 0x3fe) {
		chipid &= 0xffffff00;
		chipid |= 0xd0;
	}

	return chipid;
}

static uint32_t dvb_get_formal_chipid(void) 
{
	uint32_t chipid;
	/* get the chip id */
	chipid = rReg32(0xb468);
	chipid &= 0x00ffffff;

	return chipid;
}

static int dvb_chip_init(void)
{
	uint32_t chipid;
	uint32_t tmp, ver, subver;
	int ix = pchp->crystalindex;
	uint32_t rb414 = dvbcrystaltbl[ix].regb414;

	memset((void *)pdvb, 0, sizeof(DVBPRIV));

	/* check the chip id */
	chipid = dvb_get_chipid();
	pdvb->chipid = chipid;

	if (!ISNM201 && !ISNM311 && !ISNM312) {
		nmi_debug(_ERR_, "Error: can't find chip id...(%08x)\n", chipid);
		printk(KERN_INFO"can't find chip id");
		return -1;
	}

	subver = chipid & 0x1;
	ver = chipid & 0xfffffffe;
	if ((ver == 0x201a0) || (ver == 0x201b0)) {
		//wReg32(0xb414, 0x25150132);
		if (subver)
			tmp = 0x10d;
		else
			tmp = 0x10c;
		wReg32(0xb418, tmp);
		//wReg32(0xb41c, 0x40200300);
		wReg32(0xb42c, 0);
		wReg32(0xb42c, 0x1000c000);
		nmi_delay(100);
	} else {

  		wReg32(0xb414, rb414);
		wReg32(0xb418, 0x10d);
		wReg32(0xb41c, 0x40200300);

		wReg32(0xb42c, 0);
		wReg32(0xb42c, 0x1000c000);		/* ykk */
		nmi_delay(1);
		nmi_debug(_INFO_, "0xb414(%08x)\n", rReg32(0xb414));

		/* review: this is C2 chip */
		if (ISNM312A0) {
#ifdef _HW_TIME_SLICE_
			if (pchp->chipver == 1)						/* C0 */
				wReg32(0xb464, 0x09ff0787);
			else if (pchp->chipver == 0)
				wReg32(0xb464, 0x09fc0000);		/* A0 */
#else
			if (pchp->chipver == 1)
				wReg32(0xb464, 0x09ff0000);		/* C0 */
			else if (pchp->chipver == 0)		
				wReg32(0xb464, 0x09fc0000);		/* A0 */
#endif
		} else { 
			wReg32(0xb464, 0x29ff0000);			/* C2, D0 */
		}
	}

	/* peripheral global reset */
	wReg32(0xb40c, 0xffffffff);

	/* peripheral clock enable */
#ifdef _HW_TIME_SLICE_
	wReg32(0xb410, 0xfffffcd4/*0xfffffcc0*/);
#else
	wReg32(0xb410, 0xfffffffc);		/* review: need to enable the last two bits for DVB-H table reading */
#endif
	wReg32(0xb434, 0xffffffff);

	if ((ver == 0x201a0) || (ver == 0x201b0)) {
		wReg32(0xf2c4, 0x201);
		wReg32(0xf121, 0x2);
		wReg32(0xf120, 0x1);
		wReg32(0xb44c, 0);
		wReg32(0xb464, 0x20000);
	} 

	if ((ver == 0x201a0) || (ver == 0x201b0)) {
		wReg32(0xb404, 0x929d81);
		wReg32(0xb408, 0x9016);
	} else {
		wReg32(0xb404, 0x920d81);
		wReg32(0xb408, 0x11000);
	}

	/* configure gio pin to output and drive them low */
	wReg32(0xb400, 0x00000000);		/* set to normal mode */
	wReg32(0xb004, 0xffffffff);
	wReg32(0xb000, 0x00000000);

#ifdef _EXT_LNA_
	/* select UHF band and turn the LNA on */
	tmp = (1 << 12);
	wReg32(0xb000, tmp);
#endif

	wReg32(0xf2c4, 0x71);
	wReg32(0xf120, 0xe1);

	/* RF I2C setup */
	wReg32(0xb36c, 0);
	wReg32(0xb300, 0x63);
	wReg32(0xb31c, 0x6);
	wReg32(0xb320, 0xd);
	wReg32(0xb304, 0x60);
	wReg32(0xb330, 0xaff);
	wReg32(0xb338, 0x1);
	wReg32(0xb33c, 0);
	wReg32(0xb36c, 0x1);

	/* set up tuner and demod */
	dvb_tuner_init();
	dvb_demod_init();

	dvb_lnk_init();
	dvb_intr_init();
	return 0;
}

static void dvb_set_agc_thold(void *pv)
{
	uint32_t val32;
	DVBAGCTHRESHOLD *p = (DVBAGCTHRESHOLD *)pv;

	val32 = (uint32_t)p->vgaupper;
	wReg8(0xf154, (uint8_t)(val32 & 0xff));		/* VGA upper threshold */
	wReg8(0xf155, (uint8_t)((val32 >> 8) & 0x3));

	val32 = (uint32_t)p->vgalower;
	wReg8(0xf156, (uint8_t)(val32 & 0xff));		/* VGA lower threshold */
	wReg8(0xf157, (uint8_t)((val32 >> 8) & 0x3));

	val32 = (uint32_t)p->rssiupper;
	wReg8(0xf158, (uint8_t)val32);						/* RSSI upper threshold */
	val32 = (uint32_t)p->rssilower;
	wReg8(0xf159, (uint8_t)val32);						/* RSSI lower threshold */
}

static NMIDTVVTBL *dvb_init(void)
{
	memset((void *)pdvb, 0, sizeof(DVBPRIV));
	
	if (dvb_chip_init() < 0)
		return NULL;

	/* set up vtable */
	pchp->dtv.nmi_config_tuner = dvb_run_tuner;
	pchp->dtv.nmi_config_demod = dvb_run_demod;
	pchp->dtv.nmi_config_mac = dvb_run_lnk;

	pchp->dtv.nmi_fec_lock = dvb_fec_lock;
	pchp->dtv.nmi_get_chipid = dvb_get_chipid;
	pchp->dtv.nmi_get_formal_chipid = dvb_get_formal_chipid;
	pchp->dtv.nmi_get_snr = dvb_get_snr;
	pchp->dtv.nmi_get_ber = dvb_get_ber;
	pchp->dtv.nmi_get_per = dvb_get_per;
	//pchp->dtv.nmi_track_agc = dvb_track_agc;
	pchp->dtv.nmi_rst_ber = dvb_rst_ber;
	pchp->dtv.nmi_rst_per = dvb_rst_per;
	pchp->dtv.nmi_rf_read_reg = dvbRfRead;
	pchp->dtv.nmi_rf_write_reg = dvbRfWrite;
	pchp->dtv.nmi_soft_reset = dvb_soft_rst;
	//pchp->dtv.nmi_fec_lock = dvb_chk_fec;
	pchp->dtv.nmi_get_dvb_mode = dvb_get_mode;
	pchp->dtv.nmi_get_dvb_sta = dvb_get_sta;
	pchp->dtv.nmi_handle_intr = dvb_handle_intr;
	pchp->dtv.nmi_dvb_rst_lnk = dvb_rst_lnk;
	pchp->dtv.nmi_dvb_rst_lnk_cnt = dvb_rst_lnk_cnt;
	pchp->dtv.nmi_dvb_get_lnk_sta = dvb_get_lnk_sta;
	pchp->dtv.nmi_dvb_track_alg = dvb_track_alg;
	pchp->dtv.nmi_dvb_get_ber_count = dvb_get_ber_count;
	pchp->dtv.nmi_dvb_get_ber_period = dvb_get_ber_period;
	pchp->dtv.nmi_dvb_get_per_count = dvb_get_per_count;
	pchp->dtv.nmi_dvb_get_per_period = dvb_get_per_period;
	pchp->dtv.nmi_dvb_band_scan = dvb_band_scan;	
	pchp->dtv.nmi_agc_lock = dvb_agc_lock;	
	pchp->dtv.nmi_syr_lock = dvb_syr_lock;	
	pchp->dtv.nmi_tps_lock = dvb_tps_lock;	
	pchp->dtv.nmi_chc_lock = dvb_chc_lock;	
	pchp->dtv.nmi_dvb_ts_shutdown = dvb_ts_shutdown;
	pchp->dtv.nmi_dvb_ts_wakeup = dvb_ts_wakeup;
	pchp->dtv.nmi_dvb_ts_delta_t = dvb_ts_delta_t;	
	pchp->dtv.nmi_dvb_set_gio = dvb_set_gio;
	pchp->dtv.nmi_dvb_get_psisi_tbl = dvb_get_psisi_tbl;

	pchp->dtv.nmi_dvb_set_pid = dvb_t_set_pid;
	pchp->dtv.nmi_dvb_add_pid = dvb_t_filter_add_pid;
	pchp->dtv.nmi_dvb_remove_pid = dvb_t_filter_remove_pid;

	pchp->dtv.nmi_dvb_get_sq = dvb_get_sq;
	pchp->dtv.nmi_dvb_set_agc_thold = dvb_set_agc_thold;

	return &pchp->dtv;
}

NMIDTVVTBL *nmi_common_init(NMICMN *pdrv)
{
	NMIDTVVTBL *ptv;

	memset((void *)pchp, 0, sizeof(NMICHIP));

	/* save the driver info */
	pchp->hlp.nmi_write_reg = pdrv->tbl.nmi_write_reg;
	pchp->hlp.nmi_read_reg = pdrv->tbl.nmi_read_reg;
	pchp->hlp.nmi_delay = pdrv->tbl.nmi_delay;
	pchp->hlp.nmi_get_tick = pdrv->tbl.nmi_get_tick;
	pchp->hlp.nmi_log = pdrv->tbl.nmi_log;
	pchp->hlp.nmi_dma_read = pdrv->tbl.nmi_dma_read;
	pchp->hlp.nmi_dvb_read_tbl = pdrv->tbl.nmi_dvb_read_tbl;

	pchp->dtvtype = pdrv->dtvtype;
	pchp->dbg = pdrv->dbgflag;
	pchp->bustype = pdrv->bustype;
	pchp->crystalindex = pdrv->crystalindex;
	pchp->chipver = pdrv->chipver;

	if (pchp->dtvtype == DVB) {
		ptv = dvb_init();
	} else {
		return ((void *)0);
	}

	return ptv;
}



