/*!  

	\file		SmsLitePlatDefs.h
	
	\brief		Platform dependent types and definitions
	
	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  
*/


#ifndef SMS_PLAT_DEFS_H
#define SMS_PLAT_DEFS_H

#ifdef __cplusplus
extern "C"{
#endif

/*********************/
/* Platform Includes */
/*********************/
#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//#include <stdio.h>
//#include <assert.h>
//#include <time.h>
//#include <string.h>
//#include <memory.h>
//#include <stdlib.h>
//#include <math.h>
//#include "SmsPlatDefs.h"
#include <linux/string.h>
#include <linux/types.h>

#ifndef UINT8
typedef uint8_t UINT8;
#endif
#ifndef INT8
typedef int8_t INT8;
#endif
#ifndef UINT16
typedef uint16_t UINT16;
#endif
#ifndef INT16
typedef int16_t INT16;
#endif
#ifndef UINT32
typedef uint32_t UINT32;
#endif
#ifndef INT32
typedef int32_t INT32;
#endif
#ifndef UINT64
typedef uint64_t UINT64;
#endif
#ifndef INT64
typedef int64_t INT64;
#endif
#ifndef BOOL
typedef bool BOOL;
#endif
#ifndef BOOLEAN
typedef bool BOOLEAN;
#endif
#ifndef LONG
typedef int32_t LONG;
#endif
//typedef uint8_t u8;

#ifndef FALSE
#define FALSE false
#endif
#ifndef TRUE
#define TRUE true
#endif

#define IN
#define OUT

#define SMS_ASSERT(x)		//assert(x)

//#define ARGUSE(x)

typedef enum
{
	OSW_TASK_PRI_LOW	= 0,
	OSW_TASK_PRI_NORMAL	= 1,
	OSW_TASK_PRI_HIGH	= 2
} OSW_TASK_PRI;

// Tasks stack size
#define OSW_SMSHOSTLIBTASK_STACK					( 0x800 )
#define OSW_SMSHOSTMONITORTASK_STACK				( 0x800 )
#define OSW_SMSHOSTLIBTASK_SRVM_STACK				( 0x800 )
#define OSW_SMSHOSTLIBTASK_SCAN_STACK				( 0x800 )
#define OSW_SMSHOSTLIB_WAKEUP_TASK_STACK			( 0x800 )
#define OSW_SMSHOSTLIB_ICALL_TASK_STACK				( 0x800 )


#define OSW_SMSHOSTLIBTASK_PRI						OSW_TASK_PRI_NORMAL
#define OSW_SMSHOSTMONITORTASK_PRI					OSW_TASK_PRI_NORMAL
#define OSW_SMSHOSTLIB_ICALL_TASK_PRI				OSW_TASK_PRI_NORMAL
#define OSW_SMSHOSTLIBTASK_SCAN_PRI					OSW_TASK_PRI_NORMAL
#define OSW_SMSHOSTLIB_WAKEUP_TASK_PRI				OSW_TASK_PRI_NORMAL

//#define MAX_NUM_MESSAGES_IN_Q	20

//cover up for windows specific garbage in the code

#define INFINITE 0

#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT  1
#define WAIT_FAILED   2

	//! error codes for the os abstraction layer
#define PTHREAD_MUTEX_ERROR 1000
#define PTHREAD_COND_VAR_ERROR 2000
#define PTHREAD_ERROR 3000
#define PTHREAD_ATTR_ERROR 4000

/**********/
/* Types */
/**********/


/**********************************/
/* Platform OSW types and defines */
/**********************************/

// Task type
typedef void* OSW_TaskId;

// Mutex type
//typedef pthread_mutex_t Mutex;	//!< linux mutex definition
typedef int Mutex;

// File type
typedef void* OSW_FILEHANDLE;

//! the linux event representation
typedef struct
{
	int set;
	//pthread_cond_t cond;
	//pthread_mutex_t mutex;
	int cond;
	int mutex;
} Event;

// Timer types
/*! prototype for the timer's callback function
\param[in]	lpArgToCompletionRoutine: opaque data supplied at timer creation
\param[in] 	dwTimerLowValue: timer low value
\param[in] 	dwTimerHighValue: timer high value
*/
typedef void (*TimerCBFunc) (
			  void *lpArgToCompletionRoutine,
			  UINT32 dwTimerLowValue,
			  UINT32 dwTimerHighValue );

/**************/
/* Debug Logs */
/**************/
#ifdef SMSHOST_ENABLE_LOGS

#define SMSHOST_FILE		__FILE__
#define SMSHOST_LINE		__LINE__
#define SMSHOST_FUNCTION	__FUNCTION__


//extern void SmsLinuxLog(UINT32 mask,char* log_str);

#define USER_PRINTF(mask,log_str) \
	//SmsLinuxLog(mask,log_str)

#define SMSHOST_DEFAULT_LOG_MASK (0xFFFFFFFF)

#endif




/*************************************************************************
*			 Macros
*************************************************************************/
#ifdef _DEBUG
	#define SMS_ASSERT(x)   assert(x)
#else
	#define SMS_ASSERT(x)
#endif // DEBUG

#define ARGUSE(x) (x);
#define MAX_DIR_NAME_SIZE	256

// Lite OSW
#define OswLiteMemAlloc 	OSW_MemAlloc
#define OswLiteMemFree		OSW_MemFree
//#define OswLiteTaskSleep	Sleep
#define OswLiteTaskSleep	OSW_TaskSleep

// Logs
//#define SMSHOST_ENABLE_LOGS
#define SMSHOST_LINE		__LINE__
#define SMSHOST_FUNCTION	__FUNCTION__

//extern void SmsWin32Log(UINT32 mask,char* log_str);
void SmsWin32Log(UINT32 mask,char* log_str);

#define USER_PRINTF(mask,log_str) \
	SmsWin32Log(mask,log_str)

#ifdef __cplusplus
}
#endif

#endif  /* SMS_PLAT_DEFS_H */
