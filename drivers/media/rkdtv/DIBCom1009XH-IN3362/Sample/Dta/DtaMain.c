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
* @brief DiBcom Test Application - Main Application.
*
***************************************************************************************************/
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WINCE
#include <signal.h>
#endif

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
#include "DtaRawTs.h"
#include "DtaPlayout.h"

#ifdef DTA_TCP
#include "DtaTcp.h"
#endif

#ifdef WINCE
#include "DtaMain.h"
#endif

#ifdef USE_NAGRA
#include "DtaCasNagra.h"
#endif 
#ifdef USE_BCAS
#include "DtaCasBcas.h"
#endif

#if DIB_INTERNAL_MONITORING == 1
#include "DtaDebug.h"
#endif

#ifdef XCONSOLE_LOG
FILE *f_xc = NULL;
#endif

#ifdef RTM_TRACE
FILE *f_tr = NULL;
#endif
#include "errno.h"
extern uint8_t gMaxDemod;

/*
 * Local Functions
 */
static void DtaGetCommand(char *cmd);
static void DtaGetParam(uint32_t *Val, const char *string, uint8_t bmode, FILE * fb);
static void DtaDisplayHelp(void);
#ifndef WINCE
static void DtaProcessCommand(char *cmd, uint8_t, FILE * fb);
#endif
static void DtaGetFileName(char *name, const char *string, uint8_t bmode, FILE * fb);
static void DtaGetParamHex(uint32_t *Val, const char *string, uint8_t bmode, FILE * fb);
static void DtaGetString(char *str, uint8_t batch_mode, FILE * fb);
static void DtaForgetLine(uint8_t batch_mode, FILE * fb);
static void	*DtaCloseThread(void *param);

/*
 * Static variables
 */

extern struct DibDriverContext * gUserContext;

#define CONTEXT (gUserContext)


#ifdef DTA_TCP
static struct DibDtaDebugPlatformContext DtaTcp;
#endif


/* DtaClose procedure */
static THREAD_HANDLE	DtaCloseThreadHandle;
volatile uint8_t		DtaCloseProcedure = 0;

static uint32_t      FactorTime = 1;


/**
 * MAIN
 */
void SigQuitDta(int signal_id)
{
   int status;

   if(DtaCloseProcedure == 0)
   {
      DtaCloseProcedure = 1;

      status = OsCreateThread(&DtaCloseThreadHandle, (PTHREAD_FN)DtaCloseThread, (void *)NULL);

      printf(CRB "DtaClose Thread created, status=%d" CRA, status);
   }

}

#ifdef COSIM
int32_t dta(enum DibBoardType BoardType, BOARD_HDL BoardHdl)
{
   char      			cmd[64];

	if(BoardType < eBOARD_DEFAULT || BoardType >= eMAX_NB_BOARDS)
		/* print valid values and return error */
#else
int32_t main(int32_t argc, char **argv)
{
   char  cmd[10];

   enum DibBoardType BoardType = eBOARD_DEFAULT ;
   int32_t BoardNb = 0;
   BOARD_HDL BoardHdl = 0; /* default value */

   if(argc >= 2)
   {
      /* Board Type supplied! */
      BoardType = (enum DibBoardType)(atoi(argv[1]));

      if(BoardType >= 0 && BoardType < eMAX_NB_BOARDS)
      {
         printf(CRB "Board Type %d" CRA, BoardType);
      }
      else
#endif
      {
         printf(CRB "Valid Board types are:" CRA);
         printf(CRB "  0: 7078" CRA);
         printf(CRB "  1: 7070" CRA);
         printf(CRB "  2: 9080" CRA);
         printf(CRB "  3: Stk7078" CRA);
         printf(CRB "  4: Nim9080md" CRA);
         printf(CRB "  5: Stk7078md4" CRA);
         printf(CRB "  6: 9090" CRA);
         printf(CRB "  7: Nim9090md" CRA);
         printf(CRB "  8: 19080" CRA);
         printf(CRB "  9: Nim29098" CRA);
         printf(CRB " 10: Stk29098md4" CRA);
         printf(CRB " 11: Mod29098SHA" CRA);
         printf(CRB " 12: NIM10098v1_0__H1" CRA);
         printf(CRB " 13: NIM9090H" CRA);
         printf(CRB " 14: NIM1009xHx" CRA);
         printf(CRB " 15: NIM10096MD2" CRA);
         printf(CRB " 16: NIM10096MD3" CRA);
         printf(CRB " 17: NIM10096MD4" CRA);
         printf(CRB " 18: NIM_3009xHx" CRA);
         printf(CRB " 19: NIM_30092MD2" CRA);
         printf(CRB " 20: NIM_30092MD3" CRA);
         printf(CRB " 21: NIM_30092MD4" CRA);
         printf(CRB " 22: NIM_30062M" CRA);
         printf(CRB " 23: NIM_30062MD2" CRA);
         printf(CRB " 24: NIM_30062MD3" CRA);
         printf(CRB " 25: NIM_30062MD4" CRA);
         printf(CRB " 26: NIM_30062H" CRA);
         printf(CRB " 27: NIM_1009xAPMD2" CRA);
         printf(CRB " 28: NIM_3009xAPMD2" CRA);
         printf(CRB " 29: NIM_3006xAP" CRA);
         printf(CRB " 30: NIM_3006xAPMD2" CRA);
         printf(CRB " 31: NIM_3006xAPMD3" CRA);
         printf(CRB " 32: NIM_3006xAPMD4" CRA);
         return 0;
      }
#ifndef COSIM
   }
   if(argc >= 3)
   {
      /* Board handler supplied! */
      BoardNb = atoi(argv[2]);
      printf(CRB "Board Handler %d" CRA, BoardNb);
      BoardHdl = (void*)&BoardNb;
   }
#endif

   cmd[0] = 0;

#ifdef XCONSOLE_LOG
   signal(SIGINT, SIG_IGN);
   f_xc = popen("xconsole -file /dev/stdin -geometry 600x300+10+10 -notify -daemon", "w");
   signal(SIGINT, SIG_DFL);
   LOGT(CRB "Hello DVB-H people!" CRA);
#endif
#ifndef WINCE
   signal(SIGINT, SigQuitDta);
   signal(SIGTERM, SigQuitDta);
#endif

#ifdef RTM_TRACE
   f_tr = fopen("trace.txt", "w");

   if(!f_tr)
   {
      printf(CRB "%s: *** Error: Cannot open file 'trace.txt': stdout instead" CRA, __FUNCTION__);
      f_tr = stdout;
   }
#endif

   DtaInitSoft(BoardType, BoardHdl);
   DtaInit();
#ifdef DTA_TCP
   memset(&DtaTcp,0,sizeof (struct DibDtaDebugPlatformContext));
   DtaTcpInit(&DtaTcp,CONTEXT);
#endif

#if DIB_INTERNAL_MONITORING == 1
   DtaDebugInit(BoardNb);
#endif

   while(strcmp(cmd, "quit") != 0)
   {
      /*int32_t rc; */
      /*DtaDisplayHelp();*/
      printf(CRB "> " CRA); /* Keep line break at end of promt for ddd */
      DtaGetCommand(cmd);
      DtaProcessCommand(cmd, 0, NULL);
   }

#if DIB_INTERNAL_MONITORING == 1
   DtaDebugClose();
#endif

   DtaDeInit();

#ifdef XCONSOLE_LOG
   LOGT(CRB "Good Bye!" CRA);
   OsMSleep(1000);
   system("killall xconsole");
   pclose(f_xc);
   f_xc = NULL;
#endif

#ifdef RTM_TRACE
   fclose(f_tr);
   f_tr = NULL;
#endif

   return 0;
}

/**
 * DtaProcessCommand
 */
static void DtaDisplayHelp(void)
{
   printf(CRB "-- Scanning commands -------------------------------------------------------------------" CRA);
   printf(CRB "- monit std freq bw                                     => scan and monitor a frequency for a std" CRA);
   printf(CRB "- scan freq bw                                          => scan and monitor a frequency" CRA);
   printf(CRB "- tune freq bw                                          => tune and monitor a frequency" CRA);
   printf(CRB "- freqscan start_f stop_f bw                            => scan a list of frequency" CRA);
   printf(CRB "- freqrefresh                                           => refresh previous list" CRA);
   printf(CRB "- freqstat                                              => display scan statistics" CRA);
   printf(CRB "- freqclear                                             => clear scan statistics" CRA);
   printf(CRB "-- Main commands -----------------------------------------------------------------------" CRA);
   printf(CRB "- gch freq bw mode prio chid                            => get a channel channel" CRA);
   printf(CRB "- sdemod NbWantedDemod                                  => change NbWantedDemod for tune, scan and gch" CRA);
   printf(CRB "- dch chid                                              => delete channel" CRA);
   printf(CRB "- gfi chid datatype fid                                 => get a new filter" CRA);
   printf(CRB "- dfi fid                                               => delete a filter" CRA);
   printf(CRB "- atf fid [..] itid                                     => add to filter" CRA);
   printf(CRB "- dtf itid                                              => delete from filter" CRA);
   printf(CRB "- spc                                                   => switch priority channel" CRA);
   printf(CRB "- asf                                                   => add a sipsi filter on header." CRA);
   printf(CRB "- dsf                                                   => remove a sipsi filter on header." CRA);
   printf(CRB "- spid mode                                             => turn on/off prefetch mode" CRA);
   printf(CRB "- strat mode                                            => set strategy" CRA);
   printf(CRB "- tson/tsoff                                            => Set Power Saving Mode" CRA);
   printf(CRB "- mnt / mntdvbh / mntf                                  => Monitoring" CRA);
   printf(CRB "- mntstart/mntstop                                      => Start/Stop Monitoring thread" CRA);
   printf(CRB "- mpemnt                                                => print mpe monitoring info oly when tables arrive." CRA);
   printf(CRB "- mntsig chid demod                                     => get signal monitoring." CRA);
   printf(CRB "- psistart/psistop                                      => Monitoring sipsi." CRA);

   printf(CRB "-- Control commands --------------------------------------------------------------------" CRA);
   printf(CRB "- batch / batchl                    => batch Mode and batch loop" CRA);
   printf(CRB "- pause / rpause ms                 => sleep random or not" CRA);
   printf(CRB "- log file text                     => log some text in a file" CRA);
   printf(CRB "- logtime file text                 => log some text in a file adding time info" CRA);
   printf(CRB "- logdrv text                       => log driver stats to a file" CRA);

   printf(CRB "-- Debug commands ----------------------------------------------------------------------" CRA);
   printf(CRB "- dh func value                     => change debug function value" CRA);
   printf(CRB "- show                              => show channel/filter/item allocations" CRA);
   printf(CRB "- prof                              => enable/disable hbm profiling" CRA);
#if DIB_INTERNAL_MONITORING == 1
   printf(CRB "- agc_freeze                        => enable/disable agc freeze" CRA);
   printf(CRB "- acqui_control                     => enable/disable acquisition mode" CRA);
   printf(CRB "- set_clock                         => change octopus/reflex clock" CRA);
#endif

   printf(CRB "-- Asynchronous commands ---------------------------------------------------------------" CRA);
   printf(CRB "- scan_async freq bw                => scan and monitor a frequency" CRA);
   printf(CRB "- tune_async freq bw                => tune and monitor a frequency" CRA);
   printf(CRB "- mntsig_async                      => tune and monitor a frequency" CRA);

   printf(CRB "-- Extended info Set / Get -------------------------------------------------------------" CRA);
   printf(CRB "- info                              => get extended information" CRA);
   printf(CRB "- gpiotest                          => change system led state" CRA);
   printf(CRB "- toggle_mpeg                       => toggle mpeg1 <-> mpeg2" CRA);
   printf(CRB "- toggle_mpeg_status                => get toggle mpeg1 <-> mpeg2 status" CRA);
   printf(CRB "- set_per_period                    => change the per period" CRA);
   printf(CRB "- get_per_period                    => get the wanted per period" CRA);

   printf(CRB "----------------------------------------------------------------------------------------" CRA);
   printf(CRB "- quit                              => Exit Program" CRA);
   printf(CRB "----------------------------------------------------------------------------------------" CRA);
}

uint32_t DtaGetStreamParam(uint8_t batch_mode, FILE * fb)
{
   /* Simple API */
   uint32_t StreamType;
   uint32_t Ts204 = 0;
   uint32_t MpegParallel = 0;
   DtaGetParam(&StreamType, "Stream Type: HOST_DMA(0) MPEG_1(1) MPEG_2(2)", batch_mode, fb);
   if(StreamType > 0)
   {
      DtaGetParam(&MpegParallel, "Mpeg Interface: SERIAL(0) PARALLEL(1)", batch_mode, fb);
      DtaGetParam(&Ts204, "Ts Size: 188 Bytes(0)  204 Bytes(1)", batch_mode, fb);
   }
   return (StreamType & 0xFFFF) | ((MpegParallel & 1) << 16) | ((Ts204 & 1) << 17);
}

uint32_t DtaGetStreamOptions(uint32_t StreamType, uint8_t batch_mode, FILE * fb)
{
   uint32_t Options = 0;
   uint32_t MpegParallel;
   uint32_t Ts204;

   if(StreamType > 0)
   {
      DtaGetParam(&MpegParallel, "SERIAL(0) PARALLEL(1)", batch_mode, fb);
      DtaGetParam(&Ts204, "Ts Size: 188 Bytes(0)  204 Bytes(1)", batch_mode, fb);
      Options = ((MpegParallel & 1) | ((Ts204 & 1) << 1));
   }
   return Options;
}

uint32_t DtaGetStandard(uint8_t batch_mode, FILE * fb)
{
   uint32_t Type;
   printf("Std: 0=use stream %d=dvbth %d=isdbt %d=dab %d=analog_audio " \
         "%d=analog %d=atsc %d=dvbsh %d=fm %d=cmmb %d=isdbt_1seg %d=cttb %d=atscmh ",
         eSTANDARD_DVB,
         eSTANDARD_ISDBT,
         eSTANDARD_DAB,
         eSTANDARD_ANALOG_AUDIO,
         eSTANDARD_ANALOG,
         eSTANDARD_ATSC,
         eSTANDARD_DVBSH,
         eSTANDARD_FM,
         eSTANDARD_CMMB,
         eSTANDARD_ISDBT_1SEG,
         eSTANDARD_CTTB,
         eSTANDARD_ATSCMH);
   DtaGetParam(&Type, "", batch_mode, fb);
   return Type;
}

uint32_t DtaGetData(uint8_t batch_mode, FILE * fb)
{
   uint32_t Type;
   DtaGetParam(&Type, "DataType: ts=0  si=1 mpefec=2 fig=5 tdmb=6 msc=7 mscpacket=8  pes: V=10 A=11 O=12 pcr=13 mpeifec=14 cmmb=17 atsc/ip=19 atsc/fic=20 lowspeedts=21", batch_mode, fb);
   return Type;
}

static char * DtaExInfoGetSdramState(uint8_t state)
{
   switch (state)
   {
   case eINFO_SDRAM_NOTINIT:
      return "Not initialized yet, presence unknown";
      break;

   case eINFO_SDRAM_PRESENT:
      return "Present but directory not read or unvalid";
      break;

   case eINFO_SDRAM_HD_VALID:
      return "Present and directory valid";
      break;

   case eINFO_SDRAM_NOT_PRESENT:
      return "Initialization failed";
      break;

   case eINFO_SDRAM_UNKNOW:
      return "unknow";
      break;
   }
}

/* Use those defines to identify SOC version */
#define SOC               0x02
#define SOC_7090_P1G_11R1 0x82
#define SOC_7090_P1G_21R1 0x8a
#define SOC_8090_P1G_11R1 0x86
#define SOC_8090_P1G_21R1 0x8e

/* else use thos ones to check */
#define P1A_B      0x0
#define P1C        0x1
#define P1D_E_F    0x3
#define P1G        0x7
#define P1G_21R2   0xf
#define P1H_11R1   0x9
#define P1H_21R2   0xb

#define MP001 0x1 /* Single 9090/8096 */
#define MP005 0x4 /* Single Sband */
#define MP008 0x6 /* Dual diversity VHF-UHF-LBAND */
#define MP009 0x7 /* Dual diversity 29098 CBAND-UHF-LBAND-SBAND */

static void DtaDisplayTunerIdentity(struct DibInfoTunerIdentity *TunerIdentity)
{
   printf(CRB "\t\tTuner version" CRA);

    if ((TunerIdentity->Version & 0x3) == SOC)
    {
       switch(TunerIdentity->Version)
       {
            case SOC_8090_P1G_11R1:
               printf(CRB "\t\t\t\t\tSOC 8090 P1-G11R1 Has been detected" CRA);
               break;
            case SOC_8090_P1G_21R1:
               printf(CRB "\t\t\t\t\tSOC 8090 P1-G21R1 Has been detected" CRA);
               break;
            case SOC_7090_P1G_11R1:
               printf(CRB "\t\t\t\t\tSOC 7090 P1-G11R1 Has been detected" CRA);
               break;
            case SOC_7090_P1G_21R1:
               printf(CRB "\t\t\t\t\tSOC 7090 P1-G21R1 Has been detected" CRA);
               break;
            default :
               break;
        }
    }
    else
    {
        switch ((TunerIdentity->Version >> 5)&0x7)
        {
            case MP001:
               printf(CRB "\t\t\t\t\tMP001 : 9090/8096" CRA);
               break;
            case MP005:
               printf(CRB "\t\t\t\t\tMP005 : Single Sband" CRA);
               break;
            case MP008:
               printf(CRB "\t\t\t\t\tMP008 : diversity VHF-UHF-LBAND" CRA);
               break;
            case MP009:
               printf(CRB "\t\t\t\t\tMP009 : diversity CBAND-UHF-LBAND-SBAND" CRA);
               break;
            case 0:
               printf(CRB "\t\t\t\t\tP1H : Is it P1H ?" CRA);
               break;
            default :
               break;
        }

        switch(TunerIdentity->Version & 0x1f)
        {
            case P1H_11R1:
               printf(CRB "\t\t\t\t\tP1H_11R1 detected" CRA);
               break;
            case P1H_21R2:
               printf(CRB "\t\t\t\t\tP1H_21R2 detected" CRA);
               break;
            case P1G_21R2:
               printf(CRB "\t\t\t\t\tP1G_21R2 detected" CRA);
               break;
            case P1G:
               printf(CRB "\t\t\t\t\tP1G detected" CRA);
               break;
            case P1D_E_F:
               printf(CRB "\t\t\t\t\tP1D/E/F detected" CRA);
               break;
            case P1C:
               printf(CRB "\t\t\t\t\tP1C detected" CRA);
               break;
            case P1A_B:
                printf(CRB "\t\t\t\t\tP1-A/B detected: driver is deactivated" CRA);
                break;
            default :
                break;
        }
    }
}

static void DtaDisplayExInfo(void)
{
   int32_t               rc = 0;
   struct DibInfoRequest InfoRequest;
   struct DibInfoData    InfoData;
   uint32_t              index;
   uint8_t               gpioIndex;

   for (index = 0; index < globalInfo.NumberOfChips; index ++)
   {
      InfoRequest.Type = DIB_INFO_MASK_EFUSE | DIB_INFO_MASK_SDRAM | DIB_INFO_MASK_GPIO | DIB_INFO_MASK_EFUSE_TUNER | DIB_INFO_MASK_TUNER_IDENTITY;
      InfoRequest.ChipId = index;

      printf(CRB "\t[chip %u]" CRA, index);

      if((rc = DibGetInfo(CONTEXT, &InfoRequest, &InfoData)) == DIBSTATUS_SUCCESS)
      {
         if (DIBSTATUS_SUCCESS == InfoData.Efuse.Status)
         {
            printf(CRB "\t\tEFUSE\tis read ? %s" CRA, (InfoData.Efuse.IsRead == 1) ? "yes" : "no");
         }
         else
         {
            printf(CRB "\t\tEFUSE\taccess failed" CRA, index);
         }

         if (DIBSTATUS_SUCCESS == InfoData.Sdram.Status)
         {
            printf(CRB "\t\tSDRAM\tState : %s - is used for caching ? %s" CRA,
                   DtaExInfoGetSdramState(InfoData.Sdram.State),
                   (InfoData.Sdram.IsUsedForCaching == 1) ? "yes" : "no");
         }
         else
         {
            printf(CRB "\t\tSDRAM\taccess failed" CRA, index);
         }

         if (DIBSTATUS_SUCCESS == InfoData.Gpio.Status)
         {
            printf(CRB "\t\tGPIO" CRA);
            for (gpioIndex = 0 ; gpioIndex < InfoData.Gpio.Config.Num; gpioIndex ++)
            {
               printf(CRB "\t\t\t\t\tGPIO %u ->\t%s\tvalue\t%s" CRA,
                      gpioIndex,
                      (InfoData.Gpio.Config.Dir & (1 << gpioIndex)) ? "OUT" : "IN",
                      (InfoData.Gpio.Config.Val & (1 << gpioIndex)) ? "1" : "0");
            }
         }
         else
         {
            printf(CRB "\t\tGPIO access failed" CRA, index);
         }

         if (DIBSTATUS_SUCCESS == InfoData.EfuseTuner.Status)
         {
            printf(CRB "\t\tEFUSE Tuner\tlevel %u" CRA, InfoData.EfuseTuner.Level);
         }

         if (DIBSTATUS_SUCCESS == InfoData.TunerIdentity.Status)
         {
            DtaDisplayTunerIdentity(&InfoData.TunerIdentity);
         }
      }
      else
      {
         printf(CRB "Getting info failed." CRA);
      }
   }
}

static uint32_t gpiotestval = 0;

static void DtaDisplayGpioTest(void)
{
   int32_t           rc;
   struct DibInfoSet InfoSet;
   uint8_t           Status;
   int               nbloop;
   uint32_t          index;

   InfoSet.Type = eINFO_GPIO;
   InfoSet.Param.Gpio.Dir = 1;

   rc = DIBSTATUS_SUCCESS;
   Status = DIBSTATUS_SUCCESS;
   nbloop = 49;
   while (nbloop != 0 && Status == DIBSTATUS_SUCCESS && rc == DIBSTATUS_SUCCESS)
   {
      gpiotestval = (gpiotestval == 1) ? 0 : 1;
      InfoSet.Param.Gpio.Val = gpiotestval;

      for (index = 0; index < globalInfo.NumberOfChips; index ++)
      {
         if (index < 2)
         {
            InfoSet.Param.Gpio.Num = 0;
         }
         else
         {
            InfoSet.Param.Gpio.Num = 1;
         }

         InfoSet.ChipId = index;
         rc = DibSetInfo(CONTEXT, &InfoSet, &Status);
      }

      OsMSleep(100);
      nbloop --;
   }

   if(rc != DIBSTATUS_SUCCESS)
   {
      printf(CRB "Setting info failed." CRA);
   }

   if (Status != DIBSTATUS_SUCCESS)
   {
      printf(CRB "Can not setting info." CRA);
   }

}

/**
 * DtaProcessCommand
 */
static THREAD_HANDLE	AbortThreadHandle;
volatile uint8_t		AbortThreadRunning = 0;

static void	*AbortThread(void *param)
{
   AbortThreadRunning = 1;

   printf(CRB "Abort Thread Started" CRA);

   while(AbortThreadRunning)
   {
   OsMSleep(2000);

   printf(CRB "ABORT RQT" CRA);
   DibAbortTuneMonitChannel(CONTEXT);
   }

   printf(CRB "Abort Thread Stopped" CRA);

   AbortThreadRunning = 0;
   return (0);
}

/* management of sig handler in a deferred thread to properly take eventual semaphore inside DibClose */
static void	*DtaCloseThread(void *param)
{
   printf(CRB "DtaCloseThread" CRA);

   DtaDeInit();

   printf(CRB "Bye ! Bye !" CRA);

   exit (0);
}
#ifdef WINCE
void DtaProcessCommand(char *cmd, uint8_t batch_mode, FILE * fb)
#else
static void DtaProcessCommand(char *cmd, uint8_t batch_mode, FILE * fb)
#endif
{
   /* Useful in batch Mode only. Commented out till end of line */
   if(cmd[0] == '#')
   {
      DtaForgetLine(batch_mode, fb);
      return;
   }

   LOGT("%s ", cmd);

   if(strcmp(cmd, "help") == 0)
   {
      DtaDisplayHelp();
      return;
   }
   else if(strcmp(cmd, "quit") == 0)
   {
      /* Do nothing and bail out. */
      return;
   }
   else if(strcmp(cmd, "init") == 0)
   {
      DtaInit();
      return;
   }
   else if(strcmp(cmd, "reinit") == 0)
   {
      unsigned int board;
      DtaGetParam(&board, "Board", batch_mode, fb);
      DtaTestReinit((enum DibBoardType)board);
   }
   else if(strcmp(cmd, "emb") == 0)
   {
      unsigned int nb;
      DtaGetParam(&nb, "NbTest", batch_mode, fb);
      DtaTestEmbVersion(nb);
   }
   else if(strcmp(cmd, "abort") == 0)
   {
      int32_t status = 0;

      if(AbortThreadRunning == 0)
      {
         AbortThreadRunning = 0;
         status = OsCreateThread(&AbortThreadHandle, (PTHREAD_FN)AbortThread, (void *)NULL);

         if(status == 0)
         {
            /** Wait that thread is really started */
            while(AbortThreadRunning == 0)
               OsMSleep(1);

            printf(CRB "ABORT Thread created, status=%d" CRA, status);
         }
         else
         {
            printf(CRB "ABORT Thread create FAILED!!!!, status=%d" CRA, status);
         }
      }
   }
   else if(strcmp(cmd, "nabort") == 0)
   {
      int32_t status;

      if(AbortThreadRunning)
      {
         AbortThreadRunning = 0;

         status = OsJoinThread(AbortThreadHandle);
      }
   }
   else if(strcmp(cmd, "tune") == 0)
   {
      uint32_t rf;
      uint32_t bw;
      uint32_t Type;
      uint32_t StreamParam;

      DtaGetParam(&rf, "RF Frequency in KHz", batch_mode, fb);
      DtaGetParam(&bw, "Bandwidth in MHz", batch_mode, fb);
      DtaGetParam(&Type, "Demod (1 : dvb, 3 : dab, 7 : dvbsh, fdvb : 129, fdvbsh : 135)", batch_mode, fb);
      StreamParam = DtaGetStreamParam(batch_mode, fb);
      DtaTuneChannel((enum DibDemodType)Type, bw * 10, rf, SCAN_FLAG_PLAIN_TUNE, 0, StreamParam);
   }
   else if(strcmp(cmd, "tune_async") == 0)
   {
      uint32_t rf;
      uint32_t bw;
      uint32_t Type;
      uint32_t StreamParam;

      DtaGetParam(&rf, "RF Frequency in KHz", batch_mode, fb);
      DtaGetParam(&bw, "Bandwidth in MHz", batch_mode, fb);
      DtaGetParam(&Type, "Demod (1 : dvb, 3 : dab, 7 : dvbsh, fdvb : 129, fdvbsh : 135)", batch_mode, fb);
      StreamParam = DtaGetStreamParam(batch_mode, fb);
      DtaTuneChannel((enum DibDemodType)Type, bw * 10, rf, SCAN_FLAG_PLAIN_TUNE, 1, StreamParam);
   }
   else if(strcmp(cmd, "scan") == 0)
   {
      uint32_t Rf;
      uint32_t Bw;
      uint32_t Type;

      DtaGetParam(&Rf,   "RF Frequency in KHz", batch_mode, fb);
      DtaGetParam(&Bw,   "Bandwidth in MHz", batch_mode, fb);
      DtaGetParam(&Type, "Demod (0 : use stream, 1 : dvb, 3 : dab, 7 : dvbsh, fdvb : 129, fdvbsh : 135)", batch_mode, fb);

      if(Type == 0)
      {
         uint32_t Str;
         DtaGetParam(&Str,   "Stream Num", batch_mode, fb);
         /* Extended API */
         DtaTuneChannelEx(Str, Bw * 10, Rf, SCAN_FLAG_AUTOSEARCH, 0);
      }
      else
      {
         /* Simple API */
         uint32_t StreamParam;
         StreamParam = DtaGetStreamParam(batch_mode, fb);
         DtaTuneChannel((enum DibDemodType)Type, Bw * 10, Rf, SCAN_FLAG_AUTOSEARCH, 0, StreamParam);
      }
   }
   else if(strcmp(cmd, "monit") == 0)
   {
      uint32_t rf;
      uint32_t bw;
      uint32_t n_ts_channel;
      uint32_t Type;
      uint32_t StreamParam;

      DtaGetParam(&rf, "RF Frequency in KHz", batch_mode, fb);

      if(rf < 3000)
      {
         /* Take SBAND into account */
         rf          *= 1000;
         n_ts_channel = 0;
         bw           = 8;   /* default is 8 MHz */
      }
      else
      {
         DtaGetParam(&bw, "Bandwidth in MHz", batch_mode, fb);
      }
      DtaGetParam(&Type, "Demod (1 : dvb, 3 : dab, 7 : dvbsh, fdvb : 129, fdvbsh : 135)", batch_mode, fb);
      StreamParam = DtaGetStreamParam(batch_mode, fb);
      DtaTuneChannel((enum DibDemodType)Type, bw * 10, rf, SCAN_FLAG_AUTOSEARCH, 0, StreamParam);
   }
   else if(strcmp(cmd, "scan_async") == 0)
   {
      uint32_t rf;
      uint32_t bw;
      uint32_t n_ts_channel;
      uint32_t Type;
      uint32_t StreamParam;

      DtaGetParam(&rf, "RF Frequency in KHz", batch_mode, fb);

      if(rf < 3000)
      {
         /* Take SBAND into account */
         rf          *= 1000;
         n_ts_channel = 0;
         bw           = 8;   /* default is 8 MHz */
      }
      else
      {
         DtaGetParam(&bw, "Bandwidth in MHz", batch_mode, fb);
      }
      DtaGetParam(&Type, "Demod (1 : dvb, 3 : dab, 7 : dvbsh, fdvb : 129, fdvbsh : 135)", batch_mode, fb);
      StreamParam = DtaGetStreamParam(batch_mode, fb);
      DtaTuneChannel((enum DibDemodType)Type, bw * 10, rf, SCAN_FLAG_AUTOSEARCH, 1, StreamParam);
   }
   else if(strcmp(cmd, "freqscan") == 0)
   {
      uint32_t start;
      uint32_t stop;
      uint32_t bw;
      uint32_t Type;

      DtaGetParam(&start, "Start Freq (KHz)", batch_mode, fb);
      DtaGetParam(&stop, "Stop Freq (KHz)", batch_mode, fb);
      DtaGetParam(&bw, "Bandwidth (MHz)", batch_mode, fb);
      DtaGetParam(&Type, "Demod (1 : dvb, 3 : dab, 7 : dvbsh, fdvb : 129, fdvbsh : 135)", batch_mode, fb);
      DtaStartScan((enum DibDemodType)Type, start, stop, bw);
   }
   else if(strcmp(cmd, "freqrefresh") == 0)
   {
      DtaRefreshFreqScan();
   }
   /* Display Scan/Tune attempts statistics to stdout. */
   else if(strcmp(cmd, "freqstat") == 0)
   {
      printf(CRB "---------- SCAN/TUNE ATTEMPTS -------" CRA);
      DtaLogScanStatus(stdout);
      printf(CRB "-------------------------------------" CRA);
   }
   else if(strcmp(cmd, "freqclear") == 0)
   {
      DtaInitTuneArray();
   }
   else if(strcmp(cmd, "drvdmsk") == 0)
   {
      uint32_t Mask;

      DtaGetParamHex(&Mask, "Driver Debug Mask in HEX (32 bits)", batch_mode, fb);
      DtaSetDriverDebugMask(Mask);
   }
   else if(strcmp(cmd, "startts") == 0)
   {
      DtaStartRawTs();
   }
   else if(strcmp(cmd, "stopts") == 0)
   {
      DtaStopRawTs();
   }
   else if(strcmp(cmd, "dmsk") == 0)
   {
      uint32_t Mask;

      DtaGetParamHex(&Mask, "Embedded Debug Mask in HEX (8 bits)", batch_mode, fb);
      DtaSetDmsk((uint8_t)Mask);
   }
   else if(strcmp(cmd, "dh") == 0)
   {
      uint32_t i, v;

      DtaGetParam(&i, "IDX? ", batch_mode, fb);
      DtaGetParamHex(&v, "Value? ", batch_mode, fb);
      DtaSetDhelp((uint16_t) i, (uint16_t) v, 0);
   }
   else if(strcmp(cmd, "dhc") == 0)
   {
      uint32_t i, v, ch;

      DtaGetParam(&i, "IDX? ", batch_mode, fb);
      DtaGetParamHex(&v, "Value? ", batch_mode, fb);
      DtaGetParamHex(&ch, "Chip? ", batch_mode, fb);
      DtaSetDhelp((uint16_t) i, (uint16_t) v, (uint16_t) ch);
   }
#if (DEBUG_LIB == 1)
   else if(strcmp(cmd, "dumpdf") == 0)
   {
      static uint32_t Done = 0, FirmSize, SubFirmSize, RoData, NbTimes = 1;
      extern void IntDriverDragonflyDownloadCheck(struct DibDriverContext *pContext, uint32_t FirmSize, uint32_t SubFirmSize);
      if(Done == 0)
      {
         DtaGetParamHex(&RoData, "RoData? ", batch_mode, fb);
         DtaGetParamHex(&SubFirmSize, "SubFirmSize? ", batch_mode, fb);
         DtaGetParamHex(&FirmSize, "TextSize? ", batch_mode, fb);
         SubFirmSize = (RoData + SubFirmSize ) / 4;
         FirmSize /= 4;
         Done = 1;
      }
      else
      {
         DtaGetParam(&NbTimes, "Loop? ", batch_mode, fb);
      }
      while(NbTimes > 0)
      {
         IntDriverDragonflyDownloadCheck(CONTEXT, FirmSize, SubFirmSize);
         NbTimes--;
      }
   }
#endif
   /* Display services statistics to stdout. */
   else if(strcmp(cmd, "datastat") == 0)
   {
      printf(CRB "---------- DATA STATISTICS ----------" CRA);
      DtaGetDvbHMonitInfo(stdout, DTA_SHORT_DISPLAY, 1);
      DtaLogSiPsiStatus(stdout);
      DtaGetCmmbMonitInfo(stdout, DTA_SHORT_DISPLAY, 1);
      printf(CRB "-------------------------------------" CRA);
   }
   /* Show list of active Elementary Streams. */
   else if(strcmp(cmd, "show") == 0)
   {
      DtaShowInternal();
   }
   /* Disables all active Elementary Streams and frees all TS Channels */
   else if(strcmp(cmd, "clearall") == 0)
   {
      uint32_t i;

      for(i = 0; i < DIB_MAX_NB_CHANNELS; i++)
      {
         if(ChannelArray[i].Used)
            DtaDeleteChannel(i);
      }
   }
   /** Get power mode */
   else if(strcmp(cmd, "pm") == 0)
   {
      uint32_t StreamNum;
      DtaGetParam(&StreamNum, "StreamNum", batch_mode, fb);
      DtaGetPowerMode(StreamNum);
   }
   /** Set power mode for one stream */
   else if(strcmp(cmd, "pmon") == 0)
   {
      uint32_t StreamNum;
      DtaGetParam(&StreamNum, "StreamNum", batch_mode, fb);
      DtaSetPowerMode(eDIB_TRUE, StreamNum);
   }
   else if(strcmp(cmd, "pmoff") == 0)
   {
      uint32_t StreamNum;
      DtaGetParam(&StreamNum, "StreamNum", batch_mode, fb);
      DtaSetPowerMode(eDIB_FALSE, StreamNum);
   }

   /** Set power mode on */
   else if(strcmp(cmd, "tson") == 0)
   {
      DtaSetGlobalPowerMode(eDIB_TRUE);
   }
   /** Set power mode off */
   else if(strcmp(cmd, "tsoff") == 0)
   {
      DtaSetGlobalPowerMode(eDIB_FALSE);
   }
   else if(strcmp(cmd, "layout") == 0)
   {
      DtaDisplayLayoutInfo();
   }
   /* use  hbm mode only */
   else if(strcmp(cmd, "hbm") == 0)
   {
      DtaSetHbmMode(eHBM_MODE);
   }
   /* use bbm mode only */
   else if(strcmp(cmd, "bbm") == 0)
   {
      DtaSetHbmMode(eBBM_MODE);
   }
   /* use mixed buffering mode */
   else if(strcmp(cmd, "mbm") == 0)
   {
      DtaSetHbmMode(eBOTH_MODE);
   }
   /** Set Pid */
   else if(strcmp(cmd, "spid") == 0)
   {
      uint32_t Element;
      uint32_t Prefetch;
      DtaGetParam(&Element, "Item", batch_mode, fb);
      DtaGetParam(&Prefetch, "Mode (0 : Active, 1 : Prefetch)", batch_mode, fb);

      DtaSetPrefetch((ELEM_HDL)Element, (uint16_t)Prefetch);
   }
   else if(strcmp(cmd, "mnt") == 0)
   {
      uint32_t n_ts_channel;
      uint32_t n_demod;
      uint32_t n_iter;

      DtaGetParam(&n_ts_channel, "Channel Identifier", batch_mode, fb);
      DtaGetParam(&n_iter, "Number of iterations", batch_mode, fb);
      DtaGetParam(&n_demod, "Number of demod", batch_mode, fb);
      DtaSignalMonitLoop(n_ts_channel, n_iter, n_demod);
   }
#if (DIB_INTERNAL_DEBUG == 1) && (DF_DBG_SPAL == 1)
   else if(strcmp(cmd, "mntspal") == 0)
   {
      DtaSpalMnt();
   }
#endif
   else if(strcmp(cmd, "mntf") == 0)
   {
      uint32_t n_ts_channel;
      uint32_t n_demod;
      uint32_t n_iter;
      uint32_t period;
      char fileName[128];

      DtaGetParam(&n_ts_channel, "Channel Identifier", batch_mode, fb);
      DtaGetParam(&n_iter, "Nb of iter", batch_mode, fb);
      DtaGetParam(&n_demod, "Nb of demod", batch_mode, fb);
      DtaGetFileName(fileName, "Save to file:", batch_mode, fb);
      DtaGetParam(&period, "Period(ms)", batch_mode, fb);
      DtaSignalMonitLoopInFile(n_ts_channel, n_iter, n_demod, fileName, period);
   }
   else if(strcmp(cmd, "mntstart") == 0)
   {
      printf(CRB "------------------------" CRA);
      printf(CRB "Start Monitoring Thread" CRA);
      printf(CRB "------------------------" CRA);
      DtaStartMonit();
   }
   else if(strcmp(cmd, "mpemnt") == 0)
   {
      printf(CRB "------------------------" CRA);
      printf(CRB "Start/Stop Monitoring Thread" CRA);
      printf(CRB "------------------------" CRA);
      DtaMpeMnt();
   }
   else if(strcmp(cmd, "prof") == 0)
   {
      printf(CRB "------------------------" CRA);
      printf(CRB "Start/Stop  HBM profiler" CRA);
      printf(CRB "------------------------" CRA);
      DtaHbmProf();
   }
   else if(strcmp(cmd, "mntstop") == 0)
   {
      printf(CRB "------------------------" CRA);
      printf(CRB "Stop Monitoring Thread" CRA);
      printf(CRB "------------------------" CRA);
      DtaStopMonit();
   }
   else if(strcmp(cmd, "mntdvbh") == 0)
   {
      printf(CRB "---------- MPE STATISTICS -----------" CRA);
      DtaGetDvbHMonitInfo(stdout, DTA_FULL_DISPLAY, 1);
      printf(CRB "-------------------------------------" CRA);
   }
   else if(strcmp(cmd, "mntdvbsh") == 0)
   {
      printf(CRB "---------- MPE STATISTICS -----------" CRA);
      DtaGetDvbSHMonitInfo(stdout, DTA_FULL_DISPLAY, 1);
      printf(CRB "-------------------------------------" CRA);
   }
   else if(strcmp(cmd, "mntsig") == 0)
   {
      uint32_t n_channel;
      uint32_t n_demod;

      DtaGetParam(&n_channel, "Channel Number", batch_mode, fb);
      DtaGetParam(&n_demod, "Nb Demod (0=AUTO)", batch_mode, fb);
      printf(CRB "---- START MONIT (CHANNEL_ID = %d, NB_DEMOD = %d)---" CRA, n_channel, n_demod);
      DtaGetSignalMonitInfo(n_channel, (uint8_t)n_demod, DTA_SHORT_DISPLAY, 0, stdout);
      printf(CRB "-------------------------------------" CRA);
   }
   else if(strcmp(cmd, "mntsigsh") == 0)
   {
      printf(CRB "---- START MONIT (CHANNEL_ID 0)---" CRA);
      DtaGetSHMonitInfo(0);
      printf(CRB "-------------------------------------" CRA);
   }
   else if(strcmp(cmd, "mntsig_async") == 0)
   {
      uint32_t n_channel;
      uint32_t n_demod;

      DtaGetParam(&n_channel, "Channel Number", batch_mode, fb);
      DtaGetParam(&n_demod, "Demod number", batch_mode, fb);
      printf(CRB "---- START MONIT (CHANNEL_ID = %d, DEMOD = %d)---" CRA, n_channel, n_demod);
      DtaGetSignalMonitInfo(n_channel, (uint8_t)n_demod, DTA_SHORT_DISPLAY, 1, stdout);
      printf(CRB "-------------------------------------" CRA);
   }
   else if(strcmp(cmd, "mntrefresh") == 0)
   {
      DtaRefreshMonitArray();
   }
   /* Display Monit attempts statistics to stdout. */
   else if(strcmp(cmd, "mntstat") == 0)
   {
      printf(CRB "---------- MONIT ATTEMPTS -----------" CRA);
      DtaLogMonitStatus(stdout);
      printf(CRB "-------------------------------------" CRA);
   }
   else if(strcmp(cmd, "mntclear") == 0)
   {
      DtaClearAllMonitArray();
   }
   else if(strcmp(cmd, "readsection") == 0)
   {
      uint32_t ItemId;
      uint32_t n_iter;

      DtaGetParam(&ItemId, "Item Identifier", batch_mode, fb);
      DtaGetParam(&n_iter, "Time in millisecond", batch_mode, fb);
      DtaReadSiPsiSection(ItemId, n_iter);
   }

#if (DEBUG_LIB == 1)
   /*----------------------------------------------------------------
   DEBUG COMMAND
   ----------------------------------------------------------------*/
   else if(strcmp(cmd, "readregi") == 0)
   {
      uint32_t Addr;

      printf(CRB "------------------------" CRA);
      DtaGetParamHex(&Addr, "Addr", batch_mode, fb);
      DtaDebugReadRegInternal(Addr);
      printf(CRB "------------------------" CRA);
   }
   else if(strcmp(cmd, "writeregi") == 0)
   {
      uint32_t Addr;
      uint32_t Data;

      printf(CRB "------------------------" CRA);
      DtaGetParamHex(&Addr, "Addr", batch_mode, fb);
      DtaGetParam(&Data, "Data", batch_mode, fb);
      DtaDebugWriteRegInternal(Addr, Data);
      printf(CRB "------------------------" CRA);
   }
   else if(strcmp(cmd, "r16") == 0)
   {
      uint32_t Addr;
      printf(CRB "------------------------" CRA);
      DtaGetParamHex(&Addr, "Addr", batch_mode, fb);
      DtaDebugReadReg16(Addr);
      printf(CRB "------------------------" CRA);
   }
   else if(strcmp(cmd, "rb32") == 0)
   {
      uint32_t Addr;
      uint32_t Size;
      printf(CRB "------------------------" CRA);
      DtaGetParamHex(&Addr, "Addr", batch_mode, fb);
      DtaGetParam(&Size, "Size", batch_mode, fb);
      DtaDebugReadBuf32(Addr,Size);
      printf(CRB "------------------------" CRA);
   }
   else if(strcmp(cmd, "r32") == 0)
   {
      uint32_t Addr;
      printf(CRB "------------------------" CRA);
      DtaGetParamHex(&Addr, "Addr", batch_mode, fb);
      DtaDebugReadReg32(Addr);
      printf(CRB "------------------------" CRA);
   }
   else if(strcmp(cmd, "w16") == 0)
   {
      uint32_t Addr;
      uint32_t Data;
      printf(CRB "------------------------" CRA);
      DtaGetParamHex(&Addr, "Addr", batch_mode, fb);
      DtaGetParamHex(&Data, "Data", batch_mode, fb);
      DtaDebugWriteReg16(Addr, Data);
      printf(CRB "------------------------" CRA);
   }
   else if(strcmp(cmd, "w32") == 0)
   {
      uint32_t Addr;
      uint32_t Data;
      printf(CRB "------------------------" CRA);
      DtaGetParamHex(&Addr, "Addr", batch_mode, fb);
      DtaGetParamHex(&Data, "Data", batch_mode, fb);
      DtaDebugWriteReg32(Addr, Data);
      printf(CRB "------------------------" CRA);
   }
   else if(strcmp(cmd, "dump") == 0)
   {
      printf(CRB "------------------------" CRA);
      DtaDebugDumpReg();
      printf(CRB "------------------------" CRA);
   }
#endif

   else if(strcmp(cmd, "pause") == 0)
   {
      uint32_t delay;

      DtaGetParam(&delay, "delay", batch_mode, fb);
      OsMSleep(delay);
   }
   else if(strcmp(cmd, "settime") == 0)
   {
      uint32_t delay;

      DtaGetParam(&delay, "factor", batch_mode, fb);
      FactorTime = delay;
   }
   else if(strcmp(cmd, "fpause") == 0)
   {
      uint32_t delay;

      DtaGetParam(&delay, "delay", batch_mode, fb);
      OsMSleep(FactorTime * delay);
   }
   else if(strcmp(cmd, "wait") == 0)
   {
      getchar();
   }

   else if(strcmp(cmd, "rpause") == 0)
   {
      double time = (double)OsRandom() / RAND_MAX;
      uint32_t delay;

      DtaGetParam(&delay, "delay", batch_mode, fb);
      time *= delay;

      OsMSleep((uint32_t)(time));
   }
   else if(strcmp(cmd, "batch") == 0)
   {
      char fname[20];             /* File name. */
      char bcmd[100];             /* Command name */

      DtaGetFileName(fname, "batch command file", batch_mode, fb);
      fb = fopen(fname, "r");

      if(!fb)
      {
         perror("fopen");
         fprintf(stderr, CRB "%s: *** Error: Cannot open file %s %#x" CRA, __FUNCTION__,
            fname, errno);
         return;
      }

      while(fscanf(fb, "%s", bcmd) != EOF)
      {
         DtaProcessCommand(bcmd, 1, fb);
      }
      fclose(fb);
   }
   else if(strcmp(cmd, "batchl") == 0)
   {
      char fname[20];             /* File name. */
      uint32_t loop;
      uint32_t i;
      char bcmd[100];             /* Command name */

      DtaGetFileName(fname, "batch command file", batch_mode, fb);
      DtaGetParam(&loop, "Nb Loop", batch_mode, fb);
      for(i = 0; i < loop; i++)
      {
          fprintf(stderr, CRB "BATCH LOOP #%d / #%d\n" CRA, i+1, loop);
         fb = fopen(fname, "r");
         if(!fb)
         {
            fprintf(stderr, CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
            return;
         }

         while(fscanf(fb, "%s", bcmd) != EOF)
         {
            DtaProcessCommand(bcmd, 1, fb);
         }
         fclose(fb);
      }
   }
   /* Log constant string to file. */
   else if(strcmp(cmd, "log") == 0)
   {
      char  string[100];
      char  fname[30];
      FILE *fdlog;

#ifdef WINCE
      OsDtaGetLogFileName(fname, "log file", batch_mode, fb);
#else
	  DtaGetFileName(fname, "log file", batch_mode, fb);
#endif
      DtaGetString(string, batch_mode, fb);
      fdlog = fopen(fname, "a");

      if(!fdlog)
      {
         printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
         return;
      }

      fprintf(fdlog, CRB "%s" CRA, string);
      fclose(fdlog);
   }
   /* Log Data and channel statistics to file. */
   else if(strcmp(cmd, "logdrv") == 0)
   {
      char  fname[30];
      FILE *fdlog;

#ifdef WINCE
      OsDtaGetLogFileName(fname, "log file", batch_mode, fb);
#else
	  DtaGetFileName(fname, "log file", batch_mode, fb);
#endif
      fdlog = fopen(fname, "a");

      if(!fdlog)
      {
         printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
         return;
      }

      fprintf(fdlog, CRB "---------- DATA STATISTICS ----------" CRA);
      DtaGetDvbHMonitInfo(fdlog, DTA_SHORT_DISPLAY, 1);
      DtaLogSiPsiStatus(fdlog);
      DtaGetCmmbMonitInfo(fdlog, DTA_SHORT_DISPLAY, 1);
      fprintf(fdlog, CRB "-------------------------------------" CRA);

      fprintf(fdlog, CRB "---------- CHANNEL STATISTICS -------" CRA);
      DtaLogScanStatus(fdlog);
      DtaLogMonitStatus(fdlog);
      fprintf(fdlog, CRB "-------------------------------------" CRA);

      fclose(fdlog);
   }
   /* Log current time along with label to file. */
   else if(strcmp(cmd, "logtime") == 0)
   {
      char  string[100];
      char  fname[30];
      FILE *fdlog;

#ifdef WINCE
      OsDtaGetLogFileName(fname, "log file", batch_mode, fb);
#else
	  DtaGetFileName(fname, "log file", batch_mode, fb);
#endif
      DtaGetString(string, batch_mode, fb);

      fdlog = fopen(fname, "a");

      if(!fdlog)
      {
         printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
         return;
      }
      DtaTimeLogDate(fdlog, string);
      fclose(fdlog);
   }
   else if(!strcmp(cmd, "logrt"))
   {
      char Mode[10];

      DtaGetString(Mode, batch_mode, fb);

      if(!strcmp(Mode, "start"))
         DtaRealTimeLogInit();
      else if(!strcmp(Mode, "stop"))
         DtaRealTimeLogExit();
      else
      {
         printf(CRB "%s: *** Error: Illegal Mode for real-time trace (%s)" CRA, __FUNCTION__, Mode);
         return;
      }
   }
   /* Log MPE and SI/PSI Data statistics to file. */
   else if(strcmp(cmd, "logd") == 0)
   {
      char  fname[30];
      FILE *fdlog;

#ifdef WINCE
      OsDtaGetLogFileName(fname, "log file", batch_mode, fb);
#else
	  DtaGetFileName(fname, "log file", batch_mode, fb);
#endif
      fdlog = fopen(fname, "a");
      if(!fdlog)
      {
         printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
         return;
      }

      fprintf(fdlog, CRB "---------- DATA STATISTICS ----------" CRA);
      DtaGetDvbHMonitInfo(fdlog, DTA_SHORT_DISPLAY, 1);
      DtaGetDvbSHMonitInfo(fdlog, DTA_SHORT_DISPLAY, 1);
      DtaLogSiPsiStatus(fdlog);
      DtaGetCmmbMonitInfo(fdlog, DTA_SHORT_DISPLAY, 1);
      fprintf(fdlog, CRB "-------------------------------------" CRA);

      fclose(fdlog);
   }
   /* Log DAB Data statistics to file. */
   else if(strcmp(cmd, "loga") == 0)
   {
      char  fname[30];
      uint32_t type;
      FILE *fdlog;

#ifdef WINCE
      OsDtaGetLogFileName(fname, "log file", batch_mode, fb);
#else
	   DtaGetFileName(fname, "log file", batch_mode, fb);
#endif

      DtaGetParam(&type, "filter Type", batch_mode, fb);
      fdlog = fopen(fname, "a");
      if(!fdlog)
      {
         printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
         return;
      }

      fprintf(fdlog, CRB "---------- DATA STATISTICS ----------" CRA);
      DtaGetDabMonitInfo(fdlog, type, DTA_SHORT_DISPLAY);
      fprintf(fdlog, CRB "-------------------------------------" CRA);

      fclose(fdlog);
   }
   /* Log channel Scan/Tune/Monit statistics to file. */
   else if(strcmp(cmd, "logc") == 0)
   {
      char fname[30];
      FILE *fdlog;

#ifdef WINCE
      OsDtaGetLogFileName(fname, "log file", batch_mode, fb);
#else
	  DtaGetFileName(fname, "log file", batch_mode, fb);
#endif
      fdlog = fopen(fname, "a");

      if(!fdlog)
      {
         printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
         return;
      }

      fprintf(fdlog, CRB "---------- CHANNEL STATISTICS -------" CRA);
      DtaLogScanStatus(fdlog);
      DtaLogMonitStatus(fdlog);
      fprintf(fdlog, CRB "-------------------------------------" CRA);

      fclose(fdlog);
   }
   /* Log Ts data info */
   else if(strcmp(cmd, "logts") == 0)
   {
      char fname[30];
      FILE *fdlog;

#ifdef WINCE
      OsDtaGetLogFileName(fname, "log file", batch_mode, fb);
#else
	  DtaGetFileName(fname, "log file", batch_mode, fb);
#endif
      if (strcmp(fname, "console") == 0)
      {
        fdlog = NULL;
      }
      else
      {
        fdlog = fopen(fname, "a");

        if(!fdlog)
        {
          printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, fname);
          return;
        }
      }

      DtaLogTsStatus(fdlog);

      if (fdlog != NULL)
        fclose(fdlog);
   }
   else if(strcmp(cmd, "gstr") == 0)
   {
      uint32_t StreamNum;
      uint32_t Type;
      uint32_t Options;
      uint32_t EnPowerSaving;

      DtaGetParam(&StreamNum, "Stream Number", batch_mode, fb);
      if(StreamNum >= DIB_MAX_NB_OF_STREAMS)
      {
         printf("StreamNum out of bounds\n");
         return;
      }
      Type = DtaGetStandard(batch_mode,fb);
      Options = DtaGetStreamOptions(StreamArray[StreamNum].Attr.Type, batch_mode, fb);
      DtaGetParam(&EnPowerSaving, "EnPowerSaving (0: off, 1: on, 2:auto)", batch_mode, fb);
      DtaGetStream(StreamNum, Type, Options, EnPowerSaving);
   }
   else if(strcmp(cmd, "dstr") == 0)
   {
      uint32_t StreamNum;

      DtaGetParam(&StreamNum, "Stream Number", batch_mode, fb);
      DtaDeleteStream(StreamNum);
   }
   else if(strcmp(cmd, "gfe") == 0)
   {
      uint32_t StreamNum;
      uint32_t FeNum;

      DtaGetParam(&StreamNum, "Stream Number", batch_mode, fb);
      DtaGetParam(&FeNum, "Frontend Number", batch_mode, fb);
      DtaGetFrontend(StreamNum, FeNum, -1 /*FeOutputNum*/, 0/*Force*/);
   }
   else if(strcmp(cmd, "dfe") == 0)
   {
      uint32_t FeNum;

      DtaGetParam(&FeNum, "Frontend Number", batch_mode, fb);
      DtaDeleteFrontend(FeNum);
   }

   else if(strcmp(cmd, "k1") == 0)
   {
      uint32_t Val;
      DtaGetParam(&Val, "K1 ", batch_mode, fb);
      DtaSetDhelp((uint8_t) 3, (uint8_t) Val, 0);
   }
   else if(strcmp(cmd, "k2") == 0)
   {
      uint32_t Val;
      DtaGetParam(&Val, "K2 ", batch_mode, fb);
      DtaSetDhelp((uint8_t) 4, (uint8_t) Val, 0);
   }
   else if(strcmp(cmd, "p") == 0)
   {
      uint32_t Val;
      DtaGetParam(&Val, "PERIOD_MS ", batch_mode, fb);
      DtaSetDhelp((uint8_t) 5, (uint8_t) Val, 0);
   }

   else if(strcmp(cmd, "gch") == 0)
   {
      uint32_t Rf;
      uint32_t Bw;
      uint32_t Type;
      uint32_t ChId;

      DtaGetParam(&Rf,   "RF Frequency in KHz", batch_mode, fb);
      DtaGetParam(&Bw,   "Bandwidth in MHz", batch_mode, fb);
      Type = DtaGetStandard(batch_mode,fb);

      /* Convert it to the enum */
      Bw = Bw * 10;

      if(Type == 0)
      {
         /* Extended API */
         struct DibTuneChan TuneChan;
         uint32_t           Str;
         uint32_t           Async;
         uint32_t           ScanType;

         DtaGetParam(&Str,      "Stream Num", batch_mode, fb);
         DtaGetParam(&ScanType, "Phy Param: 0 : Scan, 1 : Fast Scan, 2 : Use cache, 3 : ALWAYS_SUCCESS, 4 : Automatic Tune ,5+ : Prompt", batch_mode, fb);
         DtaGetParam(&ChId,     "Channel Identifier (store the Channel handler)", batch_mode, fb);
         DtaGetParam(&Async,    "Asynchronous", batch_mode, fb);

         if(Str >= globalInfo.NumberOfStreams)
         {
            printf("-E Invalid Stream number\n");
            return;
         }
         if(StreamArray[Str].Used == 0)
         {
            printf("-E Stream not found\n");
            return;
         }

         Type = StreamArray[Str].Std;

         TuneChan.Bw      = Bw;
         TuneChan.RFkHz   = Rf;
         TuneChan.InvSpec = -1;

         if((ScanType == 0) || (ScanType == 1))
         {
            /* gch with auto-seach */
            memset(&TuneChan.Std, -1, sizeof(TuneChan.Std));
            if (Type == eSTANDARD_ISDBT_1SEG) {
                TuneChan.Std.Isdbt.SbMode            = 1;
                TuneChan.Std.Isdbt.PartialReception  = 0; /* tune in 1 seg not in 3 seg */
            }

            ScanType = (ScanType == 0) ? SCAN_FLAG_AUTOSEARCH : SCAN_FLAG_FAST_SEARCH;
         }
         else if(ScanType == 2)
         {
            /* gch with parameters found in the cache */
            ScanType = SCAN_FLAG_PLAIN_TUNE;
            if(GetTuneChan(Rf, Bw, &TuneChan) == DTA_NO_INDEX)
            {
               printf(CRB "-E Phy Param not found in the cache" CRA);
               return;
            }
         }
         if(ScanType == 3)
         {
            /* gch with auto-seach */
            memset(&TuneChan.Std, -1, sizeof(TuneChan.Std));
            if (Type == eSTANDARD_ISDBT_1SEG) {
                TuneChan.Std.Isdbt.SbMode            = 1;
                TuneChan.Std.Isdbt.PartialReception  = 0; /* tune in 1 seg not in 3 seg */
            }

            ScanType = SCAN_FLAG_ALWAYS_TUNE_SUCCESS;
         }
         if(ScanType == 4)
         {
            ScanType = SCAN_FLAG_AUTOTUNE;
            memset(&TuneChan.Std, -1, sizeof(TuneChan.Std));
            if (Type == eSTANDARD_ISDBT_1SEG) {
                TuneChan.Std.Isdbt.SbMode            = 1;
                TuneChan.Std.Isdbt.PartialReception  = 0; /* tune in 1 seg not in 3 seg */
            }
         }
         else if(ScanType >= 5)
         {
            /* gch with parameters given by the user */
            ScanType = SCAN_FLAG_MANUAL_TUNE;

            switch(Type)
            {
#if (DIB_DVB_STD == 1)
               case eSTANDARD_DVB:
               {
                  uint32_t InvSpec;
                  uint32_t Nfft;
                  uint32_t Guard;
                  uint32_t Nqam;
                  uint32_t IntlvNative;
                  uint32_t VitHrch;
                  uint32_t VitAlpha;
                  uint32_t VitCodeRateHp;
                  uint32_t VitCodeRateLp;

                  DtaGetParam(&InvSpec,       "InvSpec (0: no_iv, 1:inv)", batch_mode, fb);
                  DtaGetParam(&Nfft,          "Nfft (0:2K, 1:8K, 2:4K, 3:1K)", batch_mode, fb);
                  DtaGetParam(&Guard,         "Guard (0:1/32, 1:1/16, 2:1/8, 3:1/4)", batch_mode, fb);
                  DtaGetParam(&Nqam,          "Nqam (0:QPSK, 1:16QAM, 3:64QAM, 4:DQPSK)", batch_mode, fb);
                  DtaGetParam(&IntlvNative,   "IntlvNative (0:OFF, 1:ON)", batch_mode, fb);
                  DtaGetParam(&VitHrch,       "VitHrch (0:OFF, 1:ON)", batch_mode, fb);
                  DtaGetParam(&VitAlpha,      "VitAlpha (1:alpha1, 2:alpha2, 4: alpha4)", batch_mode, fb);
                  DtaGetParam(&VitCodeRateHp, "VitCodeRateHp (1:1/2, 2:2/3, 3:3/4, 5:5/6, 7:7/8)", batch_mode, fb);
                  DtaGetParam(&VitCodeRateLp, "VitCodeRateLp", batch_mode, fb);

                  TuneChan.InvSpec               = InvSpec;
                  TuneChan.Std.Dvb.Nfft          = Nfft;
                  TuneChan.Std.Dvb.Guard         = Guard;
                  TuneChan.Std.Dvb.Nqam          = Nqam;
                  TuneChan.Std.Dvb.IntlvNative   = IntlvNative;
                  TuneChan.Std.Dvb.VitHrch       = VitHrch;
                  TuneChan.Std.Dvb.VitSelectHp   = eDIB_VIT_PRIORITY_AUTO;
                  TuneChan.Std.Dvb.VitAlpha      = VitAlpha;
                  TuneChan.Std.Dvb.VitCodeRateHp = VitCodeRateHp;
                  TuneChan.Std.Dvb.VitCodeRateLp = VitCodeRateLp;
                  break;
               }
#endif
#if (DIB_ISDBT_STD == 1)
               case eSTANDARD_ISDBT:
               {
                  uint32_t InvSpec;
                  uint32_t Nfft;
                  uint32_t Guard;
                  uint32_t SbMode;
                  uint32_t PartialReception;
                  uint32_t SbNbTotCon;
                  uint32_t SbNbWantedSeg;
                  uint32_t SbSubCh;

                  uint32_t Nqam[3];
                  uint32_t CodeRate[3];
                  uint32_t NbSeg[3];
                  uint32_t TimeIntlv[3];

                  DtaGetParam(&InvSpec, "InvSpec (0: no_iv, 1:inv)", batch_mode, fb);
                  DtaGetParam(&Nfft, "Nfft (0:2K, 1:8K, 2:4K, 3:1K)", batch_mode, fb);
                  DtaGetParam(&Guard, "Guard (0:1/32, 1:1/16, 2:1/8, 3:1/4)", batch_mode, fb);
                  DtaGetParam(&SbMode, "SbMode (0 or 1)", batch_mode, fb);
                  DtaGetParam(&PartialReception, "PartialReception (0 or 1)", batch_mode, fb);
                  DtaGetParam(&SbNbTotCon, "SbNbTotCon", batch_mode, fb);
                  DtaGetParam(&SbNbWantedSeg, "SbNbWantedSeg", batch_mode, fb);
                  DtaGetParam(&SbSubCh, "SbSubCh", batch_mode, fb);
                  DtaGetParam(&Nqam[0], "ConstellationA (0:QPSK, 1:16QAM, 3:64QAM, 4:DQPSK)", batch_mode, fb);
                  DtaGetParam(&Nqam[1], "ConstellationB (0:QPSK, 1:16QAM, 3:64QAM, 4:DQPSK)", batch_mode, fb);
                  DtaGetParam(&Nqam[2], "ConstellationC (0:QPSK, 1:16QAM, 3:64QAM, 4:DQPSK)", batch_mode, fb);
                  DtaGetParam(&CodeRate[0], "CodeRateA (1:1/2, 2:2/3, 3:3/4, 5:5/6, 7:7/8)", batch_mode, fb);
                  DtaGetParam(&CodeRate[1], "CodeRateB (1:1/2, 2:2/3, 3:3/4, 5:5/6, 7:7/8)", batch_mode, fb);
                  DtaGetParam(&CodeRate[2], "CodeRateC (1:1/2, 2:2/3, 3:3/4, 5:5/6, 7:7/8)", batch_mode, fb);
                  DtaGetParam(&NbSeg[0], "NbSegA", batch_mode, fb);
                  DtaGetParam(&NbSeg[1], "NbSegB", batch_mode, fb);
                  DtaGetParam(&NbSeg[2], "NbSegC", batch_mode, fb);
                  DtaGetParam(&TimeIntlv[0], "TimeIntlvA (0,1,2,3)", batch_mode, fb);
                  DtaGetParam(&TimeIntlv[1], "TimeIntlvB (0,1,2,3)", batch_mode, fb);
                  DtaGetParam(&TimeIntlv[2], "TimeIntlvC (0,1,2,3)", batch_mode, fb);

                  TuneChan.InvSpec                          = InvSpec;
                  TuneChan.Std.Isdbt.Nfft                   = Nfft;
                  TuneChan.Std.Isdbt.Guard                  = Guard;
                  TuneChan.Std.Isdbt.SbMode                 = SbMode;
                  TuneChan.Std.Isdbt.PartialReception       = PartialReception;
                  TuneChan.Std.Isdbt.SbConnTotalSeg         = SbNbTotCon;
                  TuneChan.Std.Isdbt.SbWantedSeg            = SbNbWantedSeg;
                  TuneChan.Std.Isdbt.SbSubchannel           = SbSubCh;
                  TuneChan.Std.Isdbt.layer[0].Constellation = (enum DibConstellation)Nqam[0];
                  TuneChan.Std.Isdbt.layer[1].Constellation = (enum DibConstellation)Nqam[1];
                  TuneChan.Std.Isdbt.layer[2].Constellation = (enum DibConstellation)Nqam[2];
                  TuneChan.Std.Isdbt.layer[0].CodeRate      = (enum DibVitCoderate)CodeRate[0];
                  TuneChan.Std.Isdbt.layer[1].CodeRate      = (enum DibVitCoderate)CodeRate[1];
                  TuneChan.Std.Isdbt.layer[2].CodeRate      = (enum DibVitCoderate)CodeRate[2];
                  TuneChan.Std.Isdbt.layer[0].NbSegments    = NbSeg[0];
                  TuneChan.Std.Isdbt.layer[1].NbSegments    = NbSeg[1];
                  TuneChan.Std.Isdbt.layer[2].NbSegments    = NbSeg[2];
                  TuneChan.Std.Isdbt.layer[0].TimeIntlv     = TimeIntlv[0];
                  TuneChan.Std.Isdbt.layer[1].TimeIntlv     = TimeIntlv[1];
                  TuneChan.Std.Isdbt.layer[2].TimeIntlv     = TimeIntlv[2];
                  break;
               }
#endif
#if (DIB_CTTB_STD == 1)
               case eSTANDARD_CTTB:
               {
                  uint32_t Nfft;
                  uint32_t Intlv;
                  uint32_t Guard;
                  uint32_t Nqam;
                  uint32_t CodeRate;

                  DtaGetParam(&Intlv,     "Intlv (0: short, 1:long)", batch_mode, fb);
                  DtaGetParam(&Guard,     "Guard (4:PN420, 5:PN595, 6:PN945)", batch_mode, fb);
                  DtaGetParam(&Nqam,      "Constellation (0:QPSK, 1:16QAM, 2:32QAM, 3:64QAM, 9:QPSK-NR)", batch_mode, fb);
                  DtaGetParam(&CodeRate,  "CodeRate (0:0.4, 1:0.6, 2:0.8)", batch_mode, fb);
                  DtaGetParam(&Nfft,      "FFT (2:4K, 4:single)", batch_mode, fb);

                  TuneChan.Std.Cttb.Nfft                    = Nfft;
                  TuneChan.Std.Cttb.Guard                   = Guard;
                  TuneChan.Std.Cttb.Nqam                    = Nqam;
                  TuneChan.Std.Cttb.LdpcCodeRate            = CodeRate;
                  TuneChan.Std.Cttb.TimeIntlv               = Intlv;
                  break;
               }
#endif

               default:
                  printf("standard not prompatble, autosearch forced\n");
                  memset(&TuneChan.Std, -1, sizeof(TuneChan.Std));
                  ScanType = SCAN_FLAG_AUTOSEARCH;
                  break;
            }
         }
         DtaGetChannelEx(ChId, Str, &TuneChan, Async, ScanType);
      }
      else
      {
         /* Simple API */
         uint32_t StreamParam;
         StreamParam = DtaGetStreamParam(batch_mode, fb);
         DtaGetParam(&ChId,     "Channel Identifier (store the Channel handler)", batch_mode, fb);
         DtaGetChannel(ChId, DIB_DEMOD_AUTO, gMaxDemod, StreamParam, (enum DibDemodType)Type, Rf, Bw, 0, SCAN_FLAG_AUTOSEARCH);
      }
   }
   else if(strcmp(cmd, "gdemod") == 0)
   {
      printf(CRB "NbWantedDemod=%d%s" CRA,gMaxDemod,(gMaxDemod==DIB_DEMOD_AUTO)?" (AUTO)":"");
   }
   else if(strcmp(cmd, "sdemod") == 0)
   {
      uint32_t MaxDemod;
      DtaGetParam(&MaxDemod, "NbWantedDemod (1..MAX_FE | 0=AUTO)", batch_mode, fb);
      gMaxDemod = MaxDemod;
   }

   else if(strcmp(cmd, "gch_async") == 0)
   {
      uint32_t rf;
      uint32_t bw;
      uint32_t Type;
      uint32_t StreamParam;
      uint32_t ChId;

      DtaGetParam(&rf, "RF Frequency in KHz", batch_mode, fb);
      DtaGetParam(&bw, "Bandwidth in MHz", batch_mode, fb);
      Type = DtaGetStandard(batch_mode,fb);
      StreamParam = DtaGetStreamParam(batch_mode, fb);
      DtaGetParam(&ChId, "Channel Identifier (store the Channel handler)", batch_mode, fb);
      DtaGetChannel(ChId, DIB_DEMOD_AUTO, gMaxDemod, StreamParam, (enum DibDemodType)Type, rf, (enum DibSpectrumBW)(bw*10),1,SCAN_FLAG_AUTOSEARCH);
   }
   else if(strcmp(cmd, "dch") == 0)
   {
      uint32_t idx = 0;
      DtaGetParam(&idx, "Channel Indentifier", batch_mode, fb);
      DtaDeleteChannel(idx);
   }
   else if(strcmp(cmd, "ts_start") == 0)
   {
      uint32_t idx = 0;
      DtaGetParam(&idx, "idx", batch_mode, fb);
      DtaTsMonitStart(idx);
   }
   else if(strcmp(cmd, "ts_stop") == 0)
   {
      DtaTsMonitStop();
   }
   else if(strcmp(cmd, "ts_free") == 0)
   {
      uint32_t idx = 0;
      DtaGetParam(&idx, "idx", batch_mode, fb);
      DtaTsMonitFree(idx);
   }
   else if(strcmp(cmd, "ts_rec") == 0)
   {
      uint32_t enable = 0;
      DtaGetParam(&enable, "Enable: 1, Disable: 0", batch_mode, fb);
      DtaTsRecordEnable(enable);
   }
   else if(strcmp(cmd, "asf") == 0)
   {
      uint32_t idx = 0;
      uint32_t i,val;
      struct DibSipsiFilterCfg SetFilter;
      DtaGetParam(&idx, "Item Indentifier", batch_mode, fb);
      printf(CRB "BitMaskEqu" CRA);
      for(i=0; i<12; i++)
      {
         DtaGetParam(&val, "", batch_mode, fb);
         SetFilter.BitMaskEqu.Byte[i] = val;
      }
      printf(CRB "BitMaskXor" CRA);
      for(i=0; i<12; i++)
      {
         DtaGetParam(&val, "", batch_mode, fb);
         SetFilter.BitMaskXor.Byte[i] = val;
      }
      printf(CRB "Value" CRA);
      for(i=0; i<12; i++)
      {
         DtaGetParam(&val, "", batch_mode, fb);
         SetFilter.Value.Byte[i] = val;
      }
      SetFilter.Enable = eDIB_TRUE;
      SetFilter.ItemHdl = ItemArray[idx].ItemHdl;
      if(idx >= DIB_MAX_NB_ITEMS || !ItemArray[idx].Used)
      {
         printf(CRB "Item Identifier %d unknown" CRA, idx);
         return;
      }

      DibSetConfig(CONTEXT, eSIPSI_MASK, (union DibParamConfig *)&SetFilter);
   }
   else if(strcmp(cmd, "dsf") == 0)
   {
      uint32_t idx = 0;
      struct DibSipsiFilterCfg SetFilter;
      DtaGetParam(&idx, "Item Indentifier", batch_mode, fb);
      if(idx >= DIB_MAX_NB_ITEMS || !ItemArray[idx].Used)
      {
         printf(CRB "Item Identifier %d unknown" CRA, idx);
         return;
      }
      SetFilter.Enable = eDIB_FALSE;
      SetFilter.ItemHdl = ItemArray[idx].ItemHdl;

      DibSetConfig(CONTEXT, eSIPSI_MASK, (union DibParamConfig *)&SetFilter);
   }

   else if(strcmp(cmd, "gfi") == 0)
   {
      uint32_t Type, ch;
      uint32_t idx = 0;

      DtaGetParam(&ch, "Channel Identifier", batch_mode, fb);
      Type = DtaGetData(batch_mode, fb);
      DtaGetParam(&idx, "Filter Indentifier", batch_mode, fb);
      DtaCreateFilter(idx, ch, (enum DibDataType) Type, 0);
   }
   else if(strcmp(cmd, "gfi_async") == 0)
   {
      uint32_t Type, ch;
      uint32_t idx = 0;

      DtaGetParam(&ch, "Channel Identifier", batch_mode, fb);
      Type = DtaGetData(batch_mode, fb);
      DtaGetParam(&idx, "Filter Indentifier", batch_mode, fb);
      DtaCreateFilter(idx, ch, (enum DibDataType) Type, 1);
   }
   else if(strcmp(cmd, "dfi") == 0)
   {
      uint32_t idx = 0;

      DtaGetParam(&idx, "Filter Identifier", batch_mode, fb);
      DtaDeleteFilter(idx);
   }
   else if(strcmp(cmd, "psistart") == 0)
   {
      DtaStartSiPsi();
   }
   else if(strcmp(cmd, "psistop") == 0)
   {
      DtaStopSiPsi();
   }
#if (DIB_MPEFEC_DATA == 1)
   else if(strcmp(cmd, "atfp") == 0)
   {
      uint32_t FilterId   = 0;
      uint32_t FilterType = 0;
      uint32_t var        = 0;
      uint32_t ItemId;
      union DibFilters FilDesc;

      DtaGetParam(&FilterId, "Filter Identifier", batch_mode, fb);
      FilterType = FilterArray[FilterId].DataType;

      DtaGetParam(&var, "Pid", batch_mode, fb);
      FilDesc.MpeFec.Pid = var;

      DtaGetParam(&var, "Nb rows", batch_mode, fb);
      FilDesc.MpeFec.NbRows = (enum DibFecNbrows)var;

      DtaGetParam(&var, "max burst duration", batch_mode, fb);
      FilDesc.MpeFec.MaxBurstDuration = var;

      if(FilDesc.MpeFec.MaxBurstDuration == 0)
         FilDesc.MpeFec.MaxBurstDuration = DIB_MAX_BURST_TIME_UNKNOWN;

      FilDesc.MpeFec.Prefetch = ePREFETCH;
      DtaGetParam(&ItemId, "Item Identifier", batch_mode, fb);

      DtaAddToFilter(ItemId, FilterId, &FilDesc, FilterType);
   }
#endif
   else if(strcmp(cmd, "atf") == 0)
   {
      uint32_t FilterId   = 0;
      uint32_t FilterType = 0;
      uint32_t var        = 0;
      uint32_t ItemId;
      union DibFilters FilDesc;

      DtaGetParam(&FilterId, "Filter Identifier", batch_mode, fb);
      FilterType = FilterArray[FilterId].DataType;
      switch(FilterType)
      {
#if (DIB_RAWTS_DATA == 1)
      case eTS:
      case eLOWSPEEDTS:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.Ts.Pid = var;
         break;
#endif

#if (DIB_PES_DATA == 1)
      case ePESVIDEO:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.PesVideo.Pid = var;
         break;
      case ePESAUDIO:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.PesAudio.Pid = var;
         break;
      case ePESOTHER:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.PesOther.Pid = var;
         break;
      case ePCR:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.Pcr.Pid = var;
         break;
#endif

#if (DIB_SIPSI_DATA == 1)
      case eSIPSI:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.SiPsi.Pid = var;

         DtaGetParam(&var, "Pid watch", batch_mode, fb);
         FilDesc.SiPsi.PidWatch = (enum DibBool)var;

         DtaGetParam(&var, "crc", batch_mode, fb);
         FilDesc.SiPsi.Crc = (enum DibBool)var;
         break;
#endif

#if (DIB_MPEFEC_DATA == 1)
      case eMPEFEC:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.MpeFec.Pid = var;

         DtaGetParam(&var, "Nb rows", batch_mode, fb);
         FilDesc.MpeFec.NbRows = (enum DibFecNbrows)var;

         DtaGetParam(&var, "max burst duration", batch_mode, fb);
         FilDesc.MpeFec.MaxBurstDuration = var;

         if(FilDesc.MpeFec.MaxBurstDuration == 0)
            FilDesc.MpeFec.MaxBurstDuration = DIB_MAX_BURST_TIME_UNKNOWN;

         FilDesc.MpeFec.Prefetch = eACTIVE;
         break;
#endif

#if (DIB_MPEIFEC_DATA == 1)
      case eMPEIFEC:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.MpeIFec.Pid = var;

         DtaGetParam(&var, "Nb rows", batch_mode, fb);
         FilDesc.MpeIFec.NbRows = (enum DibFecNbrows) var;

         DtaGetParam(&var, "Encoding Parallelization (B)", batch_mode, fb);
         FilDesc.MpeIFec.EncodParal = var;

         DtaGetParam(&var, "Spreading Factor (S)", batch_mode, fb);
         FilDesc.MpeIFec.SpreadingFactor = var;

         DtaGetParam(&var, "Datagram Burst Sending Delay (D)", batch_mode, fb);
         FilDesc.MpeIFec.SendingDelay = var;

         DtaGetParam(&var, "Adst Columns (C)", batch_mode, fb);
         FilDesc.MpeIFec.NbAdstColumns = var;

         DtaGetParam(&var, "Max IFEC Sections (R)", batch_mode, fb);
         FilDesc.MpeIFec.MaxIFecSect = var;

         DtaGetParam(&var, "max burst duration", batch_mode, fb);
         FilDesc.MpeIFec.MaxBurstDuration = var;

         if(FilDesc.MpeIFec.MaxBurstDuration == 0)
            FilDesc.MpeIFec.MaxBurstDuration = DIB_MAX_BURST_TIME_UNKNOWN;

         FilDesc.MpeIFec.Prefetch = eACTIVE;
         break;
#endif

#if (DIB_TDMB_DATA == 1)
      case eTDMB:
         DtaGetParam(&var, "subchannel", batch_mode, fb);
         FilDesc.Tdmb.SubCh = var;
         break;
#endif

#if (DIB_EDAB_DATA == 1)
      case eEDAB:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.eDab.Pid = var;

         DtaGetParam(&var, "subchannel", batch_mode, fb);
         FilDesc.eDab.SubCh = var;
         break;
#endif

#if (DIB_DAB_DATA == 1)
      case eDAB:
         DtaGetParam(&var, "Subchannel", batch_mode, fb);
         FilDesc.Dab.SubCh = var;
         DtaGetParam(&var, "Type", batch_mode, fb);
         FilDesc.Dab.Type = var;
         FilDesc.Dab.UseFec = 0;
         if(var == 1)
         {
            DtaGetParam(&var, "Fec", batch_mode, fb);
            FilDesc.Dab.UseFec = var;
         }
         break;
#endif

#if (DIB_DABPACKET_DATA == 1)
      case eDABPACKET:
         DtaGetParam(&var, "Subchannel", batch_mode, fb);
         FilDesc.DabPacket.SubCh = var;
         DtaGetParam(&var, "Address", batch_mode, fb);
         FilDesc.DabPacket.Address = var;
         DtaGetParam(&var, "Fec", batch_mode, fb);
         FilDesc.DabPacket.UseFec = var;
         DtaGetParam(&var, "DG", batch_mode, fb);
         FilDesc.DabPacket.UseDataGroup = var;
         break;
#endif

#if (DIB_FIG_DATA == 1)
      case eFIG:
         break;
#endif

#if (DIB_CMMB_DATA == 1)
      case eCMMBSVC:
         DtaGetParam(&var, "Mode to identify service (0:MF_ID, 1:SVC_ID)", batch_mode, fb);
         FilDesc.CmmbSvc.Mode = var;
         if(FilDesc.CmmbSvc.Mode == eMF_ID)
         {
            DtaGetParam(&var, "Multiplex Frame Identifier (6 bit)", batch_mode, fb);
            FilDesc.CmmbSvc.MfId = var;
         }
         else
         {
            DtaGetParam(&var, "Service Identifier (16 bit)", batch_mode, fb);
            FilDesc.CmmbSvc.SvcId = var;
         }
         break;
#endif /* DIB_CMMB_DATA */

#if (DIB_ATSCMH_DATA == 1)
      case eATSCMHIP:
      case eATSCMHFIC:
          DtaGetParam(&var, "Parade Ensemble Mode (FindEnsemble=0,FindServGuide=1,FindService=2)", batch_mode, fb);
          FilDesc.AtscmhEns.Mode = var;
          DtaGetParam(&var, "Parade Ensemble Id", batch_mode, fb);
          FilDesc.AtscmhEns.Id = var;
          break;
 #endif

      case eFM:
      case eANALOG:
		default:
         break;
      }
      DtaGetParam(&ItemId, "Item Identifier", batch_mode, fb);

      DtaAddToFilter(ItemId, FilterId, &FilDesc, FilterType);
   }
   else if(strcmp(cmd, "dtf") == 0)
   {
      uint32_t idx = 0;

      DtaGetParam(&idx, "Item Identifier", batch_mode, fb);
      DtaRemoveFromFilter(idx);
   }
   else if(strcmp(cmd, "event") == 0)
   {
      uint32_t type = 0;
      uint32_t set = 0;

      DtaGetParam(&type, "Which ?", batch_mode, fb);
      DtaGetParam(&set, "Add ?", batch_mode, fb);

      DtaSetEventCallback((enum DibEvent) type, set);
   }
   else if(strcmp(cmd, "casstart") == 0)
   {
#ifdef USE_NAGRA
     DIBSTATUS cbStatus = DIBSTATUS_SUCCESS;
     cbStatus = DibRegisterMessageCallback(CONTEXT, MSG_API_TYPE_NAGRA, MsgBuf, sizeof(MsgBuf), DibNagraDisplay, NULL);
     if(cbStatus == DIBSTATUS_SUCCESS)
       printf("Nagra interface registered\n");
     else
       printf("Failed to register Nagra interface %d\n", cbStatus);
#endif /* USE_NAGRA */
#ifdef USE_BCAS
     DIBSTATUS cbStatus = DIBSTATUS_SUCCESS;
     cbStatus = DibRegisterMessageCallback(CONTEXT, MSG_API_TYPE_BCAS, MsgBuf, sizeof(MsgBuf), DibBCasDisplay, NULL);
     if(cbStatus == DIBSTATUS_SUCCESS)
        printf("BCAS interface registered\n");
     else
        printf("Failed to register BCAS interface %d\n", cbStatus);
#endif /* USE_BCAS */

      union DibParamConfig ParamConfig;
      ParamConfig.CasEnable.Enable = eDIB_TRUE;
      DibSetConfig(CONTEXT, eCAS_ENABLE, &ParamConfig);
   }
   else if(strcmp(cmd, "casstop") == 0)
   {
      union DibParamConfig ParamConfig;
      ParamConfig.CasEnable.Enable = eDIB_FALSE;
      DibSetConfig(CONTEXT, eCAS_ENABLE, &ParamConfig);
   }

/* In a test environment, Nagra messages enable
 * console I/O to the testsuite library
 * (first stage of integration)
 * Needs make USE_NAGRA=1
 */
/* #define USE_NAGRA_TEST */

#ifdef USE_NAGRA_TEST
   else if(strcmp(cmd, "cassend") == 0)
   {
      char chbuf[2];
      int inchar;
      uint8_t retbuf[32];
      uint32_t retlen = 32;
      DIBSTATUS stat;
      chbuf[1] = 0;
      inchar = getchar(); /* eat cr */
      printf("send to CAS (end with ^D):"); /* DibGetString does not handle whitespace */
      while ((inchar = getchar()) != EOF) {
        chbuf[0] = (char) inchar;
/*         DtaGetString(chbuf, batch_mode, fb); */
        stat = DibSendMessage(CONTEXT, MSG_API_TYPE_NAGRA, NAGRA_CHAR_INPUT, (uint8_t *)chbuf, strlen(chbuf), retbuf, &retlen);
        if (stat != DIBSTATUS_SUCCESS)
          printf("Error sending message (%d)", stat);
      }
   }
#endif /* USE_NAGRA_TEST */
#if (DEBUG_LIB == 1)
   else if(strcmp(cmd, "debug") == 0)
   {
      DtaTestDfly();
   }
#endif
#if (DIB_CMMB_STD == 1) && (DIB_CMMB_DATA == 1) && (DIB_INTERNAL_DEBUG == 1)
   else if(strcmp(cmd, "postart") == 0)
   {
      printf(CRB "------------------------" CRA);
      printf(CRB "Start Playout Thread" CRA);
      printf(CRB "------------------------" CRA);
      DtaPlayoutStart();
   }
   else if(strcmp(cmd, "postop") == 0)
   {
      printf(CRB "------------------------" CRA);
      printf(CRB "Stop Playout Thread" CRA);
      printf(CRB "------------------------" CRA);
      DtaPlayoutStop();
   }
   else if(strcmp(cmd, "poopen") == 0)
   {
      char fpayld[80];
      char fdescr[80];
      uint32_t NbPhyFrames = 0;
      memset(fpayld, 0, 80);
      memset(fdescr, 0, 80);
      DtaGetFileName(fpayld, "Playout file", batch_mode, fb);
      DtaGetFileName(fdescr, "Description file", batch_mode, fb);
      DtaGetParam(&NbPhyFrames, "Number of physical frames", batch_mode, fb);
      DtaPlayoutOpen(fpayld, fdescr, (uint8_t)NbPhyFrames);
   }
   else if(strcmp(cmd, "poclose") == 0)
   {
      DtaPlayoutClose();
   }
   else if(strcmp(cmd, "postep") == 0)
   {
      DtaPlayoutStep();
   }
   else if(strcmp(cmd, "postepm") == 0)
   {
      uint32_t m = 0;
      DtaGetParam(&m, "loops", batch_mode, fb);
      while(m>0)
      {
         DtaPlayoutStep();
         m--;
      }
   }
   else if(strcmp(cmd, "pocorrupt") == 0)
   {
      uint32_t Corruption = 0;
      DtaGetParam(&Corruption, "Corruption number", batch_mode, fb);
      DtaPlayoutCorrupt((uint8_t)Corruption);
   }
   else if(strcmp(cmd, "porun") == 0)
   {
      DtaPlayoutRun();
   }
   else if(strcmp(cmd, "popause") == 0)
   {
      DtaPlayoutPause();
   }
   else if(strcmp(cmd, "porewind") == 0)
   {
      DtaPlayoutRewind();
   }
   else if(strcmp(cmd, "porecord") == 0)
   {
      uint8_t Descriptor[64] = { 0 }; /* Header(size): 1 byte, Payload: <= 63 bytes. */
      uint32_t NbBytes = 0;
      DtaGetParam(&NbBytes, "Record descriptor size (header included)", batch_mode, fb);
      if(NbBytes > 64)
         NbBytes = 64;
      Descriptor[0] = (uint8_t)NbBytes;
      uint8_t i;
      for(i = 1; i < NbBytes; i++)
      {
         DtaGetParam((uint32_t*)(&Descriptor[i]), "Description byte", batch_mode, fb);
      }
      DtaPlayoutRecord(Descriptor);
   }
   else if(strcmp(cmd, "poerase") == 0)
   {
      uint32_t Handle;
      DtaGetParam(&Handle, "Handle", batch_mode, fb);
      DtaPlayoutErase((uint8_t)Handle);
   }
#endif

   else if (strcmp(cmd, "info") == 0)
   {
      DtaDisplayExInfo();
   }
   else if (strcmp(cmd, "gpiotest") == 0)
   {
      DtaDisplayGpioTest();
   }
   else if (strcmp(cmd, "toggle_mpeg") == 0)
   {
      int32_t           rc;
      struct DibInfoSet InfoSet;
      uint8_t           Status;

      InfoSet.ChipId = 0;
      InfoSet.Type = eINFO_TOGGLE_MPEG;
      rc = DibSetInfo(CONTEXT, &InfoSet, &Status);

      if (Status != DIBSTATUS_SUCCESS || rc != DIBSTATUS_SUCCESS)
      {
         printf(CRB "Error : Can not toggle mpeg" CRA);
      }
      else
      {
         printf(CRB "Toggle mpeg sucess !" CRA);
      }
   }
   else if (strcmp(cmd, "toggle_mpeg_status") == 0)
   {
      int32_t               rc;
      struct DibInfoRequest InfoRequest;
      struct DibInfoData    InfoData;

      InfoRequest.ChipId = 0;
      InfoRequest.Type = DIB_INFO_MASK_TOGGLE_MPEG;
      rc = DibGetInfo(CONTEXT, &InfoRequest, &InfoData);

      if (InfoData.ToggleMpeg.Status != DIBSTATUS_SUCCESS || rc != DIBSTATUS_SUCCESS)
      {
         printf(CRB "Error : Can not get toggle mpeg status" CRA);
      }
      else
      {
         printf(CRB "Toggle mpeg sucess %s" CRA, InfoData.ToggleMpeg.IsToggled ? "ON" : "OFF");
      }
   }
   else if (strcmp(cmd, "set_per_period") == 0)
   {
      int32_t           rc;
      struct DibInfoSet InfoSet;
      uint8_t           Status;
      uint32_t          period;

      InfoSet.ChipId = 0;
      InfoSet.Type = eINFO_PER_PERIOD;

      DtaGetParam(&period, "Period (0 = 1s - 1 = 500ms - 2 = 250ms)", batch_mode, fb);

      InfoSet.Param.PerPeriod.Period = (enum DibPerPeriod) period;

      rc = DibSetInfo(CONTEXT, &InfoSet, &Status);

      if (Status != DIBSTATUS_SUCCESS || rc != DIBSTATUS_SUCCESS)
      {
         if (rc == DIBSTATUS_INVALID_PARAMETER)
            printf(CRB "Error : Per Period set [Invalid parameters]" CRA);
         else
            printf(CRB "Error : Per Period set" CRA);
      }
      else
      {
         printf(CRB "Per Period set successfully !" CRA);
      }
   }
   else if (strcmp(cmd, "get_per_period") == 0)
   {
      int32_t               rc;
      struct DibInfoRequest InfoRequest;
      struct DibInfoData    InfoData;

      InfoRequest.ChipId = 0;
      InfoRequest.Type = DIB_INFO_MASK_PER_PERIOD;
      rc = DibGetInfo(CONTEXT, &InfoRequest, &InfoData);

      if (InfoData.PerPeriod.Status != DIBSTATUS_SUCCESS || rc != DIBSTATUS_SUCCESS)
      {
         printf(CRB "Error : Can not get per period" CRA);
      }
      else
      {
         char period[8];

         switch (InfoData.PerPeriod.Period)
         {
         case ePER_PERIOD_1000_MS:
            strcpy(period, "1s");
            break;
         case ePER_PERIOD_500_MS:
            strcpy(period, "500ms");
            break;
         case ePER_PERIOD_250_MS:
            strcpy(period, "250ms");
            break;
         default:
            strcpy(period, "unknow");
            break;
         }

         printf(CRB "Per Period = %s" CRA, period);
      }
   }
#if DIB_INTERNAL_MONITORING == 1

   else if (strcmp(cmd, "agc_freeze") == 0)
   {
      uint32_t ChipId = 0;
      uint32_t Enable = 0;
      uint32_t AntennaId = 0;

      DtaGetParam(&ChipId, "Chip Id", batch_mode, fb);
      if (ChipId >= globalInfo.NumberOfChips)
      {
         printf("Error : Unavailable chip id (%u) - max = %u\\n", ChipId, globalInfo.NumberOfChips - 1);
         return;
      }

      DtaGetParam(&Enable, "Enable (0:1)", batch_mode, fb);
      DtaGetParam(&AntennaId, "AntennaId", batch_mode, fb);

      dbg_acg_freeze(ChipId, Enable, AntennaId);
   }
   else if (strcmp(cmd, "acqui_control") == 0)
   {
      uint32_t ChipId = 0;
      uint32_t Enable = 0;
      uint32_t Type = 0;

      DtaGetParam(&ChipId, "Chip Id", batch_mode, fb);
      if (ChipId >= globalInfo.NumberOfChips)
      {
         printf("Error : Unavailable chip id (%u) - max = %u\n", ChipId, globalInfo.NumberOfChips - 1);
         return;
      }

      DtaGetParam(&Enable, "Enable (0:1)", batch_mode, fb);
      DtaGetParam(&Type, "Type (0:ADC 1:ANALOG_FE 2:OCTOPUS 3:REFLEX)", batch_mode, fb);

      dbg_acqui(ChipId, Enable, Type);
   }
   else if (strcmp(cmd, "set_clock") == 0)
   {
      uint32_t ChipId = 0;
      uint32_t Type = 0;
      uint32_t parameter;
      union ClockFrequency Param;

      DtaGetParam(&ChipId, "Chip Id", batch_mode, fb);
      if (ChipId >= globalInfo.NumberOfChips)
      {
         printf("Error : Unavailable chip id (%u) - max = %u\\n", ChipId, globalInfo.NumberOfChips - 1);
         return;
      }

      DtaGetParam(&Type, "Type (0:Octopus 1:Reflex)", batch_mode, fb);

      if (Type > 1)
      {
         printf("Error : Unknown type (%u)\n", Type);
         return;
      }

      switch(Type)
      {
      case 0:
         DtaGetParam(&parameter, "Octo Clock", batch_mode, fb);
         Param.Octopus.OctoClk = parameter;
         break;

      case 1:
         DtaGetParam(&parameter, "Femto Clock", batch_mode, fb);
         Param.Reflex.FemtoClk = parameter;

         DtaGetParam(&parameter, "Cce Clock", batch_mode, fb);
         Param.Reflex.CceClk = parameter;
         break;
      }

      dbg_set_clock(ChipId, Type, &Param);
   }

#endif

   /* If command not found then assume it is the name of a batch. */
   else
   {
      char fname[20];             /* File name. */
      char bcmd[100];             /* Command name */

      /*strcpy(fname, "./.batch/");*/
      /*strcat(fname, cmd);*/
      strcpy(fname, cmd);

      fb = fopen(fname, "r");
      if(!fb)
      {
         printf(CRB "%s: *** Error: Not a command nor a batch file (%s)" CRA, __FUNCTION__, fname);
         return;
      }

      while(fscanf(fb, "%s", bcmd) != EOF)
      {
         DtaProcessCommand(bcmd, 1, fb);
      }
      fclose(fb);
   }
}

static void DtaGetCommand(char *cmd)
{
   scanf("%s", cmd);
}

static void DtaGetParam(uint32_t *Val, const char *string, uint8_t batch_mode, FILE * fb)
{
   int32_t rc;

   if(batch_mode == 0)
   {
      /* Prompt Mode. */
      printf("%s: ", string);
      rc = scanf("%d", Val);
   }
   else
   {
      /* Batch Mode. */
      fscanf(fb, "%d", Val);
   }

   LOG("%d ", *Val);
}

static void DtaGetParamHex(uint32_t *Val, const char *string, uint8_t batch_mode, FILE * fb)
{
   int32_t rc;

   if(batch_mode == 0)
   {
      /* Prompt Mode. */
      printf("%s: ", string);
      rc = scanf("%x", Val);
   }
   else
   {
      /* Batch Mode. */
      fscanf(fb, "%x", Val);
   }

   LOG("%x ", *Val);
}

static void DtaGetFileName(char *name, const char *string, uint8_t batch_mode, FILE * fb)
{
   int32_t rc;

   if(batch_mode == 0)
   {
      /* Prompt Mode. */
      printf("%s: ", string);
      rc = scanf("%s", name);
   }
   else
   {
      /* Batch Mode. */
      fscanf(fb, "%s", name);
   }

   LOG("%s ", name);
}

static void DtaGetString(char *str, uint8_t batch_mode, FILE * fb)
{
   int32_t rc;

   if(batch_mode == 0)
   {
      /* Prompt Mode. */
      rc = scanf("%s", str);
   }
   else
   {
      /* Batch Mode. */
      fscanf(fb, "%s", str);
   }

   LOG("%s ", str);
}

static void DtaForgetLine(uint8_t batch_mode, FILE * fb)
{
   char str[255];

   if(batch_mode == 0)
   {
      /* Prompt Mode. */
      scanf("%[^\n]", str);
   }
   else
   {
      /* Batch Mode. */
      fscanf(fb, "%[^\n]", str);
   }
}
