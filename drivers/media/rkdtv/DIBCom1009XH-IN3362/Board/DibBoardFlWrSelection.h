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
#ifndef DIB_BOARDFLW_SELECTION_H
#define DIB_BOARDFLW_SELECTION_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if ((EMBEDDED_FLASH_SUPPORT == 1) && (WRITE_FLASH_SUPPORT != eWRFL_NONE))

#include "DibBoardParameters.h"
#include "DibExtAPI.h"

extern struct DibDemodBoardConfig ConfigFlWrNim1009xxx;
extern struct DibDemodBoardConfig ConfigFlWrNim3009xxx;
extern struct DibDemodBoardConfig ConfigFlWrNim3006X;

extern struct DibDemodBoardConfig *BoardFlwrList[eMAX_NB_BOARDS];

#endif /* ((EMBEDDED_FLASH_SUPPORT == 1) && (WRITE_FLASH_SUPPORT != eWRFL_NONE)) */

#endif
