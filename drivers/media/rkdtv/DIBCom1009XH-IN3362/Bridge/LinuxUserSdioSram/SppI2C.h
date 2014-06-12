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
* @file "SppI2C.h"
* @brief Target Specific.
*
***************************************************************************************************/
#if (DIBCOM_TEST_MODE == TEST_MODE_SPP)

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

#ifndef SPP_I2C_H
#define SPP_I2C_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

/* Errors and warnings */
#ifndef SPP_I2C_OFFSET
#define SPP_I2C_OFFSET               0
#endif

#define NO_PORT_FOUND_ERROR            (SPP_I2C_OFFSET+1)
#define SDA_LOW_TEST_ERROR             (SPP_I2C_OFFSET+2)
#define SDA_HIGH_TEST_ERROR            (SPP_I2C_OFFSET+3)
#define I2C_TX_BUFFER_ERROR            (SPP_I2C_OFFSET+4)
#define I2C_RX_BUFFER_ERROR            (SPP_I2C_OFFSET+5)
#define I2C_NACK_ERROR                 (SPP_I2C_OFFSET+6)
#define I2C_DEV_ADR_NACK_ERROR         (SPP_I2C_OFFSET+7)
#define I2C_WR_NACK_ERROR              (SPP_I2C_OFFSET+8)
#define SPP_SUPERUSER_RIGHTS_ERROR     (SPP_I2C_OFFSET+9)
#define SPP_INVALID_PORT_TYPE          (SPP_I2C_OFFSET+10)

struct i2clpt
{
   uint16_t portaddr;
   int      mindelay;
   int      enabled;
   int      ctrl;
   int      status;
   int      Data;
};

/* int dib_usleep(int t_us ); */
int ExistParallelPort(int * portaddr);
int ConnectParallelPort(int portaddr , int port_type);

/* check if this physical LPT port exist (1=yes, 0=no) */
int ExistPort(int port);

/* initialize printer port and check if device present */
int SppInit(int port, int timestep, int *version);
int SppInit4Spi(int port, int timestep, int *version);

/* program Value on Data pins of printer port */
int DataBus(int datavalue);

/* transmit txcnt bytes to device */
int SppI2CWrite(int Addr,  uint8_t *i2c_tx_buf, int txcnt);

/* transmit txcnt bytes to device then read rxcnt bytes */
int SppI2CWriteRead(int Addr,  uint8_t  *i2c_tx_buf, int txcnt, uint8_t *i2c_rx_buf, int rxcnt);

int32_t SppSpiRead(uint8_t *rxbuf, int32_t rxcnt);
int32_t SppSpiWrite(uint8_t *txbuf, int32_t txcnt);
int sppspics0();
int sppspidataout0();
int sppspiclk0();
int sppspics1();
int sppspidataout1();
int sppspiclk1();
int sppspidatain();

/* elementary functions */
int sdaout1();
int sdaout0();
int sclout1();
int sclout0();
int sdain();
int start();
int stop();
int sendbyte(int databyte);
int32_t sendSPIbyte(int32_t databyte);
int rcvdbyte(uint8_t *databyte, int ack);
int rcvdSPIbyte(uint8_t *databyte);

#endif

#endif
