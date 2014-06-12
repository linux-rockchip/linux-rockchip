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
* @file "SppI2C.c"
* @brief Target Specific.
*
***************************************************************************************************/
/* The following convention is used to named the printer port
The "port" is a number corresponding to a physical I/O address 
This number is between 0 and 8 :
0 --> 0x3bc
1 --> 0x378 (This is generally LPT1)
2 --> 0x278 (This is often LPT2)
3 --> 0x368
4 --> 0x268
5 --> 0x358
6 --> 0x258
7 --> 0x348
8 --> 0x248
Check the hardware ressources used by LPT port in the System
Control Panel of Windows 95 */
/* the "speed" is an integer which give the period of I2C SCL */
/* these number must be given to the init function */

#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTargetTypes.h"
#include "SppI2C.h"

#if (DIBCOM_TEST_MODE == TEST_MODE_SPP)

/* SDAOUT = AUTO_FD_XT (pin 14 of sub D 25) hardware inverted on cable */
/* bit 1 of ctrl port (base+2) */
#define SDAOUT      0x02

/* SCLOUT = SLCT_IN (pin 17 of sub D 25) hardware inverted on cable */
/* bit 3 of ctrl port (base+2) */
#define SCLOUT      0x08

/* SDAIN = BUSY (pin 11 of sub D 25) hardware inverted on cable */
/* bit  of status port (base+1) */
#define SDAIN       0x80


/* software invertion of bus (use of hardware inverter or Buffer) */
#if (DIBINTERF_PHY == DIBINTERF_PHY_I2C)
#define IICINV         0
#else
#define IICINV         0
#endif

/***************************************************************/
/* SPI bus                                                     */
/***************************************************************/
/* CS = Strobe (Pin 1 of sub D 25) hardware inverted on cable */
/* bit 0 of ctrl port (base+2) */
#define SPP_CS      0x01

/* DATA_OUT = AUTO_FD_XT (pin 14 of sub D 25) hardware inverted on cable */
/* bit 1 of ctrl port (base+2) */
#define SPP_DATAOUT     0x02

/* SPP_CLK = SLCT_IN (pin 17 of sub D 25) hardware inverted on cable */
/* bit 3 of ctrl port (base+2) */
#define SPP_CLK       0x08

/* DATA_IN = BUSY (pin 11 of sub D 25) hardware inverted on cable */
/* bit 7 of status port (base+1) */
#define SPP_DATAIN      0x80


/* this is a internal private variable */
static struct i2clpt i2c;

int32_t ExistParallelPort(int32_t * portaddr)
{

   int32_t baseportaddr[9] = { 0x3bc, 0x378, 0x278, 0x368, 0x268, 0x358, 0x258, 0x348, 0x248 };
   int32_t iport = 0;
   int32_t found = 0;

   *portaddr = -1;

   /* New linux */

   while((found==0) && (iport<9))
   {
      uint16_t Addr = baseportaddr[iport];

      if(ioperm(Addr,3,1))
      {
         printf("Problem, you must be root !");
         return SPP_SUPERUSER_RIGHTS_ERROR;
      }

      outb(0x80,Addr);
      if(inb(Addr)==0x80)
      {
         outb(0x00,Addr);
         if(inb(Addr)==0x00)
         {
            found = 1;
            *portaddr = Addr;
         }
      }
      iport++;
   }

   return found;
}


int32_t ConnectParallelPort(int32_t portaddr , int32_t port_type)
{
   int32_t version = 0;

   if(portaddr == 0x378)
      return (SppInit(1,10,&version));
   else if(portaddr == 0x278 )
      return (SppInit(2,80,&version));
   else 
      return(SPP_INVALID_PORT_TYPE);
}


int32_t ExistPort(int32_t port)
{
   int32_t baseportaddr[9] = { 0x3bc, 0x378, 0x278, 0x368, 0x268, 0x358, 0x258, 0x348, 0x248 };
   uint16_t Addr = baseportaddr[port];

   /* New linux */
   if(ioperm(Addr,3,1))
   {
      printf("Problem, you must be SU !");
      return SPP_SUPERUSER_RIGHTS_ERROR;
   }

   outb(0x80,Addr);
   if(inb(Addr)!=0x80)
      return 0;

   outb(0x00,Addr);
   if(inb(Addr)!=0x00)
      return 0;

   return 1;
}

int32_t SppInit4Spi(int32_t port, int32_t timestep, int32_t *version)
{
   /* base addresses of printer ports on PCs */
   int32_t baseportaddr[9] = { 0x3bc, 0x378, 0x278, 0x368, 0x268, 0x358, 0x258, 0x348, 0x248 };

   if(port <= 8 && port >= 0)
      i2c.portaddr = baseportaddr[port];
   else
      i2c.portaddr = baseportaddr[0];

   i2c.mindelay = timestep;
   i2c.enabled  = 1;
   i2c.Data     = inb(i2c.portaddr);
   i2c.status   = inb((uint16_t)(i2c.portaddr+1));
   i2c.status  &= 0xff;
   i2c.ctrl     = inb((uint16_t)(i2c.portaddr+2));
   i2c.ctrl    &= 0xff;

   /* Set CS & Clock &dataout High state */
   sppspics1();
   sppspiclk1();
   sppspidataout1();

   return 0;
}


/* init */
int32_t SppInit(int32_t port, int32_t timestep, int32_t *version)
{
   /* base addresses of printer ports on PCs */
   int32_t baseportaddr[9] = { 0x3bc, 0x378, 0x278, 0x368, 0x268, 0x358, 0x258, 0x348, 0x248 };
   int32_t Err=0;

   if(port <= 8 && port >= 0)
      i2c.portaddr = baseportaddr[port];
   else
      i2c.portaddr = baseportaddr[0];

   i2c.mindelay = timestep;
   i2c.enabled  = 1;
   i2c.Data     = inb(i2c.portaddr);
   i2c.status   = inb((uint16_t)(i2c.portaddr+1));
   i2c.status  &= 0xff;
   i2c.ctrl     = inb((uint16_t)(i2c.portaddr+2));
   i2c.ctrl    &= 0xff;

   /* set SCl high */
   sclout1();

   /* set SDA low */
   sdaout0();

   /* check if SDA is 0 */
   if(sdain() != 0)
      Err = SDA_LOW_TEST_ERROR;

   /* set SDA high */
   sdaout1();

   /* check if SDA is 1 */
   if(sdain()==0)
      Err = SDA_HIGH_TEST_ERROR;

   /* set SDA low */
   sdaout0();

   /* check if SDA is 0 */
   if(sdain()!=0)   
      Err = SDA_LOW_TEST_ERROR;      

   /* set SDA high */
   sdaout1();

   /* check again if SDA is 1 */
   if(sdain() == 0)
      Err = SDA_HIGH_TEST_ERROR;   

   if(Err)
   {
      return Err;
   }

   /* we should check for the device address now */   
   /*     if(baseadr)
           {
              int32_t i2c_rx_buf[2];
              int32_t   i;
              i2c_tx_buf[0] = 0xE;   
              for(i=0 ; i<4 ; i++)
              {   
                 if(i2c.enabled)
                    Err = SppI2CWriteRead(baseadr+i, i2c_tx_buf, 1, i2c_rx_buf, 2);
                 if(Err==0 && i2c_rx_buf[0]==0x65 && i2c_rx_buf[1]==0x10 )
                    return INFO_I2C_ADDR00-i;
              }
           }
   */

   stop();
   return 0;
}


/* program Value on Data pins of printer port */
int32_t DataBus(int32_t datavalue)
{
   if(i2c.enabled)
      outb(datavalue,i2c.portaddr);

   i2c.Data = datavalue;

   return 0;
}

/* transmit txcnt bytes to device */
int32_t SppI2CWrite(int32_t Addr, uint8_t *i2c_tx_buf, int32_t txcnt)
{
   int32_t i;
   int32_t Err;

   if(!i2c_tx_buf && txcnt)         /* check tx Buffer */
      return I2C_TX_BUFFER_ERROR;

   start();                     /* start */

   Err = sendbyte(Addr & 0xfe);      /* device address (write Mode) */
   if(Err==I2C_NACK_ERROR)
   {
      stop();
      return I2C_DEV_ADR_NACK_ERROR;
   }

   for(i=0 ; i<txcnt ; i++)
   {
      Err = sendbyte(i2c_tx_buf[i]);   /* write txcnt Data bytes */      

      if(Err == I2C_NACK_ERROR)
      {
         stop();
         return I2C_WR_NACK_ERROR;
      }
   }

   stop();
   return 0;
}

int32_t SppSpiWrite(uint8_t *txbuf, int32_t txcnt)
{
    int32_t i;

    /* Set CS low */
    sppspics0();

    /* send bytes */
    for(i=0;i<txcnt;i++)
        sendSPIbyte(txbuf[i]);

    /* set Cs high */
    sppspics1();

    return 0;
}

int32_t SppSpiWorkaround()
{
    /* Set CS high */
    sppspics1();

    /* Send a dummy read */
    sendSPIbyte(0);
    return 0;
}

int32_t SppSpiRead(uint8_t *rxbuf, int32_t rxcnt)
{
    int32_t i;

    /* Set CS low */
    sppspics0();

    /* receive bytes */
    for(i=0;i<rxcnt;i++)
        rcvdSPIbyte(&rxbuf[i]);

    /* set Cs high */
    sppspics1();

    return 0;

}

/* transmit txcnt bytes to device then read rxcnt bytes */
int32_t SppI2CWriteRead(int32_t Addr,  uint8_t  *i2c_tx_buf, int32_t txcnt, uint8_t *i2c_rx_buf, int32_t rxcnt)
{
   int32_t i;
   int32_t Err;
   int32_t ack;

   if(!i2c_tx_buf && txcnt)         /* check tx Buffer */
      return I2C_TX_BUFFER_ERROR;

   if(!i2c_rx_buf && rxcnt)         /* check rx Buffer */
      return I2C_RX_BUFFER_ERROR;

   for(i=0 ; i<rxcnt ; i++)         /* initialize receive Buffer */
      i2c_rx_buf[i] = 0xfe;

   if(txcnt >0) /* ************ START MODIF for simple read access */
   {
      start();                     /* start */
      Err = sendbyte(Addr & 0xfe);      /* device address (write Mode) */
      
      if(Err == I2C_NACK_ERROR)
      {
         stop();
         return I2C_DEV_ADR_NACK_ERROR;
      }

      for(i=0 ; i<txcnt ; i++)
      {
         Err = sendbyte(i2c_tx_buf[i]);   /* write Data bytes */
         
         if(Err == I2C_NACK_ERROR)
         {
            stop();
            return I2C_WR_NACK_ERROR;
         }
      }
   } /* ************ ENF MODIF */

   start();                  /* repeated start condition */

   Err = sendbyte(Addr | 0x01);   /* device address (read Mode) */
   if(Err==I2C_NACK_ERROR)
   {
      stop();
      return I2C_DEV_ADR_NACK_ERROR;
   }

   for(i=0 ; i<rxcnt ; i++)
   {
      ack = ((i+1)!=rxcnt);           /* no ack for the last byte */
      rcvdbyte(&i2c_rx_buf[i],ack);   /* read 1 Data byte  */      
   }

   stop();                        /* stop */
   return 0;
}

/***********************************/
/*  elementary functions (private) */
/***********************************/
int32_t sppspics0()  
{
#if (IICINV == 1)
   i2c.ctrl |= SPP_CS;
#else
   i2c.ctrl &= ~SPP_CS;
#endif   

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sppspics1()  
{
#if (IICINV == 0)
   i2c.ctrl |= SPP_CS;
#else
   i2c.ctrl &= ~SPP_CS;
#endif

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sppspiclk0()  
{
#if (IICINV == 1)
   i2c.ctrl |= SPP_CLK;
#else
   i2c.ctrl &= ~SPP_CLK;
#endif   

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sppspiclk1()  
{
#if (IICINV == 0)
   i2c.ctrl |= SPP_CLK;
#else
   i2c.ctrl &= ~SPP_CLK;
#endif

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sppspidataout0()  
{
#if (IICINV == 1)
   i2c.ctrl |= SPP_DATAOUT;
#else
   i2c.ctrl &= ~SPP_DATAOUT;
#endif   

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sppspidataout1()  
{
#if (IICINV == 0)
   i2c.ctrl |= SPP_DATAOUT;
#else
   i2c.ctrl &= ~SPP_DATAOUT;
#endif

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sppspidatain()  
{
   if(i2c.enabled)
   {
      i2c.status = inb((uint16_t)(i2c.portaddr+1));
#if (IICINV == 1)
      return ((i2c.status & SPP_DATAIN) == 0);
#else
      return ((i2c.status & SPP_DATAIN) != 0);
#endif      
   }
   else
   {
      return 0;
   }
}

int32_t sdaout0()  
{
#if (IICINV == 1)
   i2c.ctrl |= SDAOUT;
#else
   i2c.ctrl &= ~SDAOUT;
#endif   

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sdaout1()  
{
#if (IICINV == 0)
   i2c.ctrl |= SDAOUT;
#else
   i2c.ctrl &= ~SDAOUT;
#endif

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sclout0()  
{
#if (IICINV == 1)
   i2c.ctrl |= SCLOUT;
#else
   i2c.ctrl &= ~SCLOUT;
#endif

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sclout1()  
{
#if (IICINV == 0)
   i2c.ctrl |= SCLOUT;
#else
   i2c.ctrl &= ~SCLOUT;
#endif

   if(i2c.enabled)
      outb(i2c.ctrl,(uint16_t)(i2c.portaddr+2));

   return 0;
}

int32_t sdain()  
{
   if(i2c.enabled)
   {
      i2c.status = inb((uint16_t)(i2c.portaddr+1));
#if (IICINV == 1)
      return ((i2c.status & SDAIN) == 0);
#else
      return ((i2c.status & SDAIN) != 0);
#endif      
   }
   else
   {
      return 0;
   }
}

/* do a little pause (must be calibrated) */
int32_t start()
{
   if(i2c.enabled)
   {
      sdaout1();         /* if SCL=1 this is a stop condition */
      sclout1();         /* prepare for start */
      sdaout0();         /* start condition */
      sclout0();         /* prepare next bit */
   }

   return 0;
}

int32_t stop()
{
   if(i2c.enabled)
   {      
      sdaout0();         /* if SCL=1 this is a start condition */
      sclout1();         /* prepare for stop */
      sdaout1();         /* stop condition */
   }   

   return 0;
}

/* CS must be low before */
int32_t sendSPIbyte(int32_t databyte)
{
   int32_t i;
   
   /*printf("Writing byte 0x%x\n", databyte);*/
   /* send byte MSB first */
   for(i = 7; i >= 0 ; i--)
   {
      if((databyte>>i) & 0x1)
         sppspidataout1();
      else
         sppspidataout0();
      sppspiclk0();
      sppspiclk1();
   }

   return 0;

}

int32_t sendbyte(int32_t databyte)
{
   int32_t i;
   
   /* send byte MSB first */
   for(i = 7; i >= 0 ; i--)
   {
      if((databyte>>i) & 0x1)
         sdaout1();
      else
         sdaout0();

      sclout1();
      sclout0();
   }

   /* check for device Acknowledge */
   sdaout1();
   sclout1();

   if(sdain()!=0)
   {
      return I2C_NACK_ERROR;
   }

   sclout0();

   return 0;

}

int32_t rcvdSPIbyte(uint8_t *databyte)
{
   int32_t i;

   *databyte = 0;

   /* rcvd byte MSB first */
   for(i = 7; i >= 0 ; i--)
   {      
      sclout0();
      *databyte |= (sppspidatain()<<i);
      sclout1();
   }

   return 0;
}
int32_t rcvdbyte(uint8_t *databyte, int32_t ack)
{
   int32_t i;

   *databyte = 0;

   /* rcvd byte MSB first */
   for(i = 7; i >= 0 ; i--)
   {      
      sclout1();
      *databyte |= (sdain()<<i);
      sclout0();
   }

   /* send Acknowledge (if ack==1) to device */
   if(ack) 
      sdaout0();
   else 
      sdaout1();

   sclout1();
   sclout0();
   sdaout1();
   
   return 0;
}
#endif /*(DIBCOM_TEST_MODE == TEST_MODE_SPP)*/
