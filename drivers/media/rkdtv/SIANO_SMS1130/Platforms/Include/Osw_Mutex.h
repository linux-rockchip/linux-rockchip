/*!  

	\file		Osw_Mutex.h
	
	\brief		OS Wrapper - Mutex header. 
	
				The following are functions for Mutex mechanism.
																
	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  

    \note 
	    -# Example of Mutex definition: \n
		    <CODE> typedef HANDLE Mutex; </CODE> \n
        Should you need to change the above definition, do as in \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Win/Include/Osw_MutexTarget.h </CODE> \n
        but in your own OS flavor directory (for example \"Linux\") \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Linux/Include/Osw_MutexTarget.h </CODE>
																	 	
*/

#ifndef __OS_MUTEX_H
#define __OS_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include_internal/Include/sms_common.h"

///////////////////////////////////////////////////////////////////////////////
/*! 
	Create a mutual exclusion synchronization object to handle critical
	sections in task context.

	\param[out] pMutex		Where to store the mutex semaphore

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexCreate
	(
	Mutex*					pMutex
	);


/*!
    Delete a previously created Mutex object [reverse operation to OSW_MutexCreate()]

	\param[in]  pMutex      Pointer to Mutex to delete

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexDelete
	(
	Mutex*					pMutex
	);


/*!
    Mutex GET operation (at the start of a critical section)

	\param[in]  pMutex      Pointer to Mutex to get

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexGet
	(
	Mutex*					pMutex
	);


/*!
    Mutex PUT operation (at the end of a critical section)

	\param[in]  pMutex      Pointer to Mutex to put

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexPut
	(
	Mutex*					pMutex
	);


#ifdef __cplusplus
}
#endif

#endif
