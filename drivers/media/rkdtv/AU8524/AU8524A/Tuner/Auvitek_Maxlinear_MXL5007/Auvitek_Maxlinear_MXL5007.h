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
****************************************************************************************
**  Name:				Auvitek_MXL_5007.h
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

#if !defined( __AUVITEK__MXL5007_H )
#define __AUVITEK__MXL5007_H


#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage                  */
{
#endif

#ifdef IICTUNERLIB
	#include "../Common/AuvitekTunerInterfaceTypedef.h"	
#else

	#include "AuvitekTunerInterfaceTypedef.h"	

#endif

/*****************************************************************************
* function:     AU_TunerOpen_MXL5007
* Description:  Open the turner 
* Input:        nAPHandle: the handler of the turner
*				nHWAddress:	the address of the turner
* Return:
*               TurnerUint: the number of the turner
*               -1: in error
*****************************************************************************/	
INT32	AU_TunerOpen_MXL5007(void* nAPHandle,UINT32 nHWAddress);
/*****************************************************************************
* function:     AU_TunerClose_MXL5007
* Description:  Close the turner 
* Input:        TurnerUint: the number of the turner
* Return:		status:
*                    TUNER_STATUS_SUCCESS
*                    TUNER_STATUS_UNSUCCESSFUL
*****************************************************************************/
TUNER_NTSTATUS	AU_TunerClose_MXL5007(UINT32 TunerUnit);
/*****************************************************************************
* function:     AU_SetTunerParameter_MXL5007
* Description:  Set the parameter of the turner 
* Input:        TurnerUint: the number of the turner
*				nTurnerParameter: the parameter
* Return:		status:
*                    TUNER_STATUS_SUCCESS
*                    TUNER_STATUS_UNSUCCESSFUL
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerParameter_MXL5007(UINT32 TunerUnit,PAU_TunerParameter nTunerParameter);
/*****************************************************************************
* function:     AU_SetTunerFrequency_MXL5007
* Description:  Set the frequency of the turner
* Input:        TurnerUint: the number of the turner
*				nFrequency: the frequency
* Return:		status:
*                    TUNER_STATUS_SUCCESS
*                    TUNER_STATUS_UNSUCCESSFUL
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerFrequency_MXL5007(UINT32 TunerUnit,UINT32 nFrequency);
/*****************************************************************************
* function:     AU_GetTunerCapabilities_MXL5007
* Description:  Get the capabilities of the turner
* Input:        
*				TurnerUint: the number of the turner
* Return:		
*               m_TunerCapabilities_MXL5007[TunerUnit]
*				0           
*****************************************************************************/
PAU_TunerCapabilities	AU_GetTunerCapabilities_MXL5007(UINT32 TunerUnit);

#if defined( __cplusplus )
}
#endif

#endif
