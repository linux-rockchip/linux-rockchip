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
* TITLE 	  : RAONTECH TV RF ADC data header file. 
*
* FILENAME    : raontv_rf_adc_data.h
*
* DESCRIPTION : 
*		All the declarations and definitions necessary for the setting of RF ADC.
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
********************************************************************************/

#if (RTV_SRC_CLK_FREQ_KHz == 13000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x0D, 0x01, 0x1F, 0x27, 0x07, 0x80, 0xB9}, // Based 13MHz, 8MHz 
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 13MHz, 8.192MHz
	{0x0D, 0x01, 0x1F, 0x27, 0x07, 0xB0, 0xB9}, // Based 13MHz, 9MHz 
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}  // Based 13MHz, 9.6MHz
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x99}, {0x39, 0x9C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x70}, {0x39, 0x6C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x70}, {0x39, 0x6C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x70}, {0x39, 0x6C}};
 #endif

  
#elif (RTV_SRC_CLK_FREQ_KHz == 16000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x04, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8},	// Based 16MHz,	8MHz	   External Clock4
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 16MHz, 8.192MHz
	{0x04, 0x01, 0x0F, 0x27, 0x07, 0x6C, 0xB8}, // Based 16MHz, 9MHz	   External Clock5
	{0x05, 0x01, 0x1F, 0x27, 0x07, 0x90, 0xB8}	// Based 16MHz, 9.6MHz	   External Clock6
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x7D}, {0x39, 0x7C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x5B}, {0x39, 0x5C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x5B}, {0x39, 0x5C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x5B}, {0x39, 0x5C}};
 #endif

	
#elif (RTV_SRC_CLK_FREQ_KHz == 16384)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
  	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 16.384MHz, 8MHz
	{0x04, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8},	// Based 16.384MHz,	8.192MHz   External Clock7
	{0x10, 0x01, 0x1F, 0x27, 0x07, 0x77, 0xB9},	// Based 16.384MHz,	8MHz	   External Clock8
	{0x08, 0x01, 0x1F, 0x27, 0x06, 0xE1, 0xB8}	// Based 16.384MHz,	9.6MHz	   External Clock9
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x7A}, {0x39, 0x7C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x59}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x59}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x59}, {0x39, 0x4C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 18000)	
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] = 
  {
	{0x06, 0x01, 0x13, 0x25, 0x06, 0x80, 0xB4},	// Based 18MHz,	8MHz	   External Clock10
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 18MHz, 8.192MHz
	{0x06, 0x01, 0x13, 0x25, 0x06, 0x90, 0xB4},	// Based 18MHz,	9MHz	   External Clock11
	{0x05, 0x01, 0x13, 0x25, 0x06, 0x80, 0xB4}	// Based 18MHz,	9.6MHz	   External Clock12
  };
  
 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x6F}, {0x39, 0x6C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x51}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x51}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x51}, {0x39, 0x4C}};
 #endif
  

#elif (RTV_SRC_CLK_FREQ_KHz == 19200)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x04, 0x01, 0x0B, 0x23, 0x06, 0x50, 0xB0},	// Based 19.2MHz, 8MHz
	{0x19, 0x01, 0x1F, 0x3A, 0x0A, 0x00, 0xA2},	// Based 19.2MHz, 8.192MHz
	{0x04, 0x01, 0x0B, 0x23, 0x06, 0x5A, 0xB0},	// Based 19.2MHz, 9MHz
	{0x04, 0x01, 0x0B, 0x23, 0x06, 0x60, 0xB0}	// Based 19.2MHz, 9.6MHz
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x68}, {0x39, 0x6C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x4C}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x4C}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x4C}, {0x39, 0x4C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 24000)	
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x06, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8},	// Based 24MHz,	8MHz	   External Clock17
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 24MHz, 8.192MHz
	{0x06, 0x01, 0x0F, 0x27, 0x07, 0x6C, 0xB8},	// Based 24MHz,	9MHz	   External Clock18
	{0x05, 0x01, 0x0B, 0x23, 0x06, 0x60, 0xB0}	// Based 24MHz,	9.6MHz	   External Clock19
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x53}, {0x39, 0x5C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x3D}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x3D}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x3D}, {0x39, 0x3C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 24576)		
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
  	{0x08, 0x01, 0x13, 0x25, 0x06, 0x7D, 0xB4},	// Based 24.576MHz,	8MHz	   External Clock21
	{0x06, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8},	// Based 24.576MHz,	8.192MHz   External Clock20
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 24.576MHz, 9MHz
	{0x0C, 0x01, 0x1F, 0x27, 0x06, 0xE1, 0xB8}  // Based 24.576MHz,	9.6MHz	   External Clock22
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x51}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x3B}, {0x39, 0x6C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x3B}, {0x39, 0x6C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x3B}, {0x39, 0x6C}};
 #endif

  
#elif (RTV_SRC_CLK_FREQ_KHz == 26000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x0D, 0x01, 0x1F, 0x27, 0x06, 0xC0, 0xB8}, // Based 26MHz,	8MHz       External Clock23
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 26MHz, 8.192MHz
	{0x0D, 0x01, 0x1F, 0x27, 0x06, 0xD8, 0xB8}  // Based 26MHz,	9MHz	   External Clock24
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 26MHz, 9.6MHz
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x4C}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x38}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x38}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x38}, {0x39, 0x3C}};
 #endif
 
    
#elif (RTV_SRC_CLK_FREQ_KHz == 27000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x09, 0x01, 0x13, 0x25, 0x06, 0x80, 0xB4}, // Based 27MHz,	8MHz	   External Clock25
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 27MHz, 8.192MHz
	{0x06, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8}  // Based 27MHz,	9MHz	   External Clock26
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 27MHz, 9.6MHz
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x4A}, {0x39, 0x4C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x36}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x36}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x36}, {0x39, 0x2C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 32000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x08, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8}, // Based 32MHz,	8MHz	   External Clock27
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 32MHz, 8.192MHz
	{0x08, 0x01, 0x0F, 0x27, 0x07, 0x6C, 0xB8}, // Based 32MHz,	9MHz	   External Clock28
	{0x0A, 0x01, 0x1F, 0x27, 0x07, 0x90, 0xB8}  // Based 32MHz,	9.6MHz	   External Clock29
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x3E}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x2D}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x2D}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x2D}, {0x39, 0x2C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 32768)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x20, 0x01, 0x1F, 0x27, 0x07, 0x77, 0xB9}, // Based 32.768MHz,	8MHz	   External Clock31
	{0x08, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8}, // Based 32.768MHz,	8.192MHz   External Clock30
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 32.768MHz, 9MHz
	{0x10, 0x01, 0x1F, 0x27, 0x06, 0xE1, 0xB8}  // Based 32.768MHz,	9.6MHz	   External Clock32
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x3D}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x2C}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x2C}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x2C}, {0x39, 0x2C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 36000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x09, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8}, // Based 36MHz, 8MHz	   External Clock33
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, /* Unsupport */
	{0x09, 0x01, 0x0F, 0x27, 0x07, 0x6C, 0xB8}, // Based 36MHz, 9MHz	   External Clock34
	{0x0A, 0x01, 0x13, 0x25, 0x06, 0x80, 0xB4}  // Based 36MHz, 9.6MHz	   External Clock35
  };
  
 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x37}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x28}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x28}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x28}, {0x39, 0x2C}};
 #endif

  
#elif (RTV_SRC_CLK_FREQ_KHz == 38400)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x08, 0x01, 0x0B, 0x23, 0x06, 0x50, 0xB0}, // Based 38.4MHz, 8MHz	   External Clock36
	{0x19, 0x01, 0x1F, 0x27, 0x06, 0x00, 0xB9}, // Based 38.4MHz, 8.192MHz   External Clock37
	{0x08, 0x01, 0x0B, 0x23, 0x06, 0x5A, 0xB0}, // Based 38.4MHz, 9MHz	   External Clock38
	{0x0A, 0x01, 0x0F, 0x27, 0x07, 0x78, 0xB8}  // Based 38.4MHz, 9.6MHz	   External Clock39
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x34}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x26}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x26}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x26}, {0x39, 0x2C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 40000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x0A, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8}, // Based 40MHz,	8MHz	   External Clock40
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 40MHz, 8.192MHz
	{0x0A, 0x01, 0x0F, 0x27, 0x07, 0x6C, 0xB8}, // Based 40MHz,	9MHz	   External Clock41
	{0x19, 0x01, 0x1F, 0x27, 0x06, 0x20, 0xB9}  // Based 40MHz,	9.6MHz	   External Clock42
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x32}, {0x39, 0x3C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x24}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x24}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x24}, {0x39, 0x2C}};
 #endif


#elif (RTV_SRC_CLK_FREQ_KHz == 48000)
  static const U8 g_abAdcClkSynTbl[MAX_NUM_RTV_ADC_CLK_FREQ_TYPE][7] =
  {
	{0x0C, 0x01, 0x0F, 0x27, 0x07, 0x60, 0xB8}, // Based 48MHz,	8MHz       External Clock43
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Based 48MHz, 8.192MHz
	{0x0C, 0x01, 0x0F, 0x27, 0x07, 0x6C, 0xB8}, // Based 48MHz,	9MHz       External Clock44
	{0x0A, 0x01, 0x0B, 0x23, 0x06, 0x60, 0xB0}  // Based 48MHz,	9.6MHz     External Clock45
  };

 #ifdef RTV_ISDBT_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_ISDBT[] = {{0x37, 0x29}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_TDMB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_TDMB[] = {{0x37, 0x1E}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_DAB_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_DAB[] = {{0x37, 0x1E}, {0x39, 0x2C}};
 #endif

 #ifdef RTV_FM_ENABLE
  static const RTV_REG_INIT_INFO g_atAutoLnaInitData_FM[] = {{0x37, 0x1E}, {0x39, 0x2C}};
 #endif

#else
	#error "Unsupport external clock freqency!"
#endif




