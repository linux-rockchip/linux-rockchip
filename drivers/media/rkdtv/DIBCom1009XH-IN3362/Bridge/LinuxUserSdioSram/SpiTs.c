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
#include "DibBridgeCommon.h"
#include "DibBridge.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"

#if (DIBINTERF_PHY == DIBINTERF_PHY_SPITS)
#include "SpiTs.h"

/** Local functions ***/
static int SpiTsInitMode(struct DibBridgeContext *pContext);

int SpiInit(struct DibBridgeContext *pContext);
int SpiWrite(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);
int SpiRead(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);


/****************************************************
 SpiTsInit
****************************************************/
int SpiTsInit(struct DibBridgeContext *pContext)
{
	int rc;
	rc = SpiInit(pContext);
	if (rc == 0)
	{
    	  SpiTsInitMode(pContext);
	}
	return rc;
}

/****************************************************
 SpiTsDeInit
****************************************************/

int SpiTsDeInit(struct DibBridgeContext *pContext)
{
  return 0;
}

/****************************************************
 SpiTsSetClock
****************************************************/

int SpiTsSetClock(struct DibBridgeContext *pContext,unsigned int clock)
{
   return 0;
}

/****************************************************
 SpiTsWrite
****************************************************/
int SpiTsWrite(struct DibBridgeContext *pContext, unsigned int addr,unsigned char* buf, unsigned int size)
{
  unsigned char command[6];
  unsigned int commandsize;

  /****** Build Command ******
  - b31: 0     => Start Bit
  - b30: 1     => Write
  - b29-28:    => Transfer Size
  - b27:       => Auto Increment
  - b26-b0:    => Formatted Address
  - b15-b0:    => Transfer Size
  ***************************/
  command[0] = (addr & 0x3F000000) >> 24;
  command[1] = (addr & 0x00FF0000) >> 16;
  command[2] = (addr & 0x0000FF00) >> 8;
  command[3] = (addr & 0x000000FF) >> 0;
  command[0] |= 0x40; /* Write */

  commandsize = size;
  command[4] = (commandsize & 0xFF00) >> 8;
  command[5] = (commandsize & 0x00FF);

  /** Write Command **/
  SpiWrite(pContext,command,6);

  SpiWrite(pContext,buf,size);

  return 0;
}

/****************************************************
 SpiTsRead
****************************************************/
int SpiTsRead(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int size)
{

   unsigned char command[6];
   unsigned int commandsize;

   /****** Build Command ******
   - b31: 0     => Start Bit
   - b30: 0     => Read
   - b29-28:    => Transfer Size
   - b27:       => Auto Increment
   - b26-b0:    => Formatted Address
   - b15-b0:    => Transfer Size
   ***************************/

   command[0] = (addr & 0x3F000000) >> 24;
   command[1] = (addr & 0x00FF0000) >> 16;
   command[2] = (addr & 0x0000FF00) >> 8;
   command[3] = (addr & 0x000000FF) >> 0;

   commandsize = size;
   command[4] = (commandsize & 0xFF00) >> 8;
   command[5] = (commandsize & 0x00FF);

  /** Write Command **/
  SpiWrite(pContext,command,6);

  SpiRead(pContext,buf,size);

  return 0;
}

/****************************************************
 SpiTsInitMode
****************************************************/
static int SpiTsInitMode(struct DibBridgeContext *pContext)
{
  unsigned char initbuffer[20]; 

#if (DIBCTRL_ADDR == DIBCTRL_DEFAULT_ADDR)
    initbuffer[0]=0x10;
    initbuffer[1]=0x20;
#else
    initbuffer[0]=0x0F;
    initbuffer[1]=0xE0;
#endif

  initbuffer[2]=0x00;
  initbuffer[3]=0x2C;

  initbuffer[4]=0x16;
  initbuffer[5]=0x00;
  initbuffer[6]=0x00;
  initbuffer[7]=0x00;

  initbuffer[8]=0x00;
  initbuffer[9]=0x3F;
  initbuffer[10]=0x00;
  initbuffer[11]=0x00;

  initbuffer[12]=0x00;
  initbuffer[13]=0x00;
  initbuffer[14]=0x00;
  initbuffer[15]=0x03;

  initbuffer[16]=0xFF;
  initbuffer[17]=0xFF;
  initbuffer[18]=0xFF;
  initbuffer[19]=0xFF;

  SpiWrite(pContext,initbuffer, 20);
  return 0;
}

/**************************************************************/
/****** LOW LEVEL SPI BUS ACCESS ******************************/
/**************************************************************/
int SpiInit(struct DibBridgeContext *pContext)
{
  return DIBSTATUS_SUCCESS;
}

int SpiRead(struct DibBridgeContext *pContext, unsigned char* buf, unsigned int size)
{
  return DIBSTATUS_SUCCESS;
}

int SpiWrite(struct DibBridgeContext *pContext, unsigned char* buf, unsigned int size)
{
  return DIBSTATUS_SUCCESS;
}

#endif

