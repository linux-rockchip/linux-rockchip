/*******************************************************************************
 *
 * FILE NAME          : MxL_Common.h
 * 
 * AUTHOR             : Kyle Huang
 * DATE CREATED       : May 05, 2008
 *
 * DESCRIPTION        : 
 *
 *******************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ******************************************************************************/
 
#ifndef __MXL5007_COMMON_H
#define __MXL5007_COMMON_H

//#include <asm/arch/typedef.h>

/******************************************************************************
*						User-Defined Types (Typedefs)
******************************************************************************/


/****************************************************************************
*       Imports and definitions for WIN32                             
****************************************************************************/
//#include <windows.h>
#include "AuvitekTunerInterfaceTypedef.h"
#if 0
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef char           SINT8;
typedef short          SINT16;
typedef int            SINT32;
typedef float          REAL32;
typedef int				BOOL;
#define FALSE 0
#define TRUE 1
#endif
typedef char           SINT8;
typedef short          SINT16;
typedef int            SINT32;
typedef float          REAL32;
typedef int				BOOL;
#define FALSE 0
#define TRUE 1
/****************************************************************************\
*      Imports and definitions for non WIN32 platforms                   *
\****************************************************************************/
/*
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef char           SINT8;
typedef short          SINT16;
typedef int            SINT32;
typedef float          REAL32;

// create a boolean 
#ifndef __boolean__
#define __boolean__
typedef enum {FALSE=0,TRUE} BOOL;
#endif //boolean
*/


/****************************************************************************\
*          Definitions for all platforms					                 *
\****************************************************************************/
#ifndef NULL
#define NULL (void*)0
#endif



/******************************/
/*	MxL5007 Err message  	  */
/******************************/
typedef enum{
	MxL_OK				=   0,
	MxL_ERR_INIT		=   1,
	MxL_ERR_RFTUNE		=   2,
	MxL_ERR_SET_REG		=   3,
	MxL_ERR_GET_REG		=	4,
	MxL_ERR_OTHERS		=   10
}MxL_ERR_MSG;

/******************************/
/*	MxL5007 Chip verstion     */
/******************************/
typedef enum{
	MxL_UNKNOWN_ID		= 0x00,
	MxL_5007T_V4		= 0x14,
	MxL_GET_ID_FAIL		= 0xFF
}MxL5007_ChipVersion;


/******************************************************************************
    CONSTANTS
******************************************************************************/

#ifndef MHz
	#define MHz 1000000
#endif

#define MAX_ARRAY_SIZE 100


// Enumeration of Mode
// Enumeration of Mode
typedef enum 
{
	MxL_MODE_ISDBT = 0,
	MxL_MODE_DVBT = 1,
	MxL_MODE_ATSC = 2,	
	MxL_MODE_CABLE = 0x10
} MxL5007_Mode ;

typedef enum
{
	MxL_IF_4_MHZ	  = 4000000,
	MxL_IF_4_5_MHZ	  = 4500000,
	MxL_IF_4_57_MHZ	  =	4570000,
	MxL_IF_5_MHZ	  =	5000000,
	MxL_IF_5_38_MHZ	  =	5380000,
	MxL_IF_6_MHZ	  =	6000000,
	MxL_IF_6_28_MHZ	  =	6280000,
	MxL_IF_9_1915_MHZ =	9191500,
	MxL_IF_35_25_MHZ  = 35250000,
	MxL_IF_36_15_MHZ  = 36150000,
	MxL_IF_44_MHZ	  = 44000000
} MxL5007_IF_Freq ;

typedef enum
{
	MxL_XTAL_16_MHZ		= 16000000,
	MxL_XTAL_20_MHZ		= 20000000,
	MxL_XTAL_20_25_MHZ	= 20250000,
	MxL_XTAL_20_48_MHZ	= 20480000,
	MxL_XTAL_24_MHZ		= 24000000,
	MxL_XTAL_25_MHZ		= 25000000,
	MxL_XTAL_25_14_MHZ	= 25140000,
	MxL_XTAL_27_MHZ		= 27000000,
	MxL_XTAL_28_8_MHZ	= 28800000,
	MxL_XTAL_32_MHZ		= 32000000,
	MxL_XTAL_40_MHZ		= 40000000,
	MxL_XTAL_44_MHZ		= 44000000,
	MxL_XTAL_48_MHZ		= 48000000,
	MxL_XTAL_49_3811_MHZ = 49381100	
} MxL5007_Xtal_Freq ;

typedef enum
{
	MxL_BW_6MHz = 6,
	MxL_BW_7MHz = 7,
	MxL_BW_8MHz = 8
} MxL5007_BW_MHz;

typedef enum
{
	MxL_NORMAL_IF = 0 ,
	MxL_INVERT_IF

} MxL5007_IF_Spectrum ;

typedef enum
{
	MxL_LT_DISABLE = 0 ,
	MxL_LT_ENABLE

} MxL5007_LoopThru ;

typedef enum
{
	MxL_CLKOUT_DISABLE = 0 ,
	MxL_CLKOUT_ENABLE

} MxL5007_ClkOut;

typedef enum
{
	MxL_CLKOUT_AMP_0 = 0 ,
	MxL_CLKOUT_AMP_1,
	MxL_CLKOUT_AMP_2,
	MxL_CLKOUT_AMP_3,
	MxL_CLKOUT_AMP_4,
	MxL_CLKOUT_AMP_5,
	MxL_CLKOUT_AMP_6,
	MxL_CLKOUT_AMP_7
} MxL5007_ClkOut_Amp;

typedef enum
{
	MxL_I2C_ADDR_96 = 96 ,
	MxL_I2C_ADDR_97 = 97 ,
	MxL_I2C_ADDR_98 = 98 ,
	MxL_I2C_ADDR_99 = 99 	
} MxL5007_I2CAddr;

//
// MxL5007 TunerConfig Struct
//
typedef struct _MxL5007_TunerConfigS
{
	MxL5007_I2CAddr		I2C_Addr;
	MxL5007_Mode		Mode;
	int				IF_Diff_Out_Level;
	MxL5007_Xtal_Freq	Xtal_Freq;
	MxL5007_IF_Freq	    IF_Freq;
	MxL5007_IF_Spectrum IF_Spectrum;
	MxL5007_ClkOut		ClkOut_Setting;
    MxL5007_ClkOut_Amp	ClkOut_Amp;
	MxL5007_BW_MHz		BW_MHz;
	unsigned int				RF_Freq_Hz;
} MxL5007_TunerConfigS;




#endif /* __MXL5007_COMMON_H__*/

