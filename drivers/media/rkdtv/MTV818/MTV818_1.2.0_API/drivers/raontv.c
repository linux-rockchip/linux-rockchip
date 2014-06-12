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
* TITLE 	  : RAONTECH TV device driver API header file. 
*
* FILENAME    : raontv.c
*
* DESCRIPTION : 
*		Configuration for RAONTECH TV Services.
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
********************************************************************************/

#include "raontv.h"
#include "raontv_rf.h"
#include "raontv_internal.h"

volatile BOOL g_fRtvChannelChange = FALSE;

volatile E_RTV_ADC_CLK_FREQ_TYPE g_eRtvAdcClkFreqType = RTV_ADC_CLK_FREQ_8_MHz; // default

#ifdef RTV_IF_EBI2
VU8 g_bRtvEbiMapSelData = 0x7;
#endif


void rtv_ResetMemoryMSC0(void)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x47, 0x00);  // MSC0 memory control register clear.
}

void rtv_ResetMemoryMSC1(void)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x48, 0x00);  // MSC1 memory control register clear.
}



void rtv_ConfigureMemeoryMSC1(E_RTV_TV_MODE_TYPE eTvMode, U16 wThresholdSize)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x47, 0x00); // MSC0

	if((eTvMode == RTV_TV_MODE_1SEG) || (eTvMode == RTV_TV_MODE_FM))
		RTV_REG_SET(0x45, 0xA0);

	RTV_REG_SET(0x56,(wThresholdSize>>8) & 0x0F);
	RTV_REG_SET(0x57,(wThresholdSize & 0xFF));	
    
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)
    rtv_ConfigureTsifFormat();
    RTV_REG_SET(0x47, 0x53); // MSC1 TSI enable 
    	
    RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x48, 0x01);
	RTV_REG_SET(0x48, 0x05);
	if(eTvMode == RTV_TV_MODE_1SEG) /* VIDEO */
		RTV_REG_SET(0x3A, 0xC0);
		
	RTV_REG_SET(0x35, 0x04); // MSC1 Interrupt status clear.

#else
	RTV_REG_SET(0x48, 0x01);
	RTV_REG_SET(0x48, 0x0D);
	RTV_REG_SET(0x3A, 0xC0);
	RTV_REG_SET(0x35, 0x04); // MSC1 Interrupt status clear.
#endif	
}


void rtv_EnableInterrupt(BOOL fEnable) 
{
	RTV_REG_MAP_SEL(HOST_PAGE);
	
	if(fEnable == FALSE) // disable INT
	{
   		RTV_REG_SET(0x62, 0xFF); //[6] MSC1 over-run interrupt [5] MSC1 under-run interrupt [4] MSC1 interrupt

		RTV_REG_MAP_SEL(DD_PAGE);
		RTV_REG_SET(0x48, 0x01);  // MSC1 memory control register clear.	
	}
#ifndef RTV_TDMB_ENABLE	// MSC0, MSC1, FIC
	else
	{
	    RTV_REG_MAP_SEL(HOST_PAGE);
	    RTV_REG_SET(0x62, 0x8F); //[6] MSC1 over-run interrupt [5] MSC1 under-run interrupt [4] MSC1 interrupt
	    
	    RTV_REG_MAP_SEL(DD_PAGE);
  #if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)
	    RTV_REG_SET(0x48, 0x01);	
	    RTV_REG_SET(0x48, 0x05);
  #else
	    RTV_REG_SET(0x48, 0x01);	
	    RTV_REG_SET(0x48, 0x0D);
  #endif
	    RTV_REG_SET(0x35, 0x04); // MSC1 Interrupt status clear.
	}
#endif	
}


INT rtv_InitSystem(E_RTV_TV_MODE_TYPE eTvMode, E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType)
{
	INT nRet;
	
	// Save the ADC clock type.
	g_eRtvAdcClkFreqType = eAdcClkFreqType;
	
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x7D, 0x06);
		
	rtvRF_ConfigurePowerType();

	if((nRet=rtvRF_ConfigureAdcClock(eTvMode, eAdcClkFreqType)) != RTV_SUCCESS)
		return nRet;
		
	return RTV_SUCCESS;
}





