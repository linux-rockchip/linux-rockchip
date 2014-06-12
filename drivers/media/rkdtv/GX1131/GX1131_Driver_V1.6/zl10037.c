/*******************************************************************************
*                          CONFIDENTIAL                             
*        Hangzhou NationalChip Science and Technology Co., Ltd.             
*                      (C)2008, All right reserved
********************************************************************************

********************************************************************************
* File Name :   zl10037.c
* Author    :   Louhq
* Project   :   GXAPI 
* Type      :   Driver
********************************************************************************
* Purpose   :   TUNER ZL10037 Çý¶¯
********************************************************************************
* Release History:
  VERSION   Date              AUTHOR         Description
  1.00      2010.2.7          Louhq           creation
*******************************************************************************/
#if TUNER_ZL10037
#include "GX1131.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


//u8 gIQ_swap=1;
#define GX_TUNER_ZL37_ADDRESS 0xC0


#define ZL10037_CRYSTAL 10111//10111//KHZ//
#define ZL10037_BANDGAIN 11         //[0 - 15]
/*
********************************************************************************
Function:	GX_Set_ZL10037
Input:
	RFfreq-- rf freq (unit:Mhz)
	Symbol_Rate_Value--symbol rate freq(kHz)
Output:
	SUCCESS or FAILURE
********************************************************************************
*/
GX1131_STATE GX_Set_ZL10037(u32 RFfreq,u32 Symbol_Rate_Value)
{
	GX1131_STATE	state;
	u32 BandwidthFilter;
	u8 ucWriteTemp;
	u32 nR = 0;
	u32 nBR;
	u32 nPll;
	u32 crystal = ZL10037_CRYSTAL; //KHZ
	u8 ucBandGain = ZL10037_BANDGAIN;
	u8 ucDeviceAddr = 0xC0;
	
	/* reset chip */
	ucWriteTemp = 0xC1;
	state=GX_I2cReadWrite(WRITE,ucDeviceAddr, 0x0F ,&ucWriteTemp, 1);
	GX_Delay_N_ms(20);
	ucWriteTemp = 0x01;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr, 0x0F, &ucWriteTemp, 1);
	GX_Delay_N_ms(20);
	
	/* init reg */
	if (crystal < 4000 )
	{
		ucWriteTemp = 0x20;
		state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x02,&ucWriteTemp,1);
		nR = 2;
	}
	else if (crystal < 8000 )
	{
		ucWriteTemp = 0x21;
		state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x02,&ucWriteTemp,1);
		nR = 4;
	}
	else if (crystal < 16000 )
	{
		ucWriteTemp = 0x22;
		state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x02,&ucWriteTemp,1);
		nR = 8;
	}
	else if (crystal < 24000 )
	{
		ucWriteTemp = 0x23;
		state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x02,&ucWriteTemp,1);
		nR = 16;
	}
	else if (crystal < 32000 )
	{
		ucWriteTemp = 0x24;
		state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x02,&ucWriteTemp,1);
		nR = 32;
	}
	else 
	{
		ucWriteTemp = 0x25;
		state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x02,&ucWriteTemp,1);
		nR = 64;
	}
	
	ucWriteTemp=0x08;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr, 0x06,&ucWriteTemp,1);

	ucWriteTemp = ucBandGain << 3;;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x07,&ucWriteTemp,1);
	ucWriteTemp = 0x05;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x00,&ucWriteTemp,1);
	ucWriteTemp = 0xFD;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x01,&ucWriteTemp,1);

	/* set PLL */
	nPll =(u32)((RFfreq * nR * 10000 / crystal + 5) / 10); //f_tuner * nR / crystal + 0.5
	
	ucWriteTemp = (nPll >> 8) & 0x7F;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x00,&ucWriteTemp,1);
	ucWriteTemp = nPll & 0xFF;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x01,&ucWriteTemp,1);

	/* set LPF */
	nBR = 4;
	ucWriteTemp = nBR;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x06,&ucWriteTemp,1);


     
	if (Symbol_Rate_Value <= 32000)
	{
		BandwidthFilter=(100000+Symbol_Rate_Value*30/4+5000)/10;
	}
	else
	{
		BandwidthFilter=(50000+Symbol_Rate_Value*30/4+5000)/10;
	} 

	  
	ucWriteTemp = (u8)((BandwidthFilter * 5.088 *nBR /crystal -5) ); 
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x05,&ucWriteTemp,1);
	
	ucWriteTemp = nBR | 0x20;
	state|=GX_I2cReadWrite(WRITE,ucDeviceAddr,0x06,&ucWriteTemp,1);
	
	return state;
	
 }

#endif
