/*******************************************************************************
*                          CONFIDENTIAL                             
*        Hangzhou NationalChip Science and Technology Co., Ltd.             
*                      (C)2009, All right reserved
********************************************************************************

********************************************************************************
* File Name :   rf_tfs5812.c
* Author    :   Louhq
* Project   :    
* Type      :   Driver
********************************************************************************
* Purpose   :   TUNER RDA5812 Çý¶¯
********************************************************************************
* Release History:
  ase History:
  VERSION   Date              AUTHOR         Description
   1.00    2010.2.7      	     Louhq	          Open Code    
*******************************************************************************/
#if TUNER_RDA5812
#include "GX1131.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


//u8 gIQ_swap=0;
#define GX_TUNER_RDA5812_ADDRESS 0x18
GX1131_STATE RDA5812Init(void);
/*
GX1131_STATE GX_RDA5812_Init_tuner(void)//20090731 gechq
{     
	s32 Read_tem;
	//sprintf(PrintfBuffer,"\n[RF]----rda5812\n");
	//GX1121_Printf(PrintfBuffer);

	Read_tem=GX1131_Read_one_Byte(0x06);
	if (Read_tem != FAILURE)
	{
		Read_tem |=0x80;     
		GX1131_Write_one_Byte(0x06,(u8)Read_tem);
	}
	RDA5812Init();
	
	Read_tem=GX1131_Read_one_Byte(0x06);
	if (Read_tem != FAILURE)
	{
		Read_tem&=0x7f;  
		GX1131_Write_one_Byte(0x06,(u8)Read_tem);
	}
	
	return GX1131_Init_Chip();
}
*/

/*
********************************************************************************
Function:	GX_Set_RDA5812
Input:
	RFfreq-- rf freq (unit:Mhz)
	Symbol_Rate_Value--symbol rate freq(kHz)
Output:
	SUCCESS or FAILURE
********************************************************************************
*/

GX1131_STATE GX_Set_RDA5812(u32 RFfreq,u32 Symbol_Rate_Value)
{
	GX1131_STATE	state;
	u8	data[4];
	u8	crystal=27; // 27Mhz
	u32 BandwidthFilter;
	u32 calfreq=0;
	u8 Filter_bw_control_bit;
	static u8 tuner_initial =0;

	if (tuner_initial==0) 
		{
			RDA5812Init();
			tuner_initial =1;
		}
	if(Symbol_Rate_Value==0)return FAILURE;
	data[0]=GX_TUNER_RDA5812_ADDRESS;    //tunerµÄµØÖ· 


	data[1] = 0x81;
	state |= GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,0x04,&data[1],1);

	calfreq =(2<<20)/4*RFfreq;
	calfreq /= crystal;
	calfreq *=4;

	data[1]=((u8)((calfreq>>24) & 0xFF));
	state |= GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,0x07,&data[1],1);
	data[1]=((u8)((calfreq>>16) & 0xFF));
	state |= GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,0x08,&data[1],1);
	data[1]=((u8)((calfreq>>8) & 0xFF));
	state |= GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,0x09,&data[1],1);
	data[1]=((u8)(calfreq & 0xFF));
	state |= GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,0x0A,&data[1],1);

	BandwidthFilter = (Symbol_Rate_Value*135)/200+500;
	Filter_bw_control_bit = (u8)(BandwidthFilter/1000); //MHz
	if(Filter_bw_control_bit>40)
	{
		Filter_bw_control_bit=40;
	}
	else if(Filter_bw_control_bit<4)
	{
		Filter_bw_control_bit=4;
	}

	data[1] = Filter_bw_control_bit & 0x3F;
	state |= GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,0x0B,&data[1],1);


	data[1] = 0x83;
	state |= GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,0x04,&data[1],1);

	GX_Delay_N_ms(30);
	return state;
}
/*
********************************************************************************
Function:	RDA5812Init
Input:
Output:
	SUCCESS or FAILURE
********************************************************************************
*/
GX1131_STATE RDA5812Init(void)//20090818 gechq
{
	u8 i = 0;
    GX1131_STATE state = SUCCESS;

	u8 regaddr[]={
		0x04,0x04,0x30,0x31,0x38,0x3A,0x3B,0x44,0x7C,0x39,0x3E,0x41,0x4F,0x53,
		0x67,0x68,0x69,0x6B,0x6C,0x6D,0x71,0x05,0x15,0x16,0x17,0x18,0x19,0x1A,
		0x1B,0x06,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
		0x28,0x29,0x2A,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,
		0x8C,0x8D,0x8E,0x91,0x92};
	u8 regdata[]={
		0x04,0x05,0x60,0x04,0x03,0x06,0x6B,0x55,0xC4,0xBA,0x83,0xA2,0x07,0xAC,
		0x1C,0x81,0x47,0x18,0xC6,0x4B,0x8E,0x10,0x11,0x25,0x2a,0x2e,0x3E,0x3F,
		0x3F,0x2a,0x5f,0x44,0x7f,0x4b,0x7f,0x3e,0x72,0x36,0x6e,0x3e,0x55,0x39,
		0x70,0x71,0xab,0x9a,0xb3,0xa8,0x84,0x76,0xa0,0xaa,0x10,0x00,0x12,0x14,
		0x0b,0x00,0x39,0x00,0x6F};


	for(i=0;i<sizeof(regaddr);i++)
	{
		state|=GX_I2cReadWrite(WRITE,GX_TUNER_RDA5812_ADDRESS,regaddr[i],&regdata[i],1);
	}
	GX_Delay_N_ms(10);
	return SUCCESS;
}

#endif

