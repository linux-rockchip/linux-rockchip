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
* @file "DibDriverDragonflyDataSiPsi.c"
* @brief Dragonfly Driver specific SiPSi Data handler
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_SIPSI_DATA == 1)

DIBSTATUS DibDriverDragonflyAddItemSiPsi(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, ELEM_HDL * pElemHdl, uint32_t ItemHdl, struct MsgCreateItem * msg)
{
   struct DibDriverSipsiServiceCtx  *pSipsiService;
   struct DibDriverItem             *pItem;
   uint8_t                          *pTable = 0;

   msg->Param.Sipsi.Pid       = pFilterDesc->SiPsi.Pid;
   msg->Param.Sipsi.PidWatch  = pFilterDesc->SiPsi.PidWatch;
   msg->Param.Sipsi.PidCrc    = pFilterDesc->SiPsi.Crc;

   pSipsiService = (struct DibDriverSipsiServiceCtx *) DibMemAlloc(sizeof(struct DibDriverSipsiServiceCtx));
   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pSipsiService = pSipsiService;
   if(pSipsiService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for SIPSI service" CRA));
      return DIBSTATUS_ERROR;
   }
   pItem->ItemCtx.pSipsiService->Nb           = 0;
   pItem->ItemCtx.pSipsiService->pFilterSipsi = NULL;

   /** allocate a table for retreiving sipsi sections */
   pTable = (uint8_t *) DibMemAlloc(MAX_SECT_LENGTH);
   if(pTable == NULL)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB " IntDriverDragonflyAddItem: no able to allocate sipsi buffer memory" CRA));
      return DIBSTATUS_RESOURCES;
   }

   pItem->ItemCtx.pSipsiService->Table = pTable;
   DibZeroMemory(pItem->ItemCtx.pSipsiService->Table, MAX_SECT_LENGTH);

   return DIBSTATUS_SUCCESS;
}


/**
 * Retreive section fragment header information
 */
DIBSTATUS DibDriverDragonflyGetSiPsiInfo(struct DibDriverContext * pContext, uint8_t * pPacketAddr, int32_t *pPacketSize, uint32_t *pItemIndex, uint8_t *pStartSection, uint8_t *pEndSection, uint8_t *pCrcValid, uint8_t *pHeaderLen, uint8_t *pCrcLen)
{
   int32_t PacketSize;

   *pPacketSize = PacketSize = pPacketAddr[2] | (pPacketAddr[3] << 8);

   if(PacketSize < 4)
   {
      DIB_DEBUG(PACKET_LOG, (CRB " IntDriverDragonflyGetSiPsiInfo: invalid Length: %d" CRA, PacketSize));
      return DIBSTATUS_ERROR;
   }

   /** Top Header and Crc at the bottom (if EndSection) */
   *pHeaderLen = SIPSI_HEADER_SIZE;
   *pCrcLen = 0;

   *pItemIndex    = pPacketAddr[0];
   *pStartSection = 1;
   *pEndSection   = 1;
   *pCrcValid     = 1;

   if((*pItemIndex == DIB_UNSET) || (*pItemIndex >= DIB_MAX_NB_ITEMS))
   {
      DIB_DEBUG(PACKET_LOG, (CRB " IntDriverDragonflyGetSiPsiInfo: Section not requested" CRA));
      return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyRemoveItemSiPsi(struct DibDriverContext *pContext, struct DibDriverItem *pItem, ELEM_HDL ElemHdl)
{
   struct DibSipsiFilterCfg *pFirstFilter;
   struct DibSipsiFilterCfg *pNextFilter;

   DIB_ASSERT(pItem->ItemCtx.pSipsiService->Table != NULL);

   DibMemFree(pItem->ItemCtx.pSipsiService->Table, MAX_SECT_LENGTH);
   pItem->ItemCtx.pSipsiService->Table = NULL;

   pFirstFilter = pContext->ItemInfo[ElemHdl].ItemCtx.pSipsiService->pFilterSipsi;
   while(pFirstFilter)
   {
      pNextFilter = pFirstFilter->pNext;
      DIB_ASSERT(pFirstFilter);
      DibMemFree(pFirstFilter, sizeof(struct DibSipsiFilterCfg));
      pFirstFilter = pNextFilter;
   }
   pContext->ItemInfo[ElemHdl].ItemCtx.pSipsiService->pFilterSipsi = NULL;

   DIB_ASSERT(pItem->ItemCtx.pSipsiService != NULL);
   DibMemFree(pItem->ItemCtx.pSipsiService, sizeof(struct DibDriverSipsiServiceCtx));
   pItem->ItemCtx.pSipsiService = NULL;

   return DIBSTATUS_SUCCESS;
}
#endif


#endif /* USE_DRAGONFLY */
