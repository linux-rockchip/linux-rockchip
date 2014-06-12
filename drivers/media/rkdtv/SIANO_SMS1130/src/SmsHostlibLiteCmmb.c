#include "../Platforms/RKOS/SmsLitePlatDefs.h"
#include "../include/SmsHostLibTypes.h"
#include "../include/SmsHostLibLiteCmmb.h"
#include "../include_internal/SmsFirmwareApi.h"
#include "../include_internal/SmsHostLibLiteCommon.h"
#include "../include_internal/SmsLiteAppDriver.h"

/*************************************************************************
*			 Macros
*************************************************************************/

#define MAX_DIR_NAME_SIZE	256
#define SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE (3)

/*************************************************************************
*			 Structs
*************************************************************************/

// Host lib state global to all instances
typedef struct SmsLiteCmmbGlobalState_S
{
	BOOL						IsLibInit;
	SmsHostLiteDataCbFunc		pDataCallback; 
	UINT32						Crystal;
	BOOL						SyncFlag;
	SMSHOSTLIB_STATISTICS_CMMB_ST StatsCache;
	SMSHOSTLIB_STATISTICS_CMMB_ST UserStatsToReturn;
	UINT32						ReceptionQualityHistory[SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE];
	UINT32						ReceptionQualityCounter;
	BOOL						IsBlockInterface;
//	UINT32						OpenServiceHandlesMask;
} SmsLiteCmmbGlobalState_ST ;

/*************************************************************************
*			 Fwd Declarations
*************************************************************************/
//extern SMSHOSTLIB_ERR_CODES_E SmsLiteInit( SmsHostLiteCbFunc pCtrlCallback );

static UINT32 SmsHostCmmbGetQuality( SMSHOSTLIB_STATISTICS_CMMB_ST *stats );
static void SmsLiteCmmbDataCallback(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size );
static void SmsLiteCmmbControlRxCallback(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size );

/*************************************************************************
*			 Globals
*************************************************************************/
SmsLiteCmmbGlobalState_ST	g_LibCmmbState = { 0 } ;

 
//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteCmmbLibInit( SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST* pInitLibParams )
{
	SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST LocalInitParams = {0};
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;

	SMSHOST_LOG1(SMSLOG_APIS,"Crystal=%d", pInitLibParams->Crystal );
 
	if ( g_LibCmmbState.IsLibInit )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_ERROR,"Return err 0x%x",SMSHOSTLIB_ERR_LIB_ALREADY_INITIATED);
		return SMSHOSTLIB_ERR_LIB_ALREADY_INITIATED;
	}

	//ZERO_MEM_OBJ(&g_LibCmmbState);
        memset(&g_LibCmmbState, 0, sizeof(g_LibCmmbState)); //jan

	if ( pInitLibParams == NULL 
		|| pInitLibParams->pCtrlCallback == NULL 
		|| pInitLibParams->Size == 0 )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_ERROR,"Return err 0x%x",SMSHOSTLIB_ERR_INVALID_ARG);
		return SMSHOSTLIB_ERR_INVALID_ARG;
	}

	memcpy( &LocalInitParams, pInitLibParams, pInitLibParams->Size );
	
//	SmsLiteInit( LocalInitParams.pCtrlCallback );
	g_LibCmmbState.pDataCallback = LocalInitParams.pDataCallback;
	g_LibCmmbState.Crystal = LocalInitParams.Crystal;
	if ( LocalInitParams.Crystal == 0 )
	{
		g_LibCmmbState.Crystal = SMSHOSTLIB_DEFAULT_CRYSTAL;
	}

	RetCode = SmsLiteAdrInit( SMSHOSTLIB_DEVMD_CMMB, SmsLiteCmmbControlRxCallback, SmsLiteCmmbDataCallback );
	if ( RetCode != SMSHOSTLIB_ERR_OK )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_ERROR,"Return err 0x%x",RetCode);
		return RetCode ;
	}

	// Set crystal message
	if ( g_LibCmmbState.Crystal != SMSHOSTLIB_DEFAULT_CRYSTAL )
	{
		SmsMsgData_ST SmsMsg = {0};
		SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
		SmsMsg.xMsgHeader.msgType  = MSG_SMS_NEW_CRYSTAL_REQ;
		SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

		SmsMsg.msgData[0] = g_LibCmmbState.Crystal;

		g_LibCmmbState.SyncFlag = FALSE;
		SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg );

		// Wait for device init response
		if ( !SmsHostWaitForFlagSet( &g_LibCmmbState.SyncFlag, 200 ) )
		{
			return SMSHOSTLIB_ERR_DEVICE_NOT_INITIATED;
		}
	}

	g_LibCmmbState.IsLibInit = TRUE ;
#ifdef SMSHOST_ENABLE_LOGS
	SmsLiteSetDeviceFwLogState();
#endif

	//SmsLiteGetVersion_Req();
	 
	SMSHOST_LOG0(SMSLOG_APIS,"LibInit OK");
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteCmmbLibTerminate( void )
{
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;

	SMSHOST_LOG0(SMSLOG_APIS,"START");

	g_LibCmmbState.IsLibInit = FALSE;
	g_LibCmmbState.pDataCallback = NULL;

	RetCode = SmsLiteAdrTerminate();
	if ( RetCode != SMSHOSTLIB_ERR_OK )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_ERROR,"Return err 0x%x",RetCode);
		return RetCode;
	}

	SMSHOST_LOG0(SMSLOG_APIS,"END");
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteCmmbGetStatistics_Req( void )
{
	SmsMsgData_ST SmsMsg = {0};
	SMSHOST_LOG0(SMSLOG_APIS,"START");

	//ZERO_MEM_OBJ( &g_LibCmmbState.UserStatsToReturn );
        memset(&g_LibCmmbState.UserStatsToReturn, 0, sizeof(g_LibCmmbState.UserStatsToReturn)); //jan
	
	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	g_LibCmmbState.UserStatsToReturn = g_LibCmmbState.StatsCache;

	SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES,
		SMSHOSTLIB_ERR_OK,
		&g_LibCmmbState.UserStatsToReturn,
		sizeof(g_LibCmmbState.UserStatsToReturn) );

	SMSHOST_LOG0(SMSLOG_APIS,"END");
} // SmsLiteCmmbGetStatistics_Req

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteCmmbTune_Req( UINT32 Frequency, UINT32 Bandwidth )
{
	UINT8 MsgBuf[sizeof(SmsMsgHdr_ST) + 6*sizeof(UINT32)] = {0};
	SmsMsgData_ST* pSmsMsg = (SmsMsgData_ST*)MsgBuf;
	SMSHOST_LOG2(SMSLOG_APIS,"Frequency %u, Bandwidth %d", Frequency, Bandwidth );

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_TUNE_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( pSmsMsg );
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_SCAN_START_REQ;
	pSmsMsg->xMsgHeader.msgLength = (UINT16)sizeof(MsgBuf);
	pSmsMsg->msgData[0] = Bandwidth;		// Bandwidth
	pSmsMsg->msgData[1] = 0;				// Flags
	pSmsMsg->msgData[2] = SCAN_PARAM_RANGE;	// Scan by range or table
	pSmsMsg->msgData[3] = Frequency;		// Range start frequency
	pSmsMsg->msgData[4] = 0;				// Gap 
	pSmsMsg->msgData[5] = 0;				// Range end frequency 

	SmsLiteSendCtrlMsg( pSmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteCmmbStartControlInfo_Req( void )
{
	SmsMsgData2Args_ST SmsMsg = {0};

	SMSHOST_LOG0(SMSLOG_APIS,"START");
	
	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_START_CONTROL_INFO_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_START_CONTROL_INFO_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	SmsMsg.msgData[0] = SMS_CMMB_NETWORK_LEVEL_USE_CURRENT;	// Unique network level
	SmsMsg.msgData[1] = 0xFFFF;	// Don't care


	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteCmmbStopControlInfo_Req( void )
{
	SmsMsgData_ST SmsMsg = {0};

	SMSHOST_LOG0(SMSLOG_APIS,"START");

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_STOP_CONTROL_INFO_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteCmmbStartService_Req( UINT32 ServiceId )
{
	SmsMsgData3Args_ST SmsMsg = {0};

	SMSHOST_LOG1(SMSLOG_APIS,"ServiceId %d", ServiceId);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_START_SERVICE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	
	SmsMsg.msgData[0] = SMS_CMMB_NETWORK_LEVEL_USE_CURRENT;	// Unique network level
	SmsMsg.msgData[1] = 0xFFFF;	// Don't care
	SmsMsg.msgData[2] = ServiceId;


	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteCmmbStopService_Req( UINT32 ServiceHandle )
{
	SmsMsgData_ST SmsMsg = {0};

	SMSHOST_LOG1(SMSLOG_APIS,"Service Handle %d", ServiceHandle);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_STOP_SERVICE_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_STOP_SERVICE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsMsg.msgData[0] = ServiceHandle;

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SmsLiteCmmbControlRxCallback( UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size )
{
	SmsMsgData_ST* pSmsMsg = (SmsMsgData_ST*)p_buffer;
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_UNDEFINED_ERR;
	UINT32 ResponseMsgType = SMSHOSTLIB_MSG_INVALID_RESPONSE_VAL;
	UINT8* pPayload = NULL;
	UINT32 PayloadLength = 0;

	// Return code and payload for the messages which have retcode as the first 4 bytes
	UINT8* pPayloadWoRetCode = NULL;
	UINT32 PayloadLengthWoRetCode = 0;
	SMSHOSTLIB_ERR_CODES_E RetCodeFromMsg = SMSHOSTLIB_ERR_UNDEFINED_ERR;

	SMS_ASSERT( handle_num == 0 );
	SMS_ASSERT( p_buffer != NULL );
	SMS_ASSERT( buff_size != 0 );
	SMS_ASSERT( buff_size >= pSmsMsg->xMsgHeader.msgLength );
	SMS_ASSERT( pSmsMsg->xMsgHeader.msgLength >= sizeof( SmsMsgHdr_ST ) );

	pPayload = (UINT8*)&pSmsMsg->msgData[0];
	PayloadLength = pSmsMsg->xMsgHeader.msgLength - sizeof( SmsMsgHdr_ST );

	if ( PayloadLength >= 4 )
	{
		RetCodeFromMsg = pSmsMsg->msgData[0];
		pPayloadWoRetCode = pPayload + 4;
		PayloadLengthWoRetCode = PayloadLength - 4;
	}
	
	SMSHOST_LOG3( SMSLOG_ERROR, "Control RX callback. Type %d, Retcode %#x, Payload Length %d", 
		pSmsMsg->xMsgHeader.msgType,
		RetCodeFromMsg,
		PayloadLength );

	switch( pSmsMsg->xMsgHeader.msgType )
	{
	case MSG_SMS_GET_STATISTICS_EX_RES:
		{
			// This response is unsolicited - i.e. it comes periodically from the chip
			// without being requested.
			// We cache the responses to deliver later to stats requests from the app
			// We also use each response for reception quality calculation
			//
			// Note that the memcpy here is not protected - there is a race between here
			// and the request - when the cache is delivered to the app it could be updated
			// at the same time here. 
			//
			SMSHOSTLIB_STATISTICS_CMMB_ST* pStats = (SMSHOSTLIB_STATISTICS_CMMB_ST*)pPayloadWoRetCode;
			UINT32 i;
			UINT32 RecQualSum = 0;
			if ( pStats == NULL )
			{
				break;
			}
			memcpy( &g_LibCmmbState.StatsCache,
				pStats, 
				SMS_MIN( PayloadLengthWoRetCode, sizeof(SMSHOSTLIB_STATISTICS_CMMB_ST) ) );
			
			i = g_LibCmmbState.ReceptionQualityCounter++ % SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE;
			g_LibCmmbState.ReceptionQualityHistory[i] = SmsHostCmmbGetQuality( pStats );
			for ( i = 0 ; i < SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE ; i++ )
			{
				RecQualSum += g_LibCmmbState.ReceptionQualityHistory[i];
			}
			g_LibCmmbState.StatsCache.ReceptionQuality = RecQualSum / SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE;
		}
		break;
	case MSG_SMS_SCAN_COMPLETE_IND:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_TUNE_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_CMMB_START_CONTROL_INFO_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_START_CONTROL_INFO_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode; 
		}
		break;
	case MSG_SMS_CMMB_STOP_CONTROL_INFO_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_STOP_CONTROL_INFO_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_CMMB_START_SERVICE_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_CMMB_STOP_SERVICE_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_STOP_SERVICE_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_LOG_ENABLE_CHANGE_RES:
		// NOP
		break;
	case MSG_SMS_INTERFACE_LOCK_IND:
		{
			// Firmware has requested that the host avoid sending messages to it
			SMSHOST_LOG0(SMSLOG_COMM, "Interface lock");
			g_LibCmmbState.IsBlockInterface = TRUE;
		}
		break;
	case MSG_SMS_INTERFACE_UNLOCK_IND:
		{
			// Firmware allows host to resume transmission
			SMSHOST_LOG0(SMSLOG_COMM, "Interface unlock");
			g_LibCmmbState.IsBlockInterface = FALSE;
		}
		break;

	case MSG_SMS_CMMB_GET_CHANNELS_INFO_RES:
	case MSG_SMS_CMMB_GET_SERVICES_RES:
		{
			// "Internal" messages - forward to host as is
			ResponseMsgType = pSmsMsg->xMsgHeader.msgType;
		}
		break;
	default:
		SmsLiteCommonControlRxHandler( handle_num, p_buffer, buff_size );
		break;
	}

	
	// Call the user callback
	if ( ResponseMsgType != SMSHOSTLIB_MSG_INVALID_RESPONSE_VAL )
	{
		SmsLiteCallCtrlCallback( ResponseMsgType, 
								RetCode,
								pPayload,
								PayloadLength );
	}
}

//*******************************************************************************
// 
void SmsLiteCmmbDataCallback( UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size )
{
	if (g_LibCmmbState.pDataCallback)
	{
		g_LibCmmbState.pDataCallback( handle_num, p_buffer, buff_size );
	}
}

//*******************************************************************************
// Reception quality tables
//

typedef struct QualityCalcEntry_S
{
	INT32	RangeEnd ;
	INT32	Grade ;
} QUALITY_CALC_ENTRY_ST ;

static const QUALITY_CALC_ENTRY_ST CmmbLdpcBpskTable[] =
{
	// LDPC grades for CMMB running at BPSK
	//	RangeEnd 		Grade
	{	887,			100	},
	{	2000,			80	},
	{	4000,			60	},
	{	6000,			40	},
	{	8000,			20	},
	{	0x7FFFFFFF,		0	},
	{	-1,				-1	}
} ;

static const QUALITY_CALC_ENTRY_ST CmmbLdpcQpskTable[] =
{
	// LDPC grades for CMMB running at QPSK
	//	RangeEnd 		Grade
	{	887,			100	},
	{	2000,			80	},
	{	3500,			60	},
	{	4500,			40	},
	{	6000,			20	},
	{	0x7FFFFFFF,		0	},
	{	-1,				-1	}
} ;

static const QUALITY_CALC_ENTRY_ST CmmbLdpcQAM16Table[] =
{
	// LDPC grades for CMMB running at QAM16
	//	RangeEnd 		Grade
	{	887,			100	},
	{	1600,			80	},
	{	2400,			60	},
	{	3200,			40	},
	{	4000,			20	},
	{	0x7FFFFFFF,		0	},
	{	-1,				-1	}
} ;

//*******************************************************************************
// Get grade according to value ranges table
static INT32 SmsHostVal2Grade( const QUALITY_CALC_ENTRY_ST Table[], INT32 Val )
{
	UINT32 i=0 ;
	for ( i=0 ; Table[i].RangeEnd != -1 ; i++ )
	{
		if ( Val <= Table[i].RangeEnd )
		{
			SMS_ASSERT( Table[i].Grade <= 100 && Table[i].Grade >= 0 ) ;
			return Table[i].Grade ;
		}
	}
	return 0 ;
}


//*******************************************************************************
// Returns CMMB reception quality value based on statistics
// To change calculation parameters, calibrate the grades tables above,
// and the weights of each grade
//
static UINT32 SmsHostCmmbGetQuality( SMSHOSTLIB_STATISTICS_CMMB_ST *stats )
{
	double	receptionQuality = 0 ;
	UINT32	sum = 0;
	UINT32	i	= 0;
	UINT32	NumOfGrades = 0;
	INT32 grade = 0 ;


	if (stats->NumActiveChannels > 0)
	{
		for ( i = 0 ; i < stats->NumActiveChannels ; i++ )
		{
			SMSHOSTLIB_CMMB_CHANNEL_STATS_ST* pChannelStats = &stats->ChannelsStatsArr[i];

			if (pChannelStats->LdpcCycleCountAvg > 0)
			{
				switch (pChannelStats->Constellation)
				{
				case 0:	//BPSK
					grade = SmsHostVal2Grade( CmmbLdpcBpskTable, pChannelStats->LdpcCycleCountAvg );
					break;
				case 1:	//QPSK
					grade = SmsHostVal2Grade( CmmbLdpcQpskTable, pChannelStats->LdpcCycleCountAvg );
					break;
				case 2:	//QAM16
					grade = SmsHostVal2Grade( CmmbLdpcQAM16Table, pChannelStats->LdpcCycleCountAvg );
					break;
				default:
					grade = 0;
					break;
				}
				sum += grade;
				NumOfGrades++;
			}

		}

		receptionQuality = ( ((double)sum)/(NumOfGrades*100)*4)+1.5 ;
	}

	return (UINT32)receptionQuality;

}

//*******************************************************************************
// Undocumented API to send a FW message
SMSHOSTLITE_API SMSHOSTLIB_ERR_CODES_E SmsLiteCmmbSendFwMsg( SmsMsgData_ST* pMsg )
{
	return SmsLiteSendCtrlMsg( pMsg );
}
