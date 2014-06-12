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
#include "string.h"
#include "math.h"

/*********************************************
        CTTB Multiple Chip Sample Code
**********************************************
This Example shows how to
- Open the driver,
- Display Board layout: Stream and Frontends for information
- Configure 3 streams using 3 different Frontends
- Tune the 3 streams on different frequencies
- Monitor Signal quality of each Stream
- Close driver
*********************************************/

/*********************************************/
/*** Global Variables ************************/
struct DibDriverContext *pContext = NULL;
struct DibGlobalInfo    globalInfo;
CHANNEL_HDL             DibChannelHdl[3];
FILTER_HDL              CttbFilterHdl[3];
ELEM_HDL                CttbItemHdlList[4];
union  DibDemodMonit    Monit[4];
/*********************************************/

/*********************************************/
/*** Local Functions *************************/
/*********************************************/
int      Open( enum DibBoardType BoardType );
int      Tune(unsigned char streamid, unsigned int Frequency, enum DibSpectrumBW Bw );
int      SignalMonitor (unsigned char streamid);
void     DisplayStandards(uint32_t Standards);
uint32_t GetFrontendNumByHandle(FRONTEND_HDL FeHdl);
int      ReceiveDataWithMpegTS (unsigned char streamid);

/*********************************************/
/*** Test Parameters *************************/
/*********************************************/
/*enum DibBoardType  Board       = eBOARD_NIM_1009xHx;*/
enum DibBoardType  Board      = eBOARD_NIM_30092MD4;     
uint32_t           FrequencyStream1  = 474000;
uint32_t           FrequencyStream2  = 498000;
uint32_t           FrequencyStream3  = 586000;
enum DibSpectrumBW Bandwidth         = eCOFDM_BW_8_0_MHZ;
unsigned char MpegTsMode             = 1; /* 0 => Serial, 1=> Parallel **/
unsigned char MpegTsSize             = 1; /* 0 => 188,    1=> 204 **/
/*********************************************/

/*********************************************/
/*** MAIN ************************************/
/*********************************************/
int main(int argc, char** argv)
{
  DIBSTATUS rc;
	unsigned int i;

  DibChannelHdl[0] = DIB_UNSET;
  DibChannelHdl[1] = DIB_UNSET;
  DibChannelHdl[2] = DIB_UNSET;
  DibChannelHdl[3] = DIB_UNSET;

  /*** Open Driver ***/
	rc = Open(Board);

  /*** Display Board Layout ***/
	DisplayLayout();

	if (rc == DIBSTATUS_SUCCESS)
	{

    /*** Tune Channel Stream 1, Chip 0, MPEG Serial ***/
	  rc = Tune(1, FrequencyStream1, Bandwidth);
    printf("Press Key to Continue\n");
    getchar();

    /*** Tune Channel Stream 2, Chip 1, MPEG Serial ***/
	  rc = Tune(2, FrequencyStream2, Bandwidth);
    printf("Press Key to Continue\n");
    getchar();

    /*** Tune Channel Stream 3, Chip 2, MPEG Serial ***/
	  rc = Tune(3, FrequencyStream3, Bandwidth);
    printf("Press Key to Continue\n");
    getchar();

	  for (i=0;i<10;i++)
    {
       /*** Monitor Signal Stream 1 ***/
       SignalMonitor(1);
    }
	  for (i=0;i<10;i++)
    {
      /*** Monitor Signal Stream 2 ***/
      SignalMonitor(2);
    }
	  for (i=0;i<10;i++)
    {
      /*** Monitor Signal Stream 3 ***/
       SignalMonitor(3);
    }

    printf("Press Key to Enable MPEG TS\n");
    /*** Enable MPEG TS on Stream 1 ***/
    ReceiveDataWithMpegTS (1);
    /*** Enable MPEG TS on Stream 2 ***/
    ReceiveDataWithMpegTS (2);
    /*** Enable MPEG TS on Stream 3 ***/
    ReceiveDataWithMpegTS (3);

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

   return DIBSTATUS_SUCCESS;
}


/**********************************************/
/********** Display Board Layout  *************/
/**********************************************/
int	DisplayLayout(void)
{
   uint32_t FeNum, StreamNum, i;
   struct DibStreamAttributes   StreamAttr;
   struct DibFrontendAttributes FrontendAttr;
   const char * StreamTypeString[]= {"MPEG_1", "MPEG_2", "HOST_DMA"};

   /*** Get Global Information ***/
   DibGetGlobalInfo(pContext, &globalInfo);

   printf("-------------------------------\n");
   printf("Driver ver: %d.%d.%d\n",
           DIB_MAJOR_VER(globalInfo.DriverVersion),
           DIB_MINOR_VER(globalInfo.DriverVersion),
           DIB_REVISION(globalInfo.DriverVersion));
   printf("Embedded SW ver: %d.%02d (IC: %d)\n",
           (globalInfo.EmbVersions[0] >> 10),
           globalInfo.EmbVersions[0] & 0x03ff,
           globalInfo.EmbVersions[1]);

   printf("     Layout Desccription: \n");
   printf("-------------------------------\n");
   printf("NumberOfChips     = %d\n", globalInfo.NumberOfChips);
   printf("NumberOfFrontends = %d\n", globalInfo.NumberOfFrontends);
   printf("NumberOfStreams   = %d\n", globalInfo.NumberOfStreams);


   /*** Get and Display Stream Attributes ***/

   printf("      Stream attributes:\n");
   printf("-------------------------------\n");

   for(StreamNum = 0; StreamNum < globalInfo.NumberOfStreams; StreamNum++)
   {
      DibGetStreamAttributes(pContext, globalInfo.Stream[StreamNum], &StreamAttr);

      printf("Stream Number: %3d Type: %10s ChipId: %3d Standard: ",
            StreamNum,
            StreamTypeString[StreamAttr.Type],
            StreamAttr.ChipId);

      DisplayStandards(StreamAttr.Standards);
      printf("\n");
      i = 0;
      while(StreamAttr.PossibleDirectInputs[i].Fe)
      {
         printf("  --> Input FeNum %d Stds = ",
               GetFrontendNumByHandle(StreamAttr.PossibleDirectInputs[i].Fe));
         DisplayStandards(StreamAttr.PossibleDirectInputs[i].Standard);
         printf("\n");
         i++;
      }
   }

   /*** Get and Display Frontend Attributes ***/

   printf("-------------------------------\n");
   printf("     Frontend attributes:\n");
   printf("-------------------------------\n");
   for(FeNum = 0; FeNum < globalInfo.NumberOfFrontends; FeNum++)
   {
      DibGetFrontendAttributes(pContext, globalInfo.Frontend[FeNum], &FrontendAttr);

      printf("Frontend Number: %3d ChipId: %3d FeId: %3d" CRA,
            FeNum,
            FrontendAttr.ChipId, FrontendAttr.FeId);
      printf(" Std = ");
      DisplayStandards(FrontendAttr.Standards);
      printf("\n");

      i = 0;
      while(FrontendAttr.PossibleDirectInputs[i].Fe)
      {
         printf("  --> MRC Input: FeNum: %d Stds = ",
               GetFrontendNumByHandle(FrontendAttr.PossibleDirectInputs[i].Fe));
         DisplayStandards(FrontendAttr.PossibleDirectInputs[i].Standard);
         printf("\n");
         i++;
      }
   }
   printf("-------------------------------\n");
}
/**********************************************/
/********** Tune Channel **********************/
/**********************************************/
int Tune(unsigned char streamid, unsigned int Frequency, enum DibSpectrumBW Bw )
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   /* Tuning parameters */
   struct DibChannel  ChDesc;

   unsigned char FrontendId;


   /*** Master Host DMA Stream, using frontend 0 ***/
   if (streamid == 0)
   {
     FrontendId = 0;
   }
   /*** Master MPEG Stream, using frontend 0 ***/
   else if (streamid == 1)
   {
     FrontendId = 0;
   }
   /*** Slave 1 MPEG Stream, using frontend 1 ***/
   else if (streamid == 2)
   {
     FrontendId = 1;
   }
   /*** Slave 2 MPEG Stream, using frontend 2 ***/
   else if (streamid == 3)
   {
     FrontendId = 2;
   }

   /** Get the stream in DVBT mode**/
   Status = DibGetStream(pContext, globalInfo.Stream[streamid], eSTANDARD_CTTB, MpegTsMode | (MpegTsSize << 1), ePOWER_ON);

   if (Status != DIBSTATUS_SUCCESS)
   {
     printf("DibGetStream Failed \n");
     return -1;
   }
   else
   {
     printf("DibGetStream Successfull, Stream: %d\n", streamid);
   }

   /** Attach a frontend  **/
   Status = DibAddFrontend(pContext, globalInfo.Stream[streamid], globalInfo.Frontend[FrontendId], NULL, 0);

   if (Status != DIBSTATUS_SUCCESS)
   {
     printf("DibAddFrontend Failed \n");
     return -1;
   }
   else
   {
     printf("DibAddFrontend Successfull, Connect Frontend %d to Stream %d\n",FrontendId,streamid);
   }

   /* Initialise tune parameters */
   memset(&ChDesc, -1, sizeof(ChDesc));
   ChDesc.Type                                    = eSTANDARD_CTTB;
   ChDesc.ChannelDescriptor.Bw                    = Bw;
   ChDesc.ChannelDescriptor.RFkHz                 = Frequency;
   ChDesc.ChannelDescriptor.InvSpec		          = eDIB_SPECTRUM_AUTO;

   /* Tune Channel */
   Status = DibGetChannelEx(pContext, globalInfo.Stream[streamid], &ChDesc, &DibChannelHdl[streamid], NULL, NULL);

   if   ( (Status == DIBSTATUS_SUCCESS)
	    &&  (ChDesc.ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS))

   {
     printf("Tune Successfull CTTB on %d KHz\n", Frequency);
	   return DIBSTATUS_SUCCESS;
   }
   else
   {
	   printf("Tune Failed CTTB on %d KHz\n",Frequency);
	   return DIBSTATUS_ERROR;
   }
}

/**********************************************/
/********** Signal Monitoring *****************/
/**********************************************/
int SignalMonitor (unsigned char streamid)
{
  DIBSTATUS Status = DIBSTATUS_SUCCESS;
  uint8_t NbDemod = 1;
  double Per;
  double CN;
  unsigned int CoffLock;


  /*** Get Signal Monitoring ***********************/
  /*** Use Channel handle to identify the stream ***/
  Status = DibGetSignalMonitoring(pContext,
                                     DibChannelHdl[streamid],
                                     &NbDemod,
                                     &Monit[0],
                                     NULL,
                                     NULL);
  if (Status == DIBSTATUS_SUCCESS)
  {
	  Per = (double) Monit[0].Cttb.Per / 1e8;
	  CoffLock= DIB_GET_COFF_LOCK(Monit[0].Cttb.Dvb.Locks);

	  if( (Monit[0].Cttb.Dvb.SigFp == 0) || (Monit[0].Cttb.Dvb.WgnFp==0) )
	  {
				CN = -100;
	  }
	  else
	  {
	 	double sig_dB = 10*log10((double)Monit[0].Cttb.Dvb.SigFp) - 320*log10((double)2);
		double wgn_dB = 10*log10((double)Monit[0].Cttb.Dvb.WgnFp) - 320*log10((double)2);
		CN = sig_dB - wgn_dB;
	  }
	  printf("Signal Monitoring of Stream %d:\n",streamid);
	  printf("PER: %5.4lf\n",Per);
	  printf("C/N: %5.4lf\n",CN);
	  printf("Signal Lock : %d\n",CoffLock);

  }
  return DIBSTATUS_SUCCESS;
}


/**********************************************/
/********** Data reception using MPEG TS bus***/
/**********************************************/
int ReceiveDataWithMpegTS (unsigned char streamid)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = NULL;
    info.CallbackUserCtx = NULL;

    /*** Create TS Filter ***/
    Status = DibCreateFilter(pContext, DibChannelHdl[streamid], eTS, &info, &CttbFilterHdl[streamid]);

    if(Status == DIBSTATUS_SUCCESS)
    {
      union DibFilters Filter[1];

      printf("CTTB Raw TS Filter Create Successfull on Stream %d \n",streamid);

      Filter[0].Ts.Pid = DIB_TSALLPIDS;

      /*** Add all PIDs ***/
      Status = DibAddToFilter(pContext, CttbFilterHdl[streamid], 1, Filter, CttbItemHdlList);

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



void DisplayStandards(uint32_t Standards)
{
  const char * StdString[eMAX_STANDARD_DEMOD]= {"UNK", "DVBT", "ISDBT", "DAB", "AUDIO", "ANALOG", "ATSC", "DVBSH", "FM", "CMMB", "ISDBT_1seg"};
   uint32_t Std = 1;
   for(Std = 1; Std < eMAX_STANDARD_DEMOD; Std++)
   {
      if((1 << Std) & Standards) printf("%s ", StdString[Std]);
   }
}

uint32_t GetFrontendNumByHandle(FRONTEND_HDL FeHdl)
{
   uint32_t FeNum;
   for(FeNum = 0; FeNum < globalInfo.NumberOfFrontends; FeNum++)
   {
      if(FeHdl == globalInfo.Frontend[FeNum])
         break;
   }
   return FeNum;
}
