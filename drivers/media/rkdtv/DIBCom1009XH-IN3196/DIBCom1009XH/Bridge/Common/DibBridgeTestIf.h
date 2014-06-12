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
* @file "DibBridgeTestIf.h"
* @brief Test Bridge Interface.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_TEST_IF_H
#define DIB_BRIDGE_TEST_IF_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define BASIC_READ_TEST    1   
#define REGISTER_TEST      2
#define INT_RAM_TEST       4
#define EXT_RAM_TEST       8
#define FAST_RAM_TEST     16   
#define PERF_TEST         32   
#define FULL_TEST         63   

/**** Test Configuration ****/
#define WORD_TEST_LOOP   100   /* Number of register test */
#define RAM_TEST_LOOP    100   /* Number of RAM test      */
#define RAM_TEST_SIZE   4096   /* Test RAM Buffer Size    */

#define REGISTER_MODE     DIBBRIDGE_BIT_MODE_16
#define DATA_BYTE_MODE    DIBBRIDGE_BIT_MODE_8

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))
void IntBridgeTestIfWriteFastMemory(struct DibBridgeContext *pContext, uint8_t *Buffer, uint32_t Size, uint32_t Offset);
void IntBridgeTestIfReadFastMemory(struct DibBridgeContext *pContext, uint8_t *Buffer, uint32_t Size,  uint32_t Offset);
void IntBridgeInitRamBuf(uint8_t Val, uint32_t Size);
int  DibBridgeTestIf(struct DibBridgeContext *pContext,uint8_t Mode);
#endif

#if (TEST_TRANSFERT == 1)
void DibBridgeSwap32(uint8_t * buf, int32_t Size)
void DibBridgeTestDmaTransfert(struct DibBridgeContext *pContext, uint32_t add, uint32_t *dst, uint32_t DmaLen)
#endif

#endif
