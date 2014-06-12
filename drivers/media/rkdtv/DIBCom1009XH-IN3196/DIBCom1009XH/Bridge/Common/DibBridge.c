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
* @file "DibBridge.c"
* @brief Bridge functionality.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridgeTarget.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeTestIf.h"
#include "DibBridgeRawTs.h"
#include "DibBridge.h"
#include "DibBridgeData.h"

#if (USE_FIREFLY == 1)
#include "DibBridgeFirefly.h"
#endif

#if (USE_DRAGONFLY == 1)
#include "DibBridgeDragonfly.h"
#endif
/* Private SIOC flags for IOCTL */
#define SIOC_ENABLE_HS          1

#define SIOC_ENABLE_IRQ         2

/****************************************************************************
* Map specific chip operations to DibBridgeChipOperations
****************************************************************************/
DIBSTATUS DibBridgeRegisterChipIf(struct DibBridgeContext *pContext, uint32_t * Config)
{
   switch(pContext->DibChip)
   {
#if (USE_DRAGONFLY == 1)
      case DIB_VOYAGER:
      case DIB_NAUTILUS:
         DibBridgeDragonflyRegisterIf(pContext, Config);
         return DIBSTATUS_SUCCESS;
#endif

#if (USE_FIREFLY == 1)
      case DIB_FIREFLY:
         DibBridgeFireflyRegisterIf(pContext);
         return DIBSTATUS_SUCCESS;
#endif

      default:
         DIB_DEBUG(PORT_ERR, (CRB "DibBridgeRegisterChipIf is not defined" CRA));
         DIB_DEBUG(PORT_ERR, (CRB "" CRA));
         return DIBSTATUS_ERROR;
   }
}

/****************************************************************************
* DibBridgeInit.
* Called when the module is opened by the user (not during init_module)
****************************************************************************/
DIBSTATUS DibBridgeInit(struct DibBridgeContext *pContext, BOARD_HDL BoardHdl, uint32_t ChipSelect, uint32_t * Config, uint32_t DebugMask)
{
#if (mSDK == 0)
   uint8_t i;
#endif

   DIB_ASSERT((ChipSelect == DIB_FIREFLY) || (ChipSelect == DIB_VOYAGER) || (ChipSelect == DIB_NAUTILUS));

   pContext->BridgeDebugMask = DebugMask;

   /* Get Chip config */
   pContext->DibChip = ChipSelect;
   if(DibBridgeRegisterChipIf(pContext, Config) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(PORT_ERR,(CRB "DibBridgeRegisterChipIf Error" CRA));
      return DIBSTATUS_ERROR;
   }

   DIB_DEBUG(PORT_LOG, (CRB "Using debug Mask = 0x%x" CRA, DebugMask));

   pContext->DevFailed = 0;
   pContext->BoardHdl  = BoardHdl;

#if (mSDK == 0)
   DibBridgeInitMpeFecCtx(pContext);
   DibBridgeInitMpeIFecCtx(pContext);
   DibBridgeInitCmmbCtx(pContext);
   DibBridgeInitDabCtx(pContext);

   for(i = 0; i < DIB_MAX_NB_SERVICES; i++) 
   {
      pContext->FireflyInfo[i].ItemIndex = DIB_UNSET; 
   }
   
   DibBridgeInitMpeFecBuffers(pContext);

   for(i = 0; i < DIB_MAX_NB_FILTERS; i++) 
   {
      pContext->FilterInfo[i].DataMode = eUNSET;
      pContext->FilterInfo[i].DataType = eUNKNOWN_DATA_TYPE;
      pContext->FilterInfo[i].ItemCount   = 0;
#if DIB_CHECK_RAWTS_DATA == 1
      DibZeroMemory(&pContext->FilterInfo[i].CheckRawTs, sizeof(struct CheckRawTs));
#endif
   }

   for(i = 0; i < MAX_Q_BUFF; i++) 
   {
      pContext->DataInfoUp[i].pBuff = 0;
      pContext->DataInfoUp[i].FilterIndex = DIB_UNSET;
   }

   pContext->NextFreeWrBuff = 0;
   pContext->NextFreeRdBuff = 0;
   
   pContext->RawDataCtx.ReadPtr  = 0;
   pContext->RawDataCtx.WritePtr = 0;
   pContext->RawDataCtx.TheFireflyRawTsFilter = DIB_UNSET;
   DibZeroMemory(&(pContext->RawDataCtx.BufList), RAWTS_NB_BUFFER * sizeof(uint8_t *));
   
#endif /* mSDK == 0 */

   pContext->IrqEnabled = 0;
   pContext->ResetApb   = 0;

   memset(&pContext->DmaCtx, 0, sizeof(struct DibBridgeDmaCtx));

#if (USE_DRAGONFLY == 1)
   SerialBufInit(&pContext->TxSerialBuf, pContext->TxBuffer, 32);
#endif

#if (DEMOD_TRACE == 1)
   /*** Demod trace Buffer allocation*/
   pContext->DemodTraceBuf = DibMemAlloc(DTRACE_BUFF_SIZE + 2); /* including the Type byte (driver stuff) */

   if(pContext->DemodTraceBuf == 0) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "Unable to allocate demod trace Buffer" CRA));
      goto InitError;
   }
#endif


  /** Initialize IpCallback Functions **/
  pContext->IpCallback.DataMpegTs = NULL;
  pContext->IpCallback.DataMpe = NULL;
  pContext->IpCallback.DataCmmb = NULL;
  pContext->IpCallback.DataTdmb = NULL;
  pContext->IpCallback.DataDab = NULL;
  pContext->IpCallback.DataDabPlus = NULL;

#if (mSDK == 0)
   /*** SIPSI Buffer allocation*/
   pContext->SiPsiBuf = (uint8_t *)DibMemAlloc(SIPSI_BUFF_SIZE);

   if(pContext->SiPsiBuf == 0) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "Unable to allocate sipsi Buffer" CRA));
      goto InitError;
   }

   /* Init SliceBuf */
   pContext->SliceBuf = 0;

#endif /* mSDK == 0 */

   /* Init low level */
   if(DibBridgeTargetInit(pContext) != DIBSTATUS_SUCCESS) 
   {
      /* Because the init failed, we can't really use the debug function
      which uses a Mask inside the context ... */
      DIB_DEBUG(PORT_ERR, (CRB "Low level initialization failed" CRA));
      goto InitError;
   }

#if (DRIVER_AND_BRIDGE_MERGED == 0)
   /* Init Bridge2Driver msg queue */
   DibBridgeInitMsgQueue(pContext);
#endif

   /* Init chipset dependant software && hardware */
   if(DibBridgeChipsetInit(pContext) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(PORT_ERR, (CRB "Chipset bridge init failed" CRA));
      goto InitError;
   }

#if (DIB_BRIDGE_TESTIF_PREINIT == 1)
      /* Bridge Interface Pre-initialisation test*/
      if(DibBridgeTestIf(pContext, BASIC_READ_TEST | REGISTER_TEST) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(PORT_ERR, (CRB "DibBridgeTestIf failed before init." CRA));
         return DIBSTATUS_ERROR;
      }
#endif


   return DIBSTATUS_SUCCESS;

InitError:

#if (DEMOD_TRACE == 1)
   if(pContext->DemodTraceBuf)
      DibMemFree(pContext->DemodTraceBuf, DTRACE_BUFF_SIZE + 2);
#endif

   if(pContext->SiPsiBuf)
      DibMemFree(pContext->SiPsiBuf, SIPSI_BUFF_SIZE);

   return DIBSTATUS_ERROR;
}

/****************************************************************************
* DibBridgePostIinit
****************************************************************************/
DIBSTATUS DibBridgePostInit(struct DibBridgeContext *pContext, uint32_t Flag)
{
#if (DIB_BRIDGE_TESTIF_POSTINIT == 1)
   uint8_t TestFlag;
#endif

   if(Flag & SIOC_ENABLE_IRQ)
      DibBridgeEnableIrq(pContext);

   if(Flag & SIOC_ENABLE_HS) 
   {
      if(DibBridgeTargetSetHighSpeed(pContext) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(PORT_ERR,(CRB "DibBridgeTargetSetHighSpeed Error" CRA));
         return DIBSTATUS_ERROR;
      }

#if (DIB_BRIDGE_TESTIF_POSTINIT == 1)
      /* Call postinit test if needed (depends on DIB_BRIDGE_TESTIF_POSTINIT define) */
      /* Disable external RAM test if HBM mode is enabled */
      TestFlag = FULL_TEST;
      if (HBM_SELECT_GET(Flag))
         TestFlag &= ~EXT_RAM_TEST;

      /* Post initialisation interface test */
      if(DibBridgeTestIf(pContext, TestFlag) != 0)
      {
         DIB_DEBUG(PORT_ERR,(CRB "DibBridgeTestIf failed after post init." CRA));
         return DIBSTATUS_ERROR;
      }
#endif
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* DibBridgeEnableIrq
****************************************************************************/
void DibBridgeEnableIrq(struct DibBridgeContext *pContext)
{
   /* Enable IRQ */
   if(!pContext->IrqEnabled) 
   {
      pContext->IrqEnabled = 1;
      DibBridgeTargetSetupIrqHandler(pContext);
      DibBridgeTargetEnableIrq(pContext);
      DIB_DEBUG(MAILBOX_LOG, (CRB "Host IRQ is now Enabled" CRA));
   }
}

/****************************************************************************
* DibBridgeDeinit
****************************************************************************/
DIBSTATUS DibBridgeDeinit(struct DibBridgeContext *pContext)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;
   
   DIB_DEBUG(PORT_LOG, (CRB "DibBridgeDeinit!" CRA));

   /* Deinit the msg queue Bridge2Driver */
   if(pContext->IrqEnabled)
   {
      DibBridgeTargetRemoveIrqHandler(pContext);
      pContext->IrqEnabled = 0;
   }
   
   ret = DibBridgeTargetDeinit(pContext);
   
#if (DRIVER_AND_BRIDGE_MERGED == 0)
   DibBridgeTerminateMsgQueue(pContext);
#endif

#if (mSDK == 0)
   DibBridgeFreeMpeFecBuffers(pContext);
   DibBridgeFreeMpeIFecBuffers(pContext);

   if(pContext->SiPsiBuf != NULL) 
   {
      DibMemFree(pContext->SiPsiBuf, SIPSI_BUFF_SIZE);
      pContext->SiPsiBuf = NULL;
   }
#endif /* mSDK == 0 */

#if (DEMOD_TRACE == 1)
   if(pContext->DemodTraceBuf)
   {
      DibMemFree(pContext->DemodTraceBuf, DTRACE_BUFF_SIZE + 2);
      pContext->DemodTraceBuf = NULL;
   }
#endif

#if (mSDK == 0)
   if(pContext->SliceBuf != NULL) 
   {
      DibBridgeTargetFreeBuf(pContext->SliceBuf, HBM_SLICE_SIZE);
      pContext->SliceBuf = NULL;
   }
#endif

   DibBridgeChipsetDeinit(pContext);

   return ret;
}

/**
 * Fill the "DataType" and "FireflyInfo" information with the driver's information concerning the link
 * between service (in firefly) and item and concerning the Data Mode: ip or callback. Also fill ItSvc and FilterInfo table to keep needed information
 * about filter and item. This function handle item and filter removing and inserting.
 * (the eUNSET value is used to reset the entry when removing a service)
 * @param pContext   pointer to the bridge context
 * @param svc        firefly's service 
 * @param item       item's number concerned
 * @param DataType   Type of the Filter.
 * @param DataMode   Mode of the Data (ip or callback). if DataMode is eUNSET, the service is removed from item and filter
 */

#if (mSDK == 0)
DIBSTATUS DibBridgeSetInfo(struct DibBridgeContext *pContext, enum DibBridgeInfoType BridgeInfoType, union DibInformBridge * pInfoBridge)
{
   /** Disable bridge irq to protect bridge ItSvc and FilterInfo structure */
   DibBridgeTargetDisableIRQ(pContext);

   if(BridgeInfoType == eSET_SERVICE)
   {
      struct DibSetService * pSetService = &pInfoBridge->SetService;
      uint8_t Svc = pSetService->Svc;
      ELEM_HDL ItemHdl = pSetService->ItemHdl;
      FILTER_HDL FilterHdl = pSetService->FilterHdl;
      enum DibDataType DataType = pSetService->DataType;
      enum DibDataMode DataMode = pSetService->DataMode;
      uint32_t StreamId = pSetService->StreamId;

      DIB_ASSERT((DataMode == eUNSET) || (DataMode == eIP) || (DataMode == eCLBACK));
      if(DataMode != eUNSET)
      {
         /** Adding a new Svc */
         DIB_ASSERT(pContext->ItSvc[ItemHdl].FilterParent == DIB_UNSET);

         /** If filter is not allocated, allocate one */
         if(pContext->FilterInfo[FilterHdl].DataMode == eUNSET)
         {
            DIB_ASSERT(pContext->FilterInfo[FilterHdl].ItemCount == 0);
            pContext->FilterInfo[FilterHdl].DataMode = DataMode;
            pContext->FilterInfo[FilterHdl].DataType = DataType;
            pContext->FilterInfo[FilterHdl].StreamId = StreamId;

#if DIB_CHECK_RAWTS_DATA == 1
            if(DataType == eTS)
            {
               DibSetMemory(&pContext->FilterInfo[FilterHdl].CheckRawTs, 0, sizeof(struct CheckRawTs));
            }
#endif
            DIB_DEBUG(PORT_LOG,(CRB "AddBridgeFilter[%d] .DataMode=%d .DataType=%d .ItemCount=%d" CRA,  FilterHdl, pContext->FilterInfo[FilterHdl].DataMode, pContext->FilterInfo[FilterHdl].DataType, pContext->FilterInfo[FilterHdl].ItemCount));
         }
         else
         {
            DIB_ASSERT(pContext->FilterInfo[FilterHdl].ItemCount > 0);
            DIB_ASSERT(pContext->FilterInfo[FilterHdl].DataMode == DataMode);
            DIB_ASSERT(pContext->FilterInfo[FilterHdl].DataType == DataType);
            DIB_ASSERT(pContext->FilterInfo[FilterHdl].StreamId == StreamId);
#if DIB_CHECK_RAWTS_DATA == 1
            if(DataType == eTS)
            {
               /* Clear the PidOk bit to reset continuity */
               if(pSetService->Pid < 8190)
               {
                  pContext->FilterInfo[FilterHdl].CheckRawTs.PidsOk[pSetService->Pid / 8] &= ~(1 << (pSetService->Pid & 7));
               }
            }
#endif
         }

         pContext->FilterInfo[FilterHdl].ItemCount++;
         pContext->ItSvc[ItemHdl].FilterParent = FilterHdl;

         /* Clear IP/RTP checker monitoring info */
         if((DataType == eMPEFEC) || (DataType == eMPEIFEC) || (DataType == eCMMBSVC))
         {
            pContext->ItSvc[ItemHdl].CcFailCnt = 0;
            pContext->ItSvc[ItemHdl].ErrCnt = 0;
            pContext->ItSvc[ItemHdl].CurCc = 0xffff;
         }
         if((DataType == eDAB))
         {
            pContext->ItSvc[ItemHdl].CcFailCnt     = 0;
            pContext->ItSvc[ItemHdl].ErrCnt        = 0;
            pContext->ItSvc[ItemHdl].CurCc         = 0;            
            pContext->ItSvc[ItemHdl].DataLenRx     = 0;
#if (DIB_DAB_DATA == 1)
            pContext->ItSvc[ItemHdl].NbMaxFrames   = 0;
#endif
         }

         DibBridgeSetService(pContext, Svc, ItemHdl, FilterHdl, DataType, DataMode);

         DIB_DEBUG(RAWTS_LOG, (CRB "NEW : s %d i %d f %d m %d" CRA CRA, Svc, ItemHdl, FilterHdl, DataMode));   
      }
      else
      {
         /** Svc is removing */
         DIB_ASSERT(pContext->FilterInfo[FilterHdl].ItemCount > 0);
         DIB_ASSERT(pContext->ItSvc[ItemHdl].FilterParent < DIB_MAX_NB_FILTERS);

         pContext->ItSvc[ItemHdl].FilterParent = DIB_UNSET;
         pContext->FilterInfo[FilterHdl].ItemCount--;

         if(pContext->FilterInfo[FilterHdl].ItemCount == 0)
         {
            DIB_DEBUG(PORT_LOG,(CRB "RemoveBridgeFilter[%d] .DataMode=%d .DataType=%d" CRA,  FilterHdl, pContext->FilterInfo[FilterHdl].DataMode, pContext->FilterInfo[FilterHdl].DataType));
            pContext->FilterInfo[FilterHdl].DataMode = eUNSET;
            pContext->FilterInfo[FilterHdl].DataType = eUNKNOWN_DATA_TYPE;
         }

         DibBridgeSetService(pContext, Svc, ItemHdl, FilterHdl, DataType, eUNSET);
       
         DIB_DEBUG(RAWTS_LOG, (CRB "RES : s %d i %d f %d m %d" CRA, Svc, ItemHdl, FilterHdl, DataMode));   
         DIB_DEBUG(RAWTS_LOG, (CRB "" CRA));
      }
   }

   DibBridgeTargetEnableIrq(pContext);

   DIB_DEBUG(RAWTS_LOG, ("---"));
   return DIBSTATUS_SUCCESS;
}
#endif /* mSDK == 0 */

/****************************************************************************
* Bridge Target must declare at init was is the supported HostInterface type
****************************************************************************/
DIBSTATUS DibBridgeSetHostIfMode(struct DibBridgeContext *pContext, enum DibBridgeHostIfMode HostIfMode)
{
   pContext->HostIfMode = HostIfMode;
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Read a buffer independantly of the chipset and independantly of the target
****************************************************************************/
DIBSTATUS DibBridgeRead(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint8_t * Buf, uint32_t Cnt)
{
   DIBSTATUS rc = DIBSTATUS_CONTINUE;
   uint32_t  FormattedAddr;

   if(pContext->DevFailed)
      return DIBSTATUS_ERROR;

   /* Format Input Address. The 3 last paramaters are intended for access workarounds, data swapping if Write & Dragonfly-1 & SDIO 
    * Can return DIBSTATUS_SUCCESS if access as been done inside this function */
   FormattedAddr = Addr;

   if(pContext->HostIfMode != eNONE)
   {
      rc = DibBridgePreFormat(pContext, ByteMode, &FormattedAddr, 0, Buf, Cnt);
   }

   if(rc == DIBSTATUS_CONTINUE)
   {
      rc = DibBridgeTargetRead(pContext, FormattedAddr, ByteMode, Cnt, Buf);
   }

   if(rc == DIBSTATUS_SUCCESS)
   {
      if(pContext->HostIfMode != eNONE)
      {
         /* Perform data swapping if dragonfly-1 sdio */
         rc = DibBridgePostFormat(pContext, ByteMode, &FormattedAddr, 0, Buf, Cnt);
      }
   }
   else
   {
      /* Device Failed to respond */
      pContext->DevFailed = 1;
   }

   return rc;
}

/****************************************************************************
* Write a buffer independantly of the chipset and independantly of the target
****************************************************************************/
DIBSTATUS DibBridgeWrite(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint8_t * Buf, uint32_t Cnt)
{
   DIBSTATUS rc = DIBSTATUS_CONTINUE;
   uint32_t  FormattedAddr;

   if(pContext->DevFailed)
      return DIBSTATUS_ERROR;

   /* Format Input Address. The 3 last paramaters are intended for access workarounds, data swapping if Write & Dragonfly-1 & SDIO 
    * Can return DIBSTATUS_SUCCESS if access as been done inside this function */
   FormattedAddr = Addr;

   if(pContext->HostIfMode != eNONE)
   {
      rc = DibBridgePreFormat(pContext, ByteMode, &FormattedAddr, 1, Buf, Cnt);
   }

   if(rc == DIBSTATUS_CONTINUE)
   {
      rc = DibBridgeTargetWrite(pContext, FormattedAddr, ByteMode, Cnt, Buf);
   }

   if(rc == DIBSTATUS_SUCCESS)
   {
      if(pContext->HostIfMode != eNONE)
      {
         /* Perform data swapping if dragonfly-1 sdio */
         rc = DibBridgePostFormat(pContext, ByteMode, &FormattedAddr, 1, Buf, Cnt);
      }
   }
   else
   {
      /* Device Failed to respond */
      pContext->DevFailed = 1;
   }

   return rc;
}


/****************************************************************************
* Read a single word
****************************************************************************/
DIBSTATUS DibBridgeReadReg8(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t * Value)
{
   return DibBridgeRead(pContext, Addr, DIBBRIDGE_BIT_MODE_8, Value, 1);
}

/****************************************************************************
* Read a single word
****************************************************************************/
DIBSTATUS DibBridgeWriteReg8(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t Value)
{
   return DibBridgeWrite(pContext, Addr, DIBBRIDGE_BIT_MODE_8, &Value, 1);
}

/****************************************************************************
* Read a single word
****************************************************************************/
DIBSTATUS DibBridgeReadReg16(struct DibBridgeContext *pContext, uint32_t Addr, uint16_t *Value)
{
   DIBSTATUS rc;
   uint8_t   rxbuffer[2];

   rc = DibBridgeRead(pContext, Addr, DIBBRIDGE_BIT_MODE_16, rxbuffer, 2);

   if(rc == DIBSTATUS_SUCCESS)
   {
      /* Bytes are ranged in big endian order on chipset memories. */
      *Value = ((rxbuffer[0] << 8) & 0xFF00) | rxbuffer[1];
   }

   return rc;
}

/****************************************************************************
* Write a single word
****************************************************************************/
DIBSTATUS DibBridgeWriteReg16(struct DibBridgeContext *pContext, uint32_t Addr, uint16_t Value)
{
   uint8_t   txbuffer[2];

   /* Bytes are ranged in big endian order on chipset memories. */
   txbuffer[0] = (Value >> 8) & 0xFF;
   txbuffer[1] = (Value) & 0xFF;

   return DibBridgeWrite(pContext, Addr, DIBBRIDGE_BIT_MODE_16, txbuffer, 2);

}

/****************************************************************************
* Write a buffer of 32 bit values.
* Warning, the buffer is also used for swapping the bytes
****************************************************************************/
DIBSTATUS DibBridgeWriteBuffer32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t * Buffer, uint32_t Count)
{
   uint8_t * txbuffer;
   uint32_t i;
   uint32_t Value;

   txbuffer = (uint8_t *)Buffer;
   for(i = 0; i < (Count / 4); i++)
   {
      /* Bytes are ranged in big endian order on chipset memories. */
      Value = Buffer[i];
      txbuffer[0] = (uint8_t) ((Value >> 24) & 0xFF);
      txbuffer[1] = (uint8_t) ((Value >> 16) & 0xFF);
      txbuffer[2] = (uint8_t) ((Value >> 8)  & 0xFF);
      txbuffer[3] = (uint8_t) ((Value) & 0xFF);
      txbuffer += 4;
   }

   return DibBridgeWrite(pContext, Addr, DIBBRIDGE_BIT_MODE_32, (uint8_t *)Buffer, Count);
}


/****************************************************************************
* Write a single word
****************************************************************************/
DIBSTATUS DibBridgeWriteReg32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t Value)
{
   return DibBridgeWriteBuffer32(pContext, Addr, &Value, 4);
}

/****************************************************************************
* Read a buffer of 32 bit values
*****************************************************************************/
DIBSTATUS DibBridgeReadBuffer32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Buffer, uint32_t Count)
{
   DIBSTATUS rc;
   uint8_t * rxbuffer;
   uint32_t i;
   uint32_t Value, ByteMode = DIBBRIDGE_BIT_MODE_32;

#if ENG3_COMPATIBILITY == 1
   /* Force 32 bit read register access to be 8 bit ones due to sh interleaver conflict with voyager full-mask chipsets */
   if(pContext->DibChip == DIB_VOYAGER)
      ByteMode = DIBBRIDGE_BIT_MODE_8;
#endif

   rc = DibBridgeRead(pContext, Addr, ByteMode, (uint8_t*)Buffer, Count);

   if(rc == DIBSTATUS_SUCCESS)
   {
      rxbuffer = (uint8_t *)Buffer;
      for(i = 0; i < (Count / 4); i++)
      {
         /* Bytes are ranged in big endian order on chipset memories. */
         Value = ((rxbuffer[0] << 24) & 0xFF000000) |
                 ((rxbuffer[1] << 16) & 0xFF0000)   |
                 ((rxbuffer[2] << 8)  & 0xFF00)     |
                 ((rxbuffer[3])       & 0xFF);
         Buffer[i] = Value;
         rxbuffer += 4;
      }
   }

   return rc;
}

/****************************************************************************
* Read a buffer of 32 bit values in 32 bits mode
*****************************************************************************/
DIBSTATUS DibBridgeRead32Buffer32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Buffer, uint32_t Count)
{
   DIBSTATUS rc;
   uint8_t * rxbuffer;
   uint32_t i;
   uint32_t Value, ByteMode = DIBBRIDGE_BIT_MODE_32;

   rc = DibBridgeRead(pContext, Addr, ByteMode, (uint8_t*)Buffer, Count);

   if(rc == DIBSTATUS_SUCCESS)
   {
      rxbuffer = (uint8_t *)Buffer;
      for(i = 0; i < (Count / 4); i++)
      {
         /* Bytes are ranged in big endian order on chipset memories. */
         Value = ((rxbuffer[0] << 24) & 0xFF000000) |
                 ((rxbuffer[1] << 16) & 0xFF0000)   |
                 ((rxbuffer[2] << 8)  & 0xFF00)     |
                 ((rxbuffer[3])       & 0xFF);
         Buffer[i] = Value;
         rxbuffer += 4;
      }
   }
   return rc;
}

/****************************************************************************
* Write a single word
****************************************************************************/
DIBSTATUS DibBridgeReadReg32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Value)
{
   return DibBridgeReadBuffer32(pContext, Addr, Value, 4);
}

DIBSTATUS DibBridgeRead32Reg32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Value)
{
   return DibBridgeRead32Buffer32(pContext, Addr, Value, 4);
}


/**
* Swap every 16 bits words of a buffer. This is needed if the host interface of the chip requires that the data are sent
* using msb first.
*/
void DibBridgeSwap16(uint8_t * Buf, int size)
{
   uint8_t reg;
   size -= 2;

   while(size >= 0)
   {
      reg         = Buf[size];
      Buf[size]   = Buf[size+1];
      Buf[size+1] = reg;
      size       -= 2;
   }
}

/**
* Swap every 32 bits words of a buffer. This is needed if the host interface of the chip requires that the data are sent
* using msb first.
*/
void DibBridgeSwap32(uint8_t * Buf, int size)
{
   uint8_t reg;
   size -= 4;
   while(size >= 0)
   {
      reg         = Buf[size];
      Buf[size]   = Buf[size+3];
      Buf[size+3] = reg;
      reg         = Buf[size+1];
      Buf[size+1] = Buf[size+2];
      Buf[size+2] = reg;
      size       -= 4;
   }
}
