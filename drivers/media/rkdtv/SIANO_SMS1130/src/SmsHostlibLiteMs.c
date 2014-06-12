#include <linux/slab.h>

#include "../Platforms/RKOS/SmsLitePlatDefs.h"
#include "../include/SmsHostLibTypes.h"
#include "../include/SmsHostLibLiteMs.h"
#include "../include_internal/SmsFirmwareApi.h"
#include "../include_internal/SmsHostLibLiteCommon.h"
#include "../include_internal/SmsLiteAppDriver.h"

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

/*************************************************************************
*			 Macros
*************************************************************************/

#define MAX_DIR_NAME_SIZE	256
#define SMS_LITE_MS_IS_DVBT(_mode) ((_mode) == SMSHOSTLIB_DEVMD_DVBT || (_mode) == SMSHOSTLIB_DEVMD_DVBT_BDA )
#define SMS_DVBT_RECEPTION_QUALITY_HISTORY_SIZE (3)
#define SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( _pMsg ) \
		{	\
			(_pMsg)->xMsgHeader.msgSrcId = SMS_HOST_LIB; \
			(_pMsg)->xMsgHeader.msgDstId = HIF_TASK; \
			(_pMsg)->xMsgHeader.msgFlags = MSG_HDR_FLAG_STATIC_MSG; \
		}

//manipulators for statistics parameters.
//serve to change  old statistics and extended statistics structures.
#define CORRECT_STAT_RSSI(_stat) (_stat).RSSI *= -1
#define CORRECT_STAT_MRC_RSSI(_stat) (_stat).MRC_RSSI *= -1
#define CORRECT_STAT_BANDWIDTH(_stat) _stat.Bandwidth = 8 - _stat.Bandwidth
#define CORRECT_STAT_TRANSMISSON_MODE(_stat) \
			if(_stat.TransmissionMode == 0) _stat.TransmissionMode = 2; \
		else if(_stat.TransmissionMode == 1) _stat.TransmissionMode = 8; \
		else _stat.TransmissionMode = 4;

/*************************************************************************
*			 Structs
*************************************************************************/

// Host lib state global to all instances
typedef struct SmsLiteMsGlobalState_S
{
	BOOL						IsLibInit;
	SmsHostLiteDataCbFunc		pDataCallback; 
	SMSHOSTLIB_DEVICE_MODES_E	DeviceMode;
	UINT32						Crystal;
	BOOL						SyncFlag;
	BOOL						IsBlockInterface;

	SMSHOSTLIB_STATISTICS_DVBT_ST DvbtStatsCache;
	SMSHOSTLIB_STATISTICS_DVBT_ST DvbtUserStatsToReturn;

	UINT32						ReceptionQualityHistory[SMS_DVBT_RECEPTION_QUALITY_HISTORY_SIZE];
	UINT32						ReceptionQualityCounter;

	//	UINT32						OpenServiceHandlesMask;
} SmsLiteMsGlobalState_ST ;

/*************************************************************************
*			 Fwd Declarations
*************************************************************************/
void SmsLiteMsDataCallback(  UINT32 handle_num, 
						  UINT8* p_buffer, 
						  UINT32 buff_size );
void SmsLiteMsControlRxCallback(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size );

static SMSHOSTLIB_RECEPTION_QUALITY_ET SmsLiteMsCalcDvbtReceptionQuality( UINT32 SNR,
																	   INT32  InBand,
																	   UINT32 TSPackets,
																	   UINT32 ErrorTSPackets,
																	   UINT32 Constellation,
																	   UINT32 CodeRate);
static SMSHOSTLIB_ERR_CODES_E	SmsHandlePerSlicesIndication( SmsMsgData_ST* pSmsMsg );

//extern SMSHOSTLIB_ERR_CODES_E SmsLiteInit( SmsHostLiteCbFunc pCtrlCallback );


/*************************************************************************
*			 Globals
*************************************************************************/
SmsLiteMsGlobalState_ST	g_LibMsState				= { 0 };

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SmsLiteMsLibInit( SMSHOSTLIBLITE_MS_INITLIB_PARAMS_ST* pInitLibParams )
{
	SMSHOSTLIBLITE_MS_INITLIB_PARAMS_ST LocalInitParams = {0};
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;
	SmsMsgData3Args_ST SmsMsg = {0};

        SMSHOST_LOG0(SMSLOG_APIS,"");
        DBG("TSTV: #################Sms###############\t%s[%d]\n", __FUNCTION__, __LINE__);

	if ( g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_ERROR,"Return err 0x%x",SMSHOSTLIB_ERR_LIB_ALREADY_INITIATED);
		return SMSHOSTLIB_ERR_LIB_ALREADY_INITIATED;
	}

	//ZERO_MEM_OBJ(&g_LibMsState);
        memset(&g_LibMsState, 0, sizeof(g_LibMsState)); //jan

	if ( pInitLibParams == NULL 
		|| pInitLibParams->pCtrlCallback == NULL 
		|| pInitLibParams->Size == 0 )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_ERROR,"Return err 0x%x",SMSHOSTLIB_ERR_INVALID_ARG);
		return SMSHOSTLIB_ERR_INVALID_ARG;
	}
	memcpy( &LocalInitParams, pInitLibParams, pInitLibParams->Size );

	SmsLiteInit( LocalInitParams.pCtrlCallback );
	
	g_LibMsState.DeviceMode = LocalInitParams.DeviceMode;

	g_LibMsState.pDataCallback = LocalInitParams.pDataCallback;
	g_LibMsState.Crystal = LocalInitParams.Crystal;
	if ( LocalInitParams.Crystal == 0 )
	{
		g_LibMsState.Crystal = SMSHOSTLIB_DEFAULT_CRYSTAL;
	}
	
	RetCode = SmsLiteAdrInit( g_LibMsState.DeviceMode, 
					SmsLiteMsControlRxCallback, 
					SmsLiteMsDataCallback );

	if ( RetCode != SMSHOSTLIB_ERR_OK )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_ERROR,"Return err 0x%x",RetCode);
		return RetCode ;
	}

	// Device init message
	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_INIT_DEVICE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsMsg.msgData[0] = g_LibMsState.DeviceMode;
	
	g_LibMsState.SyncFlag = FALSE;
	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg );

	// Wait for device init response
	if ( !SmsHostWaitForFlagSet( &g_LibMsState.SyncFlag, 200 ) )
	{
		return SMSHOSTLIB_ERR_DEVICE_NOT_INITIATED;
	}


	// Set crystal message
	if ( g_LibMsState.Crystal != SMSHOSTLIB_DEFAULT_CRYSTAL )
	{
		SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
		SmsMsg.xMsgHeader.msgType  = MSG_SMS_NEW_CRYSTAL_REQ;
		SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

		SmsMsg.msgData[0] = g_LibMsState.Crystal;

		g_LibMsState.SyncFlag = FALSE;
		SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg );

		// Wait for device init response
		if ( !SmsHostWaitForFlagSet( &g_LibMsState.SyncFlag, 200 ) )
		{
			return SMSHOSTLIB_ERR_DEVICE_NOT_INITIATED;
		}
	}


	g_LibMsState.IsLibInit = TRUE ;
#ifdef SMSHOST_ENABLE_LOGS
	SmsLiteSetDeviceFwLogState();
#endif

	//SmsLiteGetVersion_Req();

	SMSHOST_LOG0(SMSLOG_APIS,"LibInit OK");
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLITE_API SmsLiteMsLibTerminate( void )
{
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;

	SMSHOST_LOG0(SMSLOG_APIS,"");

	g_LibMsState.IsLibInit = FALSE;
	g_LibMsState.pDataCallback = NULL;

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
void SMSHOSTLITE_API SmsLiteMsGetStatistics_Req( void )
{
	SMSHOST_LOG0(SMSLOG_APIS,"START");

	if ( !g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR,"Error- Lib not initialized!");
		return;
	}

	if ( !SMS_LITE_MS_IS_DVBT(g_LibMsState.DeviceMode) )
	{
		// ISDBT statistics - send a request to the chip
		SmsMsgData_ST SmsMsg = {0};
		SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
		SmsMsg.xMsgHeader.msgType  = MSG_SMS_GET_STATISTICS_EX_REQ;
		SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
		SmsLiteSendCtrlMsg( &SmsMsg );
	}
	else
	{
		SmsMsgData_ST SmsMsg = {0};
		SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
		SmsMsg.xMsgHeader.msgType  = MSG_SMS_GET_STATISTICS_REQ;
		SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
		SmsLiteSendCtrlMsg( &SmsMsg );
	}

	// DVBT statistics - return the cached statistics
	//ZERO_MEM_OBJ( &g_LibMsState.DvbtUserStatsToReturn );

	//g_LibMsState.DvbtUserStatsToReturn = g_LibMsState.DvbtStatsCache;

	//SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES,
	//	SMSHOSTLIB_ERR_OK,
	//	&g_LibMsState.DvbtUserStatsToReturn,
	//	sizeof(g_LibMsState.DvbtUserStatsToReturn) );

	SMSHOST_LOG0(SMSLOG_APIS,"END");
} // SmsLiteMsGetStatistics_Req

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteDvbtTune_Req( UINT32 Frequency, UINT32 Bandwidth )
{
	SmsMsgData3Args_ST SmsMsg = {0};
	SMSHOST_LOG2(SMSLOG_APIS,"Frequency %u, Bandwidth %d", Frequency, Bandwidth );

	if ( !g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR,"Error- Lib not initialized!");
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_RF_TUNE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	SmsMsg.msgData[0] = Frequency;		// Bandwidth
	SmsMsg.msgData[1] = Bandwidth;		// Flags
	SmsMsg.msgData[2] = g_LibMsState.Crystal;

	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteDvbtAddPidFilter_Req( UINT32 Pid )
{
	SmsMsgData_ST SmsMsg;
	SMSHOST_LOG1(SMSLOG_APIS, "PID=%d", Pid);

	if ( !g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR,"Error- Lib not initialized!");
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_ADD_PID_FILTER_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);;

	SmsMsg.msgData[0] = Pid;

	SmsLiteSendCtrlMsg( &SmsMsg );
	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteDvbtRemovePidFilter_Req( UINT32 Pid )
{
	SmsMsgData_ST SmsMsg = {0}; 
	SMSHOST_LOG1(SMSLOG_APIS, "PID=%d", Pid);

	if ( !g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR,"Error- Lib not initialized!");
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_REMOVE_PID_FILTER_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);;

	SmsMsg.msgData[0] = Pid;

	SmsLiteSendCtrlMsg( &SmsMsg );
	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteIsdbtTune_Req( UINT32 Frequency,				// Frequency in Hz 
									SMSHOSTLIB_FREQ_BANDWIDTH_ET Bandwidth,	// Bandwidth 
									UINT32 SegmentNumber)					// Segment number 0-13
{
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK ;
	UINT8 MsgBuf[sizeof(SmsMsgHdr_ST) + 4*sizeof(UINT32)] = {0};
	SmsMsgData_ST* pSmsMsg = (SmsMsgData_ST*)MsgBuf;
	SMSHOST_LOG3(SMSLOG_APIS,"Freq %d BW %d SegmentNumber %d",Frequency,Bandwidth,SegmentNumber);

	if ( !g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR,"Error- Lib not initialized!");
		return;
	}

	if (SegmentNumber > 13) // valid numbers 0-13
	{
		SMSHOST_LOG1( SMSLOG_ERROR, "Invalid SegmentNumber %d", SegmentNumber );
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_ISDBT_TUNE_RES, SMSHOSTLIB_ERR_INVALID_ARG, NULL, 0 );
		return;
	}

	if ((Bandwidth != BW_ISDBT_1SEG) && (Bandwidth != BW_ISDBT_3SEG))
	{
		SMSHOST_LOG1(SMSLOG_ERROR, "Invalid Bandwidth %d", Bandwidth );
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_ISDBT_TUNE_RES, SMSHOSTLIB_ERR_INVALID_ARG, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( pSmsMsg );
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_ISDBT_TUNE_REQ;
	pSmsMsg->xMsgHeader.msgLength = (UINT16)sizeof(MsgBuf);

	pSmsMsg->msgData[0] = Frequency;
	pSmsMsg->msgData[1] = Bandwidth;
	pSmsMsg->msgData[2] = g_LibMsState.Crystal;
	pSmsMsg->msgData[3] = SegmentNumber; // needed for ISDB-T SB (audio)

	SmsLiteSendCtrlMsg( pSmsMsg );
	SMSHOST_LOG0(SMSLOG_APIS,"END");
}


//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteMsSetAES128Key_Req( const char* KeyString )
{

	SmsMsgData_ST* pSmsMsg;
	UINT32 totalLen;

	SMSHOST_LOG1(SMSLOG_APIS, "KeyString=%s", KeyString);

	if ( !g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR,"Error- Lib not initialized!");
		return;
	}

	if (KeyString == NULL ) 
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_SET_AES128_KEY_RES, SMSHOSTLIB_ERR_INVALID_ARG, NULL, 0 );
		return;
	}
	
	totalLen = (UINT32)sizeof(SmsMsgHdr_ST) + (UINT32)strlen(KeyString);
	//pSmsMsg = (SmsMsgData_ST*)OswLiteMemAlloc(totalLen);
        pSmsMsg = (SmsMsgData_ST*)kzalloc(totalLen, GFP_KERNEL); //jan

	if( pSmsMsg == NULL)
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_SET_AES128_KEY_RES, SMSHOSTLIB_ERR_MEM_ALLOC_FAILED, NULL, 0 );
		return;
	}

	pSmsMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB;
	pSmsMsg->xMsgHeader.msgDstId = HIF_TASK;
	pSmsMsg->xMsgHeader.msgFlags = MSG_HDR_DEFAULT_DYNAMIC_MSG;
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_SET_AES128_KEY_REQ;
	pSmsMsg->xMsgHeader.msgLength = (UINT16)totalLen;

	memcpy(pSmsMsg->msgData,KeyString,strlen(KeyString));
	SmsLiteSendCtrlMsg( pSmsMsg );
	SMSHOST_LOG0(SMSLOG_APIS,"END");

}

//*******************************************************************************
// 
void SMSHOSTLITE_API SmsLiteDvbtRetrievePidFilterList_Req( void )
{
	SmsMsgData_ST SmsMsg = {0};
	SMSHOST_LOG0(SMSLOG_APIS, "START");

	if ( !g_LibMsState.IsLibInit )
	{
		SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR,"Error- Lib not initialized!");
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_GET_PID_FILTER_LIST_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);;

	SmsLiteSendCtrlMsg( &SmsMsg );
	SMSHOST_LOG0(SMSLOG_APIS,"END");
}

//*******************************************************************************
// 
void SmsLiteMsControlRxCallback(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size )
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

	SMSHOST_LOG3( SMSLOG_ERROR, "Control callback. Type %d, Retcode %#x, Payload Length %d", 
		pSmsMsg->xMsgHeader.msgType,
		RetCode,
		PayloadLength );

	switch( pSmsMsg->xMsgHeader.msgType )
	{
	case MSG_SMS_NEW_CRYSTAL_RES:
	case MSG_SMS_INIT_DEVICE_RES:
		{
			g_LibMsState.SyncFlag = TRUE;
		}
		break;
	case MSG_SMS_TRANSMISSION_IND:
		{
			// Update the DVBT statistics. No need for a response to the app.

			memcpy(	&g_LibMsState.DvbtStatsCache.TransmissionData,
				(TRANSMISSION_STATISTICS_ST*)pSmsMsg->msgData,
				sizeof( g_LibMsState.DvbtStatsCache.TransmissionData ));
			g_LibMsState.DvbtStatsCache.ReceptionData.IsDemodLocked = 0;

			//no need to correct guard interval (as opposed to old statistics message).
			CORRECT_STAT_BANDWIDTH(g_LibMsState.DvbtStatsCache.TransmissionData);
			CORRECT_STAT_TRANSMISSON_MODE(g_LibMsState.DvbtStatsCache.TransmissionData);
		}
		break;
	case MSG_SMS_HO_PER_SLICES_IND:
		{
			// Update the DVBT statistics. No need for a response to the app.
			SmsHandlePerSlicesIndication(pSmsMsg);
		}
		break;
	case MSG_SMS_SIGNAL_DETECTED_IND:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_SMS_SIGNAL_DETECTED_IND;
			RetCode = SMSHOSTLIB_ERR_OK;
		}
		break;
	case MSG_SMS_NO_SIGNAL_IND:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_SMS_NO_SIGNAL_IND;
			RetCode = SMSHOSTLIB_ERR_OK;
		}
		break;
	case MSG_SMS_ADD_PID_FILTER_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_ADD_PID_FILTER_RES;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;

			switch( RetCodeFromMsg )
			{
			case SMS_S_OK:
				RetCode = SMSHOSTLIB_ERR_OK;
				break;
			case SMS_E_ALREADY_EXISTING:
				RetCode = SMSHOSTLIB_ERR_ALREADY_EXIST;
				break;
			case SMS_E_MAX_EXCEEDED:
				RetCode = SMSHOSTLIB_ERR_LIST_FULL;
				break;
			default:
				RetCode = SMSHOSTLIB_ERR_UNDEFINED_ERR;
				break;
			}
		}
		break;
	case MSG_SMS_REMOVE_PID_FILTER_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_REMOVE_PID_FILTER_RES;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			RetCode = RetCodeFromMsg;

			if ( RetCodeFromMsg == SMS_E_NOT_FOUND )
			{
				RetCode = SMSHOSTLIB_ERR_PID_FILTER_DOES_NOT_EXIST;
			}
		}
		break;
	case MSG_SMS_GET_PID_FILTER_LIST_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_RETRIEVE_PID_FILTER_LIST_RES;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			RetCode = RetCodeFromMsg;
		}
		break;
	case MSG_SMS_RF_TUNE_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_TUNE_RES;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			RetCode = RetCodeFromMsg;
		}
		break;
	case MSG_SMS_ISDBT_TUNE_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_ISDBT_TUNE_RES;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			RetCode = RetCodeFromMsg;
		}
		break;
	case MSG_SMS_GET_STATISTICS_EX_RES:
		{
			// Statistics EX response - relevant only for ISDBT
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			ResponseMsgType = SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES;
			RetCode = RetCodeFromMsg;
		}
		break;
	case MSG_SMS_GET_STATISTICS_RES:
		{
			// Statistics EX response - relevant only for ISDBT
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			ResponseMsgType = SMSHOSTLIB_MSG_GET_STATISTICS_RES;
			RetCode = RetCodeFromMsg;
		}
		break;
	case MSG_SMS_DUMMY_STAT_RES:
		{
			// I2C Statistics response - relevant only for DVB-T
			//pPayload = pPayloadWoRetCode;
			//PayloadLength = PayloadLengthWoRetCode;
			ResponseMsgType = SMSHOSTLIB_MSG_GET_STATISTICS_RES;
			RetCode = SMSHOSTLIB_ERR_OK;
		}
		break;
	case MSG_SMS_SET_AES128_KEY_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_SET_AES128_KEY_RES;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			RetCode = RetCodeFromMsg;
		}
		break;

	default:
		SmsLiteCommonControlRxHandler( handle_num, p_buffer, buff_size );
		break;
	}

	// Call the user callback
	if ( ResponseMsgType != SMSHOSTLIB_MSG_INVALID_RESPONSE_VAL )
	{
		SmsLiteCallCtrlCallback( ResponseMsgType, RetCode, pPayload, PayloadLength );
	}
}

//*******************************************************************************
// 
void SmsLiteMsDataCallback(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size )
{
	if (g_LibMsState.pDataCallback)
	{
		g_LibMsState.pDataCallback( handle_num, p_buffer, buff_size );
	}
}

//*******************************************************************************
// Reception quality 
//

static SMSHOSTLIB_RECEPTION_QUALITY_ET SmsLiteMsCalcDvbtReceptionQuality(UINT32 SNR,
																	   INT32  InBand,
																	   UINT32 TSPackets,
																	   UINT32 ErrorTSPackets,
																	   UINT32 Constellation,
																	   UINT32 CodeRate)
{
	UINT32								Offset = 0;
	UINT32								TsPer;
	SMSHOSTLIB_RECEPTION_QUALITY_ET		SnrQuality;
	SMSHOSTLIB_RECEPTION_QUALITY_ET		InBandQuality;
	SMSHOSTLIB_RECEPTION_QUALITY_ET		TsPerQuality;
	SMSHOSTLIB_RECEPTION_QUALITY_ET		ResQuality;

	// calc offset
	// Offset is calculation of constellation (2 for QPSK, 4 and 6) X code rate X 3:
	Constellation = (Constellation + 1) * 2 * 3;
	switch(CodeRate)
	{
	case CR_1_2:
		Offset = Constellation * 1 / 2;
		break;
	case CR_2_3:
		Offset = Constellation * 2 / 3;
		break;
	case CR_3_4:
		Offset = Constellation * 3 / 4;
		break;
	case CR_5_6:
		Offset = Constellation * 5 / 6;
		break;
	case CR_7_8:
		Offset = Constellation * 7 / 8;
		break;
	}

	// calc TS PER
	if ((TSPackets + ErrorTSPackets) > 0)
	{
		TsPer = (ErrorTSPackets * 100) / (TSPackets + ErrorTSPackets);		
	}
	else
	{
		TsPer = 0;
	}

	// calc SNR Quality
	if (SNR > (13 + Offset))
	{
		SnrQuality = RECEPTION_QUALITY_EXCELLENT;
	}
	else if (SNR > (10 + Offset))
	{
		SnrQuality = RECEPTION_QUALITY_GOOD;
	}
	else if (SNR > (7 + Offset))
	{
		SnrQuality = RECEPTION_QUALITY_AVERAGE;
	}
	else
	{
		SnrQuality = RECEPTION_QUALITY_POOR;
	}

	// calc In-Band Quality
	if (InBand > (-70 + ((INT32)Offset)))
	{
		InBandQuality = RECEPTION_QUALITY_EXCELLENT;
	}
	else if (InBand > (-74 + ((INT32)Offset)))
	{
		InBandQuality = RECEPTION_QUALITY_GOOD;
	}
	else if (InBand > (-78 + ((INT32)Offset)))
	{
		InBandQuality = RECEPTION_QUALITY_AVERAGE;
	}
	else
	{
		InBandQuality = RECEPTION_QUALITY_POOR;
	}

	// calc TS PER Quality
	if (TsPer < 3)
	{
		TsPerQuality = RECEPTION_QUALITY_EXCELLENT;
	}
	else if (TsPer < 5)
	{
		TsPerQuality = RECEPTION_QUALITY_GOOD;
	}
	else if (TsPer < 7)
	{
		TsPerQuality = RECEPTION_QUALITY_AVERAGE;
	}
	else
	{
		TsPerQuality = RECEPTION_QUALITY_POOR;
	}

	// return the lowest quality from all the three
	ResQuality = SnrQuality;
	if (ResQuality > InBandQuality)
	{
		ResQuality = InBandQuality;
	}
	if (ResQuality > TsPerQuality)
	{
		ResQuality = TsPerQuality;
	}

	return ResQuality;
}

//*******************************************************************************
// 
static SMSHOSTLIB_ERR_CODES_E SmsHandlePerSlicesIndication( SmsMsgData_ST* pSmsMsg )
{
	RECEPTION_STATISTICS_ST* pReceptionData;
	UINT32 i;
	UINT32* pMsgData = (UINT32*)&pSmsMsg->msgData;
	UINT32 RecQualSum = 0;
	UINT32 Snr = pMsgData[1];
	UINT32 InBandPower = (INT32)pMsgData[2];
	UINT32 TsPackets = pMsgData[3];
	UINT32 EtsPackets = pMsgData[4];
	UINT32 Constellation = pMsgData[5];
	UINT32 HpCode = pMsgData[6];

	pReceptionData = &g_LibMsState.DvbtStatsCache.ReceptionData;

	pReceptionData->IsRfLocked			= pMsgData[16];				
	pReceptionData->IsDemodLocked		= pMsgData[17];	
	pReceptionData->ModemState			= pMsgData[12];	
	pReceptionData->SNR					= pMsgData[1];						
	pReceptionData->BER					= pMsgData[13]; 	
	pReceptionData->RSSI				= pMsgData[14];
	CORRECT_STAT_RSSI(*pReceptionData); 

	pReceptionData->InBandPwr			= (INT32)pMsgData[2];		
	pReceptionData->CarrierOffset		= (INT32)pMsgData[15];	
	pReceptionData->TotalTSPackets		= pMsgData[3];
	pReceptionData->ErrorTSPackets		= pMsgData[4];


	//TSPER
	if ((TsPackets + EtsPackets) > 0)
	{
		pReceptionData->TS_PER = (EtsPackets * 100) / 
			(TsPackets + EtsPackets);		
	}
	else
	{
		pReceptionData->TS_PER = 0;
	}

	pReceptionData->BERBitCount					= pMsgData[18];						
	pReceptionData->BERErrorCount				= pMsgData[19];

	pReceptionData->MRC_SNR						= pMsgData[20];
	pReceptionData->MRC_InBandPwr				= pMsgData[21];
	pReceptionData->MRC_RSSI					= pMsgData[22];
	CORRECT_STAT_MRC_RSSI(*pReceptionData); 

	pReceptionData->RefDevPPM					= pMsgData[23];
	pReceptionData->FreqDevHz					= pMsgData[24];

	i = g_LibMsState.ReceptionQualityCounter++ % SMS_DVBT_RECEPTION_QUALITY_HISTORY_SIZE;
	g_LibMsState.ReceptionQualityHistory[i] = SmsLiteMsCalcDvbtReceptionQuality( Snr,
		InBandPower,
		TsPackets,
		EtsPackets,
		Constellation,
		HpCode );

	for ( i = 0 ; i < SMS_DVBT_RECEPTION_QUALITY_HISTORY_SIZE ; i++ )
	{
		RecQualSum += g_LibMsState.ReceptionQualityHistory[i];
	}
	g_LibMsState.DvbtStatsCache.ReceptionQuality = RecQualSum / SMS_DVBT_RECEPTION_QUALITY_HISTORY_SIZE;


	if (g_LibMsState.DvbtStatsCache.TransmissionData.Frequency == 0)
	{
		memset(pReceptionData, 0, sizeof(*pReceptionData)); 
	}
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// Undocumented API to send a FW message
SMSHOSTLITE_API SMSHOSTLIB_ERR_CODES_E SmsLiteMsSendFwMsg( SmsMsgData_ST* pMsg )
{
	return SmsLiteSendCtrlMsg( pMsg );
}
