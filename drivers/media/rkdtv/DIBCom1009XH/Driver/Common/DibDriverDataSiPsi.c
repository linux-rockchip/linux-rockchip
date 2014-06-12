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
* @file "DibDriverIf.c"
* @brief Driver Interface.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriver.h"

#if (DIB_SIPSI_DATA == 1)

/**
 * Check SiPsi Parameters
 * @param[in]  pContext: driver context
 * @param[in]  pFilterDesc: filter descripter handler.
 * @param[out] Pid: Pointer to output Pid for filter coherency check
 */
DIBSTATUS DibDriverCheckParamSiPsi(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if(pFilterDesc->SiPsi.Pid >= DIB_POSITIVE_PIDS)
      goto End;

   if((pFilterDesc->SiPsi.PidWatch != eDIB_TRUE) && (pFilterDesc->SiPsi.PidWatch != eDIB_FALSE))
      goto End;
   
   /* TBD : PidInfo.Specific.SiPsiSpecific.extendedPowerOnTime; */
   *Pid    = pFilterDesc->SiPsi.Pid;
    Status = DIBSTATUS_SUCCESS;

End:
   return Status;
}


/**
 * Runtime parameter changing. This function can do the following updates:
 * - Turn on/off a PID to/from prefetch mode
 * @param[in] pContext: driver context
 * @param[in] pParamConfig: Config.Parameter whom configuration need to be changed.
 */
DIBSTATUS DibDriverSetConfigSiPsi(struct DibDriverContext *pContext, union DibParamConfig * pParamConfig)
{
   FILTER_HDL FilterHdl;
   DIBSTATUS  ret = DIBSTATUS_INVALID_PARAMETER;

   /** Check only if the ItemHdl exist and filter is SIPSI */
   if(pParamConfig->SipsiFilter.ItemHdl < DIB_MAX_NB_ITEMS)
   {
      /** Prevent driver from internal structure modification */
      DibDriverTargetDisableIrqProcessing(pContext);

      FilterHdl = pContext->ItemInfo[pParamConfig->SipsiFilter.ItemHdl].ParentFilter;
      DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);

      if(pContext->FilterInfo[FilterHdl].TypeFilter == eSIPSI)
      {
         struct DibSipsiFilterCfg *pSetFilter = &pParamConfig->SipsiFilter;
         struct DibSipsiFilterCfg *pFirstFilter;

         DIB_ASSERT(pSetFilter->ItemHdl < DIB_MAX_NB_ITEMS);

         pFirstFilter = pContext->ItemInfo[pSetFilter->ItemHdl].ItemCtx.pSipsiService->pFilterSipsi;

         if(pSetFilter->Enable)
         {
            /** the filter is added at the head */
            pContext->ItemInfo[pSetFilter->ItemHdl].ItemCtx.pSipsiService->pFilterSipsi = (struct DibSipsiFilterCfg *)DibMemAlloc(sizeof(struct DibSipsiFilterCfg));
            DIB_DEBUG(PACKET_LOG, (CRB "Setup Sipsi Filter of item %d" CRA,pSetFilter->ItemHdl));

            /** copy the structure entirely */
            DibMoveMemory(pContext->ItemInfo[pSetFilter->ItemHdl].ItemCtx.pSipsiService->pFilterSipsi, pSetFilter, sizeof(struct DibSipsiFilterCfg));

            pContext->ItemInfo[pSetFilter->ItemHdl].ItemCtx.pSipsiService->pFilterSipsi->pNext = pFirstFilter;

         }
         else if(pFirstFilter)
         {
            struct DibSipsiFilterCfg *pNextFilter;
            do
            {
               pNextFilter = pFirstFilter->pNext;
               DibMemFree(pFirstFilter, sizeof(struct DibSipsiFilterCfg));
               pFirstFilter = pNextFilter;
            } 
            while(pFirstFilter);

            DIB_DEBUG(PACKET_LOG, (CRB "Free Sipsi Filter of item %d" CRA,pSetFilter->ItemHdl));
            pContext->ItemInfo[pSetFilter->ItemHdl].ItemCtx.pSipsiService->pFilterSipsi = NULL;
         }
         ret = DIBSTATUS_SUCCESS;
      }
      else
      {
         ret = DIBSTATUS_INVALID_PARAMETER;
      }

      DibDriverTargetEnableIrqProcessing(pContext);
   }
   else
   {
      ret = DIBSTATUS_INVALID_PARAMETER;
   }

   return ret;
}


/**
 * Query info about specific settings. This function can do the following updates:
 * @param[in] pContext: driver context
 * @param[in] pParamConfig: Config.Parameter whom configuration need to be changed.
 */
DIBSTATUS DibDriverGetConfigSiPsi(struct DibDriverContext *pContext, union DibParamConfig *pParamConfig)
{
   FILTER_HDL FilterHdl;
   DIBSTATUS  ret = DIBSTATUS_INVALID_PARAMETER;

   /** Check only if the ItemHdl exist and filter is SIPSI */
   if(pParamConfig->SipsiFilter.ItemHdl < DIB_MAX_NB_ITEMS)
   {
      /** Prevent driver from internal structure modification */
      DibDriverTargetDisableIrqProcessing(pContext);

      FilterHdl = pContext->ItemInfo[pParamConfig->SipsiFilter.ItemHdl].ParentFilter;
      DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);

      if(pContext->FilterInfo[FilterHdl].TypeFilter == eSIPSI)
      {
         if(pContext->ItemInfo[pParamConfig->SipsiFilter.ItemHdl].ItemCtx.pSipsiService->pFilterSipsi)
         {
            DibMoveMemory(&pParamConfig->SipsiFilter, pContext->ItemInfo[pParamConfig->SipsiFilter.ItemHdl].ItemCtx.pSipsiService->pFilterSipsi, sizeof(struct DibSipsiFilterCfg));
            ret = DIBSTATUS_SUCCESS;
         }
      }
      else
      {
         ret = DIBSTATUS_INVALID_PARAMETER;
      }

      DibDriverTargetEnableIrqProcessing(pContext);
   }
   else
   {
      ret = DIBSTATUS_INVALID_PARAMETER;
   }

   return ret;
}

/**
 * @param pContext: driver context structure
 * @param ItemHdl: item handler associated with this sipsi section
 * @param pBuf: the sipsi section with its header
 * @return: uint32_t 0 is pass, 1 if filtered
 */
static uint32_t IntDriverFilterSectionHeader(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, uint8_t * pBuf)
{
   struct DibDriverSipsiServiceCtx *pSipsi = pContext->ItemInfo[ItemHdl].ItemCtx.pSipsiService;
   struct DibSipsiFilterCfg *pFilter;
   uint32_t Filtered = 0;
   uint32_t i, XorIsZero;

   /** Aliased Sipsi services can have pSipsi set to zero */
   if(!pSipsi)
      return 0;

   pFilter = pSipsi->pFilterSipsi;
   while(pFilter && (Filtered == 0))
   {
      /** Check if passed Equ value condition */ 
      for(i = 0; (i < DIB_SIPSI_HEADER_LENGTH) && (Filtered == 0); i++)
      {
         /** continue if Byte[i] == 0 */
         if(pFilter->BitMaskEqu.Byte[i] == 0) continue;

         /** section is filtered if the field is different from the value */
         if((pFilter->BitMaskEqu.Byte[i] & pBuf[i]) != (pFilter->BitMaskEqu.Byte[i] & pFilter->Value.Byte[i]))
         {
            Filtered = 1;
         }
      }

      /** Check if passed Xor value condition */ 
      if(Filtered == 0)
      {
         XorIsZero = 1;
         for(i = 0; i < DIB_SIPSI_HEADER_LENGTH; i++)
         {
            /** continue if Byte[i] == 0 */
            if(pFilter->BitMaskXor.Byte[i] == 0) continue;

            XorIsZero = 0;

            /** section pass if the field is different from the value */
            if((pFilter->BitMaskXor.Byte[i] & pBuf[i]) != (pFilter->BitMaskXor.Byte[i] & pFilter->Value.Byte[i]))
            {
               break;
            }
         }
         /** Filter the section if XorIsNotZero and field is different from the value */
         if((i == DIB_SIPSI_HEADER_LENGTH) && !XorIsZero)
         {
            Filtered = 1;
         }
      }
      pFilter = pFilter->pNext;
   }
   return Filtered;
}


uint8_t IntDriverComputeCrc32(void *pTable, uint32_t bufferLength)
{
   uint32_t crc32bis = 0xffffffff;

   crc32bis = IntDriverGetCrc32(pTable, bufferLength, 0xffffffff);

   if(crc32bis == 0)
      return 1;
   else
      return 0;
}

/**
 * When SIPSI section is complete, pass it to the USER using a callback and a USER Buffer
 * @param pContext       pointer to the context
 * @param ChannelHdl     index of the channel tuned
 * @param FilterHdl      index of the sipsi filter 
 * @param ItemHdl        index of the sipsi item 
 * @param Pid            sipsi Pid concerned
 * @param Data           pointer to data
 * @param Size           size of the buffer
 */
uint32_t IntDriverProcessSiPsiSection(struct DibDriverContext * pContext, CHANNEL_HDL ChannelHdl, FILTER_HDL FilterHdl, ELEM_HDL ItemHdl, uint16_t Pid, uint8_t * Data, uint32_t Size)
{
   struct DibBufContext BuffCtx;

   uint32_t CrcValid;
   DIBSTATUS            status = DIBSTATUS_ERROR;

   DIB_ASSERT(Data);

   /** Software calculation of crc is needed when using firefly due to possible overflow cases */
   if(pContext->ItemInfo[ItemHdl].Config.SiPsi.Crc)
   {
      CrcValid = IntDriverComputeCrc32(Data, Size);
      if(!CrcValid)
      {
#if (CHECK_SIPSI_CRC == 1)
         uint32_t i;
         printf(CRB "SIPSI CORRUPTED Size %d CRC %d" CRA, Size, CrcValid);
         for(i=0; i<Size; i++)
         {
            printf("%02x ",Data[i]);
            if((i % 32) == 31) printf(CRB "" CRA);
         }
#endif
         return 1;
      }
   }

   /** Filter unwanted sections */
   if(IntDriverFilterSectionHeader(pContext, ItemHdl, Data))
   {
      struct DibDriverEventInfoUp DataInfo;
      DataInfo.EventType = eSIPSI_FILTERED;
      DataInfo.EventConfig.SipsiFilterEvent.ItemHdl = ItemHdl;
      DibDriverPostEvent(pContext, &DataInfo);
      DIB_DEBUG(PACKET_LOG, (CRB " IntDriverFilterSectionHeader: section of Size %d filtered on item %d" CRA, Size, ItemHdl));
      return 0;
   }

   status = DibDriverGetNewBuffer(pContext, FilterHdl, &BuffCtx, 1);

   if(status == DIBSTATUS_SUCCESS)
   {
      /*  Add the section to the FIFO */
      if(Size > BuffCtx.BufSize)
      {
         DIB_DEBUG(PACKET_ERR, (CRB " IntDriverProcessSiPsiSection: section is too large: %d bytes" CRA, Size));
         Size = 0;
      }

      DibMoveMemory(BuffCtx.BufAdd, Data, Size);

      BuffCtx.BufSize = Size;
      BuffCtx.ItemHdl = ItemHdl;

      DibDriverTargetEnableIrqProcessing(pContext);
      DibDriverDataCallback(pContext, &BuffCtx, eTIMEOUT);
      DibDriverTargetDisableIrqProcessing(pContext);
   }
   else
   {
      struct DibDriverEventInfoUp DataInfo;
      DataInfo.EventType = eSIPSI_LOST;
      DataInfo.EventConfig.SipsiLostEvent.ItemHdl = ItemHdl;
      DibDriverPostEvent(pContext, &DataInfo);
      DIB_DEBUG(PACKET_LOG, (CRB "IntDriverProcessSiPsiSection Buffer allocation error for Item %d, status=%d" CRA, ItemHdl, status));
   }
   return 0;
}

#define  DibDriverGetSiPsiInfo(pContext, pPacketAddr, pPacketSize, pItemIdx, pStartSection, pEndSection, pCrcValid, pHeaderLen, pCrcLen) \
         pContext->ChipOps.GetSiPsiInfo(pContext, pPacketAddr, pPacketSize, pItemIdx, pStartSection, pEndSection, pCrcValid, pHeaderLen, pCrcLen)

/**
 * Rebuilds SI/PSI sections from firefly formatted packets.
 * @param pContext pointer to the context
 * @param Size     size of the buffer from the bridge
 * @param pBuf     pointer to the beginning of this buffer
 */
void DibDriverProcessSiPsiBuffer(struct DibDriverContext * pContext, uint32_t Size, uint8_t *pBuf)
{
   int32_t               PacketSize;
   uint32_t              ItemHdl, FilterHdl, ChannelHdl;
   uint8_t              *pCurrentPacket;
   uint8_t               StartSection;
   uint8_t               EndSection;
   uint8_t               CrcValid;
   uint8_t               HeaderLen;
   uint8_t               CrcLen;
   DIBSTATUS             status = DIBSTATUS_ERROR;
   struct DibDriverItem *pItem;

#if (CHECK_SIPSI_CRC == 1)
   uint32_t NbPackets = 0, NbSipsi = 0;
#endif

   /* BRIDGE */
   pCurrentPacket = pBuf;
   
   DIB_ASSERT(pBuf);

   do
   {
      /* retreive the item index depending on  */
      status = DibDriverGetSiPsiInfo(pContext, pCurrentPacket, &PacketSize, &ItemHdl, &StartSection, &EndSection, &CrcValid, &HeaderLen, &CrcLen);

      /** retreive filter index and channel index. Warning, it is possible that the chip add stored obsolete section when
       * we removed a service so we have to filter unfresh services. TODO: pass a fresh service flag in section header. */
      if(status == DIBSTATUS_SUCCESS)
      {
         FilterHdl = pContext->ItemInfo[ItemHdl].ParentFilter;
   
         if((FilterHdl >= DIB_MAX_NB_FILTERS) || (pContext->FilterInfo[FilterHdl].TypeFilter != eSIPSI))
         {
            status = DIBSTATUS_ERROR;
         }
         else
         {
            ChannelHdl = pContext->FilterInfo[FilterHdl].ParentChannel;
          
            if((ChannelHdl >= DIB_MAX_NB_CHANNELS) || !(pContext->ChannelInfo[ChannelHdl].InUse))
               status = DIBSTATUS_ERROR;
         }
      }

      if(status == DIBSTATUS_SUCCESS)
      {
         pItem = &pContext->ItemInfo[ItemHdl];

#if (CHECK_SIPSI_CRC == 1)
         NbPackets++;
#endif

         if((pContext->FilterInfo[FilterHdl].TypeFilter != eSIPSI) || (pItem->ItemCtx.pSipsiService == NULL) || (pItem->ItemCtx.pSipsiService->Table == NULL))
         {
            DIB_DEBUG(PACKET_LOG, (CRB " DibDriverProcessSiPsiBuffer: Filter %d Error, ItemHdl is %d Type is %d, pSipsiSvc is %p!!" CRA, FilterHdl, ItemHdl, pContext->FilterInfo[FilterHdl].TypeFilter, pItem->ItemCtx.pSipsiService));
         }
         else 
         {
            DIB_ASSERT(pItem->Config.SiPsi.Pid < DIB_ALL_PIDS);
            
            DIB_DEBUG(PACKET_LOG, (CRB " DibDriverProcessSiPsiBuffer: ItemHdl = %d (Pid=%d)" CRA, ItemHdl, pItem->Config.SiPsi.Pid));

            if(StartSection) 
            {
               if(pItem->ItemCtx.pSipsiService->Nb > 0) 
               {
                  DIB_DEBUG(PACKET_ERR, (CRB " DibDriverProcessSiPsiBuffer: Section begin while previous one not ended!" CRA));
               }

               pItem->ItemCtx.pSipsiService->Nb = 0;

               DibZeroMemory(pItem->ItemCtx.pSipsiService->Table, MAX_SECT_LENGTH);
            }

            if(!EndSection) 
            {
               if(pItem->ItemCtx.pSipsiService->Nb + PacketSize - HeaderLen > MAX_SECT_LENGTH) 
               {
                  DIB_DEBUG(PACKET_ERR, (CRB " DibDriverProcessSiPsiBuffer: error in Size ! line %d in file %s" CRA, __LINE__, __FILE__));
               }
               else 
               {
                  DibMoveMemory(&(pItem->ItemCtx.pSipsiService->Table[pItem->ItemCtx.pSipsiService->Nb]), pCurrentPacket + HeaderLen, PacketSize - HeaderLen);

                  pItem->ItemCtx.pSipsiService->Nb += PacketSize - HeaderLen;
               }
            }
            else /* End of section */
            {
               if( !StartSection && (pItem->ItemCtx.pSipsiService->Nb == 0) )
               {
                  DIB_DEBUG(PACKET_ERR, (CRB " DibDriverProcessSiPsiBuffer: EOS Error: StartSection %d Nb %d" CRA, StartSection, pItem->ItemCtx.pSipsiService->Nb));
               }
               if(pItem->ItemCtx.pSipsiService->Nb + PacketSize - HeaderLen - CrcLen > MAX_SECT_LENGTH) 
               {
                  DIB_DEBUG(PACKET_ERR, (CRB " DibDriverProcessSiPsiBuffer: error in Size Nb = %d Size = %d ! line %d in file %s" CRA,pItem->ItemCtx.pSipsiService->Nb,PacketSize, __LINE__, __FILE__));
               }
               else 
               {
                  DibMoveMemory(&pItem->ItemCtx.pSipsiService->Table[pItem->ItemCtx.pSipsiService->Nb], pCurrentPacket + HeaderLen, PacketSize - HeaderLen - CrcLen);

                  if(!CrcValid && pItem->Config.SiPsi.Crc) 
                  {
                     DIB_DEBUG(PACKET_ERR, (CRB " DibDriverProcessSiPsiBuffer: ItemHdl %x, CRC INVALID" CRA, ItemHdl));
                  }
                  else if(CrcValid || !pItem->Config.SiPsi.Crc) 
                  {
                     ELEM_HDL AliasItem;
                     DIB_DEBUG(PACKET_LOG, (CRB " DibDriverProcessSiPsiBuffer: ItemHdl %x, valid CRC" CRA, ItemHdl));
                     pItem->ItemCtx.pSipsiService->Nb += PacketSize - HeaderLen - CrcLen;

                     AliasItem = ItemHdl;
                     /** Item aliasing: browse each connected item are give it to the corresponding filter */
                     while(AliasItem != DIB_UNSET)
                     {
                        IntDriverProcessSiPsiSection(pContext, ChannelHdl, pContext->ItemInfo[AliasItem].ParentFilter, AliasItem, pItem->Config.SiPsi.Pid, pItem->ItemCtx.pSipsiService->Table, pItem->ItemCtx.pSipsiService->Nb);

                        AliasItem = pContext->ItemInfo[AliasItem].AliasItem;
                     }

#if (CHECK_SIPSI_CRC == 1)
                     NbSipsi++;
#endif
                  }
                  else 
                  {
                     DIB_DEBUG(PACKET_LOG, (CRB " DibDriverProcessSiPsiBuffer: Desynchro ! CRC_OK byte = 0x%x" CRA, (*(pCurrentPacket + PacketSize - 1))));
                  }

                  DibZeroMemory(pItem->ItemCtx.pSipsiService->Table, MAX_SECT_LENGTH);
                  pItem->ItemCtx.pSipsiService->Nb = 0;
               }
            }
         }
      }

      /** If packet Size is invalid, we are forced to flush the sipsi fifo. Otherwise we continue with the next section. */
      if(PacketSize == DIB_INVALID_NUMBER)
      {
         break;
      }
      else
      {
         pCurrentPacket += PacketSize;
      }
   }
   while(pCurrentPacket + HeaderLen < pBuf + Size);
}

#endif
