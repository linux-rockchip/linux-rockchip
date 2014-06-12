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
* @file "DibDriverTarget.c"
* @brief Linux User to Kernel Bridge Target functionality.
*
***************************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverTargetTypes.h"
#include "DibDriverCommon.h"
#include "DibDriverTargetDebug.h"
#include "DibDriver.h"
#include "DibDriverMessages.h"
#include "DibDriverPacketDemux.h"
#include "DibDriverData.h"

void *IntDriverTargetIrqRead(void *arg);

/* This file makes a link between the high level and the low level driver */

/****************************************************************************
 *  Init the bridge from the driver
 ****************************************************************************/
DIBSTATUS DibDriverTargetInit(struct DibDriverContext *pContext)
{
   struct IoctlInit pReq;

   if((pContext->DriverTargetCtx.BridgeFd = open(DEVICE_FILE_NAME, O_RDWR)) < 0) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "Could not open device %s" CRA, DEVICE_FILE_NAME));
      return DIBSTATUS_ERROR;
   }

   /* this will create and initialise bridge */
   pReq.BoardHdl   = pContext->BoardHdl;
   pReq.ChipSelect = pContext->DibChip;

#if (USE_DRAGONFLY == 1)
   if((pContext->DibChip == DIB_VOYAGER) || (pContext->DibChip == DIB_NAUTILUS))
   {
     pReq.Config[0]  = pContext->DragonflyRegisters.JedecAddr;
     pReq.Config[1]  = pContext->DragonflyRegisters.JedecValue;
     pReq.Config[2]  = pContext->DragonflyRegisters.MacMbxSize;
     pReq.Config[3]  = pContext->DragonflyRegisters.MacMbxStart;
     pReq.Config[4]  = pContext->DragonflyRegisters.MacMbxEnd;
     pReq.Config[5]  = pContext->DragonflyRegisters.HostMbxSize;
     pReq.Config[6]  = pContext->DragonflyRegisters.HostMbxStart;
     pReq.Config[7]  = pContext->DragonflyRegisters.HostMbxEnd;
     pReq.Config[8]  = pContext->DragonflyRegisters.HostMbxRdPtrReg;
     pReq.Config[9]  = pContext->DragonflyRegisters.HostMbxWrPtrReg;
     pReq.Config[10] = pContext->DragonflyRegisters.MacMbxRdPtrReg;
     pReq.Config[11] = pContext->DragonflyRegisters.MacMbxWrPtrReg;
   }
   else
#endif
   DibZeroMemory(pReq.Config, sizeof(pReq.Config));

   if(ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_BRIDGE_MODULE_INIT, &pReq) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(PORT_ERR, (CRB "Could initialise Bridge %s" CRA, DEVICE_FILE_NAME));

      /* close open device and free allocated memory*/
      close(pContext->DriverTargetCtx.BridgeFd);
      return DIBSTATUS_ERROR;
   }

   DibAllocateEvent(&pContext->DriverTargetCtx.IrqReadDone);
   DibInitNotificationEvent(&pContext->DriverTargetCtx.IrqReadDone);

   DibAllocateLock(&pContext->DriverTargetCtx.IrqLock);
   DibInitLock(&pContext->DriverTargetCtx.IrqLock);

   DIB_DEBUG(PORT_LOG, (CRB "Launching interrupt thread" CRA));

   pthread_create(&pContext->DriverTargetCtx.IrqThread, NULL, IntDriverTargetIrqRead, (void *)pContext);

   DibWaitForEvent(&pContext->DriverTargetCtx.IrqReadDone, 0);
   return DIBSTATUS_SUCCESS;
}

/*******************************************************************************
 *  Inform bridge that init is completed
 ******************************************************************************/
DIBSTATUS DibD2BPostInit(struct DibDriverContext * pContext, uint32_t flags)
{
   /** ioctl return a DIBSTATUS for REQUEST_BRIDGE_POSTINIT */
   return ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_BRIDGE_POSTINIT, flags);
}

/****************************************************************************
 * DeInit the bridge from the driver
 ****************************************************************************/
DIBSTATUS DibDriverTargetDeinit(struct DibDriverContext * pContext)
{
   /* this will destroy bridge thread and then destroy driver thread and trigger IrqRead event */
   ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_BRIDGE_MODULE_SHUTDOWN, NULL);

   DibWaitForEvent(&pContext->DriverTargetCtx.IrqReadDone, 0);

   /** Free thread internal information */
   pthread_join(pContext->DriverTargetCtx.IrqThread, NULL);

   close(pContext->DriverTargetCtx.BridgeFd);

   DIB_DEBUG(PORT_LOG, (CRB "Stopped reading thread" CRA));

   DibDeAllocateEvent(&pContext->DriverTargetCtx.IrqReadDone);
   DibDeAllocateLock(&pContext->DriverTargetCtx.IrqLock);

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Receive msg from the bridge
****************************************************************************/
void *IntDriverTargetIrqRead(void *arg)
{
    /* Dirty change to fix alignment issue */
    /* Previously msg was aligned on a 16 bit boundary and then used as a 32 bit
     * pointer, so 32 bits access => alignment issue
     * Now msg is aligned on 32 bit boundary */

   uint8_t  *msg;
   uint16_t *Type;
   uint8_t  *read_buffer2 = DibMemAlloc(4096+sizeof(*Type));
   uint8_t  *read_buffer = &read_buffer2[sizeof(*Type)];

   struct DibDriverDataInfoUp dataUp;
   struct DibDriverEventInfoUp EventUp;
   struct DibDriverContext   *pContext = (struct DibDriverContext *) arg;

   /* Signal task started */
   DibSetEvent(&pContext->DriverTargetCtx.IrqReadDone);

   while(1) 
   {
      /* read on the char device for SIPSI/MSG/TS */
      ssize_t bytes_read = ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_READ_MSG, read_buffer);

      /* Extract the Buffer Type (sipsi/msg/ts and get the msg */
      Type        = (uint16_t *) read_buffer;
      msg         = read_buffer + sizeof(*Type);
      bytes_read -= sizeof(*Type);

      /* Process the msg */
      DIB_DEBUG(PORT_LOG, (CRB "Low Speed IRQ: %d, Size= %d" CRA, msg[0], (uint32_t)bytes_read));

      if(bytes_read > 0) 
      {
         /** acquired driver lock */
         DibDriverTargetDisableIrqProcessing(pContext);

         switch (*Type) 
         {
         case UPMESSAGE_TYPE_MSG:
            DibDriverProcessMessage(pContext, bytes_read, (uint16_t *) msg);
            break;

         case UPMESSAGE_TYPE_SIPSI:
            DibDriverProcessSiPsiBuffer(pContext, bytes_read, msg);
            break;

         case UPMESSAGE_TYPE_DATA:
            DibMoveMemory((char *)&dataUp, msg, sizeof(struct DibDriverDataInfoUp));
            DIB_DEBUG(PACKET_LOG, (CRB "ioctl bytes_read %d Size : %d item : %d" CRA, (int32_t)bytes_read, dataUp.Length, dataUp.ItemIndex));
            DibDriverRawDataRead(pContext, dataUp);
            break;

         case UPMESSAGE_TYPE_EVENT:
            DibMoveMemory((char *)&EventUp, msg, sizeof(struct DibDriverEventInfoUp));
            DIB_DEBUG(CALLBACK_ERR, (CRB "Event up %d Filter : %d" CRA, EventUp.EventType, EventUp.EventConfig.DataLostEvent.ItemHdl));
            DibDriverPostEvent(pContext, &EventUp);
            break;
            
#if (DEMOD_TRACE == 1)
         case UPMESSAGE_TYPE_DTRACE:
            {
               /* Insert your DTRACE logger here */
               uint16_t y;
               uint16_t *rb;

               y  = 0;
               rb = (uint16_t *)msg;

               fprintf(stderr, "\E[H\E[2J");
               while(y < (DTRACE_BUFF_SIZE / 2)) 
               {
                  if(!(y & 0x07)) 
                     fprintf(stderr, "%04X: ", y);

                  y++;
                  fprintf(stderr, CRB "%04X%s", *rb, (y & 0x07) ? " " : "" CRA);
                  rb++;
               }

               if(y & 0x07) 
                  fprintf(stderr, CRB "" CRA);
            }
            break;
#endif

         default:
            DIB_DEBUG(PORT_ERR, (CRB "Low Speed IRQ unknow message %d" CRA, msg[0]));
            break;
         }

         /** release driver lock */
         DibDriverTargetEnableIrqProcessing(pContext);
      }
      else 
      {
         break;
      }
   }

   DibMemFree(read_buffer2, 4096 + sizeof(*Type));

   DibSetEvent(&pContext->DriverTargetCtx.IrqReadDone);
   DIB_DEBUG(PORT_LOG, (CRB "Interrupt thread died" CRA));

   return NULL;
}

/*******************************************************************************
 * Read a register from the chip
 ******************************************************************************/
DIBSTATUS DibD2BReadReg(struct DibDriverContext * pContext, uint32_t Addr, uint8_t ReadMode, uint32_t Cnt, uint8_t * pBuf)
{
   int32_t status;
   struct IoctlHead pReq;

   DIB_ASSERT(pBuf);
   if(Cnt > MAX_BRIDGE_DRIVER_BUFFER) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "DibD2BReadReg can't read more than %d bytes" CRA, MAX_BRIDGE_DRIVER_BUFFER));
      return DIBSTATUS_ERROR;
   }

   /* prepare IOCTL request */
   pReq.Nb            = Cnt;
   pReq.param.address = Addr;
   pReq.param.Mode    = ReadMode;
   pReq.Buffer        = pBuf;

   status = ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_REG_UP, &pReq);

   /** ioctl return a DIBSTATUS for REQUEST_REEG_UP */
   return status;
}

/*******************************************************************************
 *  Write a register to the chip
 ******************************************************************************/
DIBSTATUS DibD2BWriteReg(struct DibDriverContext *pContext, uint32_t Addr, uint8_t WriteMode, uint32_t Cnt, uint8_t *pBuf)
{
   int32_t status;
   struct IoctlHead pReq;

   if(Cnt > MAX_BRIDGE_DRIVER_BUFFER) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "DibD2BWriteReg can't write more than %d bytes, will split" CRA, MAX_BRIDGE_DRIVER_BUFFER));

      while(Cnt > MAX_BRIDGE_DRIVER_BUFFER) 
      {
          /* Prepare IOCTL request */
         pReq.Nb            = MAX_BRIDGE_DRIVER_BUFFER;
         pReq.param.address = Addr;
         pReq.param.Mode    = WriteMode;
         pReq.Buffer        = pBuf;

         status = ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_REG_DOWN, &pReq);

         Cnt  -= MAX_BRIDGE_DRIVER_BUFFER;
         pBuf += MAX_BRIDGE_DRIVER_BUFFER;
         /* TODO: check if no autoincr if we have to increase the address too */
         
         /** ioctl return a DIBSTATUS for REQUEST_REG_DOWN */
         if(status != DIBSTATUS_SUCCESS) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "Could not perform IOCTL (status %d)" CRA, status));
            return DIBSTATUS_ERROR;
         }
      }
   }


   pReq.Nb            = Cnt;
   pReq.param.address = Addr;
   pReq.param.Mode    = WriteMode;
   pReq.Buffer        = pBuf;
   
   /** ioctl return a DIBSTATUS for REQUEST_REG_DOWN */
   return ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_REG_DOWN, &pReq);
}

/*******************************************************************************
 *  Send a msg to the chip
 ******************************************************************************/
DIBSTATUS DibD2BSendMsg(struct DibDriverContext *pContext, uint32_t Cnt, uint32_t * buf)
{
   struct IoctlHead pReq;

   if(Cnt > MAX_BRIDGE_DRIVER_BUFFER) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "DibD2BSendMsg can't read more than %d bytes" CRA, MAX_BRIDGE_DRIVER_BUFFER));
      return DIBSTATUS_ERROR;
   }

   /* Prepare IOCTL request */
   pReq.Nb     = Cnt;
   pReq.Buffer = (uint8_t *) buf;
   
   /** ioctl return a DIBSTATUS for REQUEST_MSG_DOWN */
   return ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_MSG_DOWN, &pReq);
}

/*******************************************************************************
 *  Transfert a buffer to embedded memory, passing throuht the bridge
 ******************************************************************************/
DIBSTATUS DibD2BTransfertBuf(struct DibDriverContext *pContext, struct DibDriverDmaCtx * pDmaCtx)
{
   struct IoctlHead pReq;

   /* Prepare IOCTL request */
   pReq.Nb     = sizeof(struct DibDriverDmaCtx);
   pReq.Buffer = (uint8_t *) pDmaCtx;
   
   /** ioctl return a DIBSTATUS for REQUEST_MSG_DOWN */
   return ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_TRANSERT_BUF, &pReq);
}

/*******************************************************************************
 *  Read Data from the bridge
 ******************************************************************************/
DIBSTATUS DibDriverTargetDataRead(struct DibDriverContext * pContext, uint8_t * Buffer, uint32_t * bytes_read, FILTER_HDL FilterHdl)
{
   int32_t ret, status;
   struct IoctlReadData ReadData;
   
   DIB_DEBUG(PACKET_LOG, (CRB "DibDriverTargetDataRead : %d" CRA, (int32_t)(*bytes_read)));

   ReadData.FilterHdl = FilterHdl;
   ReadData.HostBuffer = Buffer;
   ReadData.Mode = eDIB_TRUE;
   ReadData.SizeBuffer = *bytes_read;
   
   if((ret = ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_READ_DATA, &ReadData)) != DIBSTATUS_SUCCESS)
   {
      *bytes_read = 0;
      status = DIBSTATUS_ERROR; 
      DIB_DEBUG(PACKET_LOG, (CRB "DibDriverTargetDataRead ret : %d" CRA, ret));
   }
   else
   {   
      status = DIBSTATUS_SUCCESS;
      DIB_DEBUG(PACKET_LOG, (CRB "DibDriverTargetDataRead size : %d" CRA, ReadData.SizeBuffer));
   }
   
   return status;
}

/*******************************************************************************
 *  Flush data from bridge
 ******************************************************************************/
DIBSTATUS DibDriverTargetDataFlush(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint32_t Size)
{   
   int32_t ret;
   struct IoctlReadData ReadData;

   DIB_DEBUG(PACKET_LOG, (CRB "DibDriverTargetDataFlush : %d" CRA, FilterHdl));

   ReadData.FilterHdl = FilterHdl;
   ReadData.HostBuffer = NULL;
   ReadData.Mode = eDIB_FALSE;
   ReadData.SizeBuffer = Size;

   if((ret = ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_READ_DATA, &ReadData)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(PACKET_ERR, (CRB "DibDriverTargetDataFlush : %d" CRA, ret));
   }
   
   return ret;
}

/*******************************************************************************
 *  Drive to Bridge information concerning the Data path from down to up
 ******************************************************************************/
DIBSTATUS DibD2BInformBridge(struct DibDriverContext *pContext, enum DibBridgeInfoType BridgeInfoType, union DibInformBridge * pInfoBridge)
{
   if(BridgeInfoType == eSET_SERVICE)
   {
      struct IoctlSvcInfo DataInfo;

      DataInfo.DataMode = pInfoBridge->SetService.DataMode;
      DataInfo.DataType = pInfoBridge->SetService.DataType;
      DataInfo.item     = pInfoBridge->SetService.ItemHdl;
      DataInfo.filter   = pInfoBridge->SetService.FilterHdl;
      DataInfo.svc      = pInfoBridge->SetService.Svc;
      DataInfo.StreamId = pInfoBridge->SetService.StreamId;

      /** ioctl return a DIBSTATUS for REQUEST_DATAPATH_INFO */
      return ioctl(pContext->DriverTargetCtx.BridgeFd, REQUEST_DATAPATH_INFO, &DataInfo);
   }
   return DIBSTATUS_ERROR;
}

/*******************************************************************************
 *  Disable IRQ processing
 ******************************************************************************/
void DibDriverTargetDisableIrqProcessing(struct DibDriverContext *pContext)
{
  DibAcquireLock(&pContext->DriverTargetCtx.IrqLock);
}

/*******************************************************************************
 *  Enable IRQ processing
 ******************************************************************************/
void DibDriverTargetEnableIrqProcessing(struct DibDriverContext *pContext)
{
  DibReleaseLock(&pContext->DriverTargetCtx.IrqLock);
}
