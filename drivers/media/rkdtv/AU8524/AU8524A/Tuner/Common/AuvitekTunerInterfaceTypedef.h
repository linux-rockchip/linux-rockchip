/*************************************************************************************
*  CONFIDENTIAL AND PROPRIETARY SOFTWARE/DATA OF Auvitek International, Ltd.         *
*																					 *
*  Copyright (c) 2005-2018   Auvitek International, Ltd.  All Rights Reserved.       *
*																					 *
*  Use of this Software/Data is subject to the terms and conditions of               *
*  the applicable agreement between Auvitek International, Ltd, and receiving party. *
*  In addition, this Software/Data is protected by copyright law and international   *
*  treaties.                                                                         *
*																					 *
*  The copyright notice(s) in this Software/Data does not indicate actual            *
*  or intended publication of this Software/Data.                                    *
**************************************************************************************
**  Name:			AuvitekTunerInterfaceTypedef.h
**
**  Project:            	Au85xx IIC Controller
**
**  Description:		mainly define common header files and macro.		    
**                    
**  Functions
**  Implemented:   
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History: 
**  -------------------------------------------------------------------------
*      SCR      Date         Author     Description

*****************************************************************************/

#if !defined(AUVITEKTUNERINTERFACETYPEDEF__H_)
#define AUVITEKTUNERINTERFACETYPEDEF__H_
//#include <asm/arch/typedef.h>


#pragma once

#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage */                 
{
#endif




/*  Macro to be used to check for errors  */
#define STATUS_IS_ERROR(s)			(((s) >> 14) != 0)
#define STATUS_NO_ERROR(s)			(((s) >> 14) == 0)

/* error define */
#define STATUS_ERROR				(0x1 << 15)
#define STATUS_USER_ERROR			(0x1 << 14)
                        
#define STATUS_OK					(0x0000)
/*  Unknown error  */ 
#define STATUS_UNKNOWN				(0x8001)
	
#define TUNER_MAX_UNITS 10
#if 1
typedef signed char         INT8,   *PINT8;
typedef signed short        INT16,  *PINT16;
typedef signed int          INT32,  *PINT32;
typedef signed long			LONG,	*PLONG;

typedef unsigned char       UINT8,  *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned long		ULONG,	*PULONG;
#endif
typedef enum  
{
    TUNER_STATUS_SUCCESS	= 0,        
	TUNER_STATUS_UNSUCCESSFUL
}TUNER_NTSTATUS;

typedef enum 
{
	TUNER_XG_DVT6AW41F2	= 0,
	TUNER_QJ_DTZ1W40x,
	TUNER_MT_2131,		
	TUNER_MT_2130,		
	TUNER_NXP_TDA18271C1,	
	TUNER_TH_DTT7683x,
	TUNER_SANYO_UBD00Ax,	
	TUNER_XC_5000,
	TUNER_MXL_5003s_5,
	TUNER_TH_DTT768xx,
	TUNER_Philips_TD1336,
	TUNER_NXP_TD1636ALF,
	TUNER_NXP_TD1736,
	TUNER_QJ_DAM2W400,
	TUNER_XC_3028,
	TUNER_Himax_HXT5X51,
	TUNER_TH_DTT7601,
	TUNER_MT_7136,
	TUNER_MT_2060,
	TUNER_QJ_DTZ0R401,
	TUNER_QJ_DTZ0W400G,
	TUNER_SS_DTVS205CH201A,
	TUNER_QJ_DTP4W300,
	TUNER_NXP_TDA18271C2,
	TUNER_NXP_FQD1236MK5,
	TUNER_SS_DTVS22DPV261A,
	TUNER_TCL_DZ28WP6E,
	TUNER_MXL_5007,
	TUNER_TCL_DA37TWZ4E,
	TUNER_LG_TDVSH702F1,
	TUNER_PANASONIC_ENG36E32KF,
	TUNER_QJ_DTZ0R502,
	TUNER_TH_DTT75450,
	TUNER_SHARP_VA1T1ED2093,
	TUNER_FORWARD_TAEH69P31FR3,
	TUNER_NXP_TD1611ALF,
	TUNER_NXP_TD1116,
	TUNER_NXP_FQD1136,
	TUNER_NXP_TD1636FN,
	TUNER_PANASONIC_ENV59M01D8,

    TUNER_All_TestMode,
	TUNER_UNKNOWN,
}AU_TUNERTYPE_T;


typedef enum
{
	TUNER_AnalogVideo_None     		= 0x00000000,
	TUNER_AnalogVideo_NTSC_M   		= 0x00000001,
	TUNER_AnalogVideo_NTSC_M_J 		= 0x00000002,
	TUNER_AnalogVideo_NTSC_433 		= 0x00000004,

    TUNER_AnalogVideo_PAL_B    		= 0x00000010,
    TUNER_AnalogVideo_PAL_D    		= 0x00000020,
    TUNER_AnalogVideo_PAL_G    		= 0x00000040,
    TUNER_AnalogVideo_PAL_H    		= 0x00000080,
    TUNER_AnalogVideo_PAL_I    		= 0x00000100,
    TUNER_AnalogVideo_PAL_M    		= 0x00000200,
    TUNER_AnalogVideo_PAL_N    		= 0x00000400,

    TUNER_AnalogVideo_PAL_60   		= 0x00000800,

    TUNER_AnalogVideo_SECAM_B  		= 0x00001000,
    TUNER_AnalogVideo_SECAM_D  		= 0x00002000,
    TUNER_AnalogVideo_SECAM_G  		= 0x00004000,
    TUNER_AnalogVideo_SECAM_H  		= 0x00008000,
    TUNER_AnalogVideo_SECAM_K  		= 0x00010000,
    TUNER_AnalogVideo_SECAM_K1 		= 0x00020000,
    TUNER_AnalogVideo_SECAM_L  		= 0x00040000,
    TUNER_AnalogVideo_SECAM_L1 		= 0x00080000,

    TUNER_AnalogVideo_PAL_N_COMBO	= 0x00100000

} TUNER_AnalogVideoStandard;


typedef enum
{
    TUNER_Digital_MOD_NOT_SET 			= 0x00000000,
    TUNER_Digital_MOD_16QAM 			= 0x00000001,
    TUNER_Digital_MOD_32QAM				= 0x00000002,
    TUNER_Digital_MOD_64QAM				= 0x00000004,
    TUNER_Digital_MOD_80QAM				= 0x00000008,
    TUNER_Digital_MOD_96QAM				= 0x00000010,
    TUNER_Digital_MOD_112QAM			= 0x00000020,
    TUNER_Digital_MOD_128QAM			= 0x00000040,
    TUNER_Digital_MOD_160QAM			= 0x00000080,
    TUNER_Digital_MOD_192QAM			= 0x00000100,
    TUNER_Digital_MOD_224QAM			= 0x00000200,
    TUNER_Digital_MOD_256QAM			= 0x00000400,
    TUNER_Digital_MOD_320QAM			= 0x00000800,
    TUNER_Digital_MOD_384QAM			= 0x00001000,
    TUNER_Digital_MOD_448QAM			= 0x00002000,
    TUNER_Digital_MOD_512QAM			= 0x00004000,
    TUNER_Digital_MOD_640QAM			= 0x00008000,
    TUNER_Digital_MOD_768QAM			= 0x00010000,
    TUNER_Digital_MOD_896QAM			= 0x00020000,
    TUNER_Digital_MOD_1024QAM			= 0x00040000,
    TUNER_Digital_MOD_8VSB				= 0x00080000,
	TUNER_Digital_MOD_16VSB				= 0x00100000,
	TUNER_Digital_MOD_DVB_T				= 0x00200000,
	TUNER_Digital_MOD_DVB_C				= 0x00400000,
	TUNER_Digital_MOD_DVB_H				= 0x00800000,
	TUNER_Digital_MOD_DVB_S				= 0x01000000,
	TUNER_Digital_MOD_DTMB				= 0x02000000,
} TUNER_DigitalDemodStand;

typedef enum
{
    TUNERPROPERTY_TUNER_MODE_FM_RADIO  = 0x00000002,
    TUNERPROPERTY_TUNER_MODE_AM_RADIO  = 0x00000004

} TUNER_TUNER_MODE_RADIO;



typedef enum  
{
    AU_ADMod_ANALOGMOD				= 0,        
	AU_ADMod_DIGITALMOD
}AU_ADMod;

typedef enum  
{
    AU_TVMod_TV						= 0,        
	AU_TVMod_FM
}AU_TVMod;

typedef enum  
{
    AU_AnalogVideoStand_NTSC		= 0,        
	AU_AnalogVideoStand_PAL_I,
	AU_AnalogVideoStand_PAL_BG,
	AU_AnalogVideoStand_PAL_DK
}AU_AnalogVideoStand;

typedef enum  
{
    AU_AnalogInputSource_RF			= 0,        
	AU_AnalogInputSource_COMPOSITE,
	AU_AnalogInputSource_S_VIDEO
}AU_AnalogInputSource;

typedef enum  
{
    AU_TunerInputType_AIR			= 0,        
	AU_TunerInputType_CABLE
}AU_TunerInputType;

typedef enum  
{
    AU_DigitalDemodStand_ATSC		= 0,        
	AU_DigitalDemodStand_QAM64,
	AU_DigitalDemodStand_QAM256,
	AU_DigitalDemodStand_DVB_T,
	AU_DigitalDemodStand_DTMB,
	AU_DigitalDemodStand_DVB_C
}AU_DigitalDemodStand;

typedef enum   
{
    AU_AudioFormat_A2				= 0,        
	AU_AudioFormat_BTSC
}AU_AudioFormat;


typedef struct 
{
	AU_ADMod				ADMod;
	AU_TVMod				TVMod;
	AU_AnalogVideoStand		AnalogVideoStand;
	AU_AnalogInputSource	AnalogInputSource;
	AU_TunerInputType		TunerInputType;
	AU_DigitalDemodStand	DigitalDemodStand;
	AU_AudioFormat			AudioFormat;
	ULONG					Reserve;	
}AU_TunerParameter, *PAU_TunerParameter;


typedef struct 
{
	ULONG	DigitalTunerCapabilities;
	ULONG	AnalogTunerCapabilities;
	ULONG	RadioTunerCapabilities;
	INT32	LowIF_ATSC;
	INT32	LowIF_QAM;
	INT32	LowIF_DVB_T;
	INT32	LowIF_NTSC;
	INT32	LowIF_PAL_I;
	INT32	LowIF_PAL_BG;
	INT32	LowIF_PAL_DK;
	INT32	LowIF_FM;
	INT32	LowIF_DTMB;
	INT32	LowIF_DVB_C;
	ULONG	Reserve;
}AU_TunerCapabilities, *PAU_TunerCapabilities;

typedef struct 
{
	void*	APHandle;
	UINT32	HWAddress;
}AU_TunerHandle;


typedef struct 
{
	INT32					(*AU_TunerOpen_Interface)(void* nAPHandle,UINT32 nHWAddress);
	TUNER_NTSTATUS			(*AU_TunerClose_Interface)(UINT32 TunerUnit);
	TUNER_NTSTATUS			(*AU_SetTunerParameter_Interface)(UINT32 TunerUnit,PAU_TunerParameter nTunerParameter);
	TUNER_NTSTATUS			(*AU_SetTunerFrequency_Interface)(UINT32 TunerUnit,UINT32 nFrequency);
	PAU_TunerCapabilities	(*AU_GetTunerCapabilities_Interface)(UINT32 TunerUnit);

	INT32					TunerHandle;
    INT32					TunerType;
	void*					APHandle;
	UINT32					HWAddress;

}AU_TunerInterFaceST;

#if defined( __cplusplus )     
}
#endif

#endif
