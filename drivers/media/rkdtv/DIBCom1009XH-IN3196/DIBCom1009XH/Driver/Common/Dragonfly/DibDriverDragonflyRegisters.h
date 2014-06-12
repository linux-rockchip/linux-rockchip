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
* @file "DibDriverDragonflyRegisters.h"
* @brief Dragonfly Register mapping.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DRAGONFLY_REGISTERS_H
#define DIB_DRIVER_DRAGONFLY_REGISTERS_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/**************************************************************
 *
 *         Useful macro to read or write bit fields
 *
 *************************************************************/

#define FIELD(Size, Off)            ((Size) << 8 | (Off)) 
#define SHIFT(Field)                (Field & 0xFF) 
#define SIZE(Field)                 (Field >> 8) 
#define MASK(Field)                 ((0xFFFFFFFF >> (32-SIZE(Field))) << SHIFT(Field)) 
#define FIELD_ALL                   FIELD(32, 0) 
#define GFLD(RVal, Field)           ((RVal & MASK(Field)) >> SHIFT(Field))
#define SFLD(RVal, Data, Field)     ((RVal & ~MASK(Field)) | (Data << SHIFT(Field)));


/**************************************************************
 *
 *                  Host interface
 *
 *************************************************************/

#define REG_HOSTIF_OFFSET            0x8000B000
#define REG_HIF_INT_STAT            (REG_HOSTIF_OFFSET + 0x00)
#define REG_HIF_INT_SET             (REG_HOSTIF_OFFSET + 0x04)
#define REG_HIF_INT_CLR             (REG_HOSTIF_OFFSET + 0x08)
#define REG_HIF_INT_EN              (REG_HOSTIF_OFFSET + 0x0C)
#define REG_HIF_INT_MIRROR          (REG_HOSTIF_OFFSET + 0x10)

#define FLD_HIF_IRQ(Irq)            FIELD( 1, Irq)    /* [15: 0]  Bit wise interrupt flags   */
#define FLD_HIF_MBX_NEW             FIELD( 1, 0)      /* [    0]  New Message in Msg Box     */
#define FLD_HIF_MBX_OVF             FIELD( 1, 1)      /* [    1]  Message Box Overflow       */
#define FLD_HIF_IRQ_ALL             FIELD(16, 0)

#define REG_HIF_HOST_IRQ_MODE       (REG_HOSTIF_OFFSET + 0x14)
#define FLD_HIF_IRQ_OD              FIELD( 1, 0)      /* [    0]  host irq driven by...             */
#define FLD_HIF_IRQ_LEVEL           FIELD( 1, 1)      /* [    1]  1 if active high, 0 if active low */

#define REG_HIF_HOST_IRQ_EN         (REG_HOSTIF_OFFSET + 0x18)
#define REG_HIF_HOST_ADDR           (REG_HOSTIF_OFFSET + 0x1C)

#define REG_HIF_SDIO_IRQ_EN         (REG_HOSTIF_OFFSET + 0x24)
#define FLD_HIF_SDIO_INV               FIELD( 1, 0) /* [    0]  Polarity */
#define FLD_HIF_SDIO_EN                FIELD( 1, 1) /* [    1]  Enable SDIO irq engine */
#define FLD_HIF_SDIO_SWAP              FIELD( 1, 3) /* [    3]  Data swapping */


#endif
