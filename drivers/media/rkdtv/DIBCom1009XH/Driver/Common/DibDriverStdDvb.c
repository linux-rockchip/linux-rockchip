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

#if (DIB_DVB_STD == 1)

/**
 *  Check input channel parameters
 *  @param[in] pDesc: reference to the channel descriptor to check
 */
DIBSTATUS DibDriverCheckParamChannelDvb(struct DibTuneChan *pDesc)
{
   struct DibDvbTuneChan *pDescDvb = &(pDesc->Std.Dvb);

   if(((pDesc->Bw != eCOFDM_BW_8_0_MHZ) && (pDesc->Bw != eCOFDM_BW_7_0_MHZ) && (pDesc->Bw != eCOFDM_BW_6_0_MHZ) && (pDesc->Bw != eCOFDM_BW_5_0_MHZ)
                                        && (pDesc->Bw != eCOFDM_BW_4_5_MHZ) && (pDesc->Bw != eCOFDM_BW_3_5_MHZ) && (pDesc->Bw != eCOFDM_BW_3_0_MHZ) 
                                        && (pDesc->Bw != eCOFDM_BW_1_75_MHZ) && (pDesc->Bw != eCOFDM_BW_1_5_MHZ)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDesc->RFkHz < DIB_MIN_FREQUENCY) || (pDesc->RFkHz > DIB_MAX_FREQUENCY)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->Guard != eDIB_GUARD_INTERVAL_AUTO) && (pDescDvb->Guard != eDIB_GUARD_INTERVAL_1_32) && (pDescDvb->Guard != eDIB_GUARD_INTERVAL_1_16) 
                                                && (pDescDvb->Guard != eDIB_GUARD_INTERVAL_1_8) && (pDescDvb->Guard !=eDIB_GUARD_INTERVAL_1_4)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->Nfft != eDIB_FFT_AUTO) && (pDescDvb->Nfft != eDIB_FFT_2K) && (pDescDvb->Nfft !=eDIB_FFT_8K) && (pDescDvb->Nfft !=eDIB_FFT_1K) && (pDescDvb->Nfft != eDIB_FFT_4K)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->Nqam != eDIB_QAM_AUTO) && (pDescDvb->Nqam != eDIB_QAM_QPSK) && (pDescDvb->Nqam != eDIB_QAM_16QAM) && (pDescDvb->Nqam != eDIB_QAM_64QAM) && (pDescDvb->Nqam != eDIB_QAM_QPSK) && (pDescDvb->Nqam != eDIB_QAM_DQPSK)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->IntlvNative != eDIB_INTLV_NATIVE_AUTO) && (pDescDvb->IntlvNative != eDIB_INTLV_NATIVE_ON) && (pDescDvb->IntlvNative != eDIB_INTLV_NATIVE_OFF)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDesc->InvSpec != eDIB_SPECTRUM_NON_INVERSE) && (pDesc->InvSpec != eDIB_SPECTRUM_INVERSE) && (pDesc->InvSpec != eDIB_SPECTRUM_AUTO)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->VitHrch != eDIB_VIT_HRCH_AUTO) && (pDescDvb->VitHrch != eDIB_VIT_HRCH_OFF) && (pDescDvb->VitHrch != eDIB_VIT_HRCH_ON)))
         return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->VitAlpha != eDIB_VIT_ALPHA_AUTO) && (pDescDvb->VitAlpha != eDIB_VIT_ALPHA_1) && (pDescDvb->VitAlpha != eDIB_VIT_ALPHA_2) && (pDescDvb->VitAlpha != eDIB_VIT_ALPHA_4)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->VitCodeRateHp != eDIB_VIT_CODERATE_AUTO) && (pDescDvb->VitCodeRateHp != eDIB_VIT_CODERATE_1_2) && (pDescDvb->VitCodeRateHp != eDIB_VIT_CODERATE_2_3)
                                                      && (pDescDvb->VitCodeRateHp != eDIB_VIT_CODERATE_3_4) && (pDescDvb->VitCodeRateHp != eDIB_VIT_CODERATE_5_6)
                                                      && (pDescDvb->VitCodeRateHp != eDIB_VIT_CODERATE_7_8)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->VitCodeRateLp != eDIB_VIT_CODERATE_AUTO) && (pDescDvb->VitCodeRateLp != eDIB_VIT_CODERATE_1_2) && (pDescDvb->VitCodeRateLp != eDIB_VIT_CODERATE_2_3)
                                                      && (pDescDvb->VitCodeRateLp != eDIB_VIT_CODERATE_3_4) && (pDescDvb->VitCodeRateLp != eDIB_VIT_CODERATE_5_6)
                                                      && (pDescDvb->VitCodeRateLp != eDIB_VIT_CODERATE_7_8)))
      return DIBSTATUS_INVALID_PARAMETER;

   else if(((pDescDvb->VitSelectHp != eDIB_VIT_PRIORITY_AUTO) && (pDescDvb->VitSelectHp != eDIB_VIT_PRIORITY_LP) && (pDescDvb->VitSelectHp != eDIB_VIT_PRIORITY_HP)))
      return DIBSTATUS_INVALID_PARAMETER;

   return DIBSTATUS_SUCCESS;
}

#endif
