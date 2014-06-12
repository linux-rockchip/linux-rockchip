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

#include "DibMsgFields.h"

#define  ___SIGN_BIT___(size)                 (1 << ((size)-1))
#define  ___SIGN_EXT___(size)                 (((size) == 32) ? 0 : (0xFFFFFFFF << (size)))
#define  ___DATA_MASK___(size)                (((size) == 32) ? 0xFFFFFFFF : (uint32_t)((1 << (size)) - 1))

static void SerialBufWriteWord(struct SerialBuf * ctx, uint32_t value, uint32_t j)
{
   if(ctx->w == 32)
      ((uint32_t *)ctx->buf)[j] = value;
   else if(ctx->w == 16)
      ((uint16_t *)ctx->buf)[j] = value;
   else if(ctx->w == 8)
      ((uint8_t *)ctx->buf)[j] = value;
}

static uint32_t SerialBufReadWord(struct SerialBuf * ctx, uint32_t j)
{
   uint32_t value = 0;
   if(ctx->w == 32)
      value = ((uint32_t *)ctx->buf)[j];
   else if(ctx->w == 16)
      value = ((uint16_t *)ctx->buf)[j];
   else if(ctx->w == 8)
      value = ((uint8_t *)ctx->buf)[j];
   return value;
}


void SerialBufInit(struct SerialBuf * ctx, void * buf, uint32_t w)
{
   ctx->buf = buf,
   ctx->w = w;
   ctx->i = 0;
   ctx->j = 0;
}

void SerialBufRestart(struct SerialBuf * ctx)
{
   ctx->i = 0;
   ctx->j = 0;
}

#if 0
void SerialBufAlign(struct SerialBuf * ctx, uint32_t align)
{
   if(ctx->i > 0)
   {
      ctx->i = 0;
      ctx->j = ctx->j + 1;
   }
}
#endif

void SerialBufWriteField(struct SerialBuf * ctx, uint32_t s, int32_t v)
{
   uint32_t i, j, m, k, w;

   /* load context */
   i = ctx->i;
   j = ctx->j;
   w = ctx->w;

   /* printf("SerialBufWriteField: buf %p offset %d size %d value %08x\n", buf, i+j*w, s, v); */

   while(s > 0)
   {
      if((i + s) > w)
         k = w - i;
      else
         k = s;

      m = ___DATA_MASK___(k);

      /* printf("-> off %d write %08x\n", i+j*w, (v & m)); */

      SerialBufWriteWord(ctx, (SerialBufReadWord(ctx, j) & ~(m << i)) | ((v & m) << i), j);
      s =  s - k;

      i += k;

      if(i >= w)
      {
         j = j + 1;
         i -= w;
      }

      v = v >> k;
   }

   /* restore context */
   ctx->i = i;
   ctx->j = j;
}

/*
 *  inside buf, get a field value, eventually extend ts sign if needed
 */
int32_t SerialBufReadField(struct SerialBuf * ctx, uint32_t s, uint32_t z)
{
   uint32_t i, j, m, k, w, t;
   int32_t  v;

   /* load context */
   i = ctx->i;
   j = ctx->j;
   w = ctx->w;

   t = s;
   v = 0;

   while(s > 0)
   {
      if(i + s > w)
         k = w - i;
      else
         k = s;

      m = ___DATA_MASK___(k);

      /* printf("-> off %d read %08x\n", i+j*w, ((buf[j] >> i) & m)); */

      v = v | (((SerialBufReadWord(ctx, j) >> i) & m) << (t-s));

      s =  s - k;

      i += k;

      if(i >= w)
      {
         j = j + 1;
         i -= w;
      }
   }

   /* sign extension */
   if(z && (___SIGN_BIT___(t) & v))
      v |= ___SIGN_EXT___(t);

   /* printf("SerialBufReadField: offset %d size %d value %08x\n", i+j*w, t, v); */

   /* restore context */
   ctx->i = i;
   ctx->j = j;

   return v;
}

