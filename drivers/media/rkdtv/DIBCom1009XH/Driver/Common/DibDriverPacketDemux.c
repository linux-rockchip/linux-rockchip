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
* @file "DibDriverPacketDemux.c"
* @brief .
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverConstants.h"
#include "DibDriverTargetTypes.h"
#include "DibDriverCommon.h"
#include "DibDriverTargetDebug.h"
#include "DibDriverMessages.h"
#include "DibDriverPacketDemux.h"
#include "DibDriverIf.h"


#if (mSDK == 0)

/**
 * Process of the data that comes from the chip with a filter in a "call back" mode
 * @param pContext pointer to the context
 * @param dataInfo structure (very small) with information useful to retrieve the data from the bridge
 */
void DibDriverRawDataRead(struct DibDriverContext * pContext, struct DibDriverDataInfoUp dataInfo)
{
   DIBSTATUS  status    = DIBSTATUS_ERROR;
   uint32_t   Length    = dataInfo.Length;
   ELEM_HDL   ItemHdl   = dataInfo.ItemIndex;
   uint8_t    FirstFrag = (dataInfo.Flags & 0x02) >> 1;
   uint8_t    LastFrag  = (dataInfo.Flags & 0x01);
   FILTER_HDL FilterHdl;
   struct DibBufContext BuffCtx;
   uint8_t GoToNextBuf = 1, Idx;

#if DIB_INTERNAL_MONITORING == 1
   /* TODO
    *
    * if (dataInfo.target != -1) {
    *    DibDriverTargetDataRead(pContext, BuffCtx.BufAdd, &Length, FilterHdl);
    *    DibDriverDebugRawDataDispatcher(pContext, &dataInfo, )
    *    to be completed
    *    use Filter?
    * }
    */
#endif

   FilterHdl = pContext->ItemInfo[ItemHdl].ParentFilter;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter != eUNKNOWN_DATA_TYPE);   /** we are sure filter is in use */

   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverRawDataRead %d" CRA, FilterHdl));

   if(pContext->FilterInfo[FilterHdl].FlushActive == eDIB_TRUE)
   {
      DibDriverTargetDataFlush(pContext, FilterHdl, Length);
      return;
   }
   /* Get user's buffer info */
   if(!(LastFrag)
      &&(pContext->FilterInfo[FilterHdl].TypeFilter == ePESVIDEO
      || pContext->FilterInfo[FilterHdl].TypeFilter == ePESAUDIO
      || pContext->FilterInfo[FilterHdl].TypeFilter == ePESOTHER))
   {
      /* if it is a PES segment, we keep the same buffer until the last fragment */
      GoToNextBuf = 0;
   }
   status = DibDriverGetNewBuffer(pContext, FilterHdl, &BuffCtx, GoToNextBuf);

   if(status == DIBSTATUS_SUCCESS)
   {
      if(Length > BuffCtx.BufSize - BuffCtx.FillSize)
      {
         DIB_DEBUG(PACKET_ERR, (CRB "" CRA));
         DIB_DEBUG(PACKET_ERR, (CRB "DibDriverRawDataRead: Size Buffer too large: %d bytes (over %d)" CRA, Length, BuffCtx.BufSize));
         Length = BuffCtx.BufSize - BuffCtx.FillSize;
      }
      DIB_DEBUG(PACKET_LOG, (CRB "" CRA));
      DIB_DEBUG(PACKET_LOG, (CRB "DibDriverRawDataRead: Size Buffer : %d bytes" CRA, Length));

      /* Read buffer from bridge and callback */
      if((FirstFrag && LastFrag)
         ||(pContext->FilterInfo[FilterHdl].TypeFilter != ePESVIDEO
         && pContext->FilterInfo[FilterHdl].TypeFilter != ePESAUDIO
         && pContext->FilterInfo[FilterHdl].TypeFilter != ePESOTHER))
      {
         DibDriverTargetDataRead(pContext, BuffCtx.BufAdd, &Length, FilterHdl);
         BuffCtx.BufSize = Length;
         BuffCtx.ItemHdl = ItemHdl;
         DibDriverTargetEnableIrqProcessing(pContext);
         DibDriverDataCallback(pContext, &BuffCtx, eTIMEOUT);
         DibDriverTargetDisableIrqProcessing(pContext);
      }
      /* if PES fragment, reconstruct PES section before callback */
      else
      {
         DibDriverTargetDataRead(pContext, BuffCtx.BufAdd + BuffCtx.FillSize, &Length, FilterHdl);
         BuffCtx.FillSize += Length;
         if(LastFrag)
         {
            BuffCtx.BufSize = BuffCtx.FillSize;
            BuffCtx.ItemHdl = ItemHdl;
            DibDriverTargetEnableIrqProcessing(pContext);
            DibDriverDataCallback(pContext, &BuffCtx, eTIMEOUT);
            DibDriverTargetDisableIrqProcessing(pContext);
            BuffCtx.FillSize = 0;
         }
         Idx = pContext->FilterInfo[FilterHdl].FirstBuf;
         pContext->CallbackBufCtx[Idx].Ctx.FillSize = BuffCtx.FillSize;
      }
   }
   else
   {
      DibDriverTargetDataFlush(pContext, FilterHdl, Length);
      DIB_DEBUG(PACKET_ERR, (CRB "" CRA));
      DIB_DEBUG(PACKET_ERR, (CRB "DibDriverGetNewBuffer ERROR " CRA));
   }
}
#endif

