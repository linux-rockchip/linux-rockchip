/*!  

	\file		Osw_Time.h
	
	\brief		OS Wrapper - Time header
	
				The following are time stamp reading functions for debug purpose.
																
	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  
																	 */

#ifndef __OSW_TIME_H
#define __OSW_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include_internal/Include/sms_common.h"

/*!
	Returns the number of milliseconds elapsed since OS started.

	\return		Number of milliseconds elapsed since OS started
*/	
UINT32 OSW_GetCurTimeStampInMSec( void );


#ifdef __cplusplus
}
#endif

#endif
