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
* TITLE 	  : RAONTECH TV configuration header file. 
*
* FILENAME    : raontv_port.h
*
* DESCRIPTION : 
*		Configuration for RAONTECH TV Services.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 10/12/2010  Ko, Kevin        Added the code of conutry for RYV_CONUTRY_ARGENTINA.
* 10/01/2010  Ko, Kevin        Changed the debug message macro names.
* 09/27/2010  Ko, Kevin        Creat for CS Realease
*             /Yang, Maverick  1.Reformating for CS API
*                              2.pll table, ADC clock switching, SCAN function, 
*								 FM function added..
* 04/09/2010  Yang, Maverick   REV1 SETTING 
* 01/25/2010  Yang, Maverick   Created.                                                   
********************************************************************************/

#ifndef __RAONTV_PORT_H__
#define __RAONTV_PORT_H__

#ifdef __cplusplus 
extern "C"{ 
#endif  


/*==============================================================================
 * Includes the user header files if neccessry.
 * File include if necessary for use RTV_DELAY_MS, RTV_DBGMSG0, RTV_DBGMSG1,
 * RTV_DBGMSG2, RTV_DBGMSG3, RTV_REG_GET and RTV_REG_SET.
 *============================================================================*/ 
//#include <stdio.h>
#include <linux/device.h>
#include <linux/delay.h>

/*==============================================================================
 * The slave address for I2C and SPI, the base address for EBI2.
 *============================================================================*/ 
#define RAONTV_CHIP_ADDR	0x86 


/*==============================================================================
 * Modifies the basic data types if neccessry.
 *============================================================================*/ 
typedef int					BOOL;
typedef signed char			S8;
typedef unsigned char		U8;
typedef signed short		S16;
typedef unsigned short		U16;
typedef signed int			S32;
typedef unsigned int		U32;

typedef int                 INT;
typedef unsigned int        UINT;
typedef long                LONG;
typedef unsigned long       ULONG;
 
typedef volatile U8			VU8;
typedef volatile U16		VU16;
typedef volatile U32		VU32;

#define INLINE				__inline


/*==============================================================================
 * Defines the package type of chip to target product.
 *============================================================================*/ 
//#define RAONTV_CHIP_PKG_WLCSP
#define RAONTV_CHIP_PKG_QFN	
//#define RAONTV_CHIP_PKG_LGA

/*==============================================================================
 * Defines the conutry to target product.
  *============================================================================*/ 
#define RTV_CONUTRY_BRAZIL		
//#define RYV_CONUTRY_ARGENTINA 
//#define RTV_CONUTRY_JAPAN		
//#define RTV_CONUTRY_KOREA	
	


/*==============================================================================
 * Selects the TV mode(s) to target product.
 *============================================================================*/ 
#define RTV_ISDBT_ENABLE
//#define RTV_TDMB_ENABLE
//#define RTV_FM_ENABLE
//#define RTV_DAB_ENABLE


/*==============================================================================
 * Defines the NOTCH FILTER setting Enable.
 * In order to reject GSM/CDMA blocker, NOTCH FILTER must be defined.
 *============================================================================*/ 
#if defined(RTV_ISDBT_ENABLE)
	#define RTV_NOTCH_FILTER_ENABLE  
#endif


/*==============================================================================
 * Defines the external source freqenecy in KHz.
 * Ex> #define RTV_SRC_CLK_FREQ_KHz	36000 // 36MHz
 *============================================================================*/ 
#define RTV_SRC_CLK_FREQ_KHz			32000//24576//32000//24576
	

/*==============================================================================
 * Defines the delay macro in milliseconds.
 *============================================================================*/  
//extern void oem_delay_ms(int ms);
//#define RTV_DELAY_MS(ms)    oem_delay_ms(ms) 
#define RTV_DELAY_MS(ms)    mdelay(ms) // Linux


/*==============================================================================
 * Defines the debug message macro.
 *============================================================================*/  
#if 1
	//void my_printf(const char * /*format*/, ...);
	
	//#define RTV_DBGMSG0(fmt)					my_printf(fmt)
	//#define RTV_DBGMSG1(fmt, arg1)				my_printf(fmt, arg1) 
	//#define RTV_DBGMSG2(fmt, arg1, arg2)		my_printf(fmt, arg1, arg2) 
	//#define RTV_DBGMSG3(fmt, arg1, arg2, arg3)	my_printf(fmt, arg1, arg2, arg3) 

    #define RTV_DBGMSG0(fmt)					printk(fmt)
	#define RTV_DBGMSG1(fmt, arg1)				printk(fmt, arg1) 
	#define RTV_DBGMSG2(fmt, arg1, arg2)		printk(fmt, arg1, arg2) 
	#define RTV_DBGMSG3(fmt, arg1, arg2, arg3)	printk(fmt, arg1, arg2, arg3) 
#else
	/* To eliminates the debug messages. */
	#define RTV_DBGMSG0(fmt)					((void)0) 
	#define RTV_DBGMSG1(fmt, arg1)				((void)0) 
	#define RTV_DBGMSG2(fmt, arg1, arg2)		((void)0) 
	#define RTV_DBGMSG3(fmt, arg1, arg2, arg3)	((void)0) 
#endif


/*==============================================================================
 * Defines the Host interface.
 *============================================================================*/  
//#define RTV_IF_MPEG2_SERIAL_TSIF // I2C + TSIF Master Mode. 
#define RTV_IF_MPEG2_PARALLEL_TSIF // I2C + TSIF Master Mode. Support only 1seg Application!
//#define RTV_IF_QUALCOMM_TSIF // I2C + TSIF Master Mode
//#define RTV_IF_SPI // SPI Slave Mode
//#define RTV_IF_EBI2 // External Bus Interface Slave Mode


/*==============================================================================
 * Defines the TSIF interface for MPEG2 or QUALCOMM TSIF.
 * Refet to "3.1.2 TSIF Format Configuration" chapter in the MTV_Device_Driver_User_Guide.pdf.
 *============================================================================*/  
#define RTV_TSIF_FORMAT_1
//#define RTV_TSIF_FORMAT_2
//#define RTV_TSIF_FORMAT_3
//#define RTV_TSIF_FORMAT_4
//#define RTV_TSIF_FORMAT_5


//TODO: confirm power type and I/O voltage

/*==============================================================================
 * Define the power type.
 *============================================================================*/  
//#define RTV_PWR_EXTERNAL
#define RTV_PWR_LDO
//#define RTV_PWR_DCDC


/*==============================================================================
 * Defines the I/O voltage.
 *============================================================================*/  
//#define RTV_IO_1_8V
//#define RTV_IO_2_5V
#define RTV_IO_3_3V


#define	RTV_TS_PACKET_SIZE		188
/*==============================================================================
 * Defines the maximum TS packet size.
 *============================================================================*/  
#if defined(RTV_TDMB_ENABLE)
	#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
		#define RTV_MAX_VIDEO_TSP_SIZE		(RTV_TS_PACKET_SIZE * 1) // for TDMB
		#define RTV_MAX_AUDIO_TSP_SIZE		(RTV_TS_PACKET_SIZE * 1) // for TDMB
		#define RTV_MAX_DATA_TSP_SIZE		(RTV_TS_PACKET_SIZE * 1) // for TDMB

		#define RTV_MAX_TSP_SIZE		(RTV_TS_PACKET_SIZE * 1) // for FM, DAB

	#elif defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
		#define RTV_MAX_VIDEO_TSP_SIZE		(RTV_TS_PACKET_SIZE * 10)
		#define RTV_MAX_AUDIO_TSP_SIZE		(RTV_TS_PACKET_SIZE * 10)
		#define RTV_MAX_DATA_TSP_SIZE		(RTV_TS_PACKET_SIZE * 10)

		#define RTV_MAX_TSP_SIZE		(RTV_TS_PACKET_SIZE * 10)
	#endif

#else
	#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
		#define RTV_MAX_TSP_SIZE		(RTV_TS_PACKET_SIZE * 1)	
		
	#elif defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
		#define RTV_MAX_TSP_SIZE		(RTV_TS_PACKET_SIZE * 10)
	#endif
#endif


/*==============================================================================
 * Defines the register I/O macros.
 *============================================================================*/  
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	unsigned char dxbIO_ISDB_I2C_Read(unsigned char Addr);
	void dxbIO_ISDB_I2C_Write(unsigned char Addr, unsigned char Data);

    #define	RTV_REG_GET(reg)            dxbIO_ISDB_I2C_Read((unsigned char)reg)
    #define	RTV_REG_SET(reg, val)       dxbIO_ISDB_I2C_Write((unsigned char)reg, (unsigned char)val)
	#define	RTV_REG_MASK_SET(reg, mask, val) 								\
		do {																\
			U8 tmp;															\
			tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));	\
			RTV_REG_SET(reg, tmp);											\
		} while(0)

#elif defined(RTV_IF_SPI)
	unsigned char rtvOEM_spi_read(unsigned char reg);
	void rtvOEM_spi_read_burst(unsigned char reg, unsigned char *buf, int size);
	void rtvOEM_spi_write(unsigned char reg, unsigned char data);
    #define	RTV_REG_GET(reg)            		(U8)rtvOEM_spi_read(reg)
	#define	RTV_REG_BURST_GET(reg, buf, size) 	rtvOEM_spi_read_burst(reg, buf, size)
    #define	RTV_REG_SET(reg, val)       		rtvOEM_spi_write(reg, val) 
	#define	RTV_REG_MASK_SET(reg, mask, val) 								\
		do {																\
			U8 tmp;															\
			tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));	\
			RTV_REG_SET(reg, tmp);											\
		} while(0)
    
#elif defined(RTV_IF_EBI2)
	#define RTV_EBI2_MEM_WITDH  8 // 
	//#define RTV_EBI2_MEM_WITDH  16 // 
	//#define RTV_EBI2_MEM_WITDH  32 //
		
    #if (RTV_EBI2_MEM_WITDH == 8)
	extern VU8 g_bRtvEbiMapSelData;

	static INLINE U8 RTV_REG_GET(U8 reg)
	{	
		U8 bData;
		if(reg == 0x3)
		{
			bData = g_bRtvEbiMapSelData;
			if(bData ==0x09 || bData == 0x0A || bData == 0x0B || bData == 0x0C )
			{
				*(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = (unsigned char)(0x03);
				*(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = (unsigned char)(0x00);
				*(VU8 *)(RAONTV_CHIP_ADDR | 0x00 ) = bData;
			}
		}
		else
		{
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = reg;
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = g_bRtvEbiMapSelData;
			   bData= *(VU8 *)(RAONTV_CHIP_ADDR | 0x00 );
		
		}	

		return bData;
	}

	static INLINE void RTV_REG_SET(U8 reg, U8 val)
	{
		if(reg == 0x3) 
		{
		   	g_bRtvEbiMapSelData = val;
			if(val ==0x09 || val ==0x0A || val ==0x0B || val ==0x0C )
			{
				   *(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = 0x03;
				   *(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = 0x00;
				   *(VU8 *)(RAONTV_CHIP_ADDR | 0x00 ) = val;
			}
		}
		else
		{
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = reg;
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = g_bRtvEbiMapSelData;
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x00 ) = val;
		}
	}
	
	#define RTV_REG_MASK_SET(reg, mask, val)								\
		do {																\
		U8 tmp;															\
		tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));	\
		RTV_REG_SET(reg, tmp);											\
		} while(0)
		    
    #elif (RTV_EBI2_MEM_WITDH == 16)
    
    #elif (RTV_EBI2_MEM_WITDH == 32)
        
    #else
        #error "Can't support to memory witdh!"
    #endif
#else
    #error "Must define the interface definition !"
#endif


/*==============================================================================
 * Check erros
 *============================================================================*/  
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)  || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_SPI)
    #if (RAONTV_CHIP_ADDR >= 0xFF)
        #error "Invalid chip address"
    #endif
#elif defined(RTV_IF_EBI2)
    #if (RAONTV_CHIP_ADDR <= 0xFF)
        #error "Invalid chip address"
    #endif
#endif


#if defined(RTV_TDMB_ENABLE)
	#if (RTV_MAX_VIDEO_TSP_SIZE > (RTV_TS_PACKET_SIZE * 10))
	 || (RTV_MAX_AUDIO_TSP_SIZE > (RTV_TS_PACKET_SIZE * 10))
	 || (RTV_MAX_DATA_TSP_SIZE > (RTV_TS_PACKET_SIZE * 10))
		#error "Maximum TSP size must be less than (188 * 10) bytes"
	#endif
#else
	#if (RTV_MAX_TSP_SIZE > (RTV_TS_PACKET_SIZE * 10))
		#error "Maximum TSP size must be less than (188 * 10) bytes"
	#endif
#endif


#ifdef RTV_IF_MPEG2_PARALLEL_TSIF
	#if defined(RTV_TDMB_ENABLE) || defined(RTV_FM_ENABLE) || defined(RTV_DAB_ENABLE) || defined(RAONTV_CHIP_PKG_WLCSP)  || defined(RAONTV_CHIP_PKG_LGA)
		#error "Not support parallel TSIF!"
	#endif
#endif


void rtvOEM_ConfigureInterrupt(void);
void rtvOEM_ConfigureHostIF(void);
int  rtvOEM_ConfigureGPIO(void);
void rtvOEM_PowerOn(int on);

#ifdef __cplusplus 
} 
#endif 

#endif /* __RAONTV_PORT_H__ */

