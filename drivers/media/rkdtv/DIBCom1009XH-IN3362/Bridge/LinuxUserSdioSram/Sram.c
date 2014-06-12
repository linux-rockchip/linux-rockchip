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
* @file "SramSkeleton.c"
* @brief Target Specific.
*
***************************************************************************************************/
#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTargetCommon.h"
#include "DibBridge.h"

#if (DIBINTERF_PHY == DIBINTERF_PHY_SRAM)

#include "Sram.h"

/****************************************************************************
* SramInit
****************************************************************************/
void SramInit(struct DibBridgeContext *pContext)
{
   /** Sram Controller Initialization code ***/
}

#endif /* DIBINTERF_PHY_SRAM */
