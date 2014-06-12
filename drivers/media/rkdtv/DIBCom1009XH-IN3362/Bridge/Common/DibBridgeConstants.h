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
* @file "DibBridge.c"
* @brief Bridge functionality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_CONSTANTS_H
#define DIB_BRIDGE_CONSTANTS_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define IRQ_LOG     (1 << 0)
#define MAILBOX_LOG (1 << 1)
#define CHECKER_LOG (1 << 2)
#define DMA_LOG     (1 << 3)
#define MPEBUF_LOG  (1 << 4)
#define RAWTS_LOG   (1 << 5)
#define CMMB_LOG    (1 << 6)
#define PORT_LOG    (1 << 8)

#define IRQ_ERR     (IRQ_LOG << 16)
#define MAILBOX_ERR (MAILBOX_LOG << 16)
#define CHECKER_ERR (CHECKER_LOG << 16)
#define DMA_ERR     (DMA_LOG << 16)
#define PORT_ERR    (PORT_LOG << 16)
#define MPEBUF_ERR  (MPEBUF_LOG << 16)
#define RAWTS_ERR   (RAWTS_LOG << 16)
#define CMMB_ERR    (CMMB_LOG << 16)

#define DEBUG_MASK_ALL_ERRORS   0xffff0000
#define DEBUG_MASK_ALL_WARNINGS 0x0000ffff

#endif
