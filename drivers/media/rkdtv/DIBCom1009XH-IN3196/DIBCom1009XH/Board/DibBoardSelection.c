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
* @file "DibBoardSelection.c"
* @brief Supported reference Design.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include "DibBoardSelection.h"
#if (EMBEDDED_FLASH_SUPPORT == 0)

struct DibDemodBoardConfig *BoardList[eMAX_NB_BOARDS] =
{
   NULL, /* DIB_BOARD_SDIO7078 */
   NULL, /* DIB_BOARD_SDIO7070 */

#if (DIB_BOARD_SDIO9080 == 1)
   &Config9080,
#else
   NULL,
#endif

   NULL, /* DIB_BOARD_STK7078 */

#if (DIB_BOARD_NIM9080MD == 1)
   &ConfigNim9080,
#else
   NULL,
#endif

   NULL, /* DIB_BOARD_STK7078MD4 */

#if (DIB_BOARD_SDIO9090 == 1)
   &Config9090,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM9090MD == 1)
   &ConfigNim9090,
#else
   NULL,
#endif

#if (DIB_BOARD_SDIO19088 == 1)
   &Config19080,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM29098 == 1)
   &Config29098,
#else
   NULL,
#endif

#if (DIB_BOARD_STK29098MD4 == 1)
   &ConfigStk29098md4,
#else
   NULL,
#endif

#if (DIB_BOARD_MOD29098SHA == 1)
   &ConfigMod29098SHA,
#else
   NULL,
#endif

   NULL,

#if (DIB_BOARD_NIM9090H == 1)
   &ConfigNim9090H,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM1009xHx == 1)
   &ConfigNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD2 == 1)
   &ConfigNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD3 == 1)
   &ConfigNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD4 == 1)
   &ConfigNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3009xHx == 1)
   &ConfigNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD2 == 1)
   &ConfigNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD3 == 1)
   &ConfigNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD4 == 1)
   &ConfigNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006X == 1)
   &ConfigNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM1009xM2D2P2 == 1)
   &ConfigNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
   &ConfigNim3009xxx,
#else
   NULL,
#endif
};
#endif
