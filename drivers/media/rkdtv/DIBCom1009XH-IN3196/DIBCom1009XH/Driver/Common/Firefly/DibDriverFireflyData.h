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
* @file "DibDriverFireflyData.h"
* @brief Dragonfly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_FIREFLY_DATA_H
#define DIB_DRIVER_FIREFLY_DATA_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/*************************************************************/
/*** RAWTS                                                ****/
/*************************************************************/
#if (DIB_RAWTS_DATA == 1)
DIBSTATUS DibDriverFireflyAddItemRawTs(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo);
void      DibDriverFireflyProcessMsgMonitorRawTs(struct DibDriverContext *pContext, uint16_t * Data);
DIBSTATUS DibDriverFireflyAddPidRawTs(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode);
#else
#define DibDriverFireflyAddItemRawTs(pContext, pFilterDesc, PidInfo)                      DIBSTATUS_ERROR
#define DibDriverFireflyProcessMsgMonitorRawTs(pContext, Data) 
#define DibDriverFireflyAddPidRawTs(pContext, pItem, FilterHdl, format, isWatch, PidMode) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** ELEMENTARY STREAM                                    ****/
/*************************************************************/
#if (DIB_PES_DATA == 1)
DIBSTATUS DibDriverFireflyAddItemPesVideo(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo);
DIBSTATUS DibDriverFireflyAddItemPesAudio(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo);
DIBSTATUS DibDriverFireflyAddItemPesOther(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo);
DIBSTATUS DibDriverFireflyAddItemPesPcr(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo);
DIBSTATUS DibDriverFireflyAddPidPesVideo(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode);
DIBSTATUS DibDriverFireflyAddPidPesAudio(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode);
DIBSTATUS DibDriverFireflyAddPidPesOther(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode);
DIBSTATUS DibDriverFireflyAddPidPesPcr(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode);
#else
#define DibDriverFireflyAddItemPesVideo(pContext, pFilterDesc, PidInfo)                      DIBSTATUS_ERROR
#define DibDriverFireflyAddItemPesAudio(pContext, pFilterDesc, PidInfo)                      DIBSTATUS_ERROR
#define DibDriverFireflyAddItemPesOther(pContext, pFilterDesc, PidInfo)                      DIBSTATUS_ERROR
#define DibDriverFireflyAddItemPesPcr(pContext,   pFilterDesc, PidInfo)                      DIBSTATUS_ERROR
#define DibDriverFireflyAddPidPesVideo(pContext, pItem, FilterHdl, format, isWatch, PidMode) DIBSTATUS_ERROR
#define DibDriverFireflyAddPidPesAudio(pContext, pItem, FilterHdl, format, isWatch, PidMode) DIBSTATUS_ERROR
#define DibDriverFireflyAddPidPesOther(pContext, pItem, FilterHdl, format, isWatch, PidMode) DIBSTATUS_ERROR
#define DibDriverFireflyAddPidPesPcr(pContext,   pItem, FilterHdl, format, isWatch, PidMode) DIBSTATUS_ERROR
#endif


/*************************************************************/
/*** SIPSI                                                ****/
/*************************************************************/
#if (DIB_SIPSI_DATA == 1)
DIBSTATUS DibDriverFireflyGetSipsiInfo(struct DibDriverContext * pContext, uint8_t *pPacketAddr, int32_t *pPacketSize, uint32_t *pItemIndex, uint8_t *pStartSection, uint8_t *pEndSection, uint8_t *pCrcValid, uint8_t *pHeaderLen, uint8_t *pCrcLen);
DIBSTATUS DibDriverFireflyAddItemSiPsi(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo);
DIBSTATUS DibDriverFireflyAddPidSiPsi(struct DibDriverContext *pContext, struct DibDriverItem *pItem, struct DibAddPidInfo * pPidInfo, FILTER_HDL FilterHdl, ELEM_HDL AliasItemHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode);
DIBSTATUS DibDriverFireflyDelPidSiPsi(struct DibDriverContext *pContext, struct DibDriverItem *pItem, ELEM_HDL elemHdl);
#else
#define DibDriverFireflyGetSipsiInfo                                                                 DibDriverError
#define DibDriverFireflyAddItemSiPsi(pContext, pFilterDesc, PidInfo)                                 DIBSTATUS_ERROR
#define DibDriverFireflyAddPidSiPsi(pContext, pItem, pPidInfo, FilterHdl, Alias, format, isWatch, PidMode)  DIBSTATUS_ERROR
#define DibDriverFireflyDelPidSiPsi(pContext, pItem, elemHdl)                                        DIBSTATUS_ERROR
#endif


/*************************************************************/
/*** MPEFEC                                               ****/
/*************************************************************/
#if (DIB_MPEFEC_DATA == 1)
void      DibDriverFireflyProcessMsgMonitorMpeFec(struct DibDriverContext *pContext, uint16_t * Data);
void      DibDriverFireflyProcessMsgHbmProf(struct DibDriverContext *pContext, uint16_t * Data);
DIBSTATUS DibDriverFireflyAddItemMpeFec(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo);
DIBSTATUS DibDriverFireflyAddPidMpeFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t SvcIdx, enum DibPidMode *PidMode, struct DibAddPidInfo *pPidInfo);
DIBSTATUS DibDriverFireflyDelPidMpeFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem, ELEM_HDL elemHdl);
DIBSTATUS DibDriverFireflySetPidMode(struct DibDriverContext *pContext, struct DibPidModeCfg *pPidInfo);
#else
#define DibDriverFireflyProcessMsgMonitorMpeFec(pContext, Data)
#define DibDriverFireflyProcessMsgHbmProf(pContext, Data)
#define DibDriverFireflyAddItemMpeFec(pContext, FilterHdl, pFilterDesc, PidInfo)                      DIBSTATUS_ERROR
#define DibDriverFireflyAddPidMpeFec(pContext, pItem, FilterHdl, format, SvcIdx, PidMode, pPidInfo)   DIBSTATUS_ERROR
#define DibDriverFireflyDelPidMpeFec(pContext, pItem, elemHdl)                                        DIBSTATUS_ERROR
#define DibDriverFireflySetPidMode                                                                    DibDriverError
#endif


#endif
