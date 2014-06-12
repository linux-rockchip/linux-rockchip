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
* TITLE 	  : RAONTECH TV FM services source file. 
*
* FILENAME    : raontv_fm.c
*
* DESCRIPTION : 
*		Library of routines to initialize, and operate on, the RAONTECH FM demod.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 10/01/2010  Ko, Kevin        Modified scan function prototype.
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


#ifdef RTV_FM_ENABLE

#undef OFDM_PAGE
#define OFDM_PAGE	0x6

#undef FEC_PAGE
#define FEC_PAGE	0x09


static void fm_InitDemod_TOP(void)
{
	RTV_REG_MAP_SEL(FM_PAGE);  
    RTV_REG_SET(0x07, 0x08);
	RTV_REG_SET(0x05, 0x17); 
	RTV_REG_SET(0x06, 0x10);	
	RTV_REG_SET(0x0A, 0x00);   
}


/*===============================================================================
 * fm_InitDemod_HOST
 *
 * DESCRIPTION : 
 *		This function sets HOST page. 
 *		
 *
 * ARGUMENTS : none.
 * RETURN VALUE : none.
 *============================================================================*/
static void fm_InitDemod_HOST(void)
{
	RTV_REG_MAP_SEL(HOST_PAGE);

	RTV_REG_SET(0x10,0x00);
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


static void fm_InitDemod(void)
{
	fm_InitDemod_TOP();
	fm_InitDemod_HOST();

	RTV_REG_MAP_SEL(COMM_PAGE);
	RTV_REG_SET(0x6A, 0x0D);

	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x2B, 0x03);	
	RTV_REG_SET(0x30, 0x80);

	if(g_eRtvAdcClkFreqType == RTV_ADC_CLK_FREQ_8_MHz)
	{
		RTV_REG_SET(0xc8, 0x03);
	} 
	
	RTV_REG_SET(0x06, 0x04);

	RTV_REG_MAP_SEL(FM_PAGE);
	RTV_REG_SET(0x33, 0x02);
	RTV_REG_SET(0x3F, 0x10);
	RTV_REG_SET(0x44, 0x09); 
	
	RTV_REG_SET(0x45, 0x14); 
	RTV_REG_SET(0x46, 0x50);
	RTV_REG_SET(0x47, 0x7A);
	RTV_REG_SET(0x48, 0xD3); 
	RTV_REG_SET(0x49, 0xF3);
	

	  
	RTV_REG_SET(0x6D, 0x88);
	RTV_REG_SET(0xD2, 0x70);
	RTV_REG_SET(0xD3, 0x6A);
	RTV_REG_SET(0xD4, 0xBC);	
	RTV_REG_SET(0xD5, 0x74);
	RTV_REG_SET(0xD6, 0x13);
	RTV_REG_SET(0xF8, 0x91);
	
	RTV_REG_SET(0x54, 0x88);
	RTV_REG_SET(0x55, 0x06);
	RTV_REG_SET(0x56, 0x06);
	RTV_REG_SET(0x59, 0x51);
	RTV_REG_SET(0x5A, 0x1C);
	RTV_REG_SET(0x38, 0x00);	

	if(g_eRtvAdcClkFreqType == RTV_ADC_CLK_FREQ_8_MHz)
	{
		RTV_REG_SET(0x42, 0x00);		 
		RTV_REG_SET(0x43, 0x10);	
	}
	else if(g_eRtvAdcClkFreqType == RTV_ADC_CLK_FREQ_8_192_MHz)
	{
		RTV_REG_SET(0x42, 0xA0);		 
		RTV_REG_SET(0x43, 0x0F);	
	}
	RTV_REG_SET(0x53, 0x06);		
	RTV_REG_SET(0x85, 0x3A);
	RTV_REG_SET(0xBA, 0x08);
	
	RTV_REG_SET(0xD2, 0x80);
	RTV_REG_SET(0xE5, 0x07);
	RTV_REG_SET(0xE7, 0x2B);
	RTV_REG_SET(0xE8, 0x01);
	RTV_REG_SET(0xE9, 0x01);
	RTV_REG_SET(0xF5, 0x60);
	RTV_REG_SET(0xF9, 0x20);
	RTV_REG_SET(0xD7, 0x4F); 
	
	RTV_REG_SET(0xF0, 0x41); 
	RTV_REG_SET(0xF1, 0xE4); 
	RTV_REG_SET(0xF3, 0x01);
	RTV_REG_SET(0xF4, 0x20);
	
	RTV_REG_SET(0xE5, 0x07);
	RTV_REG_SET(0xF6, 0xA7); 
	RTV_REG_SET(0xED, 0x01);

	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x7D, 0x05);
	
	rtvOEM_ConfigureInterrupt(); 
	rtv_ConfigureMemeoryMSC1(RTV_TV_MODE_FM, RTV_MAX_TSP_SIZE);
	rtvOEM_ConfigureHostIF();

	RTV_REG_MAP_SEL(FM_PAGE);
	RTV_REG_SET(0x10, 0x48); 
	RTV_DELAY_MS(10);  // 10ms Delay
	RTV_REG_SET(0x10, 0xC9);
	RTV_DELAY_MS(100);
}


void rtvFM_StandbyMode(int on)
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


void rtvFM_GetLockStatus(UINT *pLockVal, UINT *pLockCnt)
{
	U8 Data0;

	*pLockVal = 0;
	*pLockCnt = 0;
	
	RTV_REG_MAP_SEL(FM_PAGE);
	
	Data0 = RTV_REG_GET(0xEF);
	if( Data0 & 0x01 )
		*pLockVal = RTV_FM_PILOT_LOCK_MASK;		
	
	Data0 = RTV_REG_GET(0xDB);
	if( Data0 & 0x01 )
		*pLockVal |= RTV_FM_RDS_LOCK_MASK;		

	*pLockCnt = RTV_REG_GET(0xEE);
}


#define RTV_FM_RSSI_DIVIDER		10.0

INT rtvFM_ScanFrequency(U32 *pChBuf, UINT nNumChBuf, U32 dwStartFreqKHz, U32 dwEndFreqKHz)
{
	U32 fm_frequency = dwStartFreqKHz;
	S32 RF_RSSI=0;
	S32 FM_DC=0,dc_sum=0, dc_buff[3] = {0,};
	UINT i=0,j=0;
	U32 freq_buff[3] = {0,};
	U8 FM_DC_Value_D1=0, FM_DC_Value_D2=0, FM_DC_Value_D3=0;
	unsigned char RD00 = 0;
	INT nScanCnt=0;
		
	if((dwStartFreqKHz<RTV_FM_CH_MIN_FREQ_KHz) || (dwStartFreqKHz>RTV_FM_CH_MAX_FREQ_KHz))
	{
		RTV_DBGMSG2("[rtvFM_SetFrequency] dwStartFreqKHz must be in the (%dKHz ~ %dKHz)", RTV_FM_CH_MIN_FREQ_KHz, RTV_FM_CH_MAX_FREQ_KHz);
		return RTV_INVAILD_FREQ;
	}
	
	if((dwEndFreqKHz<RTV_FM_CH_MIN_FREQ_KHz) || (dwEndFreqKHz>RTV_FM_CH_MAX_FREQ_KHz))
	{
		RTV_DBGMSG2("[rtvFM_SetFrequency] dwEndFreqKHz must be in the (%dKHz ~ %dKHz)", RTV_FM_CH_MIN_FREQ_KHz, RTV_FM_CH_MAX_FREQ_KHz);
		return RTV_INVAILD_FREQ;
	}
	
	if((dwStartFreqKHz % RTV_FM_CH_STEP_FREQ_KHz) != 0) 
	{
		RTV_DBGMSG1("[rtvFM_SetFrequency] The step of dwStartFreqKHz must be %dKHz", RTV_FM_CH_STEP_FREQ_KHz);
		return RTV_INVAILD_FREQ; 
	}
	
	if((dwEndFreqKHz % RTV_FM_CH_STEP_FREQ_KHz) != 0) 
	{
		RTV_DBGMSG1("[rtvFM_SetFrequency] The step of dwEndFreqKHz must be %dKHz", RTV_FM_CH_STEP_FREQ_KHz);
		return RTV_INVAILD_FREQ;
	}
			
	g_fRtvChannelChange = TRUE;
	
	RTV_REG_MAP_SEL(FM_PAGE);
	RTV_REG_SET(0xF1, 0xE4);

	while(1)
	{     
		if((nScanCnt=rtvRF_SetFrequency(RTV_TV_MODE_FM, 0, fm_frequency)) != RTV_SUCCESS)
			goto FM_SCAN_FREQ_EXIT;

		RTV_REG_MAP_SEL(FM_PAGE);
		RTV_REG_SET(0x10, 0x48); 
		RTV_DELAY_MS(10);  // 10ms Delay
		
		RTV_REG_SET(0x10, 0xC9);
	    RTV_DELAY_MS(25); // 25ms Delay

		for(i=0; i<3; i++)
		{
		    RTV_DELAY_MS(2);  // 2ms Delay

			// Get the DC value.
			RTV_REG_MAP_SEL(FM_PAGE);
			RTV_REG_SET(0xF7, 0x03); // FM Variance, DC value display start
			FM_DC_Value_D1 = RTV_REG_GET(0xFC);
			FM_DC_Value_D2 = RTV_REG_GET(0xFD);
			FM_DC_Value_D3 = RTV_REG_GET(0xFE);
			FM_DC = ((FM_DC_Value_D3 & 0x1F) << 16) | (FM_DC_Value_D2 << 8) | FM_DC_Value_D1;

			FM_DC = FM_DC/2048;

			if(0x200&FM_DC)
			{		
				FM_DC=0xFFFFFE00 | FM_DC;
			}

			dc_sum += FM_DC;
		}

		freq_buff[j] = fm_frequency;
		dc_buff[j] = dc_sum;
		dc_sum = 0;

		RTV_REG_MAP_SEL(RF_PAGE);
		RD00 = RTV_REG_GET(0x00);

		RF_RSSI = -((((RD00 & 0x30) >> 4) * (S32)(12*RTV_FM_RSSI_DIVIDER) ) + ((RD00 & 0x0F) * (S32)(3 *RTV_FM_RSSI_DIVIDER) )  + (( (RTV_REG_GET(0x02) & 0x1E) >> 1 ) * (S32)(3*RTV_FM_RSSI_DIVIDER) ) + ((RTV_REG_GET(0x04) &0x7F) * (S32)(0.4*RTV_FM_RSSI_DIVIDER) )); 
		if((RD00&0xC0) == 0x40) 
			RF_RSSI += (S32)(30 * RTV_FM_RSSI_DIVIDER);
		else
			RF_RSSI += (S32)(20 * RTV_FM_RSSI_DIVIDER);  //Sens Mode, Normal Mode

		if (j==2)
		{
			if((dc_buff[0]<-200) && (ABS(dc_buff[1])<200) && (dc_buff[2]>200) && (RF_RSSI > (S32)(-80*RTV_FM_RSSI_DIVIDER)))
			{
				if(++nScanCnt > nNumChBuf)
				{
					nScanCnt = RTV_INSUFFICIENT_CHANNEL_BUF;
					goto FM_SCAN_FREQ_EXIT;
				}

				*pChBuf++ = freq_buff[1];  //Added FM CHANNEL OK List.				
			}
			
			freq_buff[0] = freq_buff[1];
			freq_buff[1] = freq_buff[2];
			dc_buff[0] = dc_buff[1];
			dc_buff[1] = dc_buff[2];
			j=2;
		}
		else j++;
		
		if(fm_frequency == dwEndFreqKHz)
			break;
			
		if(fm_frequency == RTV_FM_CH_MAX_FREQ_KHz)
			fm_frequency = RTV_FM_CH_MIN_FREQ_KHz;
		else
			fm_frequency += RTV_FM_CH_STEP_FREQ_KHz;	
	}
	
FM_SCAN_FREQ_EXIT:
	RTV_REG_MAP_SEL(FM_PAGE);
	RTV_REG_SET(0xF1, 0xCC);	
	
	g_fRtvChannelChange = FALSE;
	
	return nScanCnt;
}



INT rtvFM_SetFrequency(U32 dwChFreqKHz)
{
	INT nRet;
	INT Pilot_Lock;
	
	if((dwChFreqKHz<RTV_FM_CH_MIN_FREQ_KHz) || (dwChFreqKHz>RTV_FM_CH_MAX_FREQ_KHz))
	{
		RTV_DBGMSG2("[rtvFM_SetFrequency] dwChFreqKHz must be in the (%dKHz ~ %dKHz)", RTV_FM_CH_MIN_FREQ_KHz, RTV_FM_CH_MAX_FREQ_KHz);
		return RTV_INVAILD_FREQ;
	}

	if((dwChFreqKHz % RTV_FM_CH_STEP_FREQ_KHz) != 0)
	{
		RTV_DBGMSG1("[rtvFM_SetFrequency] The step of dwChFreqKHz must be %dKHz", RTV_FM_CH_STEP_FREQ_KHz);
		return RTV_INVAILD_FREQ;
	}
	
    RTV_INT_ENABLE(FALSE);
    
    RTV_REG_MAP_SEL(FM_PAGE);
	
	RTV_REG_SET(0xF1, 0xE4); 
	RTV_REG_SET(0xF4, 0x20);
	RTV_REG_SET(0xED, 0x01);
    
	nRet = rtvRF_SetFrequency(RTV_TV_MODE_FM, 0, dwChFreqKHz);
	
	RTV_DELAY_MS(200);
	
	RTV_REG_MAP_SEL(FM_PAGE);
	
	Pilot_Lock = RTV_REG_GET(0xEF);
	if( Pilot_Lock & 0x01 )
	{
		RTV_REG_SET(0xF0, 0x41); 
		RTV_REG_SET(0xF1, 0xEC); 
		RTV_REG_SET(0xF4, 0x20);
	}
	else
	{
		RTV_REG_SET(0xF0, 0x43); 
		RTV_REG_SET(0xF1, 0xFC); 
		RTV_REG_SET(0xF4, 0x30);
	}
		
	RTV_INT_ENABLE(TRUE);

	return nRet;
}


INT rtvFM_Initialize(E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType)
{
	INT nRet;
	
	if(eAdcClkFreqType==RTV_ADC_CLK_FREQ_9_MHz || eAdcClkFreqType==RTV_ADC_CLK_FREQ_9_6_MHz)
		return RTV_UNSUPPORT_ADC_CLK;
		
	nRet = rtv_InitSystem(RTV_TV_MODE_FM, eAdcClkFreqType);
	if(nRet != RTV_SUCCESS)
		return nRet;

	fm_InitDemod();

	nRet = rtvRF_Initilize(RTV_TV_MODE_FM);
	if(nRet != RTV_SUCCESS)
		return nRet;

	return RTV_SUCCESS;
}
#endif
