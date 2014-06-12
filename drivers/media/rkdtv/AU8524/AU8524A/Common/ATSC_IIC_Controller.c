/******************************************************************/
/*   Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved.  */
/*******************************************************************


*********************************************************************/
#include<linux/string.h>
#include <linux/module.h>
//#include "AUChipTypedef.h"
#include "AuvitekTunerInterfaceTypedef.h"
#include <linux/i2c.h>

extern int tstv_control_set_reg(struct i2c_client *client, unsigned char reg, unsigned char const buf[], int len);
extern int  tstv_control_read_regs(struct i2c_client *client, unsigned char reg, unsigned char buf[], unsigned int len);
extern struct i2c_client *gTSTVModuleClient;
#define ATSC_AU8524A_ADD    0x47

/**************************************************************************
* 函数描述: 
* 入口参数: 
* 出口参数: 
* 返回值:   
***************************************************************************/
int ATSC_I2C_Init(UINT8 SlaveAddress, UINT16 nKHz)
{
	gTSTVModuleClient->addr = SlaveAddress>>0x1;

}

/**************************************************************************
* 函数描述: 
* 入口参数: 
* 出口参数: 
* 返回值:   
***************************************************************************/
int ATSC_I2C_Write8(UINT8 RegAddr, UINT8 *pData, UINT16 size)
{
	int result = -1, len;
	static unsigned char buf[1024];

	if(size > 1020)
	{
	    return 0;
	}
	
       buf[0] = RegAddr;
	len = (size+1);
	memcpy(&buf[1], pData, size);
       //gTSTVModuleClient->addr = ATSC_AU8524A_ADD;
	result = tstv_control_set_reg(gTSTVModuleClient,0,buf,len);

	if(result == 0)
	{
	    return 1;
	}
	else
	{
	    return 0;	
	}	
}

/**************************************************************************
* 函数描述: 
* 入口参数: 
* 出口参数: 
* 返回值:   
***************************************************************************/
int ATSC_I2C_Write(UINT16 RegAddr, UINT8 *pData, UINT16 size)
{
	int result = -1, len;
	static unsigned char buf[1024];
	   
	if(size > 1020)
	{
	    return 0;
	}
	
       buf[0] = (UINT8)(RegAddr >> 8);
       buf[1] = (UINT8)(RegAddr & 0xFF);	 
	len = (size+2);
	memcpy(&buf[2], pData, size);
       //gTSTVModuleClient->addr = ATSC_AU8524A_ADD;
	result = tstv_control_set_reg(gTSTVModuleClient,0,buf,len);

	if(result == 0)
	{
	    return 1;
	}
	else
	{
	    return 0;	
	}	
}

/**************************************************************************
* 函数描述: 
* 入口参数: 
* 出口参数: 
* 返回值:   
***************************************************************************/
int ATSC_I2C_Read8(UINT8 RegAddr, UINT8 *pData, UINT16 size)
{
	int result = -1, len;
	static unsigned char buf[1024];
	   
	if(size > 1020)
	{
	    return 0;
	}
	
       buf[0] = 0xfb;
       buf[1] = RegAddr;	 
	len = 2;
       //gTSTVModuleClient->addr = ATSC_AU8524A_ADD;
	result = tstv_control_set_reg(gTSTVModuleClient,0,buf,len);

	result = tstv_control_read_regs(gTSTVModuleClient,0,pData,size);

	if(result == 0)
	{
	    return 1;
	}
	else
	{
	    return 0;	
	}	
}


/**************************************************************************
* 函数描述: 
* 入口参数: 
* 出口参数: 
* 返回值:   
***************************************************************************/
int ATSC_I2C_Read(UINT16 RegAddr, UINT8 *pData, UINT16 size)
{
	int result = -1, len;
	static unsigned char buf[1024];
	   
	if(size > 1020)
	{
	    return 0;
	}
	
       buf[0] = (UINT8)(RegAddr >> 8);
       buf[1] = (UINT8)(RegAddr & 0xFF);	  
       buf[2] = 0x11;	  	   
	len = 3;
       //gTSTVModuleClient->addr = ATSC_AU8524A_ADD;
	result = tstv_control_set_reg(gTSTVModuleClient,0,buf,len);

	result = tstv_control_read_regs(gTSTVModuleClient,0,pData,size);

	if(result == 0)
	{
	    return 1;
	}
	else
	{
	    return 0;	
	}	
}

/**************************************************************************
* 函数描述: 
* 入口参数: 
* 出口参数: 
* 返回值:   
***************************************************************************/
int ATSC_I2C_Deinit(void)
{

}

