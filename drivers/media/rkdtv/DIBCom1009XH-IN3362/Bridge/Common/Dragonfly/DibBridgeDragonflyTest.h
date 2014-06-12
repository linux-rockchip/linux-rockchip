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
* @file "DibBridgeDragonflyTest.h"
* @brief Dragonfly sprecific bridge tests.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_DRAGONFLY_TEST_H
#define DIB_BRIDGE_DRAGONFLY_TEST_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeCommon.h"

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1) || (TEST_DMA == 1))
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeTestIf.h"
#include "DibBridge.h"
#include "DibBridgeDragonflyRegisters.h"
#include "DibBridgeDragonfly.h"
#include "DibBridgeDragonflyTest.h"

#if (DRIVER_AND_BRIDGE_MERGED == 0)
#include "DibBridgeTargetModule.h"
#endif /*DRIVER_AND_BRIDGE_MERGED */

#if (DIB_CHECK_DATA == 1)
#include "DibBridgePayloadCheckers.h"
#endif

#define IN_MSG_VERSION                5

#define WRAP                        211                /* Patterns are wrapped to zero when reaching WRAP-1 */

#define TEST_HOST2MAC                 0
#define TEST_MAC2HOST                 1

#define SYSRAM_START         0x70000000
#define SYSRAM_END           0x70010000
#define SYSRAM_SIZE          0x1000
#define SYSRAM_STEP          257        /* if > 0: add offset. if <0, mult by -value */

#define URAM_START           0x70100000
#define URAM_NB_BANK         1           /* Has 8 banks, but only the first ne is tested to shorten test time */
#define URAM_BANK_SIZE       0x20000
#define URAM_BANK(bank)      (URAM_START+URAM_BANK_SIZE*(bank))
#define URAM_END             0x70200000
#define URAM_SIZE            0x1000
#define URAM_STEP            257         /* if > 0: add offset. if <0, mult by -value */

#define AHBSTART_DEMOD      0x8000E000
#define AHBEND_DEMOD        0x8000FFFC

int32_t IntBridgeVoyager1TestRegister(struct DibBridgeContext *pContext);
int32_t IntBridgeNautilus1TestRegister(struct DibBridgeContext *pContext);
int32_t IntBridgeNautilus1MPTestRegister(struct DibBridgeContext *pContext);

int32_t IntBridgeDragonflyTestBasicRead(struct DibBridgeContext *pContext);
int32_t IntBridgeDragonflyTestInternalRam(struct DibBridgeContext *pContext);

int32_t IntBridgeVoyager1TestExternalRam(struct DibBridgeContext *pContext);
int32_t IntBridgeNautilus1TestExternalRam(struct DibBridgeContext *pContext);
int32_t IntBridgeDragonflyTestExternalRam(struct DibBridgeContext *pContext);

uint32_t DibBridgeDragonflyGetRamAddr(struct DibBridgeContext *pContext);

#else /* If tests are disabled */


#endif
#endif
