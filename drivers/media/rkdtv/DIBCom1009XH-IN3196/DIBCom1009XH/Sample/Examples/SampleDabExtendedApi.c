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
        DVBT Multiple Chip Sample Code
**********************************************
This Example shows how to 
- Open the driver, 
- Display Board layout: Stream and Frontends for information
- Configure 3 streams using 3 different Frontends
- Tune the 3 streams on different frequencies 
- Monitor Signal quality of each Stream 
- Close driver
*********************************************/

struct CbCtx_t
{
  unsigned char streamid;
};
/*********************************************/
/*** Global Variables ************************/
struct DibDriverContext *pContext = NULL;
struct DibGlobalInfo    globalInfo;
CHANNEL_HDL             DibChannelHdl[3];
FILTER_HDL              DabFilterHdl[3];
ELEM_HDL                DabItemHdl[3];
union  DibDemodMonit    Monit[4];	

#define DAB_BUFFER_SIZE 200000
unsigned char DabDataBuffer[3][DAB_BUFFER_SIZE];
struct CbCtx_t DataCtx[3];
/*********************************************/


/*********************************************/
/*** Local Functions *************************/
/*********************************************/
int      Open( enum DibBoardType BoardType );
int      Tune(unsigned char streamid, unsigned int Frequency, enum DibSpectrumBW Bw );
int      SignalMonitor (unsigned char streamid);
int      AddService(unsigned char streamid, unsigned short SubChId);
void     DisplayStandards(uint32_t Standards);
uint32_t GetFrontendNumByHandle(FRONTEND_HDL FeHdl);
unsigned char GetChIdFromStrId(unsigned char streamid);

/*********************************************/
/*** Test Parameters *************************/
/*********************************************/
/*enum DibBoardType  Board       = eBOARD_NIM_1009xHx;*/
enum DibBoardType  Board      = eBOARD_NIM_10096MD4;     
uint32_t           FrequencyStream1  = 220352;
uint32_t           FrequencyStream2  = 220352;
uint32_t           FrequencyStream3  = 220352;
enum DibSpectrumBW Bandwidth     = eCOFDM_BW_1_5_MHZ;
enum  DibDataType  DabMode       = eTDMB; /* eDAB or eTDMB */
unsigned char      DabAudioMode  = 0;    /* 0= DAB, 1= DAB+ */
unsigned short     DabSubChannel = 1;
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

    /*** Tune Channel Stream 0, Chip 0, HOST DMA ***/
	  rc = Tune(0, FrequencyStream1, Bandwidth);
    printf("Press Key to Continue\n");
    getchar();

    /*** Tune Channel Stream 4, Chip 0, HOST DMA ***/
	  rc = Tune(4, FrequencyStream2, Bandwidth);
    printf("Press Key to Continue\n");
    getchar();

    /*** Tune Channel Stream 5, Chip 0, HOST DMA ***/
	  rc = Tune(5, FrequencyStream3, Bandwidth);
    printf("Press Key to Continue\n");
    getchar();

	  for (i=0;i<10;i++)
    {
       /*** Monitor Signal Stream 1 ***/
       SignalMonitor(0);
       SignalMonitor(4);
       SignalMonitor(5);
    }

    printf("Press Key to Continue\n");

    /*** Add Subchannel 0 on Stream 0 ***/
    AddService(0,1);
    /*** Add Subchannel 0 on Stream 0 ***/
    AddService(4,1);
    /*** Add Subchannel 0 on Stream 0 ***/
    AddService(5,2);


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
   unsigned char ChId;

   
   /*** Master Host DMA Stream, using frontend 0 ***/
   if (streamid == 0)
   {
     FrontendId = 0;
   }
   /*** Slave 1 Host DMA Stream, using frontend 1 ***/
   else if (streamid == 4)
   {
     FrontendId = 1;
   }
   /*** Slave 2 Host DMA Stream, using frontend 2 ***/
   else if (streamid == 5)
   {
     FrontendId = 2;
   }
   else
   {
     printf("Only Stream 0, 4 and 5 can be used in DAB\n");
     return -1;
   }

   ChId  = GetChIdFromStrId(streamid);
   
   /** Get the stream in DVBT mode**/
   Status = DibGetStream(pContext, globalInfo.Stream[streamid], eSTANDARD_DAB, 0, ePOWER_ON);

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
   ChDesc.Type                                    = eSTANDARD_DAB;
   ChDesc.ChannelDescriptor.Bw                    = Bandwidth;	
   ChDesc.ChannelDescriptor.RFkHz                 = Frequency;
   ChDesc.ChannelDescriptor.InvSpec               = eDIB_SPECTRUM_AUTO;
   ChDesc.ChannelDescriptor.Std.Dab.NbActiveSub   = -1;
   ChDesc.ChannelDescriptor.Std.Dab.TMode	        = eDIB_DAB_TMODE_AUTO;

   /* Tune Channel */
   Status = DibGetChannelEx(pContext, globalInfo.Stream[streamid], &ChDesc, &DibChannelHdl[ChId], NULL, NULL);

   if   ( (Status == DIBSTATUS_SUCCESS) 
	    &&  (ChDesc.ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)) 

   {
     printf("Tune Successfull DVBT on %d KHz\n", Frequency);
	   return DIBSTATUS_SUCCESS;
   }
   else
   {
	   printf("Tune Failed DVBT on %d KHz\n",Frequency);
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
  double CN;
  double Ber;
  union DibDataMonit DataMonit;
  unsigned char ChId=0;

  ChId = GetChIdFromStrId(streamid);
  if (ChId == 0xFF)
    return -1;

  /*** Get Signal Monitoring ***********************/
  /*** Use Channel handle to identify the stream ***/
  Status = DibGetSignalMonitoring(pContext, 
                                     DibChannelHdl[ChId], 
                                     &NbDemod,
                                     &Monit[0], 
                                     NULL, 
                                     NULL);
  
  if (Status == DIBSTATUS_SUCCESS)
  {
    if (DabMode == eTDMB)
	    Ber = (double) Monit[0].Dab.Dvb.FecBerRs / 1e8;

	  if( (Monit[0].Cmmb.Dvb.SigFp == 0) || (Monit[0].Cmmb.Dvb.WgnFp==0) ) 
	  {
				CN = -100;
	  } 
	  else 
	  {
      double sig_dB = 10*log10((double)Monit[0].Isdbt.Dvb.SigFp) - 320*log10((double)2);
      double wgn_dB = 10*log10((double)Monit[0].Isdbt.Dvb.WgnFp) - 320*log10((double)2);
      CN = sig_dB - wgn_dB;
	  }

	  printf("Signal Monitoring Stream: %d : ",streamid);
	  printf("C/N: %5.4lf",CN);

    if (DabMode == eTDMB)
	    printf("BER: %f\n", Ber);
	  
  }
  return DIBSTATUS_SUCCESS;
}


    /*** register again the same buffer ***/
/**********************************************/
/********** Add DAB Service  *****************/
/**********************************************/
void DabDataCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
    struct CbCtx_t *pCtx = (struct CbCtx_t * )dataContext;
    unsigned char ChId = 0;

    if (Status == eREMOVED)
    {
        return;
    }
    printf("DAB Frame received from Stream %d, Size = %d\n",pCtx->streamid,pBuffCtx->BufSize);

    ChId = GetChIdFromStrId(pCtx->streamid);

    /*** register again the same buffer ***/
    DibRegisterBuffer(pContext, DabFilterHdl[ChId], DabDataBuffer[ChId], DAB_BUFFER_SIZE, 0);
}

int AddService(unsigned char streamid, unsigned short SubChannel)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;
    unsigned char ChId;
  
    ChId  = GetChIdFromStrId(streamid);
    DataCtx[ChId].streamid = streamid;

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = DabDataCallback;
    info.CallbackUserCtx = (void*)&DataCtx[ChId];

    /*** Create Filter, only for monitoring ***/
    Status = DibCreateFilter(pContext, DibChannelHdl[ChId], DabMode, &info, &DabFilterHdl[ChId]);

    if(Status == DIBSTATUS_SUCCESS)
    {
      printf(("DAB/TDMB Filter Create Successfull \n"));

      /*** Register a Buffer to receive DAB frames ***/
      Status = DibRegisterBuffer(pContext, DabFilterHdl[ChId], DabDataBuffer[ChId], DAB_BUFFER_SIZE, 0);

      if(Status == DIBSTATUS_SUCCESS)
      {
          union DibFilters Filter;

          Filter.Dab.Type     = DabAudioMode;
          Filter.Dab.SubCh    = SubChannel;
    
          /*** Start Service, only for monitoring ***/
          Status = DibAddToFilter(pContext, DabFilterHdl[ChId], 1, &Filter, &DabItemHdl[ChId]);
  
          if(Status == DIBSTATUS_SUCCESS)
          {
            printf("Add SubChannel %d on Stream %d Succesfull\n",SubChannel, streamid);
          }
          else
          {
            printf("Add SubChannel failed\n");
          }
        }
      }
      else
      {
            printf(("DAB Filter Create Failed \n"));
      }
      return Status;
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
unsigned char GetChIdFromStrId(unsigned char streamid)
{
    unsigned char ChId =0xFF;
    /*** Master Host DMA Stream, using frontend 0 ***/
    if (streamid == 0)
    {
      ChId = 0;
    }
    /*** Slave 1 Host DMA Stream, using frontend 1 ***/
    else if (streamid == 4)
    {
      ChId = 1;
    }
    /*** Slave 2 Host DMA Stream, using frontend 2 ***/
    else if (streamid == 5)
    {
      ChId = 2;
    }
    return ChId;
}
