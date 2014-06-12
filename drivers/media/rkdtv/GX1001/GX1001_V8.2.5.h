//请参考《GX10011001P软件包说明-V8.2.5.doc》
//Please refer to <GX10011001 Software Developer Kit User's Manual_V8.2.5>
/*
   Abbreviation
   GX1001		--	GUOXIN 
   IF		--	S32ermediate frequency
   RF		--      radiate frequency
   SNR		--	signal to noise ratio
   OSC		--	oscillate
   SPEC	--	spectrum
   FREQ	--	frequency
   */

//#ifndef __GX1001_V8_2_5_H__
//#define __GX1001_V8_2_5_H__

//#include "stddefs.h"
//#include "data_type.h"
//#include "frontend.h"
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/i2c.h>
#include <linux/kthread.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>

#include <asm/system.h>

//#include "gxav_frontend_propertytypes.h"					//dingfb20100622

typedef unsigned   int 	U32;
typedef int 	S32;
typedef unsigned   short 	U16;
typedef short 	S16;
typedef unsigned   char 	U8;
typedef char 	S8;
//typedef int fe_tunertype_t;
    typedef enum fe_tunertype
    {
        TUNER_ALPSTDQE3 = 0,                      
        TUNER_ALPSTDAE,                     
        TUNER_USER1,                     
    }   fe_tunertype_t, *pfe_tunertype_t;

#define NEWONE  1
#define OLDONE  -1

#define SUCCESS  true
#define FAILURE  false

#define ENABLE    1
#define DISABLE   0

#define WRITE     1
#define READ      0

#define  GX1001_CHIP_ADDRESS	  0x18    /*GX10011001 chip address*/

#define  GX1001_OSCILLATE_FREQ    28800	  //(oscillate frequency) ( Unit: KHz ) 
#define  GX1001_IF_FREQUENCY	  5000	  //(tuner carrier center frequency) ( Unit: KHz ) 

#define  GX1001_TS_OUTPUT_MODE    1       // 1: Parallel output,  0: Serial output
#define  GX1001_PLL_M_VALUE		  0x0b    // This parameter is effective only for GX1001B
#define  GX1001_PLL_N_VALUE		  0x00    // This parameter is effective only for GX1001B
#define  GX1001_PLL_L_VALUE		  0x05    // This parameter is effective only for GX1001B
#define  GX1001_RF_AGC_MIN_VALUE  0x00    // This parameter is effective only for GX1001B
#define  GX1001_RF_AGC_MAX_VALUE  0xff    // This parameter is effective only for GX1001B
#define  GX1001_IF_AGC_MIN_VALUE  0x00    // This parameter is effective only for GX1001B
#define  GX1001_IF_AGC_MAX_VALUE  0xff    // This parameter is effective only for GX1001B
#define  SFenable                 DISABLE //DISABLE :do not open the sf function,ENABLE:open the sf function
#define  FMenable                 ENABLE  //DISABLE:do not open the fm function,ENABLE:opne the fm function


/*-- Register Address Defination begin ---------------*/

#define GX1001_CHIP_ID                    0x00
#define GX1001_MAN_PARA                0x10
#define GX1001_INT_PO1_SEL            0x11
#define GX1001_SYSOK_PO2_SEL       0x12
#define S32_IND                                  0x13
#define GX1001_TST_SEL                    0x14
#define GX1001_I2C_RST                    0x15
#define GX1001_MAN_RST                   0x16
#define GX1001_BIST                          0x18
#define GX1001_MODE_AGC                0x20
#define GX1001_AGC_PARA                 0x21
#define GX1001_AGC2_THRES              0x22
#define GX1001_AGC12_RATIO             0x23
#define GX1001_AGC_STD                 0x24
#define GX1001_SCAN_TIME               0x25
#define GX1001_DCO_CENTER_H            0x26
#define GX1001_DCO_CENTER_L            0x27
#define GX1001_BBC_TST_SEL             0x28
#define GX1001_AGC_ERR_MEAN            0x2B
#define GX1001_FREQ_OFFSET_H           0x2C
#define GX1001_FREQ_OFFSET_L           0x2D
#define GX1001_AGC1_CTRL               0x2E
#define GX1001_AGC2_CTRL               0x2F
#define GX1001_FSAMPLE_H               0x40
#define GX1001_FSAMPLE_M               0x41
#define GX1001_FSAMPLE_L               0x42
#define GX1001_SYMB_RATE_H             0x43
#define GX1001_SYMB_RATE_M             0x44
#define GX1001_SYMB_RATE_L             0x45
#define GX1001_TIM_LOOP_CTRL_L         0x46
#define GX1001_TIM_LOOP_CTRL_H         0x47
#define GX1001_TIM_LOOP_BW             0x48
#define GX1001_EQU_CTRL                0x50
#define GX1001_SUM_ERR_POW_L           0x51
#define GX1001_SUM_ERR_POW_H           0x52
#define GX1001_EQU_BYPASS              0x53
#define GX1001_EQU_TST_SEL             0x54
#define GX1001_EQU_COEF_L              0x55
#define GX1001_EQU_COEF_M              0x56
#define GX1001_EQU_COEF_H              0x57
#define GX1001_EQU_IND                 0x58
#define GX1001_RSD_CONFIG              0x80
#define GX1001_ERR_SUM_1               0x81
#define GX1001_ERR_SUM_2               0x82
#define GX1001_ERR_SUM_3               0x83
#define GX1001_ERR_SUM_4               0x84
#define GX1001_RSD_DEFAULT             0x85
#define GX1001_OUT_FORMAT              0x90

/*---GX1001B New Address Defination Start--*/
#define GX1001_CHIP_IDD					0x01
#define GX1001_CHIP_VERSION				0x02
#define GX1001_PLL_M					0x1A	
#define GX1001_PLL_N					0x1B	 
#define GX1001_PLL_L					0x1B	
#define GX1001_MODE_SCAN_ENA			0x29	
#define GX1001_FM_CANCEL_CTRL			0x30	
#define GX1001_SF_DECT_FM 				0x30
#define GX1001_FM_CANCEL_ON				0x30
#define GX1001_FM_SUB_ENA				0x30	
#define GX1001_FM_ENA_FM				0x30
#define GX1001_FM_UNLOCK_TIME			0x30
#define GX1001_FM_SUBENA_TIME			0x30	
#define GX1001_FM_CANCEL_LMT			0x31
#define GX1001_FM_UNLOCK_LMT			0x31
#define GX1001_FM_SUBENA_LMT			0x31		
#define GX1001_SF_CANCEL_CTRL			0x32		
#define GX1001_SF_DECT_SF				0x32	
#define GX1001_SF_BUSY					0x32	
#define GX1001_SF_LOCKED				0x32		
#define GX1001_FM_ENA_SF				0x32		
#define GX1001_SF_BW_SEL				0x32		
#define GX1001_SCAN_TOUT_SET			0x32	
#define GX1001_SF_LMT_FM				0x33		
#define GX1001_SF_LMT_H					0x34		
#define GX1001_SF_LMT_L					0x35		
#define GX1001_AGC_SET					0x39		
#define	GX1001_AGC_HOLD					0x39	
#define GX1001_RF_SET_EN				0x39	
#define GX1001_IF_SET_EN				0x39	
#define GX1001_RF_SET_DAT				0x3A		
#define GX1001_IF_SET_DAT				0x3B		
#define GX1001_RF_MIN					0x3C		
#define	GX1001_RF_MAX					0x3D		
#define	GX1001_IF_MIN					0x3E		
#define GX1001_IF_MAX					0x3F		
#define GX1001_SF_FREQ_OUT_H			0xA0		
#define GX1001_SF_FREQ_OUT_L			0xA1		
#define GX1001_FM_FREQ_OUT_H			0xA2		
#define GX1001_FM_FREQ_OUT_L			0xA3      
#define GX1001_AUTO_THRESH				0xA6		
#define GX1001_THRESH_AUTO				0xA6	
#define GX1001_THRESH_STEP				0xA6
#define GX1001_THRESH_OUT				0xA7		
#define GX1001_TIM_JIT_BOUND			0x49		
#define GX1001_TIM_SCAN_SPEED			0x4A		
#define GX1001_TIM_SCAN_ENA				0x4B		
#define GX1001_AGC_LMT_2DELTA			0x4C		
#define GX1001_AGC_LMT_3DELTA			0x4D		
#define GX1001_DIGITAL_AGC_ON			0x4E		
#define GX1001_agc_amp_ditter_on		        0x4E
#define GX1001_SPECTRUM_INV				0x60		
#define GX1001_PIN_SEL_1				0x91		
#define GX1001_PIN_SEL_2				0x92
#define GX1001_PIN_SEL_3				0x93
#define GX1001_PIN_SEL_4				0x94
#define GX1001_PIN_SEL_5				0x95
#define GX1001_PIN_SEL_6				0x96

/*--GX1001B New Address Defination End-----*/

/*-- Register Address Defination end ---------------*/


/* -------------------------- User-defined GX1001 software S32erface begin  ---------------------*/
//#define GX1001_printf(fmt, args...)	 printk(fmt, ##args)

extern S32 GXSDK_Get_Signal_Strength(void);
extern S32 GXSDK_Lock_Tuner(U32 Symbol_1,U32 Symbol_2,U8 Spec_Mode,U8 Qam_Size,U32 RF_Freq,U32 Wait_OK_X_ms);
extern S32 GXSDK_Get_ErrorRate(U32 *E_param);
extern S32 GXSDK_Get_State(void);
extern S32 GXSDK_Get_Signal_Quality(void);

/*-------------------------- User-defined GX1001 software S32erface end -------------------------*/

extern S32 GX1001_Set_ALPSTDQE3_RFFrequency(U32 fvalue);

extern S32 GX1001_Set_AGC_Parameter(fe_tunertype_t  tunertype);				//DINGFB 20100622

extern void GX1001_TDA18250_Init_Chip(void);

extern S32 GX1001_Set_TDA18250_RFFrequency(U32 fvalue);

extern unsigned char GX1001_Get_TDA18250_Signal_Strength(void);

/*-------------------------- Control set begin ------------------------*/

extern S32 GX1001_Write_one_Byte(U8 RegAddress,U8 WriteValue);

extern S32 GX1001_Write_one_Byte_NoReadTest(U8 RegAddress,U8 WriteValue);

extern S32 GX1001_Read_one_Byte(U8 RegAddress);

extern S32 GX1001_Init_Chip(fe_tunertype_t  tunertype);

extern S32 GX1001_HotReset_CHIP(void);

extern S32 GX1001_CoolReset_CHIP(void);

extern S32 GX1001_Set_OutputMode(U8 mode);

extern S32 GX1001_Select_DVB_QAM_Size(U8 size);

extern U32 GX1001_Read_Lock_Status(u32 DelayTime,u8 MaskBit);

extern S32 GX1001_Read_EQU_OK(void);

extern S32 GX1001_Set_Tunner_Repeater_Enable(U8 OnOff);

extern S32 GX1001_Get_Version(void);

extern S32 GX1001_Set_Mode_Scan(U8 mode);

extern S32 GX1001_Set_FM(U8 mode);

extern S32 GX1001_Set_SF(U8 mode);

extern S32 GX1001_Set_RF_Min(U8 value);

extern S32 GX1001_Set_RF_Max(U8 value);

extern S32 GX1001_set_IF_Min(U8 value);

extern S32 GX1001_set_IF_Max(U8 value);

extern S32 GX1001_SET_AUTO_IF_THRES(U8 mode);

extern S32 GX1001_Set_Tim_Scan(U8 mode);

extern S32 GX1001_Set_Digital_AGC(U8 mode);

extern U32 GX1001_Set_Sleep(U8 Sleep_Enable);

extern S32 GX1001_Set_Pll_Value(U8 Pll_M_Value,U8 Pll_N_Value,U8 Pll_L_Value);

/*-------------------------- Control set end ---------------------------*/


/*--------------- Symbol recovery begin --------------------------------*/

extern S32 GX1001_SetSymbolRate(U32 Symbol_Rate_Value);
extern S32 GX1001_SetOSCFreq(void);

/*--------------- Symbol recovery end ----------------------------------*/


//===========================================================================================================================================

//extern U32 GX1001_Change2percent(U8 value,U8 low,U8 high);
extern U32 GX1001_Change2percent(U32 value,U32 low,U32 high); //zhangxf modify 20100426
extern U32 GX1001_100Log(U32 iNumber_N);
extern U32 GX1001_Get_SNR(void);
extern U32 GX1001_Get_ALPSTDQE3_Signal_Strength(void);
extern U32 GX1001_Get_ALPSTDAE3_Signal_Strength(void);
extern U32 GX1001_Get_ErrorRate(U32 *E_param);

extern S32 GX1001_SetSpecInvert(U8 Spec_invert);
extern S32 GX1001_Search_Signal(U32 Symbol_1,U32 Symbol_2,U8 Spec_Mode,U8 Qam_Size,U32 RF_Freq,U32 Wait_OK_X_ms,fe_tunertype_t  tunertype);

U32 GX1001_Get_Signal_Quality(void);
//============================================================================================================================================
