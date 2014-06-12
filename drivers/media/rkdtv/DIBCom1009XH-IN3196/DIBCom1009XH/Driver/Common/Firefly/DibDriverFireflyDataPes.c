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
* @file "DibDriverFireflyDataPes.c"
* @brief Firefly Driver specific Pes Data handler
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverFirefly.h"

#if (DIB_PES_DATA == 1)

DIBSTATUS DibDriverFireflyAddItemPesVideo(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo)
{
   PidInfo->PidTarget = eTARGET_PES_VIDEO;
   PidInfo->Pid       = pFilterDesc->PesVideo.Pid;
   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddItemPesAudio(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo)
{
   PidInfo->PidTarget = eTARGET_PES_AUDIO;
   PidInfo->Pid       = pFilterDesc->PesAudio.Pid;
   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddItemPesOther(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo)
{
   PidInfo->PidTarget = eTARGET_PES_OTHER;
   PidInfo->Pid       = pFilterDesc->PesOther.Pid;
   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddItemPesPcr(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo)
{
   PidInfo->PidTarget = eTARGET_PCR;
   PidInfo->Pid       = pFilterDesc->Pcr.Pid;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddPidPesVideo(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode)
{
   pContext->FilterInfo[FilterHdl].NbActivePids++;
   if(pContext->FilterInfo[FilterHdl].NbActivePids > DIB_MAX_PES_SERVICES)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverif_PidFilter: can only handle %d PES video PID" CRA, DIB_MAX_PES_SERVICES));
      return DIBSTATUS_ERROR;
   }
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverif_PidFilter: Active PES video PIDs: %d" CRA, pContext->FilterInfo[FilterHdl].NbActivePids));

   *format = FORMAT_PES_VIDEO;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddPidPesAudio(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode)
{
   pContext->FilterInfo[FilterHdl].NbActivePids++;
   if(pContext->FilterInfo[FilterHdl].NbActivePids > DIB_MAX_PES_SERVICES)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverif_PidFilter: can only handle %d PES audio PID" CRA, DIB_MAX_PES_SERVICES));
      return DIBSTATUS_ERROR;
   }
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverif_PidFilter: Active PES audio PIDs: %d" CRA, pContext->FilterInfo[FilterHdl].NbActivePids));

   *format = FORMAT_PES_AUDIO;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddPidPesOther(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode)
{
   pContext->FilterInfo[FilterHdl].NbActivePids++;
   if(pContext->FilterInfo[FilterHdl].NbActivePids > DIB_MAX_PES_SERVICES)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverif_PidFilter: can only handle %d PES other PID" CRA, DIB_MAX_PES_SERVICES));
      return DIBSTATUS_ERROR;
   }
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverif_PidFilter: Active PES other PIDs: %d" CRA, pContext->FilterInfo[FilterHdl].NbActivePids));

   *format = FORMAT_PES_OTHER;
   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddPidPesPcr(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode)
{
   pContext->FilterInfo[FilterHdl].NbActivePids++;
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].NbActivePids <= DIB_MAX_PCR_SERVICES);
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverif_PidFilter: Active PCR PIDs: %d" CRA, pContext->FilterInfo[FilterHdl].NbActivePids));

   *format = FORMAT_PCR;
      
   return DIBSTATUS_SUCCESS;
}

#endif


#endif /* USE_FIREFLY */
