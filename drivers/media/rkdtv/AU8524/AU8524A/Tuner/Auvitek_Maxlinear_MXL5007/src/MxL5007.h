/*
 
 Driver APIs for MxL5007 Tuner
 
 Copyright, Maxlinear, Inc.
 All Rights Reserved
 
 File Name:      MxL5007.h

 */
#ifndef __MxL5007_H
#define __MxL5007_H

#include "MxL5007_Common.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	UINT8 Num;	//Register number
	UINT8 Val;	//Register value
} IRVType, *PIRVType;


UINT32 MxL5007_Init(UINT8* pArray,				// a array pointer that store the addr and data pairs for I2C write
					UINT32* Array_Size,			// a integer pointer that store the number of element in above array
					UINT8 Mode,				
					SINT32 IF_Diff_Out_Level,
					UINT32 Xtal_Freq_Hz,		
					UINT32 IF_Freq_Hz,		
					UINT8 Invert_IF,			
					UINT8 Clk_Out_Enable,    
					UINT8 Clk_Out_Amp		
					);
UINT32 MxL5007_RFTune(UINT8* pArray, UINT32* Array_Size, 
					 UINT32 RF_Freq,			// RF Frequency in Hz
					 UINT8 BWMHz		// Bandwidth in MHz
					 );
UINT32 SetIRVBit(PIRVType pIRV, UINT8 Num, UINT8 Mask, UINT8 Val);

#ifdef __cplusplus
}
#endif

#endif
