/*******************************************************************************
*                          CONFIDENTIAL                                                                                         
*        Hangzhou NationalChip Science and Technology Co., Ltd.                                               
*                      (C)2009, All right reserved                                                                            
********************************************************************************

********************************************************************************
* File Name	 :   	Sharp7306.c                                                                                                 
* Author		 :     Louhq                                                                                                 
* Project    	 :   	GXAPI 
* Type      	 :     Driver
********************************************************************************
* Purpose   :   Sharp6306/7306  driver
********************************************************************************
* Release History:
* VERSION   Date              AUTHOR          Description
*    1.1      2009.12.23        Louhq       Original version   
********************************************************************************
*Abbreviation
*   GX		--	GUOXIN 
*   RF		--  radiate frequency
*   SNR		--	signal to noise ratio
*   OSC		--	oscillate
*   SPEC		--	spectrum
*   FREQ		--	frequency
********************************************************************************/
#include "GX1131.h"

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

//#include <math.h>
//#include <stdlib.h>
//#include <stdio.h>

#if TUNER_SHARP7306 
//u8 gIQ_swap=0; 
#define GX_TUNER_ADDRESS 0xC0
/*******************************************************************************
*Function:	GX1131_Set_Tuner
*Input:
*		RFfreq:	 		Fcenter at L band, unit:MHz
*		Symbol_Rate:		symbol rate, unit:kBaud
*Output:
*		SUCCESS or FAILURE
********************************************************************************/
GX1131_STATE GX_Set_Sharp7306(u32 RFfreq,u32 Symbol_Rate)
{   
	u8 	PSC,PD,BA,DIV;  
	u8 	Fosc=4;
	u8	A,N;
	u8 	REF=0;
	u8 	R;

	u8 	C=3;			/*charge pump current: 0: 78mA min,120mA typ, 150mA max
										 1: 169mA min,260mA typ, 325mA max
										 2: 360mA min,555mA typ, 694mA max
										 3: 780mA min,1200mA typ, 1500mA max*/
										 
	u8 	BB_Gain=2;	/*Baseband Amp control: 	0 and 1: 0dB
											2: -2dB
											3: -3dB	 */

	u32 	freq_tmp;
	u8 	regdata[5];
	GX1131_STATE 	temp;
		
	//if (RFfreq<950)
	//	return FAILURE;
	if(RFfreq<=986)
	{
		PSC = 1;
		DIV = 1;
		BA = 5;	
	}
	else if(RFfreq<=1073)
	{
		PSC = 1;
		DIV = 1;
		BA = 6;
	}
	else if(RFfreq<=1154)
	{
		PSC = 0;
		DIV = 1;
		BA = 7;
	}
	else if(RFfreq<=1291)
	{
		PSC = 0;
		DIV = 0;
		BA = 1;
	}
	else if(RFfreq<=1447)
	{
		PSC = 0;
		DIV = 0;
		BA = 2;	
	}
	else if(RFfreq<=1615)
	{
		PSC = 0;
		DIV = 0;
		BA = 3;
	}
	else if(RFfreq<=1791)
	{
		PSC = 0;
		DIV = 0;
		BA = 4;
	}
	else if(RFfreq<=1972)
	{
		PSC = 0;
		DIV = 0;
		BA = 5;
	}
 	//else if(RFfreq<=2155)
 	else
	{
		PSC = 0;
		DIV = 0;
		BA = 6;	
	}
	//else 
	//	return FAILURE;	

	Symbol_Rate=Symbol_Rate/1000;
	Symbol_Rate=(Symbol_Rate*8+5)/10;
	if(Symbol_Rate<10) Symbol_Rate=10;
	if(Symbol_Rate>34) Symbol_Rate=34;
	PD=(u8)((Symbol_Rate-4)/2);	

	if(REF==0) R=4;
	else if(REF==1) R=8;
	
	freq_tmp= RFfreq*R/Fosc;	

	if(PSC==0)	
	{
		A = (u8) (freq_tmp%32);
		N = (u8) (freq_tmp /32);		
	}
	else		
	{
		A = (u8) (freq_tmp%16);
		N = (u8) (freq_tmp/16);
	}		
	
//	first time				
	regdata[0] = (u8)(0xc0);	 	
	regdata[1] = (u8)((((BB_Gain&0x03)<<5) |((N&0xf8)>>3))& 0x7f);
	regdata[2] = (u8)(((N&0x07) << 5) | ((A&0x1f)<<0)); 
	regdata[3] = (u8)(0x80 | ((C&0x03)<<5) | ((PD&0x01)<<4) | (((PD&0x02)>>1)<<3) | ((REF&0x01)<<0));
	regdata[4] = (u8)((((BA&0x07) << 5) | ((DIV&0x01) << 1) | (((PD&0x04)>>2)<<3) | (((PD&0x08)>>3)<<2) | ((PSC&0x01) << 4)) & 0xfe); 
	temp=GX_I2cReadWrite(WRITE, regdata[0], regdata[1], &regdata[2],3);	
		
//	second time
	regdata[3] = (u8)(0x84 | ((C&0x03)<<5) | ((PD&0x01)<<4) | (((PD&0x02)>>1)<<3) | ((REF&0x01)<<0));
	temp=GX_I2cReadWrite(WRITE, regdata[0], regdata[3], &regdata[4],0);		
	GX_Delay_N_ms(30);		//it's important, adjust to your decoder
	
//	third time
	regdata[3] = (u8)(0x84 | ((C&0x03)<<5) | ((PD&0x01)<<4) | (((PD&0x02)>>1)<<3) | ((REF&0x01)<<0));
	regdata[4] = (u8)((((BA&0x07) << 5) | ((DIV&0x01) << 1) | (((PD&0x04)>>2)<<3) | (((PD&0x08)>>3)<<2)  | ((PSC&0x01) <<4))  & 0xfe);
  	temp=GX_I2cReadWrite(WRITE, regdata[0], regdata[3], &regdata[4],1);	
	//GX_Delay_N_ms(5);	
	return SUCCESS;
}
#endif
