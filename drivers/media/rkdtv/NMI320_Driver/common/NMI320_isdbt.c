//#include "isdbt_lut.h"

#include "NMI320_nmidrv.h"
#include <stdarg.h>
//#include <stdio.h>
#include "NMI320_isdbt.h"

#include <linux/unistd.h>

#include <linux/delay.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>




#define INVERT_EXT_LNA


typedef struct {
	int nseg;
	int subchannel;
#ifdef _HAVE_FLOAT_
	double		aber_a;
	double		aber_b;
	long		bercnt;
	double		aper_a;
	double		aper_b;
	double		aberb4viterbi_a;
	double		aberb4viterbi_b;
	long		b4viterbibercnt_a;
	long		b4viterbibercnt_b;
#endif
	uint32_t freq;
	long channeltunedonce;
	long decoderconfigured;
	long channelnumber;
	long modulation_layerA;
	long coderate_layerA;
	long modulation_layerB;
	long coderate_layerB;
	long guardintervalrate;
	long previous_rssi;
	long previous_extlna_mode;
	long external_lna_control_counter;
	long avgVGAGain;
	long avgRSSI;
	long fec_lock_a;
	long fec_lock_b;
	long UHF_channelnum;
	long chipid;
	long rst_pidfilter_cnt;
	long preempt_agc_tracking;
	uint8_t		rfreg_14;
	uint32_t	bbreg_6468;
	uint32_t	channel_scan;
} ISDBTPRIV;

#ifdef _HAVE_FLOAT_
typedef struct {
	double dCrystalFreq;
	uint32_t	reg6460; 
} NMICRYSTALTBL;

static NMICRYSTALTBL crystalTbl[] = {
{12, ((3 << 28)|(63 << 18)|(30 << 8)|(32 << 1))},
{13, ((3 << 28)|(273 << 18)|(270 << 8)|(30 << 1))},
{19.2, ((9 << 28)|(21 << 18)|(0 << 8)|(60 << 1))},
{19.68, 0x9d74e53a/*((9 << 28)|(861 << 18)|(459 << 8)|(59 << 1))*/},
{19.8, ((9 << 28)|(693 << 18)|(619 << 8)|(59 << 1))},
{26, 0xf4450C97/*((6 << 28)|(273 << 18)|(270 << 8)|(30 << 1))*/},
{27, 0x38dd381c/*((6 << 28)|(567 << 18)|(58 << 8)|(28 << 1))*/},
{32, 0xf0540178}
};
#else
typedef struct {
	uint32_t	reg6460; 
} NMICRYSTALTBL;

static NMICRYSTALTBL crystalTbl[] = {
{((3 << 28)|(63 << 18)|(30 << 8)|(32 << 1))},						/* 12 MHz*/
{((3 << 28)|(273 << 18)|(270 << 8)|(30 << 1))},						/* 13 MHz*/
{((9 << 28)|(21 << 18)|(0 << 8)|(60 << 1))},						/* 19.2 MHz*/
{0x9d74e53a/*((9 << 28)|(861 << 18)|(459 << 8)|(59 << 1))*/},		/* 19.68 MHz*/
{((9 << 28)|(693 << 18)|(619 << 8)|(59 << 1))},						/* 19.8 MHz*/
{0xf4450C97/*((6 << 28)|(273 << 18)|(270 << 8)|(30 << 1))*/},		/* 26 MHz*/
{0x38dd381c/*((6 << 28)|(567 << 18)|(58 << 8)|(28 << 1))*/},		/* 27 MHz*/
{0xf0540178}														/* 32 MHz*/
};
#endif


#define ISNM320A0 ((((pisdbt->chipid) & 0xfffff) == 0x320a0) ? 1 : 0)
#define ISNM320C0 ((((pisdbt->chipid) & 0xfffff) == 0x320c0) ? 1 : (((pisdbt->chipid) & 0xfffff) == 0x320d0)?1:(((pisdbt->chipid) & 0xfffff) == 0x320d1)?1:0)
#define ISNM321A0 ((((pisdbt->chipid) & 0xfffff) == 0x321a0) ? 1 : (((pisdbt->chipid) & 0xfffff) == 0x321A1)?1:0)

//static unsigned char channelnumber = 0;

#ifndef _HAVE_FLOAT_
static int isdbt_snr[] = {
       50000 ,      50000,       43979,       40458,       37959,       36021,       34437,       33098,
       31938 ,      30915,       30000,       29172,       28416,       27721,       27077,       26478,
       25918 ,      25391,       24895,       24425,       23979,       23556,       23152,       22765,
       22396 ,      22041,       21701,       21373,       21057,       20752,       20458,       20173,
       19897 ,      19630,       19370,       19119,       18874,       18636,       18404,       18179,
       17959 ,      17744,       17535,       17331,       17131,       16936,       16745,       16558,
       16375 ,      16196,       16021,       15849,       15680,       15514,       15352,       15193,
       15036 ,      14883,       14731,       14583,       14437,       14293,       14152,       14013,
       13876 ,      13742,       13609,       13479,       13350,       13223,       13098,       12975,
       12853 ,      12734,       12615,       12499,       12384,       12270,       12158,       12047,
       11938 ,      11830,       11724,       11618,       11514,       11412,       11310,       11210,
       11110 ,      11012,       10915,       10819,       10724,       10630,       10537,       10446,
       10355 ,      10265,       10175,       10087,       10000,        9914,        9828,        9743,
        9659 ,       9576,        9494,        9412,        9332,        9251,        9172,        9094,
        9016 ,       8938,        8862,        8786,        8711,        8636,        8562,        8489,
        8416 ,       8344,        8273,        8202,        8132,        8062,        7993,        7924,
        7856 ,       7788,        7721,        7655,        7589,        7523,        7458,        7393,
        7329 ,       7266,        7202,        7140,        7077,        7016,        6954,        6893,
        6833 ,       6773,        6713,        6654,        6595,        6536,        6478,        6420,
        6363 ,       6306,        6250,        6193,        6138,        6082,        6027,        5972,
        5918 ,       5863,        5810,        5756,        5703,        5650,        5598,        5546,
        5494 ,       5442,        5391,        5340,        5289,        5239,        5189,        5139,
        5090 ,       5041,        4992,        4943,        4895,        4846,        4799,        4751,
        4704 ,       4657,        4610,        4563,        4517,        4471,        4425,        4379,
        4334 ,       4289,        4244,        4199,        4155,        4111,        4067,        4023,
        3979 ,       3936,        3893,        3850,        3807,        3765,        3723,        3681,
        3639 ,       3597,        3556,        3514,        3473,        3432,        3392,        3351,
        3311 ,       3271,        3231,        3191,        3152,        3112,        3073,        3034,
        2995 ,       2956,        2918,        2879,        2841,        2803,        2765,        2728,
        2690 ,       2653,        2616,        2579,        2542,        2505,        2468,        2432,
        2396 ,       2360,        2324,        2288,        2252,        2217,        2181,        2146,
        2111 ,       2076,        2041,        2007,        1972,        1938,        1903,        1869};
#endif

static FREQ_3SEG FreqTBL[] = {
	{188571	,188714	,188857	},
	{189000	,189143	,189285	},
	{189428	,189571	,189714	},
	{189857	,190000	,190142	},
	{190285	,190428	,190571	},
	{190714	,190857	,191000	},
	{191142	,191286	,191428	},
	{191571	,191714	,191857	},
	{194571 ,194714 ,194857 },
	{195000 ,195143 ,195285 },
	{195428 ,195571 ,195714 },
	{195857 ,196000 ,196142 },
	{196285 ,196428 ,196571 },
	{196714 ,196857 ,197000 },
	{197142 ,197286 ,197428 },
	{197571 ,197714 ,197857 }
};
static CHANNEL_3SEG ChannelTBL[] = {
	{4		,5		,6		},
	{7		,8		,9		},
	{10		,11		,12		},
	{13		,14		,15		},
	{16		,17		,18		},
	{19		,20		,21		},
	{22		,23		,24		},
	{25		,26		,27		},
	{18		,19		,20		},
	{21		,22		,23		},
	{24		,25		,26		},
	{27		,28		,29		},
	{30		,31		,32		},
	{33		,34		,35		},
	{36		,37		,38		},
	{39		,40		,41		}
};

//static void isdbt_config_decoder(void *p);
//static void isdbt_config_extLNA_GPIO(uint8_t lna_gain);

ISDBTPRIV isdbt;
ISDBTPRIV *pisdbt = &isdbt; 

extern NMICHIP chip;
extern NMICHIP *pchp ;//= &chip;


/********************************************
	Bus Read/Write Functions
********************************************/
EXPORT_SYMBOL(rReg8);
uint8_t rReg8(uint32_t adr)
{
	uint8_t val;

	if (pchp->hlp.nmi_read_reg != NULL) {
		pchp->hlp.nmi_read_reg(adr, (uint8_t *)&val, 1);
		return val;
	}

	return 0; 
}

EXPORT_SYMBOL(wReg8);
void wReg8(uint32_t adr, uint8_t val)
{
	if (pchp->hlp.nmi_write_reg != NULL) {
		pchp->hlp.nmi_write_reg(adr, (uint8_t *)&val, 1);
	}

	return; 
}

EXPORT_SYMBOL(rReg32);
uint32_t rReg32(uint32_t adr)
{
	uint32_t val;
	if (pchp->hlp.nmi_read_reg != NULL) {
		pchp->hlp.nmi_read_reg(adr, (uint8_t *)&val, 4);
		return val;
	}
	return 0; 
}

EXPORT_SYMBOL(wReg32);
void wReg32(uint32_t adr, uint32_t val)
{
	if (pchp->hlp.nmi_write_reg != NULL) {
		pchp->hlp.nmi_write_reg(adr, (uint8_t *)&val, 4);
	}
	return; 
}

//#define NMI_Bool_Write_log

#ifdef NMI_Bool_Write_log

MY_FILE *g_FileHwd =NULL;
char g_szlog[100*1024];
int g_szlogLen =0;

#endif


void NMI_WriteDibcomLog(unsigned char * szRow,int iLen)
{	
#ifdef NMI_Bool_Write_log    

	if(g_FileHwd ==NULL)
	{
		 g_FileHwd = FSFileOpen("D:\\Tunerlog.txt", "rb+");
	        if(g_FileHwd==NULL)
	        {
	            g_FileHwd = FSFileOpen("D:\\Tunerlog.txt", "wb");
	        }
		memset(g_szlog,0,102400);
		g_szlogLen =0;		
	}
	if(g_FileHwd ==NULL)
		return ;
	if((g_szlogLen +iLen)>102400)
	{
		FSFileSeek(g_FileHwd, 0, SEEK_END);
		FSFileWrite(g_szlog, g_szlogLen, g_FileHwd);
		memset(g_szlog,0,102400);
		g_szlogLen =0;
	}
	memcpy(&g_szlog[g_szlogLen],szRow,iLen);
	g_szlogLen +=iLen;

#endif    
	
}


void NMI_WriteDibcomLogClose(void)
{
#ifdef NMI_Bool_Write_log      
		if(g_FileHwd ==NULL)
			return ;
		FSFileSeek(g_FileHwd, 0, SEEK_END);
		FSFileWrite(g_szlog, g_szlogLen, g_FileHwd);
		FSFileClose(g_FileHwd);
		g_FileHwd =NULL;
		memset(g_szlog,0,102400);
		g_szlogLen =0;		
#endif    		
}

void nmi_log(char *str)
{
	uint32_t ilen;
	ilen =strlen(str);
	NMI_WriteDibcomLog((unsigned char *)str,ilen);
}

uint32_t nmi_get_tick(void)
{
    uint32_t     ticks = 0;
	
#if 1	
    struct timespec cur_time;  
    long time_ms = 0;

    do_gettimeofday(&cur_time);
	time_ms = cur_time.tv_sec*1000 + ((cur_time.tv_nsec / 1000)/1000);
	ticks = (uint32_t)time_ms;	
#else	
	ticks = jiffies;
#endif

    return (ticks);
}

/********************************************
	Debug Functions
********************************************/
void nmi_debug(uint32_t flag, char *fmt,...)
{
	char buf[256];
	va_list args;
	int len;

	if (flag & pchp->dbg) { 
		va_start(args, fmt);
#ifdef _WINXP_KERNEL_
		RtlStringCbVPrintfA(buf, 256, fmt, args);
#else
		len = vsprintf(buf, fmt, args);
#endif
		va_end(args);

		if (pchp->hlp.nmi_log) {
			nmi_log(buf);
		}
	}

	return;
}

void nmi_delay(uint32_t msec)
{
	//DelayMs_nops(msec);
	//mdelay(msec);
	msleep(msec);
}

/********************************************
	ISDBT RF Functions
********************************************/
void isdbtRfburstWrite(uint32_t startadr, uint8_t *val,uint8_t cnt)
{
	int retry,i;
	uint32_t reg;

	wReg32(0x6500, 5);
	wReg32(0x6504, startadr);
	for(i=0;i<(cnt);i++) {
		wReg32(0x6504, *((uint8_t *)val+i));
	}
	wReg32(0x6508, 0x1);

	retry = 300;
	do {
		reg = rReg32(0x6508);
		if ((reg & 0x1) == 0)
			break;
	} while (retry--);
	
	reg = rReg32(0x650c);
	if (!(reg & 0x1)) {
		nmi_debug(_ERR_, "RF write reg: Fail, startadr (%02x), val (%02x)\n", startadr, val);
	}

	return;
}

/*
	This function is to write registers in the tuner. T
	his function implements the RFI2C protocol for RF register writes defined by NMI
	adr:Address of the RF register to be written
	val:Data to be written in the specified register
*/	

void isdbtRfWrite(uint32_t adr, uint8_t val)
{
	int retry;
	uint32_t reg;

	wReg32(0x6500, 2);
	wReg32(0x6504, adr);
	wReg32(0x6504, val);
	wReg32(0x6508, 0x1);

	retry = 100;
	do {
		reg = rReg32(0x6508);
		if ((reg & 0x1) == 0)
			break;
	} while (retry--);
	
	reg = rReg32(0x650c);
	if (!(reg & 0x1)) {
		nmi_debug(_ERR_, "RF write reg: Fail, addr (%02x), val (%02x)\n", adr, val);
	}

	return;
}

/*
	This function is to read a register from the tuner. 
	This function implements the RFI2C protocol for RF register reads defined by NMI
	adr:Address of the RF register to be read

	return :uint8_t value of the tuner register specified
*/
uint8_t isdbtRfRead(uint32_t adr)
{
	int retry;
	uint32_t reg;
	
	/* write register address */
	wReg32(0x6310, adr);

	retry = 100;
	/* wait for STOP_DET_INTR */
	do {
		reg = rReg32(0x632c);
	} while (!(reg & 0x200) && retry--);

	if (retry <= 0) {
		nmi_debug(_ERR_, "RF Read reg: Fail, wait STOP DET intr, (%08x)\r\n", reg);
		return 0;
	}

	/* clear interrupt */
	rReg32(0x6340);

	/* write 0x100 */
	wReg32(0x6310, 0x100);

	retry = 100;
	/* wait for STOP_DET_INTR */
	do {
		reg = rReg32(0x632c);		
	} while (!(reg & 0x200) && retry--);

	if (retry <= 0) {
		nmi_debug(_ERR_, "RF Read reg: Fail, wait STOP DET intr, (%08x)\r\n", reg);
		return 0;
	}

	/* read data */
	reg = rReg32(0x6310);

	/* clear interrupt */
	rReg32(0x6340);		

	return (uint8_t)reg;
}

unsigned char isdbt_get_channel_number(long freq,unsigned char channel_select)
{
	unsigned int	i;
	unsigned long	max3SegChannel;
	unsigned char	channelNum ;
	PFREQ_3SEG		freq3Seg;
	PCHANNEL_3SEG	channel3Seg;
	channelNum = 0;
	max3SegChannel = sizeof(FreqTBL)/sizeof(FREQ_3SEG);
	freq3Seg = (PFREQ_3SEG)&FreqTBL;
	channel3Seg = (PCHANNEL_3SEG)&ChannelTBL;
	for(i=0;i<max3SegChannel;i++)
	{
		switch(channel_select)
		{
		case 0:
			if(freq3Seg->SideFreq1 == freq)
				channelNum = channel3Seg->SideChannel1;
			break;
		case 1:
			if(freq3Seg->CentreFreq == freq)
				channelNum = channel3Seg->CenterChannel;
			break;
		case 2:
			if(freq3Seg->SideFreq2 == freq)
				channelNum = channel3Seg->SideChannel2;
			break;
		default:
			break;
		}
		if(channelNum != 0)
		{
			break;
		}
		else
		{
			freq3Seg++;
			channel3Seg++;
		}
	}
	if(channelNum == 0) {
		channelNum = 21;
	}
	return channelNum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////Function: freq_conversion_fp_Hz//////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Author: Ahmed Ragab																								  //
//Last modified date: 8/2/2007																					      //
//Input arguments:																									  //	
//		dFreq :the input freq in Hz																					  //	
//		IntegerPart : the returned integer part																		  //	
//		FractionalPart : the returned fractional part																  //	
//Function description :																							  //
//		According to the input frequency range , a certain factor is multiplied by the input frequency.				  //		
//		This factor is 16/26,8/26,4/26,2/26 for frequencies <= 200000000 , 250000000 ,900000000 and else respectively.// 
//		It also divides the input frequency by 1000000. The problem is to make these two divisions in 32 bit          //  
//      registers without losing much percision.																	  //	
//		The two division are made on two steps: division by 13*125*125. then shifting by the remaining factor		  //	
//		for example if input_freq <= 250000000, so it'll be multiplied by 8/26/1000000 which is simplified to		  //	
//      1/(13*125*125)*8/2*1/64 = 1/(13*125*125)*1/16. This is done by division by (13*125*125) then shifting by 4    //
//      to the right.
//		version 0.2
//					- Supporting Crystal Frequencies 12,13,19.2,19.68,19.8,24,26,27 Mhz
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void freq_conversion_fp(uint32_t dFreq,uint32_t * IntegerPart,uint32_t * FractionalPart)
{

	uint32_t shift,shiftIndex=0;
	uint32_t fixed_point_num;

	uint32_t binary_factor;
	uint32_t k;

	uint32_t fractional_part_mask = (1<<24) - 1;
	uint8_t rounding_carry;

	switch(pchp->CrystalIndex)
	{
	case _12_MHz:
		{
			binary_factor = 0xB2F4FC08;	
			break;
		}
	case _13_MHz:
		{
			binary_factor = 0xA530E8A5;			
			break;
		}
	case _19_2_MHz:
		{
			binary_factor = 0xDFB23B09;			
			break;
		}
	case _19_68_MHz:
		{
			binary_factor = 0xDA3D7E48;			
			break;
		}
	case _19_8_MHz:
		{
			binary_factor = 0xD8EAE3EA;			
			break;
		}
	case _26_MHz:
		{
			binary_factor = 0xA530E8A5;			
			break;
		}
	case _27_MHz:
		{
			binary_factor = 0x9F12A723;			
			break;
		}
	case _32_MHz:
		{
			binary_factor = 0x8637BD06;
			shiftIndex = 1;
			break;
		}

	}

	if (dFreq <= 200000000)
		shift = 3;	   //factor = 16/26 = 8 * 1/13. 8 means a shift of 3 and we have a shift of -6 
	//(1/1000000 = 1/64 * 1/125*1/125).At the end we have a shift of 18 to the right (because the first one of 
	//the binary_factor -mentioned bellow- comes at the 18th place) and because the fraction part is needed in 
	//24 bits so we make a shift of 3 initially (shift = 24 -18 + 3 - 6).
	else if (dFreq <= 250000000)
		shift = 2;	   //factor = 8/26
	else if (dFreq <= 900000000)
		shift = 1;	   //factor = 4/26
	else
		shift = 0;	   //factor = 2/26

	dFreq <<= (shift+shiftIndex);

	fixed_point_num = 0;

	//binary_factor = 1/(13*125*125) = [0000 0000 0000 0000 0] 1010 0101 0011 0000 1110 1000 1010 0101 (bin) then
	//we throw the first 18 zeros away and consider it when taking the fractional part at the end. 
	//So finally binary_factor becomes 1010 0101 0011 0000 1110 1000 1010 0101 (bin)

	for (k = 0 ; k < 31 ; k++)
	{
		if ((binary_factor >> k) & 0x1) 
		{
			if ((dFreq >> (31-k-1)) & 0x1)
				rounding_carry = 1;
			else
				rounding_carry = 0;

			fixed_point_num = fixed_point_num + (dFreq >> (31-k)) + rounding_carry;
		}
	}

	fixed_point_num += dFreq;

	*IntegerPart = fixed_point_num >> 24;
	*FractionalPart = fixed_point_num & fractional_part_mask;
}

/******************************************************************************
Function: LUT

Description: This function eleminates look up table.

Return: None

Author: A.Nour

Version: 0.1

Date: 30-July-07'
******************************************************************************/
/*!Description: This function eleminates look up table.*/

void LUT(uint32_t mode,uint32_t guardtype,uint32_t numsegments,uint32_t modulationA,uint32_t ratetypeA,uint32_t modulationB,uint32_t ratetypeB)
{
	int tsBuff_1[4]={0},tsBuff_0[4]={0},
	tsIdx_1=0,useBuffer_0_S3 = 0,useBuffer_0_S4,hBufferB=0,tspTime=0,nc=0,kth_clk=0,FS=0,
	tmp,ntotalCLKs,kth_clk_all,hBufferA =0,tsIdx_0 = 0,rateANum = 0 , rateADeNum = 0,rateBNum = 0 , rateBDeNum = 0,
	iteration = 0,count = 0,TwoBitRead=0;
	uint32_t WriteWord = 0;
	int OutPutValue = 0,padding=0;
	/*Remapping mode and num. of seg from chip values to algorithmic values*/
	numsegments = (numsegments==0)?1:3;
	mode +=2;
	if(numsegments == 1)
	{
		FS = 1024;
		tspTime = 816;
	}
	else 
	{
		FS = 2048;
		tspTime = 408;
	}


	
	ntotalCLKs = ( FS/(1 << (3-mode)) ) + (( FS/(1 << (3-mode) ) ) >> (5-guardtype));

	nc = 96*( 1<<(mode-1) );

	if(ratetypeA == 0)
	{
		rateANum   = 1;
		rateADeNum = 2;
	}
		
	else 
	{
		rateANum   = 2;
		rateADeNum = 3;
	}
	
	if(ratetypeB == 0)
	{
		rateBNum   = 1;
		rateBDeNum = 2;
	}
	else 
	{
		rateBNum   = 2;
		rateBDeNum = 3;
	}

	for(kth_clk_all = 0;kth_clk_all<=(ntotalCLKs*204-1);kth_clk_all++)
	{
		kth_clk =  kth_clk_all%ntotalCLKs ;
		
		if(mode == 2)
		{
			useBuffer_0_S3=1-(int)(kth_clk_all/102/ntotalCLKs);
			useBuffer_0_S4=1-(int)(kth_clk_all/102/ntotalCLKs);
		}
		else
		{
			useBuffer_0_S3=1-(int)(kth_clk_all/51/ntotalCLKs)%2;
			useBuffer_0_S4=1-(int)(kth_clk_all/51/ntotalCLKs)%2;
		}
		if (kth_clk < nc)
		{
			tmp=2*( (int)( (kth_clk+1)*2*modulationA*rateANum/rateADeNum) - (int)( (kth_clk+0)*2*modulationA*rateANum/rateADeNum) );
			hBufferA = hBufferA + tmp;
			if((hBufferA)>=(204*8*2))
			{
				hBufferA=hBufferA-(204*8*2);
				if (useBuffer_0_S3>0)
				{
					tsIdx_0 ++;
					tsBuff_0[tsIdx_0]=1;
				}          
				else
				{
					tsIdx_1++;
					tsBuff_1[tsIdx_1]=1;
				}
                

			}
		}
		if(numsegments == 3)
		{
			if (kth_clk>=nc && kth_clk<(3*nc))
			{
				tmp=2*( (int)((kth_clk+1)*2*modulationB*rateBNum/rateBDeNum)- (int)( (kth_clk+0)*2*modulationB*rateBNum/rateBDeNum) );
				hBufferB+=tmp;
				if((hBufferB)>=(204*8*2))
				{
					hBufferB=hBufferB-204*8*2;
					if (useBuffer_0_S3>0)
					{
						tsIdx_0++;
						tsBuff_0[tsIdx_0]=2;
					}               

					else
					{
						tsIdx_1++;
						tsBuff_1[tsIdx_1]=2;
					}      

				}
			}


		}
		if ( ( (kth_clk_all+1)%tspTime) == 0)
		{
			if (useBuffer_0_S4>0)
			{
				if (tsIdx_0==0)
				{
					OutPutValue = 0;
				}
				else
				{
					
					OutPutValue = tsBuff_0[1];
					tsBuff_0[1] = tsBuff_0[2];
					tsBuff_0[2] = tsBuff_0[3];
					tsBuff_0[3] = 0;					
					tsIdx_0--;
				}
				
			}
			else
			{
				if (tsIdx_1 == 0)
				{

					OutPutValue = 0;
				}
				
				else
				{

					OutPutValue = tsBuff_1[1];
					tsBuff_1[1] = tsBuff_1[2];
					tsBuff_1[2] = tsBuff_1[3];
					tsBuff_1[3] = 0;					
					tsIdx_1--;
				}
			}

			
				TwoBitRead = ( (OutPutValue&3) ) << (2*iteration);
				WriteWord = WriteWord | TwoBitRead ;
				iteration ++;
				if(iteration == 16)
				{
					
					//cout<<"["<<count<<"] = "<<hex<<setfill('0')<<setw(8)<<(int)WriteWord<<endl ;
				//	fileName<<"["<<count<<"] = "<<hex<<setfill('0')<<setw(8)<<(int)WriteWord<<endl ;
					//wReg32(0xde00+(count*4),(uint8_t*)&WriteWord);
					wReg32((0xde00 + (count * 4)), WriteWord);
					count++;
					WriteWord = 0;
					iteration = 0;
				}
		} 
	}
	for(padding=count;padding<80;padding++)
	{
		WriteWord = 0;
		wReg32(0xde00+(padding*4),WriteWord);

	}

}

/*
	This function programs the tuner with the given frequency. 
	pv :Pointer to ISDBTTUNE data structure containing the frequency we want to tune the tuner.
*/

int isdbt_config_tuner(void *pv)
{
	ISDBTTUNE *p = (ISDBTTUNE *)pv;
	uint8_t 	val;
	uint32_t	dfreq = p->freq/1000;							/*in KHz */
	//uint64_t	target;
	uint32_t	N0, alpha;
	uint32_t	val32;
	//uint8_t 	i,j;
#ifndef _FRQUENCY_CONVERSION_
   	double target;
	double refclk1;
	int ix = pchp->CrystalIndex;
	double refclk = crystalTbl[ix].dCrystalFreq;
#endif
	if(ISNM321A0 && (p->freq < 200000000))
		return 0;
	if((pchp->BoardType == _EVAL_BOARD_) || (pchp->BoardType == _USB_SDIO_DONGLE_)) {
		/* Viswa on Feb12 2008 driving the GPIO 14 to calculate the timing for TS output.*/
		val32 = rReg32(0x6000);
		val32 |= (1 << 14);
		wReg32(0x6000,val32);
	}

	pisdbt->freq = p->freq/1000;
	if(p->subchannel == 0) {
		pisdbt->subchannel = isdbt_get_channel_number(dfreq,1);
		p->subchannel = pisdbt->subchannel;
	}
	else {
		pisdbt->subchannel = p->subchannel;
	}
	// calculating the UHF channel number.
	if(pisdbt->freq > 200000) {
		pisdbt->UHF_channelnum = 13 + (pisdbt->freq - 473143)/6000;
	}
	else {
		pisdbt->UHF_channelnum = 0;
	}
	if((dfreq >= 194571) && (dfreq <= 197857))	{			/* VHF channel 8*/
		pisdbt->channelnumber = 8;
		if(pisdbt->nseg == 1) {
			if(ISNM320A0) {
				if((dfreq == 194714) && (pchp->BoardType == _USB_DONGLE_ || pchp->BoardType == _USB_SDIO_DONGLE_)) {
					dfreq = dfreq - 500;
				}
				else {
					dfreq = dfreq + 500;
				}
			}
			else {
				dfreq = dfreq + 500;
			}
		}
		else {
			if(ISNM320A0) {
				if(dfreq == 194714) {
					dfreq = dfreq - 1000;
				}
				else {
					dfreq = dfreq + 1000;
				}
			}
			else {
				dfreq = dfreq + 1000;
			}
		}
	}
	else if((dfreq <= 191857) && (dfreq >= 188571)) {		/* VHF channel 7*/
		pisdbt->channelnumber = 7;
		if(pisdbt->nseg == 1) {
			dfreq = dfreq - 500;
		}
		else {
			dfreq = dfreq - 1000;
		}
	}
	else {													/* UHF */
		if(pisdbt->nseg == 1) {
			dfreq = dfreq - 500;
		}
		else {
			dfreq = dfreq - 1000;
		}
		pisdbt->subchannel = 21;
		p->subchannel = pisdbt->subchannel;
		pisdbt->channelnumber = 0;
	}

	nmi_debug(_INFO_,"dfreq:[%d] subchannel:[%d]\n",pisdbt->freq,pisdbt->subchannel);
	if(ISNM320A0 || ISNM320C0 || (!pisdbt->channeltunedonce)) {
		if ((dfreq) <= 200000) {								// VHF
			val = 0x5f;
		} else {
			val = 0x9f;
		}
		isdbtRfWrite(0x00, val);
	}

#ifndef _FRQUENCY_CONVERSION_
		if (refclk < 19)
		refclk1 = refclk * 2;
	else if(refclk == 32)
		refclk1 = refclk/2;
	else 
		refclk1 = refclk;

	if (dfreq <= 200000) {
   		target = (dfreq * 16)/(refclk1 * 1000)/*26.0*/;
  	} else if (dfreq <= 250000) {
      	target = (dfreq * 8)/(refclk1 * 1000)/*26.0*/;
  	} else if (dfreq <= 900000) {
      	target = (dfreq * 4)/(refclk1 * 1000)/*26.0*/;
  	} else {
      target = (dfreq *2)/(refclk1 * 1000)/*26.0*/;
	}

	N0 = (long)(target); 
   	val = (uint8_t)N0;
	
   	isdbtRfWrite(0x01, val);
   	alpha = (long)((target - N0)*(0x1000000));

   	val = (uint8_t)(alpha & 0xff);
   	isdbtRfWrite(0x02, val);
   	val = (uint8_t)((alpha >> 8) & 0xff);
   	isdbtRfWrite(0x03, val);
   	val = (uint8_t)((alpha >> 16) & 0xff);
   	isdbtRfWrite(0x04, val);
#else

	/*
	if ((dfreq/1000) <= 200) {
		target = (dfreq * 173215370283); 
		target >>= 24;
	} else if ((dfreq/1000) <= 250) {
		target = (dfreq * 86607685142);
		target >>= 24;
	} else if ((dfreq/1000) <= 900) {
		target = (dfreq * 43303842571);
		target >>= 24;
	} else {
		target = (dfreq * 21651921285);
		target >>= 24;
	}*/

	//N0 = (long)(target)  & 0xff000000; 
	freq_conversion_fp(dfreq*1000,&N0,&alpha);
	val = (uint8_t)(N0);
	val32 = val; 
   	//isdbtRfWrite(0x01, val);


	//alpha = (long)(target) & 0x00ffffff;
   	val = (uint8_t)(alpha & 0xff);
	val32 |= (val << 8);
   	//isdbtRfWrite(0x02, val);
   	val = (uint8_t)((alpha >> 8) & 0xff);
	val32 |= (val << 16);
   	//isdbtRfWrite(0x03, val);
   	val = (uint8_t)((alpha >> 16) & 0xff);
	val32 |= (val << 24);
	isdbtRfburstWrite(0x01,(uint8_t *)&val32,4);
   	//isdbtRfWrite(0x04, val);
#endif

	if (ISNM320A0) {
		if (pisdbt->nseg == 3) {
			if (dfreq <= 200000) { // VHF
				val = 0xb5;
			} else {
				val = 0x95;
			}	  
		} else {
			if (dfreq <= 200000) { // VHF
				val = 0xb4;	
			} else {
				val = 0x94;
			}
		}
	} else if (ISNM321A0 || ISNM320C0) {
		if(pchp->CrystalIndex == 7) {				/* 32 MHz*/
			val = 0x94;
		}
		else {
			if(dfreq < 200000) {
				if(pisdbt->nseg == 3)
					val = 0xB1;
				else
					val = 0xB0;
			}
			else
				val = 0x90;	/* C0 */
		}
	}
	if(ISNM320A0 || ISNM320C0 || (!pisdbt->channeltunedonce))
		isdbtRfWrite(0x05, val);

	if(ISNM320A0 || ISNM320C0 || (!pisdbt->channeltunedonce)) {
		if (pisdbt->nseg == 3) 
			val = 0x3f;
		else	
			val = 0x3f;
		isdbtRfWrite(0x17, val);
		
		if (ISNM320A0) {
			if(pisdbt->channelnumber == 8) {
				if((pisdbt->freq == 194714) && (pchp->BoardType == _USB_DONGLE_ || pchp->BoardType == _USB_SDIO_DONGLE_))
					isdbtRfWrite(0x1b,0x83);
				else
					isdbtRfWrite(0x1b, 0x87);
			}
			else
				isdbtRfWrite(0x1b, 0x83);
		} else if (ISNM321A0 || ISNM320C0) {
			if(pisdbt->channelnumber == 8)
				isdbtRfWrite(0x1b,0x87);
			else
				isdbtRfWrite(0x1b, 0x83);	/* C0 */
		}
	}

	// Viswa on Sept25 2007 for new USB Dongle with external LNA configuring the GPIO register controlling the External LNA.
	if(pchp->BoardType == _USB_DONGLE_) {
		if ((dfreq) <= 200000) {								// VHF
			if(pisdbt->channel_scan) {
				isdbt_config_extLNA_GPIO(_VHF_HIGH_GAIN_);
				pisdbt->previous_extlna_mode = 1;
			}
			else {
				isdbt_config_extLNA_GPIO(_VHF_LOW_GAIN_);
				pisdbt->previous_extlna_mode = 0;
			}
		} else {
			if(pisdbt->channel_scan) {
				isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
				pisdbt->previous_extlna_mode = 1;
			}
			else {
				isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
				pisdbt->previous_extlna_mode = 0;
			}
		}
	}
	else if(pchp->BoardType == _YUAN_DONGLE_) {
		if ((dfreq) <= 200000) {								// VHF
			isdbt_config_extLNA_GPIO(_VHF_LOW_GAIN_);
		} else if(dfreq >= 473143 && dfreq <= 665143){
			isdbt_config_extLNA_GPIO(_CH13_45_UHF_LOW_GAIN_);
		} else if(dfreq >= 671143 && dfreq <= 767143) {
			isdbt_config_extLNA_GPIO(_CH46_62_UHF_LOW_GAIN_);
		}
		pisdbt->previous_extlna_mode = 0;
		//pisdbt->external_lna_control_counter = 1;
	}
	// viswa on Nov 13 2007 temperature setting for reg 0x14 given by Trang.
	switch(pisdbt->UHF_channelnum) {
	case 0:
		//isdbtRfWrite(0x14,0x7C);
		val = 0x7C;
		break;
	case 13:
		//isdbtRfWrite(0x14,0x7F);
		val = 0x7F;
		break;
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
		val = 0x7E;
		//isdbtRfWrite(0x14,0x7E);
		break;
	case 19:
	case 20:
	case 21:
	case 22:
		val = 0x7D;
		//isdbtRfWrite(0x14,0x7D);
		break;
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
		val = 0x7C;
		//isdbtRfWrite(0x14,0x7C);
		break;
	default:
		val = 0x7B;
		//isdbtRfWrite(0x14,0x7B);
		break;
	}
	if(val != pisdbt->rfreg_14) {
		isdbtRfWrite(0x14,val);
		pisdbt->rfreg_14 = val;
	}
	/* grouping all register writes that are common for all frequencies*/
	if(pisdbt->channeltunedonce == 0)
	{
		isdbtRfWrite(0x06, 0x70);
		isdbtRfWrite(0x07, 0xbf);
		isdbtRfWrite(0x08, 0xc3);
		isdbtRfWrite(0x09, 0xd0);
		// change to enable synthesizer to relock due to temperature variance
		//isdbtRfWrite(0x0a, 0x4e);
		isdbtRfWrite(0x0a, 0x5e);
		/* Viswa on Dec17 2007 modified the value of RF register 0x0b for C0 to 0x5A (Trang Alphs board) but could be in all C0*/
		if(ISNM321A0 || ISNM320C0) {
			/* Viswa on Feb11 2008 instead of crystal frequency we use board type to modify this register value 0x5a to 0x59*/
			if((pchp->BoardType == _EVAL_BOARD_)||(pchp->BoardType == _USB_SDIO_DONGLE_ || ISNM321A0))
				isdbtRfWrite(0x0b,0x59);
			else
				isdbtRfWrite(0x0b, 0x5a);
		}
		else {
			isdbtRfWrite(0x0b, 0xdc);
		}
		/* Viswa on Feb11 2008 changed the register setting for 0x0c from 0x90 to 0x40*/
		isdbtRfWrite(0x0c, 0x40);
		//Bias calibration changes
		isdbtRfWrite(0x0d, 0x40);
		isdbtRfWrite(0x0e, 0x4B);
		isdbtRfWrite(0x0f, 0x18);
		isdbtRfWrite(0x10, 0x16);
		
		isdbtRfWrite(0x11, 0x01);
		isdbtRfWrite(0x12, 0x00);
		isdbtRfWrite(0x13, 0x53);
		//isdbtRfWrite(0x14, 0x7c);
		isdbtRfWrite(0x15, 0x41);
		isdbtRfWrite(0x16, 0x25);
		
		//if (pisdbt->nseg == 3) 
		//	val = 0x3f;
		//else	
		//	val = 0x3e;
		//isdbtRfWrite(0x17, val);
		
		isdbtRfWrite(0x18, 0x80);
		isdbtRfWrite(0x19, 0x04);
		/* Viswa on Jan04 2008 modified the value of RF register 0x1a for C0 from 0x80 to 0x40*/
		if(ISNM321A0 || ISNM320C0) {
			// on Mar 07 2008 Trang modified this register value to 0x80 for all clock frequency.
			isdbtRfWrite(0x1a,0x80);
		}
		else
			isdbtRfWrite(0x1a, 0x40);
		//if(pisdbt->channelnumber == 8)
		//	isdbtRfWrite(0x1b, 0x87);
		//else
		//	isdbtRfWrite(0x1b, 0x83);
		isdbtRfWrite(0x1c, 0x94);
		isdbtRfWrite(0x1d, 0x02);
		isdbtRfWrite(0x1e, 0x40);
		isdbtRfWrite(0x1f, 0x47);
		isdbtRfWrite(0x20, 0x00);
		isdbtRfWrite(0x21, 0x4a);
		isdbtRfWrite(0x22, 0x00);
		isdbtRfWrite(0x23, 0x00);
		isdbtRfWrite(0x24, 0x00);
		isdbtRfWrite(0x25, 0x36);
		isdbtRfWrite(0x26, 0x7e);
		isdbtRfWrite(0x27, 0x00);
		isdbtRfWrite(0x28, 0x00);
		// viswa on Jan04 2008 modified the value of RF register 0x29 for NM320C0 & NM321A0 from 0x03 to 0x83
		if(ISNM321A0 || ISNM320C0) {
			isdbtRfWrite(0x29, 0x83);
		}
		else {
			isdbtRfWrite(0x29, 0x03);
		}
		
		if(ISNM321A0 || ISNM320C0) {						/* C0 */
			//isdbtRfWrite(0x2A,0x80);
			isdbtRfWrite(0x2b,0x40);		
		}
		
		isdbtRfWrite(0x2c, 0xa1);
		isdbtRfWrite(0x2d, 0xcb);
		
		if (ISNM320A0) {
			isdbtRfWrite(0x2e, 0xdc);
		} else if (ISNM321A0 || ISNM320C0) {
			isdbtRfWrite(0x2e, 0xde);	/* C0 */
		}
		
		// Bias calibration changes
		isdbtRfWrite(0x2f, 0x00);
		isdbtRfWrite(0x30, 0x00);
		
		isdbtRfWrite(0x31, 0xcb);
		
		isdbtRfWrite(0x32, 0x10);
		isdbtRfWrite(0x33, 0x40);
	}
    return 1;
}

/********************************************
	ISDBT Functions
********************************************/
/*
	This function is to select either 1-segment or 3-segment in the ASIC driver. 
	Normally this function will be called before configuring the tuner and demodulator.
	seg:Segment type, 1 for 1-segment and 3 for 3-segment
*/
void isdbt_set_segment(int seg)
{
	//nmi_debug(_INFO_, "Segment (%d)\n", seg);
	pisdbt->nseg = seg;
	return;	
}

void isdbt_software_reset(void)
{
	//nmi_debug(_INFO_,"soft reset \n");
#if 0
	/* Viswa on Nov28 2007 added the reset of pll clock setting as suggested by Trang*/
	if(ISNM321A0 || ISNM320C0) {
		wReg32(0x6418,0xC000);
		wReg32(0x6418,0x8000);
	}
#endif
	wReg8(0xa400, 0x0);
	wReg8(0xa400, 0x20);
}
/*
	This function is to get the current SNR value.
	pv: Pointer to ISDBTSNR data structure
*/

void isdbt_get_snr(void *pv)
{
#ifdef _HAVE_FLOAT_
	ISDBTSNR *p = (ISDBTSNR *)pv;
  	uint8_t usnr;

	usnr = rReg8(0xa494);
	if (usnr != 0) {
		p->dmp_snr = log10(usnr) * (-20.)+ 50.;
	}
	else {
		p->dmp_snr = 0;
	}

	usnr = rReg8(0xa474);
	if (usnr != 0) {
		p->snr_a = log10(usnr) * (-20.) + 50.;
	}
	else {
		p->snr_a = 0.;
	}

	usnr = rReg8(0xa475);
	if (usnr != 0) {
		p->snr_b = log10(usnr) * (-20.) + 50.;
	}
	else
	{
		p->snr_b = 0.;
	}
	/* review: need to find the snr B */
#else
	ISDBTSNR *p = (ISDBTSNR *)pv;
	uint8_t usnr;
	int snr;

	usnr = rReg8(0xa494);
	snr = isdbt_snr[usnr];
	p->dmp_snr = snr;

	usnr = rReg8(0xa474);
	snr = isdbt_snr[usnr];
	p->snr_a = snr;

	usnr = rReg8(0xa475);
	snr = isdbt_snr[usnr];
	p->snr_b = snr;
	/* review: */

#endif
	return;
}

/*
	This function is to get the current BER value.
	pv :Pointer to ISDBTBER data structure
*/	
void isdbt_get_ber(void *pv)
{
#ifdef _HAVE_FLOAT_
	ISDBTBER *p = (ISDBTBER *)pv;
	uint32_t 	uber;
  	uint32_t 	denom;
	double 	dber;
	//uint8_t		val;
	uint32_t		val;

	/* get the A ber */
	val = (uint8_t)rReg32(0xaba8);
	uber = val;
	val = (uint8_t)rReg32(0xabac);
	uber |= (val << 8);
	val = (uint8_t)rReg32(0xabb0);
	uber |= (val << 16);

	//nmi_debug(_INFO_, "BER: (%08x)\n", uber);

	denom = rReg32(0xaba4);
	denom &= 0x7;
	denom = (1 << denom) * 8 * 1024;

	dber = (double)uber/(double)denom ;	
	pisdbt->aber_a = (pisdbt->aber_a * pisdbt->bercnt + dber)/(pisdbt->bercnt + 1);

	/* get the B ber */
	val = (uint8_t)rReg32(0xabe8);
	uber = val;
	val = (uint8_t)rReg32(0xabec);
	uber |= (val << 8);
	val = (uint8_t)rReg32(0xabf0);
	uber |= (val << 16);

	denom = rReg32(0xabe4);
	denom &= 0x7;
	denom = (1 << denom) * 8 * 1024;

	dber = (double)uber/(double)denom ;	
	pisdbt->aber_b = (pisdbt->aber_b * pisdbt->bercnt + dber)/(pisdbt->bercnt + 1);

	/* increment count */
	pisdbt->bercnt++;


	/* return the value */
	p->ber_a = pisdbt->aber_a;
	p->ber_b = pisdbt->aber_b;
#else

#endif

	return;
}

void isdbt_get_ber_b4viterbi(void *pv)
{
#ifdef _HAVE_FLOAT_
	ISDBTBER_B4VITERBI *p = (ISDBTBER_B4VITERBI *)pv;
	uint32_t uber;
	uint32_t total_no_bits;
	uint32_t val32;
	uint8_t complete_ber;
	double dber;
	//layer A
	// reading the COR_STAT register.
	val32	= rReg32(0xab10);
	complete_ber = (uint8_t)(val32 & 0x8);
	p->overflow_bera = (uint8_t)(val32 & 0x10);
	// clearing the COR_STAT register;
	wReg32(0xab10,0xFFFFFFFF);
	// calculating the bits in error.
	val32	= rReg32(0xab2c);
	uber	= val32 << 8;
	val32	= rReg32(0xab30);
	uber	= uber | (val32 & 0xFF);
	// calculating the total number of bits.
	val32			= rReg32(0xab20);
	total_no_bits	= val32 << 16;
	val32			= rReg32(0xab24);
	total_no_bits	|= val32 << 8;
	val32			= rReg32(0xab28);
	total_no_bits	|= (val32 & 0xFF);
	total_no_bits	= total_no_bits * 256;
	if(complete_ber && !p->overflow_bera) {
		//wReg32(0xab10,0xFFFFFFFF);
		dber	= (double)uber/(double)total_no_bits;
		pisdbt->aberb4viterbi_a = (pisdbt->aberb4viterbi_a * pisdbt->b4viterbibercnt_a + dber) / (pisdbt->b4viterbibercnt_a + 1);
		pisdbt->b4viterbibercnt_a++;
	}

	nmi_debug(_INFO_,"overflow [%d] uber[%d] complete_ber[%d]\n",p->overflow_bera,uber,complete_ber);
	// layer B
	// reading the COR_STAT register.
	val32	= rReg32(0xab50);
	complete_ber = (uint8_t)(val32 & 0x8);
	p->overflow_berb = (uint8_t)(val32 & 0x10);
	// clearing the COR_STAT register;

	wReg32(0xab50,0xFFFFFFFF);
	// calculating the bits in error.
	val32	= rReg32(0xab6c);
	uber	= val32 << 8;
	val32	= rReg32(0xab70);
	uber	= uber | (val32 & 0xFF);
	// calculating the total number of bits.
	val32			= rReg32(0xab60);
	total_no_bits	= val32 << 16;
	val32			= rReg32(0xab64);
	total_no_bits	|= val32 << 8;
	val32			= rReg32(0xab68);
	total_no_bits	|= (val32 & 0xFF);
	total_no_bits	= total_no_bits * 256;
	if(complete_ber && !p->overflow_berb) {
		dber	= (double)uber/(double)total_no_bits;
		pisdbt->aberb4viterbi_b = (pisdbt->aberb4viterbi_b * pisdbt->b4viterbibercnt_b + dber) / (pisdbt->b4viterbibercnt_b + 1);
		pisdbt->b4viterbibercnt_b++;
	}

	p->ber_a = pisdbt->aberb4viterbi_a;
	p->ber_b = pisdbt->aberb4viterbi_b;
#else
#endif
}

void isdbt_set_new_bertimer(void *pv)
{
	uint32_t val32;
	ISDBT_BERTIMER *p = (ISDBT_BERTIMER *)pv;
	wReg32(0xab00,0x0);
	//layer A
	val32 = ((p->bertimer_a >> 16) & 0x3F);
	wReg32(0xab20,val32);
	val32 = ((p->bertimer_a >> 8) & 0xFF);
	wReg32(0xab24,val32);
	val32 = (p->bertimer_a & 0xFF);
	wReg32(0xab28,val32);
	wReg32(0xab00,0x2);
	//layer B
	wReg32(0xab40,0x0);
	val32 = ((p->bertimer_b >> 16) & 0x3F);
	wReg32(0xab60,val32);
	val32 = ((p->bertimer_b >> 8) & 0xFF);
	wReg32(0xab64,val32);
	val32 = (p->bertimer_b & 0xFF);
	wReg32(0xab68,val32);
	wReg32(0xab40,0x2);
}
/*
	This function is to get the current PER value.
	pv :Pointer to ISDBTPER data structure
*/	
void isdbt_get_per(void *pv)
{
#ifdef _HAVE_FLOAT_
	ISDBTPER *p = (ISDBTPER *)pv;
	uint32_t 	val;
	uint8_t 	winsize;
	uint32_t 	packets;
	uint32_t 	packetsInError;
	double 	dper;
	double 	forgetting = 0.96;

	/* calculate PER A */
	val = rReg32(0xa830);
	//nmi_debug(_INFO_, "PER: (%08x)\n", val);

	winsize = (uint8_t)(val & 0xf);
	if (winsize == 0) 
		winsize = 16;
	packets = (1 << winsize) - 1;
	packetsInError = (val >> 16) & 0xFFFF;
	dper = (double)packetsInError / (double)packets;
	pisdbt->aper_a = forgetting * pisdbt->aper_a + (1 - forgetting) * dper;
	/* calculate PER B */
	val = rReg32(0xa834);
	winsize = (uint8_t)(val & 0xf);
	if (winsize == 0) 
		winsize = 16;
	packets = (1 << winsize) - 1;
	packetsInError = (val >> 16) & 0xFFFF;
	dper = (double)packetsInError / (double)packets;
	pisdbt->aper_b = forgetting * pisdbt->aper_b + (1 - forgetting) * dper;

	/* return the value */
	p->per_a  = pisdbt->aper_a;
	p->per_b = pisdbt->aper_b;
#else


#endif

	return;
}

void isdbt_get_channel_length(void *p)
{
#ifdef _HAVE_FLOAT_
	ISDBTCHLEN *channel_length = (ISDBTCHLEN *)p;
	uint8_t mode;
	uint8_t delay_spread;
	delay_spread	= rReg8(0xA49A);
	mode			= (((rReg8(0xA412)) >> 2) & 0x1);
	if(mode) {	// mode == 3
		channel_length->channelLength = (double)(((double)delay_spread/256) * (1008/3));
	}
	else {		// mode == 2
		channel_length->channelLength = (double)(((double)delay_spread/256) * (504/3));
	}
#else

#endif
	return;
}
/*
	This function is to check whether TMCC is locked or not
	This function doesn't have any parameter


	return :int type value specifying the TMCC lock.
*/	
int isdbt_check_tmc_lock(void)
{
	uint8_t val;
	int lock = 0;
	//uint32_t val32;

	val = rReg8(0xa401);
	nmi_debug(_INFO_, "TMC Lock (%02x)\n", val);
	// Viswa Aug 02 07 bit 7(chptracking) is used instead of bit 6(tmcclocked) to say whether a channel is available on this frequency.
	if ((val >> 7) & 0x1) {
		lock = 1;
		/* Viswa on Feb12 2008 driving the GPIO 14 to calculate the timing for TS output.*/
		//val32 = rReg32(0x6000);
		//val32 |= (1 << 14);
		//wReg32(0x6000,val32);
	} 
	return lock;
}
/*
	This function is to check whether FEC is locked or not(state machine is in locked state).
	pv:Pointer to the data structure ISDBTFEC
*/	
void isdbt_check_fec_lock(void *pv)
{
	ISDBTFEC *p = (ISDBTFEC *)pv;
	uint32_t val32;

	val32 = rReg32(0xa80c);
	//nmi_debug(_INFO_, "FEC Lock (%08x)\n", val32);

	if (((val32 >> 20) & 0x3) == 2) {
		p->fec_lock_a = 1;
		pisdbt->fec_lock_a = 1;
	} else {
		p->fec_lock_a = 0;
		pisdbt->fec_lock_a = 0;
	}

	if (((val32 >> 22) & 0x3) == 2) {
		p->fec_lock_b = 1;
		pisdbt->fec_lock_b = 1;
	} else {
		p->fec_lock_b = 0;
		pisdbt->fec_lock_b = 0;
	}

	return;

}

void isdbt_check_soft_reset(void)
{
	uint8_t val;
	uint32_t val32;
	uint32_t per_a,per_b,per_cons;
	int lock = 0;
	int issue_reset = 0;

	if(ISNM320A0) {
		val = rReg8(0xa401);
		if ((val >> 7) & 0x1){
			lock = 1;
		}
		//check with kevin..
		if(lock) {
			if(pisdbt->nseg == 1) {
				per_a = rReg32(0xa830);
				per_cons = ((per_a >> 5) & 0xFF);
				if(pisdbt->modulation_layerA == 0x1) {			//QPSK
					if(pisdbt->coderate_layerA == 0x0) {			// 1/2 coderate
						switch(pisdbt->guardintervalrate)
						{
						case 1:										// 1/16 GuardInterval	
							if(per_cons > 110) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						case 2:										// 1/8 GuardInterval
							if(per_cons > 104) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						case 3:										// 1/4 GuardInterval
							if(per_cons > 94) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						default:
							if(per_cons == 255) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						}
					}
					else if(pisdbt->coderate_layerA == 0x1) {		// 2/3 code rate	
						switch(pisdbt->guardintervalrate)
						{
						case 1:										// 1/16 GuardInterval	
							if(per_cons > 147) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						case 2:										// 1/8 GuardInterval
							if(per_cons > 138) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						case 3:										// 1/4 GuardInterval
							if(per_cons > 125) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						default:
							if(per_cons == 255) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						}
					}
				}
				else if(pisdbt->modulation_layerA == 0x2) {		// 16QAM
					if(pisdbt->coderate_layerA == 0x0) {		// 1/2 code rate
						switch(pisdbt->guardintervalrate)
						{
						case 1:										// 1/16 GuardInterval	
							if(per_cons > 220) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						case 2:										// 1/8 GuardInterval
							if(per_cons > 208) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						case 3:										// 1/4 GuardInterval
							if(per_cons > 188) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						default:
							if(per_cons == 255) {
								issue_reset = 1;
								//isdbt_software_reset();
								//nmi_delay(600);
							}
							break;
						}
					}
					else {
						if(per_cons == 255) {
							issue_reset = 1;
							//isdbt_software_reset();
							//nmi_delay(600);
						}
					}
				}
			}
			else if(pisdbt->nseg == 3) {
				per_b = rReg32(0xa834);
				if(per_b > 50) {
					issue_reset = 1;
					//isdbt_software_reset();
					//nmi_delay(600);
				}
			}
		}
		else {
			val = rReg8(0xa470);
			if(val == 255) {
				issue_reset = 1;
				//isdbt_software_reset();
				//nmi_delay(600);
			}
		}
		if(issue_reset) {
			// viswa on Jan08 2008 for Bug 557 misalignment of 188 bytes at weak signal.
			// disable the TS output 
			val32 = rReg32(0xa804);
			val32 &= ~0x1;
			wReg32(0xa804,val32);
			// issue reset
			isdbt_software_reset();
			nmi_delay(600);
			// enable the TS output
			val32 = rReg32(0xa804);
			val32 |= 0x1;
			wReg32(0xa804,val32);
			nmi_debug(_INFO_,"****Issued soft reset****\n");
		}
		val = rReg8(0xa401);
		if ((val >> 7) & 0x1){
			lock = 1;
			if(!pisdbt->decoderconfigured) {
				isdbt_config_decoder(NULL);
			}
		}
	}
	else {				// if C0/321A0
		val = rReg8(0xa401);
		if ((val >> 7) & 0x1){
			lock = 1;
			if(!pisdbt->decoderconfigured) {
				isdbt_config_decoder(NULL);
			}
		}
		if(((val & 0xF) == 0x06) && ((rReg8(0xa462) & 0x07) == 0x0)) {
			isdbt_software_reset();
		}
	}
	return;
}

/*
	This function configures the demodulator for given frequency and subchannel number.
	pv :Pointer to ISDBTTUNE data structure containing the frequency and its corresponding subchannel number.
*/
int isdbt_config_demod(void *pv) 
{
	ISDBTTUNE *p = (ISDBTTUNE *)pv;
	uint8_t val;
	uint32_t val32;
	int ix = pchp->CrystalIndex;

	if(ISNM321A0 && (p->freq < 200000000))
		return 0;

	if(p->subchannel == 0) {
		pisdbt->subchannel = isdbt_get_channel_number(pisdbt->freq,1);
		p->subchannel = pisdbt->subchannel;
	}
	else {
		pisdbt->subchannel = p->subchannel;
	}

	if(((pisdbt->channelnumber == 7) || (pisdbt->channelnumber == 8)) && (pisdbt->freq != 195143) && (ISNM321A0 || ISNM320C0)) {
			val32 = 0x472000C0;
			//wReg32(0x6468,val32);
	}
	else if(pisdbt->freq == 190857) {
		if(ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200FC0;
		}
		//wReg32(0x6468,val32);
	}
	else if(pisdbt->freq == 191286) {
		if(ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200FC0;
		}
		//wReg32(0x6468,val32);
	}
	else if(pisdbt->freq == 194714) {
		if(ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200040;
		}
		//wReg32(0x6468,val32);
	}
	else if(pisdbt->freq == 195143) {
		if(ISNM320C0) {
			val32 = 0x47200FC0;
		}
		else {
			val32 = 0x44200FC0;
		}
		//wReg32(0x6468,val32);
	}
	else if(pisdbt->freq == 195571) {
		if(ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200040;
		}
		//wReg32(0x6468,val32);
	}
	else if(pisdbt->freq == 196000) {
		if(ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200040;
		}
		//wReg32(0x6468,val32);
	}
	/* viswa on Feb05 2008 since the input frequency is +- 30KHz from the center frequency we have this change*/
	else if((pisdbt->freq >= 521113) && (pisdbt->freq <= 521173)) {
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200540;
		}
		//wReg32(0x6468, val32);
	}
	else if((pisdbt->freq >= 569113) && (pisdbt->freq <= 569173))	{
		if(ISNM321A0 || ISNM320C0) {
			if(pchp->CrystalIndex == 7)
				val32 = 0x47200AC0;
			else
				val32 = 0x47200fC0;
		}
		else {
			val32 = 0x44200500;
		}
		//wReg32(0x6468, val32);
	}
	else if((pisdbt->freq >= 581113) && (pisdbt->freq <= 581173)) {
		if(ISNM321A0 || ISNM320C0) {
			if(pchp->CrystalIndex == 7)
				val32 = 0x47200AC0;
			else 
				val32 = 0x472003C0;
		}
		else {
			val32 = 0x44200100;
		}
		//wReg32(0x6468,val32);
	}
	else if((pisdbt->freq >= 593113) && (pisdbt->freq <= 593173))
	{
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x472005C0;
		}
		else  {
			val32 = 0x44200FC0;
		}
		//wReg32(0x6468, val32);
	}
	else if((pisdbt->freq >= 605113) && (pisdbt->freq <= 605173)) {
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x472003C0;
		}
		else {
			val32 = 0x44200100;
		}
		//wReg32(0x6468,val32);
	}
	else if((pisdbt->freq >= 617113) && (pisdbt->freq <= 617173))
	{
		if(ISNM321A0 || ISNM320C0) {
			if(pchp->CrystalIndex == 7)
				val32 = 0x47200040;
			else 
				val32 = 0x47200FC0;
		}
		else {
			val32 = 0x44200040;
		}
		//wReg32(0x6468, val32);
	}
	else if((pisdbt->freq >= 635113) && (pisdbt->freq <= 635173))
	{
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200440;
		}
		//wReg32(0x6468, val32);
	}
	else if((pisdbt->freq >= 641113) && (pisdbt->freq <= 641173)) {
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x47200080;
		}
		else {
			val32 = 0x44200100;
		}
		//wReg32(0x6468,val32);
	}
	else if((pisdbt->freq >= 665113) && (pisdbt->freq <= 665173)) {
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x47200080;
		}
		else {
			val32 = 0x44200100;
		}
	}
	else if((pisdbt->freq >= 683113) && (pisdbt->freq <= 683173))
	{
		if(ISNM321A0 || ISNM320C0) {
			if(pchp->CrystalIndex == 7)
				val32 = 0x47200080;
			else 
				val32 = 0x47200FC0;
		}
		else {
			val32 = 0x442003C0;
		}
		//wReg32(0x6468, val32);
	}
	else if((pisdbt->freq >= 707113) && (pisdbt->freq <= 707173)) {
		if(ISNM321A0 || ISNM320C0) {
			if(pchp->CrystalIndex == 7)
				val32 = 0x47200AC0;
			else 
				val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200100;
		}
		//wReg32(0x6468,val32);
	}
	else if((pisdbt->freq >= 731113) && (pisdbt->freq <= 731173))
	{
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x47200FC0;
		}
		else {
			val32 = 0x44200FC0;
		}
		//wReg32(0x6468,val32);
	}
	else if((pisdbt->freq >= 749113) && (pisdbt->freq <= 749173))
	{
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200040;
		}
		//wReg32(0x6468, val32);
	}
	else
	{
		if(ISNM321A0 || ISNM320C0) {
			val32 = 0x472000C0;
		}
		else {
			val32 = 0x44200100;
		}
		//wReg32(0x6468, val32);
	}
	if(!pisdbt->channel_scan) {
		if(pisdbt->bbreg_6468 != val32) {
			wReg32(0x6468, val32);
			pisdbt->bbreg_6468 = val32;
			if(ISNM321A0 || ISNM320C0) {
				// on Nov26 2007 changed the following value to register 0x6468 from 0x442001C0 to 0x04200042
				//val32 = 0x47200100;
				//wReg32(0x6468,val32);
				// on Nov26 2007 Trang asked specifically to repeat the following writes to register 0x6418 twice and hence so.
				val32 = 0xc000;
				wReg32(0x6418, val32);
				nmi_delay(1);
				val32 = 0x8000;
				wReg32(0x6418, val32);
			}
			val32 = 0xc000;
			wReg32(0x6418, val32);
			nmi_delay(1);
			val32 = 0x8000;
			wReg32(0x6418, val32);
		}
	}

	/* CHC algorithm */
	if(!pisdbt->channeltunedonce || ISNM320C0 || ISNM320A0)
		wReg8(0xa48a, (uint8_t)pisdbt->subchannel);

	/* grouping all register writes that are common for all frequencies*/
	if(pisdbt->channeltunedonce == 0)
	{
		// Viswa on Sept20 2007 modified the AGC_TARGET register setting from 0x16 to 0x8 (Trang request)
		if(ISNM321A0 || ISNM320C0) {
			/* 
				viswa on Dec10 2007 commented the following if condition to program the agc target value for C0 
				based on the input frequency since it is same across all frequencies.
			*/
			if((pchp->BoardType == _USB_DONGLE_) & (ISNM321A0)) {
				wReg8(0xa408,0xE);
				//wReg8(0xa408,0x1b);
			}
			else {
				wReg8(0xa408,0x8);
			}
		}
		else {
			if(pchp->BoardType == _USB_DONGLE_ || pchp->BoardType == _YUAN_DONGLE_) {
				wReg8(0xa408, 0x8);
			}
			else {
				wReg8(0xa408, 0x16);
			}
		}
		wReg8(0xa48b, 0x2);
		wReg8(0xa440, 0x10);
		// good up to 110Hz (112 Hz is ok)
		wReg8(0xa457, 0x2);
		wReg8(0xa458, 0x10);
		wReg8(0xa459, 0xe);
		wReg8(0xa45a, 0x2);
		wReg8(0xa45b, 0x10);
		wReg8(0xa45c, 0xe);
		wReg8(0xa47c, 0x5F);
		wReg8(0xa489, 0x5);
		wReg8(0xa496, 0x26);
		wReg8(0xa48c, 0xff);
		wReg8(0xa48d, 0x0);
		wReg8(0xa444, 0x3f);
		/* Viswa on Dec13 2007 added the code for enabling the automatic reset*/
		if(ISNM321A0 || ISNM320C0) {
			wReg8(0xa435,0x10);
			val = rReg8(0xa437);
			wReg8(0xa437,(uint8_t)(val | 0x30));
		}
		/* C0 */
		if (ISNM321A0 || ISNM320C0) {
			wReg8(0xa4c6, 0x1d);
			wReg8(0xa44f, 0x1);
		}
		if(ISNM321A0) {
			val = rReg8(0xa404);
			wReg8(0xa404, (uint8_t)(val & ~0x20));
			wReg8(0xa44c, 0x71);
			wReg8(0xa44d, 0x40);
		}
	}

	if(ISNM320A0) {
		if(pisdbt->channelnumber == 8){
#ifdef _HAVE_FLOAT1_
			if((pisdbt->freq == 194.714) && (pchp->BoardType == _USB_DONGLE_ || pchp->BoardType == _USB_SDIO_DONGLE_)) 
#else
				if((pisdbt->freq == 194714) && (pchp->BoardType == _USB_DONGLE_ || pchp->BoardType == _USB_SDIO_DONGLE_)) 
#endif
				{
					wReg8(0xa40c,0x40);
					wReg8(0xa40d,0x30);
				}
				else {
					wReg8(0xa40c,0xbf);
					wReg8(0xa40d,0xf);
				}
		}
		else {
			wReg8(0xa40c,0x40);
			wReg8(0xa40d,0x30);
		}
	}
	else {
		if(pisdbt->channelnumber == 8) {
			wReg8(0xa40c,0xbf);
			wReg8(0xa40d,0xf);
		}
		else if(!pisdbt->channeltunedonce || ISNM320A0 || ISNM320C0){
			wReg8(0xa40c,0x40);
			wReg8(0xa40d,0x30);
		}
	}

	// 3 seg only
	if(ISNM320A0 || ISNM320C0) {
		if (pisdbt->nseg == 3) {
			val = rReg8(0xa404);
			wReg8(0xa404, (uint8_t)(val | 0x20));
			wReg8(0xa44c, 0xe3);
			wReg8(0xa44d, 0x50);
		} else {
			val = rReg8(0xa404);
			wReg8(0xa404, (uint8_t)(val & ~0x20));
			wReg8(0xa44c, 0x71);
			wReg8(0xa44d, 0x40);
		}
	}

	wReg8(0xa400, 0x0);
	wReg8(0xa400, 0x20);

	pisdbt->channeltunedonce = 1;
	return 1;
}
/*
	his function program the decoder.
	p :Pass NULL; Currently we don't require any parameter for this function. We defined a void pointer for future use.
*/
void isdbt_config_decoder(void *p)
{
	uint8_t		val8;
	uint32_t	val32;
 	uint8_t 	val;
	uint32_t 	mode, guardtype, modulationA, ratetypeA, temp;
	uint32_t 	modulationB, ratetypeB, nsegs, TPS_Quarter;
	uint32_t 	ab_part;

	val = rReg8(0xa412);
	mode = (val >> 2) & 0x1;
	guardtype =  val & 0x3;
	pisdbt->guardintervalrate = guardtype;
	val = rReg8(0xa462);
	modulationA = val & 0x07;
	pisdbt->modulation_layerA = modulationA;
	ratetypeA = (val >> 4) & 0x7;
	pisdbt->coderate_layerA = ratetypeA;
	val = rReg8(0xa404);
	nsegs = (val >> 5) & 0x01;

	if(modulationA == 0) {
		nmi_debug(_ERR_,"modulation A is zero\n");
		return;
	}

	val = rReg8(0xa464);
	modulationB = val & 0x07;
	ratetypeB = (val >> 4) & 0x07;
	if(modulationB == 0) {
		nmi_debug(_ERR_,"modulationB is zero\n");
		return;
	}
	if (nsegs == 0) {	/* one segment */
		// One segment formula
		//switch_index=(mode) + (guardtype-1)*2 + (modulationA-1)*8 + ratetypeA*32;

		if (modulationA == 1) {
			wReg8(0xa48c, 0);
			wReg8(0xa48d, 0x1);
		} else if (modulationA == 2) {
			wReg8(0xa48c, 0x80);
			wReg8(0xa48d, 0x0);
		} else {
			wReg8(0xa48c, 0x40);
			wReg8(0xa48d, 0x0);
		}

	} else { /* three segment */
		uint32_t mod;


		// Original Statement
		//switch_index = 64 + mode + (guardtype-1)*2 + (modulationA-1)*8 + ratetypeA*32 + (modulationB-1)*64 + ratetypeB*256;

		// Modified statement
		//switch_index=64+mode+(guardtype-1)*2+(modulationA-1)*64+ ratetypeA*256 + (modulationB-1)*8 + ratetypeB*32;
		if (modulationA > modulationB)
			mod = modulationA;
		else 
			mod = modulationB;

		if (mod == 1) {
			wReg8(0xa48c, 0);
			wReg8(0xa48d, 0x1);
		} else if (mod == 2) {
			wReg8(0xa48c, 0x80);
			wReg8(0xa48d, 0x0);
		} else {
			wReg8(0xa48c, 0x40);
			wReg8(0xa48d, 0x0);
		}

	}

	/* Viswa on Nov28 2007 commented this part of filling up the chip memory with a function call to avoid lookup table*/

	/*lut_index = LUT_validrate[switch_index];
    if (lut_index == 0x7f) 	{
		 nmi_debug(_ERR_,"This rate is invalid\r\n");
		 isdbtlut_base = 0;
	} else {
		temp = rReg32(0xa800);
		temp &= 0xfffffc01;
		temp = temp | (LUT_TPSPERQUATER[lut_index] *2) | 0x8c00;
		wReg32(0xa800, temp);
		isdbtlut_base = lut_index * 80;
		for (temp = 0; temp < 80; temp++)
			wReg32((0xde00 + temp * 4), LUT_ISDBT[temp +isdbtlut_base]);
	}*/

	TPS_Quarter = (mode+1)*( 64 + 64/( 1<<(5-guardtype) ) );
	if(nsegs == 1)
	{
		TPS_Quarter *= 4;
	}
	temp = rReg32(0xa800);
	temp &= 0xfffffc01;
	temp = temp | TPS_Quarter | 0x8c00;
	wReg32(0xa800, temp);
	LUT(mode,guardtype,nsegs,modulationA,ratetypeA,modulationB,ratetypeB);

	if (nsegs == 0) {
		// one segment
		ab_part=511;
	} else {
	    if ((modulationA-1) == 0 && (modulationB-1) == 0 && ratetypeA == 0 && ratetypeB == 0 ) ab_part= 255 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 0 && ratetypeA == 0 && ratetypeB == 1 )ab_part= 208 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 0 && ratetypeA == 1 && ratetypeB == 0 )ab_part= 306 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 0 && ratetypeA == 1 && ratetypeB == 1 )ab_part= 255 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 1 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 153 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 1 && ratetypeA == 0 && ratetypeB == 1 )ab_part= 120 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 1 && ratetypeA == 1 && ratetypeB == 0 )ab_part= 191 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 1 && ratetypeA == 1 && ratetypeB == 1 )ab_part= 153 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 2 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 109 ;
        else if ( (modulationA-1) == 0 && (modulationB-1) == 2 && ratetypeA == 1 && ratetypeB == 0 )ab_part= 139 ;
        else if ( (modulationA-1) == 1 && (modulationB-1) == 0 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 383 ;
        else if ( (modulationA-1) == 1 && (modulationB-1) == 0 && ratetypeA == 0 && ratetypeB == 1 )ab_part= 328 ;
        else if ( (modulationA-1) == 1 && (modulationB-1) == 1 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 255 ;
        else if ( (modulationA-1) == 1 && (modulationB-1) == 1 && ratetypeA == 0 && ratetypeB == 1 )ab_part= 208 ;
        else if ( (modulationA-1) == 1 && (modulationB-1) == 2 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 191 ;
        else if ( (modulationA-1) == 2 && (modulationB-1) == 0 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 460 ;
        else if ( (modulationA-1) == 2 && (modulationB-1) == 0 && ratetypeA == 0 && ratetypeB == 1 )ab_part= 406 ;
        else if ( (modulationA-1) == 2 && (modulationB-1) == 1 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 328 ;
        else if ( (modulationA-1) == 2 && (modulationB-1) == 1 && ratetypeA == 0 && ratetypeB == 1 )ab_part= 275 ;
        else if ( (modulationA-1) == 2 && (modulationB-1) == 2 && ratetypeA == 0 && ratetypeB == 0 )ab_part= 255 ; 
		else ab_part = 255;
	}

	temp = rReg32(0xa804);

	if (!((pchp->bustype == _SDIO_) || (pchp->bustype == _SPI_))) {
		/* TS interface */
#ifdef _HAVE_FLOAT_
		if(ISNM321A0 || ISNM320C0) {
			temp = (0xbfc0403 & 0xf003ffff) | (ab_part << 18);
		} 	else {
			temp = (0xbfc0443 & 0xf003ffff) | (ab_part << 18);
		}
#else
	temp = (0xbfc0403 & 0xf003ffff) | (ab_part << 18);
#endif
	// Viswa on August 10 2007 for support of new Empia chip (PID 0x2874 and other latest series) which supports encryption.
	if(pchp->TSType == TS_SERIAL) {
		temp &= ~0x2;
		} else {
			temp |= 0x2;
		}
	} else {
		temp |= (ab_part << 18);
	}

	temp  |= 0x01;//added by hyl 2008.06.12
	temp  |= 0x40;//added by hyl 2008.06.15
	wReg32(0xa804, temp); //used to be 3fe0443 0xbfe0443

	/* Viswa on Dec13 2007 added the code for setting the PER threshold for automatic reset in C0*/
	if(ISNM321A0 || ISNM320C0) {
		wReg8(0xa436,0xFF);
		val8 = rReg8(0xa437);
		//val8 |= 0x7;//val8 &= ~0x7;
		val8 = 0x0;//val8 &= ~0x7;
		wReg8(0xa437,val8);
		if(pisdbt->modulation_layerA == 0x1) {			// QPSK
			if(pisdbt->coderate_layerA == 0x0) {		// 1/2 code rate
				val32 = 48 * 4;
			}
			else if(pisdbt->coderate_layerA == 0x01) {	// 2/3 code rate
				val32 = 64 * 4;
			}
		}
		else if(pisdbt->modulation_layerA == 0x2) {		// 16QAM
			if(pisdbt->coderate_layerA == 0x0) {		// 1/2 code rate
				val32 = 96 * 4;
			}
			else if(pisdbt->coderate_layerA == 0x1) {	// 2/3 code rate
				val32 = 128 * 4;
			}
		}
		else {
			val32 = 255;
		}
		val8 = (unsigned char)val32 & 0xFF;
		wReg8(0xa436,val8);
		val8 = rReg8(0xa437);
		//val8 |= (unsigned char)((val32 >> 8) & 0x07);
		val8 |= (((unsigned char)((val32 >> 8) & 0x07)) | 0x30);
		wReg8(0xa437,val8);
	}
	// Viswa on Feb26 2008 the BER window size for both layer A and layer B is modified based on the modulation type and code rate
	if((pisdbt->modulation_layerA == 0x01) && (pisdbt->coderate_layerA == 0x0)) {
		wReg32(0xaba4,0x1D);
	}
	else {
		wReg32(0xaba4,0x1E);
	}
	if((pisdbt->modulation_layerB == 0x01) && (pisdbt->coderate_layerB == 0x0)) {
		wReg32(0xabe4,0x1D);
	}
	else {
		wReg32(0xabe4,0x1E);
	}

	if(pisdbt->decoderconfigured == 0)
	{
		// Viswa on Oct10 2007 added the register setting for DRAM Refreshing.
		if(pisdbt->nseg == 1) {
			if (ISNM320A0) {
				wReg32(0xa820,0x21);
			} else if (ISNM321A0 || ISNM320C0) {
				//wReg32(0xa820,0x21);
				/* Viswa on Feb 11 2008 modifed the DRAM refresh setting from 0x1027 to 0x1031*/
				wReg32(0xa820, 0x1031);
				wReg32(0xa824, 0x2ee0);
				wReg32(0xa828, 0x1027);
				wReg32(0xa82c, 0x2ee0);
				
			}
		} else if(pisdbt->nseg == 3) {
			if (ISNM320A0) {
				wReg32(0xa820,0x21);
				wReg32(0xa828,0x21);
			} else if (ISNM321A0 || ISNM320C0) {
				wReg32(0xa820, 0x1061);
				wReg32(0xa824, 0x5dc0);
				wReg32(0xa828, 0x1061);
				wReg32(0xa82c, 0x5dc0);
			}
		}
		wReg32((0xa818), 0xffffffff);
		wReg32((0xab00), 0x0);
		wReg32((0xab00), 0x2);
		wReg32((0xab20), 0x0);
		wReg32((0xab24), 0x1);
		wReg32((0xab28), 0x0);

		wReg32((0xab40), 0x0);
		wReg32((0xab40), 0x2);
		wReg32((0xab60), 0x0);
		wReg32((0xab64), 0x1);
		wReg32((0xa468), 0x0);

		wReg32((0xab80), 0x1d);
		wReg32((0xab80), 0x3f);		
		wReg32((0xab9c), 0xb);

		wReg32(0xa830, 0x17);
		wReg32(0xa834, 0x17);

		//wReg32(0xaba4, 0x1C); 
		//wReg32(0xabe4, 0x19);
	}
	
	temp = rReg32(0xa800);
	temp = temp & 0xfffffffe;
	wReg32(0xa800, temp);
		
	pisdbt->decoderconfigured = 1;
	if((pchp->BoardType == _EVAL_BOARD_) || (pchp->BoardType == _USB_SDIO_DONGLE_)) {
		/* Viswa on Feb12 2008 for calculating the Time for TS output*/
		val32 = rReg32(0x6000);
		val32 &= ~(1 << 14);
		wReg32(0x6000,val32);
	}
	return;
}

void isdbt_config_extLNA_GPIO(uint8_t lna_gain)
{
	/* Viswa on Jan18 2008 changing the polarity of GPIO 15 for controlling external LNA
		for ALPS */
	if(pchp->BoardType == _USB_DONGLE_) {
		if(lna_gain == _UHF_HIGH_GAIN_) {
#ifndef INVERT_EXT_LNA
			wReg32(0x6000,0x6000);
#else
			wReg32(0x6000,0xA000);
#endif
		}
		else if(lna_gain == _UHF_LOW_GAIN_) {
#ifndef INVERT_EXT_LNA
			wReg32(0x6000,0xA000);
#else
			wReg32(0x6000,0x6000);
#endif
		}
		else if(lna_gain == _VHF_HIGH_GAIN_) {
#ifndef INVERT_EXT_LNA
			wReg32(0x6000,0x4000);
#else
			wReg32(0x6000,0x8000);
#endif
		}
		else if(lna_gain == _VHF_LOW_GAIN_) {
#ifndef INVERT_EXT_LNA
			wReg32(0x6000,0x8000);
#else
			wReg32(0x6000,0x4000);
#endif
		}
	}
	else if(pchp->BoardType == _YUAN_DONGLE_) {
		if(lna_gain == _CH13_45_UHF_HIGH_GAIN_) {
			wReg32(0x6000,0x0000);
		}
		else if(lna_gain == _CH13_45_UHF_LOW_GAIN_) {
			wReg32(0x6000,0x8000);
		}
		else if(lna_gain == _CH46_62_UHF_HIGH_GAIN_) {
			wReg32(0x6000,0x0000);
		}
		else if(lna_gain == _CH46_62_UHF_LOW_GAIN_) {
			wReg32(0x6000,0x6000);
		}
		else if(lna_gain == _YUAN_VHF_HIGH_GAIN_) {
			wReg32(0x6000,0x0);
		}
		else if(lna_gain == _YUAN_VHF_LOW_GAIN_) {
			wReg32(0x6000,0x6000);
		}
	}
}

uint32_t isdbt_agc_gain(void)
{
	uint32_t  uGain;
	uint32_t 	val;
#if 0
	val = rReg8(0xa409);
	uGain = val & 0xff;
	val = rReg8(0xa40a);
	val = ((val+ 2) & 0x03) << 8;
	uGain |= val;
#else
	val = rReg8(0xa441);
	uGain = val * 4;
#endif

  	return uGain;
}
/*
	This function is for tracking AGC
	p:Pointer to ISDBTAGC data structure
*/	
void isdbt_track_agc(void *p)
{
#ifdef _HAVE_FLOAT_
	ISDBTAGC *agc;
	int i;
	int RSSI;
	int RsiAgcTarget;   // RSSI target. 
    double RsiFgtFactor = 0.95;
	//int RsiFgtFactor;
    int RsiHls;
    int RsiGainFactor;     // LNA step size, was 6dB 
    int RsiError;
    int RsiCompOut;
    double RsiLfout = 0;
	uint8_t  rfagc,prerfagc;
    uint32_t RSSI_1;
	agc = (ISDBTAGC *)p;
	RSSI = 0;
	RSSI_1 = 0;
	if(ISNM320A0) {
		RsiAgcTarget = 20;
		//RsiFgtFactor = 95;
		RsiHls = 14;
		RsiGainFactor = 3;
		RsiError = 0;
		RsiCompOut = 0;
		RsiLfout = 0;
		rfagc = rReg8(0xa444);
		prerfagc = rfagc;
		for(i=0;i<64;i++){
			RSSI_1		= rReg8(0xa443);
			RsiError	= (int)(RSSI_1 - RsiAgcTarget);
			RSSI = RSSI_1 + RSSI;
			// Viswa on July 31 modified the logic for calculating this new algorithm.
			RsiLfout =   (1-RsiFgtFactor)*RsiError + RsiFgtFactor*RsiLfout;                                    
			if(pisdbt->preempt_agc_tracking) {
				break;
			}
		}
		RSSI = RSSI >> 6;
		if (RsiLfout > RsiHls + 25)
			RsiCompOut = - 5;
		else if (RsiLfout>RsiHls + 12)
			RsiCompOut = -3;
		else if(RsiLfout>RsiHls+6)
			RsiCompOut = -2;
		else if(RsiLfout>RsiHls)
			RsiCompOut = -1;
		else if (RsiLfout < -RsiHls-5)
			RsiCompOut = 3;
		else if(RsiLfout <-RsiHls)
			RsiCompOut = 2;
		else if(RsiLfout <-RsiHls+3)
			RsiCompOut = 1;
		else
			RsiCompOut = 0;

		// Viswa on August 07 07 added this condition to check some spurious results on 
		if((pisdbt->previous_rssi - RSSI > 30) ||(RSSI - pisdbt->previous_rssi > 30))
		{
			RsiCompOut = -1;
		}

		rfagc = rfagc + RsiCompOut*RsiGainFactor;
		rfagc = (rfagc > 0x3f)?0x3f:(rfagc < 0)?0:rfagc;
		if (prerfagc != rfagc)
		{
				wReg8(0xa444,rfagc);
		}
		agc->RSSI			= RSSI;
		agc->RfAgcCurrent	= rfagc;
		// Viswa on Mar 07 2008 added this following fix for SNR dip at power level.
		if((agc->RfAgcCurrent <= 0x1e)&&(agc->RfAgcCurrent >= 0x1a)) {
			if(isdbtRfRead(0x0f) != 0x10) {
				isdbtRfWrite(0x0f,0x10);
				isdbtRfWrite(0x2d,0xf0);
				wReg8(0xa4c0,0xaa);
			}
		}
		else {
			if(isdbtRfRead(0x0f) != 0x18) {
				isdbtRfWrite(0x0f,0x18);
				isdbtRfWrite(0x2d,0xcb);
				wReg8(0xa4c0,0xa7);
			}
		}
		agc->RfAgcCurrent	= rReg8(0xa444);		
		agc->IfVGAGain		= isdbt_agc_gain();
	}
	else {
		/* viswa on Dec20 2007 Trang requested for C0 get the averaged value of RSSI to display on the GUI.*/
		for(i=0;i<64;i++){
			RSSI_1		= rReg8(0xa443);
			RSSI		= RSSI_1 + RSSI;
			if(pisdbt->preempt_agc_tracking) {
				break;
			}
		}
		RSSI = RSSI >> 6;
		agc->RSSI			= RSSI;
		agc->RfAgcCurrent	= rReg8(0xa4c7);		/* On Dec 6 2007 rfagc register for C0 is changed from 0xa444 to 0xa437*/
		// Viswa on Mar 07 2008 added this following fix for SNR dip at power level.
		if((agc->RfAgcCurrent <= 0x1e)&&(agc->RfAgcCurrent >= 0x1a)) {
			if(isdbtRfRead(0x0f) != 0x10) {
				isdbtRfWrite(0x0f,0x10);
				isdbtRfWrite(0x2d,0xf0);
				wReg8(0xa4c0,0xaa);
			}
		}
		else {
			if(isdbtRfRead(0x0f) != 0x18) {
				isdbtRfWrite(0x0f,0x18);
				isdbtRfWrite(0x2d,0xcb);
				wReg8(0xa4c0,0xa7);
			}
		}
		agc->RfAgcCurrent	= rReg8(0xa4c7);		
		agc->IfVGAGain		= isdbt_agc_gain();
	}
	nmi_debug(_INFO_,"IfVGAGain: [%04d],RFAGC Gain: [%02x]\n",agc->IfVGAGain,agc->RfAgcCurrent);
#else
	ISDBTAGC *agc;
	int i;
	int RSSI;
	int RsiAgcTarget;   // RSSI target. 
    //double RsiFgtFactor = 0.95;
	signed long long temp1;
	signed long long temp2;
	signed long long temp3;
	signed long long RsiError_05;
	signed long long RsiLfout_95;
	//int RsiFgtFactor;
    int RsiHls;
    int RsiGainFactor;     // LNA step size, was 6dB 
    int RsiError;
    int RsiCompOut;
    int RsiLfout;
	//int RsiLfout;
	//int temp1,temp2,temp3;

	uint8_t  rfagc,prerfagc;
    uint32_t RSSI_1;
	agc = (ISDBTAGC *)p;
	RSSI = 0;
	RSSI_1 = 0;
	if(ISNM320A0) {
		RsiAgcTarget = 20;
		//RsiFgtFactor = 95;
		RsiHls = 14;
		RsiGainFactor = 3;
		RsiError = 0;
		RsiCompOut = 0;
		RsiLfout = 0;

		rfagc = rReg8(0xa444);
		prerfagc = rfagc;
		for(i=0;i<64;i++){
			RSSI_1		= rReg8(0xa443);
			RsiError	= (int)(RSSI_1 - RsiAgcTarget);
			RSSI = RSSI_1 + RSSI;
			//RsiLfout =   (1-RsiFgtFactor)*RsiError + RsiFgtFactor*RsiLfout;			/* (.05)RsiError + (.95)RsiLfout*/                                 
			/* Viswa on Oct17 2007 modifying this fractional calculation to fixed point for kernel mode driver */
			temp1 = (signed long long)RsiError << 30;
			temp2 = (signed long long)temp1 >> 6;		/*(temp1 / 64)*/
			temp3 = (signed long long)temp1 >> 5;		/*(temp1 / 32)*/
			RsiError_05 = temp2+temp3;
			/*(.95)RsiLfout*/
			temp1 = (signed long long)RsiLfout << 30;
			temp2 = (signed long long)temp1 >> 6;
			temp3 = (signed long long)temp1 >> 5;
			RsiLfout_95 = temp1 - (temp2 + temp3);
			temp1 = RsiError_05 + RsiLfout_95;
			RsiLfout = (long)(temp1 >> 30);
			if(pisdbt->preempt_agc_tracking) {
				break;
			}
		}
		RSSI = RSSI >> 6;
		if (RsiLfout > RsiHls + 25)
			RsiCompOut = - 5;
		else if (RsiLfout>RsiHls + 12)
			RsiCompOut = -3;
		else if(RsiLfout>RsiHls+6)
			RsiCompOut = -2;
		else if(RsiLfout>RsiHls)
			RsiCompOut = -1;
		else if (RsiLfout < -RsiHls-5)
			RsiCompOut = 3;
		else if(RsiLfout <-RsiHls)
			RsiCompOut = 2;
		else if(RsiLfout <-RsiHls+3)
			RsiCompOut = 1;
		else
			RsiCompOut = 0;

		// Viswa on August 07 07 added this condition to check some spurious results on 
		if((pisdbt->previous_rssi - RSSI > 30) ||(RSSI - pisdbt->previous_rssi > 30))
		{
			RsiCompOut = -1;
		}

		rfagc = rfagc + RsiCompOut*RsiGainFactor;
		rfagc = (rfagc > 0x3f)?0x3f:(rfagc < 0)?0:rfagc;
		if (prerfagc != rfagc)
		{
				wReg8(0xa444,rfagc);
		}
		agc->RSSI			= RSSI;
		agc->RfAgcCurrent	= rfagc;
		// Viswa on Mar 07 2008 added this following fix for SNR dip at power level.
		if((agc->RfAgcCurrent <= 0x1e)&&(agc->RfAgcCurrent >= 0x1a)) {
			if(isdbtRfRead(0x0f) != 0x10) {
				isdbtRfWrite(0x0f,0x10);
				isdbtRfWrite(0x2d,0xf0);
				wReg8(0xa4c0,0xaa);
			}
		}
		else {
			if(isdbtRfRead(0x0f) != 0x18) {
				isdbtRfWrite(0x0f,0x18);
				isdbtRfWrite(0x2d,0xcb);
				wReg8(0xa4c0,0xa7);
			}
		}
		agc->RfAgcCurrent	= rReg8(0xa444);		
		agc->IfVGAGain		= isdbt_agc_gain();
	}
	else {
		/* viswa on Dec20 2007 Trang requested for C0 get the averaged value of RSSI to display on the GUI.*/
		for(i=0;i<64;i++){
			RSSI_1		= rReg8(0xa443);
			RSSI		= RSSI_1 + RSSI;
			if(pisdbt->preempt_agc_tracking) {
				break;
			}
		}
		RSSI = RSSI >> 6;
		agc->RSSI			= RSSI;
		agc->RfAgcCurrent	= rReg8(0xa4c7);		/* On Dec 6 2007 rfagc register for C0 is changed from 0xa444 to 0xa437*/
		// Viswa on Mar 07 2008 added this following fix for SNR dip at power level.
		if((agc->RfAgcCurrent <= 0x1e)&&(agc->RfAgcCurrent >= 0x1a)) {
			if(isdbtRfRead(0x0f) != 0x10) {
				isdbtRfWrite(0x0f,0x10);
				isdbtRfWrite(0x2d,0xf0);
				wReg8(0xa4c0,0xaa);
			}
		}
		else {
			if(isdbtRfRead(0x0f) != 0x18) {
				isdbtRfWrite(0x0f,0x18);
				isdbtRfWrite(0x2d,0xcb);
				wReg8(0xa4c0,0xa7);
			}
		}
		agc->RfAgcCurrent	= rReg8(0xa4c7);		
		agc->IfVGAGain		= isdbt_agc_gain();
	}
	nmi_debug(_INFO_,"VGA: [%04d],RSSI: [%02x], RFGain[%02x]\n",agc->IfVGAGain,agc->RSSI,agc->RfAgcCurrent);

#endif
	// Viswa on Sept20 2007 new logic for controlling the external LNA gain.
	agc->ExtLNA = pisdbt->previous_extlna_mode;
	if((pchp->BoardType == _USB_DONGLE_)) {
#ifdef _HAVE_FLOAT1_
		if(pisdbt->external_lna_control_counter % 5) {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain = 0;
		}
		else {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain /= 5;
			if(pisdbt->previous_extlna_mode == 1) {		// EXT LNA ON
				if(pisdbt->freq > 200) {				// UHF
					if((agc->RfAgcCurrent < 0x36) && (agc->IfVGAGain < 800) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {									// VHF
					if((agc->RfAgcCurrent < 0x36) && (agc->IfVGAGain < 765) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_VHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
			else {											// EXT LNA OFF
				if(pisdbt->freq > 200) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 885)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 800)) {
						isdbt_config_extLNA_GPIO(_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
		}
#else
		if(pisdbt->external_lna_control_counter % 5) {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain = 0;
		}
		else if(ISNM321A0) {
			if(pisdbt->previous_extlna_mode == 1) {		// EXT LNA ON
				if((pisdbt->UHF_channelnum >= 13) && (pisdbt->UHF_channelnum <= 31)) {				// UHF
					if((agc->RfAgcCurrent <= 0x3C) && (agc->IfVGAGain < 284) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {									// VHF
					if((agc->RfAgcCurrent <= 0x3D) && (agc->IfVGAGain < 284) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
			else {											// EXT LNA OFF
				if((pisdbt->UHF_channelnum >= 13) && (pisdbt->UHF_channelnum <= 31)) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 710)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 710)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
		}
		else {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain /= 5;
			if(pisdbt->previous_extlna_mode == 1) {		// EXT LNA ON
				if(pisdbt->freq > 200000) {				// UHF
					if((agc->RfAgcCurrent < 0x36) && (agc->IfVGAGain < 800) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {									// VHF
					if((agc->RfAgcCurrent < 0x36) && (agc->IfVGAGain < 765) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_VHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
			else {											// EXT LNA OFF
				if(pisdbt->freq > 200000) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 885)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 800)) {
						isdbt_config_extLNA_GPIO(_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
		}
#endif
	}
	else if(pchp->BoardType == _YUAN_DONGLE_) {
#ifdef _HAVE_FLOAT1_
		if(pisdbt->external_lna_control_counter % 5) {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain = 0;
		}
		else {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain /= 5;
			if(pisdbt->previous_extlna_mode == 1) {		// EXT LNA ON
				if((pisdbt->freq >= 473.143 && pisdbt->freq <= 665.143) ) {				// UHF
					if((agc->RfAgcCurrent < 0x36) && (agc->IfVGAGain < 175) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_CH13_45_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else if((pisdbt->freq >= 671.143 && pisdbt->freq <= 767.143)) {
					if((agc->RfAgcCurrent < 0x3c) && (agc->IfVGAGain < 145) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_CH46_62_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_CH46_62_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {									// VHF
					if((agc->RfAgcCurrent <= 0x3f) && (agc->IfVGAGain < 150) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_YUAN_VHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_YUAN_VHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
			else {											// EXT LNA OFF
				/*if((pisdbt->freq == 473.143 && pisdbt->freq <= 665.143) ) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 570)) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
				}
				else*/if((pisdbt->freq >= 473.143 && pisdbt->freq <= 665.143)) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 570)) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else if((pisdbt->freq >= 671.143 && pisdbt->freq <= 767.143)) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 570)) {
						isdbt_config_extLNA_GPIO(_CH46_62_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_CH46_62_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 600)) {
						isdbt_config_extLNA_GPIO(_YUAN_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_YUAN_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
		}
#else
		if(pisdbt->external_lna_control_counter % 5) {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain = 0;
		}
		else {
			//pisdbt->avgVGAGain += agc->IfVGAGain;
			//pisdbt->avgVGAGain /= 5;
			if(pisdbt->previous_extlna_mode == 1) {		// EXT LNA ON
				if((pisdbt->freq >= 473143 && pisdbt->freq <= 665143) ) {				// UHF
					if((agc->RfAgcCurrent < 0x36) && (agc->IfVGAGain < 175) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_CH13_45_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else if((pisdbt->freq >= 671143 && pisdbt->freq <= 767143)) {
					if((agc->RfAgcCurrent < 0x3c) && (agc->IfVGAGain < 570) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_CH46_62_UHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_CH46_62_UHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {									// VHF
					if((agc->RfAgcCurrent <= 0x3F) && (agc->IfVGAGain < 150) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						if(pisdbt->avgVGAGain != 1023) {
							isdbt_config_extLNA_GPIO(_YUAN_VHF_LOW_GAIN_);
							agc->ExtLNA = 0;
							nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
						}
					}
					else if(!(pisdbt->fec_lock_a || pisdbt->fec_lock_b) && agc->RfAgcCurrent < 0x3f) {
						isdbt_config_extLNA_GPIO(_YUAN_VHF_LOW_GAIN_);
						agc->ExtLNA = 0;
						nmi_debug(_INFO_,"Ext LNA turned OFF RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
			else {											// EXT LNA OFF
				/*if((pisdbt->freq == 473143) ) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 620)) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
				}
				else*/ if((pisdbt->freq >= 473143 && pisdbt->freq <= 665143)) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 570)) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_CH13_45_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else if((pisdbt->freq >= 671143 && pisdbt->freq <= 767143)) {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 570)) {
						isdbt_config_extLNA_GPIO(_CH46_62_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_CH46_62_UHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
				else {
					if((agc->RfAgcCurrent == 0x3F) && (pisdbt->fec_lock_a || pisdbt->fec_lock_b) && (agc->IfVGAGain > 600)) {
						isdbt_config_extLNA_GPIO(_YUAN_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					else if((agc->RfAgcCurrent == 0x3F) && !(pisdbt->fec_lock_a || pisdbt->fec_lock_b)) {
						isdbt_config_extLNA_GPIO(_YUAN_VHF_HIGH_GAIN_);
						agc->ExtLNA = 1;
						nmi_debug(_INFO_,"Ext LNA turned ON RF Gain[%02x] VGA Gain[%d]\n",agc->RfAgcCurrent,agc->IfVGAGain);
					}
					pisdbt->avgVGAGain = agc->IfVGAGain;
				}
			}
		}
#endif
	}
	pisdbt->previous_rssi			= agc->RSSI;
	pisdbt->previous_extlna_mode	= agc->ExtLNA;
	++pisdbt->external_lna_control_counter;
	return;
}
/*
	This function is to read the chip ID of the connected chip
	This function  doesn't have any parameter
*/	
uint32_t isdbt_get_chipid(void) 
{
	uint32_t chipid = 0;

	/* get the chip id */
	chipid = rReg32(0x6400);
	chipid &= 0x00ffffff;
	pisdbt->chipid = chipid;
	nmi_debug(_ERR_,"chipid [%x]\n",pisdbt->chipid);

	return chipid;
}
/*
	This function is to reset the variable used to store the BER value.
	This function  doesn't have any parameter
*/	
void isdbt_rst_ber(void)
{
#ifdef _HAVE_FLOAT_
	pisdbt->aber_a = 0;
	pisdbt->aber_b = 0;
	pisdbt->bercnt = 0;
	pisdbt->aberb4viterbi_a = 0;
	pisdbt->aberb4viterbi_b = 0;
	pisdbt->b4viterbibercnt_a = 0;
	pisdbt->b4viterbibercnt_b = 0;
#endif

}
/*
	This function is to reset the variable used to store the PER value.
	This function  doesn't have any parameter
*/
void isdbt_rst_per(void)
{
#ifdef _HAVE_FLOAT_
	pisdbt->aper_a = 0;
	pisdbt->aper_b = 0;
#endif
}


void isdbt_get_time_offset(void *timeoffset)
{
	/* Viswa on Feb07 08 just for our internal debugging we changed the flag to support Floating point calcuation*/
#ifdef _HAVE_FLOAT_
	uint32_t uTimeOffset = 0;
	int	sTimeOffset;
	double dAdcFreq = 20.48;
	double dNominalRate;
	double *dTimeOffset;
	dTimeOffset = (double *)timeoffset;

	wReg8(0xa434, 1);	/* PIR_CTL */
	
	uTimeOffset = rReg8(0xa41e);
	uTimeOffset <<= 8;
	uTimeOffset |= rReg8(0xa41d);
	//nmi_debug(_INFO_, "Time Offset (%04x)\n", uTimeOffset);

	wReg8(0xa434, 0);	/* PIR_CTL */
	sTimeOffset = ((uTimeOffset & (1 << 15)) == (1 << 15))? -1:0;
	sTimeOffset &= ~((1 << 16)-1); 
	sTimeOffset |= (uTimeOffset & ((1 << 16)-1));

	dNominalRate = (64.0/7.0) * 8 * 65536.0 / (8.0 * dAdcFreq);
   *dTimeOffset  = sTimeOffset / 268.4;
   *dTimeOffset /= (dNominalRate / 65536.0);

	//return dTimeOffset;
#else

	uint32_t uTimeOffset = 0;
	//int	sTimeOffset;
	int dAdcFreq = 20;
	signed int dNominalRate;
	signed int *dTimeOffset;
	dTimeOffset = (signed int *)timeoffset;

	wReg8(0xa434, 1);	/* PIR_CTL */
	
	uTimeOffset = rReg8(0xa41e);
	uTimeOffset <<= 8;
	uTimeOffset |= rReg8(0xa41d);
	//nmi_debug(_INFO_, "Time Offset (%04x)\n", uTimeOffset);

	wReg8(0xa434, 0);	/* PIR_CTL */
	//sTimeOffset = ((uTimeOffset & (1 << 15)) == (1 << 15))? -1:0;
	//sTimeOffset &= ~((1 << 16)-1); 
	//sTimeOffset |= (uTimeOffset & ((1 << 16)-1));

	dNominalRate = (64/7) * 8 * 65536 / (8 * dAdcFreq);
   *dTimeOffset  = (signed short)uTimeOffset / 268;
   *dTimeOffset *= (65536 / dNominalRate);
#endif
}

void isdbt_get_frequency_offset(void *freqoffset)
{
	/* Viswa on Feb07 08 just for our internal debugging we changed the flag to support Floating point calcuation*/
#ifdef _HAVE_FLOAT_
	uint8_t mode;
   	uint32_t uFreqOffset = 0;
   	int32_t iFreqOffset;
   	double *dFreqOffset;
	dFreqOffset = (double *)freqoffset;
    
	wReg8(0xa434, 1);						/* PIR_CTL */
	uFreqOffset = (rReg8(0xa422) << 16);	/* CRL_FREQ_3 */
	uFreqOffset |= (rReg8(0xa421) << 8);	/* CRL_FREQ_2 */
	uFreqOffset |= rReg8(0xa420);			/* CRL_FREQ_1 */
	wReg8(0xa434, 0);						/* PIR_CTL */
	
  	iFreqOffset  = ((uFreqOffset & (1<<23)) == (1<<23)) ? -1 : 0;
  	iFreqOffset &= ~((1<<24)-1);
  	iFreqOffset |= (uFreqOffset & ((1<<24)-1));

	*dFreqOffset  = (125000/63) * (double)iFreqOffset/(double)(1<<14);   
	mode = rReg8(0xa412);
	if ((mode >> 2) & 0x1) 
		*dFreqOffset /= 2;

   	//return (-dFreqOffset);
#else
	uint8_t mode;
   	uint32_t uFreqOffset = 0;
   	//int32_t iFreqOffset;
   	signed int *dFreqOffset;
	dFreqOffset = (signed int *)freqoffset;
    
	wReg8(0xa434, 1);						/* PIR_CTL */
	uFreqOffset = (rReg8(0xa422) << 16);	/* CRL_FREQ_3 */
	uFreqOffset |= (rReg8(0xa421) << 8);	/* CRL_FREQ_2 */
	uFreqOffset |= rReg8(0xa420);			/* CRL_FREQ_1 */
	wReg8(0xa434, 0);						/* PIR_CTL */
	
  	uFreqOffset  |= ((uFreqOffset & (1<<23)) == (1<<23)) ? 0xff000000 : 0;
  	//iFreqOffset &= ~((1<<24)-1);
  	//iFreqOffset |= (uFreqOffset & ((1<<24)-1));

	*dFreqOffset  = (125000/63) * (signed int)uFreqOffset/(signed int)(1<<14);   
	mode = rReg8(0xa412);
	if ((mode >> 2) & 0x1) 
		*dFreqOffset /= 2;

#endif
}

int isdbt_agc_lock(void)
{
	uint8_t reg8;
	reg8 = rReg8(0xa401);
	if((reg8 & 0x10))
		return 1;
	else 
		return 0;
}

int isdbt_symbol_lock(void)
{
	uint8_t reg8;
	reg8 = rReg8(0xa401);
	if((reg8 & 0x20))
		return 1;
	else 
		return 0;
}

void isdbt_get_rfagcgain(void *rfgain)
{
	uint8_t *gain;
	gain = (uint8_t *)rfgain;
	if(ISNM320A0)
		*gain = rReg8(0xa444);
	else if(ISNM321A0 || ISNM320C0)
		*gain = rReg8(0xa4c7);
	//nmi_debug(_INFO_,"rfagc[%02x]\n",*gain);
}

void isdbt_set_rfagcgain(void *rfgain)
{
	wReg8(0xa444,*((uint8_t *)rfgain));
}

int isdbt_add_new_pid(void *pidconfig)
{
	uint32_t addr;
	PPID_CONFIG pid_config;
	pid_config = (PPID_CONFIG)pidconfig;
	if(pid_config->rst_pid_table_index) {
		pisdbt->rst_pidfilter_cnt = 0;
		addr = 0xA848;
	} else {
		if(pisdbt->rst_pidfilter_cnt > 15)
			return 1;
		else {
			addr = 0xA848 + (pisdbt->rst_pidfilter_cnt * 4);
		}
	}
	wReg32(addr,pid_config->pid_num);
	pisdbt->rst_pidfilter_cnt++;

	return 0;
}

void isdbt_enable_pid_filter(uint8_t enable)
{
	uint32_t val32;
	val32 = rReg32(0xA804);
	if(enable)
		val32 |= (1 << 31);
	else
		val32 &= ~(1 << 31);
	wReg32(0xA804,val32);
}

void isdbt_enable_encrypt(void *encrypt)
{
	uint32_t aes_ctrl;
	PENCRYPT_CONFIG encrypt_config;
	encrypt_config = (PENCRYPT_CONFIG)encrypt;
	if(encrypt_config->enable_config) {
		aes_ctrl = (encrypt_config->enable_encrypt)|(encrypt_config->key_select << 1);
		wReg32(0xA8A0,aes_ctrl);
		if(!(encrypt_config->key_select)) {
			wReg32(0xA8A4,encrypt_config->key_w0);
			wReg32(0xA8A8,encrypt_config->key_w1);
			wReg32(0xA8AC,encrypt_config->key_w2);
			wReg32(0xA8B0,encrypt_config->key_w3);
		}
	}
	else {
		aes_ctrl = rReg32(0xA8A0);
		if(encrypt_config->enable_encrypt) {
			aes_ctrl |= 0x1;
		}
		else {
			aes_ctrl &= 0x10;
		}
		wReg32(0xA8A0,aes_ctrl);
	}
}

void isdbt_demod_enable(uint8_t enable)
{
	uint8_t val8;
	val8 = rReg8(0xA400);
	if(!enable) {
		val8 &= ~(1 << 5);
	}
	else {
		val8 |= (1 << 5);
	}
	wReg8(0xA400,val8);
}

uint8_t isdbt_get_subchannel(void *pv)
{
	ISDBTTUNE *p = (ISDBTTUNE *)pv;
	uint8_t 	val;
	uint32_t	dfreq = p->freq;							/*in KHz */
	if(dfreq > 200000) {
		val = 21;
	}
	else {
		val = isdbt_get_channel_number(dfreq,1);
	}
	return val;
}

void isdbt_get_coarsefreqoffset(void *p)
{
	signed char *ppm_max;
	uint8_t core_state;
	uint32_t delay;
	delay = 0;
	//uint32_t mode,guard;
	ppm_max = (signed char *)p;
	if(ISNM321A0) {
		do {
			nmi_delay(2);
			delay += 2;
			core_state = (rReg8(0xa401) & 0x0F);
		}while((core_state != 0x5) && (delay <= 100));		// WAIT_TMCC
	}
	else {
		do {
			nmi_delay(2);
			delay += 2;
			core_state = (rReg8(0xa4ab) & 0x1F);
		}while(!(core_state >= 0x8) && (delay <= 100));		// WAIT_TMCC
	}
	*ppm_max = rReg8(0xa45d);
}

void isdbt_RF_powerdown(void)
{
	uint8_t val8;
	val8 = isdbtRfRead(0x00);
	val8 &= ~((1 << 4) | (1 << 3) | (1 << 0));
	isdbtRfWrite(0x00,val8);
}

void isdbt_RF_powerup(void)
{
	uint8_t val8;
	val8 = isdbtRfRead(0x00);
	val8 |= ((1 << 4) | (1 << 3) | (1 << 0));
	isdbtRfWrite(0x00,val8);
}

void isdbt_enable_dma(void *pv)
{
	uint32_t val32;
	DMA_CONFIG *p = (DMA_CONFIG *)pv;

	if (p->dma_cfg) {
		val32 = rReg32(0xa804);
		val32 &= ~0x1;						/* dir_en */
		if (p->dma_enable) 
			val32 |= (0x1 << 8);			/* dma_en */
		else
			val32 &= ~(0x1 << 8);

		if ((pchp->bustype == _SDIO_) || (pchp->bustype == _EBI_)) {
			val32 &= ~(0x1 << 9);	/* dma_host, SDIO/EBI */
		}
		if (pchp->bustype == _SPI_) {
			val32 |= (0x1 << 9);		/* dma_host, SPI */
		}
		val32 &= ~(0x1f << 10);	/* block_size */
		if (p->block_size > 0) {
			val32 |= (p->block_size << 10);
		} else {
			val32 |= (0xd << 10);	/*default size */
		}
		val32 &= ~(0x1 << 30);		/* null_en, disable */
		wReg32(0xA804,val32);
		nmi_debug(_INFO_, "[ISDBT]: 0xa804 (%08x)\n", rReg32(0xa804));

		wReg32(0xa818, 0x7fff);		/* IRQ_STS, clear */
		if (pchp->bustype == _SDIO_) {
			val32 = (0x1 << 4);
			wReg32(0xa814, val32);		/* IRQ_EN_SDIO, enable dma_rdy */
			nmi_debug(_INFO_, "[ISDBT]: 0xa814 (%08x)\n", rReg32(0xa814));

			val32 = rReg32(0x8030);	/* SDIO block interrupt */
			val32 |= 0x2;
			wReg32(0x8030, val32);
			nmi_debug(_INFO_, "[ISDBT]: 0x8030 (%08x)\n", rReg32(0x8030));

		}

		if (pchp->bustype == _SPI_) {

			val32 = rReg32(0x6100);
			if (!((val32 >> 5) & 0x1)) {
				val32 |= (1 << 5) /*| (1 << 4) | (1 << 7) | (1 << 23)*/;
				wReg32(0x6100, val32);
			}

			val32 = (0x1 << 4);
			wReg32(0xa810, val32);		/* IRQ_EN, enable dma_rdy */

			val32 = rReg32(0x788c);	/* INTR_ENABLE_REG */
			val32 |= 0x7;
			wReg32(0x788c, val32);

			val32 = rReg32(0x7894);
			val32 |= 0x1;
			wReg32(0x7894, val32);
		}
	} else {
		val32 = rReg32(0xa804);
		if (p->dma_enable) 
			val32 |= (0x1 << 8);			/* dma_en */
		else
			val32 &= ~(0x1 << 8);
		wReg32(0xA804,val32);
	}

}

void isdbt_config_TS_output(void *ts)
{
	uint32_t val32;
	TSO_CONFIG *tso_config = (TSO_CONFIG *)ts;
	val32 = rReg32(0xA804);
	if(tso_config->ts_cfg) {
		val32 &= ~(1 << 8);
		if (tso_config->ts_enable)
			val32 |= 0x1;
		else
			val32 &= ~0x1;
		val32 &= ~(0x1 << 1);
		val32 |= (tso_config->ts_type << 1);
		val32 &= ~(0x7 << 2);
		val32 |= ((tso_config->ts_clkrate & 0x7) << 2);
		val32 &= ~(0x1 << 6);
		val32 |= (tso_config->ts_gatedclk << 6);
	} else {
		if(tso_config->ts_enable) {
			val32 |= (1 << 0);
		} 	else {
			val32 &= ~(1 << 0);
		}
	}
	wReg32(0xA804,val32);
	if(tso_config->rs_error_intr_enable) {
		if(ISNM321A0) {
			val32 = rReg32(0xa818);
			val32 &= (1 << 2);
			wReg32(0xa818,val32);
			val32 = rReg32(0xa810);
			val32 |= (1 << 2);
			wReg32(0xa810,val32);
			val32 = rReg32(0x6100);
			val32 |= (1 << 5);
			wReg32(0x6100,val32);
			val32 = rReg32(0x6408);
			val32 |= (2 << 6);
			wReg32(0x6408,val32);
		}
		else {
			val32 = rReg32(0xa818);
			val32 &= (3 << 2);		// clear both layer A and layer B RS error status bit intially.
			wReg32(0xa818,val32);
			val32 = rReg32(0xa810);
			val32 |= (3 << 2);		// enable both layer A and layer B RS error intr enable
			wReg32(0xa810,val32);
		}
	}
}

void isdbt_check_DRAM(void)
{
	volatile uint32_t val32,rdval32;
	uint8_t delay;
	delay = 0;
	if(ISNM320C0) {
		wReg8(0xa400,0x0);
		val32 = rReg32(0x10000);
		val32 += 1;
		wReg32(0x10000, val32);
		rdval32 = rReg32(0x10000);
		if(val32 != rdval32) {
			nmi_debug(_ERR_,"Reset DRAM \n");
			wReg32(0x642c,0x80001bbe);
			wReg32(0x6200,0x9);
			do {
				nmi_delay(1);
				val32 = rReg32(0x6200);
				delay += 1;
			}while((val32 != 0) && (delay <= 100));
			pisdbt->channeltunedonce = 0;
			pisdbt->decoderconfigured = 0;
		}
		if(delay > 100) {
			nmi_debug(_ERR_,"DRAM reset unsuccessful\n");
		}
	}
}

int isdbt_scan_frequency(void *p)
{
	uint8_t symbollock,agcdelay;
	uint8_t agclock;
	uint32_t delay;
	signed char ppm_max;
	uint8_t val;
	int result;
	int lock	= 0;
	delay		= 0;
	agcdelay	= 0;
	result		= 0;

	isdbt_check_DRAM();
	//wReg32(0x6430,0x2fff);
	//wReg32(0x6434,0x0);
	pisdbt->channel_scan = 1;
	result = isdbt_config_tuner(p);
	if(!pisdbt->channeltunedonce)
		nmi_delay(40);
	result = isdbt_config_demod(p);
	pisdbt->channel_scan = 0;
	//wReg32(0x6430,0x3fff);
	//wReg32(0x6434,0xffffffff);
	nmi_delay(5);
#if 0
	do {
		nmi_delay(20);
		if(isdbt_agc_lock()) {
			nmi_delay(100);
			if(isdbt_symbol_lock())
				lock = 1;
			else  {
				lock = 0;
				break;
			}
		}
		else {
			isdbt_get_rfagcgain(&rfagc);
			vgagain = isdbt_get_vgagain();
			if((rfagc = 0x3f) && (vgagain ==1023)) {
				lock = 0;
				break;
			}
			else if((rfagc < 0x3f) && (vgagain > 1020)) {
				rfagc = 0x3f;
				isdbt_set_rfagcgain(&rfagc);
				lock = 0;
			}
			else if((rfagc > 0x1F) && (vgagain < 900)) {
				rfagc = 0x1F;
				isdbt_set_rfagcgain(&rfagc);
				lock = 0;
			}
		}
	}while(!lock);
#else
	do {
		if(result == 0)
			break;
		agclock = (uint8_t)isdbt_agc_lock();
		if(agclock) {
			//nmi_debug(_ERR_,"AGC Lock\n");
                        printk("TSTV:NMI320: AGC Lock\n");
			do {
				symbollock = (uint8_t)isdbt_symbol_lock();
				nmi_delay(5);
				delay += 5;
				if(delay == 100)
					break;
			}while(!symbollock);
			if(symbollock) {
				val = rReg8(0xa412);
				val = ((val >> 2) & 1);
				isdbt_get_coarsefreqoffset(&ppm_max);
				//nmi_debug(_INFO_,"[%d]\n",ppm_max);
                                printk("TSTV:NMI320: [%d]\n", ppm_max);
				if((ppm_max >= 0x0A) && (val == 1)) {							// mode 3
					lock = 1;
					break;
				}
				else if((ppm_max < 0x0A) && (ppm_max >6) && (val == 1)) {		// mode 3
					// check TMCC
					//nmi_debug(_INFO_,"TMCC check\n");
                                        printk("TSTV:NMI320: TMCC check\n");
					for(delay = 0;delay <= 300; delay += 10) {
						if (isdbt_check_tmc_lock()) {
							lock = 1;
							break;
						}
						nmi_delay(10);
					}
					if(lock)
						break;
					else
						break;
				}
				else if((ppm_max >= 0x0d) && (val == 0)) {						// mode 2
					lock = 1;
					break;
				}
				else if((ppm_max < 0x0d) && (ppm_max >6) && (val == 0)) {		// mode 2
					// check TMCC
					//nmi_debug(_INFO_,"TMCC check\n");
                                        printk("TSTV:NMI320: TMCC check\n");
					for(delay = 0;delay <= 300; delay += 10) {
						if (isdbt_check_tmc_lock()) {
							lock = 1;
							break;
						}
						nmi_delay(10);
					}
					if(lock)
						break;
					else
						break;
				}
				else {
					lock = 0;
					break;
				}
			}
			else  {
				lock = 0;
				break;
			}
		}
		else {
			agcdelay += 5;
			if(agcdelay < 60) {
				nmi_delay(5);
				continue;
			}
			else {
				lock = 0;
				break;
			}
		}
	}while(!lock);
#endif
	if(lock)
		return 1;
	else 
		return 0;
}

void isdbt_master_reset(void)
{
	//uint32_t val32;
	wReg32(0x6414,0xFFFE);
	//val32 = rReg32(0x6414);
}

void isdbt_start(void)
{
	uint32_t val32;
	nmi_bus_mode(0);
		
	val32 = 0x8000 ;//0xc000;//
	wReg32(0x6418,val32);

	val32 = 0x101 ;
	wReg32(0x6458,val32);

	val32 = 0x800e;//0x8000e ;
	wReg32(0x644c,val32);

	val32 = 0xff3180 ;
	wReg32(0x6448,val32);
	
	//Mimic ontimer from hiphop
	val32 = rReg8(0xa401);
	val32 = rReg32(0xa80c);
	val32 = rReg32(0xa830);
	val32 = rReg32(0xaba8);
	isdbt_software_reset();
	//isdbt_master_reset();
}

void isdbt_stop(void)
{
	uint32_t val32;

	val32 = 0x8400 ;
	wReg32(0x6418,val32);

	val32 = 0x0000 ;
	wReg32(0x6458,val32);

	val32 = 0x1800e ;
	wReg32(0x644c,val32);

	val32 = 0xff317c ;
	wReg32(0x6448,val32);

	//This call will disable all read write operations	
	nmi_bus_mode(1);
}

void isdbt_preempt_track_agc(void)
{
	if(pisdbt->preempt_agc_tracking)
		pisdbt->preempt_agc_tracking = 0;
	else 
		pisdbt->preempt_agc_tracking = 1;
}

void isdbt_get_signal_rssi(void *p)
{
	int32_t *RSSI= (int32_t *)p;
	uint32_t u32LNA = 0,u32DAC = 0,u32temp = 0;
	
	//nm_read_reg(ISDBT_CTL_WBRSSI_ADDR,(uint8_t*)&u32LNA,1);
	if(ISNM320A0)
		u32LNA = rReg8(0xA444);
	else if(ISNM321A0 || ISNM320C0)
		u32LNA = rReg8(0xA4c7);
	
	//nm_read_reg(ISDBT_AGC_GAIN_2_ADDR,(uint8_t*)&u32temp,1);
	u32temp = rReg8(0xA40A);
	
	u32temp += 2;
	u32temp &= 0x3;
	u32temp <<= 8;
	//nm_read_reg(ISDBT_AGC_GAIN_1_ADDR,(uint8_t*)&u32DAC,1);
	u32DAC = rReg8(0xA409);
		
	u32DAC  &= 0xff;
	u32DAC |=u32temp;
	*RSSI = (int32_t )(RF_CALIBRATION_VALUE - u32LNA - u32DAC/14 );
	if(u32LNA <=33)
	{
		u32temp = (u32LNA <=21)? (29):(36);
		*RSSI -= (u32temp-u32LNA);
	}
}

void isdbt_handle_spi_intr(void *pv)
{
	uint32_t sta, enable;
	int skip = 0;

	if (pv != NULL)
		*(int32_t *)pv = 1;

	enable = rReg32(0xa810);
	sta = rReg32(0xa818);		/* read the status */
	nmi_debug(_INTR_, "[ISDBT]: SPI, IRQ_STS (%08x)\n", sta);
	wReg32(0xa818, sta);			/* clear the status */

	if(((sta >> 2) & 0x1) || ((sta >>3) & 0x1)) {
		nmi_debug(_ERR_,"[ISDBT]: SPI, RS Error \n");
		skip = 1;
		if (pv != NULL)
			*(int32_t *)pv = 0;
	}

	/* any overflow */
	if ((sta >> 12) & 0x1) {
		nmi_debug(_ERR_, "[ISDBT]: SPI, *** Buffer OverFlow ***\n");
		skip = 1;
		if (pv != NULL)
			*(int32_t *)pv = 0;
	} 

	/* dma ready */ 
	if ((sta >> 4) & 0x1) {
		do {
			if (skip) {	/* this is to clear the DMA condition */
				uint32_t adr, cnt;
				adr = rReg32(0x7864);
				cnt = rReg32(0x7868);
				nmi_debug(_INTR_, "[ISDBT]: 1. adr (%08x), cnt (%d)\n", adr, cnt);
				adr += cnt - 4;
				nmi_debug(_INTR_, "[ISDBT]: 2. adr (%08x)\n", adr);
				wReg32(0x7864, adr);
				wReg32(0x7868, 4);
			}
			nmi_dma_read(NULL);
			sta = rReg32(0xa818);
			nmi_debug(_INTR_, "[ISDBT]: SPI, IRQ Status 1, (%08x)\n", sta);
			wReg32(0xa818, sta);
			if ((sta >> 5) & 0x1) {
				nmi_debug(_INTR_, "[ISDBT]: SPI, *** DMA Done ***\n");
				sta = rReg32(0xa818);
				nmi_debug(_INTR_, "[ISDBT]: SPI, IRQ Status 2, (%08x)\n", sta);
				if (!((sta >> 4) & 0x1))
					break;
			} 
		} while (1);
	} 

	return;
}

void isdbt_handle_sdio_intr(void *pv)
{
	uint32_t 				sta,enable, val32;
	ISDBTSDIODMA dma;
	uint32_t 				sadr;
	int 						nblk, nleft, cnt; 	
	int 						i, dummy;

	enable = rReg32(0xa810);
	sta = rReg32(0xa818);		/* read the status */

	nmi_debug(_INTR_, "[ISDBT]: SDIO, IRQ_STS (%08x)\n", sta);
	wReg32(0xa818, sta);			/* clear the status */

	if(((sta >> 2) & 0x1) || ((sta >>3) & 0x1)) {
		nmi_debug(_ERR_,"[ISDBT]: SDIO, *** RS Error ***\n");
	}

	if ((sta >> 12) & 0x1) {
		nmi_debug(_ERR_, "[ISDBT]: SDIO, *** Buffer Overflow ***\n");
	}

	if ((sta >> 4) & 0x1) {
		
		nmi_debug(_INTR_, "[ISDBT]: SDIO, *** DMA Ready ***\n");

		do {
			cnt 	= rReg32(0x801c);
			sadr = rReg32(0x8014);
			nblk = cnt/1024;
			nleft = cnt%1024;
			nmi_debug(_INFO_, "[ISDBT]: SDIO, cnt (%d), sadr (%08x), block (%d), left(%d)\n", cnt, sadr, nblk, nleft);

#if 0
			if (nblk > 0) {
				if (nleft == 0) {
					nblk = cnt/1023;
					nleft = cnt%1023;
					dma.nblk = nblk;
					dma.szblk = 1023;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
					/* delay loop */
					for (i=0; i< 10000;i++){ 
						if (i == 1000) 
							dummy = i;
					}
					sadr += nblk*1023;
					wReg32(0x8014, sadr);					
					val32 = rReg32(0x801c);
					val32 = rReg32(0x8014);
					//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", rReg32(0xd01c), rReg32(0xd014));
					dma.nblk = 1;
					dma.szblk = nleft;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
				} else {
					dma.nblk = nblk;
					dma.szblk = 1024;
					dma.last = 0;
					//wReg32(0xd01c, (nblk * 1024));					
					nmi_dma_read((void *)&dma);
						/* delay loop */
						for (i=0; i< 10000;i++){ 
							if ( i == 1000) 
								dummy = i;
						}

					//wReg32(0xd01c, nleft);
					sadr += nblk*1024;
					wReg32(0x8014, sadr);					
					val32 = rReg32(0x801c);
					val32 = rReg32(0x8014);
					//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", rReg32(0xd01c), rReg32(0xd014));
					dma.nblk = 1;
					dma.szblk = nleft;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
				} 
			}else {
				int blksz = nleft/2 + 1;

				dma.nblk = 1;
				dma.szblk = blksz;
				dma.last = 0;
				nmi_dma_read((void *)&dma);

				/* delay loop */
						for (i=0; i< 10000;i++){ 
							if ( i == 1000) 
								dummy = i;
						}

				sadr += blksz;
				wReg32(0x8014, sadr);					
				val32 = rReg32(0x801c);
				val32 = rReg32(0x8014);

				//nmi_debug(_INFO_, "[DVB]: transfer count (%d), address (%08x)\n", rReg32(0xd01c), rReg32(0xd014));
				dma.nblk = 1;
				dma.szblk = nleft - blksz;				
				dma.last = 0;
				nmi_dma_read((void *)&dma);
			}

#else
			if (nblk > 0) {
				if (nleft == 0) {
					if (nblk > 1) {
						dma.nblk 	= nblk-1;
						dma.szblk 	= 1024;
						dma.last 		= 0;
						nmi_dma_read((void *)&dma);
						/* delay loop */
						for (i=0; i< 10000;i++){ 
							if ( i == 1000) 
								dummy = i;
						}
						sadr += (nblk-1)*1024;
						wReg32(0x8014, sadr);					
						cnt = rReg32(0x801c);
						sadr = rReg32(0x8014);
						nmi_debug(_VERB_, "[ISDBT]: SDIO, transfer count (%d), address (%08x)\n", cnt, sadr);
						dma.nblk = 1;
						dma.szblk = 1024;
						dma.last = 0;
						nmi_dma_read((void *)&dma);
					} else {
						dma.nblk 	= 1;
						dma.szblk 	= 1020;
						dma.last 		= 0;
						nmi_dma_read((void *)&dma);
						/* delay loop */
					for (i=0; i< 10000;i++){ 
						if ( i == 1000) 
							dummy = i;
					}
						sadr += 1020;
					wReg32(0x8014, sadr);					
					cnt = rReg32(0x801c);
					sadr = rReg32(0x8014);
						nmi_debug(_VERB_, "[ISDBT]: SDIO, transfer count (%d), address (%08x)\n", cnt, sadr);
						dma.nblk 	= 1;
						dma.szblk 	= 4;
						dma.last 		= 0;
						dma.adr 		= sadr;
						nmi_dma_read((void *)&dma);
					}
				} else {
					dma.nblk 	= nblk;
					dma.szblk 	= 1024;
					dma.last 		= 0;
					nmi_dma_read((void *)&dma);
				for (i=0; i< 10000;i++){ 
					if ( i == 1000) 
						dummy = i;
				}
					sadr += nblk*1024;
				wReg32(0x8014, sadr);					
				cnt = rReg32(0x801c);
				sadr = rReg32(0x8014);
					nmi_debug(_VERB_, "[ISDBT]: SDIO, transfer count (%d), address (%08x)\n", cnt, sadr);
					dma.nblk = 1;
					dma.szblk = nleft;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
				} 
			} else {
				//int blksz = nleft/2 + 1;
				if (nleft > 4) {
					int blksz = nleft - 4;
					dma.nblk = 1;
					dma.szblk = blksz;
					dma.last = 0;
					nmi_dma_read((void *)&dma);
					/* delay loop */
					for (i=0; i< 10000;i++){ 
						if ( i == 1000) 
							dummy = i;
					}
					sadr += blksz;
					wReg32(0x8014, sadr);					
					cnt = rReg32(0x801c);
					sadr = rReg32(0x8014);
					//nmi_debug(_INFO_, "[ISDBT]: transfer count (%d), address (%08x)\n", cnt, sadr);
					dma.nblk = 1;
					dma.szblk = 4;
					dma.last = 0;
					dma.adr = sadr;
					nmi_dma_read((void *)&dma);
				} else {	/* the minimum size of the block has to be 4 bytes */
					dma.nblk = 1;
					dma.szblk = 4;
					dma.last = 1;
					dma.adr = sadr;
					nmi_dma_read((void *)&dma);

				}
			}
#endif

			val32 = rReg32(0xa818);
			nmi_debug(_VERB_, "[ISDBT]: IRQ Status 1, (%08x)\n", val32);
			wReg32(0xa818, val32);

			if ((val32 >> 5) & 0x1) {
				nmi_debug(_INTR_, "[ISDBT]: *** DMA Done ***\n");
				val32 = rReg32(0xa818);
				nmi_debug(_VERB_, "[ISDBT]: IRQ Status 2, (%08x)\n", val32);

				if (!((val32 >> 4) & 0x1))
					break;
			} 

		} while (1);
	}

}

void isdbt_handle_intr(void *pv)
{
	uint32_t val32;
	if (pchp->bustype == _SPI_) {
		isdbt_handle_spi_intr(pv);
	} else if (pchp->bustype == _SDIO_) {
		isdbt_handle_sdio_intr(pv);
	} else if (pchp->bustype == _EBI_) {

	} else if(pchp->bustype == _I2C_) {
		val32 = rReg32(0x6100);
		if(val32 & (1 << 5)) {			// decoder intr enabled
			val32 = rReg32(0xa418);
			val32 &= (1 << 2);			// clearing RS error indicator.
			wReg32(0xa418,val32);
		}
	}
}

int isdbt_chip_init(void)
{
	//uint32_t chipid;
	int ix = pchp->CrystalIndex;
	uint32_t r6460 = crystalTbl[ix].reg6460;
	uint32_t temp;
	mdelay(10);
	/* get the chip id */
	pisdbt->chipid = isdbt_get_chipid();
	if (!(ISNM320A0 || ISNM321A0 || ISNM320C0)) {
		nmi_debug(_ERR_,"failed to read chipid [%x]\n",pisdbt->chipid);
		return -1;
	}

	/* Viswa on Dec6 2007 for C0 we have make our chip operating voltage as 1.0V so we write the RF register*/
	if(ISNM321A0 || ISNM320C0) {
		//isdbtRfWrite(0x2A,0x80);
		wReg32(0x641c,0xDE);
	}
	else {
		wReg32(0x641c,0x00);
	}

	/* initialize the chip */
	if(ISNM321A0 || ISNM320C0) {
		if(pchp->CrystalIndex == 7) {	// 32 MHz
			wReg32(0x6460, r6460);
		}
		/* Viswa on Feb07 2008 modified the PLL programming for 26 MHz to reduce the ppm in timeoffset*/
		else if(pchp->CrystalIndex == 5) {	// 26 MHz
			wReg32(0x6460, r6460);
		}
		else {
			wReg32(0x6460, 0xf4450e97);
		}		
	}
	else {
		wReg32(0x6460, r6460/*0x64450e3c*/);
	}
	//wReg32(0x6468, 0x40200100);
	wReg32(0x6464,0x10d);
#ifdef _HAVE_FLOAT1_
	if (crystalTbl[ix].dCrystalFreq == 19.68)
		wReg32(0x6468, 0x44100100);
	else
		wReg32(0x6468, 0x44200100);
#else
	if(ISNM321A0 || ISNM320C0)
		wReg32(0x6468, 0x472000C0);
	else 
		wReg32(0x6468, 0x44200100);
#endif

	wReg32(0x6418, 0xc000);
	nmi_delay(1);
	wReg32(0x6418, 0x8000);
	wReg32(0x6404, 0x12492492);
	wReg32(0x6408, 0x2000012);
	//wReg32(0x6408, 0x12);
	wReg32(0x640c, 0);
	wReg32(0x6414, 0xffffffff);
	//wReg32(0x641c, 0);
	wReg32(0x6420, 0);
	if(pisdbt->chipid == 0x321A1) {
		wReg32(0x6424,0x0800);
	}
	else {
		wReg32(0x6424, 0);
	}
	//wReg32(0x6428, 0);
	wReg32(0x6430, 0xffffffff);
	wReg32(0x6434, 0xffffffff);
	wReg32(0x6448, 0x00ff3180);
	wReg32(0x644c, 0x800e);
	wReg32(0x6458, 0x101);
	wReg32(0x646c, 0);
	if (ISNM320A0)
		wReg32(0x6470, 0);
	else if (ISNM320C0)
		wReg32(0x6470, 0x183);		/* btrim */
	else if(ISNM321A0)
		wReg32(0x6470,0x03);

	wReg32(0x636c, 0);
	wReg32(0x6300, 0x63);
	wReg32(0x631c, 0x6);
	wReg32(0x6320, 0xd);
	wReg32(0x6304, 0x60);
	wReg32(0x6330, 0xaff);
	wReg32(0x6338, 0x1);
	wReg32(0x633c, 0);
	wReg32(0x636c, 0x1);
	// Viswa on Sept20 2007 added the GPIO register configuration setting for new ISDBT USB dongle(Trang)
	if(pchp->BoardType == _USB_DONGLE_ || pchp->BoardType == _YUAN_DONGLE_ || pchp->BoardType == _USB_SDIO_DONGLE_)
		wReg32(0x6004, 0xE000);
	// Viswa on Jan08 2008 for 320C0 and 321A0 Trang changed the value for RF register 0x2a from 0xe0 to 0xc0
	// Viswa on Mar07 2008 Trang changed the value for RF register 0x2A from 0x2A from 0xC0 to 0xA0.
	if(ISNM321A0 || ISNM320C0)
		isdbtRfWrite(0x2a,0xA0);

	temp = rReg32(0xa800);
	temp = temp & 0xfffffffe;
	wReg32(0xa800, temp);
	nmi_debug(_INFO_,"reg[%x]",temp);

	pisdbt->rst_pidfilter_cnt = 0;
	pisdbt->channeltunedonce = 0;
	pisdbt->decoderconfigured = 0;
	pisdbt->channel_scan	= 0;
	return 0;
}


int isdbt_init(void)
{
	if(isdbt_chip_init() < 0)
        {
                printk("TSTV:NMI320: isdbt_init failed \t%s[%d]\n", __FUNCTION__, __LINE__);
		return -1;
        }

        printk("TSTV:NMI320: isdbt_init success \t%s[%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

void nmi_dma_read(void *pv)
{
	if (pchp->hlp.nmi_dma_read) {
		pchp->hlp.nmi_dma_read(pv);
	}
}
void nmi_bus_mode(uint8_t enable)
{
	if(pchp->hlp.nmi_bus_mode != NULL) {
		pchp->hlp.nmi_bus_mode(enable);
	}
	return;
}

/*
This function is the entry point for the ISDBT ASIC driver. 
Client layer or the higher layer driver calls this function to open the handle to the underlying ISDBT device. 
This function initializes the chip and creates function handles for other modules to access the chip.
*/
int nmi_common_init(void *pdrv,void *dtvvtbl)
{
	NMICMN		*nmi_cmn;
	NMIDTVVTBL	*nmi_dtvvtbl;
	int			init;
	nmi_cmn = (NMICMN *)pdrv;
	nmi_dtvvtbl = (NMIDTVVTBL *)dtvvtbl;
	memset((void *)pchp, 0, sizeof(NMICHIP));

	/* save the driver info */
	pchp->hlp.nmi_write_reg		= nmi_cmn->tbl.nmi_write_reg;
	pchp->hlp.nmi_read_reg		= nmi_cmn->tbl.nmi_read_reg;
	pchp->hlp.nmi_delay			= nmi_cmn->tbl.nmi_delay;
	pchp->hlp.nmi_get_tick		= nmi_cmn->tbl.nmi_get_tick;
	pchp->hlp.nmi_log			= nmi_cmn->tbl.nmi_log;
	pchp->hlp.nmi_dma_read		= nmi_cmn->tbl.nmi_dma_read;

	pchp->dtvtype		= nmi_cmn->dtvtype;
	pchp->dbg			= nmi_cmn->dbgflag;
	pchp->bustype		= nmi_cmn->bustype;
	pchp->CrystalIndex	= nmi_cmn->crystalindex;
	pchp->BoardType		= nmi_cmn->boardtype;
	pchp->TSType		= nmi_cmn->tstype;

	if (pchp->dtvtype == DVB) {
#ifdef _NMI_INCLUDE_DVB_
		dtvvtbl = dvb_init();
#endif
	} else if (pchp->dtvtype == ISDBT) {
		init = isdbt_init();
		if(init < 0)
			return -1;
		/* set up vtable */
		nmi_dtvvtbl->nmi_config_tuner			= isdbt_config_tuner;
		nmi_dtvvtbl->nmi_config_demod			= isdbt_config_demod;
		nmi_dtvvtbl->nmi_config_mac				= isdbt_config_decoder;
		nmi_dtvvtbl->nmi_get_chipid				= isdbt_get_chipid;
		nmi_dtvvtbl->nmi_get_snr				= isdbt_get_snr;
		nmi_dtvvtbl->nmi_get_ber				= isdbt_get_ber;
		nmi_dtvvtbl->nmi_get_per				= isdbt_get_per;
		nmi_dtvvtbl->nmi_get_channel_length		= isdbt_get_channel_length;
		nmi_dtvvtbl->nmi_get_freq_offset		= isdbt_get_frequency_offset;
		nmi_dtvvtbl->nmi_get_time_offset		= isdbt_get_time_offset;
		nmi_dtvvtbl->nmi_track_agc				= isdbt_track_agc;
		nmi_dtvvtbl->nmi_rst_ber				= isdbt_rst_ber;
		nmi_dtvvtbl->nmi_rst_per				= isdbt_rst_per;
		nmi_dtvvtbl->nmi_rf_read_reg			= isdbtRfRead;
		nmi_dtvvtbl->nmi_rf_write_reg			= isdbtRfWrite;
		nmi_dtvvtbl->nmi_soft_reset				= isdbt_software_reset;
		nmi_dtvvtbl->nmi_set_segment			= isdbt_set_segment;
		nmi_dtvvtbl->nmi_fec_lock				= isdbt_check_fec_lock;
		nmi_dtvvtbl->nmi_tmc_lock				= isdbt_check_tmc_lock;
		nmi_dtvvtbl->nmi_check_soft_reset		= isdbt_check_soft_reset;
		nmi_dtvvtbl->nmi_config_extLNA_GPIO		= isdbt_config_extLNA_GPIO;
		nmi_dtvvtbl->nmi_agc_lock				= isdbt_agc_lock;
		nmi_dtvvtbl->nmi_symbol_lock			= isdbt_symbol_lock;		
		nmi_dtvvtbl->nmi_get_rfagcgain			= isdbt_get_rfagcgain;
		nmi_dtvvtbl->nmi_set_rfagcgain			= isdbt_set_rfagcgain;
		nmi_dtvvtbl->nmi_get_vgagain			= isdbt_agc_gain;
		nmi_dtvvtbl->nmi_add_new_pid			= isdbt_add_new_pid;
		nmi_dtvvtbl->nmi_enable_pid_filter		= isdbt_enable_pid_filter;
		nmi_dtvvtbl->nmi_enable_encrypt			= isdbt_enable_encrypt;
		nmi_dtvvtbl->nmi_demod_enable			= isdbt_demod_enable;
		nmi_dtvvtbl->nmi_get_subchannel			= isdbt_get_subchannel;
		nmi_dtvvtbl->nmi_RF_powerdown			= isdbt_RF_powerdown;
		nmi_dtvvtbl->nmi_RF_powerup				= isdbt_RF_powerup;
		nmi_dtvvtbl->nmi_enable_dma				= isdbt_enable_dma;
		nmi_dtvvtbl->nmi_config_TS_output		= isdbt_config_TS_output;
		nmi_dtvvtbl->nmi_master_reset			= isdbt_master_reset;
		nmi_dtvvtbl->nmi_start_isdbt			= isdbt_start;
		nmi_dtvvtbl->nmi_stop_isdbt				= isdbt_stop;
		nmi_dtvvtbl->nmi_preempt_trackagc		= isdbt_preempt_track_agc;
		nmi_dtvvtbl->nmi_get_signal_rssi		= isdbt_get_signal_rssi;
		nmi_dtvvtbl->nmi_get_ber_b4viterbi		= isdbt_get_ber_b4viterbi;
		nmi_dtvvtbl->nmi_set_new_ber_timer		= isdbt_set_new_bertimer;
		nmi_dtvvtbl->nmi_handle_intr			= isdbt_handle_intr;
		nmi_dtvvtbl->nmi_get_coarsefreqoffset	= isdbt_get_coarsefreqoffset;
		nmi_dtvvtbl->nmi_scan_frequency			= isdbt_scan_frequency;
		nmi_dtvvtbl->nmi_check_DRAM				= isdbt_check_DRAM;
		/*
	nmi_dtvvtbl->nmi_config_tuner		= ptv->nmi_config_tuner;
	nmi_dtvvtbl->nmi_config_demod		= ptv->nmi_config_demod;
	nmi_dtvvtbl->nmi_config_mac			= ptv->nmi_config_mac;
	nmi_dtvvtbl->nmi_get_chipid			= ptv->nmi_get_chipid;
	nmi_dtvvtbl->nmi_get_snr			= ptv->nmi_get_snr;
	nmi_dtvvtbl->nmi_get_ber			= ptv->nmi_get_ber;
	nmi_dtvvtbl->nmi_get_per			= ptv->nmi_get_per;
	nmi_dtvvtbl->nmi_get_channel_length	= ptv->nmi_get_channel_length;
	nmi_dtvvtbl->nmi_get_freq_offset	= ptv->nmi_get_freq_offset;
	nmi_dtvvtbl->nmi_get_time_offset	= ptv->nmi_get_time_offset;
	nmi_dtvvtbl->nmi_track_agc			= ptv->nmi_track_agc;
	nmi_dtvvtbl->nmi_rst_ber			= ptv->nmi_rst_ber;
	nmi_dtvvtbl->nmi_rst_per			= ptv->nmi_rst_per;
	nmi_dtvvtbl->nmi_rf_read_reg		= isdbtRfRead;
	nmi_dtvvtbl->nmi_rf_write_reg		= isdbtRfWrite;
	nmi_dtvvtbl->nmi_soft_reset			= ptv->nmi_soft_reset;
	nmi_dtvvtbl->nmi_set_segment		= ptv->nmi_set_segment;
	nmi_dtvvtbl->nmi_fec_lock			= ptv->nmi_fec_lock;
	nmi_dtvvtbl->nmi_tmc_lock			= ptv->nmi_tmc_lock;
	nmi_dtvvtbl->nmi_check_soft_reset	= ptv->nmi_check_soft_reset;
	nmi_dtvvtbl->nmi_config_extLNA_GPIO	= ptv->nmi_config_extLNA_GPIO;
	nmi_dtvvtbl->nmi_agc_lock			= ptv->nmi_agc_lock;
	nmi_dtvvtbl->nmi_symbol_lock		= ptv->nmi_symbol_lock;		
	nmi_dtvvtbl->nmi_get_rfagcgain		= ptv->nmi_get_rfagcgain;
	nmi_dtvvtbl->nmi_set_rfagcgain		= ptv->nmi_set_rfagcgain;
	nmi_dtvvtbl->nmi_get_vgagain		= ptv->nmi_get_vgagain;
	nmi_dtvvtbl->nmi_add_new_pid		= ptv->nmi_add_new_pid;
	nmi_dtvvtbl->nmi_enable_pid_filter	= ptv->nmi_enable_pid_filter;
	nmi_dtvvtbl->nmi_enable_encrypt		= ptv->nmi_enable_encrypt;
	nmi_dtvvtbl->nmi_demod_enable		= ptv->nmi_demod_enable;
	nmi_dtvvtbl->nmi_get_subchannel		= ptv->nmi_get_subchannel;
	nmi_dtvvtbl->nmi_RF_powerdown		= ptv->nmi_RF_powerdown;
	nmi_dtvvtbl->nmi_RF_powerup			= ptv->nmi_RF_powerup;
	nmi_dtvvtbl->nmi_enable_dma			= ptv->nmi_enable_dma;
	nmi_dtvvtbl->nmi_config_TS_output	= ptv->nmi_config_TS_output;
	nmi_dtvvtbl->nmi_master_reset		= ptv->nmi_master_reset;
	nmi_dtvvtbl->nmi_start_isdbt		= ptv->nmi_start_isdbt;
	nmi_dtvvtbl->nmi_stop_isdbt			= ptv->nmi_stop_isdbt;
	nmi_dtvvtbl->nmi_preempt_trackagc	= ptv->nmi_preempt_trackagc;
	nmi_dtvvtbl->nmi_get_signal_rssi	= ptv->nmi_get_signal_rssi;
		*/
	} else {
		//return ((NMIDTVVTBL *)0);
	}

	//return dtvvtbl;
	return 1;
}




