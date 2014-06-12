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

#ifndef __MSG_TYPE_DATA__
#define __MSG_TYPE_DATA__

#ifdef __cplusplus
#define EXTERN  extern "C"
#else
#define EXTERN
#endif


/* Max message length */
#define MAX_MSG_DATA_BITS              224

/* Input message identifiers */
#define IN_MSG_DATA                    0
#define IN_MSG_FRAME_INFO              1
#define IN_MSG_CTL_MONIT               2
#define IN_MSG_ACK_FREE_ITEM           3
#define IN_MSG_CHECKER                 4
#define MAX_MSG_DATA_IN                5

/* Output message identifiers */
#define OUT_MSG_HBM_ACK                0
#define OUT_MSG_HOST_BUF_FAIL          1
#define OUT_MSG_CTL_MONIT              18
#define OUT_MSG_CLEAR_HOST_IRQ         27
#define OUT_MSG_UDIBADAPTER_CFG        45
#define MAX_MSG_DATA_OUT               46




#include "DibMsgTypes.h"
#include "DibMsgGeneric.h"
#include "DibMsgFields.h"
#include "DibMsgDataIf.h"


/* ---------------------------------- MsgData --------------------------------*/
/*
 * a 32 bit that contains specific information for data retreival
 */
struct DataFormat
{
   uint8_t FrameId;
   uint8_t NbRows;
   uint8_t ItemId;
   uint8_t Type;
   uint8_t LastFrag;
   uint8_t FirstFrag;
   uint8_t BlockType;
   uint8_t BlockId;
};
/* size of sipsi header for SIPSI delivery encapsulation */
#define SIPSI_HEADER_SIZE              4

/*  Format types */
#define FORMAT_MPE                     1
#define FORMAT_SIPSI                   2
#define FORMAT_RAWTS                   3
#define FORMAT_PES_VIDEO               4
#define FORMAT_PES_AUDIO               5
#define FORMAT_PES_OTHER               6
#define FORMAT_PCR                     7
#define FORMAT_FRG                     8
#define FORMAT_LAST_FRG                9
#define FORMAT_MPE_CORR                10
#define FORMAT_SLICE_REQ               11
#define FORMAT_SLICE_DONE              12
#define FORMAT_FLUSH_SVC               13
#define FORMAT_DTRACE                  14
#define FORMAT_FIG                     15
#define FORMAT_MSC                     16
#define FORMAT_AUDIO                   17
#define FORMAT_VIDEO                   18
#define FORMAT_SH_FRG_DATA             19
#define FORMAT_SH_FRG_IFEC             20
#define FORMAT_SH_PATTERN_DATA         21
#define FORMAT_SH_PATTERN_IFEC         22
#define FORMAT_SH_ALLOC                23
#define FORMAT_SH_FILL_DATA            24
#define FORMAT_SH_FILL_IFEC            25
#define FORMAT_IMAGE                   26
#define FORMAT_SAMPLE                  27
#define FORMAT_EVENT                   28
#define FORMAT_MSCPLUS                 28
#define FORMAT_CMMB_RSM                29
#define FORMAT_TRACE_BUFFER            30
#define FORMAT_MSCPACKET               31
#define FORMAT_RAWMEM                  32
#define FORMAT_UNKNOWN                 255


/* ----------------------------- MsgData -----------------------------*/
/*
 * Used for HBM/BBM tranfers between host and chip
 */
struct MsgData
{
   struct MsgHeader Head;
   uint32_t Format;
   uint32_t Min;
   uint32_t Max;
   uint32_t Add;
   uint32_t Len;
   uint32_t AddMsb;
};
/* ----------------------------- MsgFrameInfo -----------------------------*/
/*
 * Table of info sent to the host for hbm mainly
 */
/*  Format for IN_MSG_FRAME_INFO (8 bits) */

#define FORMAT_FLUSH_DATA              16
#define FORMAT_MOVE_DATA               17
#define FORMAT_MAPP_TO_ADST            32
#define FORMAT_IFEC_RESET_ADT_PTR      64
#define FORMAT_IFEC_RESET_ALL_PTR      65


struct MsgFrameInfo
{
   struct MsgHeader Head;
};
/* ----------------------------- MsgCtrlMonit -----------------------------*/
/*
 * Description : host requests to clear firmware monitoring info
 */
struct MsgCtrlMonit
{
   struct MsgHeader Head;

   uint8_t ItemId;
   uint8_t Cmd;
};

/* ------------------------------ MsgAcknowledgeApi ----------------------------*/

/* --------------------------------- MsgAckData --------------------------------*/
/*
 * Description : Host acknowledge a MsgData send by the firmware
 */
struct MsgAckData
{
   struct MsgHeader Head;

   uint32_t Format;
   uint32_t Status;
};
/* ----------------------------- MsgChecker -----------------------------*/
/*
 * Give user space statictics comming with ip data analysis, so that it can be added to monitoring information
 * sent by the firmware.
 */
struct MsgChecker
{
   struct MsgHeader Head;

   uint32_t ItemId;
   uint32_t Total;
   uint32_t CcFailCnt;
   uint32_t ErrCnt;
};

/* ----------------------------- MsgHostBufFail -----------------------------*/
/*
 * Host indeicates emb that it cannot bufferize incoming data
 */
struct MsgHostBufFail
{
   struct MsgHeader Head;
};
/* ----------------------------- MsgClearHostIrq ------------------------------*/
/*
 * Ack firmware to clear host interface irq itself. This is sometimes not possible to
 * do it directly from host due to apb conflicts
 */
struct MsgClearHostIrq
{
   struct MsgHeader Head;
};

/* LAYOUT config id - used to trigger sensitive RESETs */





/* ---------------------------------- MsgData --------------------------------*/
/*
 * a 32 bit that contains specific information for data retreival
 */
#define     DataFormatBits 32
#define     DataFormatPackInit(s, ctx)  SerialBufRestart(ctx); DataFormatPack(s, ctx);
#define     DataFormatUnpackInit(ctx, s)  SerialBufRestart(ctx); DataFormatUnpack(ctx, s);
EXTERN void DataFormatPack(struct DataFormat *s, struct SerialBuf *ctx);
EXTERN void DataFormatUnpack(struct SerialBuf *ctx, struct DataFormat *s);

/* size of sipsi header for SIPSI delivery encapsulation */

/*  Format types */


/* ----------------------------- MsgData -----------------------------*/
/*
 * Used for HBM/BBM tranfers between host and chip
 */
#define     MsgDataBits 224
#define     MsgDataPackInit(s, ctx)  SerialBufRestart(ctx); MsgDataPack(s, ctx);
#define     MsgDataUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDataUnpack(ctx, s);
EXTERN void MsgDataPack(struct MsgData *s, struct SerialBuf *ctx);
EXTERN void MsgDataUnpack(struct SerialBuf *ctx, struct MsgData *s);

/* ----------------------------- MsgFrameInfo -----------------------------*/
/*
 * Table of info sent to the host for hbm mainly
 */
/*  Format for IN_MSG_FRAME_INFO (8 bits) */



#define     MsgFrameInfoBits 32
#define     MsgFrameInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgFrameInfoPack(s, ctx);
#define     MsgFrameInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgFrameInfoUnpack(ctx, s);
#define     MsgFrameInfoPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgFrameInfoUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* ----------------------------- MsgCtrlMonit -----------------------------*/
/*
 * Description : host requests to clear firmware monitoring info
 */
#define     MsgCtrlMonitBits 48
#define     MsgCtrlMonitPackInit(s, ctx)  SerialBufRestart(ctx); MsgCtrlMonitPack(s, ctx);
#define     MsgCtrlMonitUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCtrlMonitUnpack(ctx, s);
EXTERN void MsgCtrlMonitPack(struct MsgCtrlMonit *s, struct SerialBuf *ctx);
EXTERN void MsgCtrlMonitUnpack(struct SerialBuf *ctx, struct MsgCtrlMonit *s);


/* ------------------------------ MsgAcknowledgeApi ----------------------------*/

/* --------------------------------- MsgAckData --------------------------------*/
/*
 * Description : Host acknowledge a MsgData send by the firmware
 */
#define     MsgAckDataBits 96
#define     MsgAckDataPackInit(s, ctx)  SerialBufRestart(ctx); MsgAckDataPack(s, ctx);
#define     MsgAckDataUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAckDataUnpack(ctx, s);
EXTERN void MsgAckDataPack(struct MsgAckData *s, struct SerialBuf *ctx);
EXTERN void MsgAckDataUnpack(struct SerialBuf *ctx, struct MsgAckData *s);

/* ----------------------------- MsgChecker -----------------------------*/
/*
 * Give user space statictics comming with ip data analysis, so that it can be added to monitoring information
 * sent by the firmware.
 */
#define     MsgCheckerBits 160
#define     MsgCheckerPackInit(s, ctx)  SerialBufRestart(ctx); MsgCheckerPack(s, ctx);
#define     MsgCheckerUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCheckerUnpack(ctx, s);
EXTERN void MsgCheckerPack(struct MsgChecker *s, struct SerialBuf *ctx);
EXTERN void MsgCheckerUnpack(struct SerialBuf *ctx, struct MsgChecker *s);


/* ----------------------------- MsgHostBufFail -----------------------------*/
/*
 * Host indeicates emb that it cannot bufferize incoming data
 */
#define     MsgHostBufFailBits 32
#define     MsgHostBufFailPackInit(s, ctx)  SerialBufRestart(ctx); MsgHostBufFailPack(s, ctx);
#define     MsgHostBufFailUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHostBufFailUnpack(ctx, s);
#define     MsgHostBufFailPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgHostBufFailUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* ----------------------------- MsgClearHostIrq ------------------------------*/
/*
 * Ack firmware to clear host interface irq itself. This is sometimes not possible to
 * do it directly from host due to apb conflicts
 */
#define     MsgClearHostIrqBits 32
#define     MsgClearHostIrqPackInit(s, ctx)  SerialBufRestart(ctx); MsgClearHostIrqPack(s, ctx);
#define     MsgClearHostIrqUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgClearHostIrqUnpack(ctx, s);
#define     MsgClearHostIrqPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgClearHostIrqUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))


/* LAYOUT config id - used to trigger sensitive RESETs */

#endif /*__MSG_TYPE_DATA__*/


