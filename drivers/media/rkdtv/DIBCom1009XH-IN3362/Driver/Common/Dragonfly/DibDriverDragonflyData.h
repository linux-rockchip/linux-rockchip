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
* @file "DibDriverDragonflyData.h"
* @brief Dragonfly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DRAGONFLY_DATA_H
#define DIB_DRIVER_DRAGONFLY_DATA_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */


/*************************************************************/
/*** RAWTS / TDMB                                         ****/
/*************************************************************/
#if (DIB_RAWTS_DATA == 1) || (DIB_TDMB_DATA == 1) 
void        DibDriverDragonflyProcessMsgMonitorRawTs(struct DibDriverContext *pContext, uint32_t *Data);
#else
#define     DibDriverDragonflyProcessMsgMonitorRawTs(pContext, Data) 
#endif

/*************************************************************/
/*** RAWTS                                                ****/
/*************************************************************/
#if (DIB_RAWTS_DATA == 1)
DIBSTATUS DibDriverDragonflyAddItemRawTs(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, ELEM_HDL * pElemHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
DIBSTATUS IntDriverDragonflyRemoveItemRawTs(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
#else
#define DibDriverDragonflyAddItemRawTs(pContext, pFilterDesc, pElemHdl, ItemHdl, msg) DIBSTATUS_ERROR
#define IntDriverDragonflyRemoveItemRawTs(pContext, pItem)                            DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** ELEMENTARY STREAM                                    ****/
/*************************************************************/
#if (DIB_PES_DATA == 1)
#else
#endif


/*************************************************************/
/*** SIPSI                                                ****/
/*************************************************************/
#if (DIB_SIPSI_DATA == 1)
DIBSTATUS DibDriverDragonflyAddItemSiPsi(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, ELEM_HDL * pElemHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
DIBSTATUS DibDriverDragonflyRemoveItemSiPsi(struct DibDriverContext *pContext, struct DibDriverItem *pItem, ELEM_HDL ElemHdl);
DIBSTATUS DibDriverDragonflyGetSiPsiInfo(struct DibDriverContext * pContext, uint8_t * pPacketAddr, int32_t *pPacketSize, uint32_t *pItemIndex, uint8_t *pStartSection, uint8_t *pEndSection, uint8_t *pCrcValid, uint8_t *pHeaderLen, uint8_t *pCrcLen);
#else
#define DibDriverDragonflyAddItemSiPsi(pContext, pFilterDesc, pElemHdl, ItemHdl, msg)         DIBSTATUS_ERROR
#define DibDriverDragonflyRemoveItemSiPsi(pContext, pItem, ElemHdl)                           DIBSTATUS_ERROR
#define DibDriverDragonflyGetSiPsiInfo                                                        DibDriverError
#endif


/*************************************************************/
/*** MPEFEC                                               ****/
/*************************************************************/
#if (DIB_MPEFEC_DATA == 1)
void      DibDriverDragonflyProcessMsgMonitorMpeFec(struct DibDriverContext *pContext, uint32_t * Data);
DIBSTATUS DibDriverDragonflyAddItemMpeFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
DIBSTATUS DibDriverDragonflyRemoveItemMpeFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
#else
#define DibDriverDragonflyProcessMsgMonitorMpeFec(pContext, Data) 
#define DibDriverDragonflyAddItemMpeFec(pContext, pFilterDesc, FilterHdl, ItemHdl, msg) DIBSTATUS_ERROR
#define DibDriverDragonflyRemoveItemMpeFec(pContext, pItem)                             DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** MPEIFEC                                              ****/
/*************************************************************/
#if (DIB_MPEIFEC_DATA == 1)
void      DibDriverDragonflyProcessMsgMonitorMpeIFec(struct DibDriverContext *pContext, uint32_t * Data);
DIBSTATUS DibDriverDragonflyRemoveItemMpeIFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
DIBSTATUS DibDriverDragonflyAddItemMpeIFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
#else
#define DibDriverDragonflyProcessMsgMonitorMpeIFec(pContext, Data) 
#define DibDriverDragonflyRemoveItemMpeIFec(pContext, pItem)                             DIBSTATUS_ERROR
#define DibDriverDragonflyAddItemMpeIFec(pContext, pFilterDesc, FilterHdl, ItemHdl, msg) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** CMMB                                                 ****/
/*************************************************************/
#if (DIB_CMMB_DATA == 1)
void      DibDriverDragonflyProcessMsgMonitorCmmbRsm(struct DibDriverContext *pContext, uint32_t *Data);
void      DibDriverDragonflyProcessMsgMonitorCmmbMf(struct DibDriverContext *pContext, uint32_t *Data);
DIBSTATUS DibDriverDragonflyAddItemCmmbSvc(struct DibDriverContext *pContext,
                                           union DibFilters *pFilterDesc,
                                           FILTER_HDL FilterHdl,
                                           uint32_t ItemHdl,
                                           struct MsgCreateItem * msg);
DIBSTATUS DibDriverDragonflyRemoveItemCmmbSvc(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
#else
#define DibDriverDragonflyProcessMsgMonitorCmmbRsm(pContext, Data)
#define DibDriverDragonflyProcessMsgMonitorCmmbMf(pContext, Data)
#define DibDriverDragonflyAddItemCmmbSvc(pContext, pFilterDesc, FilterHdl, ItemHdl, msg) DIBSTATUS_ERROR
#define DibDriverDragonflyRemoveItemCmmbSvc(pContext, pItem)                             DIBSTATUS_ERROR
#endif /* DIB_CMMB_DATA */

/*************************************************************/
/*** DAB - FIG                                              ****/
/*************************************************************/
#if (DIB_FIG_DATA == 1)
DIBSTATUS DibDriverDragonflyRemoveItemFig(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
DIBSTATUS DibDriverDragonflyAddItemFig(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
void        DibDriverDragonflyProcessMsgMonitorFic(struct DibDriverContext *pContext, uint32_t *Data);
#else
#define DibDriverDragonflyRemoveItemFig(pContext, pItem)                             DIBSTATUS_ERROR
#define DibDriverDragonflyAddItemFig(pContext, pFilterDesc, FilterHdl, ItemHdl, msg) DIBSTATUS_ERROR
#define DibDriverDragonflyProcessMsgMonitorFic(pContext, Data)
#endif

/*************************************************************/
/*** DAB - TDMB                                              ****/
/*************************************************************/
#if (DIB_TDMB_DATA == 1)
DIBSTATUS DibDriverDragonflyRemoveItemTdmb(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
DIBSTATUS DibDriverDragonflyAddItemTdmb(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
#else
#define DibDriverDragonflyRemoveItemTdmb(pContext, pItem)                             DIBSTATUS_ERROR
#define DibDriverDragonflyAddItemTdmb(pContext, pFilterDesc, FilterHdl, ItemHdl, msg) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** DAB - Audio                                              ****/
/*************************************************************/
#if (DIB_DAB_DATA == 1)
DIBSTATUS DibDriverDragonflyRemoveItemDabAudio(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
DIBSTATUS DibDriverDragonflyAddItemDabAudio(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
void      DibDriverDragonflyProcessMsgMonitorDabPlus(struct DibDriverContext *pContext, uint32_t * Data);
#else
#define DibDriverDragonflyRemoveItemDabAudio(pContext, pItem)                             DIBSTATUS_ERROR
#define DibDriverDragonflyAddItemDabAudio(pContext, pFilterDesc, FilterHdl, ItemHdl, msg) DIBSTATUS_ERROR
#define DibDriverDragonflyProcessMsgMonitorDabPlus(pContext, Data)  DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** DAB - Packet                                              ****/
/*************************************************************/
#if (DIB_DABPACKET_DATA == 1)
DIBSTATUS DibDriverDragonflyRemoveItemDabPacket(struct DibDriverContext *pContext, struct DibDriverItem *pItem);
DIBSTATUS DibDriverDragonflyAddItemDabPacket(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg);
void      DibDriverDragonflyProcessMsgMonitorMscPacket(struct DibDriverContext *pContext, uint32_t * Data);
#else
#define DibDriverDragonflyRemoveItemDabPacket(pContext, pItem)                                        DIBSTATUS_ERROR
#define DibDriverDragonflyAddItemDabPacket(pContext, pFilterDesc, FilterHdl, ItemHdl, msg)            DIBSTATUS_ERROR
#define DibDriverDragonflyProcessMsgMonitorMscPacket(pContext, Data)  DIBSTATUS_ERROR
#endif

#endif
