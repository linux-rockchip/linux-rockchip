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
**  Name:           AuvitekChipDev.c
**
**  Description:
**
**
**  Functions
**  Requiring
**  Implementation: 
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**	               
**  Revision History:
**
**
**************************************************************************************/

#include "./Common/AuvitekChipDev.h"
#include "../Common/Auvitek_Userdef.h"



/*****************************************************************************
**
**  Name: Auvitek_WriteReg
**
**  Description:    Write values to device .
**
**  Parameters:   
**                       nAddr       - the register address of AU85xx to be written
**                       bData       - the data to be written to the register
**  Global variable		 bMainAddr   - the device address (IIC address) of AU85xx. 0x8E or 0x80
**									   For Auvitek module or the module built following Auvitek 
**									   reference design, it is 0x8E. And the default value of
**									   this variable in our code is 0x8E. 
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
UINT16 AuvitekChip_WriteReg(void * DevHandle,UINT16 nAddr, UINT8  bData)
{
	return Auvitek_WriteReg(nAddr,bData);
}
           
/*****************************************************************************
**
**  Name: Auvitek_WriteReg
**
**  Description:    Write values to device .
**
**  Parameters:   
**                       nAddr       - the register address of AU85xx to be written
**                       bData       - the data to be written to the register
**  Global variable		 bMainAddr   - the device address (IIC address) of AU85xx. 0x8E or 0x80
**									   For Auvitek module or the module built following Auvitek 
**									   reference design, it is 0x8E. And the default value of
**									   this variable in our code is 0x8E. 
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
UINT16 AuvitekChip_RepeatWriteReg(void * DevHandle,UINT16 nAddr, UINT8 *bData, int nLength)
{
	return Auvitek_RepeatWriteReg(nAddr, bData, nLength);
}
/*****************************************************************************
**
**  Name: Auvitek_ReadReg
**
**  Description:    Read values from the register .
**
**  Parameters:   
**						 nAddr       - the register address to be read
**						 pData      - pointer to the Data to receive the value from the register 
**
**  Global variable		 bMainAddr   - the device address (IIC address) of AU85xx. 0x8E or 0x80
**									   For Auvitek module or the module built following Auvitek 
**									   reference design, it is 0x8E. And the default value of
**									   this variable in our code is 0x8E. 
**  Returns:        status:
**                      STATUS_OK            - No errors
**                      user-defined
**
**  Notes:            You MUST provide code for this function
**
**  Revision History:
**
*****************************************************************************/
UINT16 AuvitekChip_ReadReg(void * DevHandle,UINT16 nAddr, UINT8 *pData)
{
	return Auvitek_ReadReg(nAddr,pData);
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

TUNER_NTSTATUS AuvitekChip_TimeDelay(UINT16 nMinDelayTime)
{
	 Auvitek_TimeDelay(nMinDelayTime);

	 return TUNER_STATUS_SUCCESS;
}


/*****************************************************************************
**
**  Name: Auvitek_ResetChip
**
**  Description:    reset the chip when mode changes
**
**
**  Parameters:     
**                  None
**
**  Returns:        None.
**
**
**  Revision History:
**
**
*****************************************************************************/
TUNER_NTSTATUS AuvitekChip_ResetChip(void * DevHandle)
{
	return (TUNER_NTSTATUS)Auvitek_ResetChip();
}