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

/*********************************************/
/*** Global Variables ************************/
struct DibGlobalInfo    globalInfo;
struct DibDriverContext *pContext = NULL;
CHANNEL_HDL             DibChannelHdl;
FILTER_HDL              IsdbtFilterHdl;
ELEM_HDL                IsdbtItemHdlList[4];
union  DibDemodMonit    Monit[4];	

#define ISDBT_BUFFER_SIZE 200000
unsigned char IsdbtDataBuffer[ISDBT_BUFFER_SIZE];
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
uint32_t           Frequency  = 737143;
enum DibSpectrumBW Bandwidth  = eCOFDM_BW_6_0_MHZ;
/*enum DibStreamTypes StreamType =  eSTREAM_TYPE_HOST_DMA;*/ 
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
	     for (i=0;i<1000;i++)
       {
         /*** Monitor Signal ***/
         SignalMonitor();
         usleep(500000);
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

   return DIBSTATUS_SUCCESS;
}

/**********************************************/
/********** Tune Channel **********************/
/**********************************************/
int Tune(unsigned int Frequency,  enum DibSpectrumBW Bw )
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;
   struct DibChannel  ChDesc;
   int i;
   
   /*** Get Stream 1: MPEG_TS1 ***/
   Status = DibGetStream(pContext, globalInfo.Stream[1], eSTANDARD_ISDBT, MpegTsMode | (MpegTsSize<<1), ePOWER_ON);

   if (Status != DIBSTATUS_SUCCESS)
   {
     printf("DibGetStream Failed \n");
     return -1;
   }
   else
   {
     printf("DibGetStream Successfull\n");
   }

   /** Attach frontend 0 to Stream 1  **/
   Status = DibAddFrontend(pContext, globalInfo.Stream[1], globalInfo.Frontend[0], NULL, 0);

   if (Status != DIBSTATUS_SUCCESS) 
   {
     printf("DibAddFrontend Failed \n");
     return -1;
   }
   else
   {
     printf("DibAddFrontend Successfull, Connect Frontend 0 to Stream 1\n");
   }

   /** Attach frontend 1 to Stream 1 **/
   Status = DibAddFrontend(pContext, globalInfo.Stream[1], globalInfo.Frontend[1], NULL, 0);

   if (Status != DIBSTATUS_SUCCESS) 
   {
     printf("DibAddFrontend Failed \n");
     return -1;
   }
   else
   {
     printf("DibAddFrontend Successfull, Connect Frontend 1 to Stream 1\n");
   }

   /* Initialise tune parameters */

   /** Physical Layer **/
   ChDesc.Type                                     = eSTANDARD_ISDBT;
   ChDesc.ChannelDescriptor.Bw						         = eCOFDM_BW_6_0_MHZ;

   /** Common Parameters **/
   ChDesc.ChannelDescriptor.RFkHz                  = Frequency;
   ChDesc.ChannelDescriptor.InvSpec                = eDIB_SPECTRUM_AUTO;

   /** ISDBT Parameters **/
   ChDesc.ChannelDescriptor.Std.Isdbt.Nfft         = eDIB_FFT_AUTO;
   ChDesc.ChannelDescriptor.Std.Isdbt.Guard        = eDIB_GUARD_INTERVAL_AUTO;

   /*** Partial Reception = AUTO ***/
   ChDesc.ChannelDescriptor.Std.Isdbt.PartialReception   = -1;

   /*** Disable SB mode ***/
   ChDesc.ChannelDescriptor.Std.Isdbt.SbMode            = 0;
   ChDesc.ChannelDescriptor.Std.Isdbt.SbConnTotalSeg    = -1;
   ChDesc.ChannelDescriptor.Std.Isdbt.SbWantedSeg       = -1;
   ChDesc.ChannelDescriptor.Std.Isdbt.SbSubchannel      = -1;

   /*** Initialize Layers ***/
   for (i=0;i<3;i++)
   {
     ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].NbSegments                  = -1;
     ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].Constellation               = eDIB_QAM_AUTO;
     ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].CodeRate                    = eDIB_VIT_CODERATE_AUTO;
     ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].TimeIntlv                   = -1;
   }


   /* Create, initialise a channel and tune on specified frequency */
   Status = DibGetChannelEx(pContext, globalInfo.Stream[1], &ChDesc, &DibChannelHdl, NULL, NULL);

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
  uint8_t NbDemod = 2;
  double CN[4];
  unsigned int i;
  unsigned int MpegLockA[4],MpegLockB[4],MpegLockC[4];
  double BerA[4],BerB[4],BerC[4];

  
  /*** Get Signal Monitoring ***/
  Status = DibGetSignalMonitoring(pContext, 
                                     DibChannelHdl, 
                                     &NbDemod,
                                     &Monit[0], 
                                     NULL, 
                                     NULL);

  if (Status == DIBSTATUS_SUCCESS)
  {
    for (i=0;i<2;i++)
    {
	    BerA[i] = (double) Monit[i].Isdbt.Layer[0].FecBerRs / 1e8;
	    BerB[i] = (double) Monit[i].Isdbt.Layer[1].FecBerRs / 1e8;
	    BerC[i] = (double) Monit[i].Isdbt.Layer[2].FecBerRs / 1e8;
	  
      MpegLockA[i]=DIB_GET_ISDBT_LOCK_MPEG0(Monit[i].Isdbt.Dvb.Locks);
      MpegLockB[i]=DIB_GET_ISDBT_LOCK_MPEG1(Monit[i].Isdbt.Dvb.Locks);
      MpegLockC[i]=DIB_GET_ISDBT_LOCK_MPEG2(Monit[i].Isdbt.Dvb.Locks);
  
	      if( (Monit[i].Isdbt.Dvb.SigFp == 0) || (Monit[i].Isdbt.Dvb.WgnFp==0) ) 
	    {
        CN[i] = -100;
	    } 
	    else 
	    {
        double sig_dB = 10*log10((double)Monit[i].Isdbt.Dvb.SigFp) - 320*log10((double)2);
          double wgn_dB = 10*log10((double)Monit[i].Isdbt.Dvb.WgnFp) - 320*log10((double)2);
        CN[i] = sig_dB - wgn_dB;
	    }
    }

	  printf("Signal Monitoring:\n");
    for (i=0;i<2;i++)
	  {
	    printf("Chip %d:\n", i);
      printf("C/N: %5.4lf\n",CN[i]);
	    printf("MPEG Lock Layer A : %d, BER: %f\n",MpegLockA[i], BerA[i]);
	    printf("MPEG Lock Layer B : %d, BER: %f\n",MpegLockB[i], BerB[i]);
	    printf("MPEG Lock Layer C : %d, BER: %f\n",MpegLockC[i], BerC[i]);
    }
	  
  }
  return DIBSTATUS_SUCCESS;
}

/**********************************************/
/********** Data reception using MPEG TS bus***/
/**********************************************/
int ReceiveDataWithMpegTS (void)
{
  printf("Enable MPEG TS Output \n");
}

/************************************************/
/* Data reception using filter in Host DMA mode */
/************************************************/
void IsdbtDataCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
    if (Status == eREMOVED)
    {
        return;
    }
    printf("ISDBT MPEG TS block received, Size = %d\n",pBuffCtx->BufSize);

    /*** register again the same buffer ***/
    DibRegisterBuffer(pContext, IsdbtFilterHdl, IsdbtDataBuffer, ISDBT_BUFFER_SIZE, 0);
}

int ReceiveDataWithHostDma(void)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = IsdbtDataCallback;
    info.CallbackUserCtx = NULL;

    /*** Create Filter, only for monitoring ***/
    Status = DibCreateFilter(pContext, DibChannelHdl, eTS, &info, &IsdbtFilterHdl);

    if(Status == DIBSTATUS_SUCCESS)
    {
      printf(("ISDBT Raw TS Filter Create Successfull \n"));

      /*** Register a Buffer to receive DAB frames ***/
      Status = DibRegisterBuffer(pContext, IsdbtFilterHdl, IsdbtDataBuffer, ISDBT_BUFFER_SIZE, 0);

      if(Status == DIBSTATUS_SUCCESS)
      {
          union DibFilters Filter[4];

          Filter[0].Ts.Pid = PidList[0];
          Filter[1].Ts.Pid = PidList[1];
          Filter[2].Ts.Pid = PidList[2];
          Filter[3].Ts.Pid = PidList[3];
    
          /*** Start Service, only for monitoring ***/
          Status = DibAddToFilter(pContext, IsdbtFilterHdl, 4, Filter, IsdbtItemHdlList);
  
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
            printf(("ISDBT Filter Create Failed \n"));
      }
      return Status;
}
