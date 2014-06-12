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

/**************************************************************************************************
* @file "DtaCmds.c"
* @brief DiBcom Test Application - Commands Functions.
*
***************************************************************************************************/
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DibConfig.h"
#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaCmds.h"
#include "DtaScan.h"
#include "DtaSiPsi.h"
#include "DtaMonit.h"
#include "DtaTime.h"

#if (EMBEDDED_FLASH_SUPPORT == 1)
#include "DibExtFlashAPI.h"
#endif /* (EMBEDDED_FLASH_SUPPORT == 1) */

struct DibDriverContext * gUserContext;

#define CONTEXT (gUserContext)

uint32_t Current_RF_Freq;

int32_t DtaNbMpeRunning = 0;
int32_t DtaNbSiPsiRunning[DIB_MAX_NB_CHANNELS];

#define BUFF_MAX_DTA     500000

unsigned char TsDataBuff[BUFF_MAX_DTA];
unsigned char TsDataRecord = 0;
FILE *File;
unsigned char PesVideoDataBuff[BUFF_MAX_DTA];
unsigned char PesAudioDataBuff[BUFF_MAX_DTA];
unsigned char PesOtherDataBuff[BUFF_MAX_DTA];

const char * StdString[eMAX_STANDARD_DEMOD]= {
   "UNK", "DVBT", "ISDBT", "DAB", "AUDIO", "ANALOG", "ATSC", "DVBSH", "FM", "CMMB", "ISDBT_1seg", "CTTB", "ATSCMH"
};

const char * StreamTypeString[]= {
   "HOST_DMA", "MPEG_1", "MPEG_2"
};

const char * StreamOptionString[]= {
   "NO_204", "NO_188", "NO_SERIAL", "NO_PARALLEL"
};

struct DibGlobalInfo         globalInfo;

#if (DIB_CMMB_DATA == 1)
unsigned char MultiplexFrameBuff[BUFF_MAX_DTA];
#endif /* DIB_CMMB_DATA */

#if (DIB_FIG_DATA == 1)
FILE * pFigFile;
#endif
#if (DIB_TDMB_DATA == 1)
FILE * pTdmbFile;
#endif

volatile unsigned char StopDataRunning  = 1;
unsigned char          NbDefinedFilters = 0;

/* Monitoring for Ts packets test */
#define TP_SIZE_BYTES   188
unsigned char          CurrentTsMonit = DIB_UNSET;
struct DtaTsInfo       gTsDataInfo[MAX_TS_MONIT_CTX];
unsigned char          PidsCc[MAX_PID_BY_CTX];

/* Array of activated Elementary Streams. */
#define PID_FILTER_EMPTY    0xFFFF

struct DtaChannel  ChannelArray[DIB_MAX_NB_CHANNELS];
struct DtaFilter   FilterArray[DIB_MAX_NB_FILTERS];
struct DtaItem     ItemArray[DIB_MAX_NB_ITEMS];

struct DtaStream   StreamArray[DIB_MAX_NB_OF_STREAMS];
struct DtaFrontend FrontendArray[DIB_MAX_NB_OF_FRONTENDS];

const char * DataTypeString[] =
{
    "TS",		/* [eTS]	      */
    "SIPSI",	/* [eSIPSI]    */
    "MPEFEC",	/* [eMPEFEC]   */
    "FM",		/* [eFM]	      */
    "ANALOG",	/* [eANALOG]   */

    "FIG",		/* [eFIG]	   */
    "TDMB",		/* [eTDMB]	   */
    "DAB",		/* [eDAB]	   */
    "DABPACKET",/* [eDABPACKET]*/
    "eDAB",		/* [eEDAB]	   */
    "PESVIDEO",/* [ePESVIDEO] */
    "PESAUDIO",/* [ePESAUDIO] */
    "PESOTHER",/* [ePESOTHER] */
    "PCR",     /* [ePCR]      */
    "MPEIFEC", /* [eMPEIFEC]  */
    "CMMBMFS",   /* [eCMMBMFS]   */
    "CMMBCIT",   /* [eCMMBCIT]   */
    "CMMBSVC",   /* [eCMMBSVC]   */
    "CMMBXPE",   /* [eCMMBXPE]   */
    "ATSCMHIP",  /* [eATSCMHIP]  */
    "ATSCMHFIC", /* [eATSCMHFIC] */
    "LOWSPEEDTS",/* [eLOWSPEEDTS]*/
};

static struct DibDebugMode debugMode = { 0xFFFF0000, 0, 0x80 };

#ifdef USEFIFO
void InitFifo();
void PushToFifo(unsigned char *buffer, unsigned int size, unsigned char fifoid);
#endif

void DisplayStandards(uint32_t Standards)
{
   uint32_t Std = 1;
   for(Std = 1; Std < eMAX_STANDARD_DEMOD; Std++)
   {
      if((1 << Std) & Standards) printf("%s ", StdString[Std]);
   }
}

void DisplayStreamOptions(uint32_t StreamOptions)
{
   uint32_t Opt = 0;
   for(Opt = 0; Opt < 4; Opt++)
   {
      if((1 << Opt) & StreamOptions) printf("%s ", StreamOptionString[Opt]);
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

/*
 * Remove for every item of a filter
 */
void DtaRemoveItemsFromFilter(uint32_t FilterId)
{
   int32_t ItemId, DataType;
   DataType = FilterArray[FilterId].DataType;
   for(ItemId = 0; ItemId < DIB_MAX_NB_ITEMS; ItemId++)
   {
      if(ItemArray[ItemId].Used && (ItemArray[ItemId].ParentFilter == FilterId))
      {
         ItemArray[ItemId].Used = 0;
      }
   }
}

/*
 * Remove for every filters of a channel
 */
void DtaRemoveFiltersFromChannel(uint32_t ChannelId)
{
   int32_t FilterId;

   /* remove every PidCounter of this channel */
   for(FilterId = 0; FilterId < DIB_MAX_NB_FILTERS; FilterId++)
   {
      if(FilterArray[FilterId].Used && (FilterArray[FilterId].ParentChannel == ChannelId))
      {
         DtaRemoveItemsFromFilter(FilterId);

         FilterArray[FilterId].Used = 0;
      }
   }
}

/*
 * Remove for every channel of a stream
 */
void DtaRemoveChannelsFromStream(uint32_t StreamId)
{
   int32_t ChannelId;

   /* remove every PidCounter of this channel */
   for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
   {
      if(ChannelArray[ChannelId].Used && (ChannelArray[ChannelId].StreamNum == StreamId))
      {
         DtaRemoveFiltersFromChannel(ChannelId);

         ChannelArray[ChannelId].Used = 0;
      }
   }
}

/*
 * Remove for every channel of a stream
 */
void DtaRemoveFrontendsFromStream(uint32_t StreamId)
{
   uint32_t FeNum;

   /* remove every PidCounter of this channel */
   for(FeNum = 0; FeNum < DIB_MAX_NB_OF_FRONTENDS; FeNum++)
   {
      if(FrontendArray[FeNum].Used && (FrontendArray[FeNum].StreamNum == StreamId))
      {
         FrontendArray[FeNum].Used = 0;
      }
   }
}

/*
 * Remove for every buffers of a filter
 */
void DtaRemoveBuffersFromFilter(uint32_t FilterId)
{
   int32_t ItemId, DataType;
   DataType = FilterArray[FilterId].DataType;
   for(ItemId = 0; ItemId < DIB_MAX_NB_ITEMS; ItemId++)
   {
      if(ItemArray[ItemId].Used && (ItemArray[ItemId].ParentFilter == FilterId))
      {
#if (DIB_SIPSI_DATA == 1)
         if(DataType == eSIPSI)
            DtaRemoveBuffersFromSipsiItem(ItemArray[ItemId].Config.SiPsi.Pid, ItemId);
#endif
      }
   }
}

/*
 * Remove for every buffers of a channel
 */
void DtaRemoveBuffersFromChannel(uint32_t ChannelId)
{
   int32_t FilterId;

   /* remove every PidCounter of this channel */
   for(FilterId = 0; FilterId < DIB_MAX_NB_FILTERS; FilterId++)
   {
      if(FilterArray[FilterId].Used && (FilterArray[FilterId].ParentChannel == ChannelId))
      {
         DtaRemoveBuffersFromFilter(FilterId);
      }
   }
}

/*
 * Remove for every buffers of a stream
 */
void DtaRemoveBuffersFromStream(uint32_t StreamId)
{
   int32_t ChannelId;

   /* remove every PidCounter of this channel */
   for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
   {
      if(ChannelArray[ChannelId].Used && (ChannelArray[ChannelId].StreamNum == StreamId))
      {
         DtaRemoveBuffersFromChannel(ChannelId);
      }
   }
}


void DtaDataCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
   int ii;
   if(CurrentTsMonit != DIB_UNSET)
   {
      if(pBuffCtx->BufSize % TP_SIZE_BYTES != 0)
      {
         gTsDataInfo[CurrentTsMonit].BufferError++;
      }else{
         uint32_t size = pBuffCtx->BufSize;
         uint8_t * pBuf = pBuffCtx->BufAdd;
         uint32_t Pid;
         uint8_t Cc = 0;
         while(size != 0)
         {
            uint8_t CcTestEnable = 0;
            if(pBuf[0] != 0x47)
            {
               gTsDataInfo[CurrentTsMonit].CorruptedPacketsCount++;
            }
            if((pBuf[1]&0x80) != 0)
            {
               gTsDataInfo[CurrentTsMonit].ErrorIndicFlag++;
            }
            Pid  = (pBuf[1] & 0x1F) << 8;
            Pid |= pBuf[2];
            for(ii = 0; ii < MAX_PID_BY_CTX; ii++)
            {
               if(gTsDataInfo[CurrentTsMonit].PidsVal[ii] == Pid)
               {
                  CcTestEnable = 1;
                  break;
               }
               if(gTsDataInfo[CurrentTsMonit].PidsVal[ii] == 0xFFFF)
               {
                  gTsDataInfo[CurrentTsMonit].PidsVal[ii] = Pid;
                  Cc = pBuf[3] & 0x0f;
                  PidsCc[ii] = (Cc + 1) & 0x0f;
                  break;
               }
            }
            if(Pid != 8191)
            {
               if(CcTestEnable == 1)
               {
                  Cc = pBuf[3] & 0x0f;
                  if(PidsCc[ii] != Cc)
                  {
                     gTsDataInfo[CurrentTsMonit].DiscontinuitiesCount++;
                     gTsDataInfo[CurrentTsMonit].PidsCCCount[ii]++;
                  }else{
                     gTsDataInfo[CurrentTsMonit].TramesOk++;
                  }
                  PidsCc[ii] = (Cc + 1) & 0x0f;
               }
            }else{
               gTsDataInfo[CurrentTsMonit].NullPackets++;
            }

            pBuf += TP_SIZE_BYTES;
            size -= TP_SIZE_BYTES;
         }
      }
   }
   else
   {
      printf(CRB "DtaDataCallback Filter: %d, Size: %d,  %#x %#x %#x %#x"
        CRA,
      pBuffCtx->FilterIdx, pBuffCtx->BufSize ,pBuffCtx->BufAdd[0],
      pBuffCtx->BufAdd[1], pBuffCtx->BufAdd[2], pBuffCtx->BufAdd[3]);
   }

   if(TsDataRecord == 1)
   {
     if (File != NULL)
     {
       fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
     }
   }

   if((!StopDataRunning) && (Status != eREMOVED))
   {
      DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, &TsDataBuff[0], BUFF_MAX_DTA, 0);
   }
}

/**
 * DtaPesVideoCallback ()
 */
void DtaPesVideoCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
   FILE *File;
   DIBSTATUS rc;
   if((File= fopen("video_driver.pes", "ab"))== NULL)
   {
      printf("ERROR Cannot open file video_driver");
   }
   fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
/*   printf(CRB "-- Writing in video_driver.pes size=%d %02x %02x %02x %02x" CRA, pBuffCtx->BufSize,pBuffCtx->BufAdd[0],pBuffCtx->BufAdd[1],pBuffCtx->BufAdd[2],pBuffCtx->BufAdd[3]);*/
   fclose(File);

   if (Status != eREMOVED)
   {
     rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, BUFF_MAX_DTA, pBuffCtx->BufId);
     if(rc != DIBSTATUS_SUCCESS)
     {
        printf(CRB "CLBACK PES Video: RegBuf id %d error: rc %d " CRA, pBuffCtx->BufId, rc);
     }
   }
}

/**
 * DtaPesAudioCallback ()
 */
void DtaPesAudioCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
   FILE *File;
   DIBSTATUS rc;

   if((File= fopen("audio_driver.pes", "ab"))== NULL)
   {
      printf("ERROR Cannot open file audio_driver");
   }
   fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
 /*  printf(CRB "-- Writing in audio_driver.pes size=%d %02x %02x %02x %02x %02x %02x %02x %02x" CRA, pBuffCtx->BufSize,pBuffCtx->BufAdd[0],pBuffCtx->BufAdd[1],pBuffCtx->BufAdd[2],pBuffCtx->BufAdd[3],pBuffCtx->BufAdd[4],pBuffCtx->BufAdd[5],pBuffCtx->BufAdd[6],pBuffCtx->BufAdd[7]);*/
   fclose(File);

   if (Status != eREMOVED)
   {
     rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, BUFF_MAX_DTA, pBuffCtx->BufId);
     if(rc != DIBSTATUS_SUCCESS)
     {
        printf(CRB "CLBACK PES Audio: RegBuf id %d error: rc %d " CRA, pBuffCtx->BufId, rc);
     }
   }
}
/**
 * DtaPesOtherCallback ()
 */
void DtaPesOtherCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
   FILE *File;
   DIBSTATUS rc;

   if((File= fopen("other_driver.pes", "ab"))== NULL)
   {
      printf("ERROR Cannot open file other_driver");
   }
   fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
 /* printf(CRB "-- Writing in other_driver.pes size=%d %02x %02x %02x %02x %02x %02x %02x %02x" CRA, pBuffCtx->BufSize,pBuffCtx->BufAdd[0],pBuffCtx->BufAdd[1],pBuffCtx->BufAdd[2],pBuffCtx->BufAdd[3],pBuffCtx->BufAdd[4],pBuffCtx->BufAdd[5],pBuffCtx->BufAdd[6],pBuffCtx->BufAdd[7]);*/
   fclose(File);

   if (Status != eREMOVED)
   {
     rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, BUFF_MAX_DTA, pBuffCtx->BufId);
     if(rc != DIBSTATUS_SUCCESS)
     {
        printf(CRB "CLBACK PES Other: RegBuf id %d error: rc %d " CRA, pBuffCtx->BufId, rc);
     }
   }
}

/**
 * DtaFigCallback
 */
void DtaFigCallback(void * dataContext, struct DibBufContext * pBuffCtx, enum DibDataStatus Status)
{
#if (DIB_FIG_DATA == 1)
  /* FILE * File = pFigFile;
   uint32_t length = 0, ii = 0;*/
   DIBSTATUS rc;

/*   File = fopen("fig.dab", "ab");

   if(File != NULL)
   {
      fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
      fclose(File);
   }
*/
   /*File = fopen("fig2.fig", "a");
   if(File != NULL)
   {
      while(length < pBuffCtx->BufSize)
      {
         for(ii = 0; ii < 30; ii++)
         {
            fprintf(File, "%#02x ", pBuffCtx->BufAdd[length]);
            length++;
         }
         fprintf(File, "\n");
      }
      fflush(File);
   }*/
/*   printf(CRB "DtaFigCallback Filter: %d, Size: %d,  %#x %#x %#x %#x (%d)" CRA, pBuffCtx->FilterIdx, pBuffCtx->BufSize ,pBuffCtx->BufAdd[0], pBuffCtx->BufAdd[1], pBuffCtx->BufAdd[2], pBuffCtx->BufAdd[3], Status);*/
   if(Status != eREMOVED)
   {
      /* TODO : adjust the size */
     rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, BUFF_MAX_DTA, pBuffCtx->BufId);
     if(rc != DIBSTATUS_SUCCESS)
     {
        printf(CRB "CLBACK FIG : RegBuf id %d error: rc %d " CRA, pBuffCtx->BufId, rc);
     }
   }
#endif
}

/**
 * DtaTdmbCallback()
 */
void DtaTdmbCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
#if (DIB_TDMB_DATA == 1)
   FILE * File = pTdmbFile;
   DIBSTATUS rc;
   char * FileName;

   if(TsDataRecord != 0)
   {
      FileName = (char *)malloc((strlen("tdmb.ts")+4)*sizeof(char));
      sprintf(FileName, "%s_%d", "tdmb.ts", pBuffCtx->FilterIdx);
      File = fopen(FileName, "ab");
      if(File != NULL)
      {
         fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
         fclose(File);
      }
   }

   if(Status != eREMOVED)
   {
#ifdef USEFIFO
     PushToFifo(pBuffCtx->BufAdd,pBuffCtx->BufSize,pBuffCtx->FilterIdx);
#endif

     rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, BUFF_MAX_DTA, pBuffCtx->BufId);
     if(rc != DIBSTATUS_SUCCESS)
     {
        printf(CRB "CLBACK Tdmb : RegBuf id %d error: rc %d " CRA, pBuffCtx->BufId, rc);
     }
   }
#endif
}

#if (DIB_CMMB_DATA == 1)
/**
 * DtaCmmbMultiplexFrameCallback
 */
void DtaCmmbMultiplexFrameCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
   /* Multiplex Frame is available at 'pBuffCtx->BufAdd', number of bytes is 'pBuffCtx->BufSize'. */
   FILE * File;
   DIBSTATUS rc;

   if(TsDataRecord != 0)
   {
      File = fopen("cmmb.mfs", "ab");
      if(File != NULL)
      {
         fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
         fclose(File);
      }
   }

   if(Status != eREMOVED)
   {
      rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, BUFF_MAX_DTA, pBuffCtx->BufId);
      if(rc != DIBSTATUS_SUCCESS)
      {
         printf(CRB "CLBACK CMMB Multiplex Frame: RegBuf id %d error: rc %d" CRA, pBuffCtx->BufId, rc);
      }
   }
}
#endif /* DIB_CMMB_DATA */

void DtaMultiplexFrameCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
   /* Multiplex Frame is available at 'pBuffCtx->BufAdd', number of bytes is 'pBuffCtx->BufSize'. */
   FILE * File;
   DIBSTATUS rc;

   if(TsDataRecord != 0)
   {
      File = fopen("mulpframe.log", "ab");
      if(File != NULL)
      {
         fwrite(pBuffCtx->BufAdd, 1, pBuffCtx->BufSize, File);
         fclose(File);
      }
   }

   if(Status != eREMOVED)
   {
      rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, BUFF_MAX_DTA, pBuffCtx->BufId);
      if(rc != DIBSTATUS_SUCCESS)
      {
         printf(CRB "CLBACK CMMB Multiplex Frame: RegBuf id %d error: rc %d" CRA, pBuffCtx->BufId, rc);
      }
   }
}
/**
 * Initialize the Driver
 */
int32_t DtaInitSoft(enum DibBoardType BoardType, BOARD_HDL BoardHdl)
{
   DIBSTATUS status;

#if (EMBEDDED_FLASH_SUPPORT == 1)
   status = DibOpenFromFlash(&CONTEXT, BoardType, BoardHdl);
#else
   status = DibOpen(&CONTEXT, BoardType, BoardHdl);
#endif

   if(status != DIBSTATUS_SUCCESS)
   {
      printf(CRB "DibOpen Failed" CRA);
      exit(-1);
   }

   return (status);
}

int32_t DtaTestReinit(enum DibBoardType BoardType)
{
   DIBSTATUS status;

   status = DibClose(CONTEXT);

   DtaInitInternal();

   printf(CRB "Close Driver => rc= %d " CRA, status);

   status = DibOpen(&CONTEXT, BoardType, 0);

   printf(CRB "Open Driver => rc= %d " CRA, status);
   return 0;
}

void DtaDisplayLayoutInfo(void)
{
   uint32_t FeNum, StreamNum, i;
   printf(CRB "Layout Info: " CRA);
   printf(CRB "\tNumberOfChips     = %d" CRA, globalInfo.NumberOfChips);
   printf(CRB "\tNumberOfFrontends = %d" CRA, globalInfo.NumberOfFrontends);
   printf(CRB "\tNumberOfStreams   = %d" CRA, globalInfo.NumberOfStreams);
   printf(CRB "\n\tStream attributes:" CRA);
   for(StreamNum = 0; StreamNum < globalInfo.NumberOfStreams; StreamNum++)
   {
      DibGetStreamAttributes(CONTEXT, globalInfo.Stream[StreamNum], &StreamArray[StreamNum].Attr);
      printf(CRB "\n\t\t StNum %3d Hdl %p Type = %10s ChipId %3d Options : ",
            StreamNum, globalInfo.Stream[StreamNum], StreamTypeString[StreamArray[StreamNum].Attr.Type],
            StreamArray[StreamNum].Attr.ChipId);
      DisplayStreamOptions(StreamArray[StreamNum].Attr.Options);
      printf(CRB " Std: ");
      DisplayStandards(StreamArray[StreamNum].Attr.Standards);
      printf(CRA);
      i = 0;
      while(StreamArray[StreamNum].Attr.PossibleDirectInputs[i].Fe && i < DIB_MAX_NB_OF_FRONTENDS)
      {
         printf(CRB "\t\t          Input = FeNum %d (FeHdl %p) Stds = ",
               GetFrontendNumByHandle(StreamArray[StreamNum].Attr.PossibleDirectInputs[i].Fe),
               StreamArray[StreamNum].Attr.PossibleDirectInputs[i].Fe);
         DisplayStandards(StreamArray[StreamNum].Attr.PossibleDirectInputs[i].Standard);
         printf(CRA);
         i++;
      }
   }
   printf(CRB "\n\tFrontend attributes:" CRA);
   for(FeNum = 0; FeNum < globalInfo.NumberOfFrontends; FeNum++)
   {
      DibGetFrontendAttributes(CONTEXT, globalInfo.Frontend[FeNum], &FrontendArray[FeNum].Attr);
      printf(CRB "\n\t\t FeNum %3d Hdl %p ChipId %3d FeId %3d" CRA,
            FeNum, globalInfo.Frontend[FeNum], FrontendArray[FeNum].Attr.ChipId, FrontendArray[FeNum].Attr.FeId);
      printf(CRB "\t\t          Std = ");
      DisplayStandards(FrontendArray[FeNum].Attr.Standards);
      printf(CRA);

      i = 0;
      while(FrontendArray[FeNum].Attr.PossibleDirectInputs[i].Fe)
      {
         printf(CRB "\t\t            Input = FeNum %d (FeHdl %p) Stds = ",
               GetFrontendNumByHandle(FrontendArray[FeNum].Attr.PossibleDirectInputs[i].Fe),
               FrontendArray[FeNum].Attr.PossibleDirectInputs[i].Fe);
         DisplayStandards(FrontendArray[FeNum].Attr.PossibleDirectInputs[i].Standard);
         printf(CRA);
         i++;
      }
   }
}

/**
 * DtaInit
 */
int32_t DtaInit(void)
{
   int32_t  rc = 0;
   uint32_t ChannelId;

   /*  Get driver version */
   if((rc = DibGetGlobalInfo(CONTEXT, &globalInfo)) == DIBSTATUS_SUCCESS)
   {
      printf(CRB "Driver ver: %d.%d.%d" CRA, DIB_MAJOR_VER(globalInfo.DriverVersion), DIB_MINOR_VER(globalInfo.DriverVersion), DIB_REVISION(globalInfo.DriverVersion));
      printf(CRB "Embedded SW ver: %d.%02d (IC: %d)" CRA, (globalInfo.EmbVersions[0] >> 10), globalInfo.EmbVersions[0] & 0x03ff, globalInfo.EmbVersions[1]);
      printf(CRB "HAL version: %d.%d.%d%s" CRA, globalInfo.HalVersion >> 24,
              (globalInfo.HalVersion >> 12) & 0xfff,
              (globalInfo.HalVersion      ) & 0xfff,
              globalInfo.HalVersionExt);

      DtaInitInternal();

      DtaDisplayLayoutInfo();

      for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
      {
         DtaResetAllSiPsiCounters(ChannelId);
         DtaNbSiPsiRunning[ChannelId] = 0;
      }

      DtaInitTuneArray();
      DtaTsMonitInit();
      DtaTimeLogInit();
   }
   else
   {
      printf(CRB "Getting Global info failed." CRA);
      DibClose(CONTEXT);
      exit(-1);
   }

   return (rc);
}

/**
 * DtaDeInit
 */
int32_t DtaDeInit(void)
{
   DibClose(CONTEXT);
   return (0);
}

/**
 * DtaTestEmbVersion
 */
int32_t DtaTestEmbVersion(uint32_t Nb )
{
   int32_t  rc = 0;
   uint32_t  i;

   struct DibGlobalInfo globalInfo;

   for(i=0; i<Nb; i++)
   {
     memset(&globalInfo,0,sizeof(struct DibGlobalInfo));

     rc = DibGetGlobalInfo(CONTEXT, &globalInfo);

     if (rc != DIBSTATUS_SUCCESS)
     {
       printf("Error Embededded Version !\n");
       return 0;
     }

     printf(CRB "Driver ver: %d.%d.%d" CRA, DIB_MAJOR_VER(globalInfo.DriverVersion), DIB_MINOR_VER(globalInfo.DriverVersion), DIB_REVISION(globalInfo.DriverVersion));
     printf(CRB "Embedded SW ver: %d.%02d (IC: %d)" CRA, (globalInfo.EmbVersions[0] >> 10), globalInfo.EmbVersions[0] & 0x03ff, globalInfo.EmbVersions[1]);
   }

   return (rc);
}

/**
 * DtaDeInit
 */
/**
 * DtaSetDmsk
 */
void DtaSetDmsk(uint8_t Mask)
{
   debugMode.EmbeddedDebugMask = Mask;

   printf(CRB "---------------------------------------" CRA);
   printf(CRB "DMASK is now %02X" CRA, Mask);
   printf(CRB "---------------------------------------" CRA);

   DibSetConfig(CONTEXT, eDEBUG_MODE, (union DibParamConfig *)&debugMode);
}

/**
 * DtaSetDhelp
 */
void DtaSetDhelp(uint16_t i, uint16_t v, uint16_t ch)
{
   debugMode.DHelp = (v << 16) | i;
   debugMode.ChipId = ch;

   DibSetConfig(CONTEXT, eDEBUG_MODE, (union DibParamConfig *)&debugMode);
}

/**
 * DtaSetDriverDebugMask
 */
void DtaSetDriverDebugMask(uint32_t Mask)
{
   debugMode.DriverDebugMask = Mask;

   DibSetConfig(CONTEXT, eDEBUG_MODE, (union DibParamConfig *)&debugMode);
}

/**
 * DtaGetPowerMode
 */
int32_t DtaGetPowerMode(uint32_t StreamNum)
{
   if(StreamNum < globalInfo.NumberOfStreams)
   {
      union DibParamConfig ParamConfig;

      ParamConfig.TimeSlicing.PhysicalStream = StreamArray[StreamNum].Hdl;
      DibGetConfig(CONTEXT, eTIMESLICING, &ParamConfig);
      printf("TimeSlicing is %s on Stream Num %d\n", ParamConfig.TimeSlicing.Enable ? "ENABLED" :"DISABLED", StreamNum);
      return ParamConfig.TimeSlicing.Enable;
   }
   else
   {
      printf("-E Bad Stream number\n");
      return -1;
   }
}

int32_t DtaSetPowerMode(enum DibBool TimeSlicing, uint32_t StreamNum)
{
   DIBSTATUS rc;
   union DibParamConfig ParamConfig;

   ParamConfig.TimeSlicing.Enable = TimeSlicing;
   ParamConfig.TimeSlicing.PhysicalStream = (STREAM_HDL)StreamArray[StreamNum].Hdl;

   rc = DibSetConfig(CONTEXT, eTIMESLICING, &ParamConfig);
   if(rc != DIBSTATUS_SUCCESS)
   {
      printf(CRB "-E Cannot Change TimeSlicing !" CRA);
      return rc;
   }
   printf(CRB "TimeSlicing %s for stream %d" CRA, TimeSlicing ? "ON" : "OFF", StreamNum);
   return (int32_t)rc;
}

/**
 * DtaSetPowerMode
 */
int32_t DtaSetGlobalPowerMode(enum DibBool TimeSlicing)
{
   DIBSTATUS rc;
   uint32_t StreamNum;

   for(StreamNum = 0; StreamNum < globalInfo.NumberOfStreams; StreamNum++)
   {
      rc = DtaSetPowerMode(TimeSlicing, StreamNum);
      if(rc != DIBSTATUS_SUCCESS)
         break;
   }

   return (int32_t)rc;
}

/**
 * DtaSetHbmMode
 */
int32_t DtaSetHbmMode(enum DibHbmMode HbmMode)
{
   DIBSTATUS rc;
   union DibParamConfig ParamConfig;

   ParamConfig.HbmMode = HbmMode;

   rc = DibSetConfig(CONTEXT, eSET_HBM_MODE, &ParamConfig);
   if(rc != DIBSTATUS_SUCCESS)
   {
      printf(CRB "-E Change HbmMode" CRA);
      return rc;
   }
   return (int32_t)rc;
}


/**
 * DtaSetPrefetch
 */
int32_t DtaSetPrefetch(ELEM_HDL ElemHdl, uint16_t Prefetch)
{
   int32_t rc;
   union DibParamConfig ParamConfig;

   if((Prefetch != 0) && (Prefetch != 1))
      return DIBSTATUS_INVALID_PARAMETER;

   ParamConfig.PidMode.ElemHdl = ElemHdl;
   ParamConfig.PidMode.Mode = (enum DibPidMode)Prefetch;

   rc = (int32_t)DibSetConfig(CONTEXT, ePID_MODE, &ParamConfig);
   return (rc);
}


/**
 * DtaGetChannelCallback
 */
struct GetChannelContext
{
   volatile int32_t    GetChannelFinished;
   struct DibTuneChan *GetChannelInfo;
};

void DtaGetChannelCallback(struct DibTuneChan *pChannelDescriptor, void * pContext)
{
   struct GetChannelContext *gcc = (struct GetChannelContext *)pContext;

   *(gcc->GetChannelInfo) = *pChannelDescriptor;
   gcc->GetChannelFinished= 1;
   printf(CRB "DtaTuneMonitCallback" CRA);
}

/**
 * Stream allocation
 */
DIBSTATUS DtaGetStream(uint32_t StreamNum, uint32_t Std, uint32_t Options, uint32_t EnPowerSaving)
{
   DIBSTATUS Status;

   if(StreamArray[StreamNum].Used)
   {
      printf(CRB "Stream Identifier already used" CRA);
      return DIBSTATUS_ERROR;
   }

   Status = DibGetStream(CONTEXT, StreamArray[StreamNum].Hdl, (enum DibDemodType)Std, Options, (enum DibPowerMode)EnPowerSaving);

   if(Status == DIBSTATUS_SUCCESS)
   {
      StreamArray[StreamNum].Used = 1;
      StreamArray[StreamNum].Std = Std;
      StreamArray[StreamNum].Options = Options;

      printf(CRB "---------- STREAM NUM %d ADDED ------------" CRA, StreamNum);
   }
   else
   {
      printf(CRB "GET STREAM FAILED (status = %d)" CRA, Status);
   }

   return Status;
}

/**
 * Stream desallocation
 */
DIBSTATUS DtaDeleteStream(uint32_t StreamNum)
{
   DIBSTATUS Status;

   if(StreamArray[StreamNum].Used == 0)
   {
      printf(CRB "Stream Identifier was not used" CRA);
      return DIBSTATUS_ERROR;
   }

   DtaRemoveBuffersFromStream(StreamNum);

   Status = DibDeleteStream(CONTEXT, StreamArray[StreamNum].Hdl);

   if(Status == DIBSTATUS_SUCCESS)
   {
     /* Remove every channel of this stream */
      DtaRemoveFrontendsFromStream(StreamNum);
      DtaRemoveChannelsFromStream(StreamNum);

      StreamArray[StreamNum].Used = 0;
      StreamArray[StreamNum].Std = eSTANDARD_UNKNOWN;
      StreamArray[StreamNum].Options = 0;

       printf(CRB "---------- STREAM NUM %d REMOVED ------------" CRA, StreamNum);
   }
   else
   {
       printf(CRB "-------- STREAM REMOVING FAILED %d ---------" CRA, StreamNum);
   }

   return Status;
}

/**
 * Frontend allocation
 */
DIBSTATUS DtaGetFrontend(uint32_t StreamNum, uint32_t FeNum, uint32_t OutputFeNum, uint32_t Force)
{
   DIBSTATUS Status;
   FRONTEND_HDL OutputFeHdl = NULL;

   if(FrontendArray[FeNum].Used)
   {
      printf(CRB "Fe Identifier already used" CRA);
      return DIBSTATUS_ERROR;
   }

   if(StreamArray[StreamNum].Used == 0)
   {
      printf(CRB "Stream Identifier not found" CRA);
      return DIBSTATUS_ERROR;
   }

   if(OutputFeNum != -1)
   {
      if(FrontendArray[OutputFeNum].Used == 0)
      {
         printf(CRB "Output Fe Identifier not found" CRA);
         return DIBSTATUS_ERROR;
      }

      if(FrontendArray[OutputFeNum].StreamNum != StreamNum)
      {
         printf(CRB "Output Fe not connected to the same stream" CRA);
         return DIBSTATUS_ERROR;
      }

      OutputFeHdl = FrontendArray[OutputFeNum].Hdl;
   }

   Status = DibAddFrontend(CONTEXT, StreamArray[StreamNum].Hdl, FrontendArray[FeNum].Hdl, OutputFeHdl, Force);

   if(Status == DIBSTATUS_SUCCESS)
   {
      FrontendArray[FeNum].Used = 1;
      FrontendArray[FeNum].StreamNum = StreamNum;
      printf(CRB "---------- FRONTEND NUM %d ADDED ------------" CRA, FeNum);
   }
   else
   {
      printf(CRB "---------- GET FRONTEND FAILED (status = %d) ------------" CRA, Status);
   }

   return Status;
}

/**
 * Frontend desallocation
 */
DIBSTATUS DtaDeleteFrontend(uint32_t FeNum)
{
   DIBSTATUS Status;

   if(FrontendArray[FeNum].Used == 0)
   {
      printf(CRB "Frontend Identifier was not used" CRA);
      return DIBSTATUS_ERROR;
   }

   Status = DibRemoveFrontend(CONTEXT, FrontendArray[FeNum].Hdl);

   if(Status == DIBSTATUS_SUCCESS)
   {
      FrontendArray[FeNum].Used = 0;
      printf(CRB "---------- FRONTEND NUM %d REMOVED ------------" CRA, FeNum);
   }
   else
   {
      printf(CRB "---------- FRONTEND NUM %d REMOVED FAILED ------------" CRA, FeNum);
   }

   return Status;
}


/**
 * DtaGetChannel
 */
DIBSTATUS DtaGetChannel(uint32_t ChannelId, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParam, enum DibDemodType TypeDemod, uint32_t Frequency, uint16_t BW, uint8_t Async, uint8_t Flag)
{
   CHANNEL_HDL Hdl;
   DIBSTATUS   Status;
   struct DibChannel    *ChDesc;
   struct GetChannelContext gcc;
   struct DibTuneChan   *pTuneChan;

   if(Async)
   {
      gcc.GetChannelFinished = 0;
      gcc.GetChannelInfo     = &ChannelArray[ChannelId].Info.ChannelDescriptor;
   }

   if(ChannelArray[ChannelId].Used)
   {
      printf(CRB "Channel Identifier already used" CRA);
      return DIBSTATUS_ERROR;
   }

   ChDesc = &ChannelArray[ChannelId].Info;
   pTuneChan = &ChDesc->ChannelDescriptor;

   ChDesc->Type                     =  TypeDemod;
   pTuneChan->Bw                    = (enum DibSpectrumBW)  BW;
   pTuneChan->RFkHz                 =  Frequency;
   pTuneChan->InvSpec               = (enum DibSpectrumInv)  -1;

   /* Set modulation parameters. */
   switch(Flag)
   {
   /* Unknown parameters. */
   case SCAN_FLAG_AUTOSEARCH:
      printf(CRB "Get Channel with autosearch" CRA);
      memset(&pTuneChan->Std, -1, sizeof(pTuneChan->Std));
      if (TypeDemod == eSTANDARD_ISDBT_1SEG) {
          pTuneChan->Std.Isdbt.SbMode            = 1;
          pTuneChan->Std.Isdbt.PartialReception  = 0; /* tune in 1 seg not in 3 seg */
      }
      break;

   /* Lookup frequency and bandwidth in TuneChanArray[]. */
   case SCAN_FLAG_PLAIN_TUNE:
      if(GetTuneChan(Frequency, BW, pTuneChan) == DTA_NO_INDEX)
      {
         printf(CRB "Get Channel by bypassing the tune" CRA);
         /* Pass firmware 0 parameters to avoid the tune. This behaviour can be needed
          * to tune on a slave tuner if no input is connected to the master firmware */
         memset(&pTuneChan->Std, 0, sizeof(pTuneChan->Std));
      }
      else
      {
         printf(CRB "Get Channel with well known parameters" CRA);
      }
      break;

   default:
      printf(CRB "%s: *** Error: Illegal scan flag (%d)" CRA, __FUNCTION__, Flag);
      return DIBSTATUS_ERROR;
      break;
   }

   Status = DibGetChannel(CONTEXT, MinNbDemod, MaxNbDemod, StreamParam, ChDesc, &Hdl, NULL, NULL);
   if(Status == DIBSTATUS_SUCCESS)
   {
      if(Async)
      {
         while(gcc.GetChannelFinished == 0)
         {
            OsMSleep(100);
         }
      }
      DtaShowChannelParams(&(ChDesc->ChannelDescriptor), ChDesc->Type, DTA_FULL_DISPLAY);

      printf(CRB "" CRA);
      printf(CRB "---------- CHANNEL HDL %d ADDED ------------" CRA, Hdl);
      ChannelArray[ChannelId].Used=1;
      ChannelArray[ChannelId].ChannelHdl=Hdl;
   }
   else
   {
      if(Status == DIBSTATUS_INVALID_PARAMETER){
         printf(CRB "INVALID PARAM" CRA);
      }
      else
      {
         if(Status == DIBSTATUS_RESOURCES)
         {
            printf(CRB "" CRA);
            printf("RESSOURCES PB" CRA);
         }
         else
         {
            printf(CRB "" CRA);
            printf(CRB "CREATE CHANNEL FAILED (status = %d) ScanStatus=%d" CRA, Status, ChDesc->ChannelDescriptor.ScanStatus );
         }
      }
   }

   if(ChDesc->ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)
   {
      /* In case of successful tune, set channel monitoring capabilities */
      DtaSetMonitArray(ChannelId);
   }

   /* Saves off modulation parameters and updates statistics on global tune attempts. */
   DtaSetTuneChan(&(ChDesc->ChannelDescriptor));
   return Status;
}

/**
 * DtaGetChannelEx
 */

DIBSTATUS DtaGetChannelEx(uint32_t ChannelId, uint32_t StreamNum, struct DibTuneChan *pChannelInfo, uint8_t Async, uint8_t Flag)
{
   CHANNEL_HDL Hdl;
   DIBSTATUS   Status;
   struct DibChannel    *pDibChannel;
   struct GetChannelContext gcc;
   struct DibTuneChan   *pTuneChan;
   struct DtaStream * pStream;

   if(Async)
   {
      gcc.GetChannelFinished = 0;
      gcc.GetChannelInfo     = &ChannelArray[ChannelId].Info.ChannelDescriptor;
   }

   if(ChannelArray[ChannelId].Used)
   {
      printf(CRB "Channel Identifier already used" CRA);
      return DIBSTATUS_ERROR;
   }

   pStream = &StreamArray[StreamNum];
   if(pStream->Used == 0)
   {
      printf(CRB "Stream %d not found" CRA, StreamNum);
      return DIBSTATUS_ERROR;
   }

   pDibChannel = &ChannelArray[ChannelId].Info;

   pTuneChan = &pDibChannel->ChannelDescriptor;

   pDibChannel->Type = pStream->Std;

   /* Fast scan flag */
   if(Flag == SCAN_FLAG_FAST_SEARCH)
      pDibChannel->Type |= 0x80;

   /* Force single carrier */
   if(Flag == SCAN_FLAG_ALWAYS_TUNE_SUCCESS)
      pDibChannel->Type |= 0x40;

   /* Force auto-retune */
   if(Flag == SCAN_FLAG_AUTOTUNE)
      pDibChannel->Type |= 0x20;

   memcpy(pTuneChan, pChannelInfo, sizeof(struct DibTuneChan));

   Status = DibGetChannelEx(CONTEXT, pStream->Hdl, pDibChannel, &Hdl, NULL, NULL);
   if(Status == DIBSTATUS_SUCCESS)
   {
      if(Async)
      {
         while(gcc.GetChannelFinished == 0)
         {
            OsMSleep(100);
         }
      }
      DtaShowChannelParams(&pDibChannel->ChannelDescriptor, pDibChannel->Type, DTA_FULL_DISPLAY);

      printf(CRB "" CRA);
      ChannelArray[ChannelId].Used=1;
      ChannelArray[ChannelId].ChannelHdl=Hdl;
      ChannelArray[ChannelId].StreamNum = StreamNum;
      printf(CRB "---------- CHANNEL HDL %d ADDED ------------" CRA, Hdl);
   }
   else
   {
      if(Status == DIBSTATUS_INVALID_PARAMETER){
         printf(CRB "INVALID PARAM" CRA);
      }
      else
      {
         if(Status == DIBSTATUS_RESOURCES)
         {
            printf(CRB "" CRA);
            printf("RESSOURCES PB" CRA);
         }
         else
         {
            printf(CRB "" CRA);
            printf(CRB "CREATE CHANNEL FAILED (status = %d) ScanStatus=%d" CRA, Status, pDibChannel->ChannelDescriptor.ScanStatus );
         }
      }
   }

   if(pDibChannel->ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)
   {
      /* In case of successful tune, set channel monitoring capabilities */
      DtaSetMonitArray(ChannelId);
   }

   /* Saves off modulation parameters and updates statistics on global tune attempts. */
   DtaSetTuneChan(&pDibChannel->ChannelDescriptor);

   return Status;
}

/**
 * DtaDeleteChannel
 */
DIBSTATUS DtaDeleteChannel(uint32_t ChannelId)
{
   DIBSTATUS Status;

   if(!ChannelArray[ChannelId].Used)
   {
      printf(CRB "DELETE CHANNEL FAILED: Channel not used" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* remove every PidCounter of this channel */
   DtaRemoveBuffersFromChannel(ChannelId);

   Status = DibDeleteChannel(CONTEXT, ChannelArray[ChannelId].ChannelHdl);

   if(Status == DIBSTATUS_SUCCESS)
   {
      /* delete every item and filters linked to this channel */
      DtaRemoveFiltersFromChannel(ChannelId);

      /** all counter of this channel should have been removed */
      printf(CRB "" CRA);
      printf(CRB "---------- CHANNEL HDL %d REMOVED ------------" CRA, ChannelArray[ChannelId].ChannelHdl);
      ChannelArray[ChannelId].Used=0;
   }
   else
   {
      if(Status == DIBSTATUS_INVALID_PARAMETER)
      {
         printf(CRB "INVALID PARAM" CRA);
      }
      else
      {
         if(Status == DIBSTATUS_RESOURCES)
         {
            printf(CRB "" CRA);
            printf(CRB "RESSOURCES PB" CRA);
         }
         else
         {
            printf(CRB "" CRA);
            printf(CRB "DELETE CHANNEL FAILED" CRA);
         }
      }
   }
   return Status;
}

/**
 * DtaCreateFilter
 */
DIBSTATUS DtaCreateFilter(uint32_t FilterId, uint32_t ChannelId, enum DibDataType DataType, uint8_t async)
{
   DIBSTATUS  Status;
   FILTER_HDL Hdl;
   struct DibDataBuffer info;

   if((ChannelId >= DIB_MAX_NB_CHANNELS) || (FilterId >= DIB_MAX_NB_FILTERS))
   {
      printf(CRB "CREATE FILTER FAILED: ChannelId or FilterId out of range" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   if(FilterArray[FilterId].Used)
   {
      printf(CRB "CREATE FILTER FAILED: Filter Already used" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   if(!ChannelArray[ChannelId].Used)
   {
      printf(CRB "CREATE FILTER FAILED: Channel Not used" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   info.Timeout = DIB_TIMEOUT_IMMEDIATE;

   if(DataType == eSIPSI)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaSiPsiCallback;
   }
   else if(DataType == ePESVIDEO)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaPesVideoCallback;
   }

   else if(DataType == ePESAUDIO)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaPesAudioCallback;
   }
   else if(DataType == ePESOTHER)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaPesOtherCallback;
   }

   else if(DataType == eFIG)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaFigCallback;
   }
   else if(DataType == eTDMB)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaTdmbCallback;
   }
   else if(DataType == eDAB)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaTdmbCallback;
   }
   else if(DataType == eDABPACKET)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaTdmbCallback;
   }
#if (DIB_CMMB_DATA == 1)
   else if(DataType == eCMMBSVC)
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaCmmbMultiplexFrameCallback;
   }
#endif /* DIB_CMMB_DATA */
#if (DIB_ATSCMH_DATA == 1)
   else if((DataType == eATSCMHIP) || (DataType == eATSCMHFIC))
   {
      info.DataMode     = eCLBACK;
      info.CallbackFunc = DtaMultiplexFrameCallback;
   }
#endif /* DIB_CMMB_DATA */
   else
   {
      if(!async)
      {
         info.DataMode     = eIP;
      }
      else
      {
         info.DataMode     = eCLBACK;
         info.CallbackFunc = DtaDataCallback;
      }
/*      info.DataMode     = eCLBACK; */
/*      info.CallbackFunc = DtaDataCallback; */
      StopDataRunning   = 0;
   }

   info.CallbackUserCtx = 0;

   Status = DibCreateFilter(CONTEXT, ChannelArray[ChannelId].ChannelHdl, DataType, &info, &Hdl);

//---fred add delay to fix pid loss
//DibMSleep(1000);


   if(Status == DIBSTATUS_SUCCESS)
   {
      NbDefinedFilters++;
      printf(CRB "" CRA);
      printf(CRB "---------- FILTER HDL %d CREATED (TO CHANNEL HDL %d)------------" CRA, Hdl, ChannelArray[ChannelId].ChannelHdl);

      FilterArray[FilterId].DataType      = DataType;
      FilterArray[FilterId].FilterHdl     = Hdl;
      FilterArray[FilterId].Used          = 1;
      FilterArray[FilterId].ParentChannel = ChannelId;

      if((info.DataMode == eCLBACK) && (DataType != eSIPSI))
      {
         if(DataType == eTS)
           DibRegisterBuffer(CONTEXT, Hdl, &TsDataBuff[0], BUFF_MAX_DTA, 0);
         if(DataType == eLOWSPEEDTS)
           DibRegisterBuffer(CONTEXT, Hdl, &TsDataBuff[0], BUFF_MAX_DTA, 0);
         if(DataType == eMPEFEC)
           DibRegisterBuffer(CONTEXT, Hdl, &TsDataBuff[0], BUFF_MAX_DTA, 0);
         if(DataType == ePESVIDEO)
           DibRegisterBuffer(CONTEXT, Hdl, &PesVideoDataBuff[0], BUFF_MAX_DTA, 0);
         if(DataType == ePESAUDIO)
           DibRegisterBuffer(CONTEXT, Hdl, &PesAudioDataBuff[0], BUFF_MAX_DTA, 0);
         if(DataType == ePESOTHER)
           DibRegisterBuffer(CONTEXT, Hdl, &PesOtherDataBuff[0], BUFF_MAX_DTA, 0);
         if((DataType == eTDMB) || (DataType == eFIG) || (DataType == eDAB) || (DataType == eDABPACKET))
            DibRegisterBuffer(CONTEXT, Hdl, &TsDataBuff[0], BUFF_MAX_DTA, 0);
#if (DIB_CMMB_DATA == 1)
         if(DataType == eCMMBSVC)
           DibRegisterBuffer(CONTEXT, Hdl, &MultiplexFrameBuff[0], BUFF_MAX_DTA, 0);
#endif /* DIB_CMMB_DATA */
#if (DIB_ATSCMH_DATA == 1)
         if((DataType == eATSCMHIP) || (DataType == eATSCMHFIC))
           DibRegisterBuffer(CONTEXT, Hdl, &MultiplexFrameBuff[0], BUFF_MAX_DTA, 0);
#endif
      }
   }
   else
   {
      if(Status == DIBSTATUS_INVALID_PARAMETER)
      {
         printf(CRB "INVALID PARAM" CRA);
      }
      else
      {
         if(Status == DIBSTATUS_RESOURCES)
         {
            printf(CRB "" CRA);
            printf(CRB "RESSOURCES PB" CRA);
         }
         else
         {
            printf(CRB "" CRA);
            printf(CRB "CREATE FILTER FAILED" CRA);
         }
      }
   }
   return Status;
}
/**
 * DtaDeleteFilter
 */
DIBSTATUS DtaDeleteFilter(uint32_t FilterId)
{
   DIBSTATUS Status;

   if(FilterId >= DIB_MAX_NB_FILTERS)
   {
      printf(CRB "DELETE FILTER FAILED: Filter Id out of range" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   if(!FilterArray[FilterId].Used)
   {
      printf(CRB "DELETE FILTER FAILED: Filter not used" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* delete every PID counter of this filter */
   DtaRemoveBuffersFromFilter(FilterId);

   /** then delete the filter at sdk level */
   Status = DibDeleteFilter(CONTEXT, FilterArray[FilterId].FilterHdl);

   if(Status == DIBSTATUS_SUCCESS)
   {
      /* delete every item linked to this filter */
      DtaRemoveItemsFromFilter(FilterId);

      printf(CRB "" CRA);
      printf(CRB "---------- FILTER HDL %d REMOVED ------------" CRA, FilterArray[FilterId].FilterHdl);

      /* now remove the filter */
      FilterArray[FilterId].Used = 0;

      NbDefinedFilters--;

      if(NbDefinedFilters == 0)
         StopDataRunning = 1;
   }
   else
   {
      if(Status == DIBSTATUS_INVALID_PARAMETER)
      {
         printf(CRB "INVALID PARAM" CRA);
      }
      else
      {
         if(Status == DIBSTATUS_RESOURCES)
         {
            printf(CRB "" CRA);
            printf("RESSOURCES PB" CRA);
         }
         else
         {
            printf(CRB "" CRA);
            printf("DELETE FILTER FAILED" CRA);
         }
      }
   }
   return Status;
}

/**
 * DtaAddToFilter
 */
DIBSTATUS DtaAddToFilter(uint32_t ItemId, uint32_t FilterId, union DibFilters *pFilterDescriptor, int32_t filter_type)
{
   DIBSTATUS Status;
   ELEM_HDL Hdl;

   if((ItemId >= DIB_MAX_NB_ITEMS) || (FilterId >= DIB_MAX_NB_FILTERS))
   {
      printf(CRB "" CRA);
      printf(CRB "ADD TO FILTER FAILED: ItemId or FilterId out of range" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   if(ItemArray[ItemId].Used)
   {
      printf(CRB "" CRA);
      printf(CRB "ADD TO FILTER FAILED: Item already used" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   if(!FilterArray[FilterId].Used)
   {
      printf(CRB "" CRA);
      printf(CRB "ADD TO FILTER FAILED: Filter Not Used" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   Status = DibAddToFilter(CONTEXT, FilterArray[FilterId].FilterHdl, 1, pFilterDescriptor, &Hdl);
   if(Status == DIBSTATUS_SUCCESS)
   {
      printf(CRB "" CRA);
      printf(CRB "---------- ITEM %d HDL %d ADDED (TO FILTER HDL %d)------------" CRA, ItemId, Hdl, FilterArray[FilterId].FilterHdl);

      ItemArray[ItemId].Used        =  1;
      ItemArray[ItemId].ItemHdl     =  Hdl;
      ItemArray[ItemId].ParentFilter=  FilterId;
      ItemArray[ItemId].Config      = *pFilterDescriptor;
#if (DIB_SIPSI_DATA == 1)
      if(filter_type == eSIPSI)
         DtaAddSiPsiPidCounter(pFilterDescriptor->SiPsi.Pid, ItemId);
#endif
   }
   else
   {
      if(Status == DIBSTATUS_INVALID_PARAMETER)
      {
         printf(CRB "INVALID PARAM" CRA);
      }
      else
      {
         if(Status == DIBSTATUS_RESOURCES)
         {
            printf(CRB "" CRA);
            printf(CRB "RESSOURCES PB" CRA);
         }
         else
         {
            printf(CRB "" CRA);
            printf(CRB "ADD TO FILTER FAILED" CRA);
         }
      }
   }
   return Status;
}

/**
 * DtaRemoveFromFilter
 */
DIBSTATUS DtaRemoveFromFilter(uint32_t ItemId)
{
   uint32_t FilterId;
   DIBSTATUS Status;

   if(ItemId >= DIB_MAX_NB_ITEMS)
   {
      printf(CRB "REMOVE ITEM: ItemId out of range" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   if(!ItemArray[ItemId].Used)
   {
      printf(CRB "REMOVE ITEM: Item not used" CRA);
      return DIBSTATUS_INVALID_PARAMETER;
   }

   FilterId= ItemArray[ItemId].ParentFilter;

#if (DIB_SIPSI_DATA == 1)
   /** remove Pid counter (with allocated buffers) */
   DtaRemoveBuffersFromSipsiItem(ItemArray[ItemId].Config.SiPsi.Pid, ItemId);
#endif

   /** now we can remove the item from sdk */
   Status = (uint8_t)DibRemoveFromFilter(CONTEXT, ItemArray[ItemId].ItemHdl);

   if(Status == DIBSTATUS_SUCCESS)
   {
      printf(CRB "" CRA);
      printf(CRB "---------- ITEM HDL %d REMOVED ------------" CRA, ItemArray[ItemId].ItemHdl);
      ItemArray[ItemId].Used=0;
   }
   else
   {
      if(Status == DIBSTATUS_INVALID_PARAMETER)
      {
         printf(CRB "INVALID PARAM" CRA);
      }
      else
      {
         if(Status == DIBSTATUS_RESOURCES)
         {
            printf(CRB "" CRA);
            printf(CRB "RESSOURCES PB" CRA);
         }
         else
         {
            printf(CRB "" CRA);
            printf(CRB "REMOVE TO FILTER FAILED" CRA);
         }
      }
   }

   return Status;
}

uint8_t DtaEventSet[eMAX_NB_EVENTS];

void DtaEventCallback(enum DibEvent Event, union DibEventConfig *pEventConfig, void *pContext)
{
   printf(CRB "DtaEventCallback %d %d" CRA, Event, pEventConfig->SipsiLostEvent.ItemHdl);
}

void DtaSetEventCallback(enum DibEvent Event, uint32_t Add)
{
   static uint8_t FirstEvent = 1;
   DIBSTATUS Status;
   DibGetEventCallback pCallback = DtaEventCallback;
   union DibParamConfig Config;

   if(FirstEvent)
   {
      FirstEvent = 0;
      if(Add)
      {
         Status = DibRegisterEventCallback(CONTEXT, pCallback, 0, 1, &Event);
      }
      else
      {
         printf(CRB "configuration not possible" CRA);
      }
   }
   else
   {
      if(DtaEventSet[Event] != Add)
      {
         Config.EventMode.Callback = pCallback;
         Config.EventMode.Event    = Event;
         Config.EventMode.Mode     = (enum DibEventMode) Add;

         DibSetConfig(CONTEXT, eEVENT, (union DibParamConfig *)&Config);

         Status = DibRegisterEventCallback(CONTEXT, pCallback, 0, 1, &Event);

         DtaEventSet[Event] = 1;
      }
      else
      {
         printf(CRB "configuration already current" CRA);
      }
    }
}
#if (DEBUG_LIB == 1)

DIBSTATUS DibDriverReadInternalBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t *pData);
DIBSTATUS DibDriverWriteInternalBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Data);
DIBSTATUS DibDriverReadDirectBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t *pData);
DIBSTATUS DibDriverWriteDirectBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Data);
DIBSTATUS DibDriverWriteReg8(struct DibDriverContext  * pContext, uint32_t Addr, uint8_t Value);
DIBSTATUS DibDriverReadReg8(struct DibDriverContext  * pContext, uint32_t Addr, uint8_t * Value);
DIBSTATUS DibDriverReadReg16(struct DibDriverContext  * pContext, uint32_t Addr, uint16_t * Value);
DIBSTATUS DibDriverWriteReg16(struct DibDriverContext  * pContext, uint32_t Addr, uint16_t Value);
DIBSTATUS DibDriverReadReg32(struct DibDriverContext  * pContext, uint32_t Addr, uint32_t * Value);
DIBSTATUS DibDriverWriteReg32(struct DibDriverContext  * pContext, uint32_t Addr, uint32_t Value);
DIBSTATUS DibDriverInternalTest(struct DibDriverContext *pContext, uint32_t test);

double extern_function(double * argv, int32_t argc)
{
    uint32_t data32;
    uint16_t data16;
    uint8_t  data8;
    double   ret = 0;
    int32_t  Type= (int32_t)argv[0];

    switch(Type)
    {
        case 0:
            DibDriverReadReg32(CONTEXT,(uint32_t)argv[1],&data32);
            ret=(double)data32;
            break;
        case 1:
            DibDriverWriteReg32(CONTEXT, (uint32_t)argv[1], (uint32_t)argv[2]);
            break;
        case 2:
            DibDriverReadReg16(CONTEXT,(uint32_t)argv[1],&data16);
            ret=(double)data16;
            break;
        case 3:
            DibDriverWriteReg16(CONTEXT, (uint32_t)argv[1], (uint16_t)argv[2]);
            break;
        case 4:
            DibDriverReadReg8(CONTEXT,(uint32_t)argv[1],&data8);
            ret=(double)data8;
            break;
        case 5:
            DibDriverWriteReg8(CONTEXT, (uint32_t)argv[1], (uint8_t)argv[2]);
            break;
        case 6:
            DibDriverReadInternalBus(CONTEXT,(uint32_t)argv[1],&data32);
            ret=(double)data32;
            break;
        case 7:
            DibDriverWriteInternalBus(CONTEXT, (uint32_t)argv[1], (uint32_t)argv[2]);
            break;
    }
    return ret;
}



/**
 * DtaDebugReadRegInternal
 */
void DtaDebugReadRegInternal(uint32_t Addr)
{
   uint32_t Data;

   DibDriverReadInternalBus(CONTEXT, Addr, &Data);
   printf(CRB "Address: %08x, Data: %08x (%d) " CRA, Addr, Data, Data);
}

/**
 * DtaDebugWriteRegInternal
 */
void DtaDebugWriteRegInternal(uint32_t Addr, uint32_t Data)
{
   DibDriverWriteInternalBus(CONTEXT, Addr, Data);
   printf(CRB "Address: %08x, Data: %08x " CRA, Addr, Data);
}

/**
 * DtaDebugReadReg32
 */
void DtaDebugReadReg32(uint32_t Addr)
{
   uint32_t Data;
   DibDriverReadReg32(CONTEXT, Addr, &Data);
   printf(CRB "Address: %08x, Data: %08x (%d) " CRA, Addr, Data, Data);
}

void DtaDebugReadBuf32(uint32_t Addr, uint32_t size)
{
   FILE *ff;
   unsigned char *buf = (unsigned char*)malloc(size);
   ff = fopen("dump.bin","wb");
   DibD2BReadReg(CONTEXT, Addr,2,size,buf);
   fwrite(buf,1,size,ff);
   fclose(ff);
   free(buf);
}
/**
 * DtaDebugReadReg16
 */
void DtaDebugReadReg16(uint32_t Addr)
{
   uint16_t Data;
   DibDriverReadReg16(CONTEXT, Addr, &Data);
   printf(CRB "Address: %04x, Data: %04x (%d) " CRA, Addr, Data, Data);
}

/**
 * DtaDebugWriteReg32
 */
void DtaDebugWriteReg32(uint32_t Addr, uint32_t Data)
{
   DibDriverWriteReg32(CONTEXT, Addr, Data);
   printf(CRB "Address: %08x, Data: %08x (%d) " CRA, Addr, Data, Data);
}
/**
 * DtaDebugWriteReg16
 */
void DtaDebugWriteReg16(uint32_t Addr, uint32_t Data)
{
   DibDriverWriteReg16(CONTEXT, Addr, Data);
   printf(CRB "Address: %08x, Data: %04x (%d) " CRA, Addr, Data, Data);
}

/**
 * DtaDebugDumpReg
 */
void DtaDebugDumpReg()
{
   int32_t  i;
   uint32_t Data;

   for(i = 0; i < 952; i++)
   {
      DibDriverReadInternalBus(CONTEXT, i, &Data);
      printf(CRB "Register: %d, Value Read: %04x (%d)" CRA, i, Data, Data);
   }
}

/**
 * DtaTestDfly
 */
extern int32_t entry(void);

void DtaTestDfly(void)
{
   entry();
}
#endif

/**
 * DtaInitInternal
 */
void DtaInitInternal(void)
{
   uint32_t StreamNum, FeNum;

   memset(StreamArray,0,sizeof(StreamArray));
   memset(FrontendArray,0,sizeof(FrontendArray));
   memset(ChannelArray,0,sizeof(ChannelArray));
   memset(FilterArray,0,sizeof(FilterArray));
   memset(ItemArray,0,sizeof(ItemArray));

   /* Init the Stream handles */
   for(StreamNum = 0; StreamNum < globalInfo.NumberOfStreams; StreamNum++)
      StreamArray[StreamNum].Hdl = globalInfo.Stream[StreamNum];

   /* Init the Frontend handles */
   for(FeNum = 0; FeNum < globalInfo.NumberOfFrontends; FeNum++)
      FrontendArray[FeNum].Hdl = globalInfo.Frontend[FeNum];
   
#ifdef USEFIFO
   InitFifo();
#endif
}

/**
 * DtaShowInternal
 */
void DtaShowInternal(void)
{
   int32_t i,Type;
   int32_t NbEs = 0;
   struct DtaItem   *es;
   union DibFilters *cfg;

   printf(CRB "----- ACTIVE STREAMS -----" CRA);
   for(i = 0; i < DIB_MAX_NB_OF_STREAMS; i++)
   {
      if(StreamArray[i].Used)
      {
         printf(CRB " STR_NUM %d: HDL %p, Std %s" CRA, i, StreamArray[i].Hdl, StdString[StreamArray[i].Std]);
      }
   }

   printf(CRB "" CRA);
   printf(CRB "----- ACTIVE FRONTENDS -----" CRA);
   for(i = 0; i < DIB_MAX_NB_OF_FRONTENDS; i++)
   {
      if(FrontendArray[i].Used)
      {
         printf(CRB " FE_NUM %d: HDL %p, Stream %d" CRA, i, FrontendArray[i].Hdl, FrontendArray[i].StreamNum);
      }
   }

   printf(CRB "" CRA);
   printf(CRB "----- ACTIVE CHANNELS -----" CRA);
   for(i = 0; i < DIB_MAX_NB_CHANNELS; i++)
   {
      if(ChannelArray[i].Used)
      {
         printf(CRB " CH_ID %d: HDL %d, STR_NUM %d RfKhz=%d" CRA, i, ChannelArray[i].ChannelHdl, ChannelArray[i].StreamNum, ChannelArray[i].Info.ChannelDescriptor.RFkHz);
      }
   }

   printf(CRB "" CRA);
   printf(CRB "----- ACTIVE FILTERS -----" CRA);
   for(i = 0; i < DIB_MAX_NB_FILTERS; i++)
   {
      if(FilterArray[i].Used)
      {
         printf(CRB " FILT_ID %d: HDL %d, DataType %s, Parent CH_ID %d" CRA, i, FilterArray[i].FilterHdl, DataTypeString[FilterArray[i].DataType], FilterArray[i].ParentChannel);
      }
   }
   printf(CRB "" CRA);
   printf(CRB "----- ACTIVE ELEMENTARY STREAMS -----" CRA);
   for(i = 0; i < DIB_MAX_NB_ITEMS; i++)
   {
      es = &ItemArray[i];
      if(es->Used)
      {
         NbEs++;
         Type = FilterArray[es->ParentFilter].DataType;

         printf(" ES_ID %d: HDL %d, Parent FILT_ID=%4d, Parent CH_ID=%d, ", i, es->ItemHdl, es->ParentFilter, FilterArray[es->ParentFilter].ParentChannel);
         cfg = &(es->Config);

         switch (Type)
         {
#if (DIB_MPEFEC_DATA == 1)
         case eMPEFEC:
            printf(CRB "MPE: PID=%d, ROWS=%4d, MPE-FEC=1, MBD=%d" CRA, cfg->MpeFec.Pid, cfg->MpeFec.NbRows, cfg->MpeFec.MaxBurstDuration);
            break;
#endif

#if (DIB_MPEIFEC_DATA == 1)
         case eMPEIFEC:
            printf(CRB "MPE: PID=%d, ROWS=%4d, C=%d, R=%d, B=%d, S=%d, D=%d, MBD=%d" CRA,
                        cfg->MpeIFec.Pid,
                        cfg->MpeIFec.NbRows,
                        cfg->MpeIFec.NbAdstColumns,
                        cfg->MpeIFec.MaxIFecSect,
                        cfg->MpeIFec.EncodParal,
                        cfg->MpeIFec.SpreadingFactor,
                        cfg->MpeIFec.SendingDelay,
                        cfg->MpeIFec.MaxBurstDuration);
            break;
#endif

#if (DIB_SIPSI_DATA == 1)
         case eSIPSI:
            printf(CRB "SIPSI: PID=%d, WATCH=%d, CRC=%d" CRA, cfg->SiPsi.Pid, cfg->SiPsi.PidWatch, cfg->SiPsi.Crc);
            break;
#endif

#if (DIB_RAWTS_DATA == 1)
         case eTS:
            printf(CRB "RAW TS: PID=%d" CRA,cfg->Ts.Pid);
            break;
         case eLOWSPEEDTS:
            printf(CRB "LOW SPEED TS: PID=%d" CRA,cfg->Ts.Pid);
            break;
#endif
         default:
            printf(CRB "Illegal Type (%d)" CRA, Type);
            break;
         }
      }
   }

   if(!NbEs)
   {
      printf(CRB "  Elementary Streams array empty" CRA);
   }
}

/**
 *
 */
void DtaMpeMnt(void)
{
   /* Toggle bit 9 of DriverDebugMask thanks to XOR */
   debugMode.DriverDebugMask ^= (1<<9);

   DibSetConfig(CONTEXT, eDEBUG_MODE, (union DibParamConfig *)&debugMode);
}

/**
 *
 */
void DtaHbmProf(void)
{
   /* Toggle bit 10 of DriverDebugMask thanks to XOR */
   debugMode.DriverDebugMask ^= (1<<10);

   DibSetConfig(CONTEXT, eDEBUG_MODE, (union DibParamConfig *)&debugMode);
}

void DtaTsMonitStart(uint32_t ctx)
{
   int ii = 0;
   if(ctx > (MAX_TS_MONIT_CTX-1))
      return;
   if(CurrentTsMonit == DIB_UNSET)
   {
      CurrentTsMonit = ctx;
      memset(&gTsDataInfo[ctx], 0, sizeof(struct DtaTsInfo));
      for(ii = 0; ii < MAX_PID_BY_CTX; ii++)
      {
        gTsDataInfo[ctx].PidsVal[ii] = 0xFFFF;
      }
   }
   else
      printf("ts_stop should be called first");
}

void DtaTsMonitStop(void)
{
   CurrentTsMonit = DIB_UNSET;

   memset(PidsCc, 0, MAX_PID_BY_CTX);

}

void DtaTsMonitFree(uint32_t ctx)
{
   int ii = 0;
   if(ctx > (MAX_TS_MONIT_CTX-1))
      return;
   if(CurrentTsMonit == ctx)
   {
      printf("ts_stop should be called first");
      return;
   }
   memset(&gTsDataInfo[ctx], 0, sizeof(struct DtaTsInfo));
   for(ii = 0; ii < MAX_PID_BY_CTX; ii++)
   {
      gTsDataInfo[ctx].PidsVal[ii] = 0xFFFF;
      gTsDataInfo[ctx].PidsCCCount[ii] = 0;
   }

}

void DtaTsMonitInit(void)
{
   int32_t ii, jj;

   for(ii = 0; ii < MAX_TS_MONIT_CTX; ii++)
   {
     for(jj = 0; jj < MAX_PID_BY_CTX; jj++)
     {
      gTsDataInfo[ii].PidsVal[jj] = 0xFFFF;
     }
   }
}

void DtaTsRecordEnable(unsigned char enable)
{
  TsDataRecord = enable;
   if(TsDataRecord == 1)
     File= fopen("RawTs.mpg", "wb");
   else
       fclose(File);
}

void DtaLogTsStatus(FILE * f)
{
   int32_t ii, jj;

   if(!f)
   {
      f = stdout;
   }

   fprintf(f, CRB "------ RawTS Monitoring -------" CRA);
   for(ii = 0; ii < MAX_TS_MONIT_CTX; ii++)
   {
      for(jj = 0; jj < MAX_PID_BY_CTX; jj++)
      {
         if(gTsDataInfo[ii].PidsVal[jj] != 0xFFFF)
         {
            fprintf(f, CRB "Ctx %d Pid %d CCError: %d" CRA, ii, gTsDataInfo[ii].PidsVal[jj],gTsDataInfo[ii].PidsCCCount[jj]);
         }else
            break;
      }
      if(jj != 0)
      {
         fprintf(f, CRB "BE/CC/EI/DC/OK/NL :  %d  %d  %d  %d  %d  %d" CRA,
               gTsDataInfo[ii].BufferError,
               gTsDataInfo[ii].CorruptedPacketsCount,
               gTsDataInfo[ii].ErrorIndicFlag,
               gTsDataInfo[ii].DiscontinuitiesCount,
               gTsDataInfo[ii].TramesOk,
               gTsDataInfo[ii].NullPackets);
      }
   }
   fflush(f);
}

#ifdef USEFIFO

int fifofd[10];
void InitFifo()
{
  int i;
  for (i=0;i<10;i++)
    fifofd[i] = -1;
}
void PushToFifo(unsigned char *buffer, unsigned int size, unsigned char fifoid)
{
  int rc;
  unsigned int len;
  if (fifofd[fifoid] == -1)
  {  
    char fname[50];
    sprintf(fname,"/tmp/tfifo%d",fifoid);
    fifofd[fifoid] = open (fname,O_WRONLY|O_NONBLOCK);
  }

  if (fifofd[fifoid] != -1)
  {
      len = size;
      while (len > 0)
      {
         rc = write(fifofd[fifoid],buffer,size);
         if (rc > 0)
            len -= rc;
      }
  }
  else
    printf("Error Open Fifo\n");
}
#endif
