//
//
/********************************************************************************************/
#include <TSTV.h>
#include "User/Common/DibStatus.h"
#include "User/Common/DibExtDefines.h"
#include "User/Common/DibDefines.h"
#include "User/Common/DibExtAPI.h"
#include "Driver/Common/DibDriverCtx.h"
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include "DibBCasMsg.h"

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

typedef struct
{
      uint32_t   bUsed;
      uint32_t iPid;
}DVBPIDFilterInfo;

#define MASTER_DIB1009XH_I2C_ADDRESS 0x80
#define MAX_NUM_OF_HARDWARE_PID_FILTER DIB_MAX_NB_FILTERS

/* Define TS partial Stream */
#define NB_ELEM MAX_NUM_OF_HARDWARE_PID_FILTER 

DVBPIDFilterInfo g_iPidFilter[MAX_NUM_OF_HARDWARE_PID_FILTER];
enum DibBoardType  Board       = eBOARD_NIM_1009xHx;
struct DibDriverContext *pContext = NULL;
struct DibGlobalInfo    globalInfo;
CHANNEL_HDL             DibChannelHdl = DIB_UNSET;
union  DibDemodMonit    Monit[1];
enum DibStreamTypes StreamType =  eSTREAM_TYPE_MPEG_1;
unsigned char MpegTsMode       = 1; /* 0 => Serial, 1=> Parallel **/
unsigned char MpegTsSize       = 0; /* 0 => 188,    1=> 204 **/
FILTER_HDL              gDtvFilterHdl = DIB_UNSET;
union DibFilters gServ[1] ;
ELEM_HDL ElemHdl[NB_ELEM] = {DIB_UNSET};

#define TUNER_DVB_T		0		/*DVB-T/ DVB-H*/	
#define TUNER_ISDBT_T 	1		/*ISDB-T*/


#if CONFIG_DIBCOM1009XH_DVB_CONTROL
unsigned char gDVBStandardInfo =  TUNER_DVB_T;
#endif

#if CONFIG_DIBCOM1009XH_ISDB_ONESEG_CONTROL 
unsigned char gDVBStandardInfo =  TUNER_ISDBT_T;
#endif

#if CONFIG_DIBCOM1009XH_ISDB_FULLSEG_CONTROL
unsigned char gDVBStandardInfo =  TUNER_ISDBT_T;
#endif

unsigned char MsgBuf[256];

/********************************************************************************************/
//
//
//    OVER
//
//
/********************************************************************************************/


/********************************************************************************************/
//
//dibcom 1009XH 需要的程序
//
//
/********************************************************************************************/
/*********************************************/

void DibBCasDisplay(void * pCallbackCtx, uint16_t MsgType, uint16_t MsgIndex, uint8_t * pMsgBuf, uint32_t MsgLen)
{
   switch(MsgIndex)
   {
      case BCAS_CARDTYPE:       
        DBG( "[BCAS] Card Type 1: %02x\n" , pMsgBuf[0]); 
        break;
      case BCAS_CARDID:  
      {
        unsigned char i;
        unsigned char M;

        printk( "[BCAS] Card Type 2: %c%d\n",pMsgBuf[1],pMsgBuf[2] );

        printk( "[BCAS] Card Id: " );
        for (i=1;i<11;i++)
          printk( "%02x " , pMsgBuf[i]);
        printk( "\n");

        M = pMsgBuf[0];
        if (M == 1)
          printk( "[BCAS] Group Id: Not Set\n" );
        else
          printk( "[BCAS] Group Id: %d\n",M );
      }
        break;
	case BCAS_GROUPID:	   printk( "[BCAS] Group ID: %d\n"	, pMsgBuf[0]); 
	  break;
	case BCAS_PRESENT:
	  printk("[BCAS] BCAS card present\n");
	  break;
	case BCAS_NOTPRESENT:
	  printk("[BCAS] BCAS card not present\n");
	  break;
	default:			   printk( "[BCAS] RECEIVED UNKNOWN MSG %d\n" , MsgIndex); 
	  break;
   };
}

/**********************************************/
/********** Driver Initialization *************/
/**********************************************/
static int Open(enum DibBoardType BoardType)
{
	DBG("TSTV:DIBCOM!1009XH: %s[%d]\n", __FUNCTION__, __LINE__);
	#if 0
	while(1){
		DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
		if(spi_write(gTSTVModuleSpi, "a", sizeof("a")) < 0)
		{
			DBG("spi_write failed: \t%s[%d]\n", __FUNCTION__, __LINE__);	
			break;
		}
		msleep(1);
	}
			if(pContext != NULL)
	{
		DBG("Open already!\n");
		return DIBSTATUS_SUCCESS;
	}

	#endif

	
	if(DibOpen(&pContext, BoardType, 0) != DIBSTATUS_SUCCESS)
	{
		DBG("TSTV:DIBCOM!1009XH: Open Driver Failed %s[%d]\n", __FUNCTION__, __LINE__);
		return DIBSTATUS_ERROR;
	}
	
	/*Get driver version*/
	DibGetGlobalInfo(pContext, &globalInfo);
	DBG("Driver ver: %d.%d.%d\t",
			DIB_MAJOR_VER(globalInfo.DriverVersion),
			DIB_MINOR_VER(globalInfo.DriverVersion),
			DIB_REVISION(globalInfo.DriverVersion));
	DBG("Embedded SW ver: %d.%02d (IC: %d)\t",
			(globalInfo.EmbVersions[0] >> 10),
			globalInfo.EmbVersions[0] & 0x03ff,
			globalInfo.EmbVersions[1]);
	DBG("TSTV:DIBCOM!1009XH: Number of demod %d \t%s[%d]\n\n", globalInfo.NumberOfDemods,__FUNCTION__, __LINE__);
	DBG("TSTV:DIBCOM!1009XH: NumberOfStreams: %d\n", globalInfo.NumberOfStreams);
	return DIBSTATUS_SUCCESS;
}


static int Tune_DVB(unsigned int Frequency,  enum DibSpectrumBW Bw )
{
	DBG("TSTV:DIBCOM1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	DIBSTATUS Status = DIBSTATUS_SUCCESS;  
    /* Tuning parameters */
    struct DibChannel  ChDesc;

#if 0
	/** Get the stream in DVBT mode**/
  	/Status = DibGetStream(pContext, globalInfo.Stream[1], eSTANDARD_DVB, MpegTsMode | (MpegTsSize << 1), ePOWER_ON);
	if(Status != DIBSTATUS_SUCCESS)
	{
		DBG("TSTV:DIBCOM1009XH: DibGetStream failed \t%s[%d]\n", __FUNCTION__, __LINE__);
	}
   /** Attach a frontend  **/
	
    Status = DibAddFrontend(pContext, globalInfo.Stream[1], globalInfo.Frontend[0], NULL, 0);
   if(Status != DIBSTATUS_SUCCESS)
	{
		DBG("TSTV:DIBCOM1009XH: DibAddFrontend failed \t%s[%d]\n", __FUNCTION__, __LINE__);
	}
#endif
   /* Initialise tune parameters */
    ChDesc.ChannelDescriptor.Bw                    = Bw;
    ChDesc.ChannelDescriptor.RFkHz                 = Frequency;
	ChDesc.Type                                    = eSTANDARD_DVB;
	ChDesc.ChannelDescriptor.InvSpec				        = eDIB_SPECTRUM_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.Nfft		      = eDIB_FFT_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.Guard		      = eDIB_GUARD_INTERVAL_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.Nqam		      = eDIB_QAM_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.IntlvNative	  = eDIB_INTLV_NATIVE_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.VitHrch		    = eDIB_VIT_HRCH_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.VitSelectHp  	= eDIB_VIT_PRIORITY_HP;
	ChDesc.ChannelDescriptor.Std.Dvb.VitAlpha		  = eDIB_VIT_ALPHA_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.VitCodeRateHp	= eDIB_VIT_CODERATE_AUTO;
	ChDesc.ChannelDescriptor.Std.Dvb.VitCodeRateLp	= eDIB_VIT_CODERATE_AUTO;

   // Status = DibGetChannelEx(pContext, globalInfo.Stream[1], &ChDesc,  &DibChannelHdl, NULL, NULL);
   /* Create, initialise a channel and tune on specified frequency */
   Status = DibGetChannel(pContext, 
						  DIB_DEMOD_AUTO, 
						  DIB_DEMOD_AUTO,
						  StreamType | (MpegTsMode << 16) | (MpegTsSize<<17),
						  &ChDesc, 
						  &DibChannelHdl, 
						  NULL, 
						  NULL);
	if	( (Status == DIBSTATUS_SUCCESS) 
			&&  (ChDesc.ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)) 

	{
		DBG("TSTV:DIBCOM1009XH: Tune Successfull \t%s[%d]\n", __FUNCTION__, __LINE__);
		//return DIBSTATUS_SUCCESS;
	}
	else
	{
		DBG("TSTV:DIBCOM1009XH: Tune failed failed\t%s[%d]\n", __FUNCTION__, __LINE__);
		//return DIBSTATUS_ERROR;
	} 

	return DIBSTATUS_SUCCESS;
}

static int Tune_ISDB(unsigned int Frequency,  enum DibSpectrumBW Bw )
{
	DBG("TSTV:DIBCOM1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	DIBSTATUS Status = DIBSTATUS_SUCCESS;  
	/* Tuning parameters */
	struct DibChannel  ChDesc;
	int i;

	#if 0
	/*** Get Stream 1: MPEG_TS1 ***/
   	Status = DibGetStream(pContext, globalInfo.Stream[1], eSTANDARD_ISDBT_1SEG, MpegTsMode | (MpegTsSize<<1), ePOWER_ON);
	if(Status != DIBSTATUS_SUCCESS)
	{
		DBG("TSTV:DIBCOM1009XH: DibGetStream failed \t%s[%d]\n", __FUNCTION__, __LINE__);
	}

	/** Attach frontend 0 to Stream 1  **/
    Status = DibAddFrontend(pContext, globalInfo.Stream[1], globalInfo.Frontend[0], NULL, 0);
	if(Status != DIBSTATUS_SUCCESS)
	{
		DBG("TSTV:DIBCOM1009XH: DibAddFrontend failed\t%s[%d]\n", __FUNCTION__, __LINE__);
	}
	#endif
	
	/* Initialise tune parameters */
	ChDesc.ChannelDescriptor.Bw 				   = Bw;
	ChDesc.ChannelDescriptor.RFkHz				   = Frequency;
	
	/** Physical Layer **/
	ChDesc.Type 									= eSTANDARD_ISDBT;

	/** Common Parameters **/
	ChDesc.ChannelDescriptor.InvSpec				= eDIB_SPECTRUM_AUTO;

	/** ISDBT Parameters **/
	ChDesc.ChannelDescriptor.Std.Isdbt.Nfft 		= eDIB_FFT_AUTO;
	ChDesc.ChannelDescriptor.Std.Isdbt.Guard		= eDIB_GUARD_INTERVAL_AUTO;

	/*** Partial Reception = AUTO ***/
	ChDesc.ChannelDescriptor.Std.Isdbt.PartialReception   = -1;
	
	/*** Disable SB mode ***/
	ChDesc.ChannelDescriptor.Std.Isdbt.SbMode			 = 0;
	ChDesc.ChannelDescriptor.Std.Isdbt.SbConnTotalSeg	 = -1;
	ChDesc.ChannelDescriptor.Std.Isdbt.SbWantedSeg		 = -1;
	ChDesc.ChannelDescriptor.Std.Isdbt.SbSubchannel 	 = -1 ;
	
	/*** Initialize Layers ***/
	for (i=0;i<3;i++)
	{
	ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].NbSegments					= -1;
	ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].Constellation				= eDIB_QAM_AUTO;
	ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].CodeRate					= eDIB_VIT_CODERATE_AUTO;
	ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].TimeIntlv					= -1;
	}
	
   /* Create, initialise a channel and tune on specified frequency */
   Status = DibGetChannel(pContext, 
						  DIB_DEMOD_AUTO, 
						  DIB_DEMOD_AUTO,
						  StreamType | (MpegTsMode << 16) | (MpegTsSize<<17),
						  &ChDesc, 
						  &DibChannelHdl, 
						  NULL, 
						  NULL);
   
  // Status = DibGetChannelEx(pContext, globalInfo.Stream[1], &ChDesc,	&DibChannelHdl, NULL, NULL);
  
	if	( (Status == DIBSTATUS_SUCCESS) 
			&&	(ChDesc.ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)) 

	{
		DBG("TSTV:DIBCOM1009XH: Tune Successfull \t%s[%d]\n", __FUNCTION__, __LINE__);
		//return DIBSTATUS_SUCCESS;
	}
	else
	{
		DBG("TSTV:DIBCOM1009XH: Tune failed\t%s[%d]\n", __FUNCTION__, __LINE__);
		//return DIBSTATUS_ERROR;
	}  

return DIBSTATUS_SUCCESS;	
}

static long ComputeRfPower(uint16_t agc_global, uint8_t LnaStatus, uint8_t Lock)
{
    if (Lock)
    {
        return ((long)-73*(long)agc_global/(long)65535 - (long)26);
    }
    else
    {
        return -999;
   	}
}

char DVB_Module_Init(void)
{
	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	DIBSTATUS rc = DIBSTATUS_SUCCESS;
	unsigned int MsgBufLen = sizeof(MsgBuf);
	union DibParamConfig ParamConfig;

	/*** Open Driver ***/	
	rc = Open(Board);

	#if CONFIG_DIBCOM1009XH_ISDB_BCAS
	 /* Register a callback for BCAS messages */
	//rc = DibRegisterMessageCallback(pContext, MSG_API_TYPE_BCAS, MsgBuf, MsgBufLen, DibBCasDisplay, NULL);
	memset(MsgBuf, 0, MsgBufLen);
	rc = DibRegisterMessageCallback(pContext, MSG_API_TYPE_BCAS, MsgBuf, MsgBufLen, NULL, NULL);
	if(rc == DIBSTATUS_SUCCESS)
	   DBG("BCAS interface registered\n");
	else
	   DBG("Failed to register BCAS interface\n");
	
	/* Enable CAS system */
	DBG("Enable CAS \n");
	ParamConfig.CasEnable.Enable = eDIB_TRUE;
	rc = DibSetConfig(pContext, eCAS_ENABLE, &ParamConfig);
	if(rc == DIBSTATUS_SUCCESS)
	   DBG("BCAS enabled\n");
	else
	   DBG("Failed to enable BCAS\n");
	#endif

	if(rc == DIBSTATUS_SUCCESS)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*frequency单位Hz, bandwidth单位MHz*/
void DVB_Module_Tune(unsigned int frequency_KHz, unsigned int  bandwidth_KHz)
{
	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	
	int rf_khz = frequency_KHz;
	int rf_bandwidth = (bandwidth_KHz/1000) * 10;
	uint8_t Idx = 0;
	
	DBG("DVB_Module_Tune: Freq = %dKHz, Bandwidth = %dKHz \n",rf_khz, bandwidth_KHz);

	#if 1	
if(pContext)
{
	/*Remove Created Items*/
	for(Idx = 0; Idx < NB_ELEM; Idx++)
	{
		if(ElemHdl[Idx] != DIB_UNSET)
		{
			DibRemoveFromFilter(pContext,ElemHdl[Idx]);
			ElemHdl[Idx] = DIB_UNSET;
		}
	}

	/*Delete Filter*/
	if(gDtvFilterHdl != DIB_UNSET)
	{
		DibDeleteFilter(pContext,gDtvFilterHdl);
		gDtvFilterHdl = DIB_UNSET;
	}
	
	/*Delete Channel*/
	if(DibChannelHdl != DIB_UNSET){
		DibDeleteChannel(pContext,DibChannelHdl);
		DibChannelHdl = DIB_UNSET;
	}
}
    #endif
	
	switch(gDVBStandardInfo)
	{
		case TUNER_DVB_T:
		//rf_khz = 506000;
		//rf_bandwidth = eCOFDM_BW_8_0_MHZ;
			Tune_DVB(rf_khz, rf_bandwidth);
			break;
			
		case TUNER_ISDBT_T:
		//rf_khz = 473143;
		//rf_bandwidth = eCOFDM_BW_6_0_MHZ;
			Tune_ISDB(rf_khz, rf_bandwidth);
			break;
			
		default:
			DBG("TSTV:DIBCOM!1009XH: DVB Standard Error\t%s[%d]\n", __FUNCTION__, __LINE__);
			break;
			
	}
}


void DVB_Module_Deinit(void)
{
	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	uint8_t Idx = 0;

	if(pContext)
	{
		/*Remove Created Items*/
		for(Idx = 0; Idx < NB_ELEM; Idx++)
		{
			if(ElemHdl[Idx] != DIB_UNSET)
			{
				DibRemoveFromFilter(pContext,ElemHdl[Idx]);
				ElemHdl[Idx] = DIB_UNSET;
			}
		}

		/*Delete Filter*/
		if(gDtvFilterHdl != DIB_UNSET)
		{
			DibDeleteFilter(pContext,gDtvFilterHdl);
			gDtvFilterHdl = DIB_UNSET;
		}
		
		/*Delete Channel*/
		if(DibChannelHdl != DIB_UNSET){
			DibDeleteChannel(pContext,DibChannelHdl);
			DibChannelHdl = DIB_UNSET;
		}
		/***Close Driver***/
		DibClose(pContext);
		pContext = NULL;
	}
	
}


char DVB_Module_Lockstatus(void)
{
	//DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
    uint16_t bLock;
	uint8_t iLock;
	uint8_t NbDemod = 1;
	DIBSTATUS Status = DIBSTATUS_SUCCESS;
	unsigned int MpegLockA,MpegLockB,MpegLockC;
	static unsigned int count = 0;

 	memset(&Monit, 0, sizeof(union DibDemodMonit)*NbDemod);

       if(gDVBStandardInfo == TUNER_ISDBT_T)
       {
	    msleep(500);
       }

	Status = DibGetSignalMonitoring(pContext,
										DibChannelHdl,                                     
										&NbDemod,                                     
										&Monit[0],                                     
										NULL,                                      
										NULL);
	if(Status != DIBSTATUS_SUCCESS)
	{
		//DBG("TSTV:DIBCOM1009XH:DibGetSignalMonitoring failed \t%s[%d]\n", __FUNCTION__, __LINE__);
		//return 0;
	}
	
	#if 0
	 /*** Get Signal Monitoring ***/
	Status = DibGetSignalMonitoring(pContext,
									DibChannelHdl,                                     
									&NbDemod,                                     
									&Monit[0],                                     
									NULL,                                      
									NULL); 
	 #endif

	switch (Status) 
   {
   		case DIBSTATUS_TIMEOUT:
      		DBG("TSTV:DIBCOM!1009XH: Time Out \t%s[%d]\n", __FUNCTION__, __LINE__);
      		break;
   		case DIBSTATUS_SUCCESS:
			memcpy(&(Monit[0]), Monit, sizeof(union DibDemodMonit)*NbDemod);
			if(gDVBStandardInfo == TUNER_DVB_T)
			{
				bLock = Monit[0].Dvb.Locks;
				iLock = DIB_GET_FEC_LOCK_MPEG(bLock);
				DBG("TSTV:DIBCOM!1009XH:**iLock = %d**\t%s[%d]\n",iLock,__FUNCTION__, __LINE__);
				return iLock;
			}
			else if(gDVBStandardInfo == TUNER_ISDBT_T)
			{
				bLock = Monit[0].Isdbt.Dvb.Locks;
				MpegLockA=DIB_GET_ISDBT_LOCK_MPEG0(bLock);
			    MpegLockB=DIB_GET_ISDBT_LOCK_MPEG1(bLock);
			    MpegLockC=DIB_GET_ISDBT_LOCK_MPEG2(bLock);

				DBG("TSTV:DIBCOM!1009XH: MPEG Lock Layer A = %d \t%s[%d]\n",MpegLockA, __FUNCTION__, __LINE__);
			    DBG("TSTV:DIBCOM!1009XH: MPEG Lock Layer B = %d \t%s[%d]\n",MpegLockB, __FUNCTION__, __LINE__);
			    DBG("TSTV:DIBCOM!1009XH: MPEG Lock Layer C = %d \t%s[%d]\n",MpegLockC, __FUNCTION__, __LINE__);
				return (MpegLockA | MpegLockB | MpegLockC);
				
			}
			else
			{
				DBG("TSTV:DIBCOM!1009XH: DVB Standard Error\t%s[%d]\n", __FUNCTION__, __LINE__);
			}
			break;
		default:
		case DIBSTATUS_ERROR:
			DBG("TSTV:DIBCOM!1009XH: DibGetSignalMonitoring Error\t%s[%d]\n", __FUNCTION__, __LINE__);
			break;
   }
	return 0;	
}

void DVB_Module_signal_strenth_quality(unsigned char *signal_quality, unsigned char *signal_strength)
{
	//DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	
  	DIBSTATUS Status = DIBSTATUS_SUCCESS;
	uint8_t NbDemod = 1;
	uint8_t MpegDataLock;
	int SQ, SS;
	
#if 1
	*signal_quality = 100;
	*signal_strength = 100;
	
	return;
#endif	
	
	*signal_quality =0;
	*signal_strength =0;
  	MpegDataLock = 1;
	Status = DibGetSignalMonitoring(pContext,
										DibChannelHdl,                                     
										&NbDemod,                                     
										&Monit[0],                                     
										NULL,                                      
										NULL);
	if(Status != DIBSTATUS_SUCCESS)
	{
		DBG("TSTV:DIBCOM!1009XH: DibGetSignalMonitoring Error\t%s[%d]\n", __FUNCTION__, __LINE__);
		*signal_quality  = 100;
	       *signal_strength = 100;
		return;
	}
	
	if(gDVBStandardInfo == TUNER_DVB_T)
	{
		SQ = 100 - Monit[0].Dvb.SignalQuality;
	
		SS = 100 + ComputeRfPower(Monit[0].Dvb.AgcGlobal,Monit[0].Dvb.LnaStatus,MpegDataLock);

	}
	else if(gDVBStandardInfo == TUNER_ISDBT_T)
	{
		SQ = 100 - Monit[0].Isdbt.Dvb.SignalQuality;

		SS = 100 + ComputeRfPower(Monit[0].Isdbt.Dvb.AgcGlobal,Monit[0].Isdbt.Dvb.LnaStatus,MpegDataLock);
		SQ = 10;
		SS = 10;

	}
	else
	{
		DBG("TSTV:DIBCOM!1009XH: DVB Standard Error\t%s[%d]\n", __FUNCTION__, __LINE__);
		return;
	}

	*signal_quality  =SQ*10;
	*signal_strength =SS*10;

	DBG("TSTV:DIBCOM!1009XH:  AgcPower = %d ------ \t%s[%8d]\n", Monit[0].Dvb.AgcPower, __FUNCTION__, __LINE__);
	
	//DBG("TSTV:DIBCOM!1009XH: quality = %d strength = %d \t%s[%5d]\n", SQ, SS, __FUNCTION__, __LINE__);
	return;
}

//! PID filter control
/**
 * Enables or disabled the internal PID filter of a demod. If it is disabled,
 * the complete MPEG2 Transport is put out otherwise only selected PIDs are
 * passed through (see demod_pid_filter).
 *
 * \param onoff 0 = disable PID filter (default), 1 = enable
 */
int  DVB_demod_pid_filter_ctrl(uint8_t onoff)
{	
	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	if(onoff ==1 )
	{
		 DIBSTATUS Status = DIBSTATUS_SUCCESS;
		struct DibDataBuffer TsInfo;
		/* Create a data path for DVB-T TS on the allocated channel */
		TsInfo.Timeout      = DIB_TIMEOUT_IMMEDIATE;		
		TsInfo.DataMode     = eIP;	/* User CallBack Mode*/
		TsInfo.CallbackFunc = NULL;	/* TS Callback **/
		TsInfo.CallbackUserCtx = NULL;	/* Pass Driver Context to get it in the Callback */

	    if(gDtvFilterHdl == DIB_UNSET)
		{
			Status = DibCreateFilter(pContext, DibChannelHdl, eTS, &TsInfo, &gDtvFilterHdl);
			if(Status != DIBSTATUS_SUCCESS)
			{
				DBG("TSTV:DIBCOM!1009XH: Create filter failed \t%s[%d]\n", __FUNCTION__, __LINE__);
			}
			else
			{
				DBG("TSTV:DIBCOM!1009XH: Create filter Successfull \t%s[%d]\n", __FUNCTION__, __LINE__);
			}
	    }		
	}
	else
	{
		uint32_t i;
		for(i=0;i<MAX_NUM_OF_HARDWARE_PID_FILTER;i++)
		{
			if(g_iPidFilter[i].bUsed ==1)
			{
				g_iPidFilter[i].bUsed =0;
				g_iPidFilter[i].iPid  =0x1fff;
			}
		}	
		if(gDtvFilterHdl != DIB_UNSET)
		{
			DibDeleteFilter(pContext,gDtvFilterHdl);
			gDtvFilterHdl = DIB_UNSET;
		}
	}
	return 1;
}

//! PID filter table
/**
 * Enable a PID to be filtered. Disable the consideration of a position of the internal table.
 *
 * \param id          index of the internal table
 * \param pid        PID to be filtered
 * \param onoff     activate de-activate the given table index
 */
int DVB_demod_pid_filter(uint8_t id, uint16_t pid, uint8_t onoff)
{
	/* Add Pids to the partial TS stream */
	/* Can be done in one shot or pid per pid */

	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
	unsigned int iPidCount =0;
	DIBSTATUS  Status;
		
	if(onoff )
	{
		uint32_t i;
		struct DibDataBuffer TsInfo;
		/* Create a data path for DVB-T TS on the allocated channel */
		TsInfo.Timeout      = DIB_TIMEOUT_IMMEDIATE;
		TsInfo.DataMode     = eIP;	/* User CallBack Mode*/
		TsInfo.CallbackFunc = NULL;	/* TS Callback **/
		TsInfo.CallbackUserCtx = NULL;	/* Pass Driver Context to get it in the Callback */

	    if(gDtvFilterHdl == DIB_UNSET)
		{
			Status = DibCreateFilter(pContext, DibChannelHdl, eTS, &TsInfo, &gDtvFilterHdl);
			if(Status != DIBSTATUS_SUCCESS)
			{
				DBG("TSTV:DIBCOM!1009XH: Create filter failed \t%s[%d]\n", __FUNCTION__, __LINE__);
			}
			else
			{
				DBG("TSTV:DIBCOM!1009XH: Create filter Successfull \t%s[%d]\n", __FUNCTION__, __LINE__);
			}
	    }
		
		for(i=0;i<MAX_NUM_OF_HARDWARE_PID_FILTER;i++)
		{
			if(g_iPidFilter[i].bUsed ==0)
			{
				g_iPidFilter[i].bUsed =1;
				g_iPidFilter[i].iPid =pid;
				break;
			}
		}
		
		for(i=0;i<MAX_NUM_OF_HARDWARE_PID_FILTER;i++)
		{
			if(g_iPidFilter[i].bUsed ==1)
			{
				iPidCount ++;
			}
		}
		if(iPidCount == MAX_NUM_OF_HARDWARE_PID_FILTER)
		{
		  goto end;
		}
	
		gServ[0].Ts.Pid =pid;
		
		Status = DibAddToFilter(pContext,  DibChannelHdl, 1, gServ, &ElemHdl[iPidCount]);
		if(Status != DIBSTATUS_SUCCESS )
		{
			DBG("TSTV:DIBCOM!1009XH: DibAddToFilter failed \t%s[%d]\n", __FUNCTION__, __LINE__);
		}
		else
		{
			DBG("TSTV:DIBCOM!1009XH: DibAddToFilter Successfull \t%s[%d]\n", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		uint32_t i;
		for(i=0;i<MAX_NUM_OF_HARDWARE_PID_FILTER;i++)
		{
			if((g_iPidFilter[i].bUsed ==1)&&(g_iPidFilter[i].iPid ==pid))
			{
				g_iPidFilter[i].bUsed =0;
				g_iPidFilter[i].iPid  =0x1fff;
				break;
			}
		}
	}
end:
	return 1;
}

int DVB_Module_i2c_addr(void)
{
	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
    return MASTER_DIB1009XH_I2C_ADDRESS;
}

unsigned short DVB_Module_i2c_speed(void)
{
	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
    return 120;//80;//100;//500;
}

char DVB_Module_NULL(void)
{
	DBG("TSTV:DIBCOM!1009XH: \t%s[%d]\n", __FUNCTION__, __LINE__);
    return 1;
}

/*tstv func*/
struct TSTV_Module_t gTSTVModule = {
    "DIBCOM1009XH_CONTROL",
    DVB_Module_Init,
    DVB_Module_Deinit,
    DVB_Module_Tune,
    DVB_Module_Lockstatus,
    DVB_demod_pid_filter,
    DVB_demod_pid_filter_ctrl,
    DVB_Module_signal_strenth_quality,
    DVB_Module_i2c_addr,
    DVB_Module_i2c_speed,
    DVB_Module_NULL
};

/********************************************************************************************/
//
//
//    OVER
//
//
/********************************************************************************************/

