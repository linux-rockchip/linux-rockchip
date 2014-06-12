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

/*////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
// File Name   : OSTypes.h                                                              //
//                                                                                      //
// Description : OS independent threads/events creation & synchro                       //
//                                                                                      //
// Copyright (C) 2008 DiBcom SA                                                         //
//                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////*/
#ifndef OS_TYPES_H
#define OS_TYPES_H

/*///////////////////
// LINUX SPECIFIC  //
///////////////////*/
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "DibExtDefines.h"


/*
 * Wait primitives
 */
/**
 * OsMSleep (uint32_t ms)
 */
static __inline void OsMSleep(uint32_t timeout_ms)
{
   struct timeval tv;
   tv.tv_sec = timeout_ms / 1000;
   tv.tv_usec = (timeout_ms % 1000) * 1000;
   select(0, NULL, NULL, NULL, &tv);
}

/*
 * Random number primitives
 */
#define OsRandom			random


/*
 * Time primitives
 */
#define OsTimeval_t				timeval
#define OsGetTime(_PVAL_REF_)	gettimeofday(_PVAL_REF_, NULL)

/*
 * Thread primitives
 */
#define THREAD_HANDLE	pthread_t
#define PTHREAD_FN      void *

#define OsCreateThread(_PHANDLE_, _PFUNc_, _PCONTEXT_)	pthread_create(_PHANDLE_, NULL, (void* (*)(void*))_PFUNc_, _PCONTEXT_)
#define OsJoinThread(_HANDLE_)							pthread_join(_HANDLE_,NULL)
#define OsExitThread(_EXIT_CODE_)						pthread_exit(NULL)

/*
 * Miscelaneous primitives
 */
#if 0
#define OsClearScreen() do { } while (0)
#else
#define OsClearScreen()		printf("\E[H\E[2J")
#endif

#endif	/* OS_TYPES_H */
