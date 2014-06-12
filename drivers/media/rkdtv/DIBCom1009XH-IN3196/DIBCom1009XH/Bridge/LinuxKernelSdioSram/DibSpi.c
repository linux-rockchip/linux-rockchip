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

#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
#include "Hook.h"
#endif

/** Local functions ***/
static int DibSpiInitMode(struct DibBridgeContext *pContext);

#if (USE_DIBSPI_WORKAROUND == 1)
static int DibSpiWorkaround(struct DibBridgeContext *pContext);
#endif

int SpiInit(struct DibBridgeContext *pContext);
int SpiWrite(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);
int SpiRead(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);

#if (USE_SDIOSPI_MODE == 1)
#include "SdioSpi.h"
#endif


/****************************************************
 DibSpiInit
****************************************************/
int DibSpiInit(struct DibBridgeContext *pContext)
{
	int rc;
	rc = SpiInit(pContext);
	if (rc == 0)
	{
	  if (DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_16)
	  {
#if (USE_SDIOSPI_MODE == 1)
        DibSdioSpiInitMode(pContext);
#else
    	DibSpiInitMode(pContext);
#endif
	  }
	  else
	  {

#if (USE_SDIOSPI_MODE == 1)
        DibSdioSpiInitMode(pContext);
#else
    	DibSpiInitMode(pContext);
#endif
	  }
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
#if (USE_SDIOSPI_MODE == 1)
      DibSdioSpiWrite(pContext,addr, buf, size);
#else
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

	  /* dummy write added to force interrupt edge */
#endif /* USE_SDIOSPI_MODE */
	}
    else
    {
#if (USE_SDIOSPI_MODE == 1)
DibSdioSpiWrite(pContext,addr, buf, size);
#else
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

#endif /* USE_SDIOSPI_MODE */
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
#if (USE_SDIOSPI_MODE == 1)
       DibSdioSpiRead(pContext,addr, buf, size);
#else
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

#endif /* USE_SDIOSPI_MODE */
   }
   else
   {
#if (USE_SDIOSPI_MODE == 1)
       DibSdioSpiRead(pContext,addr, buf, size);
#else
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

#endif /* USE_SDIOSPI_MODE */

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
#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
  unsigned char tbuf[1];
  SpiRead(pContext,tbuf,0x4001);
#elif (DIBCOM_TEST_MODE == TEST_MODE_SPP)
  SppSpiWorkaround();
#endif
  return 0;
}
#endif
/**************************************************************/
/****** LOW LEVEL SPI BUS ACCESS ******************************/
/**************************************************************/
int SpiInit(struct DibBridgeContext *pContext)
{
#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
  HookInit(pContext,DIBBRIDGE_MODE_SDIO,pContext->BoardHdl);
#endif
  return DIBSTATUS_SUCCESS;
}

void  PrintB(char* str,unsigned char *buf,unsigned int size)
{
  return;
  int i;
  printk("%s: ",str);
  for (i=0;i<size;i++)
    printk(" %02x",buf[i]);
  printk("\n");

}

int SpiRead(struct DibBridgeContext *pContext, unsigned char* buf, unsigned int size)
{
#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
  unsigned int msb= size & 0x0000C000;
  unsigned int lsize=size & 0x3FFF;
  while (lsize > 64)
  {
    HookSpiBusRead(pContext,buf,64 | msb);
    lsize -= 64;
    buf += 64;
  }
  if (lsize > 0)
  {
    HookSpiBusRead(pContext,buf,lsize | msb);
  }
  PrintB("RD",buf,size&0x3FFF);
#endif
  return DIBSTATUS_SUCCESS;
}

int SpiWrite(struct DibBridgeContext *pContext, unsigned char* buf, unsigned int size)
{
#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
  unsigned int lsize=size;

  PrintB("WR",buf,size);
  while (lsize > 64)
  {
    HookSpiBusWrite(pContext,buf,64);
    lsize -= 64;
    buf += 64;
  }
  if (lsize > 0)
  {
    HookSpiBusWrite(pContext,buf,lsize);
  }
#endif
  return DIBSTATUS_SUCCESS;
}

#endif

