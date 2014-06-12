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
* @file "I2CSkeleton.h"
* @brief Target Specific.
*
***************************************************************************************************/
#include "SppI2C.h"

#define I2C_MAX_RDWR_SIZE 64
int  I2CInit(void);
int  I2CWrite(uint32_t dev_addr, uint8_t *txdata, uint32_t txlen, uint32_t byteMode);
int  I2CRead(uint32_t dev_addr, uint8_t *txdata, uint32_t txlen, uint8_t *rxdata, uint32_t rxlen, uint32_t byteMode );
