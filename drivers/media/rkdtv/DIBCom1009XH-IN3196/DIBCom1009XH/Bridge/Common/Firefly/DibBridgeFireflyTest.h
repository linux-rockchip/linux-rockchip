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
* @file "DibBridgeFireflyTest.h"
* @brief Firefly sprecific bridge tests.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_FIREFLY_TEST_H
#define DIB_BRIDGE_FIREFLY_TEST_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridge.h"
#include "DibBridgeTarget.h"

#define OFFSET_RAM_EXT    0x800000
#define OFFSET_RAM_INT    0

int32_t IntBridgeFireflyTestBasicRead(struct DibBridgeContext *pContext);
int32_t IntBridgeFireflyTestRegister(struct DibBridgeContext *pContext);
int32_t IntBridgeFireflyTestInternalRam(struct DibBridgeContext *pContext);
int32_t IntBridgeFireflyTestExternalRam(struct DibBridgeContext *pContext);
uint32_t DibBridgeFireflyGetRamAddr(struct DibBridgeContext *pContext);


#endif


 
