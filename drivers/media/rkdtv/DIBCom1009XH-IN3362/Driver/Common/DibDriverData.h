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
* @file "DibDriverData.h"
* @brief Generic Driver interface.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DATA_H
#define DIB_DRIVER_DATA_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibExtAPI.h"

struct DibDriverContext;

/*************************************************************/
/*** RAWTS / TDMB                                             ****/
/*************************************************************/
#if ((DIB_RAWTS_DATA == 1) || (DIB_TDMB_DATA == 1))
DIBSTATUS   DibDriverGetMonitoringRawTs(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit);
#else
#define     DibDriverGetMonitoringRawTs(pContext, ItemHdl, pDvbTMonit, ClearMonit)    DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** RAWTS                                                ****/
/*************************************************************/
#if (DIB_RAWTS_DATA == 1)
DIBSTATUS DibDriverCheckParamRawTs(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid);
#else
#define DibDriverCheckParamRawTs(pContext, pFilterDesc, Pid)                      DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** ELEMENTARY STREAM                                    ****/
/*************************************************************/
#if (DIB_PES_DATA == 1)
DIBSTATUS DibDriverCheckParamPes(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid, enum DibDataType TypeFilter);
#else                        
#define DibDriverCheckParamPes(pContext, pFilterDesc, Pid, TypeFilter)  DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** SIPSI                                                ****/
/*************************************************************/
#if (DIB_SIPSI_DATA == 1)
DIBSTATUS DibDriverCheckParamSiPsi(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid);
DIBSTATUS DibDriverSetConfigSiPsi(struct DibDriverContext *pContext, union DibParamConfig * pParamConfig);
DIBSTATUS DibDriverGetConfigSiPsi(struct DibDriverContext *pContext, union DibParamConfig *pParamConfig);
void      DibDriverProcessSiPsiBuffer(struct DibDriverContext * pContext, uint32_t Size, uint8_t *pBuf);
#else                        
#define DibDriverCheckParamSiPsi(pContext, pFilterDesc, Pid) DIBSTATUS_ERROR
#define DibDriverSetConfigSiPsi(pContext, pParamConfig)      DIBSTATUS_ERROR
#define DibDriverGetConfigSiPsi(pContext, pParamConfig)      DIBSTATUS_ERROR
#define DibDriverProcessSiPsiBuffer(pContext, Size, pBuf) 
#endif

/*************************************************************/
/*** MPEFEC                                               ****/
/*************************************************************/
#if (DIB_MPEFEC_DATA == 1)
DIBSTATUS DibDriverGetMonitoringMpeFec(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit);
DIBSTATUS DibDriverCheckParamMpeFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid);
DIBSTATUS DibDriverGetPidMode(struct DibDriverContext *pContext, union DibParamConfig *pParamConfig);
#else
#define DibDriverGetMonitoringMpeFec(pContext, ItemHdl, pDvbHMonit, ClearMonit)  DIBSTATUS_ERROR
#define DibDriverCheckParamMpeFec(pContext, pFilterDesc, Pid)                    DIBSTATUS_ERROR
#define DibDriverGetPidMode(pContext, pParamConfig)                              DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** MPEIFEC                                              ****/
/*************************************************************/
#if (DIB_MPEIFEC_DATA == 1)
DIBSTATUS DibDriverGetMonitoringMpeIFec(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit);
DIBSTATUS DibDriverCheckParamMpeIFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid);
#else
#define DibDriverGetMonitoringMpeIFec(pContext, ItemHdl, pDvbSHMonit, ClearMonit) DIBSTATUS_ERROR
#define DibDriverCheckParamMpeIFec(pContext, pFilterDesc, Pid)                    DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** DABaudio / TDMB / FIC                                ****/
/*************************************************************/ 
#if (DIB_FIG_DATA == 1)
DIBSTATUS DibDriverGetMonitoringFic(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit);
#else
#define DibDriverGetMonitoringFic(pContext, ItemHdl, pDataMonit, ClearMonit) DIBSTATUS_ERROR
#endif

#if (DIB_DAB_DATA == 1)
DIBSTATUS DibDriverGetMonitoringDabAudio(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit);
DIBSTATUS DibDriverCheckParamDabAudio(struct DibDriverContext *pContext, union DibFilters *pFilterDesc);
#else
#define DibDriverGetMonitoringDabAudio(pContext, ItemHdl, pDataMonit, ClearMonit)   DIBSTATUS_ERROR
#define DibDriverCheckParamDabAudio(pContext, pFilterDesc)                          DIBSTATUS_ERROR
#endif

#if (DIB_DABPACKET_DATA == 1)
DIBSTATUS DibDriverGetMonitoringDabPacket(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, union DibDataMonit * pDataMonit, uint8_t ClearMonit);
DIBSTATUS DibDriverCheckParamDabPacket(struct DibDriverContext * pContext, union DibFilters * pFilterDesc);
#else
#define DibDriverGetMonitoringDabPacket(pContext, ItemHdl, pDataMonit, ClearMonit)  DIBSTATUS_ERROR
#define DibDriverCheckParamDabPacket(pContext, pFilterDesc)                         DIBSTATUS_ERROR
#endif

#if (DIB_TDMB_DATA == 1)
DIBSTATUS DibDriverGetMonitoringTdmb(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, union DibDataMonit * pDataMonit, uint8_t ClearMonit);
DIBSTATUS DibDriverCheckParamTdmb(struct DibDriverContext * pContext, union DibFilters * pFilterDesc);
#else
#define DibDriverGetMonitoringTdmb(pContext, ItemHdl, pDataMonit, ClearMonit)       DIBSTATUS_ERROR
#define DibDriverCheckParamTdmb(pContext, pFilterDesc)                              DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** CMMB                                                 ****/
/*************************************************************/
#if(DIB_CMMB_DATA == 1)
DIBSTATUS DibDriverGetMonitoringCmmb(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit);
void    DibDriverProcessCmmbRsMatrix(struct DibDriverContext *pContext, uint32_t Size, uint8_t *pBuf, ELEM_HDL ItemIndex, uint32_t NbRows, uint32_t NbCols);
void    DibInitChannelDecoder(struct DibDriverContext *pContext);
#else
#define DibDriverGetMonitoringCmmb(pContext, ItemHdl, pDataMonit, ClearMonit) DIBSTATUS_ERROR
#define DibDriverProcessCmmbRsMatrix(pContext, Size, pBuf, ItemIndex, NbRows, NbCols)
#define DibInitChannelDecoder(pContext);
#endif /* DIB_CMMB_DATA */

#endif  /* DIB_DRIVER_IF_H */
