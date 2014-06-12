//请参考《GX10011001P软件包说明-V8.2.5.doc》
//Please refer to <GX10011001 Software Developer Kit User's Manual_V8.2.5>
/*
   Abbreviation
   GX1001		--	GUOXIN 
   IF		--	intermediate frequency
   RF		--  radiate frequency
   SNR		--	signal to noise ratio
   OSC		--	oscillate
   SPEC	--	spectrum
   FREQ	--	frequency
   */
#include "GX1001_V8.2.5.h"
//#include "frontend.h"

bool I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number);

/*================  User-defined GX10011001 software S32erface end ===========*/
/*============================================================================*/
/*============================================================================*/

/*
Function:	Set RF frequency (Unit: KHz)
Input:
fvalue -- RF frequency (Unit: KHz)
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Set_ALPSTDQE3_RFFrequency(U32 fvalue)
{
	S32 UCtmp = FAILURE;
	U8 data[6];
	U32 freq;

	freq=(fvalue+GX1001_IF_FREQUENCY)*10/625;              /*freq=(fvalue+GX1001_IF_FREQUENCY)*/
	data[0] = 0xc0;	                                /*Tunner Address*/
	data[1] =(unsigned char)((freq>>8)&0xff);	
	data[2] =(unsigned char)(freq&0xff);	
	data[3] = 0x8b;	/*62.5KHz*/

	if (fvalue < 153000) 
		data[4] = 0x60;
	else if (fvalue < 430000) 
		data[4] = 0xa2;
	else 
		data[4] = 0xaa; 

	data[5] = 0xc6;

	if (SUCCESS == GX1001_Set_Tunner_Repeater_Enable(1))	/*open the chip repeater */
	{   msleep(10);
		if (SUCCESS == I2cReadWrite( WRITE, data[0], data[1], &data[2], 4 ))	
		{
			msleep(10);
			UCtmp = GX1001_Set_Tunner_Repeater_Enable(0);	/*close the chip repeater*/
		}
	}

	if (SUCCESS == UCtmp)
	{
		msleep(50);
		UCtmp = GX1001_HotReset_CHIP();
	}

	return UCtmp;
}

/*
Function:   get the signal S32ensity expressed in percentage
Output:
The signal Strength value  ( Range is [0,100] )
*/
U32 GX1001_Get_ALPSTDQE3_Signal_Strength(void)
{
	S32 iAGC1_word=300,iAGC2_word=300,Amp_Value;
	S32 agc1_temp=0,agc2_temp=0;

	//the following parameters are specific for certain tuner
	U8 C0=95;
	U8 C1=0xb2,	A1=20;
	U8 C2=204,		A2=0;
	U8 C3=0x8c,	A3=20;
	U8 C4=179,		A4=0;
	//-----------------------------------------------

	U8 i=0;

	while(i<40)
	{
		agc1_temp =GX1001_Read_one_Byte(GX1001_AGC1_CTRL);
		agc2_temp =GX1001_Read_one_Byte(GX1001_AGC2_CTRL);


		if((agc1_temp>0)&&(agc2_temp>0))
		{

			if((abs(agc1_temp - iAGC1_word)<5)&&(abs(agc2_temp - iAGC2_word)<5))
			{
				break; 
			}

			iAGC1_word = agc1_temp;
			iAGC2_word = agc2_temp;
		}

		msleep(10);
		i++;

	}

	if (i>=40) 
	{
		iAGC1_word =GX1001_Read_one_Byte(GX1001_AGC1_CTRL);
		iAGC2_word =GX1001_Read_one_Byte(GX1001_AGC2_CTRL);
	}

	if(iAGC1_word > 0xe4)
	{
		iAGC1_word = 0xe4;
	}
	Amp_Value = C0 - ((iAGC1_word-C1)*(A1-A2))/(C2-C1) - ((iAGC2_word-C3)*(A3-A4))/(C4-C3);

	return GX1001_Change2percent(Amp_Value,0,100);
}

