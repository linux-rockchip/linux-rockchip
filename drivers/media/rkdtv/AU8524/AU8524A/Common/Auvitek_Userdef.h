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
**  Name:             Auvitek_userdef.h
**
**  Description:    User-defined data types needed by Auvitek source code.
**
**                  Customers must provide the code for these functions
**                  in the file "Auvitek_userdef.c".
**
**                  Customers must verify that the typedef's in the 
**                  "Data Types" section are correct for their platform.
**
**  Functions
**  Requiring
**  Implementation: 
**                  Auvitek_WriteReg
**                  Auvitek_ReadReg
**                  Auvitek_WriteTuner
**                  Auvitek_TimeDelay
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**	               
**  Revision History:
**
**
**************************************************************************************/
#if !defined( __AUVITEK_USERDEF_H )
#define __AUVITEK_USERDEF_H

#include "AUChipTypedef.h"


#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage */                  
{
#endif


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
UINT16 Auvitek_WriteReg(UINT16 nAddr, UINT8  bData);
           
/*****************************************************************************
**
**  Name: Auvitek_RepeatWriteReg
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
UINT16 Auvitek_RepeatWriteReg(UINT16 nAddr, UINT8  *bData, int nLength);

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
UINT16 Auvitek_ReadReg(UINT16 nAddr, UINT8 *pData);

/*****************************************************************************
**
**  Name: Auvitek_WriteTuner
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
UINT16 Auvitek_WriteTuner(UINT8 nDeviceAdd, UINT16 nRegAdd,UINT8 *pData, UINT32 nNum);

/*****************************************************************************
**
**  Name: Auvitek_ReadTuner
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

UINT16 Auvitek_ReadTuner(UINT8 nDevice,UINT16 nAdd,UINT8 *pData, UINT32 nNum);

/*****************************************************************************
**
**  Name: Auvitek_TimeDelay
**
**  Description:    Delay execution for "nMinDelayTime" milliseconds
**
**  Parameters:     
**                      nMinDelayTime - Delay time in milliseconds
**
**  Returns:        None.
**
**  Notes:          For future use
**  Revision History:
**
**
*****************************************************************************/
void Auvitek_TimeDelay(UINT16 nMinDelayTime);

/*****************************************************************************
**
**  Name: Auvitek_ResetChip
**
**  Description:    reset the chip when mode changes
**					Device address: 0x8E
**    				Command:0x20
**    				Data:0x00,0x00
** 
**    				When it is merged into 4 bytes IIC command, it will be:
**    				0x8E200000
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
UINT16 Auvitek_ResetChip();

/*****************************************************************************
**
**  Name: Auvitek_OpenIIC2
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
UINT16 Auvitek_OpenIIC2(int flag);

#if defined( __cplusplus )     
}
#endif

#endif
