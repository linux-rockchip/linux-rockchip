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
* @file "DibDriverDragonfly.h"
* @brief Dragonfly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DRAGONFLY_H
#define DIB_DRIVER_DRAGONFLY_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriver.h"

#include "DibDriverDragonflyTest.h" 

/* Specific Build Option */ 
#define DF_PROFILE                  0
#define SUPPORT_CRYPTED_DOWNLOAD    0
#define FAST_DOWNLOAD               1
#define CHECK_FIRMWARE_DOWNLOAD     0


/** new api */
void IntDriverDragonflyProcessMsgAck(struct DibDriverContext * pContext, uint16_t id, uint32_t * pData);
void IntDriverDragonflyTuneConfirm(struct DibDriverContext * pContext, uint32_t * pData);
void IntDriverDragonflyProcessMsgApi(struct DibDriverContext * pContext, uint32_t * pData);
void IntDriverDragonflyProcessMsgApiUp(struct DibDriverContext * pContext, uint32_t * pData);
void IntDriverDragonflyChannelStatus(struct DibDriverContext * pContext, uint32_t * pData);

/** subfirmware communication */
void IntDriverDragonflySendMsgUdib(struct DibDriverContext *pContext, uint32_t *pdata, uint16_t len);
void IntDriverDragonflyProcessMsgUdib(struct DibDriverContext *pContext, uint32_t *pdata, uint16_t len);
void DibDriverDragonflyRegisterChip(struct DibDriverContext *pContext);
void DibDriverDragonflyMSdkRegisterChip(struct DibDriverContext *pContext);

#if (DIB_CMMB_DATA == 1) && (DIB_INTERNAL_DEBUG == 1)
/** Host emulates Reflex */
void IntDriverDragonflyHostReflexInit(struct DibDriverContext *pContext, uint32_t *Data);
void IntDriverDragonflyHostReflexStep(struct DibDriverContext *pContext, uint32_t *Data);
void IntDriverDragonflyHostReflexSetConfig(struct DibDriverContext *pContext, uint32_t *Data);
void IntDriverDragonflyHostReflexSetSvcParam(struct DibDriverContext *pContext, uint32_t *Data);
#endif /* DIB_CMMB_DATA */


void IntDriverVoyager1ResetCpu(struct DibDriverContext *pContext);
void IntDriverNautilus1ResetCpu(struct DibDriverContext *pContext);
void IntDriverDragonflyInitCpu(struct DibDriverContext *pContext);
void IntDriverDragonflyDownloadCheck(struct DibDriverContext *pContext, uint32_t DeviceAddr, uint8_t * HostBuf, uint32_t Size);
void IntDriverDragonflyStartCpu(struct DibDriverContext *pContext);
void IntDriverDragonflyRegisterIfInit(struct DibDriverContext *pContext);
void IntDriverDragonflyEnableClearIrq(struct DibDriverContext *pContext);
DIBSTATUS IntDriverDragonflyWaitForMsgAck(struct DibDriverContext *pContext, uint32_t Timeout);
DIBSTATUS IntDriverDragonflyFrontendInit(struct DibDriverContext *pContext);
DIBSTATUS IntDriverDragonflySetUploadMicrocode(struct DibDriverContext *pContext, uint32_t Size, uint16_t key, const uint8_t * code);
DIBSTATUS IntDriverDragonflyUploadMicrocode(struct DibDriverContext *pContext);
DIBSTATUS IntDriverDragonflyInitMailbox(struct DibDriverContext *pContext);
DIBSTATUS IntDriverDragonflyWaitForFirmware(struct DibDriverContext *pContext);

#endif
