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
**  Name:             AuvitekTunerDev.h
**
**  Description:    User-defined data types needed by Auvitek source code.
**
**                  Customers must provide the code for these functions
**                  in the file "AuvitekTunerDev.c".
**
**                  Customers must verify that the typedef's in the 
**                  "Data Types" section are correct for their platform.
**
**  Functions
**  Requiring
**  Implementation: 
**                  	AuvitekTuner_WriteReg
**                  	AuvitekTuner_ReadReg
**                  	AuvitekTuner_WriteTuner
**                  	AuvitekTuner_TimeDelay
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**	               
**  Revision History:
**
**
**************************************************************************************/
#if !defined( __AUVITEKTUNERDEV_H )
#define __AUVITEKTUNERDEV_H

#include "AuvitekTunerInterfaceTypedef.h"

#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage */                  
{
#endif


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

TUNER_NTSTATUS AuvitekTuner_WriteReg(void * DevHandle,UINT16 nAddr, UINT8  bData);

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
       

TUNER_NTSTATUS AuvitekTuner_WriteMutipleReg(void * DevHandle,UINT16 nAddr, UINT8 *pData , UINT8 nLength);          

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


TUNER_NTSTATUS AuvitekTuner_ReadReg(void * DevHandle,UINT16 nAddr, UINT8 *pData);

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


TUNER_NTSTATUS AuvitekTuner_WriteTuner(void * DevHandle,UINT16 nDeviceAdd, UINT16 nRegAdd,UINT8 *pData, UINT32 nNum);

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

TUNER_NTSTATUS AuvitekTuner_ReadTuner(void * DevHandle,UINT8 nDeviceAdd,UINT16 nRegAdd,UINT8 *pData, UINT32 nNum);


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

void AuvitekTuner_TimeDelay(UINT16 nMinDelayTime);


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


void AuvitekTuner_ResetTuner(void * DevHandle);

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

UINT16 I2C_EnableSecondPort(void * DevHandle,int flag);


#if defined( __cplusplus )     
}
#endif

#endif
