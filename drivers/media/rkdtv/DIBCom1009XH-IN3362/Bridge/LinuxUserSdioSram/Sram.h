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
* @file "Sram.h"
* @brief Target Specific.
*
***************************************************************************************************/


/*** Base Address of Memory Bank used for Dibcom Chip ****/
/*** Depends on customer design                       ****/

#define DIB_BASE    	 0x60000000 

#define DIB16_ADDR_MSB   (DIB_BASE + 0x3) 
#define DIB16_ADDR_LSB   (DIB_BASE + 0x2) 
#define DIB16_DATA_MSB   (DIB_BASE + 0x1) 
#define DIB16_DATA_LSB   (DIB_BASE + 0x0) 

#define DIB32_ADDR_MSB   (DIB_BASE + 0x7)
#define DIB32_ADDR_mSB   (DIB_BASE + 0x6)
#define DIB32_ADDR_lSB   (DIB_BASE + 0x5)
#define DIB32_ADDR_LSB   (DIB_BASE + 0x4)
#define DIB32_DATA_MSB   (DIB_BASE + 0x3)
#define DIB32_DATA_mSB   (DIB_BASE + 0x2)
#define DIB32_DATA_lSB   (DIB_BASE + 0x1)
#define DIB32_DATA_LSB   (DIB_BASE + 0x0)



#define DIB_ACCESS(a) (*(volatile uint8_t*)a)
#define SramWrite(p,a,b) DIB_ACCESS(a) = (b)
#define SramRead(p,a) DIB_ACCESS(a)


void SramInit(struct DibBridgeContext *pContext);
