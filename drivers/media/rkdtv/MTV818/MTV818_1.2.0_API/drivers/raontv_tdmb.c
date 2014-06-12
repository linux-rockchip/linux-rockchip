/******************************************************************************** 
* (c) COPYRIGHT 2010 RAONTECH, Inc. ALL RIGHTS RESERVED.
* 
* This software is the property of RAONTECH and is furnished under license by RAONTECH.                
* This software may be used only in accordance with the terms of said license.                         
* This copyright noitce may not be remoced, modified or obliterated without the prior                  
* written permission of RAONTECH, Inc.                                                                 
*                                                                                                      
* This software may not be copied, transmitted, provided to or otherwise made available                
* to any other person, company, corporation or other entity except as specified in the                 
* terms of said license.                                                                               
*                                                                                                      
* No right, title, ownership or other interest in the software is hereby granted or transferred.       
*                                                                                                      
* The information contained herein is subject to change without notice and should 
* not be construed as a commitment by RAONTECH, Inc.                                                                    
* 
* TITLE 	  : RAONTECH TV T-DMB services source file. 
*
* FILENAME    : raontv_tdmb.c
*
* DESCRIPTION : 
*		Library of routines to initialize, and operate on, the RAONTECH T-DMB demod.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 09/27/2010  Ko, Kevin        Creat for CS Realease
*             /Yang, Maverick  1.Reformating for CS API
*                              2.pll table, ADC clock switching, SCAN function, 
*								 FM function added..
* 04/09/2010  Yang, Maverick   REV1 SETTING 
* 01/25/2010  Yang, Maverick   Created.                                                              
********************************************************************************/

#include "raontv.h"
#include "raontv_rf.h"
#include "raontv_internal.h"


#ifdef RTV_TDMB_ENABLE

#undef OFDM_PAGE
#define OFDM_PAGE	0x6

#undef FEC_PAGE
#define FEC_PAGE	0x09


/*==============================================================================
 * Replace the below code to eliminates the sqrt() and log10() functions.
 * In addtion, to eliminates floating operation, we multiplied by RTV_TDMB_SNR_DIVIDER to the floating SNR.
 * SNR = (double)(100/(sqrt((double)data) - log10((double)data)*log10((double)data)) -7);
 *============================================================================*/
static const U16 g_awSNR_15_160[] = 
{
	33163/* 15 */, 32214/* 16 */, 31327/* 17 */, 30496/* 18 */, 29714/* 19 */, 
	28978/* 20 */, 28281/* 21 */, 27622/* 22 */, 26995/* 23 */, 26400/* 24 */, 
	25832/* 25 */, 25290/* 26 */, 24772/* 27 */, 24277/* 28 */, 23801/* 29 */, 
	23345/* 30 */, 22907/* 31 */, 22486/* 32 */, 22080/* 33 */, 21690/* 34 */, 
	21313/* 35 */, 20949/* 36 */, 20597/* 37 */, 20257/* 38 */, 19928/* 39 */, 
	19610/* 40 */, 19301/* 41 */, 19002/* 42 */, 18712/* 43 */, 18430/* 44 */, 
	18156/* 45 */, 17890/* 46 */, 17632/* 47 */, 17380/* 48 */, 17135/* 49 */, 
	16897/* 50 */, 16665/* 51 */, 16438/* 52 */, 16218/* 53 */, 16002/* 54 */, 
	15792/* 55 */, 15587/* 56 */, 15387/* 57 */, 15192/* 58 */, 15001/* 59 */, 
	14814/* 60 */, 14631/* 61 */, 14453/* 62 */, 14278/* 63 */, 14107/* 64 */, 
	13939/* 65 */, 13775/* 66 */, 13615/* 67 */, 13457/* 68 */, 13303/* 69 */, 
	13152/* 70 */, 13004/* 71 */, 12858/* 72 */, 12715/* 73 */, 12575/* 74 */, 
	12438/* 75 */, 12303/* 76 */, 12171/* 77 */, 12041/* 78 */, 11913/* 79 */, 
	11788/* 80 */, 11664/* 81 */, 11543/* 82 */, 11424/* 83 */, 11307/* 84 */, 
	11192/* 85 */, 11078/* 86 */, 10967/* 87 */, 10857/* 88 */, 10749/* 89 */, 
	10643/* 90 */, 10539/* 91 */, 10436/* 92 */, 10334/* 93 */, 10235/* 94 */, 
	10136/* 95 */, 10039/* 96 */, 9944/* 97 */, 9850/* 98 */, 9757/* 99 */, 
	9666/* 100 */, 9576/* 101 */, 9487/* 102 */, 9400/* 103 */, 9314/* 104 */, 
	9229/* 105 */, 9145/* 106 */, 9062/* 107 */, 8980/* 108 */, 8900/* 109 */, 
	8820/* 110 */, 8742/* 111 */, 8664/* 112 */, 8588/* 113 */, 8512/* 114 */, 
	8438/* 115 */, 8364/* 116 */, 8292/* 117 */, 8220/* 118 */, 8149/* 119 */, 
	8079/* 120 */, 8010/* 121 */, 7942/* 122 */, 7874/* 123 */, 7807/* 124 */, 
	7742/* 125 */, 7676/* 126 */, 7612/* 127 */, 7548/* 128 */, 7485/* 129 */, 
	7423/* 130 */, 7362/* 131 */, 7301/* 132 */, 7241/* 133 */, 7181/* 134 */, 
	7123/* 135 */, 7064/* 136 */, 7007/* 137 */, 6950/* 138 */, 6894/* 139 */, 
	6838/* 140 */, 6783/* 141 */, 6728/* 142 */, 6674/* 143 */, 6621/* 144 */, 
	6568/* 145 */, 6516/* 146 */, 6464/* 147 */, 6412/* 148 */, 6362/* 149 */, 
	6311/* 150 */, 6262/* 151 */, 6212/* 152 */, 6164/* 153 */, 6115/* 154 */, 
	6067/* 155 */, 6020/* 156 */, 5973/* 157 */, 5927/* 158 */, 5881/* 159 */, 
	5835/* 160 */
};



static void tdmb_InitTOP(void)
{
	RTV_REG_MAP_SEL(OFDM_PAGE);
    RTV_REG_SET(0x07, 0x08); 
	RTV_REG_SET(0x05, 0x17); 
	RTV_REG_SET(0x06, 0x10);	
	RTV_REG_SET(0x0A, 0x00);   
}

//============================================================================
// Name    : tdmb_InitCOMM
// Action  : MAP SEL COMM Register Init
// Input   : Chip Address
// Output  : None
//============================================================================
static void tdmb_InitCOMM(void)
{
	RTV_REG_MAP_SEL(COMM_PAGE);
	RTV_REG_SET(0x10, 0x91);	  
	RTV_REG_SET(0xE1, 0x00);
	RTV_REG_SET(0x35, 0X8B);
	RTV_REG_SET(0x3B, 0x3C);   

	RTV_REG_SET(0x36, 0x67);   
	RTV_REG_SET(0x3A, 0x0F);	   

	RTV_REG_SET(0x3C,0x20); 
	RTV_REG_SET(0x3D,0x0B);   
	RTV_REG_SET(0x3D,0x09);  
	RTV_REG_SET(0xa6,0x10);  
}

//============================================================================
// Name    : tdmb_InitHOST
// Action  : MAP SEL HOST Register Init
// Input   : Chip Address
// Output  : None
//============================================================================
static void tdmb_InitHOST(void)
{
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x10, 0x00);
	RTV_REG_SET(0x13,0x16);
	RTV_REG_SET(0x14,0x00);
	RTV_REG_SET(0x19,0x0A);
	RTV_REG_SET(0xF0,0x00);
	RTV_REG_SET(0xF1,0x00);
	RTV_REG_SET(0xF2,0x00);
	RTV_REG_SET(0xF3,0x00);
	RTV_REG_SET(0xF4,0x00);
	RTV_REG_SET(0xF5,0x00);
	RTV_REG_SET(0xF6,0x00);
	RTV_REG_SET(0xF7,0x00);
	RTV_REG_SET(0xF8,0x00);	
    RTV_REG_SET(0xFB,0xFF);  

}


//============================================================================
// Name    : tdmb_InitOFDM
// Action  : MAP SEL OFDM Register Init
// Input   : Chip Address
// Output  : None
//============================================================================
static void tdmb_InitOFDM(void)
{
	U8 INV_MODE;      
	U8 FREQ_PNCO_HL;  
	U8 FREQ_PNCO_HH;  
	U8 PWM_COM;       
	U8 WAGC_COM;      
	U8 AGC_MODE;      
	U8 POST_INIT;     
	U8 AGC_CYCLE;     

	INV_MODE = 1;		
	PWM_COM = 0x08;
	FREQ_PNCO_HL = 0x00;   
	FREQ_PNCO_HH = 0x00;
	WAGC_COM = 0x03;
	AGC_MODE = 0x06; 
	POST_INIT = 0x09;
	AGC_CYCLE = 0x10;

	RTV_REG_MAP_SEL(OFDM_PAGE);

	RTV_REG_SET(0x12,0x04); 
	
	RTV_REG_SET(0x13,0xB2); 
	RTV_REG_SET(0x14,0x07); 

	RTV_REG_SET(0x16,0x6C); 

	RTV_REG_SET(0x38,0x01);	

    RTV_REG_SET(0x20,0x5B); 

    RTV_REG_SET(0x25,0x09);

    RTV_REG_SET(0x44,0x00 | (POST_INIT)); 

	RTV_REG_SET(0x46,0xA0); 
	RTV_REG_SET(0x47,0x0F);

	RTV_REG_SET(0x48,0xB8); 
	RTV_REG_SET(0x49,0x0B);  
	RTV_REG_SET(0x54,0x58); 
	
	RTV_REG_SET(0x56,0x00 | (AGC_CYCLE));         

	RTV_REG_SET(0x59,0x51); 
                                            
	RTV_REG_SET(0x5A,0x1C); 

	RTV_REG_SET(0x6D,0x00); 
	RTV_REG_SET(0x8B,0x24); 

	RTV_REG_SET(0x6B,0x2D); 
	RTV_REG_SET(0x85,0x35); 
	RTV_REG_SET(0x8E,0x01); 

	RTV_REG_SET(0x33, 0x00 | (INV_MODE<<1)); 
	RTV_REG_SET(0x53,0x00 | (AGC_MODE)); 

	RTV_REG_SET(0x6F,0x00 | (WAGC_COM)); 
	
	RTV_REG_SET(0xBA,PWM_COM);

	switch( g_eRtvAdcClkFreqType )
	{
		case RTV_ADC_CLK_FREQ_8_MHz: 
			RTV_REG_MAP_SEL(COMM_PAGE);
			RTV_REG_SET(0x6A,0x01); 
			   
			RTV_REG_MAP_SEL(OFDM_PAGE);
			RTV_REG_SET(0x3c,0x4B); 
			RTV_REG_SET(0x3d,0x37); 
			RTV_REG_SET(0x3e,0x89); 
			RTV_REG_SET(0x3f,0x41);
			break;
			
		case RTV_ADC_CLK_FREQ_8_192_MHz: 
			RTV_REG_MAP_SEL(COMM_PAGE);
			RTV_REG_SET(0x6A,0x01); 

			RTV_REG_MAP_SEL(OFDM_PAGE);
			RTV_REG_SET(0x3c,0x00); 
			RTV_REG_SET(0x3d,0x00); 
			RTV_REG_SET(0x3e,0x00); 
			RTV_REG_SET(0x3f,0x40);
			break;
			
		case RTV_ADC_CLK_FREQ_9_MHz: 
			RTV_REG_MAP_SEL(COMM_PAGE);
			RTV_REG_SET(0x6A,0x21); 

			RTV_REG_MAP_SEL(OFDM_PAGE);
			RTV_REG_SET(0x3c,0xB5); 
			RTV_REG_SET(0x3d,0x14); 
			RTV_REG_SET(0x3e,0x41); 
			RTV_REG_SET(0x3f,0x3A);
			break;
			
		case RTV_ADC_CLK_FREQ_9_6_MHz:
			RTV_REG_MAP_SEL(COMM_PAGE);
			RTV_REG_SET(0x6A,0x31); 

			RTV_REG_MAP_SEL(OFDM_PAGE);
			RTV_REG_SET(0x3c,0x69); 
			RTV_REG_SET(0x3d,0x03); 
			RTV_REG_SET(0x3e,0x9D); 
			RTV_REG_SET(0x3f,0x36);
			break;
			
		default:
			RTV_DBGMSG0("[tdmb_InitOFDM] Upsupport ADC clock type! \n");
			break;
	}
	
	RTV_REG_SET(0x42,0x00); 
	RTV_REG_SET(0x43,0x00); 

	RTV_REG_SET(0x94,0x08); 

	RTV_REG_SET(0x98,0x05); 
	RTV_REG_SET(0x99,0x03); 
	RTV_REG_SET(0x9B,0xCF); 
	RTV_REG_SET(0x9C,0x10); 
	RTV_REG_SET(0x9D,0x1C); 
	RTV_REG_SET(0x9F,0x32); 
	RTV_REG_SET(0xA0,0x90); 

	RTV_REG_SET(0xA4,0x01); 

	RTV_REG_SET(0xA8,0xF6); 
	RTV_REG_SET(0xA9,0x89);
	RTV_REG_SET(0xAA,0x0C); 
	RTV_REG_SET(0xAB,0x32); 

	RTV_REG_SET(0xAC,0x14); 
	RTV_REG_SET(0xAD,0x09); 

	RTV_REG_SET(0xAE,0xFF); 

    RTV_REG_SET(0xEB,0x6B); 
}

//============================================================================
// Name    : tdmb_InitFEC
// Action  : MAP SEL FEC Register Init
// Input   : Chip Address
// Output  : None
//============================================================================
static void tdmb_InitFEC(void)
{
	RTV_REG_MAP_SEL(FEC_PAGE);
	RTV_REG_SET(0x80, 0x80);   
	RTV_REG_SET(0x81, 0xFF);
	RTV_REG_SET(0x87, 0x07);
	RTV_REG_SET(0x45, 0xA0);
	RTV_REG_SET(0xDD, 0xD0); 
	RTV_REG_SET(0x39, 0x07);
	RTV_REG_SET(0xE6, 0x10);    
	RTV_REG_SET(0xA5, 0xA0);
}


static void tdmb_InitDemod(void)
{
#ifdef USING_INIT_TABLE
	int nNumTblEntry = sizeof(g_atDemodInitTbl_ISDBT) / sizeof(RTV_REG_INIT_INFO);
	const RTV_REG_INIT_INFO *ptInitTbl = g_atDemodInitTbl_ISDBT;

	do
	{
		RTV_REG_SET(ptInitTbl->bReg, ptInitTbl->bVal);
		ptInitTbl++;						
	} while( --nNumTblEntry );

#else
	tdmb_InitTOP();
	tdmb_InitCOMM();
	tdmb_InitHOST();
#endif	
	tdmb_InitOFDM();
	tdmb_InitFEC();	

	rtvOEM_ConfigureInterrupt(); 
	rtvOEM_ConfigureHostIF();
}




static void tdmb_SoftReset(void)
{
	RTV_REG_MAP_SEL(OFDM_PAGE);
	RTV_REG_SET(0x10, 0x48); // FEC reset enable
	RTV_DELAY_MS(10);
	RTV_REG_SET(0x10, 0xC9); // OFDM & FEC Soft reset
}



//============================================================================
// Name    : Set_Path_Video_SUBCH
// Action  : A/V service setting.
// Input   : Chip Address, subch num, on_off
// Output  : None
//============================================================================
static INLINE void tdmb_Set_MSC1_SUBCH0(UINT nSubChID, BOOL fSubChEnable, BOOL fRsEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);	
	RTV_REG_SET(0x3A, (fSubChEnable << 7) | (fRsEnable << 6) | nSubChID);
}


void tdmb_Set_MSC1_SUBCH1(UINT nSubChID, BOOL fSubChEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);	
	RTV_REG_SET(0x3B, (fSubChEnable << 7) | nSubChID);
}


//......



void rtvTDMB_StandbyMode(int on)
{
	if( on )
	{ 
		RTV_REG_MAP_SEL(RF_PAGE); 
		RTV_REG_MASK_SET(0x57,0x04, 0x04);  //SW PD ALL      
	}
	else
	{	  
		RTV_REG_MAP_SEL(RF_PAGE); 
		RTV_REG_MASK_SET(0x57,0x04, 0x00);  //SW PD ALL	
	}
}


UINT rtvTDMB_GetLockStatus(void)
{
	U8 lock_stat;
	UINT lock_st = 0;
	
   	if(g_fRtvChannelChange) 
   	{
   		RTV_DBGMSG0("[rtvTDMB_GetLockStatus] RTV Freqency change state! \n");	
		return 0x0;	 
	}
		
    RTV_REG_MAP_SEL(DD_PAGE);
	lock_stat = RTV_REG_GET(0x37);	
	if( lock_stat & 0x01 )
        lock_st = RTV_TDMB_OFDM_LOCK_MASK;

    RTV_REG_MAP_SEL(FEC_PAGE);
	lock_stat = RTV_REG_GET(0xFB);	
	if((lock_stat & 0x03) == 0x03)
        lock_st |= RTV_TDMB_FEC_LOCK_MASK;

	return lock_st;
}


U32 rtvTDMB_GetPER(void)
{
    U8 rdata0, rdata1, rs_sync;

	if(g_fRtvChannelChange) 
	{
		RTV_DBGMSG0("[rtvTDMB_GetPER] RTV Freqency change state! \n");
		return 0;	 
	}	
    
   	RTV_REG_MAP_SEL(FEC_PAGE);
	rdata0 = RTV_REG_GET(0xD7);

	rs_sync = (rdata0 & 0x08) >> 3;
	if(rs_sync != 0x01)
		return 0;	

	rdata1 = RTV_REG_GET(0xB4);
	rdata0 = RTV_REG_GET(0xB5);
	
	return  ((rdata1 << 8) | rdata0);
}


S32 rtvTDMB_GetRSSI(void) 
{
	U8 RD00;
	S32 nRssi;
	
	if(g_fRtvChannelChange) 
	{
		RTV_DBGMSG0("[rtvTDMB_GetRSSI] RTV Freqency change state! \n");
		return 0;	 
	}	

	RTV_REG_MAP_SEL(RF_PAGE);	
	RD00 = RTV_REG_GET(0x00); 

	nRssi = -( (((RD00 & 0x30) >> 4) * (S32)(12*RTV_TDMB_RSSI_DIVIDER)) 
			+ ((RD00 & 0x0F) * (S32)(2.8*RTV_TDMB_RSSI_DIVIDER))	
			+ (( (RTV_REG_GET(0x02) & 0x1E) >> 1 ) * (S32)(2.7*RTV_TDMB_RSSI_DIVIDER)) 
			+ ((RTV_REG_GET(0x04) &0x7F) * (S32)(0.4*RTV_TDMB_RSSI_DIVIDER) ) - (S32)(10*RTV_TDMB_RSSI_DIVIDER) ); 

	 if((RD00 & 0xC0) == 0x40)
		 nRssi -= 5;

	return  nRssi;
}


U32 rtvTDMB_GetCNR(void)
{
	U8 data1=0, data2=0;
	U8 data=0;
	S32 SNR=0; 

	if(g_fRtvChannelChange) 
	{
		RTV_DBGMSG0("[rtvTDMB_GetCNR] RTV Freqency change state! \n");
		return 0;	 
	}	

	RTV_REG_MAP_SEL(OFDM_PAGE); 

	RTV_REG_SET(0x1C, 0x10); 

	data1 = RTV_REG_GET(0x26);
	data2 = RTV_REG_GET(0x27);

	RTV_REG_SET(0x82, 0x01);	
	data1 = RTV_REG_GET(0x7E);
	data2 = RTV_REG_GET(0x7F);

	data = ((data2 & 0x1f) << 8) + data1;
	
	if(data == 0)
	{
		return 0;
	}
	else if((data > 0) && (data < 15))
	{
		SNR = (S32)(33 * RTV_TDMB_CNR_DIVIDER);
	}
	else if((data >= 15) && (data <= 160))
	{
		SNR = g_awSNR_15_160[data-15];
	}
	else if(data > 160)
	{
		SNR = (S32)(5.44 * RTV_TDMB_CNR_DIVIDER);
	}

	return SNR;
}


U32 rtvTDMB_GetBER(void)
{
	U8 rdata0=0, rdata1=0, rdata2=0;
	U8 rcnt0, rcnt1, rcnt2;
	U8 rs_sync;
	U32 val;
	U8 cnt;

	if(g_fRtvChannelChange) 
	{
		RTV_DBGMSG0("[rtvTDMB_GetPER] RTV Freqency change state! \n");
		return 0;	 
	}	

	RTV_REG_MAP_SEL(FEC_PAGE);
	rdata0 = RTV_REG_GET(0xD7);

	rs_sync = (rdata0 & 0x08) >> 3;
	if(rs_sync != 0x01)
		return 0;
	
	rcnt2 = RTV_REG_GET(0xA6);
	rcnt1 = RTV_REG_GET(0xA7);
	rcnt0 = RTV_REG_GET(0xA8);
	cnt = (rcnt2 << 16) | (rcnt1 << 8) | rcnt0;

	rdata2 = RTV_REG_GET(0xA9);
	rdata1 = RTV_REG_GET(0xAA);
	rdata0 = RTV_REG_GET(0xAB);
	val = (rdata2 << 16) | (rdata1 << 8) | rdata0; // max 24 bit

	if(cnt == 0)
		return 0;
	else
		return ((val * (U32)RTV_TDMB_BER_DIVIDER) / cnt);
}


void rtvTDMB_ConfigureFIC(void)
{
	/* Disable the FIC interrupt. */
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x62, 0xFF);
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)
    TSIF_Disable();
#endif

	rtv_ResetMemoryFIC();
	rtv_SetupMemoryFIC();

	/* Enable the FIC interrupt. */
	RTV_REG_MAP_SEL(HOST_PAGE);
	//RTV_REG_SET(0x62, (U8)~0x01);
    RTV_REG_SET(0x62, (U8)0xFE);

#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)
    FIC_TSIF_Enable();
#endif
}


/*
2010/08/27: 현재는 MSC1만 사용하는 것으로 결정! */

static U32 g_dwPrevChFreqKHz = 0x0;


INT rtvTDMB_SetSubChannel(UINT nSubChID, E_RTV_TDMB_SERVICE_TYPE eServiceType, U32 dwChFreqKHz)
{
	INT nRet;
		
	/* Disable all interrupts. (MSC0, MSC1, FIC) */
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x62, 0xFF);

#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)
    TSIF_Disable();
#endif
	
	if(g_dwPrevChFreqKHz != dwChFreqKHz)
	{
		if((nRet=rtvRF_SetFrequency(RTV_TV_MODE_TDMB, 0, dwChFreqKHz)) != RTV_SUCCESS)
			return nRet;
		
		g_dwPrevChFreqKHz = dwChFreqKHz;
	}
	
	// Check the use of MSC0 or MSC1 ?
	
	switch( eServiceType )
	{
		case RTV_TDMB_SERVICE_VIDEO:
			rtv_ResetMemoryMSC1();
			tdmb_Set_MSC1_SUBCH0(nSubChID, TRUE, TRUE);
			rtv_ConfigureMemeoryMSC1(RTV_TV_MODE_TDMB, RTV_MAX_VIDEO_TSP_SIZE);
			break;
			
		case RTV_TDMB_SERVICE_AUDIO:
			rtv_ResetMemoryMSC1();
			tdmb_Set_MSC1_SUBCH0(nSubChID, TRUE, TRUE);
			rtv_ConfigureMemeoryMSC1(RTV_TV_MODE_TDMB, RTV_MAX_AUDIO_TSP_SIZE);			   			
			break;
		
		case RTV_TDMB_SERVICE_DATA:
			rtv_ResetMemoryMSC1();			
			tdmb_Set_MSC1_SUBCH0(nSubChID, TRUE, TRUE);
			rtv_ConfigureMemeoryMSC1(RTV_TV_MODE_TDMB, RTV_MAX_DATA_TSP_SIZE);			
			break;
	}

	/* Only MSC1 interrupt. */
	//RTV_REG_MASK_SET(0x62, 0x70, 0x00); 
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x62, (U8)~0x70);//0x8F); 

#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)
    MSC1_TSIF_Enable();
#endif


	return RTV_SUCCESS;
}



INT rtvTDMB_ScanFrequency(U32 dwChFreqKHz)
{
	INT nRet;
	U8 scan_done, OFDM_L=0, ccnt = 0, NULL_C=0, SCV_C=0;
	U8 scan_pwr1=0, scan_pwr2=0, DAB_Mode=0xFF, DAB_Mode_Chk=0xFF;
	U8 pre_agc1=0, pre_agc2=0, pre_agc_mon=0, ASCV=0;
	INT scan_flag = 0;
	U16 SPower =0, PreGain=0, PreGainTH=0, PWR_TH = 0, ILoopTH =0; 
	U8 Cfreq_HTH = 0,Cfreq_LTH=0;
	U8 i=0,j=0, m=0,n=0;
	U8 varyLow=0,varyHigh=0;
	U16 varyMon=0;
	U8 MON_FSM=0, FsmCntChk=0;
	U8 test0=0,test1=0;
	U16 NullLengthMon=0;
	U16 fail = 0;
	U8  FecResetCh=0xff;
	U8 FEC_SYNC=0xFF,CoarseFreq=0xFF, NullTh=0xFF,NullChCnt=0;	
	U8 rdata0 =0, rdata1=0;
	U16 i_chk=0, q_chk=0;	

	/* Disable all interrupts. (MSC0, MSC1, FIC) */
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x62, 0xFF);
	
//	RTV_INT_ENABLE(FALSE);	//? MSC0, MSC1, FIC ? all ?
//	g_fRtvChannelChange = TRUE;

	if((nRet=rtvRF_SetFrequency(RTV_TV_MODE_TDMB, 0, dwChFreqKHz)) != RTV_SUCCESS)
		return nRet;

	RTV_REG_MAP_SEL(OFDM_PAGE);
	RTV_REG_SET( 0x54, 0x70); 

	tdmb_SoftReset();
		
	FecResetCh = 0xff;
	fail = 0xFFFF;

	while(1)
	{			
		RTV_REG_MAP_SEL(OFDM_PAGE);
		scan_done = RTV_REG_GET(0xCF); // Scan-done flag & scan-out flag check

		RTV_REG_MAP_SEL(COMM_PAGE);		    // Scan-Power monitoring
		scan_pwr1 = RTV_REG_GET(0x38);
		scan_pwr2 = RTV_REG_GET(0x39);
		SPower = (scan_pwr2<<8)|scan_pwr1;

		RTV_REG_MAP_SEL(OFDM_PAGE);

		if(scan_done != 0xff)
		{
			NULL_C = 0;
			SCV_C = 0;
			pre_agc_mon = RTV_REG_GET(0x53);
			RTV_REG_SET(0x53, (pre_agc_mon | 0x80));		// Pre-AGC Gain monitoring One-shot
			pre_agc1 = RTV_REG_GET(0x66);
			pre_agc2 = RTV_REG_GET(0x67);
			PreGain = (pre_agc2<<2)|(pre_agc1&0x03);

			DAB_Mode = RTV_REG_GET(0x27);	// DAB TX Mode monitoring
			DAB_Mode = (DAB_Mode & 0x30)>>4;

			switch( g_eRtvAdcClkFreqType )
			{
				case RTV_ADC_CLK_FREQ_8_MHz :
					PreGainTH = 405;					
					switch(DAB_Mode) // tr mode
					{
				              case 0:
							PWR_TH = 2400;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
						case 1:
							PWR_TH = 2000;
							ILoopTH = 180;
							Cfreq_HTH = 242;
							Cfreq_LTH = 14;
							break;
						case 2:
							PWR_TH = 1300;
							ILoopTH = 180;
							Cfreq_HTH = 248;
							Cfreq_LTH = 8;
							break;
						case 3:
							PWR_TH = 280;
							ILoopTH = 180;
							Cfreq_HTH = 230;
							Cfreq_LTH = 26;
							break;
						default:
							PWR_TH = 2400;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
					}
					break;

				case RTV_ADC_CLK_FREQ_8_192_MHz :
					PreGainTH = 405;
					
					switch(DAB_Mode)
					{
		                               case 0:
							PWR_TH = 1700;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
						case 1:
							PWR_TH = 1500;
							ILoopTH = 180;
							Cfreq_HTH = 242;
							Cfreq_LTH = 14;
							break;
						case 2:
							PWR_TH = 1200;
							ILoopTH = 180;
							Cfreq_HTH = 248;
							Cfreq_LTH = 8;
							break;
						case 3:
							PWR_TH = 1900;
							ILoopTH = 180;
							Cfreq_HTH = 230;
							Cfreq_LTH = 26;
							break;
						default:
							PWR_TH = 1700;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
					}				
					break;

				case RTV_ADC_CLK_FREQ_9_MHz :
					PreGainTH = 380;
					switch(DAB_Mode)
					{
		                                case 0:
							PWR_TH = 7000;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
						case 1:
							PWR_TH = 5000;
							ILoopTH = 180;
							Cfreq_HTH = 242;
							Cfreq_LTH = 14;
							break;
						case 2:
							PWR_TH = 1300;
							ILoopTH = 180;
							Cfreq_HTH = 248;
							Cfreq_LTH = 8;
							break;
						case 3:
							PWR_TH = 8000;
							ILoopTH = 180;
							Cfreq_HTH = 230;
							Cfreq_LTH = 26;
							break;
						default:
							PWR_TH = 8000;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
					}
					break;

				case RTV_ADC_CLK_FREQ_9_6_MHz :
					PreGainTH = 380;
					
					switch(DAB_Mode)
					{
		                                case 0:
							PWR_TH = 7000;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
						case 1:
							PWR_TH = 5000;
							ILoopTH = 180;
							Cfreq_HTH = 242;
							Cfreq_LTH = 14;
							break;
						case 2:
							PWR_TH = 1300;
							ILoopTH = 180;
							Cfreq_HTH = 248;
							Cfreq_LTH = 8;
							break;
						case 3:
							PWR_TH = 8000;
							ILoopTH = 180;
							Cfreq_HTH = 230;
							Cfreq_LTH = 26;
							break;
						default:
							PWR_TH = 8000;
							ILoopTH = 200;
							Cfreq_HTH = 206;
							Cfreq_LTH = 55;
							break;
					}
				
					break;

				default:
					return RTV_UNSUPPORT_ADC_CLK;
			}

			if(scan_done == 0x01)			 /* Not DAB signal channel */
			{
				scan_flag = RTV_CHANNEL_NOT_DETECTED;
				fail = 0xFF01;	
				
				return scan_flag;		/* Auto-scan result return */
			}
			else if(scan_done == 0x03)	/* DAB signal channel */
			{
				RTV_REG_MAP_SEL(OFDM_PAGE); 
				CoarseFreq = RTV_REG_GET( 0x18);	/* coarse freq */
						
#if defined(RTV_CONUTRY_KOREA)
				if(DAB_Mode > 0)   /* Tr_mode detection miss for T-DMB [Only Tr_Mode 1인 경우에만 사용가능한 조건] */
				{
					scan_flag = RTV_CHANNEL_NOT_DETECTED;
					fail = 0xF002;
					return scan_flag;	 /* Auto-scan result return  */
				}
#endif
			
				if((CoarseFreq	< Cfreq_HTH) && (CoarseFreq  > Cfreq_LTH))
				{
					CoarseFreq = 0x33;					
					scan_flag = RTV_CHANNEL_NOT_DETECTED;
					fail = 0xFF33;	
					return scan_flag;					
				}	
				
				if(SPower<PWR_TH)  /* Scan Power Threshold	*/
				{		
					scan_flag = RTV_CHANNEL_NOT_DETECTED;
					fail = 0xFF03;	
					return scan_flag;  
				}
				else
				{
					if ((PreGain < PreGainTH)||(PreGain==0))   /* PreAGC Gain threshold check */
					{
						scan_flag = RTV_CHANNEL_NOT_DETECTED;
						fail = 0xFF04;	 	
						return scan_flag;
					}
					else
					{
						for(m =0; m<16; m++)
						{
							NullTh = RTV_REG_GET( 0x1C);
							RTV_REG_SET( 0x1C, (NullTh | 0x10));	
							test0 = RTV_REG_GET( 0x26);
							test1 = RTV_REG_GET( 0x27);
							NullLengthMon = ((test1&0x0F)<<8)|test0;
							
							DAB_Mode_Chk = RTV_REG_GET( 0x27);	 /* DAB TX Mode monitoring */
							DAB_Mode_Chk = (DAB_Mode_Chk & 0x30)>>4;				
							if(DAB_Mode != DAB_Mode_Chk)
							{
								scan_flag = RTV_CHANNEL_NOT_DETECTED;
								fail = 0xF000;	
								return scan_flag; 
							}
							
							if((NullLengthMon == 0) || (NullLengthMon > 3000))
							{
								NullChCnt++;
							}
							if((NullChCnt > 10) && (m > 14)&& (PreGain < 400))	
							{
								scan_flag = RTV_CHANNEL_NOT_DETECTED;
								fail = 0xFF05;	
								return scan_flag;							
							}
							else if(m>14)
							{
								fail = 0x1111;	
								scan_flag=RTV_SUCCESS;
								break;
							}							
							RTV_DELAY_MS(10);	/* 10ms로 맞춰줘야 함 */
						}
					}
					if(scan_flag == RTV_SUCCESS)
					{						
						for(i=0; i<ILoopTH; i++)
						{
							RTV_DELAY_MS(10);	/* 10ms로 맞춰줘야 함 */
							
							RTV_REG_MAP_SEL(OFDM_PAGE);
							ASCV = RTV_REG_GET( 0x30);
							ASCV = ASCV&0x0F;											
							if((SCV_C > 0) && (ASCV > 7))		  // ASCV count
							{
								scan_flag = RTV_CHANNEL_NOT_DETECTED;
								fail = 0xFF08;	
								return scan_flag;  /* Auto-scan result return */
							}								
							if(ASCV > 7)
							{
								SCV_C++;		
							}						
							
							DAB_Mode_Chk = RTV_REG_GET( 0x27);	 /* DAB TX Mode monitoring */
							DAB_Mode_Chk = (DAB_Mode_Chk & 0x30)>>4;				
							if(DAB_Mode != DAB_Mode_Chk)
							{
								scan_flag = RTV_CHANNEL_NOT_DETECTED;
								fail = 0xF100;	
								return scan_flag; 
							}							

							RTV_REG_MAP_SEL( COMM_PAGE); 
							RTV_REG_MASK_SET(0x4D, 0x04, 0x00); 
							RTV_REG_MASK_SET(0x4D, 0x04, 0x04); 
							rdata0 = RTV_REG_GET( 0x4E);
							rdata1 = RTV_REG_GET( 0x4F);
							i_chk = (rdata1 << 8) + rdata0;
							
							rdata0 = RTV_REG_GET( 0x50);
							rdata1 = RTV_REG_GET( 0x51);
							q_chk = (rdata1 << 8) + rdata0; 
							if((((i_chk>5) && (i_chk<65530)) || ((q_chk>5) && (q_chk<65530))) && (PreGain<500))
							{
								scan_flag = RTV_CHANNEL_NOT_DETECTED;
								fail = 0xF200;	
								return scan_flag; 
							}

							/* //////////////////////// FSM Monitoring check//////////////////////////////// */
							RTV_REG_MAP_SEL(OFDM_PAGE); 
							MON_FSM = RTV_REG_GET( 0x37);
							MON_FSM = MON_FSM & 0x07;

							if((MON_FSM == 1) && (PreGain < 400))	
							{
								FsmCntChk++;
								if(NullChCnt > 14)
									FsmCntChk += 3;
							}
							if((MON_FSM == 1) && (FsmCntChk > 9) && (ccnt < 2))
							{
								scan_flag = RTV_CHANNEL_NOT_DETECTED;									
								fail = 0xFF0A;	
								FsmCntChk = 0;
								
								return scan_flag;	/* Auto-scan result return */
							}	
							/* /////////////////////////////////////////////////////////////////////////////// */
							/* ///////////////////////// Coarse Freq. check/////////////////////////////////// */
							/* /////////////////////////////////////////////////////////////////////////////// */
							ccnt = RTV_REG_GET( 0x17);	/* Coarse count check */
							ccnt &= 0x1F;
							if(ccnt > 1)
							{
								for(j=0;j<25;j++)
								{
									RTV_DELAY_MS(10);	/* 5ms로 맞춰줘야 함 */
									RTV_REG_MAP_SEL( OFDM_PAGE);	
									OFDM_L = RTV_REG_GET( 0x12);
									RTV_REG_MASK_SET(0x82, 0x01, 0x01);	
									varyLow = RTV_REG_GET( 0x7E);
									varyHigh = RTV_REG_GET( 0x7F);			   
									varyMon = ((varyHigh & 0x1f) << 8) + varyLow; 
									if((OFDM_L&0x80) && (varyMon > 0))
									{
										RTV_REG_MAP_SEL(OFDM_PAGE);
										RTV_REG_SET(0x54,0x58); 
										break;
									}
								}
								if(OFDM_L&0x80)
								{
									RTV_REG_MAP_SEL( FEC_PAGE);
									FEC_SYNC = RTV_REG_GET( 0xFB);
									FEC_SYNC = FEC_SYNC & 0x03;
									if(FEC_SYNC == 0x03)
									{
										scan_flag = RTV_SUCCESS;	   /* OFDM_Lock & FEC_Sync OK */
										fail = 0xFF70;	
										return scan_flag;
									}
									else if(FEC_SYNC == 0x02)
									{
										for(n=0;n<25;n++)
										{
											RTV_DELAY_MS(10);	/* 5ms로 맞춰줘야 함 */
											RTV_REG_MAP_SEL( FEC_PAGE);
											FEC_SYNC = RTV_REG_GET( 0xFB);
											FEC_SYNC = FEC_SYNC & 0x03;
											RTV_REG_MAP_SEL( OFDM_PAGE);
											OFDM_L = RTV_REG_GET( 0x12);
											OFDM_L = OFDM_L & 0x80;
											if(OFDM_L == 0x80)
											{
												if(FEC_SYNC == 0x03)
												{
													scan_flag = RTV_SUCCESS;
													fail = 0xFF71;	  /* FEC_Sync OK */ 								
													return scan_flag;
												}
												else if(FEC_SYNC == 0x02)
												{
													scan_flag = RTV_SUCCESS;
													fail = 0xFF72;	  /* FEC_Sync miss */
												}
												else
												{
													scan_flag = RTV_CHANNEL_NOT_DETECTED;
													fail = 0xFF73;
												}
											}
											else
											{
												scan_flag = RTV_CHANNEL_NOT_DETECTED;
												fail = 0xFF74;
												return scan_flag;						 /* Auto-scan result return */
											}
										}
									}
									else
									{
										fail = 0xFF75;
										scan_flag = RTV_CHANNEL_NOT_DETECTED;	   /* OFDM_Lock & FEC_Miss */
									}
								}
								else
								{
									scan_flag = RTV_CHANNEL_NOT_DETECTED;	   /* OFDM_Unlock */
									fail = 0xFF0B;	
								}		
								return scan_flag;						 /* Auto-scan result return */
							}
							else
							{
								scan_flag = RTV_CHANNEL_NOT_DETECTED;	
							}
						}
						fail = 0xFF0C;	
						scan_flag = RTV_CHANNEL_NOT_DETECTED;
						return scan_flag;
					}
				}
			}
		}		
	}

	fail = 0xFF0D;	
	return scan_flag;	  /* Auto-scan result return */
}



INT rtvTDMB_SetFrequency(U32 dwChFreqKHz)
{
	INT nRet;
	
    RTV_INT_ENABLE(FALSE);
    
	nRet = rtvRF_SetFrequency(RTV_TV_MODE_TDMB, 0, dwChFreqKHz);
	
	RTV_INT_ENABLE(TRUE);
	
	return nRet;
}


INT rtvTDMB_Initialize(E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType)
{
	INT nRet;
		
	nRet = rtv_InitSystem(RTV_TV_MODE_TDMB, eAdcClkFreqType);
	if(nRet != RTV_SUCCESS)
		return nRet;

	tdmb_InitDemod();
		
	nRet = rtvRF_Initilize(RTV_TV_MODE_TDMB);
	if(nRet != RTV_SUCCESS)
		return nRet;
	
	return RTV_SUCCESS;
}
#endif




