/*                                                                       */
/*   Copyright (C) 2006 Siano Mobile Silicon Ltd. All rights reserved    */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
#include <linux/delay.h>
#include <linux/slab.h>

#include "../Platforms/RKOS/SmsLitePlatDefs.h"

#include "../include/SmsHostLibTypes.h"
#include "../include/SmsHostLibLiteMs.h"
//#include "../include/SmsHostLibLiteCmmb.h"
//#include "../include/SmsHostLibLiteMs.h"
#include "../include_internal/SmsFirmwareApi.h"
#include "../include_internal/SmsHostLibLiteCommon.h"
#include "../include_internal/SmsLiteAppDriver.h"

/*************************************************************************
*			 Macros
*************************************************************************/
#define SMSHOSTLIB_MAX_NUM_LOG_PARAMETERS                   ( 8 )
#define SMS_HOST_LITE_VERSION_STRING	("SMS Lite Host Library Version: 0.0.1 " __DATE__ " " __TIME__ )/*!< Release code */

#if !defined(SMSHOST_LINE) || !defined(SMSHOST_FUNCTION) || !defined(USER_PRINTF)
	#error missing defines for using Sms Host Lib Logs. Please define all required defines above.
#endif

/* if the user didn't define default value, define the default value */

#ifndef SMSHOST_DEFAULT_LOG_MASK
	//#define SMSHOST_DEFAULT_LOG_MASK	( SMSLOG_ERROR | SMSLOG_WARRING )
	#define SMSHOST_DEFAULT_LOG_MASK	( ~0)
#endif


/*************************************************************************
*			 Structs
*************************************************************************/
typedef struct SMSHOSTLIB_LOG_HEADER_S
{
	UINT32 n_filtering_classification;
	UINT32 e_format_string_tag;
	UINT32 n_timestamp_microseconds;
} SMSHOSTLIB_LOG_HEADER_ST;

typedef struct SMSHOSTLIB_LOG_ITEM_S
{
	SMSHOSTLIB_LOG_HEADER_ST x_header;
	UINT32                   a_n_parameters[SMSHOSTLIB_MAX_NUM_LOG_PARAMETERS];
} SMSHOSTLIB_LOG_ITEM_ST;

typedef struct SMSHOSTLIB_LOG_ITEM_EX_S
{
	UINT32 NumLogs;
	SMSHOSTLIB_LOG_ITEM_ST LogItems[1]; //Variable size for printf logs.
} SMSHOSTLIB_LOG_ITEM_EX_ST;

typedef struct SmsLiteCommon_S
{
	BOOL IsBlockInterface;
	SmsHostLiteCbFunc pCtrlCallback;
	char VersionStr[SMSHOSTLIB_MAX_MODEM_VERSION_STRING_SIZE];
} SmsLiteCommon_ST;


/*************************************************************************
*			 Forward Declarations
*************************************************************************/
static int sms_snprintf(char* buff, size_t buffSize, const char *format, ...);

/*************************************************************************
*			 Globals
*************************************************************************/
/* global logging mask */
UINT32 g_SmsDbgLogMask = SMSHOST_DEFAULT_LOG_MASK;
const char g_LibStrVersion[] = SMS_HOST_LITE_VERSION_STRING;

SmsLiteCommon_ST g_LiteCommon = {0};



//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SmsLiteInit( SmsHostLiteCbFunc pCtrlCallback )

{
	//ZERO_MEM_OBJ( &g_LiteCommon );
        memset(&g_LiteCommon, 0, sizeof(g_LiteCommon));//jan
	g_LiteCommon.pCtrlCallback = pCtrlCallback;
	return SMSHOSTLIB_ERR_OK;
}


//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteGetVersion_Req( void )
{
	SmsMsgData_ST SmsMsg = {0};
	SMSHOST_LOG0(SMSLOG_APIS,"START");

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_GET_VERSION_EX_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS,"END");
	return;
} //SmsLiteGetVersion_Req


#ifdef SMSHOST_ENABLE_LOGS

// Create a strings array for processing FW logs

#include "strings_and_tags.h"

#undef X

// (3) Define X-Macro for generating string names
#define X(id, idString) idString,
const char* g_a_sms_strings[] =
{
	GENERATE_IDS
		NULL
};
#undef X

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteSetDbgLogMask(UINT32 newDbgLogMask)
{
	BOOL fwBitChanged = ((g_SmsDbgLogMask ^ newDbgLogMask) & (SMSLOG_FW_DEBUG|SMSLOG_FW_INFO|SMSLOG_FW_ERROR));

	g_SmsDbgLogMask = newDbgLogMask;

	if (fwBitChanged)
	{
		SmsLiteSetDeviceFwLogState();
	}
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API
	SmsLiteGetDbgLogMask(UINT32* pCurDbgLogMask)
{
	*pCurDbgLogMask = g_SmsDbgLogMask;
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
void SmsHostLog(UINT32 mask,char* format, ...) 
{
	va_list args;
	if (mask & g_SmsDbgLogMask)
	{
		char log_str[SMS_MAX_FORMATED_LOG_STRING];
		va_start(args, format);
		vsprintf(log_str, format, args);

		USER_PRINTF(mask,log_str);
		va_end(args);
	}
}

//*******************************************************************************
// 
UINT32 SmsProcessLog( SMSHOSTLIB_LOG_ITEM_ST *p_x_log_item )
{
	// We can use a static buffer here because this function is always called from 
	// the same context - the control callback 
	static char c_str[SMS_MAX_FORMATED_LOG_STRING];

	UINT32 MsgDataLen;
	static UINT32 prevTime;
	static BOOL	firstTime = TRUE;

	c_str[0] = 0;

	if (firstTime)
	{
		firstTime = FALSE;
	}
	else if ((p_x_log_item->x_header.n_timestamp_microseconds - prevTime) > 1500000) // 1.5 sec
	{
		SMSHOST_LOG0(SMSLOG_FW_DEBUG|SMSLOG_FW_INFO|SMSLOG_FW_ERROR,"-------------------------------------");
	}
	prevTime = p_x_log_item->x_header.n_timestamp_microseconds;

	if ( p_x_log_item->x_header.e_format_string_tag == STR_TAG_SMS_PRINTF)
	{
		sms_snprintf( c_str, sizeof(c_str), "%s", (char*)p_x_log_item->a_n_parameters );
	}
	else if ( p_x_log_item->x_header.e_format_string_tag < ( sizeof( g_a_sms_strings )/sizeof( g_a_sms_strings[0] ) ) )
	{
		sms_snprintf( c_str, sizeof(c_str), g_a_sms_strings[p_x_log_item->x_header.e_format_string_tag],
			p_x_log_item->a_n_parameters[0],p_x_log_item->a_n_parameters[1],
			p_x_log_item->a_n_parameters[2],p_x_log_item->a_n_parameters[3],
			p_x_log_item->a_n_parameters[4],p_x_log_item->a_n_parameters[5],
			p_x_log_item->a_n_parameters[6],p_x_log_item->a_n_parameters[7] );
	}
	else
	{
		sms_snprintf( c_str, sizeof(c_str), "Unknown format: %x %x %x %x %x %x %x %x",
			(unsigned)p_x_log_item->a_n_parameters[0],(unsigned)p_x_log_item->a_n_parameters[1],
			(unsigned)p_x_log_item->a_n_parameters[2],(unsigned)p_x_log_item->a_n_parameters[3],
			(unsigned)p_x_log_item->a_n_parameters[4],(unsigned)p_x_log_item->a_n_parameters[5],
			(unsigned)p_x_log_item->a_n_parameters[6],(unsigned)p_x_log_item->a_n_parameters[7] );
	}

	SMSHOST_LOG4(SMSLOG_FW_DEBUG|SMSLOG_FW_INFO|SMSLOG_FW_ERROR,"FW MSG: %3d.%03d %03d %s",
		p_x_log_item->x_header.n_timestamp_microseconds/1000000, 
		(p_x_log_item->x_header.n_timestamp_microseconds /1000) % 1000, 
		p_x_log_item->x_header.n_timestamp_microseconds%1000, 
		c_str);



	if(p_x_log_item->x_header.e_format_string_tag == STR_TAG_SMS_PRINTF)
	{
		UINT32 StrLen = (UINT32)strlen((char *)p_x_log_item->a_n_parameters);

		//Terminating \0
		StrLen++;

		//4 bytes align
		StrLen = ( (StrLen+3) & ~0x3 );

		MsgDataLen = sizeof(SMSHOSTLIB_LOG_HEADER_ST);

		MsgDataLen += StrLen;

	}
	else
	{
		MsgDataLen = sizeof(SMSHOSTLIB_LOG_ITEM_ST);
	}

	return MsgDataLen;
}


#else	// of ifdef SMSHOST_ENABLE_LOGS

/* implement functions to return not supported when SMSHOST_ENABLE_LOGS is undefined */

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteSetDbgLogMask(UINT32 newDbgLogMask)
{
	ARGUSE(newDbgLogMask);
	return SMSHOSTLIB_ERR_NOT_SUPPORTED;
}

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteGetDbgLogMask(UINT32* pCurDbgLogMask)
{
	ARGUSE(pCurDbgLogMask);
	return SMSHOSTLIB_ERR_NOT_SUPPORTED;
}

#endif // SMSHOST_ENABLE_LOGS

//*******************************************************************************
// 
static int sms_snprintf(char* buff, size_t buffSize, const char *format, ...)
{

	int ret =0;
#if 0
	va_list ap;												// argument pointer


	if ((buff == NULL) || (buffSize <= 0) || (format == NULL))
	{
		return -1; // failure
	}


	va_start (ap, format);									// initialize argument pointer

#ifdef WIN32
	ret = _vsnprintf (buff, buffSize, format, ap);			// use safer variant for WIN32

#else
	// In case we're not in WIN32, provide a naive protection
	if (strlen(format) > buffSize)
	{
		return -1;
	}

	ret = vsprintf (buff, format, ap);						// standard version for other platforms

#endif
	va_end( ap );
#endif	
	return ret;
} 

//*******************************************************************************
// 
void SmsLiteCommonControlRxHandler(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size )
{
	SmsMsgData_ST* pSmsMsg = (SmsMsgData_ST*)p_buffer;
	switch( pSmsMsg->xMsgHeader.msgType )
	{
	case MSG_SMS_LOG_ITEM:
		{
#ifdef SMSHOST_ENABLE_LOGS
			UINT32 Len = SmsProcessLog( (struct SMSHOSTLIB_LOG_ITEM_S*)&pSmsMsg->msgData[0] );
#else
			// When logs are disabled - report the log item to the host
			SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_LOG_ITEM_IND,
							SMSHOSTLIB_ERR_OK,
							(UINT8*)&pSmsMsg->msgData[0] ,
							pSmsMsg->xMsgHeader.msgLength - sizeof(SmsMsgHdr_ST) );

#endif
		}
		break;
	case MSG_SMS_LOG_EX_ITEM:
		{
#ifdef SMSHOST_ENABLE_LOGS
			UINT32 i;
			UINT8 *pItemBuf;
			INT32 SizeLeft = pSmsMsg->xMsgHeader.msgLength - sizeof(SmsMsgHdr_ST);
			SMSHOSTLIB_LOG_ITEM_EX_ST *pLogEx = (SMSHOSTLIB_LOG_ITEM_EX_ST *)&pSmsMsg->msgData[0];
			pItemBuf = (UINT8 *)&(pLogEx->LogItems[0]);
			for(i = 0 ; i < pLogEx->NumLogs ; i++)
			{
				UINT32 CurItemLen = SmsProcessLog((SMSHOSTLIB_LOG_ITEM_ST *)pItemBuf);
				pItemBuf += CurItemLen;
				SizeLeft -= CurItemLen;

			}
#else
			SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_LOG_ITEM_IND,
					SMSHOSTLIB_ERR_OK,
					(UINT8*)&pSmsMsg->msgData[0],
					pSmsMsg->xMsgHeader.msgLength - sizeof(SmsMsgHdr_ST) );
#endif
		}
		break;
	case MSG_SMS_INTERFACE_LOCK_IND:
		{
			// Firmware has requested that the host avoid sending messages to it
			SMSHOST_LOG0(SMSLOG_COMM, "Interface lock");
			g_LiteCommon.IsBlockInterface = TRUE;
		}
		break;
	case MSG_SMS_INTERFACE_UNLOCK_IND:
		{
			// Firmware allows host to resume transmission
			SMSHOST_LOG0(SMSLOG_COMM, "Interface unlock");
			g_LiteCommon.IsBlockInterface = FALSE;
		}
		break;

	case MSG_SMS_LOG_ENABLE_CHANGE_RES:
		// NOP
		break;
	case MSG_SMS_GET_VERSION_EX_RES:
		{
			UINT32 CurStrLen = 0 ; 
			UINT32 Len;
			SMSHOSTLIB_VERSION_ST VerStruct = {0};
			memcpy(&VerStruct, pSmsMsg->msgData, sizeof(VerStruct));

			// Copy chip version string
			VerStruct.TextLabel[33] = 0;
			Len = (UINT32)SMS_MIN((UINT32) strlen( (char *)VerStruct.TextLabel ), sizeof( g_LiteCommon.VersionStr ) - CurStrLen - 1 );
			memcpy( &g_LiteCommon.VersionStr[CurStrLen], VerStruct.TextLabel, Len );
			CurStrLen += Len;
			
			Len = SMS_MIN( sizeof(g_LibStrVersion), sizeof( g_LiteCommon.VersionStr ) - CurStrLen - 1);
			memcpy(&g_LiteCommon.VersionStr[CurStrLen], g_LibStrVersion, Len);
			CurStrLen += Len;
			g_LiteCommon.VersionStr[CurStrLen] = 0;

			SMSHOST_LOG1(SMSLOG_APIS,"%s", g_LiteCommon.VersionStr );
			SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_GET_VERSION_RES, 
				SMSHOSTLIB_ERR_OK, 
				g_LiteCommon.VersionStr, 
				CurStrLen );
		}
		break;

	default:
		break;
	}
}


//*******************************************************************************
// 
#ifdef SMSHOST_ENABLE_LOGS
SMSHOSTLIB_ERR_CODES_E SmsLiteSetDeviceFwLogState( void )
{
	SmsMsgData2Args_ST		SmsMsg;
	UINT32					CurLogMask;
	LogSeverity_ET LogSevirity = FW_LOG_SEVERITY_NONE; 


	SMSHOST_LOG0(SMSLOG_APIS,"START");

	SmsLiteGetDbgLogMask(&CurLogMask);

	if(CurLogMask&SMSLOG_FW_ERROR)
	{
		LogSevirity = FW_LOG_SEVERITY_ERROR; 
	}
	if(CurLogMask&SMSLOG_FW_INFO)
	{
		LogSevirity = FW_LOG_SEVERITY_INFO; 
	}
	if(CurLogMask&SMSLOG_FW_DEBUG)
	{
		LogSevirity = FW_LOG_SEVERITY_DEBUG; 
	}

	SmsMsg.xMsgHeader.msgSrcId = SMS_HOST_LIB; 
	SmsMsg.xMsgHeader.msgDstId = HIF_TASK; 
	SmsMsg.xMsgHeader.msgFlags = MSG_HDR_FLAG_STATIC_MSG; 

	SmsMsg.xMsgHeader.msgType  = MSG_SMS_LOG_ENABLE_CHANGE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	SmsMsg.msgData[0] = LogSevirity == FW_LOG_SEVERITY_NONE ? LOG_TYPE_DISABLED : LOG_TYPE_EXTENDED;
	SmsMsg.msgData[1] =  LogSevirity;

	SmsLiteAdrWriteMsg( (SmsMsgData_ST*)&SmsMsg );

	SMSHOST_LOG1(SMSLOG_APIS,"return err 0x%x",SMSHOSTLIB_ERR_OK);
	return SMSHOSTLIB_ERR_OK;

} 
#endif

//*******************************************************************************
// 
BOOL SmsHostWaitForFlagSet(BOOL* pFlag, UINT32 TimeToWaitMs)
{
	UINT32		elapsedTime;
	volatile BOOL* pVFlag = (volatile BOOL*)pFlag;
	//OswLiteTaskSleep (1);
        msleep(1); //jan
        //mdelay(1); //jan
	return TRUE;
	SMS_ASSERT (pVFlag != NULL);
	if (*pVFlag) return TRUE;

	for (elapsedTime = 0; elapsedTime < TimeToWaitMs; elapsedTime += 10)
	{
		OswLiteTaskSleep (10);
		if (*pVFlag) return TRUE;
	}

	return FALSE;

} /* SmsHostWaitForFlagSet */

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SmsLiteSendCtrlMsg( SmsMsgData_ST* pMsg )
{
	SMSHOSTLIB_ERR_CODES_E RetCode;
	UINT32 FirstLoop = TRUE;

	while ( g_LiteCommon.IsBlockInterface )
	{
		if ( FirstLoop )
		{
			SMSHOST_LOG0(SMSLOG_COMM, "Interface blocked. Waiting...");
		}
		FirstLoop = FALSE;
		//OswLiteTaskSleep(1);
                msleep(1); //jan
                //mdelay(1); //jan
	}

	RetCode = SmsLiteAdrWriteMsg(  pMsg );
	if ( !(pMsg->xMsgHeader.msgFlags & MSG_HDR_FLAG_STATIC_MSG) )
	{
		//OswLiteMemFree( pMsg );
                if(pMsg)
                {
                    kfree(pMsg);
                }
	}

	return RetCode;
}

//*******************************************************************************
// 
void SmsLiteCallCtrlCallback(	SMSHOSTLIB_MSG_TYPE_RES_E	MsgType,
								SMSHOSTLIB_ERR_CODES_E		RetCode,
								void*						pPayload,
								UINT32						PayloadLen)
{
	if ( g_LiteCommon.pCtrlCallback != NULL)
	{
		SMSHOST_LOG3(SMSLOG_APIS,"Call user CB : Status 0x%x MsgType %d MsgLen %d",
			RetCode,
			MsgType,
			PayloadLen);

		g_LiteCommon.pCtrlCallback(MsgType,
			RetCode,
			pPayload,
			PayloadLen);
	}
}
