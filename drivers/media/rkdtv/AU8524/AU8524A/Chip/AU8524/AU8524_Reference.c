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

*****************************************************************************************
**  Name:				AU8524_Reference.c
**  Project:            AU8524 IIC Controller		
**
**  Description:		Auvitek Au8524 SW interface describes how to configure the chip 
**                      in different mode.
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History: 
*****************************************************************************************/

#include "AU8524_RegisterDef.h"
#include "AU8524_Reference.h"
#include "../Common/AuvitekChipDev.h"
//#include <linux/math.h>
#include <linux/string.h>
#include <linux/delay.h>

/*  Version of this module                         */
#define VERSION 2281141         /*  Version 228.1141 */

#define ABS(x) ((x) < 0 ? -(x) : (x))

AU_ChipParamete	m_AUChipHandle_8524[CHIP_MAX_UNITS];

/*****************************************************************************
* function:    AU8524_ChipOpen
* Description: Open chip
* Input:       nAPHandle
*
* Return:      ChipUnit
*                  
*****************************************************************************/
INT32 AU8524_ChipOpen(void* nAPHandle)
{
	int   nPos = 0;
	INT32 ChipUnit = -1;

	static int initflag = 0;

	if(initflag == 0)
	{
		for(nPos=0;nPos<CHIP_MAX_UNITS;nPos++)
		{
			m_AUChipHandle_8524[nPos].APHandle		= 0;
			m_AUChipHandle_8524[nPos].TunerType		= TUNER_UNKNOWN;
			m_AUChipHandle_8524[nPos].ChipMod		= TV_UNKNOWN;
			m_AUChipHandle_8524[nPos].TSOutput		= TS_UNKNOWN;

		}

		initflag = 1;
	}

	if(nAPHandle == 0)
	{
		return -1;
	}

	for(nPos=0;nPos<CHIP_MAX_UNITS;nPos++)
	{
		if( m_AUChipHandle_8524[nPos].APHandle == nAPHandle )
		{
			return nPos;
		}
	}

	for(nPos=0;nPos<CHIP_MAX_UNITS;nPos++)
	{
		if( m_AUChipHandle_8524[nPos].APHandle == 0 )
		{
			ChipUnit = nPos;
			
			break;
		}
	}

	if(ChipUnit == -1)
	{
		return -1;
	}
			

	m_AUChipHandle_8524[ChipUnit].APHandle							= nAPHandle;

	return ChipUnit;
}

/*****************************************************************************
* function:    AU8524_SetTunerCapabilities
* Description: Set chip support TV format
* Input:       nAPHandle
*
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetTunerCapabilities(UINT32 ChipUnit,INT32 nTunerType,AU_TunerCapabilities nTunerCapabilities)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	m_AUChipHandle_8524[ChipUnit].TunerType							= nTunerType;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_ATSC		= nTunerCapabilities.LowIF_ATSC;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_QAM		= nTunerCapabilities.LowIF_QAM;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_DVB_T		= nTunerCapabilities.LowIF_DVB_T;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_PAL_I		= nTunerCapabilities.LowIF_PAL_I;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_PAL_BG	= nTunerCapabilities.LowIF_PAL_BG;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_PAL_DK	= nTunerCapabilities.LowIF_PAL_DK;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_FM		= nTunerCapabilities.LowIF_FM;
	m_AUChipHandle_8524[ChipUnit].TunerCapabilities.LowIF_DTMB		= nTunerCapabilities.LowIF_DTMB;
	
	return ntStatus;	
}

/*****************************************************************************
* function:    AU8524_ChipClose
* Description: Close chip
* Input:       nAPHandle
*
* Return:      ChipUnit
*                  
*****************************************************************************/
TUNER_NTSTATUS	AU8524_ChipClose(UINT32 ChipUnit)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	m_AUChipHandle_8524[ChipUnit].APHandle  = 0;
	m_AUChipHandle_8524[ChipUnit].TunerType = TUNER_UNKNOWN;
	m_AUChipHandle_8524[ChipUnit].ChipMod	= TV_UNKNOWN;
	m_AUChipHandle_8524[ChipUnit].TSOutput	= TS_UNKNOWN;
	
	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_SetATVColorParamete
* Description: Set ATV parameter(sample:Contrast, hue, brightness)
* Input:		nAPHandle
*				nATVColorParamete
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVColorParamete(UINT32 ChipUnit,AU_ATVColorParamete nATVColorParamete)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_UNSUCCESSFUL;


	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_SetATVParamete
* Description: Set ATV parameter
* Input:		nAPHandle
*				nATVParamete
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVParamete(UINT32 ChipUnit,AU_ATVParamete nATVParamete)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_UNSUCCESSFUL;

	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_SetATVAudioFormat
* Description: Set ATV Audio format
* Input:		nAPHandle
*				nATVAudioFormat
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVAudioFormat(UINT32 ChipUnit,AU_ATVAudioFormat nATVAudioFormat)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_UNSUCCESSFUL;

	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_SetATVVolumeValue
* Description: Set ATV Audio Volume
* Input:		nAPHandle
*				nATVVolumeValue
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SetATVVolumeValue(UINT32 ChipUnit,AU_ATVVolumeValue nATVVolumeValue)	
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_UNSUCCESSFUL;


	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_GetChipLock
* Description: Get Chip lock status
* Input:		nAPHandle
*				* nLock
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_GetChipLock(UINT32 ChipUnit,UINT32 * nLock)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}


	if(nLock)
	{
		switch(m_AUChipHandle_8524[ChipUnit].ChipMod)
		{
		case TV_ATSC:
			{
				*nLock = AU8524_IsATSCSignalLocked(m_AUChipHandle_8524[ChipUnit].APHandle);
			}
			break;

		case TV_QAM256:
			{
				*nLock = AU8524_IsJ83B256SignalLocked(m_AUChipHandle_8524[ChipUnit].APHandle,m_AUChipHandle_8524[ChipUnit].TunerType);
			}
			break;

		case TV_QAM64:
			{
				*nLock = AU8524_IsJ83B64SignalLocked(m_AUChipHandle_8524[ChipUnit].APHandle,m_AUChipHandle_8524[ChipUnit].TunerType);
			}
			break;
		default:
			{
				*nLock = SIGNAL_NOT_LOCKED;
			}
		}
	}
	else
	{
		ntStatus = TUNER_STATUS_UNSUCCESSFUL;
	}


	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_GetChipSignalState
* Description: Get Signal Status
* Input:		nAPHandle
*				p_para
*				SignalState
* Return:      ntStatus
*                  
*****************************************************************************/
TUNER_NTSTATUS  AU8524_GetChipSignalState(UINT32 ChipUnit, AU_ChipParamete p_para, AU_SignalState * nSignalState)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(nSignalState)
	{
		{
			AU8524_GetSignalLevel(	m_AUChipHandle_8524[ChipUnit].APHandle,
									p_para.ChipMod,
									p_para.TunerType,
									&(nSignalState->SignalLevel));
		}


		switch(m_AUChipHandle_8524[ChipUnit].ChipMod)
		{
		case TV_ATSC:
			{
				AU8524_GetATSCSignalQuality(m_AUChipHandle_8524[ChipUnit].APHandle,&(nSignalState->SignalQuality));
			}
			break;

		case TV_QAM256:
			{
				AU8524_GetJ83BSignalQuality(m_AUChipHandle_8524[ChipUnit].APHandle,&(nSignalState->SignalQuality),TV_QAM256);
			}
			break;

		case TV_QAM64:
			{
				AU8524_GetJ83BSignalQuality(m_AUChipHandle_8524[ChipUnit].APHandle,&(nSignalState->SignalQuality),TV_QAM64);
			}
			break;

		default:
			{
				nSignalState->SignalQuality = 0;
			}
		}
	}
	else
	{
		ntStatus = TUNER_STATUS_UNSUCCESSFUL;
	}

	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_SetChipMod
* Description: set demodulator
* Input:		ChipUnit
*				p_para
*
* Return:      status:
*                  STATUS_OK: no error
*****************************************************************************/
TUNER_NTSTATUS	AU8524_SetChipMod(UINT32 ChipUnit, AU_ChipParamete p_para)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	int status	= SIGNAL_NOT_LOCKED;
	int size	= 0;
	int i		= 0;
	UINT8 Need_SWT = 1;	//Need SettingWithTuner
	static AU_TV_FORMAT_T Pre_ChipMode[CHIP_MAX_UNITS] = {TV_UNKNOWN};

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if (!memcmp(&m_AUChipHandle_8524[ChipUnit], &p_para, sizeof(AU_ChipParamete)))
		return ntStatus;
	else
		memcpy(&m_AUChipHandle_8524[ChipUnit], &p_para, sizeof(AU_ChipParamete));

	if(m_AUChipHandle_8524[ChipUnit].TunerType == TUNER_UNKNOWN)
	{
		return TUNER_STATUS_UNSUCCESSFUL;
	}

	AU8524_AfterSetTuner(m_AUChipHandle_8524[ChipUnit].APHandle);

	switch(p_para.ChipMod) 
	{
	case TV_ATSC:
			//if (Pre_ChipMode[ChipUnit] != m_AUChipHandle_8524[ChipUnit].ChipMod)
			status = AU8524_InitialATSC(m_AUChipHandle_8524[ChipUnit].APHandle);
			printk("AU8524 INITIAL ATSC status is %d\n",status);
			//status = AU8524_InitialATSC(m_AUChipHandle_8524[ChipUnit].APHandle);
                           status = AU8524_SetTSOutput(m_AUChipHandle_8524[ChipUnit].APHandle, p_para.TSOutput);
			printk("AU8524 set ts output status is %d\n",status);
			m_AUChipHandle_8524[ChipUnit].ChipMod = TV_ATSC;
			break;
			
		case TV_QAM256:
			if (Pre_ChipMode[ChipUnit] != m_AUChipHandle_8524[ChipUnit].ChipMod)
				status = AU8524_InitialJ83B256(m_AUChipHandle_8524[ChipUnit].APHandle);

			status = AU8524_SetTSOutput(m_AUChipHandle_8524[ChipUnit].APHandle, p_para.TSOutput);

			m_AUChipHandle_8524[ChipUnit].ChipMod = TV_QAM256;
			break;
			
		case TV_QAM64:
			if (Pre_ChipMode[ChipUnit] != m_AUChipHandle_8524[ChipUnit].ChipMod)
				status = AU8524_InitialJ83B64(m_AUChipHandle_8524[ChipUnit].APHandle);

			status = AU8524_SetTSOutput(m_AUChipHandle_8524[ChipUnit].APHandle, p_para.TSOutput);

			m_AUChipHandle_8524[ChipUnit].ChipMod = TV_QAM64;
			break;	
			
		case TV_QAM:
			status = AU8524_InitialJ83B256(m_AUChipHandle_8524[ChipUnit].APHandle);

			m_AUChipHandle_8524[ChipUnit].ChipMod = TV_QAM256;

			AU8524_SettingWithTuner(ChipUnit);

			status = AU8524_IsJ83B256SignalLocked(	m_AUChipHandle_8524[ChipUnit].APHandle,
													m_AUChipHandle_8524[ChipUnit].TunerType);

			if (status == SIGNAL_NOT_LOCKED)
			{

				if(m_AUChipHandle_8524[ChipUnit].TunerType == TUNER_NXP_TDA18271C1 || m_AUChipHandle_8524[ChipUnit].TunerType == TUNER_NXP_TDA18271C2)
				{
					status = AU8524_InitialJ83B256(m_AUChipHandle_8524[ChipUnit].APHandle);

					m_AUChipHandle_8524[ChipUnit].ChipMod = TV_QAM256;

					AU8524_SettingWithTuner(ChipUnit);

					status = AU8524_IsJ83B256SignalLocked(	m_AUChipHandle_8524[ChipUnit].APHandle,
															m_AUChipHandle_8524[ChipUnit].TunerType);
				}

				if(status == SIGNAL_NOT_LOCKED)
				{
					#include "../AU852X_ATSC_QAM/AU852X_ATSCQAM_RegisterDef.h"

					status = AU8524_WriteReg(m_AUChipHandle_8524[ChipUnit].APHandle,0x0a4,0xc0);

					size = sizeof(AU852X_MODJ83B64) / sizeof(UINT16);
					
					for (i=0; i<size/2; i++)
					{
						if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(m_AUChipHandle_8524[ChipUnit].APHandle,AU852X_MODJ83B64[2*i],(UINT8)AU852X_MODJ83B64[2*i+1]);
					}

					status = AU8524_WriteReg(m_AUChipHandle_8524[ChipUnit].APHandle,0x0a4,0xca);

					m_AUChipHandle_8524[ChipUnit].ChipMod = TV_QAM64;

					AU8524_SettingWithTuner(ChipUnit);
					
					status = AU8524_IsJ83B64SignalLocked(	m_AUChipHandle_8524[ChipUnit].APHandle,
															m_AUChipHandle_8524[ChipUnit].TunerType);	
				}
			}

			status = AU8524_SetTSOutput(m_AUChipHandle_8524[ChipUnit].APHandle, p_para.TSOutput);
			break;

		default:
			status = STATUS_ERROR;
			break;
	}

	if (Need_SWT)
		AU8524_SettingWithTuner(ChipUnit);

	m_AUChipHandle_8524[ChipUnit].TSOutput= p_para.TSOutput;
	Pre_ChipMode[ChipUnit] = m_AUChipHandle_8524[ChipUnit].ChipMod;

	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_OnTime
* Description: Timer
* Input:		ChipUnit
*
* Return:      status:
*
*****************************************************************************/
TUNER_NTSTATUS  AU8524_OnTime(UINT32 ChipUnit)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	AU8524_Timer(	m_AUChipHandle_8524[ChipUnit].APHandle, m_AUChipHandle_8524[ChipUnit]);

	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_GetModOffset
* Description: Get Timing or Frequency offset
* Input:		ChipUnit
*				nMod
*				*Offset
* Return:      status:
*
*****************************************************************************/
TUNER_NTSTATUS  AU8524_GetModOffset(UINT32 ChipUnit,AU_MODOFFSET_T nMod,double *Offset)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}
	printk("IN AU8524_GetModOffset the nMod is %d offset value is %d\n",nMod,*Offset);
#ifdef IIC_Demo
	switch(nMod)
	{
	case ATSC_FREQ_OFFSET:
		{
			AU8524_GetATSC_Freq_Offset(m_AUChipHandle_8524[ChipUnit].APHandle,Offset);
		}
		break;
	case ATSC_TIMING_OFFSET:
		{
			AU8524_GetATSC_Timing_Offset(m_AUChipHandle_8524[ChipUnit].APHandle,Offset);
		}
		break;
	case J83B_FREQ_OFFSET:
		{
			AU8524_GetJ83B_Freq_Offset(m_AUChipHandle_8524[ChipUnit].APHandle,Offset);
		}
		break;
	case J83B_TIMING_OFFSET:
		{
			AU8524_GetJ83B_Timing_Offset(m_AUChipHandle_8524[ChipUnit].APHandle,m_AUChipHandle_8524[ChipUnit].ChipMod,Offset);
		}
		break;
	}
#endif
	return ntStatus;
}

/*****************************************************************************
* function:    AU8524_SettingWithTuner
* Description: Set chip register setting for tuner
* Input:		ChipUnit
*
*
* Return:      status:
*
*****************************************************************************/
TUNER_NTSTATUS  AU8524_SettingWithTuner(UINT32 ChipUnit)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (ChipUnit > CHIP_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	if(m_AUChipHandle_8524[ChipUnit].TunerType == TUNER_UNKNOWN)
	{
		return TUNER_STATUS_UNSUCCESSFUL;
	}

	switch(m_AUChipHandle_8524[ChipUnit].ChipMod) 
	{
		case TV_ATSC:
			{
				AU8524_ATSCSettingWithTuner(m_AUChipHandle_8524[ChipUnit].APHandle, m_AUChipHandle_8524[ChipUnit]);
			}
			break;

		case TV_QAM256:
		case TV_QAM64:
		case TV_QAM:
			{
				AU8524_J83BSettingWithTuner(m_AUChipHandle_8524[ChipUnit].APHandle, m_AUChipHandle_8524[ChipUnit]);
			}
			break;
	}



	return ntStatus;	
}

/*****************************************************************************
* function:    AU8524_GetDemodAbility
* Description: Get chip support mode
* Input:		ChipUnit
*
*
* Return:
*****************************************************************************/
UINT32 AU8524_GetDemodAbility(UINT32 ChipUnit, AU_CHIPTYPE_T nChipType) 
{
	return Chip_ATSC | Chip_QAM ;
}

/*****************************************************************************
* function:    AU8524_GetAudioAbility
* Description: Get Audio support mode
* Input:		ChipUnit
*
*
* Return:
*****************************************************************************/	
UINT32 AU8524_GetAudioAbility(UINT32 ChipUnit) 
{
	return 0;
}

/* ATSC module */

/*****************************************************************************
* function:    AU8524_InitialATSC
* Description: set chip in ATSC mode
* Input:       void
* Return:      status:
*                  STATUS_OK: no error
*****************************************************************************/
UINT16 AU8524_InitialATSC(void* nAPHandle) 
{
	#include "../AU852X_ATSC_QAM/AU852X_ATSCQAM_RegisterDef.h"

	UINT16 status = STATUS_OK;
	UINT16 i = 0;
	UINT16 AU852X_MODATSC_Size = sizeof(AU852X_MODATSC) / sizeof(UINT16);
	
	if(STATUS_NO_ERROR(status)) status = AuvitekChip_ResetChip(nAPHandle);
	msleep(50);
	for (i=0; i<AU852X_MODATSC_Size/2; i++)
	{
		if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU852X_MODATSC[2*i],(UINT8)AU852X_MODATSC[2*i+1]);
	}
		
	status = AU8524_WriteReg(nAPHandle,0x0a4, 0xe8);

	return status;		
}

/*****************************************************************************
* function:    AU8524_ATSCSettingWithTuner()
* Description: Some settings(for example, AGC) need to modify with tuner type
* Input:       void
* Return:     status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_ATSCSettingWithTuner(void* nAPHandle, AU_ChipParamete p_para)
{
	UINT16 status = STATUS_OK;
	UINT8  i   = 0;
	UINT16 size = 0;

	switch(p_para.TunerType) 
	{
		case TUNER_Philips_TD1336:

			size =  sizeof(AU8524_TUNPHILIPSTD1336_MODATSC) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNPHILIPSTD1336_MODATSC[2*i],(UINT8)AU8524_TUNPHILIPSTD1336_MODATSC[2*i+1]);
			}
			
			break;
			
		case TUNER_XG_DVT6AW41F2:

			size = sizeof(AU8524_TUNXGDVT6AW41F2_MODATSC) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNXGDVT6AW41F2_MODATSC[2*i],(UINT8)AU8524_TUNXGDVT6AW41F2_MODATSC[2*i+1]);
			}

			break;

		case TUNER_QJ_DTZ1W40x:

			size = sizeof(AU8524_TUNQJDTZ1W40x_MODATSC) / sizeof(UINT16);
			
			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ1W40x_MODATSC[2*i],(UINT8)AU8524_TUNQJDTZ1W40x_MODATSC[2*i+1]);
			}

			if (p_para.FreqHz <= 153*_1M)
			{
				size = sizeof(AU8524_TUNQJDTZ1W40x_MODATSC_FRQVL) / sizeof(UINT16);

				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ1W40x_MODATSC_FRQVL[2*i],(UINT8)AU8524_TUNQJDTZ1W40x_MODATSC_FRQVL[2*i+1]);
				}
			}
			else
			{
				size = sizeof(AU8524_TUNQJDTZ1W40x_MODATSC_FRQVHU) / sizeof(UINT16);

				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ1W40x_MODATSC_FRQVHU[2*i],(UINT8)AU8524_TUNQJDTZ1W40x_MODATSC_FRQVHU[2*i+1]);
				}
			}
			
			break;

		case TUNER_NXP_TDA18271C1:
			{
				size = sizeof(AU8524_TUNNXPTDA18271C1_MODATSC) / sizeof(UINT16);
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNNXPTDA18271C1_MODATSC[2*i],(UINT8)AU8524_TUNNXPTDA18271C1_MODATSC[2*i+1]);
				}

				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_ATSC/10000);
			}

			break;
	
		case TUNER_TH_DTT768xx:
		case TUNER_TH_DTT7683x:	
			size = sizeof(AU8524_TUNTHDTT768xx_MODATSC) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNTHDTT768xx_MODATSC[2*i],(UINT8)AU8524_TUNTHDTT768xx_MODATSC[2*i+1]);
			}
			
			break;

		case TUNER_SANYO_UBD00Ax:
			
			size = sizeof(AU8524_TUNSANYOUBD00Ax_MODATSC) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle, AU8524_TUNSANYOUBD00Ax_MODATSC[2*i],(UINT8)AU8524_TUNSANYOUBD00Ax_MODATSC[2*i+1]);
			}
			
			break;
			
		case TUNER_XC_5000:
			{
				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_ATSC/10000);
			}
			break;
			
		case TUNER_NXP_TD1636ALF:
		case TUNER_NXP_TD1636FN:	
			size = sizeof(AU8524_TUNNXPTD1636ALF_MODATSC) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNNXPTD1636ALF_MODATSC[2*i],(UINT8)AU8524_TUNNXPTD1636ALF_MODATSC[2*i+1]);
			}
			
			break;

		case TUNER_MXL_5003s_5:
			{
				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_ATSC/10000);
			}
			
			break;

		case TUNER_QJ_DTZ0R401:
		case TUNER_QJ_DTZ0W400G:
			
			size = sizeof(AU8524_TUNQJDTZ0R401_MODATSC) / sizeof(UINT16);
			
			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ0R401_MODATSC[2*i],(UINT8)AU8524_TUNQJDTZ0R401_MODATSC[2*i+1]);
			}
			if (p_para.FreqHz <= 153*_1M)
			{
				size = sizeof(AU8524_TUNQJDTZ0R401_MODATSC_FRQVL) / sizeof(UINT16);
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ0R401_MODATSC_FRQVL[2*i],(UINT8)AU8524_TUNQJDTZ0R401_MODATSC_FRQVL[2*i+1]);
				}
			}
			if (p_para.FreqHz >= 432*_1M)
			{
				size = sizeof(AU8524_TUNQJDTZ0R401_MODATSC_FRQVHU) / sizeof(UINT16);
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ0R401_MODATSC_FRQVHU[2*i],(UINT8)AU8524_TUNQJDTZ0R401_MODATSC_FRQVHU[2*i+1]);
				}
			}
			break;

		case TUNER_NXP_TDA18271C2:
			{				
				size = sizeof(AU8524_TUNNXPTDA18271C2_MODATSC) / sizeof(UINT16);
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNNXPTDA18271C2_MODATSC[2*i],(UINT8)AU8524_TUNNXPTDA18271C2_MODATSC[2*i+1]);
				}

				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_ATSC/10000);
			}
			break;

		case TUNER_MXL_5007:
			status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_ATSC/10000);
			break;
			
		default:
			
			return STATUS_ERROR;
	}

	return status;
}

/*****************************************************************************
* function:    AU8524_LowIF
* Description: Set AU85xx input IF according to the IF output from the tuner
* Input:       fIF (MHz)
* Return:     status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_LowIF(void* nAPHandle, AU_TUNERTYPE_T nTunerType, AU_TV_FORMAT_T nTVFormat, UINT32 fIF)
{
	UINT16 status = STATUS_OK;
	UINT32 iData  = 0;
	UINT8  data   = 0;
	UINT8  bTemp  = 0;
         UINT8 *ke_tt;
	
    //iData= (UINT32)(-262144*(double)fIF/100/54);	
    iData = (UINT32)(-262144/(INT32)(5400/fIF));
    //iData = (UINT32)((-262144)/9);

	if (nTunerType == TUNER_NXP_TDA18271C1 ||nTunerType == TUNER_NXP_TDA18271C2)
	{
	     iData=~iData;
 	}

	if (nTunerType == TUNER_MXL_5007 \
		&& (nTVFormat == TV_QAM || nTVFormat == TV_QAM64 || nTVFormat == TV_QAM256))
	{
		iData=~iData;
  	}

	/* adjust input the para of the low if
	*/
	bTemp=iData&0xff;
	if(STATUS_NO_ERROR(status)) status =AU8524_WriteReg(nAPHandle,0x0b7,bTemp);
	bTemp =(iData>>8)&0xff;
	if(STATUS_NO_ERROR(status)) status =AU8524_WriteReg(nAPHandle,0x0b6,bTemp);
	bTemp =(iData>>16)&0xff;
	if(STATUS_NO_ERROR(status)) status =AU8524_WriteReg(nAPHandle,0x0b5,bTemp);
        
        


	/*enable low if
	*/
	if(STATUS_NO_ERROR(status)) status =AU8524_ReadReg(nAPHandle,0x080,&data);
	data=data|0x80;
	if(STATUS_NO_ERROR(status)) status =AU8524_WriteReg(nAPHandle,0x080,data);

	return status;

}
  
/*****************************************************************************
* function:    AU8524_IsATSCSignalLocked
* Description: check ATSC signal lock status
* Input:      void
* Return:     1 SIGNAL_IS_LOCKED 
*             2 SIGNAL_NOT_LOCKED
*		      3 status:
*			   STATUS_OK: no error
*                    user-defined
*****************************************************************************/
UINT16 AU8524_IsATSCSignalLocked(void* nAPHandle)
{
	UINT8 bCur=0;
	UINT16 status = STATUS_OK;
	UINT16 nLockNum = 0;
	UINT32 i = 0;
	
	for(i = 0; i < 10; i++)
	{
		status = AU8524_ReadReg(nAPHandle,0x88,&bCur);
		if(STATUS_NO_ERROR(status))
		{
			if (bCur ==0x03)
			{
				nLockNum++;
			}
			if (nLockNum >= 3) 
			{
				return SIGNAL_IS_LOCKED;
			}			
		}
		AuvitekChip_TimeDelay(100);
	}

	return SIGNAL_NOT_LOCKED;	
}


UINT16 AU8524_DVT6AW41F2_level[11]		= { 0xF0, 0xCC, 0xA2, 0x98, 0x94, 0x8F, 0x8B, 0x87, 0x83, 0x7F, 0x7B};
UINT16 AU8524_MT2131_level[10]			= { 245, 230, 215, 200, 185, 170, 155, 140, 128,0};
UINT16 AU8524_DTZ1W40x_NTSC_level[10]	= {0x170, 0x160, 0x150, 0x140, 0x130, 0x120, 0x110, 0x100, 0xF0,0x0};
/*****************************************************************************
* function:    AU8524_GetSignalLevel
* Description: get signal level with all tuner
*				(include NTSC, ATSC, QAM)
* Input:       p_para
*					the pointer of the parameters to set the AU85xx chip and tuner
*
* Output:        pSignalLevel: the pointer to  signal level
				(the range is 0 to 10, the best level is 10, the worst is 0)
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_GetSignalLevel(void* nAPHandle,AU_TV_FORMAT_T TVMod,INT32 nTunerType,UINT16 *pSignalLevel)
{
	UINT8  ndata0ec = 0x0;
	UINT8  ndata0ed = 0x0;
	UINT8  ndata0eb = 0x0;
	UINT8  nCur		= 0x0;
	UINT8  nCur2	= 0x0;
	UINT32 nCompareVal = 0;
	UINT16* pArray;  
	UINT16 status   = STATUS_OK;
	UINT16 i;
	

/*	if (p_para_status->b_rst_atsc == 1)
	{
		AuvitekChip_TimeDelay(100);
		p_para_status->b_rst_atsc = 0;
	}
*/	
	switch(TVMod)
	{
		case TV_NTSC:
			if(STATUS_NO_ERROR(status))	status = AU8524_ReadReg(nAPHandle,0x00,&nCur2);
			if( nCur2 != 0xa2 && nCur2 != 0xa0 )
			{
			   *pSignalLevel = 0;
			   return status;
			}	
			break;
		case TV_ATSC:
			if(STATUS_NO_ERROR(status))	status = AU8524_ReadReg(nAPHandle,0x88,&nCur2);
			if( nCur2 != 0x3 )
			{
			   *pSignalLevel = 0;
			   return status;
			}	
			break;
		case TV_QAM:
		case TV_QAM64:
		case TV_QAM256:
			if(STATUS_NO_ERROR(status)) status = AU8524_ReadReg(nAPHandle,0x541,&nCur2);			
			if (((nCur2&0xc0) != 0x80) || ((nCur2&0x30) != 0x20) )
			{
			   *pSignalLevel = 0;
			   return status;
			}			
			break;
		default:
			   *pSignalLevel = 0;
			   return status;
	}

	if(STATUS_NO_ERROR(status))	status = AU8524_ReadReg(nAPHandle,0x0EB,&ndata0eb); 
	if(STATUS_NO_ERROR(status))	status = AU8524_ReadReg(nAPHandle,0x0EC,&ndata0ec); 
	if(STATUS_NO_ERROR(status))	status = AU8524_ReadReg(nAPHandle,0x0ED,&ndata0ed);
	
	switch(nTunerType) 
	{
		case TUNER_MT_2131:
			nCompareVal = ndata0ec + 13*ndata0ed;
			pArray = AU8524_MT2131_level;
			break;

		case TUNER_QJ_DTZ1W40x:
		case TUNER_QJ_DTZ0R401:
		case TUNER_QJ_DTZ0W400G:
			if (TVMod == TV_NTSC)
			{
				nCompareVal = ndata0eb + ndata0ec + 13*ndata0ed;
				pArray = AU8524_DTZ1W40x_NTSC_level;
			}
			else
			{
				nCompareVal = ndata0ec + 13*ndata0ed;
				pArray = AU8524_MT2131_level;
			}
			break;

		case TUNER_XG_DVT6AW41F2:
			nCompareVal = ndata0ec + 13*ndata0ed;
			pArray = AU8524_MT2131_level;
			break;
			
		default:
			nCompareVal = ndata0ec + 13*ndata0ed;
			pArray = AU8524_MT2131_level;
			break;
	}

	for (i = 0; i < 10; i++)
	{
		if (nCompareVal >= pArray[i] )
		{
		   *pSignalLevel = i+1;

			return status;
		}
	}

    *pSignalLevel = 10;

	return status;  

}

const UINT32 AU8524_mse2snr_atsc[256] = {
  270, 270, 250, 240, 230, 230, 220, 220, 210, 210, 200, 200, 200, 195, 190, 190,
  185, 185, 180, 180, 175, 175, 170, 170, 170, 165, 165, 165, 160, 160, 160, 160,
  158, 156, 154, 152, 152, 150, 148, 148, 146, 144, 142, 142, 140, 135, 135, 135,
  135, 130, 130, 125, 125, 125, 120, 120, 120, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115};
  
/*****************************************************************************
* function:    AU8524_GetATSCSignalQuality
* Description: get signal quality of ATSC 
* Output:      fSignalQuality: the pointer to signal quality
			 pSignalQuality: the pointer to signal Quality,return snr in dB	
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_GetATSCSignalQuality(void* nAPHandle,UINT16 *fSignalQuality)
{
	UINT16 status = STATUS_OK;
	UINT8 nCur=0x0;

	/* if signal is not locked, return 0
	*/
	if(STATUS_NO_ERROR(status))	status = AU8524_ReadReg(nAPHandle,0x88,&nCur);
	if( nCur != 0x3 )
	{
	   *fSignalQuality = 0;
	   return status;
	}	

	/*read register 0x541 to check lock
	*/
	if(STATUS_NO_ERROR(status)) status = AU8524_ReadReg(nAPHandle,0x311,&nCur);
	*fSignalQuality = AU8524_mse2snr_atsc[nCur];

	return status;
	 
}

/*****************************************************************************
* function:    AU8524_ShowError
* Description: show error information, you can call this function when you display
*				the information, the best is about every 50 ms call one time to 
*				check whether error
*				
* Input:	   nMode
*				0:ATSC_MODE
*				1:J83B64_MODE
*				2:J83B256_MODE 
* Return:       1, error
*			    0, no error  
*****************************************************************************/
UINT16 AU8524_ShowError(void* nAPHandle,UINT8 nMode) 
{	
	UINT8  bCur=0;
	UINT8  bCur1=0;
	UINT16 bErr=0;	
	UINT16 status=STATUS_OK;
	
	if (nMode ==TV_ATSC)		
	{		
		status=AU8524_ReadReg(nAPHandle,0x87,&bCur);
		
		if( STATUS_NO_ERROR(status)) 	status=AU8524_ReadReg(nAPHandle,0x88,&bCur1);  
		
		if (bCur1 !=3)		bErr = 1;			
		
		else			
		{			
			if (bCur!=0)	bErr = 1;											
		}
		
	}

	else if ((nMode ==TV_QAM256) || (nMode == TV_QAM64))

	{				
		if( STATUS_NO_ERROR(status))	status=AU8524_ReadReg(nAPHandle,0x541,&bCur1);  
		
		if( STATUS_NO_ERROR(status))	status=AU8524_ReadReg(nAPHandle,0x543,&bCur);         
		
		
		if (!((bCur1&0xc0) == 0x80) && ((bCur1&0x30) == 0x20) )			
							bErr = 1;											
		
		else			
		{			
			if (bCur !=0)	bErr = 1;																		
		}
		
	}

	return bErr;
} 
  
/*****************************************************************************
*function:	   AU8524_ReadReg
*Description:  Read values from the register .
* Input:      1 nAddr: the register address to be read
*             2 pData: pointer to the Data to receive the value from the register
* Return:      status:
*              STATUS_OK: no error
*              user-defined
*****************************************************************************/
UINT16 AU8524_ReadReg(void* nAPHandle,UINT16 nAddr, UINT8 *pData)
{
	UINT16 status = STATUS_OK;
	UINT8  bCur   = 0;
	
	status = AuvitekChip_ReadReg(nAPHandle,nAddr,&bCur);

	if (STATUS_NO_ERROR(status)) 
	{
			*pData = bCur;
			return STATUS_OK;
	}
	
	return STATUS_ERROR;
	
}

/*****************************************************************************
*function:	   AU8524_WriteReg
*Description:  Write values to device .
* Input:      1 nAddr: the register address to be written
*             2 pData: the data to be written to the  register
* Return:      status:
*              STATUS_OK: no error
*              user-defined
*****************************************************************************/
UINT16 AU8524_WriteReg(void* nAPHandle,UINT16 nAddr, UINT8 pData)
{
	UINT16 status = STATUS_OK;

	status = AuvitekChip_WriteReg(nAPHandle,nAddr, pData);
	
	if (STATUS_NO_ERROR(status)) 
	{
		return STATUS_OK;
	}

	return STATUS_ERROR;

}

/*****************************************************************************
* function:    AU8524_AfterSetTuner(void)
* Description: set some registers after set tuner(when tuner frequency changes)
* Input:       void
* Return:     status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_AfterSetTuner(void* nAPHandle)
{
	UINT16 status = STATUS_OK;
	UINT8  bCur   = 0;

	status=AU8524_ReadReg(nAPHandle,0xa4,&bCur);
	printk("IN AU8524 AftersetTuner the readreg status is %d\n",status);
	printk("the register 0xa4 value is %d\n",bCur);
	if( STATUS_NO_ERROR(status))	status=AU8524_WriteReg(nAPHandle,0xa4,0x00);
	if( STATUS_NO_ERROR(status))	status=AU8524_WriteReg(nAPHandle,0xa4,bCur);

	return status;
}

/*****************************************************************************
* function:    AU8524_Timer(void)
* Description: call this function after a time(25ms is recommended) 
*			   when play TV.
*		If 25ms-timer is not supported by your system: 
*	       The micro define "TIMER_231_DLY" in Auvitek_Typedef.h can be 
*	       adjusted to a minimal timer your system supports .
*	       For example, if the minimal timer on your system is 50ms, 
*	       "#define TIMER_231_DLY 25" should be amended to
*	       "#define TIMER_231_DLY 50" in Auvitek_Typedef.h
*			   This function can be a callback function of a timer.
* Input:       p_para
*					the pointer of the parameters to set the AU85xx chip and tuner
* Return:     status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_Timer(void* nAPHandle, AU_ChipParamete p_para)
{
	UINT8	bCur = 0;
	UINT16	status = STATUS_OK;
	static UINT32 nCount = 0;
	static UINT32 iCount = 0;
	static UINT32 Count1_0x21a = 0;
	static UINT32 Count2_0x21a = 0;
	static UINT32 count1 = 0; 
	static UINT32 count2 = 0; 
	UINT8   value0x087 = 0;
	UINT8   value0x231 = 0;
	UINT8	value0xf0 = 0;
	UINT8	value0xf1 = 0;
	UINT8	value0x605 = 0;
	UINT8	value0x0db = 0;
	UINT8	value0x0b0 = 0;

	if(count1) count1--;
	if(count2) count2--;

	iCount++;
	
	if (p_para.ChipMod == TV_ATSC)
	{
		if (iCount >= COUNT3_TH) 
		{
			iCount = 0;
			status = AU8524_ReadReg(nAPHandle,0x88,&bCur);

			if (STATUS_NO_ERROR(status)) 
			{	
			
				if ( bCur != 0x03 ) 
				{
					nCount++;
				}
				else
					nCount = 0;

				if(nCount >= 2)
				{
					if (STATUS_NO_ERROR(status))	status = AU8524_WriteReg(nAPHandle,0xa4,0x00);
/*
					if (nTunerType == TUNER_MT_2131 || nTunerType == TUNER_MT_2130)
					{
						AU_SelectTuner(p_para, p_para_status);
					}
*/
					if (STATUS_NO_ERROR(status))	status = AU8524_WriteReg(nAPHandle,0xa4,0xe8);
					
					nCount = 0;
				}
			}		
		}

		AU8524_ReadReg(nAPHandle,0x087, &value0x087);

		if (value0x087 != 0)
		{
			/* TS errors detected  */
			if((!count1) && count2)
			{
				count1 = COUNT1_TH;

				AU8524_ReadReg(nAPHandle,0x231, &value0x231);

				switch(value0x231) 
				{
					case 0x13:
						AU8524_WriteReg(nAPHandle,0x231, 0x73);
						break;
					case 0x73:
						AU8524_WriteReg(nAPHandle,0x231, 0x33);
						break;
					case 0x33:
						AU8524_WriteReg(nAPHandle,0x231, 0x13);
						break;
					default:
						AU8524_WriteReg(nAPHandle,0x231, 0x13);
				}				
			}
						  
			count2 = COUNT2_TH; 				
		}

		if (TUNER_NXP_TDA18271C2 == p_para.TunerType && 1 == p_para.Auto_AGC)
		{
			AU8524_ReadReg(nAPHandle,0x0db,  &value0x0db);
			AU8524_ReadReg(nAPHandle,0x087,  &value0x087);
			AU8524_ReadReg(nAPHandle,0x0b0,  &value0x0b0);

			if(value0x0db<3 && Count1_0x21a<COUNT4_TH)
				Count1_0x21a++;
			else if(3 == value0x0db)
				Count1_0x21a=0;
			
			if((Count1_0x21a==COUNT4_TH) && (value0x0b0!=0) && (Count2_0x21a>=COUNT5_TH))
			{
				AU8524_WriteReg(nAPHandle,0xb0,0x0);
				Count2_0x21a = 0;
			}
			else if((value0x0db==3) && (value0x087==0xff) && (value0x0b0==0) && (Count2_0x21a>=COUNT5_TH))
			{
				AU8524_WriteReg(nAPHandle,0xb0,0x7b);
				Count2_0x21a = 0;
			}
			else if((value0x0db==3) && (value0x087==0xff) && (value0x0b0==0x7b) && (Count2_0x21a>=COUNT5_TH))
			{
				AU8524_WriteReg(nAPHandle,0xb0,0x8b);
				Count2_0x21a = 0;
			}
			else if((value0x0db==3) && (value0x087==0xff) && (value0x0b0==0x8b) && (Count2_0x21a>=COUNT5_TH))
			{
				AU8524_WriteReg(nAPHandle,0xb0,0x9b);
				Count2_0x21a = 0;
			}
			else if((value0x0db==3) && (value0x087==0xff) && (value0x0b0==0x9b) && (Count2_0x21a>=COUNT5_TH))
			{
				AU8524_WriteReg(nAPHandle,0xb0,0xab);
				Count2_0x21a = 0;
			}
			else if((value0x0db==3) && (value0x087==0xff) && (value0x0b0==0xab) && (Count2_0x21a>=COUNT5_TH))
			{
				AU8524_WriteReg(nAPHandle,0xb0,0xbb);
				Count2_0x21a = 0;
			}
			else if((value0x0db==3) && (value0x087==0xff) && (value0x0b0==0xbb) && (Count2_0x21a>=COUNT5_TH))
			{
				AU8524_WriteReg(nAPHandle,0xb0,0x0);
				Count2_0x21a = 0;
			}
			
			if(Count2_0x21a<COUNT5_TH)
			{
				Count2_0x21a ++;
			}
		}
	}

	return status;
}


/* J83B module */
/*****************************************************************************
* function:    AU8524_InitialJ83B64
* Description: set chip in J83B64 mode
* Input:       void
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_InitialJ83B64(void* nAPHandle) 
{
	#include "../AU852X_ATSC_QAM/AU852X_ATSCQAM_RegisterDef.h"

	UINT16 status = STATUS_OK;
	UINT16 size = 0;
	UINT16 i = 0;

	if(STATUS_NO_ERROR(status)) status = AuvitekChip_ResetChip(nAPHandle);
	
	size = sizeof(AU852X_MODJ83B) / sizeof(UINT16);

	for (i=0; i<size/2; i++)
	{
		if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU852X_MODJ83B[2*i],(UINT8)AU852X_MODJ83B[2*i+1]);
	}

	size = sizeof(AU852X_MODJ83B64) / sizeof(UINT16);

	for (i=0; i<size/2; i++)
	{
		if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU852X_MODJ83B64[2*i],(UINT8)AU852X_MODJ83B64[2*i+1]);
	}

	status = AU8524_WriteReg(nAPHandle,0x0a4, 0xca);

	return status;	
}
		

/*****************************************************************************
* function:    AU8524_InitialJ83B256
* Description: set chip in J83B256 mode
* Input:       void
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_InitialJ83B256(void* nAPHandle) 
{
	#include "../AU852X_ATSC_QAM/AU852X_ATSCQAM_RegisterDef.h"

	UINT16 status =STATUS_OK;
	UINT16 size = 0;
	UINT16 i = 0;

	if(STATUS_NO_ERROR(status)) status = AuvitekChip_ResetChip(nAPHandle);

	size = sizeof(AU852X_MODJ83B) / sizeof(UINT16);

	for (i=0; i<size/2; i++)
	{
		if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU852X_MODJ83B[2*i],(UINT8)AU852X_MODJ83B[2*i+1]);
	}

	size = sizeof(AU852X_MODJ83B256) / sizeof(UINT16);

	for (i=0; i<size/2; i++)
	{
		if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU852X_MODJ83B256[2*i],(UINT8)AU852X_MODJ83B256[2*i+1]);
	}

	status = AU8524_WriteReg(nAPHandle,0x0a4, 0xca);

	return status;

}

/*****************************************************************************
* function:    AU8524_J83BSettingWithTuner()
* Description: Some settings(for example, AGC) need to modify with tuner type
* Input:       void
* Return:     status:
*			   STATUS_OK: no error
*                     user-defined
*****************************************************************************/
UINT16 AU8524_J83BSettingWithTuner(void* nAPHandle, AU_ChipParamete p_para)
{
	UINT16 status = STATUS_OK;
	UINT16 size = 0;
	UINT8  i   = 0;

	switch(p_para.TunerType) 
	{

		case TUNER_XG_DVT6AW41F2:

			size = sizeof(AU8524_TUNXGDVT6AW41F2_MODJ83B) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNXGDVT6AW41F2_MODJ83B[2*i],(UINT8)AU8524_TUNXGDVT6AW41F2_MODJ83B[2*i+1]);
			}

			break;

		case TUNER_QJ_DTZ1W40x:

			size = sizeof(AU8524_TUNQJDTZ1W40x_MODJ83B) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ1W40x_MODJ83B[2*i],(UINT8)AU8524_TUNQJDTZ1W40x_MODJ83B[2*i+1]);
			}

			if (p_para.FreqHz <= 153*_1M)
			{
				size = sizeof(AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVL) / sizeof(UINT16);

				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVL[2*i],(UINT8)AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVL[2*i+1]);
				}
			}
			else
			{
				size = sizeof(AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVHU) / sizeof(UINT16);

				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVHU[2*i],(UINT8)AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVHU[2*i+1]);
				}
			}
			
			break;

		case TUNER_NXP_TDA18271C1:
			{

				size = sizeof(AU8524_TUNNXPTDA18271C1_MODJ83B) / sizeof(UINT16);

				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNNXPTDA18271C1_MODJ83B[2*i],(UINT8)AU8524_TUNNXPTDA18271C1_MODJ83B[2*i+1]);
				}
				
				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_QAM/10000);

			}

			break;
			
		case TUNER_XC_5000:
			{
				
				size = sizeof(AU8524_TUNXC5000_MODJ83B) / sizeof(UINT16);

				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNXC5000_MODJ83B[2*i],(UINT8)AU8524_TUNXC5000_MODJ83B[2*i+1]);
				}

				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_QAM/10000);
			}
			
			break;

		case TUNER_NXP_TD1636ALF:
		case TUNER_NXP_TD1636FN:	
			size = sizeof(AU8524_TUNNXPTD1636ALF_MODJ83B) / sizeof(UINT16);

			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNNXPTD1636ALF_MODJ83B[2*i],(UINT8)AU8524_TUNNXPTD1636ALF_MODJ83B[2*i+1]);
			}
			
			break;

		case TUNER_MXL_5003s_5:
			{
				
				size = sizeof(AU8524_TUNMXL50035_MODJ83B) / sizeof(UINT16);
				
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNMXL50035_MODJ83B[2*i],(UINT8)AU8524_TUNMXL50035_MODJ83B[2*i+1]);
				}

				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_QAM/10000);

			}
			break;

		case TUNER_QJ_DTZ0R401:
		case TUNER_QJ_DTZ0W400G:
			
			size = sizeof(AU8524_TUNQJDTZ0R401_MODJ83B) / sizeof(UINT16);
			
			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ0R401_MODJ83B[2*i],(UINT8)AU8524_TUNQJDTZ0R401_MODJ83B[2*i+1]);
			}
			if (p_para.FreqHz <= 153*_1M)
			{
				size = sizeof(AU8524_TUNQJDTZ0R401_MODJ83B_FRQVL) / sizeof(UINT16);
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ0R401_MODJ83B_FRQVL[2*i],(UINT8)AU8524_TUNQJDTZ0R401_MODJ83B_FRQVL[2*i+1]);
				}
			}
			if (p_para.FreqHz >= 432*_1M)
			{
				size = sizeof(AU8524_TUNQJDTZ0R401_MODJ83B_FRQVHU) / sizeof(UINT16);
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNQJDTZ0R401_MODJ83B_FRQVHU[2*i],(UINT8)AU8524_TUNQJDTZ0R401_MODJ83B_FRQVHU[2*i+1]);
				}
			}
			break;

		case TUNER_NXP_TDA18271C2:
			{
				
				size = sizeof(AU8524_TUNNXPTDA18271C2_MODJ83B) / sizeof(UINT16);
				
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle,AU8524_TUNNXPTDA18271C2_MODJ83B[2*i],(UINT8)AU8524_TUNNXPTDA18271C2_MODJ83B[2*i+1]);
				}

				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_QAM/10000);
			}
			
			break;

		case TUNER_SANYO_UBD00Ax:
			size = sizeof(AU8524_TUNSANYOUBD00Ax_MODJ83B) / sizeof(UINT16);
				
			for (i=0; i<size/2; i++)
			{
				if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle, AU8524_TUNSANYOUBD00Ax_MODJ83B[2*i],(UINT8)AU8524_TUNSANYOUBD00Ax_MODJ83B[2*i+1]);
			}
			break;

		case TUNER_MXL_5007:
			{
				status = AU8524_LowIF(nAPHandle, p_para.TunerType, p_para.ChipMod, p_para.TunerCapabilities.LowIF_QAM/10000);
				
				size = sizeof(AU8524_TUNMXL5007_MODJ83B) / sizeof(UINT16);
				
				for (i=0; i<size/2; i++)
				{
					if(STATUS_NO_ERROR(status)) status = AU8524_WriteReg(nAPHandle, AU8524_TUNMXL5007_MODJ83B[2*i],(UINT8)AU8524_TUNMXL5007_MODJ83B[2*i+1]);
				}
			}
			break;

		default:
			
			return STATUS_ERROR;
	}

	return status;
}


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
UINT16 AU8524_IsJ83B256SignalLocked(void* nAPHandle,INT32 nTunerType)
{
	UINT16	status = STATUS_OK;
	UINT16	i = 0;
	UINT8	bCur=0;
	UINT16  num = 16;

	if (nTunerType == TUNER_QJ_DTZ1W40x || nTunerType == TUNER_QJ_DTZ0R401 || nTunerType == TUNER_QJ_DTZ0W400G)
	{
		num = 5;
	}
	
	for (i=0; i<num; i++)
	{		
			
		if(STATUS_NO_ERROR(status)) status = AU8524_ReadReg(nAPHandle,0x541,&bCur);
		
		if (((bCur&0xc0) == 0x80) && ((bCur&0x30) == 0x20) )
		{
			return SIGNAL_IS_LOCKED;
		}
		
		AuvitekChip_TimeDelay(50);

	}
	return SIGNAL_NOT_LOCKED;		
	
}

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
UINT16 AU8524_IsJ83B64SignalLocked(void* nAPHandle,INT32 nTunerType)
{
	
	return AU8524_IsJ83B256SignalLocked(nAPHandle,nTunerType);			
}

const UINT32 AU8524_mse2snr_q64[256] = {
  290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290,
  290, 288, 286, 284, 282, 281, 279, 277, 275, 273, 271, 269, 268, 266, 264, 262,
  260, 259, 258, 256, 255, 254, 252, 251, 250, 249, 248, 246, 245, 244, 242, 241,
  240, 239, 239, 238, 237, 237, 236, 235, 235, 234, 233, 233, 232, 231, 231, 230,
  229, 229, 228, 228, 227, 226, 226, 225, 224, 224, 223, 222, 222, 221, 221, 220,
  220, 219, 219, 218, 218, 218, 217, 217, 217, 216, 216, 215, 215, 214, 212, 211,
  210, 210, 209, 209, 208, 208, 207, 206, 206, 205, 205, 205, 204, 204, 204, 204,
  203, 203, 203, 202, 202, 202, 202, 201, 201, 201, 201, 200, 200, 200, 199, 199,
  199, 198, 198, 197, 197, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190};

const UINT32 AU8524_mse2snr_q256[256] = {
  400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
  400, 400, 398, 396, 394, 392, 390, 388, 386, 384, 382, 380, 379, 378, 377, 376,
  375, 374, 373, 372, 371, 370, 362, 354, 346, 338, 330, 328, 326, 324, 322, 320,
  319, 318, 317, 316, 315, 314, 313, 312, 311, 310, 308, 306, 304, 302, 300, 298,
  295, 295, 294, 294, 294, 293, 293, 292, 292, 292, 291, 291, 291, 290, 290, 289,
  288, 288, 287, 286, 285, 284, 283, 282, 282, 281, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280};


/*****************************************************************************
* function:    AU8524_GetJ83BSignalQuality
* Description: get signal quality of J83B(include J83B64 and J83B256)
* call AU8524_GetJ83BSignalQuality every ~30 ms, display signal quality(snr) every >0.5 sec 
* Input:       mode 
			     1: J83B64_MODE
			     2: J83B256_MODE
* Output:      pSignalQuality: the pointer to signal Quality,return snr in dB
* Return:      status:
*                  STATUS_OK: no error
*                  user-defined
*****************************************************************************/
UINT16 AU8524_GetJ83BSignalQuality( void* nAPHandle, UINT16 *pSignalQuality,AU_TV_FORMAT_T mode)
{
	static INT32 i_signal=0,i_lastSignal=0;
	UINT8 nCur=0x0;
	UINT16 status = STATUS_OK;
	INT32 i_index = 0;

	/* read register address 0x522 */
	if(STATUS_NO_ERROR(status))	status = AU8524_ReadReg(nAPHandle,0x522,&nCur); 	
	
	i_signal = i_signal - (i_signal>>7) + nCur;
	
	if( ABS(i_signal - i_lastSignal) > 3*128 )
	{
		i_lastSignal = i_signal;  
	}
	
	i_index = i_lastSignal>>7;
	
	/*out of bound protection*/
	if(i_index < 0) i_index = 0;
	else if(i_index > 255) i_index = 255; 	
	
	if (TV_QAM64 == mode)
	{
		*pSignalQuality = AU8524_mse2snr_q64[i_index];
	}

	if (TV_QAM256 == mode)
	{
		*pSignalQuality = AU8524_mse2snr_q256[i_index];		
	}
	
	/*read register 0x541 to check lock */
	if(STATUS_NO_ERROR(status)) status = AU8524_ReadReg(nAPHandle,0x541,&nCur);
			
	if (((nCur&0xc0) == 0x80) && ((nCur&0x30) == 0x20) )
	{
		/* signal is locked */
		return status;
	}

	/*signal is not locked, return 0 */
	*pSignalQuality = 0;

	return status;

}

#ifdef IIC_Demo
/*****************************************************************************
*function:	AU8524_GetATSC_Freq_offset
*Description:	Get ATSC Freq offset
*input:		*freqOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetATSC_Freq_Offset(void* nAPHandle,long *FreqOffset)
{
	UINT16 status = STATUS_OK;
	UINT8 value0xdf = 0;

	status = AU8524_ReadReg(nAPHandle,0xdf, &value0xdf);

	//*FreqOffset = (value0xdf-0x43)*3.125/2;
	*FreqOffset = ((long)value0xdf -0x43)*3125/2000;

	return status;

}

/*****************************************************************************
*function:	AU8524_GetATSC_Timing_Offset
*Description:	Get ATSC Timing offset
*input:		*TimingOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetATSC_Timing_Offset(void* nAPHandle,long *TimingOffset)
{
	UINT16 status = STATUS_OK;
	INT8 value0x312 = 0;

	status = AU8524_ReadReg(nAPHandle,0x312, (UINT8*)(&value0x312));

	//*TimingOffset = value0x312*125.44/64;
      *TimingOffset = (long)value0x312*12544/6400;	

	return status;
	
}

/*****************************************************************************
*function:	AU8524_GetJ83B_Freq_Offset
*Description:	Get J83B Freq offset
*input:		*FreqOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetJ83B_Freq_Offset(void* nAPHandle,double *FreqOffset)
{
	UINT16 status = STATUS_OK;
	UINT8 value0x201 = 0;
	UINT8 value0x202 = 0;
	INT16 CarrierFreq = 0;

	status = AU8524_ReadReg(nAPHandle,0x201, &value0x201);
	status = AU8524_ReadReg(nAPHandle,0x202, &value0x202);
	CarrierFreq = ((value0x202 & 0x0f) << 8) + value0x201;
    if(CarrierFreq&0x800) CarrierFreq |= 0xf000;

	//*FreqOffset = -CarrierFreq/pow(2, 18) * 54e3;
	*FreqOffset = 0;

	return status;
}

/*****************************************************************************
*function:	AU8524_GetJ83B_Timing_Offset
*Description:	Get J83B Timing offset
*input:		TVFormat
*				1:J83B64
*				2:J83B256
*			*FreqOffset
*Retrun:	status:
*				STATUS_OK: no error
*				user-defined
******************************************************************************/
UINT16 AU8524_GetJ83B_Timing_Offset(void* nAPHandle,AU_TV_FORMAT_T nChipMod, double *TimingOffset)
{
	UINT16 status = STATUS_OK;
	UINT8 value0x214 = 0;
	UINT8 value0x215 = 0;
	UINT8 value0x216 = 0;
	UINT8 value0x217 = 0;
	UINT32 FreqOut = 0; 
	double fsym;
	
	status = AU8524_ReadReg(nAPHandle,0x214, &value0x214);
	status = AU8524_ReadReg(nAPHandle,0x215, &value0x215);
	status = AU8524_ReadReg(nAPHandle,0x216, &value0x216);
	status = AU8524_ReadReg(nAPHandle,0x217, &value0x217);
	
	FreqOut = (value0x217<<24)|(value0x216<<16)|(value0x215<<8)|value0x214;

	if (nChipMod == TV_QAM64)
		fsym = 5.056941;
	else
		fsym = 5.360537;

	//*TimingOffset = -(FreqOut - pow(2, 28))/pow(2, 38)*2*fsym/27*1e6;
      *TimingOffset = 0;

	return status;

}
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
UINT16 AU8524_SetTSOutput(void* nAPHandle, AU_TS_OUTPUT_T nTSOutput)
{
	UINT16 status = STATUS_OK;
         UINT8 data;

	if (nTSOutput == TS_PARALLEL_OUTPUT)
		status = AU8524_WriteReg(nAPHandle, 0x0a5, 0x80);
	else if (nTSOutput == TS_SERIAL_OUTPUT)
		status = AU8524_WriteReg(nAPHandle, 0x0a5, 0x40);
        AU8524_ReadReg(nAPHandle, 0x101, &data);
        data |= 0x01;
        
        status = AU8524_WriteReg(nAPHandle, 0x101, 0x1);
	
	return status;
}
