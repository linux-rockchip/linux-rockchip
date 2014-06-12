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
* @file "DibDriverFireflyStd.h"
* @brief Firefly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_FIREFLY_STD_H
#define DIB_DRIVER_FIREFLY_STD_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/*************************************************************/
/*** DVB                                                  ****/
/*************************************************************/
#if (DIB_DVB_STD == 1)
DIBSTATUS DibDriverFireflyGetChannelInfoDvb(struct DibDriverContext * pContext, uint8_t ChannelIdx, uint8_t ScanStatus, uint16_t *Data, uint8_t TuneMonit);
DIBSTATUS DibDriverFireflyScanChannelDvb(struct DibDriverContext *pContext, uint8_t tsChNum, struct DibTuneChan *pTc, enum DibDemodType Type);
void      DibDriverFireflyProcessMsgMonitDemodDvb(struct DibDriverContext * pContext, uint16_t *Data, struct DibTotalDemodMonit * pTotalMonitInfo);
uint32_t DibDriverFireflyFrontendRfTotalGain(uint8_t LnaStatus, uint32_t AgcGlobal, struct DibDriverContext *pContext);

#define DIB_DRIVER_FIREFLY_GET_TIMEOUT pTotalMonitInfo->DemodMonit[TmpFp].Dvb.Timeout


#else                                         
DIBSTATUS DibDriverFireflyGetChannelInfoDvb(struct DibDriverContext * pContext, uint8_t ChannelIdx, uint8_t ScanStatus, uint16_t *Data, uint8_t TuneMonit) {return DIBSTATUS_ERROR;}
DIBSTATUS DibDriverFireflyScanChannelDvb(struct DibDriverContext *pContext, uint8_t tsChNum, struct DibTuneChan *pTc, enum DibDemodType Type) {return DIBSTATUS_ERROR;}
void      DibDriverFireflyProcessMsgMonitDemodDvb(struct DibDriverContext * pContext, uint16_t *Data, struct DibTotalDemodMonit * pTotalMonitInfo) {;}
uint32_t  DibDriverFireflyFrontendRfTotalGain(uint8_t LnaStatus, uint32_t AgcGlobal, struct DibDriverContext *pContext) {return 0;}

#define DIB_DRIVER_FIREFLY_GET_TIMEOUT 0

#endif

#endif  /* DIB_DRIVER_IF_H */
