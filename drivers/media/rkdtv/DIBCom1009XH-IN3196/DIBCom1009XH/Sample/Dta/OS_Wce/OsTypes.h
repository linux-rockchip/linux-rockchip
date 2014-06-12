/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
// File Name   : OSTypes.h                                                              //
//                                                                                      //
// Description : OS independent threads/events creation & synchro                       //
//                                                                                      //
// Copyright (C) 2008 DiBcom SA                                                         //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef OS_TYPES_H
#define OS_TYPES_H

////////////////////////
// WINDOWS SPECIFIC  //
////////////////////////
#include <windows.h>

#include "DibExtDefines.h"
#include "OsDebug.h"

void OsDtaGetLogFileName(char *name, const char *string, uint8_t batch_mode, FILE * fb);

/*
 * Wait primitives 
 */
#define OsMSleep			Sleep



/*
 * Random number primitives 
 */
#define OsRandom			rand



/*
 * Time primitives 
 */
struct OsTimeval_t
{
	DWORD tv_sec;	/* seconds since January 1th 1601 */
	DWORD tv_usec;	/* ellapsed micro-seconds of the current second */
};

extern int32_t OsGetTime(struct OsTimeval_t *timeval);

/* 
 * Thread primitives 
 */
#define THREAD_HANDLE	HANDLE
#define PTHREAD_FN      void *

extern int32_t OsCreateThread(THREAD_HANDLE *HandlePtr, void *StartRoutinePtr, void *ContextPtr);
extern int32_t OsJoinThread(THREAD_HANDLE Handle);

#define OsExitThread(_EXIT_CODE_)	ExitThread(_EXIT_CODE_)



/*
 * Miscelaneous primitives
 */
#define OsClearScreen()		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n")



#endif	/* OS_TYPES_H */
