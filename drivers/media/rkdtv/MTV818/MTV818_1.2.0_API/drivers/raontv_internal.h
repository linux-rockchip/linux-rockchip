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
* TITLE 	  : RAONTECH TV internal header file. 
*
* FILENAME    : raontv_internal.h
*
* DESCRIPTION : 
*		All the declarations and definitions necessary for the RAONTECH TV driver.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 10/01/2010  Ko, Kevin        Changed the order of E_RTV_TV_MODE_TYPE for HW spec.
* 09/29/2010  Ko, Kevin        Added the FM freq definition of brazil.
* 09/27/2010  Ko, Kevin        Creat for CS Realease
*             /Yang, Maverick  1.Reformating for CS API
*                              2.pll table, ADC clock switching, SCAN function, 
*								 FM function added..
* 04/09/2010  Yang, Maverick   REV1 SETTING 
* 01/25/2010  Yang, Maverick   Created.                                                              
********************************************************************************/

#ifndef __RAONTV_INTERNAL_H__
#define __RAONTV_INTERNAL_H__

#ifdef __cplusplus 
extern "C"{ 
#endif  

#include "raontv.h"


// Do not modify the order!
typedef enum
{	
	RTV_TV_MODE_TDMB   = 0,     // Band III 
	RTV_TV_MODE_DAB_B3 = 1,      // L-Band
	RTV_TV_MODE_DAB_L  = 2,      // L-Band	
	RTV_TV_MODE_1SEG   = 3, // UHF
	RTV_TV_MODE_FM     = 4,       // FM
	MAX_NUM_RTV_MODE
} E_RTV_TV_MODE_TYPE;


typedef struct
{
	U8	bReg;
	U8	bVal;
} RTV_REG_INIT_INFO;


typedef struct
{
	U8	bReg;
	U8  bMask;
	U8	bVal;
} RTV_REG_MASK_INFO;


#define MODE1 2 		
#define MODE2 1
#define MODE3 0


#define MAP_SEL_REG 	0x03

#define OFDM_PAGE       0x02 // for 1seg
#define FEC_PAGE        0x03 // for 1seg
#define COMM_PAGE       0x04
#define FM_PAGE         0x06 // OFDM for TDMB, DAB, FM 
#define HOST_PAGE       0x07
#define CAS_PAGE        0x08
#define DD_PAGE         0x09 // FEC for TDMB, DAB, FM

#define FIC_PAGE        0x0A
#define MSC0_PAGE       0x0B
#define MSC1_PAGE       0x0C
#define RF_PAGE         0x0F


#define DEMOD_0SC_DIV2_ON  0x80
#define DEMOD_0SC_DIV2_OFF 0x00

#if (RTV_SRC_CLK_FREQ_KHz >= 32000)
	#define DEMOD_OSC_DIV2 	DEMOD_0SC_DIV2_ON
#else 
	#define DEMOD_OSC_DIV2 	DEMOD_0SC_DIV2_OFF
#endif 


#define MAP_SEL_VAL(page)		(DEMOD_OSC_DIV2|page)
#define RTV_REG_MAP_SEL(page)	RTV_REG_SET(MAP_SEL_REG, MAP_SEL_VAL(page));



// ISDBT Channel 
#ifdef RTV_ISDBT_ENABLE
	#define ISDBT_CH_NUM_START__JAPAN			13
	#define ISDBT_CH_NUM_END__JAPAN				62
	#define ISDBT_CH_FREQ_START__JAPAN			473143
	#define ISDBT_CH_FREQ_STEP__JAPAN			6000

	#define ISDBT_CH_NUM_START__BRAZIL			14
	#define ISDBT_CH_NUM_END__BRAZIL			69
	#define ISDBT_CH_FREQ_START__BRAZIL			473143
	#define ISDBT_CH_FREQ_STEP__BRAZIL			6000

	#define ISDBT_CH_NUM_START__ARGENTINA		14
	#define ISDBT_CH_NUM_END__ARGENTINA			69
	#define ISDBT_CH_FREQ_START__ARGENTINA		473143
	#define ISDBT_CH_FREQ_STEP__ARGENTINA		6000

	#if defined(RTV_CONUTRY_JAPAN)
		#define ISDBT_CH_NUM_START			ISDBT_CH_NUM_START__JAPAN
		#define ISDBT_CH_NUM_END			ISDBT_CH_NUM_END__JAPAN

		#define ISDBT_CH_FREQ_START			ISDBT_CH_FREQ_START__JAPAN
		#define ISDBT_CH_FREQ_STEP  		ISDBT_CH_FREQ_STEP__JAPAN
		
	#elif defined(RTV_CONUTRY_BRAZIL) 
		#define ISDBT_CH_NUM_START			ISDBT_CH_NUM_START__BRAZIL
		#define ISDBT_CH_NUM_END			ISDBT_CH_NUM_END__BRAZIL

		#define ISDBT_CH_FREQ_START			ISDBT_CH_FREQ_START__BRAZIL
		#define ISDBT_CH_FREQ_STEP  		ISDBT_CH_FREQ_STEP__BRAZIL
		
	#elif defined(RYV_CONUTRY_ARGENTINA)
		#define ISDBT_CH_NUM_START			ISDBT_CH_NUM_START__ARGENTINA
		#define ISDBT_CH_NUM_END			ISDBT_CH_NUM_END__ARGENTINA

		#define ISDBT_CH_FREQ_START			ISDBT_CH_FREQ_START__ARGENTINA
		#define ISDBT_CH_FREQ_STEP  		ISDBT_CH_FREQ_STEP__ARGENTINA
	#else
		#error "Code not present"		
	#endif
#endif


#ifdef RTV_TDMB_ENABLE
	#define TDMB_CH_FREQ_START__KOREA		175280
	#define TDMB_CH_FREQ_STEP__KOREA		1728 // about...

	#if defined(RTV_CONUTRY_KOREA)
		#define TDMB_CH_FREQ_START			TDMB_CH_FREQ_START__KOREA
		#define TDMB_CH_FREQ_STEP  			TDMB_CH_FREQ_STEP__KOREA
	#else
		#error "Code not present"
	#endif
#endif



#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2) 
	void rtv_EnableInterrupt(BOOL fEnable); // FALSE: Disable, TRUE:Enable
	#define RTV_INT_ENABLE(en)		rtv_EnableInterrupt(en)
#else	
	#define RTV_INT_ENABLE(en)		((void)0)
#endif


extern volatile E_RTV_ADC_CLK_FREQ_TYPE g_eRtvAdcClkFreqType;
extern volatile BOOL g_fRtvChannelChange;


/*==============================================================================
 *
 * Common inline functions.
 *
 *============================================================================*/ 
static INLINE void rtv_ConfigureTsifFormat(void)
{
    RTV_REG_MAP_SEL(COMM_PAGE);  // DATA DECODER Interface Format setting..

#if defined(RTV_IF_MPEG2_SERIAL_TSIF) 
  #if defined(RTV_TSIF_FORMAT_1)
    RTV_REG_SET(0x45, 0x00);    
  #elif defined(RTV_TSIF_FORMAT_2)
    RTV_REG_SET(0x45, 0x02);
  #elif defined(RTV_TSIF_FORMAT_3)
    RTV_REG_SET(0x45, 0x21);
  #elif defined(RTV_TSIF_FORMAT_4)
    RTV_REG_SET(0x45, 0x23);
  #else
    #error "Code not present"
  #endif
    RTV_REG_SET(0x46, 0x80);                                                    

#elif defined(RTV_IF_QUALCOMM_TSIF)
  #if defined(RTV_TSIF_FORMAT_1)
    RTV_REG_SET(0x45, 0x00);    
    RTV_REG_SET(0x46, 0xA0); 
  #elif defined(RTV_TSIF_FORMAT_2)
    RTV_REG_SET(0x45, 0xE9);

  #elif defined(RTV_TSIF_FORMAT_3)
    RTV_REG_SET(0x45, 0xE1);
  #elif defined(RTV_TSIF_FORMAT_4)
    RTV_REG_SET(0x45, 0x40);
  #elif defined(RTV_TSIF_FORMAT_5)
    RTV_REG_SET(0x45, 0x21);    
  #else
    #error "Code not present"
  #endif
    RTV_REG_SET(0x46, 0xA0); 
#endif	
}

static INLINE void MSC1_TSIF_Enable(void)
{	
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x29, 0x00); ///[3] Interrupt status register clear condition - 0:read data by memory access 1:status register access
	
	RTV_REG_SET(0x22, 0x48);
	RTV_REG_SET(0x04, 0x29); // GPDD SEL 
	RTV_REG_SET(0x0C, 0xF4); // MPEG PAD Enable

    rtv_ConfigureTsifFormat();
	RTV_REG_SET(0x47, 0x53); // MSC1 TSI enable

}


static INLINE void rtv_SetParallelTsif_1SEG_Only(void)
{
	RTV_REG_MAP_SEL(FEC_PAGE);
	
#if defined(RTV_TSIF_FORMAT_1)
    RTV_REG_SET(0x6E, 0x11);    
#elif defined(RTV_TSIF_FORMAT_2)
    RTV_REG_SET(0x6E, 0x13);    
#elif defined(RTV_TSIF_FORMAT_3)
    RTV_REG_SET(0x6E, 0x10);    
#elif defined(RTV_TSIF_FORMAT_4)
    RTV_REG_SET(0x6E, 0x12);    
#else
    #error "Code not present"
#endif
	RTV_REG_SET(0x6F, 0x02);
    RTV_REG_SET(0x70, 0x88);   
}



/*==============================================================================
 *
 * T-DMB inline functions.
 *
 *============================================================================*/ 
static INLINE void rtv_SetupMemoryFIC(void)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x46, 0x10); // auto user clr, get fic  CRC 2byte including[4]
	RTV_REG_SET(0x46, 0x16); // FIC enable
}

static INLINE void rtv_ResetMemoryFIC(void)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x46, 0x10); 
}

static INLINE void FIC_TSIF_Enable(void)
{
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x29, 0x00); ///[3] Interrupt status register clear condition - 0:read data by memory access 1:status register access

	RTV_REG_SET(0x22, 0x48);
	RTV_REG_SET(0x04, 0x29); // GPDD SEL 
	RTV_REG_SET(0x0C, 0xF4); // MPEG PAD Enable

    rtv_ConfigureTsifFormat();
	RTV_REG_SET(0x47, 0x47); // MSC1 TSI enable
}

static INLINE void TSIF_Disable(void)
{
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x29, 0x08); ///[3] Interrupt status register clear condition - 0:read data by memory access 1:status register access

	RTV_REG_SET(0x04, 0x70); // MPEG Serial IF out from GPDD
	RTV_REG_SET(0x0C, 0xF5); // MPEG PAD Disable

	RTV_REG_MAP_SEL(COMM_PAGE);
	RTV_REG_SET(0x45, 0x00);
	RTV_REG_SET(0x46, 0x80);
	RTV_REG_SET(0x47, 0x00); // MSC1 TSI disable 
	
}



/*==============================================================================
 * External functions for RAONTV driver core.
 *============================================================================*/ 
void rtv_EnableInterrupt(BOOL fEnable);
void rtv_ResetMemoryMSC1(void);
void rtv_ResetMemoryMSC0(void);
void rtv_ConfigureMemeoryMSC1(E_RTV_TV_MODE_TYPE eTvMode, U16 wThresholdSize);
INT  rtv_InitSystem(E_RTV_TV_MODE_TYPE eTvMode, E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType);

#ifdef __cplusplus 
} 
#endif 

#endif /* __RAONTV_INTERNAL_H__ */

