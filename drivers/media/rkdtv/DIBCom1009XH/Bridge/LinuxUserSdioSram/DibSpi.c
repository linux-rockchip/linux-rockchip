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

#if (DIBINTERF_PHY == DIBINTERF_PHY_SPI)
#include "DibSpi.h"

/** Local functions ***/
static int DibSpiInitMode(struct DibBridgeContext *pContext);

#if (USE_DIBSPI_WORKAROUND == 1)
static int DibSpiWorkaround(struct DibBridgeContext *pContext);
#endif

int SpiInit(struct DibBridgeContext *pContext);
int SpiWrite(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);
int SpiRead(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);


/****************************************************
 DibSpiInit
****************************************************/
int DibSpiInit(struct DibBridgeContext *pContext)
{
	int rc;
	rc = SpiInit(pContext);
	if (rc == 0)
	{
    	DibSpiInitMode(pContext);
	}
	return rc;
}

/****************************************************
 DibSpiDeInit
****************************************************/

int DibSpiDeInit(struct DibBridgeContext *pContext)
{
  return 0;
}

/****************************************************
 DibSpiSetClock
****************************************************/

int DibSpiSetClock(struct DibBridgeContext *pContext,unsigned int clock)
{

	return 0;
}

/****************************************************
 DibSpiWrite
****************************************************/
int DibSpiWrite(struct DibBridgeContext *pContext, unsigned int addr,unsigned char* buf, unsigned int size)
{
  unsigned char command[6];
  unsigned int commandsize;

    if (DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_16)
    {
        /****** Build Command ******
        - b31: 0     => Start Bit
        - b30: 0     => Write
        - b29:       => Autoincrement
        - b28:       => Byte Mode
        - b27-b16:   => Address
        - b15-b0:    => Transfer Size
        ***************************/
	  command[0] = (addr & 0x0f00) >> 8;
	  if (addr & 0x1000)
		command[0] |= 0x10 ;
	  if (addr & 0x2000)
		command[0] |= 0x20 ;

	  command[1] = addr & 0x00FF ;
	  commandsize = size - 1;
	  command[2] = (commandsize & 0xFF00) >> 8;
	  command[3] = (commandsize & 0x00FF);

	  SpiWrite(pContext,command,4);

	  SpiWrite(pContext,buf,size);

	}
    else
    {
	/****** Build Command ******
        - b31: 0     => Start Bit
        - b30: 0     => Write
        - b29-28:    => Transfer Size
        - b27:       => Auto Increment
        - b26-b0:    => Formatted Address
        - b15-b0:    => Transfer Size
        ***************************/

	  command[0] = (addr & 0x3F000000) >> 24;
	  command[1] = (addr & 0x00FF0000) >> 16;
	  command[2] = (addr & 0x0000FF00) >> 8;
	  command[3] = (addr & 0x000000FF) >> 0;
	  command[0] |= 0x40;

	  commandsize = size;
	  command[4] = (commandsize & 0xFF00) >> 8;
	  command[5] = (commandsize & 0x00FF);

          /** Write Command **/
	  SpiWrite(pContext,command,6);

	  SpiWrite(pContext,buf,size);

#if (USE_DIBSPI_WORKAROUND == 1)
          DibSpiWorkaround(pContext);
#endif
	}
	return 0;
}

/****************************************************
 DibSpiRead
****************************************************/
int DibSpiRead(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int size)
{

   unsigned char command[6];
   unsigned int commandsize;

   if (DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_16)
   {
     /****** Build Command ******
     - b31: 0     => Start Bit
     - b30: 1     => Read
     - b29:       => Autoincrement
     - b28:       => Byte Mode
     - b27-b16:   => Address
     - b15-b0:    => Transfer Size
     ***************************/

	 command[0] = (addr & 0x0f00) >> 8;
	 command[0] |= 0x40;

 	 if (addr & 0x1000)
		command[0] |= 0x10 ;
	 if (addr & 0x2000)
		command[0] |= 0x20 ;

	 command[1] = addr & 0x00FF ;
	 commandsize = size - 1;
	 command[2] = (commandsize & 0xFF00) >> 8;
	 command[3] = (commandsize & 0x00FF);

	 SpiWrite(pContext,command,4);

	 SpiRead(pContext,buf,size);

   }
   else
   {
      /****** Build Command ******
      - b31: 0     => Start Bit
      - b30: 1     => Read
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
#if (USE_DIBSPI_WORKAROUND == 1)
         DibSpiWorkaround(pContext);
#endif

   }

   return 0;
}

/****************************************************
 DibSpiInitMode
****************************************************/
static int DibSpiInitMode(struct DibBridgeContext *pContext)
{
  unsigned char initbuffer[16] = "";

  /*** Send 16 Clock Pulse to init the SDIO Card ***/
  memset(initbuffer, 0xFF, 16);
  SpiWrite(pContext,initbuffer, 16);

  /*** Send Special CMD 0 with CS Low to enter DIBSPI Mode ***/
  initbuffer[0] = 0x40;
  initbuffer[1] = 0x00;
  initbuffer[2] = 0x00;
  initbuffer[3] = 0x00;
  initbuffer[4] = 0x01;
  initbuffer[5] = 0x87;

  SpiWrite(pContext,initbuffer, 6);

  /*** Read Answer ***/
  memset(initbuffer, 0xFF, 16);
  SpiWrite(pContext,initbuffer, 16);
  return 0;
}

/**************************************************************/
/** Workaround DibSpi for 29098 and 10098 1.0 *****************/
/**************************************************************/
/** Write 1 Byte with Chip Select High ************************/
/**************************************************************/
#if (USE_DIBSPI_WORKAROUND == 1)
static int DibSpiWorkaround(struct DibBridgeContext *pContext)
{
  /** Send a dummy read command to the Hook to force        ***/
  /** a SPI read (or write) of 1 byte with Chip Select High ***/
  return 0;
}
#endif
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

