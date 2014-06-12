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
* @file "DibDriverDragonflyIntDbg.h"
* @brief Dragonfly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DRAGONFLY_INT_DBG_H
#define DIB_DRIVER_DRAGONFLY_INT_DBG_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriver.h"

#if (DIB_INTERNAL_DEBUG == 0)

#define DF_DBG_SPAL                 0  
#define IntDriverDragonflyInit(pContext)
#define IntDriverWaitIfDebugMode(pContext)
#define IntDriverDragonflyProcessMsgIntDbg(pContext, RxCnt, Data)
#define IntDriverDragonflyProcessMsgWave(pContext, RxCnt, Data)
#define IntDriverDragonflyProcessMsgProfile(pContext, RxCnt, Data)
#define IntDriverDragonflyProcessMsgFilePrint(ProfiledFunctionName)
#define IntDriverDragonflyProcessMsgDebugBuf(pContext, rx_nb, Data)

#if (DIB_CMMB_DATA == 1)
   #define IntDriverDragonflyHostReflexInit(pContext, Data);
   #define IntDriverDragonflyHostReflexSetConfig(pContext, Data);
   #define IntDriverDragonflyHostReflexSetSvcParam(pContext, Data);
#endif


#else

#define DF_PROFILE                  0  /* Debug */
#define DF_DBG_PRINT                0
#define DF_WAVE_ANALYZER            0  /* Debug */
#define DF_GDB_STUB                 0  /* Debugguer */
#define DF_DBG_SPAL                 0  /* Spal Debug */

#if (DF_WAVE_ANALYZER == 1)
#include <math.h>
#endif


DIBSTATUS IntDriverDragonflyInit(struct DibDriverContext *pContext);
void      IntDriverWaitIfDebugMode(struct DibDriverContext *pContext);
DIBSTATUS IntDriverDragonflyProcessMsgIntDbg(struct DibDriverContext *pContext, uint32_t RxCnt, uint32_t * Data);
void      IntDriverDragonflyProcessMsgWave(struct DibDriverContext *pContext, uint32_t RxCnt, uint32_t * Data);
void      IntDriverDragonflyProcessMsgProfile(struct DibDriverContext *pContext, uint32_t RxCnt, uint32_t * Data);
void      IntDriverDragonflyProcessMsgFilePrint(uint8_t *ProfiledFunctionName);
void      IntDriverDragonflyProcessMsgDebugBuf(struct DibDriverContext *pContext, uint32_t rx_nb, uint32_t *Data);
DIBSTATUS IntDriverDragonflySubmitBlock(struct DibDriverContext *pContext, struct DibBlock *pBlock);
int   IntDbgProcessMsgPrint(unsigned char id, const char *str);

#if (DIB_CMMB_DATA == 1)
/** Host emulates Reflex */
void IntDriverDragonflyHostReflexInit(struct DibDriverContext *pContext, uint32_t *Data);
void IntDriverDragonflyHostReflexSetConfig(struct DibDriverContext *pContext, uint32_t *Data);
void IntDriverDragonflyHostReflexSetSvcParam(struct DibDriverContext *pContext, uint32_t *Data);

uint32_t IntDriverDragonflyReflexCmmbSMNewTimeSlot(struct DibDriverContext *pContext, struct DibBlock *pBlock);
void     IntDriverDragonflyReflexCmmbSMOutput     (struct DibDriverContext *pContext, struct DibBlock *pBlock);

/*******************************************************************************
* DO NOT CHANGE
* STRUCTURE STORED IN URAM AND SHARED IN BETWEEN FIRMWARE AND HOST
* FIRMWARE ALLOCATES MEMORY
* FIRMWARE ONLY READS
* HOST ONLY WRITES
*******************************************************************************/
struct ReflexCmmbMonitoring
{
   /* Monitoring registers. */
   uint32_t n_stimi_RS_service_id;               /* Parameters array entry.   */
   uint32_t n_stimi_RS_Slot_num;                 /* Time slot number.         */
   uint32_t n_stimi_RS_cfg_AB;                   /* 0:A, 1:B.                 */
   uint32_t n_stimi_RS_base_adr;                 /* 0:BA0, 1:BA1.             */
   uint32_t n_stimi_RS_cpt_Slot;                 /* 0:1 slot, .. 7:8 slots.   */
   uint32_t n_stimi_nb_ldpc_fault;               /* Nb erroneous LDPC blocks. */
   uint32_t n_stimi_RS_error_tab_0;              /* LDPC blocks  0 to 15      */
   uint32_t n_stimi_RS_error_tab_1;              /* LDPC blocks 16 to 31      */
   uint32_t n_stimi_RS_error_tab_2;              /* LDPC blocks 32 to 47      */
   uint32_t n_stimi_RS_error_tab_3;              /* LDPC blocks 48 to 59      */

   /* Additionnal indication information         . */
   uint32_t Byte_interleaver_sync_flag;          /* Continuous pilots bit 6   */
   uint32_t Configuration_changed_flag;          /* Continuous pilots bit 7   */
};

#define N_STIMI_RS_SERVICE_ID       (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_service_id)
#define N_STIMI_RS_SLOT_NUM         (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_Slot_num)
#define N_STIMI_RS_CFG_AB           (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_cfg_AB)
#define N_STIMI_RS_BASE_ADR         (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_base_adr)
#define N_STIMI_RS_CPT_SLOT         (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_cpt_Slot)
#define N_STIMI_NB_LDPC_FAULT       (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_nb_ldpc_fault)
#define N_STIMI_RS_ERROR_TAB_0      (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_error_tab_0)
#define N_STIMI_RS_ERROR_TAB_1      (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_error_tab_1)
#define N_STIMI_RS_ERROR_TAB_2      (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_error_tab_2)
#define N_STIMI_RS_ERROR_TAB_3      (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.n_stimi_RS_error_tab_3)
#define BYTE_INTERLEAVER_SYNC_FLAG  (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.Byte_interleaver_sync_flag)
#define CONFIGURATION_CHANGED_FLAG  (pContext->ChannelDecoder.ReflexCmmbContext.Monitoring.Configuration_changed_flag)
/******************************************************************************/

/**
 * Unit is byte.
 * Size of LDPC decoded data carried by one time slot.
 * [bandwidth][constellation][ldpc_cr]
 */
static const uint32_t CmmbTimeSlotSize[2][3][2] = {
   /* Bandwidth 2 MHz */
   {
      /* BPSK */
      {  1728 ,  2592 },
      /* QPSK */
      {  3456 ,  5184 },
      /* 16-QAM */
      {  6912 , 10368 }
   },
   /* Bandwidth 8 MHz */
   {
      /* BPSK */
      {  8640 , 12960 },
      /* QPSK */
      { 17280 , 25920 },
      /* 16-QAM */
      { 34560 , 51840 }
   }
};

/**
 * Number of time slots RS matrix is mapped into.
 * [bandwidth][constellation][byte_interleaver_mode]
 */
static const uint8_t CmmbRSMatrixNbSlots[2][3][4] = {
   /* Bandwidth 2 MHz */
   {
      /* BPSK */
      { 0 , 5 , 0 , 0 },
      /* QPSK */
      { 0 , 0 , 5 , 0 },
      /* 16-QAM */
      { 0 , 0 , 0 , 5 }
   },
   /* Bandwidth 8 MHz */
   {
      /* BPSK */
      { 0 , 2 , 4 , 8 },
      /* QPSK */
      { 0 , 1 , 2 , 4 },
      /* 16-QAM */
      { 0 , 1 , 1 , 2 }
   }
};

/**
 * Unit is byte.
 * Size of RS matrix.
 * Size if no byte interleaving is fixed to 8640 bytes (cf. time slot zero).
 * [ldpc_cr][bandwidth][byte_interleaver_mode]
 */
static const uint32_t CmmbRSMatrixSize[2][2][4] = {
   /* LDPC Code Rate 1/2 */
   {
      /* Bandwidth 2 MHz */
      {   8640 ,   8640 ,  17280 ,  34560 },
      /* Bandwidth 8 MHz */
      {   8640 ,  17280 ,  34560 ,  69120 }
   },
   /* LDPC Code Rate 3/4 */
   {
      /* Bandwidth 2 MHz */
      {   8640 ,  12960 ,  25920 ,  51840 },
      /* Bandwidth 8 MHz */
      {   8640 ,  25920 ,  51840 , 103680 }
   }
};

/**
* Reflex uses 64 sets of parameters for CMMB support.
*/
struct CmmbParams
{
   /* Multiplex frame identifier and occupation indicator (-1 means free entry). */
   int8_t                        mf_id;

   /* Parameters. */
   int8_t                        intlv_mode;
   int8_t                        ldpc_cr;
   int8_t                        constellation;
   uint32_t                      BaseAddress0;
   uint32_t                      BaseAddress1;

   /* Useful info depending only on parameters. */
   uint8_t                       nb_ts_per_rs_matrix;
   uint32_t                      time_slot_size; /* Bytes. */
   uint32_t                      rs_matrix_size; /* Bytes. */
};

/**
 * Reflex is seen as a state machine.
 * Each set of parameters is assigned some state variables.
 */
struct CmmbParamsState
{
   uint8_t                       CurrentBaseAddress;  /* 0:BA0, 1:BA1. */
   int8_t                        TimeSlotCount;       /* 0:1 slot, ... , 7:8 slots. */
   int8_t                        LatestTimeSlotIndex; /* -1: no time slot yet. */
};

/**
* Reflex context for CMMB support.
*/
struct ReflexCmmbCtx
{
   /* Configuration. */
   uint8_t                       Bandwidth;      /* 0:2 MHz, 1:8 MHz. */
   struct CmmbParams             ParamArray[64];
   struct CmmbParams             IncomingParamArray[64];
   uint8_t                       ConfigA[40];    /* Even frames.              */
   uint8_t                       ConfigB[40];    /* Odd frames.               */

   /* State (state machine description). */
   uint32_t                      GlobalTimeSlotCount; /* Transmitted time slots.   */
   uint8_t                       CurrentConfig;       /* 0:A, 1:B. */
   struct CmmbParamsState        StateArray[64];

   /* Monitoring (state machine output). */
   struct ReflexCmmbMonitoring   Monitoring;
   uint32_t                      MonitAddress;   /* In URAM (provided by FW). */
   uint32_t                      WriteAddress;   /* In URAM (computed by Host). */

   DIB_EVENT                     TimeSlotEvent;
};

/**
* RS engine context for CMMB support.
*/
struct RSEngineCmmbCtx
{
uint8_t empty;
};

/**
* DibDriverChannelDecoder
*/
struct DibDriverChannelDecoder
{
   struct ReflexCmmbCtx          ReflexCmmbContext;
   struct RSEngineCmmbCtx        RSEngineCmmbContext;
};

#endif /* DIB_CMMB_DATA */

#endif /* DIB_INTERNAL_DEBUG */
#endif
