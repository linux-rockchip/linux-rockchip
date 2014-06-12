//请参考《GX1001P软件包说明-V8.2.5.doc》
//Please refer to <GX1001 Software Developer Kit User's Manual_V8.2.5>
/*
   Abbreviation
   GX1001		--	GUOXIN 
   IF		--	S32ermediate frequency
   RF		--  radiate frequency
   SNR		--	signal to noise ratio
   OSC		--	oscillate
   SPEC	--	spectrum
   FREQ	--	frequency
   */
#include "GX1001_V8.2.5.h"

#if 1
#define GX1001_printf(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define GX1001_printf(x...)
#endif

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

bool I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number);
S32 GX1001_Set_ALPSTDAE3_RFFrequency(U32 fvalue);

/* 
Function: Write one byte to chip
Input:
RegAdddress -- The register address
WriteValue  -- The write value
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Write_one_Byte(U8 RegAddress,U8 WriteValue)
{
	S32 UCtmp=FAILURE;
	U8 data[2];
	U8 ChipAddress ;

	ChipAddress  = GX1001_CHIP_ADDRESS;

	data[0] = RegAddress;
	data[1] = WriteValue; 

	UCtmp = I2cReadWrite( WRITE, ChipAddress, data[0], &data[1], 1 );

	if (SUCCESS == UCtmp)//ok
	{
		if ((WriteValue&0xff) == (GX1001_Read_one_Byte(RegAddress)&0xff))
			return SUCCESS;
	}
	return FAILURE;
}


/* 
Function: Write one byte to chip with no read test 
Input:
RegAdddress -- The register address
WriteValue  -- The write value
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Write_one_Byte_NoReadTest(U8 RegAddress,U8 WriteValue)
{
	S32 UCtmp=FAILURE;
	U8 data[2];
	U8 ChipAddress ;

	ChipAddress  = GX1001_CHIP_ADDRESS;

	data[0] = RegAddress;
	data[1] = WriteValue; 

	UCtmp = I2cReadWrite( WRITE, ChipAddress,data[0], &data[1], 1 );

	return UCtmp;
}



/* 
Function: Read one byte from chip
Input:
RegAdddress -- The register address
Output:
success: Read value
FAILURE:  FAILURE 
*/
S32 GX1001_Read_one_Byte(U8 RegAddress)
{
	S32 UCtmp=FAILURE;
	U8 ChipAddress;
	U8 Temp_RegAddress=0;
	U8 Temp_Read_data =0;

	Temp_RegAddress = RegAddress;
	ChipAddress=GX1001_CHIP_ADDRESS;

	UCtmp = I2cReadWrite(READ, ChipAddress,Temp_RegAddress, &Temp_Read_data, 1 );

	if (SUCCESS == UCtmp)//ok
		return (Temp_Read_data&0xff) ;
	else
		return FAILURE;
}



/*
Function: get  the version of chip 
Input:None
Output:
NEWONE--- new version
OLDONE--- old version 
*/
S32 GX1001_Get_Version(void)
{
	S32 temp1=0,temp2=0;
	temp1 = GX1001_Read_one_Byte(GX1001_CHIP_IDD);
	temp2 = GX1001_Read_one_Byte(GX1001_CHIP_VERSION);
	if((temp1==0x01)&&((temp2==0x02)||(temp2==0x82)))
	{
		return NEWONE;
	}
	else
	{
		return OLDONE;
	}
}




/*
Function: Set Mode Scan Mode
Input: mode		--0:disable mode scan
1:enable mode scan
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Set_Mode_Scan(U8 mode)
{
	S32 temp;

	temp=GX1001_Read_one_Byte(GX1001_MODE_SCAN_ENA);

	if( 0==mode)
		temp&=0x7f;
	else
		temp|=0x80;

	if(GX1001_Write_one_Byte(GX1001_MODE_SCAN_ENA,temp))
		return SUCCESS;
	else
		return FAILURE;
}



/*
Function: Set FM Mode
Input:mode   ---0:disable FM
1:enable FM
Output:
SUCCESS or FAILURE

*/
S32 GX1001_Set_FM(U8 mode)
{
	S32 temp;
	temp=GX1001_Read_one_Byte(GX1001_FM_CANCEL_CTRL);
	if( 0==mode )
		temp&=0xef;
	else
		temp|=0x10;
	if(GX1001_Write_one_Byte(GX1001_FM_CANCEL_CTRL,temp))
		return SUCCESS;
	else
		return FAILURE;

}


/*
Function: Set SF Mode
Input: ----0:disable SF
1:enable SF
Output:
SUCCESS or FAILURE

*/
S32 GX1001_Set_SF(U8 mode)
{
	S32 temp;
	temp=GX1001_Read_one_Byte(GX1001_SF_CANCEL_CTRL);
	if(0==mode)
		temp&=0xef;
	else
		temp|=0x10;
	if(GX1001_Write_one_Byte(GX1001_SF_CANCEL_CTRL,temp))
		return SUCCESS;
	else
		return FAILURE;

}


/*
Function:Set the Minimum of RF AGC Controller
Input: --------S32 value:the minimum value
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Set_RF_Min(U8 value)
{
	if(GX1001_Write_one_Byte(GX1001_RF_MIN,value))
		return SUCCESS;
	else
		return FAILURE;
}


/*
Function:Set the Maximum value of RF AGC Controller
Input:-------S32 value :the maximum value
Output:
SUCCESS or FAILURE

*/
S32 GX1001_Set_RF_Max(U8 value)
{
	if(GX1001_Write_one_Byte(GX1001_RF_MAX,value))
		return SUCCESS;
	else
		return FAILURE;
}


/*
Function:Set the Minimum of IF AGC Controller
Input:---------S32 value :the maximum value
Output:
SUCCESS or FAILURE

*/
S32 GX1001_set_IF_Min(U8 value)
{
	if(GX1001_Write_one_Byte(GX1001_IF_MIN,value))
		return SUCCESS;
	else
		return FAILURE;
}

/*
Funtion:Set the Maximum value of IF AGC Controller
Input:---------S32 value: the Maximum value
Output:
SUCCESS or FAILURE

*/
S32 GX1001_set_IF_Max(U8 value)
{
	if(GX1001_Write_one_Byte(GX1001_IF_MAX,value))
		return SUCCESS;
	else
		return FAILURE;
}

/*
Function: Set  IF ThresHold Auto Adjust 
Input:--------0:disable
1:enable
Output:
SUCCESS or FAILURE

*/
S32 GX1001_SET_AUTO_IF_THRES(U8 mode)
{
	S32 temp;
	temp=GX1001_Read_one_Byte(GX1001_AUTO_THRESH);
	if(0==mode)
		temp&=0x7f;
	else
		temp|=0x80;
	if(GX1001_Write_one_Byte(GX1001_AUTO_THRESH,temp))
		return SUCCESS;
	else 
		return FAILURE;
}



/*
Function: Set Tim Scan
Input: -------- 0:disable
1:enable
Output:
SUCCESS or FAILURE

*/
S32 GX1001_Set_Tim_Scan(U8 mode)
{
	S32 temp;
	temp = GX1001_Read_one_Byte(GX1001_TIM_SCAN_ENA);
	if(0==mode)
		temp&=0x7f;
	else
		temp|=0x80;
	if(GX1001_Write_one_Byte(GX1001_TIM_SCAN_ENA,temp))
		return SUCCESS;
	else
		return FAILURE;
}


/*
Function: Set Digital AGC
Input: -------- 0:disable
1:enable
Output:
SUCCESS or FAILURE

*/
S32 GX1001_Set_Digital_AGC(U8 mode)
{
	S32 temp;
	temp = GX1001_Read_one_Byte(GX1001_DIGITAL_AGC_ON);
	if(0==mode)
		temp&=0x7f;
	else
		temp|=0x80;
	if(GX1001_Write_one_Byte(GX1001_DIGITAL_AGC_ON,temp))
		return SUCCESS;
	else
		return FAILURE;
}
/*
Funtion:	Set chip wake up
Input:
Sleep_Enable    --    1: Sleep
0: Working
Output:
SUCCESS or FAILURE

*/
U32 GX1001_Set_Sleep(U8 Sleep_Enable)
{
	S32 UCtmp1 = FAILURE,UCtmp2 = FAILURE;

	if(NEWONE==GX1001_Get_Version())
	{	
		S32 temp1=0,temp2=0;
		//		S32 UCtmp1 = FAILURE,UCtmp2 = FAILURE;
		temp1=GX1001_Read_one_Byte(GX1001_MAN_PARA);	/*0x10 - bit2*/
		temp2=GX1001_Read_one_Byte(0x14);/*0x14 - - bit4-bit5*/
		if ((temp1!= FAILURE)&&(temp2!= FAILURE))
		{
			temp1 &=0xfb;
			temp2 &=0xcf;

			temp1 |= 0x04&(Sleep_Enable<<2);
			temp2 |= 0x10&(Sleep_Enable<<4);
			temp2 |= 0x20&(Sleep_Enable<<5);

			UCtmp1 = GX1001_Write_one_Byte(GX1001_MAN_PARA,temp1);
			UCtmp2 = GX1001_Write_one_Byte(0x14,temp2);

			if ((SUCCESS == UCtmp1)&&(SUCCESS == UCtmp2))
			{
				if (0==Sleep_Enable )
				{
					UCtmp1 =GX1001_HotReset_CHIP();
				}
			}
		}
		//		return UCtmp1&&UCtmp2;
	}
	return UCtmp1&&UCtmp2;
}


/*
Function:  Init the GX1001
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Init_Chip(fe_tunertype_t  tunertype)
{ 
	S32 UCtmp=FAILURE;
	
        GX1001_printf("!!!!!!!!!!!!! into GX1001_Init_Chip<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
//while(1)
{        
	UCtmp = GX1001_Set_AGC_Parameter(tunertype); //Set AGC parameter
	//msleep(100);
}	
	if (SUCCESS == UCtmp)
	{
            GX1001_printf("!!!!!!!!!!!!! GX1001_Init_Chip GX1001_Set_AGC_Parameter success <<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        }
        else
	{
            GX1001_printf("!!!!!!!!!!!!! GX1001_Init_Chip GX1001_Set_AGC_Parameter failed<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        }
                
	if (SUCCESS == UCtmp)
	{
		UCtmp = GX1001_SetOSCFreq();                        /* set crystal frequency */  

		if (SUCCESS == UCtmp)
		{
			UCtmp = GX1001_Set_OutputMode(GX1001_TS_OUTPUT_MODE);    /* set the TS output mode */
		}		
	}	
	if( ( NEWONE == GX1001_Get_Version())&&( SUCCESS == UCtmp ) )
	{

		UCtmp = GX1001_Set_Digital_AGC(ENABLE);
		if(UCtmp==FAILURE)return UCtmp;
		UCtmp = GX1001_Set_RF_Min(GX1001_RF_AGC_MIN_VALUE);
		if(UCtmp==FAILURE)return UCtmp;
		UCtmp = GX1001_Set_RF_Max(GX1001_RF_AGC_MAX_VALUE);	
		if(UCtmp==FAILURE)return UCtmp;
		UCtmp = GX1001_set_IF_Min(GX1001_IF_AGC_MIN_VALUE);
		if(UCtmp==FAILURE)return UCtmp;
		UCtmp = GX1001_set_IF_Max(GX1001_IF_AGC_MAX_VALUE);
		if(UCtmp==FAILURE)return UCtmp;
		UCtmp = GX1001_Set_Pll_Value(GX1001_PLL_M_VALUE,GX1001_PLL_N_VALUE,GX1001_PLL_L_VALUE);	
		if(UCtmp==FAILURE)return UCtmp;
	}

	return UCtmp;
}



/*
Function: Set Pll Value
Input:
Pll_M_Value;
Pll_N_Value;
Pll_L_Value;
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Set_Pll_Value(U8 Pll_M_Value,U8 Pll_N_Value,U8 Pll_L_Value)
{
	if(SUCCESS == GX1001_Write_one_Byte(GX1001_PLL_M,Pll_M_Value))
	{
		if(SUCCESS == GX1001_Write_one_Byte(GX1001_PLL_L,((Pll_N_Value<<4)&0xf0)|(Pll_L_Value&0x0f)))
			return SUCCESS;	
		else
			return FAILURE;
	}
	else
		return FAILURE;

}

/*
Function:
*/
/*
Function:	Search signal with setted parameters
Input:
Symbol_Rate_1   --  Used first symbol rate value (range: 450 -- 9000)     (Unit: kHz)
Symbol_Rate_2	--  Used second symbol rate value. Please set 0 if no use	(Unit: kHz)

Spec_Mode	    --	0：search only positive spectrum
1：search only negative spectrum
2：first positive and then negative spectrum
3：first negative and then positive spectrum

Qam_Size	    --  0-2 = reserved;
3 = 16QAM;
4 = 32QAM; 
5 = 64QAM; 
6 = 128QAM;
7 = 256QAM.

RF_Freq		    --  The RF frequency (KHz)

Wait_OK_X_ms    --  The waiting time before give up one search ( Unit: ms )
(Range: 250ms -- 2000ms, Recommend: 700ms)


Output:
SUCCESS --  Have signal
FAILURE	--  No signal
*/
S32 GX1001_Search_Signal(  U32 Symbol_Rate_1,
		U32 Symbol_Rate_2,
		U8 Spec_Mode,
		U8 Qam_Size,
		U32 RF_Freq,
		U32 Wait_OK_X_ms, fe_tunertype_t tunertype)
{
	U32 After_EQU_OK_Delay	= 60;	//60 ms,  
	U8 spec_invert_enable	= 0;	//spec invert enable flag
	U8 spec_invert_value	= 0;	//next spec invert value
	U8 symbol_2_enable		= 0;	//Symbol_Rate_2 enable flag

	U32 wait_ok_x_ms_temp	= 0;    //for save Wait_OK_X_ms
	U32 wait_ok_SF_temp=0;              // wait for lock SF 
	U8 GX1001Bflag=0;
	//-----------------------------------------------------------------------------
        DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
  
	GX1001_CoolReset_CHIP();
	if (FAILURE == GX1001_Init_Chip(tunertype)) return FAILURE;
        DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
	//-----------------------------------------------------------------------------
	wait_ok_x_ms_temp = Wait_OK_X_ms/10;	//as 700 ms = 70 * 10ms_Delay

	if (FAILURE == GX1001_Select_DVB_QAM_Size(Qam_Size)) return FAILURE;	//Set QAM size

	if (FAILURE == GX1001_SetSymbolRate(Symbol_Rate_1)) return FAILURE;	//Set Symbol rate value

	switch(tunertype)
	{
		case TUNER_ALPSTDQE3:
			{
				        DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
				GX1001_Set_ALPSTDQE3_RFFrequency(RF_Freq);		//Set tuner frequency  shipei100621
				break;
			}
        case TUNER_ALPSTDAE:
                DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
            GX1001_Set_ALPSTDAE3_RFFrequency(RF_Freq);
            break;
		case TUNER_USER1://TUNER_TDA18250
		        DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
			GX1001_Set_TDA18250_RFFrequency(RF_Freq);
			break;
		default:
			return FAILURE;

	}


	if (Symbol_Rate_2 >= 4500) symbol_2_enable = 1;	//Symbol_Rate_2 enable

	if (Symbol_Rate_1<2500) After_EQU_OK_Delay = 100;   //100ms   (if  <2.5M  = 100ms)

	//-----------------------------------------------------------------------------
	if(NEWONE == GX1001_Get_Version())     //the chip version is GX1001B
	{
		GX1001Bflag=1;
		Spec_Mode=0;
	}

SYMBOL_2_SEARCH:
	switch (Spec_Mode)
	{
		case 3:	// first negative and then positive
			{
				spec_invert_enable = 1;
				spec_invert_value  = 0;	//next spec invert value
			}
		case 1:	//negative
			{
				GX1001_SetSpecInvert(1);
			}
			break;
		case 2:// first positive and then negative
			{
				spec_invert_enable = 1;
				spec_invert_value  = 1;	//next spec invert value
			}
		default://positive
			{
				GX1001_SetSpecInvert(0);
			}
			break;
	}
	//-----------------------------------------------------------------------------

SPEC_INVERT_SEARCH:	
	if (FAILURE == GX1001_HotReset_CHIP()) return FAILURE;

	wait_ok_x_ms_temp = Wait_OK_X_ms/10;	//as 700 ms = 70 * 10ms_Delay

	while ((FAILURE == GX1001_Read_EQU_OK()) && (wait_ok_x_ms_temp))
	{
		wait_ok_x_ms_temp --;
		msleep(10);		//Delay 10 ms
	}

	if ( 0 == wait_ok_x_ms_temp)           //Read EQU time over
	{    
		if(GX1001Bflag==1&&SFenable==ENABLE)     //the chip version is GX1001B
		{  GX1001_Set_SF(ENABLE);
			GX1001_Set_FM(ENABLE);
			GX1001_HotReset_CHIP();
			wait_ok_SF_temp=80;
			while ((FAILURE == GX1001_Read_Lock_Status(0, 0)) && (wait_ok_SF_temp))
			{
				wait_ok_SF_temp --;
				msleep(20);		//Delay 20 ms	
			}
			if(SUCCESS==GX1001_Read_Lock_Status(0, 0))    //SUCCESS while open SF&FM
			{ 
				return SUCCESS;
			}
		}
		else if (symbol_2_enable)
		{
			symbol_2_enable = 0;
			if (Symbol_Rate_2<25000) 
				After_EQU_OK_Delay = 100;   //100ms
			else
				After_EQU_OK_Delay = 60;   //60ms
			GX1001_SetSymbolRate(Symbol_Rate_2) ;
			if(GX1001Bflag==1)
			{GX1001_Set_SF(DISABLE);
				GX1001_Set_FM(DISABLE);
			}
			goto SYMBOL_2_SEARCH;
		}
		else
			return FAILURE;
	}	

	msleep(After_EQU_OK_Delay);		//Delay After_EQU_OK_Delay ms

	if (SUCCESS == GX1001_Read_Lock_Status(0, 0))	//All ok
	{
		if(GX1001Bflag==1&&FMenable==ENABLE)     //the chip version is GX1001B
			GX1001_Set_FM(ENABLE);                 //open FM for GX1001B
		return SUCCESS;
	}
	else
	{
		if (spec_invert_enable)
		{
			spec_invert_enable = 0;				//disable spec invert
			if (FAILURE == GX1001_SetSpecInvert(spec_invert_value))  return FAILURE;	//spec invert
			else
				goto SPEC_INVERT_SEARCH;
		}
		else 
			return FAILURE;

	}
}

//========================================================================================================================

/*
Function: Set TS output mode
Input:
0 - Serial
1 - Parallel
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Set_OutputMode(U8 mode)
{
	S32 temp=0;
	S32 UCtmp = FAILURE;
	temp=GX1001_Read_one_Byte(GX1001_OUT_FORMAT);	/*0x90 - bit6*/

	if (temp != FAILURE)
	{
		temp &= 0xbf;
		if (mode) temp+=0x40;

		UCtmp = GX1001_Write_one_Byte(GX1001_OUT_FORMAT,temp);
	}
	return UCtmp;
}



/* 
Function: Select QAM size (4 - 256), only for DVB.
Input:
size  --  0-2 = reserved;
3 = 16QAM;
4 = 32QAM; 
5 = 64QAM; 
6 = 128QAM;
7 = 256QAM.
utput:
SUCCESS or FAILURE
*/
S32 GX1001_Select_DVB_QAM_Size(U8 size)
{
	S32 temp=0;
	S32 UCtmp = FAILURE;

	if ((size>7)||(size<=2)) size = 5;
	size<<=5;

	temp=GX1001_Read_one_Byte(GX1001_MODE_AGC);

	if (temp != FAILURE)
	{
		temp &= 0x1f;
		temp += size;
		UCtmp = GX1001_Write_one_Byte(GX1001_MODE_AGC,temp);  /*0x20 - bit7:5   */
	}
	return UCtmp;
}



/* 
Function: Set symbol rate 
Input:
Symbol_Rate_Value :  The range is from 450 to 9000	(Unit: kHz)

Output:
SUCCESS or FAILURE
*/
S32 GX1001_SetSymbolRate(U32 Symbol_Rate_Value)
{
	S32 UCtmp = FAILURE;
	U32 temp_value=0;

	temp_value = Symbol_Rate_Value*1000;        

	UCtmp =	GX1001_Write_one_Byte(GX1001_SYMB_RATE_H,((S32)((temp_value>>16)&0xff)));	/*0x43*/

	if (SUCCESS == UCtmp)
	{
		UCtmp = GX1001_Write_one_Byte(GX1001_SYMB_RATE_M,((S32)((temp_value>>8)&0xff)));	/*0x44*/

		if (SUCCESS == UCtmp)
		{
			UCtmp = GX1001_Write_one_Byte(GX1001_SYMB_RATE_L,((S32)( temp_value&0xff))); /*0x45*/
		}
	}
	return UCtmp;
}


/*
Function: Set oscillate frequancy 
Output:
SUCCESS or FAILURE 
*/
S32 GX1001_SetOSCFreq(void)
{
	S32 UCtmp = FAILURE;
	U32 temp=0;
	U32 OSC_frequancy_Value =0; 
        DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
	if( NEWONE==GX1001_Get_Version() )	
	{	
		OSC_frequancy_Value=GX1001_OSCILLATE_FREQ*( GX1001_PLL_M_VALUE+1)/((GX1001_PLL_N_VALUE+1)*(GX1001_PLL_L_VALUE+1))/2;
	}
	else
	{
		OSC_frequancy_Value = GX1001_OSCILLATE_FREQ;       // KHz
	}   
	temp=OSC_frequancy_Value*250;  
        DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
	UCtmp =GX1001_Write_one_Byte(GX1001_FSAMPLE_H,((S32)((temp>>16)&0xff)));       //0x40
        DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
	if (SUCCESS == UCtmp)
	{
		DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
		UCtmp = GX1001_Write_one_Byte(GX1001_FSAMPLE_M,((S32)((temp>>8)&0xff)));   //0x41

		if (SUCCESS == UCtmp)
		{
			UCtmp = GX1001_Write_one_Byte(GX1001_FSAMPLE_L,((S32)( temp&0xff)));   //0x42
		}
	}
	DBG("TSTV:GX1001: ------ \t%s[%8d]\n", __FUNCTION__,	 __LINE__);
	return UCtmp;
}



/* 
Function: Hot reset the Chip 
Output:
SUCCESS or FAILURE 
*/
S32 GX1001_HotReset_CHIP(void)
{
	S32 UCtmp = FAILURE;
	S32 temp;

	temp=GX1001_Read_one_Byte(GX1001_MAN_PARA);

	if (temp != FAILURE)
	{
		temp|=0x02;
		UCtmp = GX1001_Write_one_Byte_NoReadTest(GX1001_MAN_PARA,temp);
	}

	return UCtmp;
}



/* 
Function: Cool reset the Chip 
Output:
SUCCESS or FAILURE 
*/
S32 GX1001_CoolReset_CHIP(void)
{
	S32 UCtmp = FAILURE;
	S32 temp;

	temp=GX1001_Read_one_Byte(GX1001_MAN_PARA);

	if (temp != FAILURE)
	{
		temp|=0x08;
		UCtmp = GX1001_Write_one_Byte_NoReadTest(GX1001_MAN_PARA,temp);
	}

	return UCtmp;
}



/* 
Function: Read EQU OK
Output:
SUCCESS - EQU OK, FAILURE - EQU Fail
*/
S32 GX1001_Read_EQU_OK(void)
{
	S32 Read_temp=0;

	Read_temp=GX1001_Read_one_Byte(S32_IND);         /*0x13*/

	if (Read_temp != FAILURE)
	{
		if ((Read_temp&0xe0)==0xe0)                     
			return SUCCESS;
	}
	return FAILURE;
}

/* 
Function: Read ALL OK
Output:
SUCCESS - all ok, FAILURE - not all ok 
*/
U32 GX1001_Read_Lock_Status(u32 DelayTime,u8 MaskBit)
{
	S32 Read_temp=0;

	Read_temp=GX1001_Read_one_Byte(S32_IND);         /*0x13*/

	if (Read_temp != FAILURE)
	{
		if ((Read_temp&0xf1)==0xf1)                 /*DVB-C : 0xF1*/
			return SUCCESS;
	}
	return FAILURE;
}


/* Function: Enable/Disable the Tunner repeater
Input:	
1 - On
0 - Off
Output:
SUCCESS or FAILURE 
*/
S32 GX1001_Set_Tunner_Repeater_Enable(U8 OnOff)
{
	S32 UCtmp = FAILURE;
	S32 Read_temp;

	Read_temp=GX1001_Read_one_Byte(GX1001_MAN_PARA);

	if (Read_temp != FAILURE)
	{
		if(OnOff)
		{
			Read_temp|=0x40;        /*Open*/
		}
		else
		{
			Read_temp&=0xbf;        /*Close*/
		}

		UCtmp = GX1001_Write_one_Byte(GX1001_MAN_PARA,Read_temp);
	}

	return UCtmp;
}



//==============================================================================================

/*
Function:   convert a S32eger to percentage ranging form 0% to 100%  
Input:
value - S32eger
low   - lower limit of input,corresponding to 0%  .if value <= low ,return 0
high  - upper limit of input,corresponding to 100%.if value >= high,return 100
Output:
0~100 - percentage
*/
//U32 GX1001_Change2percent(U8 value,U8 low,U8 high)
U32 GX1001_Change2percent(U32 value,U32 low,U32 high) //zhangxf modify 20100426
{
	U32 temp=0;
	if (value<=low) return 0;
	if (value>=high) return 100;
	temp = (unsigned char)((value-low)*100/(high-low));
	return temp;
}


/* 
Function:   100LogN calculating function 
Output:
= 100LogN
*/
U32 GX1001_100Log(U32 iNumber_N)
{
	S32 iLeftMoveCount_M=0;
	S32 iChangeN_Y=0;
	S32 iBuMaY_X=0;
	S32 iReturn_value=0;
	long iTemp=0,iResult=0,k=0;

	iChangeN_Y=iNumber_N;

	for (iLeftMoveCount_M=0;iLeftMoveCount_M<16;iLeftMoveCount_M++)
	{
		if ((iChangeN_Y&0x8000)==0x8000)
			break;
		else
		{
			iChangeN_Y=iNumber_N<<iLeftMoveCount_M;
		}
	}

	iBuMaY_X=0x10000-iChangeN_Y;	//get 2's complement

	k=(long)iBuMaY_X*10000/65536;

	//iTemp= k+(1/2)*(k*k)+(1/3)*(k*k*k)+(1/4)*(k*k*k*k)
	iTemp = k + (k*k)/20000 + ((k*k/10000)*(k*33/100))/10000 + ((k*k/100000)*(k*k/100000))/400;

	//iResult=4.816480-(iTemp/2.302585);
	iResult=48165-(iTemp*10000/23025);	//4.8165 = lg2^16

	k=iResult-3010*(iLeftMoveCount_M-1);

	iReturn_value=(k/100);   //magnify logN by 100 times

	return iReturn_value;
}       



/*
Function : get the signal quality expressed in percentage
Output:
The SNR value (range is [0,100])   ( 0 express SNR = 5 dB ,  100  express SNR = 35 dB )
*/
U32  GX1001_Get_SNR(void)
{       
	U32 S_N_value=0,read_temp=0;
	S32 read_temp1=0;
	S32 read_temp2=0;

	if (GX1001_Read_Lock_Status(0, 0)==SUCCESS)
	{
		read_temp1 =( GX1001_Read_one_Byte(GX1001_SUM_ERR_POW_L)&0xff);
		read_temp2 =( GX1001_Read_one_Byte(GX1001_SUM_ERR_POW_H)&0xff);
		if ((read_temp1>0)||(read_temp2>0))
		{
			read_temp = read_temp1 + (read_temp2<<8);       //SN= 49.3-10log(read_temp) 
			S_N_value = 493 - GX1001_100Log(read_temp);         //magnifid by 10 times
			return GX1001_Change2percent(S_N_value,50,350);	
		}
	}
	return 0;
}       

//========================================================================================================================

/*
Function: Set spectrum invert
Input:   
Spec_invert         : 1 - Yes, 0 - No.
Output:SUCCESS or FAILURE 
*/
S32 GX1001_SetSpecInvert(U8 Spec_invert)
{
	S32 write_value=0;
	U32 OSC_frequancy_Value=0;
	U32 Carrier_center			=	GX1001_IF_FREQUENCY;       

	if( NEWONE==GX1001_Get_Version() )	
	{	
		OSC_frequancy_Value=GX1001_OSCILLATE_FREQ*( GX1001_PLL_M_VALUE+1)/((GX1001_PLL_N_VALUE+1)*(GX1001_PLL_L_VALUE+1))/2;
	}
	else
	{
		OSC_frequancy_Value = GX1001_OSCILLATE_FREQ;       // KHz
	}   

	if (Carrier_center<OSC_frequancy_Value)
	{
		if (Spec_invert)
			write_value=(S32)(((OSC_frequancy_Value-Carrier_center)*1000)/1024);
		else
			write_value=(S32)((Carrier_center*1000)/1024);
	}
	else
	{
		if (Spec_invert)
			write_value=(S32)((((2*OSC_frequancy_Value-Carrier_center)*1000)/1024));
		else
			write_value=(S32)(((Carrier_center-OSC_frequancy_Value)*1000)/1024);
	}

	if (SUCCESS == GX1001_Write_one_Byte(GX1001_DCO_CENTER_H,(((write_value>>8)&0xff))))           //0x26
	{
		if (SUCCESS ==	GX1001_Write_one_Byte(GX1001_DCO_CENTER_L,(( write_value&0xff))))          //0x27
			return SUCCESS;
	}
	return FAILURE;
}


//========================================================================================================================


/*
Function: 	Get Error Rate value 
Input:		
 *E_param: for get the exponent of E
Output:
FAILURE:    Read Error
other: 		Error Rate Value

Example:	if  return value = 456 and  E_param = 3 ,then here means the Error Rate value is : 4.56 E-3
*/
U32 GX1001_Get_ErrorRate(U32 *E_param)
{
	S32 flag = 0;
	S32 e_value = 0;
	S32 return_value = 0;
	S32 temp=0;
	U8 Read_Value[4];
	U32 Error_count=0;
	U8 i=0;
	unsigned long divied = 53477376;	//(2^20 * 51)

	*E_param = 0;

	if (GX1001_Read_Lock_Status(0, 0) == FAILURE)
	{
		*E_param = 0;
		return 1;
	}

	for (i=0;i<4;i++)	//Read Error Count
	{
		flag = GX1001_Read_one_Byte(GX1001_ERR_SUM_1 + i);
		if (FAILURE == flag)
		{
			*E_param = 0;
			return 1;
		}
		else
		{
			Read_Value[i] = (unsigned char)flag;
		}
	}
	Read_Value[3] &= 0x03;
	Error_count = (unsigned long)(Read_Value[0]+ (Read_Value[1]<<8) + (Read_Value[2]<<16) + (Read_Value[3]<<24));

	//ERROR RATE = (ERROR COUNT) /（ ( 2^(2* 5 +5))*204*8）    //bit      

	for (i=0;i<20;i++)
	{
		temp = Error_count/divied;

		if (temp)
		{
			return_value = Error_count/(divied/100);
			break;
		}
		else
		{
			e_value +=1;
			Error_count *=10;
		}
	}
	*E_param = e_value;
	return return_value;
}

/*
Function:   Set AGC parameter
Output:
SUCCESS or FAILURE
*/
S32 GX1001_Set_AGC_Parameter(fe_tunertype_t  tunertype)			//DINGFB 20100622
{
	S32 temp=0;

	switch(tunertype)  //DINGFB 20100622
	{ 	case TUNER_ALPSTDQE3:
        case TUNER_ALPSTDAE:
		case TUNER_USER1://TUNER_TDA18250	
		GX1001_Write_one_Byte(GX1001_AGC_STD,23);
		break;
		default :
		GX1001_printf("%s():  property or size error!\n", __func__);
		return -1;
	}

	temp=GX1001_Read_one_Byte(GX1001_MODE_AGC);
	if(temp!=FAILURE)
	{
		temp&=0xe6;
		temp=temp+0x10;
		temp=GX1001_Write_one_Byte(GX1001_MODE_AGC,temp);

	}		
//	gx_rtc_delay_ms(100);
	return temp;
}

S32 GX_Read_ALL_OK(void)
{
	int Read_temp=0;

	Read_temp=GX1001_Read_one_Byte(S32_IND);         /*0x13*/

	if (Read_temp != FAILURE)
	{
		if ((Read_temp&0xf1)==0xf1)                 /*DVB-C : 0xF1*/
			return SUCCESS;
	}
	return FAILURE;
}


U32 GX1001_Get_Signal_Quality(void)
{
	int S_N_value=0,read_temp=0;
	int read_temp1=0;
	int read_temp2=0;

	if (GX_Read_ALL_OK()==SUCCESS)
	{
		read_temp1 =( GX1001_Read_one_Byte(GX1001_SUM_ERR_POW_L)&0xff);
		read_temp2 =( GX1001_Read_one_Byte(GX1001_SUM_ERR_POW_H)&0xff);
		if ((read_temp1>0)||(read_temp2>0))
		{
			read_temp = read_temp1 + (read_temp2<<8);       //SN= 49.3-10log(read_temp) 
			S_N_value = 493 - GX1001_100Log(read_temp);         //magnifid by 10 times
			return GX1001_Change2percent(S_N_value,50,350);	
		}
	}
	return 0;
}


