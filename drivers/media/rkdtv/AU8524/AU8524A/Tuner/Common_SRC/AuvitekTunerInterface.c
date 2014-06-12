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

****************************************************************************************
**  Name:		AuvitekTunerInterface.c
**
**  Project:            Au85xx IIC Controller
**
**  Description:				    
**                    
**  Functions
**  Implemented:   
**						INT32	AU_TunerOpen
**						TUNER_NTSTATUS	AU_TunerClose
**						TUNER_NTSTATUS	AU_SetTunerParameter
**						TUNER_NTSTATUS	AU_SetTunerFrequency
**						PAU_TunerCapabilities	AU_GetTunerCapabilities
**						
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History: 
**  -------------------------------------------------------------------------
*      SCR      Date         Author     Description

*****************************************************************************/
#include "../Common/AuvitekTunerInterface.h"
#include "../Auvitek_Maxlinear_MXL5007/Auvitek_Maxlinear_MXL5007.h"

AU_TunerInterFaceST m_AU_TunerInterFaceST[TUNER_MAX_UNITS];

/*****************************************************************************
**
**  Name: AU_TunerOpen
**
**  Description:    open the tuner.
**
**  Parameters:   
**                  nAPHandle	- the tuner handle
**                  nHWAddress	- the IIC address of the tuner
**
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
INT32	AU_TunerOpen(void* nAPHandle,UINT32 nHWAddress,INT32 nTunerType)
{
    	
	int   nPos = 0;
	int pos_found = 0;
	INT32 TunerUnit = -1;
	static int initflag = 0;
	
	if(initflag == 0)
	{
		for(nPos=0;nPos<TUNER_MAX_UNITS;nPos++)
		{
			m_AU_TunerInterFaceST[nPos].AU_TunerOpen_Interface				= 0;
			m_AU_TunerInterFaceST[nPos].AU_TunerClose_Interface				= 0;
			m_AU_TunerInterFaceST[nPos].AU_SetTunerParameter_Interface		= 0;
			m_AU_TunerInterFaceST[nPos].AU_SetTunerFrequency_Interface		= 0;
			m_AU_TunerInterFaceST[nPos].AU_GetTunerCapabilities_Interface	= 0;
			m_AU_TunerInterFaceST[nPos].APHandle							= 0;
			m_AU_TunerInterFaceST[nPos].HWAddress							= 0;
			m_AU_TunerInterFaceST[nPos].TunerHandle							= -1;
			m_AU_TunerInterFaceST[nPos].TunerType							= -1;
		}

		initflag = 1;
	}


	if(nAPHandle == 0 || nHWAddress == 0)
	{
		return -1;
	}

	pos_found = 0;
	for(nPos=0;nPos<TUNER_MAX_UNITS;nPos++)
	{
		if(		m_AU_TunerInterFaceST[nPos].APHandle  == nAPHandle 
			&&	m_AU_TunerInterFaceST[nPos].HWAddress == nHWAddress
			&&	m_AU_TunerInterFaceST[nPos].TunerType == nTunerType)
		{
			TunerUnit = nPos;
			pos_found = 1;
			break;
		}
	}
	if (!pos_found)
	{
		for(nPos=0;nPos<TUNER_MAX_UNITS;nPos++)
		{
			if(		m_AU_TunerInterFaceST[nPos].APHandle	== 0 
				&&	m_AU_TunerInterFaceST[nPos].HWAddress	== 0 
				&&	m_AU_TunerInterFaceST[nPos].TunerHandle == -1
				&&	m_AU_TunerInterFaceST[nPos].TunerType	== -1)
			{
				TunerUnit = nPos;
				
				break;
			}
		}
	}

	if(TunerUnit == -1)
	{
		return -1;
	}

	m_AU_TunerInterFaceST[TunerUnit].AU_TunerOpen_Interface				= AU_TunerOpen_MXL5007;
	m_AU_TunerInterFaceST[TunerUnit].AU_TunerClose_Interface			= AU_TunerClose_MXL5007;
	m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerParameter_Interface		= AU_SetTunerParameter_MXL5007;
	m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerFrequency_Interface		= AU_SetTunerFrequency_MXL5007;
	m_AU_TunerInterFaceST[TunerUnit].AU_GetTunerCapabilities_Interface	= AU_GetTunerCapabilities_MXL5007;

	m_AU_TunerInterFaceST[TunerUnit].APHandle  = nAPHandle;
	m_AU_TunerInterFaceST[TunerUnit].HWAddress = nHWAddress;
	m_AU_TunerInterFaceST[TunerUnit].TunerType = nTunerType;


	if(m_AU_TunerInterFaceST[TunerUnit].AU_TunerOpen_Interface)
	{
		INT32 ResultHandle = m_AU_TunerInterFaceST[TunerUnit].AU_TunerOpen_Interface(nAPHandle,nHWAddress);

		if(ResultHandle != -1)
		{
			m_AU_TunerInterFaceST[TunerUnit].TunerHandle = ResultHandle;
		}
		else
		{
			m_AU_TunerInterFaceST[TunerUnit].AU_TunerOpen_Interface				= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_TunerClose_Interface			= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerParameter_Interface		= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerFrequency_Interface		= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_GetTunerCapabilities_Interface	= 0;
			m_AU_TunerInterFaceST[TunerUnit].APHandle							= 0;
			m_AU_TunerInterFaceST[TunerUnit].HWAddress							= 0;
			m_AU_TunerInterFaceST[TunerUnit].TunerHandle						= -1;
			m_AU_TunerInterFaceST[TunerUnit].TunerType							= -1;

			TunerUnit = -1;
		}

		
	}
	return TunerUnit;
}

/*****************************************************************************
**
**  Name: AU_TunerClose
**
**  Description:    close the tuner.
**
**  Parameters:   
**                  TunerUnit	- the tuner number
**
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS	AU_TunerClose(UINT32 TunerUnit)
{
	TUNER_NTSTATUS ntStatus=TUNER_STATUS_SUCCESS;

	if(m_AU_TunerInterFaceST[TunerUnit].AU_TunerClose_Interface)
	{
		ntStatus = m_AU_TunerInterFaceST[TunerUnit].AU_TunerClose_Interface(m_AU_TunerInterFaceST[TunerUnit].TunerHandle);

		if(ntStatus == TUNER_STATUS_SUCCESS)
		{
			m_AU_TunerInterFaceST[TunerUnit].AU_TunerOpen_Interface				= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_TunerClose_Interface			= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerParameter_Interface		= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerFrequency_Interface		= 0;
			m_AU_TunerInterFaceST[TunerUnit].AU_GetTunerCapabilities_Interface	= 0;
			m_AU_TunerInterFaceST[TunerUnit].APHandle							= 0;
			m_AU_TunerInterFaceST[TunerUnit].HWAddress							= 0;
			m_AU_TunerInterFaceST[TunerUnit].TunerHandle						= -1;
			m_AU_TunerInterFaceST[TunerUnit].TunerType							= -1;

			return ntStatus;
		}
	}

	return TUNER_STATUS_UNSUCCESSFUL;
}

/*****************************************************************************
**
**  Name: AU_SetTunerParameter
**
**  Description:    set parameter to the tuner.
**
**  Parameters:   
**                  TunerUnit		- the tuner number
**		    PAU_TunerParameter	- the parameter
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerParameter(UINT32 TunerUnit,PAU_TunerParameter nTunerParameter)
{
	if(m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerParameter_Interface)
	{
		return m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerParameter_Interface(m_AU_TunerInterFaceST[TunerUnit].TunerHandle,nTunerParameter);
	}
	return TUNER_STATUS_UNSUCCESSFUL;
}

/*****************************************************************************
**
**  Name: AU_SetTunerFrequency
**
**  Description:    set frequency to the tuner.
**
**  Parameters:   
**                  TunerUnit	- the tuner number
**		    nFrequency	- the frequency
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
TUNER_NTSTATUS	AU_SetTunerFrequency(UINT32 TunerUnit,UINT32 nFrequency)
{
	if(m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerFrequency_Interface)
	{
		return m_AU_TunerInterFaceST[TunerUnit].AU_SetTunerFrequency_Interface(m_AU_TunerInterFaceST[TunerUnit].TunerHandle,nFrequency);
	}
	

	return TUNER_STATUS_UNSUCCESSFUL;
}

/*****************************************************************************
**
**  Name: AU_GetTunerCapabilities
**
**  Description:    get capabilities to the tuner.
**
**  Parameters:   
**                  TunerUnit		- the tuner number
**
**  Returns:        status:
**                  user-defined
**                  TUNER_STATUS_UNSUCCESSFUL  - errors
**
**  Notes:          
**
**  Revision History:
**
*****************************************************************************/
PAU_TunerCapabilities	AU_GetTunerCapabilities(UINT32 TunerUnit)
{
	if(m_AU_TunerInterFaceST[TunerUnit].AU_GetTunerCapabilities_Interface)
	{
		return m_AU_TunerInterFaceST[TunerUnit].AU_GetTunerCapabilities_Interface(m_AU_TunerInterFaceST[TunerUnit].TunerHandle);
	}
	
	return 0;
}
