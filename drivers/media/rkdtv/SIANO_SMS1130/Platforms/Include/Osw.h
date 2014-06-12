/*!  

	\file		Osw.h
	
	\brief		OS Wrapper - Master header, the only one which should be included
																
	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  
																	 
*/

#ifndef __OSW_MAIN_INCLUDE_H
#define __OSW_MAIN_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#define		OSW_OK						0
#define		OSW_TIMEOUT					1
#define		OSW_ERROR					2

#define		OSW_WAIT_FOREVER			0xFFFFFFFF

#include "../../include_internal/Include/sms_common.h"
#include "Osw_Task.h"
#include "Osw_Mutex.h"
#include "Osw_Event.h"
#include "Osw_Memory.h"
#include "Osw_Time.h"
#include "Osw_FileSystem.h"

void OSW_OsInit(void);
void OSW_OsDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // __OSW_MAIN_INCLUDE_H

