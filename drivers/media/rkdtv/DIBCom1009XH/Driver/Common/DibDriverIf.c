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
* @file "DibDriverIf.c"
* @brief Driver Interface.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriver.h"
#include "DibDriverIf.h"
#include "DibDriverMessages.h"
#include "DibDriverDowncalls.h"
#include "DibBoardParameters.h"
#include "DibBoardSelection.h"
#include "DibDriverData.h"
#include "DibDriverStd.h"


#if (USE_DRAGONFLY == 1)
#include "DibDriverDragonflyTest.h"
#endif

/* Function prototype */
DIBSTATUS IntDriverCheckMinMaxDemod(struct DibDriverContext *pContext, uint8_t MinNdDemod, uint8_t MaxNbDemod);

/**
 * Retreive monitoring information on filter item, and eventually clear Data information.
 * @param[in] pContext: driver context pointer
 * @param[in] ItemHdl: FIlter item handler
 * @param[out] pDataMonit: union of Data information of the stream.
 * @param[in] ClearMonit: clean Data information
 */
#if (mSDK == 0)
DIBSTATUS DibDriverGetDataMonitoring(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, enum DibBool ClearMonit)
{
   DIBSTATUS  Status   = DIBSTATUS_INVALID_PARAMETER;
   FILTER_HDL FilterHdl;

   if(!pContext)
      goto End;

   if(ItemHdl >= DIB_MAX_NB_ITEMS)
      goto End;

   FilterHdl = pContext->ItemInfo[ItemHdl].ParentFilter;

   if(FilterHdl >= DIB_MAX_NB_FILTERS)
      goto End;

   if(!pDataMonit)
      goto End;

   if((ClearMonit != eDIB_TRUE) && (ClearMonit != eDIB_FALSE))
      goto End;

   switch(pContext->FilterInfo[FilterHdl].TypeFilter)
   {
      case eTS:
         Status = DibDriverGetMonitoringRawTs(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      case eTDMB:
         Status = DibDriverGetMonitoringTdmb(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      case eDAB:
         Status = DibDriverGetMonitoringDabAudio(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      case eFIG:
         Status = DibDriverGetMonitoringFic(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      case eDABPACKET:
         Status = DibDriverGetMonitoringDabPacket(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      case eMPEFEC:
         Status = DibDriverGetMonitoringMpeFec(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      case eMPEIFEC:
         Status = DibDriverGetMonitoringMpeIFec(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      case eCMMBSVC:
         Status = DibDriverGetMonitoringCmmb(pContext, ItemHdl, pDataMonit, ClearMonit);
         break;

      default:
         DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverGetDataMonitoring, filter Type not supported" CRA));
         Status =  DIBSTATUS_ERROR;
         break;
   }

End:
   return Status;
}
#endif

/**
 * Get global monitoring information (driver version mainly).
 * @param[in]  pContext       driver context reference
 * @param[out] pGlobalInfo    output global information
 */
DIBSTATUS DibDriverGetGlobalInfo(struct DibDriverContext * pContext, struct DibGlobalInfo * pGlobalInfo)
{
   uint32_t StreamNum, FeNum;
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverGetGlobalInfo" CRA);
   }
#endif
   if(!pContext)
      goto End;

   if(!pGlobalInfo)
      goto End;

   /** retrieve embedded version, NumberOfDemods, supported Modes, Initialization status */
   DibResetEvent(&pContext->ChipContext.GlobalMonitEvent);

   Status = DibDriverGetEmbeddedInfo(pContext);

   if(Status == DIBSTATUS_SUCCESS)
      Status = DibWaitForEvent(&pContext->ChipContext.GlobalMonitEvent, EMB_VERSION_TIMEOUT_MS);

   if(Status == DIBSTATUS_SUCCESS)
   {
      /** Get embedded version of the firmware */
      pGlobalInfo->EmbVersions[0] = pContext->ChipContext.EmbVersions[0];
      pGlobalInfo->EmbVersions[1] = pContext->ChipContext.EmbVersions[1];
      pGlobalInfo->HalVersion     = pContext->ChipContext.HalVersion;
      DibMoveMemory(pGlobalInfo->HalVersionExt,
              pContext->ChipContext.HalVersionExt, sizeof(pGlobalInfo->HalVersionExt));

      /** Get Driver version */
      pGlobalInfo->DriverVersion  = DIB_INTERF_DRIVER_VERSION;
      pGlobalInfo->SupportedModes = pContext->ChipContext.SupportedModes;
      pGlobalInfo->MaxMpeServices = DIB_MAX_MPE_SERVICES;
      pGlobalInfo->NumberOfDemods = pContext->Hd.BoardConfig->NbFrontends;

      /* layout information */
      pGlobalInfo->NumberOfStreams = pContext->NbStreams;
      pGlobalInfo->NumberOfFrontends = pContext->NbFrontends;
      pGlobalInfo->NumberOfChips = pContext->NbChips;

      for(StreamNum = 0; StreamNum < pContext->NbStreams; StreamNum++)
         pGlobalInfo->Stream[StreamNum] = (STREAM_HDL)&pContext->Stream[StreamNum];

      for(FeNum = 0; FeNum < pContext->NbFrontends; FeNum++)
         pGlobalInfo->Frontend[FeNum] = (FRONTEND_HDL)&pContext->Frontend[FeNum];
   }
   else
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetEmbeddedInfo: No answer from Firmware" CRA));
   }

End:
   return Status;
}

DIBSTATUS DibDriverGetStreamAttributes(struct DibDriverContext * pContext, STREAM_HDL StreamHdl, struct DibStreamAttributes * Attributes)
{
   struct DibStream * pStream = (struct DibStream *)StreamHdl;

   *Attributes = pStream->Attributes;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverGetFrontendAttributes(struct DibDriverContext * pContext, FRONTEND_HDL FrontendHdl, struct DibFrontendAttributes * Attributes)
{
   struct DibFrontend * pFrontend = (struct DibFrontend *)FrontendHdl;

   *Attributes = pFrontend->Attributes;

   return DIBSTATUS_SUCCESS;
}

#if (DIB_PARAM_CHECK == 1)
/**
 *  Check input channel parameters
 *  @param[in] pDesc: reference to the channel descriptor to check
 */
static DIBSTATUS DibDriverCheckParamChannel(struct DibTuneChan *pDesc, enum DibDemodType Type)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   switch((int)Type)
   {
   case eSTANDARD_DVB:
   case (eSTANDARD_DVB | eFAST):
   case (eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS):
      Status = DibDriverCheckParamChannelDvb(pDesc);
      break;
   case eSTANDARD_DVBSH:
   case (eSTANDARD_DVBSH | eFAST):
      Status = DibDriverCheckParamChannelDvbSh(pDesc);
      break;
   case eSTANDARD_DAB:
   case (eSTANDARD_DAB | eALWAYS_TUNE_SUCCESS):
      Status = DibDriverCheckParamChannelDab(pDesc);
      break;
   case eSTANDARD_CMMB:
   case (eSTANDARD_CMMB | eALWAYS_TUNE_SUCCESS):
      Status = DibDriverCheckParamChannelCmmb(pDesc);
      break;
   case eSTANDARD_ISDBT:
   case eSTANDARD_ISDBT_1SEG:
   case (eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS):
      Status = DibDriverCheckParamChannelIsdbt(pDesc);
      break;
   case eSTANDARD_CTTB:
      Status = DibDriverCheckParamChannelCttb(pDesc);
      break;
   default:
      /* Need to add param check */
      break;
   }

   return Status;
}
#endif


#if (mSDK == 0)
/**
 * Aborts any pending Tune Monit Request
 */
DIBSTATUS DibDriverAbortTuneMonitChannel(struct DibDriverContext *pContext)
{
   return DibDriverSpecifAbortTuneMonitChannel(pContext);
}

/**
 * Temporarly scan a channel to found channel characteristics (fft, code rate...) and mesure signal quality (snr, ber ....). This
 * interface doesn't allocate a permanent channel. Can be used in asynchronous mode if a callback is defined. In asynchronous mode,
 * the callback won't be called is status is not DIBSTATUS_SUCCESS.
 * To improve signal quality, several demod can be added using \a MinDemod and \a MaxDemod.
 * @param[in] pContext: driver context reference
 * @param[in] MinDemod: Minimum number of Demod to use to mesure the signal quality. Can be set to DIB_DEMOD_AUTO with MaxNbDemod.
 * @param[in] MaxDemod: Maximum number of Demod to use to mesure the signal quality. Can be set to DIB_DEMOD_AUTO with MinNbDemod.
 * @param[in,out] pMonitDescriptor: Input is channel description (fft, Bw, rf...). Output are monitoring information for each demod.
 * @param[in] pCallBack: function called in asynchronous mode when scaning and monitoring are finished.
 */
DIBSTATUS DibDriverTuneMonitChannel(struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType Type, struct DibTuneMonit * pMonitDescriptor, DibTuneMonitCallback pCallBack, void *pCallCtx)
{
   DIBSTATUS          ret = DIBSTATUS_SUCCESS;
   uint32_t           ii  = 0;
   uint8_t            Free= DIB_MAX_NB_CHANNELS;
   enum DibSpectrumBW Bw  = eBW_UNSET;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverTuneMonitChannel: on %d kHz, Bw= %d" CRA,
                                 pMonitDescriptor->ChannelDescriptor.All.RFkHz,
                                 pMonitDescriptor->ChannelDescriptor.All.Bw);
   }
#endif

#if (DIB_PARAM_CHECK == 1)
   if(!pContext)
      return DIBSTATUS_INVALID_PARAMETER;

   /** in synchrone mode, pMonitDescriptor is essential */
   if(!pCallBack && !pMonitDescriptor)
      return DIBSTATUS_INVALID_PARAMETER;

   if((MaxNbDemod != DIB_DEMOD_AUTO) && (MaxNbDemod > pContext->Hd.BoardConfig->NbFrontends))
      return DIBSTATUS_INVALID_PARAMETER;

   if((MinNbDemod != DIB_DEMOD_AUTO) && (MinNbDemod > pContext->Hd.BoardConfig->NbFrontends))
      return DIBSTATUS_INVALID_PARAMETER;

   if((MaxNbDemod != DIB_DEMOD_AUTO) && (MinNbDemod > MaxNbDemod))
      return DIBSTATUS_INVALID_PARAMETER;

   if(Type == eSTANDARD_UNKNOWN)
      return DIBSTATUS_INVALID_PARAMETER;

   if(DibDriverCheckParamChannel(&pMonitDescriptor->ChannelDescriptor, Type) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverTuneMonitChannel : Channel parameters error" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }
#endif

   /* Set bandwitdh */
   Bw = pMonitDescriptor->ChannelDescriptor.Bw;

   /* Get channel lock to go through the chanel list */
   DibAcquireLock(&pContext->ChannelLock);

   /* Check Min and Max Nb demods */
   if(IntDriverCheckMinMaxDemod(pContext, MinNbDemod, MaxNbDemod) != DIBSTATUS_SUCCESS)
   {
      DibReleaseLock(&pContext->ChannelLock);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Check for a free spot and also ensure bw coherency */
   for(ii = 0; ii < DIB_MAX_NB_CHANNELS; ii++)
   {
      /** Take the first free channel : the numero of channel is choosed here and not in the firefly, in contrario of the dragonfly */
      if(pContext->ChannelInfo[ii].InUse == 0)
      {
         if(Free == DIB_MAX_NB_CHANNELS)
            Free = ii;
      }
   }

   DibReleaseLock(&pContext->ChannelLock);

   /** no more channel avalaible */
   if(Free == DIB_MAX_NB_CHANNELS)
      return DIBSTATUS_RESOURCES;

   /* Only one monit tune allowed */
   DibAcquireLock(&pContext->MonitLock);

   /* Index (Free) is protected automatically by user lock */
   IntDriverAddChannel(pContext, Free, MinNbDemod, MaxNbDemod, StreamParameters, Type, &(pMonitDescriptor->ChannelDescriptor));
   pContext->ChannelInfo[Free].Temporary = eDIB_TRUE;

   DibResetEvent(&pContext->ChannelInfo[Free].MonitContext.MonitEvent);

   /* Set default status to failure */
   pMonitDescriptor->ChannelMonit.MonitStatus     = DIB_MONIT_FAILURE;
   pMonitDescriptor->ChannelDescriptor.ScanStatus = DIB_SCAN_FAILURE;

   /* We copy the given information to the context structure. The scan() and tune()
   * functions will need them */
   DibMoveMemory(&pContext->ChannelInfo[Free].MonitContext.TuneMonit, pMonitDescriptor, sizeof(struct DibTuneMonit));

   /* register the callback if exist in the channel info structure */
   pContext->ChannelInfo[Free].MonitContext.pTuneMonitCb  = pCallBack;
   pContext->ChannelInfo[Free].MonitContext.pTuneMonitCtx = pCallCtx;

   ret = DibDriverSpecifTuneMonitChannel(pContext, Free,  MinNbDemod, MaxNbDemod, StreamParameters, pMonitDescriptor);

   /* Asynchronous call handling */
   if(!pCallBack)
   {
      /* Copy Scan result */
      DibMoveMemory(&pMonitDescriptor->ChannelDescriptor, &pContext->ChannelInfo[Free].MonitContext.TuneMonit.ChannelDescriptor, sizeof(struct DibTuneChan));

      /* Only wait for Monitoring Info if Scan Passed -- TuneMonit is always synchronous. */
      if(ret == DIBSTATUS_SUCCESS)
      {
         /* Copy Scan result */
         /*DibMoveMemory(&pMonitDescriptor->ChannelDescriptor, &pContext->ChannelInfo[Free].MonitContext.TuneMonit.ChannelDescriptor, sizeof(struct DibTuneChan));*/

         /* Wait for Monit Result */
         ret = DibWaitForEvent(&pContext->ChannelInfo[Free].MonitContext.MonitEvent, TUNE_TIMEOUT_MS);

         if(ret != DIBSTATUS_SUCCESS)
         {
            DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverTuneMonitChannel: scan timeout: no answer from firmware" CRA));
            goto End;
         }

         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel: end of tune monitoring request" CRA));
         /* To do : to optimize could only copy monit result */
         DibMoveMemory(pMonitDescriptor, &pContext->ChannelInfo[Free].MonitContext.TuneMonit, sizeof(struct DibTuneMonit));
      }


      if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_TIMEOUT) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_TIMEOUT))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel: tune monit timeout: firmware answered it could not process request" CRA));
         ret = DIBSTATUS_TIMEOUT;
      }
      else if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_FAILURE) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_FAILURE))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel: tune monit failure: scan or monit have failed" CRA));
         ret = DIBSTATUS_ERROR;
      }
      else if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_ABORTED) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_ABORTED))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel Aborted: tune monit aborted: scan or monit has been aborted" CRA));
         ret = DIBSTATUS_ABORTED;
      }
      else if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_INVALID_PARAM) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_INVALID_PARAM))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel error: tune monit failed: scan and monit request could not be completed" CRA));
         ret = DIBSTATUS_INVALID_PARAMETER;
      }
      else
      {
         ret = DIBSTATUS_SUCCESS;
      }
   }

End:
   if((ret != DIBSTATUS_SUCCESS) || (!pCallBack))
   {
      struct DibStream * pStream = pContext->ChannelInfo[Free].pStream;
      DibReleaseLock(&pContext->MonitLock);
      /** Free this channel, it is volatile */
      IntDriverRemoveChannel(pContext, Free, eDIB_FALSE);
      if(pStream)
      {
         pContext->ChannelInfo[Free].pStream = NULL;
         IntDriverStreamAutoRemoving(pContext, pStream);
      }
      DIB_DEBUG(MSG_LOG, (CRB "Channel NbDemod resetted to 0" CRA));
   }

   return ret;
}

/****************************************************************************
* Tune and get signal monitoring for a frequency
****************************************************************************/
DIBSTATUS DibDriverTuneMonitChannelEx(struct DibDriverContext *pContext, STREAM_HDL StreamHdl, struct DibTuneMonit * pMonitDescriptor, DibTuneMonitCallback pCallBack, void *pCallCtx)
{
   DIBSTATUS          ret = DIBSTATUS_SUCCESS;
   CHANNEL_HDL        ChannelHdl;
   struct DibStream * pStream = (struct DibStream *)StreamHdl;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverTuneMonitChannel: on %d kHz, Bw= %d" CRA,
                                 pMonitDescriptor->ChannelDescriptor.All.RFkHz,
                                 pMonitDescriptor->ChannelDescriptor.All.Bw);
   }
#endif

#if (DIB_PARAM_CHECK == 1)
   if(!pContext || !pMonitDescriptor || !pStream)
      return DIBSTATUS_INVALID_PARAMETER;

   if(pStream->Used == 0)
      return DIBSTATUS_INVALID_PARAMETER;

   if(pStream->AutoRemove)
   {
      /* Forbid mixt of old and new pi on the same stream */
      return DIBSTATUS_ERROR;
   }

   /** in synchrone mode, pMonitDescriptor is essential */
   if(!pCallBack && !pMonitDescriptor)
      return DIBSTATUS_INVALID_PARAMETER;

   if(DibDriverCheckParamChannel(&pMonitDescriptor->ChannelDescriptor, pStream->Std) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverTuneMonitChannel : Channel parameters error" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }
#endif

   /* Get channel lock to go through the chanel list */
   DibAcquireLock(&pContext->ChannelLock);

   /* Check for a free spot and also ensure bw coherency */
   for(ChannelHdl = 0; ChannelHdl < DIB_MAX_NB_CHANNELS; ChannelHdl++)
   {
      /** Take the first free channel : the numero of channel is choosed here and not in the firefly, in contrario of the dragonfly */
      if(pContext->ChannelInfo[ChannelHdl].InUse == 0)
         break;
   }

   DibReleaseLock(&pContext->ChannelLock);

   /** no more channel avalaible */
   if(ChannelHdl == DIB_MAX_NB_CHANNELS)
      return DIBSTATUS_RESOURCES;

   /* Only one monit tune allowed */
   DibAcquireLock(&pContext->MonitLock);

   /* Index (ChannelHdl) is protected automatically by user lock */
   IntDriverAddChannel(pContext, ChannelHdl, 0, 0, 0, pStream->Std, &(pMonitDescriptor->ChannelDescriptor));
   pContext->ChannelInfo[ChannelHdl].Temporary = eDIB_TRUE;
   pContext->ChannelInfo[ChannelHdl].pStream = pStream;

   DibResetEvent(&pContext->ChannelInfo[ChannelHdl].MonitContext.MonitEvent);

   /* Set default status to failure */
   pMonitDescriptor->ChannelMonit.MonitStatus     = DIB_MONIT_FAILURE;
   pMonitDescriptor->ChannelDescriptor.ScanStatus = DIB_SCAN_FAILURE;

   /* We copy the given information to the context structure. The scan() and tune()
   * functions will need them */
   DibMoveMemory(&pContext->ChannelInfo[ChannelHdl].MonitContext.TuneMonit, pMonitDescriptor, sizeof(struct DibTuneMonit));

   /* register the callback if exist in the channel info structure */
   pContext->ChannelInfo[ChannelHdl].MonitContext.pTuneMonitCb  = pCallBack;
   pContext->ChannelInfo[ChannelHdl].MonitContext.pTuneMonitCtx = pCallCtx;

   ret = DibDriverSpecifTuneMonitChannelEx(pContext, pStream, ChannelHdl, pMonitDescriptor);

   /* Asynchronous call handling */
   if(!pCallBack)
   {
      /* Copy Scan result */
      DibMoveMemory(&pMonitDescriptor->ChannelDescriptor, &pContext->ChannelInfo[ChannelHdl].MonitContext.TuneMonit.ChannelDescriptor, sizeof(struct DibTuneChan));

      /* Only wait for Monitoring Info if Scan Passed -- TuneMonit is always synchronous. */
      if(ret == DIBSTATUS_SUCCESS)
      {
         /* Copy Scan result */
         /*DibMoveMemory(&pMonitDescriptor->ChannelDescriptor, &pContext->ChannelInfo[ChannelHdl].MonitContext.TuneMonit.ChannelDescriptor, sizeof(struct DibTuneChan));*/

         /* Wait for Monit Result */
         ret = DibWaitForEvent(&pContext->ChannelInfo[ChannelHdl].MonitContext.MonitEvent, TUNE_TIMEOUT_MS);

         if(ret != DIBSTATUS_SUCCESS)
         {
            DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverTuneMonitChannel: scan timeout: no answer from firmware" CRA));
            goto End;
         }

         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel: end of tune monitoring request" CRA));
         /* To do : to optimize could only copy monit result */
         DibMoveMemory(pMonitDescriptor, &pContext->ChannelInfo[ChannelHdl].MonitContext.TuneMonit, sizeof(struct DibTuneMonit));
      }


      if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_TIMEOUT) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_TIMEOUT))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel: tune monit timeout: firmware answered it could not process request" CRA));
         ret = DIBSTATUS_TIMEOUT;
      }
      else if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_FAILURE) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_FAILURE))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel: tune monit failure: scan or monit have failed" CRA));
         ret = DIBSTATUS_ERROR;
      }
      else if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_ABORTED) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_ABORTED))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel Aborted: tune monit aborted: scan or monit has been aborted" CRA));
         ret = DIBSTATUS_ABORTED;
      }
      else if((pMonitDescriptor->ChannelMonit.MonitStatus == DIB_MONIT_INVALID_PARAM) || (pMonitDescriptor->ChannelDescriptor.ScanStatus == DIB_SCAN_INVALID_PARAM))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverTuneMonitChannel error: tune monit failed: scan and monit request could not be completed" CRA));
         ret = DIBSTATUS_INVALID_PARAMETER;
      }
      else
      {
         ret = DIBSTATUS_SUCCESS;
      }
   }

End:
   if((ret != DIBSTATUS_SUCCESS) || (!pCallBack))
   {
      /** ChannelHdl this channel, it is volatile */
      DibReleaseLock(&pContext->MonitLock);
      IntDriverRemoveChannel(pContext, ChannelHdl, eDIB_FALSE);
      pContext->ChannelInfo[ChannelHdl].pStream = NULL;
      IntDriverStreamAutoRemoving(pContext, pStream);
      DIB_DEBUG(MSG_LOG, (CRB "Channel NbDemod resetted to 0" CRA));
   }

   return ret;
}
#endif

/**
* Retreive the channel description information.
* @param[in] pContext: driver context reference.
* @param[in] ChannelHdl: channel handler needed to retreive channel description.
* @param[out] pDescriptor: channel description structure.
*/
DIBSTATUS DibDriverGetChannelDescriptor(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, struct DibTuneChan *pDescriptor)
{
#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverGetChannelDescriptor" CRA);
   }
#endif

   if(ChannelHdl >= DIB_MAX_NB_CHANNELS)
      return DIBSTATUS_INVALID_PARAMETER;

   if(!pDescriptor)
      return DIBSTATUS_INVALID_PARAMETER;

   if(!pContext->ChannelInfo[ChannelHdl].InUse)
      return DIBSTATUS_INVALID_PARAMETER;

   DibMoveMemory(pDescriptor, &pContext->ChannelInfo[ChannelHdl]. ChannelDescriptor, sizeof(struct DibTuneChan));

   return DIBSTATUS_SUCCESS;
}

/**
 * Give one buffer more to a filter to retreive data. The filter must be in CallBack mode (not ip mode). Data are skipped if no buffer exists.
 * If a buffer is not enough big to own one packet, the status for this buffer is overflow and another buffer (if exist) is used.
 *
 * @param[in] pContext: driver context reference
 * @param[in] FilterHdl: handler to the filter at which we give the buffer
 * @param[in] BufAdd: The user buffer address.
 * @param[in] BufSize: The maximum available size of this buffer. A minimum size is required, see values in extAPI.h
 * @param[in] BufId: An identifer for user purpose only. Not used in the SDK. However, the address is used to remove the buffer if needed.
 *
 */
#if (mSDK == 0)
DIBSTATUS DibDriverRegisterBuffer(struct DibDriverContext * pContext, FILTER_HDL FilterHdl, uint8_t * BufAdd, uint32_t BufSize, uint8_t BufId)
{
   uint8_t          BufIdx;
   uint8_t          FreeIdx;
   enum DibDataType FilterType;
   uint32_t         MinSize;
   DIBSTATUS        Status = DIBSTATUS_INVALID_PARAMETER;

   DibDriverTargetDisableIrqProcessing(pContext);

   if(FilterHdl >= DIB_MAX_NB_FILTERS)
      goto End;

   FilterType = pContext->FilterInfo[FilterHdl].TypeFilter;

   if(pContext->FilterInfo[FilterHdl].ParentChannel == DIB_UNSET)
      goto End;

   if(!BufAdd)
      goto End;

   /** Check buffer size depending on filter type */
   switch(FilterType)
   {
      case eTS:
      case ePESVIDEO:
      case ePESAUDIO:
      case ePESOTHER:
      case eTDMB:
         MinSize = DIB_TS_PACKET_SIZE;
         break;
      case ePCR:
         MinSize = DIB_PCR_DATA_SIZE;
         break;
      case eSIPSI:
         MinSize = DIB_SIPSI_SECTION_SIZE;
         break;
      case eEDAB:
      case eMPEFEC:
      case eMPEIFEC:
         MinSize = DIB_IP_PACKET_SIZE;
         break;
      case eFM:
         MinSize = DIB_FM_SECTION_SIZE;
         break;
      case eANALOG:
         MinSize = DIB_VIDEO_PACKET_SIZE;
         break;
      case eFIG:
         MinSize = DIB_FIC_BLOCK_SIZE;
         break;
      case eDABPACKET:
      case eDAB:
         MinSize = DIB_DAB_PACKET_SIZE;
         break;
      case eCMMBSVC:
         MinSize = DIB_CMMB_RSM_SIZE;
         break;
      case eUNKNOWN_DATA_TYPE:
      default:
         goto End;
   }

   if(BufSize < MinSize)
      goto End;


   /** Is there any free context for a new callback ? */
   for(FreeIdx = 0; FreeIdx < DIB_MAX_NB_BUF_CONTEXT; FreeIdx++)
   {
      if(pContext->CallbackBufCtx[FreeIdx].Ctx.FilterIdx == DIB_UNSET)
      {
         break;
      }
   }

   if(FreeIdx == DIB_MAX_NB_BUF_CONTEXT)
   {
      Status = DIBSTATUS_RESOURCES;
      goto End;
   }

   /** Memorisation of the new Buffer */
   BufIdx = pContext->FilterInfo[FilterHdl].FirstBuf;

   if(BufIdx == DIB_UNSET)
   {
      pContext->FilterInfo[FilterHdl].FirstBuf = FreeIdx;
   }
   else
   {
      while(pContext->CallbackBufCtx[BufIdx].NextBuf != DIB_UNSET)
      {
         BufIdx = pContext->CallbackBufCtx[BufIdx].NextBuf;

         if(BufIdx >= DIB_MAX_NB_BUF_CONTEXT)
         {
            Status = DIBSTATUS_ERROR;
            goto End;
         }
      }
      pContext->CallbackBufCtx[BufIdx].NextBuf = FreeIdx;
   }

   pContext->CallbackBufCtx[FreeIdx].Ctx.FilterIdx = FilterHdl;
   pContext->CallbackBufCtx[FreeIdx].Ctx.BufId     = BufId;
   pContext->CallbackBufCtx[FreeIdx].Ctx.BufAdd    = BufAdd;
   pContext->CallbackBufCtx[FreeIdx].Ctx.BufSize   = BufSize;
   pContext->CallbackBufCtx[FreeIdx].Ctx.FillSize  = 0;
   pContext->CallbackBufCtx[FreeIdx].NextBuf       = DIB_UNSET;

   Status = DIBSTATUS_SUCCESS;

End:
   DibDriverTargetEnableIrqProcessing(pContext);
   return Status;
}

/**
 * Retreive a previously registered buffer if not already used.
 *
 * @param[in] pContext: driver context reference
 * @param[in] FilterHdl: handler to the filter at which we give the buffer
 * @param[in] BufAdd: The user buffer address.
 *
 */
DIBSTATUS DibDriverUnregisterBuffer(struct DibDriverContext * pContext, FILTER_HDL FilterHdl, uint8_t * BufAdd)
{
   uint8_t BufIdx, LastIdx = DIB_UNSET, NextIdx = DIB_UNSET;
   DIBSTATUS        Status = DIBSTATUS_INVALID_PARAMETER;

   DibDriverTargetDisableIrqProcessing(pContext);

   if(FilterHdl >= DIB_MAX_NB_FILTERS)
      goto End;

   if((pContext->FilterInfo[FilterHdl].TypeFilter == eUNKNOWN_DATA_TYPE) || (pContext->FilterInfo[FilterHdl].ParentChannel == DIB_UNSET))
      goto End;

   BufIdx = pContext->FilterInfo[FilterHdl].FirstBuf;

   while(BufIdx != DIB_UNSET)
   {
      if(pContext->CallbackBufCtx[BufIdx].Ctx.BufAdd == BufAdd)
         break;

      LastIdx = BufIdx;

      BufIdx = pContext->CallbackBufCtx[BufIdx].NextBuf;
   }

   /** Check is buffer really exist */
   if(BufIdx == DIB_UNSET)
   {
      Status = DIBSTATUS_ERROR;
      goto End;
   }

   NextIdx = pContext->CallbackBufCtx[BufIdx].NextBuf;

   /** Remove the buffer from the filter list */
   if(LastIdx == DIB_UNSET)
      pContext->FilterInfo[FilterHdl].FirstBuf = NextIdx;
   else
      pContext->CallbackBufCtx[LastIdx].NextBuf = NextIdx;

   /** Remove the buffer entry  */
   pContext->CallbackBufCtx[BufIdx].NextBuf       = DIB_UNSET;
   pContext->CallbackBufCtx[BufIdx].Ctx.FilterIdx = DIB_UNSET;
   pContext->CallbackBufCtx[BufIdx].Ctx.BufId     = 0;
   pContext->CallbackBufCtx[BufIdx].Ctx.BufAdd    = 0;
   pContext->CallbackBufCtx[BufIdx].Ctx.BufSize   = 0;
   pContext->CallbackBufCtx[BufIdx].Ctx.FillSize  = 0;

   Status = DIBSTATUS_SUCCESS;
End:

   DibDriverTargetEnableIrqProcessing(pContext);
   return Status;
}
#endif

/**
 * Retreive monitoring information (SNR, BER.. measurement) on a channel.
 * Can be used in asynchronous mode if a callback is defined.
 * @param[in] pContext: driver context.
 * @param[in] ChannelHdl: handler of the channel to monitor.
 * @param[out] buf: Monitoring output, for each demod linked to the channel.
 * @param[in] pCallBack: if defined, asynchronously call this function when monitoring is finished.
 * @param[in] pCallCtx: User context when calling the function.
 * @return DIBSTATUS.
 */
DIBSTATUS DibDriverGetSignalMonitoring(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, uint8_t * NbDemod, union DibDemodMonit * buf, DibGetSignalCallback pCallBack, void *pCallCtx)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;
   uint32_t NbConnFrontends;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      uint8_t *format_s[2] = { (uint8_t *) "Synchronous", (uint8_t *) "Asynchronous" };

      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverGetSignalMonitoring: %s on tsChannel %d" CRA, format_s[sync], buf->TsChannelNumber);
   }
#endif

   if(!pContext || !NbDemod)
      return DIBSTATUS_INVALID_PARAMETER;

   if((ChannelHdl >= DIB_MAX_NB_CHANNELS) || (!pContext->ChannelInfo[ChannelHdl].InUse))
      return DIBSTATUS_INVALID_PARAMETER;

   /** The user knows the number of demod for this channel and wants the monit for a part of them */
   if(!buf)
      return DIBSTATUS_INVALID_PARAMETER;

   /* Only one monit tune allowed */
   DibAcquireLock(&pContext->SignalMonitLock);
   DibDriverTargetDisableIrqProcessing(pContext);
   if(pContext->ChannelInfo[ChannelHdl].InUse == 0)
   {
      Status = DIBSTATUS_INVALID_PARAMETER;
      goto End;
   }

   if((*NbDemod) == 0)
   {
      Status = DIBSTATUS_INVALID_PARAMETER;
      goto End;
   }

   NbConnFrontends = pContext->ChannelInfo[ChannelHdl].pStream->NbConnFrontends;

   /** The user does not know the number of demodulators activated for this channel */
   if((*NbDemod) == DIB_UNSET)
   {
      /** Warning: monitoring concerns only activated demods, not total nb frontends (in pBoard) */
      (*NbDemod) = NbConnFrontends;
      Status = DIBSTATUS_SUCCESS;
      /* Terminate this function as if we have no callback to always release the signal lock */
      pCallBack = NULL;
      goto End;
   }

   if((*NbDemod) > NbConnFrontends)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverGetSignalMonitoring request. more demod than %d on the channel %d : %d" CRA, NbConnFrontends, ChannelHdl, *NbDemod));
      (*NbDemod) = NbConnFrontends;
   }

   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetSignalMonitoring requesting monit info on Channel %d for %d Demods" CRA, ChannelHdl, *NbDemod));
   DibResetEvent(&pContext->ChannelInfo[ChannelHdl].MonitContext.MonitEvent);

   pContext->ChannelInfo[ChannelHdl].pGetSignalCb  = pCallBack;
   pContext->ChannelInfo[ChannelHdl].pGetSignalCtx = pCallCtx;
   pContext->ChannelInfo[ChannelHdl].NbDemodsUser  = (*NbDemod);

   DibDriverTargetEnableIrqProcessing(pContext);

   Status = DibDriverGetMonit(pContext, ChannelHdl);

   if((Status == DIBSTATUS_SUCCESS) && !pCallBack)
      Status = DibWaitForEvent(&pContext->ChannelInfo[ChannelHdl].MonitContext.MonitEvent, MONIT_TIMEOUT_MS);

   DibDriverTargetDisableIrqProcessing(pContext);
   if(Status != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetSignalMonitoring: monit timeout: no answer from firmware" CRA));
      goto End;
   }

   /* Protecting from content changes */
   if(!pCallBack)
   {
      DibMoveMemory(buf, &(pContext->ChannelInfo[ChannelHdl].ChannelMonit.DemodMonit), (*NbDemod)*sizeof(union DibDemodMonit));

      if(pContext->ChannelInfo[ChannelHdl].ChannelMonit.MonitStatus == DIB_MONIT_TIMEOUT)
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetSignalMonitoring: monit timeout: firmware answered it could not process request" CRA));
         Status = DIBSTATUS_TIMEOUT;
      }
      else if(pContext->ChannelInfo[ChannelHdl].ChannelMonit.MonitStatus == DIB_MONIT_FAILURE)
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetSignalMonitoring: monit failed" CRA));
         Status = DIBSTATUS_ERROR;
      }
      else if(pContext->ChannelInfo[ChannelHdl].ChannelMonit.MonitStatus == DIB_MONIT_PARTIAL)
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetSignalMonitoring: monit partial" CRA));

         Status = DIBSTATUS_PARTIAL;
      }
   }

End:
   DibDriverTargetEnableIrqProcessing(pContext);

   if((Status != DIBSTATUS_SUCCESS) || (!pCallBack))
      DibReleaseLock(&pContext->SignalMonitLock);

   return Status;
}


/**
 * Runtime parameter changing. This function can do the following updates:
 * - Turn on/off a PID to/from prefetch mode
 * @param[in] pContext: driver context
 * @param[in] Param: Parameter type to modify.
 * @param[in] pParamConfig: Config.Parameter whom configuration need to be changed.
 */
DIBSTATUS DibDriverSetConfig(struct DibDriverContext *pContext, enum DibParameter Param, union DibParamConfig * pParamConfig)
{
   DIBSTATUS ret = DIBSTATUS_INVALID_PARAMETER;
   uint8_t ii = 0;

   if(!pContext || !pParamConfig)
      return DIBSTATUS_INVALID_PARAMETER;

   switch(Param)
   {
      case ePID_MODE:
         {
            uint32_t ElemHdl = pParamConfig->PidMode.ElemHdl;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
            DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
            DibLogToFileMesg(gLogFile, CRB "DibDriverSetConfig: Item %d Mode %d" CRA,
                                         pParamConfig->PidMode.ElemHdl, pParamConfig->PidMode.Mode);
#endif

            if((ElemHdl >= DIB_MAX_NB_ITEMS) || (pContext->ItemInfo[ElemHdl].ParentFilter == DIB_UNSET))
               goto End;

            if((pParamConfig->PidMode.Mode != ePREFETCH) && (pParamConfig->PidMode.Mode != eACTIVE))
               goto End;

            ret = DibDriverSetPidMode(pContext, &pParamConfig->PidMode);
         }
         break;

      case eTIMESLICING:
#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
         {
            DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
            DibLogToFileMesg(gLogFile, CRB "DibDriverSetConfig: %s time slicing : %d" CRA, pParamConfig->TimeSlicing);
         }
#endif

         if((pParamConfig->TimeSlicing.Enable != eDIB_TRUE) && (pParamConfig->TimeSlicing.Enable != eDIB_FALSE))
         {
            ret = DIBSTATUS_INVALID_PARAMETER;
         }
         else
         {
            DibAcquireLock(&pContext->ChannelLock);
            ret = DibDriverSetPowerManagement(pContext, pParamConfig->TimeSlicing.Enable, pParamConfig->TimeSlicing.PhysicalStream);
            DibReleaseLock(&pContext->ChannelLock);
         }

        break;

      case eDEBUG_MODE:
         {
            uint16_t Fonction, Value, ChipId;
            DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverSetConfig - SetDebugMode: Driver Debug Mask set to %08X" CRA, pParamConfig->DebugMode.DriverDebugMask));
            pContext->DriverDebugMask = pParamConfig->DebugMode.DriverDebugMask;

            Fonction = (uint16_t) (pParamConfig->DebugMode.DHelp & 0xffff);            /* Warning : only 8 bits for Firefly */
            Value    = (uint16_t) (pParamConfig->DebugMode.DHelp >> 16);               /* Warning : only 8 bits for Firefly */
            ChipId    = (uint16_t) (pParamConfig->DebugMode.ChipId);                   /* Warning : Set to 0 for Firefly */

            DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverSetConfig - SetDebugMode: Debug help(%d) = %02X" CRA, Fonction, Value));
            DibDriverSetDebugHelp(pContext, Fonction, Value, ChipId);

            ret = DIBSTATUS_SUCCESS;
         }
         break;

      case eEVENT:
         if(pParamConfig->EventMode.Event >= eMAX_NB_EVENTS)
            return DIBSTATUS_INVALID_PARAMETER;

         /** Protect Driver from event callback modification */
         DibDriverTargetDisableIrqProcessing(pContext);

         for(ii = 0; ii < eMAX_NB_EVENTS; ii++)
         {
            if(pContext->EventCallbackDesc[ii].List == pParamConfig->EventMode.Callback)
            {
               break;
            }
         }
         ret = DIBSTATUS_SUCCESS;
         if(ii == eMAX_NB_EVENTS)
         {
            ret = DIBSTATUS_INVALID_PARAMETER;
         }
         else if(pParamConfig->EventMode.Mode == eREMOVE_EVENT)
         {
            /** does this event exist ? */
            if(pContext->EventTypeCallbackIndex[pParamConfig->EventMode.Event] != DIB_UNSET)
               pContext->EventTypeCallbackIndex[pParamConfig->EventMode.Event] = DIB_UNSET;
            else
               ret = DIBSTATUS_INVALID_PARAMETER;
         }
         else if(pParamConfig->EventMode.Mode == eADD_EVENT)
         {
            /** does this event not already exist ? */
            if(pContext->EventTypeCallbackIndex[pParamConfig->EventMode.Event] == DIB_UNSET)
               pContext->EventTypeCallbackIndex[pParamConfig->EventMode.Event] = ii;
            else
               ret = DIBSTATUS_INVALID_PARAMETER;
         }
         else
         {
            ret = DIBSTATUS_INVALID_PARAMETER;
         }
         DibDriverTargetEnableIrqProcessing(pContext);
         break;

      case eSIPSI_MASK:
         ret = DibDriverSetConfigSiPsi(pContext, pParamConfig);
         break;

      case eSET_HBM_MODE:
         ret = DibDriverSetHbm(pContext, pParamConfig->HbmMode);
         break;

      case eCAS_ENABLE:
         DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverSetConfig - Enable/Disable CAS %d" CRA, pParamConfig->CasEnable.Enable));
         ret = DibDriverEnableCas(pContext, pParamConfig->CasEnable.Enable);
         break;

      default:
         ret = DIBSTATUS_INVALID_PARAMETER;
         break;
   }

End:
   return ret;
}


/**
 * Query info about specific settings. This function can do the following updates:
 * @param[in] pContext: driver context
 * @param[in] Param: Parameter type to modify.
 * @param[in] pParamConfig: Config.Parameter whom configuration need to be changed.
 */
DIBSTATUS DibDriverGetConfig(struct DibDriverContext *pContext, enum DibParameter Param, union DibParamConfig *pParamConfig)
{
   DIBSTATUS ret = DIBSTATUS_INVALID_PARAMETER;

   if(!pContext || !pParamConfig)
      return DIBSTATUS_INVALID_PARAMETER;

   switch(Param)
   {
      case ePID_MODE:
            ret = DibDriverGetPidMode(pContext, pParamConfig);
         break;

       case eTIMESLICING:
#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
         {
            DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
            DibLogToFileMesg(gLogFile, CRB "DibDriverGetConfig: %s time slicing : %d" CRA, pParamConfig->TimeSlicing);
         }
#endif
         DibAcquireLock(&pContext->ChannelLock);
         ret = DibDriverGetPowerManagement(pContext, pParamConfig->TimeSlicing.PhysicalStream, &pParamConfig->TimeSlicing.Enable);
         DibReleaseLock(&pContext->ChannelLock);

         break;

      case eDEBUG_MODE:
         {
            DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverGetConfig - GetDebugMode: Driver Debug Mask  %08X" CRA, pContext->DriverDebugMask));

            pParamConfig->DebugMode.DriverDebugMask = pContext->DriverDebugMask;
            ret = DIBSTATUS_SUCCESS;
         }
         break;

      case eEVENT:
         if(pParamConfig->EventMode.Event >= eMAX_NB_EVENTS)
            return DIBSTATUS_INVALID_PARAMETER;

         /** Protect Driver from event callback modification */
         DibDriverTargetDisableIrqProcessing(pContext);
         ret                              = DIBSTATUS_SUCCESS;
         pParamConfig->EventMode.Mode     = (pContext->EventTypeCallbackIndex[pParamConfig->EventMode.Event] == DIB_UNSET) ? eREMOVE_EVENT : eADD_EVENT;
         pParamConfig->EventMode.Callback = pContext->EventCallbackDesc[pParamConfig->EventMode.Event].List;
         DibDriverTargetEnableIrqProcessing(pContext);
         break;

      case eSIPSI_MASK:
         ret = DibDriverGetConfigSiPsi(pContext, pParamConfig);
         break;

      default:
         ret = DIBSTATUS_INVALID_PARAMETER;
         break;
   }

   return ret;
}

/****************************************************************************
* allocate a stream
****************************************************************************/
DIBSTATUS DibDriverGetStream(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream,
        enum DibDemodType        Standard,
        uint32_t                 OutputOptions,
        enum DibPowerMode        EnPowerSaving)
{
   DIBSTATUS Status;
   struct DibStream * pStream = (struct DibStream *)Stream;

   /* Check the Standard */
   if(!pContext || !pStream)
      return DIBSTATUS_INVALID_PARAMETER;

   if(Standard >= eMAX_STANDARD_DEMOD)
      return DIBSTATUS_INVALID_PARAMETER;

   /* Check that the stream is not used. */
   if(pStream->Used)
      return DIBSTATUS_ERROR;

   Status = DibDriverSpecifGetStream(pContext, pStream, Standard, OutputOptions, EnPowerSaving);

   if(Status == DIBSTATUS_SUCCESS)
   {
      pStream->Used = 1;
      pStream->AutoRemove = 0;

      if(EnPowerSaving == ePOWER_SAVING)
         pStream->ChipTimeSlicing = 1;
      else if(EnPowerSaving == ePOWER_ON)
         pStream->ChipTimeSlicing = 0;
      /* else value remains unchanged */

      pStream->Options = OutputOptions;
      pStream->Std = Standard;
   }

   return Status;
}


/****************************************************************************
* add a frontend to a stream
****************************************************************************/
DIBSTATUS DibDriverAddFrontend(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream,
        FRONTEND_HDL             Fe,
        FRONTEND_HDL             FeOutput, /**< NULL - automatic mode, parameter is unused for now - will be used in some cases in future releases */
        uint32_t                 Force     /**< Force remove of Frontend if already in use - calls implicitly DibRemoveFrontend() */
)
{
   DIBSTATUS Status;
   struct DibStream * pStream = (struct DibStream *)Stream;
   struct DibFrontend * pFrontend = (struct DibFrontend *)Fe;
   struct DibFrontend * pOutputFrontend = (struct DibFrontend *)FeOutput;

   /* Check the Standard */
   if(!pContext || !pStream || !pFrontend)
      return DIBSTATUS_INVALID_PARAMETER;

   /* Check that the stream is used. */
   if(pStream->Used == 0)
      return DIBSTATUS_ERROR;

   /* Check that if we force removal the frontend has a stream connected */
   if(Force && (pFrontend->pStream == 0))
      return DIBSTATUS_ERROR;

   if(Force)
   {
      /* Remove the frontend from the stream before adding it to another one */
      Status = DibDriverRemoveFrontend(pContext, Fe);
      if(Status != DIBSTATUS_SUCCESS)
         return Status;
   }

   /* Check that the stream is not used. */
   if(pFrontend->pStream)
      return DIBSTATUS_ERROR;

   /* Check that the output frontend on the same stream. */
   if(pOutputFrontend)
      return DIBSTATUS_ERROR;

   Status = DibDriverSpecifAddFrontend(pContext, pStream, pFrontend, NULL);

   if(Status == DIBSTATUS_SUCCESS)
   {
      struct DibFrontend ** pInputPointer;
      struct DibFrontend * pFe;

      pFrontend->pStream = pStream;

      pInputPointer = &pStream->pInput;

      /* sort the list in increasing order */
      pFe = pStream->pInput;
      while(pFe)
      {
         if(pFe->FeId > pFrontend->FeId)
            break;

         pInputPointer = &pFe->pInput;
         pFe = pFe->pInput;
      }

      /* Save the next frontend */
      pFe = *pInputPointer;

      /* Insert the pFrontend */
      *pInputPointer = pFrontend;
      pFrontend->pInput = pFe;
      pStream->NbConnFrontends++;
   }

   return Status;
}

/****************************************************************************
* tune on a frequency
****************************************************************************/
DIBSTATUS DibDriverGetChannelEx(
        struct DibDriverContext *pContext,
        STREAM_HDL               Hdl,
        struct  DibChannel      *pDescriptor,
        CHANNEL_HDL             *pChannelHdl
)
{
   struct DibStream * pStream = (struct DibStream *)Hdl;

   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverGetChannel" CRA );
   }
#endif

   if(!pContext || !pDescriptor || !pStream)
      return DIBSTATUS_INVALID_PARAMETER;

   if(pStream->Used == 0)
      return DIBSTATUS_INVALID_PARAMETER;

   if(pStream->AutoRemove)
   {
      /* Forbid mixt of old and new pi on the same stream */
      return DIBSTATUS_ERROR;
   }

   DibAcquireLock(&pContext->ChannelLock);

#if (DIB_PARAM_CHECK == 1)
   if(DibDriverCheckParamChannel(&pDescriptor->ChannelDescriptor, pStream->Std) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB " DibDriverGetChannel : Channel parameters error" CRA));
      goto End;
   }
#endif

   Status = DibDriverSpecifGetChannelEx(pContext, pStream, pDescriptor, pChannelHdl);

End:
   DibReleaseLock(&pContext->ChannelLock);

   return Status;

}


/****************************************************************************
* Retreive specific frontend monitoring
****************************************************************************/
DIBSTATUS DibDriverGetFrontendMonit(
        struct DibDriverContext *pContext,
        FRONTEND_HDL             Fe,
        union DibDemodMonit     *Monit
)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   return Status;
}

/****************************************************************************
* removes frontend instance from stream
****************************************************************************/
DIBSTATUS DibDriverRemoveFrontend(
        struct DibDriverContext *pContext,
        FRONTEND_HDL             Fe
)
{
   DIBSTATUS Status;

   struct DibFrontend * pFrontend = (struct DibFrontend *)Fe;
   struct DibStream * pStream;

   /* Check the Standard */
   if(!pContext || !pFrontend)
      return DIBSTATUS_INVALID_PARAMETER;

   pStream = pFrontend->pStream;

   /* Check that the frontend is used. */
   if(pStream == 0)
      return DIBSTATUS_ERROR;

   Status = DibDriverSpecifRemoveFrontend(pContext, pFrontend);

   if(Status == DIBSTATUS_SUCCESS)
   {
      struct DibFrontend ** pInputPointer = &pStream->pInput;
      struct DibFrontend * pFe = pStream->pInput; /* should not be null */

      /* update linked list (should match) */
      while(pFe && (pFe != pFrontend))
      {
         pInputPointer = &pFe->pInput;
         pFe = pFe->pInput;
      }
      *pInputPointer = pFrontend->pInput;

      /* free the frontend */
      pFrontend->pInput = 0;
      pFrontend->pStream = 0;
      pStream->NbConnFrontends--;
   }

   return Status;
}

/****************************************************************************
* Convenience function to remove from a stream the last connected frontend
****************************************************************************/
DIBSTATUS IntDriverRemoveLastFrontendFromStream(struct DibDriverContext *pContext, struct DibStream * pStream)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   struct DibFrontend * pFe;

   /* Find the last connected frontend */
   pFe = pStream->pInput;

   if(pFe)
   {
      while(pFe->pInput) pFe = pFe->pInput;

      Status = DibDriverRemoveFrontend(pContext, pFe);
   }

   return Status;
}

/****************************************************************************
* Convenience function to find the next pluggable frontend to a stream for
* a the current instanciated standard of the stream if exists or using the std passed
* as argument otherwise
****************************************************************************/
struct DibFrontend * IntDriverGetFrontendForStream(struct DibStream * pStream, enum DibDemodType Standard)
{
   uint32_t Std = (1 << Standard);
   uint32_t i = 0;

   struct DibFrontend *pFrontend = NULL;
   struct DibFrontend *pFe = NULL;

   /* if stream is connected, check that the standard is the same */
   if(pStream->Used && (pStream->Std != Standard))
      return NULL;

   /* if adapter has no current inputs */
   if(pStream->pInput == NULL)
   {
      pFrontend = (struct DibFrontend *)pStream->Attributes.PossibleDirectInputs[i].Fe;
      while(pFrontend)
      {
         /* If frontend is not used and is pluggable for Std, its ok */
         if( (pFrontend->pStream == NULL) && (pStream->Attributes.PossibleDirectInputs[i].Standard & Std) )
            break;
         i++;
         pFrontend = (struct DibFrontend *)pStream->Attributes.PossibleDirectInputs[i].Fe;
      }
   }
   else
   {
      /* find the last frontend of the Input list and browse input caps for this last frontend */
      for (pFe = pStream->pInput; pFe->pInput != NULL; pFe = pFe->pInput);

      pFrontend = (struct DibFrontend *)pFe->Attributes.PossibleDirectInputs[i].Fe;
      while(pFrontend)
      {
         /* If frontend is not used and is pluggable for Std, its ok */
         if( (pFrontend->pStream == NULL) && (pFe->Attributes.PossibleDirectInputs[i].Standard & Std) )
            break;
         i++;
         pFrontend = (struct DibFrontend *)pFe->Attributes.PossibleDirectInputs[i].Fe;
      }
   }

   return pFrontend;
}

/****************************************************************************
* Convenience function to update the number of frontends depending on Min/Max demods
****************************************************************************/

void IntDriverUpdateFrontends(struct DibDriverContext *pContext, struct DibStream * pStream, uint32_t MinDemod, uint32_t MaxDemod)
{
   /*
    * Update the number of ConnectedFrontends depending on the strategy, the min/max demods, and the pStream->NbConnected
    * Note we should check min/max demods for all channels of the stream but as this is the old api...
    */
   uint32_t NbDemod = pStream->NbConnFrontends;
   struct DibFrontend * pFe;

   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverUpdateFrontends : Wanted [%d,%d] demods on stream %d. Demods %d =>",
             MinDemod, MaxDemod, pStream->StreamId, NbDemod));

   if(NbDemod > MaxDemod)
   {
      /* remove exceeding frontents */
      while ((NbDemod > MaxDemod) && (IntDriverRemoveLastFrontendFromStream(pContext, pStream) == DIBSTATUS_SUCCESS))
         NbDemod--;
   }
   else if(NbDemod < MaxDemod)
   {
      /* add the maximum possible frontends that have the good capability */
      while(((pFe = IntDriverGetFrontendForStream(pStream, pStream->Std)) != 0) && (NbDemod < MaxDemod))
      {
         if(DibDriverAddFrontend(pContext, (STREAM_HDL)pStream, (FRONTEND_HDL)pFe, NULL, 0) == DIBSTATUS_SUCCESS)
            NbDemod++;
         else
            break;
      }
   }

   DIB_DEBUG(SOFT_IF_LOG, (" %d\n" CRA, NbDemod));
}

/*
 * Check if stream was created unsing the old api, and if yes check that if we removed the last channel,
 * the stream needs to be deleted
 */
DIBSTATUS IntDriverStreamAutoRemoving(struct DibDriverContext *pContext, struct DibStream * pStream)
{
   uint32_t ChannelId;
   if(pStream->AutoRemove)
   {
      for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
      {
         if(pContext->ChannelInfo[ChannelId].pStream == pStream)
            break;
      }
      if(ChannelId == DIB_MAX_NB_CHANNELS)
      {
         return DibDriverDeleteStream(pContext, (STREAM_HDL)pStream);
      }
   }
   return DIBSTATUS_SUCCESS;
}


/****************************************************************************
* Releases all resources allocated to this stream (Frontends and Channels)
****************************************************************************/
DIBSTATUS DibDriverDeleteStream(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream
)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   uint32_t ChannelNum;

   struct DibStream * pStream = (struct DibStream *)Stream;

   /* Check the Standard */
   if(!pContext || !pStream)
      return DIBSTATUS_INVALID_PARAMETER;

   /* Check that the stream is used. */
   if(pStream->Used == 0)
      return DIBSTATUS_ERROR;

   if (!pStream->AutoRemove) 
   {
       /** delete every channel connected */
       for(ChannelNum = 0; (Status == DIBSTATUS_SUCCESS) && (ChannelNum < DIB_MAX_NB_CHANNELS); ChannelNum++)
       {
           if(pContext->ChannelInfo[ChannelNum].pStream == pStream) {
               Status = DibDriverDeleteChannel(pContext, ChannelNum);
           }
       }
   }

   /* delete every connected frontends. Warning, this should be done in the inverse order of insertion */
   while((Status == DIBSTATUS_SUCCESS) && pStream->pInput)
   {
      Status = IntDriverRemoveLastFrontendFromStream(pContext, pStream);
   }

   if(Status == DIBSTATUS_SUCCESS) {
      Status = DibDriverSpecifDeleteStream(pContext, pStream);
   }

   if(Status == DIBSTATUS_SUCCESS)
   {
      pStream->Used = 0;
      pStream->AutoRemove = 0;
   }

   return Status;
}


/**
 * Get a new channel. If a callback is set (not NULL), the channel is really allocated once the callback is sent, providing scanning status is
 * successfull. In asynchronous mode, the channel handler is indicated in the channel descriptor.
 * @param pContext pointer to the current context
 * @param MinNbDemod: min number of demodulators for this channel. Can be set to DIB_DEMOD_AUTO.
 * @param MaxNbDemod: max number of demodulators for this channel. Can be set to DIB_DEMOD_AUTO.
 * @param PriorityChannel  indicate if this channel will be used to retreive the Data. Among Channels linked to the same adapter (each channel
 *        is associated with an adapter (ie a tuner)), only one is able to give Data. This is the case for DVB-H because two services existing
 *        on two different frequencies are not synchronized together. It is mandatory that one and only one Active channel exists.
 * @param pDescriptor: pointer to a structure with all the useful information for the configuration of the channel
 * @param pChannelHdl: reference for the new channel, if successful
 * @return: DIBSTATUS
 */

DIBSTATUS DibDriverGetChannel(struct DibDriverContext * pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibChannel * pDescriptor, CHANNEL_HDL * pChannelHdl)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverGetChannel" CRA );
   }
#endif

   if(!pContext || !pDescriptor)
      return DIBSTATUS_INVALID_PARAMETER;

   DibAcquireLock(&pContext->ChannelLock);

#if (DIB_PARAM_CHECK == 1)
   if((pDescriptor->Type >= eMAX_STANDARD_DEMOD) && (pDescriptor->Type != (eSTANDARD_DVB | eFAST)))
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB " DibDriverGetChannel : Type of demodulation does not exist" CRA));
      goto End;
   }

   if(DibDriverCheckParamChannel(&pDescriptor->ChannelDescriptor, pDescriptor->Type) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB " DibDriverGetChannel : Channel parameters error" CRA));
      goto End;
   }

   if(IntDriverCheckMinMaxDemod(pContext, MinNbDemod, MaxNbDemod) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_INVALID_PARAMETER;

#endif

   Status = DibDriverSpecifGetChannel(pContext, MinNbDemod, MaxNbDemod, StreamParameters, pDescriptor, pChannelHdl);

End:
   DibReleaseLock(&pContext->ChannelLock);

   return Status;
}

/**
 * Delete a channel
 * @param pContext: pointer to the driver context
 * @param ChannelHdl: Handler of the channel that the user want to delete
 */
DIBSTATUS DibDriverDeleteChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl)
{
   DIBSTATUS res = DIBSTATUS_INVALID_PARAMETER;
   uint32_t ChannelIndex;
   struct DibStream *pStream;
   

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverDeleteChannel:  %d" CRA, ChannelHdl);
   }
#endif

   if(!pContext)
      return DIBSTATUS_INVALID_PARAMETER;

   DibAcquireLock(&pContext->MonitLock);
   DibAcquireLock(&pContext->SignalMonitLock);
   DibAcquireLock(&pContext->ChannelLock);

   if(ChannelHdl >= DIB_MAX_NB_CHANNELS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverDeleteChannel : numero of  channel over than the max (%d)" CRA, ChannelHdl));
      goto End;
   }

   if(!pContext->ChannelInfo[ChannelHdl].InUse)
      goto End;

   DIB_ASSERT(pContext->ChannelInfo[ChannelHdl].Type != eSTANDARD_UNKNOWN);

   /* Get pStream and check if channel use 'old' API */
   /* If old API => remove streams/frontends */
   pStream = pContext->ChannelInfo[ChannelHdl].pStream;

   res = DibDriverSpecifDeleteChannel(pContext, ChannelHdl);
   for(ChannelIndex = 0; ChannelIndex < DIB_MAX_NB_CHANNELS; ChannelIndex++)
   {
      if(pContext->ChannelInfo[ChannelIndex].InUse)
            break;
   }

   if (pStream->AutoRemove)
   {
       res = DibDriverDeleteStream(pContext, pStream);
       if (res != DIBSTATUS_SUCCESS)
           goto End;
   }

End:
   DibReleaseLock(&pContext->ChannelLock);
   DibReleaseLock(&pContext->SignalMonitLock);
   DibReleaseLock(&pContext->MonitLock);
   return res;
}

#if (mSDK == 0)
/**
 * Create a new filter for one channel.
 * @param pContext: pointer to the current context.
 * @param ChannelHdl: handler of the channel on which we add a filter.
 * @param DataType: Type of the filter (SIPSI, MPEFEC...).
 * @param pDataBuf: Buffering Strat: IP or CallBack. If null, the driver sends packet to IP stack by default (if type is eDAB, DABIP or MPEFEC).
 * @param pFilterHdl: pointer of the new filter, if command successful, choosed by the firmware in dragonfly case.
 * @return: result of the command.
 */
DIBSTATUS DibDriverCreateFilter(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer * pDataBuf, FILTER_HDL * pFilterHdl)
{
   enum DibDataMode DataMode;
   DIBSTATUS        res     = DIBSTATUS_INVALID_PARAMETER;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverCreateFilter:  %d with DataType %d" CRA, ChannelHdl, DataType);
   }
#endif

   if(!pContext)
      goto End;

#if (DIB_PARAM_CHECK == 1)
   if(ChannelHdl >= DIB_MAX_NB_CHANNELS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverCreateFilter : numero of  channel over than the max (%d)" CRA, ChannelHdl));
      goto End;
   }

   if(!pContext->ChannelInfo[ChannelHdl].InUse)
      goto End;

   if(!pFilterHdl)
      goto End;

   if(pDataBuf)
   {
      DataMode = pDataBuf->DataMode;
   }
   else
   {
      DataMode = eIP;
   }

   if((DataMode != eIP) && (DataMode != eCLBACK))
      goto End;

   /** Check that DataType is correct and that callback IP is associated with eDAB, DABIP or MPEFEC only */
   switch(DataType)
   {
      case eTDMB:
      case eTS:
      case eDABPACKET:
      case eDAB:
      case eFIG:
      case ePESVIDEO:
      case ePESAUDIO:
      case ePESOTHER:
      case ePCR:
      case eEDAB:
      case eMPEFEC:
      case eMPEIFEC:
      case eCMMBMFS:
      case eCMMBCIT:
      case eCMMBSVC:
      case eCMMBXPE:
         break;

      case eFM:
      case eANALOG:
         break;

      case eSIPSI:
         if(DataMode == eIP)
            goto End;
         break;

      default:
         goto End;
   }
   /** the Type of filter corresponds correctly to the demodulation of this channel ? */
   switch((int)pContext->ChannelInfo[ChannelHdl].Type)
   {
   case eSTANDARD_DVB:
   case eSTANDARD_ISDBT:
   case eSTANDARD_ISDBT_1SEG:
   case (eSTANDARD_DVB | eFAST):
      if((DataType != eTS) && (DataType != eMPEFEC) && (DataType != eMPEIFEC) && (DataType != eSIPSI) && (DataType != ePCR)
         && (DataType != ePESVIDEO) && (DataType != ePESAUDIO) && (DataType != ePESOTHER))
         goto End;
      break;

   case eSTANDARD_DVBSH:
   case (eSTANDARD_DVBSH | eFAST):
      if((DataType != eTS) && (DataType != eMPEFEC) && (DataType != eMPEIFEC) && (DataType != eSIPSI) && (DataType != ePCR)
         && (DataType != ePESVIDEO) && (DataType != ePESAUDIO) && (DataType != ePESOTHER))
         goto End;
      break;

   case eSTANDARD_CMMB:
      break;

   case eSTANDARD_CTTB:
      if((DataType != eTS) && (DataType != eSIPSI))
         goto End;
      break;

   case eSTANDARD_DAB:
      if((DataType != eTS) && (DataType != eFIG) && (DataType != eTDMB) && (DataType != eDAB) && (DataType != eDABPACKET) && (DataType != eEDAB))
      {
         goto End;
      }
      break;

   case eSTANDARD_FM:
      if(DataType != eFM)
         goto End;
      break;

   case eSTANDARD_ANALOG:
      if(DataType != eANALOG)
         goto End;
      break;

   /** this channel does not exist : no filter possible */
   default:
   case eSTANDARD_UNKNOWN:
     goto End;
   }
#endif

   res = DibDriverSpecifCreateFilter(pContext, ChannelHdl, DataType, pDataBuf, pFilterHdl);

   if(res == DIBSTATUS_SUCCESS)
   {
      switch(DataType)
      {
         case eTS:
            /* Clear Rawts monitoring */
            DibZeroMemory(&pContext->FilterInfo[*pFilterHdl].DvbTMonitoring, sizeof(struct DvbTMonit));
            break;

         default:
            break;
      }
   }

End:
   return res;
}

/**
 * Delete a filter.
 * @param pContext pointer to the current context
 * @param FilterHdl Handler of the filter that the user want to delete
 */
DIBSTATUS DibDriverDeleteFilter(struct DibDriverContext *pContext, FILTER_HDL FilterHdl)
{
   DIBSTATUS res = DIBSTATUS_INVALID_PARAMETER;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, "DibDriverDeleteFilter:  %d" CRA, FilterHdl);
   }
#endif

   if(!pContext)
      goto End;

   if(FilterHdl >= DIB_MAX_NB_FILTERS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverDeleteFilter : numero of filter over than the max (%d)" CRA, FilterHdl));
      goto End;
   }

   /** to be deleted, need to exist */
   if(pContext->FilterInfo[FilterHdl].ParentChannel == DIB_UNSET)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverDeleteFilter : filter not allocated yet (%d)" CRA, FilterHdl));
      goto End;
   }

   res = DibDriverSpecifDeleteFilter(pContext, FilterHdl);

End:
   return res;
}

/**
 * Add one or more items to a specific filter. If an error occurs while inserting one of the item, it continues with the
 * other items, but the final status will be DIBSTATUS_ERROR.
 * @param pContext      pointer to the current context
 * @param FilterHdl     associated filter
 * @param nbElements    number of items to add
 * @param pFilterDesc   pointer to the first item to add structure of information, which is an union depending of the Type of filter
 * @param pElemHdl      pointer to the first item's handler, in case of success (chosed by the firmware in case of dragonfly)
 * @return: DIBSTATUS_XXX
 */
DIBSTATUS DibDriverAddToFilter(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint32_t nbElements, union DibFilters *pFilterDesc, ELEM_HDL * pElemHdl)
{
   ELEM_HDL ItemIdx;
   uint32_t Nb = nbElements;
   uint32_t Pid;
   uint32_t Idx = 0;
#if (DIB_PARAM_CHECK == 1)
   uint32_t CheckPid = 1;
#endif

   DIBSTATUS res = DIBSTATUS_INVALID_PARAMETER;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
  {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverAddToFilter:  %d items to filter %d" CRA, nbElements, FilterHdl);
  }
#endif

#if (DIB_PARAM_CHECK == 1)
   if(!pContext || !pFilterDesc || !pElemHdl)
      goto End;

   if(nbElements == 0)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverAddToFilter : need at least one item to add" CRA));
      goto End;
   }

   if(FilterHdl >= DIB_MAX_NB_FILTERS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverAddToFilter : numero of filter over than the max (%d)" CRA, FilterHdl));
      goto End;
   }

   if(pContext->FilterInfo[FilterHdl].ParentChannel == DIB_UNSET)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverAddToFilter : the associated filter is not allocated" CRA));
      goto End;
   }

   DIB_ASSERT(pContext->ChannelInfo[pContext->FilterInfo[FilterHdl].ParentChannel].InUse);

   DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverAddToFilter %d" CRA, FilterHdl));

   /** pFilterDesc checking depending on filter type */
   /* Check Param for all Filter desc */
   for(Idx = 0; Idx < Nb; Idx++)
   {
      union DibFilters *pFilterDescCur = &pFilterDesc[Idx];

      switch(pContext->FilterInfo[FilterHdl].TypeFilter)
      {
         case eMPEFEC:
            if(DibDriverCheckParamMpeFec(pContext, pFilterDescCur, &Pid) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case eMPEIFEC:
            if(DibDriverCheckParamMpeIFec(pContext, pFilterDescCur, &Pid) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case eTS:
            if(DibDriverCheckParamRawTs(pContext, pFilterDescCur, &Pid) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case ePESVIDEO:
         case ePESAUDIO:
         case ePESOTHER:
         case ePCR:
            if(DibDriverCheckParamPes(pContext, pFilterDescCur, &Pid, pContext->FilterInfo[FilterHdl].TypeFilter) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case eCMMBSVC:
         case eCMMBCIT:
         case eCMMBMFS:
         case eCMMBXPE:
            CheckPid = 0; /* No Pid in Cmmb */
            break;

         case eSIPSI:
            if(DibDriverCheckParamSiPsi(pContext, pFilterDescCur, &Pid) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case eTDMB:
            CheckPid = 0;
            if(DibDriverCheckParamTdmb(pContext, pFilterDescCur) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case eDAB:
            CheckPid = 0;
            if(DibDriverCheckParamDabAudio(pContext, pFilterDescCur) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case eDABPACKET:
            CheckPid = 0;
            if(DibDriverCheckParamDabPacket(pContext, pFilterDescCur) != DIBSTATUS_SUCCESS)
               goto End;
            break;

         case eFIG:
            CheckPid = 0;
            break;

         default:
            CheckPid = 0;
            /** default: Other types support are handled by chip specific code */
            break;
      }

      if(CheckPid)
      {
         /** Check that Pids of a same filter are different */
         ItemIdx = pContext->FilterInfo[FilterHdl].FirstItem;
         while(ItemIdx != DIB_UNSET)
         {
            if(pContext->ItemInfo[ItemIdx].Config.Common.Pid == Pid)
               goto End;
            ItemIdx = pContext->ItemInfo[ItemIdx].NextItem;
         }
      }
   }
#endif

   while(Nb > 0)
   {
      if(DibDriverSpecifAddItem(pContext, FilterHdl, pFilterDesc, &pElemHdl[nbElements - Nb]) != DIBSTATUS_SUCCESS)
      {
         res = DIBSTATUS_ERROR;
         goto End;
      }

      pFilterDesc++;
      Nb--;
   }

   res = DIBSTATUS_SUCCESS;

End:
   return res;
}

/**
 * Delete an item from a filter
 * @param pContext pointer to the current context
 * @param ElemHdl Handler of the item that the user want to delete
 */
DIBSTATUS DibDriverRemoveFromFilter(struct DibDriverContext *pContext, ELEM_HDL ElemHdl)
{
/*
   struct DibSipsiFilterCfg *pFirstFilter;
   struct DibSipsiFilterCfg *pNextFilter;
*/
   DIBSTATUS res = DIBSTATUS_INVALID_PARAMETER;

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, CRB "DibDriverRemoveFromFilter:  %d " CRA, ElemHdl);
   }
#endif
   if(!pContext)
      goto End;

   if(ElemHdl >= DIB_MAX_NB_ITEMS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverRemoveFromFilter : item over the max possible %d" CRA, ElemHdl));
      goto End;
   }

   /** to be deleted, need to exist */
   if(pContext->ItemInfo[ElemHdl].ParentFilter == DIB_UNSET)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverRemoveFromFilter : item not allocated actually" CRA));
      goto End;
   }

   /** Prevent driver from internal structure modification, cause we may delete pFilterSipsi structure
   DibDriverTargetDisableIrqProcessing(pContext);

   DIB_DEBUG(PORT_LOG, (CRB "Free Sipsi Filter of item %d" CRA,ElemHdl));
   pFirstFilter = pContext->ItemInfo[ElemHdl].ItemCtx.pSipsiService->pFilterSipsi;
   while(pFirstFilter)
   {
      pNextFilter = pFirstFilter->pNext;
      DIB_ASSERT(pFirstFilter);
      DibMemFree(pFirstFilter, sizeof(struct DibSipsiFilterCfg));
      pFirstFilter = pNextFilter;
   }
   pContext->ItemInfo[ElemHdl].ItemCtx.pSipsiService->pFilterSipsi = NULL;

   DibDriverTargetEnableIrqProcessing(pContext);
*/
   DIB_DEBUG(SOFT_IF_LOG, (CRB " DibDriverRemoveFromFilter %d" CRA, ElemHdl));
   res = DibDriverSpecifRemoveItem(pContext, ElemHdl);

End:
   return res;
}

/**
 * Register a callback for one or more events.
 * @param pContext   pointer to the current context
 * @param pCallback  callback called when events appears
 * @param pCallCtx   pointer to the callback context
 * @param NbEvents   number of events set
 * @param pEvent     pointer to the list of event type
 */
DIBSTATUS DibDriverRegisterEventCallback(struct DibDriverContext * pContext, DibGetEventCallback pCallback, void * pCallCtx, uint8_t NbEvents, enum DibEvent * pEvent)
{
   uint8_t ii = 0, jj = 0;
   enum DibEvent Event;
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   if(NbEvents == 0)
      return DIBSTATUS_INVALID_PARAMETER;

   if(NbEvents > eMAX_NB_EVENTS)
      return DIBSTATUS_INVALID_PARAMETER;

   if(!pCallback || !pContext)
      return DIBSTATUS_INVALID_PARAMETER;

   DibDriverTargetDisableIrqProcessing(pContext);
   for(jj = 0; jj < NbEvents; jj ++)
   {
      Event = pEvent[jj];
      /** Is one callback avalaible ? */
      for(ii = 0; ii < eMAX_NB_EVENTS; ii++)
      {
         if(pContext->EventCallbackDesc[ii].List == 0)
            break;
      }
      if(ii == eMAX_NB_EVENTS)
      {
         Status = DIBSTATUS_RESOURCES;
         break;
      }

      /** Is this event already activated in another callback ? */
      if(pContext->EventTypeCallbackIndex[Event] != DIB_UNSET)
      {
         Status = DIBSTATUS_INVALID_PARAMETER;
         break;
      }

      /** Register */
      pContext->EventTypeCallbackIndex[Event] = ii;
      pContext->EventCallbackDesc[ii].List = pCallback;
      pContext->EventCallbackDesc[ii].pContext = pCallCtx;
   }
   DibDriverTargetEnableIrqProcessing(pContext);

   return Status;
}

/**
 * Unregister an event callback.
 * @param pContext   pointer to the current context
 * @param pCallback  callback to unregister
 */
DIBSTATUS DibDriverUnregisterEventCallback(struct DibDriverContext * pContext, DibGetEventCallback pCallback)
{
   uint8_t ii = 0, jj = 0;
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if(!pContext || !pCallback)
      return DIBSTATUS_INVALID_PARAMETER;

   DibDriverTargetDisableIrqProcessing(pContext);

   /* Remove call back function for all events registered on this callback */
   for(ii = 0; ii < eMAX_NB_EVENTS; ii++)
   {
      if(pContext->EventCallbackDesc[ii].List == pCallback)
      {
         /* We found at least one matching event */
         Status = DIBSTATUS_SUCCESS;

         for(jj = 0; jj < eMAX_NB_EVENTS; jj++)
         {
            if(pContext->EventTypeCallbackIndex[jj] == ii)
            {
               pContext->EventTypeCallbackIndex[jj] = DIB_UNSET;
            }
         }

         pContext->EventCallbackDesc[ii].List = 0;
         pContext->EventCallbackDesc[ii].pContext = 0;
      }
   }

   DibDriverTargetEnableIrqProcessing(pContext);

   return Status;
}

#endif

/*
 * Iternal usage. Paremeter check to ensure coherency between Min and Max demod number requested per channel.
 * IMPOTANT !!! This function must be called while holding the channel lock, as we are going through the channel list.
 */
DIBSTATUS IntDriverCheckMinMaxDemod(struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod)
{
   if((MaxNbDemod != DIB_DEMOD_AUTO) && (MaxNbDemod > pContext->Hd.BoardConfig->NbFrontends))
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "Requested number of demod is exceeding design capacity. " CRA));
      goto End;
   }

   /* MinDemod deprecated */
   if(MinNbDemod != DIB_DEMOD_AUTO)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "Requested number of demod is exceeding design capacity. " CRA));
      goto End;
   }

   return DIBSTATUS_SUCCESS;

End:
   return DIBSTATUS_INVALID_PARAMETER;
}

/**
 * DibDriverSendMessageApi
 */
DIBSTATUS DibDriverSendMessageApi(struct DibDriverContext *pContext, uint16_t MsgType, uint16_t MsgIndex, uint8_t *pSendBuf, uint32_t SendLen, uint8_t *pRecvBuf, uint32_t * pRecvLen)
{
   DIBSTATUS Status;

   Status = DibDriverSendMsgApi(pContext, MsgType, MsgIndex, pSendBuf, SendLen, pRecvBuf, pRecvLen);

   return Status;
}

/**
 * DibDriverRegisterMessageCallback
 */
DIBSTATUS DibDriverRegisterMessageCallback(struct DibDriverContext * pContext, uint16_t MsgType, uint8_t * pMsgBuf, uint32_t MsgBufLen, DibMessageCallback pCallback, void * pCallbackCtx)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   if(!pContext)
      return DIBSTATUS_INVALID_PARAMETER;

   if(pCallback)
   {
      DibDriverTargetDisableIrqProcessing(pContext);

      /** Register */
      pContext->ApiMsgCbCtx.BufferSize = MsgBufLen;
      pContext->ApiMsgCbCtx.pBuffer = pMsgBuf;
      pContext->ApiMsgCbCtx.pCallback = pCallback;
      pContext->ApiMsgCbCtx.pCallbackCtx = pCallbackCtx;

      DibDriverTargetEnableIrqProcessing(pContext);

      Status = DibDriverEnableMsgApiUp(pContext, MsgType, eDIB_TRUE);
      if(Status == DIBSTATUS_SUCCESS)
         return Status;
   }

   DibDriverTargetDisableIrqProcessing(pContext);

   /** Unregister */
   pContext->ApiMsgCbCtx.BufferSize = 0;
   pContext->ApiMsgCbCtx.pBuffer = NULL;
   pContext->ApiMsgCbCtx.pCallback = NULL;
   pContext->ApiMsgCbCtx.pCallbackCtx = NULL;

   DibDriverTargetEnableIrqProcessing(pContext);

   Status = DibDriverEnableMsgApiUp(pContext, MsgType, eDIB_TRUE);
   return Status;
}


DIBSTATUS DibDriverError()
{
   return DIBSTATUS_ERROR;
}

DIBSTATUS DibDriverSuccess()
{
   return DIBSTATUS_SUCCESS;
}
