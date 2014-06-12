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




#include "DibMsgGeneric.h"

/**
 Header : first struct of all messages
 @param MsgSize Size in 32bits words of the message
 @param ChipId  Chip identification : HOST_IDENT for host, MASTER_IDENT for master, i for ith slave
 @param MsgId   Message identification associated to this "Type" of message
 @param Type    Type of message
 */

/* list of message types */

/* chipset identifiers */


void MsgHeaderPack(struct MsgHeader *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 12, s->MsgSize);
   SerialBufWriteField(ctx, 4, s->ChipId);
   SerialBufWriteField(ctx, 8, s->MsgId);
   SerialBufWriteField(ctx, 4, s->Sender);
   SerialBufWriteField(ctx, 4, s->Type);
}





/**
 Header : first struct of all messages
 @param MsgSize Size in 32bits words of the message
 @param ChipId  Chip identification : HOST_IDENT for host, MASTER_IDENT for master, i for ith slave
 @param MsgId   Message identification associated to this "Type" of message
 @param Type    Type of message
 */

/* list of message types */

/* chipset identifiers */


void MsgHeaderUnpack(struct SerialBuf *ctx, struct MsgHeader *s)
{
   s->MsgSize                     = SerialBufReadField(ctx, 12, 0);
   s->ChipId                      = SerialBufReadField(ctx, 4, 1);
   s->MsgId                       = SerialBufReadField(ctx, 8, 0);
   s->Sender                      = SerialBufReadField(ctx, 4, 1);
   s->Type                        = SerialBufReadField(ctx, 4, 0);
}


