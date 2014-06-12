/**
 * CopyRight: all rights reserved by rockchips fuzhou inc. 
 * 10.30.2010 by Jan
 */

#include "mtv818_i2c_app.h"
#include <linux/i2c.h>
#include <linux/device.h>

/*MTV818 I2C ADDRESS*/
#define CHIP_ADDR 0x86


extern struct i2c_client *gTSTVModuleClient;
extern int  tstv_control_set_reg(struct i2c_client *client, unsigned char reg, unsigned char const buf[], int len);
extern int  tstv_control_read_regs(struct i2c_client *client, unsigned char reg, unsigned char buf[], unsigned len);

/**
 * I2C READ MODE
 *
 * tx register address:
 *       7-bit         1-bit        8-bit
 * ---------------------------------------------------------------
 * |   |             |   _ |     |                  |     |      |
 * | S | I2C ADDRESS | R/W | ACK | REGISTER ADDRESS | ACK | STOP |
 * ---------------------------------------------------------------
 *
 * rx data:
 *       7-bit         1-bit         8-bit                  8-bit
 * ---------------------------------------------------------------------------------------------
 * |   |             | _   |     |  DATA            |     |  DATA              |   |     |     |
 * | S | I2C_ADDRESS | R/W | ACK |(REGISTER ADDRESS)| ACK |(REGISTER ADDRESS+1)|ACK| ... | STOP|
 * ---------------------------------------------------------------------------------------------
 */
unsigned char dxbIO_ISDB_I2C_Read(unsigned char reg)
{
    unsigned char data;
    unsigned char txdata;
    unsigned char rxdata[2];

    txdata = reg;

   gTSTVModuleClient->addr = (CHIP_ADDR >> 1);  //7bit address

    //tx register address
    tstv_control_set_reg(gTSTVModuleClient, reg, &txdata, 1);
    //rx data
    tstv_control_read_regs(gTSTVModuleClient, reg, rxdata, 1);
    data = rxdata[0];

    //printk("MTV818:(r) i2c data 0x%x\n", data);
    return data;
}

/**
 * I2C_WRITE_MODE
 *
 * tx register address and tx data:
 *       7-bit         1-bit       8-bit                      8-bit
 * -----------------------------------------------------------------------------------------------
 * |   |             |   _ |     |                  |     |    DATA            |     |     |     |
 * | S | I2C ADDRESS | R/W | ACK | REGISTER ADDRESS | ACK | (REGISTER ADDRESS) | ACK | ... | STOP|
 * -----------------------------------------------------------------------------------------------
 */
void dxbIO_ISDB_I2C_Write(unsigned char reg, unsigned char data)
{
    unsigned char txdata[3];

    txdata[0] = reg;
    txdata[1] = data;

   gTSTVModuleClient->addr = (CHIP_ADDR >> 1);  //7bit address

    //printk("MTV818:(w) i2c data 0x%x\n", data);
    //register address and tx data
    tstv_control_set_reg(gTSTVModuleClient, reg, txdata, 2);
}

