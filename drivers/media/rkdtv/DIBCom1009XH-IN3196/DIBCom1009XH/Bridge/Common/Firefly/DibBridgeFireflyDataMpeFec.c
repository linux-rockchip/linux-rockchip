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
* @file "DibBridgeFireflyDataMpeFec.c"
* @brief Firefly specific bridge functionnality.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridgeTarget.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeTestIf.h"
#include "DibBridge.h"
#include "DibBridgeFirefly.h"
#include "DibBridgeFireflyTest.h"
#include "DibBridgeFireflyRegisters.h"

#if (DIB_MPEFEC_DATA == 1)
/**
 * getting CPT from a message. The CPT is filled in pContext->ItSvc[svc].
 * @param[in] pContext: Bridge context
 * @param[in] pDataMsg: Firefly FRAME_INFO message
 */
void DibBridgeFireflyGetCpt(struct DibBridgeContext *pContext, uint16_t * pDataMsg)
{
   uint8_t *pColPresTable;
   uint8_t  Svc, ItemHdl, i, j, dump;

   dump = pDataMsg[1] & 0x80;
   Svc  = pDataMsg[1] & 0x07;

   ItemHdl=pContext->FireflyInfo[Svc].ItemIndex;

   pContext->ItSvc[ItemHdl].SkipC = pDataMsg[1] >> 8;
   pColPresTable = pContext->ItSvc[ItemHdl].Cpt;
   j = 2;

   for(i = 0; i < 32; i += 2) 
   {
      pColPresTable[i]     = pDataMsg[j] & 0xff;
      pColPresTable[i + 1] = pDataMsg[j] >> 8;
      j++;
   }
}

/******************************************************************************
* Assemble the slice from MpeBufCor to SliceBuf, and set SkipR and SKipC if not already set
******************************************************************************/
uint32_t DibBridgeFireflyAssembleSlice(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint32_t offs, uint32_t len)
{
   uint8_t *src, *dst, *dst_2, *Cpt;
   uint32_t SkipR;
   ELEM_HDL ItemHdl = pFlags->ItemHdl;
   uint8_t  SkipC, i;

   SkipC = pContext->ItSvc[ItemHdl].SkipC;
   src   = pContext->ItSvc[ItemHdl].MpeBufCor + offs;
   dst   = pContext->SliceBuf;
   dst_2 = dst + (SkipC * len);
   Cpt   = pContext->ItSvc[ItemHdl].Cpt;

   if(pContext->ItSvc[ItemHdl].SkipR == 0) 
   {
      pContext->ItSvc[ItemHdl].SkipR = (uint16_t)(offs + len);
   }

   SkipR = pContext->ItSvc[ItemHdl].SkipR;

   /* Build slice Buffer with faulty Data */
   i = 0;

   while(i < 255) 
   {
      if(i == SkipC) 
      {
         dst = dst_2;
         i = 191;
      }

      if(Cpt[i >> 3] & (1 << (i & 7))) 
      {
         /* Payload exists */
         DibMoveMemory(dst, src, len);
         src += SkipR;
      }

      dst += len;
      i++;
   }
   /* return the number of columns to transfer (255 cause slice is not compressed) */
   return 255;
}


/**
 * Send profiler info to the RISC
 */
DIBSTATUS DibBridgeFireflyHbmProfiler(struct DibBridgeContext *pContext, uint8_t idx, uint8_t page, uint8_t LastFrag)
{
#if (DIB_BRIDGE_HBM_PROFILER == 1)
   uint16_t    msg[HBM_PROF_MSG_SIZE];
   uint8_t     SvcNb;
   DIBSTATUS   rc = DIBSTATUS_ERROR;

   rc = DibBridgeFireflyGetService(pContext, idx, &SvcNb);
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);

   msg[0] = (25<<8) +2;       /* FF25 */
   msg[1] = (SvcNb<<8) | page;

   if(LastFrag) 
   {
      msg[1] |= 0xF000;
   }

   return DibBridgeFireflySendMsg(pContext, (uint32_t*)msg, HBM_PROF_MSG_SIZE << 1);
#else
   return DIBSTATUS_SUCCESS;
#endif
}

/****************************************************************************
* Checks message coming from the RISC and acts appropriately
****************************************************************************/
DIBSTATUS DibBridgeFireflySendAck(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint8_t failed)
{
   uint16_t    msg[2];
   uint8_t svc;
   DIBSTATUS rc  = DIBSTATUS_ERROR;

   /** Convert Item */ 
   rc = DibBridgeFireflyGetService(pContext, pFlags->ItemHdl, &svc);
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);

   msg[0] = (OUT_MSG_HBM_ACK << 8) | 2;
   msg[1] = ((failed & 0x1) << 15) | (svc << 12) | pContext->ItSvc[pFlags->ItemHdl].FecOffset;

   return DibBridgeFireflySendMsg(pContext, (uint32_t*)msg, 1 << 2);
}

#endif


#endif /* USE_FIREFLY */
