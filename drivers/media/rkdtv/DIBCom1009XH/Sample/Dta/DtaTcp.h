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

//#include "../../Driver/Common/DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#ifndef __DTATCP__
#define __DTATCP__
#include <pthread.h>

struct DibDtaDebugPlatformInstance
{
  pthread_t Thread;
  int Stop;
  int IsClosing;
  
  int sock;
  
  pthread_mutex_t msgBufferLock;
  pthread_cond_t msgResponseCondition;
  int expectedId;
  struct DibDtaDebugPlatformContext *parentCtx;
  
};

struct DibDtaDebugPlatformContext
{
  pthread_t Thread;
  int Stop;
  
  /* Server sockets */
  int sock;
  struct DibDtaDebugPlatformInstance DtaTcpInstance;
  struct DibDriverContext *DibDrvCtx;
  struct DibDriverDebugPlatformInstance *DibDebugInstance;
  int curChipId;
  int curStream;
};

void DtaTcpInit(struct DibDtaDebugPlatformContext *DtaTcpCtx,struct DibDriverContext *ctx);

#endif
