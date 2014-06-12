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
* @file "DibDriverConstants.h"
* @brief Generic Driver constants.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_CONSTANTS_H
#define DIB_DRIVER_CONSTANTS_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define TUNE_TIMEOUT_MS                 3000
#define MONIT_TIMEOUT_MS                3000
#define EMB_VERSION_TIMEOUT_MS          500

#define DIB_INTERF_DRIVER_VERSION       DIB_SET_VER(8, 13, 18)         /**< sdk revision */

#define DIB_MAX_NB_PIDS                 0x2000      /**< Maximum number of Pids (13 bits) */
#define DIB_ALL_PIDS                    0x2000      /**< All pids are wanted (not parsing) */
#define DIB_POSITIVE_PIDS               0x2001      /**< All pids except null packets are wanted */

/* MSC Stream Type (DAB specific) */
#define DIB_MSC_STREAM_AUDIO                 0
#define DIB_MSC_STREAM_AUDIO_PLUS            1
#define DIB_MSC_MAX_NB_TYPE                  2

/* Number of Data Channels. */
#define NUM_DATA_CHANNELS                    2

#define MAX_SECT_LENGTH                   4096

#define CMMB_MAX_MF_LENGTH              150000      /* TODO Handle that service by service. */

#define SIPSI_BUFFER_SIZE                 4096
#define PES_VIDEO_BUFFER_SIZE           150000
#define PES_AUDIO_BUFFER_SIZE             4096
#define PES_OTHER_BUFFER_SIZE             4096

#define TP_SIZE                            188

#define DIB_DAB_MAX_NB_CHANNELS             64
#define DIB_FIC_MAX_NB_CHANNELS              4
#define DIB_DABPACKET_MAX_NB_ADDRESS      1024

#define I2C_BRIDGE_FRAME_SIZE                8

#define DIB_TASK_QUEUE_SIZE                        10
#define DIB_TASK_SIPSI_BUFFER_SIZE               4096
#define DIB_TASK_RAW_TS_BUFFER_SIZE   (256 * TP_SIZE)
#define DIB_TASK_PES_VIDEO_BUFFER_SIZE          65536
#define DIB_TASK_PES_AUDIO_BUFFER_SIZE           4096
#define DIB_TASK_PES_OTHER_BUFFER_SIZE           4096
#define DIB_TASK_PCR_BUFFER_SIZE                    7

#define MPE_SVC_MSK                         0x07
#define ALL_SVC_MSK                         0x1f
#define CONF_SVC_PREFETCH                   0x20
#define CONF_SVC_TSLICED                    0x40
#define CONF_SVC_ADDING                     0x80

#define DIB_INVALID_NUMBER  -1                  /**< Invalid length. Used for packet demux */

#define MAX_BURST_TIME_NO_TIME_SLICE  0

/* Debug system  */
#define SOFT_LOG              (1 << 0)
#define MSG_LOG               (1 << 1)
#define SOFT_IF_LOG           (1 << 2)
#define PACKET_LOG            (1 << 3)
#define SIRQ_LOG              (1 << 4)
#define FRONTEND_LOG          (1 << 5)
#define CALLBACK_LOG          (1 << 6)
#define FRAGMENT_LOG          (1 << 7)

#define PORT_LOG              (1 << 8)
#define MPE_MNT_LOG           (1 << 9)
#define MPE_HBM_PROF          (1 << 10)

#define SOFT_ERR              (SOFT_LOG << 16)
#define MSG_ERR               (MSG_LOG << 16)
#define SOFT_IF_ERR           (SOFT_IF_LOG << 16)
#define PACKET_ERR            (PACKET_LOG << 16)
#define SIRQ_ERR              (SIRQ_LOG << 16)
#define FRONTEND_ERR          (FRONTEND_LOG << 16)
#define CALLBACK_ERR          (CALLBACK_LOG << 16)
#define FRAGMENT_ERR          (FRAGMENT_LOG << 16)

#define PORT_ERR              (PORT_LOG << 16)

#define DEFAULT_DEBUG_MASK    (0xFFFF0000)

#endif
