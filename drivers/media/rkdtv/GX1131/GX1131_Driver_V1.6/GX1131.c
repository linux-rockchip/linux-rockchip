/*******************************************************************************
*                          CONFIDENTIAL                                                                                         
*        Hangzhou NationalChip Science and Technology Co., Ltd.                                               
*                      (C)2009, All right reserved                                                                            
********************************************************************************

********************************************************************************
* File Name	 :   	gx1131.c                                                                                                 
* Author		 :     Mike Feng                                                                                                  
* Project    	 :   	GXAPI 
* Type      	 :     Driver
********************************************************************************
* Purpose   :   GX1131  driver
********************************************************************************
* Release History:
* VERSION   Date              AUTHOR          Description
*    1.6      2011.06.24      	Jerome Lou   
********************************************************************************
*Abbreviation
*   GX		--	GUOXIN 
*   RF		--  radiate frequency
*   SNR		--	signal to noise ratio
*   OSC		--	oscillate
*   SPEC		--	spectrum
*   FREQ		--	frequency
********************************************************************************/
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
//#include <math.h>
//#include <stdlib.h>
//#include <stdio.h>

#define DBG(x...)	printk(KERN_INFO x)

s8 gStr_driver_version[]="GX1131 V1.6 Build20110624";
extern u8 gIQ_swap;
int dat[32];
static GX1131_BCS_TYPE sg_BSType = {0};

/******************************************************************************
*Function:	Delay N ms
*Input:		
*       ms_value - delay time (ms)
*******************************************************************************/
/*void GX_Delay_N_ms(u32 ms_value)
{
	//TODO:User must add the delay function
	return 1;
}*/
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
/*GX1131_STATE GX_I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number)
{
	//TODO:User must add the IIC BUS Write and Read function
	return SUCCESS;
}*/

/******************************************************************************
*Function:	Delay N ms
*Input:		
*       ms_value - delay time (ms)
*******************************************************************************/
void GX_Delay_N_ms(u32 ms_value)
{
	//_sleep(ms_value);//TODO:User must add the IIC BUS Write and Read function
	msleep(ms_value);
}

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
/*
bool GX_I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number)
{	
	//SetPort(0xDFF0);
	u32 data_tmp[64]={0};//TODO:User must add the IIC BUS Write and Read function
	u8 i;
	//Convert u8 data array to u32 because of ReadValue and WriteValue functions definition
	if (WR_flag==0)
		{
			if(ReadValue(ChipAddress, RegAddress, data_tmp, data_number, 8, 0)==0) return FAILURE;
			for(i=0;i<data_number;i++)
				{
					data[i]=(u8)data_tmp[i];
				}
		}
	else if (WR_flag==1)	
		{
			for(i=0;i<data_number;i++)
				{
					data_tmp[i]=data[i];
				}
			if(WriteValue(ChipAddress, RegAddress, data_tmp, data_number, 8)==0) return FAILURE;
		}
	
	return SUCCESS;
}
*/
extern bool GX_I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number);

/******************************************************************************
*Function:	Get the driver version of GX1131
*output:		
*       	return the driver version of GX1131
*******************************************************************************/
s8 *GX1131_Get_Driver_Version(void)
{
	return gStr_driver_version;
}

/*******************************************************************************
*Function: Write one byte to chip                                                                                           
*Input:                                                                                                                                
*         RegAddress -- The register address                                                                          
*         WriteValue  -- The write value
*Output:
*         SUCCESS or FAILURE
********************************************************************************/
bool GX1131_Write_one_Byte(u8 RegAddress,u8 WriteValue)
{	
	return GX_I2cReadWrite( WRITE, GX1131_CHIP_ADDRESS,RegAddress,&WriteValue, 1);	
}

/*******************************************************************************
*Function: Write one byte to chip with read test 
*Input:
*        RegAddress -- The register address
*        WriteValue  -- The write value
*Output:
*       SUCCESS or FAILURE
********************************************************************************/
bool GX1131_Write_one_Byte_ReadTest(u8 RegAddress,u8 WriteValue)
{
	u8 temp;
	
	GX_I2cReadWrite(WRITE, GX1131_CHIP_ADDRESS,RegAddress,&WriteValue, 1 );	
	GX_I2cReadWrite(READ, GX1131_CHIP_ADDRESS,RegAddress,&temp,1);
	if(temp==WriteValue)
			return SUCCESS;
	else
			return FAILURE;	
}

/******************************************************************************
*Function: Read one byte from chip
*Input:
*        RegAddress -- The register address
*Output:
*        SUCCESS: the value of RegAddress
*        FAILURE  :  FAILURE 
*******************************************************************************/
u8 GX1131_Read_one_Byte(u8 RegAddress)
{
	bool state=FAILURE;	
       u8 Temp_Read_data =0;	

	state = GX_I2cReadWrite(READ, GX1131_CHIP_ADDRESS,RegAddress,&Temp_Read_data, 1 );
	if (state==SUCCESS)
		return Temp_Read_data;
	else
		return FAILURE;
}

/******************************************************************************
*Function:	Get the Chip ID of GX1131
*output:		
*       	return the Chip ID of GX1131
*******************************************************************************/
GX1131_STATE GX1131_Get_Chip_ID(void)
{
	u8 temp1,temp2;	

	temp1=GX1131_Read_one_Byte(GX1131_CHIP_ID_L);
	temp2=GX1131_Read_one_Byte(GX1131_CHIP_ID_H);			
	return ((temp2<<8)|temp1);	
}

/******************************************************************************
*Function: Hot reset the Chip 
*Output:
*        SUCCESS or FAILURE 
*******************************************************************************/
bool GX1131_HotReset_CHIP(void)
{
	u8 temp;

	temp=GX1131_Read_one_Byte(GX1131_RST);
	temp|=0x01;	 
	return GX1131_Write_one_Byte(GX1131_RST,temp);	
}

/******************************************************************************
*Function: Cool reset the Chip 
*Output:
*        SUCCESS or FAILURE 
*******************************************************************************/
bool GX1131_CoolReset_CHIP(void)
{
	u8 temp;

	temp=GX1131_Read_one_Byte(GX1131_RST);
	temp|=0x02;	
	return GX1131_Write_one_Byte(GX1131_RST,temp);	
}

/******************************************************************************
*Function: Power down the internal ADC of GX1131
*Output:
*        SUCCESS or FAILURE 
*******************************************************************************/
bool GX1131_ADC_Init(void)
{
	u8 temp;	

	temp=GX1131_Read_one_Byte(GX1131_ADC_CTRL1);	
	temp&=0x3f;
	GX1131_Write_one_Byte(GX1131_ADC_CTRL1,temp);		
	GX_Delay_N_ms(20);
	
	temp=GX1131_Read_one_Byte(GX1131_ADC_CTRL1);	
	temp|=0xc0;	
	GX1131_Write_one_Byte(GX1131_ADC_CTRL1,temp);
		
	return SUCCESS;
}

/******************************************************************************
*Function: Set PLL1 and PLL2
*input:
*	   sysclk  unit: Hz
*Output:
*	   SUCCESS or FAILURE 
*******************************************************************************/
bool GX1131_PLL_Control(void)
{	
	u8 temp;	

	//Configure PLL1
	//Set PLL1_PD to 1
	temp=GX1131_Read_one_Byte(GX1131_PLL1_CTRL_RD);	
	temp|=0x80;
	GX1131_Write_one_Byte(GX1131_PLL1_CTRL_RD,temp);	
	//Set PLL1_D
	temp=GX1131_Read_one_Byte(GX1131_PLL1_CTRL_RD);	
	temp=(temp&0x9f)|(GX1131_PLL1_D_VALUE<<5);	
	GX1131_Write_one_Byte(GX1131_PLL1_CTRL_RD,temp);	
	//Set PLL1_R
	temp=GX1131_Read_one_Byte(GX1131_PLL1_CTRL_RD);	
	temp=(temp&0xe0)|(GX1131_PLL1_R_VALUE);
	GX1131_Write_one_Byte(GX1131_PLL1_CTRL_RD,temp);	
	//Set PLL1_B
	temp=GX1131_Read_one_Byte(GX1131_PLL1_CTRL_B);	
	temp=(temp&0x00)|(GX1131_PLL1_B_VALUE);
	GX1131_Write_one_Byte(GX1131_PLL1_CTRL_B,temp);	
	//Set PLL1_PD to 0
	temp=GX1131_Read_one_Byte(GX1131_PLL1_CTRL_RD);	
	temp&=0x7f;
	GX1131_Write_one_Byte(GX1131_PLL1_CTRL_RD,temp);	
	GX_Delay_N_ms(50);
	//Configure PLL2	
	//Set PLL2_PD to 1	
	temp=GX1131_Read_one_Byte(GX1131_PLL2_CTRL_RD);
	temp|=0x80;
	GX1131_Write_one_Byte(GX1131_PLL2_CTRL_RD,temp);
	//Set PLL2_D
	temp=GX1131_Read_one_Byte(GX1131_PLL2_CTRL_RD);	
	temp=(temp&0x9f)|(GX1131_PLL2_D_VALUE<<5);
	GX1131_Write_one_Byte(GX1131_PLL2_CTRL_RD,temp);	
	//Set PLL2_R
	temp=GX1131_Read_one_Byte(GX1131_PLL2_CTRL_RD);	
	temp=(temp&0xe0)|(GX1131_PLL2_R_VALUE);	
	GX1131_Write_one_Byte(GX1131_PLL2_CTRL_RD,temp);	
	//Set PLL2_B
	temp=GX1131_Read_one_Byte(GX1131_PLL2_CTRL_B);	
	temp=(temp&0x00)|(GX1131_PLL2_B_VALUE);	
	GX1131_Write_one_Byte(GX1131_PLL2_CTRL_B,temp);	
	//Set PLL2_PD to 0
	temp=GX1131_Read_one_Byte(GX1131_PLL2_CTRL_RD);	
	temp&=0x7f;
	GX1131_Write_one_Byte(GX1131_PLL2_CTRL_RD,temp);
	GX_Delay_N_ms(20);
	//Set the ratio of demod clock and decoder colck
	GX1131_Write_one_Byte(GX1131_PLL_Cfg_Num,GX1131_PLL_CFG_NUM_VALUE);	
	GX1131_Write_one_Byte(GX1131_PLL_Cfg_Denom,GX1131_PLL_CFG_DENOM_VALUE);
	GX_Delay_N_ms(10);
	//Configure Fsample(sysclk)
	GX1131_Write_one_Byte(GX1131_Fsample_Cfg_L,GX1131_FSAMPLE_VALUE&0xff);	
	GX1131_Write_one_Byte(GX1131_Fsample_Cfg_M,(GX1131_FSAMPLE_VALUE>>8)&0xff);	
	GX1131_Write_one_Byte(GX1131_Fsample_Cfg_H,(GX1131_FSAMPLE_VALUE>>16)&0x01);

	return SUCCESS;
}

/******************************************************************************
*Funtion:	Set chip sleep or wake up
*Input:
*	   Sleep    --    1: Sleep      0: Wake up
*Output:
*        SUCCESS or FAILURE
*******************************************************************************/
bool GX1131_Set_Sleep(bool Sleep)
{
	u8 temp=0;
	
	temp=GX1131_Read_one_Byte(GX1131_CTRL0);	
	temp=(temp&0x7f)|(Sleep<<7);						
	GX1131_Write_one_Byte(GX1131_CTRL0,temp);	
	if (Sleep==0)
		GX1131_HotReset_CHIP();			
	
	return SUCCESS;
}

/******************************************************************************
*Function: Enable/Disable the Tunner repeater
*Input:	
*        1 - On
*        0 - Off
*Output:
*        SUCCESS or FAILURE 
********************************************************************************/
bool GX1131_Set_Tuner_Repeater_Enable(bool Rpt_On)
{
	u8 temp;

	temp=GX1131_Read_one_Byte(GX1131_I2C_REPEAT);	
	if(Rpt_On==1)
		temp|=0x80;        /*On*/		
	else
		temp&=0x7f;        /*Off*/		
	GX1131_Write_one_Byte(GX1131_I2C_REPEAT,temp);
	
	return SUCCESS;
}

/******************************************************************************
*Function: Enable/Disable the Ts_out
*Input:	
*        1 - Off
*        0 - On
*Output:
*        SUCCESS or FAILURE 
********************************************************************************/
bool GX1131_Set_Ts_Out_Disable(bool ts_off)
{
	u8 temp;

	temp=GX1131_Read_one_Byte(GX1131_CTRL0);
	if(ts_off==1)
		temp|=0x08;
	else
		temp&=0xf7;
	GX1131_Write_one_Byte(GX1131_CTRL0,temp);
	
	return SUCCESS;
}

/******************************************************************************
*Function: Set AGC Parameter
*Output:
*        SUCCESS , FAILURE 
*******************************************************************************/
bool GX1131_Set_AGC_Parameter(void)
{
	u8 temp;

	temp=GX1131_Read_one_Byte(GX1131_AGC_STD);	
	temp=(temp&0xc0)|(GX1131_AGC_STD_VALUE);	
	GX1131_Write_one_Byte(GX1131_AGC_STD, temp);	
	
	temp=GX1131_Read_one_Byte(GX1131_AGC_MODE);	
	temp=(temp&0xa0)|0x5c;
	if(!GX1131_AGC_POLARITY)
		temp=temp&0xbf;
	GX1131_Write_one_Byte(GX1131_AGC_MODE,temp);

	return SUCCESS;
}

/******************************************************************************
*Function: TS inteface config
*Output:
*	     SUCCESS - Configuration success, FAILURE - Configuration falure 
*******************************************************************************/
bool GX1131_Set_TS_Inteface(void)
{
	u8 temp;
	
	GX1131_Write_one_Byte(GX1131_CFG_TS_0,CFG_TS_0);
	GX1131_Write_one_Byte(GX1131_CFG_TS_2,CFG_TS_2);
	GX1131_Write_one_Byte(GX1131_CFG_TS_4,CFG_TS_4);
	GX1131_Write_one_Byte(GX1131_CFG_TS_6,CFG_TS_6);
	GX1131_Write_one_Byte(GX1131_CFG_TS_8,CFG_TS_8);
	GX1131_Write_one_Byte(GX1131_CFG_TS_A,CFG_TS_A);

	temp=GX1131_Read_one_Byte(GX1131_TS_MODE);	
	temp=(temp&0xf9)|(GX1131_TS_CLK_POL<<2)|(GX1131_TS_OUTPUT_MODE<<1);	
	GX1131_Write_one_Byte(GX1131_TS_MODE,temp);
	
	return SUCCESS;
}

/******************************************************************************
*Function: set the work mode of GX1131: lock sepcified TP or blind scan
*Input:
*	     Normal: GX1131 work in normal mode(0) or blind scan mode(1)
*Output:
*	     SUCCESS - Configuration success, FAILURE - Configuration falure 
*******************************************************************************/
bool GX1131_Set_Work_BS_Mode(bool bs)
{
	u8 temp1,temp2,temp3;

	temp1=GX1131_Read_one_Byte(GX1131_BCS_RST);	
	temp2=GX1131_Read_one_Byte(GX1131_CTRL1);	
	temp3=GX1131_Read_one_Byte(GX1131_AUTO_RST);	

	if(bs==0)
		{
			GX1131_Write_one_Byte(GX1131_BCS_RST,temp1|0x81);			
			GX1131_Write_one_Byte(GX1131_CTRL1,temp2&0xf7);			
			GX1131_Write_one_Byte(GX1131_AUTO_RST,temp3&0xbf);			
		}
	else if(bs==1)
		{
			GX1131_Write_one_Byte(GX1131_BCS_RST,(temp1&0x7f)|0x01);			
			GX1131_Write_one_Byte(GX1131_CTRL1,temp2|0x08);			
			GX1131_Write_one_Byte(GX1131_AUTO_RST,temp3|0x40);			
		}
	
	return SUCCESS;
}

/******************************************************************************
*Function: Set symbol rate 
*Input:
*        Symbol_Rate_Value (Unit: kHz)
*Output:
*        SUCCESS or FAILURE
*******************************************************************************/
s32 GX1131_SetSymbolRate(u32 Symbol_Rate_Value)
{	
   	s32 Center_shift_Value=0;		
	if(Symbol_Rate_Value==0)return FAILURE;

	GX1131_Write_one_Byte(GX1131_SYMBOL_H,(u8)((Symbol_Rate_Value>>8)&0xff));		
	GX1131_Write_one_Byte(GX1131_SYMBOL_L,(u8)(Symbol_Rate_Value&0xff));	
	//Adjust the Fc offset when small symbol rate according to different tuner
	#if(TUNER_SHARP7306==1)
		if(Symbol_Rate_Value <5000)
			Center_shift_Value=3000;
	#endif
	#if(TUNER_ZL10037==1)
		if(Symbol_Rate_Value<5000)
			Center_shift_Value=3000;
	#endif
	#if(TUNER_RDA5812==1)
		if(Symbol_Rate_Value<5000)
			Center_shift_Value=3000;
	#endif	
	#if(TUNER_AV2011==1)
		if(Symbol_Rate_Value<5000)
			Center_shift_Value=3000;
	#endif
	#if(TUNER_STV6110A==1)
		if(Symbol_Rate_Value<5000)
			Center_shift_Value=3000;
	#endif
	if(gIQ_swap==1)
		Center_shift_Value=0-Center_shift_Value;
	Center_shift_Value&=0xffff;
	GX1131_Write_one_Byte(GX1131_FC_OFFSET_H,(u8)((Center_shift_Value>>8)&0xff));	
	GX1131_Write_one_Byte(GX1131_FC_OFFSET_L,(u8)(Center_shift_Value&0xff));	
	
	return Center_shift_Value;
}

/******************************************************************************
*Function: Configure the polarity of LNB, 13V/18V is decided by the hw circuit
*input:
*  	   Polar	1:    V    
*			0:    H  
*Output:
*	   SUCCESS or FAILURE
*******************************************************************************/
bool GX1131_Set_Polar(bool Vpolor)
{      
	u8	temp;
	if(POL_SWITCH) Vpolor = !Vpolor;
	
	temp=GX1131_Read_one_Byte(GX1131_DISEQC_MODE);
	if(Vpolor==0)
		temp = temp & 0xBF;	//18V select	
	else if(Vpolor==1)
		temp=temp|0x40;	//13V select	
	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,temp);
	
	
	return  SUCCESS;	
}

/******************************************************************************
*Function: Control 22K on/off
*input:
*	   22k_on  1:	22K on
*		         0:	22K off
*Output:
*	   SUCCESS or FAILURE
*******************************************************************************/
bool GX1131_22K_sw(bool b22k_on)
{	
	u8	temp;
	u32	diseqc_Ratio;
		
	diseqc_Ratio=((((GX1131_FSAMPLE_VALUE*10)/88)+5)/10)&0x07ff;	
	GX1131_Write_one_Byte(GX1131_DISEQC_RATIO_L, (u8)(diseqc_Ratio&0xff));	
	temp=GX1131_Read_one_Byte(GX1131_DISEQC_RATIO_H);	
	temp = (temp&0xF8)|(u8)(((diseqc_Ratio&0x0700)>>8));	
	GX1131_Write_one_Byte(GX1131_DISEQC_RATIO_H, temp);	

	temp=GX1131_Read_one_Byte(GX1131_MODULE_RST);	
	temp=temp&0xbf;
	GX1131_Write_one_Byte(GX1131_MODULE_RST, temp);	

	temp=GX1131_Read_one_Byte(GX1131_DISEQC_MODE);	
	temp &= 0xF8;
	if(b22k_on ==1)
		temp |= 0x01;	
	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,temp);
	
	return SUCCESS;
}

/******************************************************************************
*Function: Diseqc1.0 control, select 1 of 4 antennas
*input:
*	   Lnb_Num: the sequence number of antenna
*Output:    
*	   SUCCESS or FAILURE
*******************************************************************************/
bool GX1131_Sel_Lnb(u8 Lnb_Num)
{	
	u8	   	buffer[4];
	u8		temp,diseqc_mode;
	u32	    	i;

	buffer[0] = GX1131_DISEQC_CMD1 & 0xFF;
	buffer[1] = GX1131_DISEQC_CMD2 & 0xFF;
	buffer[2] = GX1131_DISEQC_CMD3 & 0xFF;    
	diseqc_mode=GX1131_Read_one_Byte( GX1131_DISEQC_MODE);
	
	if(diseqc_mode & 0x1)//22k on
	{
		if(diseqc_mode & 0x40)// 13v select
		{
			switch(Lnb_Num)
            {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW02;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW06;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW10;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW14;
                    break;                                    
            } 
		}
		else
		{
			 switch(Lnb_Num)
             {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW04;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW08;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW12;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW16;
                    break;                                    
             }  
		}
	}
	else
	{
		if(diseqc_mode & 0x40)// 13v select
		{
			switch(Lnb_Num)
            {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW01;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW05;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW09;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW13;
                    break;                                    
            }  
		}
		else
		{
			 switch(Lnb_Num)
            {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW03;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW07;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW11;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW15;
                    break;                                    
            }  
		}
	}
	temp=GX1131_Read_one_Byte(GX1131_MODULE_RST);	
	temp=temp&0xbf;
	GX1131_Write_one_Byte(GX1131_MODULE_RST, temp);	
	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS1, buffer[0]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS2, buffer[1]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);		
		
	temp =  (diseqc_mode&0xc0) | 0x1c;	
	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,temp);	
	GX_Delay_N_ms(30);
	
	for(i = 0;i<1000;i++)
	{
		temp= GX1131_Read_one_Byte(GX1131_DISEQC_RD_INT);		
		if( (temp&0x10) == 0x10)
			break;		
		else if(i==999)
			return FAILURE;		
		GX_Delay_N_ms(5);
	}

	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,diseqc_mode);
	
	return SUCCESS;     
}
/******************************************************************************
*Function: Diseqc1.1 control, select 1 of 16 antennas
*input:
*	   Lnb_Num: the sequence number of antenna
*Output:    
*	   SUCCESS or FAILURE
*******************************************************************************/
bool GX1131_Diseqc_11(u8 chCom,u8 Lnb_Num)
{	
	u8	   	buffer[4];
	u8		temp,diseqc_mode;
	u32	    	i;

	buffer[0] = GX1131_DISEQC_CMD1 & 0xFF;
	buffer[1] = GX1131_DISEQC_CMD2 & 0xFF;
	buffer[2] = GX1131_DISEQC_CMD3 & 0xFF;  
	diseqc_mode=GX1131_Read_one_Byte( GX1131_DISEQC_MODE);

	if(diseqc_mode & 0x1)//22k on
	{
		if(diseqc_mode & 0x40)// 13v select
		{
			switch(chCom)
            {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW02;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW06;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW10;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW14;
                    break;                                    
            } 
		}
		else
		{
			 switch(chCom)
             {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW04;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW08;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW12;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW16;
                    break;                                    
             }  
		}
	}
	else
	{
		if(diseqc_mode & 0x40)// 13v select
		{
			switch(chCom)
            {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW01;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW05;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW09;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW13;
                    break;                                    
            }  
		}
		else
		{
			 switch(chCom)
            {
                default:
                case 1:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW03;
                    break;
                case 2:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW07;
                    break;
                case 3:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW11;
                    break;
                case 4:
                    buffer[3]   = (u8)GX1131_DISEQC_CMD4_SW15;
                    break;                                    
            }  
		}
	}
	
	temp=GX1131_Read_one_Byte(GX1131_MODULE_RST);	
	temp=temp&0xbf;
	GX1131_Write_one_Byte(GX1131_MODULE_RST, temp);	
	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS1, buffer[0]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS2, buffer[1]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);		
		
	temp =  (diseqc_mode&0xc0) | 0x1c;	
	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,temp);	
	GX_Delay_N_ms(30);
	
	for(i = 0;i<1000;i++)
	{
		temp= GX1131_Read_one_Byte(GX1131_DISEQC_RD_INT);		
		if( (temp&0x10) == 0x10)
			break;		
		else if(i==999)
			return FAILURE;		
		GX_Delay_N_ms(5);
	}

	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,diseqc_mode);

	buffer[0] = GX1131_DISEQC_CMD1 & 0xFF;
	buffer[1] = GX1131_DISEQC_CMD2 & 0xFF;
	buffer[2] = GX1131_DISEQC_CMD3_1 & 0xFF;    
	GX1131_Write_one_Byte(GX1131_DISEQC_INS1, buffer[0]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS2, buffer[1]);	
	GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);
	if(Lnb_Num>16)Lnb_Num=1;
	temp=GX1131_DISEQC_CMD4_LNB1+(Lnb_Num-1);
	GX1131_Write_one_Byte(GX1131_DISEQC_INS4, temp);		
	
	temp =  (diseqc_mode&0xc0) | 0x1c;	
	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,temp);	
	GX_Delay_N_ms(30);
	
	for(i = 0;i<1000;i++)
	{
		temp= GX1131_Read_one_Byte(GX1131_DISEQC_RD_INT);		
		if( (temp&0x10) == 0x10)
			break;		
		else if(i==999)
			return FAILURE;		
		GX_Delay_N_ms(5);
	}

	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,diseqc_mode);
	
	return SUCCESS;     
}
/******************************************************************************
*Function: Diseqc1.2, Control the positioner rotation
*				    case STORE_NN, save a satellite.case GOTO_NN, rotate to a satellite                              
*input:
*	   Pos_Ctl:	   Command to control the positioner
*	   Record_byte:  the sequence number of a satellite to save
*Output:  
*	   SUCCESS or FAILURE
*******************************************************************************/                                        
bool GX1131_Pos_Ctl(GX1131_POS_CTRL Pos_Ctl,u8 Record_byte)
{	
	u8		temp,diseqc_mode;
	u8 		buffer[8];
	u32    	i;

	buffer[0] = 0xe0;
	buffer[1] = 0x31;

	diseqc_mode=GX1131_Read_one_Byte( GX1131_DISEQC_MODE);
	temp=GX1131_Read_one_Byte(GX1131_MODULE_RST);	
	temp=temp&0xbf;
	GX1131_Write_one_Byte(GX1131_MODULE_RST, temp);	

	GX1131_Write_one_Byte( GX1131_DISEQC_INS1, buffer[0]);	
  	GX1131_Write_one_Byte( GX1131_DISEQC_INS2, buffer[1]);	
	
	switch(Pos_Ctl)
	{
		case LNB_CTRL_LIMIT_WEST:
		{
			buffer[2] = 0x67;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp & 0xc0) | 0x14;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;
		}			
		case LNB_CTRL_DRIVE_WEST:
		{
			buffer[2] = 0x69;
			buffer[3] = 0x00;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp& 0xc0) | 0x1c;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;
		}		
		case LNB_CTRL_LIMIT_EAST:
		{
			buffer[2] = 0x66;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp & 0xc0) | 0x14;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;
		}			
		case LNB_CTRL_DRIVE_EAST:
		{
			buffer[2] = 0x68;
			buffer[3] = 0x00;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp& 0xc0) | 0x1c;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE,  temp);			
			break;
		}				
		case LNB_CTRL_STOP:			
		{
			buffer[2] = 0x60;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp & 0xc0) | 0x14;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;
		}			
		case LNB_CTRL_LIMIT_OFF:
		{
			buffer[2] = 0x63;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp & 0xc0) | 0x14;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;
		}		
		case LNB_CTRL_STORE_NN:
		{
			buffer[2] = 0x6a;
			buffer[3] = Record_byte;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp& 0xc0) | 0x1c;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;
		}		
		case LNB_CTRL_GOTO_NN:
		{
			buffer[2] = 0x6b;
			buffer[3] = Record_byte;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp& 0xc0) | 0x1c;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;			
		}	

		case LNB_CTRL_GOTO_XX://louhq20100726
		{
			buffer[2] = 0x6e;
			buffer[3] = Record_byte;
			buffer[4] = Record_byte;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);
			GX1131_Write_one_Byte(GX1131_DISEQC_INS5, buffer[4]);
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp& 0xc0) | 0x1c;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;			
		}	

		case LNB_CTRL_SET_POSNS:
		{
			buffer[2] = 0x6f;
			buffer[3] = Record_byte;
			buffer[4] = Record_byte;
			buffer[5] = Record_byte;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			GX1131_Write_one_Byte(GX1131_DISEQC_INS4, buffer[3]);		
			GX1131_Write_one_Byte(GX1131_DISEQC_INS5, buffer[4]);			
			GX1131_Write_one_Byte(GX1131_DISEQC_INS6, buffer[5]);
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp& 0xc0) | 0x1c;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;			
		}			
		
		default: 
		{
			buffer[2] = 0x60;
			GX1131_Write_one_Byte(GX1131_DISEQC_INS3, buffer[2]);			
			temp = GX1131_Read_one_Byte(GX1131_DISEQC_MODE);			
			temp= (temp & 0xc0) | 0x14;
			GX1131_Write_one_Byte( GX1131_DISEQC_MODE, temp);			
			break;
		}	    
	}	
	GX_Delay_N_ms(30);
    
	for(i = 0;i<1000;i++)
	{
		temp= GX1131_Read_one_Byte(GX1131_DISEQC_RD_INT);		
		if( (temp&0x10) == 0x10)
			break;		
		else if(i==999)
			return FAILURE;		
		GX_Delay_N_ms(5);
	}

	GX1131_Write_one_Byte(GX1131_DISEQC_MODE,diseqc_mode);
	
	return SUCCESS;    
}

/******************************************************************************
*Function:   convert a integer to percentage ranging form 0% to 100%  
*Input:
*        value - integer
*        low   - lower limit of input,corresponding to 0%  .if value <= low ,return 0
*        high  - upper limit of input,corresponding to 100%.if value >= high,return 100
*Output:
*        0~100 - percentage
*******************************************************************************/
u8 GX1131_Change2percent(u32 value,u32 low,u32 high)
{
	if (value<=low) return 0;
	if (value>=high) return 100;
	return ((u8)((value-low)*100/(high-low)));
}

/******************************************************************************
*Function:   100LogN calculating function 
*Input:
*	  N: value to cal
*Output:
*        = 100LogN
*******************************************************************************/
f32 GX1131_100Log(s32 iNumber_N)
{
	s32 iLeftMoveCount_M=0;
	s32 iChangeN_Y=0;
	s32 iBuMaY_X=0;
	f32 iReturn_value=0;
	s32 iTemp=0,iResult=0,k=0;

	if(iNumber_N<=0) return 0;
	iChangeN_Y=iNumber_N;
	for (iLeftMoveCount_M=0;iLeftMoveCount_M<16;iLeftMoveCount_M++)
	{
		if ((iChangeN_Y&0x8000)==0x8000)
		{
			break;
		}
		else
		{
			iChangeN_Y=iNumber_N<<iLeftMoveCount_M;
		}
	}
	iBuMaY_X=0x10000-iChangeN_Y;	
	k=iBuMaY_X*10000/65536;
	iTemp = k + (k*k)/20000 + ((k*k/10000)*(k*33/100))/10000 + ((k*k/100000)*(k*k/100000))/400;
	iResult=48165-(iTemp*10000/23025);	
	k=iResult-3010*(iLeftMoveCount_M-1);
	iReturn_value=(f32)(k/100);   
	return iReturn_value;
}

/******************************************************************************
*Function: GX1131_Init_Chip
*Output:
*	     SUCCESS - Configuration success, FAILURE - Configuration falure 
*******************************************************************************/
bool GX1131_Init_Chip(void)
{
	u8			 	temp;		
	u32				Diseqc_Ratio;	

	GX1131_CoolReset_CHIP();		
	GX1131_PLL_Control();
	GX1131_ADC_Init();
	GX1131_Set_TS_Inteface();	
	GX1131_Set_Work_BS_Mode(0);	
	//EQU on
	temp=GX1131_Read_one_Byte(GX1131_EQU_CTL);	
	temp&=0x7f;
	GX1131_Write_one_Byte(GX1131_EQU_CTL,temp);
	//ok out and clk out select 	
	temp = GX1131_Read_one_Byte(GX1131_CLK_OK_SEL);	
	temp=(temp&0x00) |0x75;
	GX1131_Write_one_Byte(GX1131_CLK_OK_SEL,temp);		
	//Sleep off
	GX1131_Set_Sleep(0);
	//SysClk normal
	temp=GX1131_Read_one_Byte(GX1131_CTRL1);	
	temp&=0x7F;
	GX1131_Write_one_Byte(GX1131_CTRL1,temp);	
	//repeater off
	GX1131_Set_Tuner_Repeater_Enable(0);	
	//set AGC parameter
	GX1131_Set_AGC_Parameter();	
	//BBC fb_gain set to 0, 2009.12.28 mike
//	temp=GX1131_Read_one_Byte(GX1131_FB_FSCAN);	
//	temp=temp&0x1f;
//	GX1131_Write_one_Byte(GX1131_FB_FSCAN,(u8)temp);		
	//Set the freq of 22K tone  
	Diseqc_Ratio=((((GX1131_FSAMPLE_VALUE*10)/88)+5)/10)&0x07ff;	
	GX1131_Write_one_Byte(GX1131_DISEQC_RATIO_L, (u8)(Diseqc_Ratio&0xff));	
	temp=GX1131_Read_one_Byte(GX1131_DISEQC_RATIO_H);	
	temp=(temp&0xF8)|((u8)(((Diseqc_Ratio&0x0700)>>8)));	
	GX1131_Write_one_Byte(GX1131_DISEQC_RATIO_H,temp);	
	temp=GX1131_Read_one_Byte(GX1131_MODULE_RST);	
	temp&=0xbf;
	GX1131_Write_one_Byte(GX1131_MODULE_RST, temp);	
	// snr detection setting
	temp=GX1131_Read_one_Byte(GX1131_RSD_ERR_STATIC_CTRL);	
	temp=(temp&0x80)|0x05;
	GX1131_Write_one_Byte(GX1131_RSD_ERR_STATIC_CTRL,temp);	
	temp=GX1131_Read_one_Byte(GX1131_PKT_LEN_SEL);
	temp=(temp&0xfc)|0x02;
	GX1131_Write_one_Byte(GX1131_PKT_LEN_SEL,temp);	
	//Blindscan setting
	temp=GX1131_Read_one_Byte(GX1131_BCS_OUT_ADDR);	
	temp=(temp&0x1f)|0x60;
	GX1131_Write_one_Byte(GX1131_BCS_OUT_ADDR,temp);		
	GX1131_Write_one_Byte(GX1131_MIN_FS, 0x64);
	//FFt lmt setting
	temp=GX1131_Read_one_Byte(GX1131_FFE_CTRL2);	
	temp=(temp&0xc0)|0x20;
	GX1131_Write_one_Byte(GX1131_FFE_CTRL2,temp);	

	temp=GX1131_Read_one_Byte(GX1131_FFE_CTRL1);	
	temp=(temp&0xf8)|0x07;
	GX1131_Write_one_Byte(GX1131_FFE_CTRL1,temp);	

	temp=GX1131_Read_one_Byte(GX1131_AGC_AMP);	
	temp=(temp&0x0f)|0x90;
	GX1131_Write_one_Byte(GX1131_AGC_AMP,temp);

	temp=GX1131_Read_one_Byte(GX1131_BW_CTRL2);	
	temp=(temp&0x0f)|0xd0;
	GX1131_Write_one_Byte(GX1131_BW_CTRL2,temp);

	temp=GX1131_Read_one_Byte(GX1131_EQU_SPEED);	
	temp=(temp&0x00)|0x10;
	GX1131_Write_one_Byte(GX1131_EQU_SPEED,temp);
	temp=GX1131_Read_one_Byte(GX1131_FFE_CTRL1);	
	temp=(temp&0xf8)|0x07;
	GX1131_Write_one_Byte(GX1131_FFE_CTRL1,temp);	

	temp=GX1131_Read_one_Byte(GX1131_FFE_CTRL2);	
	temp=(temp&0xc0)|0x20;
	GX1131_Write_one_Byte(GX1131_FFE_CTRL2,temp);

	temp=GX1131_Read_one_Byte(GX1131_I2C_REPEAT);	
	if(IIC_REPEAT_SPEED==1)
		temp=(temp&0xe0)|0x02;
	else if(IIC_REPEAT_SPEED==2)
		temp=(temp&0xe0)|0x00;
		else
			temp=(temp&0xe0)|0x06;
	GX1131_Write_one_Byte(GX1131_I2C_REPEAT,temp);
 

	
	/*
	Write some other registers if necessary

	*/

	return SUCCESS;
}

/******************************************************************************
*Function: Read ALL OK
*Input:
*	   No input
*Output:
*        SUCCESS - all ok, FAILURE - not all ok 
*******************************************************************************/
GX1131_LOCK_STATUS GX1131_Read_Lock_Status(void)
{
	u8 temp=FAILURE;
	GX1131_LOCK_STATUS lock_status = UNLOCKED;		

	temp=GX1131_Read_one_Byte(GX1131_ALL_OK);	
    DBG("%s[%8d]: GX1131_ALL_OK value: %d \n",  __FUNCTION__, __LINE__, temp);	
	if ((temp&0x75)==0x75)			lock_status=FEC_LOCKED;			
	else if((temp&0x35)==0x35)		lock_status=EQU_LOCKED;	
	else if((temp&0x15)==0x15 )		lock_status=CRL_LOCKED;
	else if((temp&0x05)==0x05)		lock_status=TIM_LOCKED;
	else if((temp&0x03)==0x03)		lock_status=BCS_LOCKED;
	else if((temp&0x01)==0x01)		lock_status=AGC_LOCKED;
	else								lock_status=UNLOCKED;	

	return lock_status;
}

/******************************************************************************
*Function : get the course signal SNR
*Input:
*		   
*Output:
*        	   SNR value after timing lock
*******************************************************************************/
u32 GX1131_Get_100SNR_Tim(void)
{       
	u8	 temp1,temp2;
	u32  snr100;
	u32  n_pow;

	temp1=GX1131_Read_one_Byte(GX1131_SNR_DET_N_TIM_L);
	temp2=GX1131_Read_one_Byte(GX1131_SNR_DET_N_TIM_H);	
	if(GX1131_Read_Lock_Status()>UNLOCKED)		
		{
			n_pow=((temp2 &0x0f)<<8) | temp1;
    DBG("%s[%8d]: n_pow : %d \n",  __FUNCTION__, __LINE__, n_pow);			
			if(n_pow!=0)
				{
					snr100=(u32)(10*GX1131_100Log((2048/n_pow)+1/2));
//					snr100=(u32)(1000*log10((f32)(2048.0/n_pow)));
					if(snr100>4000)
						snr100=4000;
				}
			else	
				snr100=0;
		}
	else snr100=0;
	
	return snr100;		
}

/******************************************************************************
*Function : get the fine signal SNR
*Input:
*		   
*Output:
*        	   fine SNR value after fec lock
*******************************************************************************/
u32  GX1131_Get_100SNR(void)
{       
	GX1131_STATE temp1,temp2;
	u32  snr100;
	u32  n_pow;

	temp1=GX1131_Read_one_Byte(GX1131_SNR_DET_N_FINE_L);
	temp2=GX1131_Read_one_Byte(GX1131_SNR_DET_N_FINE_H);	
	
	if(GX1131_Read_Lock_Status()>=CRL_LOCKED)	
	{
		n_pow=((temp2 &0x0f)<<8) | temp1;
    DBG("%s[%8d]: n_pow : %d \n",  __FUNCTION__, __LINE__, n_pow);			
		if(n_pow!=0)
		{
			snr100=(u32)(10*GX1131_100Log((2048/n_pow)+1/2));
//			snr100=(u32)(1000*log10((f32)(2048.0/n_pow)));
			if(snr100>4000)
				snr100=4000;
		}
		else	
			snr100=0;
	}
	else		snr100=0;
			
    DBG("%s[%8d]: snr100 : %d \n",  __FUNCTION__, __LINE__, snr100);			
	return snr100;
}

/******************************************************************************
*Function: Get Signal Strength in percent according agc value
*Input:
*	   old_percent value
*Output: 
*	   Updated signal strength in percent
*******************************************************************************/
u8 GX1131_Get_Signal_Strength(void)
{
       u8	agc_h,agc_l;
	u32  agc_out;
	u8	percent=0;

	agc_l =GX1131_Read_one_Byte(GX1131_AGC_CTRL_OUT_L);	
	agc_h=GX1131_Read_one_Byte(GX1131_AGC_CTRL_OUT_H);
	
	agc_out=(((agc_h&0xf0)>>4)<<8)|agc_l;
	//if(GX1131_AGC_POLARITY==1)
		percent=(u8)((409600-100*agc_out)/4096+1/2);
	//else if(GX1131_AGC_POLARITY==0)
		//percent=(u8)((100*agc_out)/4096+0.5);
	percent=percent*8/7;
			
	if(percent>100)
	    percent=100;
	else if(percent<=0)		
	    percent=0;	
		
	return percent;
}

/******************************************************************************
*Function: Get Signal Quality
*Input:
*	   old signal quality percent
*Output: 
*	   Updated signal quality percent
*******************************************************************************/
u8 GX1131_Get_Signal_Quality(void)
{
	u32  Snr100=0;
	u8	percent=0;	
	
	if(GX1131_Read_Lock_Status()>=CRL_LOCKED)
		Snr100=GX1131_Get_100SNR();	
	else if(GX1131_Read_Lock_Status()>=AGC_LOCKED)
		Snr100=GX1131_Get_100SNR_Tim();
	else Snr100=0;
	
	percent=(u8)(Snr100/30);
	
	if(percent<30)	percent = (u8)(percent *2/3);
	else if(percent>=30)	percent = (percent -30)/2+45;
			
	if(percent>100)	percent=100;
	if(percent<=0)		percent=0;	
			
	return percent;
}

/******************************************************************************
*Function: Get Error Rate value 
*Input:		
*		*E_param: for get the exponent of E
*Output:
*          	return value = 0 and E_param = 0:		Read GX1131 Error
*          	other:								Error Rate Value
*	       Example:	if  return value = 456 and  E_param = 3 ,
*		then here means the Error Rate value is : 4.56 E-3
*******************************************************************************/
u32 GX1131_Get_ErrorRate(u8 *E_param)						//DINGFB 20100623
{
	u8 	flag = FAILURE;
	u8 	i;
	u8 	temp;
	u8	e_value = 0;
	u32 	return_value = 0;
	u32 	ErrorRate=0;
	u8	Read_Value[4];
	u32 	Inf_err_bits=0;
	u32 	divied =2;
	u32 	count_p;

	*E_param = 0;
	
	if (GX1131_Read_Lock_Status()>=CRL_LOCKED)
		{
			temp=GX1131_Read_one_Byte(GX1131_MODU_MODE);			
			temp=(temp&0xc0)>>6;

			switch(temp)
				{
					case 0: 
						{
							for (i=0;i<4;i++)								
								{
									Read_Value[i] = GX1131_Read_one_Byte(GX1131_ERR_OUT_0 + i);
								}
							Inf_err_bits = ((u32)Read_Value[0])+ (((u32)Read_Value[1])<<8) + (((u32)Read_Value[2])<<16)+(((u32)(Read_Value[3]&0x03))<<24);							
							flag=GX1131_Read_one_Byte(GX1131_RSD_ERR_STATIC_CTRL);														
							flag=(flag&0x0e)>>1;							
							count_p=(u32)(0x01<<(2*flag+5));
							divied=204*8*count_p;	
							break;
						}
					case 1:
						{
							for (i=0;i<4;i++)	
							{
								Read_Value[i] = GX1131_Read_one_Byte(GX1131_ERR_OUT_0 + i);											
							}
							Inf_err_bits = ((u32)Read_Value[0])+ (((u32)Read_Value[1])<<8) + (((u32)Read_Value[2])<<16)+(((u32)(Read_Value[3]&0x03))<<24);
							flag=GX1131_Read_one_Byte(GX1131_RSD_ERR_STATIC_CTRL);
							flag=(flag&0x0e)>>1;
							count_p=(u32)(0x01<<(2*flag+5));
							divied=146*8*count_p;	
							break;
						}
					case 2:
					case 3:
						{
							for (i=0;i<2;i++)	
							{
								Read_Value[i]  = GX1131_Read_one_Byte(GX1131_CRC_ERR_SUM_L + i);													
							}
							Inf_err_bits = ((u32)Read_Value[0])+ (((u32)Read_Value[1])<<8);												
							if(((GX1131_Read_one_Byte(GX1131_MATYPE1)&0xc0)>>6)==3)
								{
									flag = GX1131_Read_one_Byte(GX1131_PKT_LEN_SEL);									
									if((flag&0x03)==0)				divied=2048;
									else if((flag&0x03)==1)		divied=8192;
									else if((flag&0x03)==2)		divied=32768;
									else if((flag&0x03)==3)		divied=131072;
								}
							else
								{
									flag = GX1131_Read_one_Byte(GX1131_PKT_LEN_SEL);								
									if((flag&0x03)==0)				divied=64;
									else if((flag&0x03)==1)		divied=256;
									else if((flag&0x03)==2)		divied=1024;
									else if((flag&0x03)==3)		divied=4096;
								}
							break;
						}
				}
		}
	else		return 0;		
	if((Inf_err_bits==0)||(divied==0)) return 0;	
	
	while(1)
	{
		ErrorRate= 1000*Inf_err_bits/divied;
		if (ErrorRate>=1000)
		{
			return_value = (Inf_err_bits*100)/divied;
			break;
		}
		else
		{
			e_value ++;
			Inf_err_bits *=10;
		}
	}	
	*E_param = e_value;
	return return_value;
}


GX1131_STATE GX1131_set_tuner(u32 RFfreq,u32 Symbol_Rate)

{
	GX1131_STATE nReturnValue;
    #if TUNER_SHARP7306 
	    nReturnValue = GX_Set_Sharp7306(RFfreq,Symbol_Rate);
           return nReturnValue;
    #endif

     #if TUNER_RDA5812 
           nReturnValue = GX_Set_RDA5812(RFfreq,Symbol_Rate);
           return nReturnValue;
    #endif

     #if TUNER_ZL10037 
           nReturnValue = GX_Set_ZL10037(RFfreq,Symbol_Rate);
           return nReturnValue;
    #endif

     #if TUNER_AV2011
           nReturnValue = GX_Set_AV2011(RFfreq,Symbol_Rate);
           return nReturnValue;
    #endif

     #if TUNER_LW37
           nReturnValue = GX_Set_LW37(RFfreq,Symbol_Rate);
           return nReturnValue;
    #endif

	#if TUNER_WZ5001
           nReturnValue = GX_Set_WZ5001(RFfreq,Symbol_Rate);
           return nReturnValue;
    #endif
	
     #if TUNER_STV6110A 
	    nReturnValue = GX_Set_STV6110A(RFfreq,Symbol_Rate);
           return nReturnValue;
    #endif
}
/******************************************************************************
*Function: Lock TP 
*Input:		
*		RF_Freq:	       the center frequence of specified TP at L band, unit: MHz
*		Symbol_Rate: the symbol rate of specified TP, unit: KBaud
*		Polar:		polarity of LNB, control 13V/18V
*		Tone_on:		22K on/off control, select the high/low osc
*Output:
*          	SUCESS: locked,   FAILURE: can't locked
*******************************************************************************/
bool GX1131_lock_TP(GX1131_TP tp,  TunerDriver tunerDriver)
{	
	u8 temp,temp1;
	s32 center_shift_KHz;
		
	temp=GX1131_Read_one_Byte(GX1131_AUTO_RST);		
	temp|=0x01;
	GX1131_Write_one_Byte(GX1131_AUTO_RST,temp);		
	GX1131_Set_Work_BS_Mode(0);							
	GX1131_Set_Polar(tp.V_polar);		
	GX1131_22K_sw(tp._22K_on);			
	GX1131_SetSymbolRate(tp.SymRate_KSps);			
	temp=GX1131_Read_one_Byte(GX1131_FC_OFFSET_H);
	temp1=GX1131_Read_one_Byte(GX1131_FC_OFFSET_L);
	if(((temp<<8)+temp1)<=32767)		center_shift_KHz=(temp<<8)+temp1;
	else if(((temp<<8)+temp1)>32767)	center_shift_KHz=((temp<<8)+temp1) - 65536;
	if(gIQ_swap==1)	center_shift_KHz=0-center_shift_KHz;
	//print("[Lock_tp:]center_shift_KHz=%dKHz",center_shift_KHz);
	GX1131_Set_Tuner_Repeater_Enable(1);
	tunerDriver (((s32)tp.Freq_KHz+center_shift_KHz+500)/1000,tp.SymRate_KSps+abs(center_shift_KHz));
	GX1131_Set_Tuner_Repeater_Enable(0);		
	GX1131_HotReset_CHIP();		
	GX_Delay_N_ms(50);		
	
	return SUCCESS;
	
}

/******************************************************************************
*Function: Lock TP with BS on
*Input:		
*		RF_Freq:	       the center frequence of specified TP at L band, unit: MHz
*		Symbol_Rate: the symbol rate of specified TP, unit: KBaud
*		Polar:		polarity of LNB, control 13V/18V
*		Tone_on:		22K on/off control, select the high/low osc
*Output:
*          	SUCESS: locked,   FAILURE: can't locked
*******************************************************************************/
bool GX1131_lock_TP_BS(GX1131_TP tp,  TunerDriver tunerDriver)
{	
	u8 temp,temp1;
	s32 center_shift_KHz;

	temp=GX1131_Read_one_Byte(GX1131_AUTO_RST);		
	temp|=0x01;
	GX1131_Write_one_Byte(GX1131_AUTO_RST,temp);		
	GX1131_Set_Work_BS_Mode(1);			
	GX1131_Set_Polar(tp.V_polar);			
	GX1131_22K_sw(tp._22K_on);
	temp=GX1131_Read_one_Byte(GX1131_BCS_OUT_ADDR);		
	temp=temp&0xe0;
	GX1131_Write_one_Byte(GX1131_BCS_OUT_ADDR,temp);		

	GX1131_SetSymbolRate(tp.SymRate_KSps);		
	temp=GX1131_Read_one_Byte(GX1131_FC_OFFSET_H);
	temp1=GX1131_Read_one_Byte(GX1131_FC_OFFSET_L);	
	if(((temp<<8)+temp1)<=32767)		center_shift_KHz=(temp<<8)+temp1;
	if(((temp<<8)+temp1)>32767)	center_shift_KHz=((temp<<8)+temp1) - 65536;
	if(gIQ_swap==1)	center_shift_KHz=0-center_shift_KHz;
//	print("[Lock tp bs:]center_shift_KHz=%dKHz",center_shift_KHz);
	GX1131_Set_Tuner_Repeater_Enable(1);
	tunerDriver(((s32)tp.Freq_KHz+center_shift_KHz+500)/1000,tp.SymRate_KSps+abs(center_shift_KHz));		
	GX1131_Set_Tuner_Repeater_Enable(0);		
	GX1131_HotReset_CHIP();		
	GX_Delay_N_ms(50);	

	return SUCCESS;
}
/******************************************************************************
*Function:GX1131_Search_Signal
*Input:		
*		RF_Freq:	       the center frequence of specified TP at L band, unit: MHz
*		Symbol_Rate: the symbol rate of specified TP, unit: KBaud
*		Polar:		polarity of LNB, control 13V/18V
*		Tone_on:		22K on/off control, select the high/low osc
*Output:
*          	SUCESS: locked,   FAILURE: can't locked
*******************************************************************************/
bool GX1131_Search_Signal(GX1131_TP tp,  TunerDriver tunerDriver)
{
	//GX1131_TP_INFO 	tp_info;
	u8	count = 0,locked_flag = 0;
	static u8 sym_flag=0;
	s32	temp,temp1,temp2,Fs_Bs,Fc_offset_Bs,temp_Fc_offset,fc_offset_fine_K,fc_offset;
	u32	j;

	if((tp.Freq_KHz<900000) ||(tp.Freq_KHz>2200000)|| (tp.SymRate_KSps<800)|| (tp.SymRate_KSps>50000))	
	{     
		tp.Freq_KHz=10000;
		tp.SymRate_KSps=50;
	       GX1131_lock_TP(tp, tunerDriver);
		return TP_UNLOCK;	
	}
	else
	{
	GX1131_Set_Ts_Out_Disable(1);
	if(tp.SymRate_KSps<1600)
		{
			//Set PLL1_B
			temp=GX1131_Read_one_Byte(GX1131_PLL1_CTRL_B);	
			temp=(temp&0x00)|40;
			GX1131_Write_one_Byte(GX1131_PLL1_CTRL_B,temp);
			//Set PLL2_B
			temp=GX1131_Read_one_Byte(GX1131_PLL2_CTRL_B);	
			temp=(temp&0x00)|50;	
			GX1131_Write_one_Byte(GX1131_PLL2_CTRL_B,temp);	

			sym_flag=1;
			//print("sym_flag=%d", sym_flag);
	       }
	else
		{	
			if(sym_flag==1)
				{
					//Set PLL1_B
					temp=GX1131_Read_one_Byte(GX1131_PLL1_CTRL_B);	
					temp=(temp&0x00)|(GX1131_PLL1_B_VALUE);
					GX1131_Write_one_Byte(GX1131_PLL1_CTRL_B,temp);
					//Set PLL2_B
					temp=GX1131_Read_one_Byte(GX1131_PLL2_CTRL_B);	
					temp=(temp&0x00)|(GX1131_PLL2_B_VALUE);	
					GX1131_Write_one_Byte(GX1131_PLL2_CTRL_B,temp);

					sym_flag=0;
					//print("sym_flag=%d", sym_flag);
				}
		}
	GX1131_lock_TP_BS(tp,tunerDriver);
	count=100;
	/////louhq20100826
	for(j=0;j<count;j++)
		{					
			switch(GX1131_Read_Lock_Status())
				{
					case FEC_LOCKED:
						{
							locked_flag=1;
							break;
						}
					case EQU_LOCKED:
						{
							if(j>=80)
							j+=5;
							break;
						}
					case CRL_LOCKED:
						{
							if(j>=60)
							j+=5;//louhq20110503
							break;
						}
					case TIM_LOCKED:
						{
							if(j>=50)
								j+=10;//louhq20110503
							break;
						}
					case BCS_LOCKED:
						{
							if(j>=50)
								j+=10;//louhq20110503
							break;
						}
					case AGC_LOCKED:
						{
							if(j>=50)
								j+=15;
							break;
						}
					case UNLOCKED:
						{
							if(j>=50)
								j+=20;
							break;
						}
				}
		if(locked_flag==1)
			break;
		if(tp.SymRate_KSps<=5000)	
			GX_Delay_N_ms(50);
		else	if((tp.SymRate_KSps>5000)&&(tp.SymRate_KSps<=10000))	
			GX_Delay_N_ms(40);
			else			
				GX_Delay_N_ms(30);
		}
	if(locked_flag)
		{	
			if(tp.SymRate_KSps<1600)
				{
					GX1131_Set_Ts_Out_Disable(0);
					return TP_LOCK;
				}
			else
				{
					temp1=(GX1131_Read_one_Byte(GX1131_BCS_OUT_ADDR)&0xe0);	
					GX1131_Write_one_Byte(GX1131_BCS_OUT_ADDR,temp1);				
					temp1=GX1131_Read_one_Byte(GX1131_BCS_FC_OFFSET_H);	
					temp2=GX1131_Read_one_Byte(GX1131_BCS_FC_OFFSET_L);	
					if(((temp1<<8)+temp2)<=32767)		Fc_offset_Bs = ((temp1<<8)+temp2);
					if(((temp1<<8)+temp2)>32767)		Fc_offset_Bs = ((temp1<<8)+temp2)-65536;
					if(gIQ_swap==0)	Fc_offset_Bs=0 - Fc_offset_Bs;
					temp1=GX1131_Read_one_Byte(GX1131_BCS_FS_H);				
					temp2=GX1131_Read_one_Byte(GX1131_BCS_FS_L);				
					Fs_Bs = (temp1<<8)+temp2;

					temp1=GX1131_Read_one_Byte(GX1131_FREQ_BACK_L);
					temp2=GX1131_Read_one_Byte(GX1131_FREQ_BACK_H);
					if(((temp2<<8)+temp1)<=32767)	temp_Fc_offset=(temp2<<8)+temp1;
					if(((temp2<<8)+temp1)>32767)		temp_Fc_offset=((temp2<<8)+temp1)-65536;
					if(gIQ_swap==0)	temp_Fc_offset=0-temp_Fc_offset;
					fc_offset_fine_K=(temp_Fc_offset*(s32)Fs_Bs)/65536;	
					fc_offset=Fc_offset_Bs+fc_offset_fine_K;
					
					//print("fc_offset=%d", fc_offset);
					
					if(fc_offset<0)
						fc_offset=0-fc_offset;
					if(Fs_Bs>=5000)
					{
						if(fc_offset>6000)
							return TP_BAD;
						else
						{
							temp=tp.SymRate_KSps-Fs_Bs;						
							if(temp<0)
							{
								temp=Fs_Bs-tp.SymRate_KSps;
							}
							if(temp>100)
								return TP_BAD;
							else 
								{
									GX1131_Set_Ts_Out_Disable(0);
									return TP_LOCK;
								}
						}
					}
					else 
						{
							if(fc_offset>5000)
								return TP_BAD;
							else
							{
								temp=tp.SymRate_KSps-Fs_Bs;						
								if(temp<0)
								{
									temp=Fs_Bs-tp.SymRate_KSps;
								}
								if(temp>100)
									return TP_BAD;
								else 
									{
										GX1131_Set_Ts_Out_Disable(0);
										return TP_LOCK;
									}
							}
						}
					}
				}
			else	
				{
					GX1131_Set_Ts_Out_Disable(0);
					 return TP_UNLOCK;
				}
	}
}

/******************************************************************************
*Function: Get the channel info after chip locking 
*Input:		
*		
*Output:
*          	
*******************************************************************************/
GX1131_FINE_OFFSET GX1131_Get_Fine_Offset(void)
{
	u8 temp, temp1,temp2;	
	u32 sym_rate;
	s32 temp_Fc_offset,temp_Fs_offset;		
	GX1131_FINE_OFFSET fine_offset;	
	
	if(GX1131_Read_Lock_Status()>=CRL_LOCKED)
		{
			temp1=GX1131_Read_one_Byte(GX1131_SYMBOL_L);
			temp2=GX1131_Read_one_Byte(GX1131_SYMBOL_H);
			sym_rate=(temp2<<8)+temp1;			
//			temp=GX1131_Read_one_Byte(GX1131_FB_FSCAN);//mike 20091228
//			temp1=(GX1131_Read_one_Byte(GX1131_MODU_MODE)&0xc0)>>6;					
//			if(temp1<2)
//				{
//					temp=(temp&0x1f)|0x20;
//					GX1131_Write_one_Byte(GX1131_FB_FSCAN, (u8)temp);
//				}
			temp1=GX1131_Read_one_Byte(GX1131_FREQ_BACK_L);
			temp2=GX1131_Read_one_Byte(GX1131_FREQ_BACK_H);
			if(((temp2<<8)+temp1)<=32767)	temp_Fc_offset=(temp2<<8)+temp1;
			if(((temp2<<8)+temp1)>32767)		temp_Fc_offset=((temp2<<8)+temp1)-65536;
			if(gIQ_swap==0)	temp_Fc_offset=0-temp_Fc_offset;
			fine_offset.fc_offset_fine_K=(temp_Fc_offset*(s32)sym_rate)/65536;			
//			temp=GX1131_Read_one_Byte(GX1131_FB_FSCAN);			
//			temp=temp&0x1f;
//			GX1131_Write_one_Byte(GX1131_FB_FSCAN,(u8)temp);			

			temp=GX1131_Read_one_Byte(GX1131_INTG_OUT);	
			if(temp<=127)
				temp_Fs_offset=temp;
			else if(temp>127)
				temp_Fs_offset=temp-256;
			if(gIQ_swap==0) temp_Fs_offset= 0- temp_Fs_offset;
			
			if(sym_rate<=2820) 	fine_offset.fs_offset_fine_Sps=(temp_Fs_offset*91800000)/33554432;
			else if(sym_rate<=5650)	fine_offset.fs_offset_fine_Sps=(temp_Fs_offset*91800000)/16777216;
			else if(sym_rate<=11300)	fine_offset.fs_offset_fine_Sps=(temp_Fs_offset*91800000)/8388608;
			else 	fine_offset.fs_offset_fine_Sps=(temp_Fs_offset*91800000)/4194304;				
		}
	else 
		{
			fine_offset.fc_offset_fine_K=0;
			fine_offset.fs_offset_fine_Sps=0;
		}

	return fine_offset;
}

/******************************************************************************
*Function: Get the TP info after chip locking 
*Input:		
*		
*Output:
*          	
*******************************************************************************/
GX1131_TP_INFO GX1131_Get_TP_Info(u32 RF_Freq_K,u32 Symbol_Rate_K)
{
	GX1131_TP_INFO 	tp_info;
	u8 	temp,temp1,temp2,temp3;
	u16		if_frequency;		
	u16		local_frequency = 0;
	s32 rf_center_offset_KHz;
	s32 offset_bcs;
	bool bcs_on=0;
	u16 TempLocalFre;
	GX1131_FINE_OFFSET fine_offset;
	//GX1131_BCS_TYPE *bcs_type;
	tp_info.V_polor=((GX1131_Read_one_Byte(GX1131_DISEQC_MODE)&0x40)==0x40)?1:0;
	if(POL_SWITCH) 	tp_info.V_polor=!tp_info.V_polor;
	tp_info._22K_on=((GX1131_Read_one_Byte(GX1131_DISEQC_MODE)&0x07)==0x01)?1:0;
	
	if (GX1131_Read_Lock_Status()>=EQU_LOCKED)
		{
			temp=GX1131_Read_one_Byte(GX1131_MODU_MODE);
			temp1=(temp&0xc0)>>6;			
			switch(temp1)
				{
					case 0:
						{
							tp_info.modu_mode=DVBS;
							
							temp2=temp&0x07;
							switch(temp2)
								{
									case 1: 
										{
											tp_info.code_rate_s=CR12;
											break;
											}
									case 2: 
										{
											tp_info.code_rate_s=CR23;
											break;
										}
									case 3:
										{
											tp_info.code_rate_s=CR34;
											break;
										}
									case 5:
										{
											tp_info.code_rate_s=CR56;
											break;
										}
									case 7: 
										{
											tp_info.code_rate_s=CR78;
											break;
										}
									default: 
										{
											tp_info.code_rate_s=CR34;
											break;
										}
							 	}
							break;
						}
					case 1:
						{
							tp_info.modu_mode=DIRECTV;

							temp2=temp&0x07;
							switch(temp2)
								{
									case 1: 
										{
											tp_info.code_rate_s=CR12;
											break;
										}
									case 2: 
										{
											tp_info.code_rate_s=CR23;
											break;
										}
									case 6: 
										{
											tp_info.code_rate_s=CR67;
											break;
										}
									default: 
										{
											tp_info.code_rate_s=CR23;
											break;
										}
								}
							break;
						}
					case 2:
					case 3:
						{
							tp_info.modu_mode=DVBS2;

							temp3=GX1131_Read_one_Byte(GX1131_S2_MODE_CODE);
							temp3&=0x1f;
							switch(temp3)
								{
									case 4: 		
										{
											tp_info.code_rate_s2=QPSK12;
											break;
										}
									case 5: 		
										{
											tp_info.code_rate_s2=QPSK35;
											break;
										}
									case 6:
										{
											tp_info.code_rate_s2=QPSK23;
											break;
										}
									case 7: 		
										{
											tp_info.code_rate_s2=QPSK34;
											break;
										}
									case 8: 		
										{
											tp_info.code_rate_s2=QPSK45;
											break;
										}
									case 9: 		
										{
											tp_info.code_rate_s2=QPSK56;
											break;
										}
									case 10: 	
										{
											tp_info.code_rate_s2=QPSK89;
											break;
										}
									case 11: 	
										{
											tp_info.code_rate_s2=QPSK910;
											break;
										}
									case 12: 	
										{
											tp_info.code_rate_s2=_8PSK35;
											break;
										}
									case 13: 	
										{
											tp_info.code_rate_s2=_8PSK23;
											break;
										}
									case 14: 	
										{
											tp_info.code_rate_s2=_8PSK34;
											break;
										}
									case 15: 	
										{
											tp_info.code_rate_s2=_8PSK56;
											break;
										}
									case 16: 	
										{
											tp_info.code_rate_s2=_8PSK89;
											break;
										}
									case 17: 	
										{
											tp_info.code_rate_s2=_8PSK910;							
											break;
										}
									default: 		
										{
											tp_info.code_rate_s2=_8PSK23;
											break;
										}
										
								}
							break;
						}
					default:
						{
							tp_info.modu_mode=DVBS2;
							tp_info.code_rate_s=CR23;
							tp_info.code_rate_s2=_8PSK23;
							break;
						}
				}			
			}
	else
		{
			tp_info.modu_mode=UNKNOWN;
			tp_info.code_rate_s=CRXX;
			tp_info.code_rate_s2=CRXXX;
		}
	
	temp1=GX1131_Read_one_Byte(GX1131_FC_OFFSET_L);
	temp2=GX1131_Read_one_Byte(GX1131_FC_OFFSET_H);
	if(((temp2<<8)+temp1)<=32767)		rf_center_offset_KHz=(temp2<<8)+temp1;
	else if(((temp2<<8)+temp1)>32767) 	rf_center_offset_KHz=((temp2<<8)+temp1) - 65536;	
	if(gIQ_swap==1)	rf_center_offset_KHz=0-rf_center_offset_KHz;
	temp1=GX1131_Read_one_Byte(GX1131_BCS_RST);
	if((temp1&0x80)==0x00) bcs_on=1;
	fine_offset=GX1131_Get_Fine_Offset();	
	if((GX1131_Read_Lock_Status()>=EQU_LOCKED)&&(bcs_on==1))
		{
			temp1=GX1131_Read_one_Byte(GX1131_BCS_FC_OFFSET_H);			
			temp2=GX1131_Read_one_Byte(GX1131_BCS_FC_OFFSET_L);			
			if(((temp1<<8)+temp2)<=32767)	offset_bcs=(temp1<<8)+temp2;
			if(((temp1<<8)+temp2)>32767)		offset_bcs = ((temp1<<8)+temp2) - 65536;	
			if(gIQ_swap==0)	offset_bcs=0-offset_bcs;
			tp_info.Freq_true_KHz=(s32)RF_Freq_K+rf_center_offset_KHz+offset_bcs+fine_offset.fc_offset_fine_K;				
			temp1=GX1131_Read_one_Byte(GX1131_BCS_FS_H);			
			temp2=GX1131_Read_one_Byte(GX1131_BCS_FS_L);			
			tp_info.SymolRate_true_Sps =(s32)((temp1<<8)+temp2)*1000+fine_offset.fs_offset_fine_Sps;	
		}
	else if((GX1131_Read_Lock_Status()>=EQU_LOCKED)&&(bcs_on==0))
		{
			tp_info.Freq_true_KHz=(s32)RF_Freq_K+fine_offset.fc_offset_fine_K;
 			tp_info.SymolRate_true_Sps=(s32)Symbol_Rate_K*1000+fine_offset.fs_offset_fine_Sps;
		}
	else
		{
			tp_info.Freq_true_KHz=RF_Freq_K;
 			tp_info.SymolRate_true_Sps=Symbol_Rate_K*1000;
		}
	/*///////////////////////////////////////////////////////////////////////////
	printf("$$$$$$$$$$$$$$$$$$$$tp_info.Freq_true_KHz  =  %d\n",tp_info.Freq_true_KHz);
		if_frequency = (u16)((tp_info.Freq_true_KHz*1000+500)/1000);
		//if_frequency = (u16)tp_info.Freq_true_KHz*1000;
	printf("$$$$$$$$$$$$$$$$$$$$if_frequency  =  %d\n",if_frequency);
	if(sg_BSType.Ku_LNB==0)
		{
			//C
			if(sg_BSType.dual_local_osc_LNB==1)
				{
					//双本振
					if(sg_BSType.LocalFre1 > sg_BSType.LocalFre2)
						{//高低本振互换，确保 LocalFre1 为低本振
							TempLocalFre = sg_BSType.LocalFre1;
							sg_BSType.LocalFre1 = sg_BSType.LocalFre2;
							sg_BSType.LocalFre2 = TempLocalFre;
						}
					if(tp_info.V_polor==0)
						{
							//水平
							local_frequency=sg_BSType.LocalFre1;
						}
					else if(tp_info.V_polor==1)
						{
							//垂直
							local_frequency=sg_BSType.LocalFre2;
						}
					}
			else if(sg_BSType.dual_local_osc_LNB==0)
				{
					local_frequency=sg_BSType.LocalFre1;
				}
				
			if(local_frequency > if_frequency)
        				tp_info.Freq_true_KHz = local_frequency - if_frequency;
        		else
        		   		tp_info.Freq_true_KHz = local_frequency + if_frequency;
		}
	else if(sg_BSType.Ku_LNB==1)
		{
			//Ku
			if(sg_BSType.dual_local_osc_LNB==1)
				{
					//双本振
					if(sg_BSType.LocalFre1 > sg_BSType.LocalFre2)
						{//高低本振互换，确保 LocalFre1 为低本振
							TempLocalFre = sg_BSType.LocalFre1;
							sg_BSType.LocalFre1 = sg_BSType.LocalFre2;
							sg_BSType.LocalFre2 = TempLocalFre;
						}
					if(tp_info._22K_on==0)
						{
							//22k_off
							local_frequency=sg_BSType.LocalFre1;
						}
					else if(tp_info._22K_on==1)
						{
							//22k_on
							local_frequency=sg_BSType.LocalFre2;
						}
					}
			else if(sg_BSType.dual_local_osc_LNB==0)
				{
					local_frequency=sg_BSType.LocalFre1;
				}
			//tp_info.Freq_true_KHz = local_frequency + if_frequency;
			tp_info.Freq_true_KHz =if_frequency;
		}    		    		
	printf("$$$$$$$$$$$$$$$$$$$$bcs_type.Ku_LNB  =  %d\n",sg_BSType.Ku_LNB);
	
	printf("$$$$$$$$$$$$$$$$$$$$local_frequency  =  %d\n",local_frequency);
	///////////////////////////////////////////////////////////////////////////*/
	return tp_info;
}


/******************************************************************************
*Function: Perform one blind scan within a Lpf_BW window at Fcenter
*Input:
*	   Fcenter: the center freq to blind scan, unit:MHz
*	   Lpf_BW_window: the size of BW windowto blind scan, unit:KHz
*	   TP_Num_Previous_window: the TPs number of previous blind scan window
*Output:
*         the number of TPs in this window blind scan
*	    or FAILURE
*******************************************************************************/
u32 GX1131_BS_one_Window(u32 Fcenter,u32 Lpf_BW_window,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver)
{

	u32 	TP_Num=0;
	u8 	i;
	u8	temp,temp1;	
	s32	Fc_offset_Bs;
	u32	Fs_Bs;

	temp=GX1131_Read_one_Byte(GX1131_AUTO_RST)&0xfe;			
	GX1131_Write_one_Byte(GX1131_AUTO_RST,temp);	
	GX1131_Set_Work_BS_Mode(1);		
	temp=GX1131_Read_one_Byte(GX1131_BCS_RST)&0xfe;		
	GX1131_Write_one_Byte(GX1131_BCS_RST, temp);
	
	GX1131_SetSymbolRate(Lpf_BW_window);	
	GX1131_Write_one_Byte(GX1131_FC_OFFSET_H,0x00);
	GX1131_Write_one_Byte(GX1131_FC_OFFSET_L,0x00);	
	GX1131_Set_Tuner_Repeater_Enable(1);	
	tunerDriver(Fcenter,Lpf_BW_window);	
	GX1131_Set_Tuner_Repeater_Enable(0);
	GX1131_HotReset_CHIP();	
	
	for(i=0;i<16;i++)
		{
			if(GX1131_Read_Lock_Status()>=BCS_LOCKED)	
				{
						break;
				}
			if(i==15)				
				return 0;				
			GX_Delay_N_ms(10);
		}	
	
	TP_Num = GX1131_Read_one_Byte(GX1131_VID_SIG_NUM)&0x1f;
	if((TP_Num!=0)&&(TP_Num<=size))
	{
		for(i=0;i<TP_Num;i++)
			{
				temp=(GX1131_Read_one_Byte(GX1131_BCS_OUT_ADDR)&0xe0)|i;				
				GX1131_Write_one_Byte(GX1131_BCS_OUT_ADDR,temp);				

				temp=GX1131_Read_one_Byte(GX1131_BCS_FC_OFFSET_H);				
				temp1=GX1131_Read_one_Byte(GX1131_BCS_FC_OFFSET_L);								
				if(((temp<<8)+temp1)<=32767)		Fc_offset_Bs = ((temp<<8)+temp1);
				if(((temp<<8)+temp1)>32767)		Fc_offset_Bs = ((temp<<8)+temp1)-65536;
				if(gIQ_swap==0)	Fc_offset_Bs=0 - Fc_offset_Bs;
				bstp_list->Freq_KHz = (s32)Fcenter*1000+Fc_offset_Bs;	
				
				temp=GX1131_Read_one_Byte(GX1131_BCS_FS_H);				
				temp1=GX1131_Read_one_Byte(GX1131_BCS_FS_L);				
				Fs_Bs = (temp<<8)+temp1;
				bstp_list->SymRate_KSps=Fs_Bs;

				//louhq20100728
				//print("5F[7:5]=%d,%dKHz,%dKSps", (u32)((GX1131_Read_one_Byte(0x5F)>>5)&0x07),(u32)(bstp_list->Freq_KHz),Fs_Bs);

				if(Fs_Bs>5000)
					{
						if(((GX1131_Read_one_Byte(0x5F)>>5)&0x07)>=2)
					
							bstp_list++;
					}
				else if(Fs_Bs<=5000)
					{
						if(((GX1131_Read_one_Byte(0x5F)>>5)&0x07)>=3)
			
							bstp_list++;
					}
			}
	}
	else 		
		return 0;		
	
	return TP_Num;
}	

/******************************************************************************
*Function: Perform blind scan within Start_Freq to End_Freq
*Input:
*	   Start_Freq: the start freq of blind scan freq segment, unit:MHz
*	   Start_Freq: the end freq of blind scan freq segment, unit:MHz
*	   TP_Num_Previous_seg:the TPs number of previous blind scan segment
*Output:
*         the number of TPs in this freq segment
*	    or FAILURE
*******************************************************************************/
u32 GX1131_BS_Seg_Freq(u32 Start_Freq,u32 End_Freq,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver)
{
	u32 TP_Num=0;
	u32 Fcenter;
	u32 Bs_window_num=0;
	u32	TP_Num_total=0;
	u32 i;
	GX1131_TP *dd = 	bstp_list;
	if(Start_Freq<_FREQ_LOW_LIM_MHZ_) Start_Freq=_FREQ_LOW_LIM_MHZ_;
	if(End_Freq>_FREQ_HIGH_LIM_MHZ_) End_Freq=_FREQ_HIGH_LIM_MHZ_;

	Fcenter=Start_Freq;
	do
		{			
			TP_Num=GX1131_BS_one_Window(Fcenter,GX1131_BS_WINDOW_SIZE_K,bstp_list,size,tunerDriver);				
			bstp_list += TP_Num;
			size -= TP_Num;	
			TP_Num_total+=TP_Num;
			Fcenter+=GX1131_BS_STEP_MHZ;					
			Bs_window_num+=1;
		}
	while(Fcenter<=(End_Freq+GX1131_BS_STEP_MHZ));	

	return TP_Num_total;
}	

/******************************************************************************
*Function: sort the BS TPs from low freq to high freq
*Input:
*	   Start_Freq: the start freq of blind scan freq segment, unit:MHz
*	   Start_Freq: the end freq of blind scan freq segment, unit:MHz
*	   TP_Num_Previous_seg:the TPs number of previous blind scan segment
*Output:
*         the number of TPs in this freq segment
*	    or FAILURE
*******************************************************************************/
u32 GX1131_BS_TP_Sort_Seg_Freq(u32 Start_Freq,u32 End_Freq,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver)
{
	u32 i,n;
	s32 j;
	u32 k=0;
	u32 TP_Num=0;
	GX1131_TP temp_tp;	
	u32 deta_fc_K;
	u32 deta_fs_K;
	
	TP_Num=GX1131_BS_Seg_Freq(Start_Freq,End_Freq,bstp_list,size,tunerDriver);
	if (TP_Num==0) return 0;
	
	//sort the TPs in this segment according the Fc	
	bstp_list[TP_Num].Freq_KHz=0;
	bstp_list[TP_Num].SymRate_KSps = 0;	
	
	for(i=1;i<TP_Num;i++)
		{
			for(j=i-1;(bstp_list[j].Freq_KHz>bstp_list[j+1].Freq_KHz)&&(j>=0);j--)
				{
				 temp_tp = bstp_list[j];	bstp_list[j]=bstp_list[j+1];	bstp_list[j+1]=temp_tp;				
				}
		}	

/*	//Delete the tp ---louhq20100721
	bstp_list[TP_Num].Freq_KHz=0;
	bstp_list[TP_Num].SymRate_KSps = 0;	

	for(i=0;i<TP_Num; i++)
		{
			if(bstp_list[TP_Num].SymRate_KSps>=5000)
				{
					if(((GX1131_Read_one_Byte(0x5F)>>5)&0x07)<2)
						{									
							for(n=i;n<TP_Num-1;n++)
								{
									bstp_list[n]=bstp_list[n+1];												
								}
							TP_Num-=1;
							if (i>0)	i=i-1;
							bstp_list[TP_Num].Freq_KHz=0;
							bstp_list[TP_Num].SymRate_KSps = 0;	
						}
				}
			else
				{
					if(((GX1131_Read_one_Byte(0x5F)>>5)&0x07)<3)
						{								
							for(n=i;n<TP_Num-1;n++)
								{
									bstp_list[n]=bstp_list[n+1];												
								}
							TP_Num-=1;
							if (i>0)	i=i-1;
							bstp_list[TP_Num].Freq_KHz=0;
							bstp_list[TP_Num].SymRate_KSps = 0;	
						}
				}
		}
*/	
		 	
	//Delete the semblable TP	
	bstp_list[TP_Num].Freq_KHz=0;
	bstp_list[TP_Num].SymRate_KSps = 0;	
	for(i=0;i<TP_Num; i++)
		{
			if((bstp_list[i].Freq_KHz<945000)||(bstp_list[i].Freq_KHz>2155000)||(bstp_list[i].SymRate_KSps<800)||(bstp_list[i].SymRate_KSps>50000))
				{
					k+=1;									
					for(n=i;n<TP_Num-1;n++)
						{
							bstp_list[n]=bstp_list[n+1];												
						}
					TP_Num-=1;
					if (i>0)	i=i-1;
					bstp_list[TP_Num].Freq_KHz=0;
					bstp_list[TP_Num].SymRate_KSps = 0;	
				}
			else
				{
					j=i+1;					
					deta_fc_K =bstp_list[j].Freq_KHz - bstp_list[i].Freq_KHz;
					deta_fs_K =bstp_list[j].SymRate_KSps - bstp_list[i].SymRate_KSps;
					if(deta_fs_K<0) 
					deta_fs_K =bstp_list[i].SymRate_KSps - bstp_list[j].SymRate_KSps;
					if(((deta_fc_K<=bstp_list[i].SymRate_KSps/4)&&(deta_fc_K<=bstp_list[j].SymRate_KSps/4)
						&&(deta_fs_K<bstp_list[i].SymRate_KSps/32) && (deta_fs_K<bstp_list[j].SymRate_KSps/32))||(deta_fc_K<=500))
						{
							k+=1;									
							for(n=j;n<TP_Num-1;n++)
								{
									bstp_list[n]=bstp_list[n+1];																
								}
							TP_Num-=1;
							if (i>0) i=i-1;
							bstp_list[TP_Num].Freq_KHz=0;
					  		bstp_list[TP_Num].SymRate_KSps=0;
						}
				}			

		}
	
	return TP_Num;
}


/******************************************************************************
*Function: Perform step2 of blind scan and lock a TP scaned
*Input:
*	  Start_Freq:		
*	  End_Freq
*	   Polar: 			the set of polarity
*	   Tone_on: 		22K on/off
*Output:
*         the number of TPs on this satellite
*	    or FAILURE
*******************************************************************************/
bool GX1131_BS_Lock_List(u32 Start_Freq,u32 End_Freq,GX1131_TP *bstp_list,u32 size,
							  TunerDriver tunerDriver,bcs_callback lock_cb,GX1131_BsBack *BsbackPar)
{
	u32 TP_Num=0;	
	u8 i,j,k,m,temp1,temp2;		
	bool	locked_flag=0;
	u8	count;
	u8 val_5e, val_5f;
			
	TP_Num=GX1131_BS_TP_Sort_Seg_Freq(Start_Freq,End_Freq,bstp_list,size,tunerDriver);	
	
	BsbackPar->nMaxSub = TP_Num;
	BsbackPar->nSubStage = 0;
	BsbackPar->nMaskUpdate = BS_CB_SUB_STAGE | BS_CB_MAX_SUB;
	if (lock_cb(BsbackPar)==FALSE)
		return FALSE;
		
	for(i=0;i<TP_Num;i++)
	{
		//val_5e = GX1131_Read_one_Byte(0x5E);
		//val_5f = GX1131_Read_one_Byte(0x5F);
		
		//print("[Scan TP %d]:%dMHz,%dKSps,Polar=%d,Tone=%d",
			//i+1,bstp_list[i].Freq_KHz/1000,bstp_list[i].SymRate_KSps,bstp_list[i].V_polar,bstp_list[i]._22K_on);

	
		GX1131_lock_TP_BS(bstp_list[i], tunerDriver);
		locked_flag=0;
		m=0;
		k=0;		
		BsbackPar->nSubStage = i + 1;
		BsbackPar->nMaskUpdate = BS_CB_SUB_STAGE;
		count=30;
		/////louhq20100805
		for(j=0;j<count;j++)
			{					
				switch(GX1131_Read_Lock_Status())
					{
						case FEC_LOCKED:
							{
								m+=1;
								if(m>=1)		
									locked_flag=1;
								break;
							}
						case EQU_LOCKED:
							{
								if(j>=22)
									j+=1;
								break;
							}
						case CRL_LOCKED:
							{
								if(j>=18)
									j+=1;//louhq20100805
								break;
							}
						case TIM_LOCKED:
							{
								if(j>=18)
									j+=2;//louhq20100805
								break;
							}
						case BCS_LOCKED:
							{
								if(j>=18)
									j+=2;//louhq20100805
								break;
							}
						case AGC_LOCKED:
							{
								if(j>=18)
									j+=3;
								break;
							}
						case UNLOCKED:
							{
								if(j>=18)
									j+=4;
								break;
							}
					}
				if(locked_flag)	break;

				//////louhq20100805
				if(j>5)
					{
						if(bstp_list[i].SymRate_KSps>5000)
							{
								if(((GX1131_Read_one_Byte(0x5F)>>5)&0x07)<2)
									break;
							}
						if(bstp_list[i].SymRate_KSps<=5000)
							{
								if(((GX1131_Read_one_Byte(0x5F)>>5)&0x07)<3)
									break;
							}
						if(GX1131_Read_Lock_Status()>=TIM_LOCKED)
							{	
								temp1=GX1131_Read_one_Byte(0x4A);
								temp2=GX1131_Read_one_Byte(0x4B);
								//print("temp2=%x",temp2);
								if((temp2&0x0F)>9)
									{	
										break;
									}
							}
					}
				///////////////////
				if(bstp_list[i].SymRate_KSps<=5000)	
					GX_Delay_N_ms(100);
				else	if((bstp_list[i].SymRate_KSps>5000)&&(bstp_list[i].SymRate_KSps<=10000))	
					GX_Delay_N_ms(75);
				else			
					GX_Delay_N_ms(50);

				}
		if(locked_flag)
			{
				BsbackPar->tpinfo = GX1131_Get_TP_Info(bstp_list[i].Freq_KHz,bstp_list[i].SymRate_KSps);
				BsbackPar->nMaskUpdate |= BS_CB_RESULT;
			//val_5e = GX1131_Read_one_Byte(0x5E);
			//val_5f = GX1131_Read_one_Byte(0x5F);
			//print("!!!locked!\n, 5E[7:0]=%d, 5F[7:5]=%d,5F[4:0]=%d\n\n", (u32)val_5e, (u32)(val_5f >>5), (u32)(val_5f & 0x1F));
			}		

		if (lock_cb(BsbackPar)==FALSE)
			return FALSE;

		if(!locked_flag)
		{
			val_5e = GX1131_Read_one_Byte(0x5E);
			val_5f = GX1131_Read_one_Byte(0x5F);
			//print("!!!Not locked!, 5E[7:0]=%d, 5F[7:5]=%d,5F[4:0]=%d", (u32)val_5e, (u32)(val_5f >>5), (u32)(val_5f & 0x1F));
		}
	}
	
	return 1;
}

/******************************************************************************
*Function: Perform blind scan and lock on a satellite
*Input:
*	   Ku: 			the type of LNB and antenna, 0 is C band, 1 is Ku band
*	   dual_local_osc: 	the type of LNB, 0 is single local osc, 1 is dual local osc
*Output:
*         the number of TPs on this satellite
*	    or FAILURE
*******************************************************************************/
bool GX1131_BS_lock_a_Satellite(GX1131_BCS_TYPE *bcs_type,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver,
                                                               bcs_callback lock_cb,GX1131_BsBack *BsbackPar)
{
	u32 i;
		
	if(bcs_type->Start_Freq_MHz<_FREQ_LOW_LIM_MHZ_) bcs_type->Start_Freq_MHz=_FREQ_LOW_LIM_MHZ_;
	if(bcs_type->End_Freq_MHz>_FREQ_HIGH_LIM_MHZ_) bcs_type->End_Freq_MHz=_FREQ_HIGH_LIM_MHZ_;	

	memcpy(&sg_BSType,bcs_type,sizeof(GX1131_BCS_TYPE));
	
	if(bcs_type->dual_local_osc_LNB==0)
		{//单本振[C/KU]
		//for 22K SET
			GX1131_22K_sw(bcs_type->_22k_state);
			
			if(bcs_type->SearchMode == 2)//ALL
			{
				BsbackPar->nMaxMain=2;
			}
			else
			{
				BsbackPar->nMaxMain=1;
			}
			//H polarity
			//BsbackPar->nMaxMain=2;
			if(bcs_type->SearchMode != 1)// H
			{
				BsbackPar->nMainStage=1;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAX_MAIN|BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;
				
				GX1131_Set_Polar(0);
				//GX1131_22K_sw(bcs_type->_22k_state);
				for(i=0;i<size;i++)
					{
						bstp_list[i].V_polar=0;
						bstp_list[i]._22K_on=bcs_type->_22k_state;
					}
				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;							
			}
			//V polarity
			if(bcs_type->SearchMode != 0)
			{
				if(BsbackPar->nMaxMain == 2)
					BsbackPar->nMainStage=2;
				else
					BsbackPar->nMainStage=1;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;

				GX1131_Set_Polar(1);
				//GX1131_22K_sw(0);
				for(i=0;i<size;i++)
					{
						bstp_list[i].V_polar=1;
						bstp_list[i]._22K_on=bcs_type->_22k_state;
					}

				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;							
			}
			return 1;
		}
	else if((bcs_type->dual_local_osc_LNB==1)&&(bcs_type->Ku_LNB==0))
		{//C双本振	

			//for 22K SET
			GX1131_22K_sw(bcs_type->_22k_state);
			
			if(bcs_type->SearchMode == 2)//ALL
			{
				BsbackPar->nMaxMain=2;
			}
			else
			{
				BsbackPar->nMaxMain=1;
			}
			//H polarity
			//BsbackPar->nMaxMain=2;
			if(bcs_type->SearchMode != 1)//H
			{
				BsbackPar->nMainStage=1;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAX_MAIN|BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;
				
				GX1131_Set_Polar(0);
				//GX1131_22K_sw(0);		
				for(i=0;i<size;i++)
					{
						bstp_list[i].V_polar=0;
						bstp_list[i]._22K_on=bcs_type->_22k_state;
					}
				bcs_type->End_Freq_MHz=(u32)((_FREQ_LOW_LIM_MHZ_+_FREQ_HIGH_LIM_MHZ_)/2-50);
				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;	
			}
			//V polarity
			if(bcs_type->SearchMode != 0)
			{
				if(BsbackPar->nMaxMain == 2)
					BsbackPar->nMainStage=2;
				else
					BsbackPar->nMainStage=1;
				bcs_type->Start_Freq_MHz= _FREQ_LOW_LIM_MHZ_;
				bcs_type->End_Freq_MHz=_FREQ_HIGH_LIM_MHZ_;
				//BsbackPar->nMainStage=1;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;
				
				GX1131_Set_Polar(1);
				//GX1131_22K_sw(0);
				for(i=0;i<size;i++)
				{
					bstp_list[i].V_polar=1;
					bstp_list[i]._22K_on=bcs_type->_22k_state;
				}

				bcs_type->Start_Freq_MHz=(u32)(_FREQ_LOW_LIM_MHZ_+_FREQ_HIGH_LIM_MHZ_)/2;
				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;	
			}
			return 1;
		}
	else if((bcs_type->dual_local_osc_LNB==1)&&(bcs_type->Ku_LNB==1))
		{//KU双本振
			if(bcs_type->SearchMode == 2)//ALL
			{
				BsbackPar->nMaxMain=4;
			}
			else
			{
				BsbackPar->nMaxMain=2;
			}
			//low osc H pol
			//BsbackPar->nMaxMain=4;
			if(bcs_type->SearchMode != 1)
			{
				BsbackPar->nMainStage=1;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAX_MAIN|BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;
				
				GX1131_Set_Polar(0);
				GX1131_22K_sw(0);	
				for(i=0;i<size;i++)
					{
						bstp_list[i].V_polar=0;
						bstp_list[i]._22K_on=0;
					}
				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;					
			}
			//low osc V pol
			if(bcs_type->SearchMode != 0)
			{
				if(BsbackPar->nMaxMain == 2)
					BsbackPar->nMainStage=1;
				else
					BsbackPar->nMainStage=2;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;
				
				GX1131_Set_Polar(1);
				GX1131_22K_sw(0);	
				for(i=0;i<size;i++)
					{
						bstp_list[i].V_polar=1;
						bstp_list[i]._22K_on=0;
					}
				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;				
			}
			//high osc H pol
			if(bcs_type->SearchMode != 1)
			{
				if(BsbackPar->nMaxMain == 2)
					BsbackPar->nMainStage=2;
				else
					BsbackPar->nMainStage=3;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;
				
				GX1131_Set_Polar(0);
				GX1131_22K_sw(1);	
				for(i=0;i<size;i++)
					{
						bstp_list[i].V_polar=0;
						bstp_list[i]._22K_on=1;
					}
				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;					
			}
			//high osc V pol
			if(bcs_type->SearchMode != 0)
			{
				if(BsbackPar->nMaxMain == 2)
					BsbackPar->nMainStage=2;
				else
					BsbackPar->nMainStage=4;
				BsbackPar->nMaxSub = 0;
				BsbackPar->nMaskUpdate=BS_CB_MAIN_STAGE;
				if (lock_cb(BsbackPar)==FALSE)
					return FALSE;
				
				GX1131_Set_Polar(1);
				GX1131_22K_sw(1);	
				for(i=0;i<size;i++)
					{
						bstp_list[i].V_polar=1;
						bstp_list[i]._22K_on=1;
					}
				if(GX1131_BS_Lock_List(bcs_type->Start_Freq_MHz,bcs_type->End_Freq_MHz,bstp_list,size,
								 tunerDriver,lock_cb,BsbackPar)==FALSE)
					return FALSE;			
			}
			return 1;
		}		
	
	return 0;	
}


bool GX1131_DATA_CATCH_AGC(u32 S_freq_MHz,u32 E_freq_MHz,u32 BW_window,
								TunerDriver tunerDriver)
{
	u32 Fcenter;
	u32	i=0,j,k;
	u32	AGC_N[256] = {0};
	GX1131_LOCK_STATUS  temp;
	Fcenter=S_freq_MHz;	
	do
		{
			//i+4;
			//print("Fcenter=%dMHz",Fcenter);
			//print("spectrum scan,Debug,step%d",i);
			GX1131_Set_Tuner_Repeater_Enable(1);	
			tunerDriver(Fcenter,BW_window);	
			GX1131_Set_Tuner_Repeater_Enable(0);
			GX1131_HotReset_CHIP();
			temp=GX1131_Read_Lock_Status();
			while(1)
				{
					if(temp>=AGC_LOCKED)
						break;
					temp=GX1131_Read_Lock_Status();
				}
			for(k=0;k<1;k++)
			{
				i++;
				temp=GX1131_Get_Signal_Strength();
				//temp=temp*3-180;
				if(temp<=1)
					temp=1;
				if(temp>=100)
					temp=100;
				AGC_N[i]=temp;
				j=i;
				//data_agc_cb( temp,j);
			}

			//print("AGC_N[%d]=%d%",i,AGC_N[i]);
			Fcenter+=BW_window/1000;	
			
		}while(Fcenter<E_freq_MHz);
	return 1;
}

bool GX1131_Const(void)
{	
	int catch_finish=0,i,j;
	int a;
	int tmp,auto_rst_ena_tmp,auto_rst_Nena;
 	auto_rst_ena_tmp=GX1131_Read_one_Byte(0x36);
 	auto_rst_Nena = auto_rst_ena_tmp & 0xFE;
 	GX1131_Write_one_Byte(0x36,auto_rst_Nena);
	GX1131_Write_one_Byte(0x38,0x53);
     	for(i=0;i<20;i++)
		{
			catch_finish = GX1131_Read_one_Byte(0x39)&0x80;
			print("catch_finish=%d",catch_finish);
			if(catch_finish)
				{
					print("catch_finish");
					break;
				}
			GX_Delay_N_ms(50);	
     		}
	GX1131_Read_one_Byte(0x3B);
	for(i=0;i<=31;i++)
	  {  
		dat[i]=GX1131_Read_one_Byte(0x3B);
		//print( "dat[i]= %d,i=%d",dat[i],i);
	  }
	GX1131_Write_one_Byte(0x38,0x50);
 	GX1131_Write_one_Byte(0x36,auto_rst_ena_tmp);
     if(i!=32)
     {
	 return 0;
     }

	for(j=0;j<=31;j++)
	{
        	tmp= dat[j]&0x7F;
		 if(tmp>63)
		dat[j]=tmp-128;
	}
	return 1;
}
