/*
  Copyright (C) 2006-2009 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          main.c
 *
 *                3
 *
 * \date          %modify_time%
 *
 * \brief         Sample application code of the NXP TDA18250 driver.
 *
 * REFERENCE DOCUMENTS :
 *                TDA18250_Driver_User_Guide.pdf
 *
 * Detailed description may be added here.
 *
 * \section info Change Information
 *
*/

//*--------------------------------------------------------------------------------------
//* Include Standard files
//*--------------------------------------------------------------------------------------
#include "tmCompId.h"

#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"
#include "tmUnitParams.h"

#include "GX1001_V8.2.5.h"
//#include "frontend.h"

#if 1
#define GX1001_printf(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define GX1001_printf(x...)
#endif

//*--------------------------------------------------------------------------------------
//* Include Driver files
//*--------------------------------------------------------------------------------------
#include "tmbslTDA18250A.h"


U16  TDA18250_CHIP_ADDRESS	   = 0xc0;    /*TDA18250 RF chip address*/
//*--------------------------------------------------------------------------------------
//* Prototype of function to be provided by customer
//*--------------------------------------------------------------------------------------
tmErrorCode_t 	UserWrittenI2CRead(tmUnitSelect_t tUnit,UInt32 AddrSize, UInt8* pAddr,UInt32 ReadLen, UInt8* pData);
tmErrorCode_t 	UserWrittenI2CWrite (tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr,UInt32 WriteLen, UInt8* pData);
tmErrorCode_t 	UserWrittenWait(tmUnitSelect_t tUnit, UInt32 tms);
tmErrorCode_t   UserWrittenPrint(UInt32 level, const char* format, ...);
tmErrorCode_t   tmbslTDA18250A_SetXtal(  tmUnitSelect_t  tUnit,      TDA18250AXtalFreq_t eXtal  );
bool I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number);

void GX1001_TDA18250_Init_Chip(void)
{
	/* Variable declarations */
	tmErrorCode_t err = TM_OK;
	tmbslFrontEndDependency_t sSrvTunerFunc;
	static int s_have_init = 0;
	tmUnitSelect_t     TunerUnit = 0;
	u8 Temp_Read_data =0;
	if(0 == s_have_init)
	{
		s_have_init = 1;

		/* Low layer struct set-up to link with user written functions */
		sSrvTunerFunc.sIo.Write = UserWrittenI2CWrite;
		sSrvTunerFunc.sIo.Read = UserWrittenI2CRead;
		sSrvTunerFunc.sTime.Get = Null;
		sSrvTunerFunc.sTime.Wait = UserWrittenWait;
		sSrvTunerFunc.sDebug.Print = UserWrittenPrint;
		sSrvTunerFunc.sMutex.Init = NULL;
		sSrvTunerFunc.sMutex.DeInit = NULL;
		sSrvTunerFunc.sMutex.Acquire = NULL;
		sSrvTunerFunc.sMutex.Release = NULL;
		sSrvTunerFunc.dwAdditionalDataSize = 0;
		sSrvTunerFunc.pAdditionalData = Null;

		GX1001_printf("!!!!!!!!!!GX1001_TDA18250_Init_Chip\n");

		if (SUCCESS == GX1001_Set_Tunner_Repeater_Enable(1))
		{
		//	I2cReadWrite( READ,TDA18250_CHIP_ADDRESS,0x0c,&Temp_Read_data, 1 );
		//	printf("!!!![demod]0x0c ver: 0x%x\n", Temp_Read_data);
		//	Temp_Read_data=((u8)(Temp_Read_data & 0x7F));
		//	printf("!!!![demod]Temp_Read_data ver: 0x%x\n", Temp_Read_data);
		//	I2cReadWrite(WRITE,TDA18250_CHIP_ADDRESS,0x0c,&Temp_Read_data,1);
		//	I2cReadWrite( READ,TDA18250_CHIP_ADDRESS,0x0c,&Temp_Read_data, 1 );
		//	printf("!!!![demod]read 0x0c ver: 0x%x\n", Temp_Read_data);
			
			err = tmbslTDA18250A_Open(TunerUnit, &sSrvTunerFunc);

		    if (err == TM_OK)
		    {
			    err = tmbslTDA18250A_HwInit(TunerUnit);
		    }
			if (err == TM_OK)
			{
				err = tmbslTDA18250A_SetXtal(TunerUnit,TDA18250A_XtalFreq_16000000);
			}
			if (err == TM_OK)
		    {
			    err = tmbslTDA18250A_SetPowerState(TunerUnit, tmPowerOn);
		    }
			if(err != TM_OK)
			{
				GX1001_printf("!!!!!!!!!!GX1001_TDA18250_Init_Chip failed\n");			
			}
			err = tmbslTDA18250A_SetStandardMode(TunerUnit, TDA18250A_QAM_8MHz);
			if(err != TM_OK)
			{
				GX1001_printf("!!!!!!!!!!tmbslTDA18250A_SetStandardMode failed\n");	
			}
			msleep(20);
			GX1001_Set_Tunner_Repeater_Enable(0); 
		}
		else
			GX1001_printf("!!!!!!!!!!!!!GX1001_Set_Tunner_Repeater failed!!!\n");
	}
}



//*--------------------------------------------------------------------------------------
//* Function Name       : Main
//* Object              : Software entry point
//* Input Parameters    : none.
//* Output Parameters   : none.
//*--------------------------------------------------------------------------------------
S32 GX1001_Set_TDA18250_RFFrequency(U32 fvalue)
{	
	tmErrorCode_t err = TM_OK;
	UInt32 uRFMaster = fvalue*1000;
	UInt32 TunerUnit = 0;

	if (SUCCESS == GX1001_Set_Tunner_Repeater_Enable(1))
	{
		err = tmbslTDA18250A_SetRF(TunerUnit, uRFMaster);
		if(err != TM_OK)
		{
			GX1001_printf("!!!!!!!!!!tmbslTDA18250A_SetRF failed\n");
			GX1001_Set_Tunner_Repeater_Enable(0);
		   	return FAILURE;//err;
		}
		else
		{
			GX1001_printf("!!!!!!!!!!tmbslTDA18250A_SetRF successed\n");
			GX1001_Set_Tunner_Repeater_Enable(0);
			return SUCCESS;//err;
		}
	}
//	gx_rtc_delay_ms(50);
//	GX1001_HotReset_CHIP();

	return SUCCESS;

}//* End


/*
Function:   get the signal intensity expressed in percentage
Output:
        The signal Strength value  ( Range is [0,100] )
*/
unsigned char GX1001_Get_TDA18250_Signal_Strength(void)
{
    signed short iAGC1_word=300,iAGC2_word=300,Amp_Value;
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

		//printf("======1111==agc1_temp:%d,  agc2_temp:%d\n", agc1_temp, agc2_temp);

		
		if ((agc1_temp>0)&&(agc2_temp>0))
		{

			//if ((((agc1_temp - iAGC1_word)<5)||((agc1_temp - iAGC1_word)>-5))&&(((agc2_temp - iAGC2_word)<5)||((agc2_temp - iAGC2_word)>-5)))
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

	if (iAGC1_word > 0xe4) iAGC1_word = 0xe4;
	Amp_Value = C0 - ((iAGC1_word-C1)*(A1-A2))/(C2-C1) - ((iAGC2_word-C3)*(A3-A4))/(C4-C3);
	if(Amp_Value<0)
		Amp_Value = 0;
	return GX1001_Change2percent(Amp_Value,0,100);
}         




//*--------------------------------------------------------------------------------------
//* Template of function to be provided by customer
//*--------------------------------------------------------------------------------------

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CRead
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 AddrSize,
//* 						UInt8* pAddr,
//* 						UInt32 ReadLen,
//* 						UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t UserWrittenI2CRead(tmUnitSelect_t tUnit,	UInt32 AddrSize, UInt8* pAddr,
UInt32 ReadLen, UInt8* pData)
{
	#if 0
   u8 UCtmp = FAILURE;
   tmErrorCode_t err = TM_OK;
   //printf("=========READ AddrSize:%d, ReadLen:%d\n", AddrSize, ReadLen);
   if (SUCCESS == GX1001_Set_Tunner_Repeater_Enable(1))	/*open the chip repeater */
	{   
		msleep(10);
		if (SUCCESS == I2cReadWrite( READ,TDA18250_CHIP_ADDRESS,pAddr[0],pData, ReadLen ))	
		{
			msleep(10);
			UCtmp = GX1001_Set_Tunner_Repeater_Enable(0);	/*close the chip repeater*/
		}
   	}

   if (SUCCESS == UCtmp)
	{
		//GX_Delay_N_ms(50);
		//UCtmp = GX1001_HotReset_CHIP();
	}
   else
   	{
   		err = 1;
   	}

	return err;
	#else
	tmErrorCode_t err = TM_OK;
	if (SUCCESS == I2cReadWrite( READ,TDA18250_CHIP_ADDRESS,pAddr[0],pData, ReadLen ))	
	{
		msleep(10);
	}
	return err;
	#endif
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CWrite
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 AddrSize,
//* 						UInt8* pAddr,
//* 						UInt32 WriteLen,
//* 						UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t UserWrittenI2CWrite (tmUnitSelect_t tUnit, 	UInt32 AddrSize, UInt8* pAddr,
UInt32 WriteLen, UInt8* pData)
{
	#if 0
	u8 UCtmp = FAILURE;
	tmErrorCode_t err = TM_OK;
	//printf("=========WRITE AddrSize:%d, WriteLen:%d\n", AddrSize, WriteLen);
	if (SUCCESS == GX1001_Set_Tunner_Repeater_Enable(1))	/*open the chip repeater */
	{   
		msleep(10);
		if (SUCCESS == I2cReadWrite( WRITE,TDA18250_CHIP_ADDRESS ,pAddr[0],pData, WriteLen ))	
		{
			msleep(10);
			UCtmp = GX1001_Set_Tunner_Repeater_Enable(0);	/*close the chip repeater*/
		}
	}

	if (SUCCESS == UCtmp)
	{
		//GX_Delay_N_ms(50);
		//UCtmp = GX1001_HotReset_CHIP();
	}
	else
	{
		err = 1;
	}

	return err;
	#else
	tmErrorCode_t err = TM_OK;
	if (SUCCESS == I2cReadWrite( WRITE,TDA18250_CHIP_ADDRESS ,pAddr[0],pData, WriteLen ))	
	{
		msleep(10);
	}
	return err;
	#endif
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenWait
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 tms
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t UserWrittenWait(tmUnitSelect_t tUnit, UInt32 tms)
{
   tmErrorCode_t err = TM_OK;

	msleep(tms);

   return err;
}


//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenPrint
//* Object              : 
//* Input Parameters    : 	UInt32 level, const char* format, ...
//* 						
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t 			UserWrittenPrint(UInt32 level, const char* format, ...)
{
   /* Variable declarations */
	tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	GX1001_printf("%s", format);

/* ...*/
/* End of Customer code here */

	return err;
}
#if 0
//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexInit
//* Object              : 
//* Input Parameters    : 	ptmbslFrontEndMutexHandle *ppMutexHandle
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexInit(ptmbslFrontEndMutexHandle *ppMutexHandle)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/

/* ...*/
/* End of Customer code here */

   return err;
}


//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexDeInit
//* Object              : 
//* Input Parameters    : 	 ptmbslFrontEndMutexHandle pMutex
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexDeInit( ptmbslFrontEndMutexHandle pMutex)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/

/* ...*/
/* End of Customer code here */

   return err;
}



//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexAcquire
//* Object              : 
//* Input Parameters    : 	ptmbslFrontEndMutexHandle pMutex, UInt32 timeOut
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexAcquire(ptmbslFrontEndMutexHandle pMutex, UInt32 timeOut)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/

/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexRelease
//* Object              : 
//* Input Parameters    : 	ptmbslFrontEndMutexHandle pMutex
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexRelease(ptmbslFrontEndMutexHandle pMutex)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/

/* ...*/
/* End of Customer code here */

   return err;
}
#endif

