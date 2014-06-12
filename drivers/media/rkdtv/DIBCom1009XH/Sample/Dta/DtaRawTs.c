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
* @file "DtaRawTs.c"
* @brief DiBcom Test Application - RawTs Functions.
*
***************************************************************************************************/
#define _XOPEN_SOURCE 500
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

extern struct DibDriverContext * gUserContext;

#define CONTEXT (gUserContext)

#define TP_SIZE 188

uint8_t CheckTsInit(void);
void    CheckTsContinuity(uint8_t *pBuf, uint32_t Size);
void    SaveBufferToFile( uint8_t* Buffer,uint32_t Size);

static FILE *fsave = NULL;


static uint8_t PidsCc[8192];
static uint8_t PidsOk[8192 / 8];
const  uint8_t bitmask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };


THREAD_HANDLE    RawTsThreadHandle;
volatile uint8_t RequestStopRawTsThread = 0;
static void     *RawTsThread(void *param);

uint8_t          RawTsBufer[50000];


/*******************************************************************************/
/********************* RAWTS ***************************************************/
/*******************************************************************************/

/**
 * DtaStartRawTs()
 */
int32_t DtaStartRawTs(void)
{
  int32_t status = 0;

  RequestStopRawTsThread = 0;
  CheckTsInit();

  status = OsCreateThread(&RawTsThreadHandle, (PTHREAD_FN)RawTsThread, NULL);

  printf(CRB "RAWTS Thread created, status=%d" CRA, status);

  return (status);
}

/*------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
int32_t DtaStopRawTs(void)
{
  int32_t status;

  RequestStopRawTsThread = 1;

  while(RequestStopRawTsThread == 1) 
  {
    OsMSleep(1);
  }

  status = OsJoinThread(RawTsThreadHandle);

  return (status);
}

/**
 * RawTsThread ()
 */
static void *RawTsThread(void *param)
{
  uint32_t Size = 50000;

  printf(CRB "RAWTS Thread Started" CRA);

  while(RequestStopRawTsThread == 0) 
  {
    int32_t rc;

    Size = 50000;
/*    rc = DibTsRead(CONTEXT, RawTsBufer, &Size); */
rc = 0;
/**/
    printf(CRB "Read TS: Size= %d, rc= %d" CRA, Size, rc);

    CheckTsContinuity(RawTsBufer,Size);
    SaveBufferToFile(RawTsBufer,Size);
  }

  printf(CRB "RAWTS Thread Stopped" CRA);
  RequestStopRawTsThread = 0;

  return (0);
}

/**
 * CheckTsInit 
 */
uint8_t CheckTsInit(void)
{
  memset(PidsCc, 0, sizeof(PidsCc));
  memset(PidsOk, 0, sizeof(PidsOk));

  return DIBSTATUS_SUCCESS;
}

/**
 * CheckTsContinuity 
 */
void CheckTsContinuity(uint8_t *pBuf, uint32_t Size)
{
   uint16_t Pid;
   uint8_t cc;

   if(Size % TP_SIZE != 0) 
   {
      printf(CRB "TS Size not multiple of 188: is %d, %d of extra" CRA, Size, (Size % TP_SIZE));
      return;
   }

   while(Size != 0) 
   {
      if(pBuf[0] != 0x47) 
      {
         printf(CRB "########### No TS sync byte (%02X %02X %02X %02X) ######### " CRA, pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
         return;
      }

      Pid  = (pBuf[1] & 0x1F) << 8;
      Pid |= pBuf[2];

      if(Pid != 8191) 
      {
         cc = pBuf[3] & 0x0f;

         if((PidsOk[Pid / 8] & bitmask[Pid & 7]) == 0) 
         {
            PidsOk[Pid / 8] |= bitmask[Pid & 7];
            PidsCc[Pid]      = (cc + 1) & 0x0f;
            printf(CRB "--> Found PID %04d" CRA, Pid);
         }
         else 
         {
            if(PidsCc[Pid] != cc) 
            {
               printf(CRB "############ TS CC disco %04d ##############" CRA, Pid);
            }
            PidsCc[Pid] = (cc + 1) & 0x0f;
         }
      }
      pBuf += TP_SIZE;
      Size -= TP_SIZE;
   }
}

void SaveBufferToFile( uint8_t* Buffer,uint32_t Size)
{
   if(fsave == NULL)
      fsave = fopen("./record.ts","wb");

   fwrite(Buffer,1,Size,fsave);
}
