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
* @file "DibBridgePayloadCheckers.h"
* @brief Data check.
*
***************************************************************************************************/
#ifndef _PAYLOAD_CHECKERS_H_
#define _PAYLOAD_CHECKERS_H_

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (DIB_CHECK_RAWTS_DATA == 1)
uint8_t DibBridgeCheckRawTsInit(void);
void DibBridgeCheckRawTsData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, uint8_t Item);
#endif

#if (DIB_CHECK_FIG_DATA == 1)
uint8_t DibBridgeCheckFigInit(uint8_t *pBuf, uint32_t Size);
void    DibBridgeCheckFigData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size);
#endif

#if (DIB_CHECK_MSC_DATA == 1)
uint8_t DibBridgeCheckMscInit(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item);
uint8_t DibBridgeCheckMscData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item);
uint8_t DibBridgeCheckMscDataGroup(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item);
#endif

#if (DIB_CHECK_IP_DATA == 1)
uint8_t DibBridgeCheckIpData(struct DibBridgeContext *pContext, uint8_t *pdata, uint32_t Cnt, uint8_t item);
#endif

#if (DIB_CHECK_RTP_DATA == 1)
void DibBridgeCheckRtpData(struct DibBridgeContext *pContext, uint8_t *pdata, uint32_t mpe_size, uint8_t item);
#endif

#if (DIB_CHECK_PES_DATA == 1)
void DibBridgeCheckPesData(struct DibBridgeContext *pContext, uint8_t *pBuf, struct DibBridgeDmaCtx * pDmaCtx);
#endif
#if (DIB_CHECK_PCR_DATA == 1)
void    DibBridgeCheckPcrData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size);
#endif

#if (DIB_CHECK_CMMB_DATA == 1)
void DibBridgeCheckCmmbMultiplexFrame(struct DibBridgeContext *pContext, uint8_t *Mf, uint32_t Size, uint32_t ItemId);
#endif /* DIB_CHECK_CMMB_DATA */

#endif 
