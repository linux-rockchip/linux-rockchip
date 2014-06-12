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
* @file "DibDriverFireflyDataSiPsi.c"
* @brief Firefly Driver specific SiPSi Data handler
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverFirefly.h"

#if (DIB_SIPSI_DATA == 1)

DIBSTATUS DibDriverFireflyAddItemSiPsi(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo)
{
   if(pFilterDesc->SiPsi.Crc)
      PidInfo->PidTarget = eTARGET_SIPSI;
   else
      PidInfo->PidTarget = eTARGET_SIPSI_NO_CRC;

   PidInfo->Pid                             = pFilterDesc->SiPsi.Pid;
   PidInfo->Specific.SiPsiSpecific.pidWatch = pFilterDesc->SiPsi.PidWatch;

   return DIBSTATUS_SUCCESS;
}

/**
* Retreive section fragment header information
* @param pPacketSize: pointer on Packet Size. Include Top and bottom parts (header + crc).
*/
DIBSTATUS DibDriverFireflyGetSipsiInfo(struct DibDriverContext * pContext, uint8_t *pPacketAddr, int32_t *pPacketSize, uint32_t *pItemIndex, uint8_t *pStartSection, uint8_t *pEndSection, uint8_t *pCrcValid, uint8_t *pHeaderLen, uint8_t *pCrcLen)
{
   uint32_t PacketSize;
   uint8_t  SvcNb = (pPacketAddr[1] >> 3) & ALL_SVC_MSK;

   DIB_ASSERT(pPacketAddr);
   DIB_ASSERT(pPacketSize);
   DIB_ASSERT(pItemIndex);
   DIB_ASSERT(pStartSection);
   DIB_ASSERT(pEndSection);
   DIB_ASSERT(pCrcValid);

   /** Top Header and Crc at the bottom (if EndSection) */
   *pHeaderLen = 2;
   *pCrcLen = 1;

   /** Read the PacketSize before returning a DIB_ERROR, the size must be correct */
   PacketSize = pPacketAddr[0];

   if(PacketSize < 3 || PacketSize > SIPSI_BUFFER_SIZE)
   {
      DIB_DEBUG(PACKET_ERR, (CRB " DibDriverpacket_demux: invalid Length: %d" CRA, PacketSize));
      *pPacketSize=DIB_INVALID_NUMBER;
      return DIBSTATUS_ERROR;
   }
   if(SvcNb >= DIB_MAX_NB_SERVICES)
   {
      /** packet probably corrupted, need to flush the fifo. */
      DIB_DEBUG(PACKET_ERR, (CRB " DibDriverpacket_demux: invalid service %d" CRA, SvcNb));
      *pPacketSize=DIB_INVALID_NUMBER;
      return DIBSTATUS_ERROR;
   }

   *pPacketSize = PacketSize;
   *pItemIndex = pContext->FireflyService[SvcNb].ItemIndex;

   if((*pItemIndex >= DIB_MAX_NB_ITEMS) || (pContext->FireflyService[SvcNb].ValidPid == PID_FILTER_EMPTY))
   {
      /** probably old eSIPSI received while the item has just been removed. */
      return DIBSTATUS_ERROR;
   }

   DIB_ASSERT(*pItemIndex < DIB_MAX_NB_ITEMS);

   *pStartSection = (pPacketAddr[1] & 0x1);
   *pEndSection   = (pPacketAddr[1] & 0x2);

   if(pContext->ItemInfo[*pItemIndex].Config.SiPsi.Crc)
      *pCrcValid  = (*(pPacketAddr + PacketSize - 1) == 0x55) ? 1 : 0;
   else
      *pCrcValid  = 1;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddPidSiPsi(struct DibDriverContext *pContext, struct DibDriverItem *pItem, struct DibAddPidInfo * pPidInfo, FILTER_HDL FilterHdl, ELEM_HDL AliasItemHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode)
{
   struct   DibDriverSipsiServiceCtx *pSipsi;

   if(AliasItemHdl == DIB_MAX_NB_ITEMS)
   {
      pSipsi = (struct DibDriverSipsiServiceCtx *) DibMemAlloc(sizeof(struct DibDriverSipsiServiceCtx));
      pItem->ItemCtx.pSipsiService = pSipsi;
      if(pSipsi == NULL)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for SIPSI service" CRA));
         return DIBSTATUS_RESOURCES;
      }

      pItem->ItemCtx.pSipsiService->Table = (uint8_t *) DibMemAlloc(MAX_SECT_LENGTH);
      if(pItem->ItemCtx.pSipsiService->Table == NULL)
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB " IntDriverAddPid: no room for section Buffer for Pid %d" CRA, pPidInfo->Pid));
         DibMemFree(pSipsi, sizeof(struct DibDriverSipsiServiceCtx));
         return DIBSTATUS_RESOURCES;
      }
      DibZeroMemory(pItem->ItemCtx.pSipsiService->Table, MAX_SECT_LENGTH);

      pItem->ItemCtx.pSipsiService->Nb = 0;
      pItem->ItemCtx.pSipsiService->pFilterSipsi = NULL;

      /* The Alias Item List is Empty hen adding the main item */
      pItem->AliasItem = DIB_UNSET;
   }
   else
   {
      struct DibDriverItem *pAliasItem = &pContext->ItemInfo[AliasItemHdl];
      ELEM_HDL              ItemHdl    = (ELEM_HDL) (pItem - pContext->ItemInfo);

      pItem->ItemCtx.pSipsiService = NULL;
      /* Insert this item as first element of main item alias list */
      pItem->AliasItem      = pAliasItem->AliasItem;
      pAliasItem->AliasItem = ItemHdl;
   }

   *isWatch = pPidInfo->Specific.SiPsiSpecific.pidWatch;
   *format  = FORMAT_SIPSI;

   pContext->FilterInfo[FilterHdl].NbActivePids++;

   return DIBSTATUS_SUCCESS;
}


DIBSTATUS DibDriverFireflyDelPidSiPsi(struct DibDriverContext *pContext, struct DibDriverItem *pItem, ELEM_HDL elemHdl)
{
   struct DibSipsiFilterCfg *pFirstFilter;
   struct DibSipsiFilterCfg *pNextFilter;

   if(pItem->AliasItem == DIB_UNSET)
   {
      DIB_ASSERT(pItem->ItemCtx.pSipsiService->Table != NULL);
      DibMemFree(pItem->ItemCtx.pSipsiService->Table, MAX_SECT_LENGTH);
      pItem->ItemCtx.pSipsiService->Table = NULL;

      pFirstFilter = pContext->ItemInfo[elemHdl].ItemCtx.pSipsiService->pFilterSipsi;
      while(pFirstFilter)
      {
         pNextFilter = pFirstFilter->pNext;
         DIB_ASSERT(pFirstFilter);
         DibMemFree(pFirstFilter, sizeof(struct DibSipsiFilterCfg));
         pFirstFilter = pNextFilter;
      }
      pContext->ItemInfo[elemHdl].ItemCtx.pSipsiService->pFilterSipsi = NULL;

      DIB_ASSERT(pItem->ItemCtx.pSipsiService != NULL);
      DibMemFree(pItem->ItemCtx.pSipsiService, sizeof(struct DibDriverSipsiServiceCtx));
      pItem->ItemCtx.pSipsiService = NULL;
   }
   else
   {
      /**
       * Removing the main item having at least one connected alias:
       * The next alias become the main item, and the current pSipsiService (which main contain partial valid section)
       * is 'moved' to the new main item. Then the bridge is informed of that CHANGE
       */
      DIB_ASSERT(pContext->ItemInfo[pItem->AliasItem].ItemCtx.pSipsiService == NULL);
      DIB_ASSERT(pContext->ItemInfo[elemHdl].ItemCtx.pSipsiService);
      pContext->ItemInfo[pItem->AliasItem].ItemCtx.pSipsiService = pItem->ItemCtx.pSipsiService;
      pItem->ItemCtx.pSipsiService = NULL;
   }

   return DIBSTATUS_SUCCESS;
}

#endif


#endif /* USE_FIREFLY */
