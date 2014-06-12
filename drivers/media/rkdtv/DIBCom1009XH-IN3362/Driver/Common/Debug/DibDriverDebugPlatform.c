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
#include "DibMsgGeneric.h"
#include "DibMsgDebug.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <byteswap.h>

#include <sys/select.h>

#define dbgp(args...) do { } while(0) /*fprintf(stderr, args)*/
#define max(a,b) ((a)>(b)?(a):(b))

#if DIB_INTERNAL_MONITORING == 1

static void * EmuConnection(void *arg)
{
   struct DibDriverDebugInstance *instance = arg;
   struct DibDriverDebugPlatformInstance *p = &instance->Platform;
   char b[1024];

   while (!p->Stop) {
      fd_set rfds;
      int retval;

      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 50000;

      FD_ZERO(&rfds);
      FD_SET(p->Fd, &rfds);

      retval = select(p->Fd+1, &rfds, NULL, NULL, &tv);
      switch (retval) {
      case -1:
         perror("select failed");
         p->Stop = 1;
         break;
      case 0: /* timeout */
         break;
      default: { /* data to be read */
         int len;
         len = read(p->Fd, b, 1023); /* read the header */
         if (len <= 0) { /* connection closed */
            dbgp("len is not valid (%d) - connection closed\n", len);
            goto bailout;
         }
         b[len] = '\0';
         if (!DibDriverDebugTunerEmulatorAccess(instance, b, len+1))
            goto bailout;

         break;
bailout:
         p->Stop = 1;

         break;

      }
      }
   }
   p->IsClosing = 1;

   shutdown(p->Fd, SHUT_RDWR);
   p->Stop = 0;

   dbgp("%s: DebugPlatformConnectionThread stopped (%d).\n", __func__, instance->Id);

   DibDriverDebugReleaseInstance(instance);

   return NULL;
}

#define MSG_MAX_LEN 63
static void * TcpConnection(void *arg)
{
   struct DibDriverDebugInstance *instance = arg;
   struct DibDriverDebugPlatformInstance *p = &instance->Platform;
   uint32_t Msg[MSG_MAX_LEN + 1], size;
   uint8_t *b;
   struct SerialBuf RxSerial;
   struct MsgHeader Head;
   int i;

   enum {
      HEADER,
      DATA
   } state = HEADER;

   SerialBufInit(&RxSerial, Msg, 32);

   dbgp("%s: DebugPlatformConnectionThread running (%d).\n", __func__, instance->Id);

   size = 0;

   while (!p->Stop) {
      fd_set rfds;
      int retval;

      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 50000;

      FD_ZERO(&rfds);
      FD_SET(p->Fd, &rfds);

      retval = select(p->Fd+1, &rfds, NULL, NULL, &tv);
      switch (retval) {
      case -1:
         perror("select failed");
         p->Stop = 1;
         break;
      case 0: /* timeout */
         break;
      default: { /* data to be read */
         int len;

         switch (state) {
         case HEADER:
            len = read(p->Fd, &Msg[0], 4); /* read the header */
            if (len != 4) { /* connection closed */
               dbgp("len is not 4 but %d - connection closed\n", len);
               goto bailout;
            }

            MsgHeaderUnpackInit(&RxSerial, &Head);

            //            printf("msg received: %d %d %08x\n", Head.Sender, Head.MsgSize, Msg[0]);

            if (Head.MsgSize > MSG_MAX_LEN) {
               dbgp("%s: ERROR message longer than expected %d\n", __func__, Head.MsgSize);
               goto bailout;
            }

            if (Head.MsgSize < 1) {
               dbgp("%s: ERROR message shorter than expected %d\n", __func__, Head.MsgSize);
               goto bailout;
            }

            size = Head.MsgSize*4 - 4;
            b = (uint8_t *) &Msg[1];
            state = DATA;
            /* only in case of more data we want to wait for the select */
            if (size != 0)
               break;

         case DATA:
            if (size > 0) {
               len = read(p->Fd, b, size);
               if (len == 0)
                  goto bailout;

               size -= len;
               b += len;
            }

            if (size == 0) { /* MSG is in MSG and can be forwarded */
               //               dbgp("forwarding\n");
               DibDriverDebugOutMessageCollector(instance, Msg, &Head);
               state = HEADER;
            }
            break;
         default:
            break;
         }

         break;
bailout:
         p->Stop = 1;

         break;

      }
      }

   }
   p->IsClosing = 1;

   struct DibGlobalInfo info;
   DibDriverGetGlobalInfo(instance->Context, &info);

   for(i = 0 ; i < info.NumberOfChips; i++) {
      struct MsgRegisterDisablePeriodicRead req_Periodic;
      req_Periodic.Head.MsgSize = GetWords(MsgRegisterDisablePeriodicReadBits,32);
      req_Periodic.Head.ChipId  = i;
      req_Periodic.Head.MsgId   = OUT_MSG_REGISTER_DISABLE_PERIODIC_READ;
      req_Periodic.Head.Sender  = 0;
      req_Periodic.Head.Type    = MSG_TYPE_DEBUG;
      req_Periodic.Id = REGISTER_PERIODIC_READ_ID_FAILED_OR_ALL;
      MsgRegisterDisablePeriodicReadPackInit(&req_Periodic, &RxSerial);
      DibDriverDebugOutMessageCollector(instance, Msg, &req_Periodic.Head);
   }
   shutdown(p->Fd, SHUT_RDWR);
   p->Stop = 0;

   dbgp("%s: DebugPlatformConnectionThread stopped (%d).\n", __func__, instance->Id);

   DibDriverDebugReleaseInstance(instance);

   return NULL;
}

static void DibDriverDebugPlatformInstanceForwardMessage(struct DibDriverDebugInstance *instance, const uint32_t *data, uint32_t size)
{
   if(!instance->Platform.IsClosing)
      write(instance->Platform.Fd, data, size*4);
}

void DibDriverDebugPlatformInstanceWriteRaw(struct DibDriverDebugInstance *instance, const char *data, uint32_t size)
{
   if(!instance->Platform.IsClosing)
      write(instance->Platform.Fd, data, size);
}

static void DibDriverDebugPlatformAccept(struct DibDriverContext *Drv, int fd,
      void * (*handler)(void*),
      void (*msg)(struct DibDriverDebugInstance *, const uint32_t *, uint32_t))
{
   int ConnectFD = accept(fd, NULL, NULL);
   struct DibDriverDebugInstance *instance;
   if(ConnectFD < 0) {
      perror("error accept failed");
      return;
   }
   instance = DibDriverDebugAllocateInstance(Drv);
   if (instance == NULL) {
      shutdown(ConnectFD, SHUT_RDWR);
      dbgp("%s: ERROR: could not allocate instance for connection\n", __func__);
   } else {
      dbgp("%s: starting Connection for DebugInstance\n", __func__);

      instance->Platform.Fd = ConnectFD;
      instance->Platform.IsClosing = 0;
      instance->msgHandler = msg;

      pthread_mutex_init(&instance->Platform.msgBufferLock, NULL);
      pthread_cond_init(&instance->Platform.msgResponseCondition, NULL);
      SerialBufInit(&instance->Platform.SerialBuf, instance->Platform.Buffer, 32);

      pthread_create(&instance->Platform.Thread, NULL, handler, instance);
   }
}

/* Creates the server and listens for incoming connections */
static void * TcpServer(void *arg)
{
   int i;
   struct DibDriverContext *Drv = arg;
   struct DibDriverDebugPlatformContext *p = &Drv->DebugCtx.Platform;

   dbgp("%s: DebugPlatformThread running.\n", __func__);

   /* wait for Board to ready */
   while (Drv->Open != eDIB_TRUE && !p->Stop)
      DibMSleep(100);

   while (!p->Stop) {
      fd_set rfds;
      int retval;

      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 50000;

      FD_ZERO(&rfds);
      FD_SET(p->Fd, &rfds);
      FD_SET(p->FdEmu, &rfds);
      FD_SET(p->FdLor, &rfds);

      retval = select(max(p->Fd, p->FdLor)+1, &rfds, NULL, NULL, &tv);
      switch (retval) {
      case -1:
         perror("select failed");
         p->Stop = 1;
         break;
      case 0: /* timeout */
         break;
      default: {
         if (FD_ISSET(p->Fd, &rfds))
             DibDriverDebugPlatformAccept(Drv, p->Fd, TcpConnection, DibDriverDebugPlatformInstanceForwardMessage);
         if (FD_ISSET(p->FdEmu, &rfds))
             DibDriverDebugPlatformAccept(Drv, p->FdEmu, EmuConnection, DibDriverDebugTunerEmulatorAccessMsgResponse);
         if (FD_ISSET(p->FdLor, &rfds))
             DibDriverDebugPlatformAccept(Drv, p->FdLor, EmuConnection, DibDriverDebugTunerEmulatorAccessMsgResponse);
         break;
      }
      }
   }

   /* close all client connections */
   for (i = 0; i < DIB_MAX_NB_DEBUG_INSTANCES; i++) {
      if (Drv->DebugCtx.Instance[i].Context) {
         Drv->DebugCtx.Instance[i].Platform.Stop = 1;
         pthread_join(Drv->DebugCtx.Instance[i].Platform.Thread, NULL);

         DibDriverDebugReleaseInstance(&Drv->DebugCtx.Instance[i]);
      }
   }

   p->Stop = 0;

   return NULL;
}

void DibDriverDebugPlatformInstanceExit(struct DibDriverDebugInstance *i)
{
   pthread_mutex_destroy(&i->Platform.msgBufferLock);
   pthread_cond_destroy(&i->Platform.msgResponseCondition);
}

void DibDriverDebugPlatformInit(struct DibDriverContext *ctx)
{
   struct sockaddr_in stSockAddr;
   struct sockaddr_in stSockAddrLor;
   struct sockaddr_in stSockAddrEmu;
   struct DibDriverDebugPlatformContext *p = &ctx->DebugCtx.Platform;
   int reuse_addr = 1;
   int32_t BoardId;

   /* Socket for DiBug */
   p->Fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(-1 == p->Fd) {
      perror("can not create socket");
      goto out;
   }

   setsockopt(p->Fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

   memset(&stSockAddr, 0, sizeof stSockAddr);
   stSockAddr.sin_family = AF_INET;

   if(ctx->BoardHdl)
      BoardId = *((int32_t *)ctx->BoardHdl);
   else
      BoardId = 0;

   stSockAddr.sin_port = htons(25000 + 10 * BoardId);
   stSockAddr.sin_addr.s_addr = INADDR_ANY;

   if(-1 == bind(p->Fd, (struct sockaddr *)&stSockAddr, sizeof stSockAddr)) {
      perror("error bind failed");
      goto bind_error;
   }

   if(-1 == listen(p->Fd, 10)) {
      perror("error listen failed");
      goto listen_error;
   }

   /* Socket for Emu */
   p->FdEmu = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(-1 == p->FdEmu) {
      perror("can not create socket");
      goto bind_error;
   }

   setsockopt(p->FdEmu, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

   memset(&stSockAddrEmu, 0, sizeof stSockAddrEmu);
   stSockAddrEmu.sin_family = AF_INET;
   stSockAddrEmu.sin_port = htons(7001);  /* PORT to be determined by running instance */
   stSockAddrEmu.sin_addr.s_addr = INADDR_ANY;

   if(-1 == bind(p->FdEmu, (struct sockaddr *)&stSockAddrEmu, sizeof stSockAddrEmu)) {
      perror("error bind failed EMu");
      goto bind_error;
   }

   if(-1 == listen(p->FdEmu, 10)) {
      perror("error listen failed");
      goto listen_error;
   }

    /* Socket for Lor */
    p->FdLor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == p->FdLor) {
        perror("can not create socket");
        goto bind_error;
    }

    setsockopt(p->FdLor, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    memset(&stSockAddrLor, 0, sizeof stSockAddrLor);
    stSockAddrLor.sin_family = AF_INET;
    stSockAddrLor.sin_port = htons(6002);  /* PORT to be determined by running instance */
    stSockAddrLor.sin_addr.s_addr = INADDR_ANY;

    if(-1 == bind(p->FdLor, (struct sockaddr *)&stSockAddrLor, sizeof stSockAddrLor)) {
        perror("error bind failed Lor");
        goto bind_error;
    }

    if(-1 == listen(p->FdLor, 10)) {
        perror("error listen failed");
        goto listen_error;
    }

    ctx->DebugCtx.Platform.Active = 1;

   pthread_create(&ctx->DebugCtx.Platform.Thread, NULL, TcpServer, ctx);

   goto out;

listen_error:
bind_error:
   close(p->Fd);
out:
   return;
}

void DibDriverDebugPlatformExit(struct DibDriverContext *ctx)
{
   if (ctx->DebugCtx.Platform.Active) {
   close(ctx->DebugCtx.Platform.Fd);
   close(ctx->DebugCtx.Platform.FdEmu);
   ctx->DebugCtx.Platform.Stop = 1;

   pthread_join(ctx->DebugCtx.Platform.Thread, NULL);
   dbgp("%s: DebugPlatformThread stopped.\n", __func__);
   }
   ctx->DebugCtx.Platform.Active = 0;
}

#endif
