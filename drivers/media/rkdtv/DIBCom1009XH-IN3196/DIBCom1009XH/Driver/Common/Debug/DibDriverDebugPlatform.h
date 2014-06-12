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

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include <pthread.h>

struct DibDriverDebugPlatformInstance
{
   pthread_t Thread;
   int Stop;
   int IsClosing;
   int chipId;
   int Fd;

   pthread_mutex_t msgBufferLock;
   pthread_cond_t msgResponseCondition;
   uint32_t Buffer[128];
   struct SerialBuf SerialBuf;
   int expectedId;
};

struct DibDriverDebugPlatformContext
{
   pthread_t Thread;
   int Stop;
   int Active;

   /* Server sockets */
   int Fd;
   int FdEmu;
   int FdLor;
};

struct DibDriverDebugInstance;
extern void DibDriverDebugTunerEmulatorAccessMsgResponse(struct DibDriverDebugInstance *i, const uint32_t *data, uint32_t size);
