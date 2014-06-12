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
/*      SmsHostLibTypes.h		                                             */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Siano Host Library API types and structures						 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*************************************************************************/

#ifndef _SMS_HOST_LIB_LITE_MS_H_
#define _SMS_HOST_LIB_LITE_MS_H_

// Win32 import/export API macro
#if defined(WIN32) || defined(OS_WINCE)
#ifdef SMSHOSTLIBLITE_EXPORTS
#define SMSHOSTLITE_API __declspec(dllexport)
#else
#define SMSHOSTLITE_API __declspec(dllimport)
#endif
#else
#define SMSHOSTLITE_API
#endif


#ifdef __cplusplus
extern "C" {
#endif

#include "SmsHostLibTypes.h"

/// HostApi control lib client callback function prototype
typedef void (*SmsHostLiteCbFunc)(	SMSHOSTLIB_MSG_TYPE_RES_E	MsgType,		//!< Response type	
									SMSHOSTLIB_ERR_CODES_E		ErrCode,		//!< Response success code
									UINT8* 						pPayload,		//!< Response payload
									UINT32						PayloadLen );	//!< Response payload length

// Callback for service data read from an active service
typedef void ( *SmsHostLiteDataCbFunc)(UINT32	ServiceHandle, 
									  UINT8*	pBuffer, 
									  UINT32	BufferSize );


typedef struct SMSHOSTLIBLITE_MS_INITLIB_PARAMS_S
{
	UINT32						Size;				//!< Put sizeof(#SMSHOSTLIBLITE_MS_INITLIB_PARAMS_ST)into this field
	SmsHostLiteCbFunc			pCtrlCallback;		//!< Callback for control responses
	SmsHostLiteDataCbFunc		pDataCallback;		//!< Callback for asynchronous data reading
	SMSHOSTLIB_DEVICE_MODES_E	DeviceMode;			//!< Device mode - DVBT/ISDBT, BDA/Non-BDA
	UINT32						Crystal;			//!< The crystal frequency used in the chip. 12MHz is the default - use 0 to leave unchanged.
} SMSHOSTLIBLITE_MS_INITLIB_PARAMS_ST;



SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteMsLibInit( SMSHOSTLIBLITE_MS_INITLIB_PARAMS_ST* pInitLibParams );

SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteMsLibTerminate( void );

void SMSHOSTLITE_API SmsLiteGetVersion_Req( void );

void SMSHOSTLITE_API SmsLiteMsGetStatistics_Req( void ); 

// DVBT
void SMSHOSTLITE_API SmsLiteDvbtTune_Req( UINT32 Frequency, UINT32 Bandwidth );

void SMSHOSTLITE_API SmsLiteDvbtAddPidFilter_Req( UINT32 Pid );

void SMSHOSTLITE_API SmsLiteDvbtRemovePidFilter_Req( UINT32 Pid );

// PID list response format:
// UINT32 N - Number of PID filters
// Followed by an array of N Pid numbers, each UINT32 
void SMSHOSTLITE_API SmsLiteDvbtRetrievePidFilterList_Req( void );

// ISDBT
void SMSHOSTLITE_API SmsLiteIsdbtTune_Req( UINT32 Frequency,
								 SMSHOSTLIB_FREQ_BANDWIDTH_ET Bandwidth,
								 UINT32 SegmentNumber);
void SMSHOSTLITE_API SmsLiteMsSetAES128Key_Req( const char* KeyString );

SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteSetDbgLogMask(UINT32 newDbgLogMask);
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteGetDbgLogMask(UINT32* pCurDbgLogMask);


#ifdef __cplusplus
}
#endif

#endif //_SMS_HOST_LIB_LITE_MS_H_

