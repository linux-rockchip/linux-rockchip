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

////////////////////////
// WINDOWS SPECIFIC  //
////////////////////////
#include "OsTypes.h"

extern char logfilepath[30];
/*
 * Wait primitives 
 */

/*
 * Random number primitives 
 */

/*
 * Time primitives 
 */
int32_t OsGetTime(struct OsTimeval_t *timeval)
{
	SYSTEMTIME		wSystemTime;
	FILETIME		wFileTime;
	ULARGE_INTEGER	wTotalNSeconds;


	GetSystemTime(&wSystemTime);
	if (! SystemTimeToFileTime(&wSystemTime, &wFileTime))
	{
		wTotalNSeconds.HighPart	= wFileTime.dwHighDateTime;
		wTotalNSeconds.LowPart	= wFileTime.dwLowDateTime;

		timeval->tv_sec	= (DWORD)(wTotalNSeconds.QuadPart / 10000000);
		timeval->tv_usec= (DWORD)((wTotalNSeconds.QuadPart % 10000000) / 10);

		return 0L;
	}
	else
		return -1L;
}

/* 
 * Thread primitives 
 */
int32_t OsCreateThread(THREAD_HANDLE *HandlePtr, PTHREAD_FN StartRoutinePtr, void *ContextPtr)
{
	*HandlePtr = CreateThread(NULL, 0L, (LPTHREAD_START_ROUTINE)StartRoutinePtr, ContextPtr, 0L, NULL);

	return ((*HandlePtr) != NULL) ? 0L : -1L;
}


int32_t OsJoinThread(THREAD_HANDLE Handle)
{
	DWORD ulWaitReturn;

	ulWaitReturn = WaitForSingleObject(Handle, INFINITE);
	CloseHandle(Handle);

	return (ulWaitReturn == WAIT_OBJECT_0) ? 0L : -1L;
}

void OsDtaGetLogFileName(char *name, const char *string, uint8_t batch_mode, FILE * fb)
{
   char  fname[30];
   int32_t rc;

   if(batch_mode == 0)
   {
      /* Prompt Mode. */
      printf("%s: ", string);
      rc = scanf("%s", name);
   }
   else
   {
      /* Batch Mode: put the log file in the same folder as the batch file */
	  strcpy(name, logfilepath);
	  fscanf(fb, "%s", fname);
	  strcat(name, fname);
   }

   LOG("%s ", name);
}
