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
* @file "DibDriver.c"
* @brief Generic Driver functionality.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverDowncalls.h"
#include "DibBoardSelection.h"
#include "DibBoardFlWrSelection.h"
#include "DibBoardFlRdSelection.h"
#include "DibDriverData.h"

#if (USE_DRAGONFLY == 1)
#include "DibDriverDragonfly.h"
#include "DibMsg.h"
#endif

#if (USE_FIREFLY == 1)
#include "DibDriverFirefly.h"
#endif


/**
 * Driver Chip operation selection depending on DibChip Value.
 * @param pContext pointer to the context
 */
DIBSTATUS IntDriverRegisterChip(struct DibDriverContext *pContext)
{
   switch(pContext->DibChip)
   {
#if (USE_DRAGONFLY == 1)
   case DIB_VOYAGER:
   case DIB_NAUTILUS:
#if (mSDK == 1)
      DibDriverDragonflyMSdkRegisterChip(pContext);
#else
      DibDriverDragonflyRegisterChip(pContext);
#endif
      return DIBSTATUS_SUCCESS;
#endif

#if (USE_FIREFLY == 1)
   case DIB_FIREFLY:
      DibDriverFireflyRegisterChip(pContext);
      return DIBSTATUS_SUCCESS;
#endif

   default:
      return DIBSTATUS_ERROR;
   }
}

/**
 * Firmware Initialization.
 * @param pContext pointer to the context
 */
DIBSTATUS DibDriverInitFirmware(struct DibDriverContext *pContext)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;
   uint16_t  FeId   = 0;
   uint32_t StreamNum;

   struct DibDemodBoardConfig      *pBoard = pContext->Hd.BoardConfig;

   if((Status = DibDriverSetCfg(pContext, pContext->Hd.BoardConfig->BoardConfig)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInitFirmware: DibDriverSetCfg returned 0x%x" CRA, Status));
      goto End;
   }

   /* Download slave firmwares if required */
   for(FeId = 0; FeId < pBoard->NbFrontends; FeId++)
   {
      if(FeId > 0)
      {
         /* for a slave chip, we have to tell to main-risc to download a firmware first, reset the slave and then it can be initialized. */
         if((Status = DibDriverDwldSlaveFw(pContext,FeId)) != DIBSTATUS_SUCCESS)
         {
            DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInitFirmware: DibDriverDwldSlaveFw returned 0x%x" CRA, Status));
            goto End;
         }
      }

      /* [4] */
      if((Status = DibDriverSetCfgGpio(pContext, pBoard->FeCfg[FeId].GpioFunctions, FeId)) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInitFirmware: DibDriverSetCfgGpio returned 0x%x" CRA, Status));
         goto End;
      }

      /* Frequency subband selector. */
      if((Status = DibDriverSubbandSelect(pContext, &pBoard->FeCfg[FeId].SubbandSel, FeId)) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInitFirmware: DibDriverSubbandSelect returned 0x%x" CRA, Status));
         goto End;
      }

      if((Status = DibDriverInitDemod(pContext, FeId)) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInitFirmware: DibDriverInitDemod returned 0x%x" CRA, Status));
         goto End;
      }

      /* hack to work around DibTuneMonit TimeOut issue with STK7078-MS-D4 and NIM9090MD*/
      if(FeId && (FeId == (pBoard->NbFrontends - 1)))
         DibMSleep( 500);

   }

   /* set HBM if appropriate */
   if((Status = DibDriverSetHbm(pContext, pBoard->Hbm)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInitFirmware: DibDriverSetHbm returned 0x%x" CRA, Status));
      goto End;
   }

   for(StreamNum = 0; StreamNum < pContext->NbStreams; StreamNum++)
   {
      /** [5] Time slicing on on all the chips by default */
      if((Status = DibDriverSetPowerManagement(pContext, eDIB_TRUE, &pContext->Stream[StreamNum])) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInitFirmware: DibDriverSetPowerManagement returned 0x%x" CRA, Status));
         goto End;
      }
   }

End:
   return Status;
}

/*
 * Main Initialization. This is all that needs to be called by the
 * system.
 * @param pContext      pointer to the context
 * @param BoardType     enum of the type of board (7078, ...)
 */
DIBSTATUS DibDriverInit(struct DibDriverContext *pContext, enum DibBoardType BoardType, BOARD_HDL BoardHdl)
{
   DIBSTATUS ret   = DIBSTATUS_ERROR;
   uint32_t  flags = 0;

   /** choose a default Board: take the first one */
   if(BoardType == eBOARD_DEFAULT)
      BoardType = eBOARD_7078;

   if(!pContext || BoardType >= eMAX_NB_BOARDS)
      return DIBSTATUS_INVALID_PARAMETER;

   pContext->DriverDebugMask = DEFAULT_DEBUG_MASK;

   if(pContext->Open)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: Driver already opened" CRA));
      return DIBSTATUS_ERROR;
   }
#if  (EMBEDDED_FLASH_SUPPORT == 0)
   if(pContext->Hd.BoardConfig == NULL)
   pContext->Hd.BoardConfig = BoardList[BoardType];
#endif

   /* Double check if board exists in case DibConfig was not selected correctly */
   if(pContext->Hd.BoardConfig == NULL)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "No Board available" CRA));
      return DIBSTATUS_ERROR;
   }

   pContext->DibChip         = pContext->Hd.BoardConfig->DibChip;
   pContext->BoardHdl        = BoardHdl;
   pContext->BoardType       = BoardType;

   /*  register chip operations */
   if((ret = IntDriverRegisterChip(pContext)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: Invalid IC type " CRA));
      goto End;
   }

   if((ret = IntDriverAllocMem(pContext)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: Failed to allocate memory" CRA));
      goto End;
   }

   /* init messaging part */
   SerialBufInit(&pContext->TxSerialBuf, pContext->TxBuffer, 32);

   /* bridge chip selection, MUST be DibDriverTargetInit target_init and before any use of the target */
   if((ret = DibDriverTargetInit(pContext)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibDriverTargetInit returned 0x%x" CRA, ret));
      goto End;
   }

   /* Flag Target as init */
   pContext->TargetInit = eDIB_TRUE;
   IntDriverInitSoft(pContext);

   if((ret = DibDriverFrontendInit(pContext)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibDriverFrontendInit returned 0x%x" CRA, ret));
      goto End;
   }
   /* Initialize register interface */
   DibDriverRegisterIfInit(pContext);

   /* Postinit configuration => first part */
   if( pContext->DibChip != DIB_NAUTILUS )
   {
      /* Use Hbm info to know if we should test the external (or uram) memory or not. */
      HBM_SELECT_SET(flags, pContext->Hd.BoardConfig->Hbm);
      if((ret = DibD2BPostInit(pContext, flags | SIOC_ENABLE_HS)) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibD2BPostInit returned 0x%x" CRA, ret));
         goto End;
      }
   }

#if (DIB_INTERNAL_MONITORING == 1)
   DibDriverDebugInit(pContext);
#endif

   /* Init internal driver */
   DibIntDriverInit(pContext);

   /* Reset embedded CPU(s) */
   DibDriverResetCpu(pContext);

   if ( pContext->DibChip == DIB_NAUTILUS )
   {
      /* Use Hbm info to know if we should test the external (or uram) memory or not. */
      HBM_SELECT_SET(flags, pContext->Hd.BoardConfig->Hbm);

#ifndef _WINDOWS
      /* enable HS for O1 & 02 */
      /* for O2s we will do it later because chip doesn't start at full speed */
      if ((pContext->Hd.BoardConfig->DibChipVersion == VERSION(1,0)) || 
          (pContext->Hd.BoardConfig->DibChipVersion == VERSION(2,0)))
#endif
              flags |= SIOC_ENABLE_HS;

      if((ret = DibD2BPostInit(pContext, flags)) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibD2BPostInit returned 0x%x" CRA, ret));
         goto End;
      }
   }

	DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: Start 14" CRA));
   /* load the microcode to the embedded processor(s) */
   if((ret = DibDriverUploadMicrocode(pContext)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibDriverUploadMicrocode returned 0x%x" CRA, ret));
      goto End;
   }

	DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: Start 15" CRA));
   /* init the CPU, mailbox initialisation can depend on it */
   DibDriverInitCpu(pContext);

   /*  init mailboxes */
   if((ret = DibDriverInitMailbox(pContext)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibDriverInitMailbox returned 0x%x" CRA, ret));
      goto End;
   }

   /* Start the processor(s) */
   DibDriverStartCpu(pContext);

   DibDriverEnableClearIrq(pContext);

   /* Postinit configuration => second part */
   if((ret = DibD2BPostInit(pContext, SIOC_ENABLE_IRQ)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibD2BPostInit returned 0x%x" CRA, ret));
      goto End;
   }

#if (EMBEDDED_FLASH_SUPPORT == 1)
   if (pContext->Hd.BoardConfig == BoardFlrdList[BoardType])
   {
      if (pContext->ChipOps.MacWaitForFirmware(pContext)!=DIBSTATUS_SUCCESS)
         goto End;
   }
#if ((WRITE_FLASH_SUPPORT == eWRFL_STATIC) || (WRITE_FLASH_SUPPORT == eWRFL_FILE))
   if (pContext->Hd.BoardConfig != BoardFlwrList[BoardType])
#endif
#endif
   {
   /* init demod, configure Gpio, Subband selection, set hbm Mode */
   if((ret = DibDriverInitFirmware(pContext)) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibDriverInitFirmware returned 0x%x" CRA, ret));
      goto End;
   }
   }
   if ( (pContext->Hd.BoardConfig->DibChip == DIB_VOYAGER) && (pContext->Hd.BoardConfig->Hbm == 2) )
   {
      /* eBOARD_STK29098MD, need to go through deinit/init to work around the reset problem */
      DibDriverDeInitFirmware(pContext);
      if((ret = DibDriverInitFirmware(pContext)) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibDriverInitFirmware returned 0x%x" CRA, ret));
         goto End;
      }
   }

   /* Mark device as opened */
   pContext->Open = eDIB_TRUE;

#ifndef _WINDOWS
   /* Enable HS for O2s */
   if ((pContext->Hd.BoardConfig->DibChipVersion == VERSION(2,1))) 
   {
       if((ret = DibD2BPostInit(pContext, SIOC_ENABLE_HS )) != DIBSTATUS_SUCCESS)
       {
           DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: DibD2BPostInit returned 0x%x" CRA, ret));
           goto End;
       }
   }
#endif
End:
   if(ret != DIBSTATUS_SUCCESS)
      DibDriverDeinit(pContext);

   return ret;
}

/**
 * Main De-Initialization. This is all that needs to be called by the system.
 * @param pContext      pointer to the context
 */
DIBSTATUS DibDriverDeinit(struct DibDriverContext * pContext)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   /* Clean up things in reverse allocation order */
   if(!pContext)
      return DIBSTATUS_INVALID_PARAMETER;

   /* Only if full init passed */
   if(pContext->Open)
   {
#if (DIB_INTERNAL_MONITORING == 1)
      DibDriverDebugExit(pContext);
#endif
      IntDriverDeinitSoft(pContext);

      DibDriverDeInitFirmware(pContext);

      pContext->Open = eDIB_FALSE;
   }

   /* */
   if(pContext->TargetInit == eDIB_TRUE)
   {
      pContext->TargetInit = eDIB_FALSE;

      if((Status = DibDriverTargetDeinit(pContext)) != DIBSTATUS_SUCCESS)
         DIB_DEBUG(SOFT_ERR, (CRB "DibDriverInit: Target Deinit Error" CRA));
   }

   /* Free memory */
   IntDriverFreeMem(pContext);

   return DIBSTATUS_SUCCESS;
}

/**
 * To be called before using the interface
 * This function initializes the storage
 * @param pContext      pointer to the context
 */
DIBSTATUS IntDriverAllocMem(struct DibDriverContext * pContext)
{
   DIBSTATUS Status = DIBSTATUS_INSUFFICIENT_MEMORY;
   uint8_t   Idx;

#if (USE_DRAGONFLY == 1)
   /* Allocate communication buffer */
   pContext->TxBuffer = (uint32_t *)DibMemAlloc((MAX_MSG_WORDS) * sizeof(uint32_t));
   if(pContext->TxBuffer == NULL)
      goto End;
#endif

   /* Monitoring lock */
   DibAllocateLock(&pContext->MonitLock);
   if(&pContext->MonitLock == NULL)
      goto End;

   DibInitLock(&pContext->MonitLock);

   /* Channel lock */
   DibAllocateLock(&pContext->ChannelLock);
   if(&pContext->ChannelLock == NULL)
      goto End;

   DibInitLock(&pContext->ChannelLock);

   /* Signal Monitoring lock */
   DibAllocateLock(&pContext->SignalMonitLock);

   if(&pContext->SignalMonitLock == NULL)
      goto End;

   DibInitLock(&pContext->SignalMonitLock);

   /* Brige Read write lock */
   DibAllocateLock(&pContext->BridgeRWLock);
   if(&pContext->BridgeRWLock == NULL)
      goto End;

   DibInitLock(&pContext->BridgeRWLock);

   /* Allocate Events */
   for(Idx = 0; Idx < DIB_MAX_NB_CHANNELS ; Idx++)
   {
      DibAllocateEvent(&pContext->ChannelInfo[Idx].MonitContext.MonitEvent);
      if(&pContext->ChannelInfo[Idx].MonitContext.MonitEvent == NULL)
         goto End;

      DibAllocateEvent(&pContext->ChannelInfo[Idx].ScanEvent);
      if(&pContext->ChannelInfo[Idx].ScanEvent == NULL)
         goto End;
   }

   DibAllocateEvent(&pContext->ChipContext.GlobalMonitEvent);
   if(&pContext->ChipContext.GlobalMonitEvent == NULL)
         goto End;

   DibAllocateEvent(&pContext->BridgeRWEvent);
   if(&pContext->BridgeRWEvent == NULL)
         goto End;

   DibAllocateEvent(&pContext->SvcChangeEvent);
   if(&pContext->SvcChangeEvent == NULL)
         goto End;

   DibAllocateEvent(&pContext->FWDwldEvent);
   if(&pContext->FWDwldEvent == NULL)
         goto End;

   DibAllocateEvent(&pContext->MsgAckEvent);
   if(&pContext->MsgAckEvent == NULL)
         goto End;

   DibAllocateEvent(&pContext->InfoContext.InfoEvent);
   if(&pContext->InfoContext.InfoEvent == NULL)
         goto End;

   /* Everything has been allocated correctly */
   Status = DIBSTATUS_SUCCESS;

End:
   return Status;
}

/**
 * To be called before exiting
 * This function frees the SI/PSI storage
 * @param pContext      pointer to the context
 */
void IntDriverFreeMem(struct DibDriverContext * pContext)
{
   uint32_t loop;

#if (USE_DRAGONFLY == 1)
   DibMemFree(pContext->TxBuffer, (MAX_MSG_WORDS) * sizeof(uint32_t));
#endif

   if(&pContext->MsgAckEvent == NULL)
      DibDeAllocateEvent(&pContext->MsgAckEvent);

   if(&pContext->FWDwldEvent == NULL)
      DibDeAllocateEvent(&pContext->FWDwldEvent);

   if(&pContext->SvcChangeEvent)
      DibDeAllocateEvent(&pContext->SvcChangeEvent);

   if(&pContext->BridgeRWEvent)
      DibDeAllocateEvent(&pContext->BridgeRWEvent);

   if(&pContext->ChipContext.GlobalMonitEvent)
      DibDeAllocateEvent(&pContext->ChipContext.GlobalMonitEvent);

   for(loop = 0; loop < DIB_MAX_NB_CHANNELS; loop++)
   {
      if(&pContext->ChannelInfo[loop].ScanEvent)
         DibDeAllocateEvent(&pContext->ChannelInfo[loop].ScanEvent);

      if(&pContext->ChannelInfo[loop].MonitContext.MonitEvent)
         DibDeAllocateEvent(&pContext->ChannelInfo[loop].MonitContext.MonitEvent);
   }

   if(&pContext->BridgeRWLock)
      DibDeAllocateLock(&pContext->BridgeRWLock);

   if(&pContext->SignalMonitLock)
      DibDeAllocateLock(&pContext->SignalMonitLock);

   if(&pContext->ChannelLock)
      DibDeAllocateLock(&pContext->ChannelLock);

   if(&pContext->MonitLock)
      DibDeAllocateLock(&pContext->MonitLock);

   if(&pContext->InfoContext.InfoEvent)
      DibDeAllocateEvent(&pContext->InfoContext.InfoEvent);
}

/**
 * Initialisation of the structure ItemInfo for a specific item
 * @param pContext      pointer to the context
 * @param idx           index of the item to initialise
 */
static void DibInitItem(struct DibDriverContext * pContext, ELEM_HDL idx)
{
   DIB_ASSERT(idx < DIB_MAX_NB_ITEMS);
   DibZeroMemory(&(pContext->ItemInfo[idx]), sizeof(struct DibDriverItem));

   pContext->ItemInfo[idx].NextItem     = DIB_UNSET;
   pContext->ItemInfo[idx].ParentFilter = DIB_UNSET;
   pContext->ItemInfo[idx].FlushAcq     = 0;

#if (USE_FIREFLY == 1)
   pContext->ItemInfo[idx].ServId       = DIB_UNSET;
#endif
}

/**
 * Initialisation of the structure FilterInfo for a specific filter
 * @param pContext      pointer to the context
 * @param idx           index of the filter to initialise
 */
static void DibInitFilter(struct DibDriverContext * pContext, FILTER_HDL idx)
{
   DIB_ASSERT(idx < DIB_MAX_NB_FILTERS);

   DibZeroMemory(&(pContext->FilterInfo[idx]), sizeof(struct DibDriverFilter));

   pContext->FilterInfo[idx].ParentChannel= DIB_UNSET;
   pContext->FilterInfo[idx].FirstItem    = DIB_UNSET;
   pContext->FilterInfo[idx].NextFilter   = DIB_UNSET;
   pContext->FilterInfo[idx].FirstBuf     = DIB_UNSET;
   pContext->FilterInfo[idx].TypeFilter   = eUNKNOWN_DATA_TYPE;
   pContext->FilterInfo[idx].FlushActive  = eDIB_FALSE;
}

/**
 * Initialisation of the structure Channel for a specific Channel
 * @param pContext      pointer to the context
 * @param idx           index of the filter to initialise
 */
static void DibInitChannel(struct DibDriverContext * pContext, CHANNEL_HDL idx)
{
   DIB_ASSERT(idx < DIB_MAX_NB_CHANNELS);

   pContext->ChannelInfo[idx].ChannelDescriptor.RFkHz     = 0;
   pContext->ChannelInfo[idx].ChannelMonit.ChannelIndex   = (uint8_t)idx;
   pContext->ChannelInfo[idx].ChannelMonit.NbDemods       = 0;  /**< configure in the creation of the channel */
   pContext->ChannelInfo[idx].Type                        = eSTANDARD_UNKNOWN;
   pContext->ChannelInfo[idx].FirstFilter                 = DIB_UNSET;
   pContext->ChannelInfo[idx].InUse                       = 0;
   pContext->ChannelInfo[idx].Temporary                   = eDIB_FALSE;
   pContext->ChannelInfo[idx].MinNbDemod                  = DIB_DEMOD_AUTO;
   pContext->ChannelInfo[idx].MaxNbDemod                  = DIB_DEMOD_AUTO;
   pContext->ChannelInfo[idx].StreamParameters         = 0;
   pContext->ChannelInfo[idx].IsDataChannel               = eDIB_FALSE;
   pContext->ChannelInfo[idx].pStream                     = NULL;
}

void DibDriverPostEvent(struct DibDriverContext * pContext, struct DibDriverEventInfoUp * pDataInfo)
{
   uint8_t CallbackIndex = pContext->EventTypeCallbackIndex[pDataInfo->EventType];
   DibGetEventCallback pCallBack;

   /** If no callback set on this event, return */
   if(CallbackIndex == DIB_UNSET)
      return;

   /** Verification : callback must exist */
   DIB_ASSERT(pContext->EventCallbackDesc[CallbackIndex].List != 0);
   pCallBack = pContext->EventCallbackDesc[CallbackIndex].List;

   /** Call it now */
   pCallBack(pDataInfo->EventType, &(pDataInfo->EventConfig), pContext->EventCallbackDesc[CallbackIndex].pContext);
}

/**
 * Initializes all Data structures and pointers.
 * @param pContext      pointer to the context
 */
void IntDriverInitSoft(struct DibDriverContext * pContext)
{
   uint8_t                 loop;
   struct DibDriverChipCtx *pFF;

   pFF = &pContext->ChipContext;

   DibInitNotificationEvent(&pFF->GlobalMonitEvent);

   /*  Init members of the main device context */
   DibInitNotificationEvent(&pContext->FWDwldEvent);
   DibInitNotificationEvent(&pContext->BridgeRWEvent);
   DibInitNotificationEvent(&pContext->SvcChangeEvent);
   DibInitNotificationEvent(&pContext->MsgAckEvent);
   DibInitNotificationEvent(&pContext->InfoContext.InfoEvent);

#if (USE_FIREFLY == 1)
   /* Firefly specific lookup */
   for(loop = 0; loop < DIB_MAX_NB_SERVICES; loop++)
   {
      pContext->FireflyService[loop].ValidPid = PID_FILTER_EMPTY;
   }
#endif
   DibInitNotificationEvent(&pFF->GlobalMonitEvent);

   /* IP crypto block   */
   /* BRIDGE            */
   /* ip_crypto_init(); */
   /* channels and filters structures */
   pContext->MsgAckStatus = 0;

   /* Set Default Values */

#if (USE_FIREFLY == 1)
   pContext->FireflyDiversityActiv        = DIB_UNSET;
   pContext->FireflyChipTimeSlicing       = eDIB_TRUE;
#endif

   /* Layout information */
   pContext->NbChips           = 1;
   pContext->NbStreams         = 1;
   pContext->NbFrontends       = 1;
   pContext->NbCmmbSvc         = 0;

   /* CMMB */
   DibInitChannelDecoder(pContext);

   for(loop = 0; loop < DIB_MAX_NB_OF_STREAMS; loop++)
   {
      DibZeroMemory(&pContext->Stream[loop], sizeof(struct DibStream));
      pContext->Stream[loop].StreamId = loop;
   }

   for(loop = 0; loop < DIB_MAX_NB_OF_FRONTENDS; loop++)
   {
      DibZeroMemory(&pContext->Frontend[loop], sizeof(struct DibFrontend));
      pContext->Frontend[loop].FeId = loop;
   }

   for(loop = 0; loop < DIB_MAX_NB_CHANNELS; loop++)
   {
      DibInitChannel(pContext, loop);
      DibInitNotificationEvent(&pContext->ChannelInfo[loop].MonitContext.MonitEvent);

      pContext->ChannelInfo[loop].ChannelDescriptor.RFkHz   = 0;
      pContext->ChannelInfo[loop].ChannelMonit.ChannelIndex = (uint8_t)loop;
      pContext->ChannelInfo[loop].ChannelMonit.NbDemods     = 0;  /**< configure in the creation of the channel */

      DibInitNotificationEvent(&pContext->ChannelInfo[loop].ScanEvent);

      pContext->ChannelInfo[loop].Type        = eSTANDARD_UNKNOWN;
      pContext->ChannelInfo[loop].FirstFilter = DIB_UNSET;
      pContext->ChannelInfo[loop].InUse       = 0;
      pContext->ChannelInfo[loop].Temporary   = eDIB_FALSE;
   }

#if (mSDK == 0)
   /* RAW TS packets multiplex context. */
   for(loop = 0; loop < DIB_MAX_NB_FILTERS; loop++)
   {
      DibInitFilter(pContext, loop);
   }

   for(loop = 0; loop < DIB_MAX_NB_ITEMS; loop++)
   {
      DibInitItem(pContext, loop);
   }

#if (USE_FIREFLY == 1)
   for(loop = 0; loop < DIB_MAX_NB_TS_CHANNELS; loop++)
   {
      pContext->FireflyChannel[loop] = DIB_UNSET;
   }
#endif

   for(loop = 0; loop < DIB_MAX_NB_BUF_CONTEXT; loop++)
   {
      pContext->CallbackBufCtx[loop].Ctx.BufAdd    = 0;
      pContext->CallbackBufCtx[loop].Ctx.BufSize   = 0;
      pContext->CallbackBufCtx[loop].Ctx.FillSize  = 0;
      pContext->CallbackBufCtx[loop].Ctx.FilterIdx = DIB_UNSET;
      pContext->CallbackBufCtx[loop].Ctx.BufId     = DIB_UNSET;
      pContext->CallbackBufCtx[loop].NextBuf       = DIB_UNSET;
   }
   for(loop = 0; loop < eMAX_NB_EVENTS; loop++)
   {
      pContext->EventCallbackDesc[loop].List = 0;
      pContext->EventCallbackDesc[loop].pContext = 0;
      pContext->EventTypeCallbackIndex[loop] = DIB_UNSET;
   }
#endif

   /* Info context initialization */
   pContext->InfoContext.Data.Efuse.Status = DIBSTATUS_ERROR;
   pContext->InfoContext.Data.Efuse.IsRead = 0;

   pContext->InfoContext.Data.Sdram.Status = DIBSTATUS_ERROR;
   pContext->InfoContext.Data.Sdram.State = eINFO_SDRAM_UNKNOW;
   pContext->InfoContext.Data.Sdram.IsUsedForCaching = 0;

   pContext->InfoContext.Data.Gpio.Status = DIBSTATUS_ERROR;
   pContext->InfoContext.Data.Gpio.Config.Num = 0;
   pContext->InfoContext.Data.Gpio.Config.Val = 0;
   memset(pContext->InfoContext.Data.Gpio.Config.Ratios, 0, sizeof(pContext->InfoContext.Data.Gpio.Config.Ratios));
   memset(pContext->InfoContext.Data.Gpio.Config.Pwm, 0, sizeof(pContext->InfoContext.Data.Gpio.Config.Pwm));
   pContext->InfoContext.Data.Gpio.Config.Dir = 0;

   pContext->InfoContext.Set.Status = DIBSTATUS_ERROR;
   pContext->InfoContext.Set.Gpio.Num = 0;
   pContext->InfoContext.Set.Gpio.Val = 0;
   memset(pContext->InfoContext.Set.Gpio.Ratios, 0, sizeof(pContext->InfoContext.Data.Gpio.Config.Ratios));
   memset(pContext->InfoContext.Set.Gpio.Pwm, 0, sizeof(pContext->InfoContext.Data.Gpio.Config.Pwm));
   pContext->InfoContext.Set.Gpio.Dir = 0;
}

/**
 * De-Initilizes all Data structures and pointers.
 * @param pContext      pointer to the context
 */
void IntDriverDeinitSoft(struct DibDriverContext * pContext)
{
   uint32_t                 loop;
   DIBSTATUS                rc = DIBSTATUS_ERROR;
   struct DibDriverChipCtx *pFF;

   pFF = &pContext->ChipContext;

   /** Remove recursively every channels still not desallocated */
   for(loop = 0; loop < DIB_MAX_NB_CHANNELS; loop++)
   {
      if(pContext->ChannelInfo[loop].InUse)
      {
         rc = DibDriverDeleteChannel(pContext, loop);
      }
   }

#if (mSDK == 0)
   /* RAW TS packets multiplex context. */
   for(loop = 0; loop < DIB_MAX_NB_FILTERS; loop++)
   {
      pContext->FilterInfo[loop].NbActivePids = 0;
      /* Clear Dvbt monitoring */
      DibZeroMemory(&pContext->FilterInfo[loop].DvbTMonitoring, sizeof(struct DvbTMonit));
   }
#endif
}

/**
* remove an item and relink the other items and the associated filter without it.
* @param pContext   current context
* @param ItemIdx    item to remove
*/
#if (mSDK == 0)
void IntDriverRemoveItem(struct DibDriverContext *pContext, ELEM_HDL ItemHdl)
{
   struct DibDriverItem   *pItem;
   struct DibDriverFilter *pFilt;
   ELEM_HDL                NextItem;

   DibDriverTargetDisableIrqProcessing(pContext);

   pItem    = &(pContext->ItemInfo[ItemHdl]);
   pFilt    = &(pContext->FilterInfo[pItem->ParentFilter]);
   NextItem = pItem->NextItem;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pItem->ParentFilter < DIB_MAX_NB_FILTERS);

   if(pFilt->FirstItem == ItemHdl)
   {
      pFilt->FirstItem = NextItem;
   }
   else
   {
      pItem = &(pContext->ItemInfo[pFilt->FirstItem]);

      while(pItem->NextItem != ItemHdl)
      {
         pItem = &(pContext->ItemInfo[pItem->NextItem]);
      }

      pItem->NextItem = NextItem;
   }

   DibInitItem(pContext, ItemHdl);
   DibDriverTargetEnableIrqProcessing(pContext);
}

/**
* remove a filter and relink the other filters and the associated channel without it.
* @param pContext   current context
* @param FilterHdl  filter to remove
*/
void IntDriverRemoveFilter(struct DibDriverContext * pContext, FILTER_HDL FilterHdl)
{
   struct DibDriverFilter  *pFilt;
   struct DibDriverChannel *pCh;
   FILTER_HDL               NextFilter;
   DIBSTATUS                rc = DIBSTATUS_ERROR;

   DibDriverTargetDisableIrqProcessing(pContext);

   pFilt      = &(pContext->FilterInfo[FilterHdl]);
   pCh        = &(pContext->ChannelInfo[pFilt->ParentChannel]);
   NextFilter = pFilt->NextFilter;

   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(pFilt->ParentChannel < DIB_MAX_NB_CHANNELS);

   /** Give back every buffer to the user */
   rc = DibDriverFlushBuffers(pContext, FilterHdl);
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);

   /** keep the link between filters */
   if(pCh->FirstFilter == FilterHdl)
   {
      pCh->FirstFilter = NextFilter;
   }
   else
   {
      pFilt = &(pContext->FilterInfo[pCh->FirstFilter]);

      while(pFilt->NextFilter != FilterHdl)
      {
         pFilt = &(pContext->FilterInfo[pFilt->NextFilter]);
      }

      pFilt->NextFilter = NextFilter;
   }

   /** clear the struct DibDriverFilter structure */
   DibInitFilter(pContext, FilterHdl);
   DibDriverTargetEnableIrqProcessing(pContext);
}
#endif

/**
* remove a channel
* @param pContext   current context
* @param ChannelHdl  channel to remove
*/
void IntDriverRemoveChannel(struct DibDriverContext * pContext, CHANNEL_HDL ChannelHdl, enum DibBool Locked)
{
   struct DibDriverChannel *pCh = NULL;

   if(!Locked)
      DibDriverTargetDisableIrqProcessing(pContext);

   pCh = &(pContext->ChannelInfo[ChannelHdl]);

   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);
   DIB_ASSERT(pCh->FirstFilter == DIB_UNSET);

   /** clear the struct DibDriverFilter structure */
   DibInitChannel(pContext, ChannelHdl);

   if(!Locked)
      DibDriverTargetEnableIrqProcessing(pContext);
}

/**
 * Fill the ChannelInfo structure of a new channel
 * @param pContext         pointer to the context
 * @param ChannelHdl       handler of the channel concerned
 * @param MinNbDemod       min number of demodulation for this channel
 * @param MaxNbDemod       max number of demodulation for this channel
 * @param StreamParameters  StreamParameters of this channel
 * @param pDescriptor      pointer to a structure with all the useful information for the configuration of the channel
 */
void IntDriverAddChannel(struct DibDriverContext * pContext, CHANNEL_HDL ChannelHdl, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType Type, struct DibTuneChan * pChannelDescriptor)
{
   struct DibDriverChannel *pCh;

   DibDriverTargetDisableIrqProcessing(pContext);

   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);
   DIB_ASSERT(pChannelDescriptor);

   pCh = &(pContext->ChannelInfo[ChannelHdl]);

   DIB_ASSERT(pCh);
   DIB_ASSERT(pCh->InUse == 0);

   pCh->Type       = Type;
   pCh->MinNbDemod = MinNbDemod;
   pCh->MaxNbDemod = MaxNbDemod;

   DibMoveMemory(&(pCh->ChannelDescriptor), pChannelDescriptor, sizeof(struct DibTuneChan));

   pCh->FirstFilter       = DIB_UNSET;
   pCh->InUse             = 1;
   pCh->StreamParameters = StreamParameters;
   pCh->IsDataChannel     = eDIB_FALSE;
   pCh->Temporary         = eDIB_FALSE;

   DibDriverTargetEnableIrqProcessing(pContext);
}

#if (mSDK == 0)
/**
 * Fill the new FilterInfo structure of a specific filter added to a channel already defined
 * @param pContext         pointer to the context
 * @param FilterHdl        handler of the concerned filter
 * @param ChannelHdl       handler of the associated channel
 * @param DataType         Type of the filter
 * @param pDataBuf         pointer to a structure with the parameter of the data path
 */
void IntDriverAddFilter(struct DibDriverContext * pContext, FILTER_HDL FilterHdl, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer * pDataBuf)
{
   struct DibDriverChannel *pCh   = NULL;
   struct DibDriverFilter  *pFilt = NULL;

   DibDriverTargetDisableIrqProcessing(pContext);

   pCh   = &pContext->ChannelInfo[ChannelHdl];
   pFilt = &(pContext->FilterInfo[FilterHdl]);

   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);
   DIB_ASSERT(DataType != eUNKNOWN_DATA_TYPE);

   pFilt->TypeFilter    = DataType;
   pFilt->ParentChannel = ChannelHdl;
   pFilt->NextFilter    = DIB_UNSET;
   pFilt->FirstItem     = DIB_UNSET;

   if(pDataBuf)
   {
      DibMoveMemory(&(pFilt->DataInfo), pDataBuf, sizeof(struct DibDataBuffer));
   }
   else
   {
      /* Use default values */
      pFilt->DataInfo.DataMode         =  eIP;
      pFilt->DataInfo.CallbackUserCtx  =  0;
      pFilt->DataInfo.CallbackFunc     =  0;
      pFilt->DataInfo.Timeout          = -1;
   }

   if(pCh->FirstFilter == DIB_UNSET)
   {
      pCh->FirstFilter = FilterHdl;
   }
   else
   {
      pFilt = &(pContext->FilterInfo[pCh->FirstFilter]);

      while(pFilt->NextFilter != DIB_UNSET)
      {
         pFilt = &(pContext->FilterInfo[pFilt->NextFilter]);
      }

      pFilt->NextFilter = FilterHdl;
   }

   DibDriverTargetEnableIrqProcessing(pContext);
}

/**
 * Fill the ItemInfo structure of a new item added to a filter already defined
 * @param pContext      pointer to the current context
 * @param ItemHdl       handler of the concerned item
 * @param FilterHdl     associated filter
 * @param pFilterDesc   pointer to the item to add structure of information, which is an union depending of the Type of filter
 */
void IntDriverAddItem(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, FILTER_HDL FilterHdl, union DibFilters * pFilterDesc)
{
   struct DibDriverFilter *pFilt;
   struct DibDriverItem   *pItem;

   DibDriverTargetDisableIrqProcessing(pContext);

   pFilt = &(pContext->FilterInfo[FilterHdl]);
   pItem = &(pContext->ItemInfo[ItemHdl]);

   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pFilterDesc);

   DibMoveMemory(&pItem->Config, pFilterDesc, sizeof(union DibFilters));

   pItem->ParentFilter = FilterHdl;
   pItem->NextItem     = DIB_UNSET;
   pItem->AliasItem    = DIB_UNSET;

   if(pFilt->FirstItem == DIB_UNSET)
   {
      pFilt->FirstItem = ItemHdl;
   }
   else
   {
      pItem = &(pContext->ItemInfo[pFilt->FirstItem]);

      while(pItem->NextItem != DIB_UNSET)
      {
         pItem = &(pContext->ItemInfo[pItem->NextItem]);
      }

      pItem->NextItem = ItemHdl;
   }
   DibDriverTargetEnableIrqProcessing(pContext);
}

/**
 * Link to the callback. Internal use.
 * @param pContext      pointer to the current context
 * @param pDriverCtx    pointer to the structure with the driver context information
 * @return DIBSTATUS    status
 */
DIBSTATUS DibDriverDataCallback(struct DibDriverContext * pContext, struct DibBufContext * pDriverCtx, enum DibDataStatus DataStatus)
{
   void(*dataCallback) (void *, struct DibBufContext *, enum DibDataStatus);
   void *dataContext;

   DIB_ASSERT(pContext);
   DIB_ASSERT(pDriverCtx);

   dataCallback = pContext->FilterInfo[pDriverCtx->FilterIdx].DataInfo.CallbackFunc;
   dataContext  = pContext->FilterInfo[pDriverCtx->FilterIdx].DataInfo.CallbackUserCtx;

   if(dataCallback == NULL)
   {
      return DIBSTATUS_ERROR;
   }

   dataCallback(dataContext, pDriverCtx, DataStatus);
   return DIBSTATUS_SUCCESS;
}


/**
 * Recuperation of a Buffer to store Data.
 * @param pContext      pointer to the current context
 * @param FilterIdx     filter number concerned for the Buffer
 * @param pDriverCtx    pointer to the structure with all needed information stored
 * @param GoToNext      if true, go to the next buffer
 * @return DIBSTATUS    status
 * WARNING : the caller has to allocate (but not to initialize) the structure, this function copies the information and unregister the Buffer automatically
 */
DIBSTATUS DibDriverGetNewBuffer(struct DibDriverContext * pContext, FILTER_HDL FilterHdl, struct DibBufContext * pDriverCtx, uint8_t GoToNext)
{
   uint8_t idx;

   /* !!! Caller must have disabled driver irq when calling this function */
   DIB_ASSERT(pDriverCtx);
   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);

   idx = pContext->FilterInfo[FilterHdl].FirstBuf;

   if(idx == DIB_UNSET)
   {
      DIB_DEBUG(PACKET_LOG, (CRB "" CRA));
      DIB_DEBUG(PACKET_LOG, (CRB "DibDriverGetNewBuffer : Resources Pb" CRA));
      return DIBSTATUS_RESOURCES;
   }

   if(idx >= DIB_MAX_NB_BUF_CONTEXT)
   {
      DIB_DEBUG(PACKET_ERR, (CRB "" CRA));
      DIB_DEBUG(PACKET_ERR, (CRB "DibDriverGetNewBuffer : Error" CRA));
      return DIBSTATUS_ERROR;
   }

   DibMoveMemory(pDriverCtx, &(pContext->CallbackBufCtx[idx].Ctx), sizeof(struct DibBufContext));

   if(GoToNext)
   {
      pContext->FilterInfo[FilterHdl].FirstBuf = pContext->CallbackBufCtx[idx].NextBuf;

      DibZeroMemory(&(pContext->CallbackBufCtx[idx].Ctx), sizeof(struct DibBufContext));

      pContext->CallbackBufCtx[idx].NextBuf        = DIB_UNSET;
      pContext->CallbackBufCtx[idx].Ctx.FilterIdx  = DIB_UNSET;
   }

   DIB_DEBUG(PACKET_LOG, (CRB "" CRA));
   DIB_DEBUG(PACKET_LOG, (CRB "DibDriverGetNewBuffer : Success" CRA));
   return DIBSTATUS_SUCCESS;
}

/**
 * Give back to the user every buffer of the filter.
 * This function is usefull when removing a filter.
 * @param[in] pContext: driver context structure
 * @param[in] FilterIdx: The filter handler to remove
 * @return: SUCCESS if flush is finished correctly
 */
DIBSTATUS DibDriverFlushBuffers(struct DibDriverContext * pContext, FILTER_HDL FilterIdx)
{
   struct DibBufContext BuffCtx;
   uint8_t BufferIdx, NextBufferIdx;

   /* Driver Lock must be held by caller - in this case IntDriverRemoveFilter */
   BuffCtx.BufSize = 0;
   BuffCtx.ItemHdl = DIB_UNSET;
   BuffCtx.FilterIdx = FilterIdx;

   NextBufferIdx = pContext->FilterInfo[FilterIdx].FirstBuf;
   while(NextBufferIdx != DIB_UNSET)
   {
      BufferIdx = NextBufferIdx;
      DIB_ASSERT(BufferIdx < DIB_MAX_NB_BUF_CONTEXT);

      NextBufferIdx  = pContext->CallbackBufCtx[BufferIdx].NextBuf;
      BuffCtx.BufId  = pContext->CallbackBufCtx[BufferIdx].Ctx.BufId;
      BuffCtx.BufAdd = pContext->CallbackBufCtx[BufferIdx].Ctx.BufAdd;

      DIB_ASSERT(BuffCtx.BufAdd);

      DibZeroMemory(&(pContext->CallbackBufCtx[BufferIdx].Ctx), sizeof(struct DibBufContext));

      pContext->CallbackBufCtx[BufferIdx].NextBuf        = DIB_UNSET;
      pContext->CallbackBufCtx[BufferIdx].Ctx.FilterIdx  = DIB_UNSET;

      /** Give back the buffer to the user */
      DIB_DEBUG(PACKET_LOG, (CRB "Buffer %d returned to the user" CRA,BuffCtx.BufId));
      DibDriverDataCallback(pContext, &BuffCtx, eREMOVED);
   }

   pContext->FilterInfo[FilterIdx].FirstBuf = DIB_UNSET;

   return DIBSTATUS_SUCCESS;
}
#endif

/**
* Swap every 16 bits words of a buffer. This is needed if the host interface of the chip requires that the data are sent
* using msb first.
*/
void DibDriverSwap16(uint8_t * Buf, int size)
{
   uint8_t reg;
   size -= 2;

   while(size >= 0)
   {
      reg         = Buf[size];
      Buf[size]   = Buf[size+1];
      Buf[size+1] = reg;
      size       -= 2;
   }
}

/**
* Swap every 32 bits words of a buffer. This is needed if the host interface of the chip requires that the data are sent
* using msb first.
*/
void DibDriverSwap32(uint8_t * Buf, int size)
{
   uint8_t reg;
   size -= 4;
   while(size >= 0)
   {
      reg         = Buf[size];
      Buf[size]   = Buf[size+3];
      Buf[size+3] = reg;
      reg         = Buf[size+1];
      Buf[size+1] = Buf[size+2];
      Buf[size+2] = reg;
      size       -= 4;
   }
}

/**
 * Calculate crc32 using MPEG2 32 bit Crc
 */
#if (mSDK == 0)
uint32_t IntDriverGetCrc32(void *pTable, uint32_t bufferLength, uint32_t crc32bis)
{
   uint8_t  SectionByte;

   uint32_t k;
   uint8_t *section;

   static uint32_t crc32_table[256] =
   {
      0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
      0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
      0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
      0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
      0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
      0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
      0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
      0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
      0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
      0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
      0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
      0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
      0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
      0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
      0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
      0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
      0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
      0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
      0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
      0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
      0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
      0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
      0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
      0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
      0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
      0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
      0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
      0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
      0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
      0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
      0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
      0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
      0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
      0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
      0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
      0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
      0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
      0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
      0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
      0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
      0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
      0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
      0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
   };

   section = (uint8_t *) pTable;

   for(k = 0; k < bufferLength; k++)
   {
      SectionByte = section[k];

      crc32bis = (crc32bis << 8) ^ crc32_table[((crc32bis >> 24) ^ section[k]) & 0xff];
   }
   return crc32bis;
}
#endif


