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
* @file "DibBridgeTargetModule.h"
* @brief Target specific implementation.
*
***************************************************************************************************/
#ifndef __DIB_BRIDGE_TARGET_MODULE_H_
#define __DIB_BRIDGE_TARGET_MODULE_H_

/*
 * The major device number. We can't rely on dynamic
 * registration any more, because ioctls need to know
 * it.
 */
#define MAJOR_NUM                     232
#define DEVICE_FILE_NAME "/dev/dibbridge"
#define DEVICE_NAME "dibbridge"


/* IOCTL must start for SIOCDEVPRIVATE + 0 and max 16 */
#define REQUEST_REG_DOWN                (SIOCDEVPRIVATE + 1)
#define REQUEST_REG_UP                  (SIOCDEVPRIVATE + 2)
#define REQUEST_MSG_DOWN                (SIOCDEVPRIVATE + 3)
#define REQUEST_BRIDGE_MODULE_SHUTDOWN  (SIOCDEVPRIVATE + 4)
#define REQUEST_BRIDGE_POSTINIT         (SIOCDEVPRIVATE + 5)
#define REQUEST_READ_MSG                (SIOCDEVPRIVATE + 6)
#define REQUEST_DATAPATH_INFO           (SIOCDEVPRIVATE + 7)
#define REQUEST_TRANSERT_BUF            (SIOCDEVPRIVATE + 8)
#define REQUEST_READ_DATA               (SIOCDEVPRIVATE + 9)
#define REQUEST_BRIDGE_MODULE_INIT      (SIOCDEVPRIVATE + 10)

/* Private SIOC flags for IOCTL */
#define SIOC_ENABLE_HS          1
#define SIOC_ENABLE_IRQ         2

/* Msg types used between Bridge to Driver communication */
#define UPMESSAGE_TYPE_SIPSI     1
#define UPMESSAGE_TYPE_MSG       2
#define UPMESSAGE_TYPE_DTRACE    3
#define UPMESSAGE_TYPE_DATA      4
#define UPMESSAGE_TYPE_EVENT     5

#define MAX_BRIDGE_DRIVER_BUFFER 4096

struct IoctlHead
{
  uint32_t Nb;

  struct _uparam
  {
    /* if REQUEST_REG_* */
    uint32_t address;
    uint32_t Mode;
  } param;

  uint8_t   *Buffer;
  /*uint8_t   *read_buffer;*/

};

struct IoctlSvcInfo
{
   uint8_t item;
   uint8_t filter;
   uint8_t svc;
   uint8_t DataMode;
   uint8_t DataType;
   uint32_t StreamId;
};

struct IoctlReadData
{
   uint32_t    SizeBuffer;
   uint8_t *   HostBuffer;
   uint8_t     Mode;
   uint8_t     FilterHdl;
};

struct IoctlInit
{
   void     *BoardHdl;
   uint32_t  ChipSelect;
   uint32_t  Config[12];
};

#endif
