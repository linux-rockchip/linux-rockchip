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

/*******************************************************************************
* @file "DtaParsing.h"
* @brief DiBcom Test Application - Parsing functions.
*******************************************************************************/

#ifndef DTA_PARSING_H
#define DTA_PARSING_H

#include "DibExtAPI.h"

#if (DIB_CMMB_STD == 1)

typedef struct WordStream
{
   uint8_t *Buffer;
   uint32_t Address;
   uint32_t Data;
   int8_t  Msb;
}
tWordStream;

#define READW(_stm_) ( \
   (((uint32_t)((_stm_)->Buffer[(_stm_)->Address]    )) << 24) + \
   (((uint32_t)((_stm_)->Buffer[(_stm_)->Address + 1])) << 16) + \
   (((uint32_t)((_stm_)->Buffer[(_stm_)->Address + 2])) <<  8) + \
   (((uint32_t)((_stm_)->Buffer[(_stm_)->Address + 3])) <<  0))

#define STREAM_INIT(_stm_, _buf_, _add_) do { \
   (_stm_)->Buffer = (_buf_); \
   (_stm_)->Address = (_add_); \
   (_stm_)->Data = READW(_stm_); \
   (_stm_)->Msb = 31; \
} while(0)

#define STREAM_READ(_stm_) do { \
   (_stm_)->Address += 4; \
   (_stm_)->Data = READW(_stm_); \
   (_stm_)->Msb = 31; \
} while(0)

#define STREAM_OFFS(_stm_, _nbw_) do { \
   (_stm_)->Address += 4*(_nbw_); \
   (_stm_)->Data = READW(_stm_); \
} while(0)

#define STREAM_GETPTR(_stm_) \
   ((uint32_t)((_stm_)->Address + ((31 - (_stm_)->Msb)/8)))


#define STREAM_SETPTR(_stm_, _add_) do {} while(0)


#define STREAM_PRINT(_stm_) do { \
   printf("@=0x%08x D=0x%08x msb=%d\n", (_stm_)->Address, (_stm_)->Data, (_stm_)->Msb); \
} while(0)

uint32_t BitsExtract(tWordStream *Stream, int8_t BitCount);
void BitsOffset(tWordStream *Stream, uint32_t BitCount);
uint32_t Crc32Compute(uint8_t *Data, uint32_t Size);

#endif

#endif /* !DTA_PARSING_H */
