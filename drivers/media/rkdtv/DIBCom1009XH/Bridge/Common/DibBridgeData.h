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
* @file "DibBridgeData.h"
* @brief Bridge Data Handler.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_DATA_H
#define DIB_BRIDGE_DATA_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#define  DIB_HBM_DATA    0

/*************************************************************/
/*** COMMON                                               ****/
/*************************************************************/
void   DibBridgeForwardData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item);
void   DibB2DFwdRawData(struct DibBridgeContext *pContext, uint32_t Size, ELEM_HDL ItemIndex, uint8_t FirstFrag, uint8_t LastFrag);
void   DibB2DFwdSiPsiBuf(struct DibBridgeContext *pContext, uint32_t Nb, uint8_t *pBuf);
void   DibB2DFwdEvents(struct DibBridgeContext *pContext, enum DibEvent Event, union DibEventConfig * pEventConfig);
void   DibB2DFwdMsg(struct DibBridgeContext *pContext, uint32_t Nb, uint16_t *pBuf);

/* Raw Data is used for many mode */
void      DibBridgeOtherDataDone(struct DibBridgeContext * pContext, enum DibDataMode DataMode, struct DibBridgeDmaCtx * pDmaCtx);
DIBDMA    DibBridgeGetRawData(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags);
DIBSTATUS DibBridgeAllocateRawBuffer(struct DibBridgeContext *pContext, uint8_t** Buffer,uint32_t size);
DIBSTATUS DibBridgeDesallocateRawBuffer(struct DibBridgeContext *pContext, uint8_t *buffer,uint32_t size);
DIBSTATUS DibBridgeNotifyRawbufferWrite(struct DibBridgeContext *pContext, uint8_t *buffer, uint32_t len, uint8_t item);
DIBSTATUS DibBridgeGetRawBufferFull(struct DibBridgeContext *pContext, uint8_t** Buffer, FILTER_HDL * pFilterHdl);
DIBSTATUS DibBridgeNotifyRawbufferRead(struct DibBridgeContext *pContext, uint8_t * buffer, uint32_t size);


#if (DEMOD_TRACE == 1)
void DibB2DFwdDemodTrace(struct DibBridgeContext *pContext, uint32_t Nb, uint8_t *pBuf);
#endif


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
void DibBridgePesDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);
void DibBridgePcrDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);
#else                        
#define DibBridgePesDone(pContext, pDmaCtx)
#define DibBridgePcrDone(pContext, pDmaCtx)
#endif

/*************************************************************/
/*** SIPSI                                                ****/
/*************************************************************/
#if (DIB_SIPSI_DATA == 1)
void   DibBridgeSiPsiDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx *pDmaCtx);
DIBDMA DibBridgeGetSipsi(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags);

#else                        
#define DibBridgeSiPsiDone(pContext, pDmaCtx) 
#define DibBridgeGetSipsi(pContext, min, max, faddr, len, pFlags) DIB_NO_DMA
#endif

/*************************************************************/
/*** MPEFEC                                               ****/
/*************************************************************/
#if (DIB_MPEFEC_DATA == 1)
#undef   DIB_HBM_DATA
#define  DIB_HBM_DATA    1

void      DibBridgeFlushService(struct DibBridgeContext *pContext, ELEM_HDL ItemHdl, uint8_t FlushCor, uint8_t FlushRx, uint8_t Prefetch);
void      DibBridgeFreeUnusedMpeBuffer(struct DibBridgeContext *pContext);
void      DibBridgeFreeMpeBuffer(struct DibBridgeContext *pContext, uint8_t Index);
void      DibBridgeMpeDataDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);
DIBDMA    DibBridgeGetFragment(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags);
void      DibBridgeInitMpeFecCtx(struct DibBridgeContext *pContext);
void      DibBridgeInitMpeFecBuffers(struct DibBridgeContext *pContext);
void      DibBridgeFreeMpeFecBuffers(struct DibBridgeContext *pContext);

#else

#define DibBridgeFlushService(pContext, ItemHdl, FlushCor, FlushRx, Prefetch)
#define DibBridgeFreeUnusedMpeBuffer(pContext)
#define DibBridgeFreeMpeBuffer(pContext, Index)
#define DibBridgeMpeDataDone(pContext, pDmaCtx)
#define DibBridgeGetFragment(pContext, min, max, faddr, len, pFlags) DIB_NO_DMA
#define DibBridgeInitMpeFecCtx(pContext)
#define DibBridgeInitMpeFecBuffers(pContext)
#define DibBridgeFreeMpeFecBuffers(pContext)
#endif

/*************************************************************/
/*** MPEIFEC                                              ****/
/*************************************************************/
#if (DIB_MPEIFEC_DATA == 1)
#undef   DIB_HBM_DATA
#define  DIB_HBM_DATA    1

void   DibBridgeShDataDone(struct DibBridgeContext * pContext, struct DibBridgeDmaFlags * pFlags, uint32_t TableIdx);
DIBDMA DibBridgeAllocMemory(struct DibBridgeContext * pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags);
DIBDMA DibBridgeGetShFragment(struct DibBridgeContext * pContext, uint32_t Min, uint32_t Shift, uint8_t * IntAdd, uint32_t ExtAdd, uint32_t len, struct DibBridgeDmaFlags * pFlags, uint32_t External);
DIBDMA DibBridgeFillShData(struct DibBridgeContext * pContext, uint32_t Begin, uint32_t Shift, uint32_t NbCol, uint32_t Table, struct DibBridgeDmaFlags * pFlags);
DIBDMA DibBridgeFillShPattern(struct DibBridgeContext * pContext, uint32_t Begin, uint32_t Shift, uint32_t FailedSize, uint32_t CorrectSize, struct DibBridgeDmaFlags * pFlags);
void   DibBridgeInitMpeIFecCtx(struct DibBridgeContext *pContext);
void   DibBridgeFreeMpeIFecBuffers(struct DibBridgeContext *pContext);

#else

#define DibBridgeShDataDone(pContext, pFlags, TableIdx)
#define DibBridgeAllocMemory(pContext, min, max, faddr, len, pFlags)                         DIB_NO_DMA
#define DibBridgeGetShFragment(pContext, Min, Shift, IntAdd, ExtAdd, len, pFlags, External)  DIB_NO_DMA
#define DibBridgeFillShData(pContext, Begin, Shift, NbCol, Table, pFlags)                    DIB_NO_DMA
#define DibBridgeFillShPattern(pContext, Begin, Shift, FailedSize, CorrectSize, pFlags)      DIB_NO_DMA
#define DibBridgeInitMpeIFecCtx(pContext)
#define DibBridgeFreeMpeIFecBuffers(pContext)

#endif


/*************************************************************/
/*** TDMB                                                 ****/
/*************************************************************/
#if (DIB_DAB_DATA == 1)
void DibBridgeMscDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx, ELEM_HDL Item);
void DibBridgeMscPlusDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx, ELEM_HDL Item);
#else
#define DibBridgeMscDone(pContext, pDmaCtx, Item)
#define DibBridgeMscPlusDone(pContext, pDmaCtx, Item)
#endif

/*************************************************************/
/*** FIG                                                  ****/
/*************************************************************/
#if (DIB_FIG_DATA == 1)
void DibBridgeFigDone(struct DibBridgeContext * pContext, struct DibBridgeDmaCtx * pDmaCtx);
void DibBridgeInitDabCtx(struct DibBridgeContext * pContext);
#else
#define DibBridgeFigDone(pContext, pDmaCtx)
#define DibBridgeInitDabCtx(pContext)
#endif

#if (DIB_HBM_DATA == 1)
DIBSTATUS DibBridgeSliceDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);
DIBDMA    DibBridgeAssembleSlice(struct DibBridgeContext *pContext, uint32_t offs, uint32_t len, uint32_t faddr, uint32_t index, struct DibBridgeDmaFlags * pFlags);
DIBDMA    DibBridgeGetSlice(struct DibBridgeContext *pContext, uint32_t offs, uint32_t len, uint32_t faddr, uint32_t index, struct DibBridgeDmaFlags *pFlags);
void      DibBridgeSetMpeBufCor(struct DibBridgeContext *pContext, uint8_t ItemHdl);
#else
#define DibBridgeSliceDone(pContext, pDmaCtx)                              DIBSTATUS_SUCCESS
#define DibBridgeAssembleSlice(pContext, offs, len, faddr, index, pFlags)  DIB_NO_DMA
#define DibBridgeGetSlice(pContext, offs, len, faddr, index, pFlags)       DIB_NO_DMA
#define DibBridgeSetMpeBufCor(pContext, ItemHdl)
#endif

/*************************************************************/
/*** CMMB                                                  ***/
/*************************************************************/
#if (DIB_CMMB_DATA == 1)
void    DibBridgeInitCmmbCtx(struct DibBridgeContext *pContext);
void    DibBridgeCmmbRsmDone(struct DibBridgeContext *pContext, enum DibDataMode DataMode, struct DibBridgeDmaCtx *pDmaCtx);
#else
#define DibBridgeInitCmmbCtx(pContext)
#define DibBridgeCmmbRsmDone(pContext, DataMode, pDmaCtx)
#endif /* DIB_CMMB_DATA */

#endif
