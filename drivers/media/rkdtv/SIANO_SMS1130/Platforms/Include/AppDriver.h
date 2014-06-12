/*************************************************************************/
/*                                                                       */
/* Copyright (C) 2005,2006 Siano Mobile Silicon Ltd. All rights reserved */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                                             */
/*                                                                       */
/*      AppDriver.c                                                      */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Application driver unified interface   	                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*************************************************************************/

#ifndef _APP_DRIVER_INT_H
#define _APP_DRIVER_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sms_common.h"

#include "SmsLiteAppDriver.h"

#define	ADR_CTRL_HANDLE		0


///////////////////////////////////////////////////////////////////////////////
/*! 
	Plug n Play events notification callback
	This function is registered by the host lib in #ADR_Init
	It is called when a PNP event occurs - when a new device 
	appears or when a device disconnects

\param[in]	DeviceHandle	ADR Device handle of the device that connected or disconnected
\param[in]	DeviceName	Name of the device
\param[in]	IsDeviceConnected	Indicated whether this event is a connect event or a disconnect event
								if IsDeviceConnected is TRUE, then it is an event for a new device
\param[in]	CurrentMode		The current mode of the device - #SMSHOSTLIB_ERR_CODES_E when notifying
							about a new device. In case of disconnect (IsDeviceConnected==FALSE) the
							mode will be set to #SMSHOSTLIB_DEVMD_NONE
*/
typedef void ( *ADR_pfPnPCbFunc )(	void*					DeviceHandle,
									char*					DeviceName, 
									BOOL					IsDeviceConnected,  
									SMSHOSTLIB_DEVICE_MODES_E	CurrentMode );


///////////////////////////////////////////////////////////////////////////////
/*! 
	Stream Driver Initialization. 

	\param[in]	e_comm_type	Communication type
	\param[in]	p_params	Driver dependent extra parameters
	\param[in]	pfPnPCbFunc	Callback for client plug and play event

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E ADR_Init(SMSHOSTLIB_COMM_TYPES_E e_comm_type,
								void*					p_params,
								ADR_pfPnPCbFunc			pfPnPCbFunc );

///////////////////////////////////////////////////////////////////////////////
/*! 
	Stream Driver Termination. 

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E ADR_Terminate( void );


///////////////////////////////////////////////////////////////////////////////
/*! 
	Init existing device .
	
	\param[in]	DeviceHandle	Handle to the ADR device.
	\param[in]	DeviceMode		Work mode, according to #SMSHOSTLIB_DEVICE_MODES_E enumerator

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.

	\remark		After that function returns, the device should be ready to operate in the 
				requested DeviceMode.
*/
SMSHOSTLIB_ERR_CODES_E ADR_DeviceInit(	void*						DeviceHandle,
										SMSHOSTLIB_DEVICE_MODES_E	DeviceMode );

///////////////////////////////////////////////////////////////////////////////
/*! 
	Terminate existing device .
	
	\param[in]	DeviceHandle	Handle to the ADR device.

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.

	\remark		For each device used #ADR_DeviceInit the host lib needs to call
				this function for termination.
	\remark		After that function returns, the device is in unknown state.
*/
SMSHOSTLIB_ERR_CODES_E ADR_DeviceTerminate( void* DeviceHandle );



///////////////////////////////////////////////////////////////////////////////
/*! 
	Stream Driver Close.

	\param[in]	DeviceHandle	Handle to the ADR device.
	\param[in]	handle_num		Handle number generated by from control library. 0 for control library.

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code
*/
SMSHOSTLIB_ERR_CODES_E ADR_CloseHandle(	void*	DeviceHandle,
										UINT32	handle_num );

///////////////////////////////////////////////////////////////////////////////
/*! 
	Write message (little-endian ordering).
	
	\param[in]	DeviceHandle	Handle return from call to #ADR_OpenHandle.
	\param[in]	p_msg			Pointer to a message to write

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E ADR_WriteMsg(void*			DeviceHandle,
									SmsMsgData_ST*	p_msg );

///////////////////////////////////////////////////////////////////////////////
/*! 
	Read buffer (little-endian ordering)
	
	\param[in]	DeviceHandle		Handle to the ADR device.
	\param[in]	handle_num			Handle number generated from control library. 0 for control library.
	\param[out]	p_buffer			Pointer to the buffer to be read
	\param[in]	n_max_size			Maximum number of bytes to be read
	\param[out]	p_buff_size_read	Actual number of read bytes 

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code
*/
SMSHOSTLIB_ERR_CODES_E ADR_ReadBuf(	void*		DeviceHandle,
									UINT32		handle_num,
									void*		p_buffer,
									UINT32		n_max_size,
									UINT32*		p_buff_size_read );

///////////////////////////////////////////////////////////////////////////////
/*! 
	Set device power mode
	
	\param[in]	DeviceHandle		Handle to the ADR device.
	\param[in]	adrPowerMode		Power mode

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code
*/
SMSHOSTLIB_ERR_CODES_E ADR_SetPowerMode(void*				DeviceHandle,
										ADR_POWER_MODES_E	adrPowerMode);

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*! 
	Handle callback
	This callback is registered by the host lib or data lib in #ADR_OpenHandle
	It is called for delivering asynchronous information of that handle

\param[in]	ClientPtr	A client pointer that was given as an arguemnt to #ADR_OpenHandle
\param[in]	handle_num	The handle number
\param[in]	p_buffer	A buffer containing the new data from that handle
\param[in]	buff_size	The size of p_buffer in bytes

*/
typedef void ( *ADR_pfnFuncCb )(  void* ClientPtr, UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size );

///////////////////////////////////////////////////////////////////////////////
/*! 
	Stream Driver Open Data Handle. 

	\param[in]	DeviceHandle	Handle to the ADR device.
	\param[in]	handle_num		Handle number generated from control library. 0 for control.
	\param[in]	pfnFuncCb		Callback for client interrupt/event.
	\param[in]	ClientPtr		User pointer that will be passed in call to pfnFuncCb.

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.

	\remark		To retrieve data when pfnFuncCb is set to NULL,
				the user should call to #ADR_ReadBuf.
				When pfnFuncCb is set legal function address, the driver
				calls the supplied function upon data arrival, in this case the caller 
				must NOT call to #ADR_ReadBuf.
*/
SMSHOSTLIB_ERR_CODES_E ADR_OpenHandle(	void*			DeviceHandle,
										UINT32			handle_num,
										ADR_pfnFuncCb	pfnFuncCb,
										void*			ClientPtr);





#ifdef __cplusplus
}
#endif

#endif // _APP_DRIVER_INT_H
