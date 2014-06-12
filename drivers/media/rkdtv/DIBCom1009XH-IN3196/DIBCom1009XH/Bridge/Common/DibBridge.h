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
* @file "DibBridge.h"
* @brief Bridge functionality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_H
#define DIB_BRIDGE_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define DIB_FIREFLY             0
/*#define DIB_DRAGONFLY           1*/
#define DIB_VOYAGER             2
#define DIB_NAUTILUS            3

#define HBM_SELECT_GET(flag)          ( ((flag)>>23)&0x1 )

#include "DibBridgeCommon.h" /* for DmaFlags */

DIBSTATUS DibBridgeInit(struct DibBridgeContext *pContext, BOARD_HDL BoardHdl, uint32_t ChipSelect, uint32_t * Config, uint32_t DebugMask);
DIBSTATUS DibBridgeDeinit(struct DibBridgeContext *pContext);
DIBSTATUS DibBridgePostInit(struct DibBridgeContext *pContext, uint32_t Flag);
void      DibBridgeEnableIrq(struct DibBridgeContext *pContext);
DIBSTATUS DibBridgeReadReg8(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t *Value);
DIBSTATUS DibBridgeReadReg16(struct DibBridgeContext *pContext, uint32_t Addr, uint16_t *Value);
DIBSTATUS DibBridgeReadReg32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Value);
DIBSTATUS DibBridgeRead32Reg32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Value);
DIBSTATUS DibBridgeWriteReg8(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t Value);
DIBSTATUS DibBridgeWriteReg16(struct DibBridgeContext *pContext, uint32_t Addr, uint16_t Value);
DIBSTATUS DibBridgeWriteReg32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t Value);
DIBSTATUS DibBridgeReadBuffer32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Buffer, uint32_t Count);
DIBSTATUS DibBridgeRead32Buffer32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t *Buffer, uint32_t Count);
DIBSTATUS DibBridgeWriteBuffer32(struct DibBridgeContext *pContext, uint32_t Addr, uint32_t * Buffer, uint32_t Count);
DIBSTATUS DibBridgeWrite (struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint8_t * Buf, uint32_t Cnt);
DIBSTATUS DibBridgeRead (struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint8_t * Buf, uint32_t Cnt);
DIBSTATUS DibBridgeRegisterChipIf(struct DibBridgeContext *pContext, uint32_t * Config);

DIBSTATUS DibBridgeSetHostIfMode(struct DibBridgeContext *pContext, enum DibBridgeHostIfMode HostIfMode);
DIBSTATUS DibBridgeSetInfo(struct DibBridgeContext *pContext, enum DibBridgeInfoType BridgeInfoType, union DibInformBridge * pInfoBridge);

void DibBridgeSwap16(uint8_t * Buf, int size);
void DibBridgeSwap32(uint8_t * Buf, int size);


#define DibBridgeSetupDma(pContext, pDmaCtx)                 pContext->BridgeChipOps.SetupDma(pContext, pDmaCtx)
#define DibBridgeRequestDma(pContext, pDmaCtx)               pContext->BridgeChipOps.RequestDma(pContext, pDmaCtx)
#define DibBridgeTestConfiguration(pContext)                 (pContext->BridgeChipOps.SetHostIfMode ? 1 : 0)

#define DibBridgeAssembleSliceSpec(pContext, flags, offs, len) pContext->BridgeChipOps.AssembleSlice(pContext, flags, offs, len)
#define DibBridgeGetArch(pContext)                             pContext->BridgeChipOps.GetArch(pContext)
#define DibBridgePreFormat(pContext, ByteMode, Addr, IsWriteAccess, Buf, Cnt) \
                                                             pContext->BridgeChipOps.PreFormat(pContext, ByteMode, Addr, IsWriteAccess, Buf, Cnt)
#define DibBridgePostFormat(pContext, ByteMode, Addr, IsWriteAccess, Buf, Cnt) \
                                                             pContext->BridgeChipOps.PostFormat(pContext, ByteMode, Addr, IsWriteAccess, Buf, Cnt)
#define DibBridgeIncrementFormattedAddress(pContext, InFmtAddr, Offset) \
                                                             pContext->BridgeChipOps.IncrementFormattedAddress(pContext,  InFmtAddr, Offset)
#define DibBridgeSetService(pContext, Svc, ItemHdl, FilterHdl, DataType, DataMode) \
                                       pContext->BridgeChipOps.SetService(pContext, Svc, ItemHdl, FilterHdl, DataType, DataMode)

#define DibBridgeSignalBufFail(pContext, pFlags, Flush)      pContext->BridgeChipOps.SignalBufFail(pContext, pFlags, Flush)
#define DibBridgeProcessIrq(pContext)                        pContext->BridgeChipOps.ProcessIrq(pContext)
#define DibBridgeProcessDma(pContext, pDmaCtx)               pContext->BridgeChipOps.ProcessDma(pContext, pDmaCtx)
#define DibBridgeSendMsgSpec(pContext, Data, len)            pContext->BridgeChipOps.SendMsg(pContext, Data, len)
#define DibBridgeSendAck(pContext, pFlags, failed)           pContext->BridgeChipOps.SendAck(pContext, pFlags, failed)
#define DibBridgeChipsetInit(pContext)                       pContext->BridgeChipOps.ChipsetInit(pContext)
#define DibBridgeChipsetDeinit(pContext)                     pContext->BridgeChipOps.ChipsetDeinit(pContext)

#if (DIB_CHECK_DATA == 1)
#define DibBridgeClearCheckStats(pContext,buf)               pContext->BridgeChipOps.ClearCheckStats(pContext,buf)
#define DibBridgeForwardCheckStats(pContext,item)            pContext->BridgeChipOps.ForwardCheckStats(pContext,item)
#else
#define DibBridgeClearCheckStats(pContext,buf)
#define DibBridgeForwardCheckStats(pContext,item)
#endif


#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT  == 1))
#define IntBridgeTestBasicRead(pContext)                      pContext->BridgeChipOps.TestBasicRead(pContext)
#define IntBridgeTestRegister(pContext)                       pContext->BridgeChipOps.TestRegister(pContext)
#define IntBridgeTestInternalRam(pContext)                    pContext->BridgeChipOps.TestInternalRam(pContext)
#define IntBridgeTestExternalRam(pContext)                    pContext->BridgeChipOps.TestExternalRam(pContext)
#define DibBridgeGetRamAddr(pContext)                         pContext->BridgeChipOps.GetRamAddr(pContext)
#else
#define IntBridgeTestBasicRead(pContext)                      0
#define IntBridgeTestRegister(pContext)                       0
#define IntBridgeTestInternalRam(pContext)                    0
#define IntBridgeTestExternalRam(pContext)                    0
#define DibBridgeGetRamAddr(pContext)                         0
#endif

#endif
