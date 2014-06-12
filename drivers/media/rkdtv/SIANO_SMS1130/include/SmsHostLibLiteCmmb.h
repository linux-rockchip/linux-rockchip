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

#ifndef _SMS_HOST_LIB_LITE_CMMB_H_
#define _SMS_HOST_LIB_LITE_CMMB_H_

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
typedef void (*SmsHostLiteCbFunc)( SMSHOSTLIB_MSG_TYPE_RES_E	MsgType,		//!< Response type	
									 SMSHOSTLIB_ERR_CODES_E		ErrCode,		//!< Response success code
									 UINT8* 					pPayload,		//!< Response payload
									 UINT32						PayloadLen );	//!< Response payload length

// Callback for service data read from an active service
typedef void ( *SmsHostLiteDataCbFunc)( UINT32	ServiceHandle, 
									  UINT8*	pBuffer, 
									  UINT32	BufferSize );


typedef struct SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_S
{
	UINT32						Size;				//!< Put sizeof(#SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST)into this field
	SmsHostLiteCbFunc			pCtrlCallback;		//!< Callback for control responses
	SmsHostLiteDataCbFunc		pDataCallback;		//!< Callback for asynchronous data reading
	UINT32						Crystal;			//!< The crystal frequency used in the chip. 12MHz is the default - use 0 to leave unchanged.
} SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST;

SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteCmmbLibInit( SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST* pInitLibParams );

SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteCmmbLibTerminate( void );

void SMSHOSTLITE_API SmsLiteGetVersion_Req( void );

void SMSHOSTLITE_API SmsLiteCmmbGetStatistics_Req( void ); 

void SMSHOSTLITE_API SmsLiteCmmbTune_Req( UINT32 Frequency, UINT32 Bandwidth );

void SMSHOSTLITE_API SmsLiteCmmbStartControlInfo_Req( void );

void SMSHOSTLITE_API SmsLiteCmmbStopControlInfo_Req( void );

void SMSHOSTLITE_API SmsLiteCmmbStartService_Req( UINT32 ServiceId );

void SMSHOSTLITE_API SmsLiteCmmbStopService_Req( UINT32 ServiceHandle ); 

SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteSetDbgLogMask(UINT32 newDbgLogMask);
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteGetDbgLogMask(UINT32* pCurDbgLogMask);


#ifdef __cplusplus
}
#endif

#endif //_SMS_HOST_LIB_LITE_CMMB_H_