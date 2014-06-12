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
* @file "DtaDebug.c"
* @brief DiBcom Test Application - Debug Functions Prototypes.
*
***************************************************************************************************/
////////////////////////
// WINDOWS SPECIFIC  //
////////////////////////
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "OsDebug.h"

#ifdef XCONSOLE_LOG

void TraceLog(char *format, ...)
{
	va_list	arg_list;


	va_start(arg_list, format);
	fprintf(f_xc, format, arg_list);
	va_end(arg_list);

	fflush(f_xc);
}

void TraceLogTime(char *format, ...)
{
	va_list		arg_list;
	time_t		current_time;
	struct tm	*local_time;


	time(&current_time);
	local_time = localtime(&current_time);
	fprintf(f_xc, "\n%02d:%02d:%02d ", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

	va_start(arg_list, format);
	fprintf(f_xc, format, arg_list);
	va_end(arg_list);

	fflush(f_xc);
}

#else

void TraceNothing(char *format, ...)
{
}

#endif	/* XCONSOLE_LOG */
