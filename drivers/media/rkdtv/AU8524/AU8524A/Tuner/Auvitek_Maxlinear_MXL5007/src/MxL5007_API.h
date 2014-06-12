/*
 
 Driver APIs for MxL5007 Tuner
 
 Copyright, Maxlinear, Inc.
 All Rights Reserved
 
 File Name:      MxL5007_API.h
 
 */
#ifndef __MxL5007_API_H
#define __MxL5007_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "MxL5007_Common.h"

/******************************************************************************
**
**  Name: MxL_Set_Register
**
**  Description:    Write one register to MxL5007
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**					RegAddr				- Register address to be written
**					RegData				- Data to be written
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_SET_REG if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Set_Register(void * DevHandle,MxL5007_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 RegData);

/******************************************************************************
**
**  Name: MxL_Get_Register
**
**  Description:    Read one register from MxL5007
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**					RegAddr				- Register address to be read
**					RegData				- Pointer to register read
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_GET_REG if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Get_Register(void * DevHandle,MxL5007_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 *RegData);

/******************************************************************************
**
**  Name: MxL_Tuner_Init
**
**  Description:    MxL5007 Initialization
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_INIT if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Tuner_Init(void * DevHandle,MxL5007_TunerConfigS* );

/******************************************************************************
**
**  Name: MxL_Tuner_RFTune
**
**  Description:    Frequency tunning for channel
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**					RF_Freq_Hz			- RF Frequency in Hz
**					BWMHz				- Bandwidth 6, 7 or 8 MHz
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_RFTUNE if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Tuner_RFTune(void * DevHandle,MxL5007_TunerConfigS*, UINT32 RF_Freq_Hz, MxL5007_BW_MHz BWMHz);		

/******************************************************************************
**
**  Name: MxL_Soft_Reset
**
**  Description:    Software Reset the MxL5007 Tuner
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Soft_Reset(void * DevHandle,MxL5007_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_Loop_Through_On
**
**  Description:    Turn On/Off on-chip Loop-through
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**					isOn				- True to turn On Loop Through
**										- False to turn off Loop Through
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Loop_Through_On(void * DevHandle,MxL5007_TunerConfigS*, MxL5007_LoopThru);

/******************************************************************************
**
**  Name: MxL_Standby
**
**  Description:    Enter Standby Mode
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Stand_By(void * DevHandle,MxL5007_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_Wakeup
**
**  Description:    Wakeup from Standby Mode (Note: after wake up, please call RF_Tune again)
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Wake_Up(void * DevHandle,MxL5007_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_Check_ChipVersion
**
**  Description:    Return the MxL5007 Chip ID
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**			
**  Returns:        MxL_ChipVersion			
**
******************************************************************************/
MxL5007_ChipVersion MxL_Check_ChipVersion(void * DevHandle,MxL5007_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_RFSynth_Lock_Status
**
**  Description:    RF synthesizer lock status of MxL5007
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**					isLock				- Pointer to Lock Status
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_RFSynth_Lock_Status(void * DevHandle,MxL5007_TunerConfigS* , BOOL* isLock);

/******************************************************************************
**
**  Name: MxL_REFSynth_Lock_Status
**
**  Description:    REF synthesizer lock status of MxL5007
**
**  Parameters:    	myTuner				- Pointer to MxL5007_TunerConfigS
**					isLock				- Pointer to Lock Status
**
**  Returns:        MxL_ERR_MSG			- MxL_OK if success	
**										- MxL_ERR_OTHERS if fail	
**
******************************************************************************/
MxL_ERR_MSG MxL_REFSynth_Lock_Status(void * DevHandle,MxL5007_TunerConfigS* , BOOL* isLock);

#ifdef __cplusplus
}
#endif

#endif //__MxL5007_API_H