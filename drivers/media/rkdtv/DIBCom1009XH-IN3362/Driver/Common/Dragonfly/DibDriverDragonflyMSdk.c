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
* @file "DibDriverDragonfly.c"
* @brief Dragonfly Driver specific functionality.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if ( (mSDK == 1) && (USE_DRAGONFLY == 1))
#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverDowncalls.h"
#include "DibBoardSelection.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonflyErrCodes.h"
#include "DibDriverDragonfly.h"
#include "DibDriverDragonflyIntDbg.h"
#include "DibDriverDragonflyStd.h"
#include "DibDriverDragonfly.h"

/**
 * Name of the message for each ID
 */
#ifdef DIB_MSG_OUT
#undef DIB_MSG_OUT
#endif

#ifdef DIB_MSG_IN
#undef DIB_MSG_IN
#endif

#define DIB_MSG_OUT(a)
#define DIB_MSG_IN(a) #a,

/****************************************************************************
 *
 ****************************************************************************/
static void IntDriverDragonflyProcessMsgVersion(struct DibDriverContext *pContext, uint32_t * Data)
{
   PMSG_VERSION msgIn = (PMSG_VERSION)Data;

   pContext->ChipContext.EmbVersions[0] = GET_MSG_VERSION_SOFTVER(msgIn->Version);
   pContext->ChipContext.EmbVersions[1] = GET_MSG_VERSION_CHIPVER(msgIn->Version);
   pContext->ChipContext.SupportedModes = DVB_SUPPORT | DAB_SUPPORT | BBM_SUPPORT;

   DibSetEvent(&pContext->ChipContext.GlobalMonitEvent);
}

static void IntDriverDragonflyProcessMsgMonitDemod(struct DibDriverContext * pContext, uint32_t * Data)
{
   PMSG_MONIT_DEMOD msgIn = (PMSG_MONIT_DEMOD)Data;
   CHANNEL_HDL ChHdl      = GET_MONIT_DEMOD_CHANNELHDL(msgIn->MonitInfo);
   uint8_t TimeOut        = GET_MONIT_DEMOD_MONIT_STATUS(msgIn->MonitInfo);
   uint8_t DemId          = GET_MONIT_DEMOD_DEMID(msgIn->MonitInfo);
   uint8_t NbDemod        = GET_MONIT_DEMOD_NBDEM(msgIn->MonitInfo);
   uint8_t TuneMonit      = (pContext->ChannelInfo[ChHdl].Temporary == eDIB_TRUE);

   struct DibTotalDemodMonit * pTotalMonitInfo;

   /* information should be stored in a different structure where we use TuneMonit or GetChannel */
   if(TuneMonit)
      pTotalMonitInfo = &pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelMonit;
   else
      pTotalMonitInfo = &pContext->ChannelInfo[ChHdl].ChannelMonit;

   if(NbDemod != pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "MonitDemod updated: now %d demod instead of %d" CRA,
                              NbDemod, pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods));
      pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods = NbDemod;
   }

   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyProcessMsgMonitDemod DemId=%d Timeout=%d TuneMonit=%d ChHdl=%d NbDemods=%d Std=%d" CRA,
             DemId,
             TimeOut,
             TuneMonit,
             ChHdl,
             pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods,
             pContext->ChannelInfo[ChHdl].Type));

   switch(pContext->ChannelInfo[ChHdl].Type)
   {
      case eSTANDARD_DVBSH:
      case (eSTANDARD_DVBSH | eFAST):
         DibDriverDragonflyProcessMsgMonitDemodDvbSh(pContext, msgIn, pTotalMonitInfo);
         break;

      case eSTANDARD_DVB:
      case eSTANDARD_DVB | eFAST:
         DibDriverDragonflyProcessMsgMonitDemodDvb(pContext, msgIn, pTotalMonitInfo);
         break;

      case eSTANDARD_ISDBT:
      case eSTANDARD_ISDBT_1SEG:
         DibDriverDragonflyProcessMsgMonitDemodIsdbt(pContext, msgIn, pTotalMonitInfo);
         break;

      case eSTANDARD_CMMB:
         DibDriverDragonflyProcessMsgMonitDemodCmmb(pContext, msgIn, pTotalMonitInfo);
         break;

      case eSTANDARD_CTTB:
         DibDriverDragonflyProcessMsgMonitDemodCttb(pContext, msgIn, pTotalMonitInfo);
         break;

      default:
         DIB_DEBUG(MSG_ERR, (CRB "Standard not supported %d" CRA, pContext->ChannelInfo[ChHdl].Type));
         break;
   }


   if(DemId == pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods - 1)
   {
      uint32_t TmpExp, TmpDemId;
      pTotalMonitInfo->Type = pContext->ChannelInfo[ChHdl].Type;

      /** nb demodulation really fill in the demod structure */
      pTotalMonitInfo->NbDemods = pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods;

      /** General Monit Status */
      TmpExp=0; /* number of timeout */
      for(TmpDemId=0; TmpDemId < pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods; TmpDemId++)
      {
         uint8_t IntTimeout = 0;
         switch(pContext->ChannelInfo[ChHdl].Type)
         {
            case eSTANDARD_DVBSH:
            case (eSTANDARD_DVBSH | eFAST):
               IntTimeout = DibDriverDragonflyGetTimeoutDvb(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_DVB:
               IntTimeout = DibDriverDragonflyGetTimeoutDvbSh(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_CMMB:
               IntTimeout = DibDriverDragonflyGetTimeoutCmmb(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_CTTB:
               IntTimeout = DibDriverDragonflyGetTimeoutCttb(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_ISDBT:
            case eSTANDARD_ISDBT_1SEG:
               IntTimeout = DibDriverDragonflyGetTimeoutIsdbt(pTotalMonitInfo, TmpDemId);
               break;

            default:
               DIB_DEBUG(MSG_ERR, (CRB "Standard not supported %d" CRA, pContext->ChannelInfo[ChHdl].Type));
               break;
         }

         if(IntTimeout)
            TmpExp++;
      }

      /* configure general monit status */
      if(TuneMonit && (pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor.ScanStatus != DIB_SCAN_SUCCESS))
      {
         /* Monitoring fails if scanning has failed (Fixed) */
         pTotalMonitInfo->MonitStatus = pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor.ScanStatus; /* Failure, abort, invalid param */
      }
      else if(TmpExp == 0)
      {
         pTotalMonitInfo->MonitStatus = DIB_MONIT_RELIABLE;
      }
      else if(TmpExp == pTotalMonitInfo->NbDemods)
      {
         pTotalMonitInfo->MonitStatus = DIB_MONIT_TIMEOUT;
      }
      else
      {
         pTotalMonitInfo->MonitStatus = DIB_MONIT_PARTIAL;
      }

      /** nb demodulation for this channel in the actual configuration. shld be >= of pTotalMonitInfo->NbDemods */
      pTotalMonitInfo->ChannelIndex = ChHdl;
      pTotalMonitInfo->MonitoringVersion++;

      DIB_DEBUG(SOFT_IF_LOG, (CRB "MonitDemod Complete with MonitStatus %d" CRA, pTotalMonitInfo->MonitStatus));

      if(TuneMonit && (pContext->ChannelInfo[ChHdl].MonitContext.pTuneMonitCb))
      {
         DibTuneMonitCallback CallBack = pContext->ChannelInfo[ChHdl].MonitContext.pTuneMonitCb;

         CallBack(&pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit, pContext->ChannelInfo[ChHdl].MonitContext.pTuneMonitCtx);

         DIB_ASSERT(pContext->ChannelInfo[ChHdl].InUse);

         IntDriverRemoveChannel(pContext, ChHdl, eDIB_TRUE);

         /* Free monitoring channel */
         DibReleaseLock(&pContext->MonitLock);
      }
      else if(!TuneMonit && (pContext->ChannelInfo[ChHdl].pGetSignalCb))
      {
         DibGetSignalCallback CallBack = pContext->ChannelInfo[ChHdl].pGetSignalCb;

         CallBack(pTotalMonitInfo, &(pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods), pContext->ChannelInfo[ChHdl].pGetSignalCtx);

         /* Free signal monit lock */
         DibReleaseLock(&pContext->SignalMonitLock);
      }
      else
      {
         /* synchronous - lock release by caller */
         DIB_DEBUG(MSG_LOG, (CRB "  RESET EVENT" CRA));
         DibSetEvent(&pContext->ChannelInfo[ChHdl].MonitContext.MonitEvent);
      }
   }
   else
   {
      DIB_DEBUG(MSG_LOG, (CRB "Waiting for another DEMOD_INFO" CRA));
   }
}



/****************************************************************************
*
****************************************************************************/
static void IntDriverDragonflyTestMsgChecker(struct DibDriverContext *pContext, uint32_t * Data)
{
   /* Proprietary message between bridge and driver */
   PMSG_CHECKER      pMsg        = (PMSG_CHECKER)Data;
   ELEM_HDL          ItemHdl     = pMsg->ItemHdl;
   FILTER_HDL        FilterHdl;
   enum DibDataType  FilterType;
   struct DibDriverMpeIFecServiceCtx *pMpeIFec;
   struct DibDriverMpeServiceCtx     *pMpe;
   struct DibDriverFilter            *pFilter;

   DIB_ASSERT(Data);

   /* Item has been removed since last call*/
   if(ItemHdl == DIB_UNSET)
      return;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);

   FilterHdl   = pContext->ItemInfo[ItemHdl].ParentFilter;
   pFilter     = &pContext->FilterInfo[FilterHdl];
   FilterType  = pFilter->TypeFilter;

   switch (FilterType)
   {
      case eMPEFEC:
         pMpe = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService;
         if(pMpe != NULL)
         {
            DIB_DEBUG(MSG_LOG, (CRB "Received IP checker monitoring for item %d" CRA, ItemHdl));
            /* Total information is retreived from IN_MSG_MPE_MONITOR */
            pMpe->DvbHMonitoring.CcFailCnt      = pMsg->CcFailCnt;
            pMpe->DvbHMonitoring.ErrCnt         = pMsg->ErrCnt;
         }
         break;

      case eMPEIFEC:
         pMpeIFec = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeIFecService;
         if(pMpeIFec != NULL)
         {
            DIB_DEBUG(MSG_LOG, (CRB "Received IP checker monitoring for item %d" CRA, ItemHdl));
            /* Total information is retreived from IN_MSG_MPE_IFEC_MONITOR */
            pMpeIFec->DvbSHMonitoring.CcFailCnt      = pMsg->CcFailCnt;
            pMpeIFec->DvbSHMonitoring.ErrCnt         = pMsg->ErrCnt;
         }
         break;

      case eTS:
         DIB_DEBUG(MSG_LOG, (CRB "Received RAWTS checker monitoring for item %d" CRA, ItemHdl));
         /* Total information is retreived from IN_MSG_RAWTS_MONITOR */
         pFilter->DvbTMonitoring.CheckTotalPackets = pMsg->Total;
         pFilter->DvbTMonitoring.CheckCcFailCnt    = pMsg->CcFailCnt;
         pFilter->DvbTMonitoring.CheckErrCnt       = pMsg->ErrCnt;
         break;
      default:
         DIB_DEBUG(MSG_LOG, (CRB "Received unknown debug monitoring for item %d" CRA, ItemHdl));
         break;
   };

}

/****************************************************************************
*
****************************************************************************/
static void IntDriverDragonflyInternalAccessDone(struct DibDriverContext *pContext, uint32_t * Data)
{
   PMSG_INTERNAL_ACCESS pMsg = (PMSG_INTERNAL_ACCESS) Data;

   pContext->ChipContext.APBBridgeAddressRead = pMsg->Addr;
   pContext->ChipContext.APBBridgeDataRead    = pMsg->Value;

   DIB_DEBUG(MSG_LOG, (CRB "-IN_MSG_INTERNAL_ACCESS: Addr: %d, Val: %d" CRA, pMsg->Addr, pMsg->Value));
   DibSetEvent(&pContext->BridgeRWEvent);
}

/**
 * Receive an API message. Forward the buffer to the host using API message driver context
 * @param pContext   pointer to current context
 * @param pData      pointer to the received message
 */
void IntDriverDragonflyProcessMsgApi(struct DibDriverContext * pContext, uint32_t * pData)
{
   struct DibDriverDmaCtx DmaCtx;
   PMSG_API_ANS pMsg = (PMSG_API_ANS)pData;

   if(*pContext->ApiMsgCtx.pSize >= pMsg->BufferSize)
   {
      if(pMsg->BufferSize)
      {
         DmaCtx.DmaLen        = pMsg->BufferSize;
         DmaCtx.ChipBaseMin   = 0 /* not used */;
         DmaCtx.ChipBaseMax   = 0 /* not used */;
         DmaCtx.ChipAddr      = pMsg->BufferAddr;
         DmaCtx.pHostAddr     = pContext->ApiMsgCtx.pBuffer;
         DmaCtx.Dir           = DIBDRIVER_DMA_READ;
         DmaCtx.Mode          = DIBBRIGDE_BIT_MODE_8;
         DibDriverTargetEnableIrqProcessing(pContext);
         DibD2BTransfertBuf(pContext, &DmaCtx);
         DibDriverTargetDisableIrqProcessing(pContext);
      }
      if(pMsg->Status == MSG_API_SUCCESS)
      {
         pContext->ApiMsgCtx.Status = DIBSTATUS_SUCCESS;
      }
      else
      {
         DIB_DEBUG(MSG_LOG, (CRB "  IntDriverDragonflyProcessMsgApi: Communication failure %d" CRA, pMsg->Status));
         pContext->ApiMsgCtx.Status = DIBSTATUS_ERROR;
      }
   }
   else
   {
      DIB_DEBUG(MSG_LOG, (CRB "  IntDriverDragonflyProcessMsgApi: Buffer size =%d, needed %d" CRA, *pContext->ApiMsgCtx.pSize, pMsg->BufferSize));
      pContext->ApiMsgCtx.Status = DIBSTATUS_INSUFFICIENT_MEMORY;
   }

   *pContext->ApiMsgCtx.pSize = pMsg->BufferSize;

   pContext->MsgAckStatus = DIBSTATUS_SUCCESS;
   DibSetEvent(&pContext->MsgAckEvent);
}

#if (DEBUG_MODE == DEBUG_PRINT)
char * MsgIdToString(uint32_t MsgId)
{
   switch(MsgId)
   {
   case IN_MSG_ACK_INIT:
      return "IN_MSG_ACK_INIT";
   case IN_MSG_ACK_DEL_CH:
      return "IN_MSG_ACK_DEL_CH";
   case IN_MSG_ACK_CREATE_FILT:
      return "IN_MSG_ACK_CREATE_FILT";
   case IN_MSG_ACK_DEL_FILT:
      return "IN_MSG_ACK_DEL_FILT";
   case IN_MSG_ACK_ADD_TO_FILT:
      return "IN_MSG_ACK_ADD_TO_FILT";
   case IN_MSG_ACK_FREE_ITEM:
      return "IN_MSG_ACK_FREE_ITEM";
   case IN_MSG_ACK_CAS_EN:
      return "IN_MSG_ACK_CAS_EN";
   default:
      return "IN_MSG_UNK";
   }
}
#endif

/****************************************************************************
 * Display a string encapsulated in the message by the firmware
 ****************************************************************************/
void IntDriverDragonflyProcessMsgPrint(struct DibDriverContext *pContext, uint32_t RxCnt, uint32_t * Data)
{
   static uint8_t Buffer[0x81];
   uint32_t i, j, word, Addr;

   j = 0;

   for(i=1; i<RxCnt; i++)
   {
      word       = Data[i];
      Buffer[j++]= word >> 24;
      Buffer[j++]= word >> 16;
      Buffer[j++]= word >> 8;
      Buffer[j++]= word;
   }
   Buffer[j++] = 0;

   DibDriverTargetLogFunction("%s",Buffer);
}


/****************************************************************************
 * This function is called when a interrupt shows a message was sent to
 * the host by one (or both) RISC(s).
 *
 * It handles inbound Data messages, debug messages, ...
 *
 * This function does not contain any locking function, as it is only called
 * under an interruption context.
 ****************************************************************************/
static void IntDriverDragonflyProcessMsg(struct DibDriverContext *pContext, uint32_t Size, uint16_t * short_data)
{
   uint32_t *Data = (uint32_t*)short_data;
   uint16_t  msg_id;
   uint32_t  rx_nb;

   /* retrieve information from msg */
   msg_id = GET_MSG_ID(*Data);

   /* retreive the Size of the message, in 32 bit word, without he header */
   rx_nb = (Size >> 2);

   switch (msg_id)
   {
   case IN_MSG_VERSION:
      if(rx_nb < (sizeof(MSG_VERSION)/4))
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 1));
      } 
      else
      {
         IntDriverDragonflyProcessMsgVersion(pContext, Data);
      }
      break;

   case IN_MSG_PRINT:
      /* int debug */
      IntDriverDragonflyProcessMsgPrint(pContext, rx_nb, Data);
      break;

   case IN_MSG_API_ANS:
      IntDriverDragonflyProcessMsgApi(pContext, Data);
      break;

   default:
      DIB_DEBUG(MSG_ERR, (CRB "  IntDriverDragonflyProcessMsg: ERROR: unknown msg_id %d" CRA, msg_id));
      break;
   }
}


/****************************************************************************
 * Requests that the demod is initilized by the embedded software
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyInitDemod(struct DibDriverContext *pContext, uint16_t FeId)
{
   /* TODO */
   return DIBSTATUS_SUCCESS;
}


/****************************************************************************
 * Select a specific Driver Configuration
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflySetCfg(struct DibDriverContext *pContext, uint32_t Cfg)
{
   DIBSTATUS         Status = DIBSTATUS_ERROR;

   return Status;
}


/****************************************************************************
 * Select a specific Driver Configuration
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyDeInitFirmware(struct DibDriverContext *pContext)
{
   DIBSTATUS         Status = DIBSTATUS_ERROR;

   return Status;
}


/****************************************************************************
 * Sends a "get monit" command
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyGetMonit(struct DibDriverContext *pContext, uint8_t ChannelHdl)
{
   return DIBSTATUS_SUCCESS; 
}

/****************************************************************************
 * Requests the embedded softwares send their versions
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyGetEmbeddedInfo(struct DibDriverContext *pContext)
{
   struct MsgHeader Msg;

   /* Message header */
   Msg.Type      = MSG_TYPE_MAC;
   Msg.MsgId     = OUT_MSG_REQ_VERSION;
   Msg.MsgSize   = 1;
   Msg.ChipId    = MASTER_IDENT;

   MsgHeaderPackInit(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, 4);
}

/**
 * Send a message to create a new channel and get the new handler, if successful.
 * @param pContext current context
 * @param MinNbDemod       min number of demodulation for this channel
 * @param MaxNbDemod       max number of demodulation for this channel
 * @param PriorityChannel  indication if this channel will be the priority one or not
 * @param pDescriptor      pointer to a structure with all the useful information for the configuration of the channel
 * @param pChannelHdl      pointer to the result channel (chosed by the firmware in case of dragonfly), if successful
 * @return DIBSTATUS result of the command
 */
static DIBSTATUS IntDriverDragonflyGetChannel(struct DibDriverContext * pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibChannel * pDescriptor, CHANNEL_HDL * pChannelHdl, DibGetChannelCallback pCallBack, void *pCallCtx)
{
   DIBSTATUS     ret = DIBSTATUS_ERROR;
   MSG_CREATE_CH msg;
   struct DibTuneChan * pTc = &(pDescriptor->ChannelDescriptor);
   CHANNEL_HDL ii = 0;
   CHANNEL_HDL ChannelHdl = 0;
   
   /** channel handler is invalid until proben. It remains invalid in async mode until successful scan */
   *pChannelHdl = DIB_UNSET;

   /** Take the first free channel : the channel number is choosed here and not in the firefly, in contrario of the dragonfly */
   for(ChannelHdl = 0; ChannelHdl < DIB_MAX_NB_CHANNELS; ChannelHdl++)
   {
      if(pContext->ChannelInfo[ChannelHdl].InUse == 0)
         break;
   }

   if(ChannelHdl == DIB_MAX_NB_CHANNELS)
   {
      /** no more channel avalaible */
      return DIBSTATUS_RESOURCES;
   }

   /* We copy the given information to the context structure. The scan() and tune()
   * functions will need them */
   DibMoveMemory(&pContext->ChannelInfo[ChannelHdl].ChannelDescriptor, &pDescriptor->ChannelDescriptor, sizeof(struct DibTuneChan));
   pContext->ChannelInfo[ChannelHdl].Type = pDescriptor->Type;

   switch(pDescriptor->Type)
   {
   case eSTANDARD_DVB:
   case (eSTANDARD_DVB | eFAST):
      ret = DibDriverDragonflySetMsgTuneDvb(pTc, &msg);
      break;

   case eSTANDARD_ISDBT:
   case eSTANDARD_ISDBT_1SEG:
      ret = DibDriverDragonflySetMsgTuneIsdbt(pTc, &msg);
      break;

   case eSTANDARD_CTTB:
      ret = DibDriverDragonflySetMsgTuneCttb(pTc, &msg);
      break;

   default:
      return DIBSTATUS_INVALID_PARAMETER;
      break;
   }

   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.RFkHz = pTc->RFkHz;
   pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.Bw = pTc->Bw;
   pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.StreamParameters = StreamParameters;

   DibResetEvent(&pContext->MsgAckEvent);

   ret = DibDriverSendMessage(pContext, (uint32_t*)&msg, sizeof(MSG_CREATE_CH));

   /* Send Message failed */
   if(ret != DIBSTATUS_SUCCESS)
      return ret;


   ret = IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT_SETCFG);
   if(ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyGetChannel: Call to firmware failed" CRA));
   }
   else
   {
      if(pContext->MsgAckStatus == DIBSTATUS_ERROR)
      {
         if(pContext->EndOfScanStatus == DIB_SCAN_TIMEOUT)
         {
            DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyGetChannel: Timeout on Event" CRA));
            ret = DIBSTATUS_TIMEOUT;
         }
         else
         {
            DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyGetChannel: Error waiting Event %d" CRA, pContext->EndOfScanStatus));
            ret = DIBSTATUS_ERROR;
         }
      }
      else /* pContext->MsgAckStatus == DIBSTATUS_SUCCESS */
      {
         pTc = &(pContext->ChannelInfo[ChannelHdl].ChannelDescriptor);
         if(pTc->ScanStatus != DIB_SCAN_SUCCESS)
         {
            if(pTc->ScanStatus == DIB_SCAN_TIMEOUT)
            {
               DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyGetChannel: scan timeout: firmware answered it could not process request" CRA));
               ret = DIBSTATUS_TIMEOUT;
            }
            else
            {
               IntDriverRemoveChannel(pContext, (CHANNEL_HDL)ChannelHdl, eDIB_FALSE);
               ret = DIBSTATUS_ERROR;
            }
         }
         else
         {
            /** Success : add the new channel in the context */
            DibMoveMemory(&(pDescriptor->ChannelDescriptor), &pContext->ChannelInfo[ChannelHdl].ChannelDescriptor, sizeof(struct DibTuneChan));

            IntDriverAddChannel(pContext, (CHANNEL_HDL)ChannelHdl, MinNbDemod, MaxNbDemod, StreamParameters, pDescriptor->Type, &(pDescriptor->ChannelDescriptor));
            pContext->ChannelInfo[ChannelHdl].Temporary = eDIB_FALSE;

            for(ii = 0; ii < DIB_MAX_NB_CHANNELS; ii++)
            {
               if((pContext->ChannelInfo[ii].IsDataChannel == eDIB_TRUE) && (pContext->ChannelInfo[ii].StreamParameters == StreamParameters))
                  break;
            }
            if(ii == DIB_MAX_NB_CHANNELS)
            {
               pContext->ChannelInfo[ChannelHdl].IsDataChannel = eDIB_TRUE;
            }
            else
            {
               pContext->ChannelInfo[ChannelHdl].IsDataChannel = eDIB_FALSE;
            }

            ret = DIBSTATUS_SUCCESS;

            *pChannelHdl = ChannelHdl;
         }
      }
   }

   return ret;
}

/**
 * Send a message to delete a channel (and all the associated filters) in the firmware.
 * @param pContext pointer to the current context
 * @param ChannelHdl Handler of the channel that the user want to delete
 */
static DIBSTATUS IntDriverDragonflyDeleteChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl)
{
   /** Send the message */
   struct MsgDeleteChannel Msg;
   uint32_t buffer[MsgDeleteChannelWords(0)];
   DIBSTATUS  result;
   uint8_t    fi;

   Msg.Head.ChipId = MASTER_IDENT;
   Msg.Head.MsgId = OUT_MSG_REQ_DEL_CH;
   Msg.Head.MsgSize = MsgDeleteChannelWords(0);
   Msg.Head.Type = MSG_TYPE_MAC;
   Msg.ChannelHdl = ChannelHdl;
   MsgDeleteChannelPack(&Msg, buffer, 0);

   /** before : delete filter */
   fi = pContext->ChannelInfo[ChannelHdl].FirstFilter;

   DibResetEvent(&pContext->MsgAckEvent);
   if((result = DibDriverSendMessage(pContext, buffer, sizeof(buffer))) == DIBSTATUS_SUCCESS)
   {
      result = IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT);
   }

   if(result == DIBSTATUS_SUCCESS)
   {
      IntDriverRemoveChannel(pContext, ChannelHdl, eDIB_FALSE);

      return DIBSTATUS_SUCCESS;
   }
   else
   {
      if(result == DIBSTATUS_TIMEOUT)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflydelete_channel: Timeout on Event" CRA));
         return DIBSTATUS_TIMEOUT;

      }
      else
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflydelete_channel: Error waiting Event" CRA));
         return DIBSTATUS_ERROR;
      }
   }
}

void IntDriverDragonflyEndOfScan(struct DibDriverContext * pContext, uint32_t * pData)
{
   pContext->MsgAckStatus = DIBSTATUS_SUCCESS;
   DibSetEvent(&pContext->MsgAckEvent);
}

/**
* Change the strategy of power of the chip
* @param pContext   pointer to the context of the driver
* @param TimeSlicing   Type of power management
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverDragonflySetPowerManagement(struct DibDriverContext * pContext, enum DibBool TimeSlicing, uint32_t PhysicalStream)
{
   DIBSTATUS             ret = DIBSTATUS_ERROR;
   return ret;
}

/****************************************************************************
 * Configures the GPIOs
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflySetCfgGpio(struct DibDriverContext *pContext, struct DibDemodGpioFn *pConfig, uint16_t FeId)
{

  return DIBSTATUS_SUCCESS;
}

/******************************************************************************
 * IntDriverDragonflysubband_select
 ******************************************************************************/
static DIBSTATUS IntDriverDragonflySubbandSelect(struct DibDriverContext *pContext, struct DibDemodSubbandSelection *pConfig, uint16_t FeId)
{
   /* TODO */
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
 * Enables/Disables ts output
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflySetOutputMode(struct DibDriverContext *pContext, enum DibOutputMode Mode)
{
   /* TODO */
   return DIBSTATUS_SUCCESS;
}

/**
 * DibDriverSendMsgApi
 */
DIBSTATUS IntDriverDragonflySendMsgApi(struct DibDriverContext *pContext, uint16_t MsgType, uint16_t MsgIndex, uint8_t *pSendBuf, uint32_t SendLen, uint8_t *pRecvBuf, uint32_t * pRecvLen)
{
   struct DibDriverDmaCtx DmaCtx;
   DIBSTATUS   ret = DIBSTATUS_ERROR;
   struct MsgApiRequest Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_API_REQ;
   Msg.Head.MsgSize   = GetWords(MsgApiRequestBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;

   /* while there is no remote allocation mechanism, we always transfer user data to a fixed buffer in the fimware */
   Msg.ReqType        = MsgType;
   Msg.ReqId          = MsgIndex;
   Msg.TxLen          = SendLen;
   Msg.TxAddr         = pContext->DragonflyRegisters.TxRxBufAddr;

   /* send the data is exists */
   if(Msg.TxLen > 0)
   {
      DmaCtx.DmaLen        = Msg.TxLen;
      DmaCtx.ChipBaseMin   = 0 /* not used */;
      DmaCtx.ChipBaseMax   = 0 /* not used */;
      DmaCtx.ChipAddr      = Msg.TxAddr;
      DmaCtx.pHostAddr     = pSendBuf;
      DmaCtx.Dir           = DIBDRIVER_DMA_WRITE;
      DmaCtx.Mode          = DIBBRIGDE_BIT_MODE_8;
      DibD2BTransfertBuf(pContext, &DmaCtx);
   }

   /* format the message to TxBuffer */
   MsgApiRequestPackInit(&Msg, &pContext->TxSerialBuf);

   /* uppon acknowledgement, a reply buffer may be present and need to be copied to the user buffer */
   pContext->ApiMsgCtx.pSize   = pRecvLen;
   pContext->ApiMsgCtx.pBuffer = pRecvBuf;

   DibResetEvent(&pContext->MsgAckEvent);
   if((ret = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4)) == DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(MSG_LOG, (CRB "  IntDriverDragonflySendMsgApi Type %d Idx %d SendLen %d" CRA, MsgType, MsgIndex, SendLen));

      /** Wait for the answer */
      ret = IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT);
   }

   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   return pContext->ApiMsgCtx.Status;
}

/******************************************************************************
 * dibOutMessageOps::msgFireflyOps
 * Specific output message formating for dragonfly
 ******************************************************************************/
void DibDriverDragonflyMSdkRegisterChip(struct DibDriverContext *pContext)
{
   /* specific architecture function */
   switch(pContext->DibChip) {

      case DIB_VOYAGER:
         pContext->ChipOps.MacResetCpu     = IntDriverVoyager1ResetCpu;
         break;

      case DIB_NAUTILUS:
         pContext->ChipOps.MacResetCpu     = IntDriverNautilus1ResetCpu;
         break;

      default:
         break;
   }

    /* Dragonfly registers */
    DibMoveMemory(&pContext->DragonflyRegisters,
                  pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib9000.Mapping,
                  sizeof(struct DibDriverDragonflyRegisters));

    /* mac functions */
    pContext->ChipOps.MacInitCpu            = IntDriverDragonflyInitCpu;
    pContext->ChipOps.MacStartCpu           = IntDriverDragonflyStartCpu;
    pContext->ChipOps.MacUploadMicrocode    = IntDriverDragonflyUploadMicrocode;
    pContext->ChipOps.MacInitMailbox        = IntDriverDragonflyInitMailbox;
    pContext->ChipOps.MacTestRamInterface   = NULL;
    pContext->ChipOps.MacEnableClearIrq     = IntDriverDragonflyEnableClearIrq;

    /* register_if functions */
    pContext->ChipOps.RegisterIfInit        = IntDriverDragonflyRegisterIfInit;
    pContext->ChipOps.WriteDemod            = NULL;
    pContext->ChipOps.ReadDemod             = NULL;

    /* packet demux */
    pContext->ChipOps.GetSiPsiInfo          = NULL;

    /* frontend functions */
    pContext->ChipOps.FrontendInit          = IntDriverDragonflyFrontendInit;
    pContext->ChipOps.FrontendRfTotalGain   = NULL;

    /* handler for input messages */
    pContext->ChipOps.ProcessMessage        = IntDriverDragonflyProcessMsg;

    /* output message functions */
    pContext->ChipOps.SetDebugHelp          = NULL;
    pContext->ChipOps.ApbWrite              = NULL;
    pContext->ChipOps.ApbRead               = NULL;
    pContext->ChipOps.I2CWrite              = NULL;
    pContext->ChipOps.I2CRead               = NULL;
    pContext->ChipOps.AbortTuneMonitChannel = NULL;
    pContext->ChipOps.TuneMonitChannel      = NULL;
    pContext->ChipOps.InitDemod             = IntDriverDragonflyInitDemod;
    pContext->ChipOps.GetMonit              = IntDriverDragonflyGetMonit;
    pContext->ChipOps.GetEmbeddedInfo       = IntDriverDragonflyGetEmbeddedInfo;
    pContext->ChipOps.SetCfgGpio            = IntDriverDragonflySetCfgGpio;
    pContext->ChipOps.SubbandSelect         = IntDriverDragonflySubbandSelect;
    pContext->ChipOps.SetHbm                = NULL;
    pContext->ChipOps.EnableCas             = NULL;
    pContext->ChipOps.SetOutputMode         = IntDriverDragonflySetOutputMode;
    pContext->ChipOps.SendMsgApi            = IntDriverDragonflySendMsgApi;

    /* new api */
    pContext->ChipOps.GetChannel            = IntDriverDragonflyGetChannel;
    pContext->ChipOps.DeleteChannel         = IntDriverDragonflyDeleteChannel;
    pContext->ChipOps.CreateFilter          = NULL;
    pContext->ChipOps.DeleteFilter          = NULL;
    pContext->ChipOps.AddItem               = NULL;
    pContext->ChipOps.RemoveItem            = NULL;
    pContext->ChipOps.SetPowerManagement    = IntDriverDragonflySetPowerManagement;
    pContext->ChipOps.SetPidMode            = NULL;
    pContext->ChipOps.DwldSlaveFw           = NULL;
    pContext->ChipOps.ClearMonit            = NULL;
    pContext->ChipOps.SetCfg                = IntDriverDragonflySetCfg;
    pContext->ChipOps.DeInitFirmware        = IntDriverDragonflyDeInitFirmware;
}

#endif /* mSDK */
