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
**  Name: Auvitek_Userdef.c
**
**  Description:    User-defined Auvitek software interface 
**
**  Functions
**  Requiring
**  Implementation: Auvitek_WriteReg
**                  Auvitek_ReadReg
**					Auvitek_WriteTuner
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
#include "Auvitek_Userdef.h"

//#include <asm/arch/api_i2c.h>
#include <linux/delay.h>

#define ATSC_AU8524A_ADD    0x8e//0x47
#define ATSC_AU8524A_FRQ    60

extern int ATSC_I2C_Init(UINT8 SlaveAddress, UINT16 nKHz);
extern int ATSC_I2C_Deinit(void);
extern int ATSC_I2C_Write(UINT16 RegAddr, UINT8 *pData, UINT16 size);
extern int ATSC_I2C_Read(UINT16 RegAddr, UINT8 *pData, UINT16 size);
extern int ATSC_I2C_Read8(UINT8 RegAddr, UINT8 *pData, UINT16 size);
extern int ATSC_I2C_Write(UINT16 RegAddr, UINT8 *pData, UINT16 size);
extern int ATSC_I2C_Write8(UINT8 RegAddr, UINT8 *pData, UINT16 size);
/*****************************************************************************
**
**  Name: Auvitek_WriteReg
**
**  Description:    Write values to device .
**
**  Parameters:   
**                       nAddr       - the register address of AU85xx to be written
**                       bData       - the data to be written to the register
**  Global variable		 bMainAddr   - the device address (IIC address) of AU chip. 0x8E or 0x80
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
UINT16 Auvitek_WriteReg(UINT16 nAddr, UINT8  bData)
{  
	
    UINT16 status=STATUS_OK;
    int i;
    int err;
    nAddr &= 0x07ff;
    nAddr |= 0x8000;

    err = ATSC_I2C_Init(ATSC_AU8524A_ADD,ATSC_AU8524A_FRQ);
    if(!err){
        status = STATUS_USER_ERROR;
        goto writeExit;
    }

    err = ATSC_I2C_Write(nAddr,&bData,0x1);
    if(!err){
        status = STATUS_USER_ERROR;
        goto writeExit;
    }

    writeExit:         
    ATSC_I2C_Deinit();

    return status;

    /*AU85xx
    **  Todo:add code to  implement a  write operation 
    **  For IIC accessing, please read our application note. Here is just a brief.
	**  device address (bMainAddr), control code (0x80), register address of AU85xx (nAddr) and data (bData)
	**  will be put in 4 bytes and sent through IIC one by one. The following is a brief description of writting steps
	**  1. Start
	**  2. Device address with the last bit is 0
	**  3. ACK
	**  4. Control code
	**  5. ACK
	**  6. Register address
	**  7. ACK
	**  8. Data
	**  9. ACK
	**  10. Stop
    **       	
	**  Please pay more attention on control code and register address. Be noticed that the high 3 bits of register address should be put in low 3 bits
	**  of control code
    */
     /*  return status;  */
}

/*****************************************************************************
**
**  Name: Auvitek_RepeatWriteReg
**
**  Description:    Write values to device .
**
**  Parameters:   
**                       nAddr       - the register address of AU85xx to be written
**                       bData       - the data to be written to the register
**  Global variable		 bMainAddr   - the device address (IIC address) of AU chip. 0x8E or 0x80
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
UINT16 Auvitek_RepeatWriteReg(UINT16 nAddr, UINT8  *bData, int nLength)
{  
	
    UINT16 status=STATUS_OK;
    int i;
    int err;
    nAddr &= 0x07ff;
    nAddr |= 0x8000;


    err = ATSC_I2C_Init(ATSC_AU8524A_ADD,ATSC_AU8524A_FRQ);
    if(!err){
        
        status = STATUS_USER_ERROR;
        goto RepWriteExit;
    }
    err = ATSC_I2C_Write(nAddr,&bData,nLength);
    if(!err){
        status = STATUS_USER_ERROR;
        }
RepWriteExit:
    ATSC_I2C_Deinit();

    return status;
   
    /*(AU85xx needn't call this function, it for AU88xx)
    **  Todo:add code to  implement a  write operation 
    **  For IIC accessing, please read our application note. Here is just a brief.
	**  device address (bMainAddr), control code (0x80), register address of AU85xx (nAddr) and data (bData)
	**  will be put in 4 bytes and sent through IIC one by one. The following is a brief description of writting steps
	**  1. Start
	**  2. Device address with the last bit is 0
	**  3. ACK
	**  4. Control code
	**  5. ACK
	**  6. Register address
	**  7. ACK
	**  8. Data
	**  9. ACK
	**  10. Stop
    **       	
	**  Please pay more attention on control code and register address. Be noticed that the high 4 bits of register address should be put in low 4 bits
	**  of control code
    */
     /*  return status;  */
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
**  Global variable		 bMainAddr   - the device address (IIC address) of AU chip. 0x8E or 0x80
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
UINT16 Auvitek_ReadReg(UINT16 nAddr, UINT8 *pData)
{

    UINT16 status=STATUS_OK;
    int i;
    int err;
    nAddr &= 0x07ff;
    nAddr |= 0x4000;


    err = ATSC_I2C_Init(ATSC_AU8524A_ADD,ATSC_AU8524A_FRQ);    
    if(!err){
        status = STATUS_USER_ERROR;
        goto error;
    }
         
    err =  ATSC_I2C_Read(nAddr,pData,0x1);    
     if(!err){
        status = STATUS_USER_ERROR;
        goto error;
        }
     
 error:
    ATSC_I2C_Deinit();
    return status;
    

    /*AU85xx
    **  Todo: add code to implement a  read operation 
	**  For IIC accessing, please read our application note. Here is just a brief.
	**  device address (bMainAddr), control code (0x40), register address of AU85xx (nAddr) and 1 dummy byte 
	**  will be put in 4 bytes and sent through IIC one by one. The following is a brief description of writting steps
	**  1. Start
	**  2. Device address with the last bit is 0
	**  3. ACK
	**  4. Control code
	**  5. ACK
	**  6. Register address
	**  7. ACK
	**  8. 1 dummy byte
	**  9. ACK
	**  10. Stop	
	**  11. Start
	**  12. device address with the last bit is 1
	**  13. ACK
	**  14. value of a register
	**  15. ACK
	**  16. Stop
	**  Please pay more attention on control code and register address. Be noticed that the high 3 bits of register address should be put in low 3 bits
	**  of control code
    */
    /*  return status;  */
}

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

UINT16 Auvitek_ReadTuner(UINT8 nDevice,UINT16 nAdd,UINT8 *pData, UINT32 nNum)
{
    UINT16 status=STATUS_OK;
    UINT16 nNum16 = nNum & 0x0000ffff;
    UINT8 DeviceAddr = (UINT8)nDevice;
    UINT8 RegAddr = (UINT8)nAdd;

    int i;
    int err;


    err = ATSC_I2C_Init((DeviceAddr ),ATSC_AU8524A_FRQ);
    if(!err){
        status =  STATUS_USER_ERROR;
        goto error;
    }

    err = ATSC_I2C_Read8(RegAddr,pData,nNum16);
    if(!err)
        status = STATUS_USER_ERROR;
error:
    ATSC_I2C_Deinit();

    return status;
 
	/*
    **  Todo:add code to  implement a  read tuner operation 
    */
    /*  return status;  */
	// for CAN tuners which don't need register address, and silicon tuners whose register addresses are in 1 byte.Pls conver nRegAdd to 1 byte.
	// Example: UINT8 nRegAdd8 = nAdd;
	// for silicon tuners which have 2 bytes register addresses. Pls keep it like this
}

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


UINT16 Auvitek_WriteTuner(UINT8 nDeviceAdd,  UINT16 nRegAdd,UINT8 *pData, UINT32 nNum)
{
	
    UINT16 status=STATUS_OK;
    UINT16 nNum16 = nNum & 0x0000ffff;
    int i;
    int err;
    UINT8 DeviceAddr = (UINT8)nDeviceAdd;
    UINT8 RegAddr = (UINT8)nRegAdd;
    
    

    err = ATSC_I2C_Init((DeviceAddr ),ATSC_AU8524A_FRQ);
    if(!err){
        status =  STATUS_USER_ERROR;
        goto error;
    }

    err = ATSC_I2C_Write8(RegAddr,pData,nNum);
    if(!err)
        status = STATUS_USER_ERROR;
error:
    ATSC_I2C_Deinit();

    return status;
        
  
	/*
    **  Todo:add code to  implement a  write tuner operation 
    */
    /*  return status;  */
	// for CAN tuners which don't need register address, and silicon tuners whose register addresses are in 1 byte.Pls conver nRegAdd to 1 byte.
	// Example: UINT8 nRegAdd8 = nRegAdd;
	// for silicon tuners which have 2 bytes register addresses. Pls keep it like this
}
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
**
**  Revision History:
**
**
*****************************************************************************/
void Auvitek_TimeDelay(UINT16 nMinDelayTime)
{
        UINT32 count;
        //count = (UINT32) nMinDelayTime & 0x0000ffff;
        msleep( nMinDelayTime);
        return ;
  	/*
    **  ToDo: add code to implement a OS blocking
    **         for a period of "nMinDelayTime" milliseconds.
    */
}

/*****************************************************************************
**
**  Name: Auvitek_ResetChip
**
**  Description:    reset the chip when mode changes
** 
**  Parameters:     
**                  None
**
**  Returns:        status:
**                  STATUS_OK  - No errors
**                  user-defined
**
**
**  Revision History:
**
**
*****************************************************************************/
UINT16 Auvitek_ResetChip()
{
	
	UINT16	status =STATUS_OK;
         UINT8 Reset_Command[3] ;
         
         UINT16 Reset_Add = 0x2000;
         int err;
         UINT8 nData = 0x00;


	err = ATSC_I2C_Init(ATSC_AU8524A_ADD,ATSC_AU8524A_FRQ);
         if(!err){
            status = STATUS_USER_ERROR;
            goto error;
            
            }
    
	err = ATSC_I2C_Write(Reset_Add,&nData,0x1);
         if(!err)
            status = STATUS_USER_ERROR;

         
 error:
    
	ATSC_I2C_Deinit();
    
	return status;
         

	/**	AU85xx:
	**	Device address: 0x8E
	**	Command:0x20
	**	Data:0x00,0x00
	**  When it is merged into 4 bytes IIC command, it will be:0x8E200000
	** 
	**/
}

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
**  Returns:        status:
**                  STATUS_OK  - No errors
**                  user-defined
**
**  Notes:          For future use
**  Revision History:
**
**
*****************************************************************************/
UINT16 Auvitek_OpenIIC2(int flag)
{
    UINT16	status =STATUS_OK;
    int i;
    int err;
    UINT16 AU_IIC2_ADD = 0x8106;
    UINT8 IIC2_EN_DATA = 0X01;
    UINT8 IIC2_DI_DATA = 0X00;

    err = ATSC_I2C_Init(ATSC_AU8524A_ADD,ATSC_AU8524A_FRQ);
    
    if(!err){
        status = STATUS_USER_ERROR;
        goto error;
    }
    
    if(flag == 1)
        err = ATSC_I2C_Write(AU_IIC2_ADD,&IIC2_EN_DATA,0x1);
    else if(flag == 0)
        err = ATSC_I2C_Write(AU_IIC2_ADD,&IIC2_DI_DATA,0x1);
    
    if(!err)
        status = STATUS_USER_ERROR;



error:
    ATSC_I2C_Deinit();

    return status;
	
    /*
    **  Todo: add code to enable I2C second Port
	**	AU85xx:
	**	Enable IIC2: Register 0x106->0x1,
	**	Disable IIC2: Register 0x106->0x0,
	**/
}














