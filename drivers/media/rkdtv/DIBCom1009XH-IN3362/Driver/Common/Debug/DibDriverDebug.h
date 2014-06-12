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

#include "DibDriverDebugPlatform.h"

struct DibDriverContext;

struct DibDriverDebugInstance {
   int8_t Id;

   struct DibDriverDebugPlatformInstance Platform;

   struct DibDriverContext *Context;

   void (*msgHandler)(struct DibDriverDebugInstance *, const uint32_t *data, uint32_t size);
};


#define DIB_MAX_NB_DEBUG_INSTANCES 3

struct DibDriverDebugContext
{
   DIB_LOCK         Lock;

   struct SerialBuf RxSerialBuf;
   struct SerialBuf TxSerialBuf;
   uint32_t         TxBuffer[128];

   struct DibDriverDebugInstance Instance[DIB_MAX_NB_DEBUG_INSTANCES];

   struct DibDriverDebugPlatformContext Platform;
   struct DibDriverContext *Context;
};

extern void DibDriverDebugInit(struct DibDriverContext *ctx);
extern void DibDriverDebugExit(struct DibDriverContext *ctx);

extern struct DibDriverDebugInstance * DibDriverDebugAllocateInstance(struct DibDriverContext *ctx);
extern void DibDriverDebugReleaseInstance(struct DibDriverDebugInstance *instance);

extern void DibDriverDebugInMessageDispatcher(struct DibDriverContext *ctx, const uint32_t *data, uint32_t size, int8_t target);
extern void DibDriverDebugOutMessageCollector(struct DibDriverDebugInstance *instance,
      uint32_t *data,
      const struct MsgHeader *Head);

extern int DibDriverDebugTunerEmulatorAccess(struct DibDriverDebugInstance *instance, char *b, uint32_t size);

/* to be implemented by the platform */
extern void DibDriverDebugPlatformInstanceWriteRaw(struct DibDriverDebugInstance *instance, const char *data, uint32_t size);
extern void DibDriverDebugPlatformInit(struct DibDriverContext *ctx);
extern void DibDriverDebugPlatformExit(struct DibDriverContext *ctx);
extern void DibDriverDebugPlatformInstanceExit(struct DibDriverDebugInstance *i);
