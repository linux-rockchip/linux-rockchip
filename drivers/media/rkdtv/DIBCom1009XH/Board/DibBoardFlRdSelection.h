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
* @file "DibBoardSelection.h"
* @brief Supported Reference Designs configurations.
*
***************************************************************************************************/
#ifndef DIB_BOARDFL_SELECTION_H
#define DIB_BOARDFL_SELECTION_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (EMBEDDED_FLASH_SUPPORT == 1)

#include "DibBoardParameters.h"
#include "DibExtAPI.h"

extern struct DibDemodBoardConfig ConfigFlRdNim1009xxx;
extern struct DibDemodBoardConfig ConfigFlRdNim3009xxx;
extern struct DibDemodBoardConfig ConfigFlRdNim3006X;

extern struct DibDemodBoardConfig *BoardFlrdList[eMAX_NB_BOARDS];

#endif /* EMBEDDED_FLASH_SUPPORT*/
#endif
