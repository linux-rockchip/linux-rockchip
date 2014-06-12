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
**  Name:		Auvitek_Dev.c
**
**  Project:            Au85xx IIC Controller
**
**  Description:				    
**                    
**  Functions
**  Implemented:   
**
**						
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History: 
*****************************************************************************/
#include "Auvitek_Dev.h"
#include "AUChipInterface.h"
#include "AuvitekTunerInterface.h"
#include "Auvitek_Userdef.h"
#include<linux/string.h>
#include <linux/delay.h>
G_APPCTHandle m_GAPPCTHandle;
PAU_TunerCapabilities nAUTunerCapabilities = NULL;
AU_TunerParameter nAUTunerParamete;
AU_ChipParamete ATSC_ChipPara;
INT32 nAUTunerUnit;
INT32 pChipUnit;
/**************************************************************************************
**	Name: AU_SetTuner
**
**	Description:	Set Tuner
**
**	Parameters:   
**					nAPHandle	- Handle
**					p_para	- parameter for setting tuner
**
**	Returns:        status:
**					STATUS_OK	- No errors
**					user-defined
**
***************************************************************************************/
UINT32 AU_SetTuner(void* nAPHandle, AU_ChipParamete p_para)
{
	UINT32 status = STATUS_UNKNOWN;
	
	nAUTunerUnit = AU_TunerOpen(nAPHandle, p_para.TunerAddress, p_para.TunerType);
	AU_ChangeParaToTunerPara(p_para,&nAUTunerParamete);
	AU_SetTunerParameter(nAUTunerUnit,&nAUTunerParamete);
	nAUTunerCapabilities = AU_GetTunerCapabilities(nAUTunerUnit);
	status = AU_SetTunerFrequency(nAUTunerUnit, p_para.FreqHz);
	//status = nAUTunerUnit;
	return status;
}

/**************************************************************************************
**	Name: AU_OpenChip
**
**	Description:	Open chip and return handle
**
**	Parameters:   
**					nAPHandle	- Handle
**					nChipType	- Chip type
**
**	Returns:        ResultHandle
**
***************************************************************************************/
UINT32 AU_OpenChip(void* nAPHandle, AU_CHIPTYPE_T nChipType)
{
	INT32 ResultHandle = -1;

	ResultHandle = AU_ChipOpen(nAPHandle, nChipType);
	if(ResultHandle != -1)
	{
		m_GAPPCTHandle.m_AUChipHandle = ResultHandle;
		m_GAPPCTHandle.m_AUChipType	  = nChipType;
	}
	return ResultHandle;
}

/**************************************************************************************
**	Name: AU_SetChip
**
**	Description:	Set demodulator chip
**
**	Parameters:   
**					nAPHandle	- Handle
**					p_para	- parameter for setting AU85xx chip
**
**	Returns:        status:
**					STATUS_OK	- No errors
**					user-defined
**
***************************************************************************************/
UINT32 AU_SetChip(void* nAPHandle, AU_ChipParamete p_para)
{
	UINT32 status = STATUS_UNKNOWN;
	
 	if (nAUTunerCapabilities != NULL)
		memcpy(&p_para.TunerCapabilities, nAUTunerCapabilities, sizeof(AU_TunerCapabilities));
	status = AU_SetChipMod(m_GAPPCTHandle.m_AUChipHandle, p_para);

	return status;
}

/**************************************************************************************
**	Name: AU_SetChannel
**
**	Description:	set demodulator and tuner
**
**	Parameters:   
**					nAPHandle	- Handle
**					p_para	- parameter
**
**	Returns:        status:
**					STATUS_OK	- No errors
**					user-defined
**
***************************************************************************************/
UINT32 AU_SetChannel(void* nAPHandle, AU_ChipParamete p_para)
{
	UINT32 status = STATUS_UNKNOWN;

	if (p_para.ChipMod == TV_FM)
		p_para.FreqHz -= 2250000;
	status = AU_SetTuner(nAPHandle, p_para);
	printk("IN AU_SetChannel the AU_SetTuner Status is %d\n",status);
	Auvitek_TimeDelay(100);
	
 	status = AU_SetChip(nAPHandle, p_para);
	printk("IN AU_SetChannel the AU_SetChip Status is %d\n",status);
        
	

 	AU_GetChipLock(m_GAPPCTHandle.m_AUChipHandle, &status);



	if (status == SIGNAL_IS_LOCKED && p_para.ChipSeries == AU88xxSeries)
		AU_PAL_AutoSwitch(nAPHandle, p_para.ChipMod);

	return status;
}

/**************************************************************************************
**	Name: AU_PAL_AutoSwitch
**
**	Description:	Auto switch PAL-DK between PAL-I
**
**	Parameters:   
**					nAPHandle	- Handle
**					nchipMode	- Chip mode
**
**	Returns: 
**
***************************************************************************************/
void AU_PAL_AutoSwitch(void* nAPHandle, AU_TV_FORMAT_T nchipMode)
{
	UINT8 value0x660_1;
	UINT8 value0x660_2;
	UINT16 size = 0;
	int i;

	if(nchipMode == TV_PAL_DK || nchipMode == TV_PAL_I)
	{
		Auvitek_ReadReg(0x660, &value0x660_1);
		if (value0x660_1 > 0x50)
		{	
			if (nchipMode == TV_PAL_I)
			{
				size = sizeof(AU88xx_PALI2DK)/sizeof(UINT16);
				for (i=0; i<size/2; i++)
					Auvitek_WriteReg(AU88xx_PALI2DK[2*i],(UINT8)AU88xx_PALI2DK[2*i+1]);	
				Auvitek_TimeDelay(100);
				Auvitek_ReadReg(0x660, &value0x660_2);
				if(value0x660_2>value0x660_1)
				{
					size = sizeof(AU88xx_PALDK2I)/sizeof(UINT16);
					for (i=0; i<size/2; i++)
						Auvitek_WriteReg(AU88xx_PALDK2I[2*i],(UINT8)AU88xx_PALDK2I[2*i+1]);	
				}
			}
			else
			{
				size = sizeof(AU88xx_PALDK2I)/sizeof(UINT16);
				for (i=0; i<size/2; i++)
					Auvitek_WriteReg(AU88xx_PALDK2I[2*i],(UINT8)AU88xx_PALDK2I[2*i+1]);	
				Auvitek_TimeDelay(100);
				Auvitek_ReadReg(0x660, &value0x660_2);
				if(value0x660_2>value0x660_1)
				{
					size = sizeof(AU88xx_PALI2DK)/sizeof(UINT16);
					for (i=0; i<size/2; i++)
						Auvitek_WriteReg(AU88xx_PALI2DK[2*i],(UINT8)AU88xx_PALI2DK[2*i+1]);	
				}
			}
		}
	}
	else
		return;
}
/**************************************************************************************
**	Name: AU_ChangeParaToTunerPara
**
**	Description:	change globe parameter to tuner parameter
**
**	Parameters:   
**					p_para	- globe parameter
**					n_TunerParamete	- tuner parameter
**
**	Returns:        
**
***************************************************************************************/
void AU_ChangeParaToTunerPara(AU_ChipParamete p_para, PAU_TunerParameter n_TunerParamete)
{
	switch(p_para.AirCable)
	{
		case AIR_MODE:
			n_TunerParamete->TunerInputType = AU_TunerInputType_AIR;
			break;
		case CABLE_MODE:
			n_TunerParamete->TunerInputType = AU_TunerInputType_CABLE;
			break;
	}
	switch(p_para.ChipMod) 
	{
		case TV_ATSC:
			n_TunerParamete->ADMod	= AU_ADMod_DIGITALMOD;
			n_TunerParamete->DigitalDemodStand = AU_DigitalDemodStand_ATSC;
			break;
		case TV_QAM:
		case TV_QAM64:
			n_TunerParamete->ADMod	= AU_ADMod_DIGITALMOD;
			n_TunerParamete->DigitalDemodStand = AU_DigitalDemodStand_QAM64;
			break;
		case TV_QAM256:
			n_TunerParamete->ADMod	= AU_ADMod_DIGITALMOD;
			n_TunerParamete->DigitalDemodStand = AU_DigitalDemodStand_QAM256;
			break;

		case TV_NTSC:
			n_TunerParamete->ADMod	= AU_ADMod_ANALOGMOD;
			n_TunerParamete->TVMod	= AU_TVMod_TV;
			n_TunerParamete->AnalogInputSource = AU_AnalogInputSource_RF;
			n_TunerParamete->AnalogVideoStand = AU_AnalogVideoStand_NTSC;
			n_TunerParamete->AudioFormat = AU_AudioFormat_BTSC;
			break;

		case TV_FM:
			n_TunerParamete->ADMod	= AU_ADMod_ANALOGMOD;
			n_TunerParamete->TVMod	= AU_TVMod_FM;
			break;

		case TV_PAL_I:
			n_TunerParamete->ADMod	= AU_ADMod_ANALOGMOD;
			n_TunerParamete->TVMod	= AU_TVMod_TV;
			n_TunerParamete->AnalogInputSource = AU_AnalogInputSource_RF;
			n_TunerParamete->AnalogVideoStand = AU_AnalogVideoStand_PAL_I;
			break;

		case TV_PAL_BG:
			n_TunerParamete->ADMod	= AU_ADMod_ANALOGMOD;
			n_TunerParamete->TVMod	= AU_TVMod_TV;
			n_TunerParamete->AnalogInputSource = AU_AnalogInputSource_RF;
			n_TunerParamete->AnalogVideoStand = AU_AnalogVideoStand_PAL_BG;
			break;

		case TV_PAL_DK:
			n_TunerParamete->ADMod	= AU_ADMod_ANALOGMOD;
			n_TunerParamete->TVMod	= AU_TVMod_TV;
			n_TunerParamete->AnalogInputSource = AU_AnalogInputSource_RF;
			n_TunerParamete->AnalogVideoStand = AU_AnalogVideoStand_PAL_DK;
			break;

		case TV_DVB_T:
			n_TunerParamete->ADMod	= AU_ADMod_DIGITALMOD;
			n_TunerParamete->DigitalDemodStand = AU_DigitalDemodStand_DVB_T;
			break;

		case TV_DTMB:
			n_TunerParamete->ADMod	= AU_ADMod_DIGITALMOD;
			n_TunerParamete->DigitalDemodStand = AU_DigitalDemodStand_DTMB;
			break;

		case TV_DVB_C:
			n_TunerParamete->ADMod	= AU_ADMod_DIGITALMOD;
			n_TunerParamete->DigitalDemodStand = AU_DigitalDemodStand_DVB_C;
			break;
	}

	n_TunerParamete->Reserve = 0;
}

/**************************************************************************************
**	Name: AU_Initialize
**
**	Description:	initial all parameter
**
**	Parameters:   
**					p_para	- globe parameter
**
**	Returns:        status:
**					STATUS_OK	- No errors
**					user-defined
***************************************************************************************/
UINT32 AU_Initialize(AU_ChipParamete* p_para)
{
	UINT32 status = STATUS_OK;

	memset(p_para, 0x0, sizeof(p_para));
	p_para->ChipSeries					= AU85xxSeries;
	p_para->ChipType					= AU8524;
	p_para->ChipAddress					= 0x8e;
	p_para->TunerType					= TUNER_MXL_5007;
	p_para->TunerAddress				= 0xc0;
	p_para->FreqHz						= 0;
	p_para->ChipMod						= TV_ATSC;
	p_para->TSOutput					= TS_SERIAL_OUTPUT;
	p_para->ATVAudioFormat.IISFREQOUT	= IIS_FREQ_OUT_48k;
	p_para->AirCable					= AIR_MODE;
	p_para->PLL_Adj						= 0;
	p_para->Auto_AGC					= 0;
	p_para->Rst_ATSC					= 0;
	p_para->EnableIIC2					= 1;
	p_para->Enable3DFilter				= 1;
	
	return status;
}
extern AU_ChipParamete	m_AUChipHandle_8524[CHIP_MAX_UNITS];
void *ATSC_APHandle(void)
{
    int i;
    i++;
    return;
    
}
void ATSC_TEST(void)
{
    UINT32 AU_ChipHandle;
    AU_ChipParamete	para; 
    UINT16 status = STATUS_OK;
    AU_Initialize(&para);
    //para.FreqHz = 569000000;
    para.ChipMod = TV_ATSC;
    para.AirCable = AIR_MODE; /*CABLE_MODE*/
    para.TSOutput = TS_PARALLEL_OUTPUT; /* TS_PARALLEL_OUTPUT*/

    status = AU_OpenChip((void *)1, para.ChipType);

    status = AU_SetChannel((void *)1, para);
    if(status)
        return;
    
}
UINT32 AU8524_INIT(void)
{
    UINT32 Status = STATUS_OK;
    //AU_ChipParamete	para; 
    
    Status = AU_Initialize(&ATSC_ChipPara);
    
    

    
    return Status;
}
UINT32 AU8524_Close(void)
{
    UINT32 Status = 0;
    Status = AU_ChipClose(pChipUnit);
    if(Status)
        goto Error;
    Status = AU_TunerClose(nAUTunerUnit);
Error:
    return Status;
        
}
UINT8 ke_t[0x7f];

UINT32 AU8524_Set_Status(UINT32 freq)
{
    UINT32 Status = STATUS_OK;
    INT32 AU_ChipHandle;
    UINT32 ke_test;
    UINT16  i=0;
    ATSC_ChipPara.FreqHz = freq;
    ATSC_ChipPara.ChipMod = TV_ATSC;
    ATSC_ChipPara.AirCable = AIR_MODE; //CABLE_MODE
    ATSC_ChipPara.TSOutput = TS_PARALLEL_OUTPUT; // TS_PARALLEL_OUTPUT  TS_SERIAL_OUTPUT; 
    AU_ChipHandle = AU_OpenChip((void *)1, ATSC_ChipPara.ChipType);
    pChipUnit = AU_ChipHandle;
    Status = AU_SetChannel((void *)1, ATSC_ChipPara);
    /*
    for(i=1;i<0x80;i++){
    ke_test = Auvitek_ReadReg(i, ke_t+(i-1));//0xdf, 0xeb, 0xec, 0xb5, 0xb6, 0xb7, 0x88.
    if(ke_test)
        ke_test = STATUS_ERROR;
        }
    Status = ke_test;
    */
    if(Status == SIGNAL_IS_LOCKED)
        return 1;
    else
        return 0;
   
}
