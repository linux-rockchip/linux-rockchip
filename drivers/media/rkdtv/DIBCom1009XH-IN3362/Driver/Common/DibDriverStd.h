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
* @file "DibDriverStd.h"
* @brief Generic Driver interface.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_STD_H
#define DIB_DRIVER_STD_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibExtAPI.h"

/*************************************************************/
/*** DVB                                                  ****/
/*************************************************************/
#if (DIB_DVB_STD == 1)
DIBSTATUS DibDriverCheckParamChannelDvb(struct DibTuneChan *pDesc);
#else                                         
#define DibDriverCheckParamChannelDvb(pDesc) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** DVB-SH                                               ****/
/*************************************************************/
#if (DIB_DVBSH_STD == 1)
DIBSTATUS DibDriverCheckParamChannelDvbSh(struct DibTuneChan *pDesc);
#else                                         
#define DibDriverCheckParamChannelDvbSh(pDesc) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** DAB                                                  ****/
/*************************************************************/
#if (DIB_DAB_STD == 1)
DIBSTATUS DibDriverCheckParamChannelDab(struct DibTuneChan *pDesc);
#else
#define DibDriverCheckParamChannelDab(pDesc) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** CMMB                                                 ****/
/*************************************************************/
#if (DIB_CMMB_STD == 1)
DIBSTATUS DibDriverCheckParamChannelCmmb(struct DibTuneChan *pDesc);
#else
#define DibDriverCheckParamChannelCmmb(pDesc) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** ISDBT                                                ****/
/*************************************************************/
#if (DIB_ISDBT_STD == 1)
DIBSTATUS DibDriverCheckParamChannelIsdbt(struct DibTuneChan *pDesc);
#else
#define DibDriverCheckParamChannelIsdbt(pDesc) DIBSTATUS_ERROR
#endif

/*************************************************************/
/*** CTTB                                                ****/
/*************************************************************/
#if (DIB_CTTB_STD == 1)
DIBSTATUS DibDriverCheckParamChannelCttb(struct DibTuneChan * pDesc);
#else
#define DibDriverCheckParamChannelCttb(pDesc)   DIBSTATUS_ERROR
#endif

#endif  /* DIB_DRIVER_IF_H */
