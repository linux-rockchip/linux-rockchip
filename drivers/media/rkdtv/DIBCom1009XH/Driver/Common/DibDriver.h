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
* @file "DibDriver.h"
* @brief Generic Driver functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_H
#define DIB_DRIVER_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"     /* for DibDriverGpio structures */
#include "DibDriverCtx.h"        /* for DibDriverContext         */



struct DibDriverEventInfoUp;
struct DibStream;


extern DIBSTATUS DibDriverError(void);
extern DIBSTATUS DibDriverSuccess(void);

DIBSTATUS DibDriverInit(struct DibDriverContext *pContext, enum DibBoardType BoardType, BOARD_HDL BoardHdl);
DIBSTATUS DibDriverDeinit(struct DibDriverContext *pContext);

/* Must be provided by the target dependent stuff */
DIBSTATUS DibDriverTargetInit(struct DibDriverContext *pContext);
DIBSTATUS DibDriverTargetDeinit(struct DibDriverContext *pContext);
DIBSTATUS DibDriverTargetDataRead(struct DibDriverContext *pContext, uint8_t *, uint32_t *, FILTER_HDL FilterHdl);
DIBSTATUS DibDriverTargetDataFlush(struct DibDriverContext * pContext, FILTER_HDL FilterHdl, uint32_t Size);
void      DibDriverTargetDisableIrqProcessing(struct DibDriverContext *pContext);
void      DibDriverTargetEnableIrqProcessing(struct DibDriverContext *pContext);

void      DibDriverPostEvent(struct DibDriverContext * pContext, struct DibDriverEventInfoUp * pDataInfo);

/* Internal Driver calls */
void      IntDriverRemoveItem(struct DibDriverContext *pContext, ELEM_HDL ItemIdx);
void      IntDriverRemoveFilter(struct DibDriverContext *pContext, FILTER_HDL FilterIdx);
void      IntDriverRemoveChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelIdx, enum DibBool Locked);
void      IntDriverAddChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType Type, struct DibTuneChan *pChannelDescriptor);
void      IntDriverAddFilter(struct DibDriverContext *pContext, FILTER_HDL FilterIdx, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer *pDataBuf);
void      IntDriverAddItem(struct DibDriverContext *pContext, ELEM_HDL ItemIdx, FILTER_HDL FilterHdl, union DibFilters *pFilterDesc);
DIBSTATUS IntDriverAllocMem(struct DibDriverContext *pContext);
void      IntDriverFreeMem(struct DibDriverContext *pContext);
void      IntDriverInitSoft(struct DibDriverContext *pContext);
void      IntDriverDeinitSoft(struct DibDriverContext *pContext);
DIBSTATUS IntDriverRegisterChip(struct DibDriverContext *pContext);
DIBSTATUS IntDriverRemoveLastFrontendFromStream(struct DibDriverContext *pContext, struct DibStream * pStream);
struct DibFrontend * 
          IntDriverGetFrontendForStream(struct DibStream * pStream, enum DibDemodType Std);
void      IntDriverUpdateFrontends(struct DibDriverContext *pContext, struct DibStream * pStream, uint32_t MinDemod, uint32_t MaxDemod);

DIBSTATUS IntDriverStreamAutoRemoving(struct DibDriverContext *pContext, struct DibStream * pStream);

DIBSTATUS DibDriverGetNewBuffer(struct DibDriverContext * pContext, FILTER_HDL FilterHdl, struct DibBufContext * pDriverCtx, uint8_t GoToNext);
DIBSTATUS DibDriverDataCallback(struct DibDriverContext *pContext, struct DibBufContext *pDriverCtx, enum DibDataStatus);
DIBSTATUS DibDriverFlushBuffers(struct DibDriverContext *pContext, FILTER_HDL FilterHdl);
void      DibDriverSwap16(uint8_t * Buf, int size);
void      DibDriverSwap32(uint8_t * Buf, int size);
  
uint32_t  IntDriverGetCrc32(void *pTable, uint32_t bufferLength, uint32_t crc32bis);


/****************************** chip specific functions ***************************************/
#define HBM_SELECT_SET(flag_int,hbm)      ( flag_int = ((flag_int & 0xFF7FFFFF) | (((hbm)<<23) &0x00800000)) )

/*----------------------------- Mac initialisation --------------------------------------------*/
#define  DibDriverResetCpu(pContext)                                   pContext->ChipOps.MacResetCpu(pContext)
#define  DibDriverInitCpu(pContext)                                    pContext->ChipOps.MacInitCpu(pContext)
#define  DibDriverStartCpu(pContext)                                   pContext->ChipOps.MacStartCpu(pContext)
#define  DibDriverInitMailbox(pContext)                                pContext->ChipOps.MacInitMailbox(pContext)
#define  DibDriverUploadMicrocode(pContext)                            pContext->ChipOps.MacUploadMicrocode(pContext)
#define  DibDriverTestRamInterface(pContext)                           pContext->ChipOps.MacTestRamInterface(pContext)
#define  DibDriverEnableClearIrq(pContext)                             pContext->ChipOps.MacEnableClearIrq(pContext)
#define  DibDriverFrontendInit(pContext)                               pContext->ChipOps.FrontendInit(pContext)
#define  DibDriverFrontendRfTotalGain(LnaStatus, agc_global, pContext) pContext->ChipOps.FrontendRfTotalGain(LnaStatus, agc_global, pContext)

#endif /* DIB_DRIVER_H */
