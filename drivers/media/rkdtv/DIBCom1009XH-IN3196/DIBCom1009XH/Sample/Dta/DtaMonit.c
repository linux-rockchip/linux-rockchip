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
 * @file "DtaMonit.c"
 * @brief DiBcom Test Application - Monitoring Functions.
 *
 ***************************************************************************************************/
#define _XOPEN_SOURCE 500

/* #define KB_GETC */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifndef WINCE
#include <signal.h>
#endif
#include <string.h>

#include "DibConfig.h"
#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaCmds.h"
#include "DtaMonit.h"
#include "DtaScan.h"
#include "DtaTime.h"

extern struct DibDriverContext * gUserContext;

#define CONTEXT (gUserContext)

void ComputeEsr(unsigned int perr);
unsigned int gEsrLastSec;
unsigned int gEsrTotalSec;
unsigned int gEsrErrSec;
uint8_t LogFileInitDone = 0;
void SigQuitDta(int signal_id);
static double ComputeiFERrate(uint32_t total, uint32_t good, uint8_t sample);
static double ComputeiMFERrate(uint32_t total, uint32_t good, uint32_t corrected, uint8_t sample);
static double ComputeFER_MFERrate(uint32_t total, uint32_t error, uint8_t sample);
/*******************************************************************************/
/*** Tuner Selection for RF Power estimation                                  **/
/*******************************************************************************/

/*
 * Local functions and structures
 */
#if ((DIB_DVB_STD == 1) || (DIB_ISDBT_STD == 1) || (DIB_CTTB_STD == 1))
static int32_t ComputePoints(int32_t rf_gain, int32_t qual);
static double  ComputeRfPower(uint16_t agc_global, uint8_t LnaStatus, uint8_t Lock);
#endif
#if (DIB_DVB_STD == 1) || (DIB_DVBSH_STD == 1) || (DIB_ISDBT_STD == 1)
static double ComputeRfPowerDVBSH(uint16_t agc_rf, uint16_t agc_bb, uint16_t ad_power, uint8_t Lock, uint8_t lna);
#endif

static THREAD_HANDLE	MonitoringThreadHandle;
volatile uint8_t		MonitThreadRunning = 0;
static void				*MonitoringThread(void *param);

/* To keep track of TS channels that are set or unset. */
#define RF_KHZ_UNSET                (-1)

#define POWER_UP_TIME_UNKNOWN       (-1)

#define CLEAR_MONIT                 eDIB_FALSE

#ifdef KB_GETC
/*--------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------*/
#include <termios.h>

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

static struct termios TaSet, TaStore;
static uint32_t       TtyIsRaw;

static int32_t SetTtyRaw(void)
{
    int32_t i;

    i = tcgetattr(STDIN_FILENO, &TaSet);

    if(i < 0)
    {
        printf(CRB "tcgetattr() returned %d!" CRA, i);
        return -1;
    }
    TaStore = TaSet;

    TaSet.c_cflag &= ~(CSIZE | PARENB);
    TaSet.c_cflag |= CS8;
    TaSet.c_oflag &= ~OPOST;
    TaSet.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    TaSet.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    TaSet.c_cc[VMIN]  = 1;
    TaSet.c_cc[VTIME] = 0;

    i = tcsetattr(STDIN_FILENO, TCSANOW, &TaSet);
    if(i < 0)
    {
        printf(CRB "tcsetattr() returned %d!" CRA, i);
        return -1;
    }

    TtyIsRaw = 1;

    return 0;
}

static int32_t SetTtyCooked(void)
{
    if(TtyIsRaw == 0)
        return 0;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &TaStore) < 0)
        return -1;

    TtyIsRaw = 0;
    return 0;
}

uint8_t kb_getc(void)
{
    int32_t i;
    uint8_t ch;
    ssize_t Size;

    SetTtyRaw();

    TaSet.c_cc[VMIN] = 0;
    i = tcsetattr(STDIN_FILENO, TCSANOW, &TaSet);

    Size = read(STDIN_FILENO, &ch, 1);
    TaSet.c_cc[VMIN] = 1;
    i = tcsetattr(STDIN_FILENO, TCSANOW, &TaSet);

    SetTtyCooked();

    if(Size == 0)
        return 0;
    else
        return ch;
}

/*--------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------*/
#endif

/**
 * DtaStartMonit()
 */
int32_t DtaStartMonit(void)
{
    int32_t status = 0;

    if(MonitThreadRunning)
    {
        return -1;
    }
#ifdef SAME_THREAD

    MonitoringThread(0);

#else

    MonitThreadRunning = 1;
    status = OsCreateThread(&MonitoringThreadHandle, (PTHREAD_FN)MonitoringThread, NULL);

    if(status == 0)
    {
        /** Wait that thread is really started */
        while(MonitThreadRunning == 0)
        {
            OsMSleep(1);
        }
        printf(CRB "MONIT Thread created, status=%d" CRA, status);
    }
    else
    {
        printf(CRB "MONIT Thread create FAILED!!!!, status=%d" CRA, status);
    }
#endif
    return (status);
}

/**
 * DtaStopMonit()
 */
int32_t DtaStopMonit(void)
{
    int32_t status;

    if(!MonitThreadRunning)
    {
        return -1;
    }

    MonitThreadRunning = 0;

    status = OsJoinThread(MonitoringThreadHandle);

    return (status);
}

/**
 * MonitoringThread()
 */
static void *MonitoringThread(void * param)
{
    uint8_t pid = 0;
    printf(CRB "Monitoring Thread Started" CRA);

    MonitThreadRunning = 1;

    while(MonitThreadRunning)
    {
        /* display shortened signal monitoring information for each registered channel */
        DtaRefreshMonitArray();

#if (DIB_RAWTS_DATA == 1) && ((DIB_DVB_STD == 1) || (DIB_ISDBT_STD == 1) || (DIB_CTTB_STD == 1))
        /* display RAWTS monitoring information */
        pid = DtaGetDvbTMonitInfo(stdout, DTA_FULL_DISPLAY);
#endif

        /* display MPE-FEC monitoring information */
        pid += DtaGetDvbHMonitInfo(stdout, DTA_FULL_DISPLAY, 1);

        /* display MPE-IFEC monitoring information */
        pid += DtaGetDvbSHMonitInfo(stdout, DTA_FULL_DISPLAY, 1);

        /* display CMMB monitoring information */
        pid += DtaGetCmmbMonitInfo(stdout, DTA_FULL_DISPLAY, 1);

        /* display DAB monitoring information */
        pid += DtaGetDabMonitInfo(stdout, 0xFF, DTA_FULL_DISPLAY);

        if(!pid)
            printf(CRB "   No active filter" CRA);
        else
            pid = 0;

        OsMSleep(1000);
    }

    printf(CRB "Monitoring Thread Stopped" CRA);

    MonitThreadRunning = 0;

    return (0);
}

/**
 * Retreive information for every MPE service of the system, and display them.
 */
int32_t DtaGetDvbHMonitInfo(FILE * f, uint8_t display, uint32_t sample)
{
    int32_t ItemId, FilterId;
    uint8_t pid = 0;
    struct DibDvbHMonitoring *pDVBHMonit;
    double FerRatio, MFerRatio;

    pDVBHMonit = (struct DibDvbHMonitoring *)malloc(sizeof(struct DibDvbHMonitoring));

    if(!pDVBHMonit)
    {
        printf(CRB "%s: *** Error: Memory allocation failure" CRA, __FUNCTION__);
        return 0;
    }

    if(!f)
    {
        f = stdout;
    }

    /* for each MPEFEC item, read Data monitoring */
    for(ItemId = 0; ItemId < DIB_MAX_NB_ITEMS; ItemId++)
    {
        FilterId = ItemArray[ItemId].ParentFilter;

        if(ItemArray[ItemId].Used && (FilterArray[FilterId].DataType == eMPEFEC))
        {
            if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pDVBHMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
            {
                pid = 1;

                /* Actual Power-Up Time. */
                ChannelArray[FilterArray[FilterId].ParentChannel].ActualPowerUpTimeMs = pDVBHMonit->PowerUpTime;

                switch (display)
                {
                /* Full display to standard output. */
                case DTA_SHORT_DISPLAY:

                    /* MPE frames statistics. */
                    fprintf(f, CRB " ChIdx(%d): Table(%4d) Pid(%4d): T/C/F/C/I: %d / %d / %d / %d / %d  Mfer(%0.1f%%)",
                            pDVBHMonit->ChannelIndex, pDVBHMonit->TableSize, pDVBHMonit->Pid,
                            pDVBHMonit->TotalTables, pDVBHMonit->ErrorTablesBeforeFec,
                            pDVBHMonit->ErrorTablesAfterFec, pDVBHMonit->CountinuityError,
                            pDVBHMonit->Error,
                            ((double)pDVBHMonit->Mfer/(double)10.0));

                    fprintf(f, CRB "" CRA);
                    break;

                case DTA_FULL_DISPLAY:
                default:
                    /* MPE frames statistics. */
                    fprintf(f, CRB " ChIdx(%d): Table(%4d) Pid(%4d): T/C/F/C/I: %d / %d / %d / %d / %d  Mfer(%0.1f%%)",
                            pDVBHMonit->ChannelIndex, pDVBHMonit->TableSize, pDVBHMonit->Pid,
                            pDVBHMonit->TotalTables, pDVBHMonit->ErrorTablesBeforeFec,
                            pDVBHMonit->ErrorTablesAfterFec, pDVBHMonit->CountinuityError,
                            pDVBHMonit->Error,
                            ((double)pDVBHMonit->Mfer/(double)10.0));

                    /* MPE frames real-time information. */
                    fprintf(f, CRB " - Delta: %d ms (%d ms) Burst: %d ms (%d ms) PowUp: %d ms" CRA,
                            pDVBHMonit->DeltaTStream, pDVBHMonit->DeltaTDetected,
                            pDVBHMonit->BurstStream, pDVBHMonit->BurstDetected,
                            pDVBHMonit->PowerUpTime);
                    break;

                case DTA_CSV_DISPLAY:
                    {
                        static int32_t start                     = -1;
                        static int32_t StartTotalTables          = -1;
                        static int32_t StartTablesErrorBeforeFec = -1;
                        static int32_t StartTablesErrorAfterFec  = -1;
                        int32_t LocalTotalTables          = 0;
                        int32_t LocalTablesErrorBeforeFec = 0;
                        int32_t LocalTablesErrorAfterFec  = 0;

                        if(start == -1)
                        {
                            StartTotalTables          = pDVBHMonit->TotalTables          ;
                            StartTablesErrorBeforeFec = pDVBHMonit->ErrorTablesBeforeFec ;
                            StartTablesErrorAfterFec  = pDVBHMonit->ErrorTablesAfterFec  ;
                            start = 0;
                        }

                        if(start == 0)
                        {
                            LocalTotalTables          = pDVBHMonit->TotalTables          - StartTotalTables          ;
                            LocalTablesErrorBeforeFec = pDVBHMonit->ErrorTablesBeforeFec - StartTablesErrorBeforeFec ;
                            LocalTablesErrorAfterFec  = pDVBHMonit->ErrorTablesAfterFec  - StartTablesErrorAfterFec  ;
                        }

                        /* "MPEFEC Total;MPEFEC Error Before Fec;MPEFEC Error After Fec;FER ratio;MFER ratio;pid" */
                        fprintf(f, "%d;%d;%d;", LocalTotalTables, LocalTablesErrorBeforeFec, LocalTablesErrorAfterFec);

                        FerRatio  = ComputeFER_MFERrate(LocalTotalTables, LocalTablesErrorBeforeFec, sample);
                        MFerRatio = ComputeFER_MFERrate(LocalTotalTables, LocalTablesErrorAfterFec,  sample);

                        fprintf(f, "%.2f;%.2f;", FerRatio, MFerRatio);
                        fprintf(f, "%d", pDVBHMonit->Pid);
                    }
                    break;
                }
            }
            else
            {
                fprintf(f, CRB "Data monit error with ItemId=%d" CRA,ItemId);
            }
        }
    }

    free(pDVBHMonit);
    fflush(f);
    return pid;
}

/**
 * Retreive information for every MPE-IFEC service of the system, and display them.
 */
int32_t DtaGetDvbSHMonitInfo(FILE *f, uint8_t display, uint32_t sample)
{
    int32_t ItemId, FilterId;
    uint8_t pid = 0;
    double iFerRatio, iMFerRatio;
    struct DibDvbSHMonitoring *pDVBSHMonit;

    pDVBSHMonit = (struct DibDvbSHMonitoring *)malloc(sizeof(struct DibDvbSHMonitoring));

    if(!pDVBSHMonit)
    {
        printf(CRB "%s: *** Error: Memory allocation failure" CRA, __FUNCTION__);
        return 0;
    }

    if(!f)
    {
        f = stdout;
    }

    /* for each MPEFEC item, read Data monitoring */
    for(ItemId = 0; ItemId < DIB_MAX_NB_ITEMS; ItemId++)
    {
        FilterId = ItemArray[ItemId].ParentFilter;

        if(ItemArray[ItemId].Used && (FilterArray[FilterId].DataType == eMPEIFEC))
        {
            if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pDVBSHMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
            {
                pid = 1;

                /* Actual Power-Up Time. */
                ChannelArray[FilterArray[FilterId].ParentChannel].ActualPowerUpTimeMs = pDVBSHMonit->PowerUpTime;

                switch (display)
                {
                case DTA_SHORT_DISPLAY:

                    fprintf(f, CRB "==> ChIdx(%d): Rows(%4d) Pid(%4d) B(%3d) S(%3d) D(%3d) C(%3d) R(%3d) : " CRA,
                            pDVBSHMonit->ChannelIndex, pDVBSHMonit->NbRows, pDVBSHMonit->Pid,
                            pDVBSHMonit->EncodParal, pDVBSHMonit->SpreadingFactor, pDVBSHMonit->SendingDelay,
                            pDVBSHMonit->NbAdstColumns, pDVBSHMonit->MaxIFecSect);

                    fprintf(f, CRB "        - (ADST) : T/G/C/F/L   : %d / %d / %d / %d / %d " CRA,
                            pDVBSHMonit->AdstTotalTables, pDVBSHMonit->AdstTablesGood, pDVBSHMonit->AdstTablesCorrected, pDVBSHMonit->AdstTablesFailed, pDVBSHMonit->AdstTablesLost);

                    fprintf(f, CRB "        - (ADT)  : T/G/C/F/CC/I: %d / %d / %d / %d / %d / %d " CRA,
                            pDVBSHMonit->AdtTotalTables, pDVBSHMonit->AdtTablesGood, pDVBSHMonit->AdtTablesCorrected, pDVBSHMonit->AdtTablesFailed, pDVBSHMonit->CountinuityError, pDVBSHMonit->Error);

                    fprintf(f, CRB "" CRA);
                    break;

                case DTA_FULL_DISPLAY:
                default:

                    /* MPE frames statistics. */
                    /*
                       fprintf(f, CRB " ChIdx(%d): Rows(%4d) Pid(%4d) B(%3d) S(%3d) D(%3d) C(%3d) R(%3d): \n(ADST) T/M/C/F/G: %d / %d / %d / %d / %d \n(ADT) T/C/F/G/CC/I: %d / %d / %d / %d / %d/ %d",
                       pDVBSHMonit->ChannelIndex, pDVBSHMonit->NbRows, pDVBSHMonit->Pid,
                       pDVBSHMonit->EncodParal, pDVBSHMonit->SpreadingFactor, pDVBSHMonit->SendingDelay,
                       pDVBSHMonit->NbAdstColumns, pDVBSHMonit->MaxIFecSect,
                       pDVBSHMonit->AdstTotalTables, pDVBSHMonit->AdstTablesLost, pDVBSHMonit->AdstTablesCorrected, pDVBSHMonit->AdstTablesFailed, pDVBSHMonit->AdstTablesGood,
                       pDVBSHMonit->AdtTotalTables, pDVBSHMonit->AdtTablesCorrected, pDVBSHMonit->AdtTablesFailed, pDVBSHMonit->AdtTablesGood, pDVBSHMonit->CountinuityError, pDVBSHMonit->Error);
                     */

                    fprintf(f, CRB "==> ChIdx(%d): Rows(%4d) Pid(%4d) B(%3d) S(%3d) D(%3d) C(%3d) R(%3d) : " CRA,
                            pDVBSHMonit->ChannelIndex, pDVBSHMonit->NbRows, pDVBSHMonit->Pid,
                            pDVBSHMonit->EncodParal, pDVBSHMonit->SpreadingFactor, pDVBSHMonit->SendingDelay,
                            pDVBSHMonit->NbAdstColumns, pDVBSHMonit->MaxIFecSect);

                    fprintf(f, CRB "        - (ADST) : T/G/C/F/L   : %d / %d / %d / %d / %d " CRA,
                            pDVBSHMonit->AdstTotalTables, pDVBSHMonit->AdstTablesGood, pDVBSHMonit->AdstTablesCorrected, pDVBSHMonit->AdstTablesFailed, pDVBSHMonit->AdstTablesLost);

                    fprintf(f, CRB "        - (ADT)  : T/G/C/F/CC/I: %d / %d / %d / %d / %d / %d " CRA,
                            pDVBSHMonit->AdtTotalTables, pDVBSHMonit->AdtTablesGood, pDVBSHMonit->AdtTablesCorrected, pDVBSHMonit->AdtTablesFailed, pDVBSHMonit->CountinuityError, pDVBSHMonit->Error);

                    /* MPE frames real-time information. */
                    fprintf(f, CRB "        - Delta  : %d ms (%d ms) Burst: %d ms (%d ms) PowUp: %d ms" CRA,
                            pDVBSHMonit->DeltaTStream, pDVBSHMonit->DeltaTDetected,
                            pDVBSHMonit->BurstStream, pDVBSHMonit->BurstDetected,
                            pDVBSHMonit->PowerUpTime);
                    break;

                case DTA_CSV_DISPLAY:
                    {
                        static int32_t start                = -1;
                        static int32_t StartTotalTables     = -1;
                        static int32_t StartTablesGood      = -1;
                        static int32_t StartTablesFailed    = -1;
                        static int32_t StartTablesCorrected = -1;
                        static int32_t StartTablesLost      = -1;
                        int32_t LocalTotalTables     = 0;
                        int32_t LocalTablesGood      = 0;
                        int32_t LocalTablesFailed    = 0;
                        int32_t LocalTablesCorrected = 0;
                        int32_t LocalTablesLost      = 0;

                        if((start == -1) && (pDVBSHMonit->AdstTotalTables >= (pDVBSHMonit->EncodParal+pDVBSHMonit->SpreadingFactor)))
                        {
                            StartTotalTables     = pDVBSHMonit->AdstTotalTables     ;
                            StartTablesGood      = pDVBSHMonit->AdstTablesGood      ;
                            StartTablesFailed    = pDVBSHMonit->AdstTablesFailed    ;
                            StartTablesCorrected = pDVBSHMonit->AdstTablesCorrected ;
                            StartTablesLost      = pDVBSHMonit->AdstTablesLost      ;
                            start = 0;
                        }

                        if(start == 0)
                        {
                            LocalTotalTables     = pDVBSHMonit->AdstTotalTables     - StartTotalTables     ;
                            LocalTablesGood      = pDVBSHMonit->AdstTablesGood      - StartTablesGood      ;
                            LocalTablesFailed    = pDVBSHMonit->AdstTablesFailed    - StartTablesFailed    ;
                            LocalTablesCorrected = pDVBSHMonit->AdstTablesCorrected - StartTablesCorrected ;
                            LocalTablesLost      = pDVBSHMonit->AdstTablesLost      - StartTablesLost      ;
                        }

                        /* "ADST Total;ADST Good;ADST Corrected;ADST Failed;ADST Lost;iFER ratio;iMFER ratio;pid;nb rows;B;S;D;C;R" */
                        fprintf(f, "%d;%d;%d;%d;%d;", LocalTotalTables, LocalTablesGood,
                                LocalTablesCorrected, LocalTablesFailed, LocalTablesLost);

                        iFerRatio  = ComputeiFERrate (LocalTotalTables, LocalTablesGood, sample);
                        iMFerRatio = ComputeiMFERrate(LocalTotalTables, LocalTablesGood, LocalTablesCorrected, sample);

                        fprintf(f, "%.2f;%.2f;", iFerRatio, iMFerRatio);

                        fprintf(f, "%d;%d;%d;%d;%d;%d;%d", pDVBSHMonit->Pid, pDVBSHMonit->NbRows,
                                pDVBSHMonit->EncodParal,
                                pDVBSHMonit->SpreadingFactor,
                                pDVBSHMonit->SendingDelay,
                                pDVBSHMonit->NbAdstColumns,
                                pDVBSHMonit->MaxIFecSect);
                    }
                    break;
                }

            }
            else
            {
                fprintf(f, CRB "Data monit error with ItemId=%d" CRA,ItemId);
            }
        }
    }

    free(pDVBSHMonit);
    fflush(f);
    return pid;
}

/**
 * Retrieve information for every CMMB service of the system, and display them.
 */
int32_t DtaGetCmmbMonitInfo(FILE *f, uint8_t display, uint32_t sample)
{
   int32_t ItemId, FilterId;
   uint8_t mfid = 0;
   struct DibCmmbMonitoring *pCMMBMonit;

   pCMMBMonit = (struct DibCmmbMonitoring *)malloc(sizeof(struct DibCmmbMonitoring));

   if(!pCMMBMonit)
   {
      printf(CRB "%s: *** Error: Memory allocation failure" CRA, __FUNCTION__);
      return 0;
   }

   if(!f)
   {
      f = stdout;
   }

   /* for each CMMB item, read Data monitoring */
   for(ItemId = 0; ItemId < DIB_MAX_NB_ITEMS; ItemId++)
   {
      FilterId = ItemArray[ItemId].ParentFilter;

      if(ItemArray[ItemId].Used && (FilterArray[FilterId].DataType == eCMMBSVC))
      {
         if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pCMMBMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
         {
            mfid = 1;

            switch(display)
            {
            case DTA_FULL_DISPLAY:
            case DTA_SHORT_DISPLAY:
            default:
                /* CMMB LDPC blocks and RS matrices statistics. */
                fprintf(f, CRB " ChIdx(%2d): Ldpc T/F: %7d / %5d Bler(%0.1f%%): MF(%2d): T/C/F/CC/I: %4d /%4d /%4d /%4d /%4d,"
                               " RSM: T/C/F: %5d /%5d /%5d, Rsmer(%0.1f%%)" CRA,
                               pCMMBMonit->ChannelIndex,
                               pCMMBMonit->TotalLdpcBlocks, pCMMBMonit->ErrorLdpcBlocks,
                               ((double)pCMMBMonit->Bler/(double)10.0),
                               pCMMBMonit->MfId,
                               pCMMBMonit->TotalMf, pCMMBMonit->ErrorMfBeforeRs, pCMMBMonit->ErrorMfAfterRs,
                               pCMMBMonit->DiscountMf, pCMMBMonit->ErrorMf,
                               pCMMBMonit->TotalRsMatrices, pCMMBMonit->ErrorRsMatricesBeforeRs, pCMMBMonit->ErrorRsMatricesAfterRs,
                               ((double)pCMMBMonit->Rsmer/(double)10.0));
                break;

            case DTA_CSV_DISPLAY:
               break;
            }
         }
         else
         {
            fprintf(f, CRB "Data monit error with ItemId = %d" CRA, ItemId);
         }
      }
   }

   free(pCMMBMonit);
   fflush(f);
   return mfid;
}

#if (DIB_RAWTS_DATA == 1) && ((DIB_DVB_STD == 1) || (DIB_ISDBT_STD == 1))
/**
 * Retreive information for every RAWTS service of the system, and display them.
 */
int32_t DtaGetDvbTMonitInfo(FILE * f, uint8_t display)
{
    int32_t ItemId, FilterId;
    struct DibDvbTMonitoring *pDVBTMonit;
    uint8_t Pid = 0;

    pDVBTMonit = (struct DibDvbTMonitoring *)malloc(sizeof(struct DibDvbTMonitoring));
    if(!pDVBTMonit)
    {
        printf(CRB "%s: *** Error: Memory allocation failure" CRA, __FUNCTION__);
        return 0;
    }
    if(!f)
        f = stdout;

    /* for each RAWTS item, read Data monitoring */
    for(FilterId = 0; FilterId < DIB_MAX_NB_FILTERS ; FilterId++)
    {
        if(FilterArray[FilterId].Used && (FilterArray[FilterId].DataType == eTS))
        {
            /* Browe Items until we find one linked to that filter */
            for(ItemId = 0; ItemId < DIB_MAX_NB_ITEMS ; ItemId++)
            {
               if(ItemArray[ItemId].Used && ItemArray[ItemId].ParentFilter == FilterId)
                  break;
            }
            /* If no connected items, no monit to do */
            if(ItemId == DIB_MAX_NB_ITEMS)
               continue;

            if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pDVBTMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
            {
                Pid++;
                if((ItemArray[ItemId].Config.Ts.Pid & 0x2000) == 0x2000)
                {
                   fprintf(f, CRB " ChIdx(%d) FltIdx(%d) NbOfPid(%d): T/C/E: %d / - / %d TT/CC/EE: %d / %d / %d" CRA,
                         pDVBTMonit->ChannelIndex, pDVBTMonit->FilterIndex, pDVBTMonit->NbPid,
                         pDVBTMonit->TotalPackets, pDVBTMonit->Error,
                         pDVBTMonit->CheckTotalPackets, pDVBTMonit->CheckCountinuityError, pDVBTMonit->CheckError);
                }
                else
                {
                   fprintf(f, CRB " ChIdx(%d) FltIdx(%d) NbOfPid(%d): T/C/E: %d / %d / %d TT/CC/EE: %d / %d / %d" CRA,
                         pDVBTMonit->ChannelIndex, pDVBTMonit->FilterIndex, pDVBTMonit->NbPid,
                         pDVBTMonit->TotalPackets, pDVBTMonit->CountinuityError, pDVBTMonit->Error,
                         pDVBTMonit->CheckTotalPackets, pDVBTMonit->CheckCountinuityError, pDVBTMonit->CheckError);
                }
            }
        }
    }
    free(pDVBTMonit);
    fflush(f);
    return Pid;
}
#endif

/**
 * Retreive information for every DAB service of the system, and display them.
 */
int32_t DtaGetDabMonitInfo(FILE * f, uint32_t type, uint8_t display)
{
    int32_t ItemId, FilterId;
    struct DibDabMonitoring * pMonit;
    uint8_t Pid = 0;

    pMonit = (struct DibDabMonitoring *)malloc(sizeof(struct DibDabMonitoring));
    if(!pMonit)
    {
        printf(CRB "%s: *** Error: Memory allocation failure" CRA, __FUNCTION__);
        return 0;
    }
    if(!f)
        f = stdout;

    for(ItemId = 0; ItemId < DIB_MAX_NB_ITEMS ; ItemId++)
    {
        FilterId = ItemArray[ItemId].ParentFilter;
        if((type == 0xFF) || (type == FilterArray[FilterId].DataType))
        {
           if(ItemArray[ItemId].Used && (FilterArray[FilterId].DataType == eTDMB))
           {
               if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
               {
                   Pid = 1;
                   fprintf(f, CRB " TDMB      ChIdx(%d) FltIdx(%d) : T/C/E: %d / %d / %d" CRA,
                         pMonit->ChannelIndex, pMonit->FilterIndex,
                         pMonit->TotalPackets, pMonit->CountinuityError, pMonit->Error);
               }
           }
           if(ItemArray[ItemId].Used && (FilterArray[FilterId].DataType == eDAB))
           {
               if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
               {
                   Pid = 1;
                   fprintf(f, CRB " DABAudio  ChIdx(%d) FltIdx(%d) : T/C/CC/E: %d / %d / %d / %d" CRA,
                         pMonit->ChannelIndex, pMonit->FilterIndex, pMonit->TotalPackets, pMonit->CorrPackets, pMonit->CountinuityError, pMonit->Error);

               }
           }
           if(ItemArray[ItemId].Used && (FilterArray[FilterId].DataType == eFIG))
           {
              struct DibFicMonitoring * pFicMonit = (struct DibFicMonitoring *)malloc(sizeof(struct DibFicMonitoring));
              if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pFicMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
              {
                  Pid = 1;
                  fprintf(f, CRB " FIC       ChIdx(%d) FltIdx(%d) : OK/KO: %d / %d" CRA,
                        pFicMonit->ChannelIndex, pFicMonit->FilterIndex, pFicMonit->CorrectCnt, pFicMonit->ErrCnt);

              }
              free(pFicMonit);
           }

#if (DIB_DAB_STD == 1)
           if(ItemArray[ItemId].Used && (FilterArray[FilterId].DataType == eDABPACKET))
           {
              struct DibDabPacketMonitoring * pDabPacketMonit = (struct DibDabPacketMonitoring *)malloc(sizeof(struct DibDabPacketMonitoring));
              if(DibGetDataMonitoring(CONTEXT, ItemArray[ItemId].ItemHdl, (union DibDataMonit *)pDabPacketMonit, CLEAR_MONIT) == DIBSTATUS_SUCCESS)
              {
                  Pid = 1;
                  fprintf(f, CRB " DABPacket ChIdx(%d) FltIdx(%d) : PACK : OK/KO: %d / %d",
                        pDabPacketMonit->ChannelIndex, pDabPacketMonit->FilterIndex, pDabPacketMonit->MscPacketGood, pDabPacketMonit->MscPacketsError);
                  if(ItemArray[ItemId].Config.DabPacket.UseDataGroup == 1)
                     fprintf(f, " DG : OK/KO: %d / %d" CRA, pDabPacketMonit->DataGroupGood, pDabPacketMonit->DataGroupError);
                  else
                     fprintf(f, CRA);
              }
              free(pDabPacketMonit);
           }
#endif
        }
    }
    free(pMonit);
    fflush(f);
    return Pid;
}

/**
 * Display total demod monit filled structure
 */
void DtaDisplayDvbSignalMonitInfo(struct DibTotalDemodMonit *pMnt, uint8_t display, FILE* logFile)
{
#if (DIB_DVB_STD == 1)
    char buf[52];
    double tmp_pow[DIB_MAX_NB_DEMODS], ad_pow_dB[DIB_MAX_NB_DEMODS];
    double sig_dB[DIB_MAX_NB_DEMODS], wgn_dB[DIB_MAX_NB_DEMODS];
    double comb_snr[DIB_MAX_NB_DEMODS];
    uint8_t  DemId;

    for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
    {
        sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dvb.SigFp) - 320 * log10(2);
        wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dvb.WgnFp) - 320 * log10(2);

        comb_snr[DemId] = 10 * (log10(pMnt->DemodMonit[DemId].Dvb.CombSigFp) - log10(pMnt->DemodMonit[DemId].Dvb.CombWgnFp));

        tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Dvb.AgcPower);
        tmp_pow[DemId]  /= (double)(1 << 20);
        ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
    }

    switch (display)
    {
        /* Full display to standard output. */
    case DTA_FULL_DISPLAY:
        OsClearScreen();
        printf(CRB "------------------------------------------------------" CRA);
        printf("RF POWER                   \t");

        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            printf("%5.4lf\t\t", ComputeRfPower(pMnt->DemodMonit[DemId].Dvb.AgcGlobal,
                        pMnt->DemodMonit[DemId].Dvb.LnaStatus,
                        DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Dvb.Locks)));
        }
        printf(CRB "" CRA);

        printf("RF TOTAL GAIN DB              \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t", pMnt->DemodMonit[DemId].Dvb.RfTotalGain);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);
        printf("(A/D power)                  \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", ad_pow_dB[DemId]);
        printf(CRB "" CRA);

        printf("Carrier                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId]);
        printf(CRB "" CRA);

        printf("Noise                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Timing Offset                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d ppm\t\t\t", pMnt->DemodMonit[DemId].Dvb.TimingOffset);
        printf(CRB "" CRA);

        printf("Frequency Offset             \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%.3f KHz\t\t", pMnt->DemodMonit[DemId].Dvb.FrequencyOffset/1000.0);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);
        printf("lna status                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Dvb.LnaStatus);
        printf(CRB "" CRA);

        printf("AGC GLOBAL                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4g\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvb.AgcGlobal)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC RF                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvb.VAgcRf & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC BB                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvb.VAgcBB & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("WBD                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvb.AgcWbd & 0xfff)) / ((double)(1 << 12)));
        printf(CRB "" CRA);

        printf("WBD Split                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvb.AgcSplitOffset & 0xff)) / ((double)(1 << 8)));
        printf(CRB "" CRA);

        /* print monnitoring status for all demods */
        printf(CRB "------------------------------------------------------" CRA);
        printf("C/N                          \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId] - wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Combined C/N                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", comb_snr[DemId]);
        printf(CRB "" CRA);

        printf("BER                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4e\t\t", ((double)pMnt->DemodMonit[DemId].Dvb.FecBerRs) / ((double)100000000));
        printf(CRB "" CRA);

        printf("Quality                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Dvb.SignalQuality);
        printf(CRB "" CRA);

        printf("Quality_fast                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Dvb.SignalQualityFast);
        printf(CRB "" CRA);

        printf("Signal_HandOver_Points       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", ComputePoints(pMnt->DemodMonit[DemId].Dvb.RfTotalGain, pMnt->DemodMonit[DemId].Dvb.SignalQuality));
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);

        printf("FecMpfailCnt               \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Dvb.FecMpfailCnt);
        printf(CRB "" CRA);

        printf("CoffLock                    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_COFF_LOCK(pMnt->DemodMonit[DemId].Dvb.Locks));
        printf(CRB "" CRA);

        printf("VitLock                     \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_VIT_LOCK(pMnt->DemodMonit[DemId].Dvb.Locks));
        printf(CRB "" CRA);

        printf("FecLockFrm  (mpeg synchro) \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_FEC_LOCK_FRM(pMnt->DemodMonit[DemId].Dvb.Locks));
        printf(CRB "" CRA);

        printf("FecLockMpeg (mpeg Data)    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Dvb.Locks));
        printf(CRB "" CRA);

        printf("TpsSyncLock                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_TPS_SYNC_LOCK(pMnt->DemodMonit[DemId].Dvb.Locks));
        printf(CRB "" CRA);

        printf("TpsDataLock                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_TPS_DATA_LOCK(pMnt->DemodMonit[DemId].Dvb.Locks));
        printf(CRB "" CRA);
        printf(CRB "" CRA);

        printf("TPS CellID                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            if (pMnt->DemodMonit[DemId].Dvb.TransmitterId == DIB_TRANSMITTER_ID_UNAVAILABLE)
                printf("UNAVAIL\t\t\t");
            else
                printf("%d\t\t\t", pMnt->DemodMonit[DemId].Dvb.TransmitterId);

        printf(CRB "" CRA);
        printf(CRB "" CRA);

        if (DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[0].Dvb.Locks) == 0)
          ComputeEsr(1);
        else
          ComputeEsr(pMnt->DemodMonit[0].Dvb.FecMpfailCnt);
        printf("ESR : %d / %d \n",gEsrErrSec,gEsrTotalSec);

        break;

    case DTA_CSV_DISPLAY:

#if defined(WINCE) || defined(WIN32)
		buf[0] = 0;
#else
       {
          struct OsTimeval_t tv;
          OsGetTime(&tv);
          ctime_r((&tv.tv_sec), buf);
       }
#endif
        buf[strlen(buf)-1] = ' ';

        if (LogFileInitDone == 0)
        {
           LogFileInitDone = 1;
           fprintf(logFile, "date/time,");
           for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           {
              fprintf(logFile, "AGC Global, AGC RF, AGC BB, WBD, WBD Split, C/N, Ber, Packet Error, SQ, SQFast, DataLock, SignalLock,");
           }
        }

        fprintf(logFile, "%s,",buf);

        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
           fprintf(logFile, "%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%2.2f,%0.5f,%d,%d,%d,%d,%d,",
                    ((double)(pMnt->DemodMonit[DemId].Dvb.AgcGlobal)) / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Dvb.VAgcRf))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Dvb.VAgcBB))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Dvb.AgcWbd & 0xfff)) / ((double)(1 << 12)),
                    ((double)(pMnt->DemodMonit[DemId].Dvb.AgcSplitOffset & 0xff)) / ((double)(1 << 8)),
                    sig_dB[DemId]-wgn_dB[DemId],
                    ((double)pMnt->DemodMonit[DemId].Dvb.FecBerRs) / ((double)100000000),

                    DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Dvb.Locks) == 1 ? pMnt->DemodMonit[DemId].Dvb.FecMpfailCnt:10000,
                    pMnt->DemodMonit[DemId].Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Dvb.SignalQualityFast,
                    DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Dvb.Locks),
                    DIB_GET_COFF_LOCK(pMnt->DemodMonit[DemId].Dvb.Locks));

        }
        fprintf(logFile, "\n");
        break;
        /* Short display to standard output */
    case DTA_SHORT_DISPLAY:
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            LOG(CRB "" CRA);
            LOG(CRB "TS %d (demod %d): Lock=%d, C/N=%0.02f dB, SQ=%3d, SQF=%3d, LNA=%d, ADP=%0.02f dB, AGC=%0.02f, BER=%0.03le",
                    pMnt->ChannelIndex, DemId, pMnt->DemodMonit[DemId].Dvb.Locks,
                    sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Dvb.SignalQualityFast,
                    pMnt->DemodMonit[DemId].Dvb.LnaStatus, ad_pow_dB[DemId],
                    ((double)(pMnt->DemodMonit[DemId].Dvb.AgcGlobal)) /
                    ((double)(1 << 16)),
                    ((double)pMnt->DemodMonit[DemId].Dvb.FecBerRs) / ((double)100000000));
            printf(CRB "Signal: Lock: %d\t C/N: %0.02f\t BER: %5.4e\t Perr: %d\t " CRA,
                    DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Dvb.Locks), sig_dB[DemId] - wgn_dB[DemId],
                    ((double)pMnt->DemodMonit[DemId].Dvb.FecBerRs) / ((double)100000000),
                    pMnt->DemodMonit[DemId].Dvb.FecMpfailCnt);
        }
        break;

        /* Tiny display to standard output. */
    case DTA_TINY_DISPLAY:
        printf("-- Lock=%d, C/N=%0.02f dB, SQ=%3d, SQ FAST=%3d",
                DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[0].Dvb.Locks), sig_dB[0] - wgn_dB[0],
                pMnt->DemodMonit[0].Dvb.SignalQuality,
                pMnt->DemodMonit[0].Dvb.SignalQualityFast);
        break;


    default:
        break;
    }
#endif
}
/**
 * Display total demod monit filled structure
 */
void DtaDisplayCttbSignalMonitInfo(struct DibTotalDemodMonit *pMnt, uint8_t display, FILE* logFile)
{
#if (DIB_CTTB_STD == 1)
    double tmp_pow[DIB_MAX_NB_DEMODS], ad_pow_dB[DIB_MAX_NB_DEMODS];
    double sig_dB[DIB_MAX_NB_DEMODS], wgn_dB[DIB_MAX_NB_DEMODS];
    double comb_snr[DIB_MAX_NB_DEMODS];
    uint8_t  DemId;
    uint8_t flags = pMnt->DemodMonit[0].Cttb.LdpcFlags;
    uint32_t per = pMnt->DemodMonit[0].Cttb.Per;
    double Per;
    if((per&0xffff) == 0)
       Per = 0;
    else
       Per = (float)((per&0xffff0000) >> 16)/(float)(per&0xffff);

    for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
    {
        sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Cttb.Dvb.SigFp) - 320 * log10(2);
        wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Cttb.Dvb.WgnFp) - 320 * log10(2);

        comb_snr[DemId] = 10 * (log10(pMnt->DemodMonit[DemId].Cttb.Dvb.CombSigFp) - log10(pMnt->DemodMonit[DemId].Cttb.Dvb.CombWgnFp));

        tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Cttb.Dvb.AgcPower);
        tmp_pow[DemId]  /= (double)(1 << 20);
        ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
    }

    switch (display)
    {
        /* Full display to standard output. */
    case DTA_FULL_DISPLAY:
        OsClearScreen();
        printf("\n-- PER                         %f\n",            Per);
        printf("-- LdpcSyndrome                %d\n",            pMnt->DemodMonit[0].Cttb.LdpcSyndrome);
        printf("-- LdpcSyndromeFirstIter       %d\n",            pMnt->DemodMonit[0].Cttb.LdpcSyndromeFirstIter);
        printf("-- LdpcNbIter                  %d\n",            pMnt->DemodMonit[0].Cttb.LdpcNbIter);
        printf("-- Freeze error                %d\n",            flags&0x1);
        printf("-- Ldpc Overflow               %d\n",            (flags >> 1)&0x1);
        printf("-- CCE output Overflow         %d\n\n",          (flags >> 2)&0x1);
        printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);
        printf("RF POWER                   \t");

        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            printf("%5.4lf\t\t", ComputeRfPower(pMnt->DemodMonit[DemId].Cttb.Dvb.AgcGlobal,
                        pMnt->DemodMonit[DemId].Cttb.Dvb.LnaStatus,
                        DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Cttb.Dvb.Locks)));
        }
        printf(CRB "" CRA);

        printf("RF TOTAL GAIN DB              \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t", pMnt->DemodMonit[DemId].Cttb.Dvb.RfTotalGain);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);
        printf("(A/D power)                  \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", ad_pow_dB[DemId]);
        printf(CRB "" CRA);

        printf("Carrier                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId]);
        printf(CRB "" CRA);

        printf("Noise                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Timing Offset                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%f ppm\t\t\t", (double)pMnt->DemodMonit[DemId].Cttb.Dvb.TimingOffset);
        printf(CRB "" CRA);

        printf("Frequency Offset             \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%.3f KHz\t\t", (double)pMnt->DemodMonit[DemId].Cttb.Dvb.FrequencyOffset*2);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);
        printf("lna status                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Cttb.Dvb.LnaStatus);
        printf(CRB "" CRA);

        printf("AGC GLOBAL                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4g\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cttb.Dvb.AgcGlobal)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC RF                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cttb.Dvb.VAgcRf & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC BB                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cttb.Dvb.VAgcBB & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("WBD                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cttb.Dvb.AgcWbd & 0xfff)) / ((double)(1 << 12)));
        printf(CRB "" CRA);

        printf("WBD Split                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cttb.Dvb.AgcSplitOffset & 0xff)) / ((double)(1 << 8)));
        printf(CRB "" CRA);

        /* print monitoring status for all demods */
        printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);
        printf("C/N                          \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId] - wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Combined C/N                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", comb_snr[DemId]);
        printf(CRB "" CRA);

        printf("BER                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4e\t\t", ((double)pMnt->DemodMonit[DemId].Cttb.Dvb.FecBerRs) / ((double)100000000));
        printf(CRB "" CRA);

        printf("Quality                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Cttb.Dvb.SignalQuality);
        printf(CRB "" CRA);

        printf("Quality_fast                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Cttb.Dvb.SignalQualityFast);
        printf(CRB "" CRA);

        printf("Signal_HandOver_Points       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", ComputePoints(pMnt->DemodMonit[DemId].Cttb.Dvb.RfTotalGain, pMnt->DemodMonit[DemId].Cttb.Dvb.SignalQuality));
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);

        printf("CoffLock                    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_COFF_LOCK(pMnt->DemodMonit[DemId].Cttb.Dvb.Locks));
        printf(CRB "" CRA);
        printf(CRB "" CRA);

        break;

        /* Short display to standard output */
    case DTA_SHORT_DISPLAY:
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            LOG(CRB "" CRA);
            LOG(CRB "TS %d (demod %d): Lock=%d, C/N=%0.02f dB, SQ=%3d, SQF=%3d, LNA=%d, ADP=%0.02f dB, AGC=%0.02f, BER=%0.03le",
                    pMnt->ChannelIndex, DemId, pMnt->DemodMonit[DemId].Cttb.Dvb.Locks,
                    sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Cttb.Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Cttb.Dvb.SignalQualityFast,
                    pMnt->DemodMonit[DemId].Cttb.Dvb.LnaStatus, ad_pow_dB[DemId],
                    ((double)(pMnt->DemodMonit[DemId].Cttb.Dvb.AgcGlobal)) /
                    ((double)(1 << 16)),
                    ((double)pMnt->DemodMonit[DemId].Cttb.Dvb.FecBerRs) / ((double)100000000));
            printf(CRB "Signal: Lock: %d\t C/N: %0.02f\t SQ: %3d\t SQF: %3d\t Powup Time: %d" CRA,
                    DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Cttb.Dvb.Locks), sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Cttb.Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Cttb.Dvb.SignalQualityFast,
                    0/*pMnt->DemodMonit[0].power_up_time_100us / 10, */);
        }
        break;

        /* Tiny display to standard output. */
    case DTA_TINY_DISPLAY:
        printf("-- Lock=%d, C/N=%0.02f dB, SQ=%3d, SQ FAST=%3d",
              DIB_GET_COFF_LOCK(pMnt->DemodMonit[0].Cttb.Dvb.Locks), sig_dB[0] - wgn_dB[0],
                pMnt->DemodMonit[0].Cttb.Dvb.SignalQuality,
                pMnt->DemodMonit[0].Cttb.Dvb.SignalQualityFast);
        break;


    default:
        break;
    }
#endif
}

/**
 * Display total demod monit filled structure
 */
/* demod locks - different represenation of depending on the standard */
  /* ISDB-T
   * [15] AgcLock     | [14] xxx         | [13] CoffLock      | [12] xxx          |
   * [11] vit_lock_0  | [10] vit_lock_1  | [ 9] vit_lock_2    | [ 8] xxx          |
   * [ 7] mpeg_lock_0 | [ 6] mpeg_lock_1 | [ 5] mpeg_lock_2   | [ 4] xxx          |
   * [ 3] xxx         | [ 2] xxx         | [ 1] xxx           | [ 0] DvsyLock     |
   */

void DtaDisplayIsdbtSignalMonitInfo(struct DibTotalDemodMonit *pMnt, uint8_t display, FILE* logFile)
{
#if (DIB_ISDBT_STD == 1)
    char buf[52];
    double tmp_pow[DIB_MAX_NB_DEMODS], ad_pow_dB[DIB_MAX_NB_DEMODS];
    double sig_dB[DIB_MAX_NB_DEMODS], wgn_dB[DIB_MAX_NB_DEMODS];
    double comb_snr[DIB_MAX_NB_DEMODS];
    uint8_t  DemId;

    for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
    {
        sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Isdbt.Dvb.SigFp) - 320 * log10(2);
        wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Isdbt.Dvb.WgnFp) - 320 * log10(2);

        comb_snr[DemId] = 10 * (log10(pMnt->DemodMonit[DemId].Isdbt.Dvb.CombSigFp) - log10(pMnt->DemodMonit[DemId].Isdbt.Dvb.CombWgnFp));

        tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcPower);
        tmp_pow[DemId]  /= (double)(1 << 20);
        ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
    }

    switch (display)
    {
        /* Full display to standard output. */
    case DTA_FULL_DISPLAY:
        OsClearScreen();
        printf(CRB "------------------------------------------------------" CRA);
        printf("RF POWER                   \t");

        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            printf("%5.4lf\t\t", ComputeRfPower(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcGlobal,
                        pMnt->DemodMonit[DemId].Isdbt.Dvb.LnaStatus,
                        DIB_GET_ISDBT_LOCK_MPEG1(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks)));
        }
        printf(CRB "" CRA);

        printf("RF TOTAL GAIN DB              \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t", pMnt->DemodMonit[DemId].Isdbt.Dvb.RfTotalGain);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);
        printf("(A/D power)                  \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", ad_pow_dB[DemId]);
        printf(CRB "" CRA);

        printf("Carrier                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId]);
        printf(CRB "" CRA);

        printf("Noise                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Timing Offset                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d ppm\t\t", pMnt->DemodMonit[DemId].Isdbt.Dvb.TimingOffset);
        printf(CRB "" CRA);

        printf("Frequency Offset             \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%.3f KHz\t\t", pMnt->DemodMonit[DemId].Isdbt.Dvb.FrequencyOffset/1000.0);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);
        printf("lna status                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Dvb.LnaStatus);
        printf(CRB "" CRA);

        printf("AGC GLOBAL                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4g\t\t\t", ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcGlobal)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC RF                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.VAgcRf & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC BB                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.VAgcBB & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("WBD                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcWbd & 0xfff)) / ((double)(1 << 12)));
        printf(CRB "" CRA);

        printf("WBD Split                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcSplitOffset & 0xff)) / ((double)(1 << 8)));
        printf(CRB "" CRA);

        /* print monnitoring status for all demods */
        printf(CRB "------------------------------------------------------" CRA);
        printf("C/N                          \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId] - wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Combined C/N                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", comb_snr[DemId]);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);

        printf("CoffLock                    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_LOCK_COFF(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);

        printf("VitLockA                    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_LOCK_VIT0(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);
        printf("VitLockB                    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_LOCK_VIT1(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);
        printf("VitLockC                    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_LOCK_VIT2(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);

        printf("FecLockMpegA                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_LOCK_MPEG0(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);
        printf("FecLockMpegB                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_LOCK_MPEG1(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);
        printf("FecLockMpegC                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_LOCK_MPEG2(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);

        printf("BERA                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4e\t\t", ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[0].FecBerRs) / ((double)100000000));
        printf(CRB "" CRA);
        printf("BERB                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4e\t\t", ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[1].FecBerRs) / ((double)100000000));
        printf(CRB "" CRA);
        printf("BERC                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4e\t\t", ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[2].FecBerRs) / ((double)100000000));
        printf(CRB "" CRA);

        printf("FecMpfailCntA               \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[0].FecMpfailCnt);
        printf(CRB "" CRA);
        printf("FecMpfailCntB               \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[1].FecMpfailCnt);
        printf(CRB "" CRA);
        printf("FecMpfailCntC               \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[2].FecMpfailCnt);
        printf(CRB "" CRA);

        printf("QualityA                     \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[0].SignalQuality);
        printf(CRB "" CRA);
        printf("QualityB                     \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[1].SignalQuality);
        printf(CRB "" CRA);
        printf("QualityC                     \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[2].SignalQuality);
        printf(CRB "" CRA);

        printf("Quality_fastA                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[0].SignalQualityFast);
        printf(CRB "" CRA);
        printf("Quality_fastB                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[1].SignalQualityFast);
        printf(CRB "" CRA);
        printf("Quality_fastC                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", pMnt->DemodMonit[DemId].Isdbt.Layer[2].SignalQualityFast);
        printf(CRB "" CRA);

        printf("Emergency Flag                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_ISDBT_FLAG_EMERGENCY(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks));
        printf(CRB "" CRA);
        break;

    case DTA_CSV_DISPLAY:

#if defined(WINCE) || defined(WIN32)
		buf[0] = 0;
#else
       {
          struct OsTimeval_t tv;
          OsGetTime(&tv);
          ctime_r((&tv.tv_sec), buf);
       }
#endif
        buf[strlen(buf)-1] = ' ';

        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            /* "date/time;DemodId;Lock;C/N;SQ;AGC RF;AGC BB;AGC global;AD Power;RF Power" */
            fprintf(logFile, "%s;%d;%d;%.2f;%d;%.4g;%.4g;%.4g;%.2g;%.2lf;",
                    buf,
                    DemId, (int)pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks,
                    sig_dB[DemId]-wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Isdbt.Dvb.SignalQuality,
                    ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.VAgcRf))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.VAgcBB))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcGlobal)) / ((double)(1 << 16)),
                    ad_pow_dB[DemId],
                    ComputeRfPowerDVBSH(pMnt->DemodMonit[DemId].Isdbt.Dvb.VAgcRf,
                                        pMnt->DemodMonit[DemId].Isdbt.Dvb.VAgcBB,
                                        (uint16_t) ad_pow_dB[DemId],
                                        DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks), 1)
                    );
        }
        /* Short display to standard output */
    case DTA_SHORT_DISPLAY:
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            LOG(CRB "" CRA);
            LOG(CRB "TS %d (demod %d): Lock=%d, C/N=%0.02f dB, SQF= A=%3d B=%3d C=%3d, LNA=%d, ADP=%0.02f dB, AGC=%0.02f, BER A=%0.03le B=%0.03le C=%0.03le",
                    pMnt->ChannelIndex, DemId, pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks,
                    sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Isdbt.Layer[0].SignalQualityFast,
                    pMnt->DemodMonit[DemId].Isdbt.Layer[1].SignalQualityFast,
                    pMnt->DemodMonit[DemId].Isdbt.Layer[2].SignalQualityFast,
                    pMnt->DemodMonit[DemId].Isdbt.Dvb.LnaStatus, ad_pow_dB[DemId],
                    ((double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcGlobal)) /
                    ((double)(1 << 16)),
                    ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[0].FecBerRs) / ((double)100000000),
                    ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[1].FecBerRs) / ((double)100000000),
                    ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[2].FecBerRs) / ((double)100000000));
            printf(CRB "Signal: Lock: (%d,%d,%d)\t C/N: %0.02f\t BER: (%5.4e,%5.4e,%5.4e)\t Perr (%d,%d,%d)" CRA,
                    DIB_GET_ISDBT_LOCK_MPEG0(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks), 
                    DIB_GET_ISDBT_LOCK_MPEG1(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks), 
                    DIB_GET_ISDBT_LOCK_MPEG2(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks), 
                    sig_dB[DemId] - wgn_dB[DemId],
                    ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[0].FecBerRs) / ((double)100000000),
                    ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[1].FecBerRs) / ((double)100000000),
                    ((double)pMnt->DemodMonit[DemId].Isdbt.Layer[2].FecBerRs) / ((double)100000000),
                    pMnt->DemodMonit[DemId].Isdbt.Layer[0].FecMpfailCnt,
                    pMnt->DemodMonit[DemId].Isdbt.Layer[1].FecMpfailCnt,
                    pMnt->DemodMonit[DemId].Isdbt.Layer[2].FecMpfailCnt);
        }
        break;

        /* Tiny display to standard output. */
    case DTA_TINY_DISPLAY:
        printf("-- Lock=%d, C/N=%0.02f dB, SQ FAST A=%3d B=%3d C=%3d",
                DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[0].Isdbt.Dvb.Locks), sig_dB[0] - wgn_dB[0],
                pMnt->DemodMonit[0].Isdbt.Dvb.SignalQualityFast,
                pMnt->DemodMonit[1].Isdbt.Dvb.SignalQualityFast,
                pMnt->DemodMonit[2].Isdbt.Dvb.SignalQualityFast);
        break;


    default:
        break;
    }
#endif
}

/**
 * Display total demod monit filled structure
 */
void DtaDisplayDvbshSignalMonitInfo(struct DibTotalDemodMonit *pMnt, uint8_t display, FILE *logFile)
{
#if (DIB_DVBSH_STD == 1)
    char buf[52];
    double tmp_pow[DIB_MAX_NB_DEMODS], ad_pow_dB[DIB_MAX_NB_DEMODS];
    double sig_dB[DIB_MAX_NB_DEMODS], wgn_dB[DIB_MAX_NB_DEMODS];
    double comb_snr[DIB_MAX_NB_DEMODS];
    uint8_t  DemId;

    for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
    {
        sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dvbsh.Dvb.SigFp) - 320 * log10(2);
        wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dvbsh.Dvb.WgnFp) - 320 * log10(2);

        comb_snr[DemId] = 10 * (log10(pMnt->DemodMonit[DemId].Dvbsh.Dvb.CombSigFp) - log10(pMnt->DemodMonit[DemId].Dvbsh.Dvb.CombWgnFp));

        tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.AgcPower);
        tmp_pow[DemId]  /= (double)(1 << 20);
        ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
    }

    switch (display)
    {
        /* Full display to standard output. */
    case DTA_FULL_DISPLAY:
        OsClearScreen();
        printf(CRB "------------------------------------------------------" CRA);
        /* TODO: RfPower and total gain */
        printf("(A/D power)                  \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", ad_pow_dB[DemId]);
        printf(CRB "" CRA);

        printf("Carrier                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId]);
        printf(CRB "" CRA);

        printf("Noise                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Timing Offset                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d ppm\t\t", pMnt->DemodMonit[DemId].Dvbsh.Dvb.TimingOffset);
        printf(CRB "" CRA);

        printf("Frequency Offset             \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%.3f KHz\t\t", pMnt->DemodMonit[DemId].Dvbsh.Dvb.FrequencyOffset/1000.0);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);
        /* TODO; lna status */

        printf("AGC GLOBAL                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4g\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.AgcGlobal)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC RF                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.VAgcRf & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC BB                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.VAgcBB & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("WBD                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.AgcWbd & 0xfff)) / ((double)(1 << 12)));
        printf(CRB "" CRA);

        printf("WBD Split                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.AgcSplitOffset & 0xff)) / ((double)(1 << 8)));
        printf(CRB "" CRA);

        /* print monnitoring status for all demods */
        printf(CRB "------------------------------------------------------" CRA);
        printf("C/N                          \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId] - wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Combined C/N                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", comb_snr[DemId]);
        printf(CRB "" CRA);

        printf("Quality                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Dvbsh.Dvb.SignalQuality);
        printf(CRB "" CRA);

        printf("Quality_fast                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Dvbsh.Dvb.SignalQualityFast);
        printf(CRB "" CRA);

        /* TODO: signal handover point */

        printf(CRB "------------------------------------------------------" CRA);

        printf("Per                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%0.05f\t\t\t", ((double)pMnt->DemodMonit[DemId].Dvbsh.Per) / ((double)(1 << 15)));
        printf(CRB "" CRA);

        printf("CoffLock                    \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_COFF_LOCK(pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks));
        printf(CRB "" CRA);

        printf("ShLock                     \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_SH_LOCK(pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks));
        printf(CRB "" CRA);

        printf("TpsSyncLock                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_TPS_SYNC_LOCK(pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks));
        printf(CRB "" CRA);

        printf("TpsDataLock                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", DIB_GET_TPS_DATA_LOCK(pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks));
        printf(CRB "" CRA);
        printf(CRB "" CRA);

        break;

        /* Short display to standard output */
    case DTA_CSV_DISPLAY:
#if defined(WINCE) || defined(WIN32)
		buf[0] = 0;
#else
       {
          struct OsTimeval_t tv;
          OsGetTime(&tv);
          ctime_r((&tv.tv_sec), buf);
       }
#endif
        buf[strlen(buf)-1] = '>';
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            /* "date/time;DemodId;Lock;C/N;SQ;AGC RF;AGC BB;AGC global;AD Power;RF Power" */
            fprintf(logFile, "%s;%d;%d;%.2f;%d;%.4g;%.4g;%.4g;%.2g;%.2lf;",
                    buf,
                    DemId, (int)pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks,
                    sig_dB[DemId]-wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Dvbsh.Dvb.SignalQuality,
                    ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.VAgcRf))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.VAgcBB))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.AgcGlobal)) / ((double)(1 << 16)),
                    ad_pow_dB[DemId],
                    ComputeRfPowerDVBSH(pMnt->DemodMonit[DemId].Dvbsh.Dvb.VAgcRf,
                                        pMnt->DemodMonit[DemId].Dvbsh.Dvb.VAgcBB,
                                        pMnt->DemodMonit[DemId].Dvbsh.Dvb.AgcPower,
                                        DIB_GET_SH_LOCK(pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks), 1)
                    );
        }
    case DTA_SHORT_DISPLAY:
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            LOG(CRB "" CRA);
            LOG(CRB "TS %d (demod %d): Lock=%d, C/N=%0.02f dB, SQ=%3d, SQF=%3d, LNA=%d, ADP=%0.02f dB, AGC=%0.02f, PER=%0.03le",
                    pMnt->ChannelIndex, DemId, pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks,
                    sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Dvbsh.Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Dvbsh.Dvb.SignalQualityFast,
                    /*pMnt->DemodMonit[DemId].Dvbsh.LnaStatus*/0, ad_pow_dB[DemId],
                    ((double)(pMnt->DemodMonit[DemId].Dvbsh.Dvb.AgcGlobal)) /
                    ((double)(1 << 16)),
                    ((double)pMnt->DemodMonit[DemId].Dvbsh.Per) / (1 << 15));
            printf(CRB "Signal: Lock: %d\t C/N: %0.03f\t SQ: %3d\t SQF: %3d\t Powup Time: %d" CRA,
                    DIB_GET_SH_LOCK(pMnt->DemodMonit[DemId].Dvbsh.Dvb.Locks), sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Dvbsh.Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Dvbsh.Dvb.SignalQualityFast,
                    0/*pMnt->DemodMonit[0].power_up_time_100us / 10, */);
        }
        break;

        /* Tiny display to standard output. */
    case DTA_TINY_DISPLAY:
        printf("-- Lock=%d, C/N=%0.02f dB, SQ=%3d, SQ FAST=%3d",
                DIB_GET_SH_LOCK(pMnt->DemodMonit[0].Dvbsh.Dvb.Locks), sig_dB[0] - wgn_dB[0],
                pMnt->DemodMonit[0].Dvbsh.Dvb.SignalQuality,
                pMnt->DemodMonit[0].Dvbsh.Dvb.SignalQualityFast);
        break;

    default:
        break;
    }
#endif
}


#if (DIB_DAB_STD == 1)
void DtaDisplayDabSignalMonitInfo(struct DibTotalDemodMonit *pMnt, uint8_t display, FILE *logFile)
{
   char buf[52];
   double tmp_pow[DIB_MAX_NB_DEMODS], ad_pow_dB[DIB_MAX_NB_DEMODS];
   double sig_dB[DIB_MAX_NB_DEMODS], wgn_dB[DIB_MAX_NB_DEMODS];
   double comb_snr[DIB_MAX_NB_DEMODS];
   uint8_t  DemId;

   for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
   {
       sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dab.Dvb.SigFp) - 320 * log10(2);
       wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dab.Dvb.WgnFp) - 320 * log10(2);

       comb_snr[DemId] = 10 * (log10(pMnt->DemodMonit[DemId].Dab.Dvb.CombSigFp) - log10(pMnt->DemodMonit[DemId].Dab.Dvb.CombWgnFp));

       tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Dab.Dvb.AgcPower);
       tmp_pow[DemId]  /= (double)(1 << 20);
       ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
   }

   switch (display)
   {
       /* Full display to standard output. */
   case DTA_FULL_DISPLAY:
       OsClearScreen();
       printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);

       printf("(A/D power)                  \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%3.2f dB\t\t", ad_pow_dB[DemId]);
       printf(CRB "" CRA);

       printf("Carrier                      \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%3.2f dB\t\t", sig_dB[DemId]);
       printf(CRB "" CRA);

       printf("Noise                        \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%3.2f dB\t\t", wgn_dB[DemId]);
       printf(CRB "" CRA);

       printf("Timing Offset                \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d ppm\t\t\t", pMnt->DemodMonit[DemId].Dab.Dvb.TimingOffset);
       printf(CRB "" CRA);

       printf("Frequency Offset             \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%.3f KHz\t\t", pMnt->DemodMonit[DemId].Dab.Dvb.FrequencyOffset/1000.0);
       printf(CRB "" CRA);

       printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);

       printf("AGC GLOBAL                   \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4g\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dab.Dvb.AgcGlobal)) / ((double)(1 << 16)));
       printf(CRB "" CRA);

       printf("AGC RF                       \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dab.Dvb.VAgcRf & 0xffff)) / ((double)(1 << 16)));
       printf(CRB "" CRA);

       printf("AGC BB                       \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dab.Dvb.VAgcBB & 0xffff)) / ((double)(1 << 16)));
       printf(CRB "" CRA);

       printf("WBD                         \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dab.Dvb.AgcWbd & 0xfff)) / ((double)(1 << 12)));
       printf(CRB "" CRA);

       printf("WBD Split                   \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Dab.Dvb.AgcSplitOffset & 0xff)) / ((double)(1 << 8)));
       printf(CRB "" CRA);

       /* print monnitoring status for all demods */
       printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);

       printf("C/N                          \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%3.2f dB\t\t", sig_dB[DemId] - wgn_dB[DemId]);
       printf(CRB "" CRA);

       printf("Combined C/N                 \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%3.2f dB\t\t", comb_snr[DemId]);
       printf(CRB "" CRA);

       printf("BER                          \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4e\t\t", ((double)pMnt->DemodMonit[DemId].Dab.Dvb.FecBerRs) / ((double)100000000));
       printf(CRB "" CRA);
       printf("FEC BER                      \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4e\t\t", ((double)pMnt->DemodMonit[DemId].Dab.DmbBer) / ((double)100000000));
       printf(CRB "" CRA);

       printf(CRB "------------------------------------------------------------------------------------------------------------------" CRA);

       printf("DmbLockFrame                 \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", ((uint32_t)pMnt->DemodMonit[DemId].Dab.DmbLockFrame));
       printf(CRB "" CRA);

       printf("DataLock                    \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Dab.Dvb.Locks));
       printf(CRB "" CRA);

       printf("VitLock                    \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", DIB_GET_VIT_LOCK(pMnt->DemodMonit[DemId].Dab.Dvb.Locks));
       printf(CRB "" CRA);

       printf("SignalLock                    \t");
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
           printf("%d\t\t\t", DIB_GET_COFF_LOCK(pMnt->DemodMonit[DemId].Dab.Dvb.Locks));
       printf(CRB "" CRA);

       printf("-- SQ=%3d, SQ FAST=%3d",
               pMnt->DemodMonit[0].Dab.Dvb.SignalQuality,
               pMnt->DemodMonit[0].Dab.Dvb.SignalQualityFast);

       printf(CRB "" CRA);

       break;

       /* Short display to standard output */
   case DTA_CSV_DISPLAY:
#if defined(WINCE) || defined(WIN32)
     buf[0] = 0;
#else
      {
         struct OsTimeval_t tv;
         OsGetTime(&tv);
         ctime_r((&tv.tv_sec), buf);
      }
#endif
       buf[strlen(buf)-1] = '>';

   case DTA_SHORT_DISPLAY:
       for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
       {
           LOG(CRB "" CRA);
           LOG(CRB "TS %d (demod %d): Lock=%d, C/N=%0.02f dB, SQ=%3d, SQF=%3d, LNA=%d, ADP=%0.02f dB, AGC=%0.02f",
                   pMnt->ChannelIndex, DemId, pMnt->DemodMonit[DemId].Dab.Dvb.Locks,
                   sig_dB[DemId] - wgn_dB[DemId],
                   pMnt->DemodMonit[DemId].Dab.Dvb.SignalQuality,
                   pMnt->DemodMonit[DemId].Dab.Dvb.SignalQualityFast,
                   /*pMnt->DemodMonit[DemId].Dab.LnaStatus*/0, ad_pow_dB[DemId],
                   ((double)(pMnt->DemodMonit[DemId].Dab.Dvb.AgcGlobal)) /
                   ((double)(1 << 16)));

           printf(CRB "Signal: Lock: %d\t C/N: %0.03f\t SQ: %3d\t SQF: %3d\t Powup Time: %d" CRA,
                   DIB_GET_SH_LOCK(pMnt->DemodMonit[DemId].Dab.Dvb.Locks), sig_dB[DemId] - wgn_dB[DemId],
                   pMnt->DemodMonit[DemId].Dab.Dvb.SignalQuality,
                   pMnt->DemodMonit[DemId].Dab.Dvb.SignalQualityFast,
                   0/*pMnt->DemodMonit[0].power_up_time_100us / 10, */);
       }
       break;

       /* Tiny display to standard output. */
   case DTA_TINY_DISPLAY:
       printf("-- Lock=%d, C/N=%0.02f dB, SQ=%3d, SQ FAST=%3d",
               DIB_GET_SH_LOCK(pMnt->DemodMonit[0].Dab.Dvb.Locks), sig_dB[0] - wgn_dB[0],
               pMnt->DemodMonit[0].Dab.Dvb.SignalQuality,
               pMnt->DemodMonit[0].Dab.Dvb.SignalQualityFast);
       break;

   default:
       break;
   }
}
#endif

#if (DIB_CMMB_STD == 1)
/**
 * Display signal monitoring info for CMMB standard.
 */
void DtaDisplayCmmbSignalMonitInfo(struct DibTotalDemodMonit *pMnt, uint8_t display, FILE *logFile)
{
    char buf[52];
    double tmp_pow[DIB_MAX_NB_DEMODS], ad_pow_dB[DIB_MAX_NB_DEMODS];
    double sig_dB[DIB_MAX_NB_DEMODS], wgn_dB[DIB_MAX_NB_DEMODS];
    double comb_snr[DIB_MAX_NB_DEMODS];
    uint8_t  DemId;

    for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
    {
        sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Cmmb.Dvb.SigFp) - 320 * log10(2);
        wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Cmmb.Dvb.WgnFp) - 320 * log10(2);

        comb_snr[DemId] = 10 * (log10(pMnt->DemodMonit[DemId].Cmmb.Dvb.CombSigFp) - log10(pMnt->DemodMonit[DemId].Cmmb.Dvb.CombWgnFp));

        tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.AgcPower);
        tmp_pow[DemId]  /= (double)(1 << 20);
        ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
    }

    switch (display)
    {
        /* Full display to standard output. */
    case DTA_FULL_DISPLAY:
        OsClearScreen();
        printf(CRB "------------------------------------------------------" CRA);
        /* TODO: RfPower and total gain */
        printf("(A/D power)                  \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", ad_pow_dB[DemId]);
        printf(CRB "" CRA);

        printf("Carrier                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId]);
        printf(CRB "" CRA);

        printf("Noise                        \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Timing Offset                \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d ppm\t\t", pMnt->DemodMonit[DemId].Cmmb.Dvb.TimingOffset);
        printf(CRB "" CRA);

        printf("Frequency Offset             \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%.3f KHz\t\t", pMnt->DemodMonit[DemId].Cmmb.Dvb.FrequencyOffset/1000.0);
        printf(CRB "" CRA);

        printf(CRB "------------------------------------------------------" CRA);
        /* TODO; lna status */

        printf("AGC GLOBAL                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4g\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.AgcGlobal)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC RF                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.VAgcRf & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("AGC BB                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.VAgcBB & 0xffff)) / ((double)(1 << 16)));
        printf(CRB "" CRA);

        printf("WBD                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.AgcWbd & 0xfff)) / ((double)(1 << 12)));
        printf(CRB "" CRA);

        printf("WBD Split                   \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%5.4lf\t\t\t", ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.AgcSplitOffset & 0xff)) / ((double)(1 << 8)));
        printf(CRB "" CRA);

        /* print monnitoring status for all demods */
        printf(CRB "------------------------------------------------------" CRA);
        printf("C/N                          \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", sig_dB[DemId] - wgn_dB[DemId]);
        printf(CRB "" CRA);

        printf("Combined C/N                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%3.2f dB\t\t", comb_snr[DemId]);
        printf(CRB "" CRA);

        printf("Quality                      \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Cmmb.Dvb.SignalQuality);
        printf(CRB "" CRA);

        printf("Quality_fast                 \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%d\t\t\t", pMnt->DemodMonit[DemId].Cmmb.Dvb.SignalQualityFast);
        printf(CRB "" CRA);

        /* TODO: signal handover point */

        printf(CRB "------------------------------------------------------" CRA);

        printf("Per                         \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%0.05f\t\t\t", ((double)pMnt->DemodMonit[DemId].Cmmb.Per) / ((double)(1 << 15)));
        printf(CRB "" CRA);

        printf("Locks                       \t");
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
            printf("%08x\t\t\t", pMnt->DemodMonit[DemId].Cmmb.Dvb.Locks);
        printf(CRB "" CRA);

        break;

        /* Short display to standard output */
    case DTA_CSV_DISPLAY:
#if defined(WINCE) || defined(WIN32)
		buf[0] = 0;
#else
       {
          struct OsTimeval_t tv;
          OsGetTime(&tv);
          ctime_r((&tv.tv_sec), buf);
       }
#endif
        buf[strlen(buf)-1] = '>';
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            /* "date/time;DemodId;Lock;C/N;SQ;AGC RF;AGC BB;AGC global;AD Power;RF Power" */
            fprintf(logFile, "%s;%d;%d;%.2f;%d;%.4g;%.4g;%.4g;%.2g;%.2lf;",
                    buf,
                    DemId, (int)pMnt->DemodMonit[DemId].Cmmb.Dvb.Locks,
                    sig_dB[DemId]-wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Cmmb.Dvb.SignalQuality,
                    ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.VAgcRf))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.VAgcBB))    / ((double)(1 << 16)),
                    ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.AgcGlobal)) / ((double)(1 << 16)),
                    ad_pow_dB[DemId],
#if (DIB_DVBSH_STD == 1)
                    ComputeRfPowerDVBSH(pMnt->DemodMonit[DemId].Cmmb.Dvb.VAgcRf,
                                        pMnt->DemodMonit[DemId].Cmmb.Dvb.VAgcBB,
                                        pMnt->DemodMonit[DemId].Cmmb.Dvb.AgcPower,
                                        DIB_GET_SH_LOCK(pMnt->DemodMonit[DemId].Cmmb.Dvb.Locks), 1)
#else
                    0
#endif
                    );
        }
    case DTA_SHORT_DISPLAY:
        for(DemId = 0; DemId < pMnt->NbDemods; DemId++)
        {
            LOG(CRB "" CRA);
            LOG(CRB "TS %d (demod %d): Lock=%d, C/N=%0.02f dB, SQ=%3d, SQF=%3d, LNA=%d, ADP=%0.02f dB, AGC=%0.02f, PER=%0.03le",
                    pMnt->ChannelIndex, DemId, pMnt->DemodMonit[DemId].Cmmb.Dvb.Locks,
                    sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Cmmb.Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Cmmb.Dvb.SignalQualityFast,
                    /*pMnt->DemodMonit[DemId].Cmmb.LnaStatus*/0, ad_pow_dB[DemId],
                    ((double)(pMnt->DemodMonit[DemId].Cmmb.Dvb.AgcGlobal)) /
                    ((double)(1 << 16)),
                    ((double)pMnt->DemodMonit[DemId].Cmmb.Per) / (1 << 15));
            printf(CRB "Signal: Lock: %d\t C/N: %0.03f\t SQ: %3d\t SQF: %3d\t Powup Time: %d" CRA,
                    DIB_GET_SH_LOCK(pMnt->DemodMonit[DemId].Cmmb.Dvb.Locks), sig_dB[DemId] - wgn_dB[DemId],
                    pMnt->DemodMonit[DemId].Cmmb.Dvb.SignalQuality,
                    pMnt->DemodMonit[DemId].Cmmb.Dvb.SignalQualityFast,
                    0/*pMnt->DemodMonit[0].power_up_time_100us / 10, */);
        }
        break;

        /* Tiny display to standard output. */
    case DTA_TINY_DISPLAY:
        printf("-- Lock=%d, C/N=%0.02f dB, SQ=%3d, SQ FAST=%3d",
                DIB_GET_SH_LOCK(pMnt->DemodMonit[0].Cmmb.Dvb.Locks), sig_dB[0] - wgn_dB[0],
                pMnt->DemodMonit[0].Cmmb.Dvb.SignalQuality,
                pMnt->DemodMonit[0].Cmmb.Dvb.SignalQualityFast);
        break;

    default:
        break;
    }
}
#endif

/**
 * Display total demod monit filled structure
 */
void DtaDisplaySignalMonitInfo(struct DibTotalDemodMonit *pMnt, uint8_t display, FILE *logFile)
{
    switch (pMnt->Type)
    {
        /* Full display to standard output. */
    case eSTANDARD_DVB:
    case eSTANDARD_DVB | eFAST:
    case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
        DtaDisplayDvbSignalMonitInfo(pMnt, display, logFile);
        break;

    case eSTANDARD_ISDBT:
    case eSTANDARD_ISDBT_1SEG:
    case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
    case eSTANDARD_ISDBT_1SEG | eALWAYS_TUNE_SUCCESS:
        DtaDisplayIsdbtSignalMonitInfo(pMnt, display, logFile);
        break;

    case eSTANDARD_DVBSH:
    case (eSTANDARD_DVBSH | eFAST):
        DtaDisplayDvbshSignalMonitInfo(pMnt, display, logFile);
        break;
#if (DIB_DAB_STD == 1)
    case eSTANDARD_DAB:
    case eSTANDARD_DAB | eALWAYS_TUNE_SUCCESS:
        DtaDisplayDabSignalMonitInfo(pMnt, display, logFile);
       break;
#endif
#if (DIB_CMMB_STD == 1)
    case eSTANDARD_CMMB:
    case eSTANDARD_CMMB | eALWAYS_TUNE_SUCCESS:
        DtaDisplayCmmbSignalMonitInfo(pMnt, display, logFile);
        break;
#endif
#if (DIB_CTTB_STD == 1)
    case eSTANDARD_CTTB:
    case eSTANDARD_CTTB |eALWAYS_TUNE_SUCCESS:
        DtaDisplayCttbSignalMonitInfo(pMnt, display, logFile);
        break;
#endif
    default:
        printf("No monitoring for standard %d\n", pMnt->Type);
        break;
    }
}


/**
 * DtaGetSignalCallback
 */
struct GetSignalContext
{
    volatile int               GetSignalFinished;
    struct DibTotalDemodMonit *GetSignalInfo;
};

void DtaGetSignalCallback(struct DibTotalDemodMonit *pSignalDescriptor, uint8_t *pNbDemod, void *pContext)
{
    struct GetSignalContext *gsc = (struct GetSignalContext *)pContext;

    *(gsc->GetSignalInfo)   = *pSignalDescriptor;
    gsc->GetSignalFinished = 1;

    printf(CRB "DtaTuneMonitCallback : %d demod" CRA, *pNbDemod);
}

void DtaGetSHMonitInfo(uint8_t ChannelId)
{
#if (DIB_DVBSH_STD == 1)
    union DibDemodMonit      * m;
    DIBSTATUS                 rc = DIBSTATUS_SUCCESS;
    struct DibTotalDemodMonit mnt;
    uint8_t                   DemodNb = 1;
    uint8_t                   ChannelHdl = ChannelArray[ChannelId].ChannelHdl;

    m = &ChannelArray[ChannelId].ChannelMonit.DemodMonit[0];
    memset(m, 0, sizeof(union DibDemodMonit)*DIB_MAX_NB_DEMODS);

    rc = DibGetSignalMonitoring(CONTEXT, ChannelHdl, &DemodNb, m, 0, 0);

    switch (rc)
    {
    case DIBSTATUS_TIMEOUT:
        printf(CRB "  TS(%d): RF=%6d kHz: Time Out ***" CRA, ChannelId, ChannelArray[ChannelId].Info.ChannelDescriptor.RFkHz);
        ChannelArray[ChannelId].NbTimeout++;
        break;

    case DIBSTATUS_SUCCESS:

        /* change the following function to accept a DibDemodMonit */
        memcpy(&(mnt.DemodMonit[0]), m, sizeof(union DibDemodMonit));

        mnt.NbDemods     = 1;
        mnt.ChannelIndex = ChannelHdl;

        printf(CRB "-------------------------------------" CRA);
        printf(CRB "PER                   : %d" CRA, mnt.DemodMonit[0].Dvbsh.Per);
        printf(CRB "Locks 0x%4x (Corm|Coff|Coff Cpil|Lmod4|Pha3|Equal|Vit/SH|R|R|TpsSync|TpsData|TpsBch|TpsCellId|Dvsy) " CRA, mnt.DemodMonit[0].Dvbsh.Dvb.Locks);
        printf(CRB "" CRA);

        ChannelArray[ChannelId].NbReliable++;
        break;

    default:
    case DIBSTATUS_ERROR:
        printf(CRB "*** Error: 'DibGetSignalMonitoring' returned %d" CRA, rc);
        break;
    }
#endif
}

/*------------------------------------------------------------------------------
  DtaGetSignalMonitInfo
  Get signal monitoring, display the signal if reliable, and increment timeout or reliable variable
  ------------------------------------------------------------------------------*/
void DtaGetSignalMonitInfo(uint8_t ChannelId, uint8_t DemodNb, uint8_t display, uint8_t async, FILE *logFile)
{
    union DibDemodMonit      *m;
    DIBSTATUS                 rc = DIBSTATUS_SUCCESS;
    DibGetSignalCallback      CallBack = NULL;
    struct DibTotalDemodMonit mnt;
    uint8_t                   MaxNbDemod = DIB_UNSET;
    void                     *CallCtx = NULL;
    struct GetSignalContext   gsc;
    uint8_t                   ChannelHdl = ChannelArray[ChannelId].ChannelHdl;

    if (ChannelArray[ChannelId].Used == 0)
        ChannelHdl = 255;

    if(async)
    {
        gsc.GetSignalFinished = 0;
        gsc.GetSignalInfo     = &ChannelArray[ChannelId].ChannelMonit;
        CallCtx               = (void*)&gsc;
        CallBack              = DtaGetSignalCallback;
    }

    m = &ChannelArray[ChannelId].ChannelMonit.DemodMonit[0];
    memset(m, 0, sizeof(union DibDemodMonit)*DIB_MAX_NB_DEMODS);

    rc = DibGetSignalMonitoring(CONTEXT, ChannelHdl, &MaxNbDemod, m, CallBack, CallCtx);

    if(rc != DIBSTATUS_SUCCESS)
    {
        return;
    }

    if(DemodNb == DIB_DEMOD_AUTO)
    {
        DemodNb = MaxNbDemod;
    }
    else if(DemodNb > MaxNbDemod || DemodNb > DIB_MAX_NB_DEMODS)
    {
        printf(CRB "the number of demod is bigger that the max possible : %d" CRA, MaxNbDemod);
    }

    DtaRealTimeSet();

    rc = DibGetSignalMonitoring(CONTEXT, ChannelHdl, &DemodNb, m, CallBack, CallCtx);


    switch (rc)
    {
    case DIBSTATUS_TIMEOUT:
        printf(CRB "  TS(%d): RF=%6d kHz: Time Out ***" CRA, ChannelId, ChannelArray[ChannelId].Info.ChannelDescriptor.RFkHz);
        ChannelArray[ChannelId].NbTimeout++;
        break;

    case DIBSTATUS_SUCCESS:
        if(async)
        {
            while(gsc.GetSignalFinished == 0)
            {
                OsMSleep(100);
            }
        }
        /* change the following function to accept a DibDemodMonit */
        memcpy(&(mnt.DemodMonit[0]), m, sizeof(union DibDemodMonit)*DemodNb);
        if(DemodNb < MaxNbDemod)
        {
            printf(CRB "Warning we do not ask for all the demod monit : %d instead of %d" CRA, DemodNb, MaxNbDemod);
        }
        mnt.NbDemods     = DemodNb;
        mnt.ChannelIndex = ChannelHdl;
        mnt.Type = ChannelArray[ChannelId].Info.Type;

        DtaDisplaySignalMonitInfo(&mnt, display, logFile);
        ChannelArray[ChannelId].NbReliable++;
        break;

    default:
    case DIBSTATUS_ERROR:
        printf(CRB "%s: *** Error: 'DibGetSignalMonitoring' returned %d" CRA,
                __FUNCTION__, rc);
        break;
    }

    /* LOG("--> status %d, duration %d ms", m->ChannelMonit.MonitStatus, DtaRealTimeDiffMs()); */
}

/**
 * DtaSignalMonitLoop
 */
static int32_t mon_intr;

static void sigint(int32_t sig)
{
    mon_intr = 1;
}

#if (DIB_INTERNAL_DEBUG == 1) && (DF_DBG_SPAL == 1)
#ifdef KB_GETC
void DtaSpalMnt(void)
{
    do
    {
        OsClearScreen();
        IntDbgDisplaySpalMnt();
        OsMSleep(100);
    } while(kb_getc() == 0);
}

#else

void DtaSpalMnt(void)
{
    mon_intr = 0;
    do
    {
        OsClearScreen();
        IntDbgDisplaySpalMnt();
        OsMSleep(100);
    } while(!mon_intr);
}
#endif
#endif


#ifdef KB_GETC
void DtaSignalMonitLoop(uint8_t ChannelIndex, uint32_t n_iter, uint8_t n_demod)
{
    do
    {
        DtaGetSignalMonitInfo(ChannelIndex, n_demod, DTA_FULL_DISPLAY, 0, stdout);
        OsMSleep(100);
    } while(kb_getc() == 0);
}
#else

void DtaSignalMonitLoop(uint8_t ChannelIndex, uint32_t n_iter, uint8_t n_demod)
{
    mon_intr = 0;
#ifndef WINCE
    signal(SIGINT, sigint);
#endif

 gEsrLastSec =0xFFFFFFFF;
 gEsrTotalSec = 0;
 gEsrErrSec = 0;
    do
    {
        n_iter--;

        /* LOG(CRB "Monit (n_iter=%d) " CRA, n_iter); */

        DtaGetSignalMonitInfo(ChannelIndex, n_demod, DTA_FULL_DISPLAY, 0, stdout);
        OsMSleep(20);
    } while(n_iter && !mon_intr);
#ifndef WINCE
    signal(SIGINT, SigQuitDta);
#endif
}
#endif

void DtaSignalMonitLoopInFile(uint32_t ChannelIndex, uint32_t n_iter, uint32_t n_demod, char *fileName, uint32_t period)
{
	FILE *logFile;
    mon_intr = 0;
#ifndef WINCE
    signal(SIGINT, sigint);
#endif
    LogFileInitDone = 0;

    logFile = fopen(fileName, "w");

    do
    {
        n_iter--;
        DtaGetSignalMonitInfo(ChannelIndex, n_demod, DTA_FULL_DISPLAY, 0, stdout);
        DtaGetSignalMonitInfo(ChannelIndex, n_demod, DTA_CSV_DISPLAY, 0, logFile);
        OsMSleep(period);
    } while(n_iter && !mon_intr);
    fclose(logFile);
#ifndef WINCE
    signal(SIGINT, SigQuitDta);
#endif
}

/**
 * DtaLogMonitStatus
 */
void DtaLogMonitStatus(FILE * f)
{
    int32_t  NbTs = 0;
    uint32_t ChannelId;

    if(!f)
    {
        f = stdout;
    }

    for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
    {
        if(ChannelArray[ChannelId].Used)
        {
            NbTs++;
            fprintf(f, "  TS %d: Frequency %3d MHz: OK/TO: %d / %d" CRA, ChannelId, ChannelArray[ChannelId].Info.ChannelDescriptor.RFkHz / 1000, ChannelArray[ChannelId].NbReliable, ChannelArray[ChannelId].NbTimeout);
        }
    }

    if(!NbTs)
    {
        fprintf(f, "  No persistent TS channels set" CRA);
    }

    fflush(f);
}

/**
 * For each registered channel, display signal monitoring information
 */
int32_t DtaRefreshMonitArray(void)
{
    int32_t NbTs    = 0;
    int32_t ChannelId;
    uint8_t NbDemod = DIB_DEMOD_AUTO;

    for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
    {
        if(ChannelArray[ChannelId].Used)
        {
            NbTs++;
            DtaGetSignalMonitInfo(ChannelId, NbDemod, DTA_SHORT_DISPLAY, 0, stdout);
        }
    }

    if(!NbTs)
    {
        printf(CRB "%s: No persistent TS channels set" CRA, __FUNCTION__);
    }

    return NbTs;
}

/**
 * DtaSetMonitArray
 */
void DtaSetMonitArray(uint8_t ChannelId)
{
    ChannelArray[ChannelId].NbReliable = 0;
    ChannelArray[ChannelId].NbTimeout  = 0;
}

/**
 * DtaClearMonitArray
 */
void DtaClearMonitArray(uint8_t ChannelId)
{
    ChannelArray[ChannelId].NbReliable = 0;
    ChannelArray[ChannelId].NbTimeout  = 0;
}

/**
 * DtaClearAllMonitArray
 */
void DtaClearAllMonitArray(void)
{
    int32_t ChannelId;

    for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
    {
        ChannelArray[ChannelId].NbReliable = 0;
        ChannelArray[ChannelId].NbTimeout  = 0;
    }

}

/*
 * LOCAL FUNCTIONS
 */
#if ((DIB_DVB_STD == 1) || (DIB_ISDBT_STD == 1) || (DIB_CTTB_STD == 1))
static int32_t ComputePoints(int32_t rf_gain, int32_t qual)
{
    return ((150 - rf_gain) * qual);
}

static double ComputeRfPower(uint16_t agc_global, uint8_t LnaStatus, uint8_t Lock)
{
    return ((double)-73.5371*(double)agc_global/(double)65535 - (double)26.696);
}
#endif

#if (DIB_DVB_STD == 1) || (DIB_DVBSH_STD == 1) || (DIB_ISDBT_STD == 1)
static double ComputeRfPowerDVBSH(uint16_t agc_rf, uint16_t agc_bb, uint16_t ad_power, uint8_t Lock, uint8_t lna)
{
    /* Performed at 2.2GHz ; RF level at gain zero = -18 ; ADC noise floor at high BB gain = -25 (cooking)*/
    double agc_rf_r = agc_rf     / ((double)(1<<16));
    double agc_bb_r = agc_bb     / ((double)(1<<16));
    double ad_power_r = ad_power / ((double)(1<<20));
    double rf_from_agc, noise_floor;

    if (lna==0) { /* for NIM29098SH with input LNA OFF */
        rf_from_agc = -19.1 - agc_rf_r*25.3 - agc_bb_r*55.0 - (-16.0-10*log10(ad_power_r-(agc_bb_r>0.907)*(agc_bb_r<0.983)*pow(10.0,-25/10.0)));
        noise_floor = -103.0;
    }
    else if(lna==1) { /* for NIM29098SH with input LNA ON */
        rf_from_agc = 1.2 - agc_rf_r*25.3 - agc_bb_r*55.0 - (-16.0-10*log10(ad_power_r-pow(10.0,-30.15/10.0)));
        noise_floor = -106.0;
    }

    return (double)(10 * log10(pow(10.0,rf_from_agc/10.0)-pow(10.0,noise_floor/10.0)));
}
#endif

static double ComputeiFERrate(uint32_t total, uint32_t good, uint8_t sample)
{
   static int32_t fail [1000] = {0};
   static int32_t total_last = 0;
   static int32_t good_last  = 0;
   double val = -1 ;
   uint32_t nb = 0;
   uint8_t Idx ;
   uint32_t k;

   if (sample==0)
      return -99;

   Idx = (total-1+sample)%sample;
   if (total!=0) {
      nb = (total<sample)? total:sample;
      if (total_last  != total)
      {
         /* new table received */
         if (good_last != good) /* this new table is good */
            fail[Idx] = 0;
         else
            fail[Idx] = 1;
      }
      total_last = total;
      good_last  = good ;
      val = 0;

      for(k = 0; k < nb; k++)
         val += fail[k];

      val /= nb;
      val *= 100;
   }
   return val;
}

static double ComputeiMFERrate(uint32_t total, uint32_t good, uint32_t corrected, uint8_t sample)
{
   static int32_t fail [1000] = {0};
   static int32_t total_last = 0;
   static int32_t good_last  = 0;
   double val = -1 ;
   uint32_t nb = 0;
   uint8_t Idx ;
   uint32_t k;

   if(sample==0)
      return -99;

   Idx = (total-1+sample)%sample;
   if (total!=0)
   {
      nb = (total<sample)? total:sample;
      if (total_last  != total) {/* new table received */
         if (good_last != good+corrected) /* this new table is good */
            fail[Idx] = 0;
         else
            fail[Idx] = 1;
      }
      total_last = total;
      good_last  = good +corrected;
      val = 0;

      for(k = 0; k < nb; k++)
         val += fail[k];

      val /= nb;
      val *= 100;
   }
   return val;
}

static double ComputeFER_MFERrate(uint32_t total, uint32_t error, uint8_t sample)
{
   static int32_t fail [1000] = {0};
   static int32_t total_last = 0;
   static int32_t error_last = 0;
   double val = -1 ;
   uint32_t nb = 0;
   uint8_t  Idx;
   uint32_t k;

   if (sample==0)
      return -99;

   Idx = (total-1+sample)%sample;
   if (total!=0)
   {
      nb = (total<sample)? total:sample;
      if (total_last  != total)
      {
         /* new table received */
         if (error_last != error) /* this new table is erroned */
            fail[Idx] = 1;
         else
            fail[Idx] = 0;
      }
      total_last = total;
      error_last = error;
      val = 0;

      for(k = 0; k < nb; k++)
         val += fail[k];

      val /= nb;
      val *= 100;
   }
   return val;
}


void ComputeEsr(unsigned int perr)
{
   struct OsTimeval_t timeval;
   int32_t            ret;

   ret = OsGetTime( &timeval);

   if (gEsrLastSec == 0xFFFFFFFF)
      gEsrLastSec = timeval.tv_sec;
   else
   {
      if (gEsrLastSec != timeval.tv_sec)
      {
         gEsrLastSec = timeval.tv_sec;
         if (perr != 0)
            gEsrErrSec ++;

         gEsrTotalSec ++;
      }
   }
}
