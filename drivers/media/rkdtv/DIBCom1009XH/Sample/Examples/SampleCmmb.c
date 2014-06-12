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
        CMMB Sample Code
**********************************************
This Example shows how to 
- Open the driver, 
- Tune a CMMB Channel
- Start one service reception
- Monitor Signal and Data reception
- Close driver
*********************************************/

/*********************************************/
/*** Global Variables ************************/
/*********************************************/
struct DibDriverContext *pContext;
CHANNEL_HDL             DibChannelHdl;
FILTER_HDL              CmmbFilterHdl;
ELEM_HDL                CmmbItemHdl;
union DibDemodMonit     Monit[4];	

#define CMMB_BUFFER_SIZE 200000
unsigned char CmmbDataBuffer[CMMB_BUFFER_SIZE];

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
uint32_t           Frequency  = 674000;
enum DibSpectrumBW Bandwidth  = eCOFDM_BW_8_0_MHZ;
unsigned short     CmmbMFId   = 1;
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
       /*** Add CMMB Service ***/
       rc = AddService(CmmbMFId);

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

   ChDesc.Type                        = eSTANDARD_CMMB;
   ChDesc.ChannelDescriptor.Bw        = Bandwidth;
   ChDesc.ChannelDescriptor.RFkHz			= Frequency;

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
/********** Add CMMB Service  *****************/
/**********************************************/
void CmmbDataCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
    if (Status == eREMOVED)
    {
        return;
    }
    printf("CMMB Frame received, Size = %d\n",pBuffCtx->BufSize);

    /*** register again the same buffer ***/
    DibRegisterBuffer(pContext, CmmbFilterHdl, CmmbDataBuffer, CMMB_BUFFER_SIZE, 0);
}

int AddService(unsigned short MFId)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = CmmbDataCallback;
    info.CallbackUserCtx = NULL;

    /*** Create Filter, only for monitoring ***/
    Status = DibCreateFilter(pContext, DibChannelHdl, eCMMBSVC, &info, &CmmbFilterHdl);

    if(Status == DIBSTATUS_SUCCESS)
    {
      printf(("Cmmb Filter Create Successfull \n"));

      /*** Register a Buffer to receive CMMB frames ***/
      Status = DibRegisterBuffer(pContext, CmmbFilterHdl, CmmbDataBuffer, CMMB_BUFFER_SIZE, 0);

      if(Status == DIBSTATUS_SUCCESS)
      {
          union DibFilters Filter;
          Filter.CmmbSvc.Mode = eMF_ID;
          Filter.CmmbSvc.MfId = MFId;
    
          /*** Start Service, only for monitoring ***/
          Status = DibAddToFilter(pContext, CmmbFilterHdl, 1, &Filter, &CmmbItemHdl);
  
          if(Status == DIBSTATUS_SUCCESS)
          {
            printf("Add MF Succesfull\n");
          }
          else
          {
            printf("Add MF failed\n");
          }
        }
      }
      else
      {
            printf(("Cmmb Filter Create Failed \n"));
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
  unsigned int TotalLdpc, ErrorLdpc;
  double Bler;
  union DibDataMonit DataMonit;

  
  /*** Get Signal Monitoring ***/
  Status = DibGetSignalMonitoring(pContext, 
                                  DibChannelHdl, 
                                  &NbDemod,
                                  &Monit[0], 
                                  NULL, 
                                  NULL);

  Status |= DibGetDataMonitoring(pContext, CmmbItemHdl, &DataMonit, eDIB_FALSE);

  if (Status == DIBSTATUS_SUCCESS)
  {
    TotalLdpc = DataMonit.Cmmb.TotalLdpcBlocks;
    ErrorLdpc = DataMonit.Cmmb.ErrorLdpcBlocks;
    Bler      = DataMonit.Cmmb.Bler;
	
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
	  printf("Ldpc Block (Total: %d / Error: %d), Block Error Rate: %f\n", TotalLdpc,ErrorLdpc,Bler);
	  
  }
  return DIBSTATUS_SUCCESS;
}
