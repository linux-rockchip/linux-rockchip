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
* @file "SdioSkeleton.c"
* @brief Target Specific.
*
***************************************************************************************************/

#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridge.h"

#if (DIBINTERF_PHY == DIBINTERF_PHY_SDIO)

#include "Sdio.h"

/*-----------------------------------------------------------
SdioInit()
-------------------------------------------------------------*/
int32_t SdioInit(struct DibBridgeContext *pContext)
{
  return DIBSTATUS_SUCCESS;
}

int SdioCmd53(struct DibBridgeContext *pContext, SDIO_CMD *Cmd)
{
  return DIBSTATUS_SUCCESS;
}

int SdioCmd52(struct DibBridgeContext *pContext, SDIO_CMD *Cmd)
{
  return DIBSTATUS_SUCCESS;
}

/*-----------------------------------------------------------
-------------------------------------------------------------*/
#endif
