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

#include "DibDriverCommon.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibMsgDebug.h"

#if DIB_INTERNAL_MONITORING == 1

void DibDriverDebugInit(struct DibDriverContext *ctx)
{
   int i;

   DibAllocateLock(&ctx->DebugCtx.Lock);
   DibInitLock(&ctx->DebugCtx.Lock);

   for (i = 0; i < DIB_MAX_NB_DEBUG_INSTANCES; i++) {
      ctx->DebugCtx.Instance[i].Id       = -2 - i;
      ctx->DebugCtx.Instance[i].Context  = NULL;
   }

   ctx->DebugCtx.Context = ctx;

   DibDriverDebugPlatformInit(ctx);
}

void DibDriverDebugExit(struct DibDriverContext *ctx)
{
   DibDriverDebugPlatformExit(ctx);
   DibDeAllocateLock(&ctx->DebugCtx.Lock);
}

struct DibDriverDebugInstance * DibDriverDebugAllocateInstance(struct DibDriverContext *ctx)
{
   int i;
   struct DibDriverDebugInstance *instance = NULL;

   DibAcquireLock(&ctx->DebugCtx.Lock);

   for (i = 0; i < DIB_MAX_NB_DEBUG_INSTANCES; i++)
      if (ctx->DebugCtx.Instance[i].Context == NULL) {
         instance = &ctx->DebugCtx.Instance[i];
         instance->Context = ctx;
         break;
      }

   DibReleaseLock(&ctx->DebugCtx.Lock);

   return instance;
}

void DibDriverDebugReleaseInstance(struct DibDriverDebugInstance *instance)
{
   struct DibDriverContext *ctx = instance->Context;

   DibAcquireLock(&ctx->DebugCtx.Lock);

   DibDriverDebugPlatformInstanceExit(instance);
   instance->Context = NULL;

   DibReleaseLock(&ctx->DebugCtx.Lock);
}

void DibDriverDebugInMessageDispatcher(struct DibDriverContext *ctx, const uint32_t *data, uint32_t size, int8_t target)
{
   int i;

   DibAcquireLock(&ctx->DebugCtx.Lock);

   for (i = 0; i < DIB_MAX_NB_DEBUG_INSTANCES; i++)
      if (ctx->DebugCtx.Instance[i].Id == target) {
         ctx->DebugCtx.Instance[i].msgHandler(&ctx->DebugCtx.Instance[i], data, size);
         break;
      }

   DibReleaseLock(&ctx->DebugCtx.Lock);
}

static void DibDriverDebugMessageHandling(struct DibDriverDebugInstance *instance,
      const uint32_t *data,
      const struct MsgHeader *Head)
{
    struct SerialBuf TxSerialBuf;
    uint32_t Buffer[64];
    SerialBufInit(&TxSerialBuf, Buffer, 32);

    if (Head->Type != MSG_TYPE_DEBUG) {
        printf("ERROR: message to be handled by SDK is not debug-typed\n");
        return;
    }

    switch(Head->MsgId) {
    case OUT_MSG_GET_SDK_INFO: {
        struct MsgSDKInfoIndication mess;

        struct DibGlobalInfo info;

        mess.Head.MsgSize = GetWords(MsgSDKInfoIndicationBits, 32);
        mess.Head.ChipId  = instance->Id;
        mess.Head.MsgId   = IN_MSG_GET_SDK_INFO_IND;
        mess.Head.Sender  = HOST_IDENT;
        mess.Head.Type    = MSG_TYPE_DEBUG;

        DibDriverGetGlobalInfo(instance->Context, &info);

        mess.SDKVer       = info.DriverVersion;
        mess.Nb           = info.NumberOfChips;

        MsgSDKInfoIndicationPackInit(&mess, &TxSerialBuf);
        instance->msgHandler(instance, Buffer, mess.Head.MsgSize);
    } break;

    default:
        printf("ERROR: message to be handled by SDK is not yet implemented (ID: %d)\n", Head->MsgId);
        break;
    }
}

void DibDriverDebugOutMessageCollector(struct DibDriverDebugInstance *instance,
      uint32_t *data,
      const struct MsgHeader *Head)
{
   if (Head->ChipId == HOST_IDENT) {
	  DibDriverDebugMessageHandling(instance, data, Head);
      return;
   }

   /* set Sender ID */
   *data &= ~(0xf << 24);
   *data |= (instance->Id & 0xf) << 24;

   DibDriverSendMessage(instance->Context, data, Head->MsgSize * 4);
}

#endif
