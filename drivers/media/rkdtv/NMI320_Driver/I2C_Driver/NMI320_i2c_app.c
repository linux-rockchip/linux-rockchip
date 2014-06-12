#include "NMI320_i2c_app.h"
#include <linux/i2c.h>

#define I2C_Rate_NPM_K 	DVB_DEMODE_DIBCOM_I2C_RATE_K
#define BYTE unsigned char
extern int tstv_control_set_reg(struct i2c_client *client, unsigned char reg, unsigned char const buf[], int len);
extern int  tstv_control_read_regs(struct i2c_client *client, unsigned char reg, unsigned char buf[], unsigned int len);
extern struct i2c_client *gTSTVModuleClient;

static DECLARE_MUTEX(Tv_i2c_sem);


int NPM_bus_write(unsigned int adr, unsigned char *b, unsigned int sz)
{
	int result = -1, len;
	unsigned char buf[10];
	
	down(&Tv_i2c_sem);

	if (sz == 1) 
	{
		buf[0] = 0x30;
		buf[1] = (BYTE)(adr >> 16);
		buf[2] = (BYTE)(adr >> 8);
		buf[3] = (BYTE)(adr);
		buf[4] = 0x00;
		buf[5] = 0x01;
		buf[6] = b[0];
		len = 7;
	} 
	else if (sz == 4) 
	{
		buf[0] = 0x90;
		buf[1] = (BYTE)(adr >> 16);
		buf[2] = (BYTE)(adr >> 8);
		buf[3] = (BYTE)(adr);
		buf[4] = 0x00;
		buf[5] = 0x04;
		buf[6] = b[0];
		buf[7] = b[1];
		buf[8] = b[2];
		buf[9] = b[3];
		len = 10;
	} 
	else if(sz == 2) 
	{
		buf[0] = 0x50;
		buf[1] = (BYTE)(adr >> 16);
		buf[2] = (BYTE)(adr >> 8);
		buf[3] = (BYTE)(adr);
		buf[4] = 0x00;
		buf[5] = 0x02;
		buf[6] = b[0];
		buf[7] = b[1];
		len = 8;
	}
	//DTV_I2C_Init_if((unsigned char)0xc0,I2C_Rate_NPM_K);
	//result =DTV_I2C_Write_if((unsigned char)0xc0,buf,len,DirectMode);
/*	test
	buf[0] = 0;
	buf[1] = 2;
	buf[2] = 4;
	buf[3] = 8;
	buf[4] = 16;
	buf[5] = 32;
	buf[6] = 64;
	buf[7] = 128;
	len = 8;
*/
    gTSTVModuleClient->addr = (0xc0 >> 1);
	result = tstv_control_set_reg(gTSTVModuleClient,0,buf,len);
	//DTV_I2C_Deinit_if();	

	if(result == 0)
	{
		up(&Tv_i2c_sem);
	    return 1;
	}
	else
	{
	    up(&Tv_i2c_sem);
	    return 0;	
	}
}

int NPM_bus_read(unsigned int adr, unsigned char *b, unsigned int  sz)
{
	int result = -1, len;
	unsigned char buf[10];

	down(&Tv_i2c_sem);	
		if (sz == 1)
		{
			buf[0] = 0x20;
			buf[1] = (BYTE)(adr >> 16);
			buf[2] = (BYTE)(adr >> 8);
			buf[3] = (BYTE)(adr);
			buf[4] = 0x00;
			buf[5] = 0x01;
			len = 6;
		} 
		else if (sz == 4) 
		{
			buf[0] = 0x80;
	   		buf[1] = (BYTE)(adr >> 16);
	   		buf[2] = (BYTE)(adr >> 8);
	   		buf[3] = (BYTE)(adr);
	   		buf[4] = 0x00;
	   		buf[5] = 0x04;
			len = 6;
		} 
		else if(sz == 2) 
		{
			buf[0] = 0x40;
	   		buf[1] = (BYTE)(adr >> 16);
	   		buf[2] = (BYTE)(adr >> 8);
	   		buf[3] = (BYTE)(adr);
	   		buf[4] = 0x00;
	   		buf[5] = 0x02;
			len = 6;
		}
		gTSTVModuleClient->addr = (0xc0 >> 1);
		//DTV_I2C_Init_if(0xc0,I2C_Rate_NPM_K);
		result = tstv_control_set_reg(gTSTVModuleClient,0,buf,len);
		//result =DTV_I2C_Write_if(0xc0,buf,len,DirectMode);
		if (result == 0) 
			//DTV_I2C_Read_if(0xc0,b,sz,DirectMode);
			tstv_control_read_regs(gTSTVModuleClient,0,b,sz);
		//DTV_I2C_Deinit_if();
		
		if(result == 0)
		{
			up(&Tv_i2c_sem);
			return 1;
		}
		else
		{
			up(&Tv_i2c_sem);
			return 0;	
		}
}




