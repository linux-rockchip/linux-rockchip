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





#include "DibMsgData.h"

/* ---------------------------------- MsgData --------------------------------*/
/*
 * a 32 bit that contains specific information for data retreival
 */
void DataFormatPack(struct DataFormat *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 8, s->FrameId);
   SerialBufWriteField(ctx, 3, s->NbRows);
   SerialBufWriteField(ctx, 5, s->ItemId);
   SerialBufWriteField(ctx, 5, s->Type);
   SerialBufWriteField(ctx, 1, s->LastFrag);
   SerialBufWriteField(ctx, 1, s->FirstFrag);
   SerialBufWriteField(ctx, 1, s->BlockType);
   SerialBufWriteField(ctx, 8, s->BlockId);
}

/* size of sipsi header for SIPSI delivery encapsulation */

/*  Format types */


/* ----------------------------- MsgData -----------------------------*/
/*
 * Used for HBM/BBM tranfers between host and chip
 */
void MsgDataPack(struct MsgData *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 32, s->Format);
   SerialBufWriteField(ctx, 32, s->Min);
   SerialBufWriteField(ctx, 32, s->Max);
   SerialBufWriteField(ctx, 32, s->Add);
   SerialBufWriteField(ctx, 32, s->Len);
   SerialBufWriteField(ctx, 32, s->AddMsb);
}

/* ----------------------------- MsgFrameInfo -----------------------------*/
/*
 * Table of info sent to the host for hbm mainly
 */
/*  Format for IN_MSG_FRAME_INFO (8 bits) */




/* ----------------------------- MsgCtrlMonit -----------------------------*/
/*
 * Description : host requests to clear firmware monitoring info
 */
void MsgCtrlMonitPack(struct MsgCtrlMonit *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 8, s->ItemId);
   SerialBufWriteField(ctx, 8, s->Cmd);
}


/* ------------------------------ MsgAcknowledgeApi ----------------------------*/

/* --------------------------------- MsgAckData --------------------------------*/
/*
 * Description : Host acknowledge a MsgData send by the firmware
 */
void MsgAckDataPack(struct MsgAckData *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 32, s->Format);
   SerialBufWriteField(ctx, 32, s->Status);
}

/* ----------------------------- MsgChecker -----------------------------*/
/*
 * Give user space statictics comming with ip data analysis, so that it can be added to monitoring information
 * sent by the firmware.
 */
void MsgCheckerPack(struct MsgChecker *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);

   SerialBufWriteField(ctx, 32, s->ItemId);
   SerialBufWriteField(ctx, 32, s->Total);
   SerialBufWriteField(ctx, 32, s->CcFailCnt);
   SerialBufWriteField(ctx, 32, s->ErrCnt);
}


/* ----------------------------- MsgHostBufFail -----------------------------*/
/*
 * Host indeicates emb that it cannot bufferize incoming data
 */

/* ----------------------------- MsgClearHostIrq ------------------------------*/
/*
 * Ack firmware to clear host interface irq itself. This is sometimes not possible to
 * do it directly from host due to apb conflicts
 */


/* LAYOUT config id - used to trigger sensitive RESETs */





/* ---------------------------------- MsgData --------------------------------*/
/*
 * a 32 bit that contains specific information for data retreival
 */
void DataFormatUnpack(struct SerialBuf *ctx, struct DataFormat *s)
{
   s->FrameId                     = SerialBufReadField(ctx, 8, 0);
   s->NbRows                      = SerialBufReadField(ctx, 3, 0);
   s->ItemId                      = SerialBufReadField(ctx, 5, 0);
   s->Type                        = SerialBufReadField(ctx, 5, 0);
   s->LastFrag                    = SerialBufReadField(ctx, 1, 0);
   s->FirstFrag                   = SerialBufReadField(ctx, 1, 0);
   s->BlockType                   = SerialBufReadField(ctx, 1, 0);
   s->BlockId                     = SerialBufReadField(ctx, 8, 0);
}

/* size of sipsi header for SIPSI delivery encapsulation */

/*  Format types */


/* ----------------------------- MsgData -----------------------------*/
/*
 * Used for HBM/BBM tranfers between host and chip
 */
void MsgDataUnpack(struct SerialBuf *ctx, struct MsgData *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Format                      = SerialBufReadField(ctx, 32, 0);
   s->Min                         = SerialBufReadField(ctx, 32, 0);
   s->Max                         = SerialBufReadField(ctx, 32, 0);
   s->Add                         = SerialBufReadField(ctx, 32, 0);
   s->Len                         = SerialBufReadField(ctx, 32, 0);
   s->AddMsb                      = SerialBufReadField(ctx, 32, 0);
}

/* ----------------------------- MsgFrameInfo -----------------------------*/
/*
 * Table of info sent to the host for hbm mainly
 */
/*  Format for IN_MSG_FRAME_INFO (8 bits) */




/* ----------------------------- MsgCtrlMonit -----------------------------*/
/*
 * Description : host requests to clear firmware monitoring info
 */
void MsgCtrlMonitUnpack(struct SerialBuf *ctx, struct MsgCtrlMonit *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->ItemId                      = SerialBufReadField(ctx, 8, 0);
   s->Cmd                         = SerialBufReadField(ctx, 8, 0);
}


/* ------------------------------ MsgAcknowledgeApi ----------------------------*/

/* --------------------------------- MsgAckData --------------------------------*/
/*
 * Description : Host acknowledge a MsgData send by the firmware
 */
void MsgAckDataUnpack(struct SerialBuf *ctx, struct MsgAckData *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->Format                      = SerialBufReadField(ctx, 32, 0);
   s->Status                      = SerialBufReadField(ctx, 32, 0);
}

/* ----------------------------- MsgChecker -----------------------------*/
/*
 * Give user space statictics comming with ip data analysis, so that it can be added to monitoring information
 * sent by the firmware.
 */
void MsgCheckerUnpack(struct SerialBuf *ctx, struct MsgChecker *s)
{
   MsgHeaderUnpack(ctx, &s->Head);

   s->ItemId                      = SerialBufReadField(ctx, 32, 0);
   s->Total                       = SerialBufReadField(ctx, 32, 0);
   s->CcFailCnt                   = SerialBufReadField(ctx, 32, 0);
   s->ErrCnt                      = SerialBufReadField(ctx, 32, 0);
}


/* ----------------------------- MsgHostBufFail -----------------------------*/
/*
 * Host indeicates emb that it cannot bufferize incoming data
 */

/* ----------------------------- MsgClearHostIrq ------------------------------*/
/*
 * Ack firmware to clear host interface irq itself. This is sometimes not possible to
 * do it directly from host due to apb conflicts
 */


/* LAYOUT config id - used to trigger sensitive RESETs */

