/**************************************************************************************

 *  CONFIDENTIAL AND PROPRIETARY SOFTWARE/DATA OF Auvitek International, Ltd.         * 

 *                                                                                    *

 *  Copyright (c) 2005-2018 Auvitek International, Ltd.  All Rights Reserved.         *

 *                                                                                    *

 *  Use of this Software/Data is subject to the terms and conditions of               *

 *  the applicable agreement between Auvitek International, Ltd, and receiving party. *

 *  In addition, this Software/Data is protected by copyright law and international   *

 *  treaties.                                                                         *

 *                                                                                    *

 *  The copyright notice(s) in this Software/Data does not indicate actual            *

 *  or intended publication of this Software/Data.                                    *

***************************************************************************************
**
**  Name:			AU8524_Reference.h
**  Project:        AU8524 IIC Controller
**  Description:	software interface 
**
**  Functions
**  Implemented:   
**                  UINT16  AU8524_InitialATSC
**                  UINT16  AU8524_GetSignalLevel
**                  UINT16  AU8524_GetJ83BSignalQuality
**                  UINT16  AU8524_GetATSCSignalQuality
**                  UINT16  AU8524_IsATSCSignalLocked
**                  UINT16  AU8524_IsJ83B64SignalLocked
**                  UINT16  AU8524_IsJ83B256SignalLocked
**					UINT16	AU8524_ReadReg
**					UINT16	AU8524_WriteReg
**					UINT16  AU8524_AfterSetTuner
**					UINT16  AU8524_Timer
**				    UINT16	AU8524_GetFMSignalLevel	    
**				    UINT16	AU8524_ShowError  
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History:
**
**
*****************************************************************************/
#if !defined( __AU8524_REFERENCE_H )
#define __AU8524_REFERENCE_H


#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage                  */
{
#endif

#include "../Common/AUChipTypedef.h"

/*****************************************************************************
* function:    AU8524_ChipOpen
* Description: Open chip
* Input:       nAPHandle
*
* Return:      ChipUnit
*                  
*****************************************************************************/
INT32 AU8524_ChipOpen(void* nAPHandle);
/*****************************************************************************
* function:    AU8524_SetTunerCapabilities
* Description: Set chip support TV format
* Input:       nAPHandle
*
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetTunerCapabilities(UINT32 ChipUnit,INT32 nTunerType,AU_TunerCapabilities nTunerCapabilities);
/*****************************************************************************
* function:    AU8524_ChipClose
* Description: Close chip
* Input:       nAPHandle
*
* Return:      ChipUnit
*                  
*****************************************************************************/
TUNER_NTSTATUS	AU8524_ChipClose(UINT32 ChipUnit);
/*****************************************************************************
* function:    AU8524_SetATVColorParamete
* Description: Set ATV parameter(sample:Contrast, hue, brightness)
* Input:		nAPHandle
*				nATVColorParamete
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVColorParamete(UINT32 ChipUnit,AU_ATVColorParamete nATVColorParamete);
/*****************************************************************************
* function:    AU8524_SetATVAudioFormat
* Description: Set ATV Audio format
* Input:		nAPHandle
*				nATVAudioFormat
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVAudioFormat(UINT32 ChipUnit,AU_ATVAudioFormat nATVAudioFormat);
/*****************************************************************************
* function:    AU8524_SetATVVolumeValue
* Description: Set ATV Audio Volume
* Input:		nAPHandle
*				nATVVolumeValue
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVVolumeValue(UINT32 ChipUnit,AU_ATVVolumeValue nATVVolumeValue);
/*****************************************************************************
* function:    AU8524_SetATVParamete
* Description: Set ATV parameter
* Input:		nAPHandle
*				nATVParamete
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVParamete(UINT32 ChipUnit,AU_ATVParamete nATVParamete);
/*****************************************************************************
* function:    AU8524_GetChipLock
* Description: Get Chip lock status
* Input:		nAPHandle
*				* nLock
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_GetChipLock(UINT32 ChipUnit,UINT32 * nLock);
/*****************************************************************************
* function:    AU8524_GetChipSignalState
* Description: Get Signal Status
* Input:		nAPHandle
*				p_para
*				SignalState
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_GetChipSignalState(UINT32 ChipUnit, AU_ChipParamete p_para, AU_SignalState * nSignalState);
/*****************************************************************************
* function:    AU8524_SetChipMod
* Description: set demodulator
* Input:		ChipUnit
*				p_para
*
* Return:      status:
*                  STATUS_OK: no error
*****************************************************************************/
TUNER_NTSTATUS	AU8524_SetChipMod(UINT32 ChipUnit, AU_ChipParamete p_para);
/*****************************************************************************
* function:    AU8524_OnTime
* Description: Timer
* Input:		ChipUnit
*
* Return:      status:
*
*****************************************************************************/
TUNER_NTSTATUS  AU8524_OnTime(UINT32 ChipUnit);
/*****************************************************************************
* function:    AU8524_GetModOffset
* Description: Get Timing or Frequency offset
* Input:		ChipUnit
*				nMod
*				*Offset
* Return:      status:
*
*****************************************************************************/
TUNER_NTSTATUS  AU8524_GetModOffset(UINT32 ChipUnit,AU_MODOFFSET_T nMod,double *Offset);
/*****************************************************************************
* function:    AU8524_SettingWithTuner
* Description: Set chip register setting for tuner
* Input:		ChipUnit
*
*
* Return:      status:
*
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SettingWithTuner(UINT32 ChipUnit);
/*****************************************************************************
* function:    AU8524_GetDemodAbility
* Description: Get chip support mode
* Input:		ChipUnit
*
*
* Return:
*****************************************************************************/
UINT32			AU8524_GetDemodAbility(UINT32 ChipUnit, AU_CHIPTYPE_T nChipType); 	
/*****************************************************************************
* function:    AU8524_GetAudioAbility
* Description: Get Audio support mode
* Input:		ChipUnit
*
*
* Return:
*****************************************************************************/	
UINT32			AU8524_GetAudioAbility(UINT32 ChipUnit); 


/* ATSC module */
/*****************************************************************************
* function:    AU8524_InitialATSC
* Description: set chip in ATSC mode
* Input:       void
* Return:      status:
*                  STATUS_OK: no error
*****************************************************************************/
UINT16 AU8524_InitialATSC(void* nAPHandle); 
/*****************************************************************************
* function:    AU8524_ATSCSettingWithTuner()
* Description: Some settings(for example, AGC) need to modify with tuner type
* Input:       void
* Return:      status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_ATSCSettingWithTuner(void* nAPHandle, AU_ChipParamete p_para);
/*****************************************************************************
* function:    AU8524_LowIF
* Description: Set AU85xx input IF according to the IF output from the tuner
* Input:       fIF (MHz)
* Return:      status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_LowIF(void* nAPHandle, AU_TUNERTYPE_T nTunerType, AU_TV_FORMAT_T nTVFormat, UINT32 fIF);
/*****************************************************************************
* function:    AU8524_IsATSCSignalLocked
* Description: check ATSC signal lock status
* Input:       void
* Return:      1 SIGNAL_IS_LOCKED 
*              2 SIGNAL_NOT_LOCKED
*		       3 status:
*			   STATUS_OK: no error
*                    user-defined
*****************************************************************************/
UINT16 AU8524_IsATSCSignalLocked(void* nAPHandle);
/*****************************************************************************
* function:    AU8524_GetSignalLevel
* Description: get signal level with all tuner
*			   (include NTSC, ATSC, QAM)
* Input:       p_para
*				   the pointer of the parameters to set the AU85xx chip and tuner
*
* Output:      pSignalLevel: the pointer to  signal level
			   (the range is 0 to 10, the best level is 10, the worst is 0)
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_GetSignalLevel(void* nAPHandle,AU_TV_FORMAT_T TVMod,INT32 nTunerType,UINT16 *pSignalLevel);
/*****************************************************************************
* function:    AU8524_GetATSCSignalQuality
* Description: get signal quality of ATSC 
* Output:      fSignalQuality: the pointer to signal quality
			   pSignalQuality: the pointer to signal Quality,return snr in dB	
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_GetATSCSignalQuality(void* nAPHandle,UINT16 *fSignalQuality);
/*****************************************************************************
* function:     AU8524_ShowError
* Description:  show error information, you can call this function when you display
*				the information, the best is about every 50 ms call one time to 
*				check whether error
*				
* Input:	    nMode
*				0:ATSC_MODE
*				1:J83B64_MODE
*				2:J83B256_MODE 
* Return:       1, error
*			    0, no error  
*****************************************************************************/
UINT16 AU8524_ShowError(void* nAPHandle,UINT8 nMode); 
/*****************************************************************************
*function:	   AU8524_ReadReg
*Description:  Read values from the register .
* Input:       1 nAddr: the register address to be read
*              2 pData: pointer to the Data to receive the value from the register
* Return:      status:
*              STATUS_OK: no error
*              user-defined
*****************************************************************************/
UINT16 AU8524_ReadReg(void* nAPHandle,UINT16 nAddr, UINT8 *pData);
/*****************************************************************************
*function:	   AU8524_WriteReg
*Description:  Write values to device .
* Input:       1 nAddr: the register address to be written
*              2 pData: the data to be written to the  register
* Return:      status:
*              STATUS_OK: no error
*              user-defined
*****************************************************************************/
UINT16 AU8524_WriteReg(void* nAPHandle,UINT16 nAddr, UINT8 pData);
/*****************************************************************************
* function:    AU8524_AfterSetTuner(void)
* Description: set some registers after set tuner(when tuner frequency changes)
* Input:       void
* Return:      status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_AfterSetTuner(void* nAPHandle);
/*****************************************************************************
* function:    AU8524_OpenIIC2(void)
* Description: open IIC2 before set tuner in 8524
* Input:       void
* Return:      status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_Timer(void* nAPHandle, AU_ChipParamete p_para);

/* J83B module*/
/*****************************************************************************
* function:    AU8524_InitialJ83B64
* Description: set chip in J83B64 mode
* Input:       void
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_InitialJ83B64(void* nAPHandle);
/*****************************************************************************
* function:    AU8524_InitialJ83B256
* Description: set chip in J83B256 mode
* Input:       void
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_InitialJ83B256(void* nAPHandle); 
/*****************************************************************************
* function:    AU8524_J83BSettingWithTuner()
* Description: Some settings(for example, AGC) need to modify with tuner type
* Input:       void
* Return:      status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_J83BSettingWithTuner(void* nAPHandle, AU_ChipParamete p_para);
/*****************************************************************************
* function:    AU8524_IsJ83B256SignalLocked
* Description: check J83B256 signal lock status
* Input:       void
*
* Return:		1 SIGNAL_IS_LOCKED 
*				2 SIGNAL_NOT_LOCKED
*				3 status:
*			   STATUS_OK: no error
*                    user-defined
*****************************************************************************/
UINT16 AU8524_IsJ83B256SignalLocked(void* nAPHandle,INT32 nTunerType);
/*****************************************************************************
* function:    AU8524_IsJ83B64SignalLocked
* Description: check J83B64 signal lock status
* Input:       void
*
* Return:		1 SIGNAL_IS_LOCKED 
*				2 SIGNAL_NOT_LOCKED
*				3 status:
*			   STATUS_OK: no error
*                    user-defined
*****************************************************************************/
UINT16 AU8524_IsJ83B64SignalLocked(void* nAPHandle,INT32 nTunerType);
/*****************************************************************************
* function:    AU8524_GetJ83BSignalQuality
* Description: get signal quality of J83B(include J83B64 and J83B256)
*			   call AU8524_GetJ83BSignalQuality every ~30 ms, display signal quality(snr) every >0.5 sec 
* Input:       mode 
			     1: J83B64_MODE
			     2: J83B256_MODE
* Output:      pSignalQuality: the pointer to signal Quality,return snr in dB
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_GetJ83BSignalQuality(void* nAPHandle, UINT16 *pSignalQuality,AU_TV_FORMAT_T mode);
#ifdef IIC_Demo
/*****************************************************************************
*function:	AU8524_GetATSC_Freq_offset
*Description:	Get ATSC Freq offset
*input:		*freqOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetATSC_Freq_Offset(void* nAPHandle,long *FreqOffset);
/*****************************************************************************
*function:	AU8524_GetATSC_Timing_Offset
*Description:	Get ATSC Timing offset
*input:		*TimingOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetATSC_Timing_Offset(void* nAPHandle,long *TimingOffset);
/*****************************************************************************
*function:	AU8524_GetJ83B_Freq_Offset
*Description:	Get J83B Freq offset
*input:		*FreqOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetJ83B_Freq_Offset(void* nAPHandle,double *FreqOffset);
/*****************************************************************************
*function:	AU8524_GetJ83B_Freq_Offset
*Description:	Get J83B Freq offset
*input:		TVFormat
*				1:J83B64
*				2:J83B256
*			*FreqOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetJ83B_Timing_Offset(void* nAPHandle,AU_TV_FORMAT_T nChipMod,double *TimingOffset);
#endif
/*****************************************************************************
* function:    AU8524_SetTSOutput
* Description: Set TS output via serial or parallel
* Input:       nTSOutput:	1 TS_SERIAL_OUTPUT
							2 TS_PARALLEL_OUTPUT
* Return:     status:
*			   STATUS_OK: no error
*                     user-defined
******************************************************************************/
UINT16 AU8524_SetTSOutput(void* nAPHandle, AU_TS_OUTPUT_T nTSOutput);

#if defined( __cplusplus )
}
#endif

#endif
