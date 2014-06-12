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

#ifndef __MSG_TYPE_CAS__
#define __MSG_TYPE_CAS__

#ifdef __cplusplus
#define EXTERN  extern "C"
#else
#define EXTERN
#endif


/* Max message length */
#define MAX_MSG_CAS_BITS               2100

/* Input message identifiers */
#define IN_MSG_SC_STARTSTOP            1
#define IN_MSG_SC_APDU                 2
#define MAX_MSG_CAS_IN                 3

/* Output message identifiers */
#define OUT_MSG_SC_STARTSTOP           1
#define OUT_MSG_SC_APDU                2
#define MAX_MSG_CAS_OUT                3




#include "DibMsgTypes.h"
#include "DibMsgGeneric.h"
#include "DibMsgFields.h"
#include "DibMsgLayoutIf.h"
#include "DibStruct.h"
#include "DibExtAPI.h"


/*! \file DibMsgCas.h */

/*! \brief Message to Forward apdu cmd to and response from the chip
 * that contains the SmartCard
 */

/* Message to send a request start cas and receive the acknowledge
 * when Initiator = Destinator
 */

struct MsgScStartStop
{
   struct MsgHeader Head;
   uint8_t Status;
   uint8_t DoStart;
   uint8_t Initiator;
};

/* Message to send an APDU request and receive the answer
 * when Initiator = Destinator
 */

struct MsgScApdu
{
   struct MsgHeader Head;
   uint8_t Status;
   uint8_t Initiator;
   uint16_t Length;
   uint8_t Data[255];
};





/*! \file DibMsgCas.h */

/*! \brief Message to Forward apdu cmd to and response from the chip
 * that contains the SmartCard
 */

/* Message to send a request start cas and receive the acknowledge
 * when Initiator = Destinator
 */

#define     MsgScStartStopBits 45
#define     MsgScStartStopPackInit(s, ctx)  SerialBufRestart(ctx); MsgScStartStopPack(s, ctx);
#define     MsgScStartStopUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgScStartStopUnpack(ctx, s);
EXTERN void MsgScStartStopPack(struct MsgScStartStop *s, struct SerialBuf *ctx);
EXTERN void MsgScStartStopUnpack(struct SerialBuf *ctx, struct MsgScStartStop *s);


/* Message to send an APDU request and receive the answer
 * when Initiator = Destinator
 */

#define     MsgScApduBits 2100
#define     MsgScApduPackInit(s, ctx)  SerialBufRestart(ctx); MsgScApduPack(s, ctx);
#define     MsgScApduUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgScApduUnpack(ctx, s);
EXTERN void MsgScApduPack(struct MsgScApdu *s, struct SerialBuf *ctx);
EXTERN void MsgScApduUnpack(struct SerialBuf *ctx, struct MsgScApdu *s);


#endif /*__MSG_TYPE_CAS__*/


