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
* @file "DibDriverIf.h"
* @brief Generic Driver interface.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_IF_H
#define DIB_DRIVER_IF_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibExtAPI.h"

struct DibDriverContext;

#ifdef __cplusplus
extern "C" 
{
#endif

  DIBSTATUS DibDriverGetChannel(struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibChannel *pDescriptor, CHANNEL_HDL *pChannelHdl);
 
  DIBSTATUS DibDriverDeleteChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl);
 
  DIBSTATUS DibDriverCreateFilter(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer *pDataBuf, FILTER_HDL *pFilterHdl);
 
  DIBSTATUS DibDriverDeleteFilter(struct DibDriverContext *pContext, FILTER_HDL FilterHdl);
 
  DIBSTATUS DibDriverAddToFilter(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint32_t nbElements, union DibFilters *pFilterDesc, ELEM_HDL *pElemHdl);
 
  DIBSTATUS DibDriverRemoveFromFilter(struct DibDriverContext *pContext, ELEM_HDL ElemHdl);
  
  DIBSTATUS DibDriverGetDataMonitoring(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, enum DibBool ClearMonit);

  DIBSTATUS DibDriverGetGlobalInfo(struct DibDriverContext *pContext, struct DibGlobalInfo *pGlobalInfo);
  
  DIBSTATUS DibDriverGetStreamAttributes(struct DibDriverContext * pContext, STREAM_HDL, struct DibStreamAttributes * Attributes);

  DIBSTATUS DibDriverGetFrontendAttributes(struct DibDriverContext * pContext, FRONTEND_HDL, struct DibFrontendAttributes * Attributes);

  DIBSTATUS DibDriverGetChannelDescriptor(struct DibDriverContext *pContext, CHANNEL_HDL ChanneHdl, struct DibTuneChan *pDescriptor);
  
  DIBSTATUS DibDriverRegisterBuffer(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint8_t *pBufAdd, uint32_t BufSize, uint8_t BufId);
  
  DIBSTATUS DibDriverUnregisterBuffer(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint8_t *pBufAdd);

  DIBSTATUS DibDriverGetSignalMonitoring(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, uint8_t * NbDemod, union DibDemodMonit * buf, DibGetSignalCallback pCallBack, void *pCallCtx);

  DIBSTATUS DibDriverTuneMonitChannel(struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType Type, struct DibTuneMonit *pMonitDescriptor, DibTuneMonitCallback pCallBack, void *pCallCtx);
  
  DIBSTATUS DibDriverAbortTuneMonitChannel(struct DibDriverContext *pContext);

  DIBSTATUS DibDriverSetConfig(struct DibDriverContext *pContext, enum DibParameter Param, union DibParamConfig *pParamConfig);

  DIBSTATUS DibDriverGetConfig(struct DibDriverContext *pContext, enum DibParameter Param, union DibParamConfig *pParamConfig);  
  
  DIBSTATUS DibDriverRegisterEventCallback(struct DibDriverContext * pContext, DibGetEventCallback pCallback, void * pCallCtx, uint8_t NbEvents, enum DibEvent * pEvent);
  
  DIBSTATUS DibDriverUnregisterEventCallback(struct DibDriverContext * pContext, DibGetEventCallback pCallback);
  
   DIBSTATUS DibDriverSendMessageApi(struct DibDriverContext *pContext, uint16_t MsgType, uint16_t MsgIndex, uint8_t *pSendBuf, uint32_t SendLen, uint8_t *pRecvBuf, uint32_t * pRecvLen);

   DIBSTATUS DibDriverRegisterMessageCallback(struct DibDriverContext * pContext, uint16_t MsgType, uint8_t * pMsgBuf, uint32_t MsgBufLen, DibMessageCallback pCallback, void * pCallbackCtx);

   DIBSTATUS DibDriverGetStreamAttributes(struct DibDriverContext *pContext, STREAM_HDL Stream, struct DibStreamAttributes *Attributes);

   DIBSTATUS DibDriverGetFrontendAttributes(struct DibDriverContext *pContext, FRONTEND_HDL Fe, struct DibFrontendAttributes *Attributes);

   DIBSTATUS DibDriverGetStream(struct DibDriverContext *pContext, STREAM_HDL Stream, enum DibDemodType Standard, uint32_t OutputOptions, enum DibPowerMode EnPowerSaving);

   DIBSTATUS DibDriverAddFrontend(struct DibDriverContext *pContext, STREAM_HDL Stream, FRONTEND_HDL Fe, FRONTEND_HDL FeOutput, uint32_t Force);

   DIBSTATUS DibDriverGetChannelEx(struct DibDriverContext *pContext, STREAM_HDL Hdl, struct DibChannel *pDescriptor, CHANNEL_HDL *pChannelHdl);

   DIBSTATUS DibDriverTuneMonitChannelEx(struct DibDriverContext *pContext, STREAM_HDL Hdl, struct DibTuneMonit *pMonitDescriptor, DibTuneMonitCallback pCallBack, void *pCallCtx);

   DIBSTATUS DibDriverGetFrontendMonit(struct DibDriverContext *pContext, FRONTEND_HDL Fe, union DibDemodMonit *Monit);

   DIBSTATUS DibDriverRemoveFrontend(struct DibDriverContext *pContext, FRONTEND_HDL Fe);

   DIBSTATUS DibDriverDeleteStream( struct DibDriverContext *pContext, STREAM_HDL Stream);

   DIBSTATUS DibDriverGetInfo(struct DibDriverContext *pContext, struct DibInfoRequest *pInfoRequest, struct DibInfoData *pInfoData);

   DIBSTATUS DibDriverSetInfo(struct DibDriverContext *pContext, struct DibInfoSet *pInfoSet, uint8_t *pstatus);

#ifdef __cplusplus
}
#endif
#endif  /* DIB_DRIVER_IF_H */
