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
* @file "DibFirmwareSelection.h"
* @brief Board structure definition.
*
***************************************************************************************************/
#ifndef DIB_OCTOCODE_SELECTION_H
#define DIB_OCTOCODE_SELECTION_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverTargetTypes.h"

struct DibFirmwareDescription
{
   const char     FirmwareName[80];
   const char     FirmwareInfo[200];
   const uint8_t *FirmwareText;
   uint32_t       FirmwareLength;
};

extern struct DibFirmwareDescription FirmwareList[];

#endif
