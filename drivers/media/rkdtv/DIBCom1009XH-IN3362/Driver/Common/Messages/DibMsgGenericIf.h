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

#ifndef MSG_GENERIC_IF_H
#define MSG_GENERIC_IF_H

/* Fast macro to look inside a 32 bit formatted buffer */
#define GET_MSG_SIZE(Header)         ((uint16_t)  ((Header) & 0x00000FFF))
#define GET_MSG_CHIP(Header)         ((int8_t)   (((Header) & 0x0000F000) >> 12))
#define GET_MSG_ID(Header)           ((uint16_t) (((Header) & 0x00FF0000) >> 16))
#define GET_MSG_SENDER(Header)       ((int8_t)   (((Header) & 0x0F000000) >> 24))
#define GET_MSG_TYPE_ID(Header)      ((int8_t)   (((Header) & 0xF0000000) >> 28))


#define SET_MSG_HEADER(MsgId, Size)  (((MsgId) << 16) | (Size))

#endif
