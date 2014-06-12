/*
 
 Driver APIs for MxL5007 Tuner
 
 Copyright, Maxlinear, Inc.
 All Rights Reserved
 
 File Name:      MxL_User_Define.c

 */
#ifdef IICTUNERLIB
#include "../../Common/AuvitekTunerDev.h"

#else
#include "AuvitekTunerDev.h"

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//																		   //
//					I2C Functions (implement by customer)				   //
//																		   //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
int MxL_I2C_Write_Rest(void * DevHandle,unsigned char DeviceAddr, unsigned char* pArray, int count)
{
    UINT16 status = STATUS_OK;
    int i=0;
    if(I2C_EnableSecondPort(DevHandle,1))
            return STATUS_ERROR;
   // for (i=0; i<count; i+=2)
    {
    	status = AuvitekTuner_WriteTuner(DevHandle, DeviceAddr, (UINT16)pArray[i], &pArray[i+1], 0);
    	if (status != STATUS_OK)
    	{
    		status = 1;
    		//break;
    	}
    }
    if(I2C_EnableSecondPort(DevHandle,0))
            return STATUS_ERROR;
    return status;

}
/******************************************************************************
**
**  Name: MxL_I2C_Write
**
**  Description:    I2C write operations
**
**  Parameters:    	
**					DeviceAddr	- MxL5007 Device address
**					pArray		- Write data array pointer
**					count		- total number of array
**
**  Returns:        0 if success
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
int MxL_I2C_Write(void * DevHandle,unsigned char DeviceAddr, unsigned char* pArray, int count)
{
    UINT16 status = STATUS_OK;
    int i;
    if(I2C_EnableSecondPort(DevHandle,1))
            return STATUS_ERROR;
#if 1//def 1//IICTUNERLIB
    for (i=0; i<count; i+=2)
    {
    	status = AuvitekTuner_WriteTuner(DevHandle, DeviceAddr, (UINT16)pArray[i], &pArray[i+1], 1);
    	if (status != STATUS_OK)
    	{
    		status = 1;
    		break;
    	}
    }
    for (i=0; i<count; i+=2)
    {
    	status = AuvitekTuner_ReadTuner(DevHandle, DeviceAddr, (UINT16)pArray[i], &pArray[i+1], 1);
    	if (status != STATUS_OK)
    	{
    		status = 1;
    		break;
    	}
    }

#else
    if(count<2)
    {
    	status=AuvitekTuner_WriteTuner(
    		DevHandle,
    		(UINT16)DeviceAddr,
    		0,
    		(pArray),
    		2);	
    }
    else
    {
    	
    	for (i=0; i<count; i+=2)
    	{
    		status = AuvitekTuner_WriteTuner(
    			DevHandle, 
    			(UINT16)DeviceAddr,
    			0,
    			(pArray+i),
    			2);	
    		if (status != STATUS_OK)
    		{
    			status = 1;
    			break;
    		}
    	}
    }
#endif
    if(I2C_EnableSecondPort(DevHandle,0))
            return STATUS_ERROR;
    return status;
}

/******************************************************************************
**
**  Name: MxL_I2C_Read
**
**  Description:    I2C read operations
**
**  Parameters:    	
**					DeviceAddr	- MxL5007 Device address
**					Addr		- register address for read
**
**  Returns:        Register Data in unsigned char data type
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
unsigned char MxL_I2C_Read(void * DevHandle,UINT8 DeviceAddr, UINT8 Addr, UINT8* mData)
{
	UINT8 Data_Read = 0;

	AuvitekTuner_ReadTuner(DevHandle, DeviceAddr, Addr, &Data_Read, 1);

	return Data_Read;
}

/******************************************************************************
**
**  Name: MxL_Delay
**
**  Description:    Delay function in milli-second
**
**  Parameters:    	
**					mSec		- milli-second to delay
**
**  Returns:        0
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
int MxL_Delay(int mSec)
{
	AuvitekTuner_TimeDelay((UINT16)mSec);
	return 0;
}
