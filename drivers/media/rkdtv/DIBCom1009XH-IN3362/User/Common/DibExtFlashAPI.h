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
* @file "DibExtFlashAPI.h"
* @brief SDK Internal External Api functions specific to embedded flash support.
*
***************************************************************************************************/

#ifndef DIB_EXT_FLASH_API_H
#define DIB_EXT_FLASH_API_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if  (EMBEDDED_FLASH_SUPPORT == 1)
#include "DibExtAPI.h"

DIBSTATUS DibOpenFromFlash(struct DibDriverContext **pContextAddr, enum DibBoardType BoardType, BOARD_HDL BoardHdl);

#if (WRITE_FLASH_SUPPORT != eWRFL_NONE)
DIBSTATUS DibOpenWriteFlash(struct DibDriverContext **pContextAddr, enum DibBoardType BoardType, BOARD_HDL BoardHdl);
DIBSTATUS DibProgramFlash(struct DibDriverContext *pContext, char* filename);
DIBSTATUS DibProgramFlashStatus(struct DibDriverContext *pContext);
#endif /* WRITE_FLASH_SUPPORT */

#endif /* EMBEDDED_FLASH_SUPPORT */

#endif /* DIB_EXT_FLASH_API_H */
