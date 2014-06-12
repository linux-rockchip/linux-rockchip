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
* FILENAME    : raontv.h
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
* 07/10/2010  Ko, Kevin        Move the defintions to raontv_types.h
* 09/29/2010  Ko, Kevin        Splited the data type to raontv_types.h
* 09/27/2010  Ko, Kevin        Creat for CS Realease
*             /Yang, Maverick  1.Reformating for CS API
*                              2.pll table, ADC clock switching, SCAN function, 
*								 FM function added..
********************************************************************************/

#ifndef __RAONTV_H__
#define __RAONTV_H__

#ifdef __cplusplus 
extern "C"{ 
#endif  

#include "../oem/raontv_port.h"
#include "raontv_types.h"


/*==============================================================================
 * Error codes
 *============================================================================*/
#define RTV_SUCCESS							0
#define RTV_UNSUPPORT_ADC_CLK				-1
#define RTV_INVAILD_TV_MODE					-2
#define RTV_CHANNEL_NOT_DETECTED			-3
#define RTV_INSUFFICIENT_CHANNEL_BUF		-4
#define RTV_INVAILD_FREQ					-5


/*==============================================================================
 * ISDBT APIs 
 *============================================================================*/
void rtvISDBT_StandbyMode(int on);
UINT rtvISDBT_GetLockStatus(void); 
S32  rtvISDBT_GetRSSI(void);
U32  rtvISDBT_GetPER(void);
U32  rtvISDBT_GetCNR(void);
U32  rtvISDBT_GetBER(void);
void rtvISDBT_GetTMCC(RTV_ISDBT_TMCC_INFO *ptTmccInfo);
INT  rtvISDBT_SetFrequency(UINT nChNum);
INT  rtvISDBT_ScanFrequency(UINT nChNum);
INT  rtvISDBT_Initialize(E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType);


/*==============================================================================
 * FM APIs 
 *============================================================================*/
void rtvFM_StandbyMode(int on);
void rtvFM_GetLockStatus(UINT *pLockVal, UINT *pLockCnt);
INT  rtvFM_SetFrequency(U32 dwChFreqKHz);
INT  rtvFM_ScanFrequency(U32 *pChBuf, UINT nNumChBuf, U32 dwStartFreqKHz, U32 dwEndFreqKHz);
INT  rtvFM_Initialize(E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType); 


/*==============================================================================
 * TDMB APIs 
 *============================================================================*/
void rtvTDMB_StandbyMode(int on);
UINT rtvTDMB_GetLockStatus(void);
S32  rtvTDMB_GetRSSI(void);
U32  rtvTDMB_GetCNR(void);
U32  rtvTDMB_GetBER(void);
void rtvTDMB_ConfigureFIC(void);
INT  rtvTDMB_SetSubChannel(UINT nSubChID, E_RTV_TDMB_SERVICE_TYPE eServiceType, U32 dwChFreqKHz);
INT  rtvTDMB_SetFrequency(U32 dwChFreqKHz);
INT  rtvTDMB_ScanFrequency(U32 dwChFreqKHz);
INT  rtvTDMB_Initialize(E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType); 


/*==============================================================================
 * DAB APIs 
 *============================================================================*/
void rtvDAB_StandbyMode(int on);
S32  rtvDAB_GetSNR(void);
U32  rtvDAB_GetBER(void);
INT  rtvDAB_ScanFrequency(U32 dwChFreqKHz);
INT  rtvDAB_SetFrequency(U32 dwChFreqKHz);
INT  rtvDAB_Initialize(E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType); 


 
#ifdef __cplusplus 
} 
#endif 

#endif /* __RAONTV_H__ */

