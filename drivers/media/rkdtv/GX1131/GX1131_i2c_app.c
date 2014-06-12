/**
 * CopyRight: all rights reserved by rockchips fuzhou inc. 
 * 02.22.2012 by Aiyoujun
 */

#include "GX1131_i2c_app.h"
#include <linux/device.h>

#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/i2c.h>
#include <linux/kthread.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>

#include <asm/system.h>

#include "GX1131.h"

extern struct i2c_client *gTSTVModuleClient;
extern int  tstv_control_set_reg(struct i2c_client *client, unsigned char reg, unsigned char const buf[], int len);
extern int  tstv_control_read_regs(struct i2c_client *client, unsigned char reg, unsigned char buf[], unsigned len);

/******************************************************************************
*Function:	IIC BUS Write and Read function
*Input:	
*	WR_flag		-- 1: Write, 0: Read
*	ChipAddress	-- Chip Address
*	RegAddress   -- Write or Read register address
*	*data		-- the pointer of write or read data
*	data_number  -- the data number to be read or written
*Output:	
*        SUCCESS : return 1  , *data = Read_value ( only available when read operation )
*        FAILURE : return -1
*******************************************************************************/
bool GX_I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number)
{
	static unsigned char buf[1024];
	int result = -1, len;

       gTSTVModuleClient->addr = (ChipAddress >> 1);  //7bit address
	
	if (WR_flag==0) //read
	{
	    tstv_control_set_reg(gTSTVModuleClient,0,&RegAddress,1);	
	    //rx data
	    tstv_control_read_regs(gTSTVModuleClient, 0, data, data_number);	
	}
	else //write
	{
	    //register address and tx data
	    buf[0] = RegAddress;
	    len = (data_number+1);
           memcpy(&buf[1], data, data_number);		
	    tstv_control_set_reg(gTSTVModuleClient, 0, buf, len);	
	}

	return SUCCESS;
}