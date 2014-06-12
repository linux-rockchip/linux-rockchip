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
**  Name:             Auvitek_Typedef.h
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
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**	               
**  Revision History:
**
**
**************************************************************************************/
#if !defined( __AUVITEK_TYPEDEF_H )
#define __AUVITEK_TYPEDEF_H

#pragma once

#define IIC_Demo

#define Null	0
#define _1K		1000
#define _1M		1000000

#if defined( __cplusplus )     
extern "C"                     /* Use "C" external linkage */                 
{
#endif

#include "AuvitekTunerInterfaceTypedef.h"
	

typedef		double              FP32; 


#define CHIP_MAX_UNITS 10
	
/* define signal lock status */
#define SIGNAL_IS_LOCKED			0
#define SIGNAL_NOT_LOCKED			1
#define SET_TUNER_ERROR		2

/* define audio format */
#define FORMAT_MONO					1
#define FORMAT_STEREO				2
#define FORMAT_SAP					3
#define FORMAT_DUAL					4

/*define timer */
#define TIMER_231_DLY				25							/* 25 ms		*/
#define COUNT1_TH					(500/TIMER_231_DLY)         /* 500 ms		*/ 
#define COUNT2_TH					(10*60*1000/TIMER_231_DLY)  /* 10 minutes	*/
#define COUNT3_TH					(1000/TIMER_231_DLY)        /* 1000 ms		*/ 
#define COUNT4_TH					(250/TIMER_231_DLY)			/*Count = 10	*/
#define COUNT5_TH					(500/TIMER_231_DLY)			/*Count = 20	*/

/*define control code*/
#define	INVALID_DEVICE_MAIN_ADDR    0xff
#define CTRL_CODE_WRITE				0x80
#define CTRL_CODE_READ				0x40
#define CTRL_CODE_RESET				0x20

//chip ability
#define	Chip_FM				0x00000001
#define	Chip_ATSC			0x00000002
#define	Chip_QAM			0x00000004
#define	Chip_NTSC			0x00000008
#define	Chip_SV13			0x00000010
#define	Chip_SV24			0x00000020
#define	Chip_Com1			0x00000040
#define	Chip_Com2			0x00000080
#define	Chip_Com3			0x00000100
#define	Chip_Com4			0x00000200
#define	Chip_PALI			0x00000400
#define	Chip_PALBG			0x00000800
#define	Chip_PALDK			0x00001000
#define	Chip_SIF			0x00002000
#define Chip_DVBT			0x00004000
#define Chip_DVBC			0x00008000
#define Chip_DVBH			0x00010000
#define Chip_DVBS			0x00020000
#define Chip_DTMB			0x00040000
#define Chip_BB_Audio		0x00080000
#define Chip_CVBS_BB		0x00100000
#define Chip_CVBS_SIF		0x00200000
#define Chip_SV_BB			0x00400000

//chip audio ability
#define	Audio_PWM			0x00000001
#define	Audio_IIS			0x00000002
#define	Audio_A2			0x00000004
#define	Audio_BTSC			0x00000008

//QAM type
typedef enum
{
	Mode_UNKNOWN = 0,
	Mode_4QAM,
	Mode_16QAM,
	Mode_32QAM,
	Mode_64QAM,
	Mode_4QAMNR,
	Mode_128QAM,
	Mode_256QAM,
	Mode_QAM_NC,
}QAM_Type_T;

//Interleave mode
typedef enum
{
	Intl_Mode_UNKNOWN = 0,
	Intl_240,
	Intl_720,
}Intl_Mode_T;

//DTMB Code rate
typedef enum
{
	CodeRate_UNKNOWN = 0,
	CodeRate1,
	CodeRate2,
	CodeRate3,
}Code_Rate_T;

//DTMB Carrier Mode
typedef enum
{
	CarrierMode_UNKNOWN = 0,
	Single_Carrier,
	Multi_Carrier,
}Carrier_Mode_T;

//ATV Audio input
typedef enum
{
	Audio_In_UNKNOWN = 0,
	Audio_SIF,
	Audio_BaseBand,
}ATV_Au_In_Type_T;

//Chip Mode
typedef enum
{
	TV_UNKNOWN = 0,
	TV_ATSC,
	TV_QAM256,
	TV_QAM64,
	TV_QAM,
	TV_NTSC,
	TV_CVBS,
	TV_SVIDEO,
	TV_SIF,
	TV_BB_Audio,
	TV_CVBS_SIF,
	TV_CVBS_BB,
	TV_SV_BB,
	TV_FM,
	TV_PAL_I,
	TV_PAL_BG,
	TV_PAL_DK,
	TV_DVB_T,
	TV_DVB_C,
	TV_DVB_H,
	TV_DVB_S,
	TV_DTMB,
}AU_TV_FORMAT_T;

typedef enum
{
	AUDIO_UNKNOWN = 0,
	AUDIO_MONO,
	AUDIO_BTSC_STEREO,
	AUDIO_BTSC_SAP,
	AUDIO_A2_STEREO,
	AUDIO_A2_DUAL,
}AU_AUDIOFORMAT_T;

typedef enum
{
	AUDIO_OUT_UNKNOWN = 0,
	AUDIO_OUT_IIS,
	AUDIO_OUT_PWM,	
}AU_AUDIOOUT_T;

typedef enum
{
	TS_UNKNOWN = 0,
	TS_SERIAL_OUTPUT,
	TS_PARALLEL_OUTPUT,
}AU_TS_OUTPUT_T;

typedef enum
{
	AIRCABLE_UNKNOWN = 0,
	AIR_MODE,
	CABLE_MODE,
}AU_AIRCABLE_T;

typedef enum
{
	IIS_FREQ_OUT_UNKNOWN = 0,
	IIS_FREQ_OUT_44_1k,
	IIS_FREQ_OUT_48k,
	IIS_FREQ_OUT_32k,
}AU_IIS_FREQ_OUT_T;

typedef enum 
{
		AU_SERIESUNKNOWN = 0,
		AU85xxSeries,
		AU88xxSeries,
}AU_CHIPSERIES_T;

typedef enum 
{
	AU_TaurusUNKNOWN = 0,
	AU_Taurus1,
	AU_Taurus2,
}AU_TAURUS_T;

typedef enum 
{
	AU_UNKNOWN = 0,
	AU8502,
	AU8504,
	AU8522,
	AU8523,
	AU8524,
	AU8515,
	AU8800,
	AU8810,
	AU8820,
	AU8822,
	AU8824,
	AU8828,
	IM_AU8522,
}AU_CHIPTYPE_T;

typedef enum
{
	ATSC_FREQ_OFFSET = 0,
	ATSC_TIMING_OFFSET,
	J83B_FREQ_OFFSET,
	J83B_TIMING_OFFSET,
	DTMB_Carrier_Offset_fb,
	DTMB_Symbol_Timing_Offset,
	DVBC_Carrier_Offset_fb,
	DVBC_Symbol_Timing_Offset,
}AU_MODOFFSET_T;

typedef struct 
{
	AU_AUDIOFORMAT_T	AudioFormat;
	AU_AUDIOOUT_T		AudioOutType;
	AU_IIS_FREQ_OUT_T	IISFREQOUT;		/* IIS out frequency */
}AU_ATVAudioFormat;

typedef struct 
{
	UINT16	SignalQuality;
	UINT16	SignalLevel;
	Carrier_Mode_T	CarrierMode;
	QAM_Type_T	QAMType;
	Code_Rate_T	CodeRate;
	Intl_Mode_T	Intl_Mode;
}AU_SignalState;

typedef struct 
{
	UINT16	Contrast;
	UINT16	Hue;
	UINT16	Brightnes;
	UINT16	Saturation;
	UINT16	Sharpness;
}AU_ATVColorParamete;

typedef struct 
{
	UINT8				CVBSCH;
	UINT8				SVIDEOCH;		/*1 svideo 13ch, 2 svideo 24ch */
	UINT8				CVBSSIFCH;
	UINT8				SIFCH;
	ATV_Au_In_Type_T	SIF_BB;
}AU_ATVParamete;

typedef struct 
{
	UINT16	VolumeLeft;
	UINT16	VolumeRight;
}AU_ATVVolumeValue;

typedef struct 
{
	void*					APHandle;
	AU_CHIPSERIES_T			ChipSeries;
	AU_CHIPTYPE_T			ChipType;
	UINT8					ChipAddress;
	AU_TUNERTYPE_T			TunerType;
	UINT8					TunerAddress;
	UINT32					FreqHz;
	AU_TV_FORMAT_T			ChipMod;
	AU_TS_OUTPUT_T			TSOutput;
	AU_AIRCABLE_T			AirCable;
	AU_TunerCapabilities	TunerCapabilities;
	AU_ATVParamete			ATVParamete;
	AU_ATVAudioFormat		ATVAudioFormat;
	AU_ATVColorParamete		ATVColorParamete;
	AU_ATVVolumeValue		ATVVolumeValue;
	UINT8					PLL_Adj;
	INT8					Auto_AGC;
	UINT8					Rst_ATSC;
	INT8					EnableIIC2;
	UINT8					IsNeedDownloadDSP;
	AU_TAURUS_T				Taurus_Ver;
	UINT8					DVBC_VSR_EN;
	double					DVBC_VSR;	//variable symbol rate of DVB-C
	UINT8					Enable3DFilter;
}AU_ChipParamete;

typedef struct 
{
	AU_TV_FORMAT_T ChipMod;
	AU_TS_OUTPUT_T TSOutput;
	UINT32		   FreqHz;
}AU_ChipModST;


typedef struct 
{
	INT32			(*AU_ChipOpen_Interface)(void* nAPHandle);
	TUNER_NTSTATUS  (*AU_SetTunerCapabilities_Interface)(UINT32 ChipUnit,INT32 nTunerType,AU_TunerCapabilities nTunerCapabilities);
	TUNER_NTSTATUS	(*AU_ChipClose_Interface)(UINT32 ChipUnit);
	TUNER_NTSTATUS	(*AU_SetChipMod_Interface)(UINT32 ChipUnit, AU_ChipParamete p_para);
	TUNER_NTSTATUS  (*AU_GetChipLock_Interface)(UINT32 ChipUnit,UINT32 * nLock);
	TUNER_NTSTATUS  (*AU_GetChipSignalState_Interface)(UINT32 ChipUnit, AU_ChipParamete p_para, AU_SignalState * nSignalState);
	TUNER_NTSTATUS  (*AU_GetModOffset_Interface)(UINT32 ChipUnit,AU_MODOFFSET_T nMod,double *Offset);
	TUNER_NTSTATUS  (*AU_SetATVColorParamete_Interface)(UINT32 ChipUnit,AU_ATVColorParamete nATVColorParamete);
	TUNER_NTSTATUS  (*AU_SetATVAudioFormat_Interface)(UINT32 ChipUnit,AU_ATVAudioFormat nATVAudioFormat);
	TUNER_NTSTATUS  (*AU_SetATVVolumeValue_Interface)(UINT32 ChipUnit,AU_ATVVolumeValue nATVVolumeValue);
	TUNER_NTSTATUS  (*AU_SetATVParamete_Interface)(UINT32 ChipUnit,AU_ATVParamete nATVParamete);
	TUNER_NTSTATUS  (*AU_SettingWithTuner_Interface)(UINT32 ChipUnit);
	TUNER_NTSTATUS  (*AU_OnTime_Interface)(UINT32 ChipUnit);
	UINT32			(*AU_GetDemodAbility_Interface)(UINT32 ChipUnit, AU_CHIPTYPE_T nChipType);
	UINT32			(*AU_GetAudioAbility_Interface)(UINT32 ChipUnit); 

	AU_CHIPTYPE_T			ChipType;
	INT32					ChipHandle;
    INT32					TunerType;
	void*					APHandle;
}AU_ChipInterFaceST;


#if defined( __cplusplus )     
}
#endif

#endif

