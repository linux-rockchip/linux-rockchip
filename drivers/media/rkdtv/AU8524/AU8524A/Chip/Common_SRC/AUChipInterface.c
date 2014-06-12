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
**  Name: AUChipInterface.c
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

#include "../Common/AUChipInterface.h"
#include "../AU8524/AU8524_Reference.h"

AU_ChipInterFaceST m_AU_ChipInterFaceST[CHIP_MAX_UNITS];

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
INT32	AU_ChipOpen(void* nAPHandle,AU_CHIPTYPE_T nChipType)
{

	int   nPos = 0;
	int pos_Found = 0;
	INT32 ChipUnit = -1;

	static int initflag = 0;

	if(initflag == 0)
	{
		for(nPos=0;nPos<CHIP_MAX_UNITS;nPos++)
		{
			m_AU_ChipInterFaceST[nPos].AU_ChipOpen_Interface				= 0;			
			m_AU_ChipInterFaceST[nPos].AU_ChipClose_Interface				= 0;
			m_AU_ChipInterFaceST[nPos].AU_SetTunerCapabilities_Interface	= 0;
			m_AU_ChipInterFaceST[nPos].AU_SetChipMod_Interface				= 0;
			m_AU_ChipInterFaceST[nPos].AU_GetChipLock_Interface				= 0;
			m_AU_ChipInterFaceST[nPos].AU_GetChipSignalState_Interface		= 0;
			m_AU_ChipInterFaceST[nPos].AU_GetModOffset_Interface			= 0;
			m_AU_ChipInterFaceST[nPos].AU_SetATVColorParamete_Interface		= 0;
			m_AU_ChipInterFaceST[nPos].AU_SetATVAudioFormat_Interface		= 0;
			m_AU_ChipInterFaceST[nPos].AU_SetATVVolumeValue_Interface		= 0;
			m_AU_ChipInterFaceST[nPos].AU_SetATVParamete_Interface			= 0;
			m_AU_ChipInterFaceST[nPos].AU_SettingWithTuner_Interface		= 0;
			m_AU_ChipInterFaceST[nPos].AU_OnTime_Interface					= 0;
			m_AU_ChipInterFaceST[nPos].AU_GetDemodAbility_Interface			= 0;
			m_AU_ChipInterFaceST[nPos].AU_GetAudioAbility_Interface			= 0;
			m_AU_ChipInterFaceST[nPos].APHandle								= 0;
			m_AU_ChipInterFaceST[nPos].ChipType								= (AU_CHIPTYPE_T)0;
			m_AU_ChipInterFaceST[nPos].ChipHandle							= -1;
			m_AU_ChipInterFaceST[nPos].TunerType							= -1;
		}

		initflag = 1;
	}


	if(nAPHandle == 0)
	{
		return -1;
	}

	for(nPos=0;nPos<CHIP_MAX_UNITS;nPos++)
	{
		if(		m_AU_ChipInterFaceST[nPos].APHandle	== nAPHandle 
			&&	m_AU_ChipInterFaceST[nPos].ChipType	== nChipType)
		{
			ChipUnit = nPos;
			pos_Found = 1;
			break;
		}
	}

	if (!pos_Found)
	{
		for(nPos=0;nPos<CHIP_MAX_UNITS;nPos++)
		{
			if(		m_AU_ChipInterFaceST[nPos].APHandle		== 0 
				&&	m_AU_ChipInterFaceST[nPos].ChipType		== 0 
				&&	m_AU_ChipInterFaceST[nPos].ChipHandle	== -1
				&&	m_AU_ChipInterFaceST[nPos].TunerType	== -1)
			{
				ChipUnit = nPos;
				
				break;
			}
		}
	}

	if(ChipUnit == -1)
	{
		return -1;
	}

	m_AU_ChipInterFaceST[ChipUnit].AU_ChipOpen_Interface				= AU8524_ChipOpen;
	m_AU_ChipInterFaceST[ChipUnit].AU_ChipClose_Interface				= AU8524_ChipClose;
	m_AU_ChipInterFaceST[ChipUnit].AU_SetTunerCapabilities_Interface	= AU8524_SetTunerCapabilities;
	m_AU_ChipInterFaceST[ChipUnit].AU_SetChipMod_Interface				= AU8524_SetChipMod;
	m_AU_ChipInterFaceST[ChipUnit].AU_GetChipLock_Interface				= AU8524_GetChipLock;
	m_AU_ChipInterFaceST[ChipUnit].AU_GetChipSignalState_Interface		= AU8524_GetChipSignalState;
	m_AU_ChipInterFaceST[ChipUnit].AU_GetModOffset_Interface			= AU8524_GetModOffset;
	m_AU_ChipInterFaceST[ChipUnit].AU_SetATVColorParamete_Interface		= AU8524_SetATVColorParamete;
	m_AU_ChipInterFaceST[ChipUnit].AU_SetATVAudioFormat_Interface		= AU8524_SetATVAudioFormat;
	m_AU_ChipInterFaceST[ChipUnit].AU_SetATVVolumeValue_Interface		= AU8524_SetATVVolumeValue;
	m_AU_ChipInterFaceST[ChipUnit].AU_SetATVParamete_Interface			= AU8524_SetATVParamete;
	m_AU_ChipInterFaceST[ChipUnit].AU_SettingWithTuner_Interface		= AU8524_SettingWithTuner;
	m_AU_ChipInterFaceST[ChipUnit].AU_OnTime_Interface					= AU8524_OnTime;
	m_AU_ChipInterFaceST[ChipUnit].AU_GetDemodAbility_Interface			= AU8524_GetDemodAbility;
	m_AU_ChipInterFaceST[ChipUnit].AU_GetAudioAbility_Interface			= AU8524_GetAudioAbility;

	m_AU_ChipInterFaceST[ChipUnit].APHandle		= nAPHandle;
	m_AU_ChipInterFaceST[ChipUnit].ChipType		= nChipType;


	if(m_AU_ChipInterFaceST[ChipUnit].AU_ChipOpen_Interface)
	{
		INT32 ResultHandle = m_AU_ChipInterFaceST[ChipUnit].AU_ChipOpen_Interface(nAPHandle);

		if(ResultHandle != -1)
		{
			m_AU_ChipInterFaceST[ChipUnit].ChipHandle = ResultHandle;
		}
		else
		{
			m_AU_ChipInterFaceST[ChipUnit].AU_ChipOpen_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_ChipClose_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetTunerCapabilities_Interface	= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetChipMod_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetChipLock_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetChipSignalState_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetModOffset_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVColorParamete_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVAudioFormat_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVVolumeValue_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVParamete_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SettingWithTuner_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_OnTime_Interface					= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetDemodAbility_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetAudioAbility_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].APHandle								= 0;
			m_AU_ChipInterFaceST[ChipUnit].ChipType								= (AU_CHIPTYPE_T)0;
			m_AU_ChipInterFaceST[ChipUnit].ChipHandle							= -1;
			m_AU_ChipInterFaceST[ChipUnit].TunerType							= -1;

			ChipUnit = -1;
		}

		
	}

	return ChipUnit;
}

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
TUNER_NTSTATUS  AU_SetTunerCapabilities(UINT32 ChipUnit,INT32 nTunerType,AU_TunerCapabilities nTunerCapabilities)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_SetTunerCapabilities_Interface)
	{
		m_AU_ChipInterFaceST[ChipUnit].TunerType							= nTunerType;

		return m_AU_ChipInterFaceST[ChipUnit].AU_SetTunerCapabilities_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle,nTunerType,nTunerCapabilities);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
	
}

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
TUNER_NTSTATUS	AU_ChipClose(UINT32 ChipUnit)
{

	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if(m_AU_ChipInterFaceST[ChipUnit].AU_ChipClose_Interface)
	{
		ntStatus = m_AU_ChipInterFaceST[ChipUnit].AU_ChipClose_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle);

		if(ntStatus == TUNER_STATUS_SUCCESS)
		{
			m_AU_ChipInterFaceST[ChipUnit].AU_ChipOpen_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_ChipClose_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetTunerCapabilities_Interface	= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetChipMod_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetChipLock_Interface				= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetChipSignalState_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetModOffset_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVColorParamete_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVAudioFormat_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVVolumeValue_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SetATVParamete_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_SettingWithTuner_Interface		= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_OnTime_Interface					= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetDemodAbility_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].AU_GetAudioAbility_Interface			= 0;
			m_AU_ChipInterFaceST[ChipUnit].APHandle								= 0;
			m_AU_ChipInterFaceST[ChipUnit].ChipType								= (AU_CHIPTYPE_T)0;
			m_AU_ChipInterFaceST[ChipUnit].ChipHandle							= -1;

			return ntStatus;
		}
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS	AU_SetChipMod(UINT32 ChipUnit,AU_ChipParamete p_para)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_SetChipMod_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_SetChipMod_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle, p_para);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_GetChipLock(UINT32 ChipUnit,UINT32 * nLock)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_GetChipLock_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_GetChipLock_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle,nLock);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_GetChipSignalState(UINT32 ChipUnit, AU_ChipParamete p_para, AU_SignalState * nSignalState)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_GetChipSignalState_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_GetChipSignalState_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle, p_para, nSignalState);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_GetModOffset(UINT32 ChipUnit,AU_MODOFFSET_T nMod,double *Offset)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_GetModOffset_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_GetModOffset_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle,nMod,Offset);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_SetATVColorParamete(UINT32 ChipUnit,AU_ATVColorParamete nATVColorParamete)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_SetATVColorParamete_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_SetATVColorParamete_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle,nATVColorParamete);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_SetATVAudioFormat(UINT32 ChipUnit,AU_ATVAudioFormat nATVAudioFormat)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_SetATVAudioFormat_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_SetATVAudioFormat_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle,nATVAudioFormat);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_SetATVVolumeValue(UINT32 ChipUnit,AU_ATVVolumeValue nATVVolumeValue)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_SetATVVolumeValue_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_SetATVVolumeValue_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle,nATVVolumeValue);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_SetATVParamete(UINT32 ChipUnit,AU_ATVParamete nATVParamete)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_SetATVParamete_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_SetATVParamete_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle,nATVParamete);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_SettingWithTuner(UINT32 ChipUnit)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_SettingWithTuner_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_SettingWithTuner_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
TUNER_NTSTATUS  AU_OnTime(UINT32 ChipUnit)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}
	
	if(m_AU_ChipInterFaceST[ChipUnit].AU_OnTime_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_OnTime_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle);
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

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
UINT32	AU_GetDemodAbility(UINT32 ChipUnit, AU_CHIPTYPE_T nChipType)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return 0;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_GetDemodAbility_Interface)
	{
		
		return m_AU_ChipInterFaceST[ChipUnit].AU_GetDemodAbility_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle, nChipType);
	}

	return 0;
}

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
UINT32			AU_GetAudioAbility(UINT32 ChipUnit)
{
	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return 0;
	}

	if(m_AU_ChipInterFaceST[ChipUnit].AU_GetAudioAbility_Interface)
	{
		return m_AU_ChipInterFaceST[ChipUnit].AU_GetAudioAbility_Interface(m_AU_ChipInterFaceST[ChipUnit].ChipHandle);
	}

	return 0;
}
