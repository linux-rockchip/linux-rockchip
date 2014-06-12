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
* @file "DibDriverFireflyIntDbg.h"
* @brief Firefly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_FIREFLY_INT_DBG_H
#define DIB_DRIVER_FIREFLY_INT_DBG_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriver.h"

#if (DIB_INTERNAL_DEBUG == 0)

#define IntDriverFireflyProcessMsgIntDbg(pContext, rx_nb, Data)

#else

DIBSTATUS IntDriverFireflyProcessMsgIntDbg(struct DibDriverContext *pContext, uint32_t rx_nb, uint32_t *Data);
DIBSTATUS IntDriverFireflySubmitBlock(struct DibDriverContext *pContext, struct DibBlock *pBlock);

#endif

#endif  /* DIB_DRIVER_IF_H */
