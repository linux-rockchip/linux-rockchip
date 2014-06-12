/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

#include "DibExtAPI.h"
#include "stdio.h"
#include "math.h"

/*********************************************
        DVBT Sample Code
**********************************************
This Example shows how to 
- Open the driver, 
- Tune a DVBT Channel
- Monitor Signal quality 

In case Stream output is MPEG TS:
- Enable MPEG TS Output
In case Stream output is HOST Dma (SPI,SDIO,SRAM):
- Create a filter and Add some PID

- Close driver
*********************************************/

/*********************************************/
/*** Global Variables ************************/
struct DibGlobalInfo    globalInfo;
struct DibDriverContext *pContext = NULL;
CHANNEL_HDL             DibChannelHdl;
FILTER_HDL              DvbtFilterHdl;
ELEM_HDL                DvbtItemHdlList[4];
union  DibDemodMonit    Monit[4];	

#define DVBT_BUFFER_SIZE 200000
unsigned char DvbtDataBuffer[DVBT_BUFFER_SIZE];
/*********************************************/

/*********************************************/
/*** Local Functions *************************/
/*********************************************/
int Open( enum DibBoardType BoardType );
int Tune(unsigned int Frequency,  enum DibSpectrumBW Bw );
int Monitor (void);
int ReceiveDataWithMpegTS (void);
int ReceiveDataWithFilter (void);

/*********************************************/
/*** Test Parameters *************************/
/*********************************************/
/*enum DibBoardType  Board       = eBOARD_NIM_1009xHx;*/
enum DibBoardType  Board      = eBOARD_NIM_10096MD4;     
  
uint32_t           Frequency   = 498000;
enum DibSpectrumBW Bandwidth   = eCOFDM_BW_8_0_MHZ;
/*enum DibStreamTypes StreamType =  eSTREAM_TYPE_HOST_DMA; */
enum DibStreamTypes StreamType =  eSTREAM_TYPE_MPEG_1;
unsigned char MpegTsMode       = 1; /* 0 => Serial, 1=> Parallel **/
unsigned char MpegTsSize       = 1; /* 0 => 188,    1=> 204 **/
      
unsigned short     PidList[4] = {0,110,120,130};
/*********************************************/

int main(int argc, char** argv)
{
  DIBSTATUS rc;
	unsigned int i;

  /*** Open Driver ***/
	rc = Open(Board);

	if (rc == DIBSTATUS_SUCCESS)
	{

    /*** Tune Channel ***/
	  rc = Tune(Frequency, Bandwidth);

    printf("Press Key to Continue\n");
    getchar();

	  if (rc == DIBSTATUS_SUCCESS)
	  {
	     for (i=0;i<100;i++)
       {
         /*** Monitor Signal ***/
         SignalMonitor();
        }
          
	  }

    printf("Press Key to Continue\n");
    getchar();

    if(StreamType == eSTREAM_TYPE_MPEG_1) 
    {
      /*** Start Data Reception in Mpeg TS mode ***/
      ReceiveDataWithMpegTS ();
    } 
    else if(StreamType == eSTREAM_TYPE_HOST_DMA) 
    {
      /*** Start Data Reception in HostDma Mode ***/
      ReceiveDataWithHostDma();
    } 

    printf("Press Key to Continue\n");
    getchar();

    /*** Close Driver ***/
	  DibClose(pContext);
	}
	
}
/**********************************************/
/********** Driver Initialization *************/
/**********************************************/
int Open( enum DibBoardType BoardType )
{

   if(DibOpen(&pContext, BoardType, 0) != DIBSTATUS_SUCCESS) 
   {
      printf("Open Driver Failed  \n");
      return DIBSTATUS_ERROR;
   }

   /*  Get driver version */
   DibGetGlobalInfo(pContext, &globalInfo);
   printf("Driver ver: %d.%d.%d\n", 
           DIB_MAJOR_VER(globalInfo.DriverVersion), 
           DIB_MINOR_VER(globalInfo.DriverVersion), 
           DIB_REVISION(globalInfo.DriverVersion));
   printf("Embedded SW ver: %d.%02d (IC: %d)\n", 
           (globalInfo.EmbVersions[0] >> 10), 
           globalInfo.EmbVersions[0] & 0x03ff, 
           globalInfo.EmbVersions[1]);
   printf("Number of demod %d \n", globalInfo.NumberOfDemods);

   return DIBSTATUS_SUCCESS;
}

/**********************************************/
/********** Tune Channel **********************/
/**********************************************/
int Tune(unsigned int Frequency,  enum DibSpectrumBW Bw )
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;
   
   /* Tuning parameters */
   struct DibChannel  ChDesc;

   /* Initialise tune parameters */
   ChDesc.Type                                    = eSTANDARD_DVB;
   ChDesc.ChannelDescriptor.Bw                    = Bw;
   ChDesc.ChannelDescriptor.RFkHz                 = Frequency;
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

   /* Create, initialise a channel and tune on specified frequency */
   Status = DibGetChannel(pContext, 
                          DIB_DEMOD_AUTO, 
                          DIB_DEMOD_AUTO,  
                          StreamType | (MpegTsMode << 16) | (MpegTsSize<<17), 
                          &ChDesc, 
                          &DibChannelHdl, 
                          NULL, 
                          NULL);

   if   ( (Status == DIBSTATUS_SUCCESS) 
	    &&  (ChDesc.ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)) 

   {
     printf("Tune Successfull\n");
	   return DIBSTATUS_SUCCESS;
   }
   else
   {
	   printf("Tune Failed \n");
	   return DIBSTATUS_ERROR;
   }
}

/**********************************************/
/********** Signal Monitoring *****************/
/**********************************************/
int SignalMonitor (void)
{
  DIBSTATUS Status = DIBSTATUS_SUCCESS;
  uint8_t NbDemod = 1;
  double Ber;
  double CN;
  unsigned int MpegLock;
  unsigned int MpegDataLock;

  
  /*** Get Signal Monitoring ***/
  Status = DibGetSignalMonitoring(pContext, 
                                     DibChannelHdl, 
                                     &NbDemod,
                                     &Monit[0], 
                                     NULL, 
                                     NULL);

  if (Status == DIBSTATUS_SUCCESS)
  {
	  Ber = (double) Monit[0].Dvb.FecBerRs / 1e8;
	  MpegLock= DIB_GET_FEC_LOCK_FRM(Monit[0].Dvb.Locks);
      MpegDataLock = DIB_GET_FEC_LOCK_MPEG(Monit[0].Dvb.Locks);				
	
	  if( (Monit[0].Dvb.SigFp == 0) || (Monit[0].Dvb.WgnFp==0) ) 
	  {
				CN = -100;
	  } 
	  else 
	  {
	 	double sig_dB = 10*log10((double)Monit[0].Dvb.SigFp) - 320*log10((double)2);
		double wgn_dB = 10*log10((double)Monit[0].Dvb.WgnFp) - 320*log10((double)2);
		CN = sig_dB - wgn_dB;
	  }
	  printf("Signal Monitoring:\n");
	  printf("BER: %5.4lf\n",Ber);
	  printf("C/N: %5.4lf\n",CN);
	  printf("Locks Sync / Mpeg: %d/%d\n",MpegLock, MpegDataLock);
	  
  }
  return DIBSTATUS_SUCCESS;
}

/**********************************************/
/********** Data reception using MPEG TS bus***/
/**********************************************/
int ReceiveDataWithMpegTS (void)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = NULL;
    info.CallbackUserCtx = NULL;

    /*** Create TS Filter ***/
    Status = DibCreateFilter(pContext, DibChannelHdl, eTS, &info, &DvbtFilterHdl);

    if(Status == DIBSTATUS_SUCCESS)
    {
      union DibFilters Filter[1];

      printf(("DVBT Raw TS Filter Create Successfull \n"));

      Filter[0].Ts.Pid = DIB_TSALLPIDS;

      /*** Add all PIDs ***/
      Status = DibAddToFilter(pContext, DvbtFilterHdl, 1, Filter, DvbtItemHdlList);

      if(Status == DIBSTATUS_SUCCESS)
      {
        printf("Add All Pids Succesfull\n");
      }
      else
      {
         printf("Add Pids Failed\n");
      }
    }
    return 0;
  
}

/************************************************/
/* Data reception using filter in Host DMA mode */
/************************************************/
void DvbtDataCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
    if (Status == eREMOVED)
    {
        return;
    }
    printf("DVBT MPEG TS block received, Size = %d\n",pBuffCtx->BufSize);

    /*** register again the same buffer ***/
    DibRegisterBuffer(pContext, DvbtFilterHdl, DvbtDataBuffer, DVBT_BUFFER_SIZE, 0);
}

int ReceiveDataWithHostDma(void)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = DvbtDataCallback;
    info.CallbackUserCtx = NULL;

    /*** Create Filter, only for monitoring ***/
    Status = DibCreateFilter(pContext, DibChannelHdl, eTS, &info, &DvbtFilterHdl);

    if(Status == DIBSTATUS_SUCCESS)
    {
      printf(("DVBT Raw TS Filter Create Successfull \n"));

      /*** Register a Buffer to receive DAB frames ***/
      Status = DibRegisterBuffer(pContext, DvbtFilterHdl, DvbtDataBuffer, DVBT_BUFFER_SIZE, 0);

      if(Status == DIBSTATUS_SUCCESS)
      {
          union DibFilters Filter[4];

          Filter[0].Ts.Pid = PidList[0];
          Filter[1].Ts.Pid = PidList[1];
          Filter[2].Ts.Pid = PidList[2];
          Filter[3].Ts.Pid = PidList[3];
    
          /*** Start Service, only for monitoring ***/
          Status = DibAddToFilter(pContext, DvbtFilterHdl, 4, Filter, DvbtItemHdlList);
  
          if(Status == DIBSTATUS_SUCCESS)
          {
            printf("Add Pids Succesfull\n");
          }
          else
          {
            printf("Add Pids Failed\n");
          }
        }
      }
      else
      {
            printf(("DVBT Filter Create Failed \n"));
      }
      return Status;
}
