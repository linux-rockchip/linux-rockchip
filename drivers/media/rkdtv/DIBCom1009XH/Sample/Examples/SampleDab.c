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
        DAB/TDMB Sample Code
**********************************************
This Example shows how to 
- Open the driver, 
- Tune a DAB Channel
- Start one service reception
- Monitor Signal and Data reception
- Close driver
*********************************************/

/*********************************************/
/*** Global Variables ************************/
/*********************************************/
struct DibDriverContext *pContext;
CHANNEL_HDL             DibChannelHdl;
FILTER_HDL              DabFilterHdl;
ELEM_HDL                DabItemHdl;
union DibDemodMonit     Monit[4];	

#define DAB_BUFFER_SIZE 200000
unsigned char DabDataBuffer[DAB_BUFFER_SIZE];

/*********************************************/
/*** Local Functions *************************/
/*********************************************/
int Open( enum DibBoardType BoardType );
int Tune(unsigned int Frequency,  enum DibSpectrumBW Bw );
int AddService(unsigned short MFId);
int SignalMonitor (void);

/*********************************************/
/*** Test Parameters *************************/
/*********************************************/
/*enum DibBoardType  Board       = eBOARD_NIM_1009xHx;*/
enum DibBoardType  Board      = eBOARD_NIM_10096MD4;     
uint32_t           Frequency     = 220352;
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

  /*** Open Driver ***/
	rc = Open(Board);

	if (rc == DIBSTATUS_SUCCESS)
	{

    /*** Tune Channel ***/
	  rc = Tune(Frequency, Bandwidth);

    printf("Press Key to continue\n");
    getchar();

	  if (rc == DIBSTATUS_SUCCESS)
	  {
       /*** Add DAB Service ***/
       rc = AddService(DabSubChannel);

	     if (rc == DIBSTATUS_SUCCESS)
	     {
         printf("Press Key to continue\n");
         getchar();

	       for (i=0;i<500;i++)
         {
           /*** Monitor Signal and Data Reception ***/
           SignalMonitor();
           usleep(100000);
         }
	     }
    }
    /*** Close Driver ***/
	  DibClose(pContext);
	}
	
}
/**********************************************/
/********** Driver Initialization *************/
/**********************************************/
int Open( enum DibBoardType BoardType )
{
   /* General target Information */
   struct DibGlobalInfo globalInfo;

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
   struct DibChannel  ChDesc;
   int i;
   
   /* Initialise tune parameters */
   ChDesc.Type                                    = eSTANDARD_DAB;
   ChDesc.ChannelDescriptor.Bw                    = Bandwidth;	
   ChDesc.ChannelDescriptor.RFkHz                 = Frequency;
   ChDesc.ChannelDescriptor.InvSpec               = eDIB_SPECTRUM_AUTO;
   ChDesc.ChannelDescriptor.Std.Dab.NbActiveSub   = -1;
   ChDesc.ChannelDescriptor.Std.Dab.TMode	        = eDIB_DAB_TMODE_AUTO;

   /* Create, initialise a channel and tune on specified frequency */
   Status = DibGetChannel(pContext, 
                          DIB_DEMOD_AUTO, 
                          DIB_DEMOD_AUTO, 
                          0, 
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
/********** Add DAB Service  *****************/
/**********************************************/
void DabDataCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
    if (Status == eREMOVED)
    {
        return;
    }
    printf("DAB Frame received, Size = %d\n",pBuffCtx->BufSize);

    /*** register again the same buffer ***/
    DibRegisterBuffer(pContext, DabFilterHdl, DabDataBuffer, DAB_BUFFER_SIZE, 0);
}

int AddService(unsigned short SubChannel)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = DabDataCallback;
    info.CallbackUserCtx = NULL;

    /*** Create Filter, only for monitoring ***/
    Status = DibCreateFilter(pContext, DibChannelHdl, DabMode, &info, &DabFilterHdl);

    if(Status == DIBSTATUS_SUCCESS)
    {
      printf(("DAB/TDMB Filter Create Successfull \n"));

      /*** Register a Buffer to receive DAB frames ***/
      Status = DibRegisterBuffer(pContext, DabFilterHdl, DabDataBuffer, DAB_BUFFER_SIZE, 0);

      if(Status == DIBSTATUS_SUCCESS)
      {
          union DibFilters Filter;

          Filter.Dab.Type = DabAudioMode;
          Filter.Dab.SubCh    = SubChannel;
    
          /*** Start Service, only for monitoring ***/
          Status = DibAddToFilter(pContext, DabFilterHdl, 1, &Filter, &DabItemHdl);
  
          if(Status == DIBSTATUS_SUCCESS)
          {
            printf("Add SubChannel Succesfull\n");
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

/**********************************************/
/********** Signal Monitoring *****************/
/**********************************************/
int SignalMonitor (void)
{
  DIBSTATUS Status = DIBSTATUS_SUCCESS;
  uint8_t NbDemod = 1;
  double CN;
  double Ber;
  union DibDataMonit DataMonit;

  
  /*** Get Signal Monitoring ***/
  Status = DibGetSignalMonitoring(pContext, 
                                  DibChannelHdl, 
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

	  printf("Signal Monitoring:\n");
	  printf("C/N: %5.4lf\n",CN);

    if (DabMode == eTDMB)
	    printf("BER: %f\n", Ber);
	  
  }
  return DIBSTATUS_SUCCESS;
}
