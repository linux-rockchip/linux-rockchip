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
* @file "DibDriverIntDbg.h"
* @brief Internal Access functions prototypes.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_INT_DBG_H
#define DIB_DRIVER_INT_DBG_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (DIB_INTERNAL_DEBUG == 0)

#define DibIntDriverInit(pContext)
#define DibDriverSubmitBlock(pContext, pBlock)
#define DibDriverSendMsgIntDbg(pContext, Len, pBuf)
#define DibDriverRegisterProcessMsgIntDbg(pContext, pCallBack)
#define DibDriverWriteBlock(pContext, Address, Size, pBuf)
#define DibDriverReadBlock(pContext, Address, Size, pBuf)


#if (DIB_CMMB_DATA == 1)
   #define IntDriverCmmbInit(pContext)
   #define IntDriverCmmbSMInit       (pContext)
   #define IntDriverCmmbSMNewTimeSlot(pContext, pBlock)
   #define IntDriverCmmbSMOutput     (pContext, pBlock)
#endif

#else

typedef DIBSTATUS (*ProcessIntDbgMsgCb)      (struct DibDriverContext *pContext, uint32_t Rx, uint32_t *Data);

/**
* Block of data to submit to Driver along with multipurpose description bytes.
*/
struct DibBlock
{
   uint8_t  *Data;
   uint32_t  Size;
   uint8_t  *DescBytes;
   uint8_t   NbDescBytes;
};

#if 1 // by infospace, dckim
#define DibIntDriverInit(pContext)
#define DibDriverSubmitBlock(pContext, pBlock)
#define DibDriverSendMsgIntDbg(pContext, Len, pBuf)
#define DibDriverRegisterProcessMsgIntDbg(pContext, pCallBack)
#define DibDriverWriteBlock(pContext, Address, Size, pBuf)
#define DibDriverReadBlock(pContext, Address, Size, pBuf)
#else
DIBSTATUS DibIntDriverInit(struct DibDriverContext *pContext);
DIBSTATUS DibDriverWriteBlock(struct DibDriverContext *pContext, uint32_t Address, uint32_t Size, uint8_t *pBuf);
DIBSTATUS DibDriverReadBlock(struct DibDriverContext *pContext, uint32_t Address, uint32_t Size, uint8_t *pBuf);
DIBSTATUS DibDriverSubmitBlock(struct DibDriverContext *pContext, struct DibBlock *pBlock);
DIBSTATUS DibDriverSendMsgIntDbg(struct DibDriverContext *pContext, uint16_t Len, uint32_t *pBuf);
DIBSTATUS DibDriverRegisterProcessIntDbg(struct DibDriverContext *pContext, ProcessIntDbgMsgCb pCallBack);
#endif
void      DibDriverDragonflyDumpRegisters(struct DibDriverContext *pContext);
DIBSTATUS DibDriverWriteIntDbg(struct DibDriverContext *pContext, uint8_t ChipId, uint32_t Addr, uint8_t ByteMode, uint32_t Cnt, uint8_t *pBuf);
DIBSTATUS DibDriverReadIntDbg(struct DibDriverContext *pContext, uint8_t ChipId, uint32_t Addr, uint8_t ByteMode, uint32_t Cnt, uint8_t *pBuf);

DIBSTATUS DibDriverReadSlaveBuf(struct DibDriverContext *pContext, uint32_t ChipId, uint32_t Addr, uint32_t ByteMode, uint32_t Cnt, uint8_t * pBuf);
DIBSTATUS DibDriverWriteSlaveBuf(struct DibDriverContext *pContext, uint32_t ChipId, uint32_t Addr, uint32_t ByteMode, uint32_t Cnt, uint8_t * pBuf);
DIBSTATUS DibDriverReadSlave32(struct DibDriverContext *pContext, uint32_t ChipId, uint32_t Addr, uint32_t *pBuf);
DIBSTATUS DibDriverWriteSlave32(struct DibDriverContext *pContext, uint32_t ChipId, uint32_t Addr, uint32_t Value);

/* State machine. */
void IntDriverReflexCmmbInit(struct DibDriverContext *pContext);
void IntDriverReflexCmmbSMInit(struct DibDriverContext *pContext);

#endif /* DIB_INTERNAL_DEBUG */
#endif /*  DIB_DRIVER_INT_DBG_H */
