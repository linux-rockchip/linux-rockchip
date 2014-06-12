/*******************************************************************************
*                          CONFIDENTIAL                                                                                         
*        Hangzhou NationalChip Science and Technology Co., Ltd.                                               
*                      (C)2009, All right reserved                                                                            
********************************************************************************

********************************************************************************
* File Name	 :   	gx1131.h                                                                                                 
* Author		 :     Mike Feng                                                                                                  
* Project    	 :   	GXAPI 
* Type      	 :     Driver
********************************************************************************
* Purpose   :   GX1131  driver
********************************************************************************
* Release History:
* VERSION   Date              AUTHOR          Description
*    1.6      2011.06.24      	Jerome Lou   
********************************************************************************
*Abbreviation
*   GX		--	GUOXIN 
*   RF		--  radiate frequency
*   SNR		--	signal to noise ratio
*   OSC		--	oscillate
*   SPEC		--	spectrum
*   FREQ		--	frequency
********************************************************************************/
#ifndef _GX1131_H_
#define _GX1131_H_

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

#if 1
#define print printk
#else
#define print(...)   ((void)0)
#endif

//typedef int				bool;
//typedef unsigned char		u8;				/* Unsigned  8 bit quantity       */
//typedef char				s8;				/* Signed    8 bit quantity       */
//typedef unsigned short		u16;			/* Unsigned 16 bit quantity       */
//typedef signed short 		s16;			/* Signed   16 bit quantity       */
//typedef unsigned long		u32;			/* Unsigned 32 bit quantity       */
//typedef long				s32;			/* Signed   32 bit quantity       */
typedef unsigned int				f32;			/* Single precision floating point*/
typedef unsigned long			f64;			/* Double precision floating point*/

typedef unsigned long GX1131_STATE;

typedef enum _GX1131_POS_CTRL
{
	LNB_CTRL_LIMIT_WEST,
	LNB_CTRL_DRIVE_WEST,
	LNB_CTRL_LIMIT_EAST,
	LNB_CTRL_DRIVE_EAST,
	LNB_CTRL_STOP,
	LNB_CTRL_LIMIT_OFF,
	LNB_CTRL_STORE_NN,
	LNB_CTRL_GOTO_NN,
	LNB_CTRL_GOTO_XX 	,
	LNB_CTRL_SET_POSNS
}GX1131_POS_CTRL;

typedef enum _GX1131_MODU_TYPE
{
	UNKNOWN,
	DVBS,
	DIRECTV,
	DVBS2
	
}GX1131_MODU_TYPE;

typedef enum _GX1131_CODE_RATE_S
{
	CRXX,
	CR12,
	CR23,
	CR34,
	CR45,
	CR56,
	CR67,
	CR78	
}GX1131_CODE_RATE_S;

typedef enum _GX1131_CODE_RATE_S2
{
	CRXXX,
	QPSK14,
	QPSK13,
	QPSK25,
	QPSK12,
	QPSK35,
	QPSK23,
	QPSK34,
	QPSK45,
	QPSK56,
	QPSK89,
	QPSK910,
	_8PSK35,
	_8PSK23,
	_8PSK34,
	_8PSK56,
	_8PSK89,
	_8PSK910	
}GX1131_CODE_RATE_S2;

typedef enum _GX1131_LOCK_STATUS
{
	UNLOCKED,
	AGC_LOCKED,
	BCS_LOCKED,
	TIM_LOCKED,
	CRL_LOCKED,
	EQU_LOCKED,
	FEC_LOCKED
}GX1131_LOCK_STATUS;

typedef struct _GX1131_FINE_OFFSET
{	
	s32	fc_offset_fine_K;
	s32	fs_offset_fine_Sps;
}GX1131_FINE_OFFSET;

typedef struct _GX1131_TP_INFO
{
	u32			Freq_true_KHz;
	u32			SymolRate_true_Sps;
	bool			V_polor;
	bool			_22K_on;
	GX1131_MODU_TYPE		modu_mode;
	GX1131_CODE_RATE_S		code_rate_s;
	GX1131_CODE_RATE_S2	code_rate_s2;
}GX1131_TP_INFO;

typedef struct _GX1131_TP
{
	u32	Freq_KHz;
	u32	SymRate_KSps;
	bool	V_polar;
	bool	_22K_on;
}GX1131_TP;

#define BS_CB_MAIN_STAGE 1
#define BS_CB_MAX_MAIN   2
#define BS_CB_SUB_STAGE  4
#define BS_CB_MAX_SUB    8
#define BS_CB_RESULT     16

typedef struct _GX1131_BsBack
{
	u32 nMaskUpdate;
	u32 nMainStage;
	u32 nMaxMain;
	u32 nSubStage;
	u32 nMaxSub;
	GX1131_TP_INFO tpinfo;
	void *pPrivate;
}GX1131_BsBack;

typedef struct _GX1131_BCS_TYPE
{
	u32 Start_Freq_MHz;
	u32 End_Freq_MHz;
	bool Ku_LNB;
	bool dual_local_osc_LNB;
	u16 LocalFre1;
	u16 LocalFre2;
	u32 SearchMode;//0: H;1:V;2:ALL
	bool _22k_state;
}GX1131_BCS_TYPE;

typedef GX1131_STATE (*TunerDriver)(u32 RFfreq,u32 Symbol_Rate);
typedef bool (*bcs_callback)(GX1131_BsBack *bsback);

#define  FALSE				0
#define  TRUE				1

#define WRITE				1
#define READ  				0

#define SUCCESS				1
#define FAILURE				0
	
#define TP_UNLOCK			0
#define TP_LOCK				1
#define TP_BAD				2

#define ENABLE				1
#define DISABLE				0

#define POL_SWITCH			0							/*18V/13V select, 0--low level for 18V and high level for 13V*/

#define IIC_REPEAT_SPEED      0							/*0--IIC speed below 150K,1--IIC speed from 150K to 300K,2--IIC speed from300K to 400K*/							                       

#define DEMO_PCB			3

#define TUNER_ZL10037		0							/*Select a tuner to use*/
#define TUNER_SHARP7306	  0
#define TUNER_RDA5812		0
#define TUNER_AV2011		1
#define TUNER_STV6110A		0
#define TUNER_LW37     		0
#define TUNER_WZ5001		0

#define 		GX1131_CHIP_ADDRESS  			0xD0 
#define  	 	GX1131_OSCILLATE_FREQ		27	

#define	 	GX1131_TS_OUTPUT_MODE		1  			/*1: parallel output, 0:serial output*/
#define		GX1131_TS_CLK_POL 			0 			/*1: change the polarity of TS clk, 0: don't change*/

#if(DEMO_PCB==0)
#define 	  	CFG_TS_0						0x89		/*this default configure is compatible with GX3001 decoder board*/
#define 	  	CFG_TS_2						0xBA
#define 	  	CFG_TS_4						0x67
#define 	  	CFG_TS_6						0x45
#define 	  	CFG_TS_8						0x23
#define 	  	CFG_TS_A						0x01
#endif

#if(DEMO_PCB==1)
#define 	  	CFG_TS_0						0x10		/*this default configure is compatible with GX3001 decoder board*/
#define 	  	CFG_TS_2						0x32
#define 	  	CFG_TS_4						0x54
#define 	  	CFG_TS_6						0x76
#define 	  	CFG_TS_8						0xAB
#define 	  	CFG_TS_A						0x98
#endif

#if(DEMO_PCB==2) //gx1131 pare demo board
#define 	  	CFG_TS_0						0x10		/*this default configure is compatible with GX3001 decoder board*/
#define 	  	CFG_TS_2						0x32
#define 	  	CFG_TS_4						0x54
#define 	  	CFG_TS_6						0x76
#define 	  	CFG_TS_8						0xA8
#define 	  	CFG_TS_A						0xB9
#endif

#if(DEMO_PCB==3) //gx1131 av2011 onboard
#define 	  	CFG_TS_0						0xB8		/*this default configure is compatible with GX3001 decoder board*/
#define 	  	CFG_TS_2						0x67
#define 	  	CFG_TS_4						0x45
#define 	  	CFG_TS_6						0x23
#define 	  	CFG_TS_8						0x01
#define 	  	CFG_TS_A						0x9a
#endif

#if(DEMO_PCB==4)  //GSKA-89
#define 	  	CFG_TS_0						0x10		/*this default configure is compatible with GX3001 decoder board*/
#define 	  	CFG_TS_2						0x32
#define 	  	CFG_TS_4						0x54
#define 	  	CFG_TS_6						0x76
#define 	  	CFG_TS_8						0xA8
#define 	  	CFG_TS_A						0xB9
#endif

#define	  	GX1131_PLL1_D_VALUE			3			/*0: not allowed, 1: NO1=1, 2: NO1=2, 3: NO1=4*/
#define  	  	GX1131_PLL1_R_VALUE			10
#define  	  	GX1131_PLL1_B_VALUE			68			/*fpll1=fosc*2*pll1_b/(pll1_r*NO1)*/

#define  	  	GX1131_PLL2_D_VALUE			3			/*0: not allowed, 1: NO2=1, 2: NO2=2, 3: NO2=4*/
#define  	  	GX1131_PLL2_R_VALUE			10	
#define  	  	GX1131_PLL2_B_VALUE			85			/*fpll2=fosc*2*pll2_b/(pll2_r*NO2)*/

#define  	  	GX1131_FSAMPLE_VALUE			91800		/*the clk frequency of ADC sampling, same with sysclk, unit: KHz*/

#define  	  	GX1131_PLL_CFG_NUM_VALUE	4			/*pll1/pll2=Cfg_Num/CFG_Denom*/
#define  	  	GX1131_PLL_CFG_DENOM_VALUE	5

#define 	  	GX1131_AGC_STD_VALUE			28

#define		GX1131_BS_STEP_MHZ 			12
#define		GX1131_BS_WINDOW_SIZE_K		45000
#define		_FREQ_LOW_LIM_MHZ_	950
#define		_FREQ_HIGH_LIM_MHZ_	2150


#if (TUNER_ZL10037==1)										/*Select the polarity of agc accoring different tuner*/
		#define  	  	GX1131_AGC_POLARITY		1
#endif
#if (TUNER_SHARP7306==1)
		#define  	  	GX1131_AGC_POLARITY		1
#endif
#if (TUNER_RDA5812==1)
		#define  	  	GX1131_AGC_POLARITY		1
#endif
#if (TUNER_AV2011==1)
		#define  	  	GX1131_AGC_POLARITY		1
#endif
#if (TUNER_LW37==1)
		#define  	  	GX1131_AGC_POLARITY		1
#endif
#if (TUNER_WZ5001==1)
		#define  	  	GX1131_AGC_POLARITY		1
#endif
#if (TUNER_STV6110A==1)
		#define  	  	GX1131_AGC_POLARITY		0
#endif
/*-- Register Address Defination begin ---------------*/
#define GX1131_CHIP_ID_H				0x00
#define GX1131_CHIP_ID_L				0x01
#define GX1131_CHIP_VERSION			0x02
#define GX1131_CTRL0					0x04
#define GX1131_CTRL1					0x05
#define GX1131_I2C_REPEAT				0x06
#define GX1131_PLL1_CTRL_RD			0x08
#define GX1131_PLL1_CTRL_B				0x09
#define GX1131_PLL2_CTRL_RD			0x0A
#define GX1131_PLL2_CTRL_B				0x0B
#define GX1131_ADC_CTRL0				0x0C
#define GX1131_ADC_CTRL1				0x0D
#define GX1131_PLL_Cfg_Num				0x0E
#define GX1131_PLL_Cfg_Denom			0x0F

#define GX1131_DISEQC_MODE			0x10
#define GX1131_DISEQC_RATIO_L			0x11
#define GX1131_DISEQC_RATIO_H			0x12
#define GX1131_DISEQC_GUARD			0x13
#define GX1131_DISEQC_WR_EN			0x14
#define GX1131_DISEQC_WR_CTRL			0x15
#define GX1131_DISEQC_RD_INT			0x16
#define GX1131_DISEQC_STATE			0x17

#define GX1131_DISEQC_INS1				0x20
#define GX1131_DISEQC_INS2				0x21
#define GX1131_DISEQC_INS3				0x22
#define GX1131_DISEQC_INS4				0x23
#define GX1131_DISEQC_INS5				0x24
#define GX1131_DISEQC_INS6				0x25
#define GX1131_DISEQC_INS7				0x26
#define GX1131_DISEQC_INS8				0x27
#define GX1131_DISEQC_RD_RESP1		0x28
#define GX1131_DISEQC_RD_RESP2		0x29
#define GX1131_DISEQC_RD_RESP3		0x2A
#define GX1131_DISEQC_RD_RESP4		0x2B
#define GX1131_DISEQC_RD_RESP5		0x2C
#define GX1131_DISEQC_RD_RESP6		0x2D
#define GX1131_DISEQC_RD_RESP7		0x2E
#define GX1131_DISEQC_RD_RESP8		0x2F

#define GX1131_RST						0x30
#define GX1131_ALL_OK					0x31
#define GX1131_NOISE_POW_L			0x32
#define GX1131_NOISE_POW_H			0x33
#define GX1131_MODULE_RST				0x34
#define GX1131_CLK_OK_SEL				0x35
#define GX1131_AUTO_RST				0x36
#define GX1131_WAIT_LENGTH			0x37
#define GX1131_DATA_SEL				0x38
#define GX1131_TRIG_POS				0x39
#define GX1131_DOWN_SAMPLE			0x3A
#define GX1131_DATA_CATCH				0x3B
#define GX1131_PDM_VALUE				0x3C
#define GX1131_TST_MOD_SEL			0x3F

#define GX1131_AGC_SPEED				0x40
#define GX1131_AGC_STD					0x41
#define GX1131_AGC_CTRL_OUT_L			0x42
#define GX1131_AGC_CTRL_OUT_H			0x43
#define GX1131_AGC_AMP_ERR			0x44
#define GX1131_AGC_SET_DATA			0x45
#define GX1131_AGC_MODE				0x46
#define GX1131_AGC_PDM_CLK			0x47
#define GX1131_AGC_LOST_TIME			0x48
#define GX1131_AGC_AMP					0x49
#define GX1131_SNR_DET_N_TIM_L		0x4A
#define GX1131_SNR_DET_N_TIM_H		0x4B

#define GX1131_MIN_FS					0x50
#define GX1131_BCS_BND1				0x51
#define GX1131_BCS_BND2				0x52
#define GX1131_BCS_BND3				0x53
#define GX1131_BCS_BND4				0x54
#define GX1131_BCS_BND5				0x55
#define GX1131_BCS_RST					0x56
#define GX1131_SAME_FC					0x57
#define GX1131_BCS_OUT_ADDR			0x58
#define GX1131_VID_SIG_NUM				0x59
#define GX1131_BCS_FC_OFFSET_L		0x5A
#define GX1131_BCS_FC_OFFSET_H		0x5B
#define GX1131_BCS_FS_L				0x5C
#define GX1131_BCS_FS_H				0x5D
#define GX1131_BCS_SIG_AMP				0x5E
#define GX1131_BCS_SIG_POW			0x5F

#define GX1131_Fsample_Cfg_L			0x70
#define GX1131_Fsample_Cfg_M			0x71
#define GX1131_Fsample_Cfg_H			0x72
#define GX1131_SYMBOL_L				0x73
#define GX1131_SYMBOL_H				0x74
#define GX1131_FC_OFFSET_L				0x75
#define GX1131_FC_OFFSET_H				0x76
#define GX1131_DAGC_CTL				0x77
#define GX1131_DAGC_STD				0x78
#define GX1131_TIM_LOOP_BW			0x79
#define GX1131_TIM_OK_BOUND			0x7A
#define GX1131_TIM_LOCK_CNT			0x7B
#define GX1131_TIM_LOST_CNT			0x7C
#define GX1131_INTG_OUT				0x7D
#define GX1131_SFIC_CTL1				0x7E
#define GX1131_SFIC_CTL2				0x7F

#define GX1131_FLAG_DMY				0x80
#define GX1131_PLH_THRESH				0x81
#define GX1131_FSCAN_RANGE			0x82
#define GX1131_FRAME_EST_NUM			0x83
#define GX1131_PD_GAIN 					0x84
#define GX1131_PFD_ALPHA				0x85
#define GX1131_BBC_BW_CTRL			0x86
#define GX1131_BW_CTRL2				0x87
#define GX1131_BW_PHASE_SEL			0x88
#define GX1131_FB_FSCAN				0x89
#define GX1131_BBC_OK_BOUND			0x8A
#define GX1131_BBC_LOCK_COUNT			0x8B
#define GX1131_FREQ_BACK_L				0x8C
#define GX1131_FREQ_BACK_H			0x8D
#define GX1131_SCAN_SPEED				0x8E
#define GX1131_MSK_PLL_OK				0x8F

#define GX1131_EQU_SPEED				0x90
#define GX1131_EQU_CTL					0x91
#define GX1131_SNR_DET_N_FINE_L		0x92
#define GX1131_SNR_DET_N_FINE_H		0x93
#define GX1131_AWGN_POW_L			0x94
#define GX1131_AWGN_POW_H			0x95
#define GX1131_SCRAM_1X1Y0X_H			0x96
#define GX1131_SCRAM_1X_M				0x97
#define GX1131_SCRAM_1X_L				0x98
#define GX1131_SCRAM_1Y_M				0x99
#define GX1131_SCRAM_1Y_L				0x9A
#define GX1131_SCRAM_0X_M				0x9B
#define GX1131_SCRAM_0X_L				0x9C
#define GX1131_FFE_CTRL1				0x9D
#define GX1131_FFE_CTRL2                          0x9E
#define GX1131_ERR_BND_AMP_RATE		0x9F

#define GX1131_LDPC_CTRL				0xA0
#define GX1131_LDPC_ITER_NUM			0xA1
#define GX1131_LDPC_SIGMA_NUM			0xA2

#define GX1131_BCH_ERR_THR				0xB0
#define GX1131_BCH_ERR_NUM_L			0xB1
#define GX1131_BCH_ERR_NUM_H			0xB2
#define GX1131_BCH_TST_SEL				0xB3
#define GX1131_BCH_ERR_STA_LEN_MOD	0xB4

#define GX1131_VIT_LEN_OPT				0xC0
#define GX1131_ICD_ERR_THR12			0xC1
#define GX1131_ICD_ERR_THR23			0xC2
#define GX1131_ICD_ERR_THR34			0xC3
#define GX1131_ICD_ERR_THR56			0xC4
#define GX1131_ICD_ERR_THR67			0xC5
#define GX1131_ICD_ERR_THR78			0xC6
#define GX1131_VIT_SYS_SEL				0xC7
#define GX1131_VIT_IQ_SHIFT			0xC8


#define GX1131_RSD_ERR_STATIC_CTRL	0xD0
#define GX1131_ERR_OUT_0				0xD1
#define GX1131_ERR_OUT_1				0xD2
#define GX1131_ERR_OUT_2				0xD3
#define GX1131_ERR_OUT_3				0xD4

#define GX1131_TS_MODE					0xE0
#define GX1131_MIS_DIST				0xE1
#define GX1131_CFG_MATYPE2			0xE2
#define GX1131_PKT_LEN_SEL				0xE3
#define GX1131_CRC_ERR_THRESH_L		0xE4
#define GX1131_CRC_ERR_THRESH_H		0xE5
#define GX1131_CFG_TS_0				0xE6
#define GX1131_CFG_TS_2				0xE7
#define GX1131_CFG_TS_4				0xE8
#define GX1131_CFG_TS_6				0xE9
#define GX1131_CFG_TS_8				0xEA
#define GX1131_CFG_TS_A				0xEB
#define GX1131_CRC_ERR_SUM_L			0xEC
#define GX1131_CRC_ERR_SUM_H			0xED
#define GX1131_MODU_MODE				0xEE
#define GX1131_S2_MODE_CODE			0xEF

#define GX1131_MATYPE1					0xF0
#define GX1131_MATYPE2					0xF1
#define GX1131_UPL_L					0xF2
#define GX1131_UPL_H					0xF3
#define GX1131_SYNC						0xF4
/*-- Register Address Defination end ---------------*/

/*-- diseqc control byte(standard) ---------------*/
#define	GX1131_DISEQC_CMD1			0xE0
#define	GX1131_DISEQC_CMD2			0x10
#define	GX1131_DISEQC_CMD3			0x38
#define	GX1131_DISEQC_CMD3_1			0x39
#define	GX1131_DISEQC_CMD4_LNB1		0xF0
#define	GX1131_DISEQC_CMD4_LNB2		0xF4
#define	GX1131_DISEQC_CMD4_LNB3		0xF8
#define	GX1131_DISEQC_CMD4_LNB4		0xFC

#define	GX1131_DISEQC_CMD4_SW01               0xF0
#define	GX1131_DISEQC_CMD4_SW02               0xF1
#define	GX1131_DISEQC_CMD4_SW03               0xF2
#define	GX1131_DISEQC_CMD4_SW04               0xF3
#define	GX1131_DISEQC_CMD4_SW05               0xF4
#define	GX1131_DISEQC_CMD4_SW06               0xF5
#define	GX1131_DISEQC_CMD4_SW07               0xF6
#define	GX1131_DISEQC_CMD4_SW08               0xF7
#define	GX1131_DISEQC_CMD4_SW09               0xF8
#define	GX1131_DISEQC_CMD4_SW10               0xF9
#define	GX1131_DISEQC_CMD4_SW11               0xFA
#define	GX1131_DISEQC_CMD4_SW12               0xFB
#define	GX1131_DISEQC_CMD4_SW13               0xFC
#define	GX1131_DISEQC_CMD4_SW14               0xFD
#define	GX1131_DISEQC_CMD4_SW15               0xFE
#define	GX1131_DISEQC_CMD4_SW16               0xFF
/*-- diseqc control byte(standard) ---------------*/

GX1131_STATE GX1131_set_tuner(u32 RFfreq,u32 Symbol_Rate);
GX1131_STATE GX_Set_RDA5812(u32 RFfreq,u32 Symbol_Rate);
GX1131_STATE GX_Set_Sharp7306(u32 RFfreq,u32 Symbol_Rate);
GX1131_STATE GX_Set_ZL10037(u32 RFfreq,u32 Symbol_Rate);
GX1131_STATE GX_Set_AV2011( u32 RFfreq, u32 Symbol_Rate);
GX1131_STATE GX_Set_STV6110A(u32 RFfreq,u32 Symbol_Rate);
GX1131_STATE GX_Set_LW37(u32 RFfreq,u32 Symbol_Rate);
GX1131_STATE GX_Set_WZ5001(u32 RFfreq,u32 Symbol_Rate);

void GX_Delay_N_ms(u32 ms_value);
bool GX_I2cReadWrite(u8 WR_flag, u8 ChipAddress,u8 RegAddress,u8 *data, u8 data_number);
s8 *GX1131_Get_Driver_Version(void);
bool GX1131_Write_one_Byte(u8 RegAddress,u8 WriteValue);
bool GX1131_Write_one_Byte_ReadTest(u8 RegAddress,u8 WriteValue);
u8 GX1131_Read_one_Byte(u8 RegAddress);
GX1131_STATE GX1131_Get_Chip_ID(void);
bool GX1131_HotReset_CHIP(void);
bool GX1131_CoolReset_CHIP(void);
bool GX1131_ADC_Init(void);
bool GX1131_PLL_Control(void);
bool GX1131_Set_Sleep(bool Sleep);
bool GX1131_Set_Tuner_Repeater_Enable(bool Rpt_On);
bool GX1131_Set_AGC_Parameter(void);
bool GX1131_Set_TS_Inteface(void);
bool GX1131_Set_Work_BS_Mode(bool bs);
s32 GX1131_SetSymbolRate(u32 Symbol_Rate_Value);
bool GX1131_Set_Polar(bool Vpolor);
bool GX1131_22K_sw(bool b22k_on);
bool GX1131_Sel_Lnb(u8 Lnb_Num);
bool GX1131_Diseqc_11(u8 chCom,u8 Lnb_Num);
bool GX1131_Pos_Ctl(GX1131_POS_CTRL Pos_Ctl,u8 Record_byte);
u8 GX1131_Change2percent(u32 value,u32 low,u32 high);
f32 GX1131_100Log(s32 iNumber_N);
bool GX1131_Init_Chip(void);
GX1131_LOCK_STATUS GX1131_Read_Lock_Status(void);
u32 GX1131_Get_100SNR_Tim(void);
u32  GX1131_Get_100SNR(void);
u8 GX1131_Get_Signal_Strength(void);
u8 GX1131_Get_Signal_Quality(void);
u32 GX1131_Get_ErrorRate(u8 *E_param);
bool GX1131_lock_TP(GX1131_TP tp,  TunerDriver tunerDriver);
bool GX1131_lock_TP_BS(GX1131_TP tp,  TunerDriver tunerDriver);
bool GX1131_Search_Signal(GX1131_TP tp,  TunerDriver tunerDriver);
GX1131_FINE_OFFSET GX1131_Get_Fine_Offset(void);
GX1131_TP_INFO GX1131_Get_TP_Info(u32 RF_Freq_K,u32 Symbol_Rate_K);

u32 GX1131_BS_one_Window(u32 Fcenter,u32 Lpf_BW_window,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver);
u32 GX1131_BS_Seg_Freq(u32 Start_Freq,u32 End_Freq,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver);
u32 GX1131_BS_TP_Sort_Seg_Freq(u32 Start_Freq,u32 End_Freq,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver);
bool GX1131_BS_Lock_List(u32 Start_Freq,u32 End_Freq,GX1131_TP *bstp_list,u32 size,
							  TunerDriver tunerDriver,bcs_callback lock_cb,GX1131_BsBack *BsbackPar);
bool GX1131_BS_lock_a_Satellite(GX1131_BCS_TYPE *bcs_type,GX1131_TP *bstp_list,u32 size,TunerDriver tunerDriver,
                                                               bcs_callback lock_cb,GX1131_BsBack *BsbackPar);
//GX1131_STATE GX1131_set_tuner(u32 RFfreq,u32 Symbol_Rate);
bool GX1131_DATA_CATCH_AGC(u32 Start_freq_MHz,u32 End_freq_MHz,u32 BW_window,
								TunerDriver tunerDriver);
bool GX1131_Set_Ts_Out_Disable(bool  ts_off);
bool GX1131_Const(void);
//GX1131_STATE Tuner_control(u32 RFfreq, u32 Symbol_Rate);

#endif /* _GX1131_H_ */
