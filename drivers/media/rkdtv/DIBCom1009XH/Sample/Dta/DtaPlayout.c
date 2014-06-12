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

/******************************************************************************
* @file "DtaPlayout.c"
* @brief DiBcom Test Application - Playout stream directly to chipset memory
* for physical layer agnostic software development.
*******************************************************************************/

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaCrc32.h"
#include "DtaCmds.h"
#include "DtaPlayout.h"
#include "DtaParsing.h"
#include "../../Driver/Common/DibDriverIntDbg.h"

#if (DIB_CMMB_STD == 1) && (DIB_CMMB_DATA == 1) && (DIB_INTERNAL_DEBUG == 1)

extern struct DibDriverContext *gUserContext;

#define CONTEXT (gUserContext)

/* ---------------------------- PLAYOUT CONTEXT ----------------------------- */
static THREAD_HANDLE PlayoutThreadHandle;
static void *PlayoutThread(void *param);
static volatile int32_t PlayoutThreadRunning = 0;
static volatile int32_t gPlayoutFile = -1;
static volatile uint8_t gPlayoutStatus = 0;  /* 0:stop, 1:running */
static volatile uint8_t gPlayoutCorrupt = 0; /* 0:no corruption, >0:standard specific corruption. */

/* ------------------------------ CMMB SUPPORT ------------------------------ */
/* Record RS matrices if requested. */
struct RsMatrix
{
   uint8_t  Used;
   uint32_t Size;  /* min = 8640 bytes, max = 103680 bytes. */
   uint8_t  *Data;
   uint8_t  RsK;   /* nb payload columns (240 , 224 , 192 , 176). */
   uint32_t Crc32;
};
#define NB_RECORD_BUFFERS 4
struct RsMatrix gRecord[NB_RECORD_BUFFERS] = { {0} };

/* Multiplex Descriptor. */
struct TimeSlot
{
   uint8_t  TimeSlotIndex;        /* 0-39 */
   uint8_t  ByteIntlvSync;        /* 0-1 */
   uint8_t  EndOfRsMatrix;        /* 0-1 */
   uint8_t  ConfigChanged;        /* 0-1 */
   uint8_t  NbTotalLdpcBlocks;    /* 3, 6, 12, 15, 30, 60 */
   uint16_t NbTotalBytes;         /* min = 1728 bytes, max = 51840 bytes */
   uint8_t  NbFaultyLdpcBlocks;
   uint16_t ErrLdpcBlockPosit[4]; /* Bitmask for position of erroneous blocks */
   uint32_t ReadOffset;           /* Position in payload data file */
   uint32_t WriteOffset;          /* Position in record buffer if need be */
   struct RsMatrix *Record;       /* RS matrix to record if need be */
};
static struct TimeSlot gMultiplex[40*60] = { {0} };
static uint32_t gNbPhyFrames = 0;
static uint32_t gNextTimeSlotIndex = 0;

/* ----------------------------- LOCAL FUNCTIONS ---------------------------- */
static struct RsMatrix *AllocRecordBuffer(uint32_t Count, uint8_t NbPaylCol, uint8_t *Handle)
{
   /* Return pointer. */
   struct RsMatrix *pRsMatrix;

   /* Get free buffer. */
   uint8_t i;
   for(i = 0; i < NB_RECORD_BUFFERS; i++)
      if(gRecord[i].Used == 0)
         break;
   if(i == NB_RECORD_BUFFERS)
     return 0;
   pRsMatrix = &gRecord[i];

   /* Allocate memory. */
   pRsMatrix->Data = malloc((size_t)Count);
   if(pRsMatrix->Data == 0)
      return 0;

   /* Proceed. */
   if(Handle != 0)
      *Handle = i;
   pRsMatrix->Used = 1;
   pRsMatrix->Size = Count;
   pRsMatrix->RsK = NbPaylCol;
   pRsMatrix->Crc32 = 0;
   return pRsMatrix;
}

static void FreeRecordBuffer(struct RsMatrix *Buffer)
{
   /* Lookup buffer. */
   uint8_t i;
   for(i = 0; i < NB_RECORD_BUFFERS; i++)
      if(&gRecord[i] == Buffer)
         break;
   if(i == NB_RECORD_BUFFERS)
      return;

   /* Proceed. */
   Buffer->Used = 0;
   Buffer->Size = 0;
   if(Buffer->Data != 0)
   {
      free(Buffer->Data);
      Buffer->Data = 0;
   }
   Buffer->RsK = 0;
   Buffer->Crc32 = 0;
}

static void ResetAllRecordBuffers(void)
{
   uint8_t i;
   for(i = 0; i < NB_RECORD_BUFFERS; i++)
   {
      gRecord[i].Used = 0;
      gRecord[i].Size = 0;
      if(gRecord[i].Data != 0)
      {
         free(gRecord[i].Data);
         gRecord[i].Data = 0;
      }
      gRecord[i].RsK = 0;
      gRecord[i].Crc32 = 0;
   }
}

/* --------------------------- EXPORTED FUNCTIONS --------------------------- */

/**
 * DtaPlayoutStart
 */
int32_t DtaPlayoutStart(void)
{
   int32_t status = 0;

   if(PlayoutThreadRunning)
      return -1;

   /* Playout thread polling and processing commands. */
   status = OsCreateThread(&PlayoutThreadHandle, (PTHREAD_FN)PlayoutThread, NULL);

   if(status == 0)
   {
      while(PlayoutThreadRunning == 0)
      {
         OsMSleep(1);
      }

      printf(CRB "PLAYOUT Thread created, status=%d" CRA, status);
   }
   else
   {
      printf(CRB "PLAYOUT Thread create FAILED!!!!, status=%d" CRA, status);
   }

   return status;
}

/**
 * DtaPlayoutStop
 */
int32_t DtaPlayoutStop(void)
{
   int32_t status;

   if(!PlayoutThreadRunning)
   {
      printf(CRB "DtaStopPlayout FAILED" CRA);
      return -1;
   }

   PlayoutThreadRunning = 0;

   status = OsJoinThread(PlayoutThreadHandle);

   return status;
}

/**
 * PlayoutThread
 */
static void *PlayoutThread(void *param)
{
   printf(CRB "PLAYOUT Thread Started" CRA);

   PlayoutThreadRunning = 1;

   /* Wait until user stop the thread. */
   while(PlayoutThreadRunning) 
   {
      if(gPlayoutStatus == 1)
         DtaPlayoutStep();
      OsMSleep(20);
   }

   printf(CRB "PLAYOUT Thread Stopped" CRA);

   OsExitThread(0L);

   return 0;
}

/**
 * DtaPlayoutOpen
 */
int32_t DtaPlayoutOpen(char *NamePayld, char *NameDescr, uint8_t NbPhyFrames)
{
   /* Open payload data file. */
   int32_t status = open(NamePayld, O_RDONLY);
   if(status == -1)
   {
      printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, NamePayld);
      goto PlayoutOpenError;
   }

   /* Open description file. */
   FILE *fdesc = fopen(NameDescr, "r");
   if(fdesc == 0)
   {
      printf(CRB "%s: *** Error: Cannot open file %s" CRA, __FUNCTION__, NameDescr);
      goto PlayoutOpenError;
   }

   /* Read description file. TODO Support not only CMMB. */
   gMultiplex[0].ReadOffset = 0;
   uint32_t i;
   for(i = 0; i < NbPhyFrames * 40; i++)
   {
      /* Read one line of description file (one time slot). */
      if(fscanf(fdesc, "%d %d %d %d %d %d %d 0x%04x 0x%04x 0x%04x 0x%04x",
            (int*)(&gMultiplex[i].TimeSlotIndex),                /* time slot index */
            (int*)(&gMultiplex[i].ByteIntlvSync),                /* byte interleaver synchro flag */
            (int*)(&gMultiplex[i].EndOfRsMatrix),                /* end of RS matrix flag */
            (int*)(&gMultiplex[i].ConfigChanged),                /* configuration changed flag */
            (int*)(&gMultiplex[i].NbTotalLdpcBlocks),            /* nb ldpc info blocks */
            (int*)(&gMultiplex[i].NbTotalBytes),                 /* size in bytes after ldpc decoding */
            (int*)(&gMultiplex[i].NbFaultyLdpcBlocks),           /* nb faulty ldpc info blocks */
            (int*)(&gMultiplex[i].ErrLdpcBlockPosit[0]),         /* position of faulty blocks ( 0-15) */
            (int*)(&gMultiplex[i].ErrLdpcBlockPosit[1]),         /* position of faulty blocks (16-31) */
            (int*)(&gMultiplex[i].ErrLdpcBlockPosit[2]),         /* position of faulty blocks (32-47) */
            (int*)(&gMultiplex[i].ErrLdpcBlockPosit[3])) == EOF) /* position of faulty blocks (48-59) */
      {
         printf(CRB "%s: *** Error: End of description file reached (%s)" CRA, __FUNCTION__, NameDescr);
         goto PlayoutOpenError;
      }

      /* Saves off position of time slot in payload data file. */
      if(i < NbPhyFrames * 40 - 1)
         gMultiplex[i+1].ReadOffset = gMultiplex[i].ReadOffset + gMultiplex[i].NbTotalBytes;

      /* By default do not record time slot. */
      gMultiplex[i].WriteOffset = 0;
      gMultiplex[i].Record = 0;
   }
   fclose(fdesc);

   /* Reset record buffers. */
   ResetAllRecordBuffers();

   /* Saves off number of physical frames. */
   gNbPhyFrames = NbPhyFrames;

   /* Next time slot is zero. */
   gNextTimeSlotIndex = 0;

   /* Saves off payload data file descriptor. */
   gPlayoutFile = status;

   /* Default is stop. */
   gPlayoutStatus = 0;

   /* Default is no corruption. */
   gPlayoutCorrupt = 0;

   return status;

PlayoutOpenError:
   if(status != -1)
      close(status);
   if(fdesc != 0)
      fclose(fdesc);
   return -1;
}

/**
 * DtaPlayoutClose
 */
int32_t DtaPlayoutClose(void)
{
   /* Sanity check. */
   int32_t status = -1;
   if(gPlayoutFile == -1)
      return status;

   /* Stop thread before closing payload data file. */
   if(PlayoutThreadRunning == 1)
   {
      printf(CRB "%s: *** Error: Cannot close payload data file while PLAYOUT thread is running" CRA, __FUNCTION__);
      return status;
   }

   status = close(gPlayoutFile);
   gPlayoutFile = -1;
   gPlayoutStatus = 0;
   gPlayoutCorrupt = 0;

   return status;
}

void DtaSwap32(uint8_t * buf, int32_t Size)
{
   uint8_t reg;
   Size -= 4;

   while(Size >= 0)
   {
      reg         = buf[Size];
      buf[Size]   = buf[Size+3];
      buf[Size+3] = reg;
      reg         = buf[Size+1];
      buf[Size+1] = buf[Size+2];
      buf[Size+2] = reg;

      Size -= 4;
   }
}

/**
 * DtaPlayoutStep
 */
int32_t DtaPlayoutStep(void)
{
   /* Sanity check. */
   int32_t status = -1;
   struct TimeSlot *pTimeSlot;
   uint8_t *Data;
   ssize_t sz;
   uint32_t pldsz;
   uint8_t row, col;
   off_t of;
   /* Sample static volatile corruption number. */
   uint8_t NbLdpcToCorrupt;
   uint8_t Bytes[14];
   struct DibBlock Block;
   /* Corrupt payload and modify local description bytes accordingly if need be. */
   uint8_t  nb_tot_ldpc_blk;
   uint16_t nb_tot_bytes;
   uint8_t  nb_err_ldpc_blk;
   uint16_t err_ldpc_blk_pos[4];
   uint16_t ldpc_blk_size;
   uint8_t i;
   uint16_t j;
   uint8_t idx;
   uint8_t ofs;
   uint16_t msk;

   if(gPlayoutFile == -1)
      return status;

   /* Get pointer to next time slot to submit. */
   pTimeSlot = &gMultiplex[gNextTimeSlotIndex];

   /* Allocate memory for payload. */
   Data = (uint8_t*)malloc((size_t)(pTimeSlot->NbTotalBytes));
   if(!Data)
   {
      printf(CRB "%s: *** Error: Cannot allocate memory" CRA, __FUNCTION__);
      return status;
   }

   /* Set offset of payload data file to desired time slot. */
   of = lseek(gPlayoutFile, (off_t)(pTimeSlot->ReadOffset), SEEK_SET);
   if(of != (off_t)(pTimeSlot->ReadOffset))
   {
      printf(CRB "%s: *** Error: Cannot reposition offset (%ld)" CRA, __FUNCTION__, of);
      free(Data);
      return status;
   }

   if(pTimeSlot->TimeSlotIndex == 0)
   {
      static int frame=0;
      /* printf(CRB "#################### FRAME %3d ##################\n",frame); */
      frame++;
   }
   /* Manage empty slots */
   if(pTimeSlot->NbTotalBytes == 0) 
   {
      /* Update next time slot index. */
      /* printf(CRB "-------------- TS %d -------------------" CRA, pTimeSlot->TimeSlotIndex); */
      gNextTimeSlotIndex++;
      if(gNextTimeSlotIndex == 40 * gNbPhyFrames)
         gNextTimeSlotIndex = 0;

      return 0;
   }
   else
   {
      /* printf(CRB "++++++++++++++ TS %d +++++++++++++++++++" CRA, pTimeSlot->TimeSlotIndex); */
   }

   /* Get payload. */
   sz = read(gPlayoutFile, (void*)Data, (size_t)(pTimeSlot->NbTotalBytes));
   if(sz != (ssize_t)(pTimeSlot->NbTotalBytes))
   {
      printf(CRB "%s: *** Error: Cannot read enough data (%ld)" CRA, __FUNCTION__, sz);
      free(Data);
      return status;
   }

   /* swap data to emulate reflex endianness workaround */
   /* DtaSwap32(Data, pTimeSlot->NbTotalBytes); */

   /* Record payload if requested. */
   pldsz = 0;
   if((pTimeSlot->Record != 0) && (pTimeSlot->Record->Data != 0))
   {
      /* Memory copy. */
      memcpy(pTimeSlot->Record->Data + pTimeSlot->WriteOffset, Data, (size_t)(pTimeSlot->NbTotalBytes));

      /* Compute crc32 for payload if last time slot of RS matrix. */
      if(pTimeSlot->EndOfRsMatrix == 1)
      {
         pldsz = pTimeSlot->Record->Size * pTimeSlot->Record->RsK / 240;
         pTimeSlot->Record->Crc32 = Crc32Compute(pTimeSlot->Record->Data, pldsz);
         printf(CRB "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
         printf(CRB "RS MATRIX FORWARDED (%d payload bytes): CRC32 = 0x%08X" CRA, pldsz, pTimeSlot->Record->Crc32);
         printf(CRB "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
         for(row = 0; row < 4; row++)
         {
            for(col = 0; col < 64; col++)
               printf("%02x ", pTimeSlot->Record->Data[col * 288 + row]);
            printf("\n");
            printf(" ... | %02x %02x\n",
                  pTimeSlot->Record->Data[pTimeSlot->Record->RsK * 288 + row],
                  pTimeSlot->Record->Data[(pTimeSlot->Record->RsK + 1) * 288 + row]);
         }
      }
   }

   /* Sample static volatile corruption number. */
   NbLdpcToCorrupt = gPlayoutCorrupt;

   /* Corrupt payload and modify local description bytes accordingly if need be. */
   nb_tot_ldpc_blk     = pTimeSlot->NbTotalLdpcBlocks;
   nb_tot_bytes        = pTimeSlot->NbTotalBytes;
   nb_err_ldpc_blk     = pTimeSlot->NbFaultyLdpcBlocks;
   err_ldpc_blk_pos[0] = pTimeSlot->ErrLdpcBlockPosit[0];
   err_ldpc_blk_pos[1] = pTimeSlot->ErrLdpcBlockPosit[1];
   err_ldpc_blk_pos[2] = pTimeSlot->ErrLdpcBlockPosit[2];
   err_ldpc_blk_pos[3] = pTimeSlot->ErrLdpcBlockPosit[3];
   ldpc_blk_size       = nb_tot_bytes / nb_tot_ldpc_blk;
   idx = 0;
   if(NbLdpcToCorrupt != 0)
   {
      /* Loop over LDPC info blocks to corrupt. */
      for(i = 0; i < NbLdpcToCorrupt; i++)
      {
         /* Pick random LDPC block index. */
         idx = (uint8_t)(OsRandom() % nb_tot_ldpc_blk);

         /* Get offset and mask. */
         ofs = (idx >> 4) & 0x3;
         msk = (uint16_t)((1 << (idx - (idx & 0x30))) & 0xffff);

         /* Skip if LDPC block already erroneous. */
         if((err_ldpc_blk_pos[ofs] & msk) != 0)
            continue;

         /* Fill-in LDPC block with random bytes. */
         for(j = 0; j < ldpc_blk_size; j++)
            Data[idx * ldpc_blk_size + j] = /*0x47*/(uint8_t)(OsRandom() & 0xff);

         /* Mark position of faulty block and increment count. */
         err_ldpc_blk_pos[ofs] |= msk;
         nb_err_ldpc_blk++;
      }
   }

   /* Set description bytes. */
   Bytes[0]  = 0;                               /* CMMB standard identifier */
   Bytes[1]  = pTimeSlot->TimeSlotIndex;        /* time slot index */
   Bytes[2]  = pTimeSlot->ByteIntlvSync;        /* byte interleaver synchro flag */
   Bytes[3]  = pTimeSlot->ConfigChanged;        /* configuration changed flag */
   Bytes[4]  = pTimeSlot->NbTotalLdpcBlocks;    /* nb ldpc info blocks */
   Bytes[5]  = nb_err_ldpc_blk;                 /* nb faulty ldpc info blocks */
   Bytes[6]  = err_ldpc_blk_pos[0] & 0xff;      /* position of faulty blocks ( 0- 7) */
   Bytes[7]  = (err_ldpc_blk_pos[0]>>8) & 0xff; /* position of faulty blocks ( 8-15) */
   Bytes[8]  = err_ldpc_blk_pos[1] & 0xff;      /* position of faulty blocks (16-23) */
   Bytes[9]  = (err_ldpc_blk_pos[1]>>8) & 0xff; /* position of faulty blocks (24-31) */
   Bytes[10] = err_ldpc_blk_pos[2] & 0xff;      /* position of faulty blocks (32-39) */
   Bytes[11] = (err_ldpc_blk_pos[2]>>8) & 0xff; /* position of faulty blocks (40-47) */
   Bytes[12] = err_ldpc_blk_pos[3] & 0xff;      /* position of faulty blocks (48-55) */
   Bytes[13] = (err_ldpc_blk_pos[3]>>8) & 0xff; /* position of faulty blocks (56-59) */

   /* Fill-in block structure. */
   Block.Data        = Data;
   Block.Size        = pTimeSlot->NbTotalBytes;
   Block.DescBytes   = Bytes;
   Block.NbDescBytes = 14;

   /* Submit block to driver. */
   status = DibDriverSubmitBlock(CONTEXT, &Block);

   /* Free memory. */
   free(Data);

   /* Update next time slot index. */
   gNextTimeSlotIndex++;
   if(gNextTimeSlotIndex == 40 * gNbPhyFrames)
      gNextTimeSlotIndex = 0;

   return status;
}

/**
 * DtaPlayoutCorrupt
 */
int32_t DtaPlayoutCorrupt(uint8_t Corruption)
{
   /* Sanity check. */
   if(gPlayoutFile == -1)
   {
      printf(CRB "%s: Cannot corrupt data: No file opened" CRA, __FUNCTION__);
      return -1;
   }

   /* Standard specific data corruption number. */
   gPlayoutCorrupt = Corruption;

   return (int32_t)Corruption;
}

/**
 * DtaPlayoutRun
 */
int32_t DtaPlayoutRun(void)
{
   /* Sanity checks. */
   if(gPlayoutFile == -1)
   {
     printf(CRB "%s: Cannot run playout: No file opened" CRA, __FUNCTION__);
     return -1;
   }
   if(PlayoutThreadRunning == 0)
   {
     printf(CRB "%s: Cannot run playout: No PLAYOUT thread running" CRA, __FUNCTION__);
     return -1;
   }

   /* Enable data submission to driver in thread. */
   gPlayoutStatus = 1;

   return 0;
}

/**
 * DtaPlayoutPause
 */
int32_t DtaPlayoutPause(void)
{
   /* Sanity checks. */
   if(gPlayoutFile == -1)
   {
     printf(CRB "%s: Cannot pause playout: No file opened" CRA, __FUNCTION__);
     return -1;
   }
   if(PlayoutThreadRunning == 0)
   {
     printf(CRB "%s: Cannot pause playout: No PLAYOUT thread running" CRA, __FUNCTION__);
     return -1;
   }

   /* Disable data submission to driver in thread. */
   gPlayoutStatus = 0;

   return 0;
}

/**
 * DtaPlayoutRewind
 */
int32_t DtaPlayoutRewind(void)
{
   /* Sanity checks. */
   if(gPlayoutFile == -1)
   {
     printf(CRB "%s: Cannot rewind playout: No file opened" CRA, __FUNCTION__);
     return -1;
   }
   if(gPlayoutStatus == 1)
   {
     printf(CRB "%s: Cannot rewind playout: Playout is running" CRA, __FUNCTION__);
     return -1;
   }

   gNextTimeSlotIndex = 0;

   return 0;
}

/**
 * DtaPlayoutRecord
 * Descriptor[0] = NbBytes     : Descriptor size in bytes (that byte included).
 * Descriptor[1 ... NbBytes-2] : Indices of time slots to record (must form an RS matrix).
 * Descriptor[NbBytes-1]       : Number of payload columns of RS matrix.
 */
int32_t DtaPlayoutRecord(uint8_t *Descriptor)
{
   /* Sanity checks. */
   if(Descriptor == 0)
      return -1;
   if(gPlayoutFile == -1)
   {
      printf(CRB "%s: *** Error: Cannot record anything: No file opened" CRA, __FUNCTION__);
      return -1;
   }

   /* Parse descriptor. */
   uint8_t NbBytes = Descriptor[0];

   /* Check descriptor integrity. Compute also RS matrix size. */
   uint32_t Count = 0;
   uint8_t i;
   for(i = 1; i < NbBytes-1; i++)
   {
      if(Descriptor[i] > 39)
      {
         printf(CRB "%s: *** Error: Illegal time slot index (%d)" CRA, __FUNCTION__, Descriptor[i]);
         return -1;
      }
      Count += gMultiplex[Descriptor[i]].NbTotalBytes;
   }

   /* Check that first and last time slots to record match with an RS matrix bounds. */
   if((gMultiplex[Descriptor[1]].ByteIntlvSync == 0) ||
      (gMultiplex[Descriptor[NbBytes-2]].EndOfRsMatrix == 0))
   {
      printf(CRB "%s: *** Error: First and last time slot to record must match an RS matrix bounds" CRA, __FUNCTION__);
      return -1;
   }

   /* Get a free buffer. */
   uint8_t Handle = 0;
   struct RsMatrix *pRsMatrix = AllocRecordBuffer(Count, Descriptor[NbBytes-1], &Handle);
   if(pRsMatrix == 0)
   {
      printf(CRB "%s: *** Error: Record buffer allocation failure" CRA, __FUNCTION__);
      return -1;
   }

   /* Everything ok to proceed. Assign record buffer to time slots. */
   Count = 0;
   uint8_t TimeSlotIndex;
   uint8_t j;
   for(i = 1; i < NbBytes-1; i++)
   {
      /* Index of time slot to record. */
      TimeSlotIndex = Descriptor[i];
      printf("Record Time Slot %d\n", TimeSlotIndex);

      /* Loop over phy frames. */
      for(j = 0; j < gNbPhyFrames; j++)
      {
         /* Assign buffer to time slot. */
         gMultiplex[j * 40 + TimeSlotIndex].Record = pRsMatrix;

         /* Set write address. */
         gMultiplex[j * 40 + TimeSlotIndex].WriteOffset = Count;

         printf("Assign Buffer %p to Entry %d at Offset %d\n", pRsMatrix, j * 40 + TimeSlotIndex, Count);
      }

      /* Update RS matrix size. */
      Count += gMultiplex[TimeSlotIndex].NbTotalBytes;
   }

   return (int32_t)Handle;
}

/**
 * DtaPlayoutErase
 */
int32_t DtaPlayoutErase(uint8_t Handle)
{
   /* Sanity check. */
   if(Handle >= NB_RECORD_BUFFERS)
      return -1;

   /* Detach buffer from time slots. */
   struct RsMatrix *pRsMatrix = &gRecord[Handle];
   uint8_t i;
   for(i = 0; i < 40 * gNbPhyFrames; i++)
   {
      if(gMultiplex[i].Record == pRsMatrix)
      {
         gMultiplex[i].WriteOffset = 0;
         gMultiplex[i].Record = 0;
      }
   }

   /* Proceed. */
   FreeRecordBuffer(pRsMatrix);

   return 0;
}

#endif
