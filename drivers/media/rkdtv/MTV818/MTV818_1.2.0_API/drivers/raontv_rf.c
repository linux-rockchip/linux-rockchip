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
* TITLE 	  : RAONTECH TV RF services source file. 
*
* FILENAME    : raontv_rf.c
*
* DESCRIPTION : 
*		Library of routines to initialize, and operate on, the RAONTECH RF chip.
*
********************************************************************************/
 
/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 10/01/2010  Ko, Kevin        Added the auto channel setting for TDMB.
* 09/29/2010  Ko, Kevin        Modified the auto pll index comments.
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

// Only include once.
#if defined(RTV_ISDBT_ENABLE)
	#include "raontv_rf_pll_data_auto_isdbt.h"
#endif
	
#if defined(RTV_TDMB_ENABLE)
	#include "raontv_rf_pll_data_man_tdmb.h"
#endif

#if defined(RTV_FM_ENABLE)
    #include "raontv_rf_pll_data_man_fm.h"
#endif
	
#if defined(RTV_DAB_ENABLE)	
	#include "raontv_rf_pll_data_auto_dab.h"
#endif

#include "raontv_rf_adc_data.h"


#ifdef RTV_ISDBT_ENABLE

 static const RTV_REG_INIT_INFO t_ISDBT_INIT[] =   
 {
	{  0x27,  0x6a  },
	{  0x2a,  0x07  },
	{  0x2B,  0x88  },
	{  0x2d,  0xec  },
	{  0x2e,  0xb0  },
	{  0x31,  0x04  },
	{  0x34,  0xc0  },
	{  0x3a,  0x77  },
	{  0x3b,  0xff  },
	{  0x3c,  0x79  },
	{  0x3e,  0x67  },
	{  0x3f,  0x00  },
	{  0x42,  0x14  },
	{  0x44,  0x40  },
	{  0x47,  0xc0  },
	{  0x49,  0x4f  },
	{  0x4a,  0x10  },
	{  0x53,  0x20  },	
	{  0x55,  0xfc  },
	{  0x57,  0x10  },
	{  0x5a,  0x83  },
	{  0x60,  0x13  },
	{  0x6b,  0xc5  },
 #if defined(RAONTV_CHIP_PKG_QFN)
	{  0x6c,  0x8d  },  
	{  0x6d,  0x7d  },  

 #elif defined (RAONTV_CHIP_PKG_WLCSP)
	{  0x6c,  0x80  },  
	{  0x6d,  0x70  },  
 #else
	#error "Code not present"
 #endif	
	{  0x72,  0xb0  },
	{  0x73,  0xca  },
	{  0x77,  0x89  },
	{  0x84,  0x87  },//0x90  },
	{  0x85,  0x95  },//0xa0  },
	{  0x86,  0x42  },
	{  0x87,  0x60  },
	{  0x8a,  0xf6  },
	{  0x8b,  0x89  },
	{  0x8c,  0x78  },
	{  0x90,  0x07  },
	{  0xb5,  0x1b  },
	{  0xae,  0x37  },
	{  0xc0,  0x31  },
	{  0xc1,  0xe8  },
	{  0xc3,  0xa2  },
	{  0xc4,  0xac  },
	{  0xc6,  0xeb  },
	{  0xca,  0x38  },
	{  0xcb,  0x8c  },
	{  0xcd,  0xa1  },
	{  0xce,  0xfc  },
	{  0xd0,  0x3f  },
	{  0xd4,  0x13  },
	{  0xd5,  0xf9  },
	{  0xd7,  0xa6  },
	{  0xd8,  0xac  },
	{  0xd9,  0x16  },
	{  0xda,  0x79  },
	{  0xde,  0x37  },
	{  0xdf,  0x3d  },
	{  0xe1,  0xa0  },
	{  0xe2,  0x0c  },
	{  0xe4,  0x3a  },
	{  0xa5,  0x00  },
	{  0xe9,  0xc1  }, // CKO_D24_EN = 0, CK_16M clock, SET_IIR clock disable
	{  0xae,  0x77  },
	{  0xe9,  0xd1  } // CKO_D24_EN = 1, CK_16M clock, SET_IIR clock enable
 };	
#endif /* RTV_ISDBT_ENABLE */


#ifdef RTV_FM_ENABLE
 static const RTV_REG_INIT_INFO t_FM_INIT[] =   
 {
	{  0x27,  0xc2  },
	{  0x2B,  0x88  },
	{  0x2d,  0xec  },
	{  0x2e,  0xb0  },
	{  0x31,  0x04  },
	{  0x34,  0xf8  },
	{  0x3a,  0x55  },
	{  0x3b,  0x73  },
	{  0x3c,  0x00  },
	{  0x3e,  0x00  },
	{  0x3f,  0x00  },
	{  0x40,  0x10  },
	{  0x42,  0x40  },
	{  0x44,  0x64  },
	{  0x47,  0xb1  },
	{  0x53,  0x20  },
	{  0x55,  0xd6  },
	{  0x57,  0x10  },
	{  0x60,  0x11  },
	{  0x6b,  0xc5  },
	{  0x72,  0xf0  },
	{  0x73,  0xc8  },
	{  0x74,  0x70  },
	{  0x77,  0x80  },
	{  0x78,  0x47  },
	{  0x86,  0x80  },
	{  0x87,  0x98  },
	{  0x8a,  0xf6  },
	{  0x8b,  0x80  },
	{  0x8c,  0x74  },
	{  0xaf,  0x01  },
	{  0xb5,  0x5b  },
	{  0xae,  0x37  },
	{  0xbd,  0x3f  },
	{  0xbe,  0x37  },
	{  0xbf,  0x3c  },
	{  0xc0,  0x3f  },
	{  0xc1,  0x39  },
	{  0xc3,  0xdb  },
	{  0xc4,  0x3c  },
	{  0xc5,  0x1e  },
	{  0xc6,  0x6c  },
	{  0xc8,  0x7b  },
	{  0xc9,  0xec  },
	{  0xca,  0x39  },
	{  0xcb,  0x03  },
	{  0xcd,  0xb4  },
	{  0xce,  0xec  },
	{  0xcf,  0x1d  },
	{  0xd0,  0xa6  },
	{  0xd1,  0x41  },
	{  0xd2,  0x16  },
	{  0xd3,  0xec  },
	{  0xd4,  0x3f  },
	{  0xd5,  0xc3  },
	{  0xd7,  0xbd  },
	{  0xd8,  0xbc  },
	{  0xd9,  0x1b  },
	{  0xda,  0x4b  },
	{  0xdb,  0x3b  },
	{  0xdc,  0x55  },
	{  0xdd,  0xdc  },
	{  0xde,  0x1a  },
	{  0xdf,  0xed  },
	{  0xe1,  0xa7  },
	{  0xe2,  0xcc  },
	{  0xe3,  0x1a  },
	{  0xe4,  0x55  },
	{  0xe5,  0x43  },
	{  0xe6,  0x00  },
	{  0xe7,  0x0f  },
	{  0xa5,  0x00  },
	{  0xe9,  0x41  },
	{  0xae,  0x77  },
	{  0xe9,  0x51  }
 };	
#endif /* RTV_FM_ENABLE */


#ifdef RTV_DAB_ENABLE
 static const RTV_REG_INIT_INFO t_LBAND_INIT[] =   
 {
	{  0x27,  0x3e  },
	{  0x2B,  0x88  },
	{  0x2d,  0xec  },
	{  0x2e,  0xb0  },
	{  0x31,  0x04  },
	{  0x3b,  0xf7  },
	{  0x43,  0x34  },
	{  0x44,  0x00  },
	{  0x4a,  0x18  },
	{  0x53,  0x20  },
	{  0x55,  0xfa  },
	{  0x60,  0x13  },
	{  0x6b,  0xc5  },
	{  0x6e,  0x80  },
	{  0x6f,  0x70  },
	{  0x72,  0xf0  },
	{  0x73,  0xca  },
	{  0x77,  0x88  },
	{  0x78,  0x47  },
	{  0x84,  0x70  },
	{  0x86,  0x70  },
	{  0x87,  0x90  },
	{  0x8a,  0xf6  },
	{  0x8b,  0x86  },
	{  0x8c,  0x75  },
	{  0x90,  0x05  },
	{  0x99,  0x77  },
	{  0x9a,  0x2d  },
	{  0x9b,  0x1e  },
	{  0x9c,  0x47  },
	{  0x9d,  0x3a  },
	{  0x9e,  0x03  },
	{  0x9f,  0x1e  },
	{  0xa0,  0x22  },
	{  0xa1,  0x33  },
	{  0xa2,  0x51  },
	{  0xa3,  0x36  },
	{  0xa4,  0x0c  },
	{  0xae,  0x37  },
	{  0xc0,  0x31  },
	{  0xc1,  0xe8  },
	{  0xc3,  0xa2  },
	{  0xc4,  0xac  },
	{  0xc6,  0xeb  },
	{  0xca,  0x38  },
	{  0xcb,  0x8c  },
	{  0xcd,  0xa1  },
	{  0xce,  0xfc  },
	{  0xd0,  0x3f  },
	{  0xd4,  0x13  },
	{  0xd5,  0xf9  },
	{  0xd7,  0xa6  },
	{  0xd8,  0xac  },
	{  0xd9,  0x16  },
	{  0xda,  0x79  },
	{  0xde,  0x37  },
	{  0xdf,  0x3d  },
	{  0xe1,  0xa0  },
	{  0xe2,  0x0c  },
	{  0xe4,  0x3a  },
	{  0xa5,  0x00  },
	{  0xe9,  0x41  },
	{  0xae,  0x77  },
	{  0xe9,  0x51  }
 };	
#endif /* RTV_DAB_ENABLE */


#ifdef RTV_TDMB_ENABLE
 static const RTV_REG_INIT_INFO t_TDMB_INIT[] =   
 {
	{  0x2B,  0x88  },
	{  0x2d,  0xec  },
	{  0x2e,  0xb0  },
	{  0x31,  0x04  },
	{  0x34,  0xf8  },
	{  0x3b,  0x74  },
	{  0x53,  0x20  },
	{  0x55,  0xd6  },
	{  0x60,  0x13  },
	{  0x6b,  0xc5  },
	{  0x72,  0xf0  },
	{  0x73,  0xca  },
	{  0x77,  0x80  },
	{  0x78,  0x47  },
	{  0x85,  0x98  },
	{  0x86,  0x80  },
	{  0x87,  0x98  },
	{  0x8a,  0xf6  },
	{  0x8b,  0x80  },
	{  0x8c,  0x75  },
	{  0x99,  0x77  },
	{  0x9a,  0x2d  },
	{  0x9b,  0x1e  },
	{  0x9c,  0x47  },
	{  0x9d,  0x3a  },
	{  0x9e,  0x03  },
	{  0x9f,  0x1e  },
	{  0xa0,  0x22  },
	{  0xa1,  0x33  },
	{  0xa2,  0x51  },
	{  0xa3,  0x36  },
	{  0xa4,  0x0c  },
	{  0xae,  0x37  },
	{  0xb5,  0x9b  },
	{0xbd,	0x3d},	 
	{0xbe,	0x68},	
	{0xbf,	0x5c},	
	{0xc0,	0x33},	 
	{0xc1,	0xca},	
	{0xc2,	0x43},	
	{0xc3,	0x90},	 
	{0xc4,	0xec},   
	{0xc5,	0x17},   
	{0xc6,	0xda},   
	{0xc7,	0x41},   
	{0xc8,	0x49}, 
	{0xc9,	0xbc}, 
	{0xca,	0x3a},   
	{0xcb,	0x20},   
	{0xcc,	0x43},   
	{0xcd,	0xa6},   
	{0xce,	0x4c},   
	{0xcf,	0x1f},   
	{0xd0,	0x8d},   
	{0xd1,	0x3d},   
	{0xd2,	0xdf}, 
	{0xd3,	0xa4}, 
	{0xd4,	0x30},   
	{0xd5,	0x00}, 
	{0xd6,	0x41},
	{0xd7,	0x81},  
	{0xd8,	0xd0}, 
	{0xd9,	0x00},
	{0xda,	0x00}, 
	{0xdb,	0x41},
	{0xdc,	0x6d},
	{0xdd,	0xac},
	{0xde,    0x39},
	{0xdf,	0x4e},
	{0xe0,	0x43},
	{0xe1,	0xa0},
	{0xe2,	0x4c},
	{0xe3,	0x1d},
	{0xe4,	0x99},
	{0xe5,	0x43},
	{0xe6,	0x00},
	{0xe7,	0x0d},
	{  0xa5,  0x00  },
	{  0xe9,  0x41  },
	{  0xae,  0x77  },
	{  0xe9,  0x51  }
 }; 


static const E_RTV_ADC_CLK_FREQ_TYPE g_aeAdcClkTypeTbl_TDMB[] = 
 {	
	RTV_ADC_CLK_FREQ_8_192_MHz/* 175280: 7A */, RTV_ADC_CLK_FREQ_8_MHz/* 177008: 7B */,	RTV_ADC_CLK_FREQ_9_MHz/* 178736: 7C */,
	RTV_ADC_CLK_FREQ_8_192_MHz/* 181280: 8A */,	RTV_ADC_CLK_FREQ_8_MHz/* 183008: 8B */,	RTV_ADC_CLK_FREQ_9_6_MHz/* 184736: 8C */,
	RTV_ADC_CLK_FREQ_9_MHz/* 187280: 9A */,	RTV_ADC_CLK_FREQ_8_MHz/* 189008: 9B */,	RTV_ADC_CLK_FREQ_9_6_MHz/* 190736: 9C */,
	RTV_ADC_CLK_FREQ_8_192_MHz/* 193280: 10A */, RTV_ADC_CLK_FREQ_8_MHz/* 195008: 10B */, RTV_ADC_CLK_FREQ_9_6_MHz/* 196736: 10C */,
	RTV_ADC_CLK_FREQ_9_MHz/* 199280: 11A */, RTV_ADC_CLK_FREQ_8_MHz/* 201008: 11B */, RTV_ADC_CLK_FREQ_8_MHz/* 202736: 11C */,
	RTV_ADC_CLK_FREQ_9_6_MHz/* 205280: 12A */, RTV_ADC_CLK_FREQ_8_MHz/* 207008: 12B */,	RTV_ADC_CLK_FREQ_8_MHz/* 208736: 12C */,
	RTV_ADC_CLK_FREQ_9_MHz/* 211280: 13A */, RTV_ADC_CLK_FREQ_8_MHz/* 213008: 13B */, RTV_ADC_CLK_FREQ_9_6_MHz/* 214736: 13C */
 };	
#endif /* RTV_TDMB_ENABLE */



/*===============================================================================
 * rtvRF_ConfigurePowerType
 *
 * DESCRIPTION : 
 *		This function returns 
 *		
 *
 * ARGUMENTS : none.
 * RETURN VALUE : none.
 *============================================================================*/
void rtvRF_ConfigurePowerType(void)
{
#if defined(RTV_IO_2_5V)
	U8 io_type = 0;
#elif defined(RTV_IO_3_3V)	
	U8 io_type = 1;
#elif defined(RTV_IO_1_8V)	
	U8 io_type = 2;
#else
	#error "Code not present"
#endif
	U8 REG2F = 0x61;   //DCDC_OUTSEL = 0x03, EXT_IOLDOCON = 0x01
	U8 REG30 = 0xF2 & 0xF0;  //IOLDOCON__REG
	U8 REG52 = 0x06;   //LDODIG_HT = 0x06;
	U8 REG54 = 0x10;   //LDODIG_I2C = 0x04;

	REG30 = REG30 | (io_type<<1);   //IO Type Select.

#if defined(RTV_PWR_EXTERNAL)		
    REG2F = REG2F | 0x14; //PDDCDC_I2C = 1, PDLDO12_I2C = 1 ; 
#elif defined(RTV_PWR_LDO)             
    REG2F = REG2F | 0x10; //PDDCDC_I2C = 1, PDLDO12_I2C = 0 ; 
#elif defined(RTV_PWR_DCDC)            
    REG2F = REG2F | 0x04; //PDDCDC_I2C = 0, PDLDO12_I2C = 1 ; 
#else
	#error "Code not present"
#endif    

	/* Below Power Up sequence is very important.*/
	RTV_REG_MAP_SEL(RF_PAGE);
	RTV_REG_SET(0x54, REG54);
	RTV_REG_SET(0x52, REG52);
	RTV_REG_SET(0x30, REG30);
	RTV_REG_SET(0x2F, REG2F);	
}



#define REGE8 (0x46 & 0xC0) 
#define REGEA (0x07 & 0xC0)  
#define REGEB (0x27 & 0xC0)  
#define REGEC (0x1E & 0xC0)  
#define REGED (0x18 & 0x00)  
#define REGEE (0xB8 & 0x00)  

INT rtvRF_ConfigureAdcClock(E_RTV_TV_MODE_TYPE eTvMode, E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType)
{
#ifdef  RTV_ISDBT_ENABLE
	U8 REGE9 = (0xD4 & 0xF0);
#else
	U8 REGE9 = (0x54 & 0xF0);  
#endif 
	const U8 *pbAdcClkSynTbl = (const U8 *)&g_abAdcClkSynTbl[eAdcClkFreqType];
	
	if(pbAdcClkSynTbl[0] == 0xFF)
    {
        RTV_DBGMSG1("[rtvRF_ConfigureAdcClock] Unsupport ADC clock type: %d\n", eAdcClkFreqType);
		return RTV_UNSUPPORT_ADC_CLK;
    }
		
	RTV_REG_MAP_SEL(RF_PAGE);
	RTV_REG_SET(0xE8, (REGE8 | pbAdcClkSynTbl[0]));                   
	RTV_REG_SET(0xE9, (REGE9 | pbAdcClkSynTbl[1]));				
	RTV_REG_SET(0xEA, (REGEA | pbAdcClkSynTbl[2]));				
	RTV_REG_SET(0xEB, (REGEB | pbAdcClkSynTbl[3]));				
	RTV_REG_SET(0xEC, (REGEC | pbAdcClkSynTbl[4]));				
	RTV_REG_SET(0xED, (REGED | pbAdcClkSynTbl[5]));				
	RTV_REG_SET(0xEE, (REGEE | pbAdcClkSynTbl[6]));	
	
	RTV_DELAY_MS(10);
	
#ifdef RTV_ISDBT_ENABLE 
	if(eTvMode == RTV_TV_MODE_1SEG)
	{
		RTV_REG_MAP_SEL(HOST_PAGE);
		RTV_REG_SET(0x1A, 0x8B);
		RTV_REG_SET(0x18, 0xC0);
		RTV_REG_SET(0x19, 0x03);
		RTV_REG_SET(0x07, 0xF0); 
	}
#endif

	return RTV_SUCCESS;
}


INT rtvRF_ChangeAdcClock(E_RTV_TV_MODE_TYPE eTvMode, E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType)
{	
#ifdef  RTV_ISDBT_ENABLE
	U8 REGE9 = (0xD4 & 0xF0);
#else
	U8 REGE9 = (0x54 & 0xF0);  
#endif 

#ifdef RTV_PWR_DCDC
	U8 RD2F, RD15;
#endif
	const U8 *pbAdcClkSynTbl = (const U8 *)&g_abAdcClkSynTbl[eAdcClkFreqType];
	
	if(pbAdcClkSynTbl[0] == 0xFF)
    {
        RTV_DBGMSG1("[rtvRF_ChangeAdcClock] Unsupport ADC clock type: %d\n", eAdcClkFreqType);
		return RTV_UNSUPPORT_ADC_CLK;
    }
		
	RTV_REG_MAP_SEL(RF_PAGE);

#ifdef RTV_PWR_DCDC
	RD2F = RTV_REG_GET(0x2F);     
	RTV_REG_SET(0x2F, (RD2F & 0xF7));
#endif	
	
	RTV_REG_SET(0xE8, (REGE8 | pbAdcClkSynTbl[0]));                   
	RTV_REG_SET(0xE9, (REGE9 | pbAdcClkSynTbl[1]));				
	RTV_REG_SET(0xEA, (REGEA | pbAdcClkSynTbl[2]));				
	RTV_REG_SET(0xEB, (REGEB | pbAdcClkSynTbl[3]));				
	RTV_REG_SET(0xEC, (REGEC | pbAdcClkSynTbl[4]));				
	RTV_REG_SET(0xED, (REGED | pbAdcClkSynTbl[5]));				
	RTV_REG_SET(0xEE, (REGEE | pbAdcClkSynTbl[6]));	
	
	RTV_DELAY_MS(10);
		
    switch(eAdcClkFreqType)
	{
		case RTV_ADC_CLK_FREQ_8_MHz:     
            switch( eTvMode )
            {
#ifdef RTV_ISDBT_ENABLE
                case RTV_TV_MODE_1SEG :	
                    RTV_REG_MAP_SEL(OFDM_PAGE);
                    RTV_REG_SET(0x19,0xff);
                    RTV_REG_SET(0x1a,0x08);
                    RTV_REG_SET(0x1b,0x82);
                    RTV_REG_SET(0x1c,0x20);
                    
                    RTV_REG_SET(0x45,0x10);
                    RTV_REG_SET(0x46,0x04);
                    RTV_REG_SET(0x47,0x41);
                    RTV_REG_SET(0x48,0x10);
    
                    RTV_REG_SET(0x49,0x00);
                    RTV_REG_SET(0x4a,0x00);
                    RTV_REG_SET(0x4b,0x00);
                    RTV_REG_SET(0x4c,0xF0);
                    break;
#endif

#ifdef RTV_TDMB_ENABLE
		        case RTV_TV_MODE_TDMB :	
                    RTV_REG_MAP_SEL(COMM_PAGE);
                    RTV_REG_SET(0x6A,0x01); 
                    
                    RTV_REG_MAP_SEL(0x06);
                    RTV_REG_SET(0x3c,0x4B); 
                    RTV_REG_SET(0x3d,0x37); 
                    RTV_REG_SET(0x3e,0x89); 
                    RTV_REG_SET(0x3f,0x41); 
                    break;
#endif	
                default: break;	
            } /* End of switch(eTvMode) */
			break;
                
		case RTV_ADC_CLK_FREQ_8_192_MHz:	
            switch( eTvMode )
            {
#ifdef RTV_ISDBT_ENABLE
                case RTV_TV_MODE_1SEG :	
                    RTV_REG_MAP_SEL(OFDM_PAGE);
                    RTV_REG_SET(0x19,0xfd);
                    RTV_REG_SET(0x1a,0xfc);
                    RTV_REG_SET(0x1b,0xbe);
                    RTV_REG_SET(0x1c,0x1f);
                                     
                    RTV_REG_SET(0x45,0xF7);
                    RTV_REG_SET(0x46,0x7D);
                    RTV_REG_SET(0x47,0xDF);
                    RTV_REG_SET(0x48,0x0F);
                                     
                    RTV_REG_SET(0x49,0x00);
                    RTV_REG_SET(0x4a,0x00);
                    RTV_REG_SET(0x4b,0x60);
                    RTV_REG_SET(0x4c,0xF0);
                    break;
#endif

#ifdef RTV_TDMB_ENABLE
		        case RTV_TV_MODE_TDMB :	
                    RTV_REG_MAP_SEL(COMM_PAGE);
                    RTV_REG_SET(0x6A,0x01); 
                       
                    RTV_REG_MAP_SEL( 0x06);
                    RTV_REG_SET(0x3c,0x00); 
                    RTV_REG_SET(0x3d,0x00); 
                    RTV_REG_SET(0x3e,0x00); 
                    RTV_REG_SET(0x3f,0x40); 
#endif
                default: break;
            } /* End of switch(eTvMode) */
			break;
		
        case RTV_ADC_CLK_FREQ_9_MHz:	
            switch( eTvMode )
            {
#ifdef RTV_ISDBT_ENABLE
                case RTV_TV_MODE_1SEG :	
                    RTV_REG_MAP_SEL(OFDM_PAGE);
                    RTV_REG_SET(0x19,0xe4);
                    RTV_REG_SET(0x1a,0x5c);
                    RTV_REG_SET(0x1b,0xe5);
                    RTV_REG_SET(0x1c,0x1c);
                                     
                    RTV_REG_SET(0x45,0x47);
                    RTV_REG_SET(0x46,0xAE);
                    RTV_REG_SET(0x47,0x72);
                    RTV_REG_SET(0x48,0x0E);
                                     
                    RTV_REG_SET(0x49,0x72);
                    RTV_REG_SET(0x4a,0x1C);
                    RTV_REG_SET(0x4b,0xC7);
                    RTV_REG_SET(0x4c,0xF1);
                    break;
#endif
			
#ifdef RTV_TDMB_ENABLE
		        case RTV_TV_MODE_TDMB :	
                    RTV_REG_MAP_SEL(COMM_PAGE);
                    RTV_REG_SET(0x6A,0x21);
    
                    RTV_REG_MAP_SEL( 0x06);
                    RTV_REG_SET(0x3c,0xB5); 
                    RTV_REG_SET(0x3d,0x14); 
                    RTV_REG_SET(0x3e,0x41); 
                    RTV_REG_SET(0x3f,0x3A); 
                    break;
#endif
                default: break;
            } /* End of switch(eTvMode) */
			break;	
                
		case RTV_ADC_CLK_FREQ_9_6_MHz:	
            switch( eTvMode )
            {
#ifdef RTV_ISDBT_ENABLE
                case RTV_TV_MODE_1SEG :	
                    RTV_REG_MAP_SEL(OFDM_PAGE);
                    RTV_REG_SET(0x19,0xd7);
                    RTV_REG_SET(0x1a,0x08);
                    RTV_REG_SET(0x1b,0x17);
                    RTV_REG_SET(0x1c,0x1b);
                    
                    RTV_REG_SET(0x45,0x62);
                    RTV_REG_SET(0x46,0x83);
                    RTV_REG_SET(0x47,0x8B);
                    RTV_REG_SET(0x48,0x0D);
    
                    RTV_REG_SET(0x49,0xAB);
                    RTV_REG_SET(0x4a,0xAA);
                    RTV_REG_SET(0x4b,0xAA);
                    RTV_REG_SET(0x4c,0xF2);
                    break;
#endif
			
#ifdef RTV_TDMB_ENABLE
		        case RTV_TV_MODE_TDMB :	
                    RTV_REG_MAP_SEL(COMM_PAGE);
                    RTV_REG_SET(0x6A,0x31);
                           
                    RTV_REG_MAP_SEL(0x06);
                    RTV_REG_SET(0x3c,0x69); 
                    RTV_REG_SET(0x3d,0x03); 
                    RTV_REG_SET(0x3e,0x9D); 
                    RTV_REG_SET(0x3f,0x36); 
                    break;
#endif			
                default: break;
            } /* End of switch(eTvMode) */
			break;	

		default:
			break;
	} /* End of switch(eAdcClkFreqType) */
	
#ifdef RTV_PWR_DCDC
	RTV_REG_MAP_SEL(RF_PAGE);
	RD15 = RTV_REG_GET(0x15) & 0x01; 
	if(RD15) 
	{
		 RTV_REG_SET(0x2F, RD2F);
	}
	else
	{
		RTV_DBGMSG0("[rtvRF_ChangeAdcClock] CLOCK SYNTH UnLock..\n");
	}
#endif

	return RTV_SUCCESS;	
}


INT rtvRF_SetFrequency(E_RTV_TV_MODE_TYPE eTvMode, UINT nChNum, U32 dwChFreqKHz)
{
#ifdef RTV_TDMB_ENABLE
	int nIdx;
#endif
#ifdef RTV_NOTCH_FILTER_ENABLE
	U8 notch_nIdx;
#endif
	INT nRet = RTV_SUCCESS;
	E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType;
	U32 dwPllNF,rd_dwPllNF_Verify_val,wr_dwPllNF_Verify_val;
	U32 PLL_Verify_cnt=10;


    /* Get the PLLNF and ADC clock type. */
	switch( eTvMode )
	{
#ifdef RTV_ISDBT_ENABLE
		case RTV_TV_MODE_1SEG :			
	#if defined(RTV_CONUTRY_BRAZIL)	|| defined(RYV_CONUTRY_ARGENTINA)
			nChNum -= 1;
	#endif		
			switch( nChNum )
			{
				case 14: case 22: case 30: case 38: case 46: case 54: case 62: eAdcClkFreqType = RTV_ADC_CLK_FREQ_9_6_MHz; break;									
				default: eAdcClkFreqType = RTV_ADC_CLK_FREQ_8_MHz; break;
			}			
			g_eRtvAdcClkFreqType = eAdcClkFreqType;
			
    #ifdef RTV_NOTCH_FILTER_ENABLE // => RTV_1SEG_NOTCH_FILTER_ENABLE ??
			switch( nChNum )  // //notch filter value need to tuning
			{
				case 14: notch_nIdx = 0; break;									
				default: notch_nIdx = 0;	
			}
    #endif
			break;
#endif		

#ifdef RTV_FM_ENABLE
		case RTV_TV_MODE_FM :							
			dwPllNF = g_atPllNF_FM[(dwChFreqKHz - RTV_FM_CH_MIN_FREQ_KHz)/RTV_FM_CH_STEP_FREQ_KHz];	
			eAdcClkFreqType = g_eRtvAdcClkFreqType;
			break;
#endif

#ifdef RTV_TDMB_ENABLE
		case RTV_TV_MODE_TDMB :			
			nIdx = (dwChFreqKHz - TDMB_CH_FREQ_START) / TDMB_CH_FREQ_STEP;
	#if defined(RTV_CONUTRY_KOREA)			
			if(dwChFreqKHz >= 205280)		nIdx -= 2;
			else if(dwChFreqKHz >= 193280)	nIdx -= 1; 
	#else
		#error "Code not present"
	#endif	
			
            eAdcClkFreqType = g_aeAdcClkTypeTbl_TDMB[nIdx];
			g_eRtvAdcClkFreqType = eAdcClkFreqType;						
            dwPllNF = g_atPllNF_TDMB[nIdx];
			break;
#endif

#ifdef RTV_DAB_ENABLE
		case RTV_TV_MODE_DAB :
			dwPllNF = 0;
			break;
#endif

		default:
			return RTV_INVAILD_TV_MODE;
	}
		
	g_fRtvChannelChange = TRUE; 
	
	/*
	if(eAdcClkFreqType != g_ePrevAdcClkFreqType)
	*/

	if(eTvMode==RTV_TV_MODE_1SEG || eTvMode==RTV_TV_MODE_TDMB)
	{
		if((nRet=rtvRF_ChangeAdcClock(eTvMode, eAdcClkFreqType)) != RTV_SUCCESS)
			goto RF_SET_FREQ_EXIT;
	}
	
	RTV_REG_MAP_SEL(RF_PAGE);

    /* Set the PLLNF and channel. */
	switch( eTvMode )
	{
#ifdef RTV_ISDBT_ENABLE
		case RTV_TV_MODE_1SEG :			
	#ifdef RTV_NOTCH_FILTER_ENABLE
			 RTV_REG_SET(0x21, (notch_nIdx<<4) | 0x01);  //notch filter value need to tuning
	#endif			
  
			RTV_REG_SET(0x20, nChNum);  
            RTV_DELAY_MS(2);	//2ms Delay 	

            wr_dwPllNF_Verify_val = (dwPllNF + (((ISDBT_AUTO_PLL_NFSTEP * 8) & 0xFFFFF0) * (nChNum >> 3)) 
                                    + ((nChNum & 0x07) * (ISDBT_AUTO_PLL_NFSTEP &0xFFFFF0)))>>1;

			do{
				rd_dwPllNF_Verify_val = (RTV_REG_GET(0x12) << 22) + (RTV_REG_GET(0x13) << 14) +(RTV_REG_GET(0x14) << 6) +(RTV_REG_GET(0x15) >> 2);

				if((wr_dwPllNF_Verify_val == rd_dwPllNF_Verify_val ) || (wr_dwPllNF_Verify_val == (U32)(rd_dwPllNF_Verify_val>>1)))
				{
					break;
				}
				else
				{ 
					RTV_REG_SET(0x20, nChNum);	
					RTV_DELAY_MS(2);	//2ms Delay 
				//	RTV_DBGMSG3("[rtvRF_SetFrequency] 1SEG PLL verify Re-Try!!!  PLLNF_I2C = 0x%0x  RD_PLLNF = 0x%0x RD_PLLNF*2 = 0x%0x\n",wr_dwPllNF_Verify_val,rd_dwPllNF_Verify_val,rd_dwPllNF_Verify_val>>1);
				}
			}while(PLL_Verify_cnt--);


			RTV_REG_MAP_SEL(OFDM_PAGE);
			RTV_REG_SET(0x11,0x07);
			RTV_REG_SET(0x11,0x06);
			RTV_REG_MAP_SEL(FEC_PAGE);
			RTV_REG_SET(0x10,0x01); 
			RTV_REG_SET(0x10,0x00); 
			break;
#endif		

#ifdef RTV_FM_ENABLE
		case RTV_TV_MODE_FM :		
			RTV_REG_SET(0x23, (dwPllNF>>22)&0xFF);
			RTV_REG_SET(0x24, (dwPllNF>>14)&0xFF);
			RTV_REG_SET(0x25, (dwPllNF>>6)&0xFF);
			RTV_REG_SET(0x26, (((dwPllNF&0x0000003F)<<2)) | 0x00);	
			RTV_DELAY_MS(1);  //1ms Delay	
			RTV_REG_SET(0x20, 0x00);	

		    RTV_DELAY_MS(2);	//2ms Delay 	 
			wr_dwPllNF_Verify_val = dwPllNF>>1;

			do{
				rd_dwPllNF_Verify_val = (RTV_REG_GET(0x12) << 22) + (RTV_REG_GET(0x13) << 14) +(RTV_REG_GET(0x14) << 6) +(RTV_REG_GET(0x15) >> 2);

				if((wr_dwPllNF_Verify_val == rd_dwPllNF_Verify_val ) || (wr_dwPllNF_Verify_val == (U32)(rd_dwPllNF_Verify_val>>1)))
				{
					break;
				}
				else
				{ 
					RTV_REG_SET(0x20, 0x00);	
					RTV_DELAY_MS(2);	//2ms Delay 
				//	RTV_DBGMSG3("[rtvRF_SetFrequency] FM PLL verify Re-Try!!!  PLLNF_I2C = 0x%0x  RD_PLLNF = 0x%0x RD_PLLNF*2 = 0x%0x\n",wr_dwPllNF_Verify_val,rd_dwPllNF_Verify_val,rd_dwPllNF_Verify_val>>1);
				}
			}while(PLL_Verify_cnt--);

            RTV_REG_MAP_SEL(0x06); // ofdm
			RTV_REG_SET(0x10,0x48); 
			RTV_REG_SET(0x10,0xC9); 
			break;
#endif

#ifdef RTV_TDMB_ENABLE
		case RTV_TV_MODE_TDMB :	
            RTV_REG_SET(0x23, (dwPllNF>>22)&0xFF);
			RTV_REG_SET(0x24, (dwPllNF>>14)&0xFF);
			RTV_REG_SET(0x25, (dwPllNF>>6)&0xFF);
			RTV_REG_SET(0x26, (((dwPllNF&0x0000003F)<<2)) | 0x00);	
			RTV_DELAY_MS(1);  //1ms Delay	
			RTV_REG_SET(0x20, 0x00);
                        
            RTV_DELAY_MS(2);	//2ms Delay 	 
			wr_dwPllNF_Verify_val = dwPllNF>>1;

			do{
				rd_dwPllNF_Verify_val = (RTV_REG_GET(0x12) << 22) + (RTV_REG_GET(0x13) << 14) +(RTV_REG_GET(0x14) << 6) +(RTV_REG_GET(0x15) >> 2);

				if((wr_dwPllNF_Verify_val == rd_dwPllNF_Verify_val ) || (wr_dwPllNF_Verify_val == (U32)(rd_dwPllNF_Verify_val>>1)))
				{
					break;
				}
				else
				{ 
					RTV_REG_SET(0x20, 0x00);	
					RTV_DELAY_MS(2);	//2ms Delay 
				//	RTV_DBGMSG3("[rtvRF_SetFrequency] TDMB PLL verify Re-Try!!!  PLLNF_I2C = 0x%0x  RD_PLLNF = 0x%0x RD_PLLNF*2 = 0x%0x\n",wr_dwPllNF_Verify_val,rd_dwPllNF_Verify_val,rd_dwPllNF_Verify_val>>1);
				}
			}while(PLL_Verify_cnt--);


			RTV_REG_MAP_SEL(0x06); // ofdm
			RTV_REG_SET(0x10,0x48); 
			RTV_REG_SET(0x10,0xC9); 
			break;
#endif

#ifdef RTV_DAB_ENABLE
		case RTV_TV_MODE_DAB_B3 :
		case RTV_TV_MODE_DAB_L :
			RTV_REG_SET(0x23, (dwPllNF>>22)&0xFF);
			RTV_REG_SET(0x24, (dwPllNF>>14)&0xFF);
			RTV_REG_SET(0x25, (dwPllNF>>6)&0xFF);
			RTV_REG_SET(0x26, (((dwPllNF&0x0000003F)<<2)) | eTvMode);	
			RTV_DELAY_MS(1);  //1ms Delay	
			RTV_REG_SET(0x20, 0x00);
                        
            RTV_DELAY_MS(2);	//2ms Delay 	
			wr_dwPllNF_Verify_val = dwPllNF>>1;

			do{
				rd_dwPllNF_Verify_val = (RTV_REG_GET(0x12) << 22) + (RTV_REG_GET(0x13) << 14) +(RTV_REG_GET(0x14) << 6) +(RTV_REG_GET(0x15) >> 2);

				if((wr_dwPllNF_Verify_val == rd_dwPllNF_Verify_val ) || (wr_dwPllNF_Verify_val == (U32)(rd_dwPllNF_Verify_val>>1)))
				{
					break;
				}
				else
				{ 
					RTV_REG_SET(0x20, 0x00);	
					RTV_DELAY_MS(2);	//2ms Delay 
				//	RTV_DBGMSG3("[rtvRF_SetFrequency] DAB PLL verify Re-Try!!!  PLLNF_I2C = 0x%0x  RD_PLLNF = 0x%0x RD_PLLNF*2 = 0x%0x\n",wr_dwPllNF_Verify_val,rd_dwPllNF_Verify_val,rd_dwPllNF_Verify_val>>1);
				}
			}while(PLL_Verify_cnt--);
			RTV_REG_MAP_SEL(0x06); // ofdm
			RTV_REG_SET(0x10,0x48); 
			RTV_REG_SET(0x10,0xC9); 
			break;
#endif

		default:
			return RTV_INVAILD_TV_MODE;
	}

	RTV_DELAY_MS(1);  //1ms Delay

RF_SET_FREQ_EXIT:
	g_fRtvChannelChange = FALSE; 
		 
	return nRet;
}


INT rtvRF_Initilize(E_RTV_TV_MODE_TYPE eTvMode)
{
	UINT nNumTblEntry=0, nNumAutoInit=0;
	const RTV_REG_INIT_INFO *ptInitTbl = NULL;
	const RTV_REG_INIT_INFO *ptLNA = NULL;
	const RTV_REG_MASK_INFO *ptAutoCh = NULL;
    U32 dwAutoPllNF=0, dwAutoPllNFSTEP=0;
			
	g_fRtvChannelChange = FALSE;
	
	switch( eTvMode )
	{
#ifdef RTV_ISDBT_ENABLE
		case RTV_TV_MODE_1SEG :
			ptInitTbl = t_ISDBT_INIT;
			nNumTblEntry = sizeof(t_ISDBT_INIT) / sizeof(RTV_REG_INIT_INFO);
			ptLNA = g_atAutoLnaInitData_ISDBT;
			ptAutoCh = g_atAutoChInitData_ISDBT;
			nNumAutoInit = sizeof(g_atAutoChInitData_ISDBT) / sizeof(RTV_REG_MASK_INFO);	
			dwAutoPllNF = ISDBT_AUTO_PLLNF;
			dwAutoPllNFSTEP = ISDBT_AUTO_PLL_NFSTEP;
			break;
#endif		

#ifdef RTV_FM_ENABLE
		case RTV_TV_MODE_FM :
			ptInitTbl = t_FM_INIT;
			nNumTblEntry = sizeof(t_FM_INIT) / sizeof(RTV_REG_INIT_INFO);
			ptLNA = g_atAutoLnaInitData_FM;
			break;
#endif

#ifdef RTV_TDMB_ENABLE
		case RTV_TV_MODE_TDMB :
			ptInitTbl = t_TDMB_INIT;
			nNumTblEntry = sizeof(t_TDMB_INIT) / sizeof(RTV_REG_INIT_INFO);
			ptLNA = g_atAutoLnaInitData_TDMB;		
			break;
#endif

#ifdef RTV_DAB_ENABLE
		case RTV_TV_MODE_DAB_B3:
			ptInitTbl = t_LBAND_INIT;
			nNumTblEntry = sizeof(t_LBAND_INIT) / sizeof(RTV_REG_INIT_INFO);
			ptLNA = g_atAutoLnaInitData_DAB;
			ptAutoCh = g_atAutoChInitData_DAB;
			nNumAutoInit = sizeof(g_atAutoChInitData_DAB) / sizeof(RTV_REG_MASK_INFO);	
			dwAutoPllNF = DAB_AUTO_PLLNF;
			dwAutoPllNFSTEP = DAB_AUTO_PLL_NFSTEP;
			break;
			
		case RTV_TV_MODE_DAB_L :
			ptInitTbl = t_LBAND_INIT;
			nNumTblEntry = sizeof(t_LBAND_INIT) / sizeof(RTV_REG_INIT_INFO);
			ptLNA = g_atAutoLnaInitData_DAB;
			ptAutoCh = g_atAutoChInitData_DAB;
			nNumAutoInit = sizeof(g_atAutoChInitData_DAB) / sizeof(RTV_REG_MASK_INFO);	
            		dwAutoPllNF = DAB_AUTO_PLLNF;
            		dwAutoPllNFSTEP = DAB_AUTO_PLL_NFSTEP;
			break;
#endif

		default:
			return RTV_INVAILD_TV_MODE;
	}
	
	RTV_REG_MAP_SEL(RF_PAGE);
		
	do
	{
		RTV_REG_SET(ptInitTbl->bReg, ptInitTbl->bVal);
		ptInitTbl++;						
	} while( --nNumTblEntry );
	
	// Auto LNA
	RTV_REG_SET(ptLNA[0].bReg, ptLNA[0].bVal);
	RTV_REG_SET(ptLNA[1].bReg, ptLNA[1].bVal);
	
	// Auto channel setting.
	if(eTvMode == RTV_TV_MODE_1SEG)
	{
		do
		{		
			RTV_REG_MASK_SET(ptAutoCh->bReg, ptAutoCh->bMask, ptAutoCh->bVal);
			ptAutoCh++;	
		} while(--nNumAutoInit );

		RTV_REG_SET(0x23,(dwAutoPllNF>>22)&0xFF);
		RTV_REG_SET(0x24,(dwAutoPllNF>>14)&0xFF);
		RTV_REG_SET(0x25,(dwAutoPllNF>>6)&0xFF);

		RTV_REG_SET(0x26,((dwAutoPllNF&0x0000003F) << 2) | eTvMode);

		RTV_REG_SET(0x62, (dwAutoPllNFSTEP >> 16) & 0xFF);
		RTV_REG_SET(0x63, (dwAutoPllNFSTEP >> 8 ) & 0xFF);
		RTV_REG_SET(0x64, (dwAutoPllNFSTEP & 0xFF));
	}

	
#ifdef RTV_PWR_DCDC
	{
		U8 RD15 = RTV_REG_GET(0x15) & 0x01; 
		U8 RD2F = RTV_REG_GET(0x2F);       
		if(RD15)
		{
			RD2F |=0x08;
			RTV_REG_SET(0x2F, RD2F);
		}
		else
		{
			RTV_DBGMSG0("[rtvRF_Initilize] Clock Unlock \n");
		}
	}
#endif
	
	return RTV_SUCCESS;
}

