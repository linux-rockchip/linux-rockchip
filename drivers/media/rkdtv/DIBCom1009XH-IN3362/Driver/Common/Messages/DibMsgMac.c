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





#include "DibMsgMac.h"


/*----------------------------------------------------------
 * messages
 * every msg should start by MsgXXX, have a MsgHeader, and define IN_MSG_ or OUT_MSG_ values
 *---------------------------------------------------------*/

/*------------------------------MsgDump-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_DUMP
 */
/*------------------------------MsgGetStream-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_GET_STREAM.
 */
void MsgGetStreamPack(struct MsgGetStream *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->OutputOptions);
   SerialBufWriteField(ctx, 32, s->Standard);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 2, s->EnableTimeSlicing);
}

/*------------------------------MsgDeleteStream-----------------------------------*/
/**
 Msg to delete a stream
 OUT_MSG_DEL_STREAM.
 */
void MsgDeleteStreamPack(struct MsgDeleteStream *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->StreamId);
}

/*------------------------------MsgAddFrontend-------------------------------------*/
/**
 Msg to add a frontend to a stream
 OUT_MSG_ADD_FRONTEND.
 */
void MsgAddFrontendPack(struct MsgAddFrontend *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->FrontendId);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 16, s->OutputFrontendId);
}

/*------------------------------MsgRemoveFrontend-----------------------------------*/
/**
 Msg to remove a frontend to a stream
 OUT_MSG_DEL_FRONTEND.
 */
void MsgRemoveFrontendPack(struct MsgRemoveFrontend *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->FrontendId);
   SerialBufWriteField(ctx, 16, s->StreamId);
}

/*--------------------------- MsgCreateChannel --------------------------- */

void MsgCreateChannelPack(struct MsgCreateChannel *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 8, s->ChannelId);
   SerialBufWriteField(ctx, 8, s->TuneMonit);
   SerialBufWriteField(ctx, 16, s->StreamId);
   ChannelDescPack(&s->Desc, ctx);

}

/*----------------------------- MsgTuneInd ------------------------------ */

void MsgTuneIndicationPack(struct MsgTuneIndication *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 8, s->ChannelId);
   SerialBufWriteField(ctx, 4, s->TuneOption);
   SerialBufWriteField(ctx, 1, s->TuneMonit);
}

/*--------------------------- MsgAckCreateChannel --------------------------- */

void MsgAckCreateChannelPack(struct MsgAckCreateChannel *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 8, s->ChannelId);
   SerialBufWriteField(ctx, 2, s->AckOk);
}
/*--------------------------- MsgCmmbMappingExchg -------------------------- */

void MsgCmmbMapMasterPack(struct MsgCmmbMapMaster *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->Status);
   SerialBufWriteField(ctx, 8, s->Initiator);
   SerialBufWriteField(ctx, 8, s->ChannelId);
   CMMBmappingPack(&s->MappingExch, ctx);
}


/*------------------------- MsgUpdateChannelInd --------------------------- */

void MsgUpdateChannelIndicationPack(struct MsgUpdateChannelIndication *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 8, s->ChannelId);
   ChannelDescPack(&s->Desc, ctx);

}

/*--------------------------- MsgTuneStatusConfirm --------------------------- */
void MsgTuneStatusConfirmPack(struct MsgTuneStatusConfirm *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 16, s->FrontendId);
   SerialBufWriteField(ctx, 8, s->ChannelId);
   SerialBufWriteField(ctx, 8, s->Status);
   SerialBufWriteField(ctx, 8, s->State);
}

/*--------------------------- MsgSleepIndication --------------------------- */

void MsgSleepIndicationPack(struct MsgSleepIndication *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 8, s->ChannelId);
}

/*-------------------------MsgDeleteChannel-----------------------------------*/
/**
 Msg request the suppression of a channel
 OUT_MSG_REQ_DEL_CH.
 */


/*-------------------------MsgCreateFilter-----------------------------------*/
/**
 Msg request the creation of a new filter
 OUT_MSG_REQ_CREATE_FILT.
 */

void MsgCreateFilterPack(struct MsgCreateFilter *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 8, s->ChannelId);
   SerialBufWriteField(ctx, 8, s->FilterId);
   SerialBufWriteField(ctx, 8, s->Type);
}

/*-------------------------MsgDeleteFilter-----------------------------------*/
/**
 Msg request the suppression of a filter
 OUT_MSG_REQ_DEL_FILT.
 */


/*-------------------------MsgCreateItem -----------------------------------*/
void MsgCreateItemPack(struct MsgCreateItem *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 8, s->ItemId);
   SerialBufWriteField(ctx, 8, s->FilterType);
   SerialBufWriteField(ctx, 8, s->FilterId);
   ItemParametersPack(&s->Param, s->FilterType, ctx);
}

/*-------------------------MsgDeleteItem----------------------------------------*/
/**
 Msg request the suppression of a filter's item
 OUT_MSG_REQ_REM_TO_FILT
 */


/*--------------------------MsgAcknowledgeApi----------------------------------*/
/**
 Acknowledge API request.
 MSG_ACK_API identifier
 IN_MSG_ACK_DEL_CH, IN_MSG_ACK_CREATE_FILT, IN_MSG_ACK_DEL_FILT,
 IN_MSG_ACK_ADD_TO_FILT, IN_MSG_ACK_FREE_ITEM, IN_MSG_ACK_CAS_EN,
 IN_MSG_ACK_API_UP_EN
 */


/*------------------------------- MsgSetHbm ------------------------------ */

void MsgSetHbmPack(struct MsgSetHbm *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Mode);
}

/* ----------------------------- MsgRawTSMonitor-------------------------- */
/**
 */
void MsgRawTSMonitorPack(struct MsgRawTSMonitor *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->TotalPackets);
   SerialBufWriteField(ctx, 32, s->ErrorPackets);
   SerialBufWriteField(ctx, 32, s->CCPackets);
   SerialBufWriteField(ctx, 32, s->ItemHdl);
}

/* ----------------------------- MsgMpeMonitor ------------------------------ */
/**
  Firmware send MPE monitoring information after frame reception is complete
  Be careful : do not change the order of the following information
*/
void MsgMpeMonitorPack(struct MsgMpeMonitor *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->NbRxBurst);
   SerialBufWriteField(ctx, 32, s->NbErrBeforeFec);
   SerialBufWriteField(ctx, 32, s->NbErrAfterFec);
   SerialBufWriteField(ctx, 32, s->NbErrTableBeforeFec);
   SerialBufWriteField(ctx, 32, s->NbErrTableAfterFec);
   SerialBufWriteField(ctx, 32, s->NbTableMissed);
   SerialBufWriteField(ctx, 32, s->BurstStartDate);
   SerialBufWriteField(ctx, 32, s->BurstEndDate);
   SerialBufWriteField(ctx, 32, s->DeltatMin);
   SerialBufWriteField(ctx, 32, s->DeltatMax);
   SerialBufWriteField(ctx, 32, s->FrameDuration);
   SerialBufWriteField(ctx, 32, s->PowerUpDuration);
   SerialBufWriteField(ctx, 8, s->Padding);
   SerialBufWriteField(ctx, 8, s->Puncturing);
   SerialBufWriteField(ctx, 8, s->ItemHdl);
   SerialBufWriteField(ctx, 8, s->FilterId);
   SerialBufWriteField(ctx, 32, s->FrameStatus);
}

/*-------------------------------MsgMpeIfecMonitor------------------------------*/
/**
 Firmware send IFEC monitoring information (DVBSH standard)
 */
void MsgMpeIfecMonitorPack(struct MsgMpeIfecMonitor *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   /** ADT (ie. table) monitoring */
   SerialBufWriteField(ctx, 32, s->NbADTTables);
   SerialBufWriteField(ctx, 32, s->NbADTTablesCorrected);
   SerialBufWriteField(ctx, 32, s->NbADTTablesFailed);
   SerialBufWriteField(ctx, 32, s->NbADTTablesGood);

   /** ADST (ie. burst) monitoring */
   SerialBufWriteField(ctx, 32, s->NbADSTTables);
   SerialBufWriteField(ctx, 32, s->NbADSTTablesLost);
   SerialBufWriteField(ctx, 32, s->NbADSTTablesCorrected);
   SerialBufWriteField(ctx, 32, s->NbADSTTablesFailed);
   SerialBufWriteField(ctx, 32, s->NbADSTTablesGood);

   SerialBufWriteField(ctx, 32, s->BurstStartDate);
   SerialBufWriteField(ctx, 32, s->BurstEndDate);
   SerialBufWriteField(ctx, 32, s->DeltatMin);
   SerialBufWriteField(ctx, 32, s->DeltatMax);
   SerialBufWriteField(ctx, 32, s->FrameDuration);
   SerialBufWriteField(ctx, 32, s->PowerUpDuration);

   SerialBufWriteField(ctx, 16, s->TableIndex);
   SerialBufWriteField(ctx, 8, s->BurstIndex);
   SerialBufWriteField(ctx, 8, s->ItemHdl);
   SerialBufWriteField(ctx, 32, s->FrameStatus);
}

/*--------------------------------MsgFicMonitor----------------------------------*/
 /**
  Fic avalaible packets is tested with the 16bits CRC
  */
void MsgFicMonitorPack(struct MsgFicMonitor *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->ItemHdl);
   SerialBufWriteField(ctx, 32, s->ErrorPackets);
   SerialBufWriteField(ctx, 32, s->AvalaiblePackets);
}

/*-------------------------------MsgMscPacketMonitor------------------------------*/
/**
 Firmware send Msc Packet monitoring information
 */
void MsgMscPacketMonitorPack(struct MsgMscPacketMonitor *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->ItemHdl);
   SerialBufWriteField(ctx, 32, s->MscPacketsAvail);
   SerialBufWriteField(ctx, 32, s->MscPacketsError);
   SerialBufWriteField(ctx, 32, s->DataGroupAvail);
   SerialBufWriteField(ctx, 32, s->DataGroupError);
}

/*-------------------------------MsgMscPlusMonitor------------------------------*/
/**
 Firmware send DAB+ monitoring information
 */

/* ---------------------------- MsgGetSignalMonit --------------------------------*/

void MsgGetSignalMonitPack(struct MsgGetSignalMonit *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 8, s->ChannelId);
   SerialBufWriteField(ctx, 8, s->DemodId);
}

/* ---------------------------- MsgAckGetSignalMonit -----------------------------*/

void MsgAckGetSignalMonitPack(struct MsgAckGetSignalMonit *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 8, s->ChannelId);
   SerialBufWriteField(ctx, 8, s->DemodId);
   SignalMonitoringPack(&s->Mon, ctx);
}


/* --------------------------------- MsgError -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */


void MsgErrorPack(struct MsgError *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 16, s->ErrorCode);
   SerialBufWriteField(ctx, 16, s->ErrorFlag);
   for(i = 0; i < 4;i++ )
   {
      SerialBufWriteField(ctx, 32, s->Param[i]);
   }
}


/* --------------------------------- MsgVersion -----------------------------------*/
/*
 * Description : Host request the version of the firmware
 */
void MsgVersionPack(struct MsgVersion *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 16, s->SoftVer);
   SerialBufWriteField(ctx, 16, s->ChipVer);
   SerialBufWriteField(ctx, 32, s->HalVer);
   for(i = 0; i < 8;i++ )
   {
      SerialBufWriteField(ctx, 8, s->HalVerExt[i]);
   }
}

/* ----------------------------------- MsgEvent -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */
void MsgEventPack(struct MsgEvent *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 8, s->ItemId);
   SerialBufWriteField(ctx, 8, s->EventId);
}

/* -------------------------------- MsgCmmbRsmMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB LDPC and RS monitoring information at end of RS matrix.
 */
void MsgCmmbRsmMonitorPack(struct MsgCmmbRsmMonitor *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   /* General Info */
   SerialBufWriteField(ctx, 16, s->NbFecErr);
   SerialBufWriteField(ctx, 8, s->ItemId);
   SerialBufWriteField(ctx, 8, s->Status);

   /* LDPC block error rate. */
   SerialBufWriteField(ctx, 32, s->TotalLdpcBlocks);
   SerialBufWriteField(ctx, 32, s->ErrorLdpcBlocks);

   /* RS matrix error rate. */
   SerialBufWriteField(ctx, 32, s->TotalRsMatrices);
   SerialBufWriteField(ctx, 32, s->ErrorRsMatricesBeforeRs);
   SerialBufWriteField(ctx, 32, s->ErrorRsMatricesAfterRs);
}


/* ----------------------------- MsgCmmbMfMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB MF information at end of each MF.
 */
void MsgCmmbMfMonitorPack(struct MsgCmmbMfMonitor *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   /* Multiplex frame info. */
   SerialBufWriteField(ctx, 4, s->Status);
   SerialBufWriteField(ctx, 6, s->ItemId);

   /* Multiplex Frames. */
   SerialBufWriteField(ctx, 32, s->TotalMf);
   SerialBufWriteField(ctx, 32, s->ErrorMfBeforeRs);
   SerialBufWriteField(ctx, 32, s->ErrorMfAfterRs);
}


/* ----------------------------- MsgDebugHelp -----------------------------*/
/*
 * Set a Debug function of the embedded firmware.
 * Func:
 *    - 0: memory dumping
 *    - 1: ldpc discarder
 *    - 11: Enable Real Time Discarder. Value is the SNR
 *    - 12: dvb-sh discarded
 */
void MsgDebugHelpPack(struct MsgDebugHelp *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 16, s->Funct);
   SerialBufWriteField(ctx, 16, s->Value);
}


/* ----------------------------- MsgPrint -----------------------------*/
/*
 * Message used to send a string to the host
 */
void MsgPrintPack(struct MsgPrint *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);

   for(i = 0; i < 200;i++ )
   {
      SerialBufWriteField(ctx, 8, s->Data[i]);
   }
}

/* ----------------------------- MsgEnableTimeSlice -----------------------------*/
/*
 * enable/disable time-slicing for a dedicated stream
 */

/* --------------------------- MsgAbortTuneMonit ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */

/* --------------------------- MsgSetPidMode ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */
void MsgSetPidModePack(struct MsgSetPidMode *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->ItemId);
   SerialBufWriteField(ctx, 1, s->Prefetch);
}


/* --------------------------- MsgCasEnable ---------------------------*/
/*
 * Enable/disable CAS system
 */
void MsgCasEnablePack(struct MsgCasEnable *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 1, s->Enable);
}


/* ------------------------ MsgAHBAccess ----------------------------*/
/*
 * Internal access done by firmware but requested by the host
 */
void MsgAHBAccessPack(struct MsgAHBAccess *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Addr);
   SerialBufWriteField(ctx, 32, s->Value);
   SerialBufWriteField(ctx, 1, s->Mode);
}

/* ------------------------ MsgHostReflexInit ----------------------------*/
/*
 * Address in emb of reflex context needed for emulation
 */

/* ------------------------ MsgHostReflexStep ----------------------------*/
/*
 * Command to advance one step in emulation
 */

/* ------------------------ MsgHostReflexSetTsConfig ----------------------------*/
/*
 * Each time time slot configuration changes, warn the host
 */
void MsgHostReflexSetTsConfigPack(struct MsgHostReflexSetTsConfig *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Configuration);
   SerialBufWriteField(ctx, 32, s->TimeSlotMap_0_31);
   for(i = 0; i < 10;i++ )
   {
      SerialBufWriteField(ctx, 32, s->ParamIdFourTimeSlots[i]);
   }
}

/* ------------------------ MsgHostReflexSetSvcParam ----------------------------*/
/*
 * Each time service parameters changes, warn the host
 */

/* --------------------------- MsgGpioConfig ------------------------------*/

void MsgGpioConfigPack(struct MsgGpioConfig *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->NbFuncs);
   SerialBufWriteField(ctx, 16, s->FeId);
   for(i = 0; i < 6;i++ )
   {
      GpioFunctionPack(&s->Func[i], ctx);
   }
}

/* --------------------------- MsgLayoutConfig ------------------------------*/
void MsgLayoutConfigPack(struct MsgLayoutConfig *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Cfg);
   SerialBufWriteField(ctx, 32, s->LnaConfig);
   for(i = 0; i < 2;i++ )
   {
      SerialBufWriteField(ctx, 32, s->GpioDefaultDir[i]);
   }
   for(i = 0; i < 2;i++ )
   {
      SerialBufWriteField(ctx, 32, s->GpioDefaultVal[i]);
   }
   SerialBufWriteField(ctx, 16, s->CurTrig);
   SerialBufWriteField(ctx, 16, s->MaxTrig);
   SerialBufWriteField(ctx, 32, s->FeId);
   SerialBufWriteField(ctx, 32, s->GpioNum);
   SerialBufWriteField(ctx, 32, s->GpioDir);
   SerialBufWriteField(ctx, 32, s->GpioVal);
   SerialBufWriteField(ctx, 32, s->FeIdMask);
   SerialBufWriteField(ctx, 32, s->TuneStateMask);
   for(i = 0; i < 2;i++ )
   {
      SerialBufWriteField(ctx, 32, s->FreqRangeMhz[i]);
   }
   SerialBufWriteField(ctx, 32, s->ThresholdVal);
   SerialBufWriteField(ctx, 1, s->Condition);
   SerialBufWriteField(ctx, 1, s->OutsideBand);
   SerialBufWriteField(ctx, 1, s->ThresholdDir);
}


/* --------------------------- MsgAckLayoutConfig ------------------------------*/


/* --------------------------- MsgAckLayoutReset ------------------------------*/


/* ----------------------------- MsgGetLayoutInfo -----------------------------*/
/*
 * Request information concerning the layout
 */

/* ----------------------------- MsgLayoutInfo --------------------------------*/
/*
 * Reply information concerning the layout
 */

/* ----------------------------- MsgGetFrontendInfo ----------------------------*/
/*
 * Request information concerning a specific frontend
 */
void MsgGetFrontendInfoPack(struct MsgGetFrontendInfo *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->FeNum);
}

/* ----------------------------- MsgFrontendInfo -------------------------------*/
/*
 * Request information concerning a specific frontend
 */
void MsgFrontendInfoPack(struct MsgFrontendInfo *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->FeNum);
   FrontendInfoPack(&s->FeInfo, ctx);
}

/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Request information concerning a specific stream
 */

/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Reply information concerning a stream
 */

void MsgStreamInfoPack(struct MsgStreamInfo *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->StreamNum);
   StreamInfoPack(&s->StreamInfo, ctx);
}


/* ----------------------------- MsgDownloadIndication ------------------------------*/
/*
 * Request for downloading a file
 */
void MsgDownloadIndicationPack(struct MsgDownloadIndication *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   for(i = 0; i < 44;i++ )
   {
      SerialBufWriteField(ctx, 8, s->FirmwareName[i]);
   }
   SerialBufWriteField(ctx, 32, s->AddrMsb);
   SerialBufWriteField(ctx, 32, s->AddrLsb);
   SerialBufWriteField(ctx, 32, s->LengthAckMsb);
   SerialBufWriteField(ctx, 32, s->LengthAckLsb);
   SerialBufWriteField(ctx, 32, s->FileOffset);
   SerialBufWriteField(ctx, 28, s->MaxLength);
   SerialBufWriteField(ctx, 4, s->Options);
}


/* ----------------------------- MsgDownloadConfirm ------------------------------*/

/*
 * confirmation of file download
 */
void MsgDownloadConfirmPack(struct MsgDownloadConfirm *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->LengthAckMsb);
   SerialBufWriteField(ctx, 32, s->LengthAckLsb);
   SerialBufWriteField(ctx, 28, s->Length);
   SerialBufWriteField(ctx, 1, s->Complete);
   SerialBufWriteField(ctx, 1, s->WithCrc);
}

/* ----------------------------- MsgApiRequest ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer to the firmware and reecive one in return.
 * Each MsgApiRequest must be acknowledged before sending a new one.
 */


void MsgApiRequestPack(struct MsgApiRequest *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->ReqId);
   SerialBufWriteField(ctx, 16, s->ReqType);
   SerialBufWriteField(ctx, 32, s->TxLen);
   SerialBufWriteField(ctx, 32, s->TxAddr);
}

/* ----------------------------- MsgApiAnswer ------------------------------*/
/*
 * This is a reply to the MsgApiRequest, with the possibility to allow buffer retreival fron embedded buffer to
 * host buffer.
 */

/* ----------------------------- MsgApiUpEnable ------------------------------*/
/*
 * This message is used to enable/disable the fact that the firmware can send msg api information to the user
 * without any request from it. In that case, the user is notified from the sdk through a message callback mechanism, with the
 * possibility to get also data buffers.
 */

/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer from the firwmare to the host is permitted (see MsgApiUpEnable)
 * Each MsgApiUp must be acknoledged by a MsgApiUpAck whose role is to free buffer allocated at RxAddr.
 */

/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * This message is used to free the memory allocated by firmware when sending the MsgApiUp message
 */

/* ------------------------------ MsgOctoInit --------------------------*/


void MsgOctoInitPack(struct MsgOctoInit *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->GlobalHeader, ctx);
   MsgHeaderPack(&s->Header, ctx);
}

/* ------------------------- MsgOctoUtilsCheckSum ----------------------*/
void MsgOctoUtilsCheckSumPack(struct MsgOctoUtilsCheckSum *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   MsgHeaderPack(&s->InternalHead, ctx);
   SerialBufWriteField(ctx, 32, s->Hello);
}


/* --------------------------- MsgOctoRwOctopus --------------------------*/
void MsgOctoRwOctopusPack(struct MsgOctoRwOctopus *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->GlobalHeader, ctx);
   MsgHeaderPack(&s->Header, ctx);
   SerialBufWriteField(ctx, 32, s->Config);
   SerialBufWriteField(ctx, 32, s->Size);
   SerialBufWriteField(ctx, 32, s->Last);
}

/* --------------------------- MsgHostOctoInit --------------------------*/
/*
 * Indicates the host where to find the data buffer inside chipset memory
 */

/* --------------------------- MsgHostOctoRwOctopus ----------------------*/

/* --------------------------- MsgWave -----------------------------------*/

/* --------------------------- MsgIntDebug -------------------------------*/

/* --------------------------- MsgProfile --------------------------------*/


/* --------------------------- debugCmd ----------------------------------*/

/* Mode */
void MsgSlaveDataPack(struct MsgSlaveData *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->ByteMode);
   SerialBufWriteField(ctx, 1, s->Incr);
   SerialBufWriteField(ctx, 16, s->Len);
   SerialBufWriteField(ctx, 32, s->Addr);
   for(i = 0; i < 16;i++ )
   {
      SerialBufWriteField(ctx, 8, s->Buffer[i]);
   }
}

void MsgSlaveControlPack(struct MsgSlaveControl *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->ByteMode);
   SerialBufWriteField(ctx, 1, s->Incr);
   SerialBufWriteField(ctx, 16, s->Len);
   SerialBufWriteField(ctx, 32, s->Addr);
}

/* MsgSetBasicLayoutInfo */
void MsgSetBasicLayoutInfoPack(struct MsgSetBasicLayoutInfo *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   BasicLayoutInfoPack(&s->info, ctx);
}

/* --------------------------- MsgAddSlaveIndication ----------------------------------*/
void MsgAddSlaveDevicePack(struct MsgAddSlaveDevice *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   BasicLayoutInfoPack(&s->info, ctx);
   SerialBufWriteField(ctx, 8, s->deviceAddress);
   SerialBufWriteField(ctx, 16, s->gpioInterface);
}

/* --------------------------- MsgAddSlaveConfirm ----------------------------------*/
void MsgAddSlaveConfirmPack(struct MsgAddSlaveConfirm *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->status);
}

/* ---------------------------- MsgStopMacHandover --------------------------------*/

/* ---------------------------- MsgStartMacHandover --------------------------------*/
void MsgStartMacHandoverPack(struct MsgStartMacHandover *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Pattern);
   SerialBufWriteField(ctx, 8, s->ChannelId);
}

/* ---------------------------- MsgWakeUpMacHandover --------------------------------*/

/*------------------------------MsgFlashProgram-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_FLASH_PROGRAM.
 */
void MsgFlashProgramPack(struct MsgFlashProgram *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->NameLength);
   for(i = 0; i < 44;i++ )
   {
      SerialBufWriteField(ctx, 8, s->Name[i]);
   }
}


/*---------------------------MsgFlashProgramDone-----------------------------------*/
/**
 Msg to have a new stream
 IN_MSG_FLASH_PROGRAM_DONE.
 */
void MsgFlashProgramDonePack(struct MsgFlashProgramDone *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Status);
   SerialBufWriteField(ctx, 32, s->Size);
}

/* ----------------------------- MsgGetInfo -----------------------------*/
/*
 * Request information
 */

/*
 * Reply information data
 */
void MsgInfoDataPack(struct MsgInfoData *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Type);
   SerialBufWriteField(ctx, 8, s->Status);
   InfoParametersPack(&s->Param, s->Type, ctx);
}

/* ----------------------------- MsgSetInfo -----------------------------*/
/*
 * Set information
 */
void MsgInfoSetPack(struct MsgInfoSet *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Type);
   InfoParametersPack(&s->Param, s->Type, ctx);
}

/*
 * Get set information status
 */

/* ----------------------------- MsgSetInfo -----------------------------*/
/*
 * Toggle MPEG
 */










/*----------------------------------------------------------
 * messages
 * every msg should start by MsgXXX, have a MsgHeader, and define IN_MSG_ or OUT_MSG_ values
 *---------------------------------------------------------*/

/*------------------------------MsgDump-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_DUMP
 */
/*------------------------------MsgGetStream-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_GET_STREAM.
 */
void MsgGetStreamUnpack(struct SerialBuf *ctx, struct MsgGetStream *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->OutputOptions               = SerialBufReadField(ctx, 32, 0);
   s->Standard                    = SerialBufReadField(ctx, 32, 0);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->EnableTimeSlicing           = SerialBufReadField(ctx, 2, 0);
}

/*------------------------------MsgDeleteStream-----------------------------------*/
/**
 Msg to delete a stream
 OUT_MSG_DEL_STREAM.
 */
void MsgDeleteStreamUnpack(struct SerialBuf *ctx, struct MsgDeleteStream *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
}

/*------------------------------MsgAddFrontend-------------------------------------*/
/**
 Msg to add a frontend to a stream
 OUT_MSG_ADD_FRONTEND.
 */
void MsgAddFrontendUnpack(struct SerialBuf *ctx, struct MsgAddFrontend *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->FrontendId                  = SerialBufReadField(ctx, 16, 0);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->OutputFrontendId            = SerialBufReadField(ctx, 16, 1);
}

/*------------------------------MsgRemoveFrontend-----------------------------------*/
/**
 Msg to remove a frontend to a stream
 OUT_MSG_DEL_FRONTEND.
 */
void MsgRemoveFrontendUnpack(struct SerialBuf *ctx, struct MsgRemoveFrontend *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->FrontendId                  = SerialBufReadField(ctx, 16, 0);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
}

/*--------------------------- MsgCreateChannel --------------------------- */

void MsgCreateChannelUnpack(struct SerialBuf *ctx, struct MsgCreateChannel *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   s->TuneMonit                   = SerialBufReadField(ctx, 8, 0);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   ChannelDescUnpack(ctx, &s->Desc);

}

/*----------------------------- MsgTuneInd ------------------------------ */

void MsgTuneIndicationUnpack(struct SerialBuf *ctx, struct MsgTuneIndication *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   s->TuneOption                  = SerialBufReadField(ctx, 4, 1);
   s->TuneMonit                   = SerialBufReadField(ctx, 1, 0);
}

/*--------------------------- MsgAckCreateChannel --------------------------- */

void MsgAckCreateChannelUnpack(struct SerialBuf *ctx, struct MsgAckCreateChannel *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   s->AckOk                       = SerialBufReadField(ctx, 2, 0);
}
/*--------------------------- MsgCmmbMappingExchg -------------------------- */

void MsgCmmbMapMasterUnpack(struct SerialBuf *ctx, struct MsgCmmbMapMaster *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Status                      = SerialBufReadField(ctx, 8, 0);
   s->Initiator                   = SerialBufReadField(ctx, 8, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   CMMBmappingUnpack(ctx, &s->MappingExch);
}


/*------------------------- MsgUpdateChannelInd --------------------------- */

void MsgUpdateChannelIndicationUnpack(struct SerialBuf *ctx, struct MsgUpdateChannelIndication *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   ChannelDescUnpack(ctx, &s->Desc);

}

/*--------------------------- MsgTuneStatusConfirm --------------------------- */
void MsgTuneStatusConfirmUnpack(struct SerialBuf *ctx, struct MsgTuneStatusConfirm *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->FrontendId                  = SerialBufReadField(ctx, 16, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   s->Status                      = SerialBufReadField(ctx, 8, 1);
   s->State                       = SerialBufReadField(ctx, 8, 1);
}

/*--------------------------- MsgSleepIndication --------------------------- */

void MsgSleepIndicationUnpack(struct SerialBuf *ctx, struct MsgSleepIndication *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
}

/*-------------------------MsgDeleteChannel-----------------------------------*/
/**
 Msg request the suppression of a channel
 OUT_MSG_REQ_DEL_CH.
 */


/*-------------------------MsgCreateFilter-----------------------------------*/
/**
 Msg request the creation of a new filter
 OUT_MSG_REQ_CREATE_FILT.
 */

void MsgCreateFilterUnpack(struct SerialBuf *ctx, struct MsgCreateFilter *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   s->FilterId                    = SerialBufReadField(ctx, 8, 0);
   s->Type                        = SerialBufReadField(ctx, 8, 0);
}

/*-------------------------MsgDeleteFilter-----------------------------------*/
/**
 Msg request the suppression of a filter
 OUT_MSG_REQ_DEL_FILT.
 */


/*-------------------------MsgCreateItem -----------------------------------*/
void MsgCreateItemUnpack(struct SerialBuf *ctx, struct MsgCreateItem *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->ItemId                      = SerialBufReadField(ctx, 8, 0);
   s->FilterType                  = SerialBufReadField(ctx, 8, 0);
   s->FilterId                    = SerialBufReadField(ctx, 8, 0);
   ItemParametersUnpack(ctx, s->FilterType, &s->Param);
}

/*-------------------------MsgDeleteItem----------------------------------------*/
/**
 Msg request the suppression of a filter's item
 OUT_MSG_REQ_REM_TO_FILT
 */


/*--------------------------MsgAcknowledgeApi----------------------------------*/
/**
 Acknowledge API request.
 MSG_ACK_API identifier
 IN_MSG_ACK_DEL_CH, IN_MSG_ACK_CREATE_FILT, IN_MSG_ACK_DEL_FILT,
 IN_MSG_ACK_ADD_TO_FILT, IN_MSG_ACK_FREE_ITEM, IN_MSG_ACK_CAS_EN,
 IN_MSG_ACK_API_UP_EN
 */


/*------------------------------- MsgSetHbm ------------------------------ */

void MsgSetHbmUnpack(struct SerialBuf *ctx, struct MsgSetHbm *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Mode                        = SerialBufReadField(ctx, 32, 0);
}

/* ----------------------------- MsgRawTSMonitor-------------------------- */
/**
 */
void MsgRawTSMonitorUnpack(struct SerialBuf *ctx, struct MsgRawTSMonitor *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->TotalPackets                = SerialBufReadField(ctx, 32, 0);
   s->ErrorPackets                = SerialBufReadField(ctx, 32, 0);
   s->CCPackets                   = SerialBufReadField(ctx, 32, 0);
   s->ItemHdl                     = SerialBufReadField(ctx, 32, 0);
}

/* ----------------------------- MsgMpeMonitor ------------------------------ */
/**
  Firmware send MPE monitoring information after frame reception is complete
  Be careful : do not change the order of the following information
*/
void MsgMpeMonitorUnpack(struct SerialBuf *ctx, struct MsgMpeMonitor *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->NbRxBurst                   = SerialBufReadField(ctx, 32, 0);
   s->NbErrBeforeFec              = SerialBufReadField(ctx, 32, 0);
   s->NbErrAfterFec               = SerialBufReadField(ctx, 32, 0);
   s->NbErrTableBeforeFec         = SerialBufReadField(ctx, 32, 0);
   s->NbErrTableAfterFec          = SerialBufReadField(ctx, 32, 0);
   s->NbTableMissed               = SerialBufReadField(ctx, 32, 0);
   s->BurstStartDate              = SerialBufReadField(ctx, 32, 0);
   s->BurstEndDate                = SerialBufReadField(ctx, 32, 0);
   s->DeltatMin                   = SerialBufReadField(ctx, 32, 0);
   s->DeltatMax                   = SerialBufReadField(ctx, 32, 0);
   s->FrameDuration               = SerialBufReadField(ctx, 32, 0);
   s->PowerUpDuration             = SerialBufReadField(ctx, 32, 0);
   s->Padding                     = SerialBufReadField(ctx, 8, 0);
   s->Puncturing                  = SerialBufReadField(ctx, 8, 0);
   s->ItemHdl                     = SerialBufReadField(ctx, 8, 0);
   s->FilterId                    = SerialBufReadField(ctx, 8, 0);
   s->FrameStatus                 = SerialBufReadField(ctx, 32, 0);
}

/*-------------------------------MsgMpeIfecMonitor------------------------------*/
/**
 Firmware send IFEC monitoring information (DVBSH standard)
 */
void MsgMpeIfecMonitorUnpack(struct SerialBuf *ctx, struct MsgMpeIfecMonitor *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   /** ADT (ie. table) monitoring */
   s->NbADTTables                 = SerialBufReadField(ctx, 32, 0);
   s->NbADTTablesCorrected        = SerialBufReadField(ctx, 32, 0);
   s->NbADTTablesFailed           = SerialBufReadField(ctx, 32, 0);
   s->NbADTTablesGood             = SerialBufReadField(ctx, 32, 0);

   /** ADST (ie. burst) monitoring */
   s->NbADSTTables                = SerialBufReadField(ctx, 32, 0);
   s->NbADSTTablesLost            = SerialBufReadField(ctx, 32, 0);
   s->NbADSTTablesCorrected       = SerialBufReadField(ctx, 32, 0);
   s->NbADSTTablesFailed          = SerialBufReadField(ctx, 32, 0);
   s->NbADSTTablesGood            = SerialBufReadField(ctx, 32, 0);

   s->BurstStartDate              = SerialBufReadField(ctx, 32, 0);
   s->BurstEndDate                = SerialBufReadField(ctx, 32, 0);
   s->DeltatMin                   = SerialBufReadField(ctx, 32, 0);
   s->DeltatMax                   = SerialBufReadField(ctx, 32, 0);
   s->FrameDuration               = SerialBufReadField(ctx, 32, 0);
   s->PowerUpDuration             = SerialBufReadField(ctx, 32, 0);

   s->TableIndex                  = SerialBufReadField(ctx, 16, 0);
   s->BurstIndex                  = SerialBufReadField(ctx, 8, 0);
   s->ItemHdl                     = SerialBufReadField(ctx, 8, 0);
   s->FrameStatus                 = SerialBufReadField(ctx, 32, 0);
}

/*--------------------------------MsgFicMonitor----------------------------------*/
 /**
  Fic avalaible packets is tested with the 16bits CRC
  */
void MsgFicMonitorUnpack(struct SerialBuf *ctx, struct MsgFicMonitor *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->ItemHdl                     = SerialBufReadField(ctx, 32, 0);
   s->ErrorPackets                = SerialBufReadField(ctx, 32, 0);
   s->AvalaiblePackets            = SerialBufReadField(ctx, 32, 0);
}

/*-------------------------------MsgMscPacketMonitor------------------------------*/
/**
 Firmware send Msc Packet monitoring information
 */
void MsgMscPacketMonitorUnpack(struct SerialBuf *ctx, struct MsgMscPacketMonitor *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->ItemHdl                     = SerialBufReadField(ctx, 32, 0);
   s->MscPacketsAvail             = SerialBufReadField(ctx, 32, 0);
   s->MscPacketsError             = SerialBufReadField(ctx, 32, 0);
   s->DataGroupAvail              = SerialBufReadField(ctx, 32, 0);
   s->DataGroupError              = SerialBufReadField(ctx, 32, 0);
}

/*-------------------------------MsgMscPlusMonitor------------------------------*/
/**
 Firmware send DAB+ monitoring information
 */

/* ---------------------------- MsgGetSignalMonit --------------------------------*/

void MsgGetSignalMonitUnpack(struct SerialBuf *ctx, struct MsgGetSignalMonit *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   s->DemodId                     = SerialBufReadField(ctx, 8, 0);
}

/* ---------------------------- MsgAckGetSignalMonit -----------------------------*/

void MsgAckGetSignalMonitUnpack(struct SerialBuf *ctx, struct MsgAckGetSignalMonit *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
   s->DemodId                     = SerialBufReadField(ctx, 8, 0);
   SignalMonitoringUnpack(ctx, &s->Mon);
}


/* --------------------------------- MsgError -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */


void MsgErrorUnpack(struct SerialBuf *ctx, struct MsgError *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);

   s->ErrorCode                   = SerialBufReadField(ctx, 16, 0);
   s->ErrorFlag                   = SerialBufReadField(ctx, 16, 0);
   for(i = 0; i < 4;i++ )
   {
      s->Param[i]                    = SerialBufReadField(ctx, 32, 0);
   }
}


/* --------------------------------- MsgVersion -----------------------------------*/
/*
 * Description : Host request the version of the firmware
 */
void MsgVersionUnpack(struct SerialBuf *ctx, struct MsgVersion *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);

   s->SoftVer                     = SerialBufReadField(ctx, 16, 0);
   s->ChipVer                     = SerialBufReadField(ctx, 16, 0);
   s->HalVer                      = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 8;i++ )
   {
      s->HalVerExt[i]                = SerialBufReadField(ctx, 8, 0);
   }
}

/* ----------------------------------- MsgEvent -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */
void MsgEventUnpack(struct SerialBuf *ctx, struct MsgEvent *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->ItemId                      = SerialBufReadField(ctx, 8, 0);
   s->EventId                     = SerialBufReadField(ctx, 8, 0);
}

/* -------------------------------- MsgCmmbRsmMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB LDPC and RS monitoring information at end of RS matrix.
 */
void MsgCmmbRsmMonitorUnpack(struct SerialBuf *ctx, struct MsgCmmbRsmMonitor *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   /* General Info */
   s->NbFecErr                    = SerialBufReadField(ctx, 16, 0);
   s->ItemId                      = SerialBufReadField(ctx, 8, 0);
   s->Status                      = SerialBufReadField(ctx, 8, 0);

   /* LDPC block error rate. */
   s->TotalLdpcBlocks             = SerialBufReadField(ctx, 32, 0);
   s->ErrorLdpcBlocks             = SerialBufReadField(ctx, 32, 0);

   /* RS matrix error rate. */
   s->TotalRsMatrices             = SerialBufReadField(ctx, 32, 0);
   s->ErrorRsMatricesBeforeRs     = SerialBufReadField(ctx, 32, 0);
   s->ErrorRsMatricesAfterRs      = SerialBufReadField(ctx, 32, 0);
}


/* ----------------------------- MsgCmmbMfMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB MF information at end of each MF.
 */
void MsgCmmbMfMonitorUnpack(struct SerialBuf *ctx, struct MsgCmmbMfMonitor *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   /* Multiplex frame info. */
   s->Status                      = SerialBufReadField(ctx, 4, 0);
   s->ItemId                      = SerialBufReadField(ctx, 6, 0);

   /* Multiplex Frames. */
   s->TotalMf                     = SerialBufReadField(ctx, 32, 0);
   s->ErrorMfBeforeRs             = SerialBufReadField(ctx, 32, 0);
   s->ErrorMfAfterRs              = SerialBufReadField(ctx, 32, 0);
}


/* ----------------------------- MsgDebugHelp -----------------------------*/
/*
 * Set a Debug function of the embedded firmware.
 * Func:
 *    - 0: memory dumping
 *    - 1: ldpc discarder
 *    - 11: Enable Real Time Discarder. Value is the SNR
 *    - 12: dvb-sh discarded
 */
void MsgDebugHelpUnpack(struct SerialBuf *ctx, struct MsgDebugHelp *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->Funct                       = SerialBufReadField(ctx, 16, 0);
   s->Value                       = SerialBufReadField(ctx, 16, 0);
}


/* ----------------------------- MsgPrint -----------------------------*/
/*
 * Message used to send a string to the host
 */
void MsgPrintUnpack(struct SerialBuf *ctx, struct MsgPrint *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);

   for(i = 0; i < 200;i++ )
   {
      s->Data[i]                     = SerialBufReadField(ctx, 8, 1);
   }
}

/* ----------------------------- MsgEnableTimeSlice -----------------------------*/
/*
 * enable/disable time-slicing for a dedicated stream
 */

/* --------------------------- MsgAbortTuneMonit ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */

/* --------------------------- MsgSetPidMode ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */
void MsgSetPidModeUnpack(struct SerialBuf *ctx, struct MsgSetPidMode *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->ItemId                      = SerialBufReadField(ctx, 8, 0);
   s->Prefetch                    = SerialBufReadField(ctx, 1, 0);
}


/* --------------------------- MsgCasEnable ---------------------------*/
/*
 * Enable/disable CAS system
 */
void MsgCasEnableUnpack(struct SerialBuf *ctx, struct MsgCasEnable *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Enable                      = SerialBufReadField(ctx, 1, 0);
}


/* ------------------------ MsgAHBAccess ----------------------------*/
/*
 * Internal access done by firmware but requested by the host
 */
void MsgAHBAccessUnpack(struct SerialBuf *ctx, struct MsgAHBAccess *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Addr                        = SerialBufReadField(ctx, 32, 0);
   s->Value                       = SerialBufReadField(ctx, 32, 0);
   s->Mode                        = SerialBufReadField(ctx, 1, 0);
}

/* ------------------------ MsgHostReflexInit ----------------------------*/
/*
 * Address in emb of reflex context needed for emulation
 */

/* ------------------------ MsgHostReflexStep ----------------------------*/
/*
 * Command to advance one step in emulation
 */

/* ------------------------ MsgHostReflexSetTsConfig ----------------------------*/
/*
 * Each time time slot configuration changes, warn the host
 */
void MsgHostReflexSetTsConfigUnpack(struct SerialBuf *ctx, struct MsgHostReflexSetTsConfig *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   s->Configuration               = SerialBufReadField(ctx, 32, 0);
   s->TimeSlotMap_0_31            = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 10;i++ )
   {
      s->ParamIdFourTimeSlots[i]     = SerialBufReadField(ctx, 32, 0);
   }
}

/* ------------------------ MsgHostReflexSetSvcParam ----------------------------*/
/*
 * Each time service parameters changes, warn the host
 */

/* --------------------------- MsgGpioConfig ------------------------------*/

void MsgGpioConfigUnpack(struct SerialBuf *ctx, struct MsgGpioConfig *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   s->NbFuncs                     = SerialBufReadField(ctx, 16, 0);
   s->FeId                        = SerialBufReadField(ctx, 16, 0);
   for(i = 0; i < 6;i++ )
   {
      GpioFunctionUnpack(ctx, &s->Func[i]);
   }
}

/* --------------------------- MsgLayoutConfig ------------------------------*/
void MsgLayoutConfigUnpack(struct SerialBuf *ctx, struct MsgLayoutConfig *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   s->Cfg                         = SerialBufReadField(ctx, 32, 0);
   s->LnaConfig                   = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 2;i++ )
   {
      s->GpioDefaultDir[i]           = SerialBufReadField(ctx, 32, 0);
   }
   for(i = 0; i < 2;i++ )
   {
      s->GpioDefaultVal[i]           = SerialBufReadField(ctx, 32, 0);
   }
   s->CurTrig                     = SerialBufReadField(ctx, 16, 0);
   s->MaxTrig                     = SerialBufReadField(ctx, 16, 0);
   s->FeId                        = SerialBufReadField(ctx, 32, 0);
   s->GpioNum                     = SerialBufReadField(ctx, 32, 0);
   s->GpioDir                     = SerialBufReadField(ctx, 32, 0);
   s->GpioVal                     = SerialBufReadField(ctx, 32, 0);
   s->FeIdMask                    = SerialBufReadField(ctx, 32, 0);
   s->TuneStateMask               = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 2;i++ )
   {
      s->FreqRangeMhz[i]             = SerialBufReadField(ctx, 32, 0);
   }
   s->ThresholdVal                = SerialBufReadField(ctx, 32, 0);
   s->Condition                   = SerialBufReadField(ctx, 1, 0);
   s->OutsideBand                 = SerialBufReadField(ctx, 1, 0);
   s->ThresholdDir                = SerialBufReadField(ctx, 1, 0);
}


/* --------------------------- MsgAckLayoutConfig ------------------------------*/


/* --------------------------- MsgAckLayoutReset ------------------------------*/


/* ----------------------------- MsgGetLayoutInfo -----------------------------*/
/*
 * Request information concerning the layout
 */

/* ----------------------------- MsgLayoutInfo --------------------------------*/
/*
 * Reply information concerning the layout
 */

/* ----------------------------- MsgGetFrontendInfo ----------------------------*/
/*
 * Request information concerning a specific frontend
 */
void MsgGetFrontendInfoUnpack(struct SerialBuf *ctx, struct MsgGetFrontendInfo *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->FeNum                       = SerialBufReadField(ctx, 8, 0);
}

/* ----------------------------- MsgFrontendInfo -------------------------------*/
/*
 * Request information concerning a specific frontend
 */
void MsgFrontendInfoUnpack(struct SerialBuf *ctx, struct MsgFrontendInfo *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->FeNum                       = SerialBufReadField(ctx, 8, 0);
   FrontendInfoUnpack(ctx, &s->FeInfo);
}

/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Request information concerning a specific stream
 */

/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Reply information concerning a stream
 */

void MsgStreamInfoUnpack(struct SerialBuf *ctx, struct MsgStreamInfo *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->StreamNum                   = SerialBufReadField(ctx, 8, 0);
   StreamInfoUnpack(ctx, &s->StreamInfo);
}


/* ----------------------------- MsgDownloadIndication ------------------------------*/
/*
 * Request for downloading a file
 */
void MsgDownloadIndicationUnpack(struct SerialBuf *ctx, struct MsgDownloadIndication *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   for(i = 0; i < 44;i++ )
   {
      s->FirmwareName[i]             = SerialBufReadField(ctx, 8, 0);
   }
   s->AddrMsb                     = SerialBufReadField(ctx, 32, 0);
   s->AddrLsb                     = SerialBufReadField(ctx, 32, 0);
   s->LengthAckMsb                = SerialBufReadField(ctx, 32, 0);
   s->LengthAckLsb                = SerialBufReadField(ctx, 32, 0);
   s->FileOffset                  = SerialBufReadField(ctx, 32, 0);
   s->MaxLength                   = SerialBufReadField(ctx, 28, 0);
   s->Options                     = SerialBufReadField(ctx, 4, 0);
}


/* ----------------------------- MsgDownloadConfirm ------------------------------*/

/*
 * confirmation of file download
 */
void MsgDownloadConfirmUnpack(struct SerialBuf *ctx, struct MsgDownloadConfirm *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->LengthAckMsb                = SerialBufReadField(ctx, 32, 0);
   s->LengthAckLsb                = SerialBufReadField(ctx, 32, 0);
   s->Length                      = SerialBufReadField(ctx, 28, 0);
   s->Complete                    = SerialBufReadField(ctx, 1, 0);
   s->WithCrc                     = SerialBufReadField(ctx, 1, 0);
}

/* ----------------------------- MsgApiRequest ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer to the firmware and reecive one in return.
 * Each MsgApiRequest must be acknowledged before sending a new one.
 */


void MsgApiRequestUnpack(struct SerialBuf *ctx, struct MsgApiRequest *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->ReqId                       = SerialBufReadField(ctx, 16, 0);
   s->ReqType                     = SerialBufReadField(ctx, 16, 0);
   s->TxLen                       = SerialBufReadField(ctx, 32, 0);
   s->TxAddr                      = SerialBufReadField(ctx, 32, 0);
}

/* ----------------------------- MsgApiAnswer ------------------------------*/
/*
 * This is a reply to the MsgApiRequest, with the possibility to allow buffer retreival fron embedded buffer to
 * host buffer.
 */

/* ----------------------------- MsgApiUpEnable ------------------------------*/
/*
 * This message is used to enable/disable the fact that the firmware can send msg api information to the user
 * without any request from it. In that case, the user is notified from the sdk through a message callback mechanism, with the
 * possibility to get also data buffers.
 */

/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer from the firwmare to the host is permitted (see MsgApiUpEnable)
 * Each MsgApiUp must be acknoledged by a MsgApiUpAck whose role is to free buffer allocated at RxAddr.
 */

/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * This message is used to free the memory allocated by firmware when sending the MsgApiUp message
 */

/* ------------------------------ MsgOctoInit --------------------------*/


void MsgOctoInitUnpack(struct SerialBuf *ctx, struct MsgOctoInit *s)
{
   MsgHeaderUnpack(ctx, &s->GlobalHeader);
   MsgHeaderUnpack(ctx, &s->Header);
}

/* ------------------------- MsgOctoUtilsCheckSum ----------------------*/
void MsgOctoUtilsCheckSumUnpack(struct SerialBuf *ctx, struct MsgOctoUtilsCheckSum *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   MsgHeaderUnpack(ctx, &s->InternalHead);
   s->Hello                       = SerialBufReadField(ctx, 32, 0);
}


/* --------------------------- MsgOctoRwOctopus --------------------------*/
void MsgOctoRwOctopusUnpack(struct SerialBuf *ctx, struct MsgOctoRwOctopus *s)
{
   MsgHeaderUnpack(ctx, &s->GlobalHeader);
   MsgHeaderUnpack(ctx, &s->Header);
   s->Config                      = SerialBufReadField(ctx, 32, 0);
   s->Size                        = SerialBufReadField(ctx, 32, 0);
   s->Last                        = SerialBufReadField(ctx, 32, 0);
}

/* --------------------------- MsgHostOctoInit --------------------------*/
/*
 * Indicates the host where to find the data buffer inside chipset memory
 */

/* --------------------------- MsgHostOctoRwOctopus ----------------------*/

/* --------------------------- MsgWave -----------------------------------*/

/* --------------------------- MsgIntDebug -------------------------------*/

/* --------------------------- MsgProfile --------------------------------*/


/* --------------------------- debugCmd ----------------------------------*/

/* Mode */
void MsgSlaveDataUnpack(struct SerialBuf *ctx, struct MsgSlaveData *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   s->ByteMode                    = SerialBufReadField(ctx, 8, 0);
   s->Incr                        = SerialBufReadField(ctx, 1, 0);
   s->Len                         = SerialBufReadField(ctx, 16, 0);
   s->Addr                        = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 16;i++ )
   {
      s->Buffer[i]                   = SerialBufReadField(ctx, 8, 0);
   }
}

void MsgSlaveControlUnpack(struct SerialBuf *ctx, struct MsgSlaveControl *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->ByteMode                    = SerialBufReadField(ctx, 8, 0);
   s->Incr                        = SerialBufReadField(ctx, 1, 0);
   s->Len                         = SerialBufReadField(ctx, 16, 0);
   s->Addr                        = SerialBufReadField(ctx, 32, 0);
}

/* MsgSetBasicLayoutInfo */
void MsgSetBasicLayoutInfoUnpack(struct SerialBuf *ctx, struct MsgSetBasicLayoutInfo *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   BasicLayoutInfoUnpack(ctx, &s->info);
}

/* --------------------------- MsgAddSlaveIndication ----------------------------------*/
void MsgAddSlaveDeviceUnpack(struct SerialBuf *ctx, struct MsgAddSlaveDevice *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   BasicLayoutInfoUnpack(ctx, &s->info);
   s->deviceAddress               = SerialBufReadField(ctx, 8, 0);
   s->gpioInterface               = SerialBufReadField(ctx, 16, 0);
}

/* --------------------------- MsgAddSlaveConfirm ----------------------------------*/
void MsgAddSlaveConfirmUnpack(struct SerialBuf *ctx, struct MsgAddSlaveConfirm *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->status                      = SerialBufReadField(ctx, 32, 1);
}

/* ---------------------------- MsgStopMacHandover --------------------------------*/

/* ---------------------------- MsgStartMacHandover --------------------------------*/
void MsgStartMacHandoverUnpack(struct SerialBuf *ctx, struct MsgStartMacHandover *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Pattern                     = SerialBufReadField(ctx, 32, 0);
   s->ChannelId                   = SerialBufReadField(ctx, 8, 0);
}

/* ---------------------------- MsgWakeUpMacHandover --------------------------------*/

/*------------------------------MsgFlashProgram-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_FLASH_PROGRAM.
 */
void MsgFlashProgramUnpack(struct SerialBuf *ctx, struct MsgFlashProgram *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   s->NameLength                  = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 44;i++ )
   {
      s->Name[i]                     = SerialBufReadField(ctx, 8, 1);
   }
}


/*---------------------------MsgFlashProgramDone-----------------------------------*/
/**
 Msg to have a new stream
 IN_MSG_FLASH_PROGRAM_DONE.
 */
void MsgFlashProgramDoneUnpack(struct SerialBuf *ctx, struct MsgFlashProgramDone *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Status                      = SerialBufReadField(ctx, 32, 1);
   s->Size                        = SerialBufReadField(ctx, 32, 0);
}

/* ----------------------------- MsgGetInfo -----------------------------*/
/*
 * Request information
 */

/*
 * Reply information data
 */
void MsgInfoDataUnpack(struct SerialBuf *ctx, struct MsgInfoData *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Type                        = SerialBufReadField(ctx, 32, 0);
   s->Status                      = SerialBufReadField(ctx, 8, 0);
   InfoParametersUnpack(ctx, s->Type, &s->Param);
}

/* ----------------------------- MsgSetInfo -----------------------------*/
/*
 * Set information
 */
void MsgInfoSetUnpack(struct SerialBuf *ctx, struct MsgInfoSet *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Type                        = SerialBufReadField(ctx, 32, 0);
   InfoParametersUnpack(ctx, s->Type, &s->Param);
}

/*
 * Get set information status
 */

/* ----------------------------- MsgSetInfo -----------------------------*/
/*
 * Toggle MPEG
 */





