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
* @file "DibDriverIf.c"
* @brief Driver Interface.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriver.h"

#if (DIB_ISDBT_STD == 1)

static DIBSTATUS DibDriverCheckIsdbtLayerParam(struct DibIsdbtTuneChan *pDescIsdbt)
{
   struct DibIsdbtLayer *pLayer;
   int i;

   for(i = 0; i<3; i++) {
      pLayer = &pDescIsdbt->layer[i];

      if(pLayer->NbSegments != 0) {
          if(((pLayer->Constellation != eDIB_QAM_AUTO) && (pLayer->Constellation != eDIB_QAM_QPSK) && (pLayer->Constellation != eDIB_QAM_16QAM) && (pLayer->Constellation != eDIB_QAM_64QAM) && (pLayer->Constellation != eDIB_QAM_QPSK) && (pLayer->Constellation != eDIB_QAM_DQPSK)))
             return DIBSTATUS_INVALID_PARAMETER;

          else if(((pLayer->NbSegments != 0) && (pLayer->CodeRate != eDIB_VIT_CODERATE_AUTO) && (pLayer->CodeRate != eDIB_VIT_CODERATE_1_2) && (pLayer->CodeRate != eDIB_VIT_CODERATE_2_3) && (pLayer->CodeRate != eDIB_VIT_CODERATE_3_4) && (pLayer->CodeRate != eDIB_VIT_CODERATE_5_6) && (pLayer->CodeRate != eDIB_VIT_CODERATE_7_8)))
             return DIBSTATUS_INVALID_PARAMETER;
      }
   }

   return DIBSTATUS_SUCCESS;
}

/**
 *  Check input channel parameters
 *  @param[in] pDesc: reference to the channel descriptor to check
 */
DIBSTATUS DibDriverCheckParamChannelIsdbt(struct DibTuneChan *pDesc)
{
   struct DibIsdbtTuneChan *pDescIsdbt = &(pDesc->Std.Isdbt);

   if(pDesc->Bw != eCOFDM_BW_6_0_MHZ)
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDesc->RFkHz < DIB_MIN_FREQUENCY) || (pDesc->RFkHz > DIB_MAX_FREQUENCY)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDesc->InvSpec != eDIB_SPECTRUM_NON_INVERSE) && (pDesc->InvSpec != eDIB_SPECTRUM_INVERSE) && (pDesc->InvSpec != eDIB_SPECTRUM_AUTO)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescIsdbt->Guard != eDIB_GUARD_INTERVAL_AUTO) && (pDescIsdbt->Guard != eDIB_GUARD_INTERVAL_1_32) && (pDescIsdbt->Guard != eDIB_GUARD_INTERVAL_1_16) && (pDescIsdbt->Guard != eDIB_GUARD_INTERVAL_1_8) && (pDescIsdbt->Guard !=eDIB_GUARD_INTERVAL_1_4)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescIsdbt->Nfft != eDIB_FFT_AUTO) && (pDescIsdbt->Nfft != eDIB_FFT_2K) && (pDescIsdbt->Nfft !=eDIB_FFT_8K) && (pDescIsdbt->Nfft !=eDIB_FFT_1K) && (pDescIsdbt->Nfft != eDIB_FFT_4K)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescIsdbt->SbMode != 0) && (pDescIsdbt->SbMode != 1)) && ((pDescIsdbt->SbMode != 0xFF)))
      return DIBSTATUS_INVALID_PARAMETER;

   return DibDriverCheckIsdbtLayerParam(pDescIsdbt);
}

#endif
