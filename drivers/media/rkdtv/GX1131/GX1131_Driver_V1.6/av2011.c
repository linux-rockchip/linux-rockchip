/*******************************************************************************
*                          CONFIDENTIAL                             
*        Hangzhou NationalChip Science and Technology Co., Ltd.             
*                      (C)2008, All right reserved
********************************************************************************

********************************************************************************
* File Name :   rf_av2011.c
* Author    :   Louhq
* Project   :   GXAPI 
* Type      :   Driver
********************************************************************************
* Purpose   :   TUNER AV2020 Çý¶¯
********************************************************************************
* Release History:
  VERSION   Date              AUTHOR         Description
  1.00      2010.4.12           Louhq	
*******************************************************************************/
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

#if TUNER_AV2011
u8 gIQ_swap=1;
#define GX_TUNER_ADDRESS 0xC6

GX1131_STATE GX_Set_AV2011( u32 RFfreq, u32 Symbol_Rate);

static u8 deviceAddr = 0xC6;
static u32 tuner_crystal = 27; //unit is MHz


GX1131_STATE AV2011WriteReg(u8 regAdd, u8 *data, u8 len)
{
	return GX_I2cReadWrite(WRITE, deviceAddr, regAdd, data, len);
}

GX1131_STATE AV2011ReadReg(u8 regAdd)
{
	u8 retValue = 0;
	GX_I2cReadWrite(READ, deviceAddr, regAdd, &retValue, 1);
	return retValue;
}

GX1131_STATE GX_Set_AV2011(u32 RFfreq, u32 Symbol_Rate)
{

	u8 reg[50];
    	
    	u8 reg_start;
	u32 fracN;
	u32 BW;
	u32 BF;
	u8 auto_scan = 0;// Add flag for ""
	
	// Register initial flag
	static u8 tuner_initial =0;


	// At Power ON, tuner_initial = 0 
	// do the initial if not do yet.
	if (tuner_initial==0) 
	{
        //Tuner Initail registers R0~R41
    		reg[0]= (0x38);
		reg[1]= (0x00);
		reg[2]= (0x00);
		reg[3]= (0x50);
		reg[4]= (0x1f);
		reg[5]= (0xa3);
		reg[6]= (0xfd);
		reg[7]= (0x58);
		reg[8]= (0x36);//(0x0e);//modify form shenjj		
		reg[9]= (0xc2);//(0x82);//modify form shenjj
		reg[10]= (0x88);
		reg[11]= (0xb4);
 	       reg[12]= (0x96);	//RFLP=off at Power on initial  d6:on
		reg[13]= (0x40);
		reg[14]= (0x5b);
		reg[15]= (0x6a);
		reg[16]= (0x66);
		reg[17]= (0x40); 
		reg[18]= (0x80);	
		reg[19]= (0x2b);
		reg[20]= (0x6a);
		reg[21]= (0x50);	
		reg[22]= (0x91);
		reg[23]= (0x27); 
		reg[24]= (0x8f);
		reg[25]= (0xcc);
		reg[26]= (0x21);
		reg[27]= (0x10);
		reg[28]= (0x80);
		reg[29]= (0x02);
		reg[30]= (0xf5);
		reg[31]= (0x7f);
		reg[32]= (0x4a);
		reg[33]= (0x9b);
		reg[34]= (0xe0);
		reg[35]= (0xe0);
		reg[36]= (0x36);
//monsen 20080710. Disble FT-function at Power on initial
      //reg[37]= (0x02);
		reg[37]= (0x00); 
     
		reg[38]= (0xab);
		reg[39]= (0x97);
		reg[40]= (0xc5);
		reg[41]= (0xa8);
		
		//monsen 20080709. power on initial at first "Tuner_control()" call
		
		// Sequence 1    
		// Send Reg0 ->Reg11 
		reg_start = 0;
		AV2011WriteReg(reg_start,reg,12);
		
		// Time delay 1ms
		GX_Delay_N_ms(1); 
		
		// Sequence 2
		// Send Reg13 ->Reg24
		reg_start = 13;
		AV2011WriteReg(reg_start,reg+13,12);		
    	// Send Reg25 ->Reg35
		reg_start = 25;
		AV2011WriteReg(reg_start,reg+25,11);
		// Send Reg36 ->Reg41
		reg_start = 36;
		AV2011WriteReg(reg_start,reg+36,6);		
		
		// Time delay 1ms
		GX_Delay_N_ms(1); 
		
        // Sequence 3
		// send reg12		
		reg_start = 12;
		AV2011WriteReg(reg_start,reg+12,1);
	
		//monsen 20081125, Wait 100 ms	
		GX_Delay_N_ms(100);
		
		//monsen 20081030, Reinitial again
		{
		   // Sequence 1    
		   // Send Reg0 ->Reg11 
		   reg_start = 0;
		   AV2011WriteReg(reg_start,reg,12);
		
           // Time delay 1ms
		   GX_Delay_N_ms(1); 
		
		   // Sequence 2
		   // Send Reg13 ->Reg24
		   reg_start = 13;
		   AV2011WriteReg(reg_start,reg+13,12);		
           // Send Reg25 ->Reg35
		   reg_start = 25;
		   AV2011WriteReg(reg_start,reg+25,11);
		   // Send Reg36 ->Reg41
		   reg_start = 36;
		   AV2011WriteReg(reg_start,reg+36,6);		
		
		   // Time delay 1ms
		   GX_Delay_N_ms(1); 
		
           // Sequence 3
		   // send reg12		
		   reg_start = 12;
		   AV2011WriteReg(reg_start,reg+12,1);
		
		 }
		
		// Wait 4 ms		
		GX_Delay_N_ms(4);
		
		// power on initial end
		tuner_initial = 1;
		
		// Wait 4 ms		
		GX_Delay_N_ms(4);
	}
//	else 
//	{
	//***** monsen 20080709. Lock Channel sequence 
    //***** After RFAGC is starting tracking
    //***** Make sure the RFAGC do not have sharp jump after sending reg[37]   
	    // RF Channel Fregency Setting.  
	    
    	fracN = (RFfreq + tuner_crystal/2)/tuner_crystal;  
    	if(fracN > 0xff)
    		fracN = 0xff;  
    	reg[0]=(s8) (fracN & 0xff);
    	fracN = (RFfreq<<17)/tuner_crystal;
    	fracN = fracN & 0x1ffff;
    	reg[1]=(s8) ((fracN>>9)&0xff);
    	reg[2]=(s8) ((fracN>>1)&0xff);    	
        // reg[3]_D7 is frac<0>, D6~D0 is 0x50	
	    reg[3]=(s8) (((fracN<<7)&0x80) | 0x50);	// default is 0x50

	//printf("fracN = %d, reg[0]=%x,  reg[1]=%x,  reg[2]=%x,  reg[3]=%x\n", 
	//fracN, reg[0],reg[1],reg[2], reg[3]);
	
		// rolloff is 35%
    		BW = Symbol_Rate*135/200;
   	    	// monsen 20080726, BB request low IF when sym < 6.5MHz			
            // add 6M when Rs<6.5M, 
    		if(Symbol_Rate<6500)  
    			BW = BW + 6000;
   			// add 2M for LNB frequency shifting
    		BW = BW + 2000;//2000
   			// add 8% margin since fc is not very accurate	
    		BW = BW*108/100;
    		// Bandwidth can be tuned from 4M to 40M
    		if( BW< 4000)
    		      BW = 4000;
    		if( BW> 40000)
    		      BW = 40000;    		      
    		BF = (BW*127 + 21100/2) / (21100); // 	BW(MHz) * 1.27 / 211KHz
    		reg[5] = (u8)BF;
  //  	}
    	//when sym is 0 or 45000, means auto-scan channel.
	if (Symbol_Rate == 0 || Symbol_Rate == 45000) //auto-scan mode
		{
			auto_scan = 1;
			reg[5] = 0xA3; //BW=27MHz
		}
	if(auto_scan)
		{
	    	// Sequence 4	
	    	// Send Reg0 ->Reg4
	    	AV2011WriteReg(0,reg,4);
		// Time delay 4ms
  	   	GX_Delay_N_ms(4);
	   	// Sequence 5
	    	// Send Reg5  	
	    	AV2011WriteReg(5, reg+5, 1);
	  	// Fine-tune Function Control
	  	//Auto-scan mode. FT_block=1, FT_EN=0, FT_hold=1
		reg[37] = 0x05;
		AV2011WriteReg(37, reg+37, 1);
		// Time delay 4ms
		GX_Delay_N_ms(4);
		}
	else
		{
	    	// Sequence 4	
	    	// Send Reg0 ->Reg4
	    	AV2011WriteReg(0,reg,4);
		// Time delay 4ms
  	   	GX_Delay_N_ms(4);
	   	// Sequence 5
	    	// Send Reg5  	
	    	AV2011WriteReg(5, reg+5, 1);
	  	// Fine-tune Function Control
	  	//Auto-scan mode. FT_block=1, FT_EN=0, FT_hold=1
		reg[37] = 0x06;
		AV2011WriteReg(37, reg+37, 1);
		reg[12] = 0x96; //Disable RFLP at Lock Channel sequence after reg[37]		
		// reg[12] = 0xd6; //Enable RFLP at Lock Channel sequence after reg[37]  		
		AV2011WriteReg(12, reg+12, 1);
		// Time delay 4ms
		GX_Delay_N_ms(4);
		}
    	GX_Delay_N_ms(5); 
	return SUCCESS;
}

#endif

