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
* @file "DtaParsing.c"
* @brief DiBcom Test Application - Parsing functions.
*******************************************************************************/

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaCrc32.h"
#include "DtaCmds.h"
#include "DtaParsing.h"

#if (DIB_CMMB_STD == 1)

extern struct DibDriverContext *gUserContext;

#define CONTEXT (gUserContext)

/**
 * BitsExtract
 */
uint32_t BitsExtract(tWordStream *Stream, int8_t BitCount)
{
   if(BitCount > 32)
      return 0;

   uint32_t Value;
   int8_t AvailBits = Stream->Msb + 1;
   int8_t RemainBits = AvailBits - BitCount;

   /* Case no need to read new word to output value. */
   if(RemainBits >= 0)
   {
      /* Extract value from stream current word. */
      Value = (Stream->Data >> (AvailBits-BitCount)) & ((uint32_t)0xffffffff>>(32-BitCount));
      Stream->Msb -= BitCount;

      /* Case no more data. Read new word for next extraction. */
      if(Stream->Msb == -1)
         STREAM_READ(Stream);
   }
   /* Value straddles two words. */
   else
   {
      /* Most significant bits part. */
      Value = (Stream->Data) & ((uint32_t)0xffffffff>>(32-AvailBits));
      Value = Value << (-RemainBits);
      BitCount -= AvailBits;

      /* Need new word. */
      STREAM_READ(Stream);

      /* Rest of it. */
      Value |= (Stream->Data >> (32-BitCount)) & ((uint32_t)0xffffffff>>(32-BitCount));
      Stream->Msb -= BitCount;

      /* Output value is less or equal to 32 bit long. We are sure
       * that current word has available bits left. */
   }

   return Value;
}

/**
 * BitsOffset
 */
void BitsOffset(tWordStream *Stream, uint32_t BitCount)
{
   /* Compute (BitCount % 32) and (BitCount / 32). */
   uint32_t RemainBits = BitCount & 31; /* BitCount % 32  */
   uint32_t NbWords = BitCount >> 5;    /* BitCount / 32 */

   /* Offset stream by number of words. */
   STREAM_OFFS(Stream, NbWords);

   /* Case remainder is strictly less than available bits. */
   uint8_t AvailBits = Stream->Msb + 1;
   if(AvailBits > RemainBits)
   {
      Stream->Msb -= RemainBits;
   }
   /* A last offset is necessary. */
   else
   {
      STREAM_OFFS(Stream, 1);
      Stream->Msb = 31 - (RemainBits - AvailBits);
   }
}

/**
 * Crc32Compute
 * (Crc32Compute() ^ Crc32) should read zero.
 */
uint32_t Crc32Compute(uint8_t *Data, uint32_t Size)
{
   if((Data == 0) || (Size == 0))
      return 0;

   uint32_t result;
   uint32_t i, j;
   uint8_t  byte;

   result = 0xffffffff;

   for(i = 0; i < Size; i++)
   {
      byte = Data[i];
      for(j = 0; j < 8; j++)
      {
         if((byte >> 7) ^ (result >> 31))
         {
            result = (result << 1) ^ (uint32_t)0x04C11DB7;
         }
         else
         {
            result = (result << 1);
         }
         byte <<= 1;
      }
   }

   return result;
}

#endif

