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

/*****************************************************************************
* @file "MBBMSApp.c"
* @brief DiBcom MBBMS test application
*
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DibStatus.h"
#include "DibExtDefines.h"
#include "DibDefines.h"
#include "DibExtAPI.h"
#include "DibMbbmsMsg.h"
#include "DibMsgApi.h"



/****************************************************************************
* Global Variables
****************************************************************************/

#define NB_ELEM         5


#define DEVICE_NUM     0
#define CMMB_CARD_COUNTRY_CHINA  0x8986
#define CMMB_CARD_MOBILE         0x00


void Init2GGemalto(struct DibDriverContext *pContext);
void Init3GGemalto(struct DibDriverContext *pContext);
void InitEastComPeace(struct DibDriverContext *pContext);
/****************************************************************************/

void DibMbbmsGetCardId(uint8_t * pData, uint32_t DataLen)
{
   uint16_t country;
   uint8_t CardSN[4];

   if(DataLen < 12)
   {
      printf("MBBMS Card ID size not conform : %d instead of 1\n", DataLen);
      return;
   }
   country = (pData[0]<<8) + pData[1];
   printf("\nMBBMS Card Country       = %04X ", country);
   if (country == CMMB_CARD_COUNTRY_CHINA )
      printf("(China)");
   printf("\nMBBMS Card Support       = %02X", pData[2]);
   printf("\nMBBMS Card CardType      = %02X", pData[4]);
   printf("\nMBBMS Card ProvinceCode  = %02X", pData[5]);
   printf("\nMBBMS Card Year          = %02X", pData[6]);
   printf("\nMBBMS Card Manufacturer  = %01X", pData[7]>>4);
   CardSN[0]= ((pData[7]& 0x0F)<< 4) |( pData[8] >>4);
   CardSN[1]= ((pData[8]& 0x0F)<< 4) |( pData[9] >>4);
   CardSN[2]= ((pData[9]& 0x0F)<< 4) |( pData[10]>>4);
   CardSN[3]= ((pData[10]& 0x0F)<< 4)|( pData[11]>>4);
   printf("\nMBBMS Card Serial Number = %02X%02X%02X%02X \n",
         CardSN[0],CardSN[1],CardSN[2],CardSN[3],
         CardSN[0],CardSN[1],CardSN[2],CardSN[3]);
}


void DibMbbmsGetProtocolVersion(uint8_t * pData, uint32_t DataLen)
{
   printf("\nMBBMS Protocol Version   = %02X%02X%02X%02X\n",
         pData[0],pData[1],pData[2],pData[3]);
}

void DibMbbmsGetCmmbSN(uint8_t * pData, uint32_t DataLen)
{
   int i;

   printf("\nMBBMS CMMB Serial Number = ( ");
   for (i=0; i<DataLen; i++)
      printf("0x%02X ",pData[i]);
   printf(" ) >> ");
   for (i=0; i<DataLen; i++)
      printf("%c",pData[i]);
   printf("\n");
}

void DibMbbmsGetServices(uint8_t * pData, uint32_t DataLen)
{
   int i;

   printf("\nMBBMS CMMB Services supported = ");
   for (i=0; i<DataLen; i++)
      printf("%02X ",pData[i]);
   printf("\n");
}

void DibMbbmsDisplay(void * pCallbackCtx, uint16_t MsgType, uint16_t MsgIndex, uint8_t * pMsgBuf, uint32_t MsgLen)
{
   switch(MsgIndex)
   {
      case MBBMS_PROTOVER_IND : DibMbbmsGetProtocolVersion(pMsgBuf,MsgLen);   break;
      case MBBMS_CARDINFO_IND : DibMbbmsGetCardId(pMsgBuf,MsgLen);            break;
      case MBBMS_CMMBSN_IND   : DibMbbmsGetCmmbSN(pMsgBuf,MsgLen);            break;
      case MBBMS_SRVLIST_IND  : DibMbbmsGetServices(pMsgBuf,MsgLen);          break;
      default:
         printf( "[MBBMS] RECEIVED UNKNOWN MSG %d\n" , MsgIndex); break;
   };
}
/****************************************************************************/
typedef struct CasStat
{
   uint32_t RecvErr;
   uint32_t RecvTot;
   uint32_t SizeTot;
}tCasStat;

/* CMMB */
#define MF_BUF_SIZE     300000

uint8_t MultiplexFrameBuff[MF_BUF_SIZE];
//struct CmmbStream Stream;
tCasStat CasStat;

void CmmbMultiplexFrameCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
   /* Multiplex Frame is available at 'pBuffCtx->BufAdd', number of bytes is 'pBuffCtx->BufSize'. */
   struct DibDriverContext *pContext = (struct DibDriverContext *)dataContext;
   DIBSTATUS rc;
   time_t t;
   char timestr[100];
   int len=0;

   if(Status != eREMOVED)
   {
      rc = DibRegisterBuffer(pContext, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, MF_BUF_SIZE, pBuffCtx->BufId);
      if(rc != DIBSTATUS_SUCCESS)
      {
         printf(CRB "CLBACK CMMB Multiplex Frame: RegBuf id %d error: rc %d" CRA, pBuffCtx->BufId, rc);
         return;
      }
   }
/*   Stream.MfRawBuffer= pBuffCtx->BufAdd;
   if (ParseNewMultiplexFrame(&Stream) != 0)
      CasStat.RecvErr ++;
   CasStat.RecvTot++;
   time(&t);
   if (ctime_r(&t,timestr))
      len = strlen(timestr);
   if (len >0)
      timestr[len-1] = 0;
   printf("%s CMMB Multiplex Frame Total Received = %d with Error=%d\n",timestr,CasStat.RecvTot, CasStat.RecvErr);
   */

}
void CmmbResetStat ()
{
   memset(&CasStat,0 , sizeof(CasStat));
}

CHANNEL_HDL CmmbGetChannel (struct DibDriverContext *pContext, uint32_t Frequency)
{
   DIBSTATUS Status     = DIBSTATUS_SUCCESS;

      /* Tuning parameters */
   enum DibDemodType  TypeDemod = eSTANDARD_CMMB;
   enum DibSpectrumBW Bw        = eCOFDM_BW_8_0_MHZ;

   struct DibChannel  ChDesc;
   CHANNEL_HDL        ChHdl     = DIB_UNSET;

   /* Initialize tune parameters */
   ChDesc.Type                                    = TypeDemod;
   ChDesc.ChannelDescriptor.Bw                    = Bw;
   ChDesc.ChannelDescriptor.RFkHz                 = Frequency;
   ChDesc.ChannelDescriptor.InvSpec               = -1;
   ChDesc.ChannelDescriptor.Std.Cmmb.Modulation   = -1;

   /* Create, initialize a channel and tune on specified frequency */
   Status = DibGetChannel(pContext, DIB_DEMOD_AUTO, DIB_DEMOD_AUTO, 0, &ChDesc, &ChHdl, NULL, NULL);
   if(Status != DIBSTATUS_SUCCESS)
   {
      printf("Failed to get channel \n");
   }
   else printf("DibGetChannel SUCCESS \n");
   return ChHdl;
}

FILTER_HDL CmmbGetFilter (struct DibDriverContext *pContext, CHANNEL_HDL ChHdl)
{
   DIBSTATUS Status     = DIBSTATUS_SUCCESS;

   /* Filter specific information */
   struct DibDataBuffer info;
   FILTER_HDL           FilterHdl = DIB_UNSET;

   CmmbResetStat ();

   /* Create a data path for CMMB on the allocated channel */
   info.DataMode        = eCLBACK;
   info.CallbackFunc    = CmmbMultiplexFrameCallback;
   info.CallbackUserCtx = pContext;  /* Pass Driver Context to get it in the Callback */

   Status = DibCreateFilter(pContext, ChHdl, eCMMBSVC, &info, &FilterHdl);
   if(Status != DIBSTATUS_SUCCESS)
   {
      printf("Failed to create filter \n");
      return 0;
   }

   printf("DibCreateFilter SUCCESS \n");

   /*** Register a TS buffer for this filter ***/
   DibRegisterBuffer(pContext, FilterHdl, &MultiplexFrameBuff[0], MF_BUF_SIZE, 0);

   return FilterHdl;
}


ELEM_HDL CmmbGetService (struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint16_t ServiceId)
{
   DIBSTATUS Status     = DIBSTATUS_SUCCESS;
   ELEM_HDL             ElemHdl;
   union DibFilters     Service;

   Service.CmmbSvc.MfId              = 0;
   Service.CmmbSvc.SvcId             = ServiceId;
   Service.CmmbSvc.Mode              = eSVC_ID;

   /* Can be done in one shot or one by one */
   Status = DibAddToFilter(pContext, FilterHdl, 1 , &Service, &ElemHdl);
   if(Status != DIBSTATUS_SUCCESS)
   {
      printf("Failed to add item \n");
      return 0xFF;
   }
   printf("DibAddToFilter SUCCESS \n");
   return ElemHdl;
}

/****************************************************************************/

int32_t main(int32_t argc, char **argv)
{
   DIBSTATUS Status     = DIBSTATUS_SUCCESS;

    /* General target Information */
   struct DibGlobalInfo globalInfo;

   /* Target Device Context */
   struct DibDriverContext *pContext = NULL;

   /* Board Selection  */
   enum DibBoardType BoardType;

   /* Tuning parameters */
   CHANNEL_HDL        ChHdl     = DIB_UNSET;
   FILTER_HDL         FilterHdl = DIB_UNSET;
   ELEM_HDL           ElemHdl[2] = {DIB_UNSET};

   /* Message interface buffer */
   uint8_t  MsgBuf[256];
   uint32_t MsgBufLen = sizeof(MsgBuf);

   uint8_t  loop= 1;
   union DibParamConfig paramConfig;
   /************************************************************
    * Start Test
    ************************************************************/

   uint32_t DevNb = DEVICE_NUM;
   char Answer[100];
   uint32_t AnswerLen;
   uint16_t CAsystemID;
   uint32_t res;

   if ((argc<2) ||  (sscanf(argv[1],"%d",&BoardType)!= 1))
   {
      printf("SampleCmmbMbbms <BoardId> where board id is mandatory !!!\n", Status);
      return -1;
   }

   /* Get a handle to our target device */
   Status = DibOpen(&pContext, BoardType, &DevNb);
   if(Status != DIBSTATUS_SUCCESS)
   {
      printf("Failed to connect to target device, returned %d\n", Status);
      return -1;
   }
   printf("Connected to target device \n");
   /*  Get driver version */
   DibGetGlobalInfo(pContext, &globalInfo);
   printf("Driver ver: %d.%d.%d\n", DIB_MAJOR_VER(globalInfo.DriverVersion), DIB_MINOR_VER(globalInfo.DriverVersion), DIB_REVISION(globalInfo.DriverVersion));
   printf("Embedded SW ver: %d.%02d (IC: %d)\n", (globalInfo.EmbVersions[0] >> 10), globalInfo.EmbVersions[0] & 0x03ff, globalInfo.EmbVersions[1]);
   printf("Number of demod %d \n\n", globalInfo.NumberOfDemods);


   Status = DibRegisterMessageCallback(pContext, MSG_API_TYPE_MBBMS, MsgBuf, MsgBufLen, DibMbbmsDisplay, NULL);
   if(Status == DIBSTATUS_SUCCESS)
      printf("MBBMS interface registered\n");
   else
      printf("Failed to register MBBMS interface\n");


   printf("Enable CAS Process \n");
   paramConfig.CasEnable.Enable = eDIB_TRUE;
   DibSetConfig(pContext, eCAS_ENABLE,&paramConfig);

   while (loop)
   {
      printf("Initialize GBA and MSK using \n\t\t 1: Gemalto 3G\n\t\t 2: Gemalto 2G\n\t\t 3: EastComPeace\n");
      Answer[0]= getchar();
      switch (Answer[0])
      {
         case '1':Init3GGemalto(pContext); loop = 0;break;
         case '2':Init2GGemalto(pContext); loop = 0;break;
         case '3':InitEastComPeace(pContext); loop = 0;break;
         default: break;
      }
   }
   loop = 1;
   while (loop)
   {
      printf("Set CAsystemId  = ");

      if (fscanf(stdin, "%d",&res) == 1)
      {
         CAsystemID= (uint16_t)res;
         loop=0;
      }
      else
      {
         fscanf(stdin, "%s",Answer);
         printf("Wrong CA system ID <%s> must be 2 bytes wide\n", Answer);
      }
   }
   Status =DibSendMessage(pContext, MSG_API_TYPE_MBBMS, MBBMS_CASYSTEMID_SET,
                          (uint8_t *) &CAsystemID, 2,  Answer, &AnswerLen);

   if(Status != DIBSTATUS_SUCCESS)
   {
      printf("Fails setting CA system ID \n");
      goto End;
   }

   /* Initialize tune parameters */
   ChHdl = CmmbGetChannel(pContext,674000);
   FilterHdl = CmmbGetFilter (pContext, ChHdl);
   ElemHdl[0]= CmmbGetService (pContext,FilterHdl, 603);

   printf(" To Stop Test Press <q> \n");
   loop=0;
   while (getchar()!='q');

   End:

   if(pContext)
   {
      /* Delete Channel */
      if(ChHdl != DIB_UNSET)
         DibDeleteChannel(pContext, ChHdl);

         /* Close handle to the target device and free memory allocated at DibOpen */
      /* disable cas support*/
      paramConfig.CasEnable.Enable = eDIB_FALSE;
      DibSetConfig(pContext, eCAS_ENABLE, &paramConfig);

      DibClose(pContext);

   }
   return 0;
}

/************************************************************
* SMSendCommand
************************************************************/
void SMCommand( struct DibDriverContext *pContext, char* Command, uint8_t* ApduAnswer, uint32_t* pApduAnswerLen)
{
   char        tmp[3];
   DIBSTATUS   Status         = DIBSTATUS_SUCCESS;
   uint8_t     ApduCommand[264];
   int         i;
   uint32_t    Commandlen     = strlen(Command);
   int         Val;

   for (i=0;(2*i)<Commandlen;i++)
   {
      strncpy(tmp,Command+(2*i),2);
      tmp[2]=0;
      if (sscanf(tmp,"%02x",&Val) != 1)
      {
          if (sscanf(tmp,"%02X",&Val) != 1)
          {
            printf("error in scanning command at uint8_t %d\n",i);
            return;
          }
      }
      ApduCommand[i]= (uint8_t)Val;
   }
   Commandlen = i;
   /* set max to receive */
   *pApduAnswerLen = 264;
   for (i=0; i<Commandlen; i++)
        printf(" %02X", ApduCommand[i]);
   printf("\n");


   Status =DibSendMessage(pContext, MSG_API_TYPE_MBBMS, MBBMS_SCTRANSMIT,
                          (uint8_t *)ApduCommand, Commandlen,  ApduAnswer, pApduAnswerLen);


   if(Status == DIBSTATUS_SUCCESS)
   {
      printf(" card response (%d bytes):", *pApduAnswerLen);
      for (i=0; i<*pApduAnswerLen; i++)
         printf(" %02X", ApduAnswer[i]);
      printf("\n");
      printf("\n");
   }
   else
   {
      printf(" Command failed returned error 0x%x\n", Status);
   }
}

void SMSendCommand( struct DibDriverContext *pContext, char* Command)
{
   uint8_t  ApduAnswer[264];
   uint32_t ApduAnswerLen = sizeof(ApduAnswer);

   SMCommand( pContext, Command, &ApduAnswer[0], &ApduAnswerLen);

}
void    SMSendGetResp( struct DibDriverContext *pContext, char* Command, char* resp)
{
   uint8_t  ApduAnswer[264];
   uint32_t ApduAnswerLen = sizeof(ApduAnswer);
   int i;

   SMCommand( pContext, Command, ApduAnswer, &ApduAnswerLen);

   for (i=0; i<ApduAnswerLen; i++)
   {
      sprintf(resp+(2*i),"%02X",ApduAnswer[i]);
   }
}

 /************************************************************
  *  3G TESTS
  ************************************************************/

void Init3GGemalto(struct DibDriverContext *pContext)
{
   char  ApduCommand[400];
#ifdef DOGETAID
   char AIDstr[120];
   char* AIDptr;
#endif
   printf("Step : Select EFdir \n");

#ifdef DOGETAID
   strcpy(ApduCommand, "00A40004022F00");
   SMSendCommand( pContext, ApduCommand);


   printf("Step : Read AID application identificator \n");

   strcpy(ApduCommand, "00B2000226");
   TestSendGetResp( pContext, ApduCommand, AIDstr);
   AIDptr = strstr(AIDstr,"4F10");
   if (!AIDptr)
   {
      printf(" Fails to get the AID value\n");
      return;
   }

   printf("Step : Select 3G AID \n");

   strcpy(ApduCommand, "00A4040410");
   strncpy(ApduCommand+10, AIDptr+4, 32);
   ApduCommand[42]=0;
   SMSendCommand( pContext, ApduCommand);
#else
   strcpy(ApduCommand, "00A4040410A0000000871002FF47F00189000001FF");
   SMSendCommand( pContext, ApduCommand);
#endif

   printf("Step : BootStrapping Mode \n");
   strcpy(ApduCommand,"008800843DDD08A54211D5E3BA50BF10B40BA9A3C58B2A05BBF0D987B21BF8CB10F769BCD751044604127672711C6D34411023553CBE9637A89D218AE64DAE47BF35");
   SMSendCommand( pContext,  ApduCommand);

   printf("Step : SELECT EF_gbabp \n");
   strcpy(ApduCommand,"00A4000C026FD6");
   SMSendCommand( pContext,  ApduCommand);

   printf("Step : Write BTid and Ks in EFgbabp(6FD6) \n");
   strcpy(ApduCommand,"00D600112e2578684F4270656B496C42386E30735265554A384768673D3D407777772E6D6F74762e636f6d0720081206140119");
   SMSendCommand( pContext,  ApduCommand);

   printf("Step : NAF Decentralized mode Mode \n");
   strcpy(ApduCommand   , "0088008437DE12017777772E6D6F74762E636F6D0100000001223436303033303130343830303132324031302e6368696e616d6f62696c652e636f6d");
   SMSendCommand( pContext,  ApduCommand);

   printf("Step : MIKEY MSK MESSAGE \n");
   strcpy(ApduCommand    , "00880085A401A201001500AAAABBBB00010503000B0003AAAAAA0104A1A2A3A40B020000000106101234567890ABCDEFFEDCBA09876543210600000D017777772E6D6F74762E636F6D0100002578684F4270656B496C42386E30735265554A384768673D3D407777772E6D6F74762E636F6D0001001EF803DE39E567B027D221A37CA7AFC64DA12A01B63DA0C9557C89CD62301E01808F4484B971163BAB8631E26D535B2264F23FD2");
   SMSendCommand( pContext,  ApduCommand);

/*   printf("Step : MIKEY MTK MESSAGE \n");
   strcpy(ApduCommand    , "0088008554025201001500AAAABBBB0001050300110003AAAAAA0104A1A2A3A40204011015AF01020000000200010014720DA995A33323DE7AA0EB963E2AD82151A11A2E017C16BF73408F0BF1FD52CCD6ADDE8CD9A4A53B8C");
   SMSendCommand( pContext,  ApduCommand);*/
}

void Init2GGemalto(struct DibDriverContext *pContext)
{
   char  ApduCommand[400];

   /************************************************************
    * Select 2G AID
    ************************************************************/
   printf("Step : Select 2G GSM directory \n");

   strcpy(ApduCommand,"A0A40000027F20");
   SMSendCommand( pContext,  ApduCommand);

   /************************************************************
    * GBA Bootstrapping
    ************************************************************/
   printf("Step : 2G GBA Bootstrapping \n");

   strcpy(ApduCommand ,"A088008431DD044185432708B0DBD176C4FFDC001011223344556677881122334455667788100123456789ABCDEF0123456789ABCDEF");
   SMSendCommand( pContext,  ApduCommand);

   /************************************************************
    * Update EFgbabp(6FD6), write B-Tid and Ks lifetime
    ************************************************************/
   printf("Step : 2G Select EFgbabp(6FD6) \n");
   strcpy(ApduCommand ,"A0A40000026FD6");
   SMSendCommand( pContext,  ApduCommand);

   printf("Step : 2G Write BTid and Ks in EFgbabp(6FD6) \n");

   strcpy(ApduCommand ,"A0D600112E2578684F4270656B496C42386E30735265554A384768673D3D407777772E6D6F74762e636f6d0720081206140119");
   SMSendCommand( pContext,  ApduCommand);

   /************************************************************
    * Send Normal MRK
    ************************************************************/
    printf("Step : 2G Send Normal MRK \n");
    strcpy(ApduCommand    , "A088008437DE12017777772E6D6F74762E636F6D0100000001223436303033303130343830303132324031302e6368696e616d6f62696c652e636f6d");
    SMSendCommand( pContext,  ApduCommand);

   /************************************************************
    * Send  MSK
    ************************************************************/
   printf("Step : 2G Send Normal MSK \n");
   strcpy(ApduCommand , "A0880085A401A201001500AAAABBBB00010503000B0003AAAAAA0104A1A2A3A40B020000000106101234567890ABCDEFFEDCBA09876543210600000D017777772E6D6F74762E636F6D0100002578684F4270656B496C42386E30735265554A384768673D3D407777772E6D6F74762E636F6D0001001E82CA319D537EFB54787C1B56AE4B9A3EB83C6C781FFADB41F68D440111BF01E578C363FBA2BBE4B76C1813B6C54D84F1AD9ADB");
   SMSendCommand( pContext,  ApduCommand);

   /************************************************************
    * Send  MTK
    ************************************************************/
/*   printf("Step : 2G Send MTK \n");
   strcpy(ApduCommand,"A088008554025201001500AAAABBBB0001050300110003AAAAAA0104A1A2A3A40204011015AF01020000000200010014720DA995A33323DE7AA0EB963E2AD82151A11A2E017C16BF73408F0BF1FD52CCD6ADDE8CD9A4A53B8C");
   SMSendCommand( pContext,  ApduCommand);
*/
}

/************************************************************
 *  EastComPeace Init
 ************************************************************/
void InitEastComPeace(struct DibDriverContext *pContext)
{
   char  ApduCommand[400];

   printf("Step : Reset \n");

 //  SMSendCommand( pContext, "FEFEFEFE00");
   SMSendCommand( pContext, "0020000A084444444444444444");
   //SMSendCommand( pContext, "E044000000");
   SMSendCommand( pContext, "00A4080C0477F0F007");
   SMSendCommand( pContext, "00D6001005FF0FAFAEEE"); /* ;Cnonce(LV)*/
   SMSendCommand( pContext, "00A4080C047F206FD7");
   SMSendCommand( pContext, "00DC010414FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
   SMSendCommand( pContext, "00A4080C047F206FD8");
   SMSendCommand( pContext, "00DC01046700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
   SMSendCommand( pContext, "00A4080C0477F0F001");
   SMSendCommand( pContext, "00D6000006000000000000");
/*   SMSendCommand( pContext, "E044000100");
*/

   printf("Step : TEST START  \n");
/*   SMSendCommand( pContext, "FEFEFEFE00");
*/
   printf("Step : select UAM ADF  \n");
   SMSendCommand( pContext, "00A4040C10A0000000871002FF86FFFF89FFFFFFFF");

   printf("Step : GBA Bootstrapping  \n");
   printf("Expected answer = DB10663E6EBED9B55FD6ED2506D854458440040FAFAEEE 9000\n");
   SMSendCommand( pContext, "008800843DDD082E9555D6A8C089E910BE737FA6CA469EDF00EA98F1BAA5331A10B8B74AEA4F4D7E111FD7F1BDE38C7021107CCA62D4314CDC3AA4330292FC9AD630");

   printf("Step : update B-TID  \n");
   SMSendCommand( pContext, "00A4000C026FD6");
   SMSendCommand( pContext, "00D600112E28664D70693144464D3344716B4D774B532F4A72574D413D3D406368696E616D6F62696C652E636F6D049F2C984A");

   printf("Step : MRK request  \n");
   printf("Expected answer = DB207334BFC3EB2E2F260E6DD099A00ABDE314DFE6ECD94D916C1512BAEE52B46EAD 9000\n");
   SMSendCommand( pContext, "0088008434DE123139322E3136382E31302E373001000000011F343630303738303034383130313339406368696E616D6F62696C652E636F6D");

   printf("Step : MSK update  \n");
   SMSendCommand( pContext, "00880085A701A501001580AAAABBBB00010503000B000300000A0104000A000A0B020000000106101234567890ABCDEFFEDCBA09876543210600000D3139322E3136382E31302E373001000028664D70693144464D3344716B4D774B532F4A72574D413D3D406368696E616D6F62696C652E636F6D0001001E63DD4F4BF7EFFA3763C5EC6D18086BF8BCEA165F0F8610AF06540A92DFA6014A8147F3EEABD1C98EE9B8358A4D91E337999454");
/*
   printf("Step : get MTK  \n");
   printf("Expected answer =DB1045454545454545454515212224562245 9000\n");
   SMSendCommand( pContext, "0088008554025201001500AAAABBBB000105030011000300000A0104000A000A020400000D00010200000001000100144D1BA6707684D375D92776C53E1B93F51D6B2D2901C0AFBECCCF5ABC97CD2ACDD3B82B093DEDC5AB79");

   printf("Step : get MTK  \n");
   printf("Expected answer =DB1045454545454545454515212224562245 9000\n");
   SMSendCommand( pContext, "0088008554025201001500AAAABBBB000105030011000300000A0104000A000A020400000D01010200000001000100144D1BA6707684D375D92776C53E1B93F51D6B2D2901985477ACFF1507BD39A52074AAF1C7EB8475ADE0");
*/
}
