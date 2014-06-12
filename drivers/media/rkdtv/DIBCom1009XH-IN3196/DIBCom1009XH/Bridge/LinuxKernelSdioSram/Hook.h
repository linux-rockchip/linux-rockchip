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
* @file "Hook.h"
* @brief Hook Access Function Prototypes.
*
***************************************************************************************************/
#ifndef HOOK_H
#define HOOK_H

/* This is for the USB bridge used as a physical interface */
/* max len for a Data packet over EP0 */
#define MAX_EP0_DATA_PACKET_LEN 64

/* max len for a packet over EP1 */
#define MAX_EP1_DATA_PACKET_LEN 64

/* max len for the DATA packet of a read request over EP0 */
#define MAX_READ_DATA_PACKET_LEN (2*MAX_EP0_DATA_PACKET_LEN-4)

/* up to 19 Data packets per write request (burst) */
#define MAX_WRITE_BURST_LEN  (19*MAX_EP0_DATA_PACKET_LEN-2)
/* up to 2 Data packets per write request (NO burst) */
#define MAX_WRITE_NORMAL_LEN (2*MAX_EP0_DATA_PACKET_LEN-2)
#define WRITE_PACKET_HEADER_LEN (6)

#define DIB_IF_MAX_WRITE_B      (MAX_WRITE_BURST_LEN  - WRITE_PACKET_HEADER_LEN)
#define DIB_IF_MAX_WRITE_N      (MAX_WRITE_NORMAL_LEN - WRITE_PACKET_HEADER_LEN)
#define MAX_WRITE_DATA_PACKET_LEN MAX_WRITE_NORMAL_LEN

#define DIB_IF_MAX_WRITE        (MAX_WRITE_DATA_PACKET_LEN-8)
#define DIB_IF_MAX_READ_SMALL    MAX_READ_DATA_PACKET_LEN
#define DIB_IF_MAX_READ_BIG     (61*1024)

/* flags to select different read/write features */
/* access width (as defined in DragonFly)        */
#define DVB_H_SRAM_WIDTH_MASK 3
#define DVB_H_SRAM_8BIT       0
#define DVB_H_SRAM_16BIT      1
#define DVB_H_SRAM_32BIT      2

/* select Dragonfly */
#define DVB_H_SRAM_DFLY       4
#define DVB_H_SRAM_TEST       8

/************************************************************/
/* IO control codes                                         */
/************************************************************/

/* Errors and warnings */
#define HOOK_USB_OFFSET 0
#define FILE_DESCRIPTOR_OPEN_ERROR            (HOOK_USB_OFFSET-7)
#define FILE_DESCRIPTOR_CLOSE_ERROR           (HOOK_USB_OFFSET-12)

/********************** HookInit ************************************/
/* select DVB-H Mode (SubMode == 0 for SRAM 1 for SDIO) */
int HookInit(struct DibBridgeContext *pContext, uint32_t SubMode, BOARD_HDL BoardHdl);

/********************** HookClose ***********************************/
int HookClose(struct DibBridgeContext *pContext);

/********************** HookSramWrite **********************************/
int HookSramWrite(struct DibBridgeContext *pContext, uint32_t address,
                  uint8_t byteMode, uint32_t Nb,
                  uint8_t *buf);

/********************** HookSramRead ************************************/
int HookSramReadSmall(struct DibBridgeContext *pContext, uint32_t address,
                      uint8_t byteMode, uint32_t Nb,
                      uint8_t *buf);
int HookSramReadBig(struct DibBridgeContext *pContext, uint32_t address,
                    uint8_t byteMode, uint32_t Nb,
                    uint8_t *buf);

/********************** HoookSetClock   ****************************************/
int HookSetClock(struct DibBridgeContext *pContext,
                 uint16_t EnPll,
                 uint16_t PllSrc,
                 uint16_t PllRange,
                 uint16_t PllPrediv, uint16_t PllLoopdiv,
                 uint16_t FreeDiv, uint16_t DsuScaler
                 /* dsuScaler = ((ahbclk Frequency) / 921600) - 0.5 */);
int HookSpiBusInit(struct DibBridgeContext *pContext);
int HookSpiBusRead(struct DibBridgeContext *pContext, unsigned char *Buf, unsigned int);
int HookSpiBusWrite(struct DibBridgeContext *pContext,unsigned char *Buf, unsigned int); 
int HookSramBusRead(struct DibBridgeContext *pContext, uint32_t address, uint8_t *Data);
int HookSramBusWrite(struct DibBridgeContext *pContext, uint32_t address, uint8_t Data);
int HookSdioBusInit(struct DibBridgeContext *pContext);
int HookSdioBusCmd53(struct DibBridgeContext *pContext, uint8_t*, uint8_t *, unsigned int);
int HookSdioBusCmd52(struct DibBridgeContext *pContext, uint8_t*, unsigned int);

#endif
