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
#ifndef DIB_BOARD_SELECTION_H
#define DIB_BOARD_SELECTION_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include "DibBoardParameters.h"
#include "DibExtAPI.h"



extern struct DibDemodPllCfg9000  DibPllConfig9080;

#if (EMBEDDED_FLASH_SUPPORT == 0)
extern struct DibDemodBoardConfig Config9080;
extern struct DibDemodBoardConfig ConfigNim9080;
extern struct DibDemodBoardConfig Config9090;
extern struct DibDemodBoardConfig ConfigNim9090;
extern struct DibDemodBoardConfig Config19080;
extern struct DibDemodBoardConfig Config29098;
extern struct DibDemodBoardConfig ConfigStk29098md4;
extern struct DibDemodBoardConfig ConfigMod29098SHA;
extern struct DibDemodBoardConfig ConfigNim9090H;
extern struct DibDemodBoardConfig ConfigNim1009xxx;
extern struct DibDemodBoardConfig ConfigNim3009xxx;
extern struct DibDemodBoardConfig ConfigNim3006xxx;

extern struct DibDemodBoardConfig *BoardList[eMAX_NB_BOARDS];
#endif
#endif
