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
* @file "DibBridgeDragonfly.h"
* @brief Dragonfly sprecific bridge functionality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_DRAGONFLY_H
#define DIB_BRIDGE_DRAGONFLY_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridge.h"

/* host interface irq clear mode */
#define DONT_CLEAR                  0
#define CLEAR_BY_MESSAGE            1
#define CLEAR_BY_REGISTER           2

/* Specific Build Config */
/* Turn on fast 32 bit dma transfers for dragonfly chip */
#define HIGH_SPEED_DMA              1

#define CLEAR_HOST_IRQ_MODE         CLEAR_BY_MESSAGE

extern void DibBridgeDragonflyRegisterIf(struct DibBridgeContext *pContext, uint32_t * Config);
extern void DibBridgeVoyager1ConfigureSdioEndianness(struct DibBridgeContext *pContext);
extern void DibBridgeNautilus1ConfigureSdioEndianness(struct DibBridgeContext *pContext);

/* 16 bit Access to even apb address which are not demod addresses does'nt work */
#define DIB29000_APB_EVEN_ADDR(Address, Mode) (  IS_16BIT_ADDRESS(Mode)          \
                                             &&  IS_EVEN_ADDRESS(Address)        \
                                             &&  IS_APB_ADDRESS(Address)         \
                                             && !IS_DEMOD_ADDRESS(Address) )

#define IS_DEMOD_ADDRESS(address) ((address) >= 0x8000e000 && (address) <= 0x8000fc08)
#define IS_APB_ADDRESS(address) (((address) >= 0x80000000) && ((address) <= 0x8FFFFFFF))
#define IS_EVEN_ADDRESS(address) (!((address) & 0x2))
#define IS_16BIT_ADDRESS(Mode) ((Mode) & DIBBRIDGE_BIT_MODE_16)

/* SRAM address format */
#define DF_ADDR_TO_SRAM(Addr, Mode, incr, Prefetch)       ( ((Addr     <<  8) & 0xf0000000) | \
                                                            ((incr     << 27) & 0x08000000) | \
                                                            ((Mode     << 25) & 0x06000000) | \
                                                            ((Prefetch << 24) & 0x01000000) | \
                                                            ((Addr     >> 8)  & 0x00f00000) | \
                                                            ( Addr            & 0x000fffff)   )

#define DF_SRAM_TO_ADDR(Addr)                             ( (((Addr   ) & 0xf0000000 ) >> 8) | \
                                                            (((Addr   ) & 0x00f00000 ) << 8) | \
                                                            (((Addr   ) & 0x000fffff )   ) )

/* SDIO address format */
#define DF_ADDR_TO_SDIO(Addr, incr)             ( ((incr             & 0x1  )  << 31) | \
                                                  (((Addr     >>12)  & 0x7FFF) << 16) | \
                                                  (((Addr      >>28) &  0x0F)  << 12) | \
                                                  (((Addr          ) & 0xFFF)      ) )

#define DF_SDIO_TO_ADDR(Addr)                    ((((Addr          ) & 0x0000F000) << 16) | \
                                                  (((Addr          ) & 0x7FFF0000) >> 4)  | \
                                                  (((Addr          ) & 0x00000FFF)      ) )
/* SPI Format */
#define DF_ADDR_TO_SPI(Addr, Mode, incr)          ( ((Mode    & 0x3       ) <<  28) | \
                                                    ((incr   & 0x1       ) <<  27) | \
                                                    ((Addr   & 0xF0000000) >>  5)  | \
                                                    ((Addr   & 0x007FFFFF)      ) ) 

#define DF_SPI_TO_ADDR(Addr)                      (((Addr            & 0x07800000) << 5) | \
                                                   ((Addr            & 0x007FFFFF)    )  ) 

#endif
