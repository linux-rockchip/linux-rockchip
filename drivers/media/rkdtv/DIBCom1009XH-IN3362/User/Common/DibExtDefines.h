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
* @file "DibExtDefines.h"
* @brief SDK External defines/structures/enum/types.
*
***************************************************************************************************/
#ifndef DIB_EXT_DEFINES_H
#define DIB_EXT_DEFINES_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include <linux/types.h> // by infospace, dckim : Deafult = Disable

/**
 * List of defines and enums common to the dsm api and driver
 */
/* Target specific types definition */
#if BUILD_SDK == -1 // by infospace, dckim : Deafult = 1
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef unsigned short int uint16_t;
typedef signed short int   int16_t;
typedef unsigned int       uint32_t;
typedef signed int         int32_t;
#endif

/* Cofdm spectrum bandwith */
enum DibSpectrumBW
{
   eBW_UNSET          = 0,
   eCOFDM_BW_8_0_MHZ  = 80,   /* 8    MHz */
   eCOFDM_BW_7_0_MHZ  = 70,   /* 7    MHz */
   eCOFDM_BW_6_0_MHZ  = 60,   /* 6    MHz */
   eCOFDM_BW_5_0_MHZ  = 50,   /* 5    MHz */
   eCOFDM_BW_4_5_MHZ  = 45,   /* 4.5  MHz */
   eCOFDM_BW_3_5_MHZ  = 35,   /* 3.5  MHz */
   eCOFDM_BW_3_0_MHZ  = 30,   /* 3.0  MHz */
   eCOFDM_BW_1_75_MHZ = 17,   /* 1.75 MHz */
   eCOFDM_BW_1_5_MHZ  = 15    /* 1.5  MHz */
};

/**
 * Available Board Types, this must match the BoardList Array declared in Board/DibBoardSelection.c
 */
enum DibBoardType
{
  eBOARD_DEFAULT = -1,     /* Take the first configuration in the BoardList Table (declared in Board/DibBoardSelection.c) */
  eBOARD_7078    = 0,      /* 7078 SRAM/SDIO reference design              */
  eBOARD_7070,             /* 7070 SRAM/SDIO reference design              */
  eBOARD_9080,             /* 9080 SRAM/SDIO reference design              */
  eBOARD_7078_STK,         /* STK7078MD USB reference design (diversity)   */
  eBOARD_NIM_9080,         /* NIM9080MD USB reference design (diversity)   */
  eBOARD_7078MD4_STK,      /* STK7078MD4 USB reference design (diversity 4)*/
  eBOARD_9090,             /* 9090 SRAM/SDIO reference design              */
  eBOARD_NIM_9090,         /* NIM9090MD USB reference design (diversity)   */
  eBOARD_19080,            /* 19088 SRAM/SDIO reference design             */
  eBOARD_29098,            /* NIM29098 reference design                    */
  eBOARD_29098D4_STK,      /* STK29098MD4 reference design                 */
  eBOARD_29098_SHA,        /* MOD29088SHA reference design                 */
  eBOARD_NIM_10098v1_0__H1,/* NIM10098v1_0 reference design                */
  eBOARD_NIM_9090H,        /* NIM9090H USB reference design (diversity)    */
  eBOARD_NIM_1009xHx,      /* NIM10098H1/H2--NIM10096H2 Single ref design  */
  eBOARD_NIM_10096MD2,     /* NIM10096M2 Div2 reference design             */
  eBOARD_NIM_10096MD3,     /* NIM10096M2 Div3 reference design             */
  eBOARD_NIM_10096MD4,     /* NIM10096M2 Div4 reference design             */
  eBOARD_NIM_3009xHx,      /* NIM10098H1/H2--NIM10096H2 Single ref design  */
  eBOARD_NIM_30092MD2,     /* NIM10096M2 Div2 reference design             */
  eBOARD_NIM_30092MD3,     /* NIM10096M2 Div3 reference design             */
  eBOARD_NIM_30092MD4,     /* NIM10096M2 Div4 reference design             */
  eBOARD_NIM_3006xM,       /* Board based on DiB3006X */
  eBOARD_NIM_3006xMD2,     /* Board based on DiB3006XMD2 */
  eBOARD_NIM_3006xMD3,     /* Board based on DiB3006XMD3 */
  eBOARD_NIM_3006xMD4,     /* Board based on DiB3006XMD4 */
  eBOARD_NIM_3006xH,       /* Board based on DiB3006XH */
  eBOARD_NIM_1009xAPMD2,    /* Application Board based on Dib1009x         */
  eBOARD_NIM_3009xAPMD2,   /* Application Board based on Dib3009x         */
  eBOARD_NIM_3006xAPM,     /* Application Board based on Dib3006x         */
  eBOARD_NIM_3006xAPMD2,   /* Application Board based on Dib3006x         */
  eBOARD_NIM_3006xAPMD3,   /* Application Board based on Dib3006x         */
  eBOARD_NIM_3006xAPMD4,   /* Application Board based on Dib3006x         */
  eMAX_NB_BOARDS           /* Maximum Number of supported boards           */
};


/*
 * MonitoringInfo structure Type
 * NV: Normalized Value.
 * dB: Value in dB.
 */
struct DibDvbDemodMonit
{
   uint32_t AgcPower;               /* dB = 10*log10(agc_power/2^20) */
   uint32_t AgcGlobal;              /* NV = agc_global/2^16          */
   uint32_t RfTotalGain;

   uint32_t WgnFp;                  /* dB = 10*log10(wgn_fp/2^32)    */
   uint32_t SigFp;                  /* dB = 10*log10(sig_fp/2^32)    */

   uint32_t SysNoiseFp;

   uint32_t CombWgnFp;
   uint32_t CombSigFp;

   uint32_t FecBerRs;               /* DVB-T-H only NV = FecBerRs/10^8 */
   uint16_t FecMpfailCnt;           /* DVB-T-H only */

/* demod locks - different represenation of depending on the standard */
/* DVB-T:
 * [15] AgcLock     | [14] CormLock    | [13] CoffLock      | [12] CoffCpilLock |
 * [11] Lmod4Lock   | [10] Pha3Lock    | [ 9] EqualLock     | [ 8] VitLock      |
 * [ 7] FecLockFrm  | [ 6] FecLockMpeg | [ 5] TpsDecLock    | [ 4] TpsSyncLock  |
 * [ 3] TpsDataLock | [ 2] TpsBchUncor | [ 1] TpsCellIdLock | [ 0] DvsyLock      */

/* DVB-SH
 * [15] Reserved    | [14] CormLock     | [13] CoffLock      | [12] CoffCpilLock |
   [11] Lmod4Lock   | [10] Pha3Lock     | [ 9] EqualLock     | [ 8] SHLock       |
   [ 7] Reserved    | [ 6] Reserved     | [ 5] TpsDecLock    | [ 4] TpsSyncLock  |
   [ 3] TpsDataLock | [ 2] TpsBchUncor  | [ 1] TpsCellIdLock | [ 0] DvsyLock      */

 /*  TDM-B/DAB:
   * [15] Reserved      | [14] xxx          | [13] DabCoffLock   | [12] xxx                 |
   * [11] xxx           | [10] xxx          | [ 9] xxx           | [ 8] VitLock             |
   * [ 7] FecLockFrm    | [ 6] FecLockMpeg  | [ 5] xxx           | [ 4] xxx                 |
   * [ 3] xxx           | [ 2] xxx          | [ 1] xxx           | [ 0] tdint_full_internal |*/
   uint16_t Locks;

   uint16_t VAgcRf;
   uint16_t VAgcBB;

   uint16_t AgcWbd;                 /* NV = AgcWbd/2^12           */
   uint16_t AgcSplitOffset;         /* NV = AgcSplitOffset/2^8    */

   uint8_t  LnaStatus;
   uint8_t  SignalQuality;
   uint8_t  SignalQualityFast;
   uint8_t  Timeout;                /* Partial timeout for this demod. */

   int32_t TimingOffset;
   int32_t FrequencyOffset;

#define DIB_TRANSMITTER_ID_UNAVAILABLE 0xFFFFFFFF
   uint32_t TransmitterId;  /* Transmitter ID - standard dependent: CELL ID in DVB-T/H/SH , 0xffffffff if not available*/
};


/* Get Dvb locks per bit field */

/* Dvb common */
#define DIB_GET_FEC_LOCK_MPEG(Locks)   ((Locks & 0x0040) >>  6)
#define DIB_GET_FEC_LOCK_FRM(Locks)    ((Locks & 0x0080) >>  7)
#define DIB_GET_VIT_LOCK(Locks)        ((Locks & 0x0100) >>  8)

/* Sh specific */
#define DIB_GET_SH_LOCK(Locks)         ((Locks & 0x0100) >>  8)

/* Dvb-t specific */
#define DIB_GET_COFF_LOCK(Locks)       ((Locks & 0x2000) >> 13)
#define DIB_GET_TPS_SYNC_LOCK(Locks)   ((Locks & 0x0010) >>  4)
#define DIB_GET_TPS_DATA_LOCK(Locks)   ((Locks & 0x0008) >>  3)

/* Dab specific */
#define DIB_GET_DAB_LOCK_FULLINTER(Locks)    (Locks&0x0001)
#define DIB_GET_DAB_LOCK_CORMDMB(Locks)      ((Locks & 0x4000) >> 14)
#define DIB_GET_DAB_LOCK_DAB_COFF(Locks)     ((Locks & 0x2000) >> 13)
#define DIB_GET_DAB_LOCK_NDECTMODE(Locks)    ((Locks & 0x1000) >> 12)
#define DIB_GET_DAB_LOCK_NDECFRM(Locks)      ((Locks & 0x0800) >> 11)
#define DIB_GET_DAB_LOCK_VITERBI(Locks)      ((Locks & 0x0100) >> 8)
#define DIB_GET_DAB_LOCK_FECFRM(Locks)       ((Locks & 0x0080) >> 7)
#define DIB_GET_DAB_LOCK_FEC_MPEG(Locks)     ((Locks & 0x0040) >> 6)

#define DIB_GET_DAB_FULLY_LOCKED(Locks)      (DIB_GET_DAB_LOCK_DAB_COFF(Locks) && DIB_GET_DAB_LOCK_VITERBI(Locks) && DIB_GET_DAB_LOCK_FEC_MPEG(Locks))

/* Isdbt specific */
#define DIB_GET_ISDBT_LOCK_DVBSY(Locks)      (Locks&0x0001)
#define DIB_GET_ISDBT_LOCK_MPEG2(Locks)      ((Locks & 0x0020) >>  5)
#define DIB_GET_ISDBT_LOCK_MPEG1(Locks)      ((Locks & 0x0040) >>  6)
#define DIB_GET_ISDBT_LOCK_MPEG0(Locks)      ((Locks & 0x0080) >>  7)
#define DIB_GET_ISDBT_LOCK_VIT2(Locks)       ((Locks & 0x0200) >>  9)
#define DIB_GET_ISDBT_LOCK_VIT1(Locks)       ((Locks & 0x0400) >>  10)
#define DIB_GET_ISDBT_LOCK_VIT0(Locks)       ((Locks & 0x0800) >>  11)
#define DIB_GET_ISDBT_LOCK_COFF(Locks)       ((Locks & 0x2000) >> 13)
#define DIB_GET_ISDBT_LOCK_AGC(Locks)        ((Locks & 0x8000) >> 15)
#define DIB_GET_ISDBT_FLAG_EMERGENCY(Locks)  ((Locks & 0x0002) >> 1)


struct DibDabDemodMonit
{
   struct DibDvbDemodMonit Dvb;
   uint32_t DmbLockFrame;
   uint32_t DmbBer;
};

struct DibIsdbtLayerDemodMonit
{
   uint32_t FecBerRs;               /* Ber = FecBerRs/10^8          */
   uint16_t FecMpfailCnt;           /* PacketErrCount */
   uint8_t  SignalQuality;
   uint8_t  SignalQualityFast;
};

struct DibIsdbtDemodMonit
{
   struct DibDvbDemodMonit Dvb;
   struct DibIsdbtLayerDemodMonit Layer[3];
};

struct DibDvbshDemodMonit
{
   struct DibDvbDemodMonit Dvb;
   uint32_t Per;                    /* has to be divided by 2^15 */
   uint32_t Reserved;               /* Reserved for future Use */
   uint32_t Reserved2;              /* Reserved for future Use */
};

struct DibCmmbDemodMonit
{
   struct DibDvbDemodMonit Dvb;
   uint32_t Per;                    /* has to be divided by 2^15 */
   uint32_t Reserved1;
   uint32_t Reserved2;
};

struct DibCttbDemodMonit
{
   struct DibDvbDemodMonit Dvb;
   uint32_t Per;
   uint32_t LdpcSyndrome;
   uint32_t LdpcSyndromeFirstIter;
   uint8_t  LdpcNbIter;
   uint8_t  LdpcFlags;
};

struct DibAtscDemodMonit
{
   struct DibDvbDemodMonit Dvb;
   uint32_t Per;                    /* has to be divided by 2^15 */
   uint32_t Reserved1;
   uint32_t Reserved2;
};

struct DibAtscMhDemodMonit
{
   struct DibDvbDemodMonit Dvb;
   uint32_t Per;                    /* has to be divided by 2^15 */
   uint32_t Reserved1;
   uint32_t Reserved2;
};
union DibDemodMonit
{
#if (DIB_DVB_STD == 1)
   struct DibDvbDemodMonit   Dvb;
#endif

#if (DIB_DAB_STD == 1)
   struct DibDabDemodMonit   Dab;
#endif

#if (DIB_ISDBT_STD == 1)
   struct DibIsdbtDemodMonit Isdbt;
#endif

#if (DIB_DVBSH_STD == 1)
   struct DibDvbshDemodMonit Dvbsh;
#endif

#if (DIB_CMMB_STD == 1)
   struct DibCmmbDemodMonit Cmmb;
#endif

#if (DIB_CTTB_STD == 1)
   struct DibCttbDemodMonit Cttb;
#endif

#if (DIB_ATSC_STD == 1)
   struct DibAtscDemodMonit Atsc;
#endif

#if (DIB_ATSCMH_STD == 1)
   struct DibAtscMhDemodMonit AtscMh;
#endif
};


/*
 * DibTpsInfo
 */
struct DibTpsInfo
{
   uint16_t TpsDecVar;
   uint8_t  TpsDec;
   uint8_t  TpsSymbolNum;
   uint8_t  TpsFrameNum;
   uint8_t  TpsNqam;
   uint8_t  TpsHrch;
   uint8_t  TpsVitAlpha;
   uint8_t  TpsNfft;
   uint8_t  TpsCodeRateHp;
   uint8_t  TpsCodeRateLp;
   uint8_t  TpsGuard;
   uint8_t  TpsNativeIntlv;
   uint8_t  TpsDvbH;
   uint8_t  TpsTimeSlicingHp;
   uint8_t  TpsTimeSlicingLp;
   uint8_t  TpsMpeFecHp;
   uint8_t  TpsMpeFecLp;
   uint16_t TpsCellId;
   uint8_t  TpsLen;
   uint8_t  TpsReserved;
   uint8_t  TpsDecLock;
   uint8_t  TpsSyncLock;
   uint8_t  TpsDataLock;
   uint8_t  TpsBchUncor;
   uint8_t  TpsCellIdLock;

#if (DIB_DVBSH_STD == 1)
   /* specific dvb-sh */
   uint8_t  TpsDvbSH;
   uint8_t  TpsTcPuntLp;
   uint8_t  TpsTcPuntHp;
   uint8_t  TpsCommonMult;
   uint8_t  TpsNofSlices;
   uint8_t  TpsNofLateTapes;
   uint8_t  SliceDist;
   uint8_t  NonLateTapes;
#endif
};

#endif
