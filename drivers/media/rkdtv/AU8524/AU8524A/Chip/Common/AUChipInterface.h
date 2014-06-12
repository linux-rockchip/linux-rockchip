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
**  Name: AUChipInterface.h
**
**  Description:  
**
**  Functions
**  Requiring
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History:
**
**
**************************************************************************************/

#if !defined( __AUVITEKINTERFACE_H )
#define __AUVITEKINTERFACE_H

#include "AUChipTypedef.h"

#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage                  */
{
#endif

/*****************************************************************************
**
**  Name: AU_ChipOpen
**
**  Description:    Open the Chip.
**
**  Parameters:   
**                  nAPHandle	- the tuner handle
**                  nChipType	- Chip type
**
**
**  Returns:        status:
**                  user-defined
*******************************************************************************/
INT32			AU_ChipOpen(void* nAPHandle,AU_CHIPTYPE_T nChipType);
/*****************************************************************************
**
**  Name: AU_SetTunerCapabilities
**
**  Description:    Set tuner Capabilities
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					nTunerType	- Tuner type
**					nTunerCapabilities - Tuner Capabilities
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_SetTunerCapabilities(UINT32 ChipUnit,INT32 nTunerType,AU_TunerCapabilities nTunerCapabilities);
/*****************************************************************************
**
**  Name: AU_ChipClose
**
**  Description:    Close chip
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS	AU_ChipClose(UINT32 ChipUnit);
/*****************************************************************************
**
**  Name: AU_SetChipMod
**
**  Description:    Set chip mode
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					p_para		- parameter
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS	AU_SetChipMod(UINT32 ChipUnit, AU_ChipParamete p_para);
/*****************************************************************************
**
**  Name: AU_GetChipLock
**
**  Description:    Get signal lock status
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					nLock		- lock status
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_GetChipLock(UINT32 ChipUnit,UINT32 * nLock);
/*****************************************************************************
**
**  Name: AU_GetChipSignalState
**
**  Description:    Get signal status
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					p_para		- chip parameter
**					nSignalState	- signal level
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_GetChipSignalState(UINT32 ChipUnit, AU_ChipParamete p_para, AU_SignalState * nSignalState);
/*****************************************************************************
**
**  Name: AU_GetModOffset
**
**  Description:    Get chip timing and freq offset
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					nMod		- chip mode
**					Offset		- timing and freq offset
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_GetModOffset(UINT32 ChipUnit,AU_MODOFFSET_T nMod,double *Offset);
/*****************************************************************************
**
**  Name: AU_SetATVColorParamete
**
**  Description:    Set ATV color format
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					nATVColorParamete - ATV color format
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_SetATVColorParamete(UINT32 ChipUnit,AU_ATVColorParamete nATVColorParamete);
/*****************************************************************************
**
**  Name: AU_SetATVAudioFormat
**
**  Description:    Set ATV audio format
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					nATVAudioFormat - ATV audio format
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_SetATVAudioFormat(UINT32 ChipUnit,AU_ATVAudioFormat nATVAudioFormat);
/*****************************************************************************
**
**  Name: AU_SetATVVolumeValue
**
**  Description:    Set ATV Volume
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					nATVVolumeValue - ATV Volume
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_SetATVVolumeValue(UINT32 ChipUnit,AU_ATVVolumeValue nATVVolumeValue);
/*****************************************************************************
**
**  Name: AU_SetATVParamete
**
**  Description:    Set ATV parameter
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**					nATVParamete - ATV parameter
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_SetATVParamete(UINT32 ChipUnit,AU_ATVParamete nATVParamete);
/*****************************************************************************
**
**  Name: AU_SettingWithTuner
**
**  Description:    Setting with tuner
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_SettingWithTuner(UINT32 ChipUnit);
/*****************************************************************************
**
**  Name: AU_OnTime
**
**  Description:    Timer
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
TUNER_NTSTATUS  AU_OnTime(UINT32 ChipUnit);
/*****************************************************************************
**
**  Name: AU_GetDemodAbility
**
**  Description:    Get Chip demodulator ability
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
UINT32			AU_GetDemodAbility(UINT32 ChipUnit, AU_CHIPTYPE_T nChipType);
/*****************************************************************************
**
**  Name: AU_GetAudioAbility
**
**  Description:    Get Audio ability
**
**  Parameters:   
**                  ChipUnit	- Chip unit
**
**
**  Returns:        status:
**                  user-defined
**
*******************************************************************************/
UINT32			AU_GetAudioAbility(UINT32 ChipUnit); 

#if defined( __cplusplus )
}
#endif

#endif