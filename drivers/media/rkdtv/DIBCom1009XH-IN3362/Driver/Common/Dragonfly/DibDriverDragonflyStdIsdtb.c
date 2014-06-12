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
* @file "DibDriverDragonflyStdDvb.c"
* @brief Driver Interface.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_ISDBT_STD == 1)

void DibDriverDragonflyProcessMsgMonitDemodIsdbt(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * pMsg, struct DibTotalDemodMonit * pTotalMonitInfo)
{
   struct DibIsdbtDemodMonit * pIsdbtDemodMonit;
   struct DibDvbDemodMonit * pDemodMonit;
   struct SignalMonitoring * pMon = &pMsg->Mon;
   uint32_t i;

   pIsdbtDemodMonit = &pTotalMonitInfo->DemodMonit[pMsg->DemodId].Isdbt;
   pDemodMonit = &pIsdbtDemodMonit->Dvb;

   /* The followings extraction are the same as dvb */
   pDemodMonit->FecBerRs          = 0;
   pDemodMonit->FecMpfailCnt      = 0;

   pDemodMonit->Timeout           = (pMon->MonitStatus == DIB_MONIT_TIMEOUT) ? 1 : 0;
   pDemodMonit->AgcPower          = pMon->AdcPower;
   pDemodMonit->AgcGlobal         = pMon->AgcGlobal;
   pDemodMonit->WgnFp             = pMon->EqualNoise;
   pDemodMonit->SigFp             = pMon->EqualSignal;
   pDemodMonit->SysNoiseFp        = pMon->SystemNoise;
   pDemodMonit->CombWgnFp         = pMon->CombNoise;
   pDemodMonit->CombSigFp         = pMon->CombSignal;
   pDemodMonit->TimingOffset      = pMon->TimingOffset;
   pDemodMonit->FrequencyOffset   = pMon->FrequencyOffset;
   pDemodMonit->VAgcRf            = pMon->AgcRf;
   pDemodMonit->VAgcBB            = pMon->AgcBb;
   pDemodMonit->AgcWbd            = pMon->AgcWbd;
   pDemodMonit->AgcSplitOffset    = pMon->AgcSplitOffset;
   pDemodMonit->Locks             = pMon->Locks;
   pDemodMonit->SignalQuality     = 0;
   pDemodMonit->SignalQualityFast = 0;
   pDemodMonit->LnaStatus         = 0;
   pDemodMonit->TransmitterId     = pMon->TransmitterId;

   pDemodMonit->RfTotalGain       = (pDemodMonit->AgcGlobal * pContext->Hd.BoardConfig->DefAgcDynamicsDb) / (uint32_t) (1 << 16);

   /* specific isdbt part */
   for(i = 0; i<3; i++)
   {
      pIsdbtDemodMonit->Layer[i].FecBerRs          = pMon->ChanDec.Isdbt[i].Ber;
      pIsdbtDemodMonit->Layer[i].FecMpfailCnt      = pMon->ChanDec.Isdbt[i].Per;
      pIsdbtDemodMonit->Layer[i].SignalQuality     = pMon->ChanDec.Isdbt[i].Quality;
      pIsdbtDemodMonit->Layer[i].SignalQualityFast = pMon->ChanDec.Isdbt[i].FastQuality;
   }
}

uint8_t DibDriverDragonflyGetTimeoutIsdbt(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId)
{
   return pTotalMonitInfo->DemodMonit[TmpDemId].Isdbt.Dvb.Timeout;
}

void DibDriverDragonflyCheckMonitIsdbt(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl)
{
   uint8_t NbDemod;
   uint8_t i;
   uint8_t ValidChanDecMonit = 0;
   struct DibIsdbtDemodMonit * pIsdbtDemodMonit;
   struct DibDvbDemodMonit * pDemodMonit;

   NbDemod = pContext->ChannelInfo[ChannelHdl].pStream->NbConnFrontends;


   pDemodMonit = &pIsdbtDemodMonit->Dvb;

   for (i=0;i<NbDemod;i++)
   {
      pIsdbtDemodMonit = &pTotalMonitInf->DemodMonit[i].Isdbt;
      if (DIB_GET_ISDBT_LOCK_COFF(pIsdbtDemodMonit->Dvb.Locks) == 1)
        ValidChanDecMonit = 1;
   }

   pIsdbtDemodMonit = &pTotalMonitInf->DemodMonit[0].Isdbt;
   if (ValidChanDecMonit == 0)
     pIsdbtDemodMonit->Dvb.Locks &= ~0x0EE0;

   /* BER is not valid if no FEC MPEG lock */
   if (DIB_GET_ISDBT_LOCK_MPEG0(pIsdbtDemodMonit->Dvb.Locks) == 0) {
       pIsdbtDemodMonit->Layer[0].FecBerRs = 100000000;
       pIsdbtDemodMonit->Layer[0].FecMpfailCnt = 1000;
   }
   if (DIB_GET_ISDBT_LOCK_MPEG1(pIsdbtDemodMonit->Dvb.Locks) == 0) {
       pIsdbtDemodMonit->Layer[1].FecBerRs = 100000000;
       pIsdbtDemodMonit->Layer[1].FecMpfailCnt = 1000;
   }
   if (DIB_GET_ISDBT_LOCK_MPEG2(pIsdbtDemodMonit->Dvb.Locks) == 0) {
       pIsdbtDemodMonit->Layer[2].FecBerRs = 100000000;
       pIsdbtDemodMonit->Layer[2].FecMpfailCnt = 1000;
   }

}


DIBSTATUS DibDriverDragonflySetMsgTuneIsdbt(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg)
{
   uint32_t i;

   pMsg->Desc.Param.Isdbt.Fft                      = pTc->Std.Isdbt.Nfft;
   pMsg->Desc.Param.Isdbt.Guard                    = pTc->Std.Isdbt.Guard;
   pMsg->Desc.Param.Isdbt.SbMode                   = 0;
   pMsg->Desc.Param.Isdbt.PartialReception         = pTc->Std.Isdbt.PartialReception;
   pMsg->Desc.Param.Isdbt.InvSpect                 = pTc->InvSpec;
   pMsg->Desc.Param.Isdbt.SbConTotSeg              = pTc->Std.Isdbt.SbConnTotalSeg;
   pMsg->Desc.Param.Isdbt.SbWantedSeg              = pTc->Std.Isdbt.SbWantedSeg;
   pMsg->Desc.Param.Isdbt.SbSubChannel             = pTc->Std.Isdbt.SbSubchannel;

   for(i = 0; i < 3; i++)
   {
      pMsg->Desc.Param.Isdbt.Layer[i].Constellation = pTc->Std.Isdbt.layer[i].Constellation;
      pMsg->Desc.Param.Isdbt.Layer[i].CodeRate      = pTc->Std.Isdbt.layer[i].CodeRate;
      pMsg->Desc.Param.Isdbt.Layer[i].NbSegments    = pTc->Std.Isdbt.layer[i].NbSegments;
      pMsg->Desc.Param.Isdbt.Layer[i].TimeIntlv     = pTc->Std.Isdbt.layer[i].TimeIntlv;
   }

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflySetMsgTuneIsdbtOneSeg(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg)
{
   pMsg->Desc.Param.Isdbt.Fft                      = pTc->Std.Isdbt.Nfft;
   pMsg->Desc.Param.Isdbt.Guard                    = pTc->Std.Isdbt.Guard;
   pMsg->Desc.Param.Isdbt.SbMode                   = 1;
   pMsg->Desc.Param.Isdbt.PartialReception         = pTc->Std.Isdbt.PartialReception;
   pMsg->Desc.Param.Isdbt.InvSpect                 = pTc->InvSpec;
   pMsg->Desc.Param.Isdbt.SbConTotSeg              = pTc->Std.Isdbt.SbConnTotalSeg;
   pMsg->Desc.Param.Isdbt.SbWantedSeg              = pTc->Std.Isdbt.SbWantedSeg;
   pMsg->Desc.Param.Isdbt.SbSubChannel             = pTc->Std.Isdbt.SbSubchannel;

   pMsg->Desc.Param.Isdbt.Layer[0].Constellation   = pTc->Std.Isdbt.layer[0].Constellation;
   pMsg->Desc.Param.Isdbt.Layer[0].CodeRate        = pTc->Std.Isdbt.layer[0].CodeRate;
   pMsg->Desc.Param.Isdbt.Layer[0].NbSegments      = pTc->Std.Isdbt.layer[0].NbSegments;
   pMsg->Desc.Param.Isdbt.Layer[0].TimeIntlv       = pTc->Std.Isdbt.layer[0].TimeIntlv;

   /* Firmware considers one seg is 'like' isdbt */
   pMsg->Desc.Standard                             = eSTANDARD_ISDBT;

   if(pTc->Std.Isdbt.PartialReception == 1) { /* 3 SEG  = 1 SEG PR=1 on layer A + 2segs on layer B*/
       pMsg->Desc.Param.Isdbt.Layer[1].Constellation   = pTc->Std.Isdbt.layer[1].Constellation;
       pMsg->Desc.Param.Isdbt.Layer[1].CodeRate        = pTc->Std.Isdbt.layer[1].CodeRate;
       pMsg->Desc.Param.Isdbt.Layer[1].NbSegments      = pTc->Std.Isdbt.layer[1].NbSegments;
       pMsg->Desc.Param.Isdbt.Layer[1].TimeIntlv       = pTc->Std.Isdbt.layer[1].TimeIntlv;
       DibZeroMemory(&pMsg->Desc.Param.Isdbt.Layer[2], sizeof(struct ISDBTLayerParameter));
   } else { /* 1 SEG  = 1 SEG PR=0 layer A */
       DibZeroMemory(&pMsg->Desc.Param.Isdbt.Layer[1], sizeof(struct ISDBTLayerParameter));
       DibZeroMemory(&pMsg->Desc.Param.Isdbt.Layer[2], sizeof(struct ISDBTLayerParameter));
   }

   return DIBSTATUS_SUCCESS;
}

/**
 * IntDriverDragonflyParseIsdbtTmmcRegisters
 */


DIBSTATUS DibDriverDragonflyGetChannelInfoIsdbt(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * pMsg, uint16_t ChHdl)
{
   struct DibTuneChan    * pTCh;
   struct DibIsdbtTuneChan * pTcIsdbt;
   struct ISDBTChannelParameters * pParam = &pMsg->Desc.Param.Isdbt;
   uint32_t i;

   /* do not store scanning information at the same place if TuneMonit */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
      pTCh = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor);
   else
      pTCh = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor);

   pTcIsdbt = &pTCh->Std.Isdbt;

   pTCh->InvSpec                        = pParam->InvSpect;
   pTcIsdbt->Nfft                       = pParam->Fft;
   pTcIsdbt->Guard                      = pParam->Guard;
   pTcIsdbt->SbMode                     = pParam->SbMode;
   pTcIsdbt->PartialReception           = pParam->PartialReception;
   pTcIsdbt->SbConnTotalSeg             = pParam->SbConTotSeg;
   pTcIsdbt->SbWantedSeg                = pParam->SbWantedSeg;
   pTcIsdbt->SbSubchannel               = pParam->SbSubChannel;

   for(i = 0; i < 3; i++)
   {
       pTcIsdbt->layer[i].Constellation = pParam->Layer[i].Constellation;
       pTcIsdbt->layer[i].CodeRate      = pParam->Layer[i].CodeRate;
       pTcIsdbt->layer[i].NbSegments    = pParam->Layer[i].NbSegments;
       pTcIsdbt->layer[i].TimeIntlv     = pParam->Layer[i].TimeIntlv;
   }

   /* IntDriverDragonflyParseIsdbtTmmcRegisters(pContext, pMsg->TmmcRegisters, pTcIsdbt); */
   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyGetChannelInfoIsdbtOneSeg(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * pMsg, uint16_t ChHdl)
{
   /* Force some values as wanted by one seg standard */
   struct DibIsdbtTuneChan * pTcIsdbt;

   DIBSTATUS Ret = DibDriverDragonflyGetChannelInfoIsdbt(pContext, pMsg, ChHdl);

   /* do not store scanning information at the same place if TuneMonit */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
      pTcIsdbt = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor.Std.Isdbt);
   else
      pTcIsdbt = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor.Std.Isdbt);

   pTcIsdbt->SbMode                     =  1;
   /* pTcIsdbt->PartialReception           =  0; */
   /* pTcIsdbt->layer[0].NbSegments        =  1; */
/*
   for(i = 1; i < 3; i++)
   {
       pTcIsdbt->layer[i].Constellation = 0;
       pTcIsdbt->layer[i].CodeRate      = 0;
       pTcIsdbt->layer[i].NbSegments    = 0;
       pTcIsdbt->layer[i].TimeIntlv     = 0;
   }
*/
   return Ret;
}

#endif

#endif /* USE_DRAGONFLY */
