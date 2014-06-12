/*************************************************************************/
/*                                                                       */
/*   Copyright (C) 2005 Siano Mobile Silicon Ltd. All rights reserved    */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
#ifndef SMS_DBG_LOGS_H
#define SMS_DBG_LOGS_H

#ifdef __cplusplus
extern "C"{
#endif


/*************************************************************************
*			 Macros
*************************************************************************/
#define SMS_MAX_FORMATED_LOG_STRING (512)
//#define ZERO_MEM_OBJ(pObj) memset((pObj), 0, sizeof(*(pObj)))
#define SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( _pMsg ) \
	{	\
	(_pMsg)->xMsgHeader.msgSrcId = SMS_HOST_LIB; \
	(_pMsg)->xMsgHeader.msgDstId = HIF_TASK; \
	(_pMsg)->xMsgHeader.msgFlags = MSG_HDR_FLAG_STATIC_MSG; \
	}




/*************************************************************************
*			 Enums
*************************************************************************/

// Host Lib Debug Log mask bits
typedef enum
{
	// Severities bits
	SMSLOG_ERROR			= 1 << 30,
	SMSLOG_WARRING			= 1 << 29,

	// General bits
	SMSLOG_MAIN				= 1 << 0, // Main module and internal tasks communication
	SMSLOG_COMM				= 1 << 1, // communication between the host and the chip
	SMSLOG_APIS				= 1 << 2, // communication between the user and the library
	SMSLOG_SRVM				= 1 << 3, // SRVM module
	SMSLOG_SCAN				= 1 << 4, // SCAN module
	SMSLOG_AGING			= 1 << 5, // AGING module
	SMSLOG_FIC				= 1 << 6, // FIC module
	SMSLOG_DABCRC			= 1 << 7, // DABCRC module 
	SMSLOG_PWRMNG			= 1 << 8, // Power Management from FW
	SMSLOG_COMM_LOGS		= 1 << 9,// Logs from FW - as oposite to all other SMSLOG_COMM messages
	SMSLOG_CMMB				= 1 << 10,// Logs for CMMB
	SMSLOG_FW_DEBUG			= 1 << 11, // FW log message
	SMSLOG_FW_INFO			= 1 << 12,
	SMSLOG_FW_ERROR			= 1 << 13,

	SMSLOG_RESERVED_LAST	= 1 << 28

} SMSHOSTLIB_DBG_LOG_MASK_ET;

/*************************************************************************
*			 Fwd Declarations
*************************************************************************/
void SmsLiteCommonControlRxHandler(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size );
UINT32 SmsProcessLog( struct SMSHOSTLIB_LOG_ITEM_S* p_x_log_item );
SMSHOSTLIB_ERR_CODES_E SmsLiteSetDeviceFwLogState( void );
BOOL SmsHostWaitForFlagSet(BOOL* pFlag, UINT32 TimeToWaitMs);
SMSHOSTLIB_ERR_CODES_E SmsLiteSendCtrlMsg( SmsMsgData_ST* pMsg );
SMSHOSTLIB_ERR_CODES_E SmsLiteInit( SmsHostLiteCbFunc pCtrlCallback );

void SmsLiteCallCtrlCallback(	SMSHOSTLIB_MSG_TYPE_RES_E	MsgType,
							 SMSHOSTLIB_ERR_CODES_E		RetCode,
							 void*						pPayload,
							 UINT32						PayloadLen);

/*************************************************************************
*			 Globals
*************************************************************************/



/*************************************************************************
*			 Log Macros
*************************************************************************/

#ifdef SMSHOST_ENABLE_LOGS

	extern void SmsHostLog(UINT32 mask,char* format, ...);

	#define SMSHOST_LOG0(mask,format) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION )
	#define SMSHOST_LOG1(mask,format,arg1) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1)
	#define SMSHOST_LOG2(mask,format,arg1,arg2) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2)
	#define SMSHOST_LOG3(mask,format,arg1,arg2,arg3) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3)
	#define SMSHOST_LOG4(mask,format,arg1,arg2,arg3,arg4) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4)
	#define SMSHOST_LOG5(mask,format,arg1,arg2,arg3,arg4,arg5) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5)
	#define SMSHOST_LOG6(mask,format,arg1,arg2,arg3,arg4,arg5,arg6) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6)
	#define SMSHOST_LOG7(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7)
	#define SMSHOST_LOG8(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
	#define SMSHOST_LOG9(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)
	#define SMSHOST_LOG10(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10)

#else	// SMSHOST_ENABLE_LOGS

	#define SMSHOST_LOG0(mask,format){}
	#define SMSHOST_LOG1(mask,format,arg1){}
	#define SMSHOST_LOG2(mask,format,arg1,arg2){}
	#define SMSHOST_LOG3(mask,format,arg1,arg2,arg3){}
	#define SMSHOST_LOG4(mask,format,arg1,arg2,arg3,arg4){}
	#define SMSHOST_LOG5(mask,format,arg1,arg2,arg3,arg4,arg5){}
	#define SMSHOST_LOG6(mask,format,arg1,arg2,arg3,arg4,arg5,arg6){}
	#define SMSHOST_LOG7(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7){}
	#define SMSHOST_LOG8(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8){}
	#define SMSHOST_LOG9(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9){}
	#define SMSHOST_LOG10(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10){}

#endif	// SMSHOST_ENABLE_LOGS


#ifdef __cplusplus
}
#endif

#endif
 

