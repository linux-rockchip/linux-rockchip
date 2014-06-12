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
* @file "DibDriverFireflyMsgNames.h"
* @brief Firefly specific messages names.
*
***************************************************************************************************/
/*#####################################################################################################
*-------- WARNING: Any change of message id must be updated in DibBridge<CHIP>Registers.h  -----------
*####################################################################################################*/

      /* Messages transmitted by the HOST */
      DIB_MSG_OUT(OUT_MSG_HBM_ACK)        /* MUST BE 0 - REQUIRES NOTIFICATION IF OFFSET IS CHANGED - DF0 */
      DIB_MSG_OUT(OUT_MSG_HOST_BUF_FAIL)  /* MUST BE 1 - REQUIRES NOTIFICATION IF OFFSET IS CHANGED - DF1/FF1 */
      DIB_MSG_OUT(OUT_MSG_REQ_VERSION)
      DIB_MSG_OUT(OUT_MSG_BRIDGE_I2C_W)
      DIB_MSG_OUT(OUT_MSG_BRIDGE_I2C_R)
      DIB_MSG_OUT(OUT_MSG_BRIDGE_APB_W)
      DIB_MSG_OUT(OUT_MSG_BRIDGE_APB_R)
      DIB_MSG_OUT(OUT_MSG_SCAN_CHANNEL)
      DIB_MSG_OUT(OUT_MSG_MONIT_DEMOD)
      DIB_MSG_OUT(OUT_MSG_CONF_GPIO)
      DIB_MSG_OUT(OUT_MSG_DEBUG_HELP)
      DIB_MSG_OUT(OUT_MSG_SUBBAND_SEL)
      DIB_MSG_OUT(OUT_MSG_ENABLE_TIME_SLICE)
      DIB_MSG_OUT(OUT_MSG_FE_FW_DL)
      DIB_MSG_OUT(OUT_MSG_FE_CHANNEL_SEARCH)
      DIB_MSG_OUT(OUT_MSG_FE_CHANNEL_TUNE)
      DIB_MSG_OUT(OUT_MSG_FE_SLEEP)
      DIB_MSG_OUT(OUT_MSG_FE_SYNC)
      DIB_MSG_OUT(OUT_MSG_CTL_MONIT)       
      DIB_MSG_OUT(OUT_MSG_CONF_SVC)                /* 19 */
      DIB_MSG_OUT(OUT_MSG_SET_HBM)
      DIB_MSG_OUT(OUT_MSG_INIT_DEMOD)
      DIB_MSG_OUT(OUT_MSG_ENABLE_DIVERSITY)
      DIB_MSG_OUT(OUT_MSG_SET_OUTPUT_MODE)
      DIB_MSG_OUT(OUT_MSG_SET_PRIORITARY_CHANNEL)
      DIB_MSG_OUT(OUT_MSG_ACK_FRG)                 /* REQUIRES NOTIFICATION IF OFFSET IS CHANGED - FF25 */
      DIB_MSG_OUT(OUT_MSG_INIT_PMU)

      /* Messages received by host */
      DIB_MSG_IN(IN_MSG_DATA)             /* MUST BE 0 - DO NOT CHANGE */
      DIB_MSG_IN(IN_MSG_FRAME_INFO)       /* MUST BE 1 - DO NOT CHANGE */
      DIB_MSG_IN(IN_MSG_CTL_MONIT)        /* MUST BE 2 - DO NOT CHANGE */
      DIB_MSG_IN(IN_MSG_ACK_FREE_ITEM)    /* MUST BE 3 - DO NOT CHANGE */
      DIB_MSG_IN(IN_MSG_DEBUG_BUF)
      DIB_MSG_IN(IN_MSG_MPE_MONITOR)
      DIB_MSG_IN(IN_MSG_RAWTS_MONITOR)
      DIB_MSG_IN(IN_MSG_END_BRIDGE_I2C_RW)
      DIB_MSG_IN(IN_MSG_END_BRIDGE_APB_RW)
      DIB_MSG_IN(IN_MSG_VERSION)
      DIB_MSG_IN(IN_MSG_END_OF_SCAN)
      DIB_MSG_IN(IN_MSG_MONIT_DEMOD)
      DIB_MSG_IN(IN_MSG_ERROR)
      DIB_MSG_IN(IN_MSG_FE_FW_DL_DONE)    
      DIB_MSG_IN(IN_MSG_EVENT)             /* 14 */
      DIB_MSG_IN(IN_MSG_ACK_CHANGE_SVC)            /* 15 */
      DIB_MSG_IN(IN_MSG_HBM_PROF)      
      
