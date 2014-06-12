/******************************************************************************** 
* (c) COPYRIGHT 2010 RAONTECH, Inc. ALL RIGHTS RESERVED.
* 
* This software is the property of RAONTECH and is furnished under license by RAONTECH.                
* This software may be used only in accordance with the terms of said license.                         
* This copyright noitce may not be remoced, modified or obliterated without the prior                  
* written permission of RAONTECH, Inc.                                                                 
*                                                                                                      
* This software may not be copied, transmitted, provided to or otherwise made available                
* to any other person, company, corporation or other entity except as specified in the                 
* terms of said license.                                                                               
*                                                                                                      
* No right, title, ownership or other interest in the software is hereby granted or transferred.       
*                                                                                                      
* The information contained herein is subject to change without notice and should 
* not be construed as a commitment by RAONTECH, Inc.                                                                    
* 
* TITLE 	  : RAONTECH TV OEM source file. 
*
* FILENAME    : raontv_port.c
*
* DESCRIPTION : 
*		User-supplied Routines for RAONTECH TV Services.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 09/27/2010  Ko, Kevin        Creat for CS Realease
*             /Yang, Maverick  1.Reformating for CS API
*                              2.pll table, ADC clock switching, SCAN function, 
*								 FM function added..
* 04/09/2010  Yang, Maverick   REV1 SETTING 
* 01/25/2010  Yang, Maverick   Created.                                                   
********************************************************************************/

#include "raontv.h"
#include "raontv_internal.h"


void rtvOEM_ConfigureInterrupt(void) 
{
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x29, 0x00); 
	RTV_REG_SET(0x62, 0x8F); 
	RTV_REG_SET(0x63, 0xFF);

#elif defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	RTV_REG_SET(0x09, 0x00); // [6]INT1 [5]INT0 - 1: Input mode, 0: Output mode
	RTV_REG_SET(0x0B, 0x00); // [2]INT1 PAD disable [1]INT0 PAD disable

	RTV_REG_MAP_SEL( HOST_PAGE);
	RTV_REG_SET(0x28, 0x09); // [5:3]INT1 out sel [2:0] INI0 out sel - 0:Level, 1:Toggle, 2:"0", 3:"1"
	RTV_REG_SET(0x29, 0x00); // [3] Interrupt status register clear condition - 0:read data by memory access 1:status register access
	RTV_REG_SET(0x2A, 0x33); // [5]INT1 pol [4]INT0 pol - 0:Active High, 1:Active Low [3:0] Period = (INT_TIME+1)/8MHz

	RTV_REG_SET(0x62, 0xFF); // [6] MSC1 over-run interrupt [5] MSC1 under-run interrupt [4] MSC1 interrupt
#endif
}


void rtvOEM_ConfigureHostIF(void)
{
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	RTV_REG_MAP_SEL(HOST_PAGE);
    RTV_REG_SET(0x77, 0x15);   // TSIF Enable
    RTV_REG_SET(0x22, 0x48);   

  #if defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	RTV_REG_SET(0x04, 0x01);   // I2C + TSIF Mode Enable
  #else
	RTV_REG_SET(0x04, 0x29);   // I2C + TSIF Mode Enable
  #endif
  
	RTV_REG_SET(0x0C, 0xF4);   // TSIF Enable

#elif defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x77, 0x14);   //SPI Mode Enable
    RTV_REG_SET(0x04, 0x28);   // SPI Mode Enable
	RTV_REG_SET(0x0C, 0xF5);
 
#else
	#error "Code not present"
#endif
}



void rtvOEM_PowerOn(int on)
{
	if( on )
	{
		/* Set the GPIO of MTV_EN pin to low. */
		
		RTV_DELAY_MS(50);
		
		/* Set the GPIO of MTV_EN pin to high. */
		
		RTV_DELAY_MS(100);	
	}
	else
	{
		/* Set the GPIO of MTV_EN pin to low. */		
	}
}


int rtvOEM_ConfigureGPIO(void)
{
	// MTV_EN

#if defined(RTV_IF_SPI) 
	// SPI_CS

	
	// SPI_CLK

					
	// SPI_MOSI

	
	// SPI_MISO

		
#elif defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	// I2C
		
#elif defined(RTV_IF_EBI2)	
	
#else	
	#error "Code not present"
#endif

	return 0;
}
