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
* TITLE 	  : RAONTECH TV RF PLL data for TDMB in manually header file. 
*
* FILENAME    : raontv_rf_pll_data_auto_isdbt_tdmb.h
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
* 09/30/2010  Ko, Kevin        Created the file.
********************************************************************************/


#if (RTV_SRC_CLK_FREQ_KHz == 13000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 13000 KHz Ref Clk */
	0x6BDD576/*  7A: 175280 */, 0x6CED916/*  7B: 177008 */, 0x6DFDCB6/*  7C: 178736 */, 
	0x6F8E928/*  8A: 181280 */, 0x709ECC7/*  8B: 183008 */, 0x71AF067/*  8C: 184736 */, 
	0x733FCD9/*  9A: 187280 */, 0x7450078/*  9B: 189008 */, 0x7560418/*  9C: 190736 */, 
	0x76F108A/* 10A: 193280 */, 0x780142A/* 10B: 195008 */, 0x79117C9/* 10C: 196736 */, 
	0x7AA243B/* 11A: 199280 */, 0x7BB27DB/* 11B: 201008 */, 0x7CC2B7B/* 11C: 202736 */, 
	0x7E537ED/* 12A: 205280 */, 0x7F63B8C/* 12B: 207008 */, 0x8073F2C/* 12C: 208736 */, 
	0x8204B9E/* 13A: 211280 */, 0x8314F3D/* 13B: 213008 */, 0x84252DD/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 16000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 16000 KHz Ref Clk */
	0x57A3D70/*  7A: 175280 */, 0x5881062/*  7B: 177008 */, 0x595E353/*  7C: 178736 */, 
	0x5AA3D70/*  8A: 181280 */, 0x5B81062/*  8B: 183008 */, 0x5C5E353/*  8C: 184736 */, 
	0x5DA3D70/*  9A: 187280 */, 0x5E81062/*  9B: 189008 */, 0x5F5E353/*  9C: 190736 */, 
	0x60A3D70/* 10A: 193280 */, 0x6181062/* 10B: 195008 */, 0x625E353/* 10C: 196736 */, 
	0x63A3D70/* 11A: 199280 */, 0x6481062/* 11B: 201008 */, 0x655E353/* 11C: 202736 */, 
	0x66A3D70/* 12A: 205280 */, 0x6781062/* 12B: 207008 */, 0x685E353/* 12C: 208736 */, 
	0x69A3D70/* 13A: 211280 */, 0x6A81062/* 13B: 213008 */, 0x6B5E353/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 16384)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 16384 KHz Ref Clk */
	0x5596000/*  7A: 175280 */, 0x566E000/*  7B: 177008 */, 0x5746000/*  7C: 178736 */, 
	0x5884000/*  8A: 181280 */, 0x595C000/*  8B: 183008 */, 0x5A34000/*  8C: 184736 */, 
	0x5B72000/*  9A: 187280 */, 0x5C4A000/*  9B: 189008 */, 0x5D22000/*  9C: 190736 */, 
	0x5E60000/* 10A: 193280 */, 0x5F38000/* 10B: 195008 */, 0x6010000/* 10C: 196736 */, 
	0x614E000/* 11A: 199280 */, 0x6226000/* 11B: 201008 */, 0x62FE000/* 11C: 202736 */, 
	0x643C000/* 12A: 205280 */, 0x6514000/* 12B: 207008 */, 0x65EC000/* 12C: 208736 */, 
	0x672A000/* 13A: 211280 */, 0x6802000/* 13B: 213008 */, 0x68DA000/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 18000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 18000 KHz Ref Clk */
	0x4DE6F80/*  7A: 175280 */, 0x4EAB93A/*  7B: 177008 */, 0x4F702F5/*  7C: 178736 */, 
	0x5091A2B/*  8A: 181280 */, 0x51563E5/*  8B: 183008 */, 0x521AD9F/*  8C: 184736 */, 
	0x533C4D5/*  9A: 187280 */, 0x5400E90/*  9B: 189008 */, 0x54C584A/*  9C: 190736 */, 
	0x55E6F80/* 10A: 193280 */, 0x56AB93A/* 10B: 195008 */, 0x57702F5/* 10C: 196736 */, 
	0x5891A2B/* 11A: 199280 */, 0x59563E5/* 11B: 201008 */, 0x5A1AD9F/* 11C: 202736 */, 
	0x5B3C4D5/* 12A: 205280 */, 0x5C00E90/* 12B: 207008 */, 0x5CC584A/* 12C: 208736 */, 
	0x5DE6F80/* 13A: 211280 */, 0x5EAB93A/* 13B: 213008 */, 0x5F702F5/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 19200)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 19200 KHz Ref Clk */
	0x4908888/*  7A: 175280 */, 0x49C0DA7/*  7B: 177008 */, 0x4A792C5/*  7C: 178736 */, 
	0x4B88888/*  8A: 181280 */, 0x4C40DA7/*  8B: 183008 */, 0x4CF92C5/*  8C: 184736 */, 
	0x4E08888/*  9A: 187280 */, 0x4EC0DA7/*  9B: 189008 */, 0x4F792C5/*  9C: 190736 */, 
	0x5088888/* 10A: 193280 */, 0x5140DA7/* 10B: 195008 */, 0x51F92C5/* 10C: 196736 */, 
	0x5308888/* 11A: 199280 */, 0x53C0DA7/* 11B: 201008 */, 0x54792C5/* 11C: 202736 */, 
	0x5588888/* 12A: 205280 */, 0x5640DA7/* 12B: 207008 */, 0x56F92C5/* 12C: 208736 */, 
	0x5808888/* 13A: 211280 */, 0x58C0DA7/* 13B: 213008 */, 0x59792C5/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 24000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 24000 KHz Ref Clk */
	0x3A6D3A0/*  7A: 175280 */, 0x3B00AEC/*  7B: 177008 */, 0x3B94237/*  7C: 178736 */, 
	0x3C6D3A0/*  8A: 181280 */, 0x3D00AEC/*  8B: 183008 */, 0x3D94237/*  8C: 184736 */, 
	0x3E6D3A0/*  9A: 187280 */, 0x3F00AEC/*  9B: 189008 */, 0x3F94237/*  9C: 190736 */, 
	0x406D3A0/* 10A: 193280 */, 0x4100AEC/* 10B: 195008 */, 0x4194237/* 10C: 196736 */, 
	0x426D3A0/* 11A: 199280 */, 0x4300AEC/* 11B: 201008 */, 0x4394237/* 11C: 202736 */, 
	0x446D3A0/* 12A: 205280 */, 0x4500AEC/* 12B: 207008 */, 0x4594237/* 12C: 208736 */, 
	0x466D3A0/* 13A: 211280 */, 0x4700AEC/* 13B: 213008 */, 0x4794237/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 24576)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 24576 KHz Ref Clk */
	0x390EAAA/*  7A: 175280 */, 0x399EAAA/*  7B: 177008 */, 0x3A2EAAA/*  7C: 178736 */, 
	0x3B02AAA/*  8A: 181280 */, 0x3B92AAA/*  8B: 183008 */, 0x3C22AAA/*  8C: 184736 */, 
	0x3CF6AAA/*  9A: 187280 */, 0x3D86AAA/*  9B: 189008 */, 0x3E16AAA/*  9C: 190736 */, 
	0x3EEAAAA/* 10A: 193280 */, 0x3F7AAAA/* 10B: 195008 */, 0x400AAAA/* 10C: 196736 */, 
	0x40DEAAA/* 11A: 199280 */, 0x416EAAA/* 11B: 201008 */, 0x41FEAAA/* 11C: 202736 */, 
	0x42D2AAA/* 12A: 205280 */, 0x4362AAA/* 12B: 207008 */, 0x43F2AAA/* 12C: 208736 */, 
	0x44C6AAA/* 13A: 211280 */, 0x4556AAA/* 13B: 213008 */, 0x45E6AAA/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 26000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 26000 KHz Ref Clk */
	0x35EEABB/*  7A: 175280 */, 0x3676C8B/*  7B: 177008 */, 0x36FEE5B/*  7C: 178736 */, 
	0x37C7494/*  8A: 181280 */, 0x384F663/*  8B: 183008 */, 0x38D7833/*  8C: 184736 */, 
	0x399FE6C/*  9A: 187280 */, 0x3A2803C/*  9B: 189008 */, 0x3AB020C/*  9C: 190736 */, 
	0x3B78845/* 10A: 193280 */, 0x3C00A15/* 10B: 195008 */, 0x3C88BE4/* 10C: 196736 */, 
	0x3D5121D/* 11A: 199280 */, 0x3DD93ED/* 11B: 201008 */, 0x3E615BD/* 11C: 202736 */, 
	0x3F29BF6/* 12A: 205280 */, 0x3FB1DC6/* 12B: 207008 */, 0x4039F96/* 12C: 208736 */, 
	0x41025CF/* 13A: 211280 */, 0x418A79E/* 13B: 213008 */, 0x421296E/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 27000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 27000 KHz Ref Clk */
	0x33EF500/*  7A: 175280 */, 0x3472627/*  7B: 177008 */, 0x34F574E/*  7C: 178736 */, 
	0x35B66C7/*  8A: 181280 */, 0x36397EE/*  8B: 183008 */, 0x36BC915/*  8C: 184736 */, 
	0x377D88E/*  9A: 187280 */, 0x38009B5/*  9B: 189008 */, 0x3883ADC/*  9C: 190736 */, 
	0x3944A55/* 10A: 193280 */, 0x39C7B7C/* 10B: 195008 */, 0x3A4ACA3/* 10C: 196736 */, 
	0x3B0BC1C/* 11A: 199280 */, 0x3B8ED43/* 11B: 201008 */, 0x3C11E6A/* 11C: 202736 */, 
	0x3CD2DE3/* 12A: 205280 */, 0x3D55F0A/* 12B: 207008 */, 0x3DD9031/* 12C: 208736 */, 
	0x3E99FAB/* 13A: 211280 */, 0x3F1D0D1/* 13B: 213008 */, 0x3FA01F8/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 32000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 32000 KHz Ref Clk */
	0x2BD1EB8/*  7A: 175280 */, 0x2C40831/*  7B: 177008 */, 0x2CAF1A9/*  7C: 178736 */, 
	0x2D51EB8/*  8A: 181280 */, 0x2DC0831/*  8B: 183008 */, 0x2E2F1A9/*  8C: 184736 */, 
	0x2ED1EB8/*  9A: 187280 */, 0x2F40831/*  9B: 189008 */, 0x2FAF1A9/*  9C: 190736 */, 
	0x3051EB8/* 10A: 193280 */, 0x30C0831/* 10B: 195008 */, 0x312F1A9/* 10C: 196736 */, 
	0x31D1EB8/* 11A: 199280 */, 0x3240831/* 11B: 201008 */, 0x32AF1A9/* 11C: 202736 */, 
	0x3351EB8/* 12A: 205280 */, 0x33C0831/* 12B: 207008 */, 0x342F1A9/* 12C: 208736 */, 
	0x34D1EB8/* 13A: 211280 */, 0x3540831/* 13B: 213008 */, 0x35AF1A9/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 32768)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 32768 KHz Ref Clk */
	0x2ACB000/*  7A: 175280 */, 0x2B37000/*  7B: 177008 */, 0x2BA3000/*  7C: 178736 */, 
	0x2C42000/*  8A: 181280 */, 0x2CAE000/*  8B: 183008 */, 0x2D1A000/*  8C: 184736 */, 
	0x2DB9000/*  9A: 187280 */, 0x2E25000/*  9B: 189008 */, 0x2E91000/*  9C: 190736 */, 
	0x2F30000/* 10A: 193280 */, 0x2F9C000/* 10B: 195008 */, 0x3008000/* 10C: 196736 */, 
	0x30A7000/* 11A: 199280 */, 0x3113000/* 11B: 201008 */, 0x317F000/* 11C: 202736 */, 
	0x321E000/* 12A: 205280 */, 0x328A000/* 12B: 207008 */, 0x32F6000/* 12C: 208736 */, 
	0x3395000/* 13A: 211280 */, 0x3401000/* 13B: 213008 */, 0x346D000/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 36000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 36000 KHz Ref Clk */
	0x26F37C0/*  7A: 175280 */, 0x2755C9D/*  7B: 177008 */, 0x27B817A/*  7C: 178736 */, 
	0x2848D15/*  8A: 181280 */, 0x28AB1F2/*  8B: 183008 */, 0x290D6CF/*  8C: 184736 */, 
	0x299E26A/*  9A: 187280 */, 0x2A00748/*  9B: 189008 */, 0x2A62C25/*  9C: 190736 */, 
	0x2AF37C0/* 10A: 193280 */, 0x2B55C9D/* 10B: 195008 */, 0x2BB817A/* 10C: 196736 */, 
	0x2C48D15/* 11A: 199280 */, 0x2CAB1F2/* 11B: 201008 */, 0x2D0D6CF/* 11C: 202736 */, 
	0x2D9E26A/* 12A: 205280 */, 0x2E00748/* 12B: 207008 */, 0x2E62C25/* 12C: 208736 */, 
	0x2EF37C0/* 13A: 211280 */, 0x2F55C9D/* 13B: 213008 */, 0x2FB817A/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 38400)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 38400 KHz Ref Clk */
	0x2484444/*  7A: 175280 */, 0x24E06D3/*  7B: 177008 */, 0x253C962/*  7C: 178736 */, 
	0x25C4444/*  8A: 181280 */, 0x26206D3/*  8B: 183008 */, 0x267C962/*  8C: 184736 */, 
	0x2704444/*  9A: 187280 */, 0x27606D3/*  9B: 189008 */, 0x27BC962/*  9C: 190736 */, 
	0x2844444/* 10A: 193280 */, 0x28A06D3/* 10B: 195008 */, 0x28FC962/* 10C: 196736 */, 
	0x2984444/* 11A: 199280 */, 0x29E06D3/* 11B: 201008 */, 0x2A3C962/* 11C: 202736 */, 
	0x2AC4444/* 12A: 205280 */, 0x2B206D3/* 12B: 207008 */, 0x2B7C962/* 12C: 208736 */, 
	0x2C04444/* 13A: 211280 */, 0x2C606D3/* 13B: 213008 */, 0x2CBC962/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 40000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 40000 KHz Ref Clk */
	0x230E560/*  7A: 175280 */, 0x2366CF4/*  7B: 177008 */, 0x23BF487/*  7C: 178736 */, 
	0x2441893/*  8A: 181280 */, 0x249A027/*  8B: 183008 */, 0x24F27BB/*  8C: 184736 */, 
	0x2574BC6/*  9A: 187280 */, 0x25CD35A/*  9B: 189008 */, 0x2625AEE/*  9C: 190736 */, 
	0x26A7EF9/* 10A: 193280 */, 0x270068D/* 10B: 195008 */, 0x2758E21/* 10C: 196736 */, 
	0x27DB22D/* 11A: 199280 */, 0x28339C0/* 11B: 201008 */, 0x288C154/* 11C: 202736 */, 
	0x290E560/* 12A: 205280 */, 0x2966CF4/* 12B: 207008 */, 0x29BF487/* 12C: 208736 */, 
	0x2A41893/* 13A: 211280 */, 0x2A9A027/* 13B: 213008 */, 0x2AF27BB/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#elif (RTV_SRC_CLK_FREQ_KHz == 48000)

 #ifdef RTV_TDMB_ENABLE
  static const U32 g_atPllNF_TDMB[] = 
  {
   #if defined(RTV_CONUTRY_KOREA) /* 48000 KHz Ref Clk */
	0x1D369D0/*  7A: 175280 */, 0x1D80576/*  7B: 177008 */, 0x1DCA11B/*  7C: 178736 */, 
	0x1E369D0/*  8A: 181280 */, 0x1E80576/*  8B: 183008 */, 0x1ECA11B/*  8C: 184736 */, 
	0x1F369D0/*  9A: 187280 */, 0x1F80576/*  9B: 189008 */, 0x1FCA11B/*  9C: 190736 */, 
	0x20369D0/* 10A: 193280 */, 0x2080576/* 10B: 195008 */, 0x20CA11B/* 10C: 196736 */, 
	0x21369D0/* 11A: 199280 */, 0x2180576/* 11B: 201008 */, 0x21CA11B/* 11C: 202736 */, 
	0x22369D0/* 12A: 205280 */, 0x2280576/* 12B: 207008 */, 0x22CA11B/* 12C: 208736 */, 
	0x23369D0/* 13A: 211280 */, 0x2380576/* 13B: 213008 */, 0x23CA11B/* 13C: 214736 */
	
   #else
		#error "Code not present"
   #endif
  };
 #endif

#else
	#error "Code not present"
#endif

