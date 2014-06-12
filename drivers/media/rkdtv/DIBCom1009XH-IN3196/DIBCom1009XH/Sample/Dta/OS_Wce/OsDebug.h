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
* @file "DtaDebug.h"
* @brief DiBcom Test Application - Debug Functions Prototypes.
*
***************************************************************************************************/
#ifndef OS_DEBUG_H
#define OS_DEBUG_H

////////////////////////
// WINDOWS SPECIFIC  //
////////////////////////

extern void TraceNothing(char *format, ...);
extern void TraceLog(char *format, ...);
extern void TraceLogTime(char *format, ...);

/* #define XCONSOLE_LOG */
#ifdef XCONSOLE_LOG
	extern FILE *f_xc;

#	define LOG		TraceLog
#	define LOGT	TraceLogTime

#else

#	define LOG	TraceNothing
#	define LOGT	TraceNothing

#endif /* XCONSOLE_LOG */

#define ASSERT( foo ) { if( ! (foo) ) { printf("ASSERT FAILED in %s at line %d",__FILE__,__LINE__); } }

#endif
