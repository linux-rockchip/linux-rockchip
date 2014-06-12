/*************************************************************************************
*  CONFIDENTIAL AND PROPRIETARY SOFTWARE/DATA OF Auvitek International, Ltd.         *
*																					 *
*  Copyright (c) 2005-2018   Auvitek International, Ltd.  All Rights Reserved.       *
*																					 *
*  Use of this Software/Data is subject to the terms and conditions of               *
*  the applicable agreement between Auvitek International, Ltd, and receiving party. *
*  In addition, this Software/Data is protected by copyright law and international   *
*  treaties.                                                                         *
*																					 *
*  The copyright notice(s) in this Software/Data does not indicate actual            *
*  or intended publication of this Software/Data.                                    *
**************************************************************************************
**  Name:			AuvitekTunerInterface.h
**
**  Project:            	Au85xx IIC Controller
**
**  Description:		mainly define common header files and macro.		    
**                    
**  Functions
**  Implemented:   
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History: 
**  -------------------------------------------------------------------------
*      SCR      Date         Author     Description

*****************************************************************************/


#if !defined(AUVITEKTUNERINTERFACE__H_)
#define AUVITEKTUNERINTERFACE__H_

#pragma once

 #include "AuvitekTunerInterfaceTypedef.h"

#if defined( __cplusplus )     

extern "C"{
#endif

#ifdef IICTUNERLIB
#else
	//#include <wdm.h>
#endif

/*****************************************************************************
**
**  Name: AU_TunerOpen
**
**  Description:    open the tuner.
**
**  Parameters:   
**                  nAPHandle	- the tuner handle
**                  nHWAddress	- the IIC address of the tuner
**
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
INT32	AU_TunerOpen(void* nAPHandle,UINT32 nHWAddress,INT32 nTunerType);

/*****************************************************************************
**
**  Name: AU_TunerClose
**
**  Description:    close the tuner.
**
**  Parameters:   
**                  TunerUnit	- the tuner number
**
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS	AU_TunerClose(UINT32 TunerUnit);

/*****************************************************************************
**
**  Name: AU_SetTunerParameter
**
**  Description:    set parameter to the tuner.
**
**  Parameters:   
**                  TunerUnit		- the tuner number
**		    PAU_TunerParameter	- the parameter
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerParameter(UINT32 TunerUnit,PAU_TunerParameter nTunerParameter);

/*****************************************************************************
**
**  Name: AU_SetTunerFrequency
**
**  Description:    set frequency to the tuner.
**
**  Parameters:   
**                  TunerUnit	- the tuner number
**		    nFrequency	- the frequency
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerFrequency(UINT32 TunerUnit,UINT32 nFrequency);

/*****************************************************************************
**
**  Name: AU_GetTunerCapabilities
**
**  Description:    get capabilities to the tuner.
**
**  Parameters:   
**                  TunerUnit		- the tuner number
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
PAU_TunerCapabilities	AU_GetTunerCapabilities(UINT32 TunerUnit);

#if defined( __cplusplus )     
}
#endif

#endif