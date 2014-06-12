/**************************************************************************************

 *  CONFIDENTIAL AND PROPRIETARY SOFTWARE/DATA OF Auvitek International, Ltd.         * 

 *                                                                                    *

 *  Copyright (c) 2005-2018 Auvitek International, Ltd.  All Rights Reserved.         *

 *                                                                                    *

 *  Use of this Software/Data is subject to the terms and conditions of               *

 *  the applicable agreement between Auvitek International, Ltd, and receiving party. *

 *  In addition, this Software/Data is protected by copyright law and international   *

 *  treaties.                                                                         *

 *                                                                                    *

 *  The copyright notice(s) in this Software/Data does not indicate actual            *

 *  or intended publication of this Software/Data.                                    *

***************************************************************************************
**  Name:		AuvitekTunerDev.c
**
**  Project:            Au85xx IIC Controller
**
**  Description:				    
**                    
**  Functions
**  Implemented:   
**				UINT16 AuvitekTuner_WriteReg
**				UINT16 AuvitekTuner_ReadReg
**				UINT16 AuvitekTuner_ReadTuner
**				UINT16 AuvitekTuner_WriteTuner
**				void AuvitekTuner_TimeDelay
**						
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History: 
**  -------------------------------------------------------------------------
*      SCR      Date         Author     Description

*****************************************************************************/

#include "Common/AuvitekTunerDev.h"
#include "../Common/Auvitek_Userdef.h"


/*****************************************************************************
**
**  Name: AuvitekTuner_WriteReg
**
**  Description:    Write values to device .
**
**  Parameters:   
**			DevHandle   - the device handle
**			nAddr       - the register address of AU85xx to be written
**			bData       - the data to be written to the register
**  Global variable:		 
**			bMainAddr   - the device address (IIC address) of AU85xx. 0x8E or 0x80
**				      For Auvitek module or the module built following Auvitek 
**				      reference design, it is 0x8E. And the default value of
**				      this variable in our code is 0x8E. 
**
**
**  Returns:        status:
**                      STATUS_OK            - No errors
**                      user-defined
**
**  Notes:          You MUST provide code for this function
**                      
**
**  Revision History:
**
**
*****************************************************************************/
TUNER_NTSTATUS AuvitekTuner_WriteReg(void * DevHandle, UINT16 nAddr, UINT8 bData)
{  
	return (TUNER_NTSTATUS)(Auvitek_WriteReg(nAddr, bData)); 
}

/*****************************************************************************
**
**  Name: AuvitekTuner_WriteMutipleReg
**
**  Description:    Write values to device .
**
**  Parameters:   
**			DevHandle   - the device handle
**			nAddr       - the register address of AU85xx to be written
**			bData       - the data to be written to the register
**			nLength		- the data length
**  Global variable:		 
**			bMainAddr   - the device address (IIC address) of AU85xx. 0x8E or 0x80
**				      For Auvitek module or the module built following Auvitek 
**				      reference design, it is 0x8E. And the default value of
**				      this variable in our code is 0x8E. 
**
**
**  Returns:        status:
**                      STATUS_OK            - No errors
**                      user-defined
**
**  Notes:          You MUST provide code for this function
**                      
**
**  Revision History:
**
**
*****************************************************************************/  
TUNER_NTSTATUS AuvitekTuner_WriteMutipleReg(void * DevHandle,UINT16 nAddr, UINT8 *pData , UINT8 nLength)
{
	int i = 0;
	Auvitek_WriteReg(0x91,pData[0]);
	Auvitek_WriteReg(0x93,pData[2]);
	for(i=0;i<nLength-3;i++)
		Auvitek_WriteReg((UINT16)(0x94+i),pData[i+3]);

	return TUNER_STATUS_SUCCESS;
}

/*****************************************************************************
**
**  Name: AuvitekTuner_ReadReg
**
**  Description:    Read values from the register .
**
**  Parameters:   
**			nAddr      - the register address to be read
**			pData      - pointer to the Data to receive the value from the register 
**
**  Global variable:		 
**			bMainAddr   - the device address (IIC address) of AU85xx. 0x8E or 0x80
**				      For Auvitek module or the module built following Auvitek 
**				      reference design, it is 0x8E. And the default value of
**				      this variable in our code is 0x8E. 
**  Returns:        
**			status:
**                      	STATUS_OK            - No errors
**                      	user-defined
**
**  Notes:            You MUST provide code for this function
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS AuvitekTuner_ReadReg(void * DevHandle,UINT16 nAddr, UINT8 *pData)
{
	return (TUNER_NTSTATUS)(Auvitek_ReadReg(nAddr, pData));
}

/*****************************************************************************
**
**  Name: AuvitekTuner_ReadTuner
**
**  Description:    Read values to  tuner .
**
**  Parameters:   
**                  nDevice - device address (IIC address) of tuner
**                  nAdd       - a register address of tuner 
**                  pData      - pointer to the buffer  to be write  to the tuner 
**                  nNum       - number of bytes to be write 
**
**  Returns:        status:
**                  STATUS_OK  - No errors
**                  user-defined
**
**  Notes:            You MUST provide code for this function
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS AuvitekTuner_ReadTuner(void * DevHandle,UINT8 nDeviceAdd,UINT16 nRegAdd,UINT8 *pData, UINT32 nNum)
{
	return (TUNER_NTSTATUS)(Auvitek_ReadTuner((UINT8)nDeviceAdd, nRegAdd, pData, nNum));
}

/*****************************************************************************
**
**  Name: AuvitekTuner_WriteTuner
**
**  Description:    Write values to  tuner .
**
**  Parameters:   
**                  nDeviceAdd - device address (IIC address) of tuner
**                  nRegAdd       - a register address of tuner 
**                  pData      - pointer to the buffer  to be write  to the tuner 
**                  nNum       - number of bytes to be write 
**
**  Returns:        status:
**                  STATUS_OK  - No errors
**                  user-defined
**
**  Notes:            You MUST provide code for this function
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS AuvitekTuner_WriteTuner(void * DevHandle,UINT16 nDeviceAdd, UINT16 nRegAdd,UINT8 *pData, UINT32 nNum)
{	
	return (TUNER_NTSTATUS)(Auvitek_WriteTuner((UINT8)nDeviceAdd, nRegAdd, pData, nNum));
}

/*****************************************************************************
**
**  Name: AuvitekTuner_TimeDelay
**
**  Description:    Delay execution for "nMinDelayTime" milliseconds
**
**  Parameters:     
**                  nMinDelayTime - Delay time in milliseconds
**
**  Returns:        None.
**
**  Notes:          For future use
**  Revision History:
**
**
*****************************************************************************/
void AuvitekTuner_TimeDelay(UINT16 nMinDelayTime)
{
	Auvitek_TimeDelay(nMinDelayTime);
}

/*****************************************************************************
**
**  Name: AuvitekTuner_ResetTuner
**
**  Description:    Reset Tuner
**
**  Parameters:     
**                 
**
**  Returns:        None.
**
**  Notes:          For future use
**  Revision History:
**
**
*****************************************************************************/
void AuvitekTuner_ResetTuner(void * DevHandle)
{
	unsigned char bDate=0;
	unsigned char bDateRead=0;
	
	AuvitekTuner_WriteReg(0,0xe0,0xdf);
	
	AuvitekTuner_ReadReg(0,0xe2,&bDateRead);
	bDate = bDateRead|0x2;//bit1:1
	AuvitekTuner_WriteReg(0,0xe2,bDate);
	bDate  = bDateRead&0xfd;//bit1:0
	AuvitekTuner_WriteReg(0,0xe2,bDate);
	bDate  = bDateRead|0x2;//bit1:1
	AuvitekTuner_WriteReg(0,0xe2,bDate);
}


/*****************************************************************************
**
**  Name: I2C_EnableSecondPort
**
**  Description:    Enable I2C second Port
**
**  Parameters:     
**                      DevHandle - device handle
						flag      - if or not enable
**
**  Returns:        None.
**
**  Notes:          For future use
**  Revision History:
**
**
*****************************************************************************/

UINT16 I2C_EnableSecondPort(void * DevHandle,int flag)
{
	return Auvitek_OpenIIC2(flag);
}


