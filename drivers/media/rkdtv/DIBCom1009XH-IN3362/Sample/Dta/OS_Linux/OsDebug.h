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

/*///////////////////
// LINUX SPECIFIC  //
///////////////////*/

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

/* #define XCONSOLE_LOG */
#ifdef XCONSOLE_LOG
extern FILE *f_xc;

#define LOG( format... )  do { fprintf(f_xc, format); fflush(f_xc); } while(0)

#define LOGT( format... ) do { struct timeval __tv;                                                      \
                               struct tm __t;                                                            \
                               gettimeofday(&__tv, NULL);                                                \
                               localtime_r((time_t*)(&__tv.tv_sec), (struct tm*)(&__t));                 \
                               fprintf(f_xc, CRB "" CRA);                                                \
                               fprintf(f_xc, "%02d:%02d:%02d ", __t.tm_hour, __t.tm_min, __t.tm_sec);    \
                               fprintf(f_xc, format);                                                    \
                               fflush(f_xc);                                                             \
                             } while(0)


#else

#define LOG( format... )  do {} while(0)
#define LOGT( format... ) do {} while(0)

#endif /* XCONSOLE_LOG */

#if 1 // test by dckim
#undef DIB_ASSERT
#define DIB_ASSERT(foo)
#else
#define DIB_ASSERT( foo ) { if( ! (foo) ) { printf("DIB_ASSERT FAILED in %s at line %d",__FILE__,__LINE__); } }
#endif

#endif
