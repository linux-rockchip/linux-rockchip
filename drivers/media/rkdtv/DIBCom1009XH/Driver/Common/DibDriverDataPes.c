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
* @file "DibDriverDataPes.c"
* @brief Driver Pes data handling.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriver.h"

#if (DIB_PES_DATA == 1)

/**
 * Check Pes Parameters
 * @param[in]  pContext: driver context
 * @param[in]  pFilterDesc: filter descripter handler.
 * @param[out] Pid: Pointer to output Pid for filter coherency check
 * @param[in]  TypeFilter: Reset internal monitoring information after retreiving them.
 */
DIBSTATUS DibDriverCheckParamPes(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid, enum DibDataType TypeFilter)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   switch(TypeFilter)
   {
      case ePESVIDEO:
         if(pFilterDesc->PesVideo.Pid >= DIB_MAX_NB_PIDS)
            goto End;

         *Pid = pFilterDesc->PesVideo.Pid;
         break;

      case ePESAUDIO:
         if(pFilterDesc->PesAudio.Pid >= DIB_MAX_NB_PIDS)
            goto End;

         *Pid = pFilterDesc->PesAudio.Pid;
         break;

      case ePESOTHER:
         if(pFilterDesc->PesOther.Pid >= DIB_MAX_NB_PIDS)
            goto End;

         *Pid = pFilterDesc->PesOther.Pid;
         break;

      case ePCR:
         if(pFilterDesc->Pcr.Pid >= DIB_MAX_NB_PIDS)
            goto End;

         *Pid = pFilterDesc->Pcr.Pid;
         break;
      default:
         goto End;
         break;
   }
         
   Status = DIBSTATUS_SUCCESS;

End:
   return Status;
}

#endif
