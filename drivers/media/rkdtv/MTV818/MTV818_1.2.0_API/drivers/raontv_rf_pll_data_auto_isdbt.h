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
* TITLE 	  : RAONTECH TV RF PLL data for ISDB-T in automatically header file. 
*
* FILENAME    : raontv_rf_pll_data_auto_isdbt.h
*
* DESCRIPTION : 
*		All the declarations and definitions necessary for the RAONTECH TV driver.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 09/30/2010  Ko, Kevin        Splited the file from raontv_rf_pll_data_auto.h
* 09/29/2010  Ko, Kevin        Changed the value of PLL delta for ISDB-T.
* 09/27/2010  Ko, Kevin        Creat for CS Realease
*             /Yang, Maverick  1.Reformating for CS API
*                              2.pll table, ADC clock switching, SCAN function, 
*								 FM function added..
********************************************************************************/


#if (RTV_SRC_CLK_FREQ_KHz == 13000)
	#define ISDBT_AUTO_PLLNF			0x796DB9B
	#define ISDBT_AUTO_PLL_NFSTEP		0x1D89D8
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x00<<0}, {0x31, 0x02, 0x00<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 16000)
	#define ISDBT_AUTO_PLLNF			0x62A926E
	#define ISDBT_AUTO_PLL_NFSTEP		0x180000
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x01<<0}, {0x31, 0x02, 0x00<<1}};
	
#elif (RTV_SRC_CLK_FREQ_KHz == 16384)
	#define ISDBT_AUTO_PLLNF			0x6059300 // 1seg
	#define ISDBT_AUTO_PLL_NFSTEP		0x177000
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x01<<4}, {0x27, 0x03, 0x00<<0}, {0x31, 0x02, 0x00<<1}};

#elif (RTV_SRC_CLK_FREQ_KHz == 18000)	
	#define ISDBT_AUTO_PLLNF			0x57B2CD4
	#define ISDBT_AUTO_PLL_NFSTEP		0x155555
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x02<<0}, {0x31, 0x02, 0x00<<1}};
    
#elif (RTV_SRC_CLK_FREQ_KHz == 19200)
	#define ISDBT_AUTO_PLLNF			0x5237A06
	#define ISDBT_AUTO_PLL_NFSTEP		0x140000
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x01<<4}, {0x27, 0x03, 0x00<<0}, {0x31, 0x02, 0x00<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 24000)	
	#define ISDBT_AUTO_PLLNF			0x41C619F
	#define ISDBT_AUTO_PLL_NFSTEP		0x100000
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x01<<4}, {0x27, 0x03, 0x03<<0}, {0x31, 0x02, 0x00<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 24576)		
	#define ISDBT_AUTO_PLLNF			0x403B755
	#define ISDBT_AUTO_PLL_NFSTEP		0x0FA000	
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x01<<4}, {0x27, 0x03, 0x03<<0}, {0x31, 0x02, 0x01<<1}};
    
#elif (RTV_SRC_CLK_FREQ_KHz == 26000)
	#define ISDBT_AUTO_PLLNF			0x3CB6DCD
	#define ISDBT_AUTO_PLL_NFSTEP		0x0EC4EC	

	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x00<<0}, {0x31, 0x02, 0x01<<1}};
      
#elif (RTV_SRC_CLK_FREQ_KHz == 27000)
	#define ISDBT_AUTO_PLLNF			0x3A77338
	#define ISDBT_AUTO_PLL_NFSTEP		0x0E38E3
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x00<<0}, {0x31, 0x02, 0x01<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 32000)
	#define ISDBT_AUTO_PLLNF			0x3154937
	#define ISDBT_AUTO_PLL_NFSTEP		0x0C0000
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x01<<0}, {0x31, 0x02, 0x01<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 32768)
	#define ISDBT_AUTO_PLLNF			0x302C980
	#define ISDBT_AUTO_PLL_NFSTEP		0x0BB800
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x01<<4}, {0x27, 0x03, 0x00<<0}, {0x31, 0x02, 0x01<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 36000)
	#define ISDBT_AUTO_PLLNF			0x2BD966A
	#define ISDBT_AUTO_PLL_NFSTEP		0x0AAAAA
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x02<<0}, {0x31, 0x02, 0x01<<1}};
    
#elif (RTV_SRC_CLK_FREQ_KHz == 38400)
	#define ISDBT_AUTO_PLLNF			0x2B5E0B7
	#define ISDBT_AUTO_PLL_NFSTEP		0x0A0000
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x01<<4}, {0x27, 0x03, 0x00<<0}, {0x31, 0x02, 0x01<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 40000)
	#define ISDBT_AUTO_PLLNF			0x2776DC5
	#define ISDBT_AUTO_PLL_NFSTEP		0x099999	
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x01<<4}, {0x27, 0x03, 0x03<<0}, {0x31, 0x02, 0x01<<1}};
  
#elif (RTV_SRC_CLK_FREQ_KHz == 48000)
	#define ISDBT_AUTO_PLLNF			0x20E30CF
	#define ISDBT_AUTO_PLL_NFSTEP		0x080000
	
	static const RTV_REG_MASK_INFO g_atAutoChInitData_ISDBT[] = {{0x46, 0x10, 0x00<<4}, {0x27, 0x03, 0x03<<0}, {0x31, 0x02, 0x01<<1}};
  
#else
	#error "Unsupport external clock freqency!"
#endif

 