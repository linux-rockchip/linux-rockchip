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

#if (DIBCOM_TEST_MODE == 1)
#include "Hook.h"
#endif

/****************************************************************************
* SramInit
****************************************************************************/
void SramInit(struct DibBridgeContext *pContext)
{
   /** Sram Controller Initialization code ***/

#if (DIBCOM_TEST_MODE == 1)
   HookInit(pContext,DIBBRIDGE_MODE_SRAM,pContext->BoardHdl);
   if(pContext->DibChip == DIB_FIREFLY)
     DibBridgeWriteReg16(pContext, 1817, 3);
#endif
}

#if (DIBCOM_TEST_MODE == 1)
/****************************************************************************
* SramWrite
****************************************************************************/
void SramWrite(struct DibBridgeContext *pContext, uint32_t address, uint8_t Data)
{
   HookSramBusWrite(pContext,address,Data);
}

/****************************************************************************
* SramRead
****************************************************************************/
uint8_t SramRead(struct DibBridgeContext *pContext, uint32_t address)
{
   uint8_t Data = 0;
   HookSramBusRead(pContext,address,&Data);
   return (Data);
}
#endif

#endif /* DIBINTERF_PHY_SRAM */
