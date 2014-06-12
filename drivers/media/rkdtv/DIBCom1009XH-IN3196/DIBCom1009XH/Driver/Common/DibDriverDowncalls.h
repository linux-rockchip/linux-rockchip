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
* @file "DibDriverDowncalls.h"
* @brief Driver 2 Bridge function prototypes.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DOWNCALLS_H
#define DIB_DRIVER_DOWNCALLS_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define DIBBRIGDE_BIT_MODE_8      0
#define DIBBRIGDE_BIT_MODE_16     1
#define DIBBRIGDE_BIT_MODE_32     2

struct DibDriverDmaCtx;

/* Functions leading to the lower level bridge */
DIBSTATUS DibD2BReadReg(struct DibDriverContext *pContext, uint32_t Addr, uint8_t ReadMode, uint32_t Cnt, uint8_t *pBuf);
DIBSTATUS DibD2BWriteReg(struct DibDriverContext *pContext, uint32_t Addr, uint8_t WriteMode, uint32_t Cnt, uint8_t *pBuf);
DIBSTATUS DibD2BSendMsg(struct DibDriverContext *pContext, uint32_t Cnt, uint32_t * buf);
DIBSTATUS DibD2BPostInit(struct DibDriverContext *pContext, uint32_t flags);
DIBSTATUS DibD2BInformBridge(struct DibDriverContext *pContext, enum DibBridgeInfoType BridgeInfoType, union DibInformBridge * pInfoBridge);

DIBSTATUS DibD2BTransfertBuf(struct DibDriverContext *pContext, struct DibDriverDmaCtx * pDrvDmaCtx);
#endif
