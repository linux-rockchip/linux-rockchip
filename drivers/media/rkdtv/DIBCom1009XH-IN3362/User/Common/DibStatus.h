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
* @file "DibStatus.h"
* @brief Definition of status returned by DiBcom API functions.
*
***************************************************************************************************/
#ifndef DIB_STATUS_H
#define DIB_STATUS_H

typedef char DIBSTATUS;

/**
 * Possible status return by DiBcom API functions
 */
#define DIBSTATUS_SUCCESS              0x00
#define DIBSTATUS_ERROR                0x01
#define DIBSTATUS_RESOURCES            0x02
#define DIBSTATUS_TIMEOUT              0x03
#define DIBSTATUS_INVALID_PARAMETER    0x04
#define DIBSTATUS_FRONT_END_COM_ERROR  0x05
#define DIBSTATUS_FRONT_END_DEV_ERROR  0x06
#define DIBSTATUS_CONTINUE             0x07
#define DIBSTATUS_PARTIAL              0x08
#define DIBSTATUS_INSUFFICIENT_MEMORY  0x09
#define DIBSTATUS_ABORTED              0x0A
#define DIBSTATUS_INTERRUPTED          0x0B

enum DibDataMode
{
   eUNSET = 0, /* Default reset value, cannot be used.        */
   eIP,        /* Data is forwarded directly to the IP stack. */
   eCLBACK     /* Data is retrieved via callback function.    */
};

#endif
