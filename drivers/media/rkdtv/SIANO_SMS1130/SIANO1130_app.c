/********************************************************************************************/
//siano 1150 需要的程序
//
//
//siano 的头文件
//
//
/********************************************************************************************/
#include <linux/delay.h>

#include <SmsLitePlatDefs.h>
#include <SmsHostLibLiteMs.h>
//#include"DVB_global.h"
//#include"debug.h"//test
//#include "hw_hsadc.h"
//#include <linux/file.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "sms_common.h"

#include "../TSTV.h"

////////////////////////////////////////////////////////////////////////////////////////
//
// TS Processor function
//
// This function is called from the TS H/W driver when a block of TS packets is received
// src_len must be a multiple of 188 bytes
//

#define TS_SMS_MAX_MSG_LEN		4096
#define TS_NUM_OF_BUFFERS		4

#define MSG_HDR_FLAG_SPLIT_MSG_HDR  0x0004  
#define MSG_HDR_FLAG_TS_PAYLOAD		0x0008	


//UINT32 g_ts_remainder_bytes = 0;
extern UINT32 g_ts_payload_is_ts;
extern UINT8  g_ts_reassembly_buf[TS_NUM_OF_BUFFERS][TS_SMS_MAX_MSG_LEN];
extern UINT32 g_ts_current_reassembly_buf;
extern UINT32 g_ts_remainder_bytes;
extern UINT32 g_ts_bytes_in_msg;

//...............................+zzf
UINT16 	Default_Pid[]={0,1,2,16,17,18,19,20,22,30,31};
UINT8   PidFilterCtrlFlag=123;// the default value make the first call to execute a real func


#define	MANUALPIDFILTERBAK			1
#if MANUALPIDFILTERBAK
#define	SMS_MAX_MANUAL_PID_COUNT	64
//for pid 0 always open, pid 0 will not backup in this tbl for evere,0 in this tbl mean empty space
//pid 0x2000(filter ctrl use) and Default_Pid not backup here too.
UINT16	ManualSetPidTbl[SMS_MAX_MANUAL_PID_COUNT]={0};
UINT16	ManualPidTblEmptyOffset=0;
#endif
//...............................-zzf

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

#define ISDBT_USER_FREQ  509143000L 
const SMSHOSTLIB_FREQ_BANDWIDTH_ET ISDBT_USER_SCAN_BANDWIDTH = BW_ISDBT_1SEG; 

#define ISDBT_USER_CRISTAL  12000000L 
#define ISDBT_USER_MAX_ASYNC_TIME  5000
#define ISDBT_USER_MAX_PIDS  20
#define ISDBT_USER_STATS_OUTPUT_IVAL  1000
const char *ISDBT_USER_OUTPUT_FNAME = "out.dat"; 

typedef struct _QUALITY_CALC_ENTRY
{
	LONG	RangeEnd ;
	LONG	Grade ;
} QUALITY_CALC_ENTRY, *PQUALITY_CALC_ENTRY;

typedef struct IsdbtUserStats_S
{
	UINT32 NumBytes;
	UINT32 NumCallbacks;
	UINT32 LastOutputTime;
}
IsdbtUserStats_ST;

typedef struct IsdbtState_S
{
	Event hResponseEvent; 
	Event hTuneEvent;
	IsdbtUserStats_ST Stats; 
        void *pOutputFile;
	BOOL Is3Seg; 
	UINT32 CurFreq; 
	SMSHOSTLIB_MSG_TYPE_RES_E ExpectedResponse; 
	SMSHOSTLIB_ERR_CODES_E ErrCode;
	void *pPayload;
	UINT32 PayloadLen; 
	BOOL Signal_exist;
} IsdbtState_ST;

//////////////////////////////////////////////////////////////////////////
/// ISDB-T quality grades tables
QUALITY_CALC_ENTRY TsPerTable[] =
{
	// TS PER (units x1E-5
	//	RangeEnd				Grade
	{	/*0.00001*/1,			100	},
	{	/*0.0001*/10,			80	},
	{	/*0.001*/100,			50	},
	{	/*>0.001*/0x7FFFFFFF,	0	},
	{	-1,						-1	}
} ;

QUALITY_CALC_ENTRY PostViBerTable[] =
{
	// Post Viterbi BER (units Val*1E-5)
	//	RangeEnd	Grade
	{	50,			100	},
	{	55,			50  },
	{	60,			20	},
	{	0x7FFFFFFF,	0	},
	{	-1,			-1	}
} ;

QUALITY_CALC_ENTRY DvbtSnrTable[] =
{
	// SNR Grades for DVB-T
	//	RangeEnd	Grade
#ifdef TUNER_ISDB_T	
	{	7,			0	},
	{	9,			25	},
	{	10,			50	},
	{	11,			80	},
#else
	{	13,			0	},
	{	15,			25	},
	{	16,			50	},
	{	17,			80	},
#endif
{	0x7FFFFFFF,	100	},
	{	-1,			-1	}
} ;

QUALITY_CALC_ENTRY DvbtInBandTable[] =
{
	// In Band power grades for DVB-T
	//	RangeEnd	Grade
#ifdef TUNER_ISDB_T	
	{	-96,		0	},
	{	-92,		40	},
	{	-88,		70	},
	{   -26,		100 },
#else
	{	-90,		0	},
	{	-86,		40	},
	{	-82,		70	},
	{   -20,		100 },
#endif
	{	0x7FFFFFFF,	50	},
	{	-1,			-1	}
} ;

IsdbtState_ST g_IsdbtState = {0};
BOOL g_bHaveSignalIndicator =FALSE;
BOOL g_bDummyHaveSignal=FALSE;
UINT32 g_SignalStrength = 0;
UINT32 g_SNR = 0;
UINT32 g_BER = 0;
UINT32 g_RSSI = 0;
UINT32 g_InBandPower = 0;
char g_SignalQuality = 0;

extern int i2c_read(UINT32 response);//jan

LONG ReceptionQualityVal2Grade( PQUALITY_CALC_ENTRY Table, LONG Val )
{
	UINT32 i=0 ;
	for ( i=0 ; Table[i].RangeEnd != -1 ; i++ )
	{
		if ( Val <= Table[i].RangeEnd )
		{
			return Table[i].Grade ;
		}
	}
	return 0 ;
}

//////////////////////////////////////////////////////////////////////////////////////////

UINT32 GetDvbtReceptionQuality( SMSHOSTLIB_FAST_STATISTICS_ST* StatParams )
{
	UINT32 ReceptionQuality = 0 ;
	UINT32 Sum = 0;
	//UINT32 IsdbtMargin = 0;
	// Weight of parameters in quality calculation
	UINT32 TsPerW		= 0 ;
	UINT32 PostViBerW	= 40;//100 ;
	UINT32 SnrW			= 120;//50 ;
	UINT32 InBandPwrW	= 40;//50 ;

	UINT32 TsPerVal = 0 ;
	INT32 Grade = 0 ;
	
	if ((StatParams->TotalTSPackets + StatParams->ErrorTSPackets) != 0)
	{
		TsPerVal = (UINT32)(StatParams->ErrorTSPackets) / (StatParams->TotalTSPackets + StatParams->ErrorTSPackets) ;
	}
	Grade = ReceptionQualityVal2Grade( TsPerTable, TsPerVal);
	Sum += Grade * TsPerW ;

	Grade = ReceptionQualityVal2Grade( PostViBerTable, StatParams->BER);
	Sum += Grade * PostViBerW ;

	Grade = ReceptionQualityVal2Grade( DvbtSnrTable, StatParams->SNR);
	Sum += Grade * SnrW ;

	Grade = ReceptionQualityVal2Grade( DvbtInBandTable, StatParams->InBandPwr);
	Sum += Grade * InBandPwrW ;

	ReceptionQuality = (UINT32)(Sum/(TsPerW + PostViBerW + SnrW + InBandPwrW)) ;

	return (UINT32)ReceptionQuality;
}

UINT32 GetReceptionQuality( SMSHOSTLIB_STATISTICS_ISDBT_ST* StatParams )
{
	UINT32 ReceptionQuality = 0 ;
	UINT32 Sum = 0;
	//UINT32 IsdbtMargin = 0;
	// Weight of parameters in quality calculation
	UINT32 TsPerW		= 0 ;
	UINT32 PostViBerW	= 100 ;
	UINT32 SnrW			= 50 ;
	UINT32 InBandPwrW	= 50 ;

	UINT32 TsPerVal = 0 ;
	INT32 Grade = 0 ;
	
	if ((StatParams->LayerInfo[0].TotalTSPackets + StatParams->LayerInfo[0].ErrorTSPackets) != 0)
	{
		TsPerVal = (UINT32)(StatParams->LayerInfo[0].ErrorTSPackets) / (StatParams->LayerInfo[0].TotalTSPackets + StatParams->LayerInfo[0].ErrorTSPackets) ;
	}
	Grade = ReceptionQualityVal2Grade( TsPerTable, TsPerVal);
	Sum += Grade * TsPerW ;

	Grade = ReceptionQualityVal2Grade( PostViBerTable, StatParams->LayerInfo[0].BER);
	Sum += Grade * PostViBerW ;

	Grade = ReceptionQualityVal2Grade( DvbtSnrTable, StatParams->SNR);
	Sum += Grade * SnrW ;

	Grade = ReceptionQualityVal2Grade( DvbtInBandTable, StatParams->InBandPwr);
	Sum += Grade * InBandPwrW ;

	ReceptionQuality = (UINT32)(Sum/(TsPerW + PostViBerW + SnrW + InBandPwrW)) ;

	return (UINT32)ReceptionQuality;
}

// Upper-layer function prototype
extern void handle_completed_msg(SmsMsgData_ST* pMsg);

void ts_data_parser(UINT8* src_buf, UINT32 src_len,BOOL bInitial)
{
	UINT8   siano_preamble[8] = {0x53, 0x69, 0x61, 0x6E, 0x6F, 0x2D, 0x4D, 0x53}; 
	UINT8*  current_ptr;
	//UINT8*	payload_ptr;
	UINT32  bytes_to_copy;
	UINT16  ts_pid;
	SmsMsgData_ST* pMsg;
	UINT32  isSplit;

	current_ptr = src_buf;

	while (current_ptr < src_buf + src_len)
	{
		//check for beginning of SMS message
		ts_pid = (current_ptr[1] << 8) + current_ptr[2];
		g_ts_bytes_in_msg = 0;
		if ((ts_pid == 0x1FFF) && (!memcmp(&current_ptr[4], siano_preamble, 8)))
		{
			pMsg = (SmsMsgData_ST*)(void*)&current_ptr[12];
			g_ts_remainder_bytes = pMsg->xMsgHeader.msgLength;

			isSplit				= (pMsg->xMsgHeader.msgFlags & MSG_HDR_FLAG_SPLIT_MSG_HDR);
			g_ts_payload_is_ts  = (pMsg->xMsgHeader.msgFlags & MSG_HDR_FLAG_TS_PAYLOAD);

			if (isSplit)
			{
				bytes_to_copy = sizeof(SmsMsgHdr_ST);
			}
			else
			{
				bytes_to_copy = SMS_MIN(176, g_ts_remainder_bytes);
			}

			memcpy(&g_ts_reassembly_buf[g_ts_current_reassembly_buf][0], pMsg, bytes_to_copy);
			g_ts_remainder_bytes -= bytes_to_copy;
			g_ts_bytes_in_msg = bytes_to_copy;
		}
		if ((g_ts_bytes_in_msg > 0) && (g_ts_remainder_bytes == 0))
		{
			pMsg = (SmsMsgData_ST*)(void*)g_ts_reassembly_buf[g_ts_current_reassembly_buf];
			handle_completed_msg(pMsg);
			g_ts_bytes_in_msg = 0;
			g_ts_current_reassembly_buf++;
			if (g_ts_current_reassembly_buf >= TS_NUM_OF_BUFFERS) 
			{
				g_ts_current_reassembly_buf = 0;
			}
		}
		if ((g_bHaveSignalIndicator ==TRUE) || (g_bDummyHaveSignal == TRUE))
			return ;

		current_ptr += 188;		// Advance to next PID packet
	}
}

void siano_ts_data_processor(UINT8* src_buf, UINT32 src_len)
{
	UINT8   siano_preamble[8] = {0x53, 0x69, 0x61, 0x6E, 0x6F, 0x2D, 0x4D, 0x53}; 
	UINT8*  current_ptr;
	UINT8*	payload_ptr;
	UINT32  bytes_to_copy;
	UINT16  ts_pid;
	SmsMsgData_ST* pMsg;
	UINT32  isSplit;

	current_ptr = src_buf;

	while (current_ptr < src_buf + src_len)
	{
		if (g_ts_remainder_bytes > 0)
		{
			if (g_ts_payload_is_ts)
			{
				// TS payload, keep header
				bytes_to_copy = SMS_MIN(188, g_ts_remainder_bytes);
				payload_ptr = current_ptr;
			}
			else
			{
				// Non-TS payload, strip header
				bytes_to_copy = SMS_MIN(184, g_ts_remainder_bytes);
				payload_ptr = current_ptr+4;
			}
			memcpy(&g_ts_reassembly_buf[g_ts_current_reassembly_buf][g_ts_bytes_in_msg], payload_ptr, bytes_to_copy);
			g_ts_bytes_in_msg += bytes_to_copy;
			g_ts_remainder_bytes -= bytes_to_copy;
		}
		else
		{
			//check for beginning of SMS message
			ts_pid = (current_ptr[1] << 8) + current_ptr[2];
			g_ts_bytes_in_msg = 0;
			if ((ts_pid == 0x1FFF) && (!memcmp(&current_ptr[4], siano_preamble, 8)))
			{
				pMsg = (SmsMsgData_ST*)(void*)&current_ptr[12];
				g_ts_remainder_bytes = pMsg->xMsgHeader.msgLength;

				isSplit				= (pMsg->xMsgHeader.msgFlags & MSG_HDR_FLAG_SPLIT_MSG_HDR);
				g_ts_payload_is_ts  = (pMsg->xMsgHeader.msgFlags & MSG_HDR_FLAG_TS_PAYLOAD);

				if (isSplit)
				{
					bytes_to_copy = sizeof(SmsMsgHdr_ST);
				}
				else
				{
					bytes_to_copy = SMS_MIN(176, g_ts_remainder_bytes);
				}

				memcpy(&g_ts_reassembly_buf[g_ts_current_reassembly_buf][0], pMsg, bytes_to_copy);
				g_ts_remainder_bytes -= bytes_to_copy;
				g_ts_bytes_in_msg = bytes_to_copy;
			}
		}

		if ((g_ts_bytes_in_msg > 0) && (g_ts_remainder_bytes == 0))
		{
			pMsg = (SmsMsgData_ST*)(void*)g_ts_reassembly_buf[g_ts_current_reassembly_buf];
			handle_completed_msg(pMsg);
			g_ts_bytes_in_msg = 0;
			g_ts_current_reassembly_buf++;
			if (g_ts_current_reassembly_buf >= TS_NUM_OF_BUFFERS) 
			{
				g_ts_current_reassembly_buf = 0;
			}
		}
		if ((g_bHaveSignalIndicator ==TRUE)|| (g_bDummyHaveSignal == TRUE) )
			return ;

		current_ptr += 188;		// Advance to next PID packet
	}
}

#if MANUALPIDFILTERBAK
void ManualPIDBak(uint16_t pid, uint8_t onoff)	
{
	UINT16 i,j;
	printk("TSTV:SIANO: [sms]ManualPIDBak pid:%d onoff:%d ManualPidTblEmptyOffset:%d\n",pid,onoff,ManualPidTblEmptyOffset);	
	if (SMS_MAX_MANUAL_PID_COUNT==ManualPidTblEmptyOffset){
		//ASSERT(FALSE); //jan@2010-07-24
	}
	if (1==onoff){//add open pid to filter
		for (i=0;i<ManualPidTblEmptyOffset;i++){
			if (ManualSetPidTbl[i]==pid){
				return;// pid already open in filter
			}
		}
		//add a new open pid in filter
		ManualSetPidTbl[ManualPidTblEmptyOffset]=pid;
		ManualPidTblEmptyOffset++;
	} else {//remove open pid from filter
		if (0==ManualPidTblEmptyOffset){
			return;// ManualSetPidTbl is empty, and not open pid to remove
		} else {
			for (i=0;i<ManualPidTblEmptyOffset;i++){
				if (ManualSetPidTbl[i]==pid){
					if (i==(ManualPidTblEmptyOffset-1)){//remove tail one pid
						ManualSetPidTbl[i]=0;						
					} else {
						for (j=i;j<(ManualPidTblEmptyOffset-1);j++){
							ManualSetPidTbl[j]=ManualSetPidTbl[j+1]; //puss tail part pid adhead a seat
						}
						ManualSetPidTbl[ManualPidTblEmptyOffset-1]=0;
					}
					ManualPidTblEmptyOffset--;
					return;
				}
			}
		}
	}
}
BOOL SIANO1130_Module_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff);
void ManualPIDClearAll()
{
	//UINT16 i;
	while (0 != ManualPidTblEmptyOffset){
		SIANO1130_Module_pid_filter(0,ManualSetPidTbl[ManualPidTblEmptyOffset-1],0);
	}
}
#endif

//*******************************************************************************
// Control callback function. This function is being given to SMS11xx host library 
//as a callback for control events.
//
void IsdbtUserCtrlCallback( SMSHOSTLIB_MSG_TYPE_RES_E	MsgType,		
									SMSHOSTLIB_ERR_CODES_E		ErrCode,		
									void* 						pPayload,		
									UINT32						PayloadLen)

{
	//Handle indications.
	switch(MsgType)
	{
	case SMSHOSTLIB_MSG_GET_VERSION_RES:
	        printk("TSTV:SIANO: [sms]Version:%s \n",(char*)pPayload);
		return;
	case SMSHOSTLIB_MSG_SMS_SIGNAL_DETECTED_IND:
		g_bHaveSignalIndicator =TRUE;
		g_IsdbtState.Signal_exist = TRUE;
//		OSW_EventSet(&g_IsdbtState.hTuneEvent); 
		printk("TSTV:SIANO: SMSHOSTLIB_MSG_SMS_SIGNAL_DETECTED_IND !!!\n");
		return; 

	case SMSHOSTLIB_MSG_SMS_NO_SIGNAL_IND:
		g_bHaveSignalIndicator =TRUE;
		g_IsdbtState.Signal_exist = FALSE;
//		OSW_EventSet(&g_IsdbtState.hTuneEvent); 
		printk("TSTV:SIANO: SMSHOSTLIB_MSG_SMS_NO_SIGNAL_IND !!!\n");
		return; 
	case SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES:
		{
			SMSHOSTLIB_STATISTICS_ISDBT_ST* pStat = (SMSHOSTLIB_STATISTICS_ISDBT_ST*)pPayload;
			
			g_SignalQuality = (char)GetReceptionQuality(pStat);
			g_SignalStrength = pStat->InBandPwr;

			g_SNR = pStat->SNR;
            		g_BER = pStat->LayerInfo[0].BER;
           	 	g_RSSI = pStat->RSSI;
			g_InBandPower = pStat->InBandPwr;

                        printk("TSTV:SIANO: g_SignalQuality==%d \n",g_SignalQuality);
			printk("TSTV:SIANO: g_SignalStrength==%d \n",g_SignalStrength);
			printk("TSTV:SIANO: g_SNR==%d \n",g_SNR);
			printk("TSTV:SIANO: g_BER==%d \n",g_BER);
			printk("TSTV:SIANO: g_RSSI==%d \n",g_RSSI);
		}
		return ;

	case SMSHOSTLIB_MSG_GET_STATISTICS_RES:
		{
			SMSHOSTLIB_FAST_STATISTICS_ST* pStat = (SMSHOSTLIB_FAST_STATISTICS_ST*)pPayload;

			if (TRUE==pStat->DemodLocked){
				g_SignalQuality = (char)GetDvbtReceptionQuality(pStat);
			} else {
				g_SignalQuality = 0;
			}
			g_SignalStrength = (char)pStat->InBandPwr;
			g_SNR = pStat->SNR;
			g_BER = pStat->BER;
			g_InBandPower=pStat->InBandPwr;
			printk("TSTV:SIANO: [SMS]Lock=%d, InBandPwr=%d, SNR=%d, BER=%d ValidTsPak=%d, ErrTsPak=%d\n",pStat->DemodLocked,pStat->InBandPwr, pStat->SNR, pStat->BER,pStat->TotalTSPackets,pStat->ErrorTSPackets);

            if (pStat->DemodLocked == TRUE )
			 {      g_bDummyHaveSignal = TRUE;
                    g_IsdbtState.Signal_exist = TRUE;
              }
		}
		return ;

	default:
		break;
	}



	if(MsgType != g_IsdbtState.ExpectedResponse)
	{
		return; 
	}

	//
	//Handle expected response arrival.
	//

	//Copy error code.
	g_IsdbtState.ErrCode = ErrCode; 

	//Copy payload length.
	g_IsdbtState.PayloadLen = PayloadLen; 

	//Copy payload. 
	if(g_IsdbtState.PayloadLen)
	{
	//	if((g_IsdbtState.pPayload = OswLiteMemAlloc(g_IsdbtState.PayloadLen)) == NULL)
	//	{
	//		printf("Allocation failed\n");
	//		exit(1);
	//	}

	//	memcpy(g_IsdbtState.pPayload, pPayload, g_IsdbtState.PayloadLen); 
	}

	//Set response event.
//	OSW_EventSet(&g_IsdbtState.hResponseEvent); 

}



//*******************************************************************************
// Data callback function. This function is being given to SMS11xx host library 
//as a callback for data.
static void IsdbtUserDataCallback(UINT32						ServiceDevHandle,
								 UINT8*						pBuf, 
								 UINT32						BufSize)
{
	//UINT32 Now; 


	g_IsdbtState.Stats.NumBytes += BufSize; 
	g_IsdbtState.Stats.NumCallbacks++; 



}

//*******************************************************************************
// Implementation of the sequence of an example sequence of operations required 
// to get service data from a device. 
static int IsdbtUserRunDevice( void )
{

	//SMSHOSTLIB_STATISTICS_ISDBT_ST* pStats;
	//UINT32 i;

	//char IgnoredInput; 

	//
	//Get version. 
	//Indicates F/W liveness, and may help with version compliance issues.
	//

        SmsLiteGetVersion_Req();


	
	return 1; 


}


//*******************************************************************************
//Main function
//
int IsdbtUserInit()
{

	SMSHOSTLIBLITE_MS_INITLIB_PARAMS_ST InitLibParams = {0}; 

	//SMSHOSTLIB_ERR_CODES_E Error; 
	int Res; 


	//
	//Initialize SMS11xx host library. 
	//
	//printk("[sms] IsdbtUserInit start =%d",OSTimeGet());//jan
	InitLibParams.Size = sizeof(InitLibParams);
	InitLibParams.pCtrlCallback = (SmsHostLiteCbFunc)IsdbtUserCtrlCallback; 
	InitLibParams.pDataCallback = IsdbtUserDataCallback; 
	InitLibParams.Crystal = ISDBT_USER_CRISTAL;
#ifdef TUNER_ISDB_T		
	InitLibParams.DeviceMode = SMSHOSTLIB_DEVMD_ISDBT; 
#else
	InitLibParams.DeviceMode = SMSHOSTLIB_DEVMD_DVBT; 
#endif
	Res =SmsLiteMsLibInit(&InitLibParams);
	if( Res !=SMSHOSTLIB_ERR_OK)
        {
                printk("TSTV:SIANO: Sms Init Failed, result %d\t%s[%d]\n", Res, __FUNCTION__, __LINE__);
		return Res;
        }
        DBG("TSTV:SIANO: Sms Init Success, result %d\t%s[%d]\n", Res, __FUNCTION__, __LINE__);

	// Clear Init device response
	i2c_read(MSG_SMS_INIT_DEVICE_RES);	
	
	//printf("\nBye Bye\n");
	//printk("[sms] IsdbtUserInit end =%d",); //jan
	return Res;


}


char SIANO1130_Module_Init()
{
	if(IsdbtUserInit() !=SMSHOSTLIB_ERR_OK)
		return 0;

	return 1;
}

void SIANO1130_Module_Tune(unsigned int frequency_KHz, unsigned int  bandwidth_KHz)
{
	static UINT32 ilen ;
	UINT32  i;
	static BOOL bGet =FALSE;
	bGet =FALSE;
	g_bHaveSignalIndicator =FALSE;
	g_bDummyHaveSignal =FALSE;
	int bandwidth = bandwidth_KHz/1000;
	
	//frequency =frequency*1000;
	//printk("[sms] DVB_Module_Tune:%d start =%d",frequency,OSTimeGet());//jan
        printk("TSTV:SIANO: -------------------------->the frequency we tune %d<---------------------------\n", frequency);
#ifdef TUNER_ISDB_T	
	SmsLiteIsdbtTune_Req(frequency_KHz*1000,BW_ISDBT_1SEG,0);
#else
	if(bandwidth ==6)
		bandwidth =BW_6_MHZ;
	else if(bandwidth ==7)
		bandwidth =BW_7_MHZ;
	else
		bandwidth =BW_8_MHZ;
	SmsLiteDvbtTune_Req(frequency,bandwidth);
#endif
	
     for ( i =0 ; i < 10 ; i ++)
     	{
     		//OSW_TaskSleep(100);
                msleep(100); //jan
                //mdelay(100); //jan

#ifdef TUNER_ISDB_T	
		if (i2c_read(MSG_SMS_ISDBT_TUNE_RES) == 0xffff)	
#else			
		if (i2c_read(MSG_SMS_RF_TUNE_RES) == 0xffff)
#endif
		{
			printk("TSTV:SIANO: DVB response !!!\n");
			bGet =TRUE;
			break;
		}
		//else
		//SmsLiteIsdbtTune_Req(frequency,BW_ISDBT_1SEG,0);			
     	}
	if(i==10){
		//printk("[sms] DVB_Module_Tune:tune time out ",OSTimeGet());//jan
		return ;//TEST
	}
	for ( i =0; i < 15; i++)
	{
		//OSW_TaskSleep(200);
                msleep(200); //jan
                //mdelay(200); //jan
		ilen= i2c_read(0xffff);
		if(ilen>0)
			bGet =FALSE;
                printk("TSTV:SIANO: g_bHaveSignalIndicator %d, g_bDummyHaveSignal %d\t%s[%d]\n", g_bHaveSignalIndicator, g_bDummyHaveSignal, __FUNCTION__, __LINE__);
		if((g_bHaveSignalIndicator  ==TRUE) || (g_bDummyHaveSignal== TRUE))
		{
                        printk("TSTV:SIANO: g_bHaveSignalIndicator %d, g_bDummyHaveSignal %d, i %d\t%s[%d]\n", g_bHaveSignalIndicator, g_bDummyHaveSignal, i, __FUNCTION__, __LINE__);
			printk("TSTV:SIANO: [sms] Module_Tune Success !!!\n");
			break;
		}
	}
        if(i == 15)
        {
            printk("TSTV:SIANO: tune failed\n");
        }
        else
        {
            printk("TSTV:SIANO: ##############################################TUNE OK###############################################\n");
        }
	//printk("[sms] DVB_Module_Tune:end =%d",OSTimeGet());//jan
	/*
       if (g_bHaveSignalIndicator == FALSE)
       {

       }

	*/
}


void SIANO1130_Module_Deinit()
{
	SmsLiteMsLibTerminate();
}


char SIANO1130_Module_Lockstatus()
{
        DBG("TSTV:SIANO: to lock frequency xxx.xxx.000\t%s[%d]\n", __FUNCTION__, __LINE__);
	if ((g_bHaveSignalIndicator ==TRUE) || (g_bDummyHaveSignal==TRUE))
	{
                DBG("TSTV:SIANO: lock status g_IsdbtState.Signal_exist %d\t%s[%d]\n", g_IsdbtState.Signal_exist, __FUNCTION__, __LINE__);
		if(g_IsdbtState.Signal_exist == TRUE)
			return 1;
		else
			return 0;
	}
	return 0;
}

void SIANO1130_Module_signal_strenth_quality( char *signal_quality, char *signal_strength)
{
	static UINT32  i;
#ifdef TUNER_ISDB_T		
	SmsLiteMsGetStatistics_Req();
#endif	
     	for ( i =0 ; i < 10 ; i ++)
     	{

#ifdef TUNER_ISDB_T	
		if (i2c_read(MSG_SMS_GET_STATISTICS_EX_RES) == 0xffff)
#else			
		if (i2c_read(MSG_SMS_DUMMY_STAT_RES) == 0xffff)
#endif
		{
			break;
		}
     		//OSW_TaskSleep(10);
                msleep(10); //jan
                //mdelay(10); //jan
    	}
	if(i==10)
	{
		*signal_quality = 0;
		*signal_strength=0;
		return ;//TEST
	}
	
	*signal_quality = g_SignalQuality;
	*signal_strength=g_SignalStrength;

}



BOOL SIANO1130_Module_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
#ifdef TUNER_DVB_T		
	UINT32  i,tt;
	tt=sizeof(Default_Pid)/sizeof(UINT16);
	for (i=0;i<tt;i++){// for siano tuner, the default pid is already set when chip start
		if (Default_Pid[i]==pid){
			return;
		}
	}
	TR("[sms] start, onoff=%d, pid=%d, timer=%d",onoff, pid, OSTimeGet());
#if MANUALPIDFILTERBAK
	if (0x2000!=pid){
		ManualPIDBak(pid,onoff);
	}
#endif
	if (onoff == 1)
		SmsLiteDvbtAddPidFilter_Req( pid );
	else
		SmsLiteDvbtRemovePidFilter_Req( pid);

     	for ( i =0 ; i < 10 ; i ++)
     	{
     		OSW_TaskSleep(10);

		if (onoff == 1)
		{
			if (i2c_read(MSG_SMS_ADD_PID_FILTER_RES) == 0xffff)
			{
				DBG("TSTV:SIANO: [sms] MSG_SMS_ADD_PID_FILTER_RES\n");	
				break;
			}
		}
		else
		{
			if (i2c_read(MSG_SMS_REMOVE_PID_FILTER_RES) == 0xffff)
			{
				DBG("TSTV:SIANO: [sms] MSG_SMS_REMOVE_PID_FILTER_RES\n");
				break;
			}
		}
    	}
	if (i>9){
		TR("[sms] DVB_demod_pid_filter timeout  %d",OSTimeGet());	
	}
	TR("[sms] end timer=%d",OSTimeGet());
#endif	
}

BOOL  SIANO1130_Module_pid_filter_ctrl(uint8_t onoff)
{
#ifdef TUNER_DVB_T
	if (onoff!=PidFilterCtrlFlag){
		PidFilterCtrlFlag=onoff;
		return SIANO1130_Module_pid_filter(0, 0x2000, (onoff == 1 ? 0 : 1));
	} else 
		return;
#endif
}
	
int SIANO1130_Module_i2c_addr(void)
{
    return 0xd0; // 0x68 << 1;
}

unsigned short SIANO1130_Module_i2c_speed(void)
{
   return 500;//240;//150; // you can increase the i2c speed to make sure it cost less time when download firmware. 
}

char SIANO1130_Module_NULL(void)
{
    return 1;
}

/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "SIANO1130_CONTROL",
    SIANO1130_Module_Init,
    SIANO1130_Module_Deinit,
    SIANO1130_Module_Tune,
    SIANO1130_Module_Lockstatus,
    SIANO1130_Module_pid_filter,
    SIANO1130_Module_pid_filter_ctrl,
    SIANO1130_Module_signal_strenth_quality,
    SIANO1130_Module_i2c_addr,
    SIANO1130_Module_i2c_speed,
    SIANO1130_Module_NULL
};


