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
* @file "DtaScan.c"
* @brief DiBcom Test Application - Scan/Tune Functions.
*
***************************************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "DibConfig.h"
#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaCmds.h"
#include "DtaScan.h"
#include "DtaMonit.h"
#include "DtaTime.h"

extern struct DibDriverContext * gUserContext;

#define CONTEXT (gUserContext)

uint8_t  gMaxDemod = DIB_DEMOD_AUTO;

struct DtaScanStatus
{
  struct DibTuneChan ChDesc;
  uint32_t          NbFail;
  uint32_t          NbSuccess;
  uint32_t          NbTimeout;
  uint32_t          NbAbort;
};
struct DtaScanStatus TuneChanArray[DTA_MAX_FREQUENCIES];

/*
 * Local functions and structures
 */
static void    CopyTuneChan(struct DibTuneChan *dest, struct DibTuneChan *source);
static void    IncrTuneChanCounters(int32_t i);


/**
 * TS Channels Numbers
 */
enum
{
  DATA_CHANNEL_0 = 0,
  DATA_CHANNEL_1,
  SIPSI_CHANNEL,
  MONIT_CHANNEL
};

/**
  * DtaTuneMonitCallback
 */
struct TuneMonitContext
{
   volatile int32_t     TuneMonitFinished;
   struct DibTuneMonit *TuneMonitInfo;
};

void DtaTuneMonitCallback(struct DibTuneMonit *pMonitDescriptor, void * pContext)
{
   struct TuneMonitContext *tmc = (struct TuneMonitContext *)pContext;

   *(tmc->TuneMonitInfo)   = *pMonitDescriptor;
   tmc->TuneMonitFinished  = 1;

   printf(CRB "DtaTuneMonitCallback" CRA);
}

/**
* DtaTuneChannel
*/
int32_t DtaTuneChannelEx(uint32_t StreamNum, uint32_t bw, uint32_t rf, uint8_t flag, uint8_t async)
{
   int32_t             rc;
   struct DibTuneMonit md;
   struct DibTuneChan *pcd = &(md.ChannelDescriptor);
   DibTuneMonitCallback CallBack;
   void                *CallCtx;
   struct TuneMonitContext tmc;

   if((StreamNum >= DIB_MAX_NB_OF_STREAMS) || (StreamArray[StreamNum].Used == 0))
      return DIBSTATUS_INVALID_PARAMETER;

   if(async)
   {
      tmc.TuneMonitFinished = 0;
      tmc.TuneMonitInfo     = &md;

      CallCtx  = (void*)&tmc;
      CallBack = DtaTuneMonitCallback;
   }
   else
   {
      CallBack = 0;
      CallCtx = NULL;
   }

   md.ChannelMonit.Type = StreamArray[StreamNum].Std;

   /* Set modulation parameters. */
   switch(flag)
   {
   /* Unknown parameters. */
   case SCAN_FLAG_AUTOSEARCH:
      pcd->Bw                    = (enum DibSpectrumBW)        bw;
      pcd->RFkHz                 =                             rf;
      memset(&pcd->Std, -1, sizeof(pcd->Std));
      break;

   /* Lookup frequency and bandwidth in TuneChanArray[]. */
   case SCAN_FLAG_PLAIN_TUNE:
      if(GetTuneChan(rf, bw, pcd) == DTA_NO_INDEX)
      {
         printf(CRB "%s: *** Error: Channel %d Mhz Bw %d Mhz unknown: Scan channel first" CRA, __FUNCTION__, rf / 1000, bw / 10);
         return DIBSTATUS_ERROR;
      }
      break;

   default:
      printf(CRB "%s: *** Error: Illegal scan flag (%d)" CRA, __FUNCTION__, flag);
      return DIBSTATUS_ERROR;
      break;
   }

   printf(CRB "--------------------------------------------------------------------------------" CRA);
   printf(CRB "TUNE EX CHANNEL : RF Frequency=%5.3f MHz, Channel BW=%d MHz, Scan Flag=%d" CRA, (float)rf / 1000.0, bw / 10, flag);
   printf(CRB "--------------------------------------------------------------------------------" CRA);

   DtaRealTimeSet();

   /* Submits request. */
   rc = DibTuneMonitChannelEx(CONTEXT, StreamArray[StreamNum].Hdl, &md, CallBack, CallCtx);

   if(async)
   {
      while(tmc.TuneMonitFinished == 0)
      {
         OsMSleep(100);
      }
   }

   LOG("--> status %d, duration %d ms", pcd->ScanStatus, DtaRealTimeDiffMs());

   /* Post-request processing. */
   if(rc == DIBSTATUS_TIMEOUT)
   {
      printf(CRB "%s: *** Error: Time Out ( ScanStatus=%d)" CRA, __FUNCTION__, pcd->ScanStatus);
      return rc;
   }

   if(pcd->ScanStatus == DIB_SCAN_ABORTED)
   {
      printf(CRB "%s: *** Error: Scan aborted (ScanStatus=%d)" CRA, __FUNCTION__, pcd->ScanStatus);
      return rc;
   }

   /* Display returned parameters. */
   DtaShowChannelParams(pcd, StreamArray[StreamNum].Std, DTA_FULL_DISPLAY);

   if(pcd->ScanStatus == DIB_SCAN_FAILURE)
      return rc;

   DtaDisplaySignalMonitInfo(&(md.ChannelMonit), DTA_FULL_DISPLAY, stdout);

   /* Saves off modulation parameters and updates statistics on global tune attempts.*/
   DtaSetTuneChan(pcd);

   return (rc);
}
/**
* DtaTuneChannel
*/
int32_t DtaTuneChannel(enum DibDemodType Type, uint32_t bw, uint32_t rf, uint8_t flag, uint8_t async, uint32_t StreamParam)
{
   int32_t             rc;
   struct DibTuneMonit md;
   struct DibTuneChan *pcd = &(md.ChannelDescriptor);
   uint32_t            ts  = MONIT_CHANNEL;
   DibTuneMonitCallback CallBack;
   void                *CallCtx;
   struct TuneMonitContext tmc;

   if(async)
   {
      tmc.TuneMonitFinished = 0;
      tmc.TuneMonitInfo     = &md;

      CallCtx  = (void*)&tmc;
      CallBack = DtaTuneMonitCallback;
   }
   else
   {
      CallBack = 0;
      CallCtx = NULL;
   }

   md.ChannelMonit.Type = Type;

   /* Set modulation parameters. */
   switch(flag)
   {
   /* Unknown parameters. */
   case SCAN_FLAG_AUTOSEARCH:
      pcd->Bw                    = (enum DibSpectrumBW)        bw;
      pcd->RFkHz                 =                             rf;
      pcd->InvSpec               = (enum DibSpectrumInv)       -1;
      memset(&pcd->Std, -1, sizeof(pcd->Std));
      break;

   /* Lookup frequency and bandwidth in TuneChanArray[]. */
   case SCAN_FLAG_PLAIN_TUNE:
      if(GetTuneChan(rf, bw, pcd) == DTA_NO_INDEX)
      {
         printf(CRB "%s: *** Error: Channel %d Mhz Bw %d Mhz unknown: Scan channel first" CRA, __FUNCTION__, rf / 1000, bw / 10);
         return DIBSTATUS_ERROR;
      }
      break;

   default:
      printf(CRB "%s: *** Error: Illegal scan flag (%d)" CRA, __FUNCTION__, flag);
      return DIBSTATUS_ERROR;
      break;
   }

   printf(CRB "--------------------------------------------------------------------------------" CRA);
   printf(CRB "TUNE CHANNEL %d: RF Frequency=%5.3f MHz, Channel BW=%d MHz, Scan Flag=%d" CRA, ts, (float)rf / 1000.0, bw / 10, flag);
   printf(CRB "--------------------------------------------------------------------------------" CRA);

   DtaRealTimeSet();

   /* Submits request. */
   rc = DibTuneMonitChannel(CONTEXT, DIB_DEMOD_AUTO, gMaxDemod, StreamParam, Type, &md, CallBack, CallCtx);

   if(async)
   {
      while(tmc.TuneMonitFinished == 0)
      {
         OsMSleep(100);
      }
   }

   LOG("--> status %d, duration %d ms", pcd->ScanStatus, DtaRealTimeDiffMs());

   /* Post-request processing. */
   if(rc == DIBSTATUS_TIMEOUT)
   {
      printf(CRB "%s: *** Error: Time Out (ts=%d, ScanStatus=%d)" CRA, __FUNCTION__, ts, pcd->ScanStatus);
      return rc;
   }

   if(pcd->ScanStatus == DIB_SCAN_ABORTED)
   {
      printf(CRB "%s: *** Error: Scan aborted (ts=%d, ScanStatus=%d)" CRA, __FUNCTION__, ts, pcd->ScanStatus);
      return rc;
   }

   /* Display returned parameters. */
   DtaShowChannelParams(pcd, Type, DTA_FULL_DISPLAY);

   if(pcd->ScanStatus == DIB_SCAN_FAILURE)
      return rc;

   /* Additional stuff. */
   if(ts == MONIT_CHANNEL)
   {
      DtaDisplaySignalMonitInfo(&(md.ChannelMonit), DTA_FULL_DISPLAY, stdout);
   }

   /* Saves off modulation parameters and updates statistics on global tune attempts.*/
   DtaSetTuneChan(pcd);

   return (rc);
}

/**
  * DtaStartScan
 */

int32_t DtaStartScan(enum DibDemodType Type, uint32_t FreqStart, uint32_t FreqEnd, uint32_t bw)
{
   int32_t status = 0;
   int32_t freq;
   int32_t rc;
   struct DibTuneMonit md;
   struct DibTuneChan *pcd = &md.ChannelDescriptor;

   printf(CRB "---------------------------------------------------------------" CRA);
   printf(CRB "Start Scanning" CRA);
   printf(CRB "----------------------------------------------------------------" CRA);

   for(freq = FreqStart; freq <= (int32_t)FreqEnd; freq += bw * 1000)
   {
      pcd->RFkHz                 =                             freq;
      pcd->Bw                    = (enum DibSpectrumBW)       (bw * 10);
      pcd->InvSpec               = (enum DibSpectrumInv)       -1;
      memset(&pcd->Std, -1, sizeof(pcd->Std));

      DtaRealTimeSet();

      rc = DibTuneMonitChannel(CONTEXT, DIB_DEMOD_AUTO, gMaxDemod, 0, Type, &md, NULL, 0);

      LOG(CRB "" CRA);
      LOG("ScanMonit --> status %d, duration %d ms", pcd->ScanStatus, DtaRealTimeDiffMs());

      if(rc == DIBSTATUS_TIMEOUT)
      {
         printf(CRB "Freq: %d MHz ==> Timeout (%d)" CRA, md.ChannelDescriptor.RFkHz, pcd->ScanStatus);
      }
      else if((rc == DIBSTATUS_SUCCESS) && (pcd->ScanStatus == DIB_SCAN_SUCCESS))
      {
         printf("Freq: %d MHz ==> ", freq / 1000);

         DtaShowChannelParams(pcd, Type, DTA_SHORT_DISPLAY);
         DtaDisplaySignalMonitInfo(&(md.ChannelMonit), DTA_TINY_DISPLAY, stdout);

         printf(CRB "" CRA);
      }
      else
      {
         printf(CRB "Freq: %d MHz ==> No Signal, Status %d " CRA, freq / 1000, pcd->ScanStatus);
      }

      /* Saves off channel descriptor to TuneChanArray[] even in case of failure.
       * It allows to keep global statistics on multiple calls to freqscan. */
      DtaSetTuneChan(pcd);
   }

   return (status);
}

/**
 * DtaRefreshFreqScan
 */
int32_t DtaRefreshFreqScan(void)
{
   int32_t i;
   int32_t rc;
   struct DibTuneMonit md;
   struct DibTuneChan *pcd = &md.ChannelDescriptor;

   printf(CRB "-------------------------------------" CRA);
   printf(CRB "Start Refresh Frequencies Found" CRA);
   printf(CRB "-------------------------------------" CRA);

   for(i = 0; i < DTA_MAX_FREQUENCIES; i++)
   {
      if((TuneChanArray[i].ChDesc.RFkHz != DTA_FREQUENCY_EMPTY) && (TuneChanArray[i].ChDesc.Bw != DTA_BANDWIDTH_EMPTY) &&
         (TuneChanArray[i].ChDesc.ScanStatus == DIB_SCAN_SUCCESS))
      {
         CopyTuneChan(pcd, &(TuneChanArray[i].ChDesc));

         DtaRealTimeSet();

         rc = DibTuneMonitChannel(CONTEXT, DIB_DEMOD_AUTO, gMaxDemod, 0, eSTANDARD_DVB, &md, NULL, 0);

         LOG(CRB "" CRA);
         LOG("TuneMonit --> status %d, duration %d ms", pcd->ScanStatus, DtaRealTimeDiffMs());

         if(rc == DIBSTATUS_TIMEOUT)
         {
            printf(CRB "Freq: %d MHz ==> Timeout (%d)" CRA, md.ChannelDescriptor.RFkHz, pcd->ScanStatus);
         }
         else if((rc == DIBSTATUS_SUCCESS) && (pcd->ScanStatus == DIB_SCAN_SUCCESS))
         {
            printf("Freq: %d MHz ==> ", md.ChannelDescriptor.RFkHz / 1000);
            DtaShowChannelParams(pcd, eSTANDARD_DVB, DTA_SHORT_DISPLAY);
            DtaDisplaySignalMonitInfo(&(md.ChannelMonit), DTA_TINY_DISPLAY, stdout);
            printf(CRB "" CRA);
         }


         IncrTuneChanCounters(i);
      }
   }

   return (0);
}

/**
 * DtaInitTuneArray
 */
void DtaInitTuneArray(void)
{
   int32_t i;
   struct DtaScanStatus *ch;

   for(i = 0; i < DTA_MAX_FREQUENCIES; i++)
   {
      ch                               =                             &(TuneChanArray[i]);
      ch->ChDesc.Bw                    = (enum DibSpectrumBW)        DTA_BANDWIDTH_EMPTY;
      ch->ChDesc.RFkHz                 =                             DTA_FREQUENCY_EMPTY;
      ch->ChDesc.InvSpec               = (enum DibSpectrumInv)       -1;
      memset(&ch->ChDesc.Std, -1, sizeof(ch->ChDesc.Std));
      ch->NbFail                       = 0;
      ch->NbSuccess                    = 0;
      ch->NbTimeout                    = 0;
      ch->NbAbort                      = 0;
   }
}

/**
 * DtaLogScanStatus
 */
void DtaLogScanStatus(FILE * f)
{
   int32_t i;
   int32_t NbCh = 0;

   if(!f)
   {
      f = stdout;
   }

   /* fprintf(f, "---------- TUNE STATISTICS ----------" CRA); */
   for(i = 0; i < DTA_MAX_FREQUENCIES; i++)
   {
      if((TuneChanArray[i].ChDesc.RFkHz != DTA_FREQUENCY_EMPTY) && (TuneChanArray[i].ChDesc.Bw != DTA_BANDWIDTH_EMPTY))
      {
         NbCh++;
         fprintf(f, "  Freq %3d MHz, Bw %d MHz: OK/KO/TO/AB: %d / %d / %d / %d" CRA,
                                 TuneChanArray[i].ChDesc.RFkHz / 1000,
                                 TuneChanArray[i].ChDesc.Bw / 10, TuneChanArray[i].NbSuccess,
                                 TuneChanArray[i].NbFail, TuneChanArray[i].NbTimeout, TuneChanArray[i].NbAbort);
      }
   }
   if(!NbCh)
   {
      fprintf(f, "  Frequency array empty" CRA);
   }
   /* fprintf(f, "-------------------------------------" CRA); */

   fflush(f);
}

/*
 * LOCAL FUNCTIONS
 */
int32_t DtaSetTuneChan(struct DibTuneChan *tc)
{
   int32_t i;

   for(i = 0; i < DTA_MAX_FREQUENCIES; i++)
   {
      if((TuneChanArray[i].ChDesc.RFkHz == tc->RFkHz) && (TuneChanArray[i].ChDesc.Bw == tc->Bw))
         break;
   }

   if(i < DTA_MAX_FREQUENCIES)
   {
      CopyTuneChan(&(TuneChanArray[i].ChDesc), tc);
   }
   else
   {
      /* find a free slot */
      for(i = 0; i < DTA_MAX_FREQUENCIES; i++)
      {
         if((TuneChanArray[i].ChDesc.RFkHz == DTA_FREQUENCY_EMPTY) && (TuneChanArray[i].ChDesc.Bw == DTA_BANDWIDTH_EMPTY))
            break;
      }

      if(i == DTA_MAX_FREQUENCIES)
      {
         printf(CRB "%s: *** Error: No free slot" CRA, __FUNCTION__);
         return -1;
      }

      CopyTuneChan(&(TuneChanArray[i].ChDesc), tc);
   }

   IncrTuneChanCounters(i);

   return i;
}

static void IncrTuneChanCounters(int32_t i)
{
   if(i >= DTA_MAX_FREQUENCIES)
   {
      printf(CRB "%s: *** Error: Index out of range (%d)" CRA, __FUNCTION__, i);
      return;
   }

   switch (TuneChanArray[i].ChDesc.ScanStatus)
   {
   case DIB_SCAN_FAILURE:
      TuneChanArray[i].NbFail++;
      break;
   case DIB_SCAN_SUCCESS:
      TuneChanArray[i].NbSuccess++;
      break;
   case DIB_SCAN_TIMEOUT:
      TuneChanArray[i].NbTimeout++;
      break;
   case DIB_SCAN_ABORTED:
      TuneChanArray[i].NbAbort++;
      break;
   case DIB_SCAN_INVALID_PARAM:
      printf(CRB "*** Error: ScanStatus invalid param " CRA);
      break;
   default:
      printf(CRB "%s: *** Error: Illegal ScanStatus (%d)" CRA, __FUNCTION__, TuneChanArray[i].ChDesc.ScanStatus);
      break;
   }
}

int32_t GetTuneChan(uint32_t rf, uint32_t bw, struct DibTuneChan *dest)
{
   int32_t i;

   for(i = 0; i < DTA_MAX_FREQUENCIES; i++)
   {
      if((TuneChanArray[i].ChDesc.RFkHz == rf) && (TuneChanArray[i].ChDesc.Bw == (enum DibSpectrumBW) bw))
      {
         CopyTuneChan(dest, &(TuneChanArray[i].ChDesc));
         return i;
      }
   }

   return DTA_NO_INDEX;
}

static void CopyTuneChan(struct DibTuneChan *dest, struct DibTuneChan *source)
{
   memcpy(dest, source, sizeof(struct DibTuneChan));
}

const char * TurboCodeToString(int turbo_cr_id)
{
    switch(turbo_cr_id) {
        default:
        case -1:  return "TCR_UNK"; break;
        case 0 :  return "STD_1/5"; break;
        case 1 :  return "STD_2/9"; break;
        case 2 :  return "STD_1/4"; break;
        case 3 :  return "STD_2/7"; break;
        case 4 :  return "STD_1/3"; break;
        case 5 :  return "CPL_1/3"; break;
        case 6 :  return "STD_2/5"; break;
        case 7 :  return "CPL_2/5"; break;
        case 8 :  return "STD_1/2"; break;
        case 9 :  return "CPL_1/2"; break;
        case 10:  return "STD_2/3"; break;
        case 11:  return "CPL_2/3"; break;
     }
}

void DtaShowChannelParams(struct DibTuneChan *cd, enum DibDemodType Type, uint8_t display)
{
   /* strings */
   char *qam[]       = { "QPSK  ", "16-QAM", "32-QAM", "64-QAM" };
   char *guardtime[] = { "1/32", "1/16 ", "1/8 ", "1/4 " };
   char *ffttype[]   = { "2K", "8K", "4K" };
#if ((DIB_DVB_STD == 1) || (DIB_ISDBT_STD == 1))
   char *codetype[]  = { "1/2", "2/3", "3/4", "4/5", "5/6", "6/7", "7/8" };
#endif
   char *spectype[]  = { "NOINV ", "INV   " };
   char * tmode[] = {"TMODE I", "TMODE II", "TMODE III", "TMODE IV"};
   struct DibDvbTuneChan * pTcDvb = NULL;

   if(display == DTA_FULL_DISPLAY)
   {
      printf(CRB "" CRA);
      printf(CRB "------------ TUNE RESULT ------------" CRA);
      switch (cd->ScanStatus)
      {
      case DIB_SCAN_SUCCESS:
         printf("%.3f MHz: ", 1.0 * cd->RFkHz / 1000);
#if (DIB_DVB_STD == 1)
         if((Type == eSTANDARD_DVB) || (Type == (eSTANDARD_DVB | eFAST)))
            pTcDvb = &cd->Std.Dvb;
#endif
#if (DIB_DVBSH_STD == 1)
         if((Type == eSTANDARD_DVBSH) || (Type == (eSTANDARD_DVBSH | eFAST)))
            pTcDvb = &cd->Std.Dvbsh.Dvb;
#endif

         if((Type == eSTANDARD_DVB) || (Type == (eSTANDARD_DVB | eFAST))
         || (Type == eSTANDARD_DVBSH) || (Type == (eSTANDARD_DVBSH | eFAST)))
         {
            printf("%s ", ffttype[pTcDvb->Nfft]);
            printf("%s ", qam[pTcDvb->Nqam]);
            printf("%s ", guardtime[pTcDvb->Guard]);
            printf(CRB "%s " CRA, spectype[cd->InvSpec]);
            printf(CRB " \tALPHA %i " CRA, pTcDvb->VitAlpha);
            if((Type == eSTANDARD_DVB) || (Type == (eSTANDARD_DVB | eFAST)))
            {
#if (DIB_DVB_STD == 1)
               printf(CRB " \tCode Rate HP %s" CRA, codetype[(pTcDvb->VitCodeRateHp - 1)]);
               printf(CRB " \tCode Rate LP %s" CRA, codetype[(pTcDvb->VitCodeRateLp - 1)]);
#endif
            }
            else if(Type == eSTANDARD_DVBSH)
            {
#if (DIB_DVBSH_STD == 1)
               printf(CRB " \tCode Rate HP %s" CRA, TurboCodeToString(pTcDvb->VitCodeRateHp));
               printf(CRB " \tCode Rate LP %s" CRA, TurboCodeToString(pTcDvb->VitCodeRateLp));
#endif
            }
            printf(CRB " \tHRCH %i" CRA, pTcDvb->VitHrch);
         }
#if (DIB_DAB_STD == 1)
         else if(Type == eSTANDARD_DAB)
         {
            if(cd->Std.Dab.TMode == 0xFF)
               printf(CRB "Transmission Mode not specified, ");
            else
               printf(CRB "Transmission Mode %s, " , tmode[cd->Std.Dab.TMode]);
            printf(CRB "NB Activ SubId %u" CRA, cd->Std.Dab.NbActiveSub);
         }
#endif

         if((Type == eSTANDARD_DVBSH) || (Type == (eSTANDARD_DVBSH | eFAST)))
         {
#if (DIB_DVBSH_STD == 1)
            printf(" \tComonMult %d\n", cd->Std.Dvbsh.CommonMult);
            printf(" \tNbLateTaps %d\n", cd->Std.Dvbsh.NbLateTaps);
            printf(" \tNbSlices %d\n", cd->Std.Dvbsh.NbSlices);
            printf(" \tSliceDistance %d\n", cd->Std.Dvbsh.SliceDistance);
            printf(" \tNonLateIncr %d\n", cd->Std.Dvbsh.NonLateIncr);
#endif
         }
         else if((Type == eSTANDARD_ISDBT) || (Type == eSTANDARD_ISDBT_1SEG))
         {
#if (DIB_ISDBT_STD == 1)
            uint32_t i;
            printf("%s ", ffttype[cd->Std.Isdbt.Nfft]);
            printf("%s ", guardtime[cd->Std.Isdbt.Guard]);
            printf("%s ", spectype[cd->InvSpec]);

            printf(" \tSbMode %d\n", cd->Std.Isdbt.SbMode);
            printf(" \tPartialReception %d\n", cd->Std.Isdbt.PartialReception);
            printf(" \tSbConnTotalSeg %d\n", cd->Std.Isdbt.SbConnTotalSeg);
            printf(" \tSbWantedSeg %d\n", cd->Std.Isdbt.SbWantedSeg);
            printf(" \tSbSubchannel %d\n", cd->Std.Isdbt.SbSubchannel);
            for(i = 0; i<3 ;i++)
            {
               printf(" \tLayer %d\n", i);
               printf(" \t\tConstellation %s\n", qam[cd->Std.Isdbt.layer[i].Constellation]);
               printf(" \t\tCodeRate %s\n", codetype[cd->Std.Isdbt.layer[i].CodeRate - 1]);
               printf(" \t\tNbSegments %d\n", cd->Std.Isdbt.layer[i].NbSegments);
               printf(" \t\tTimeIntlv %d\n", cd->Std.Isdbt.layer[i].TimeIntlv);
            }
#endif
         }
         else if(Type == eSTANDARD_CTTB) 
         {
#if (DIB_CTTB_STD == 1)
            if (cd->Std.Cttb.Nfft  != eDIB_FFT_AUTO) 
            {
               if (cd->Std.Cttb.Nfft == eDIB_FFT_4K) 
                  printf("4K ");
               else if (cd->Std.Cttb.Nfft  == eDIB_FFT_SINGLE) 
                  printf("SC ");
               else 
                  printf("--");

               if (cd->Std.Cttb.Guard == eDIB_GUARD_INTERVAL_PN420)
                  printf("PN420 " );
               else if (cd->Std.Cttb.Guard == eDIB_GUARD_INTERVAL_PN595)
                  printf("PN595 " );
               else if (cd->Std.Cttb.Guard == eDIB_GUARD_INTERVAL_PN945)
                  printf("PN945 " );
               else 
                  printf("--" );

               if (cd->Std.Cttb.Nqam == eDIB_QAM_QPSK)
                  printf("QPSK " );
               else if (cd->Std.Cttb.Nqam == eDIB_QAM_16QAM)
                  printf("16QAM " );
               else if (cd->Std.Cttb.Nqam == eDIB_QAM_32QAM)
                 printf( "32QAM " );
               else if (cd->Std.Cttb.Nqam == eDIB_QAM_64QAM)
                  printf("64QAM " );
               else if (cd->Std.Cttb.Nqam == eDIB_QAM_QPSK_NR)
                  printf("QPSK_NR " );
               else 
                  printf("--" );
	
			  
               if (cd->Std.Cttb.LdpcCodeRate == eDIB_LDPC_CODERATE_0_4)
                  printf("0.4 ");
               else if (cd->Std.Cttb.LdpcCodeRate == eDIB_LDPC_CODERATE_0_6)
                  printf("0.6 ");
               else if (cd->Std.Cttb.LdpcCodeRate == eDIB_LDPC_CODERATE_0_8)
                  printf("0.8 ");
               else
                  printf("--");

               if (cd->Std.Cttb.TimeIntlv == eDIB_CTTB_TIME_INTERLEAVER_SHORT)
                  printf("Short Interleaver ");
               else if (cd->Std.Cttb.TimeIntlv == eDIB_CTTB_TIME_INTERLEAVER_LONG)
                  printf("Long Interleaver ");
               else
                  printf("-- ");

               printf("\n");
            }
#endif
         }

         break;

      case DIB_SCAN_ABORTED:
         printf(CRB "Scan aborted" CRA);
         break;

      case DIB_SCAN_INVALID_PARAM:
         printf(CRB "Scan invalid parameter" CRA);
         break;

      case DIB_SCAN_FAILURE:
         printf(CRB "Channel Not Found" CRA);
         break;

      case DIB_SCAN_TIMEOUT:
         printf(CRB "Time Out" CRA);
         break;

      default:
         printf(CRB "%s: *** Error: Illegal ScanStatus (%d)" CRA, __FUNCTION__, cd->ScanStatus);
         break;
      }
      printf(CRB "-------------------------------------" CRA);
   }
   else
   {
      if((Type == eSTANDARD_DVB) || (Type == (eSTANDARD_DVB | eFAST)))
      {
#if (DIB_DVB_STD == 1)
         if(cd->Std.Dvb.Nfft >= 0)
         {
            /* Short display. */
            printf("%s ", ffttype[cd->Std.Dvb.Nfft]);
            printf("%s ", qam[cd->Std.Dvb.Nqam]);
            printf("%s ", guardtime[cd->Std.Dvb.Guard]);
            printf("%s ", codetype[cd->Std.Dvb.VitCodeRateHp - 1]);
#if (DIB_DVB_ISDBT == 1)
            printf("%s ", cd->Std.Isdbt.SbMode ? "sb" : "full");
            printf("%s ", cd->Std.Isdbt.PartialReception ? "partial" : "together");
            printf("con %d ", cd->Std.Isdbt.SbConnTotalSeg);
            printf("wanted %d\n", cd->Std.Isdbt.SbWantedSeg);
            printf("sub %d\n", cd->Std.Isdbt.SbSubchannel);
#endif

         }
#endif
      }
      else if((Type == eSTANDARD_DVBSH) || (Type == (eSTANDARD_DVBSH | eFAST)))
      {
#if (DIB_DVBSH_STD == 1)
		              /* Short display. */
			printf("%s ", ffttype[cd->Std.Dvbsh.Dvb.Nfft]);
            printf("%s ", qam[cd->Std.Dvbsh.Dvb.Nqam]);
            printf("%s ", guardtime[cd->Std.Dvbsh.Dvb.Guard]);
            printf("%s ", TurboCodeToString(cd->Std.Dvbsh.Dvb.VitCodeRateHp));

#endif
      }
      else if((Type == eSTANDARD_ISDBT) || (Type == eSTANDARD_ISDBT_1SEG))
      {
#if (DIB_ISDBT_STD == 1)
         /* Short display. */
         printf("%s ", ffttype[cd->Std.Isdbt.Nfft]);
         printf("%s ", guardtime[cd->Std.Isdbt.Guard]);
         printf("A=%s B=%s C=%s", TurboCodeToString(cd->Std.Isdbt.layer[0].CodeRate)
                                , TurboCodeToString(cd->Std.Isdbt.layer[1].CodeRate)
                                , TurboCodeToString(cd->Std.Isdbt.layer[2].CodeRate));

#endif
      }

   }
}
