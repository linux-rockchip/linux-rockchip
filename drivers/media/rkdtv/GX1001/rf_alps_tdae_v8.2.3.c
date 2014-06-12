//请参考《GX1001P软件包说明-V8.2.doc》
//Please refer to <GX1001 Software Developer Kit User's Manual_V8.2>
/*
Abbreviation
    GX		--	GUOXIN 
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
S32 GX1001_Set_ALPSTDAE3_RFFrequency(U32 fvalue)
{
	u8 UCtmp = FAILURE;
	unsigned char data[6];
	unsigned long freq;

	freq=(fvalue+GX1001_IF_FREQUENCY)*10/625;              /*freq=(fvalue+GX_IF_FREQUENCY)*/
	data[0] = 0xC0;	                                /*Tunner Address*/
	data[1] =(unsigned char)((freq>>8)&0xff);	
	data[2] =(unsigned char)(freq&0xff);	
	data[3] = 0x9B;	/*62.5KHz*/
	data[4] = 0x20;
	data[5] = 0xC6;
		
	if (fvalue < 125000) 
	{
		data[4] |= 0x80;
	}	
	else if ((fvalue >= 125000) && (fvalue<366000)) 
	{
		data[4] |= 0x82;
	}	
	else if ((fvalue >= 366000) && (fvalue<622000)) 
	{
		data[4] |= 0x48;
	}	
	else if ((fvalue >= 622000) && (fvalue<726000)) 
	{
		data[4] |= 0x88;
	}	
	else if (fvalue >= 726000) 
	{
		data[4] |= 0xC8;
	}	

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
U32 GX1001_Get_ALPSTDAE3_Signal_Strength(void)
{
    unsigned short iAGC1_word=300,iAGC2_word=300,Amp_Value;
	unsigned short agc1_temp=0,agc2_temp=0;
    
     //the following parameters are specific for certain tuner
    int C0=95;
    int C1=0xb2,	A1=20;
    int C2=204,		A2=0;
    int C3=0x8c,	A3=20;
    int C4=179,		A4=0;
    //-----------------------------------------------
        
	int i=0;

	while (i<40)
	{
		agc1_temp =GX1001_Read_one_Byte(GX1001_AGC1_CTRL);
		agc2_temp =GX1001_Read_one_Byte(GX1001_AGC2_CTRL);

		
		if ((agc1_temp>0)&&(agc2_temp>0))
		{

			if ((((agc1_temp - iAGC1_word)<5)||((agc1_temp - iAGC1_word)>-5))&&(((agc2_temp - iAGC2_word)<5)||((agc2_temp - iAGC2_word)>-5)))
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

	if (iAGC1_word > 0xe4) iAGC1_word = 0xe4;
	Amp_Value = C0 - ((iAGC1_word-C1)*(A1-A2))/(C2-C1) - ((iAGC2_word-C3)*(A3-A4))/(C4-C3);

	return GX1001_Change2percent(Amp_Value,0,100);
}

