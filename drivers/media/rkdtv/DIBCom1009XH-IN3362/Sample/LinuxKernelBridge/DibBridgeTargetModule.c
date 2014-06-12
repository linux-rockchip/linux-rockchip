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
* @file "DibBridgeTargetModule.c"
* @brief Target specific implementation.
*
***************************************************************************************************/
#include <linux/kernel.h>       /* We're doing kernel work */
#include <linux/module.h>       /* Specifically, a module */
#include <linux/fs.h>
#include <asm/uaccess.h>        /* for get_user and put_user */
#include <linux/ioctl.h>

#include "DibBridgeCommon.h"
#include "DibBridgeTargetCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridge.h"
#include "DibBridgeTarget.h"
#include "DibBridgeData.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeTargetModule.h"
#include "DibBridgeTargetIrq.h"
#include "DibBridgeTargetNet.h"

#define DRIVER_VERSION "v1.0"
#define DRIVER_AUTHOR "Dibcom - http://www.dibcom.net/"
#define DRIVER_DESC "Dibcom bridge for DIB7xxx"
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/***********************************************************************/
/***               Local Variables                                   ***/
/***********************************************************************/
uint32_t            DebugMask = 0xFFFF0000;
static enum DibBool running = eDIB_FALSE;

module_param(DebugMask, uint, 0);
MODULE_PARM_DESC(DebugMask, "Debug Mask");

static void IntBridgeTargetModuleFreeStruct(struct DibBridgeContext *pContext)
{
   if(pContext)
   {
      /* Free IOCTL Lock */
      DibDeAllocateLock(&pContext->BridgeTargetCtx.IoctlLock);

      /* Free buffer for IOCTL request */
      if(pContext->BridgeTargetCtx.IoctlReq)
      {
         if(pContext->BridgeTargetCtx.IoctlReq->Buffer)
         {
            kfree(pContext->BridgeTargetCtx.IoctlReq->Buffer);
            pContext->BridgeTargetCtx.IoctlReq->Buffer = NULL;
         }

         /* Free ptr for IOCTL request */
         kfree(pContext->BridgeTargetCtx.IoctlReq);
         pContext->BridgeTargetCtx.IoctlReq = NULL;
      }

      /* Free context */
      DibBridgeTargetFreeBuf((uint8_t *)pContext, sizeof(*pContext));
   }
}


/*-----------------------------------------------------------------------
|  Release the Device Driver
|  ressources are freed on IOCTL module shutdown
|   here we only mark the device as closed !
-----------------------------------------------------------------------*/
static int32_t DibBridgeTargetModuleRelease(struct inode *inode, struct file *file)
{
   struct DibBridgeContext *pContext = (struct DibBridgeContext *) file->private_data;

   /* Only opened device can be closed */
   if(pContext == NULL) 
      return DIBSTATUS_ERROR;

   DIB_DEBUG(PORT_LOG, (CRB "dibbridge device release" CRA));

   flush_scheduled_work();

   /* Free previously allocated context in reverse order */
   /* Deinit bridge should have been called previously by shutdown ioctl */
   /*DibBridgeDeinit(pContext);*/

   if (running == eDIB_TRUE) {
       DibBridgeDeinit(pContext);
       running = eDIB_FALSE;
       printk("Bridge Deinit\n");
   }

   /* Free allocated struct */
   IntBridgeTargetModuleFreeStruct(pContext);

   file->private_data = NULL;

   module_put(THIS_MODULE);

   return DIBSTATUS_SUCCESS;
}

/*-----------------------------------------------------------------------
|  Open the Device Driver
-----------------------------------------------------------------------*/
static int32_t DibBridgeTargetModuleOpen(struct inode *inode, struct file *file)
{
   int32_t                  status         = DIBSTATUS_ERROR;
   struct DibBridgeContext *pBridgeContext = NULL;
   printk(CRB "Trying to open dibbridge..");

   pBridgeContext = (struct DibBridgeContext *) DibBridgeTargetAllocBuf(sizeof(struct DibBridgeContext));

   if(pBridgeContext == NULL)
   {
      printk(CRB "%s: Out of memory" CRA, __func__);
      return -ENOMEM;
   }

   /* Clear Content */
   DibZeroMemory(pBridgeContext, sizeof(struct DibBridgeContext));

   /* Allocate ptr for IOCTL request */
   pBridgeContext->BridgeTargetCtx.IoctlReq = kmalloc(sizeof(struct IoctlHead), GFP_KERNEL);

   if(pBridgeContext->BridgeTargetCtx.IoctlReq == NULL) 
   {
      printk(CRB "%s: Out of memory" CRA, __func__);
      status =  -ENOMEM;
      goto End;
   }

   /* Allocate buffer for IOCTL request */
   pBridgeContext->BridgeTargetCtx.IoctlReq->Buffer = kmalloc(MAX_BRIDGE_DRIVER_BUFFER, GFP_KERNEL);

   if(pBridgeContext->BridgeTargetCtx.IoctlReq->Buffer == NULL) 
   {
      printk(CRB "%s: Out of memory" CRA, __func__);
      status =  -ENOMEM;
      goto End;
   }

   /* Init IOCTL Lock */
   DibAllocateLock(&pBridgeContext->BridgeTargetCtx.IoctlLock);
   DibInitLock(&pBridgeContext->BridgeTargetCtx.IoctlLock);

   status = DIBSTATUS_SUCCESS;

End:
   if(status != DIBSTATUS_SUCCESS)
   {
      /* Free possibly allocated struct */
      IntBridgeTargetModuleFreeStruct(pBridgeContext);
   }
   else
   {
      status = (int32_t)DIBSTATUS_SUCCESS;

      /* Store Allocated Bridge Context */
      file->private_data = pBridgeContext;

   try_module_get(THIS_MODULE);
   }

   return status;
}

/**
 * Copy a kernel buffer to user buffer
 */
DIBSTATUS DibBridgeTargetCopyToUser(uint8_t *pUserBuffer, uint8_t *pKernelBuffer, uint32_t Size)
{
   int32_t ret = copy_to_user(pUserBuffer, pKernelBuffer, Size); 
   if(ret) 
      return DIBSTATUS_ERROR;

   return DIBSTATUS_SUCCESS;
}

/*-----------------------------------------------------------------------
|  Read from bridge
-----------------------------------------------------------------------*/
int32_t DibBridgeTargetModuleReadMsg(struct DibBridgeContext *pContext, uint8_t *Buffer)
{
   int32_t ret = -1; 
   DIBSTATUS rc;
   uint16_t Type, Size;

   if(pContext) 
   {
      /* retrieve a msg from the MsgQueue */
      DIB_ASSERT(Buffer);
      rc = DibBridgeGetBufFromMsgQueue(pContext, &Type, &Size, Buffer + sizeof(Type));

      if(rc == DIBSTATUS_ERROR) 
      {
         DIB_DEBUG(PORT_LOG, (CRB "MsgQueue deinitialized" CRA));
   	if (running == eDIB_TRUE) {
       		DibBridgeDeinit(pContext);
       		running = eDIB_FALSE;
       		printk("Bridge Deinit\n");
   		}
         return -1;
      }


      if(Buffer)
      {
         /* Remove flagged Type, it is not needed */
         Type &= UPMESSAGE_MASK_TYPE;

         /* copy msg Type to user Buffer */
         ret = copy_to_user(Buffer, &Type, sizeof(Type));
      }

      if (ret) 
      {
         printk(CRB "Will not complete read, device not open" CRA);
         return -1;
      }

      return Size + sizeof(Type);
   }
   else 
   {
      printk(CRB "Will not complete read, device not open" CRA);
      return -1;
   }
}

/** IOCTL implementation.
 * @param DIBSTATUS in cas of all ioctl_num except REQUEST_READ_MSG : the size is returned
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
long DibBridgeTargetModuleUnlockedIoctl(struct file *file, uint32_t ioctl_num, LINUX_PTR_CAST ioctl_param)
#else
int32_t DibBridgeTargetModuleIoctl(struct inode *inode, struct file *file, uint32_t ioctl_num, LINUX_PTR_CAST ioctl_param)
#endif
{
   int32_t                  status   = DIBSTATUS_SUCCESS;
   struct DibBridgeContext *pContext = (struct DibBridgeContext *) file->private_data;
   struct IoctlHead        *req      = pContext->BridgeTargetCtx.IoctlReq;

   DIB_ASSERT(pContext);

   switch (ioctl_num) 
   {
   case REQUEST_BRIDGE_POSTINIT:
      if(DibBridgePostInit(pContext, ioctl_param) != DIBSTATUS_SUCCESS)
         return DIBSTATUS_INVALID_PARAMETER;
      break;

   case REQUEST_BRIDGE_MODULE_INIT:
      {
         struct IoctlInit Init;

         if(copy_from_user(&Init, (struct IoctlInit *) ioctl_param, sizeof(struct IoctlInit)))
         {
            DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
            return DIBSTATUS_INVALID_PARAMETER;
         }

         running = eDIB_TRUE;

         /* Initialize bridge (common & target). TODO: Retreive the board handler from the file name */
         return DibBridgeInit(pContext, Init.BoardHdl, Init.ChipSelect, Init.Config, DebugMask);
      }
      break;

   case REQUEST_BRIDGE_MODULE_SHUTDOWN:
      if(DibBridgeDeinit(pContext) != DIBSTATUS_SUCCESS){ 
         return DIBSTATUS_INVALID_PARAMETER;
      }
      running = eDIB_FALSE;
      break;

   case REQUEST_REG_DOWN:
      DibAcquireLock(&pContext->BridgeTargetCtx.IoctlLock);
      if(copy_from_user(req, (struct IoctlHead *) ioctl_param, offsetof(struct IoctlHead, Buffer)))
      {
         DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      if(copy_from_user(req->Buffer, ((struct IoctlHead *) ioctl_param)->Buffer, req->Nb))
      {
         DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      status = DibBridgeWrite(pContext, req->param.address, req->param.Mode, req->Buffer, req->Nb);
      DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
   break;

   case REQUEST_REG_UP:
      DibAcquireLock(&pContext->BridgeTargetCtx.IoctlLock);
      if(copy_from_user(req, (struct IoctlHead *) ioctl_param, offsetof(struct IoctlHead, Buffer)))
      {
         DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      if(copy_from_user(req->Buffer, ((struct IoctlHead *) ioctl_param)->Buffer, req->Nb))
      {
         DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      status = DibBridgeRead(pContext, req->param.address, req->param.Mode, req->Buffer, req->Nb);

      if(copy_to_user((struct IoctlHead *) ioctl_param, req, offsetof(struct IoctlHead, Buffer)))
      {
         printk(KERN_WARNING CRB "problem copy_to_user" CRA);
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      if(copy_to_user(((struct IoctlHead *) ioctl_param)->Buffer, req->Buffer, req->Nb))
      {
         printk(KERN_WARNING CRB "problem copy_to_user 2" CRA);
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
      break;

   case REQUEST_TRANSERT_BUF:
      DibAcquireLock(&pContext->BridgeTargetCtx.IoctlLock);
      {
         uint8_t *pBlock, *pHostAddr;
         struct DibBridgeDmaCtx BridgeDma;
         if(copy_from_user(req, (struct IoctlHead *) ioctl_param, offsetof(struct IoctlHead, Buffer))) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
            DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
            return DIBSTATUS_INVALID_PARAMETER;
         }
         if(copy_from_user(req->Buffer, ((struct IoctlHead *) ioctl_param)->Buffer, req->Nb))
         {
            DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
            DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
            return DIBSTATUS_INVALID_PARAMETER;
         }

         memcpy(&BridgeDma, req->Buffer, sizeof(struct DibBridgeDmaCtx) - sizeof(struct DibBridgeDmaFlags));
         memset(&BridgeDma.DmaFlags, 0, sizeof(struct DibBridgeDmaFlags));

         /*pBlock = DibMemAlloc(BridgeDma.DmaLen);*/
         pBlock = DibBridgeTargetAllocBuf(BridgeDma.DmaLen);
         
         printk(CRB "Request Dma Transfert dir=%d mode=%d chip=%08x host=%p kernel=%p len=%d" CRA,
            BridgeDma.Dir, BridgeDma.Mode, BridgeDma.ChipAddr,BridgeDma.pHostAddr,pBlock,BridgeDma.DmaLen);

         /* save user host buffer and use the kernel one to do the dma transfer */
         pHostAddr = BridgeDma.pHostAddr;
         BridgeDma.pHostAddr = pBlock;

         if(BridgeDma.Dir == DIBBRIDGE_DMA_WRITE)
         {
            if(copy_from_user(pBlock, pHostAddr, BridgeDma.DmaLen)) 
            {
               DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA,__func__));
               DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
               return DIBSTATUS_INVALID_PARAMETER;
            }
         }

         status = DibBridgeTransfertBuf(pContext, &BridgeDma);

         if(BridgeDma.Dir == DIBBRIDGE_DMA_READ)
         {
            if(copy_to_user(pHostAddr, pBlock, BridgeDma.DmaLen)) 
            {
               DIB_DEBUG(PORT_ERR, (CRB "%s: copy_to_user failed" CRA, __func__));
               DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
               return DIBSTATUS_INVALID_PARAMETER;
            }
         }

         /*DibMemFree(pBlock,BridgeDma.DmaLen);*/
         DibBridgeTargetFreeBuf(pBlock,BridgeDma.DmaLen);
      }
      DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
      break;

   case REQUEST_MSG_DOWN:
      DibAcquireLock(&pContext->BridgeTargetCtx.IoctlLock);
      if(copy_from_user(req, (struct IoctlHead *) ioctl_param, offsetof(struct IoctlHead, Buffer)))
      {
         DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      if(copy_from_user(req->Buffer, ((struct IoctlHead *) ioctl_param)->Buffer, req->Nb))
      {
         DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
         DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
         return DIBSTATUS_INVALID_PARAMETER;
      }
      status = DibBridgeSendMsg(pContext, (uint32_t *) req->Buffer, req->Nb);
      DibReleaseLock(&pContext->BridgeTargetCtx.IoctlLock);
      break;

   case REQUEST_READ_MSG:
      status = DibBridgeTargetModuleReadMsg(pContext, (uint8_t *)ioctl_param);
      break;

   case REQUEST_DATAPATH_INFO:
      {
         struct IoctlSvcInfo DataInfo;
         union DibInformBridge SetBridgeInfo;

         if(copy_from_user(&DataInfo, (struct IoctlSvcInfo *) ioctl_param, sizeof(struct IoctlSvcInfo)))
         {
            DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
            return DIBSTATUS_INVALID_PARAMETER;
         }

         SetBridgeInfo.SetService.Svc = DataInfo.svc;
         SetBridgeInfo.SetService.ItemHdl = DataInfo.item;
         SetBridgeInfo.SetService.FilterHdl = DataInfo.filter;
         SetBridgeInfo.SetService.StreamId = DataInfo.StreamId;
         SetBridgeInfo.SetService.DataType = DataInfo.DataType;
         SetBridgeInfo.SetService.DataMode = DataInfo.DataMode;

         status = DibBridgeSetInfo(pContext, eSET_SERVICE, &SetBridgeInfo);
      }
      break;
      
   case REQUEST_READ_DATA:
   {
      struct IoctlReadData ReadData;
      uint32_t ReadSize;
      uint8_t * ReadBuf;
      int32_t ret;
      FILTER_HDL FiltHdl;
      if(copy_from_user(&ReadData, (struct IoctlReadData *)ioctl_param, sizeof(struct IoctlReadData)))
      {
         DIB_DEBUG(PORT_ERR, (CRB "%s: copy_from_user failed" CRA, __func__));
         return DIBSTATUS_INVALID_PARAMETER;
      }
      ReadSize = (uint32_t)(ReadData.SizeBuffer);
      FiltHdl = ReadData.FilterHdl;
      
      if(ReadData.Mode == eDIB_TRUE)
      {
         /** Resynchronise FilterHdl from user and from bridge queue */
         if(DibBridgeGetRawBufferFull(pContext, &ReadBuf, &FiltHdl) != DIBSTATUS_SUCCESS)
         {
            printk(CRB "Read Data: not able to read a buffer from the wanted filter" CRA);
            return DIBSTATUS_ERROR;
         }

         /* copy msg Type to user Buffer */
         if(ReadSize > 0) 
         {
            ret = copy_to_user(ReadData.HostBuffer, ReadBuf, ReadSize);
   
            /* check if success */
            if(ret) 
            {
               DIB_DEBUG(PORT_ERR, (CRB "%s: copy_to_user failed" CRA, __func__));
               status = DIBSTATUS_ERROR;
            }else{
               
               status = DIBSTATUS_SUCCESS;
            }
         }
          /* for mpe processing, the buffer is freed later, after having forwarded the data */
         if(pContext->FilterInfo[FiltHdl].DataType != eMPEFEC)
         {
            DibBridgeNotifyRawbufferRead(pContext, ReadBuf, ReadSize);
         }
         
      }else{
         if(DibBridgeGetRawBufferFull(pContext, &ReadBuf, &FiltHdl) != DIBSTATUS_SUCCESS)
         {
            printk(CRB "Read Data: not able to read a buffer" CRA);
            return DIBSTATUS_ERROR;
         }

         /* for mpe processing, the buffer is freed later, after having forwarded the data */
         if(pContext->FilterInfo[FiltHdl].DataType != eMPEFEC)
         {
            /** flush the buffer correctly */
            DibBridgeNotifyRawbufferRead(pContext, ReadBuf, ReadSize);
         }

         status = DIBSTATUS_SUCCESS;            
      }
   }
      break;

   default:
      DIB_DEBUG(PORT_ERR, (CRB "DIbBridgeModuleIoctl: unknown Type %d" CRA, ioctl_num));

      status = DIBSTATUS_INVALID_PARAMETER;
      break;
   }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
   return (long)status;
#else
   return status;
#endif
}

/*-----------------------------------------------------------------------
|  Read from bridge
-----------------------------------------------------------------------*/
ssize_t DibBridgeTargetModuleReadData(struct file * file_pointer, char __user * Buffer, size_t count, loff_t * f_pos)
{
   return -1;
}

/*-----------------------------------------------------------------------
|  Module Declarations
-----------------------------------------------------------------------*/
struct file_operations fops = 
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
   .unlocked_ioctl   = DibBridgeTargetModuleUnlockedIoctl,
#else
   .ioctl   = DibBridgeTargetModuleIoctl,
#endif
   .open    = DibBridgeTargetModuleOpen,
   .read    = DibBridgeTargetModuleReadData,
   .release = DibBridgeTargetModuleRelease, 
};

/*-----------------------------------------------------------------------
|  Initialise module
-----------------------------------------------------------------------*/
int32_t DibBridgeTargetModuleInit(void)
{
   int32_t ret_val;

   /* Init Net */
   ret_val = DibBridgeTargetNetInit();

   if(ret_val  < 0) 
      goto End;

   /* (at least try to) register the character device */
   ret_val = register_chrdev(MAJOR_NUM, DEVICE_FILE_NAME, &fops);

   /* Negative values signify an error */
   if(ret_val < 0) 
   {
      /* Clean up Network device */
      DibBridgeTargetNetCleanup();

      printk(CRB "Register dibbridge device driver failed with %d" CRA, ret_val);
      return ret_val;
   }
   else 
   {
      printk(CRB "dibbridge Module loaded" CRA);
   }

End:
   return ret_val;
}

/*-----------------------------------------------------------------------
|   Clean up before unload
-----------------------------------------------------------------------*/
void DibBridgeTargetModuleCleanup(void)
{
   /* Clean up Network device */
   DibBridgeTargetNetCleanup();

   flush_scheduled_work(); /* ?? */

   /*
   * Unregister the device
   */
   unregister_chrdev(MAJOR_NUM, DEVICE_FILE_NAME);

   printk(CRB "dibbridge Module unloaded" CRA);
}

module_init(DibBridgeTargetModuleInit);
module_exit(DibBridgeTargetModuleCleanup);
