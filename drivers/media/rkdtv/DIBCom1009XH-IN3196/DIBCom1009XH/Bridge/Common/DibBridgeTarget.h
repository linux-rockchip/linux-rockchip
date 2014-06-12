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
* @file "DibBridgeTarget.h"
* @brief Bridge Target function prototypes.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_TARGET_H
#define DIB_BRIDGE_TARGET_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/** ----- Select the irq mode ------ */
/** Supported Types of Physical Interface (INTERRUPT_MODE) */
#define USE_INTERRUPTS           1     /**< bridge target is supposed to handle hardware interrupts... */
#define USE_POLLING              2     /**< bridge target does not handle hardware interrupts 
                                            (or interrupt mode is disabled), no need to clear interrupts
                                            in tis mode. */

#define DIBBRIDGE_DMA_WRITE         0
#define DIBBRIDGE_DMA_READ          1

#define DIBBRIDGE_BIT_MODE_8        0
#define DIBBRIDGE_BIT_MODE_16       1
#define DIBBRIDGE_BIT_MODE_32       2

#define DIBBRIDGE_MODE_SRAM         0
#define DIBBRIDGE_MODE_SDIO         1

DIBSTATUS DibBridgeTargetInit(struct DibBridgeContext *pContext);
DIBSTATUS DibBridgeTargetSetHighSpeed(struct DibBridgeContext *pContext);
DIBSTATUS DibBridgeTargetDeinit(struct DibBridgeContext *pContext);
DIBSTATUS DibBridgeTargetCopyToUser(uint8_t *pUserBuffer, uint8_t *pKernelBuffer, uint32_t Size);

DIBSTATUS DibBridgeTargetWrite(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint32_t Nb, uint8_t *pBuf);
DIBSTATUS DibBridgeTargetRead(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint32_t Nb, uint8_t *pBuf);

DIBDMA    DibBridgeTargetRequestDma(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);

uint8_t  *DibBridgeTargetAllocBuf(uint32_t Size);
void      DibBridgeTargetFreeBuf(uint8_t *pBuf, uint32_t Size);

void      DibBridgeTargetDisableIRQ(struct DibBridgeContext *pContext);
void      DibBridgeTargetEnableIrq(struct DibBridgeContext *pContext);
void      DibBridgeTargetSetupIrqHandler(struct DibBridgeContext *pContext);
void      DibBridgeTargetRemoveIrqHandler(struct DibBridgeContext *pContext);


uint32_t  IntBridgeGetTimeInMs(void);

#endif

