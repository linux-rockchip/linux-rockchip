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
* @file "DtaSiPsi.C"
* @brief DiBcom Test Application - SI PSI Functions.
*
***************************************************************************************************/
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "DibConfig.h"
#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaCrc32.h"
#include "DtaCmds.h"
#include "DtaSiPsi.h"

extern struct DibDriverContext * gUserContext;

#define CONTEXT (gUserContext)

extern int32_t DtaNbSiPsiRunning[DIB_MAX_NB_CHANNELS];


static volatile int32_t NbRegSiPsiBuffers  = 0;
static volatile int32_t SiPsiThreadRunning= 0;

#define MAX_SIPSI_PID        10
#define NB_SIPSI_BUFFER       2
#define SIPSI_BUFFER_SIZE  4096

struct SiPsiStatus
{
  int32_t Pid;
  uint32_t ItemId;
  uint32_t NbSectionsOK;
  uint32_t NbSectionsKO;
  uint32_t BufferId[NB_SIPSI_BUFFER];
  uint8_t *Buffer[NB_SIPSI_BUFFER];
};

struct SiPsiStatus SiPsiTable[DIB_MAX_NB_CHANNELS][MAX_SIPSI_PID];


#if(DIB_SIPSI_DATA == 1)
static THREAD_HANDLE SiPsiThreadHandle;
static void          *SiPsiThread(void *param);

/*******************************************************************************/
/********************* SIPSI ***************************************************/
/*******************************************************************************/
static uint8_t GetFirstItem(FILTER_HDL Hdl)
{
   uint32_t Fhdl;
   int ii = 0;

   for(ii = 0; ii < DIB_MAX_NB_ITEMS; ii++)
   {
      Fhdl = FilterArray[ItemArray[ii].ParentFilter].FilterHdl;
      if(ItemArray[ii].Used && (Fhdl == Hdl))
         break;
   }
   return ii;
}
#endif

/**
 * DtaStartSiPsi()
 */
int32_t DtaStartSiPsi(void)
{

   int32_t status = 0;

#if(DIB_SIPSI_DATA == 1)
   int32_t ChannelId;

   (void)GetFirstItem;

   if(SiPsiThreadRunning)
   {
      return -1;
   }

   for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
   {
      DtaResetSiPsiCounters(ChannelId);
   }


   status = OsCreateThread(&SiPsiThreadHandle, (PTHREAD_FN)SiPsiThread, NULL);

   if(status == 0)
   {
      while(SiPsiThreadRunning == 0) 
      {
         OsMSleep(1);
      }

      printf(CRB "SIPSI Thread created, status=%d" CRA, status);
   }
   else
   {
      printf(CRB "SIPSI Thread create FAILED!!!!, status=%d" CRA, status);
   }
#endif

   return (status);
}

/**
 *
 */
int32_t DtaStopSiPsi(void)
{
   int32_t status = -1;

#if(DIB_SIPSI_DATA == 1)
   if(!SiPsiThreadRunning)
   {
      printf(CRB "DtaStopSiPsi FAILED" CRA);
      return -1;
   }

   SiPsiThreadRunning = 0;

   status = OsJoinThread(SiPsiThreadHandle);

#endif
   return (status);
}

/**
 * Add a SIPSI Pid for monitoring. Can be done while SIPSI thread is running
 */
void DtaAddSiPsiPidCounter(int32_t Pid, uint32_t ItemId)
{
#if(DIB_SIPSI_DATA == 1)
   uint32_t  FilterId  = ItemArray[ItemId].ParentFilter;
   uint32_t  ChannelId = FilterArray[FilterId].ParentChannel;
   int32_t   i, j;
   DIBSTATUS rc;

   for(i = 0; i < MAX_SIPSI_PID; i++) 
   {
      if(SiPsiTable[ChannelId][i].Pid == -1) 
      {
         SiPsiTable[ChannelId][i].Pid          = Pid;
         SiPsiTable[ChannelId][i].ItemId       = ItemId;
         SiPsiTable[ChannelId][i].NbSectionsOK = 0;
         SiPsiTable[ChannelId][i].NbSectionsKO = 0;

         for(j = 0; j< NB_SIPSI_BUFFER; j++)
         {
            SiPsiTable[ChannelId][i].Buffer[j]   = (uint8_t *)malloc(SIPSI_BUFFER_SIZE);
            SiPsiTable[ChannelId][i].BufferId[j] = NbRegSiPsiBuffers;

            rc = DibRegisterBuffer(CONTEXT, 
                                   FilterArray[FilterId].FilterHdl,
                                   SiPsiTable[ChannelId][i].Buffer[j],
                                   SIPSI_BUFFER_SIZE,
                                   SiPsiTable[ChannelId][i].BufferId[j]);

            if(rc == DIBSTATUS_SUCCESS)
            {
               /* printf(CRB "DtaAddSiPsiPidCounter RegBuf id %d success" CRA, SiPsiTable[ChannelId][i].BufferId[j]); */
               NbRegSiPsiBuffers++;
            }
            else
            {
               printf(CRB "DtaAddSiPsiPidCounter RegBuf id %d error: rc %d on Item Id %d" CRA, SiPsiTable[ChannelId][i].BufferId[j], rc, ItemId);
            }

         }
         DtaNbSiPsiRunning[ChannelId]++;
         break;
      }
   }
#endif
}

/**
 * Can be removed while the thread is running
 */
void DtaRemoveBuffersFromSipsiItem(int32_t Pid, uint32_t ItemId)
{
#if(DIB_SIPSI_DATA == 1)

   uint32_t  FilterId  = ItemArray[ItemId].ParentFilter;
   uint32_t  ChannelId = FilterArray[FilterId].ParentChannel;
   DIBSTATUS rc;
   int32_t   i, j;

   for(i = 0; i < MAX_SIPSI_PID; i++) 
   {
      if((SiPsiTable[ChannelId][i].Pid == Pid) && (SiPsiTable[ChannelId][i].ItemId == ItemId))
      {
         SiPsiTable[ChannelId][i].Pid = -1;

         for(j = 0; j < NB_SIPSI_BUFFER; j++)
         {
            rc = DibUnregisterBuffer(CONTEXT, FilterArray[FilterId].FilterHdl, SiPsiTable[ChannelId][i].Buffer[j]);

            if(rc == DIBSTATUS_SUCCESS)
            {
               /* printf(CRB "DtaRemoveBuffersFromSipsiItem: Buffer %d properly deregistered" CRA, SiPsiTable[ChannelId][i].BufferId[j]); */
               NbRegSiPsiBuffers--;

               free(SiPsiTable[ChannelId][i].Buffer[j]);
               SiPsiTable[ChannelId][i].Buffer[j] = NULL;
            }
            else 
            {
               /* printf(CRB "DtaRemoveBuffersFromSipsiItem: Buffer %d not deregistered, propably pending" CRA, SiPsiTable[ChannelId][i].BufferId[j]); */
               /** The buffer will be deregistered when the filter will be removed */
            }
         }

         DtaNbSiPsiRunning[ChannelId]--;
         break;
      }
   }
#endif
}


/**
 *
 */
void DtaIncSiPsiOkCounter(int32_t Pid, uint32_t ItemId)
{
#if(DIB_SIPSI_DATA == 1)

   uint32_t ChannelId = FilterArray[ItemArray[ItemId].ParentFilter].ParentChannel;
   int32_t  i;

   for(i = 0; i < MAX_SIPSI_PID; i++) 
   {
      if((SiPsiTable[ChannelId][i].Pid == Pid) && (SiPsiTable[ChannelId][i].ItemId == ItemId))
      {
         SiPsiTable[ChannelId][i].NbSectionsOK++;
         break;
      }
   }
#endif
}

/**
 *
 */
void DtaIncSiPsiKoCounter(int32_t Pid, uint32_t ItemId)
{
#if(DIB_SIPSI_DATA == 1)
   uint32_t ChannelId = FilterArray[ItemArray[ItemId].ParentFilter].ParentChannel;
   int32_t  i;

   for(i = 0; i < MAX_SIPSI_PID; i++) 
   {
      if((SiPsiTable[ChannelId][i].Pid == Pid) && (SiPsiTable[ChannelId][i].ItemId == ItemId))
      {
         SiPsiTable[ChannelId][i].NbSectionsKO++;
         printf(CRB "BAD Section:PID=%d" CRA, Pid);
         break;
      }
   }
#endif
}

/**
 * DtaResetAllSiPsiCounters
 */
void DtaResetAllSiPsiCounters(uint32_t ChannelId)
{
#if(DIB_SIPSI_DATA == 1)
   int32_t i;

   /* Even if the thread is started, we can remove the pids cause eEMPTY callback is supported */
   for(i = 0; i < MAX_SIPSI_PID; i++) 
   {
      SiPsiTable[ChannelId][i].Pid          = -1;
      SiPsiTable[ChannelId][i].NbSectionsOK = 0;
      SiPsiTable[ChannelId][i].NbSectionsKO = 0;
   }
   DtaNbSiPsiRunning[ChannelId]=0;
#endif
}

/**
 * DtaResetSiPsiCounters
 */
void DtaResetSiPsiCounters(uint32_t ChannelId)
{
#if(DIB_SIPSI_DATA == 1)

   int32_t i;

   for(i = 0; i < MAX_SIPSI_PID; i++) 
   {
      SiPsiTable[ChannelId][i].NbSectionsOK = 0;
      SiPsiTable[ChannelId][i].NbSectionsKO = 0;
   }
#endif
}

/**
 * DtaLogSiPsiStatus
 */
void DtaLogSiPsiStatus(FILE * f)
{
#if(DIB_SIPSI_DATA == 1)
   int32_t i;
   int32_t ChannelId;
   int32_t NbSipsi = 0;

   if(!f) 
   {
      f = stdout;
   }

   for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++) 
   {
      for(i = 0; i < MAX_SIPSI_PID; i++) 
      {
         if(SiPsiTable[ChannelId][i].Pid != -1) 
         {
            NbSipsi++;

            if(ItemArray[SiPsiTable[ChannelId][i].ItemId].Config.SiPsi.Crc)
            {
               fprintf(f, "ChannelId(%d) ItemId(%d) Pid (%d) : SIPSI Sections OK/KO:  %d / %d" CRA,
                              ChannelId, SiPsiTable[ChannelId][i].ItemId, SiPsiTable[ChannelId][i].Pid,
                              (int32_t) SiPsiTable[ChannelId][i].NbSectionsOK,
                              (int32_t) SiPsiTable[ChannelId][i].NbSectionsKO);
            }
            else
            {
               fprintf(f, "ChannelId(%d) ItemId(%d) Pid(%d) : SIPSI Sections : %d" CRA,
                              ChannelId, SiPsiTable[ChannelId][i].ItemId, SiPsiTable[ChannelId][i].Pid,
                              (int32_t) SiPsiTable[ChannelId][i].NbSectionsOK + 
                              (int32_t) SiPsiTable[ChannelId][i].NbSectionsKO);
            }
         }
      }
   }
   if(!NbSipsi) 
   {
      fprintf(f, "  No SI/PSI Filter running" CRA);
   }

   fflush(f);
#endif
}

int32_t DtaPrintBuffer(uint8_t * pBuf, uint32_t Size)
{
   uint32_t i;
   for(i=0; i<Size; i++)
   {
      printf("%02x ",pBuf[i]);
      if((i%16)==15) printf(CRB "" CRA);
   }
   return DIBSTATUS_SUCCESS;
}
/**
 * DtaSiPsiCallback ()
 */
void DtaSiPsiCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
#if(DIB_SIPSI_DATA == 1)
   /* as we have one SIPSI per buffer (we asked DIB_TIMEOUT_IMMEDIATE), we can use pBuffCtx->ItemHdl to retreive the PID */
   /* but we can use several PIDS in the same SIPSI filter */
   DIBSTATUS rc;
   uint32_t  ItemId;
   uint8_t   crc;
   uint32_t  Pid;

   /** If filter is closed without having deregister all the buffers, buffers are returned to the dta with callbacks */
   if(Status == eREMOVED)
   {
      uint32_t FilterId, ChannelId, SipsiIdx, j, brk;

      for(FilterId = 0; FilterId < DIB_MAX_NB_FILTERS; FilterId++)
      {
         if(FilterArray[FilterId].Used && (FilterArray[FilterId].FilterHdl == pBuffCtx->FilterIdx)) 
            break;
      }

      if(FilterId == DIB_MAX_NB_FILTERS)
      {
         printf(CRB "CLBACK error: FilterHdl %d not found" CRA,pBuffCtx->FilterIdx);
      }
      else
      {
         ChannelId = FilterArray[FilterId].ParentChannel;

         if(ChannelId == DIB_MAX_NB_CHANNELS)
         {
            printf(CRB "CLBACK error: FilterHdl %d not found" CRA,pBuffCtx->FilterIdx);
         }
         else
         {
            /** find the buffer in the SIPSI table */
            brk=0;
            for(SipsiIdx=0; SipsiIdx < MAX_SIPSI_PID; SipsiIdx++)
            {
               for(j=0; j<NB_SIPSI_BUFFER; j++)
               {
                  if(SiPsiTable[ChannelId][SipsiIdx].Buffer[j] == pBuffCtx->BufAdd)
                  {
                     brk = 1;
                     break;
                  }
               }
               if(brk) 
                  break;
            }
            if(brk)
            {
               /* printf(CRB "CLBACK buffer %d freed due to filter removal" CRA,SiPsiTable[ChannelId][SipsiIdx].BufferId[j]); */
               NbRegSiPsiBuffers--;

               free(SiPsiTable[ChannelId][SipsiIdx].Buffer[j]);
               SiPsiTable[ChannelId][SipsiIdx].Buffer[j] = NULL;
            }
            else
            {
               printf(CRB "CLBACK error: buffer %p lost" CRA,pBuffCtx->BufAdd);
            }
         }
      }
   }
   /** filter exists in the driver. Has we have one section per buffer, pBuffCtx->ItemHdl is available */
   else
   {
      /* find what is the item id knowing the item hdl */
      for(ItemId=0; ItemId < DIB_MAX_NB_ITEMS; ItemId++)
      {
         if(ItemArray[ItemId].Used && (ItemArray[ItemId].ItemHdl == pBuffCtx->ItemHdl))
            break;
      }

      if(ItemId == DIB_MAX_NB_ITEMS)
      {
         printf(CRB "Sipsi callback error: ItemHdl %d not found" CRA,pBuffCtx->ItemHdl);
      }

      crc = ItemArray[ItemId].Config.SiPsi.Crc;
      Pid = ItemArray[ItemId].Config.SiPsi.Pid;

      if((!DtaComputeCrc32(pBuffCtx->BufAdd, pBuffCtx->BufSize)) && crc)
      {
         printf(CRB "ItemId %d (Pid %d) of FilterId:%d, ###### CRC FALSE ###### size = %d" CRA, ItemId, Pid, pBuffCtx->FilterIdx, pBuffCtx->BufSize);
         DtaPrintBuffer(pBuffCtx->BufAdd, pBuffCtx->BufSize);
         DtaIncSiPsiKoCounter(Pid, ItemId);
      }
      else 
      {
         DtaIncSiPsiOkCounter(Pid, ItemId);
      }

      rc = DibRegisterBuffer(CONTEXT, pBuffCtx->FilterIdx, pBuffCtx->BufAdd, SIPSI_BUFFER_SIZE, pBuffCtx->BufId);

      if(rc != DIBSTATUS_SUCCESS)
      {
         printf(CRB "CLBACK: RegBuf id %d error: rc %d ItemId %d" CRA, pBuffCtx->BufId, rc, ItemId);
      }
   }
#endif
}

/**
 * SiPsiThread ()
 */
#if(DIB_SIPSI_DATA == 1)
static void *SiPsiThread(void *param)
{
   printf(CRB "SIPSI Thread Started" CRA);

   SiPsiThreadRunning = 1;

   /* wait until user stop the thread */
   while(SiPsiThreadRunning) 
   {
      OsMSleep(2000);

      DtaLogSiPsiStatus(stdout);
   }

   printf(CRB "SIPSI Thread Stopped" CRA);

   OsExitThread(0L);

   return (0);
}
#endif

/**
 * DtaReadSiPsiSetionFilter()
 */
void DtaReadSiPsiSection(uint32_t ItemId, uint32_t timeout)
{
#if(DIB_SIPSI_DATA == 1)
   uint32_t ChannelId, SipsiId;
   int32_t Pid;

   if(!ItemArray[ItemId].Used)
   {
      printf(CRB "ItemId not used!!!" CRA);
      return;
   }

   ChannelId = FilterArray[ItemArray[ItemId].ParentFilter].ParentChannel;

   Pid=ItemArray[ItemId].Config.SiPsi.Pid;

   for(SipsiId = 0; SipsiId < MAX_SIPSI_PID; SipsiId++)
   {
      if((SiPsiTable[ChannelId][SipsiId].Pid != -1) && (SiPsiTable[ChannelId][SipsiId].ItemId == ItemId)) break;
   }

   if(SipsiId < MAX_SIPSI_PID)
   {
      /* now wait for results OK/KO */
      OsMSleep(timeout);

      if(ItemArray[SiPsiTable[ChannelId][SipsiId].ItemId].Config.SiPsi.Crc)
      {
         printf(CRB "ChannelId(%d) ItemId(%d) Pid (%d) : SIPSI Sections OK/KO:  %d / %d" CRA,
               ChannelId, SiPsiTable[ChannelId][SipsiId].ItemId, SiPsiTable[ChannelId][SipsiId].Pid,
               (int32_t) SiPsiTable[ChannelId][SipsiId].NbSectionsOK,
               (int32_t) SiPsiTable[ChannelId][SipsiId].NbSectionsKO);
      }
      else
      {
         printf(CRB "ChannelId(%d) ItemId(%d) Pid(%d) : SIPSI Sections : %d" CRA,
               ChannelId, SiPsiTable[ChannelId][SipsiId].ItemId, SiPsiTable[ChannelId][SipsiId].Pid,
               (int32_t) SiPsiTable[ChannelId][SipsiId].NbSectionsOK + 
               (int32_t) SiPsiTable[ChannelId][SipsiId].NbSectionsKO);
      }
   }
   else
   {
      printf(CRB "Item Id not found!!!!!" CRA);
   }
#endif
}
