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
* @file "DibDriverPacketDemux.h"
* @brief Data processing function prototypes.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_PACKET_DEMUX_H
#define DIB_DRIVER_PACKET_DEMUX_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/* Fwd declare */
struct   DibDriverDataInfoUp;
void     DibDriverRawDataRead(struct DibDriverContext * pContext, struct DibDriverDataInfoUp dataInfo);

#endif
