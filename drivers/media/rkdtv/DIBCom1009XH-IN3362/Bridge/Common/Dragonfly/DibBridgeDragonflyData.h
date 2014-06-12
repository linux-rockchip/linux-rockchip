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
* @file "DibBridgeDragonfly.h"
* @brief Dragonfly sprecific bridge functionality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_DRAGONFLY_DATA_H
#define DIB_BRIDGE_DRAGONFLY_DATA_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridge.h"
#include "DibBridgeData.h"

/*************************************************************/
/*** RAWTS                                                ****/
/*************************************************************/
#if (DIB_RAWTS_DATA == 1)
#else
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
#else                        
#endif


/*************************************************************/
/*** MPEFEC                                               ****/
/*************************************************************/
#if (DIB_MPEFEC_DATA == 1)
#undef   DIB_HBM_DATA
#define  DIB_HBM_DATA    1

#else

#endif

/*************************************************************/
/*** MPEIFEC                                              ****/
/*************************************************************/
#if (DIB_MPEIFEC_DATA == 1)
#undef   DIB_HBM_DATA
#define  DIB_HBM_DATA    1

DIBDMA DibBridgeDragonflyMoveData(struct DibBridgeContext * pContext, uint32_t * Data, uint32_t Size, uint8_t Move);
void   DibBridgeDragonflyReconstructBurst(struct DibBridgeContext * pContext, uint32_t * Data, uint32_t Size);
void   DibBridgeDragonflyResetTable(struct DibBridgeContext * pContext, uint32_t * Data);
DIBDMA DibBridgeDragonflyInfoMsgHandler(struct DibBridgeContext * pContext, uint32_t * Data, uint32_t Size);

#else
#define DibBridgeDragonflyMoveData(pContext, Data, Size, Move)    DIB_NO_DMA
#define DibBridgeDragonflyResetTable(pContext, Data)  
#define DibBridgeDragonflyReconstructBurst(pContext, Data, Size)  
#define DibBridgeDragonflyInfoMsgHandler(pContext, Data, Size)    DIB_NO_DMA

#endif

#if (DIB_HBM_DATA == 1)
uint32_t DibBridgeDragonflyAssembleSlice(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint32_t offs, uint32_t len);
#else
static uint32_t DibBridgeDragonflyAssembleSlice(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint32_t offs, uint32_t len) {return 0;}
#endif

#endif
