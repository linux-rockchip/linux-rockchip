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
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "DibNagraMsg.h"

/*********************************************
        CMMB Nagra Sample Code
**********************************************
This Example shows how to 
- Open the driver, 
- Starts Nagra
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
enum DibBoardType  Board       = eBOARD_NIM_1009xHx;
/*enum DibBoardType  Board      = eBOARD_NIM_10096MD4;*/     
uint32_t           Frequency  = 674000;
enum DibSpectrumBW Bandwidth  = eCOFDM_BW_8_0_MHZ;
unsigned short     CmmbMFId   = 1;
unsigned char MsgBuf[256];
/*********************************************/


/*********************************************/
/* NAGRA MESSAGE PROCESSING FUNCTIONS */ 
/*********************************************/

void DibProcessNagraPopup(uint8_t * pMsgBuf, uint32_t MsgLen)
{
   uint8_t isPersistent, nbContent, *curLangMsg = NULL, *engMsg = NULL, *otherMsg = NULL, *rdPtr;
   char msgLang[4] = {'N', 'U', 'L', '\0'}, *curLang = "fra";
   uint32_t contentLen, tt;
   
   /* encoding of the sendBuffer:
    * sendBuf[0] = persistance (true if BDC_POP_UP_PERSISTENCE_ALWAYS)
    * sendBuf[1] = Number of content buffers (languages)
    * --- first content buffer begin ---
    * sendBuf[2..4] = language
    * sendBuf[5..8] = contentLen
    * sendBuf[9..9+contentLen-1] = content
    * --- first content buffer end ---
    * sendBuf[9+contentLen..9+contentLen+2] = language of second content buffer
    * ...
    */
   isPersistent = pMsgBuf[0];
   nbContent = pMsgBuf[1];
   rdPtr = pMsgBuf + 2;

   for (tt = 0; tt < nbContent; tt++) 
   {
      /* read language and length */ 
      if (rdPtr - pMsgBuf + 7 > MsgLen)
      {
         printf("++++++BDC popup: ERROR: popup content truncated!\n");
         break;
      }

      memcpy(msgLang, rdPtr, 3);
      rdPtr += 3;

      /* fix endianness */
      contentLen = 0;
      contentLen |= (*(rdPtr++) << 24);
      contentLen |= (*(rdPtr++) << 16);
      contentLen |= (*(rdPtr++) << 8 );
      contentLen |= (*(rdPtr++));
       
      if (rdPtr - pMsgBuf + contentLen > MsgLen)
      {
         printf("++++++BDC popup: ERROR: popup content truncated!\n");
         break;
      }

      /* read languages. We need current, english or any language */
      if (strcmp(msgLang, curLang) == 0) 
      {
         curLangMsg = malloc(contentLen+1);
         memcpy(curLangMsg, rdPtr, contentLen);
         curLangMsg[contentLen] = 0;
      }
      if (strcmp(msgLang, "eng") == 0) 
      {
         engMsg = malloc(contentLen+1);
         memcpy(engMsg, rdPtr, contentLen);
         engMsg[contentLen] = 0;
      }
      if (otherMsg == NULL) 
      {
         otherMsg = malloc(contentLen+1);
         memcpy(otherMsg, rdPtr, contentLen);
         otherMsg[contentLen] = 0;
      }
      rdPtr += contentLen;
   }
   
   if (!curLangMsg)
   {
      curLangMsg = engMsg;
      engMsg = NULL;
   }
   if (!curLangMsg)
   {
      curLangMsg = otherMsg;
      otherMsg = NULL;
   }

   printf("=======BDC pops up ");
   if (!isPersistent)
      printf("non-");
   printf("persistent message:\n");
   if (curLangMsg == NULL)
   {
      printf ("<no Msg>\n");
   }
   else if (curLangMsg[0] >= 20)
   {
      printf("%s", curLangMsg);
   }
   else
   {
      switch (curLangMsg[0])
      {
      case 0x10: printf ("<ISO 8859 Msg>\n"); break;
      case 0x11: printf ("<ISO/IEC 6937 Msg>\n"); break;
      case 0x1: 
      case 0x2: 
      case 0x3: 
      case 0x4: 
      case 0x5: printf ("<ISO/IEC 8859 5-9 Msg>\n"); break;
      default: printf ("<strange Msg 0x%x>\n", curLangMsg[0]); break;
      }
   }
   printf("\n=========\n");
   free(curLangMsg);
   free(engMsg);
   free(otherMsg);
}

void DibNagraDisplay(void * pCallbackCtx, uint16_t MsgType, uint16_t MsgIndex, uint8_t * pMsgBuf, uint32_t MsgLen)
{
   switch(MsgIndex)
   {
      case NAGRA_VERSION:       printf( "[NAGRA] CasVersion: %s\n" , pMsgBuf); break;
      case NAGRA_VENDOR:        printf( "[NAGRA] CasVendor: %s\n"  , pMsgBuf); break;
      case NAGRA_CA_SYSTEM_ID:  printf( "[NAGRA] CaSystemId: %d\n" , (pMsgBuf[0] << 8) + pMsgBuf[1]); break;
      case NAGRA_CA_CARD_ID:    printf( "[NAGRA] CardId: 0x"); 
      {
         uint8_t ii;
         for (ii=0; ii<MsgLen; ii++)
            printf("%02x", pMsgBuf[ii]);
         printf("\n");
      }
      break;
      case NAGRA_PRINT_OUTPUT : printf( "%s" , pMsgBuf); fflush(stdout); break;
      case NAGRA_BDC_POPUP    : DibProcessNagraPopup(pMsgBuf, MsgLen); break;
      case NAGRA_BDC_POPUP_REMOVE: printf( "++++++++ nagra removes popup\n"); break;
      case NAGRA_ACCESS_STATUS: 
         if(*pMsgBuf == eACCESS_GRANTED)
            printf( "Access GRANTED\n");
         else
            printf( "Access DENIED\n");
         break;
      case NAGRA_SUBSCRIBER_STATUS: 
         switch (*pMsgBuf)
         {
            case eSUBSCRIBER_UNKNOWN        : printf("Subscriber is unknown\n"); break;
            case eSUBSCRIBER_REGISTERED     : printf("Subscriber is registered\n"); break;
            case eSUBSCRIBER_SUSPENDED      : printf("Subscriber is suspended \n"); break;
            case eSUBSCRIBER_NOT_REGISTERED : printf("Subscriber not registered\n"); break;
            case eSUBSCRIBER_ERROR          : printf("Subscriber status error \n"); break;
            default: printf( "[NAGRA] invalid subscriber status message\n");
         }
         break;

      case NAGRA_ERROR: 
         switch (*pMsgBuf)
         {
            case eNAGRA_ERROR           : printf("Nagra CAS error\n"); break;
            case eNAGRA_ERROR_SMARTCARD : printf("Nagra Smartcard error\n"); break;
            default: printf( "[NAGRA] invalid Nagra error message %d\n", *pMsgBuf);
         }
         break;
      default:               printf( "[NAGRA] RECEIVED UNKNOWN MSG %d\n" , MsgIndex); break;
   };
}


/*********************************************/
/*** MAIN ************************************/
/*********************************************/
int main(int argc, char** argv)
{
  DIBSTATUS rc;
  unsigned int i;
  unsigned int MsgBufLen = sizeof(MsgBuf);
  union DibParamConfig ParamConfig;

  /*** Open Driver ***/
	rc = Open(Board);

    /* Start CAS */

     rc = DibRegisterMessageCallback(pContext, MSG_API_TYPE_NAGRA, MsgBuf, sizeof(MsgBuf), DibNagraDisplay, NULL);
     if(rc == DIBSTATUS_SUCCESS)
       printf("Nagra interface registered\n");
     else
       printf("Failed to register Nagra interface %d\n", rc);

   /* Enable CAS system */
   printf("Enable Nagra \n");
   ParamConfig.CasEnable.Enable = eDIB_TRUE;
   rc = DibSetConfig(pContext, eCAS_ENABLE, &ParamConfig);
   if(rc == DIBSTATUS_SUCCESS)
      printf("Nagra enabled\n");
   else
      printf("Failed to enable Nagra\n");

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
