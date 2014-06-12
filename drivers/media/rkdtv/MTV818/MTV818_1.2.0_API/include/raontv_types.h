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
* TITLE 	  : RAONTECH TV type header file. 
*
* FILENAME    : raontv_types.h
*
* DESCRIPTION : 
*		This file contains types and declarations associated with the RAONTECH
*		TV Services.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 10/14/2010  Ko, Kevin        Added the RTV_FM_CH_STEP_FREQ_KHz defintion.
* 10/06/2010  Ko, Kevin        Added RTV_ISDBT_FREQ2CHNUM macro for ISDB-T.
* 09/29/2010  Ko, Kevin        Created file.
********************************************************************************/

#ifndef __RAONTV_TYPES_H__
#define __RAONTV_TYPES_H__

#ifdef __cplusplus 
extern "C"{ 
#endif  


/*==============================================================================
 *
 * Common definitions and types.
 *
 *============================================================================*/
#ifndef NULL
	#define NULL    	0
#endif

#ifndef FALSE
	#define FALSE		0
#endif

#ifndef TRUE
	#define TRUE		1
#endif

#ifndef MAX
	#define MAX(a, b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a, b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
	#define ABS(x) 		 (((x) < 0) ? -(x) : (x))
#endif


// Do not modify the order!
typedef enum
{
	RTV_ADC_CLK_FREQ_8_MHz = 0,
	RTV_ADC_CLK_FREQ_8_192_MHz,
	RTV_ADC_CLK_FREQ_9_MHz,
	RTV_ADC_CLK_FREQ_9_6_MHz,
	MAX_NUM_RTV_ADC_CLK_FREQ_TYPE
} E_RTV_ADC_CLK_FREQ_TYPE;


// Modulation
typedef enum
{
	RTV_MOD_DQPSK = 0,
	RTV_MOD_QPSK,
	RTV_MOD_16QAM,
	RTV_MOD_64QAM
} E_RTV_MODULATION_TYPE;

typedef enum
{
	RTV_CODE_RATE_1_2 = 0,
	RTV_CODE_RATE_2_3,
	RTV_CODE_RATE_3_4,
	RTV_CODE_RATE_5_6,
	RTV_CODE_RATE_7_8
} E_RTV_CODE_RATE_TYPE;



/*==============================================================================
 *
 * ISDB-T definitions and types.
 *
 *============================================================================*/

#define RTV_ISDBT_OFDM_LOCK_MASK		0x1
#define RTV_ISDBT_TMCC_LOCK_MASK		0x2
#define RTV_ISDBT_CHANNEL_LOCK_OK       (RTV_ISDBT_OFDM_LOCK_MASK|RTV_ISDBT_TMCC_LOCK_MASK)

#define RTV_ISDBT_BER_DIVIDER		100000.0
#define RTV_ISDBT_CNR_DIVIDER		10000.0
#define RTV_ISDBT_RSSI_DIVIDER		10.0


#ifdef RTV_ISDBT_ENABLE
  #if defined(RTV_CONUTRY_JAPAN)	  
	#define RTV_ISDBT_FREQ2CHNUM(freq_khz)	(((freq_khz) - 395143) / 6000)
  #elif defined(RTV_CONUTRY_BRAZIL) || defined(RYV_CONUTRY_ARGENTINA)
	#define RTV_ISDBT_FREQ2CHNUM(freq_khz)	((((freq_khz) - 395143) / 6000) + 1)
  #else 
    #error "Code not present"
  #endif
#endif


typedef enum
{
	RTV_ISDBT_SEG_1 = 0,
	RTV_ISDBT_SEG_3
} E_RTV_ISDBT_SEG_TYPE;

typedef enum
{
	RTV_ISDBT_MODE_1 = 0, // 2048
	RTV_ISDBT_MODE_2,	  // 4096
	RTV_ISDBT_MODE_3      // 8192 fft
} E_RTV_ISDBT_MODE_TYPE;

typedef enum
{
	RTV_ISDBT_GUARD_1_32 = 0, /* 1/32 */
	RTV_ISDBT_GUARD_1_16,     /* 1/16 */
	RTV_ISDBT_GUARD_1_8,      /* 1/8 */
	RTV_ISDBT_GUARD_1_4       /* 1/4 */
} E_RTV_ISDBT_GUARD_TYPE;


typedef enum
{
	RTV_ISDBT_INTERLV_0 = 0,
	RTV_ISDBT_INTERLV_1,
	RTV_ISDBT_INTERLV_2,
	RTV_ISDBT_INTERLV_4,
	RTV_ISDBT_INTERLV_8,
	RTV_ISDBT_INTERLV_16,
	RTV_ISDBT_INTERLV_32
} E_RTV_ISDBT_INTERLV_TYPE;



// for Layer A.
typedef struct
{
	E_RTV_ISDBT_SEG_TYPE		eSeg;
	E_RTV_ISDBT_MODE_TYPE		eTvMode;
	E_RTV_ISDBT_GUARD_TYPE		eGuard;
	E_RTV_MODULATION_TYPE		eModulation;
	E_RTV_CODE_RATE_TYPE		eCodeRate;
	E_RTV_ISDBT_INTERLV_TYPE	eInterlv;
	int						fEWS;	
} RTV_ISDBT_TMCC_INFO;


/*==============================================================================
 *
 * FM definitions and types.
 *
 *============================================================================*/
#define RTV_FM_CH_MIN_FREQ_KHz		76000
#define RTV_FM_CH_MAX_FREQ_KHz		108000
#define RTV_FM_CH_STEP_FREQ_KHz		50 // in KHz
 
#define RTV_FM_PILOT_LOCK_MASK		0x1
#define RTV_FM_RDS_LOCK_MASK		0x2
#define RTV_FM_CHANNEL_LOCK_OK      (RTV_FM_PILOT_LOCK_MASK|RTV_FM_RDS_LOCK_MASK)


/*==============================================================================
 *
 * TDMB definitions and types.
 *
 *============================================================================*/
#define RTV_TDMB_OFDM_LOCK_MASK		0x1
#define RTV_TDMB_FEC_LOCK_MASK		0x2
#define RTV_TDMB_CHANNEL_LOCK_OK    (RTV_TDMB_OFDM_LOCK_MASK|RTV_TDMB_FEC_LOCK_MASK)

#define RTV_TDMB_BER_DIVIDER		100000.0
#define RTV_TDMB_CNR_DIVIDER		1000.0
#define RTV_TDMB_RSSI_DIVIDER		10.0

typedef enum
{
	RTV_TDMB_SERVICE_VIDEO = 0,
	RTV_TDMB_SERVICE_AUDIO,
	RTV_TDMB_SERVICE_DATA	
} E_RTV_TDMB_SERVICE_TYPE;


 
#ifdef __cplusplus 
} 
#endif 

#endif /* __RAONTV_TYPES_H__ */

