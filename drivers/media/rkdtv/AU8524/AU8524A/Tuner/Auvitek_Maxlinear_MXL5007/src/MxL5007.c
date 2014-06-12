/*
 MxL5007 Source Code : V4.1.3
 
 Copyright, Maxlinear, Inc.
 All Rights Reserved
 
 File Name:      MxL5007.c

 Description: The source code is for MxL5007 user to quickly integrate MxL5007 into their software.
	There are two functions the user can call to generate a array of I2C command that's require to
	program the MxL5007 tuner. The user should pass an array pointer and an integer pointer in to the 
	function. The funciton will fill up the array with format like follow:
	
		addr1
		data1
		addr2
		data2
		...
	
	The user can then pass this array to their I2C function to perform progromming the tuner. 
*/
//#include "StdAfx.h"
#include "MxL5007_Common.h"
#include "MxL5007.h"


UINT32 MxL5007_Init(UINT8* pArray,				// a array pointer that store the addr and data pairs for I2C write
					UINT32* Array_Size,			// a integer pointer that store the number of element in above array
					UINT8 Mode,				
					SINT32 IF_Diff_Out_Level,		 
					UINT32 Xtal_Freq_Hz,			
					UINT32 IF_Freq_Hz,				
					UINT8 Invert_IF,											
					UINT8 Clk_Out_Enable,    
					UINT8 Clk_Out_Amp													
					)
{
	
	UINT32 Reg_Index=0;
	UINT32 Array_Index=0;

	IRVType IRV_Init[]=
	{
		//{ Addr, Data}	
		{ 0x02, 0x06}, 
		{ 0x03, 0x48},
		{ 0x05, 0x04},  
		{ 0x06, 0x10}, 
		{ 0x2E, 0x15},  //Override
		{ 0x30, 0x10},  //Override
		{ 0x45, 0x58},  //Override
		{ 0x48, 0x19},  //Override
		{ 0x52, 0x03},  //Override
		{ 0x53, 0x44},  //Override
		{ 0x6A, 0x4B},  //Override
		{ 0x76, 0x00},  //Override
		{ 0x78, 0x18},  //Override
		{ 0x7A, 0x17},  //Override
		{ 0x85, 0x06},  //Override		
		{ 0x01, 0x01}, //TOP_MASTER_ENABLE=1
		{ 0, 0}
	};


	IRVType IRV_Init_Cable[]=
	{
		//{ Addr, Data}	
		{ 0x02, 0x06}, 
		{ 0x03, 0x48},
		{ 0x05, 0x04},  
		{ 0x06, 0x10},  
		{ 0x09, 0x3F},  
		{ 0x0A, 0x3F},  
		{ 0x0B, 0x3F},  
		{ 0x2E, 0x15},  //Override
		{ 0x30, 0x10},  //Override
		{ 0x45, 0x58},  //Override
		{ 0x48, 0x19},  //Override
		{ 0x52, 0x03},  //Override
		{ 0x53, 0x44},  //Override
		{ 0x6A, 0x4B},  //Override
		{ 0x76, 0x00},  //Override
		{ 0x78, 0x18},  //Override
		{ 0x7A, 0x17},  //Override
		{ 0x85, 0x06},  //Override	
		{ 0x01, 0x01}, //TOP_MASTER_ENABLE=1
		{ 0, 0}
	};
	//edit Init setting here

	PIRVType myIRV;

	switch(Mode)
	{
	case MxL_MODE_ISDBT: //ISDB-T Mode	
		myIRV = IRV_Init;
		SetIRVBit(myIRV, 0x06, 0x1F, 0x10);  
		break;
	case MxL_MODE_DVBT: //DVBT Mode			
		myIRV = IRV_Init;
		SetIRVBit(myIRV, 0x06, 0x1F, 0x11);  
		break;
	case MxL_MODE_ATSC: //ATSC Mode			
		myIRV = IRV_Init;
		SetIRVBit(myIRV, 0x06, 0x1F, 0x12);  
		break;	
	case MxL_MODE_CABLE:						
		myIRV = IRV_Init_Cable;
		SetIRVBit(myIRV, 0x09, 0xFF, 0xC1);	
		SetIRVBit(myIRV, 0x0A, 0xFF, 8-IF_Diff_Out_Level);	
		SetIRVBit(myIRV, 0x0B, 0xFF, 0x17);							
		break;
	

	}

	switch(IF_Freq_Hz)
	{
	case MxL_IF_4_MHZ:
		SetIRVBit(myIRV, 0x02, 0x0F, 0x00); 		
		break;
	case MxL_IF_4_5_MHZ: //4.5MHz
		SetIRVBit(myIRV, 0x02, 0x0F, 0x02); 		
		break;
	case MxL_IF_4_57_MHZ: //4.57MHz
		SetIRVBit(myIRV, 0x02, 0x0F, 0x03); 
		break;
	case MxL_IF_5_MHZ:
		SetIRVBit(myIRV, 0x02, 0x0F, 0x04); 
		break;
	case MxL_IF_5_38_MHZ: //5.38MHz
		SetIRVBit(myIRV, 0x02, 0x0F, 0x05); 
		break;
	case MxL_IF_6_MHZ: 
		SetIRVBit(myIRV, 0x02, 0x0F, 0x06); 
		break;
	case MxL_IF_6_28_MHZ: //6.28MHz
		SetIRVBit(myIRV, 0x02, 0x0F, 0x07); 
		break;
	case MxL_IF_9_1915_MHZ: //9.1915MHz
		SetIRVBit(myIRV, 0x02, 0x0F, 0x08); 
		break;
	case MxL_IF_35_25_MHZ:
		SetIRVBit(myIRV, 0x02, 0x0F, 0x09); 
		break;
	case MxL_IF_36_15_MHZ:
		SetIRVBit(myIRV, 0x02, 0x0F, 0x0a); 
		break;
	case MxL_IF_44_MHZ:
		SetIRVBit(myIRV, 0x02, 0x0F, 0x0B); 
		break;
	}

	if(Invert_IF) 
		SetIRVBit(myIRV, 0x02, 0x10, 0x10);   //Invert IF
	else
		SetIRVBit(myIRV, 0x02, 0x10, 0x00);   //Normal IF


	switch(Xtal_Freq_Hz)
	{
	case MxL_XTAL_16_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x00);  //select xtal freq & Ref Freq
		SetIRVBit(myIRV, 0x05, 0x0F, 0x00);
		break;
	case MxL_XTAL_20_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x10);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x01);
		break;
	case MxL_XTAL_20_25_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x20);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x02);
		break;
	case MxL_XTAL_20_48_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x30);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x03);
		break;
	case MxL_XTAL_24_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x40);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x04);
		break;
	case MxL_XTAL_25_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x50);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x05);
		break;
	case MxL_XTAL_25_14_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x60);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x06);
		break;
	case MxL_XTAL_27_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x70);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x07);
		break;
	case MxL_XTAL_28_8_MHZ: 
		SetIRVBit(myIRV, 0x03, 0xF0, 0x80);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x08);
		break;
	case MxL_XTAL_32_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0x90);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x09);
		break;
	case MxL_XTAL_40_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0xA0);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x0A);
		break;
	case MxL_XTAL_44_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0xB0);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x0B);
		break;
	case MxL_XTAL_48_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0xC0);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x0C);
		break;
	case MxL_XTAL_49_3811_MHZ:
		SetIRVBit(myIRV, 0x03, 0xF0, 0xD0);
		SetIRVBit(myIRV, 0x05, 0x0F, 0x0D);
		break;	
	}

	if(!Clk_Out_Enable) //default is enable 
		SetIRVBit(myIRV, 0x03, 0x08, 0x00);   

	//Clk_Out_Amp
	SetIRVBit(myIRV, 0x03, 0x07, Clk_Out_Amp);   

	//Generate one Array that Contain Data, Address
	while (myIRV[Reg_Index].Num || myIRV[Reg_Index].Val)
	{
		pArray[Array_Index++] = myIRV[Reg_Index].Num;
		pArray[Array_Index++] = myIRV[Reg_Index].Val;
		Reg_Index++;
	}
	    
	*Array_Size=Array_Index;
	return 0;
}


UINT32 MxL5007_RFTune(UINT8* pArray, UINT32* Array_Size, UINT32 RF_Freq, UINT8 BWMHz)
{
	IRVType IRV_RFTune[]=
	{
	//{ Addr, Data}
		{ 0x0F, 0x00},  //abort tune
		{ 0x0C, 0x15},
		{ 0x0D, 0x40},
		{ 0x0E, 0x0E},	
		{ 0x1F, 0x87},  //Override
		{ 0x20, 0x1F},  //Override
		{ 0x21, 0x87},  //Override
		{ 0x22, 0x1F},  //Override		
		{ 0x80, 0x01},  //Freq Dependent Setting
		{ 0x0F, 0x01},	//start tune
		{ 0, 0}
	};

	UINT32 dig_rf_freq=0;
	UINT32 temp;
	UINT32 Reg_Index=0;
	UINT32 Array_Index=0;
	UINT32 i;
	UINT32 frac_divider = 1000000;

	switch(BWMHz)
	{
	case MxL_BW_6MHz: //6MHz
			SetIRVBit(IRV_RFTune, 0x0C, 0x3F, 0x15);  //set DIG_MODEINDEX, DIG_MODEINDEX_A, and DIG_MODEINDEX_CSF
		break;
	case MxL_BW_7MHz: //7MHz
			SetIRVBit(IRV_RFTune, 0x0C, 0x3F, 0x2A);
		break;
	case MxL_BW_8MHz: //8MHz
			SetIRVBit(IRV_RFTune, 0x0C, 0x3F, 0x3F);
		break;
	}


	//Convert RF frequency into 16 bits => 10 bit integer (MHz) + 6 bit fraction
	dig_rf_freq = RF_Freq / MHz;
	temp = RF_Freq % MHz;
	for(i=0; i<6; i++)
	{
		dig_rf_freq <<= 1;
		frac_divider /=2;
		if(temp > frac_divider)
		{
			temp -= frac_divider;
			dig_rf_freq++;
		}
	}

	//add to have shift center point by 7.8124 kHz
	if(temp > 7812)
		dig_rf_freq ++;
    	
	SetIRVBit(IRV_RFTune, 0x0D, 0xFF, (UINT8)dig_rf_freq);
	SetIRVBit(IRV_RFTune, 0x0E, 0xFF, (UINT8)(dig_rf_freq>>8));

	if (RF_Freq >=333*MHz)
		SetIRVBit(IRV_RFTune, 0x80, 0x40, 0x40);

	//Generate one Array that Contain Data, Address 
	while (IRV_RFTune[Reg_Index].Num || IRV_RFTune[Reg_Index].Val)
	{
		pArray[Array_Index++] = IRV_RFTune[Reg_Index].Num;
		pArray[Array_Index++] = IRV_RFTune[Reg_Index].Val;
		Reg_Index++;
	}
    
	*Array_Size=Array_Index;
	
	return 0;
}

//local functions called by Init and RFTune
UINT32 SetIRVBit(PIRVType pIRV, UINT8 Num, UINT8 Mask, UINT8 Val)
{
	while (pIRV->Num || pIRV->Val)
	{
		if (pIRV->Num==Num)
		{
			pIRV->Val&=~Mask;
			pIRV->Val|=Val;
		}
		pIRV++;

	}	
	return 0;
}

