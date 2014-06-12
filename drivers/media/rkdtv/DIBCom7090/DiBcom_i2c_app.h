#ifndef __DIBCOM_I2C_HHHHHHHHHH
#define __DIBCOM_I2C_HHHHHHHHHH

//////////////////////////////////////
//include the .h file here
#if 0
#include <stdio.h>
#include "os_defs.h"
#include "globals.h"
#include "demod.h"
#include "opentvx.h"
#include "i68dbg.h"
#include "gpio.h"
#include "i2c.h"
#include "Dmx_TsdDataRam.h"
#include "a_post.h"
#include "board_config.h"
#include "gpio.h"
#endif


///////////////////////////////////////

//dibcom write
unsigned char DiBcom_i2c_write(int addr, unsigned char *i2c_tx_buf, int txcnt);

//dibcom before read , should write the register address first (write without stop)
unsigned char DiBcom_i2c_read(int addr, unsigned char *i2c_tx_buf, int txcnt, unsigned char  *i2c_rx_buf, int rxcnt);

#endif
