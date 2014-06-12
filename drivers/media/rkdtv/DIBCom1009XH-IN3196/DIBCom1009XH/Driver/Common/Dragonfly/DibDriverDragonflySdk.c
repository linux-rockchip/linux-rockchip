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

#if 1 /*( (mSDK == 0) && (USE_DRAGONFLY == 1))*/
#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibMsgApi.h"
#include "DibDriver.h"
#include "DibDriverDowncalls.h"
#include "DibBoardSelection.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"
#include "DibDriverDragonflyIntDbg.h"
#include "DibDriverDragonflyData.h"
#include "DibDriverDragonflyStd.h"
#include "DibFirmwareSelection.h"
#include "DibDriverDragonfly.h"
//#include <stdio.h>

#define MAX_IV_SIZE   16
#define MAX_KEY_SIZE  32


void IntDriverDragonflyTransferFirmware(struct DibDriverContext *pContext);
static DIBSTATUS IntDriverDragonflyDeleteChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl);

#if (WRITE_FLASH_SUPPORT != eWRFL_NONE)
void IntDriverDragonflyFlashProgramDone(struct DibDriverContext * pContext, uint32_t * pData);
#endif /* WRITE_FLASH_SUPPORT */

/**
 * DibDriverFrontendRfTotalGain
 */
static uint32_t IntDriverDragonflyFrontendRfTotalGain(uint8_t LnaStatus, uint32_t agc_global, struct DibDriverContext *pContext)
{
   return 0;
}
/**
 * Unpack stream parameters ad check for coherency
 */
static DIBSTATUS IntDriverDragonflyCheckStreamParameters(uint32_t StreamParameters, uint32_t * StreamType, uint32_t * StreamOptions)
{
   *StreamType = StreamParameters & 0xFFFF;

   switch(*StreamType)
   {
      case eSTREAM_TYPE_HOST_DMA:
         *StreamOptions = 0;
         return DIBSTATUS_SUCCESS;
      case eSTREAM_TYPE_MPEG_1:
      case eSTREAM_TYPE_MPEG_2:
         *StreamOptions = (StreamParameters >> 16) & 0x7;
         return DIBSTATUS_SUCCESS;
      default:
         return DIBSTATUS_INVALID_PARAMETER;
   }
}


/****************************************************************************
*
****************************************************************************/
static void IntDriverDragonflyProcessMsgError(struct DibDriverContext *pContext, uint32_t *Data)
{
   struct MsgError msgIn;

   MsgErrorUnpack(&pContext->RxSerialBuf, &msgIn);

   DIB_DEBUG(MSG_ERR, (CRB "****** FW ERROR (%d) =>", msgIn.ErrorCode));

   switch(msgIn.ErrorCode)
   {
   case DFERR_TSQ_OVERFLOW:
      DIB_DEBUG(MSG_ERR, ("TS queue (%d) overflow" CRA, msgIn.Param[0]));
      break;

   case DFERR_QMDMA_CTX_FULL:
      DIB_DEBUG(MSG_ERR, ("Memory Dma Context Full (%d %d) " CRA, msgIn.Param[0], msgIn.Param[1]));
      break;

   case DFERR_QSDMA_CTX_FULL:
      DIB_DEBUG(MSG_ERR, ("Stream Dma Context Full (%d %d) " CRA, msgIn.Param[0], msgIn.Param[1]));
      break;

   case DFERR_QGDMA_CTX_FULL:
      DIB_DEBUG(MSG_ERR, ("General Dma Context Full (%d %d) " CRA, msgIn.Param[0], msgIn.Param[1]));
      break;

   case DFERR_QFEC_CTX_FULL:
      DIB_DEBUG(MSG_ERR, ("Fec Context Full (%d %d) " CRA, msgIn.Param[0], msgIn.Param[1]));
      break;

   case DFERR_IFEC_FILE_LOOP:
      DIB_DEBUG(MSG_ERR, ("MPE-IFEC Error - Desyncronisation of bursts indexes reference, number of lost and received bursts are not accurate (%d %d) " CRA, msgIn.Param[0], msgIn.Param[1]));
      break;

   case DFERR_IFEC_MISEVAL:
      DIB_DEBUG(MSG_ERR, ("MPE-IFEC Warning - Desyncronisation of burst time reference (%d %d) " CRA, msgIn.Param[0], msgIn.Param[1]));
      break;

   case DFERR_IFEC_RESOURCES:
      DIB_DEBUG(MSG_ERR, ("MPE-IFEC Error - not enough memory to allocate burst (%d %d) D: %d" CRA, msgIn.Param[1], msgIn.Param[2], msgIn.Param[3]));
      break;

   default:
      DIB_DEBUG(MSG_ERR, ("P1: %d P2: %d P3: %d P4: %d" CRA, msgIn.Param[0], msgIn.Param[1], msgIn.Param[2], msgIn.Param[3]));
      break;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void IntDriverDragonflyProcessMsgVersion(struct DibDriverContext *pContext, uint32_t * Data)
{
   struct MsgVersion    msgIn;
   int32_t i;

   MsgVersionUnpack(&pContext->RxSerialBuf, &msgIn);

   pContext->ChipContext.EmbVersions[0]   = msgIn.SoftVer;
   pContext->ChipContext.EmbVersions[1]   = msgIn.ChipVer;
   pContext->ChipContext.HalVersion       = msgIn.HalVer;
   for (i = 0; i < 8; i++)
       pContext->ChipContext.HalVersionExt[i] = msgIn.HalVerExt[i];
   pContext->ChipContext.HalVersionExt[8] = '\0';

   pContext->ChipContext.SupportedModes = DVB_SUPPORT | DAB_SUPPORT | BBM_SUPPORT;

   DibSetEvent(&pContext->ChipContext.GlobalMonitEvent);
}

static uint32_t WithOneSegStandard(uint32_t Standards)
{
   /* Add 1SEG support each time isdbt is supported */
   if(Standards & (1 << eSTANDARD_ISDBT))
      Standards |= (1 << eSTANDARD_ISDBT_1SEG);

   return Standards;
}


/****************************************************************************
 *
 ****************************************************************************/
static struct DibFrontendPossibleInput *
	IntDriverDragonflyFindPossibleInput(struct DibFrontendPossibleInput List[], uint32_t FeId)
{
	int32_t i;
	for (i = 0; i < DIB_MAX_NB_OF_FRONTENDS; i++) {
		if (List[i].Fe == NULL)
			return &List[i];
		else {
			struct DibFrontend *Fe = List[i].Fe;
			if (Fe->FeId == FeId)
				return &List[i];
		}
	}
	return NULL;
}

static struct DibFrontend *
	IntDriverDragonflyFindFrontend(struct DibDriverContext *pContext, uint32_t FeId)
{
	int32_t i;
	for (i = 0; i < DIB_MAX_NB_OF_FRONTENDS; i++) {
		if (pContext->Frontend[i].FeId == FeId)
			return &pContext->Frontend[i];
	}

	return NULL;
}

static void IntDriverDragonflyProcessStreamInfo(struct DibDriverContext *pContext, uint32_t * Data)
{
   struct MsgStreamInfo MsgIn;
   struct DibStream * pStream;
   struct DibStreamAttributes * pSA;
   struct StreamInfo * StrInf = &MsgIn.StreamInfo;
   uint32_t Input;

   MsgStreamInfoUnpack(&pContext->RxSerialBuf, &MsgIn);
   DIB_DEBUG(FRONTEND_LOG,(CRB "IntDriverDragonflyProcessStreamInfo : StreamNum %d NumOfInputs %d ChipId %d PowerMode %d %04x" CRA,
            MsgIn.StreamNum,
            StrInf->NumOfInputs,
            StrInf->ChipId,
            StrInf->PowerMode,
            StrInf->StreamId
            ));

   pStream = &pContext->Stream[MsgIn.StreamNum];

   pStream->StreamId = StrInf->StreamId;
   pStream->Attributes.Type = StrInf->StreamType;
   pStream->Attributes.ChipId = StrInf->ChipId;
   pStream->Attributes.Options = StrInf->Options;
   pStream->Attributes.Standards = WithOneSegStandard(StrInf->SupportedStandards); /* Add 1SEG support each time isdbt is supported */
   pStream->ChipTimeSlicing = (StrInf->PowerMode == 1) ? 1 : 0 ;

   /* NULL termination */
   DibZeroMemory(pStream->Attributes.PossibleDirectInputs, sizeof(pStream->Attributes.PossibleDirectInputs));

   pSA = &pStream->Attributes;
   for(Input = 0; Input < StrInf->NumOfInputs; Input++) {
		struct DibFrontendPossibleInput *pi =
			IntDriverDragonflyFindPossibleInput(pSA->PossibleDirectInputs, StrInf->DirectInputs[Input].FeId);

		if(pi != NULL) {
			pi->Fe = IntDriverDragonflyFindFrontend(pContext, StrInf->DirectInputs[Input].FeId);
			pi->Standard |= WithOneSegStandard(StrInf->DirectInputs[Input].FeStandards);
		}
   }

   DibSetEvent(&pContext->ChipContext.GlobalMonitEvent);
}
/****************************************************************************
 *
 ****************************************************************************/
static void IntDriverDragonflyProcessFrontendInfo(struct DibDriverContext *pContext, uint32_t * Data)
{
   struct MsgFrontendInfo MsgIn;
   struct DibFrontend * pFrontend;
   struct FrontendInfo * FeInf = &MsgIn.FeInfo;
   uint32_t Input;

   MsgFrontendInfoUnpack(&pContext->RxSerialBuf, &MsgIn);

   DIB_DEBUG(FRONTEND_LOG,(CRB "IntDriverDragonflyProcessFrontendInfo : FeNum %d NumOfInputs %d %04x" CRA, MsgIn.FeNum, FeInf->NumOfInputs, FeInf->FeId));

   pFrontend = &pContext->Frontend[MsgIn.FeNum];

   pFrontend->FeId = FeInf->FeId;

   pFrontend->Attributes.FeId = MsgIn.FeNum; /* FeId for the user is the index in the pContext-table */
   pFrontend->Attributes.ChipId = FeInf->ChipId;
   pFrontend->Attributes.Standards = WithOneSegStandard(FeInf->SupportedStandards);

   DibZeroMemory(pFrontend->Attributes.PossibleDirectInputs, sizeof(pFrontend->Attributes.PossibleDirectInputs));
   for(Input = 0; Input < FeInf->NumOfInputs; Input++) {
		struct DibFrontendPossibleInput *pi =
			IntDriverDragonflyFindPossibleInput(pFrontend->Attributes.PossibleDirectInputs,  FeInf->DirectInputs[Input].FeId);

		if(pi != NULL) {
			pi->Fe = IntDriverDragonflyFindFrontend(pContext, FeInf->DirectInputs[Input].FeId);
/* For the moment do not allow Diversity of ISDB-T 1seg
 *       pi->Standard |= WithOneSegStandard(FeInf->DirectInputs[Input].FeStandards); */
			pi->Standard |= FeInf->DirectInputs[Input].FeStandards;
		}

   }

   DibSetEvent(&pContext->ChipContext.GlobalMonitEvent);
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverDragonflyProcessMsgEvent(struct DibDriverContext *pContext, uint32_t *Data)
{
   struct DibDriverEventInfoUp DataInfo;
   struct MsgEvent Msg;

   MsgEventUnpack(&pContext->RxSerialBuf, &Msg);

   if(Msg.EventId == 0)
   {
      DIB_DEBUG(MSG_LOG, (CRB "BURST_LOST: service %d " CRA, Msg.ItemId));
      DataInfo.EventType = eBURST_LOST;
   }
   else if(Msg.EventId == 1)
   {
      DIB_DEBUG(MSG_LOG, (CRB "BURST_FAILED: service %d " CRA, Msg.ItemId));
      DataInfo.EventType = eBURST_FAILED;
   }
   else if(Msg.EventId == 2)
   {
      DIB_DEBUG(MSG_LOG, (CRB "BURST_DETECTED: service %d " CRA, Msg.ItemId));
      DataInfo.EventType = eDATA_DETECTED;
   }
   else
   {
      DIB_DEBUG(MSG_ERR, (CRB "INVALID Event: Event %d service %d " CRA, Msg.EventId, Msg.ItemId));
      return;
   }

   DataInfo.EventConfig.BurstLostEvent.ItemHdl = Msg.ItemId;
   DibDriverPostEvent(pContext, &DataInfo);
}

static void IntDriverDragonflyProcessMsgMonitDemod(struct DibDriverContext * pContext, uint32_t * Data)
{
   struct DibTotalDemodMonit * pTotalMonitInfo;
   struct MsgAckGetSignalMonit Msg;
   struct SignalMonitoring * pMon = &Msg.Mon;
   uint8_t TuneMonit;
   CHANNEL_HDL ChHdl;

   MsgAckGetSignalMonitUnpack(&pContext->RxSerialBuf, &Msg);

   ChHdl = Msg.ChannelId;
   TuneMonit = (pContext->ChannelInfo[ChHdl].Temporary == eDIB_TRUE);

   /* information should be stored in a different structure where we use TuneMonit or GetChannel */
   if(TuneMonit)
      pTotalMonitInfo = &pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelMonit;
   else
      pTotalMonitInfo = &pContext->ChannelInfo[ChHdl].ChannelMonit;

   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyProcessMsgMonitDemod DemId=%d Timeout=%d TuneMonit=%d ChHdl=%d NbWantedDemods=%d Std=%d" CRA,
             Msg.DemodId,
             pMon->MonitStatus,
             TuneMonit,
             ChHdl,
             pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods,
             pContext->ChannelInfo[ChHdl].Type));

   switch(pContext->ChannelInfo[ChHdl].Type)
   {
      case eSTANDARD_DVBSH:
      case (eSTANDARD_DVBSH | eFAST):
         DibDriverDragonflyProcessMsgMonitDemodDvbSh(pContext, &Msg, pTotalMonitInfo);
         break;

      case eSTANDARD_DVB:
      case eSTANDARD_DVB | eFAST:
      case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
         DibDriverDragonflyProcessMsgMonitDemodDvb(pContext, &Msg, pTotalMonitInfo);
         break;

      case eSTANDARD_ISDBT:
      case eSTANDARD_ISDBT_1SEG:
      case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
      case eSTANDARD_ISDBT_1SEG | eALWAYS_TUNE_SUCCESS:
         DibDriverDragonflyProcessMsgMonitDemodIsdbt(pContext, &Msg, pTotalMonitInfo);
         break;

      case eSTANDARD_CMMB:
      case eSTANDARD_CMMB | eALWAYS_TUNE_SUCCESS:
         DibDriverDragonflyProcessMsgMonitDemodCmmb(pContext, &Msg, pTotalMonitInfo);
         break;

      case eSTANDARD_DAB:
      case eSTANDARD_DAB | eALWAYS_TUNE_SUCCESS:
         DibDriverDragonflyProcessMsgMonitDemodDab(pContext, &Msg, pTotalMonitInfo);
         break;

      case eSTANDARD_CTTB:
      case eSTANDARD_CTTB | eALWAYS_TUNE_SUCCESS:
         DibDriverDragonflyProcessMsgMonitDemodCttb(pContext, &Msg, pTotalMonitInfo);
         break;

      default:
         DIB_DEBUG(MSG_ERR, (CRB "Standard not supported %d" CRA, pContext->ChannelInfo[ChHdl].Type));
         break;
   }


   if(pMon->LastDemod)
   {
      uint32_t TmpExp, TmpDemId;
      pTotalMonitInfo->Type = pContext->ChannelInfo[ChHdl].Type;

      switch(pContext->ChannelInfo[ChHdl].Type)
      {
            case eSTANDARD_DVB:
            case eSTANDARD_DVB | eFAST:
            case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
               DibDriverDragonflyCheckMonitDvb(pContext, pTotalMonitInfo, ChHdl);
               break;
            case eSTANDARD_ISDBT:
            case eSTANDARD_ISDBT_1SEG:
               DibDriverDragonflyCheckMonitIsdbt(pContext, pTotalMonitInfo, ChHdl);
               break;
            case eSTANDARD_DAB:
            case eSTANDARD_DAB | eALWAYS_TUNE_SUCCESS:
               DibDriverDragonflyCheckMonitDab(pContext, pTotalMonitInfo, ChHdl);
               break;
            case eSTANDARD_CTTB:
            case eSTANDARD_CTTB | eALWAYS_TUNE_SUCCESS:
               DibDriverDragonflyCheckMonitCttb(pContext, pTotalMonitInfo, ChHdl);
               break;
               default:
               break;
      }

      /** nb demodulation really fill in the demod structure */
      pTotalMonitInfo->NbDemods = pContext->ChannelInfo[ChHdl].pStream->NbConnFrontends;
      /** General Monit Status */
      TmpExp=0; /* number of timeout */
      for(TmpDemId=0; TmpDemId < pContext->ChannelInfo[ChHdl].ChannelMonit.NbDemods; TmpDemId++)
      {
         uint8_t IntTimeout = 0;
         switch(pContext->ChannelInfo[ChHdl].Type)
         {
            case eSTANDARD_DVBSH:
            case (eSTANDARD_DVBSH | eFAST):
               IntTimeout = DibDriverDragonflyGetTimeoutDvbSh(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_DVB:
            case eSTANDARD_DVB | eFAST:
            case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
               IntTimeout = DibDriverDragonflyGetTimeoutDvb(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_ISDBT:
            case eSTANDARD_ISDBT_1SEG:
            case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
            case eSTANDARD_ISDBT_1SEG | eALWAYS_TUNE_SUCCESS:
               IntTimeout = DibDriverDragonflyGetTimeoutIsdbt(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_CMMB:
            case eSTANDARD_CMMB | eALWAYS_TUNE_SUCCESS:
               IntTimeout = DibDriverDragonflyGetTimeoutCmmb(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_DAB:
            case eSTANDARD_DAB | eALWAYS_TUNE_SUCCESS:
               IntTimeout = DibDriverDragonflyGetTimeoutDab(pTotalMonitInfo, TmpDemId);
               break;

            case eSTANDARD_CTTB:
            case eSTANDARD_CTTB | eALWAYS_TUNE_SUCCESS:
               IntTimeout = DibDriverDragonflyGetTimeoutCttb(pTotalMonitInfo, TmpDemId);
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
         struct DibStream * pStream = pContext->ChannelInfo[ChHdl].pStream;
         DibTuneMonitCallback CallBack = pContext->ChannelInfo[ChHdl].MonitContext.pTuneMonitCb;

         CallBack(&pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit, pContext->ChannelInfo[ChHdl].MonitContext.pTuneMonitCtx);

         DIB_ASSERT(pContext->ChannelInfo[ChHdl].InUse);

         IntDriverRemoveChannel(pContext, ChHdl, eDIB_TRUE);
         pContext->ChannelInfo[ChHdl].pStream = NULL;
         IntDriverStreamAutoRemoving(pContext, pStream);

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
   struct MsgChecker                      Msg;
   ELEM_HDL                               ItemHdl;
   FILTER_HDL                             FilterHdl;
   enum DibDataType                       FilterType;
   struct DibDriverMpeIFecServiceCtx *    pMpeIFec;
   struct DibDriverMpeServiceCtx *        pMpe;
   struct DibDriverCmmbServiceCtx *       pCmmb;
   struct DibDriverFilter *               pFilter;
   struct DibDriverDabAudioServiceCtx *   pDab;

   DIB_ASSERT(Data);

   MsgCheckerUnpack(&pContext->RxSerialBuf, &Msg);

   ItemHdl = Msg.ItemId;

   /* Item has been removed since last call*/
   if(ItemHdl == DIB_UNSET)
      return;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);

   FilterHdl   = pContext->ItemInfo[ItemHdl].ParentFilter;
   pFilter     = &pContext->FilterInfo[FilterHdl];
   FilterType  = pFilter->TypeFilter;

   switch (FilterType)
   {
      case eCMMBSVC:
         pCmmb = pContext->ItemInfo[ItemHdl].ItemCtx.pCmmbService;
         if(pCmmb != NULL)
         {
            DIB_DEBUG(MSG_LOG, (CRB "Received IP checker monitoring for item %d" CRA, ItemHdl));
            /* Total information is retreived from IN_MSG_MPE_MONITOR */
            pCmmb->CmmbMonitoring.DiscountMf    = Msg.CcFailCnt;
            pCmmb->CmmbMonitoring.ErrorMf       = Msg.ErrCnt;
         }
         break;

      case eMPEFEC:
         pMpe = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService;
         if(pMpe != NULL)
         {
            DIB_DEBUG(MSG_LOG, (CRB "Received IP checker monitoring for item %d" CRA, ItemHdl));
            /* Total information is retreived from IN_MSG_MPE_MONITOR */
            pMpe->DvbHMonitoring.CcFailCnt      = Msg.CcFailCnt;
            pMpe->DvbHMonitoring.ErrCnt         = Msg.ErrCnt;
         }
         break;

      case eMPEIFEC:
         pMpeIFec = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeIFecService;
         if(pMpeIFec != NULL)
         {
            DIB_DEBUG(MSG_LOG, (CRB "Received IP checker monitoring for item %d" CRA, ItemHdl));
            /* Total information is retreived from IN_MSG_MPE_IFEC_MONITOR */
            pMpeIFec->DvbSHMonitoring.CcFailCnt      = Msg.CcFailCnt;
            pMpeIFec->DvbSHMonitoring.ErrCnt         = Msg.ErrCnt;
         }
         break;

      case eTDMB:
      case eTS:
         DIB_DEBUG(MSG_LOG, (CRB "Received RAWTS checker monitoring for item %d : %u" CRA, ItemHdl, Msg.Total));
         /* Total information is retreived from IN_MSG_RAWTS_MONITOR */
         pFilter->DvbTMonitoring.CheckTotalPackets = Msg.Total;
         pFilter->DvbTMonitoring.CheckCcFailCnt    = Msg.CcFailCnt;
         pFilter->DvbTMonitoring.CheckErrCnt       = Msg.ErrCnt;
         break;

      case eDAB:
         /* Total information is retreived from Bridge when DIB_CHECK_MSC_DATA is set to 1 */
         pDab = pContext->ItemInfo[ItemHdl].ItemCtx.pDabAudioService;
         pDab->DabMonitoring.TotalPackets       = Msg.Total;
         pDab->DabMonitoring.CountinuityError   = Msg.CcFailCnt;
         pDab->DabMonitoring.Error              = Msg.ErrCnt;
         pDab->DabMonitoring.CorrectedPackets   = 0;
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
   struct MsgAHBAccess Msg;

   MsgAHBAccessUnpack(&pContext->RxSerialBuf, &Msg);

   pContext->ChipContext.APBBridgeAddressRead = Msg.Addr;
   pContext->ChipContext.APBBridgeDataRead    = Msg.Value;

   DIB_DEBUG(MSG_LOG, (CRB "-IN_MSG_INTERNAL_ACCESS: Addr: %d, Val: %d" CRA, Msg.Addr, Msg.Value));
   DibSetEvent(&pContext->BridgeRWEvent);
}


#if (DEBUG_MODE == DEBUG_PRINT)
char * MsgIdToString(uint32_t MsgId)
{
   switch(MsgId)
   {
   case IN_MSG_ACK_CREATE_CH:
      return "IN_MSG_ACK_CREATE_CH";
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
   case IN_MSG_ACK_API_UP_EN:
      return "IN_MSG_ACK_API_UP_EN";
   case IN_MSG_ACK_GET_STREAM:
      return "IN_MSG_ACK_GET_STREAM";
   case IN_MSG_ACK_DEL_STREAM:
      return "IN_MSG_ACK_DEL_STREAM";
   case IN_MSG_ACK_ADD_FRONTEND:
      return "IN_MSG_ACK_ADD_FRONTEND";
   case IN_MSG_ACK_DEL_FRONTEND:
      return "IN_MSG_ACK_DEL_FRONTEND";
   default:
      return "IN_MSG_UNK";
   }
}
#endif

DIBSTATUS IntDriverDragonflyWaitForMsgAck(struct DibDriverContext *pContext, uint32_t Timeout)
{
   DIBSTATUS Status;

   /** Wait for the answer */
   do
   {
      Status = DibWaitForEvent(&pContext->MsgAckEvent, Timeout);

      if(pContext->MsgAckStatus == DIBSTATUS_INTERRUPTED)
      {
         /* Scan processing octocode download */
         if(pContext->MsgAckIntType == DIB_INTERRUPT_REQUEST_BINO)
         {
            IntDriverDragonflyTransferFirmware(pContext);
         }
         /* loop again */
         DibResetEvent(&pContext->MsgAckEvent);
      }
   }
   while(pContext->MsgAckStatus == DIBSTATUS_INTERRUPTED);

   return Status;
}

#define REQUEST_FIRMWARE_ACK_COMPLETE        (1 << 31)
#define REQUEST_FIRMWARE_ACK_WITH_CRC        (1 << 30)

void IntDriverDragonflyRequestFirmwareDownload(struct DibDriverContext *pContext, uint32_t *Data)
{
   struct MsgDownloadIndication Msg;

   MsgDownloadIndicationUnpack(&pContext->RxSerialBuf, &Msg);
   DIB_DEBUG(MSG_LOG, (CRB "IntDriverDragonflyRequestFirmwareDownload: at %08x max=%d fw=%s test=%d zip=%d ack=%08x off=%08x" CRA,
             Msg.AddrLsb,
             Msg.MaxLength & (~REQUEST_FIRMWARE_OPTION_MASK),
             Msg.FirmwareName,
             (Msg.MaxLength & REQUEST_FIRMWARE_OPTION_TEST) ? 1 : 0,
             (Msg.MaxLength & REQUEST_FIRMWARE_OPTION_UNCOMPRESS) ? 1 : 0,
             Msg.LengthAckLsb,
             Msg.FileOffset));

   /* Move the message information to the DriverContext */
   DibMoveMemory(pContext->DragonflyFirmware.FirmwareName, Msg.FirmwareName, sizeof(Msg.FirmwareName));
   pContext->DragonflyFirmware.Address = Msg.AddrLsb;

   /* MaxLength: bit[31]: uncompress,   bit[30]: test mode,    bit[0-29]: max buffer length */
   pContext->DragonflyFirmware.MaxLength = Msg.MaxLength & (~REQUEST_FIRMWARE_OPTION_MASK);
   pContext->DragonflyFirmware.LengthAck = Msg.LengthAckLsb;
   pContext->DragonflyFirmware.FileOffset = Msg.FileOffset;
   pContext->DragonflyFirmware.Raw        = (Msg.Options & REQUEST_FIRMWARE_OPTION_RAW)? 1:0;

   if(1/*Msg.MaxLength & REQUEST_FIRMWARE_OPTION_TEST*/)
   {
      /* TEST mode: firmware downloaded is wanted by the firmware */
      DibDriverTargetEnableIrqProcessing(pContext);
      IntDriverDragonflyTransferFirmware(pContext);
      DibDriverTargetDisableIrqProcessing(pContext);
   }
   else
   {
      /* PRODUCTION mode: firmware is always requested while a get_channel user request is pending */
      pContext->MsgAckStatus = DIBSTATUS_INTERRUPTED;
      pContext->MsgAckIntType = DIB_INTERRUPT_REQUEST_BINO;
      /* Signal to the unser context (which is blocked in GetChannel or TuneMonit) that a download bino is requested */
      DibSetEvent(&pContext->MsgAckEvent);
   }
}

static uint32_t FromBigEndian(uint8_t * p)
{
   uint32_t value;
   value =  (*p++) << 24;
   value |= (*p++) << 16;
   value |= (*p++) << 8;
   value |= (*p++);
   return value;
}

void DibDriverDragonflyDumpRegisters(struct DibDriverContext *pContext);

/*
 * From the board read the binary header to reconstruct the DragonflyRegisters slave mapping.
 */
void IntDriverDragonflyGenerateMapping(struct DibDriverContext * pContext)
{
   uint8_t * FileBuf = (uint8_t*)pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib9000.FirmwareRiscA;
   uint32_t * MapBuf = (uint32_t *)&pContext->DragonflyRegisters;
   uint32_t  MapSize = sizeof(struct DibDriverDragonflyRegisters);
   while(MapSize > 0)
   {
      *MapBuf = FromBigEndian(FileBuf);
      MapSize -= 4;
      FileBuf += 4;
      MapBuf++;
   }
}


void IntDriverDragonflyTransferFirmware(struct DibDriverContext *pContext)
{
   int TotalLen = -1;
   struct DibDriverDragonflyFirmwareDwnld * pFirmware = &pContext->DragonflyFirmware;
   DIBSTATUS status;
   uint32_t LastTransfer = 0;
   uint32_t WithCrc = 0;
   uint32_t Crc;
   struct DibDriverDmaCtx DmaCtx;
   uint32_t MaxDmaTransfer = 255*1024, TransmittedBytes = 0;
   uint8_t * FirmwareText = NULL;
   uint32_t  FirmwareLength = 0;
   uint32_t  FreeBuffer = 0;
   struct MsgDownloadConfirm ConfirmMsg;

   DIB_DEBUG(MSG_LOG, (CRB "IntDriverDragonflyTransferFirmware: at %08x max=%d bino=%s ack=%08x off=%d" CRA, pFirmware->Address, pFirmware->MaxLength, pFirmware->FirmwareName, pFirmware->LengthAck, pFirmware->FileOffset));
   /* we need this bit if we download firmware for direct execution
    * or to write it to flash. Otherwise we need to exclude references
    * to the static tables
    */
#if ((EMBEDDED_FLASH_SUPPORT == 0) || (WRITE_FLASH_SUPPORT == eWRFL_STATIC))
   {
   /* Find in the FirmwareList a matching entry */
   int fd = 0;

   while(FirmwareList[fd].FirmwareLength > 0)
   {
      if(strcmp(FirmwareList[fd].FirmwareName, pFirmware->FirmwareName) == 0) break;
      fd++;
   }
   FirmwareLength = FirmwareList[fd].FirmwareLength;
   FirmwareText = (uint8_t *)FirmwareList[fd].FirmwareText;

   if(FirmwareLength == 0)
   {
      DIB_DEBUG(MSG_ERR, (CRB "-E Cannot find %s (static)\n" CRA, pFirmware->FirmwareName));
      goto End;
   }

   if(FirmwareText == 0)
   {
      DIB_DEBUG(MSG_ERR, (CRB "-E cannot find a buffer in entry %d\n" CRA, fd));
      goto End;
   }

   if((FirmwareLength - pFirmware->FileOffset) > (pFirmware->MaxLength-4))
   {
      TotalLen = pFirmware->MaxLength-4;
      LastTransfer = 0;
   }
   else
   {
      TotalLen = FirmwareLength - pFirmware->FileOffset;
      LastTransfer = 1;
   }
   }
#elif (WRITE_FLASH_SUPPORT == eWRFL_FILE)
   {
      FILE* fileIn= fopen(pFirmware->FirmwareName, "r");
      if (!fileIn)
      {
         DIB_DEBUG(MSG_ERR, (CRB "-E Cannot find %s (file)" CRA, pFirmware->FirmwareName));
         goto End;
      }
      FirmwareText= (uint8_t*)malloc(pFirmware->MaxLength);
      if ((FirmwareText == NULL) || (fseek(fileIn, pFirmware->FileOffset,0 ) < 0))
      {
         DIB_DEBUG(MSG_ERR, (CRB "-E Cannot %s %s" CRA, (FirmwareText == NULL)? "alloc memory for":"find the offset",pFirmware->FirmwareName));
         fclose(fileIn);
         goto End;
      }
      TotalLen = fread(FirmwareText, 1,pFirmware->MaxLength-4, fileIn );
      fclose(fileIn);
      if (TotalLen == 0)
      {
         TotalLen =-1;
         goto End;
      }
      FreeBuffer = 1;
      pFirmware->FileOffset = 0;
      LastTransfer = (TotalLen == pFirmware->MaxLength-4) ? 0 : 1;
   }
#endif /* ((EMBEDDED_FLASH_SUPPORT==0) || (WRITE_FLASH_SUPPORT)) */

   while(TransmittedBytes < TotalLen)
   {
      /* transfer the buffer */
      DmaCtx.Dir         = DIBDRIVER_DMA_WRITE;
      DmaCtx.ChipAddr    = pFirmware->Address + TransmittedBytes;
      DmaCtx.ChipBaseMin = 0;
      DmaCtx.ChipBaseMax = 0;
      DmaCtx.DmaLen      = ((TotalLen - TransmittedBytes) > MaxDmaTransfer) ? MaxDmaTransfer : TotalLen - TransmittedBytes;
      DmaCtx.pHostAddr   = (uint8_t *)(FirmwareText + pFirmware->FileOffset + TransmittedBytes);
      status = DibD2BTransfertBuf(pContext, &DmaCtx);

      if(status != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(MSG_ERR, (CRB "-E Cannot transmit buffer %p of lenght %d to chip addr %08x\n" CRA, FirmwareText, FirmwareLength, pFirmware->Address));
         TotalLen = -1;
         break;
      }

      TransmittedBytes += DmaCtx.DmaLen;
   }

   /* Perform alignement to add properly the crc at the end */

   if(WithCrc)
   {
      /* Add a Crc to the end of the chunk */
      Crc = IntDriverGetCrc32((void *)(FirmwareText + pFirmware->FileOffset), TotalLen, 0xFFFFFFFF);

      DibDriverWriteReg8(pContext, pFirmware->Address + TransmittedBytes + 0, (Crc >> 24) & 0xFF);
      DibDriverWriteReg8(pContext, pFirmware->Address + TransmittedBytes + 1, (Crc >> 16) & 0xFF);
      DibDriverWriteReg8(pContext, pFirmware->Address + TransmittedBytes + 2, (Crc >> 8 ) & 0xFF);
      DibDriverWriteReg8(pContext, pFirmware->Address + TransmittedBytes + 3, (Crc      ) & 0xFF);

      TotalLen += 4;

      TotalLen |= REQUEST_FIRMWARE_ACK_WITH_CRC;
   }
   if(LastTransfer)
         TotalLen |= REQUEST_FIRMWARE_ACK_COMPLETE;

  /* if(FreeBuffer)
      free(FirmwareText);*/

End:
   ConfirmMsg.Complete = LastTransfer;
   ConfirmMsg.Length = TotalLen;
   ConfirmMsg.LengthAckLsb = pFirmware->LengthAck;
   ConfirmMsg.LengthAckMsb = 0;
   ConfirmMsg.WithCrc      =WithCrc;
   ConfirmMsg.Head.Sender    = HOST_IDENT;
   ConfirmMsg.Head.Type      = MSG_TYPE_MAC;
   ConfirmMsg.Head.MsgId     = OUT_MSG_DOWNLOAD_CFN;
   ConfirmMsg.Head.MsgSize   = GetWords(MsgDownloadConfirmBits, 32);
   ConfirmMsg.Head.ChipId    = MASTER_IDENT;

   MsgDownloadConfirmPackInit(&ConfirmMsg, &pContext->TxSerialBuf);
   DibDriverSendMessage(pContext, pContext->TxBuffer, ConfirmMsg.Head.MsgSize * 4);

}
/****************************************************************************
 * Display a string encapsulated in the message by the firmware
 ****************************************************************************/
void IntDriverDragonflyProcessMsgPrint(struct DibDriverContext *pContext, uint32_t RxCnt, uint32_t * Data)
{
   struct MsgHeader Head;
   MsgHeaderUnpack(&pContext->RxSerialBuf, &Head);
   DibDriverTargetLogFunction("#%d: %s", Head.Sender, (const char *) (Data+1));
}

void IntDriverDragonflyAdapterCfgAck(struct DibDriverContext * pContext, uint32_t * pData)
{
   struct MsgAckLayoutConfig Msg;

   MsgAckLayoutConfigUnpack(&pContext->RxSerialBuf, &Msg);

   if(Msg.Status == ACK_OK)
   {
      pContext->MsgAckStatus = DIBSTATUS_SUCCESS;
   }
   else
      pContext->MsgAckStatus = DIBSTATUS_ERROR;

   DibSetEvent(&pContext->MsgAckEvent);
}

void IntDriverDragonflyAddSlaveConfirm(struct DibDriverContext * pContext, uint32_t * pData)
{
   struct MsgAddSlaveConfirm Msg;

   MsgAddSlaveConfirmUnpack(&pContext->RxSerialBuf, &Msg);

   if(Msg.status == 0)
      pContext->MsgAckStatus = DIBSTATUS_SUCCESS;
   else
      pContext->MsgAckStatus = DIBSTATUS_ERROR;

   DibSetEvent(&pContext->MsgAckEvent);
}

void IntDriverDragonflyProcessLayoutInfo(struct DibDriverContext * pContext, uint32_t * pData)
{
   struct MsgLayoutInfo MsgIn;

   MsgLayoutInfoUnpack(&pContext->RxSerialBuf, &MsgIn);

   pContext->NbChips           = MsgIn.MaxNbChips;
   pContext->NbStreams         = MsgIn.MaxNbStreams;
   pContext->NbFrontends       = MsgIn.MaxNbFrontends;

   DibSetEvent(&pContext->ChipContext.GlobalMonitEvent);
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
   struct MsgHeader Head;
   uint32_t *Data = (uint32_t*)short_data;
   uint16_t  msg_id;
   uint32_t  rx_nb;

   /* retreive the Size of the message, in 32 bit word, without he header */
   rx_nb = (Size >> 2);

   SerialBufInit(&pContext->RxSerialBuf, Data, 32);
   MsgHeaderUnpack(&pContext->RxSerialBuf, &Head);

#if DIB_INTERNAL_MONITORING == 1
   if (Head.ChipId != -1) {
      DibDriverDebugInMessageDispatcher(pContext, Data, rx_nb, Head.ChipId);
      return;
   }
#endif

   /* retrieve information from msg */
   msg_id = Head.MsgId;

   if(Head.Type == MSG_TYPE_LAYOUT)
   {
      DibSetEvent(&pContext->MsgAckEvent);
      return;
   }
   /* retreive the Size of the message, in 32 bit word, without he header */
   rx_nb = (Size >> 2);

   if(msg_id < MAX_MSG_MAC_IN)
   {
      DIB_DEBUG(MSG_LOG, (CRB "Driver Rcvd (%d)" CRA, msg_id));
   }
   else
   {
      DIB_DEBUG(MSG_LOG, (CRB "Driver Rcvd Internal Msg %d" CRA, msg_id));
   }

   SerialBufInit(&pContext->RxSerialBuf, Data, 32);

   switch (msg_id)
   {
   case IN_MSG_INT_DBG:
   case IN_MSG_WRITE_SLAVE_REP:
   case IN_MSG_READ_SLAVE_REP:
      /* int debug */
      IntDriverDragonflyProcessMsgIntDbg(pContext, rx_nb, Data);
      break;

   case IN_MSG_PRINT:
   case IN_MSG_PRINT_BUFFER:
      /* int debug */
      IntDriverDragonflyProcessMsgPrint(pContext, rx_nb, Data);
      break;

   case IN_MSG_PROFILE:
      /* int debug */
      IntDriverDragonflyProcessMsgProfile(pContext, rx_nb, Data);
      break;

   case IN_MSG_WAVE:
      /* int debug */
      IntDriverDragonflyProcessMsgWave(pContext, rx_nb, Data);
      break;

   case IN_MSG_VERSION:
      IntDriverDragonflyProcessMsgVersion(pContext, Data);
      break;

   case IN_MSG_LAYOUT_INFO:
      IntDriverDragonflyProcessLayoutInfo(pContext, Data);
      break;

   case IN_MSG_STREAM_INFO:
      IntDriverDragonflyProcessStreamInfo(pContext, Data);
      break;

   case IN_MSG_FRONTEND_INFO:
      IntDriverDragonflyProcessFrontendInfo(pContext, Data);
      break;

   case IN_MSG_EVENT:
      IntDriverDragonflyProcessMsgEvent(pContext, Data);
      break;

   case IN_MSG_ERROR:
      IntDriverDragonflyProcessMsgError(pContext, Data);
      break;

   case IN_MSG_RAWTS_MONITOR:
      DibDriverDragonflyProcessMsgMonitorRawTs(pContext, Data);
      break;

   case IN_MSG_FIC_MONITOR:
      DibDriverDragonflyProcessMsgMonitorFic(pContext, Data);
      break;

   case IN_MSG_MSCPACKET_MONITOR:
      DibDriverDragonflyProcessMsgMonitorMscPacket(pContext, Data);
      break;

   case IN_MSG_MSCPLUS_MONITOR:
      DibDriverDragonflyProcessMsgMonitorDabPlus(pContext, Data);
      break;

   case IN_MSG_MPE_MONITOR:
      DibDriverDragonflyProcessMsgMonitorMpeFec(pContext, Data);
      break;

   case IN_MSG_MPE_IFEC_MONITOR:
      DibDriverDragonflyProcessMsgMonitorMpeIFec(pContext, Data);
      break;

   case IN_MSG_CMMB_MF_MONITOR:
      DibDriverDragonflyProcessMsgMonitorCmmbMf(pContext, Data);
      break;

   case IN_MSG_CMMB_RSM_MONITOR:
      DibDriverDragonflyProcessMsgMonitorCmmbRsm(pContext, Data);
      break;
   case IN_MSG_TUNE_STATUS_CFN:
	  IntDriverDragonflyTuneConfirm(pContext, Data);
	  break;

   case IN_MSG_ACK_CREATE_CH:
   case IN_MSG_ACK_GET_STREAM:
   case IN_MSG_ACK_DEL_STREAM:
   case IN_MSG_ACK_ADD_FRONTEND:
   case IN_MSG_ACK_DEL_FRONTEND:
   case IN_MSG_ACK_DEL_CH:
   case IN_MSG_ACK_CREATE_FILT:
   case IN_MSG_ACK_DEL_FILT:
   case IN_MSG_ACK_ADD_TO_FILT:
   case IN_MSG_ACK_FREE_ITEM:
   case IN_MSG_ACK_CAS_EN:
   case IN_MSG_ACK_API_UP_EN:
   case IN_MSG_SET_BASIC_LAYOUT_INFO:
      {
         IntDriverDragonflyProcessMsgAck(pContext, msg_id, Data);
         DibSetEvent(&pContext->MsgAckEvent);
#if (DEBUG_MODE == DEBUG_PRINT)
         DIB_DEBUG(MSG_LOG,(CRB "ACK %s :" CRA, MsgIdToString(msg_id)));
#endif
      }
      break;

   case IN_MSG_UPDATE_CHANNEL_IND:
	  IntDriverDragonflyChannelStatus(pContext,Data);
	  break;

   case IN_MSG_UDIBADAPTER_CFG_ACK:
      IntDriverDragonflyAdapterCfgAck(pContext, Data);
      break;

   case IN_MSG_ADD_SLAVE_CONFIRM:
      IntDriverDragonflyAddSlaveConfirm(pContext, Data);
      break;

   case IN_MSG_MONIT_DEMOD:
      IntDriverDragonflyProcessMsgMonitDemod(pContext, Data);
      break;

   case IN_MSG_CHECKER:
      IntDriverDragonflyTestMsgChecker(pContext, Data);
      break;

   case IN_MSG_INTERNAL_ACCESS:
      IntDriverDragonflyInternalAccessDone(pContext, Data);
      break;

#if (DIB_CMMB_DATA == 1) && (DIB_INTERNAL_DEBUG == 1)
   case IN_MSG_HRFX_INIT:
      IntDriverDragonflyHostReflexInit(pContext, Data);
      break;

   case IN_MSG_HRFX_STEP:
      IntDriverDragonflyHostReflexStep(pContext, Data);
      break;

   case IN_MSG_HRFX_SET_CONFIG:
      IntDriverDragonflyHostReflexSetConfig(pContext, Data);
      break;

   case IN_MSG_HRFX_SET_SVCPARAM:
      IntDriverDragonflyHostReflexSetSvcParam(pContext, Data);
      break;
#endif /* DIB_CMMB_DATA */

   case IN_MSG_API_ANS:
      IntDriverDragonflyProcessMsgApi(pContext, Data);
      break;

   case IN_MSG_API_UP:
      IntDriverDragonflyProcessMsgApiUp(pContext, Data);
      break;

   case IN_MSG_DOWNLOAD_IND:
      IntDriverDragonflyRequestFirmwareDownload(pContext, Data);
      break;
#if ((WRITE_FLASH_SUPPORT == eWRFL_STATIC) || (WRITE_FLASH_SUPPORT == eWRFL_FILE))
   case IN_MSG_FLASH_PROGRAM_DONE:
      IntDriverDragonflyFlashProgramDone(pContext, Data);
      break;
#endif
   default:
      DIB_DEBUG(MSG_ERR, (CRB "  IntDriverDragonflyProcessMsg: ERROR: unknown msg_id %d" CRA, msg_id));
      break;
   }
}

/**
* IntDriverDragonflySetHbm
*/
static DIBSTATUS IntDriverDragonflySetHbm(struct DibDriverContext *pContext, uint8_t buffering_mode)
{
   return DIBSTATUS_SUCCESS;

}

/**
* Enable/disable CAS system
*  add appropriate CAS filter to specified channel and add CAT CAS item
*/
static DIBSTATUS IntDriverDragonflyEnableCas(struct DibDriverContext *pContext, enum DibBool Enable)
{
   struct MsgCasEnable Msg;
   DIBSTATUS ret;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_CAS_EN;
   Msg.Head.MsgSize   = GetWords(MsgCasEnableBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   Msg.Enable         = Enable;

   MsgCasEnablePackInit(&Msg, &pContext->TxSerialBuf);

   DIB_DEBUG(SOFT_LOG, (CRB "IntDriverDragonflyEnableCas: %d" CRA, Enable));

   DibResetEvent(&pContext->MsgAckEvent);
   ret = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   DIB_DEBUG(SOFT_LOG, (CRB "DibWaitForEvent: ACK_CAS" CRA));

   IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT_SETCFG);

   DIB_DEBUG(SOFT_LOG, (CRB "ACK_CAS RECEIVED" CRA));

   ret = (pContext->MsgAckStatus == DIBSTATUS_ERROR) ? DIBSTATUS_ERROR : DIBSTATUS_SUCCESS;

   return ret;
}

/**
* Set ECM information
*  add ECM CAS item linked to an existing item
*/
/*
static DIBSTATUS IntDriverDragonflySetEcmInfo(struct DibDriverContext *pContext, ELEM_HDL ElemHdl, union DibFilters * pEcmDesc)
{
return 0;
}
*/
/**
* Set FW debug help
*/
static DIBSTATUS IntDriverDragonflySetDebugHelp(struct DibDriverContext *pContext, uint16_t i, uint16_t v, uint16_t ch)
{
   struct MsgDebugHelp Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_DEBUG_HELP;
   Msg.Head.MsgSize   = GetWords(MsgDebugHelpBits, 32);
   Msg.Head.ChipId    = ch;

   Msg.Funct          = i;
   Msg.Value          = v;

   MsgDebugHelpPackInit(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
}

/**
* Writes in the chip (internal bus)
*/
static DIBSTATUS IntDriverDragonflyApbWrite(struct DibDriverContext *pContext, uint32_t adr, uint32_t Data)
{
   struct MsgAHBAccess Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_INTERNAL_ACCESS;
   Msg.Head.MsgSize   = GetWords(MsgAHBAccessBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   Msg.Mode = INTERNAL_WRITE /*Write*/;
   Msg.Addr = adr;
   Msg.Value = Data;

   MsgAHBAccessPack(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
}

/**
* Read in the chip (internal bus)
*/
static DIBSTATUS IntDriverDragonflyApbRead(struct DibDriverContext *pContext, uint32_t adr)
{
   struct MsgAHBAccess Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_INTERNAL_ACCESS;
   Msg.Head.MsgSize   = GetWords(MsgAHBAccessBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   Msg.Mode = INTERNAL_READ /*Write*/;
   Msg.Addr = adr;
   Msg.Value= 0;

   MsgAHBAccessPack(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
}

/**
* Writes on the I2C bus control by the chip
*/
static DIBSTATUS IntDriverDragonflyI2CWrite(struct DibDriverContext *pContext, uint8_t I2cAdr, uint8_t * txbuf, uint8_t nbtx)
{
   /* TODO */
   return DIBSTATUS_ERROR;
}

/**
 * Read  on the I2C bus control by the chip
 */
static DIBSTATUS IntDriverDragonflyI2CRead(struct DibDriverContext *pContext, uint8_t I2cAdr,
                                           uint8_t *txbuf, uint8_t nbtx, uint8_t *rxbuf, uint8_t nbrx)
{
   /* TODO */
   return DIBSTATUS_ERROR;
}


/**
 * Abort any pending TuneMonitRequests
 */
static DIBSTATUS IntDriverDragonflyAbortTuneMonitChannel(struct DibDriverContext *pContext)
{
   struct MsgAbortTuneMonit Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_ABORT_TUNE_MONIT;
   Msg.Head.MsgSize   = GetWords(MsgAbortTuneMonitBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   MsgAbortTuneMonitPackInit(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
}

/**
 * Scan the requested channel to get monitoring and channel parameters
 * @param pContext: driver context
 * @param ChannelIdx: Channel handler
 * @param pTuneChan: input parameter to know on which freq we tune, and it autoserach is requested. Supposed these params are also copied
 * in the channel structure before entering in this function
 */
static DIBSTATUS IntDriverDragonflyTuneMonitChannelEx(struct DibDriverContext *pContext, struct DibStream * pStream, CHANNEL_HDL ChannelHdl, struct DibTuneMonit * pTuneMonit)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;
   return ret;
}

/**
 * Scan the requested channel to get monitoring and channel parameters
 * @param pContext: driver context
 * @param ChannelIdx: Channel handler
 * @param in_pstc: input parameter to know on which freq we tune, and it autoserach is requested. Supposed these params are also copied
 * in the channel structure before entering in this function
 */
static DIBSTATUS IntDriverDragonflyTuneMonitChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, uint8_t MinDemod, uint8_t MaxDemod, uint32_t StreamParameters, struct DibTuneMonit * pTuneMonit)
{
   struct DibStream * pStream;
   uint32_t StreamNum;
   uint32_t RealMaxDemod;
   enum DibDemodType Std = pTuneMonit->ChannelMonit.Type & ~0x80;
   uint32_t StreamType;
   uint32_t StreamOptions;

   /* Check for stream existence */
   if(pContext->NbStreams == 0)
      return DIBSTATUS_ERROR;

   /* Unpack StreamParameters */
   if(IntDriverDragonflyCheckStreamParameters(StreamParameters, &StreamType, &StreamOptions) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_INVALID_PARAMETER;

   /* Find the first stream with this given type and standard which is free */
   for(StreamNum = 0; StreamNum < pContext->NbStreams; StreamNum++)
   {
      pStream = &pContext->Stream[StreamNum];
      if((pStream->Used == 0) && pStream->Attributes.Standards & (1 << Std) && (pStream->Attributes.Type == StreamType))
        break;
   }

   if(StreamNum >= pContext->NbStreams)
      return DIBSTATUS_ERROR;

   if(DibDriverGetStream(pContext, (STREAM_HDL)pStream, Std, StreamOptions, ePOWER_AUTO) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_ERROR;

   if(MaxDemod == DIB_DEMOD_AUTO)
      RealMaxDemod = 0xFF;
   else
      RealMaxDemod = MaxDemod;

   /* Update (plug or unplug) the number of frontends depending on MaxDemod */
   IntDriverUpdateFrontends(pContext, pStream, 1, RealMaxDemod);

   /* Check that we have the required min demods */
   if(pStream->NbConnFrontends < 1)
   {
      IntDriverUpdateFrontends(pContext, pStream, 0, 0);
      return DIBSTATUS_ERROR;
   }

   /* indicate here that the channel were created using old api so that we should remove the stream
    * when deconnecting the last channel of it */
   pStream->AutoRemove = 1;

   /* Attach a stream to this channel to that DibDriverIf can remove it */
   pContext->ChannelInfo[ChannelHdl].pStream = pStream;

   /* now we can add the channel using the new API */
   return IntDriverDragonflyTuneMonitChannelEx(pContext, pStream, ChannelHdl, pTuneMonit);

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
 * Requests that the master demod downloads slave firmware
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyDwldSlaveFw(struct DibDriverContext *pContext, uint16_t FeId)
{
   /* TODO */
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
 * Select a specific Driver Configuration
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflySetCfg(struct DibDriverContext *pContext, uint32_t Cfg)
{
   DIBSTATUS              Status = DIBSTATUS_SUCCESS;
   int32_t                FeIndex;
   int32_t                StreamNum;

   {
      /* specific layout init function */
      if((Status == DIBSTATUS_SUCCESS) && pContext->Hd.BoardConfig->LayoutInit)
      {
         Status = pContext->Hd.BoardConfig->LayoutInit(pContext);
      }
      if(Status == DIBSTATUS_SUCCESS)
      {
         /** get general layout info now the layout generic is intanciated */
         DibResetEvent(&pContext->ChipContext.GlobalMonitEvent);
         Status = DibDriverGetLayoutInfo(pContext);
         if(Status == DIBSTATUS_SUCCESS)
            Status = DibWaitForEvent(&pContext->ChipContext.GlobalMonitEvent, 500);
      }

      /* expect for information of all frontends */
      for(FeIndex = pContext->NbFrontends-1; FeIndex >= 0 && (Status == DIBSTATUS_SUCCESS); FeIndex--)
      {
         DibResetEvent(&pContext->ChipContext.GlobalMonitEvent);
         Status = DibDriverGetFrontendInfo(pContext, FeIndex);
         if(Status == DIBSTATUS_SUCCESS)
            Status = DibWaitForEvent(&pContext->ChipContext.GlobalMonitEvent, 500);
      }

      /** retreive stream and frontend information (layout configuration has been done) */
      for(StreamNum = 0; (StreamNum < pContext->NbStreams) && (Status == DIBSTATUS_SUCCESS); StreamNum++)
      {
         DibResetEvent(&pContext->ChipContext.GlobalMonitEvent);
         Status = DibDriverGetStreamInfo(pContext, StreamNum);
         if(Status == DIBSTATUS_SUCCESS)
            Status = DibWaitForEvent(&pContext->ChipContext.GlobalMonitEvent, 500);
      }
   }

   return Status;
}


/****************************************************************************
 * Select a specific Driver Configuration
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyDeInitFirmware(struct DibDriverContext *pContext)
{
   return DIBSTATUS_SUCCESS;
}


/****************************************************************************
 * Sends a "get monit" command
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyGetMonit(struct DibDriverContext *pContext, uint8_t ChannelHdl)
{
   struct MsgGetSignalMonit Msg;

   /* Message header */
   Msg.Head.Sender    = HOST_IDENT;
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_MONIT_DEMOD;
   Msg.Head.MsgSize   = GetWords(MsgGetSignalMonitBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;

   Msg.StreamId       = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;
   Msg.ChannelId      = ChannelHdl;
   Msg.DemodId        = 0;

   MsgGetSignalMonitPackInit(&Msg, &pContext->TxSerialBuf);

   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyGetMonit channel %d" CRA, ChannelHdl));

   return DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
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
   Msg.Sender    = HOST_IDENT;

   MsgHeaderPackInit(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, 4);
}

/****************************************************************************
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyGetLayoutInfo(struct DibDriverContext *pContext)
{
   struct MsgGetLayoutInfo MsgOut;

   DIB_DEBUG(MSG_LOG, (CRB "IntDriverDragonflyGetLayoutInfo" CRA));

   /* Message header */
   MsgOut.Head.Type      = MSG_TYPE_MAC;
   MsgOut.Head.MsgId     = OUT_MSG_GET_LAYOUT_INFO;
   MsgOut.Head.MsgSize   = GetWords(MsgGetLayoutInfoBits, 32);
   MsgOut.Head.ChipId    = MASTER_IDENT;
   MsgOut.Head.Sender    = HOST_IDENT;

   MsgGetLayoutInfoPackInit(&MsgOut, &pContext->TxSerialBuf);



   return DibDriverSendMessage(pContext, pContext->TxBuffer, MsgOut.Head.MsgSize * 4);
}

/****************************************************************************
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyGetStreamInfo(struct DibDriverContext *pContext, uint32_t StreamNum)
{
   struct MsgGetStreamInfo MsgOut;

   DIB_DEBUG(MSG_LOG, (CRB "IntDriverDragonflyGetStreamInfo : StreamNum %d" CRA, StreamNum));

   /* Message header */
   MsgOut.Head.Type      = MSG_TYPE_MAC;
   MsgOut.Head.MsgId     = OUT_MSG_GET_STREAM_INFO;
   MsgOut.Head.MsgSize   = GetWords(MsgGetStreamInfoBits, 32);
   MsgOut.Head.ChipId    = MASTER_IDENT;
   MsgOut.Head.Sender    = HOST_IDENT;

   MsgOut.StreamNum      = StreamNum;

   MsgGetStreamInfoPackInit(&MsgOut, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, MsgOut.Head.MsgSize * 4);
}

/****************************************************************************
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyGetFrontendInfo(struct DibDriverContext *pContext, uint32_t FeNum)
{
   struct MsgGetFrontendInfo MsgOut;

   DIB_DEBUG(MSG_LOG, (CRB "IntDriverDragonflyGetFrontendInfo : FeNum %d" CRA, FeNum));

   /* Message header */
   MsgOut.Head.Type      = MSG_TYPE_MAC;
   MsgOut.Head.MsgId     = OUT_MSG_GET_FRONTEND_INFO;
   MsgOut.Head.MsgSize   = GetWords(MsgGetFrontendInfoBits, 32);
   MsgOut.Head.ChipId    = MASTER_IDENT;
   MsgOut.Head.Sender    = HOST_IDENT;

   MsgOut.FeNum          = FeNum;

   MsgGetFrontendInfoPackInit(&MsgOut, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, MsgOut.Head.MsgSize * 4);
}

/****************************************************************************
 * Requests a new stream
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyGetStream(struct DibDriverContext *pContext, struct DibStream * pStream, enum DibDemodType Std, uint32_t Options, enum DibPowerMode EnPowerSaving)
{
   DIBSTATUS Status;
   struct MsgGetStream Msg;

   Msg.Head.ChipId   = MASTER_IDENT;
   Msg.Head.Sender   = HOST_IDENT;
   Msg.Head.MsgSize  = GetWords(MsgGetStreamBits, 32);
   Msg.Head.MsgId    = OUT_MSG_GET_STREAM;
   Msg.Head.Type     = MSG_TYPE_MAC;

   /* OneSeg support: set bit 31 of Options and force Std to ISDB for emb */
   if(Std == eSTANDARD_ISDBT_1SEG)
   {
      Options |= 0x80000000;
      Std = eSTANDARD_ISDBT;
   }

   /* Remove ePOWER_AUTO for EnPowerSaving parameter, in that case take the oldest one.. */
   if(EnPowerSaving == ePOWER_AUTO)
      EnPowerSaving = pStream->ChipTimeSlicing;

   Msg.StreamId            = pStream->StreamId;
   Msg.Standard            = Std;
   Msg.EnableTimeSlicing   = EnPowerSaving;
   Msg.OutputOptions       = Options;

   MsgGetStreamPackInit(&Msg, &pContext->TxSerialBuf);
   DibResetEvent(&pContext->MsgAckEvent);

   Status = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);

   if(Status == DIBSTATUS_SUCCESS)
   {
      /** Wait for the answer */
      Status = IntDriverDragonflyWaitForMsgAck(pContext, 500);
      if((Status == DIBSTATUS_SUCCESS) && (pContext->MsgAckStatus == DIBSTATUS_SUCCESS))
         Status = DIBSTATUS_SUCCESS;
      else
         Status = DIBSTATUS_ERROR;
   }

   return Status;
}

/****************************************************************************
 * Requests stream removal
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyDeleteStream(struct DibDriverContext *pContext, struct DibStream * pStream)
{
   DIBSTATUS Status;
   struct MsgDeleteStream Msg;

   Msg.Head.ChipId   = MASTER_IDENT;
   Msg.Head.Sender   = HOST_IDENT;
   Msg.Head.MsgId    = OUT_MSG_DEL_STREAM;
   Msg.Head.MsgSize  = GetWords(MsgDeleteStreamBits, 32);
   Msg.Head.Type     = MSG_TYPE_MAC;

   Msg.StreamId     = pStream->StreamId;

   MsgDeleteStreamPackInit(&Msg, &pContext->TxSerialBuf);
   DibResetEvent(&pContext->MsgAckEvent);

   Status = DibDriverSendMessage(pContext, pContext->TxBuffer, (Msg.Head.MsgSize << 2));

   if(Status == DIBSTATUS_SUCCESS)
   {
      /** Wait for the answer */
      Status = IntDriverDragonflyWaitForMsgAck(pContext, 500);

      if((Status == DIBSTATUS_SUCCESS) && (pContext->MsgAckStatus == DIBSTATUS_SUCCESS))
         Status = DIBSTATUS_SUCCESS;
      else
         Status = DIBSTATUS_ERROR;
   }

   return Status;
}

/****************************************************************************
 * Requests a new frontend
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyAddFrontend(struct DibDriverContext *pContext, struct DibStream * pStream, struct DibFrontend * pFrontend, struct DibFrontend * pOutputFrontend)
{
   DIBSTATUS Status;
   struct MsgAddFrontend Msg;

   Msg.Head.MsgSize = GetWords(MsgAddFrontendBits, 32);
   Msg.Head.MsgId   = OUT_MSG_ADD_FRONTEND;
   Msg.Head.ChipId  = MASTER_IDENT;
   Msg.Head.Type    = MSG_TYPE_MAC;
   Msg.Head.Sender  = HOST_IDENT;

   Msg.StreamId = pStream->StreamId;
   Msg.FrontendId = pFrontend->FeId;
   Msg.OutputFrontendId = pOutputFrontend ? pOutputFrontend->FeId : -1;
   MsgAddFrontendPackInit(&Msg, &pContext->TxSerialBuf);


   DibResetEvent(&pContext->MsgAckEvent);

   Status = DibDriverSendMessage(pContext, pContext->TxBuffer, (Msg.Head.MsgSize << 2));

   if(Status == DIBSTATUS_SUCCESS)
   {
      /** Wait for the answer */
      Status = IntDriverDragonflyWaitForMsgAck(pContext, 5000);

      if((Status == DIBSTATUS_SUCCESS) && (pContext->MsgAckStatus == DIBSTATUS_SUCCESS))
      {
         Status = DIBSTATUS_SUCCESS;
      }else
         Status = DIBSTATUS_ERROR;
   }

   return Status;

}

static int IntDriverStreamHasActiveChannels(struct DibDriverContext *pContext, struct DibStream *pStream)
{
   int i;
   for (i = 0; i < DIB_MAX_NB_CHANNELS; i++)
      if (pContext->ChannelInfo[i].pStream == pStream)
         return 1;
   return 0;
}


/****************************************************************************
 * Requests frontend removal
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyRemoveFrontend(struct DibDriverContext *pContext, struct DibFrontend * pFrontend)
{
   DIBSTATUS Status;
   struct MsgRemoveFrontend Msg;

   /* if it is the last frontend of the stream, no channel have to be activ if we delete it */
   if(IntDriverStreamHasActiveChannels(pContext, pFrontend->pStream)
         && (pFrontend->pStream->NbConnFrontends == 1))
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyRemoveFrontend: Stream tuned, the last fe can not be deleted" CRA));
      return DIBSTATUS_ERROR;
   }

   Msg.Head.MsgSize = GetWords(MsgRemoveFrontendBits, 32);
   Msg.Head.MsgId   = OUT_MSG_DEL_FRONTEND;
   Msg.Head.ChipId  = MASTER_IDENT;
   Msg.Head.Type    = MSG_TYPE_MAC;
   Msg.Head.Sender  = HOST_IDENT;

   Msg.StreamId = pFrontend->pStream->StreamId;
   Msg.FrontendId = pFrontend->FeId;

   MsgRemoveFrontendPackInit(&Msg, &pContext->TxSerialBuf);
   DibResetEvent(&pContext->MsgAckEvent);

   Status = DibDriverSendMessage(pContext, pContext->TxBuffer, (Msg.Head.MsgSize << 2));

   if(Status == DIBSTATUS_SUCCESS)
   {
      /** Wait for the answer */
      Status = IntDriverDragonflyWaitForMsgAck(pContext, 5000);

      if((Status == DIBSTATUS_SUCCESS) && (pContext->MsgAckStatus == DIBSTATUS_SUCCESS))
         Status = DIBSTATUS_SUCCESS;
      else
         Status = DIBSTATUS_ERROR;
   }

   return Status;

}

/**
 * Send a message to create a new channel and get the new handler, if successful.
 * @param pContext current context
 * @param pStream          the stream on which we add the channel
 * @param pDescriptor      pointer to a structure with all the useful information for the configuration of the channel
 * @param pChannelHdl      pointer to the result channel (chosed by the firmware in case of dragonfly), if successful
 * @return DIBSTATUS result of the command
 */

DIBSTATUS IntDriverDragonflyGetChannelEx(struct DibDriverContext *pContext, struct DibStream * pStream, struct  DibChannel *pDescriptor, CHANNEL_HDL  *pChannelHdl)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   struct MsgCreateChannel Msg;
   struct MsgTuneIndication TuneMsg;
   struct DibTuneChan* pTc = &(pDescriptor->ChannelDescriptor);
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

   /* For now we need at least one stream */
   if(pStream->pInput == NULL)
      return DIBSTATUS_ERROR;

#if (DIB_MULTI_CHANNEL_PER_STREAM == 0)
   for(ii = 0; ii < DIB_MAX_NB_CHANNELS; ii++)
   {
      if((pContext->ChannelInfo[ii].pStream ==  pStream)
         && (ChannelHdl != ii))
      {
         return DIBSTATUS_RESOURCES;
      }
   }
#endif

   /* check that pDescriptor->Type is coherent with the stream one */
   if((pDescriptor->Type & ~(0x80 | 0x40)) != pStream->Std)
      return DIBSTATUS_ERROR;

   /* We copy the given information to the context structure. The scan() and tune()
   * functions will need them */
   DibMoveMemory(&pContext->ChannelInfo[ChannelHdl].ChannelDescriptor, &pDescriptor->ChannelDescriptor, sizeof(struct DibTuneChan));
   pContext->ChannelInfo[ChannelHdl].Type = pDescriptor->Type;

   DibZeroMemory(&Msg, sizeof(struct MsgCreateChannel));

   /* COMMON part */
   Msg.Desc.Standard     = pStream->Std;
   Msg.Desc.Frequency    = pTc->RFkHz;
   Msg.Desc.Bandwidth    = pTc->Bw*100;
   Msg.StreamId	    	 = pStream->StreamId;
   Msg.ChannelId    	 = ChannelHdl;
   Msg.TuneMonit         = 0;


   /* HEADER */
   Msg.Head.Sender  = HOST_IDENT;
   Msg.Head.MsgId   = OUT_MSG_REQ_CREATE_CH;
   Msg.Head.Type    = MSG_TYPE_MAC;
   Msg.Head.MsgSize = GetWords(MsgCreateChannelBits, 32);
   Msg.Head.ChipId  = MASTER_IDENT;

   switch(pStream->Std)
   {
   case eSTANDARD_DVB:
   case (eSTANDARD_DVB | eFAST):
   case (eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS):
      ret = DibDriverDragonflySetMsgTuneDvb(pTc, &Msg);
      break;

   case eSTANDARD_ISDBT:
   case (eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS):
      ret = DibDriverDragonflySetMsgTuneIsdbt(pTc, &Msg);
      break;

   case eSTANDARD_ISDBT_1SEG:
      ret = DibDriverDragonflySetMsgTuneIsdbtOneSeg(pTc, &Msg);
      break;

   case eSTANDARD_CTTB:
   case eSTANDARD_CTTB | eALWAYS_TUNE_SUCCESS:
      ret = DibDriverDragonflySetMsgTuneCttb(pTc, &Msg);
      break;

   case eSTANDARD_DVBSH:
   case (eSTANDARD_DVBSH | eFAST):
      ret = DibDriverDragonflySetMsgTuneDvbSh(pTc, &Msg);
      break;

   case eSTANDARD_DAB:
   case (eSTANDARD_DAB | eALWAYS_TUNE_SUCCESS):
      ret = DibDriverDragonflySetMsgTuneDab(pTc, &Msg);
      break;

   case eSTANDARD_CMMB:
   case (eSTANDARD_CMMB | eALWAYS_TUNE_SUCCESS):
      ret = DibDriverDragonflySetMsgTuneCmmb(pTc, &Msg);
      break;

   default:
      return DIBSTATUS_INVALID_PARAMETER;
      break;
   }

   /* Pack the message */
   MsgCreateChannelPackInit(&Msg, &pContext->TxSerialBuf);

   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.RFkHz = pTc->RFkHz;
   pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.Bw = pTc->Bw;
   pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.StreamParameters = 0;

   DibResetEvent(&pContext->MsgAckEvent);

   ret = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);

   /* Send Message failed */
   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   ret = IntDriverDragonflyWaitForMsgAck(pContext, 12000);

   if(ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyGetChannelEx: Call to firmware failed" CRA));
      return ret;
   }
   pContext->ChannelInfo[ChannelHdl].Temporary = eDIB_FALSE;

   /* COMMON part */
   TuneMsg.ChannelId= ChannelHdl;
   TuneMsg.StreamId = pStream->StreamId;
   TuneMsg.FastScan =(pDescriptor->Type & 0x80) ? 1 : 0;
   TuneMsg.TuneMonit= 0;
    /* HEADER */
   TuneMsg.Head.Sender  = HOST_IDENT;
   TuneMsg.Head.MsgId   = OUT_MSG_TUNE_IND;
   TuneMsg.Head.Type    = MSG_TYPE_MAC;
   TuneMsg.Head.MsgSize = GetWords(MsgTuneIndicationBits, 32);
   TuneMsg.Head.ChipId  = MASTER_IDENT;

   /* Pack the message */
   MsgTuneIndicationPackInit(&TuneMsg, &pContext->TxSerialBuf);

   DibResetEvent(&pContext->MsgAckEvent);

   ret = DibDriverSendMessage(pContext, pContext->TxBuffer, TuneMsg.Head.MsgSize * 4);

   /* Send Message failed */
   if(ret != DIBSTATUS_SUCCESS)
	   return ret;

   ret = IntDriverDragonflyWaitForMsgAck(pContext, 12000);

   if(ret != DIBSTATUS_SUCCESS)
   {
	   DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyGetChannelEx: Call to firmware failed" CRA));
	   return ret;
   }

   /* Tune Success */
   if (pContext->ChannelInfo[ChannelHdl].Type & eALWAYS_TUNE_SUCCESS)
       pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.ScanStatus = DIB_SCAN_SUCCESS;

   pTc->ScanStatus = pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.ScanStatus;
   if(pTc->ScanStatus != DIB_SCAN_SUCCESS)
   {
	   if(pTc->ScanStatus == DIB_SCAN_TIMEOUT)
	   {
		   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyGetChannelEx: scan timeout: firmware answered it could not process request" CRA));
		   ret = DIBSTATUS_TIMEOUT;
	   }
       else /* (pTc->ScanStatus == DIB_SCAN_FAILURE) | (pTc->ScanStatus == DIB_SCAN_ABORTED) */
       {
           pContext->ChannelInfo[ChannelHdl].pStream = pStream;
           IntDriverDragonflyDeleteChannel(pContext,ChannelHdl);
           IntDriverRemoveChannel(pContext, (CHANNEL_HDL)ChannelHdl, eDIB_FALSE);
           pContext->ChannelInfo[ChannelHdl].pStream = NULL;
           IntDriverStreamAutoRemoving(pContext, pStream);
           ret = DIBSTATUS_ERROR;
       }
   }
   else
   {

	   memcpy(pTc,&pContext->ChannelInfo[ChannelHdl].ChannelDescriptor, sizeof(struct DibTuneChan));
	   IntDriverAddChannel(pContext, (CHANNEL_HDL)ChannelHdl, 0, 0, 0, pDescriptor->Type, pTc);
	   pContext->ChannelInfo[ChannelHdl].pStream = pStream;

	   /* Check if a data channel already exist for that stream */
	   for(ii = 0; ii < DIB_MAX_NB_CHANNELS; ii++)
	   {
		   if((pContext->ChannelInfo[ii].IsDataChannel == eDIB_TRUE) && (pContext->ChannelInfo[ii].pStream == pStream))
			   break;
	   }
	   /* If not found this channel becomes the data channel... */
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
   return ret;

}
/**
 * Send a message to create a new channel and get the new handler, if successful. => Old API.
 * @param pContext current context
 * @param MinNbDemod       min number of demodulation for this channel
 * @param MaxNbDemod       max number of demodulation for this channel
 * @param PriorityChannel  indication if this channel will be the priority one or not
 * @param pDescriptor      pointer to a structure with all the useful information for the configuration of the channel
 * @param pChannelHdl      pointer to the result channel (chosed by the firmware in case of dragonfly), if successful
 * @return DIBSTATUS result of the command
 */
static DIBSTATUS IntDriverDragonflyGetChannel(struct DibDriverContext * pContext, uint8_t MinDemod, uint8_t MaxDemod,
												uint32_t StreamParameters, struct DibChannel * pDescriptor, CHANNEL_HDL * pChannelHdl)
{
   DIBSTATUS Status;
   struct DibStream * pStream;
   struct DibFrontend * fe;
   uint32_t StreamNum;
   uint32_t FeNum;
   uint32_t RealMaxDemod;
   enum DibDemodType Std = pDescriptor->Type & ~0x80;
   uint32_t StreamType;
   uint32_t StreamOptions;

   /* Check for stream existence */
   if(pContext->NbStreams == 0)
      return DIBSTATUS_ERROR;

   /* Unpack StreamParameters */
   if(IntDriverDragonflyCheckStreamParameters(StreamParameters, &StreamType, &StreamOptions) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_INVALID_PARAMETER;

   /* Find the first stream with this given type and standard which is free */
   for(StreamNum = 0; StreamNum < pContext->NbStreams; StreamNum++)
   {
      pStream = &pContext->Stream[StreamNum];
      if((pStream->Used == 0) && pStream->Attributes.Standards & (1 << Std) && (pStream->Attributes.Type == StreamType))
        break;
   }
   if(StreamNum >= pContext->NbStreams)
      return DIBSTATUS_ERROR;

   if(MaxDemod == DIB_DEMOD_AUTO)
      RealMaxDemod = 0xFF;
   else
      RealMaxDemod = MaxDemod;

   /* indicate here that the channel were created using old api so that we should remove the stream
    * when deconnecting the last channel of it */
   Status = DibDriverSpecifGetStream(pContext, pStream, Std, StreamOptions, 0);
   if(Status == DIBSTATUS_SUCCESS)
   {
       pStream->AutoRemove = 1;
       pStream->Used = 1;
       pStream->Options = StreamOptions;
       pStream->Std = Std;

       /* Adds all available frontends */
       for(FeNum = 0; FeNum < pContext->NbFrontends; FeNum++) {
           fe = &pContext->Frontend[FeNum];
           if (fe->pStream != NULL)
               return DIBSTATUS_ERROR;
           DibDriverAddFrontend(pContext,  pStream, fe, NULL, 0);
       }

       /* now we can add the channel using the new API */
       Status = DibDriverSpecifGetChannelEx(pContext, pStream, pDescriptor, pChannelHdl);
   }
   return Status;
}

/**
 * Send a message to delete a channel (and all the associated filters) in the firmware.
 * @param pContext pointer to the current context
 * @param ChannelHdl Handler of the channel that the user want to delete
 */
static DIBSTATUS IntDriverDragonflyDeleteChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl)
{
   /** Send the message */
   DIBSTATUS  result;
   uint8_t    fi;
   struct MsgDeleteChannel Msg;
   struct MsgSleepIndication SleepMsg;
   uint16_t StreamHdl = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;

   SleepMsg.Head.ChipId    = MASTER_IDENT;
   SleepMsg.Head.MsgId     = OUT_MSG_SLEEP_IND;
   SleepMsg.Head.MsgSize   = GetWords(MsgSleepIndicationBits, 32);
   SleepMsg.Head.Type      = MSG_TYPE_MAC;
   SleepMsg.Head.Sender    = HOST_IDENT;
   SleepMsg.ChannelId      = ChannelHdl;
   SleepMsg.StreamId       = StreamHdl;

   Msg.Head.ChipId         = MASTER_IDENT;
   Msg.Head.MsgId          = OUT_MSG_REQ_DEL_CH;
   Msg.Head.MsgSize        = GetWords(MsgDeleteChannelBits, 32);
   Msg.Head.Type           = MSG_TYPE_MAC;
   Msg.Head.Sender         = HOST_IDENT;

   Msg.StreamId            = StreamHdl;
   Msg.ChannelId           = ChannelHdl;

   /** before : delete filter */
   fi = pContext->ChannelInfo[ChannelHdl].FirstFilter;
   while(fi != DIB_UNSET)
   {
      DibDriverSpecifDeleteFilter(pContext, fi);
      fi = pContext->ChannelInfo[ChannelHdl].FirstFilter;
   }

   MsgSleepIndicationPackInit(&SleepMsg, &pContext->TxSerialBuf);
   DibResetEvent(&pContext->MsgAckEvent);
   if((result = DibDriverSendMessage(pContext, pContext->TxBuffer, SleepMsg.Head.MsgSize << 2)) == DIBSTATUS_SUCCESS)
   {
       result = DibWaitForEvent(&pContext->MsgAckEvent, MSG_ACK_TIMEOUT);
   }

   if(result != DIBSTATUS_SUCCESS)
   {
       DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyDeleteChannel: Sleep fails Stream is removed\n" CRA));
   }


   MsgDeleteChannelPackInit(&Msg, &pContext->TxSerialBuf);

   DibResetEvent(&pContext->MsgAckEvent);
   if((result = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize << 2)) == DIBSTATUS_SUCCESS)
   {
      result = DibWaitForEvent(&pContext->MsgAckEvent, MSG_ACK_TIMEOUT);
   }

   if(result == DIBSTATUS_SUCCESS)
   {
      if(pContext->MsgAckStatus == DIBSTATUS_SUCCESS)
      {
         /* Backward compatibility: auto removing of the stream if this is the last channel */
         IntDriverRemoveChannel(pContext, ChannelHdl, eDIB_FALSE);
         pContext->ChannelInfo[ChannelHdl].pStream = NULL;
         return DIBSTATUS_SUCCESS;
      }
      else
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyDeleteChannel: Error during channel delete" CRA));
         return DIBSTATUS_ERROR;
      }
   }
   else
   {
      if(result == DIBSTATUS_TIMEOUT)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyDeleteChannel: Timeout on Event" CRA));
         return DIBSTATUS_TIMEOUT;
      }
      else
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyDeleteChannel: Error waiting Event" CRA));
         return DIBSTATUS_ERROR;
      }
   }
}

/**
 * Send a message to create a filter associated with a channel.
 * @param pContext   opinter to the current context
 * @param ChannelHdl handler odf the associated channel
 * @param DataType   Type of the filter
 * @param pFilterHdl pointer of the new filter, if command successful, choosed by the firmware in dragonfly case
 * @return uint8_t     result of the command
 */
static DIBSTATUS IntDriverDragonflyCreateFilter(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer * pDataBuf, FILTER_HDL * pFilterHdl)
{
   struct MsgCreateFilter Msg;
   uint32_t Size = GetWords(MsgCreateFilterBits, 32);
   uint8_t result;
   uint32_t ii;

   *pFilterHdl = DIB_UNSET;

   /** Take the first free filter */
   for(ii = 0; ii < DIB_MAX_NB_FILTERS; ii++)
   {
      if(pContext->FilterInfo[ii].TypeFilter == eUNKNOWN_DATA_TYPE)
         break;
   }

   if(ii == DIB_MAX_NB_FILTERS)
   {
      /* no more filter avalaible */
      return DIBSTATUS_RESOURCES;
   }

   Msg.Head.ChipId  = MASTER_IDENT;
   Msg.Head.MsgId   = OUT_MSG_REQ_CREATE_FILT;
   Msg.Head.MsgSize = Size;
   Msg.Head.Type    = MSG_TYPE_MAC;
   Msg.Head.Sender  = HOST_IDENT;

   Msg.Type         = DataType;
   Msg.StreamId     = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;
   Msg.ChannelId    = ChannelHdl;
   Msg.FilterId     = ii;

   MsgCreateFilterPackInit(&Msg, &pContext->TxSerialBuf);

   DibResetEvent(&pContext->MsgAckEvent);
   if((result = DibDriverSendMessage(pContext, pContext->TxBuffer, (Size << 2))) == DIBSTATUS_SUCCESS)
   {
      result = IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT);
   }

   if(result == DIBSTATUS_SUCCESS)
   {
      /** Firmware not able to add this filter  */
      if(pContext->MsgAckStatus == DIBSTATUS_ERROR)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflycreate_filter: Firmware not able to add this filter" CRA));
         return DIBSTATUS_ERROR;
      }
      else
      {
         IntDriverAddFilter(pContext, ii, ChannelHdl, DataType, pDataBuf);

         *pFilterHdl = ii;
         return DIBSTATUS_SUCCESS;
      }

   }
   else
   {
      if(result == DIBSTATUS_TIMEOUT)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflycreate_filter: Timeout on Event" CRA));
         return DIBSTATUS_TIMEOUT;

      }
      else
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflycreate_filter: Error waiting Event" CRA));
         return DIBSTATUS_ERROR;
      }
   }
}

/**
 * Send a message to delete one filter and all the associated items in the firmware.
 * @param pContext pointer to the current context
 * @param FilterHdl Handler of the filter that the user want to delete
 */
static DIBSTATUS IntDriverDragonflyDeleteFilter(struct DibDriverContext * pContext, FILTER_HDL FilterHdl)
{
   struct MsgDeleteFilter Msg;
   uint32_t Size = GetWords(MsgDeleteFilterBits, 32);
   uint8_t result;
   uint8_t it;
   uint8_t ChannelHdl = pContext->FilterInfo[FilterHdl].ParentChannel;

   Msg.Head.ChipId   = MASTER_IDENT;
   Msg.Head.MsgSize  = Size;
   Msg.Head.MsgId    = OUT_MSG_REQ_DEL_FILT;
   Msg.Head.Type     = MSG_TYPE_MAC;
   Msg.Head.Sender   = HOST_IDENT;

   Msg.StreamId      = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;
   Msg.FilterId      = (uint8_t) FilterHdl;

   pContext->FilterInfo[FilterHdl].FlushActive = eDIB_TRUE;

   /** First remove every items of the filter */
   it = pContext->FilterInfo[FilterHdl].FirstItem;

   while(it != DIB_UNSET)
   {
      DibDriverSpecifRemoveItem(pContext, it);

      it  = pContext->FilterInfo[FilterHdl].FirstItem;
   }

   MsgDeleteFilterPackInit(&Msg, &pContext->TxSerialBuf);

   DibResetEvent(&pContext->MsgAckEvent);
   if((result = DibDriverSendMessage(pContext, pContext->TxBuffer, (Size << 2))) == DIBSTATUS_SUCCESS)
   {
      result = IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT);
   }

   /* In Any case delete filter */
   IntDriverRemoveFilter(pContext, FilterHdl);

   if(result == DIBSTATUS_SUCCESS)
   {
      return DIBSTATUS_SUCCESS;
   }
   else if(result == DIBSTATUS_TIMEOUT)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflydelete_filter: Timeout on Event" CRA));
      return DIBSTATUS_TIMEOUT;

   }
   else
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflydelete_filter: Error waiting Event" CRA));
      return DIBSTATUS_ERROR;
   }
}

void IntDragonlyDriverAddItem(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, FILTER_HDL FilterHdl, union DibFilters *pFilterDesc)
{
   CHANNEL_HDL ChannelHdl;
   union DibInformBridge SetBridgeInfo;

   /** success : update of the list of items and of the link with the filter */
   IntDriverAddItem(pContext, ItemHdl, FilterHdl, pFilterDesc);

   ChannelHdl = pContext->FilterInfo[FilterHdl].ParentChannel;

   SetBridgeInfo.SetService.Svc       = DIB_UNSET /* dragonfly case */;
   SetBridgeInfo.SetService.ItemHdl   = ItemHdl;
   SetBridgeInfo.SetService.FilterHdl = FilterHdl;
   SetBridgeInfo.SetService.StreamId = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;
   SetBridgeInfo.SetService.DataType  = pContext->FilterInfo[FilterHdl].TypeFilter;
   SetBridgeInfo.SetService.DataMode  = pContext->FilterInfo[FilterHdl].DataInfo.DataMode;
   SetBridgeInfo.SetService.Pid       = pFilterDesc->Common.Pid;

   /** Success : Give the bridge all the information needed to have the Data path in the way wanted by the user (ip or callback) */
   DibD2BInformBridge(pContext, eSET_SERVICE, &SetBridgeInfo);
   /** There is at least one pid */
   pContext->FilterInfo[FilterHdl].FlushActive = eDIB_FALSE;
}

void IntDragonlyDriverDelItem(struct DibDriverContext * pContext, ELEM_HDL ElemHdl, FILTER_HDL FilterHdl)
{
   CHANNEL_HDL ChannelHdl;
   union DibInformBridge SetBridgeInfo;

   /* if it is the last pid : flush the incoming buffers */
   if(pContext->FilterInfo[FilterHdl].NbActivePids == 1)
   {
      pContext->FilterInfo[FilterHdl].FlushActive = eDIB_TRUE;
      DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyRemoveItem Flush activated %d" CRA, FilterHdl));
   }

   ChannelHdl = pContext->FilterInfo[FilterHdl].ParentChannel;

   SetBridgeInfo.SetService.Svc       = DIB_UNSET /* dragonfly case */;
   SetBridgeInfo.SetService.ItemHdl   = ElemHdl;
   SetBridgeInfo.SetService.FilterHdl = FilterHdl;
   SetBridgeInfo.SetService.StreamId = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;
   SetBridgeInfo.SetService.DataType  = pContext->FilterInfo[FilterHdl].TypeFilter;
   SetBridgeInfo.SetService.DataMode  = eUNSET;
   SetBridgeInfo.SetService.Pid       = pContext->ItemInfo[ElemHdl].Config.Common.Pid;

   /** Success : Give the bridge all the information needed to have the Data path in the way wanted by the user (ip or callback) */
   DibD2BInformBridge(pContext, eSET_SERVICE, &SetBridgeInfo);

   IntDriverRemoveItem(pContext, ElemHdl);
}

/**
 * Send a message to add an item to a filter already existing and get the handler of it.
 * @param pContext      pointer to the current context
 * @param FilterHdl     associated filter
 * @param nbElements    number of items to add
 * @param pFilterDesc   pointer to the first item to add structure of information, which is an union depending of the Type of filter
 * @param pElemHdl      pointer to the first item's handler, in case of success (chosed by the firmware in case of dragonfly)
 * @return uint8_t        result of the command
 */
static DIBSTATUS IntDriverDragonflyAddItem(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, union DibFilters *pFilterDesc, ELEM_HDL * pElemHdl)
{
   struct MsgCreateItem msg;
   uint32_t        ItemHdl;
   DIBSTATUS       Status = DIBSTATUS_ERROR;
   uint32_t        MsgSize = GetWords(MsgCreateItemBits, 32);
   uint8_t         ChannelHdl = pContext->FilterInfo[FilterHdl].ParentChannel;

   *pElemHdl = DIB_UNSET;

   /* Take the first free item */
   for(ItemHdl = 0; ItemHdl < DIB_MAX_NB_ITEMS; ItemHdl++)
   {
      if(pContext->ItemInfo[ItemHdl].ParentFilter == DIB_UNSET)
         break;
   }

   if(ItemHdl == DIB_MAX_NB_ITEMS)
   {
      /* no more filter avalaible */
      return DIBSTATUS_RESOURCES;
   }
   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyAddItem %d to filter %d type %d" CRA, ItemHdl, FilterHdl, pContext->FilterInfo[FilterHdl].TypeFilter));

   msg.Head.ChipId      = MASTER_IDENT;
   msg.Head.MsgSize     = MsgSize;
   msg.Head.Type        = MSG_TYPE_MAC;
   msg.Head.MsgId       = OUT_MSG_REQ_ADD_TO_FILT;
   msg.Head.Sender      = HOST_IDENT;

   msg.StreamId         = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;
   msg.ItemId           = ItemHdl;
   msg.FilterType       = pContext->FilterInfo[FilterHdl].TypeFilter;
   msg.FilterId         = FilterHdl;

   /** configuration of the item depending of the filter's Type */
   switch(msg.FilterType)
   {
   case eTS:
      Status = DibDriverDragonflyAddItemRawTs(pContext, pFilterDesc, pElemHdl, ItemHdl, &msg);
      break;

   case eSIPSI:
      Status = DibDriverDragonflyAddItemSiPsi(pContext, pFilterDesc, pElemHdl, ItemHdl, &msg);
      break;

   case eMPEIFEC:
      Status = DibDriverDragonflyAddItemMpeIFec(pContext, pFilterDesc, FilterHdl, ItemHdl, &msg);
      break;

   case eMPEFEC:
      Status = DibDriverDragonflyAddItemMpeFec(pContext, pFilterDesc, FilterHdl, ItemHdl, &msg);
      break;

   case eFIG:
      Status = DibDriverDragonflyAddItemFig(pContext, pFilterDesc, FilterHdl, ItemHdl, &msg);
      break;

   case eTDMB:
      Status = DibDriverDragonflyAddItemTdmb(pContext, pFilterDesc, FilterHdl, ItemHdl, &msg);
      break;

   case eDAB:
      Status = DibDriverDragonflyAddItemDabAudio(pContext, pFilterDesc, FilterHdl, ItemHdl, &msg);
      break;

   case eEDAB:
      break;

   case eDABPACKET:
      Status = DibDriverDragonflyAddItemDabPacket(pContext, pFilterDesc, FilterHdl, ItemHdl, &msg);
      break;

   case eCMMBMFS:
      break;

   case eCMMBCIT:
      break;

   case eCMMBSVC:
      /* Request Service identified either by Multiplex Frame ID or Service ID. */
      Status = DibDriverDragonflyAddItemCmmbSvc(pContext, pFilterDesc, FilterHdl, ItemHdl, &msg);
      break;

   case eUNKNOWN_DATA_TYPE:
   default:
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyadd_item: Timeout on Event" CRA));
      /** Other types are not supported by the dragonfly firmware */
      return DIBSTATUS_INVALID_PARAMETER;
   }

   if(Status != DIBSTATUS_SUCCESS)
      return Status;

   MsgCreateItemPackInit(&msg, &pContext->TxSerialBuf);
   IntDragonlyDriverAddItem(pContext, ItemHdl, FilterHdl, pFilterDesc);
   *pElemHdl = ItemHdl;

   DibResetEvent(&pContext->MsgAckEvent);
   if((Status = DibDriverSendMessage(pContext, pContext->TxBuffer, (MsgSize << 2))) == DIBSTATUS_SUCCESS)
   {
      Status = IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT);
   }

   if(Status == DIBSTATUS_SUCCESS)
   {
      if(pContext->MsgAckStatus == DIBSTATUS_ERROR)
         Status = DIBSTATUS_ERROR;
      else
         Status = DIBSTATUS_SUCCESS;
   }
   else
   {
      if(Status == DIBSTATUS_TIMEOUT)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyaAddItem: Timeout on Event" CRA));
      }
      else
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyAddItem: Error waiting Event" CRA));
         Status = DIBSTATUS_ERROR;
      }
   }

   if(Status != DIBSTATUS_SUCCESS)
      IntDragonlyDriverDelItem(pContext, ItemHdl, FilterHdl);

   return Status;
}

/**
 * Send a message to remove an item of a filter (already existed).
 * @param pContext pointer to the current context
 * @param ElemHdl Handler of the item that the user want to delete
 */
static DIBSTATUS IntDriverDragonflyRemoveItem(struct DibDriverContext * pContext, ELEM_HDL ElemHdl)
{
   struct MsgDeleteItem Msg;
   struct DibDriverItem * pItem = &pContext->ItemInfo[ElemHdl];
   FILTER_HDL        FilterHdl = pItem->ParentFilter;
   uint32_t          Size = GetWords(MsgDeleteItemBits, 32);
   uint8_t           ChannelHdl = pContext->FilterInfo[FilterHdl].ParentChannel;
   enum DibDataType  FilterType;
   DIBSTATUS         result;

   Msg.Head.MsgId   = OUT_MSG_REQ_REM_TO_FILT;
   Msg.Head.Type    = MSG_TYPE_MAC;
   Msg.Head.ChipId  = MASTER_IDENT;
   Msg.Head.MsgSize = Size;
   Msg.Head.Sender  = HOST_IDENT;

   Msg.StreamId     = pContext->ChannelInfo[ChannelHdl].pStream->StreamId;
   Msg.ItemId       = ElemHdl;

   MsgDeleteItemPackInit(&Msg, &pContext->TxSerialBuf);

   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyRemoveItem %d" CRA, ElemHdl));

   DibResetEvent(&pContext->MsgAckEvent);
   if((result = DibDriverSendMessage(pContext, pContext->TxBuffer, (Size << 2))) == DIBSTATUS_SUCCESS)
   {
      result = IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT);
   }

   if(result == DIBSTATUS_TIMEOUT)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyremove_item: Timeout on Event" CRA));
   }
   else if(result != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyremove_item: Error waiting Event" CRA));
   }

   /* Delete Item even if event timed out, otherwise causes error on WXP */
/* if(result == DIBSTATUS_SUCCESS) */
   {
/*      struct DibDriverItem *pItem = &pContext->ItemInfo[ElemHdl];*/
      /** Success : Reset the information in the bridge concerning the Data path */
/*      FilterHdl = pItem->ParentFilter;*/
      FilterType  = pContext->FilterInfo[FilterHdl].TypeFilter;

      /** Free filter-specific buffers */
      switch(FilterType)
      {
         case eMPEFEC:
            result = DibDriverDragonflyRemoveItemMpeFec(pContext, pItem);
            break;

         case eMPEIFEC:
            result = DibDriverDragonflyRemoveItemMpeIFec(pContext, pItem);
            break;

         case eSIPSI:
            result = DibDriverDragonflyRemoveItemSiPsi(pContext, pItem, ElemHdl);
            break;

         case eCMMBSVC:
            result = DibDriverDragonflyRemoveItemCmmbSvc(pContext, pItem);
            break;

         case eFIG:
            result = DibDriverDragonflyRemoveItemFig(pContext, pItem);
            break;

         case eTDMB:
            result = DibDriverDragonflyRemoveItemTdmb(pContext, pItem);
            break;

         case eDAB:
            result = DibDriverDragonflyRemoveItemDabAudio(pContext, pItem);
            break;

         case eDABPACKET:
            result = DibDriverDragonflyRemoveItemDabPacket(pContext, pItem);
            break;

         default:
            break;
      }

      /** Success : Give the bridge all the information needed to have the Data path in the way wanted by the user (ip or callback) */
      IntDragonlyDriverDelItem(pContext, ElemHdl, FilterHdl);

      return result;
   }
}
void IntDriverDragonflyChannelStatus(struct DibDriverContext * pContext, uint32_t * pData)
{
	struct MsgUpdateChannelIndication Msg;
	uint16_t ChHdl;
	struct DibTuneChan * pTc;

	MsgUpdateChannelIndicationUnpack(&pContext->RxSerialBuf, &Msg);
	ChHdl            = Msg.ChannelId;
	pTc = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor);
	DIB_ASSERT(pTc);

	switch(pContext->ChannelInfo[ChHdl].Type)
	{
		case eSTANDARD_DVB:
		case eSTANDARD_DVB | eFAST:
		case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
			DibDriverDragonflyGetChannelInfoDvb(pContext, &Msg, ChHdl);
			break;

		case eSTANDARD_ISDBT:
		case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
			DibDriverDragonflyGetChannelInfoIsdbt(pContext, &Msg, ChHdl);
			break;

		case eSTANDARD_ISDBT_1SEG:
			DibDriverDragonflyGetChannelInfoIsdbtOneSeg(pContext, &Msg, ChHdl);
			break;

		case eSTANDARD_CTTB:
        case eSTANDARD_CTTB | eALWAYS_TUNE_SUCCESS:
			DibDriverDragonflyGetChannelInfoCttb(pContext, &Msg, ChHdl);
			break;

		case eSTANDARD_DVBSH:
		case (eSTANDARD_DVBSH | eFAST):
			DibDriverDragonflyGetChannelInfoDvbSh(pContext, &Msg, ChHdl);
			break;

		case eSTANDARD_DAB:
		case eSTANDARD_DAB | eALWAYS_TUNE_SUCCESS:
			DibDriverDragonflyGetChannelInfoDab(pContext, &Msg, ChHdl);
			break;

		case eSTANDARD_CMMB:
		case eSTANDARD_CMMB | eALWAYS_TUNE_SUCCESS:
			DibDriverDragonflyGetChannelInfoCmmb(pContext, &Msg, ChHdl);
			break;

		default:
			DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDragonflyEndOfScan: Unknow standard %d" CRA, pContext->ChannelInfo[ChHdl].Type));
			break;
	}

}

void IntDriverDragonflyTuneConfirm(struct DibDriverContext * pContext, uint32_t * pData)
{
	struct MsgTuneStatusConfirm Msg;
	enum TuneStatus TStatus;
	enum AdapterState AState;
	struct DibTuneChan * pTc;
	uint16_t ChHdl;

	MsgTuneStatusConfirmUnpack(&pContext->RxSerialBuf, &Msg);
    ChHdl            = Msg.ChannelId;
	TStatus       	 = Msg.Status;
	AState           = Msg.State;

	DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverDragonflyTuneConfirm: ChHdl %d TuneStatus %d AdapterStatr = %d" CRA, ChHdl, TStatus, AState));

   pTc    = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor);
   DIB_ASSERT(pTc);

   if(AState == POWER_ON) /* Always expect MONIT_MSG if TuneMonit was requested */
   {
	  if(TStatus == TUNE_LOCKED)
	  {
      }
      else
      {
         /* No Lock, the TPS have no sense */
         DibZeroMemory(&pTc->Std, sizeof(pTc->Std));
      }
      /* 0:failure 1:success, 2:timeout */
      pTc->ChannelHdl = (CHANNEL_HDL) ChHdl;
      pTc->ScanStatus = (TStatus== TUNE_LOCKED )? 1:(TStatus== TUNE_TIMED_OUT)? 2: 0;
      pContext->MsgAckStatus = DIBSTATUS_SUCCESS;
   }
   else
   {
      pContext->TuneStatus = TStatus;
      pContext->MsgAckStatus = DIBSTATUS_ERROR;
      pTc->ScanStatus = DIB_SCAN_INVALID_PARAM;
   }
   DibSetEvent(&pContext->MsgAckEvent);
}

/**
 * function called when the acquitement of a message concerning the creation or suppression of a channel, a filter or an item of filter.
 * @param pContext  ptr to current context
 * @param id         identifiant of the recieved message
 * @param info       16 lsb : result (success/error), 16 msb : handler associated, if success
 */
void IntDriverDragonflyProcessMsgAck(struct DibDriverContext * pContext, uint16_t id, uint32_t * pData)
{
   struct MsgAcknowledgeApi Msg;
   MsgAcknowledgeApiUnpack(&pContext->RxSerialBuf, &Msg);

   if(Msg.Success == ACK_OK)
      pContext->MsgAckStatus = DIBSTATUS_SUCCESS;
   else
      pContext->MsgAckStatus = DIBSTATUS_ERROR;
}

/**
 * Receive an API message. Forward the buffer to the host using API message driver context
 * @param pContext   pointer to current context
 * @param pData      pointer to the received message
 */
void IntDriverDragonflyProcessMsgApi(struct DibDriverContext * pContext, uint32_t * pData)
{
   struct DibDriverDmaCtx DmaCtx;
   struct MsgApiAnswer Msg;

   MsgApiAnswerUnpack(&pContext->RxSerialBuf, &Msg);

   if(*pContext->ApiMsgCtx.pSize >= Msg.RxLen)
   {
      if(Msg.RxLen > 0)
      {
         DmaCtx.DmaLen        = Msg.RxLen;
         DmaCtx.ChipBaseMin   = 0 /* not used */;
         DmaCtx.ChipBaseMax   = 0 /* not used */;
         DmaCtx.ChipAddr      = Msg.RxAddr;
         DmaCtx.pHostAddr     = pContext->ApiMsgCtx.pBuffer;
         DmaCtx.Dir           = DIBDRIVER_DMA_READ;
         DmaCtx.Mode          = DIBBRIGDE_BIT_MODE_8;
         DibDriverTargetEnableIrqProcessing(pContext);   /* TODO msg reception only should be forbidden */
         DibD2BTransfertBuf(pContext, &DmaCtx);
         DibDriverTargetDisableIrqProcessing(pContext);
      }
      if(Msg.Status == MSG_API_SUCCESS)
      {
         pContext->ApiMsgCtx.Status = DIBSTATUS_SUCCESS;
      }
      else
      {
         DIB_DEBUG(MSG_LOG, (CRB "  IntDriverDragonflyProcessMsgApi: Communication failure %d" CRA, Msg.Status));
         pContext->ApiMsgCtx.Status = DIBSTATUS_ERROR;
      }
   }
   else
   {
      DIB_DEBUG(MSG_LOG, (CRB "  IntDriverDragonflyProcessMsgApi: Buffer size =%d, needed %d" CRA, *pContext->ApiMsgCtx.pSize, Msg.RxLen));
      pContext->ApiMsgCtx.Status = DIBSTATUS_INSUFFICIENT_MEMORY;
   }

   *pContext->ApiMsgCtx.pSize = Msg.RxLen;

   pContext->MsgAckStatus = DIBSTATUS_SUCCESS;
   DibSetEvent(&pContext->MsgAckEvent);
}

/**
 * Receive an API message. Forward the buffer to the host using API message callback driver context
 * @param pContext   pointer to current context
 * @param pData      pointer to the received message
 */
void IntDriverDragonflyProcessMsgApiUp(struct DibDriverContext * pContext, uint32_t * pData)
{
   struct DibDriverDmaCtx DmaCtx;
   struct MsgApiUpAck MsgOut;
   struct MsgApiUp MsgIn;

   MsgApiUpUnpack(&pContext->RxSerialBuf, &MsgIn);

   if(MsgIn.RxLen)
   {
      if(pContext->ApiMsgCbCtx.BufferSize < MsgIn.RxLen)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  IntDriverDragonflyProcessMsgApiUp: Buffer size =%d, needed %d" CRA, pContext->ApiMsgCbCtx.BufferSize, MsgIn.RxLen));
         return;
      }

      if(pContext->ApiMsgCbCtx.pCallback)
      {
         DmaCtx.DmaLen        = MsgIn.RxLen;
         DmaCtx.ChipBaseMin   = 0 /* not used */;
         DmaCtx.ChipBaseMax   = 0 /* not used */;
         DmaCtx.ChipAddr      = MsgIn.RxAddr;
         DmaCtx.pHostAddr     = pContext->ApiMsgCbCtx.pBuffer;
         DmaCtx.Dir           = DIBDRIVER_DMA_READ;
         DmaCtx.Mode          = DIBBRIGDE_BIT_MODE_8;
         DibDriverTargetEnableIrqProcessing(pContext);   /* TODO msg reception only should be forbidden */
         DibD2BTransfertBuf(pContext, &DmaCtx);
         DibDriverTargetDisableIrqProcessing(pContext);
      }

      DibDriverTargetEnableIrqProcessing(pContext);   /* TODO msg reception only should be forbidden */

      /* Message header */
      MsgOut.Head.Type      = MSG_TYPE_MAC;
      MsgOut.Head.MsgId     = OUT_MSG_API_UP_ACK;
      MsgOut.Head.MsgSize   = GetWords(MsgApiUpAckBits, 32);
      MsgOut.Head.ChipId    = MASTER_IDENT;
      MsgOut.Head.Sender    = HOST_IDENT;

      /* Buffer to free in the firmware */
      MsgOut.Addr           = MsgIn.RxAddr;

      MsgApiUpAckPackInit(&MsgOut, &pContext->TxSerialBuf);

      DibDriverSendMessage(pContext, pContext->TxBuffer, MsgOut.Head.MsgSize * 4);
      DibDriverTargetDisableIrqProcessing(pContext);
   }

   if(pContext->ApiMsgCbCtx.pCallback)
      pContext->ApiMsgCbCtx.pCallback( pContext->ApiMsgCbCtx.pCallbackCtx,
                                       MsgIn.ReqType,
                                       MsgIn.ReqId,
                                       pContext->ApiMsgCbCtx.pBuffer,
                                       MsgIn.RxLen );

}

/*
 *  Indicates if the time-slicing is enabled or not for a particular stream
 */
static DIBSTATUS IntDriverDragonflyGetPowerManagement(struct DibDriverContext * pContext, STREAM_HDL StreamHdl, enum DibBool * Enable)
{
   struct DibStream * pStream = (struct DibStream *)StreamHdl;
   *Enable = pStream->ChipTimeSlicing;

   return DIBSTATUS_SUCCESS;
}

/**
* Change the strategy of power of a stream (or all the streams if StremHdl is -1)
* @param pContext   pointer to the context of the driver
* @param TimeSlicing   Type of power management
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverDragonflySetPowerManagement(struct DibDriverContext * pContext, enum DibBool TimeSlicing, STREAM_HDL StreamHdl)
{
   struct MsgEnableTimeSlice Msg;
   DIBSTATUS                 Status = DIBSTATUS_SUCCESS;
   struct DibStream *        pStream = (struct DibStream *)StreamHdl;

   DIB_ASSERT((TimeSlicing == eDIB_TRUE) || (TimeSlicing == eDIB_FALSE));

   /* If stream is used, we need to send a message to the firmware */
   if(pStream->Used)
   {
      /* Message header */
      Msg.Head.Type      = MSG_TYPE_MAC;
      Msg.Head.MsgId     = OUT_MSG_ENABLE_TIME_SLICE;
      Msg.Head.MsgSize   = GetWords(MsgEnableTimeSliceBits, 32);
      Msg.Head.ChipId    = MASTER_IDENT;
      Msg.Head.Sender    = HOST_IDENT;

      Msg.StreamId       = pStream->StreamId;
      Msg.Enable         = TimeSlicing;

      DIB_DEBUG(MSG_LOG, (CRB "  IntDriverDragonflySetPowerManagement: Set power Mode to %d on stream %d" CRA,
                TimeSlicing, pStream->StreamId));

      MsgEnableTimeSlicePackInit(&Msg, &pContext->TxSerialBuf);

      Status = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
   }

   if(Status == DIBSTATUS_SUCCESS)
   {
      pStream->ChipTimeSlicing = TimeSlicing;
   }

   return Status;
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
 * Send API messages
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflySendMsgApi(struct DibDriverContext *pContext, uint16_t MsgType, uint16_t MsgIndex, uint8_t *pSendBuf, uint32_t SendLen, uint8_t *pRecvBuf, uint32_t * pRecvLen)
{
   struct DibDriverDmaCtx DmaCtx;
   DIBSTATUS   ret = DIBSTATUS_ERROR;
   struct MsgApiRequest Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_API_REQ;
   Msg.Head.MsgSize   = GetWords(MsgApiRequestBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   /* while there is no remote allocation mechanism, we always transfer user data to a fixed buffer in the fimware */
   Msg.ReqType        = MsgType;
   Msg.ReqId          = MsgIndex;
   Msg.TxLen          = SendLen;
   Msg.TxAddr         = pContext->DragonflyRegisters.TxRxBufAddr;

   if (SendLen %4)
   {
      SendLen= Msg.TxLen + 4 - (SendLen %4);
   }
   /* send the data is exists */
   if(Msg.TxLen > 0)
   {
      DmaCtx.DmaLen        = SendLen;
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
      ret = IntDriverDragonflyWaitForMsgAck(pContext, 4000);
   }

   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   return pContext->ApiMsgCtx.Status;
}

/****************************************************************************
 * Enable API messages UP
 ****************************************************************************/
static DIBSTATUS IntDriverDragonflyEnableMsgApiUp(struct DibDriverContext *pContext, uint16_t MsgType, enum DibBool Enable)
{
   DIBSTATUS   ret = DIBSTATUS_ERROR;
   struct MsgApiUpEnable Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_API_UP_EN;
   Msg.Head.MsgSize   = GetWords(MsgApiUpEnableBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   Msg.ReqType        = MsgType;
   Msg.Enable         = Enable;

   MsgApiUpEnablePackInit(&Msg, &pContext->TxSerialBuf);

   DIB_DEBUG(SOFT_LOG, (CRB "IntDriverDragonflyEnableMsgApiUp Type=%d, Enable=%d" CRA, MsgType, (uint16_t)Enable));

   DibResetEvent(&pContext->MsgAckEvent);
   ret = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   DIB_DEBUG(SOFT_LOG, (CRB "DibWaitForEvent: ACK_API_UP" CRA));

   IntDriverDragonflyWaitForMsgAck(pContext, MSG_ACK_TIMEOUT_SETCFG);

   DIB_DEBUG(SOFT_LOG, (CRB "ACK_API_UP RECEIVED" CRA));

   ret = (pContext->MsgAckStatus == DIBSTATUS_ERROR) ? DIBSTATUS_ERROR : DIBSTATUS_SUCCESS;

   return ret;
}

/**
 * Runtime parameter changing. This function can do the following updates:
 * - Turn on/off a PID to/from prefetch mode
 * @param[in] pContext: driver context
 * @param[in] pPidInfo: Directive to set or unset prefetch for a certain PID.
 */
static DIBSTATUS IntDriverDragonflySetPidMode(struct DibDriverContext *pContext, struct DibPidModeCfg *pPidInfo)
{
   struct MsgSetPidMode Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_SET_PID_MODE;
   Msg.Head.MsgSize   = GetWords(MsgSetPidModeBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   Msg.ItemId         = pPidInfo->ElemHdl;
   Msg.Prefetch       = (pPidInfo->Mode == eACTIVE) ? 0 : 1;

   MsgSetPidModePackInit(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);

}

/****************************************************************************
* Clear Monitoring Info
****************************************************************************/
static DIBSTATUS IntDriverDragonflyClearMonit(struct DibDriverContext *pContext, ELEM_HDL ItemHdl)
{
   struct MsgCtrlMonit Msg;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_CTL_MONIT;
   Msg.Head.MsgSize   = GetWords(MsgCtrlMonitBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   Msg.ItemId         = ItemHdl;
   Msg.Cmd            = 0x1; /* Clear Monit Info */

   MsgCtrlMonitPackInit(&Msg, &pContext->TxSerialBuf);

   return DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);
}
#if ((WRITE_FLASH_SUPPORT == eWRFL_STATIC) || (WRITE_FLASH_SUPPORT == eWRFL_FILE))
/****************************************************************************
* Clear Monitoring Info
****************************************************************************/
static DIBSTATUS IntDriverDragonflyProgramFlash(struct DibDriverContext *pContext, char* filename)
{
   struct MsgFlashProgram Msg;
   DIBSTATUS rc;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = OUT_MSG_FLASH_PROGRAM;
   Msg.Head.MsgSize   = GetWords(MsgFlashProgramBits, 32);
   Msg.Head.ChipId    = MASTER_IDENT;
   Msg.Head.Sender    = HOST_IDENT;

   Msg.NameLength = strlen(filename);
   strcpy(Msg.Name,filename);

   MsgFlashProgramPackInit(&Msg, &pContext->TxSerialBuf);

   rc = DibDriverSendMessage(pContext, pContext->TxBuffer, Msg.Head.MsgSize * 4);

   if (rc == DIBSTATUS_SUCCESS)
   {
      pContext->MsgAckStatus = DIBSTATUS_CONTINUE;
   }
   return rc;
}

#endif
/*******************************************************************************
* IntDriverDragonflyWriteDemod
*******************************************************************************/
static DIBSTATUS IntDriverDragonflyWriteDemod(struct DibDriverContext *pContext, uint8_t DemId, uint32_t SubAdd, uint16_t Value)
{
   /* IF MAIN DEMOD. */
   if(DemId == 0)
   {
      return DibDriverWriteReg16(pContext, SubAdd, Value);
   }

   return DIBSTATUS_ERROR;
}

/*******************************************************************************
* IntDriverDragonflyReadDemod
*******************************************************************************/
static uint16_t IntDriverDragonflyReadDemod(struct DibDriverContext *pContext, uint8_t DemId, uint32_t SubAdd)
{
   uint16_t Value = 0;

   /* IF MAIN DEMOD. */
   DibDriverReadReg16(pContext, SubAdd, &Value);

   return Value;
}

/****************************************************************************
 * Tests the RAM (R&W tests)
 ****************************************************************************/
static uint32_t IntDriverDragonflyTestRamInterface(struct DibDriverContext *pContext)
{
  return 0;
}

#if ((WRITE_FLASH_SUPPORT == eWRFL_STATIC) || (WRITE_FLASH_SUPPORT == eWRFL_FILE))
/****************************************************************************
 * InFl
 ****************************************************************************/
void IntDriverDragonflyFlashProgramDone(struct DibDriverContext * pContext, uint32_t * pData)
{
   struct MsgFlashProgramDone Msg;

   MsgFlashProgramDoneUnpack(&pContext->RxSerialBuf, &Msg);
   pContext->MsgAckStatus = Msg.Status;

}

#endif


/******************************************************************************
 * dibOutMessageOps::msgFireflyOps
 * Specific output message formating for dragonfly
 ******************************************************************************/
void DibDriverDragonflyRegisterChip(struct DibDriverContext *pContext)
{
   /* specific architecture function */
   switch(pContext->DibChip) {

      case DIB_VOYAGER:
          pContext->ChipOps.MacResetCpu = IntDriverVoyager1ResetCpu;

          /* Dragonfly registers */
          DibMoveMemory(&pContext->DragonflyRegisters,
                        pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib9000.Mapping,
                        sizeof(struct DibDriverDragonflyRegisters));
         break;

      case DIB_NAUTILUS:
         pContext->ChipOps.MacResetCpu = IntDriverNautilus1ResetCpu;
         IntDriverDragonflyGenerateMapping(pContext);
         break;

      default:
         break;
   }

    /* mac functions */
    pContext->ChipOps.MacInitCpu            = IntDriverDragonflyInitCpu;
    pContext->ChipOps.MacStartCpu           = IntDriverDragonflyStartCpu;
    pContext->ChipOps.MacUploadMicrocode    = IntDriverDragonflyUploadMicrocode;
    pContext->ChipOps.MacWaitForFirmware    = IntDriverDragonflyWaitForFirmware;
    pContext->ChipOps.MacInitMailbox        = IntDriverDragonflyInitMailbox;
    pContext->ChipOps.MacTestRamInterface   = IntDriverDragonflyTestRamInterface;
    pContext->ChipOps.MacEnableClearIrq     = IntDriverDragonflyEnableClearIrq;

    /* register_if functions */
    pContext->ChipOps.RegisterIfInit        = IntDriverDragonflyRegisterIfInit;
    pContext->ChipOps.WriteDemod            = IntDriverDragonflyWriteDemod;
    pContext->ChipOps.ReadDemod             = IntDriverDragonflyReadDemod;

    /* packet demux */
    pContext->ChipOps.GetSiPsiInfo          = DibDriverDragonflyGetSiPsiInfo;

    /* frontend functions */
    pContext->ChipOps.FrontendInit          = IntDriverDragonflyFrontendInit;
    pContext->ChipOps.FrontendRfTotalGain   = IntDriverDragonflyFrontendRfTotalGain;

    /* handler for input messages */
    pContext->ChipOps.ProcessMessage        = IntDriverDragonflyProcessMsg;

    /* output message functions */
    pContext->ChipOps.SetDebugHelp          = IntDriverDragonflySetDebugHelp;
    pContext->ChipOps.ApbWrite              = IntDriverDragonflyApbWrite;
    pContext->ChipOps.ApbRead               = IntDriverDragonflyApbRead;
    pContext->ChipOps.I2CWrite              = IntDriverDragonflyI2CWrite;
    pContext->ChipOps.I2CRead               = IntDriverDragonflyI2CRead;
    pContext->ChipOps.AbortTuneMonitChannel = IntDriverDragonflyAbortTuneMonitChannel;
    pContext->ChipOps.TuneMonitChannel      = IntDriverDragonflyTuneMonitChannel;
    pContext->ChipOps.TuneMonitChannelEx    = IntDriverDragonflyTuneMonitChannelEx;
    pContext->ChipOps.InitDemod             = IntDriverDragonflyInitDemod;
    pContext->ChipOps.GetMonit              = IntDriverDragonflyGetMonit;
    pContext->ChipOps.GetEmbeddedInfo       = IntDriverDragonflyGetEmbeddedInfo;
    pContext->ChipOps.SetCfgGpio            = IntDriverDragonflySetCfgGpio;
    pContext->ChipOps.SubbandSelect         = IntDriverDragonflySubbandSelect;
    pContext->ChipOps.SetHbm                = IntDriverDragonflySetHbm;
    pContext->ChipOps.EnableCas             = IntDriverDragonflyEnableCas;
    pContext->ChipOps.SendMsgApi            = IntDriverDragonflySendMsgApi;
    pContext->ChipOps.EnableMsgApiUp        = IntDriverDragonflyEnableMsgApiUp;

    /* new api */
    pContext->ChipOps.GetChannel            = IntDriverDragonflyGetChannel;
    pContext->ChipOps.DeleteChannel         = IntDriverDragonflyDeleteChannel;
    pContext->ChipOps.CreateFilter          = IntDriverDragonflyCreateFilter;
    pContext->ChipOps.DeleteFilter          = IntDriverDragonflyDeleteFilter;
    pContext->ChipOps.AddItem               = IntDriverDragonflyAddItem;
    pContext->ChipOps.RemoveItem            = IntDriverDragonflyRemoveItem;
    pContext->ChipOps.SetPowerManagement    = IntDriverDragonflySetPowerManagement;
    pContext->ChipOps.GetPowerManagement    = IntDriverDragonflyGetPowerManagement;
    pContext->ChipOps.SetPidMode            = IntDriverDragonflySetPidMode;
    pContext->ChipOps.DwldSlaveFw           = IntDriverDragonflyDwldSlaveFw;
    pContext->ChipOps.ClearMonit            = IntDriverDragonflyClearMonit;
    pContext->ChipOps.SetCfg                = IntDriverDragonflySetCfg;
    pContext->ChipOps.DeInitFirmware        = IntDriverDragonflyDeInitFirmware;
    pContext->ChipOps.GetStreamInfo         = IntDriverDragonflyGetStreamInfo;
    pContext->ChipOps.GetFrontendInfo       = IntDriverDragonflyGetFrontendInfo;
    pContext->ChipOps.GetLayoutInfo         = IntDriverDragonflyGetLayoutInfo;
    pContext->ChipOps.GetStream             = IntDriverDragonflyGetStream;
    pContext->ChipOps.DeleteStream          = IntDriverDragonflyDeleteStream;
    pContext->ChipOps.AddFrontend           = IntDriverDragonflyAddFrontend;
    pContext->ChipOps.RemoveFrontend        = IntDriverDragonflyRemoveFrontend;
    pContext->ChipOps.GetChannelEx          = IntDriverDragonflyGetChannelEx;
#if ((WRITE_FLASH_SUPPORT == eWRFL_STATIC) || (WRITE_FLASH_SUPPORT == eWRFL_FILE))
    pContext->ChipOps.ProgramFlash          = IntDriverDragonflyProgramFlash;
#endif
#if (DIB_INTERNAL_DEBUG == 1)
    pContext->ChipOps.SubmitBlock           = IntDriverDragonflySubmitBlock;
#endif
}

#endif /* USE_DRAGONFLY */
