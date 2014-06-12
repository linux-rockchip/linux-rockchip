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

****************************************************************************************
**  Name:				Auvitek_MXL_5007.c
**
**  Project:            Au85xx IIC Controller
**
**  Description:		software interface
**						set tuner MXL_5007 with difference TV mode		    
**                    
**  Functions
**  Implemented:   
**						INT32	AU_TunerOpen_MXL_5007
**						TUNER_NTSTATUS	AU_TunerClose_MXL_5007
**						TUNER_NTSTATUS	AU_SetTunerParameter_MXL_5007
**						TUNER_NTSTATUS	AU_SetTunerFrequency_MXL_5007
**						PAU_TunerCapabilities	AU_GetTunerCapabilities_MXL_5007
**						
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History: 
**  -------------------------------------------------------------------------
*      SCR      Date         Author     Description
*
*****************************************************************************************/

//Now Auvitek use the Maxlinear reference code file version:
//v2.6.14
//The Version files are recommended to be used in your system.



#include "Auvitek_Maxlinear_MXL5007.h"

#ifdef IICTUNERLIB
#include "SRC/MxL5007.h"
#include "SRC/MxL5007_API.h"
#include "../Common/AuvitekTunerDev.h"

#else

#include "MxL5007.h"
#include "MxL5007_API.h"
#include "AuvitekTunerDev.h"

#endif
#include <linux/string.h>
#include <linux/delay.h>


AU_TunerCapabilities m_TunerCapabilities_MXL5007[TUNER_MAX_UNITS];
UINT8				 m_Mode_MXL5007[TUNER_MAX_UNITS];

PAU_TunerParameter	m_MXLTunerParameter[TUNER_MAX_UNITS];
AU_TunerHandle		 m_TunerHandle_MXL5007[TUNER_MAX_UNITS];
MxL5007_TunerConfigS myTuner;
/*****************************************************************************
* function:     AU_TunerOpen_MXL5007
* Description:  Open the turner 
* Input:        nAPHandle: the handler of the turner
*				nHWAddress:	the address of the turner
* Return:
*               TurnerUint: the number of the turner
*               -1: in error
*****************************************************************************/
INT32	AU_TunerOpen_MXL5007(void* nAPHandle,UINT32 nHWAddress)
{
	int   nPos = 0;
	INT32 TunerUnit = -1;

	static int initflag = 0;

	if(initflag == 0)
	{
		for(nPos=0;nPos<TUNER_MAX_UNITS;nPos++)
		{
			m_TunerHandle_MXL5007[nPos].APHandle  = 0;
			m_TunerHandle_MXL5007[nPos].HWAddress = 0;
		}

		initflag = 1;
	}

	if(nAPHandle == 0 || nHWAddress == 0)
	{
		return -1;
	}

	for(nPos=0;nPos<TUNER_MAX_UNITS;nPos++)
	{
		if( m_TunerHandle_MXL5007[nPos].APHandle == nAPHandle && m_TunerHandle_MXL5007[nPos].HWAddress == nHWAddress )
		{
			return nPos;
		}
	}

	for(nPos=0;nPos<TUNER_MAX_UNITS;nPos++)
	{
		if( m_TunerHandle_MXL5007[nPos].APHandle == 0 && m_TunerHandle_MXL5007[nPos].HWAddress ==0 )
		{
			TunerUnit = nPos;

			break;
		}
	}

	if(TunerUnit == -1)
	{
		return -1;
	}

	m_TunerHandle_MXL5007[TunerUnit].APHandle  = nAPHandle;
	m_TunerHandle_MXL5007[TunerUnit].HWAddress = nHWAddress;

	m_TunerCapabilities_MXL5007[TunerUnit].AnalogTunerCapabilities		= TUNER_AnalogVideo_NTSC_M | TUNER_AnalogVideo_PAL_B | TUNER_AnalogVideo_PAL_D | TUNER_AnalogVideo_PAL_G | TUNER_AnalogVideo_PAL_I ;
	m_TunerCapabilities_MXL5007[TunerUnit].DigitalTunerCapabilities		=TUNER_Digital_MOD_64QAM | TUNER_Digital_MOD_256QAM | TUNER_Digital_MOD_8VSB;
	m_TunerCapabilities_MXL5007[TunerUnit].RadioTunerCapabilities			= TUNERPROPERTY_TUNER_MODE_FM_RADIO;

	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_ATSC			=	6000000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_QAM			=	6000000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_DVB_T			=	8000000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_DVB_C			=	8000000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_DTMB			=	8000000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_NTSC			=	6500000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_PAL_I			=	8500000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_PAL_BG		=	8500000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_PAL_DK		=	8500000;
	m_TunerCapabilities_MXL5007[TunerUnit].LowIF_FM			=	0;

	m_TunerCapabilities_MXL5007[TunerUnit].Reserve				=   0;
	
	return TunerUnit;
}
/*****************************************************************************
* function:     AU_TunerClose_MXL5007
* Description:  Close the turner 
* Input:        TurnerUint: the number of the turner
* Return:		status:
*                    TUNER_STATUS_SUCCESS
*                    TUNER_STATUS_UNSUCCESSFUL
*****************************************************************************/
TUNER_NTSTATUS	AU_TunerClose_MXL5007(UINT32 TunerUnit)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (TunerUnit > TUNER_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}

	m_TunerHandle_MXL5007[TunerUnit].APHandle  = 0;
	m_TunerHandle_MXL5007[TunerUnit].HWAddress = 0;
	
	return ntStatus;
}
/*****************************************************************************
* function:     AU_SetTunerParameter_MXL5007
* Description:  Set the parameter of the turner 
* Input:        TurnerUint: the number of the turner
*				nTurnerParameter: the parameter
* Return:		status:
*                    TUNER_STATUS_SUCCESS
*                    TUNER_STATUS_UNSUCCESSFUL
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerParameter_MXL5007(UINT32 TunerUnit,PAU_TunerParameter nTunerParameter)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if (TunerUnit > TUNER_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}
	m_MXLTunerParameter[TunerUnit]=nTunerParameter;

	return ntStatus;
}
/*****************************************************************************
* function:     AU_SetTunerFrequency_MXL5007
* Description:  Set the frequency of the turner
* Input:        TurnerUint: the number of the turner
*				nFrequency: the frequency
* Return:		status:
*                    TUNER_STATUS_SUCCESS
*                    TUNER_STATUS_UNSUCCESSFUL
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerFrequency_MXL5007(UINT32 TunerUnit,UINT32 nFrequency)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;
	int status=0;

	if (TunerUnit > TUNER_MAX_UNITS)
	{
        return TUNER_STATUS_UNSUCCESSFUL;
	}
	if( I2C_EnableSecondPort(m_TunerHandle_MXL5007[TunerUnit].APHandle,1))
            return TUNER_STATUS_UNSUCCESSFUL;
         
	
	myTuner.I2C_Addr = m_TunerHandle_MXL5007[TunerUnit].HWAddress;
	myTuner.Mode = MxL_MODE_ATSC;
	myTuner.IF_Diff_Out_Level = 0;
	myTuner.Xtal_Freq = MxL_XTAL_24_MHZ;
	myTuner.IF_Freq = MxL_IF_6_MHZ;
	myTuner.IF_Spectrum = 0;
	myTuner.ClkOut_Setting = 0;
	myTuner.ClkOut_Amp = 0;
	myTuner.BW_MHz = MxL_BW_6MHz;
	myTuner.RF_Freq_Hz = 0;
	switch(m_MXLTunerParameter[TunerUnit]->ADMod) 
	{
		case AU_ADMod_DIGITALMOD:
			switch(m_MXLTunerParameter[TunerUnit]->DigitalDemodStand) 
			{
				case AU_DigitalDemodStand_ATSC:
					myTuner.Mode = MxL_MODE_ATSC;
					status = MxL_Tuner_Init(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner);
					printk("in AU_SetTunerFrequency_MXL5007 the MxL_Tuner_Init status is %d\n",status);
					printk("The Tuner Set Freq is %d \n",nFrequency);
					MxL_Tuner_RFTune(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner, nFrequency,6);
					break;
				case AU_DigitalDemodStand_QAM64:
				case AU_DigitalDemodStand_QAM256:
					myTuner.Mode = MxL_MODE_CABLE;
					MxL_Tuner_Init(m_TunerHandle_MXL5007[TunerUnit].APHandle,&myTuner);
					MxL_Tuner_RFTune(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner,nFrequency,6);
					break;
				case AU_DigitalDemodStand_DVB_T:
					myTuner.Mode = MxL_MODE_DVBT;
					myTuner.BW_MHz = MxL_BW_8MHz;
					MxL_Tuner_Init(m_TunerHandle_MXL5007[TunerUnit].APHandle,&myTuner);
					MxL_Tuner_RFTune(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner,nFrequency,8);
					break;
				case AU_DigitalDemodStand_DVB_C:
					myTuner.Mode = MxL_MODE_DVBT;
					myTuner.BW_MHz = MxL_BW_8MHz;
					MxL_Tuner_Init(m_TunerHandle_MXL5007[TunerUnit].APHandle,&myTuner);
					MxL_Tuner_RFTune(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner,nFrequency,8);
					break;
				case AU_DigitalDemodStand_DTMB:
					myTuner.Mode = MxL_MODE_DVBT;
					myTuner.BW_MHz = MxL_BW_8MHz;
					MxL_Tuner_Init(m_TunerHandle_MXL5007[TunerUnit].APHandle,&myTuner);
					MxL_Tuner_RFTune(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner,nFrequency,8);
					break;
			}
			break;
		case AU_ADMod_ANALOGMOD:
			switch(m_MXLTunerParameter[TunerUnit]->AnalogVideoStand)
			{
			case AU_AnalogVideoStand_NTSC:
				myTuner.Mode = MxL_MODE_CABLE;
				MxL_Tuner_Init(m_TunerHandle_MXL5007[TunerUnit].APHandle,&myTuner);
				MxL_Tuner_RFTune(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner, nFrequency,6);
				break;
			case AU_AnalogVideoStand_PAL_DK:
			case AU_AnalogVideoStand_PAL_BG:
			case AU_AnalogVideoStand_PAL_I:
				myTuner.Mode = MxL_MODE_CABLE;
				myTuner.BW_MHz = MxL_BW_8MHz;
				MxL_Tuner_Init(m_TunerHandle_MXL5007[TunerUnit].APHandle,&myTuner);
				MxL_Tuner_RFTune(m_TunerHandle_MXL5007[TunerUnit].APHandle, &myTuner, nFrequency,8);
				break;
			}
			break;
	}
	if(I2C_EnableSecondPort(m_TunerHandle_MXL5007[TunerUnit].APHandle,0))
            return TUNER_STATUS_UNSUCCESSFUL;

	return ntStatus;
}
/*****************************************************************************
* function:     AU_GetTunerCapabilities_MXL5007
* Description:  Get the capabilities of the turner
* Input:        
*				TurnerUint: the number of the turner
* Return:		
*               m_TunerCapabilities_MXL5007[TunerUnit]
*				0           
*****************************************************************************/
PAU_TunerCapabilities	AU_GetTunerCapabilities_MXL5007(UINT32 TunerUnit)
{
	if (TunerUnit > TUNER_MAX_UNITS)
	{
        return 0;
	}

	return &m_TunerCapabilities_MXL5007[TunerUnit];
}
