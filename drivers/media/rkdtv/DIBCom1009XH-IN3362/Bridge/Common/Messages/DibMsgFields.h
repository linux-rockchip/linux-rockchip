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

#ifndef DIB_MSG_FIELDS_H
#define DIB_MSG_FIELDS_H

#include "DibMsgTypes.h"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

struct SerialBuf
{
   void   * buf;
   uint32_t w;
   uint32_t i;
   uint32_t j;
};

#define  GetWords(bits, width)           ((bits) + (width) - 1) / (width)

EXTERN void     SerialBufInit       (struct SerialBuf * ctx, void * buf, uint32_t width);
EXTERN void     SerialBufRestart    (struct SerialBuf * ctx);
EXTERN void     SerialBufAlign      (struct SerialBuf * ctx, uint32_t align);
EXTERN void     SerialBufWriteField (struct SerialBuf * ctx, uint32_t s, int32_t v);
EXTERN int32_t  SerialBufReadField  (struct SerialBuf * ctx, uint32_t s, uint32_t z);

#endif /* DIB_MSG_FIELDS_H */

