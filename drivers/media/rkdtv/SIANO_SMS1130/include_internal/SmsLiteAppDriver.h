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

#include "Include/sms_common.h"

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

	///////////////////////////////////////////////////////////////////////////////
/*! 
	Handle callback
	This callback is registered by the host lib or data lib in #ADR_OpenHandle
	It is called for delivering asynchronous information of that handle

\param[in]	handle_num	The handle number
\param[in]	p_buffer	A buffer containing the new data from that handle
\param[in]	buff_size	The size of p_buffer in bytes

*/
typedef void ( *SmsLiteAdr_pfnFuncCb )( UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size );
//typedef void ( *SmsLiteAdr_pfnFuncCb )( void* handle_num, void* p_buffer, void* buff_size );


///////////////////////////////////////////////////////////////////////////////
/*! 
	Stream Driver Initialization. 

	\param[in]	e_comm_type	Communication type
	\param[in]	p_params	Driver dependent extra parameters
	\param[in]	pfPnPCbFunc	Callback for client plug and play event

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E SmsLiteAdrInit( SMSHOSTLIB_DEVICE_MODES_E DeviceMode, 
									  SmsLiteAdr_pfnFuncCb pfnControlCb, 
									  SmsLiteAdr_pfnFuncCb pfnDataCb );

//									  void * pfnControlCb, 
//									  void * pfnDataCb );



typedef void ( *ADR_pfnFuncCb1 )(  void* ClientPtr1, UINT32 handle_num1, UINT8* p_buffer1, UINT32 buff_size1, UINT32 xx );


///////////////////////////////////////////////////////////////////////////////
/*! 
	Stream Driver Initialization. 

	\param[in]	e_comm_type	Communication type
	\param[in]	p_params	Driver dependent extra parameters
	\param[in]	pfPnPCbFunc	Callback for client plug and play event

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E ADR_OpenHandle1(	void*			DeviceHandle,
										UINT32			handle_num,
										ADR_pfnFuncCb1	pfnFuncCb1,
										void*			ClientPtr);






///////////////////////////////////////////////////////////////////////////////
/*! 
	Stream Driver Termination. 

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E SmsLiteAdrTerminate( void );

///////////////////////////////////////////////////////////////////////////////
/*! 
	Write message (little-endian ordering).
	
	\param[in]	DeviceHandle	Handle return from call to #ADR_OpenHandle.
	\param[in]	p_msg			Pointer to a message to write

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E SmsLiteAdrWriteMsg( SmsMsgData_ST* p_msg );

///////////////////////////////////////////////////////////////////////////////

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


/*! 
	Stream Driver Termination. 

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E ADR_Terminate( void );


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






#ifdef __cplusplus
}
#endif

#endif // _APP_DRIVER_INT_H
