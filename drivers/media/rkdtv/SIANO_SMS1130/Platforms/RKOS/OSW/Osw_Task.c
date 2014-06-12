#include "osw.h"


OSW_TaskId OSW_TaskCreate
	(
	const char* TaskName,
	UINT32      TaskPriority,
	UINT32      TaskStackSize,
	TaskFunc    TaskFunction,
	void*       TaskFunctionParams
	)
	{
#if 0	// peter
	DWORD		winTaskId;
	
	ARGUSE (TaskPriority);
	ARGUSE (TaskName);

	return CreateThread
		(
		NULL,											// No security attributes
		TaskStackSize,									// Default stack size
		(LPTHREAD_START_ROUTINE)TaskFunction,			// Task entry function
		TaskFunctionParams,								// No task function arguments
		0,												// Create active
		&winTaskId										// Return task identifier
		);	

#else
    //OSW_TaskId		winTaskId = 0xffffffff;
    //return winTaskId;
    return NULL;
#endif
} /* OSW_TaskCreate */



void  OSW_TaskCleanup   (OSW_TaskId TaskId)
{
#if 0	// peter
	CloseHandle (TaskId);
#endif
}


void  OSW_TaskSleep  (UINT32 TaskSleepPeriod)//pend for delay
{
#if 0   // peter
	Sleep(TaskSleepPeriod);
#else
    DelayMs_nops(TaskSleepPeriod);
#endif
}

OSW_TaskId OSW_TaskGetCurrent (void)
{
#if 0	// peter
	DWORD ThreadId = GetCurrentThreadId() ;
	return ( CAST_NOWARN(OSW_TaskId)ThreadId );
#else
	//OSW_TaskId		winTaskId = 0xffffffff;
    //   return winTaskId;
    return NULL;
#endif
}


