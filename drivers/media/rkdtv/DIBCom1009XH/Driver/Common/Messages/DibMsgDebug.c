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




#include "DibMsgDebug.h"


/* Generic acknowledge for Debug messages */
void MsgDebugAcknowledgePack(struct MsgDebugAcknowledge *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->idMsgOrigin);
}

/*------------------------------MsgOctoSetTraceLevel-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_OCTO_SETTRACELEVEL
 */
void MsgOctoSetTraceLevelPack(struct MsgOctoSetTraceLevel *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->Level);
}
/*------------------------------MsgOctoEnableDump-----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */

/*------------------------------MsgOctoEnableGet----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */



void MsgOctoInitializeDescPack(struct MsgOctoInitializeDesc *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 2, s->TransType);
}

void MsgOctoWriteMemPack(struct MsgOctoWriteMem *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->Address);
   SerialBufWriteField(ctx, 16, s->Value);
}

void MsgOctoReadMemPack(struct MsgOctoReadMem *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->Address);
}


/* Access one "Net"/Register */
void MsgRegisterAccessPack(struct MsgRegisterAccess *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 1, s->ReadWrite);
   SerialBufWriteField(ctx, 32, s->Address);
   SerialBufWriteField(ctx, 6, s->Offset);
   SerialBufWriteField(ctx, 6, s->Bits);
   SerialBufWriteField(ctx, 6, s->AccessType);
   SerialBufWriteField(ctx, 32, s->Msb);
   SerialBufWriteField(ctx, 32, s->Lsb);
}

/* This message is sent when the write or read is done
 * Msb, Lsb are used for the value read even if a write was done
 */
void MsgRegisterAccessAckPack(struct MsgRegisterAccessAck *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Msb);
   SerialBufWriteField(ctx, 32, s->Lsb);
   SerialBufWriteField(ctx, 32, s->TimeStamp);
}

void MsgRegisterEnablePeriodicgReadPack(struct MsgRegisterEnablePeriodicgRead *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Address);
   SerialBufWriteField(ctx, 16, s->ByteCount);
   SerialBufWriteField(ctx, 32, s->IntervalMsec);
   SerialBufWriteField(ctx, 8, s->AccessType);
}

/* this define is used in the ID-field of MsgRegisterEnablePeriodicReadAck and MsgRegisterDisablePeriodicRead */

void MsgRegisterEnablePeriodicReadAckPack(struct MsgRegisterEnablePeriodicReadAck *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 16, s->Id);
   SerialBufWriteField(ctx, 32, s->Address);
}



void MsgFrontendGetInfoIndicationPack(struct MsgFrontendGetInfoIndication *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->bandwidth_kHz);
   SerialBufWriteField(ctx, 8, s->standard);
   SerialBufWriteField(ctx, 5, s->FFT);
   SerialBufWriteField(ctx, 32, s->sampling_frequency_hz);
   SerialBufWriteField(ctx, 8, s->front_end);
}



void MsgPeriodicReadingValuePack(struct MsgPeriodicReadingValue *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->Size);
   SerialBufWriteField(ctx, 32, s->Address);
   SerialBufWriteField(ctx, 32, s->TimeStamp);
   for(i = 0; i < 8;i++ )
   {
      SerialBufWriteField(ctx, 8, s->Buffer[i]);
   }
}


void MsgAcquisitionModeCtrlPack(struct MsgAcquisitionModeCtrl *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 1, s->Enable);
   SerialBufWriteField(ctx, 8, s->Type);
/* Passthru enables the forwarding of the data received from a slave-device (mode as defined in Type) on this device */
   SerialBufWriteField(ctx, 1, s->Passthru);
}

void MsgTunerAgcFreezePack(struct MsgTunerAgcFreeze *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 1, s->Enable);
   SerialBufWriteField(ctx, 16, s->AntennaId);
}

void MsgGetChipInfoIndicationPack(struct MsgGetChipInfoIndication *s, struct SerialBuf *ctx)
{
   int i;
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->TimeStamp);
   SerialBufWriteField(ctx, 16, s->ChipVer);
   SerialBufWriteField(ctx, 32, s->HalVer);
   for(i = 0; i < 8;i++ )
   {
      SerialBufWriteField(ctx, 8, s->HalVerExt[i]);
   }
   SerialBufWriteField(ctx, 32, s->TunerVers);
   SerialBufWriteField(ctx, 8, s->NbTuner);
   SerialBufWriteField(ctx, 32, s->OffsetTuner);
   SerialBufWriteField(ctx, 32, s->OffsetSPAL);
}


void MsgSDKInfoIndicationPack(struct MsgSDKInfoIndication *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->SDKVer);
   SerialBufWriteField(ctx, 8, s->Nb);
}







/* Generic acknowledge for Debug messages */
void MsgDebugAcknowledgeUnpack(struct SerialBuf *ctx, struct MsgDebugAcknowledge *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->idMsgOrigin                 = SerialBufReadField(ctx, 8, 0);
}

/*------------------------------MsgOctoSetTraceLevel-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_OCTO_SETTRACELEVEL
 */
void MsgOctoSetTraceLevelUnpack(struct SerialBuf *ctx, struct MsgOctoSetTraceLevel *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Level                       = SerialBufReadField(ctx, 16, 1);
}
/*------------------------------MsgOctoEnableDump-----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */

/*------------------------------MsgOctoEnableGet----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */



void MsgOctoInitializeDescUnpack(struct SerialBuf *ctx, struct MsgOctoInitializeDesc *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->TransType                   = SerialBufReadField(ctx, 2, 1);
}

void MsgOctoWriteMemUnpack(struct SerialBuf *ctx, struct MsgOctoWriteMem *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Address                     = SerialBufReadField(ctx, 16, 0);
   s->Value                       = SerialBufReadField(ctx, 16, 1);
}

void MsgOctoReadMemUnpack(struct SerialBuf *ctx, struct MsgOctoReadMem *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Address                     = SerialBufReadField(ctx, 16, 0);
}


/* Access one "Net"/Register */
void MsgRegisterAccessUnpack(struct SerialBuf *ctx, struct MsgRegisterAccess *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->ReadWrite                   = SerialBufReadField(ctx, 1, 0);
   s->Address                     = SerialBufReadField(ctx, 32, 0);
   s->Offset                      = SerialBufReadField(ctx, 6, 0);
   s->Bits                        = SerialBufReadField(ctx, 6, 0);
   s->AccessType                  = SerialBufReadField(ctx, 6, 0);
   s->Msb                         = SerialBufReadField(ctx, 32, 0);
   s->Lsb                         = SerialBufReadField(ctx, 32, 0);
}

/* This message is sent when the write or read is done
 * Msb, Lsb are used for the value read even if a write was done
 */
void MsgRegisterAccessAckUnpack(struct SerialBuf *ctx, struct MsgRegisterAccessAck *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Msb                         = SerialBufReadField(ctx, 32, 0);
   s->Lsb                         = SerialBufReadField(ctx, 32, 0);
   s->TimeStamp                   = SerialBufReadField(ctx, 32, 0);
}

void MsgRegisterEnablePeriodicgReadUnpack(struct SerialBuf *ctx, struct MsgRegisterEnablePeriodicgRead *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Address                     = SerialBufReadField(ctx, 32, 0);
   s->ByteCount                   = SerialBufReadField(ctx, 16, 0);
   s->IntervalMsec                = SerialBufReadField(ctx, 32, 0);
   s->AccessType                  = SerialBufReadField(ctx, 8, 0);
}

/* this define is used in the ID-field of MsgRegisterEnablePeriodicReadAck and MsgRegisterDisablePeriodicRead */

void MsgRegisterEnablePeriodicReadAckUnpack(struct SerialBuf *ctx, struct MsgRegisterEnablePeriodicReadAck *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Id                          = SerialBufReadField(ctx, 16, 0);
   s->Address                     = SerialBufReadField(ctx, 32, 0);
}



void MsgFrontendGetInfoIndicationUnpack(struct SerialBuf *ctx, struct MsgFrontendGetInfoIndication *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->bandwidth_kHz               = SerialBufReadField(ctx, 32, 0);
   s->standard                    = SerialBufReadField(ctx, 8, 0);
   s->FFT                         = SerialBufReadField(ctx, 5, 0);
   s->sampling_frequency_hz       = SerialBufReadField(ctx, 32, 0);
   s->front_end                   = SerialBufReadField(ctx, 8, 0);
}



void MsgPeriodicReadingValueUnpack(struct SerialBuf *ctx, struct MsgPeriodicReadingValue *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   s->Size                        = SerialBufReadField(ctx, 8, 0);
   s->Address                     = SerialBufReadField(ctx, 32, 0);
   s->TimeStamp                   = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 8;i++ )
   {
      s->Buffer[i]                   = SerialBufReadField(ctx, 8, 0);
   }
}


void MsgAcquisitionModeCtrlUnpack(struct SerialBuf *ctx, struct MsgAcquisitionModeCtrl *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Enable                      = SerialBufReadField(ctx, 1, 0);
   s->Type                        = SerialBufReadField(ctx, 8, 1);
/* Passthru enables the forwarding of the data received from a slave-device (mode as defined in Type) on this device */
   s->Passthru                    = SerialBufReadField(ctx, 1, 0);
}

void MsgTunerAgcFreezeUnpack(struct SerialBuf *ctx, struct MsgTunerAgcFreeze *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Enable                      = SerialBufReadField(ctx, 1, 0);
   s->AntennaId                   = SerialBufReadField(ctx, 16, 0);
}

void MsgGetChipInfoIndicationUnpack(struct SerialBuf *ctx, struct MsgGetChipInfoIndication *s)
{
   int i;
   MsgHeaderUnpack(ctx, &s->Head);
   s->TimeStamp                   = SerialBufReadField(ctx, 32, 0);
   s->ChipVer                     = SerialBufReadField(ctx, 16, 0);
   s->HalVer                      = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 8;i++ )
   {
      s->HalVerExt[i]                = SerialBufReadField(ctx, 8, 0);
   }
   s->TunerVers                   = SerialBufReadField(ctx, 32, 0);
   s->NbTuner                     = SerialBufReadField(ctx, 8, 0);
   s->OffsetTuner                 = SerialBufReadField(ctx, 32, 0);
   s->OffsetSPAL                  = SerialBufReadField(ctx, 32, 0);
}


void MsgSDKInfoIndicationUnpack(struct SerialBuf *ctx, struct MsgSDKInfoIndication *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->SDKVer                      = SerialBufReadField(ctx, 32, 0);
   s->Nb                          = SerialBufReadField(ctx, 8, 0);
}



