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
#if !defined( __AUVITEK_DEV_H )
#define __AUVITEK_DEV_H

#include "AUChipTypedef.h"
 
typedef struct 
{
	INT32	m_AUChipHandle;
	INT32	m_AUTunerHandle;
	INT32	m_AUChipType;
	INT32	m_AUTunerType;
}G_APPCTHandle;

#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage                  */
{
#endif

static UINT16 AU88xx_PALI2DK[] = 
{
	0x22A, 0xee,
	0x22B, 0x08,
	0x22C, 0x5f,
	0x22D, 0xe6,
	0x22f, 0x3e,
	0x653, 0x85,
	0x654, 0xf6,
	0x655, 0x0e,
	0x656, 0x00,
	0x657, 0xe0,
	0x658, 0x57,
	0x659, 0xed,
	0x65a, 0x01,
	0x65b, 0xd3,
};

static UINT16 AU88xx_PALDK2I[] = 
{
	0x22A, 0xc7,
	0x22B, 0x0a,
	0x22C, 0x5e,
	0x22D, 0xf3,
	0x22f, 0x80,
	0x653, 0x8e,
	0x654, 0xe3,
	0x655, 0x2c,
	0x656, 0x00,
	0x657, 0x70,
	0x658, 0x7e,
	0x659, 0xf8,
	0x65a, 0x00,
	0x65b, 0xbd,
};

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
UINT32 AU_SetTuner(void* nAPHandle, AU_ChipParamete p_para);
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
UINT32 AU_OpenChip(void* nAPHandle, AU_CHIPTYPE_T nChipType);
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
UINT32 AU_SetChip(void* nAPHandle, AU_ChipParamete p_para);
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
UINT32 AU_SetChannel(void* nAPHandle, AU_ChipParamete p_para);
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
void AU_PAL_AutoSwitch(void* nAPHandle, AU_TV_FORMAT_T nchipMode);
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
void AU_ChangeParaToTunerPara(AU_ChipParamete p_para, PAU_TunerParameter n_TunerParamete);
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
UINT32 AU_Initialize(AU_ChipParamete* p_para);
UINT32 AU8524_INIT(void);
UINT32 AU8524_Set_Status(UINT32 freq);

#if defined( __cplusplus )
}
#endif

#endif