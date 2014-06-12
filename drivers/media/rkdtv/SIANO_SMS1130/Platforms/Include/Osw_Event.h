/*!  

	\file		Osw_Event.h
	
	\brief		OS Wrapper - Events header
				Events can be used to synchronize a task with another task or with an ISR. 
																				
	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  
																	 
	\note
		-# The above functions implementation may use a Mutex or Semaphore, 
			when supported by the selected OS.
		-# Example of Event definition: \n
		<CODE> typedef HANDLE Event; </CODE> \n
		Should you need to change the above definition, do as appears in \n
		<CODE> ~/HostLib/AdaptLayers/OS/Win/Include/Osw_EventTarget.h </CODE> \n
		but in your own OS flavor directory (for example \"Linux\") \n
		<CODE> ~/HostLib/AdaptLayers/OS/Linux/Include/Osw_EventTarget.h </CODE>
	
*/

#ifndef __OSW_EVENT_H
#define __OSW_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include_internal/Include/sms_common.h"

/*!
	Create an event.
	The event shall be created in cleared state, so that a subsequent call to
	OSW_EventWait() will block the calling task.
	The API shall be callable from TASK context.

	\param[out]	pEvent			Where to store the OS specific event data or handle

	\return						OSW_OK if OK - or a different value in case of error
*/	
UINT32 OSW_EventCreate
	(
	Event*		pEvent			// where to store the event 
    );


/*!
	Delete an event (the reverse operation to OSW_EventCreate).
	This function shall release the memory associated with the specific event.
	The API shall be callable from TASK context.
	
	\param[in]	pEvent			Pointer to the event to delete

	\return						OSW_OK if OK - or a different value in case of error
*/	
UINT32 OSW_EventDelete
	(
	Event*		pEvent
	);



/*!
	Set an event.

	If the event is set when a task is already waiting, the specific task shall be released.
	If the event is set when there is no task waiting for the event, the event setting shall
	be stored, and when a task that subsequently attempts to wait for the event shall be
	released with no delay.

	The API shall be callable from TASK context.
	If the specific SW porting requires (and enables) it, the API shall be also callable from
	INTERUPT context. In this case, the implementation shall be free of APIs that are not
	supported in interrupt context.
	
	\param[in]	pEvent			Pointer to the event to set

	\return						OSW_OK if OK - or a different value in case of error
*/
UINT32 OSW_EventSet
	(
	Event*		pEvent
	);


/*!
	Clear the specified event.
	The API shall be callable from TASK context.
	
	\param[in]	pEvent			Pointer to the event to clear

	\return						OSW_OK if OK - or a different value in case of error
*/	
UINT32 OSW_EventClear
	(
	Event*		pEvent
	);


/*!
	Wait for a specific event with timeout.
	If the event is already set, there is no wait. On return, the event shall be in the cleared
	state, without the need of additional calls.
	The API shall be callable from TASK context.
	
	\param[in]	pEvent			Pointer to the event to wait for
	\param[in]	timeout			The maximum time in milliseconds to wait.
								Equal to OSW_WAIT_FOREVER for unlimited wait

	\return						OSW_OK if event signalled in time, or OSW_TIMEOUT if timeout expired, or any other value in case of error
*/
UINT32 OSW_EventWait
	(
	Event*		pEvent,
	UINT32		timeout
	);

#ifdef __cplusplus
}
#endif

#endif


