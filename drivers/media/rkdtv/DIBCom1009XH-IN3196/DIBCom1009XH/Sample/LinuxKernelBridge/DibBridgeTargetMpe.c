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

/**************************************************************************************************
* @file "DibBridgeTargetMpe.c"
* @brief Target specific implementation.
*
***************************************************************************************************/
#include "DibBridgeCommon.h"
#include "DibBridgeTargetMpe.h"
#include "DibBridgeTargetNet.h"

/**
 *  Parse MPE frame and submit IP Packets to the IP stack
 */
void DibBridgeTargetParseMpeFrame(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size)
{
   uint8_t *pdata, *pCurBuf;
   uint32_t packets, error, MpeSize;

   packets = 0;
   error   = 0;
   pdata   = pBuf;
   pCurBuf = pdata;
   MpeSize = Size;

   while(pCurBuf + 6 < pdata + MpeSize) 
   {
      uint32_t Size;

      /*** End of MPE frame is reached, exit ****/
      if(*pCurBuf == 0x00) 
      {
         pCurBuf++;
         continue;
      }

      /**** IPV6 ****/
      if((*pCurBuf & 0xf0) == 0x60) 
      {
         /*** Read IP packet Size ***/
         Size = 40 + (pCurBuf[4] << 8) + pCurBuf[5];
      }
      /**** IPV4 ****/
      else if((*pCurBuf & 0xf0) == 0x40) 
      {
         /*** Read IP packet Size ***/
         Size = (pCurBuf[2] << 8) + pCurBuf[3];
      }
      else 
      {
         /*** Unknown Format: exit ****/
         DIB_DEBUG(PORT_ERR, (CRB "dibbridge MPE frame Parsing: Not IPV4 or IPV6 Header at Offset %d" CRA, (int32_t)(pCurBuf - pdata)));
         error = 1;
         break;
      }

      if(Size == 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "dibbridge MPE frame parse: null Size found!" CRA));
         error = 1;
         break;
      }

      /**** Submit IP Packet ***/
      if(pCurBuf + Size <= pdata + MpeSize) 
      {
         DibBridgeTargetNetSubmitPacket(Size, pCurBuf);
      }

      pCurBuf += Size;
      packets++;

   }

   if(error) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "dibbridge MPE parsing failed:  error after %d packets" CRA, packets));
   }
}
