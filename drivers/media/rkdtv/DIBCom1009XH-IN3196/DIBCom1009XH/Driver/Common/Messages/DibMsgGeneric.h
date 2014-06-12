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

#ifndef __MSG_TYPE_GEN__
#define __MSG_TYPE_GEN__

#ifdef __cplusplus
#define EXTERN  extern "C"
#else
#define EXTERN
#endif


/* Max message length */
#define MAX_MSG_GEN_BITS               32

/* Input message identifiers */
#define MAX_MSG_GEN_IN                 1

/* Output message identifiers */
#define MAX_MSG_GEN_OUT                1



#include "DibMsgTypes.h"
#include "DibMsgFields.h"
#include "DibMsgGenericIf.h"


/**
 Header : first struct of all messages
 @param MsgSize Size in 32bits words of the message
 @param ChipId  Chip identification : HOST_IDENT for host, MASTER_IDENT for master, i for ith slave
 @param MsgId   Message identification associated to this "Type" of message
 @param Type    Type of message
 */

/* list of message types */
#define MSG_TYPE_MAC                   0
#define MSG_TYPE_DEBUG                 1
#define MSG_TYPE_LAYOUT                2
#define MSG_TYPE_CAS                   3

/* chipset identifiers */
#define HOST_IDENT                     -1
#define MASTER_IDENT                   0


struct MsgHeader
{
   uint16_t MsgSize;
   int8_t ChipId;
   uint16_t MsgId;
   int8_t Sender;
   uint8_t Type;
};




/**
 Header : first struct of all messages
 @param MsgSize Size in 32bits words of the message
 @param ChipId  Chip identification : HOST_IDENT for host, MASTER_IDENT for master, i for ith slave
 @param MsgId   Message identification associated to this "Type" of message
 @param Type    Type of message
 */

/* list of message types */

/* chipset identifiers */


#define     MsgHeaderBits 32
#define     MsgHeaderPackInit(s, ctx)  SerialBufRestart(ctx); MsgHeaderPack(s, ctx);
#define     MsgHeaderUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHeaderUnpack(ctx, s);
EXTERN void MsgHeaderPack(struct MsgHeader *s, struct SerialBuf *ctx);
EXTERN void MsgHeaderUnpack(struct SerialBuf *ctx, struct MsgHeader *s);


#endif /*__MSG_TYPE_GEN__*/


