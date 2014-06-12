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

/**************************************************************************************************
* @file "DtaTime.c"
* @brief DiBcom Test Application - Timing function.
*
***************************************************************************************************/
#define _XOPEN_SOURCE 500

#include <stdio.h>
#ifndef WINCE
#include <time.h>
#endif

#include "DibConfig.h"
#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaTime.h"

FILE  *FileRealTime = NULL;
struct OsTimeval_t TimeValRef;

/**
* DtaTimeLogInit
*/
void DtaTimeLogInit(void)
{
   FileRealTime = stdout;
}

/**
* DtaRealTimeLogInit
*/
void DtaRealTimeLogInit(void)
{
   FileRealTime = fopen("rt.log", "a");

   if(!FileRealTime) 
   {
      printf(CRB "%s: *** Error: Cannot open file rt.log" CRA, __FUNCTION__);
      FileRealTime = stdout;
      return;
   }
   DtaTimeLogDate(FileRealTime, __FUNCTION__);
}

/**
 * DtaRealTimeLogExit
 */
void DtaRealTimeLogExit(void)
{
   if(!FileRealTime) 
   {
      printf(CRB "%s: *** Error: Null file handler" CRA, __FUNCTION__);
      FileRealTime = stdout;
      return;
   }

   DtaTimeLogDate(FileRealTime, __FUNCTION__);
   
   fclose(FileRealTime);
   FileRealTime = stdout;
}

/**
 * DtaRealTimeSet
 */
void DtaRealTimeSet(void)
{
   OsGetTime(&TimeValRef);
}

/**
 * DtaRealTimeClear
 */
void DtaRealTimeClear(void)
{
   timerclear(&TimeValRef);
}

/**
 * DtaRealTimeDiffMs
 */
int32_t DtaRealTimeDiffMs(void)
{
   struct OsTimeval_t tv;

   if(!timerisset(&TimeValRef))
      return -1;

   OsGetTime(&tv);
   timersub(&tv, &TimeValRef, &tv);
   timerclear(&TimeValRef);

   return (int32_t) timermsec(&tv);
}

/**
 * DtaRealTimeLogDiff
 */
void DtaRealTimeLogDiff(const char * string)
{
   if(!FileRealTime) 
   {
      printf(CRB "%s: Error: Null file handler" CRA, __FUNCTION__);
      FileRealTime = stdout;
      return;
   }

   fprintf(FileRealTime, "%s %d ms", string, DtaRealTimeDiffMs());
   fflush(FileRealTime);
}

/**
 * DtaTimeLogDate
 */
#ifndef WINCE
void DtaTimeLogDate(FILE * fdlog, const char * string)
{
   struct OsTimeval_t tv;

   OsGetTime(&tv);
   fprintf(fdlog, "%s %s", string, ctime((const time_t *) (&tv.tv_sec)));
   fflush(fdlog);
}
#else
void DtaTimeLogDate(FILE * fdlog, const char * string)
{
	wchar_t lpDateStr[20];
	wchar_t lpTimeStr[10];
	char DateStr[20];
	char TimeStr[10];
	SYSTEMTIME wSystemTime;

   GetSystemTime(&wSystemTime);

   // Get Date
   GetDateFormatW(LOCALE_SYSTEM_DEFAULT, 0, &wSystemTime, L"ddd','MMM dd yyyy", 
	   lpDateStr, 20);
   WideCharToMultiByte(CP_ACP, 0, lpDateStr,  -1, DateStr, 20, 0, 0);

   // Get Time
   GetTimeFormatW(LOCALE_SYSTEM_DEFAULT, 0, &wSystemTime, L"HH':'mm':'ss", 
	   lpTimeStr, 10);
   WideCharToMultiByte(CP_ACP, 0, lpTimeStr,  -1, TimeStr, 10, 0, 0);
   fprintf(fdlog, "%s %s %s\n", string, DateStr, TimeStr);
   fflush(fdlog);
}
#endif
