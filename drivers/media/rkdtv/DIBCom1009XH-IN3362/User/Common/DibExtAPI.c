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
* @file "DibExtAPI.c"
* @brief SDK Internal External Api functions.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include "DibDriverCommon.h"
#include "DibDriver.h"
#include "DibDriverMessages.h"
#include "DibDriverDowncalls.h"

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
#include <sys/time.h>
#include <time.h>

FILE    *gLogFile              = NULL;
uint32_t gLogFileStartTimeSec  = 0;
uint32_t gLogFileStartTimeMsec = 0;
#endif



/****************************************************************************
* Driver_GetDvbhMonitoring
*
* Gets Data stats
****************************************************************************/
#if (mSDK==0)
DIBSTATUS DibGetDataMonitoring(struct DibDriverContext *pContext, ELEM_HDL ElemHdl, union DibDataMonit *pMnt, enum DibBool ClearData)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetDataMonitoring(pContext, ElemHdl, pMnt, ClearData);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}
#endif

/****************************************************************************
* DibGetGlobalInfo
*
* Gets general information about SDK and the firmware.
****************************************************************************/
DIBSTATUS DibGetGlobalInfo(struct DibDriverContext *pContext, struct DibGlobalInfo *pGlobalInfo)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetGlobalInfo(pContext, pGlobalInfo);
   DibReleaseLock(&pContext->UserLock);

   return (DIBSTATUS)Status;
}


/****************************************************************************
*
****************************************************************************/

#if (mSDK==0)
DIBSTATUS DibTuneMonitChannel(struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType Type, struct DibTuneMonit *pMonitDescriptor, DibTuneMonitCallback pCallBack, void *pCallCtx)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   Status = DibDriverTuneMonitChannel(pContext, MinNbDemod, MaxNbDemod, StreamParameters, Type, pMonitDescriptor, pCallBack, pCallCtx);

   return Status;
}

/****************************************************************************
*
****************************************************************************/

DIBSTATUS DibAbortTuneMonitChannel(struct DibDriverContext *pContext)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   Status = DibDriverAbortTuneMonitChannel(pContext);

   return Status;
}
#endif

/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibGetChannelDescriptor(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, struct DibTuneChan *pDescriptor)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetChannelDescriptor(pContext, ChannelHdl, pDescriptor);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/****************************************************************************
* Register a Buffer to have sipsi or raw Data
*****************************************************************************/

#if (mSDK==0)
DIBSTATUS DibRegisterBuffer(struct DibDriverContext * pContext, FILTER_HDL FilterIdx, uint8_t *BufAdd, uint32_t BufSize, uint8_t BufId)
{
   DIB_ASSERT(pContext);

   return DibDriverRegisterBuffer(pContext, FilterIdx, BufAdd, BufSize, BufId);
}

/****************************************************************************
* Unregister a Buffer to have sipsi or raw Data
*****************************************************************************/

DIBSTATUS DibUnregisterBuffer(struct DibDriverContext * pContext, FILTER_HDL FilterIdx, uint8_t *BufAdd)
{
   DIB_ASSERT(pContext);

   return DibDriverUnregisterBuffer(pContext, FilterIdx, BufAdd);
}
#endif

/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibGetSignalMonitoring(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, uint8_t * NbDemod, union DibDemodMonit * pMonit, DibGetSignalCallback pCallBack, void * pCallCtx)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetSignalMonitoring(pContext, ChannelHdl, NbDemod, pMonit, pCallBack, pCallCtx);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}


/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibOpen(struct DibDriverContext **pContextAddr, enum DibBoardType BoardType, BOARD_HDL BoardHdl)
{
   DIBSTATUS status = DIBSTATUS_ERROR;

   struct DibDriverContext *pContext = (struct DibDriverContext *)DibMemAlloc(sizeof(struct DibDriverContext));

   if(pContext == NULL)
      return DIBSTATUS_ERROR;

   memset(pContext, 0, sizeof(struct DibDriverContext));

   DibAllocateLock(&pContext->UserLock);
   DibInitLock(&pContext->UserLock);
   DibAcquireLock(&pContext->UserLock);

   status = DibDriverInit(pContext, BoardType, BoardHdl);

   if(status == DIBSTATUS_SUCCESS)
   {
      *pContextAddr   = pContext;
      DibReleaseLock(&pContext->UserLock);
   }
   else
   {
   /* Do not set to null even if we should sice client may use twice a dibopen using the same context
      we do not check if the pointer was set to a value as the client may not have initialised it's variable */

   /* *pContextAddr = NULL; */
       DibDeAllocateLock(&pContext->UserLock);
       DibMemFree(pContext, sizeof(struct DibDriverContext));
   }


   return status;
}

/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibClose(struct DibDriverContext *pContext)
{
   DIBSTATUS status = DIBSTATUS_ERROR;

   if(pContext && pContext->Open)
   {
      DibAcquireLock(&pContext->UserLock);

      status = DibDriverDeinit(pContext);

      DibReleaseLock(&pContext->UserLock);
      DibDeAllocateLock(&pContext->UserLock);

      /* This is to help prevent double memory free by multiple calls to DibClose(),
         However as memory will be freed, it can be paged out in any case and cause an error.
         To prevent this DibClose should take struct DibDriverContext **pContext to clear out user reference to driver.*/
      pContext->Open = eDIB_FALSE;

      DibMemFree(pContext, sizeof(struct DibDriverContext));
   }

   return status;
}

/****************************************************************************
* Create a new channel
*****************************************************************************/
DIBSTATUS DibGetChannel(struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibChannel *pDescriptor, CHANNEL_HDL * pChannelHdl, DibGetChannelCallback pCallBack, void *pCallCtx)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetChannel(pContext, MinNbDemod, MaxNbDemod, StreamParameters, pDescriptor, pChannelHdl);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/***************************************************************************
* Delete an existing channel
****************************************************************************/
DIBSTATUS DibDeleteChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverDeleteChannel(pContext, ChannelHdl);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

#if (mSDK==0)
/***************************************************************************
* Create a new filter
****************************************************************************/
DIBSTATUS DibCreateFilter(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer * pDataBuf, FILTER_HDL * pFilterHdl)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverCreateFilter(pContext, ChannelHdl, DataType, pDataBuf, pFilterHdl);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/***************************************************************************
* delete an existing filter
***************************************************************************/
DIBSTATUS DibDeleteFilter(struct DibDriverContext *pContext, FILTER_HDL FilterHdl)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverDeleteFilter(pContext, FilterHdl);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/***************************************************************************
* Add a new element to an existing filter
****************************************************************************/
DIBSTATUS DibAddToFilter(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint32_t NbElements, union DibFilters *pFilterDescriptor, ELEM_HDL * pElemHdl)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverAddToFilter(pContext, FilterHdl, NbElements, pFilterDescriptor, pElemHdl);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/***************************************************************************
* Delete an existing element from a filter
****************************************************************************/
DIBSTATUS DibRemoveFromFilter(struct DibDriverContext *pContext, ELEM_HDL ElemHdl)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverRemoveFromFilter(pContext, ElemHdl);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}
#endif

/**
 * Runtime parameter changing. This function can do the following updates:
 * - Turn on/off a PID to/from prefetch mode
 * @param[in] pContext: driver context
 * @param[in] pParamConfig: Parameter whom configuration need to be changed.
 */
DIBSTATUS DibSetConfig(struct DibDriverContext *pContext, enum DibParameter Param, union DibParamConfig * pParamConfig)
{
   uint8_t status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   status = DibDriverSetConfig(pContext, Param, pParamConfig);
   DibReleaseLock(&pContext->UserLock);

   return status;
}

/**
 * Query Info.
 * @param[in] pContext: driver context
 * @param[in] pParamConfig: Parameter whom configuration need to be changed.
 */
DIBSTATUS DibGetConfig(struct DibDriverContext *pContext, enum DibParameter Param, union DibParamConfig * pParamConfig)
{
   uint8_t status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   status = DibDriverGetConfig(pContext, Param, pParamConfig);
   DibReleaseLock(&pContext->UserLock);

   return status;
}

#if (mSDK==0)
DIBSTATUS DibRegisterEventCallback(struct DibDriverContext * pContext, DibGetEventCallback pCallback, void * pCallCtx, uint8_t NbEvents, enum DibEvent * pEvent)
{
   uint8_t status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   status = DibDriverRegisterEventCallback(pContext, pCallback, pCallCtx, NbEvents, pEvent);
   DibReleaseLock(&pContext->UserLock);

   return status;
}

DIBSTATUS DibUnregisterEventCallback(struct DibDriverContext * pContext, DibGetEventCallback pCallback)
{
   uint8_t status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   status = DibDriverUnregisterEventCallback(pContext, pCallback);
   DibReleaseLock(&pContext->UserLock);

   return status;
}


#endif

DIBSTATUS DibSendMessage(struct DibDriverContext *pContext, uint16_t MsgType, uint16_t MsgIndex, uint8_t *pSendBuf, uint32_t SendLen, uint8_t *pRecvBuf, uint32_t * pRecvLen)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverSendMessageApi(pContext, MsgType, MsgIndex, pSendBuf, SendLen, pRecvBuf, pRecvLen);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

DIBSTATUS DibRegisterMessageCallback(struct DibDriverContext * pContext, uint16_t MsgType, uint8_t * pMsgBuf, uint32_t MsgBufLen, DibMessageCallback pCallback, void * pCallbackCtx)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   //DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverRegisterMessageCallback(pContext, MsgType, pMsgBuf, MsgBufLen, pCallback, pCallbackCtx);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

DIBSTATUS DibUnregisterMessageCallback(struct DibDriverContext * pContext, uint16_t MsgType)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverRegisterMessageCallback(pContext, MsgType, NULL, 0, NULL, NULL);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/****************************************************************************
* DibGetStreamAttributes
*
* Gets general information about SDK and the firmware.
****************************************************************************/
DIBSTATUS DibGetStreamAttributes(struct DibDriverContext *pContext, STREAM_HDL Stream, struct DibStreamAttributes *Attributes)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetStreamAttributes(pContext, Stream, Attributes);
   DibReleaseLock(&pContext->UserLock);

   return (DIBSTATUS)Status;
}

/****************************************************************************
* DibGetFrontendAttributes
*
* Gets general information about SDK and the firmware.
****************************************************************************/
DIBSTATUS DibGetFrontendAttributes(struct DibDriverContext *pContext, FRONTEND_HDL FeHdl, struct DibFrontendAttributes *Attributes)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetFrontendAttributes(pContext, FeHdl, Attributes);
   DibReleaseLock(&pContext->UserLock);

   return (DIBSTATUS)Status;
}

/****************************************************************************
* allocate a stream
****************************************************************************/
DIBSTATUS DibGetStream(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream,
        enum DibDemodType        Standard,
        uint32_t                 OutputOptions,
        enum DibPowerMode        EnPowerSaving)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetStream(pContext, Stream, Standard, OutputOptions, EnPowerSaving);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}


/****************************************************************************
* add a frontend to a stream
****************************************************************************/
DIBSTATUS DibAddFrontend(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream,
        FRONTEND_HDL             Fe,
        FRONTEND_HDL             FeOutput, /**< NULL - automatic mode, parameter is unused for now - will be used in some cases in future releases */
        uint32_t                 Force     /**< Force remove of Frontend if already in use - calls implicitly DibRemoveFrontend() */
)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverAddFrontend(pContext, Stream, Fe, FeOutput, Force);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/****************************************************************************
* tune on a frequency
****************************************************************************/
DIBSTATUS DibGetChannelEx(
        struct DibDriverContext *pContext,
        STREAM_HDL               Hdl,
        struct  DibChannel      *pDescriptor,
        CHANNEL_HDL             *pChannelHdl,
        DibGetChannelCallback    pCallBack,
        void                    *pCallCtx
)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetChannelEx(pContext, Hdl, pDescriptor, pChannelHdl);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/****************************************************************************
* Tune and get signal monitoring for a frequency
****************************************************************************/
DIBSTATUS DibTuneMonitChannelEx(struct DibDriverContext    *pContext,
                                STREAM_HDL                  Hdl,
                                struct DibTuneMonit        *pMonitDescriptor,
                                DibTuneMonitCallback        pCallBack,
                                void                       *pCallCtx)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverTuneMonitChannelEx(pContext, Hdl, pMonitDescriptor, pCallBack, pCallCtx);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}


/****************************************************************************
* Retreive specific frontend monitoring
****************************************************************************/
DIBSTATUS DibGetFrontendMonit(
        struct DibDriverContext *pContext,
        FRONTEND_HDL             Fe,
        union DibDemodMonit     *Monit
)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetFrontendMonit(pContext, Fe, Monit);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/****************************************************************************
* removes frontend instance from stream
****************************************************************************/
DIBSTATUS DibRemoveFrontend(
        struct DibDriverContext *pContext,
        FRONTEND_HDL             Fe
)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverRemoveFrontend(pContext, Fe);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/****************************************************************************
* Releases all resources allocated to this stream (Frontends and Channels)
****************************************************************************/
DIBSTATUS DibDeleteStream(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream
)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverDeleteStream(pContext, Stream);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}

/****************************************************************************
* DibGetInfo
*
* Gets information.
****************************************************************************/
DIBSTATUS DibGetInfo(struct DibDriverContext *pContext, struct DibInfoRequest *pInfoRequest, struct DibInfoData *pInfoData)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverGetInfo(pContext, pInfoRequest, pInfoData);
   DibReleaseLock(&pContext->UserLock);

   return (DIBSTATUS)Status;
}

/****************************************************************************
* DibSetInfo
*
* Sets information.
****************************************************************************/
DIBSTATUS DibSetInfo(struct DibDriverContext *pContext, struct DibInfoSet *pInfoSet, uint8_t *pStatus)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverSetInfo(pContext, pInfoSet, pStatus);
   DibReleaseLock(&pContext->UserLock);

   return (DIBSTATUS)Status;
}
