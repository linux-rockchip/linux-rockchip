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
* @file "DibDriverFirefly.h"
* @brief Firefly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_FIREFLY_H
#define DIB_DRIVER_FIREFLY_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriver.h"

/* Specific Build Option */
#define SLOW_MICROCODE_UPLOAD    0


/* General Firefly Defines */
#define FF_HBM_SLICE_LINES			32
#define FF_HBM_MAX_FRAGMENTS		16

#define FF_MASK_RISCA            0x01
#define FF_MASK_RISCB            0x10
#define FF_ALL_RISC              0x11

#define FF_RAWTS_SVC             8

void      DibDriverFireflyRegisterChip(struct DibDriverContext *pContext);
DIBSTATUS DibDriverFireflySetCfgSvc(struct DibDriverContext *pContext, uint8_t tsch, uint16_t ReqType, uint16_t Pid, uint16_t SvcNb, uint8_t IsWatch, uint16_t Format, enum DibPidMode Prefetch);


enum DibPidTargets 
{
   eTARGET_MPE,
   eTARGET_SIPSI,
   eTARGET_SIPSI_NO_CRC,
   eTARGET_RAW_TS,
   eTARGET_PES_VIDEO,
   eTARGET_PES_AUDIO,
   eTARGET_PES_OTHER,
   eTARGET_PCR
};

struct DibAddPidInfo
{
   /* A PID can be selected to either be MPE Data or SI/PSI */
   enum DibPidTargets  PidTarget;

   uint16_t Pid;
   uint8_t  TsChannelNumber;

   union _Specific 
   {
#if (DIB_MPEFEC_DATA == 1)
      struct _MPESpecific 
      {
         /* For MPE SERVICES */
         uint32_t          NbRows;
         uint32_t          MaxBurstDuration;
         uint8_t           HasFec;
         uint8_t           OnlyIfPrioritaryChannel;    /* do we really want this service ? */
         enum DibPidMode   Prefetch;
      } MpeSpecific;
#endif

#if (DIB_SIPSI_DATA == 1)
      struct _SIPSISpecific 
      {
         uint8_t  pidWatch;           /* 0 for full time, 1 for pidWatch */
      } SiPsiSpecific;
#endif
   } Specific;
};

struct DibDelPidInfo
{
  uint16_t Pid;
  uint8_t  TsChannelNumber;
};

#endif
