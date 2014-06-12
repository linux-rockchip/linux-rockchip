/*!  

	\file		Osw_Task.h
	
	\brief		OS Wrapper - Tasks header

				The following are functions for task mechanism.

	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  

    \note 
	    -# Example of Task structure definition can be seen for Win32 OS at: \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Win/Include/Osw_TaskTarget.h </CODE> \n
        Should you need to change the above definition, please use your own OS directory: \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Linux/Include/Osw_TaskTarget.h </CODE> \n
		The TaskPriority is exactly the selected OS priority number. 
		It is warmly recommended to add a generic priority \n 
		definition envelope to make the OS wrapper an OS independent at all. 
		In the following example one should use its OS appropriate priority numbers. \n
        <CODE> typedef enum TaskPriority_E   </CODE> \n
        <CODE> {                             </CODE> \n 
		<CODE>     TASK_PRIORITY_HIGHEST	= 0, </CODE> \n
		<CODE>     TASK_PRIORITY_2 			= 1, </CODE> \n
		<CODE>     TASK_PRIORITY_3 			= 2, </CODE> \n
		<CODE>     TASK_PRIORITY_4 			= 3, </CODE> \n
		<CODE>     TASK_PRIORITY_63 		= 62,</CODE> \n
		<CODE>     TASK_PRIORITY_LOWEST 	= 63,</CODE> \n
        <CODE> } TaskPriority_ET;            </CODE> 
						 	
*/

#ifndef __OSW_TASK_H
#define __OSW_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include_internal/Include/sms_common.h"

typedef UINT32 (*TaskFunc)(UINT32);

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*! 
	Creates a task and makes it ready to run, or suspended.

    \param[in]   TaskName            A pointer to a unique task name string.
    \param[in]   TaskPriority        The task priority.
    \param[in]   TaskStackSize       The task stack size in bytes.
    \param[in]   TaskFunction        TaskFunc type task function to run.
    \param[in]   TaskFunctionParams  A pointer to an optional parameter to be passed to the task when it is activated.

	\return		                     Task ID on success, or NULL if operation failes.

*/
OSW_TaskId OSW_TaskCreate (const char*	TaskName,
					  UINT32		TaskPriority,
					  UINT32		TaskStackSize,
					  TaskFunc		TaskFunction,
					  void*			TaskFunctionParams);


/*! 
	Cleanup after task termination - if applicable (e.g. closing task handlers)

    \param[in]   TaskId            Task ID.

*/
void  OSW_TaskCleanup   (OSW_TaskId TaskId);

/*! 

	suspend the active task for the number of milliseconds
    
    \param[in]   TaskSleepPeriod     The maximum number of milliseconds to suspend the task.

*/
void  OSW_TaskSleep  (UINT32 TaskSleepPeriod);

/*! 
	Returns the current running task ID.
    
	\return							Current running task ID.

*/
OSW_TaskId OSW_TaskGetCurrent(void);


#ifdef __cplusplus
}
#endif

#endif
