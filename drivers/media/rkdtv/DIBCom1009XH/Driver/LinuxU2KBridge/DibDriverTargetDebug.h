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
* @file "DibDriverTargetDebug.h"
* @brief Driver User to Kernel Bridge Target functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_TARGET_DEBUG_H_
#define DIB_DRIVER_TARGET_DEBUG_H_

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include <assert.h>
#include <stdio.h>

#define DibDriverTargetLogFunction printf

#define DibSetStartTime(_startTime_sec_ , _startTime_msec_ )
#define DibLogToFileTime(_file_ , _startTime_sec_ , _startTime_msec_ )
#define DibLogToFileMesg(_file_ , format... )                            fprintf(stderr, format)

extern FILE *f_dbg;

/*  #define DIB_ASSERT( foo ) { if( ! (foo) ) { fprintf(stderr, CRB "DIB_ASSERT FAILED in %s at line %d" CRA,__FILE__,__LINE__); } } */
//#define DIB_ASSERT( foo ) assert( foo );

#undef DIB_ASSERT
#define DIB_ASSERT(foo)

#endif
