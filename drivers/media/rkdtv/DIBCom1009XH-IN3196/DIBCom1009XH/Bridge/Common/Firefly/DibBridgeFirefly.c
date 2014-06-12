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
* @file "DibBridgeFirefly.c"
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
#include "DibBridgeFireflyData.h"
#include "DibBridgeData.h"

#if (DRIVER_AND_BRIDGE_MERGED == 0)
#include "DibBridgeTargetModule.h"
#endif /*DRIVER_AND_BRIDGE_MERGED */

#if (DIB_CHECK_DATA == 1)
#include "DibBridgePayloadCheckers.h"
static void DibBridgeFireflyClearCheckStats(struct DibBridgeContext *pContext, uint32_t *pBuf);
#endif

/* Bridge 2 Driver message handling function prototype */
void DibB2DFwdMsg(struct DibBridgeContext *pContext, uint32_t Nb, uint16_t *pBuf);

static uint8_t DibBridgeFireflyReceiveMsg(struct DibBridgeContext *pContext, uint16_t *Data, uint8_t RiscId);
static DIBDMA  DibBridgeFireflyMsgHandler(struct DibBridgeContext *pContext, uint16_t *RxData, uint8_t RxCnt);
static DIBDMA  DibBridgeFireflyDataMsgHandler(struct DibBridgeContext *pContext, uint16_t *Data);

#define MAC_IRQ      (1 << 1)
#define IRQ_POL_MSK  (1 << 4)

union Dwd
{
   uint32_t l;

   struct 
   {
      uint16_t l;
      uint16_t h;
   } w;

   struct 
   {
      uint8_t ll;
      uint8_t lh;
      uint8_t hl;
      uint8_t hh;
   } b;
};

/****************************************************************************
* Setup chip memory controller
****************************************************************************/
static DIBSTATUS DibBridgeFireflySetupDma(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx *pDmaCtx)
{
   union Dwd l;
   DIBSTATUS ret = DIBSTATUS_ERROR;
   uint8_t CfgBlk[14];
   uint32_t ChipAddr = pDmaCtx->ChipAddr;

   DIB_ASSERT(pDmaCtx->DmaLen > 0);

   /* Set Len of transfer at Len */
   pDmaCtx->DmaSize = pDmaCtx->DmaLen;

   /** Overwrite ChipAddr to set the HostIf Interface register instead of the real chip.
    * The pDmaCtx->ChipAddr is set to REG_RAMIF_DATA. 
    * The Byte Enabled flag will be set on the address by the DibBridgeSetHostIfMode function
    * if the ByteMode passed to DibBridgeTargetDma, DibBridgeTargetWrite or DibBridgeTargetRead
    * is DIBBRIDGE_BIT_MODE_8. The DibBridgeSetHostIfMode function will also disable autoincrement
    * is destination address is REG_RAMIF_DATA, and enable it otherwise. */

   pDmaCtx->Mode = DIBBRIDGE_BIT_MODE_8;

   /* address formatting: use byte mode */
   pDmaCtx->FmtChipAddr = REG_RAMIF_DATA | 0x2000;

   /* Firefly previous porting compatibility */
   pDmaCtx->ChipAddr = REG_RAMIF_DATA | 0x2000;

   /* Odd start address at chip external space is not supported */
   if((ChipAddr & 1) && (ChipAddr & (1L << 23))) 
   {
      DIB_DEBUG(DMA_ERR, (CRB "Forbidden: odd start at ext mem" CRA));
      return DIBSTATUS_ERROR;
   }

   ret = DibBridgeWriteReg16(pContext, REG_RAMIF_IRAMCFG, 1);
   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   CfgBlk[0] = 0;
   CfgBlk[1] = 1;

   l.l = ChipAddr;
   CfgBlk[3] = l.b.hl;
   CfgBlk[4] = l.b.lh;
   CfgBlk[5] = l.b.ll;
   
   l.l = pDmaCtx->ChipBaseMax;
   CfgBlk[7] = l.b.hl;
   CfgBlk[8] = l.b.lh;
   CfgBlk[9] = l.b.ll;
   
   l.l = pDmaCtx->ChipBaseMin;
   CfgBlk[11] = l.b.hl;
   CfgBlk[12] = l.b.lh;
   CfgBlk[13] = l.b.ll;

   
   ret = DibBridgeTargetWrite(pContext, REG_RAMIF_MODE | 0x1000, DIBBRIDGE_BIT_MODE_16, 14, &CfgBlk[0]);
   if(ret != DIBSTATUS_SUCCESS)
      return ret;

   if(pDmaCtx->Dir == DIBBRIDGE_DMA_READ) 
   {
      ret = DibBridgeWriteReg16(pContext, REG_RAMIF_MODE, (1L << 15) | 1);
      if(ret != DIBSTATUS_SUCCESS)
         return ret;
   }
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
 * Take appropriate action upon a interruption sent by the chip
 ****************************************************************************/
static DIBDMA IntBridgeFireflyProcessMacIrq(struct DibBridgeContext *pContext, uint8_t RiscId)
{
   uint16_t RxData[128];
   uint8_t  RxCnt;

   DIB_DEBUG(IRQ_LOG, (CRB "IRQ: RISC %c" CRA, 'A' + RiscId));

   RxCnt = DibBridgeFireflyReceiveMsg(pContext, RxData, RiscId);

   if(RxCnt != 0) 
   {
      return DibBridgeFireflyMsgHandler(pContext, RxData, RxCnt);
   }

   return DIB_NO_DMA;
}

/****************************************************************************
 * Check population of a RISC mailbox
 ****************************************************************************/
static uint8_t IntBridgeFireflyGetMsgCnt(struct DibBridgeContext *pContext, uint8_t RiscId)
{
   uint16_t MsgCnt = 0;

   if(RiscId == ID_RISCA) 
   {
      DibBridgeReadReg16(pContext, REG_MAC_MBXA_IN, &MsgCnt);        /* RISC A mailbox message count is here */
      MsgCnt >>= 10;
      MsgCnt &= 0x1f;    /* 5 bit field */
   }
   else 
   {
      DibBridgeReadReg16(pContext, REG_MAC_MBXB_IN, &MsgCnt); /* Ditto for RISC B */
      MsgCnt >>= 8;
      MsgCnt &= 0x7f;    /* 7 bit field */
   }

   return ((uint8_t) MsgCnt);
}

/****************************************************************************
 * There was an interrupt. Let's check the necessary action
 ****************************************************************************/
static DIBDMA DibBridgeFireflyProcessIrq(struct DibBridgeContext *pContext)
{
   uint16_t tmp = 0;
   DIBDMA  DmaStat;

   DmaStat = DIB_NO_IRQ;

   if(DibBridgeReadReg16(pContext, REG_MAC_IRQ, &tmp) != DIBSTATUS_SUCCESS)   /* Clear HW IRQ */
      return DIB_DEV_FAILED; /* Device stopped answering */

#if (DEMOD_TRACE == 1)
   if(tmp & 0x08) 
   { 
      /** Flag for APB DMA */
      struct DibBridgeDmaCtx DmaCtx;

      memset(&DmaCtx.DmaFlags,0,sizeof(struct DibBridgeDmaFlags));
 
      DmaCtx.DmaFlags.Type = FORMAT_DTRACE;

      DmaCtx.ChipBaseMin = 0;
      DmaCtx.ChipBaseMax = 0;
      DmaCtx.ChipAddr = 0;
      DmaCtx.DmaLen = DTRACE_BUFF_SIZE;
      DmaCtx.Dir = DIBBRIDGE_DMA_READ;
      DmaCtx.pHostAddr = pContext->DemodTraceBuf;

      DibBridgeFireflySetupDma(pContext, &DmaCtx);

      return DibBridgeRequestDma(pContext, &DmaCtx);
   }
#endif

   if(IntBridgeFireflyGetMsgCnt(pContext, ID_RISCB) != 0) 
   {   
      /* Is RISC B yelling us? */
      DmaStat = IntBridgeFireflyProcessMacIrq(pContext, ID_RISCB);
   }
   else if(IntBridgeFireflyGetMsgCnt(pContext, ID_RISCA) != 0) 
   {      /* Maybe RISC A? */
      DmaStat = IntBridgeFireflyProcessMacIrq(pContext, ID_RISCA);
   }

   return DmaStat;
}

static void IntBridgeFireflyByteSwap(uint16_t * d, uint8_t Cnt)
{
  uint16_t t;

  while(Cnt--) 
  {
    t = *d;
    *d++ = t >> 8 | t << 8;
  }
}


/****************************************************************************
* Sends a message to RISC B
****************************************************************************/
DIBSTATUS DibBridgeFireflySendMsg(struct DibBridgeContext *pContext, uint32_t * short_data, uint32_t len)
{
   uint16_t *Data  = (uint16_t *)short_data;
   DIBSTATUS ret   = DIBSTATUS_ERROR;
   uint16_t  retry = DIB_BRIDGE_MAX_MAILBOX_TRY;
   uint16_t  WordsInBuf;
   uint32_t  k;

   DIB_ASSERT(((len % 2) == 0));

   len = len >> 1;

   ret = DibBridgeReadReg16(pContext, REG_MAC_MBXB_OUT, &WordsInBuf);
   if(ret != DIBSTATUS_SUCCESS) 
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "  DibBridgeFireflySendMsg: read Data unsuccessful" CRA));
      return ret;
   }

   while(((WordsInBuf & 0xff) + len > MBX_MAX_WORDS) && (retry--)) 
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "  DibBridgeFireflySendMsg: RISC mbx full, retrying (%04X, %d)" CRA, *Data, len));
      DibMSleep(1);
      ret = DibBridgeReadReg16(pContext, REG_MAC_MBXB_OUT, &WordsInBuf);
      
      if(ret != DIBSTATUS_SUCCESS) 
      {
         DIB_DEBUG(MAILBOX_ERR, (CRB "  DibBridgeFireflySendMsg: read Data unsuccessful" CRA));
         return ret;
      }
   }

   if(retry == 0) 
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "  DibBridgeFireflySendMsg: max retry count exceeded" CRA));
      return DIBSTATUS_TIMEOUT;
   }

   /* write msg */
   for(k = 0; k < len; k++) 
   {
      ret = DibBridgeWriteReg16(pContext, REG_MAC_MBXB_DATA, Data[k]);

      if(ret != DIBSTATUS_SUCCESS) 
      {
         DIB_DEBUG(MAILBOX_ERR, (CRB "  DibBridgeFireflySendMsg: write Data unsuccessful" CRA));
         return ret;
      }
   }

   /* update register nb_mes_in_RX */
   ret = DibBridgeWriteReg16(pContext, REG_MAC_MBXB_OUT, (uint16_t) 1 << 14);
   return ret;
}

/****************************************************************************
* Reads a message from one of the risc
****************************************************************************/
static uint8_t DibBridgeFireflyReceiveMsg(struct DibBridgeContext *pContext, uint16_t * Data, uint8_t RiscId)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;
   uint8_t   Size   = 0;
   uint8_t   McBase = 0; /* Risc A*/

   if(RiscId == ID_RISCB) 
      McBase = 16;

   /* Length and Type in the first word */
   Status = DibBridgeReadReg16(pContext, REG_MAC_MBXA_DATA + McBase, &Data[0]);
   if(Status != DIBSTATUS_SUCCESS)
      return 0;

   /* Get Msg Size */
   Size = Data[0] & 0xff;

   /* Coherency Check */
   if((Size == 0) || (Size > 128))
   {
      Status = DIBSTATUS_ERROR;
      goto End;
   }

   /* Initial word already read */
   Size--;

   /* Fetch entire message */
   Status = DibBridgeTargetRead(pContext, REG_MAC_MBXA_DATA + McBase, DIBBRIDGE_BIT_MODE_16, Size * 2, (uint8_t *) & Data[1]);
   if(Status != DIBSTATUS_SUCCESS)
      goto End;

   /* Update register nb_mes_in_TX */
   Status = DibBridgeWriteReg16(pContext, REG_MAC_MBXA_IN + McBase, (uint32_t) 1 << 14);
   if(Status != DIBSTATUS_SUCCESS)
      goto End;

   IntBridgeFireflyByteSwap(&Data[1], Size);

End:
   if(Status != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "Mailbox Synchronisation Error Size %d Data %x" CRA, Size, Data[0]));
      return 0;
   }

   /* return Msg Size */
   return (Size + 1);
}

/**
 * Find the Firefly Service that corresponds to the Item.
 * @param[in/out] Nb: in input this is the item, in output this is the service.
 * @return: DIBSTATUS_SUCCESS if found.
 */
DIBSTATUS DibBridgeFireflyGetService(struct DibBridgeContext *pContext, uint8_t ItemHdl, uint8_t *pSvcNb)
{
   uint8_t ii = 0;

   for(ii = 0; ii < DIB_MAX_NB_SERVICES; ii++)
   {
      if(pContext->FireflyInfo[ii].ItemIndex == ItemHdl)
      {
         (*pSvcNb) = ii;
         return DIBSTATUS_SUCCESS;
      }
   }
   return DIBSTATUS_ERROR;
}

/****************************************************************************
 * Parses and processes the most prioritary messages, and passes the others
 * to the upper layer. Returns the DMA state: no DMA. done or pending.
 ****************************************************************************/
static DIBDMA DibBridgeFireflyDataMsgHandler(struct DibBridgeContext *pContext, uint16_t * Data)
{
   uint32_t min, max, add, len;
   DIBDMA  DmaStat;
   struct DibBridgeDmaFlags flags;
   uint8_t SvcNb;

   DIB_ASSERT(Data);
   memset(&flags, 0, sizeof(struct DibBridgeDmaFlags));

   /* Data notification */
   Data++;
   min = (uint32_t) Data[0] | ((uint32_t) Data[1] << 16);
   max = (uint32_t) Data[2] | ((uint32_t) Data[3] << 16);
   add = (uint32_t) Data[4] | ((uint32_t) Data[5] << 16);
   len = (uint32_t) Data[6] | ((uint32_t) Data[7] << 16);

   flags.Type       = Data[8] & 0x1f;
   SvcNb            = (Data[8] >> 8) & ~0xe0; /**< Warning this is the Firefly Service number */
   flags.FirstFrag  = (((Data[8] >> 8) & 0xe0) & 0x80)? eDIB_TRUE : eDIB_FALSE;
   flags.LastFrag   = (((Data[8] >> 8) & 0xe0) & 0x40)? eDIB_TRUE : eDIB_FALSE;
   flags.Prefetch   = (Data[8] & PREFETCH_DATA);
   flags.NbRows     = ((Data[8] & 0xc0) >> 6);

   if(flags.Type == FORMAT_SIPSI)
   {
      /** Warning: Svc is always set to 0 is FORMAT_SIPSI. svc and item will be compute later */
      flags.ItemHdl = 0;
   }
   else if(flags.Type == FORMAT_RAWTS)
   {
      /** Warning: embedded firefly firmware versions following sdk7.18 give us the last service of the last TS in the raw ts buffer.
       * This permi use to easily find the filter (and compare it to TheFireflyRawTsFilter). */
      flags.ItemHdl = pContext->FireflyInfo[SvcNb].ItemIndex;
   }
   else
   {
      /** overwrite SvcNb in case of firefly to have the item */
      flags.ItemHdl = pContext->FireflyInfo[SvcNb].ItemIndex;
   }
   DIB_ASSERT(flags.ItemHdl < DIB_MAX_NB_ITEMS);

   DIB_DEBUG(RAWTS_LOG, (CRB "FLAGS : s %d t %d f %d l %d p %d" CRA, flags.ItemHdl, flags.Type, flags.FirstFrag, flags.LastFrag, flags.Prefetch));
   DIB_DEBUG(RAWTS_LOG, (CRB CRB "" CRA CRA));
   DIB_DEBUG(RAWTS_LOG, (CRB "--- Filter %d " CRA, pContext->ItSvc[flags.ItemHdl].FilterParent));

   DmaStat = DibBridgeHighDataMsgHandlerCommon(pContext, min, max, add, len, &flags);

   if(DmaStat == DIB_NO_DMA)
   {
      DIB_DEBUG(MAILBOX_LOG,
         (CRB "%04x %04x %04x %04x %04x %04x %04x %04x %04x" CRA, Data[0],
         Data[1], Data[2], Data[3], Data[4], Data[5], Data[6], Data[7], Data[8]));
   }

   return DmaStat;      /* Tells do we have a pending DMA or not */
}

/****************************************************************************
* Checks message coming from the RISC and acts appropriately
****************************************************************************/
static DIBDMA DibBridgeFireflyMsgHandler(struct DibBridgeContext *pContext, uint16_t * RxData, uint8_t RxCnt)
{
   uint8_t MsgId;

   /* ------------------------------------------------------------------------------------ */
   /* Now we have one message, let's check the Type of it */
   MsgId = (uint8_t) (RxData[0] >> 8);
   DIB_DEBUG(IRQ_LOG, (CRB "IRQ: MSG %d, Size %d" CRA, MsgId, RxCnt));

   /* ------------------------------------------------------------------------------------ */
   /* It can be either Data, CPT or a message to passed up */
   /* ------------------------------------------------------------------------------------ */
   if(MsgId == 0) 
   {
      /* This is Data message */
      return DibBridgeFireflyDataMsgHandler(pContext, RxData);
   }
   /* ------------------------------------------------------------------------------------ */
   else if(MsgId == 1) 
   {
      /* This is CPT, Column Presence Table. Fetch it. */
      DibBridgeFireflyGetCpt(pContext, RxData);
   }
   /* ------------------------------------------------------------------------------------ */

   else if(MsgId == 2) 
   {
#if (DIB_CHECK_DATA == 1)
      /* Clear Bridge checker statistics */
      DibBridgeFireflyClearCheckStats(pContext, (uint32_t *)RxData);
#endif
   }
   /* ------------------------------------------------------------------------------------ */
   else 
   { 
      /* */
      if(MsgId == 3)
         DibBridgeFreeUnusedMpeBuffer(pContext);

      /* Other: the whole message will be passed up */
      DIB_DEBUG(MAILBOX_LOG, (CRB "MSG IN (%d) forwarded " CRA, MsgId));
      DibB2DFwdMsg(pContext, (RxCnt << 1)/*in bytes*/, RxData);
   }

   /* ------------------------------------------------------------------------------------ */
   return DIB_NO_DMA;
}

/****************************************************************************
 * Address and Data formatting, if needed, before calling target.
 * This function is also a place to implement specific chipset workarounds.
 * bits 0-11: indicates 12 bits for address (that reference a register), until 4096
 * bit 12: indicates if autoincrement is needed (for data access only).
 * bit 13: indicates is byte mode is enabled
****************************************************************************/
static DIBSTATUS DibBridgeFireflyPreFormat(struct DibBridgeContext *pContext, uint8_t ByteMode, uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * buf, uint32_t Cnt)
{
   /* 16 bit accesses cannot be done with even len */
   if(((ByteMode) == DIBBRIDGE_BIT_MODE_16) && (Cnt & 1)) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetWrite: Odd byte count" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Autoincrement is only used as an optimization with RAM_IF_MODE, otherwise it is never used */

   /* Set Byte Enabled Mode */
   if((ByteMode) == DIBBRIDGE_BIT_MODE_8)
      *Addr = *Addr | 0x2000;

   return DIBSTATUS_CONTINUE;
}

/****************************************************************************
 * Address and Data formatting, if needed, after calling target.
 * This function is also a place to implement specific chipset workarounds.
****************************************************************************/
static DIBSTATUS DibBridgeFireflyPostFormat(struct DibBridgeContext *pContext, uint8_t ByteMode, uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * buf, uint32_t Cnt)
{
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
 * Increment a formatted address, Address and Data formatting, if needed, before calling target.
 * This function is also a place to implement specific chipset workarounds.
 * bits 0-11: indicates 12 bits for address (that reference a register), until 4096
 * bit 12: indicates if autoincrement is needed (for data access only).
 * bit 13: indicates is byte mode is enabled
****************************************************************************/
static uint32_t DibBridgeFireflyIncrementFormattedAddress(struct DibBridgeContext *pContext, uint32_t InFmtAddr, int32_t Offset)
{
   /* 16 bit accesses cannot be done with even len */
   if(InFmtAddr & 0x1000) 
   {
      /* firefly is 16 bit width registers, but offset between two consecutive registers is 1 and not 2. */
      InFmtAddr += (Offset >> 1);
   }

   return InFmtAddr;
}

/******************************************************************************
 * return the bus architecture (32, 16 or 8 bits)
 ******************************************************************************/
static uint8_t DibBridgeFireflyGetArchi(struct DibBridgeContext *pContext)
{
   return DIBBRIDGE_BIT_MODE_16;
}

/******************************************************************************
 * clean checker statistics.
 * is defined.
 ******************************************************************************/
#if (DIB_CHECK_DATA == 1)
static void DibBridgeFireflyClearCheckStats(struct DibBridgeContext *pContext, uint32_t *pBuf)
{
   uint16_t          *Data = (uint16_t *)pBuf;
   uint8_t           SvcNb;
   ELEM_HDL          ItemHdl;
   FILTER_HDL        FilterHdl;
   enum DibDataType  FilterType;

   /* When ClearMonit message, clear Bridge monitoring info */
   if((Data[1] & 0x00FF) == 1)
   {
      SvcNb = Data[1] >> 8;
      ItemHdl = pContext->FireflyInfo[SvcNb].ItemIndex;
      /* ItemHdl equals DIB_UNSET if a RAWTS filter was created (no item) */
      /* in this case, monitoring info are resetted by DibBridgeSetInfo */
      if(ItemHdl != DIB_UNSET)
      {
         FilterHdl = pContext->ItSvc[ItemHdl].FilterParent;
         FilterType = pContext->FilterInfo[ItemHdl].DataType;
   
         /* DVB-H: Clear IP and RTP checker data */
         if(FilterType == eMPEFEC)
         {
            pContext->ItSvc[ItemHdl].CcFailCnt     = 0;
            pContext->ItSvc[ItemHdl].ErrCnt        = 0;
            pContext->ItSvc[ItemHdl].CurCc         = 0xffff;
         }
#if DIB_CHECK_RAWTS_DATA == 1
         /* DVB-T: Clear RAWTS checker data */
         else if (FilterType == eTS)
         {
            DibSetMemory(&pContext->FilterInfo[FilterHdl].CheckRawTs, 0, sizeof(struct CheckRawTs));
         }
#endif
   
         DIB_DEBUG(MAILBOX_LOG, (CRB "Clear checker stats for Item %d Svc %d" CRA, ItemHdl, SvcNb));
      }
   }
}


/** Build a message for driver to summarize ip checking */
/* XXX this should not go in the official release */
static void DibBridgeFireflyForwardCheckStats(struct DibBridgeContext *pContext, ELEM_HDL Item)
{
   /* It might be useful to deliver this information to the RISC */
   /* as it is already having bookkeeping of frame statuses! */
   /* Information could be then got from there by monitoring msg. */

   uint16_t Buffer[8];
   uint8_t  Filter = pContext->ItSvc[Item].FilterParent;
   uint32_t Cc, Err, Tot;

#if DIB_CHECK_RAWTS_DATA == 1
   if(pContext->FilterInfo[Filter].DataType == eTS)
   {
      Tot    = pContext->FilterInfo[Filter].CheckRawTs.TotalNbPackets;
      Cc     = pContext->FilterInfo[Filter].CheckRawTs.DiscontinuitiesCount;
      Err    = pContext->FilterInfo[Filter].CheckRawTs.CorruptedPacketsCount;
   }
   else 
#endif
   if (pContext->FilterInfo[Filter].DataType == eMPEFEC)
   {
      Tot = 0;
      Cc  = pContext->ItSvc[Item].CcFailCnt;
      Err = pContext->ItSvc[Item].ErrCnt;
   }
   else
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "DibBridgeFireflyForwardCheckStats : unsupported data type" CRA));
      return;
   }

   Buffer[0] = 0xFF << 8;
   Buffer[1] = (uint16_t) Item;
   Buffer[2] = (uint16_t) (Cc & 0xffff);
   Buffer[3] = (uint16_t) ((Cc >> 16) & 0xffff);
   Buffer[4] = (uint16_t) (Err & 0xffff);
   Buffer[5] = (uint16_t) ((Err >> 16) & 0xffff);
   Buffer[6] = (uint16_t) (Tot & 0xffff);
   Buffer[7] = (uint16_t) ((Tot >> 16) & 0xffff);

   DibB2DFwdMsg(pContext, (8 << 1), Buffer);
}
#endif

/**
 * Associate item to service at bridge levelFill the "DataType" and "FireflyInfo" information with the driver's information concerning the link
 * between service (in firefly) and item and concerning the Data Mode: ip or callback.
 * (the eUNSET value is used to reset the entry when removing a service)
 * @param pContext   pointer to the bridge context
 * @param Svc        firefly's service 
 * @param ItemHdl    item's number for adding, DIB_UNSET for removing the service
 * @param FilterHdl  retrieve's Mode of the Data (ip or callback) 
 */
static void DibBridgeFireflySetService(struct DibBridgeContext *pContext, uint8_t Svc, ELEM_HDL ItemHdl, FILTER_HDL FilterHdl, enum DibDataType DataType, enum DibDataMode DataMode)
{
   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(Svc < DIB_MAX_NB_SERVICES);

   if(DataMode == eUNSET)
   {
      pContext->FireflyInfo[Svc].ItemIndex = DIB_UNSET;
      DIB_DEBUG(PORT_LOG,(CRB "RemoveBridgeItem %d (Svc %d) from filter %d" CRA, ItemHdl, Svc, FilterHdl));
   }
   else
   {
      pContext->FireflyInfo[Svc].ItemIndex = ItemHdl;
      DIB_DEBUG(PORT_LOG,(CRB "AddBridgeItem %d (Svc %d) to filter %d" CRA, ItemHdl, Svc, FilterHdl));
   }

   /** In firefly firmware, all RAW_TS service are placed in a big buffer, thus no service id is given in IN_MSG_DATA message. 
    * This corresponds for SDK-7.2 to a unique static filter referenced by pContext->RawDataCtx.TheFireflyRawTsFilter */
   if(DataType == eTS)
   {
      if(DataMode == eUNSET)
      {
         DIB_ASSERT(pContext->FilterInfo[FilterHdl].ItemCount < DIB_MAX_NB_ITEMS - 1);
         DIB_ASSERT(pContext->RawDataCtx.TheFireflyRawTsFilter == FilterHdl);
         if(pContext->FilterInfo[FilterHdl].ItemCount == 0)
         {
            pContext->RawDataCtx.TheFireflyRawTsFilter = DIB_UNSET;
            DIB_DEBUG(PORT_LOG,(CRB "TheFireflyRawTsFilter is set to: DIB_UNSET" CRA));
         }
      }
      else
      {
         DIB_ASSERT(pContext->FilterInfo[FilterHdl].ItemCount > 0);
         if(pContext->FilterInfo[FilterHdl].ItemCount > 1)
         {
            DIB_ASSERT(pContext->RawDataCtx.TheFireflyRawTsFilter == FilterHdl);
         }
         else
         {
            DIB_ASSERT(pContext->RawDataCtx.TheFireflyRawTsFilter == DIB_UNSET);
            pContext->RawDataCtx.TheFireflyRawTsFilter = FilterHdl;
            DIB_DEBUG(PORT_LOG,(CRB "TheFireflyRawTsFilter is set to: %d" CRA, FilterHdl));
         }
      }
   }
}

/**
 * Tell RISC it can start it's work
 * @param pContext: bridge context
 * @param SvcNb: service number that failed
 */
static DIBSTATUS DibBridgeFireflySignalBufFail(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags * pFlags, uint8_t Flush)
{
   uint16_t    msg[2];
   uint8_t     SvcNb = 0;

   DIBSTATUS   rc = DIBSTATUS_ERROR;

   rc = DibBridgeFireflyGetService(pContext, pFlags->ItemHdl, &SvcNb);
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);

   /* Tell RISC it can start it's work */
   msg[0] = (1 << 8) + 2; /* FF1 */
   msg[1] = SvcNb;

   return DibBridgeFireflySendMsg(pContext, (uint32_t*)msg, 1 << 2);
}

/******************************************************************************
 * Dma if finished, acknowledge the firmware and do the job
 ******************************************************************************/
static DIBSTATUS DibBridgeFireflyProcessDma(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   /* process dma independantly of the architecture */
   return DibBridgeProcessDmaCommon(pContext, pDmaCtx);
}

/******************************************************************************
 * Init voyager software specific
 ******************************************************************************/
static DIBSTATUS DibBridgeFireflyChipsetInit(struct DibBridgeContext *pContext)
{
   return DIBSTATUS_SUCCESS;
}

/******************************************************************************
 * Deinit dragonfly and voyager software specific
 ******************************************************************************/
static void DibBridgeFireflyChipsetDeinit(struct DibBridgeContext *pContext)
{
}

/******************************************************************************
 * Specific output message formating for dragonfly
 ******************************************************************************/
void DibBridgeFireflyRegisterIf(struct DibBridgeContext *pContext)
{
   pContext->BridgeChipOps.SendMsg                   = DibBridgeFireflySendMsg;
   pContext->BridgeChipOps.SendAck                   = DibBridgeFireflySendAck;
   pContext->BridgeChipOps.AssembleSlice             = DibBridgeFireflyAssembleSlice;
   pContext->BridgeChipOps.ProcessIrq                = DibBridgeFireflyProcessIrq;
   pContext->BridgeChipOps.ProcessDma                = DibBridgeFireflyProcessDma; 
   pContext->BridgeChipOps.SetupDma                  = DibBridgeFireflySetupDma;
   pContext->BridgeChipOps.RequestDma                = DibBridgeTargetRequestDma;
   pContext->BridgeChipOps.GetArch                   = DibBridgeFireflyGetArchi;
   pContext->BridgeChipOps.PreFormat                 = DibBridgeFireflyPreFormat;
   pContext->BridgeChipOps.PostFormat                = DibBridgeFireflyPostFormat;
   pContext->BridgeChipOps.IncrementFormattedAddress = DibBridgeFireflyIncrementFormattedAddress;
   pContext->BridgeChipOps.SignalBufFail             = DibBridgeFireflySignalBufFail;
   pContext->BridgeChipOps.ChipsetInit               = DibBridgeFireflyChipsetInit;
   pContext->BridgeChipOps.ChipsetDeinit             = DibBridgeFireflyChipsetDeinit;

#if (DIB_BRIDGE_HBM_PROFILER == 1)
   pContext->BridgeChipOps.HbmProfiler               = DibBridgeFireflyHbmProfiler;
#endif

#if (DIB_CHECK_DATA == 1)
   pContext->BridgeChipOps.ClearCheckStats           = DibBridgeFireflyClearCheckStats;
   pContext->BridgeChipOps.ForwardCheckStats         = DibBridgeFireflyForwardCheckStats;
#endif

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))
   pContext->BridgeChipOps.TestBasicRead             = IntBridgeFireflyTestBasicRead;
   pContext->BridgeChipOps.TestRegister              = IntBridgeFireflyTestRegister;
   pContext->BridgeChipOps.TestInternalRam           = IntBridgeFireflyTestInternalRam;
   pContext->BridgeChipOps.TestExternalRam           = IntBridgeFireflyTestExternalRam;
   pContext->BridgeChipOps.GetRamAddr                = DibBridgeFireflyGetRamAddr;
#endif
   pContext->BridgeChipOps.SetService                = DibBridgeFireflySetService;
}


#endif /* USE_FIREFLY */
