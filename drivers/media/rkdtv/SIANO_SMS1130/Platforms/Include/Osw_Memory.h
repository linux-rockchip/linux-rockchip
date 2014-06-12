/*!  

	\file		Osw_Memory.h
	
	\brief		OS Wrapper - Memory header
	
				The following are functions for allocating and free memory space, 
				set and copy a memory space, etc.
																
	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  
																	 
*/

#ifndef __OSW_MEMORY_H
#define __OSW_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include_internal/Include/sms_common.h"

/*!
	Allocates dynamic memory space
	
	\param[in]	size		The size of the allocated memory space in bytes.

	\return		A void pointer to the allocated memory space or NULL if no space is available.
*/	
void*  OSW_MemAlloc( UINT32 size );

/*!
	Free the memory block
	
	\param[in]	pMemToFree		 A pointer to the allocated memory space to be freed.
*/	
void   OSW_MemFree( void* pMemToFree );


#ifdef __cplusplus
}
#endif

#endif
