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
* @file "DibBridgeMailboxHandler.h"
* @brief Bridge mailbox handling.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_MAILBOX_HANDLER_H
#define DIB_BRIDGE_MAILBOX_HANDLER_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define DIB_BRIDGE_MAX_MAILBOX_TRY 100

/*******************************************************/
/* HBM PROFILER                                        */
/*******************************************************/
#define HBM_PROF_MSG_SIZE                       2


/*******************************************************/
DIBSTATUS DibBridgeSendMsg(struct DibBridgeContext *pContext, uint32_t *pData, uint32_t Nb);
DIBSTATUS DibBridgeTransfertBuf(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDma);
DIBDMA    DibBridgePerformDmaCommon(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx, uint8_t HighPriority);
DIBSTATUS DibBridgeProcessDmaCommon(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);

#if (DRIVER_AND_BRIDGE_MERGED == 0)
DIBSTATUS DibBridgeGetBufFromMsgQueue(struct DibBridgeContext *pContext, uint16_t *pType, uint16_t *pSize, uint8_t *pBuf);
#endif

DIBDMA    DibBridgeHighDataMsgHandlerCommon(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t add, uint32_t len, struct DibBridgeDmaFlags * pFlags);
DIBDMA    DibBridgeLowDataMsgHandlerCommon(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t add, uint32_t len, struct DibBridgeDmaFlags *pFlags);

/* Init/deinit/get msg queue Bridge to Driver */
DIBSTATUS DibBridgeAddBufferToMessageQueue(struct DibBridgeContext *pContext, uint16_t Type, uint32_t Nb, uint8_t *pBuf);
void      DibBridgeInitMsgQueue(struct DibBridgeContext *pContext);
void      DibBridgeTerminateMsgQueue(struct DibBridgeContext *pContext);
void      DibBridgeDeinitMsgQueue(struct DibBridgeContext *pContext);

#endif
