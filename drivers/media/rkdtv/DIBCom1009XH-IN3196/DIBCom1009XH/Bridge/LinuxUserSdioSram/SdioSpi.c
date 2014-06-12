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
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridge.h"
#include "SdioSpi.h"

/**** Local Function ***/
static void SdioCmd53(unsigned char *cmd, unsigned char rw, unsigned char fct,
                      unsigned int addr, unsigned short nb, unsigned char block);
static void SdioCmd52(unsigned char *cmd, unsigned char rw, unsigned char fct,
                      unsigned int addr, unsigned char data);
static unsigned char SdioCrc7(unsigned char *buf, unsigned int nb);
#if (USE_SDIOSPI_WORKAROUND == 1)
static int DibSdioSpiWorkaround(struct DibBridgeContext *pContext);
#endif
int DibSdioSpiTest(struct DibBridgeContext *pContext);
extern int SpiWrite(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);
extern int SpiRead(struct DibBridgeContext *pContext,unsigned char* buf, unsigned int size);

#define SPI_LOG 0

static unsigned char CmdBuffer[22];

/*-----------------------------------------------------------
  DibSdioSpiWrite()
-------------------------------------------------------------
  Write a Buffer using Command 53 of SDIO protocol
-------------------------------------------------------------
  - struct DibBridgeContext *:   Brige Context
  - addr                     :   Register Address
  - buf                      :   Buffer to write
  - size                     :   Buffer size
-------------------------------------------------------------*/
int DibSdioSpiWrite(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int size)
{

  DIB_DEBUG(SPI_LOG,(CRB "--SdioSpi Wr: Addr: %08x Size:%d" CRA,addr,size));

  memset(CmdBuffer,0xFF,22);

  /*** Prepare Command 53 ***/
  if (size <= 4)
  {
	unsigned int i;
    /** Fill 6 bytes Cmd53 Command **/
    SdioCmd53(CmdBuffer, SDIO_CMD_WRITE, 1, addr & 0x0000FFFF, size, 0);

	/** Add Data Token **/
    CmdBuffer[9]=0xFE;

    /** Add Data **/
    for (i=0;i<size;i++)
    {
      CmdBuffer[10+i] = buf[i];
    }
 
    /** Write Command and Data **/
    /** Cmd (6) + Answer (2) + Data Token (1) + Data + Data Answer (8) **/
    SpiWrite(pContext,CmdBuffer,18+size);
  }
  else
  {
    /** Fill 6 bytes Cmd53 Command **/
    SdioCmd53(CmdBuffer, SDIO_CMD_WRITE, 1, addr & 0x0000FFFF, size, 0);

    /** Add Data Token **/
    CmdBuffer[9]=0xFE;

    /** Write Command + read answer + write data token ***/
    SpiWrite(pContext,CmdBuffer,10);

    /** Write Data Buffer ***/
    SpiWrite(pContext,buf,size);

    /** read Data answer **/
    memset(CmdBuffer,0xFF,8);
    SpiWrite(pContext,CmdBuffer,8);
  }

  return 0;
}

/*-----------------------------------------------------------
  DibSdioSpiRead()
-------------------------------------------------------------
  Read a Buffer using Command 53 of SDIO protocol
-------------------------------------------------------------
  - struct DibBridgeContext *:   Brige Context
  - addr                     :   Register Address
  - buf                      :   Buffer to read
  - size                     :   Buffer size
-------------------------------------------------------------*/
int DibSdioSpiRead(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int size)
{

  memset(CmdBuffer,0xFF,22);

  /** Fill 6 bytes Cmd53 Command **/
  SdioCmd53(CmdBuffer, SDIO_CMD_READ, 1, addr & 0x0000FFFF, size, 0);

  /** Write Command + read answer + read data token ***/
  SpiWrite(pContext,CmdBuffer,6);

  /*** Read Command 53 answer ***/
  
  if (DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
  {
#if (USE_SDIOSPI_WORKAROUND == 1)
    /** For 29090 and 10098, use workaround **/
    DibSdioSpiWorkaround(pContext);
    SpiRead(pContext,CmdBuffer,3);
#else
    SpiRead(pContext,CmdBuffer,4);
#endif
  }
  else
  {
    SpiRead(pContext,CmdBuffer,4);
  }

  SpiRead(pContext,buf,size);

  DIB_DEBUG(SPI_LOG,(CRB "--SdioSpi Rd: Addr: %08x Size:%d" CRA,addr,size));

  /*** Read CRC and addtionnal Byte to terminate  ***/
  memset(CmdBuffer, 0xFF, 8);
  SpiWrite(pContext,CmdBuffer,8);


  return (0);
}

/*-----------------------------------------------------------
  DibSdioSpiCmd52Write()
-------------------------------------------------------------
  Write a byte using Command 52
-------------------------------------------------------------
  - struct DibBridgeContext *:   Brige Context
  - addr                     :   Register Address
  - Data                     :   Byte to write
-------------------------------------------------------------*/

void  DibSdioSpiCmd52Write(struct DibBridgeContext *pContext,unsigned int Addr, unsigned char fct, unsigned char data)
{
  memset(CmdBuffer,0xFF,22);

  DIB_DEBUG(SPI_LOG,(CRB "--SdioSpi Cmd52: Addr: %08x BM: %d" CRA,Addr,data));
#if 1
  /** Fill 6 bytes Cmd53 Command **/
  SdioCmd52(CmdBuffer, SDIO_CMD_WRITE, fct, 0x10000 | (Addr >> 16), data);

  /** Write Command + read answer + read data token ***/
  SpiWrite(pContext,CmdBuffer,6);

  SpiRead(pContext,CmdBuffer,10);
#else

  SdioCmd53(CmdBuffer, SDIO_CMD_WRITE, 1, 0x10000 | (Addr >> 16), 1, 0);
  CmdBuffer[9]=0xFE;
  CmdBuffer[10] = data;
  SpiWrite(pContext,CmdBuffer,22);
#endif
}

/*-----------------------------------------------------------
  DibSdioSpiCmd52Read()
-------------------------------------------------------------
  Read a byte using Command 52
-------------------------------------------------------------
  - struct DibBridgeContext *:   Brige Context
  - addr                     :   Register Address
  - return byte read
-------------------------------------------------------------*/

unsigned char DibSdioSpiCmd52Read(struct DibBridgeContext *pContext,unsigned int Addr,unsigned char fct)
{
  memset(CmdBuffer,0xFF,22);

  SdioCmd52(CmdBuffer, SDIO_CMD_READ, fct, Addr, 0);

  /** Write Command + read answer + read data token ***/
  SpiWrite(pContext,CmdBuffer,6);

  SpiRead(pContext,CmdBuffer,4);
  return (CmdBuffer[2]);

}

/*-----------------------------------------------------------
  DibSdioSpiCmd53Read()
-------------------------------------------------------------
  Read a byte using Command 53
-------------------------------------------------------------
  - struct DibBridgeContext *:   Brige Context
  - addr                     :   Register Address
  - return byte read
-------------------------------------------------------------*/
unsigned char DibSdioSpiCmd53Read(struct DibBridgeContext *pContext, unsigned int Addr,unsigned char fct)
{
  memset(CmdBuffer,0xFF,22);

  SdioCmd53(CmdBuffer, SDIO_CMD_READ, fct, Addr , 1, 0);
  /** Write Command **/

  SpiWrite(pContext,CmdBuffer,6);

  /** Read Answer (3) + Data Token (1) + Data (1) + Crc (2) **/
  SpiRead(pContext,CmdBuffer,8);

  return (CmdBuffer[4]);

}

/*-----------------------------------------------------------
  DibSdioSpiInit()
-------------------------------------------------------------
  Set the SDIO slave controller in SPI mode
-------------------------------------------------------------
  - struct DibBridgeContext *:   Brige Context
-------------------------------------------------------------*/

int DibSdioSpiInitMode(struct DibBridgeContext *pContext)
{
  unsigned char initbuffer[16] = "";
  unsigned char inbuffer[16] = "";

  DIB_DEBUG(1,(CRB "--SdioSpi Init" CRA));

  /*** Send 16 Clock Pulse to init the SDIO Card ***/
  memset(initbuffer, 0xFF, 16);
  SpiWrite(pContext,initbuffer, 16);

  /*** Send CMD 0 with CS Low to enter SPI Mode ***/
  initbuffer[0] = 0x40;
  initbuffer[1] = 0x00;
  initbuffer[2] = 0x00;
  initbuffer[3] = 0x00;
  initbuffer[4] = 0x00;
  initbuffer[5] = 0x95;

  SpiWrite(pContext,initbuffer, 6);

  /*** Read Answer ***/
  memset(initbuffer, 0xFF, 16);
  SpiWrite(pContext,initbuffer, 16);

  /*** Send Again CMD 0 with CS Low to enter SPI Mode ***/
  /*** and to get the first answer in SPI Mode        ***/
  DIB_DEBUG(SPI_LOG,
            (CRB "Init SPI CMD 1, Resp: %02x %02x %02x" CRA, inbuffer[0],
             inbuffer[1], inbuffer[2]));
  /*** Send CMD 0 with CS Low to enter SPI Mode ***/
  initbuffer[0] = 0x40;
  initbuffer[1] = 0x00;
  initbuffer[2] = 0x00;
  initbuffer[3] = 0x00;
  initbuffer[4] = 0x00;
  initbuffer[5] = 0x95;

  SpiWrite(pContext,initbuffer, 6);

  /*** Read Answer ***/
  memset(initbuffer, 0xFF, 16);
  SpiRead(pContext,inbuffer, 3);

  DIB_DEBUG(SPI_LOG,
	            (CRB "Init SPI CMD 2, Resp: %02x %02x %02x" CRA, inbuffer[0],
	             inbuffer[1], inbuffer[2]));


/*  DibSdioSpiTest(pContext); */

  return (0);
}

/**************************************************************/
/** Workaround DibSpi for 29098 and 10098 1.0 *****************/
/**************************************************************/
/** Write 10 bits instead of 8 bits    ************************/
/**************************************************************/
#if (USE_SDIOSPI_WORKAROUND == 1)
static int DibSdioSpiWorkaround(struct DibBridgeContext *pContext)

{
  /** Send a dummy read command to the Hook to force        ***/
  /** a SPI read (or write) of 10 bits instead of 8        ***/
  unsigned char tbuf[1];
  SpiRead(pContext,tbuf,0x8001);
  return 0;
}
#endif


/*-----------------------------------------------------------
  DibSdioSpiTest()
-------------------------------------------------------------
  Test Basic communication
-------------------------------------------------------------
  - struct DibBridgeContext *:   Brige Context
-------------------------------------------------------------*/

int DibSdioSpiTest(struct DibBridgeContext *pContext)
{
  /*** Test Communication ****/
  unsigned int i;



  if (DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
  {

    DIB_DEBUG(SPI_LOG, (CRB "--- Test Command 52 AHB ---" CRA));
    unsigned char data8;
    unsigned int jdec;
    DibBridgeReadReg8(pContext,0x8000A08C, &data8);
    DIB_DEBUG(SPI_LOG, (CRB "--- JEDEC : %02x ---" CRA, data8));
    jdec = data8 << 24;

    DibBridgeReadReg8(pContext,0x8000A08D, &data8);
    DIB_DEBUG(SPI_LOG, (CRB "--- JEDEC : %02x ---" CRA, data8));
    jdec |= data8 << 16;
    
    DibBridgeReadReg8(pContext,0x8000A08E, &data8);
    DIB_DEBUG(SPI_LOG, (CRB "--- JEDEC : %02x ---" CRA, data8));
    jdec |= data8 << 8;

    DibBridgeReadReg8(pContext,0x8000A08F, &data8);
    DIB_DEBUG(SPI_LOG, (CRB "--- JEDEC : %02x ---" CRA, data8));
    jdec |= data8;

    DIB_DEBUG(SPI_LOG, (CRB "--- JEDEC : %08x ---" CRA, jdec));
  }

  DIB_DEBUG(SPI_LOG, (CRB "--- Test Command 52 CCCR ---" CRA));

  for (i=0;i<16;i++)
  {
    unsigned char data;
    data = DibSdioSpiCmd52Read(pContext,i,0);
    DIB_DEBUG(SPI_LOG, (CRB "--- CCCR (%d)= %02x ---" CRA, i, data ));
  }

  DIB_DEBUG(SPI_LOG, (CRB "--- Test Command 52 ---" CRA));
  for (i=0;i<7;i++)
  {
    unsigned char lsb;
    unsigned char msb;
    unsigned short bs;

    lsb = DibSdioSpiCmd52Read(pContext,0x100*(i+1) + 0x10,0);
    msb = DibSdioSpiCmd52Read(pContext,0x100*(i+1) + 0x11,0);
    bs = ((unsigned short) msb << 8) + lsb;

    DIB_DEBUG(SPI_LOG, (CRB "--- Function %d Block Size: %d bytes ---" CRA, i+1, bs ));
  }

  DIB_DEBUG(SPI_LOG, (CRB "--- Test Command 53 ---" CRA));

  for (i=0;i<7;i++)
  {
    unsigned char lsb;
    unsigned char msb;
    unsigned short bs;

    lsb = DibSdioSpiCmd53Read(pContext,0x100*(i+1) + 0x10,0);
    msb = DibSdioSpiCmd53Read(pContext,0x100*(i+1) + 0x11,0);
    bs = ((unsigned short) msb << 8) + lsb;

    DIB_DEBUG(SPI_LOG, (CRB "--- Function %d Block Size: %d bytes ---" CRA, i+1, bs ));
  }

  return (0);
}


#define BIT(a,b) (((a)>>(b))&1)

static unsigned char SdioCrc7(unsigned char *buf, unsigned int nb)
{
  unsigned char crc = 0;
  unsigned int i;
  unsigned int j;

  for(j = 0; j < nb; j++) {
    for(i = 0; i <= 7; i++) {
      unsigned int crc_in, crc_in2;

      crc_in = BIT(buf[j], (7 - i)) ^ BIT(crc, 0);
      crc_in2 = crc_in ^ BIT(crc, 4);
      crc = (crc_in << 6) + (BIT(crc, 6) << 5) + (BIT(crc, 5) << 4)
        + (crc_in2 << 3) + (BIT(crc, 3) << 2) + (BIT(crc, 2) << 1) + BIT(crc,
                                                                         1);
  }} return (BIT(crc, 0) << 7) + (BIT(crc, 1) << 6) + (BIT(crc, 2) << 5) +
    (BIT(crc, 3) << 4)
    + (BIT(crc, 4) << 3) + (BIT(crc, 5) << 2) + (BIT(crc, 6) << 1) + 0x1;
}


/*-----------------------------------------------------------
  SdioCmd53
-------------------------------------------------------------*/
static void SdioCmd53(unsigned char *cmd, unsigned char rw, unsigned char fct,
                unsigned int addr, unsigned short nb, unsigned char block)
{
  cmd[4] = nb & 0xff;
  cmd[3] = ((addr << 1) & 0xFE) | ((nb >> 8) & 0x01);
  cmd[2] = (addr >> 7) & 0xFF;
  cmd[1] = (rw << 7) + (fct << 4) +(block <<3)	 + ((addr >> 15) & 0x3);
  cmd[0] = 0x75;
  cmd[5] = SdioCrc7(cmd, 5);
  return;
}

static void SdioCmd52(unsigned char *cmd, unsigned char rw, unsigned char fct,
                unsigned int addr, unsigned char data)
{
  cmd[4] = data;
  cmd[3] = ((addr << 1) & 0xFE);
  cmd[2] = (addr >> 7) & 0xFF;
  cmd[1] = (rw << 7) + (fct << 4) + ((addr >> 15) & 0x3);
  cmd[0] = 0x74;
  cmd[5] = SdioCrc7(cmd, 5);
  return;
}

