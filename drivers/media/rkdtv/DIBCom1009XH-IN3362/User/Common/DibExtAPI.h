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

/***************************************************************************************************
 * @file "DibExtAPI.h"
 * @brief SDK Internal defines/structures/enum/types.
 *
 ***************************************************************************************************/
#ifndef DIB_EXT_API_H
#define DIB_EXT_API_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include "DibStatus.h"
#include "DibExtDefines.h"
#include "DibDefines.h"

/**
 * SECTION I: defined values
 */

/**
 * SDK Version
 */
#define DIB_SET_VER(maj, min, rev)     (((maj&0xF)<<16) | ((min&0xFF)<<8) | (rev&0xFF))
#define DIB_MAJOR_VER(ver)             ((ver & 0x000F0000) >> 16)
#define DIB_MINOR_VER(ver)             ((ver & 0x0000FF00) >>  8)
#define DIB_REVISION(ver)               (ver & 0x000000FF)

/*
 * Demod
 */
/** Scan status values. */
#define DIB_SCAN_FAILURE               0     /**< Scan Failed. Not able to have tps and locks */
#define DIB_SCAN_SUCCESS               1     /**< Scan success. Tps and locks are available   */
#define DIB_SCAN_TIMEOUT               2     /**< The firmware was not able to scan the channel in the allowed time */
#define DIB_SCAN_ABORTED               3     /**< Scan as been aborted at user's request.     */
#define DIB_SCAN_INVALID_PARAM         4     /**< Scan requested on invalid channel           */
#define DIB_SCAN_PENDING               5     /**< Scan is pending                             */
#define DIB_SCAN_RETUNING              6     /**< Scan is processedby retuning automatically  */

/** Monit status values. */
#define DIB_MONIT_FAILURE              DIB_SCAN_FAILURE        /**< Scanning Failed so monit failed */
#define DIB_MONIT_RELIABLE             DIB_SCAN_SUCCESS        /**< Channel is reliable      */
#define DIB_MONIT_TIMEOUT              DIB_SCAN_TIMEOUT        /**< Channel is not reliable  */
#define DIB_MONIT_ABORTED              DIB_SCAN_ABORTED        /**< Monit as been aborted at user's request.   */
#define DIB_MONIT_INVALID_PARAM        DIB_SCAN_INVALID_PARAM  /**< Monit request failed.    */
#define DIB_MONIT_PARTIAL              5                       /**< One of demod as a timeout, and at least one is reliable */

#define DIB_MAX_BURST_TIME_UNKNOWN 0xffff

#define DIB_DEMOD_AUTO               0x0    /**< Ask the driver to choose the best value number of demod. Depends on the strategy */
#define DIB_DEMOD_MAX               0xFF

#define DIB_MIN_FREQUENCY          50500
#define DIB_MAX_FREQUENCY        3000000


/** Register Buffer minimum allocation size */
#define DIB_TS_PACKET_SIZE           188
#define DIB_SIPSI_SECTION_SIZE      4096
#define DIB_IP_PACKET_SIZE          1522
#define DIB_FM_SECTION_SIZE          256
#define DIB_VIDEO_PACKET_SIZE       1480
#define DIB_FIC_BLOCK_SIZE            32
#define DIB_DAB_PACKET_SIZE           96
#define DIB_PCR_DATA_SIZE              7
#define DIB_CMMB_RSM_SIZE         103680
#define DIB_FIC_PACKET_SIZE           37
/**
 * List of supported features of the system (demod, firmware and driver).
 * Used in field SupportedModes of struct DibGlobalMonitoring.
 */
#define DVB_SUPPORT           0x00000001  /* Support for DVB-T and DVB-H         */
#define ISDBT_SUPPORT         0x00000002  /* Support of ISDBT standard           */
#define DAB_SUPPORT           0x00000004  /* Support of DAB standard             */
#define FM_SUPPORT            0x00000008  /* Support of FM                       */
#define ANALOG_SUPPORT        0x00000010  /* Support for analog TV               */
#define DIVERSITY_SUPPORT     0x00000014  /* Support of diversity                */
#define PVR_SUPPORT           0x00000018  /* Support for Personal Video Recorder */
#define BBM_SUPPORT           0x00000020  /* Baseband Buffer Mode                */

/**
 * Special timeout values used for data callbacks.
 */
#define DIB_TIMEOUT_INFINITE  -1 /**< Infinite timeout. Callback if buffer is full */
#define DIB_TIMEOUT_IMMEDIATE 0  /**< No timeout. Callback if one element is found exactly */

/**
 * Special PID value to disable PID filter
 */
#define DIB_TSALLPIDS               8192     /** Use this value to receive all PIDs in a MPEG Transport stream **/
#define DIB_TSALLPIDS_NONULLPACKET  8193     /** Use this value to receive all PIDs, except NULL packets **/

enum DibWORKMode
{
   eDIB_WORK_SINGLE              =  0,
   eDIB_WORK_DUAL	               =  1,
   eDIB_WORK_AUTO	               = -1
};

/**
 * SECTION II: enum
 */

/** Demodulation FFT size */
enum DibFFTSize
{
   eDIB_FFT_2K		               =  0,
   eDIB_FFT_8K		               =  1,
   eDIB_FFT_4K		               =  2,
   eDIB_FFT_1K		               =  3,
   eDIB_FFT_SINGLE               =  4,
   eDIB_FFT_AUTO	               = -1
};

enum DibSpectrumInv
{
   eDIB_SPECTRUM_NON_INVERSE     =  0,
   eDIB_SPECTRUM_INVERSE         =  1,
   eDIB_SPECTRUM_AUTO		      = -1
};

/** Demodulation Guard interval */
enum DibGuardIntervalSize
{
   eDIB_GUARD_INTERVAL_1_32      =  0,
   eDIB_GUARD_INTERVAL_1_16      =  1,
   eDIB_GUARD_INTERVAL_1_8       =  2,
   eDIB_GUARD_INTERVAL_1_4       =  3,
   eDIB_GUARD_INTERVAL_PN420     =  4,       /* ~1/9 (CTTB) */
   eDIB_GUARD_INTERVAL_PN595     =  5,       /* ~1/6 (CTTB) */
   eDIB_GUARD_INTERVAL_PN945     =  6,       /* ~1/4 (CTTB) */
   eDIB_GUARD_INTERVAL_AUTO      = -1
};

enum DibIntlvNative
{
   eDIB_INTLV_NATIVE_ON          =   1,
   eDIB_INTLV_NATIVE_OFF         =   0,
   eDIB_INTLV_NATIVE_AUTO        =  -1
};

enum DibRSMode
{
   eDIB_RS_240_240              =   0, 
   eDIB_RS_240_224              =   1,
   eDIB_RS_240_192              =   2,
   eDIB_RS_240_176              =   3,
   eDIB_RS_AUTO                 =   -1
};
enum DibScramblingMode
{
   eDIB_SCR_NONE                =   0,
   eDIB_SCR_1                   =   1,
   eDIB_SCR_2                   =   2,
   eDIB_SCR_3                   =   3,
   eDIB_SCR_4                   =   4,
   eDIB_SCR_5                   =   5,
   eDIB_SCR_6                   =   6,
   eDIB_SCR_7                   =   7,
   eDIB_SCR_AUTO                =  -1,
};

enum DibIntlvMode
{
   eDIB_INTLV_NONE               =   0, 
   eDIB_INTLV_MODE1              =   1,
   eDIB_INTLV_MODE2              =   2,
   eDIB_INTLV_MODE3              =   3,
   eDIB_INTLV_AUTO                = -1
};

enum DibVitHrch
{
   eDIB_VIT_HRCH_OFF             =  0,
   eDIB_VIT_HRCH_ON              =  1,
   eDIB_VIT_HRCH_AUTO            = -1
};

enum DibVitPriority
{
   eDIB_VIT_PRIORITY_LP          =  0,
   eDIB_VIT_PRIORITY_HP          =  1,
   eDIB_VIT_PRIORITY_AUTO        = -1
};

enum DibVitAlpha
{
   eDIB_VIT_ALPHA_1              =  1,
   eDIB_VIT_ALPHA_2              =  2,
   eDIB_VIT_ALPHA_4              =  4,
   eDIB_VIT_ALPHA_AUTO           = -1
};

/** Viterbi DVBT/H code rates */
enum DibVitCoderate
{
   eDIB_VIT_CODERATE_1_2         =  1,
   eDIB_VIT_CODERATE_2_3         =  2,
   eDIB_VIT_CODERATE_3_4         =  3,
   eDIB_VIT_CODERATE_5_6         =  5,
   eDIB_VIT_CODERATE_7_8         =  7,
   eDIB_VIT_CODERATE_AUTO        = -1
};

/** Turbocode DVBSH code rates */
enum DibTurboCoderate
{
   eDIB_TURBO_CODERATE_STD_1_5      =  0,
   eDIB_TURBO_CODERATE_STD_2_9      =  1,
   eDIB_TURBO_CODERATE_STD_1_4      =  2,
   eDIB_TURBO_CODERATE_STD_2_7      =  3,
   eDIB_TURBO_CODERATE_STD_1_3      =  4,
   eDIB_TURBO_CODERATE_CPL_1_3      =  5,
   eDIB_TURBO_CODERATE_STD_2_5      =  6,
   eDIB_TURBO_CODERATE_CPL_2_5      =  7,
   eDIB_TURBO_CODERATE_STD_1_2      =  8,
   eDIB_TURBO_CODERATE_CPL_1_2      =  9,
   eDIB_TURBO_CODERATE_STD_2_3      =  10,
   eDIB_TURBO_CODERATE_CPL_2_3      =  11,
   eDIB_TURBO_CODERATE_AUTO         =  -1
};

/** CTTB LDPC coderates */
enum DibLdpcCoderate
{
   eDIB_LDPC_CODERATE_0_4           = 0,
   eDIB_LDPC_CODERATE_0_6           = 1,
   eDIB_LDPC_CODERATE_0_8           = 2,
   eDIB_LDPC_CODERATE_AUTO          = -1
};

/** CTTB Time interleaver */
enum DibCttbTimeInterleaver
{
   eDIB_CTTB_TIME_INTERLEAVER_SHORT    = 0,
   eDIB_CTTB_TIME_INTERLEAVER_LONG     = 1,
   eDIB_CTTB_TIME_INTERLEAVER_AUTO     = -1,
};

enum DibDabTMode
{
   eDIB_DAB_TMODE_I	   =  0,
   eDIB_DAB_TMODE_II   =  1,
   eDIB_DAB_TMODE_III  =  2,
   eDIB_DAB_TMODE_IV   =  3,
   eDIB_DAB_TMODE_AUTO = -1
};

enum DibDabLock
{
   eDIB_DAB_FAST_LOCK            =  0,
   eDIB_DAB_ROBUST_LOCK	         =  1
};

/** Demodulation Constellation Type */
enum DibConstellation
{
   eDIB_QAM_QPSK                 =  0,
   eDIB_QAM_16QAM                =  1,
   eDIB_QAM_32QAM                =  2,
   eDIB_QAM_64QAM                =  3,
   eDIB_QAM_DQPSK                =  4,
   eDIB_QAM_256QAM               =  5,
   eDIB_QAM_VSB_8                =  6,
   eDIB_QAM_VBS_16               =  7,
   eDIB_QAM_BPSK                 =  8,
   eDIB_QAM_QPSK_NR              =  9,
   eDIB_QAM_AUTO                 = -1
};

/** Choice of the most appropriate solution for the number of demodulation between the min and max defined at the creation
 * of the channel, and in accordance of the board specified at the initialisation.
 */
enum DibHbmMode
{
   eBBM_MODE = 0,              /**< Use only BBM */
   eHBM_MODE,                  /**< Use only HBM */
   eBOTH_MODE                  /**< Mode auto */
};

/* WARNING a change in this order requires a firmware upgrade */
enum DibDemodType
{
   eSTANDARD_UNKNOWN = 0,         /**< must be the first one */
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
   eSTANDARD_ATSCMH,
   eMAX_STANDARD_DEMOD,          /**< must be the last one */
   eFAST = 0x80,                  /* exception this is used as a flag */
   eALWAYS_TUNE_SUCCESS = 0x40,   /* exception this is used as a flag */
   eRETUNE_AUTO = 0x20            /* retune automatically */

};

/**
 * Timeslicing configuration for a stream
 */
enum DibPowerMode
{
   ePOWER_ON = 0,
   ePOWER_SAVING,
   ePOWER_AUTO
};


/**
 *  Parameter we can change on the fly
 */
enum DibParameter
{
   ePID_MODE     = 0,  /**< Turn on/off Prefech mode on a PID */
   eTIMESLICING,       /**< Turn on/off Time Slicing */
   eDEBUG_MODE,        /**< Change a debug fonction in the firmware or the sdk */
   eEVENT,             /**< Add or delete an event selection for a specific event callback */
   eSIPSI_MASK,        /**< Set/Uset a 12 bytes filter on a SIPSI Item Element */
   eSET_HBM_MODE,      /**< Change HBM<->BBM mode in firmware */
   eCAS_ENABLE,        /**< Enable/disable CAS system */
   eMAX_NB_PARAMETERS
};

/**
 * If a Pid is activ, the data are retrieved, otherwise it is the prefetch mode
 */
enum DibPidMode
{
   eACTIVE = 0,
   ePREFETCH
};

/**
 * Configuration of the nb rows in the FEC dvbh for a specific pid.
 */
enum DibFecNbrows
{
   eNBROWS_0 = 0,
   eNBROWS_256 = 256,
   eNBROWS_512 = 512,
   eNBROWS_768 = 768,
   eNBROWS_1024 = 1024
};

/**
 * Add or delete an "enum DibEvent" event
 */
enum DibEventMode
{
   eREMOVE_EVENT = 0,
   eADD_EVENT
};

/**
 * Status returned by data callback associated with callback mode filters.
 * 0,1 or more packets can exist in registered buffers. This status indicate
 * the state of the buffer. The state depends on the timeout value affected to the filter.
 */
enum DibDataStatus
{
   eFULL = 0,  /**< The buffer is full (another packet is available in the sdk but we have not enough place here) */
   eREMOVED,   /**< The buffer is empty. This happen when unregistering a filter and registered buffers still exists. */
   eTIMEOUT,   /**< The buffer one at least one packet, but a timeout (not equal to -1) expired before arrival of new packets. */
   eOVERFLOW   /**< The buffer has overflowed, meaning that the packet is too big for that buffer. Therefore, the buffer is truncated. */
};


/**
 * An CMMB service is referred to by one of:
 * 1) 16-bit Service Identifier,
 * 2) 6-bit Multiplex Frame Identifier carrying it possibly with other services.
 */
enum DibCmmbServiceMode
{
   eMF_ID = 0,
   eSVC_ID     = 1,
   eDBG_MF_ID  = 2,  /* Always Forward MF/RSM (except TS0) even if data cannot be corrected */
   eDBG_SVC_ID = 3   /* Always Forward Service (except TS0) even if data cannot be corrected */
};

/*
 * getStream option setting 
 */
#define STREAM_OPTION_SERIAL          (0 << 0)
#define STREAM_OPTION_PARALLEL        (1 << 0)

#define STREAM_OPTION_MPEGTS_188      (0 << 1)
#define STREAM_OPTION_MPEGTS_204      (1 << 1)

#define STREAM_OPTION_MODE_NORMAL      (0 << 2)
#define STREAM_OPTION_MODE_MAIN        (1 << 2)
#define STREAM_OPTION_MODE_BGND        (2 << 2)

#define STREAM_OPTION_SERIAL_DUAL      (0 << 4)
#define STREAM_OPTION_SERIAL_MULTIPLEX (1 << 4)

#define STREAM_OPTION_DELIVER_BAD_TS_PACKET  (0 << 5)
#define STREAM_OPTION_DISCARD_BAD_TS_PACKET  (1 << 5)

#define STREAM_OPTION_LOW_DATA_RATE_OFF      (0 << 6)
#define STREAM_OPTION_LOW_DATA_RATE_ON       (1 << 6)


enum DibStreamCapability {
    eSTREAM_OPTION_MPEG_NO_204      = 1 << 0, /* this stream can't deliver 204-byte TS packets */
    eSTREAM_OPTION_MPEG_NO_188      = 1 << 1, /* this stream can't deliver 188-byte TS packets */
    eSTREAM_OPTION_MPEG_NO_SERIAL   = 1 << 2, /* this stream can't deliver TS packets in SERIAL mode */
    eSTREAM_OPTION_MPEG_NO_PARALLEL = 1 << 3, /* this stream can't deliver TS packets in PARALLEL mode */
};

enum DibPerPeriod {
   ePER_PERIOD_1000_MS = 0,
   ePER_PERIOD_500_MS,
   ePER_PERIOD_250_MS
};

/**
 * SECTION III: callback prototypes
 */
struct DibTuneMonit;
struct DibTotalDemodMonit;
struct DibTuneChan;

typedef void (*DibTuneMonitCallback) (struct DibTuneMonit       *pMonitDescriptor,                     void *pContext);
typedef void (*DibGetSignalCallback) (struct DibTotalDemodMonit *pDemodDescriptor, uint8_t * NbDemod,  void *pContext);
typedef void (*DibGetChannelCallback)(struct DibTuneChan        *pChannelDescriptor,                   void *pContext);
typedef void (*DibGetEventCallback)  (enum DibEvent Event,      union DibEventConfig *pEventConfig,    void *pContext);

typedef void (*DibMessageCallback)  (void * pCallbackCtx, uint16_t MsgType, uint16_t MsgIndex, uint8_t * pMsgBuf, uint32_t MsgLen);


/**
 * SECTION IV: Structures
 */
struct DibDvbTuneChan
{
   enum DibFFTSize                     Nfft;
   enum DibGuardIntervalSize           Guard;
   enum DibConstellation               Nqam;
   enum DibIntlvNative                 IntlvNative;
   enum DibVitHrch                     VitHrch;
   enum DibVitPriority                 VitSelectHp;
   enum DibVitAlpha                    VitAlpha;
   enum DibVitCoderate                 VitCodeRateHp;
   enum DibVitCoderate                 VitCodeRateLp;
   struct DibTpsInfo                   TpsInfo;
};

struct DibDvbSHTuneChan
{
   struct DibDvbTuneChan               Dvb;
   int8_t                              CommonMult;    /* */
   int8_t                              NbLateTaps;    /* */
   int8_t                              NbSlices;      /* */
   int8_t                              SliceDistance; /* */
   int8_t                              NonLateIncr;   /* */
};

struct DibDabTuneChan
{
   enum DibDabTMode                    TMode;
   uint8_t                             NbActiveSub;
};

struct DibIsdbtTuneChan
{
   /* ISDBT Specific */
   uint8_t                             SbMode;               /* 0 = 13seg, 1 = 1 or 3seg */
   uint8_t                             PartialReception;     /* sb_mode=0 -> 0=no, 1=yes, sb_mode=1 -> 0=1seg, 1=3seg */

   enum DibFFTSize                     Nfft;                 /* can be AUTO */
   enum DibGuardIntervalSize           Guard;                /* can be AUTO */

   uint8_t                             SbConnTotalSeg;
   uint8_t                             SbWantedSeg;
   uint8_t                             SbSubchannel;

   struct DibIsdbtLayer
   {
      enum DibConstellation            Constellation;
      enum DibVitCoderate              CodeRate;
      uint8_t                          NbSegments;
      uint8_t                          TimeIntlv;
   } layer[3];
};

struct DibAnalogTuneChan
{
   /* Analog Specific */
   uint32_t                            TunerStandard;
   uint32_t                            CountryCode;
   uint32_t                            Cable;
};

struct DibAtscTuneChan
{
   /* ATSC Specific */
   uint8_t                             Modulation;
};

struct DibCmmbTuneChan
{
   /* CMMB Specific. TODO */
   uint8_t                             Modulation;
};

struct DibCttbTuneChan
{
   enum DibFFTSize                     Nfft;
   enum DibConstellation               Nqam;
   enum DibGuardIntervalSize           Guard;
   enum DibLdpcCoderate                LdpcCodeRate;
   enum DibCttbTimeInterleaver         TimeIntlv;
};

/**
 * DibTuneChan
 */
struct DibTuneChan
{
   CHANNEL_HDL              ChannelHdl;
   uint8_t                  ScanStatus;            /* 0:failure 1:success, 2:timeout */
   uint32_t                 RFkHz;
   enum DibSpectrumBW       Bw;
   enum DibSpectrumInv      InvSpec;
   uint32_t                 StreamParameters;
   enum DibBool             IsDataChannel;

   union
   {
#if (DIB_DVB_STD == 1)
      struct DibDvbTuneChan    Dvb;
#endif

#if (DIB_DVBSH_STD == 1)
      struct DibDvbSHTuneChan  Dvbsh;
#endif

#if (DIB_DAB_STD == 1)
      struct DibDabTuneChan    Dab;
#endif

#if (DIB_ISDBT_STD == 1)
      struct DibIsdbtTuneChan  Isdbt;
#endif

#if (DIB_ANALOG_STD == 1)
      struct DibAnalogTuneChan Analog;
#endif

#if ((DIB_ATSC_STD == 1)||(DIB_ATSCMH_STD == 1))
      struct DibAtscTuneChan   Atsc;
#endif

#if (DIB_CMMB_STD == 1)
      struct DibCmmbTuneChan   Cmmb;
#endif

#if (DIB_CTTB_STD == 1)
      struct DibCttbTuneChan   Cttb;
#endif
   }Std;
};

/**
 * DibTotalDemodMonit
 */
struct DibTotalDemodMonit
{
   uint8_t             MonitStatus;
   uint8_t             NbDemods;
   CHANNEL_HDL         ChannelIndex;
   enum  DibDemodType  Type;
   uint32_t            MonitoringVersion;
   union DibDemodMonit DemodMonit[DIB_MAX_NB_DEMODS];
};

/**
 * DibTuneMonit : structure needed to tune for monitoring
 */
struct DibTuneMonit
{
   struct DibTotalDemodMonit ChannelMonit;         /**< store of the monit */
   struct DibTuneChan        ChannelDescriptor;    /**< parameters of the channel */
};

/**
 * III.2: software related structures
 */
struct DibDvbHMonitoring
{
   /*  DVB-H Monitoring */
   uint16_t Pid;                  /* Mpe Pid                                 */

   /*  Tables */
   uint32_t TotalTables;          /* Total number of MPE-FEC tables counted  */
   uint32_t ErrorTablesBeforeFec; /* Number of ADT received with errors      */
   uint32_t ErrorTablesAfterFec;  /* Number of ADT we cannot correct         */

   /*  Other */
   uint8_t  MpePadding;           /* Mpe padding columns of the last table   */
   uint8_t  MpePuncturing;        /* Mpe puncturing columns of the last table*/
   uint16_t TableSize;            /* Size of the last ADT                    */

   uint16_t DeltaTStream;
   uint16_t DeltaTDetected;
   uint16_t BurstStream;
   uint16_t BurstDetected;
   uint16_t PowerUpTime;

   uint32_t CountinuityError;     /* Number of ADT with continuity break in
                                  IP packets, but with valid data inside  */
   uint32_t Error;                /* Number of ADT with corruption of payload*/

   uint32_t Mfer;                 /* Divide by 10 to get Mfer in %           */

   CHANNEL_HDL ChannelIndex;
   FILTER_HDL  FilterIndex;
};

struct DibDvbTMonitoring
{
   /*  DVB-T Monitoring */
   CHANNEL_HDL ChannelIndex;          /* Monitored TS filter                       */
   FILTER_HDL  FilterIndex;
   uint8_t     NbPid;                 /* Number of active Pids of the filter       */
   uint32_t    TotalPackets;          /* Number of received TS packets             */
   uint32_t    CountinuityError;      /* Number of packets with continuity break,
                                         but with valid data inside                */
   uint32_t    Error;                 /* Number of corrupted packets               */
   uint32_t    CheckTotalPackets;     /* Number of check TS packets                */
   uint32_t    CheckCountinuityError; /* Number of checked packets with cc break,
                                         but with valid data inside                */
   uint32_t    CheckError;            /* Number of checked corrupted packets       */
};

struct DibDvbSHMonitoring
{
   /*  DVB-SH Data Monitoring */
   CHANNEL_HDL ChannelIndex;
   FILTER_HDL  FilterIndex;
   uint16_t    Pid;               /* Service Pid                               */

   /* Service Info */
   uint16_t  NbRows;              /* ADT NbRows                              */
   uint8_t   EncodParal;          /* Encoding parallelization          (B)   */
   uint8_t   SpreadingFactor;     /* Depth of the FEC spreading factor (S)   */
   uint8_t   SendingDelay;        /* Datagram burst sending delay      (D)   */
   uint8_t   NbAdstColumns;       /* Number of ADST column             (C)   */
   uint8_t   MaxIFecSect;         /* Maximum number of MPE IFEC sections
                                     per MPE-IFEC Time-Slice Burst     (R)   */

   /* Burst Info */
   uint16_t DeltaTStream;         /* Inter brust period expected             */
   uint16_t DeltaTDetected;       /* Inter brust period measured             */
   uint16_t BurstStream;          /* Burst duration period expected          */
   uint16_t BurstDetected;        /* Burst duration period measured          */
   uint16_t PowerUpTime;

   /* ADST Tables Info */
   uint32_t AdstTotalTables;    /* Number of ADST tables received            */
   uint32_t AdstTablesLost;     /* Number of ADST tables lost on this service*/
   uint32_t AdstTablesCorrected;/* Number of tables that require correction
                                   (using ADT Tables)                        */
   uint32_t AdstTablesFailed;   /* Number of tables that failed correction   */
   uint32_t AdstTablesGood;     /* Number of ADST fully received without
                                   errors                                    */

   /* ADT Tables Info  */
   uint32_t AdtTotalTables;     /* Number of ADT tables on the selected
                                   service */
   uint32_t AdtTablesCorrected; /* Number of tables that have been corrected */
   uint32_t AdtTablesFailed;    /* Number of tables that failed correction   */
   uint32_t AdtTablesGood;      /* Number of tables which did not require
                                   correction (entire ADT table was
                                   reconstructed without errors)             */

  uint32_t CountinuityError;     /* Number of ADT with continuity break in
                                    IP packets, but with valid data inside  */
  uint32_t Error;                /* Number of ADT with corruption of payload*/
};

struct DibCmmbMonitoring
{
   /* Item identification. */
   CHANNEL_HDL ChannelIndex;
   FILTER_HDL  FilterIndex;
   uint8_t     MfId;       /* Multiplex Frame Identifier. */

   /* LDPC blocks. */
   uint32_t TotalLdpcBlocks;
   uint32_t ErrorLdpcBlocks;

   /* RS matrices. */
   uint32_t TotalRsMatrices;
   uint32_t ErrorRsMatricesBeforeRs;
   uint32_t ErrorRsMatricesAfterRs;

   /* Multiplex Frames. */
   uint32_t TotalMf;
   uint32_t ErrorMfBeforeRs;
   uint32_t ErrorMfAfterRs;

   /* Multiplex payload checker */
   uint32_t ErrorMf;
   uint32_t DiscountMf;

   /* Error rates. */
   uint32_t Bler;          /* Divide by 10 to get Ldpc BLock Error Rate in %. */
   uint32_t Rsmer;         /* Divide by 10 to get RS Matrix Error Rate in %. */
};

struct DibDabMonitoring
{
   /*  DAB Monitoring */
   CHANNEL_HDL ChannelIndex;          /* Monitored filter                         */
   FILTER_HDL  FilterIndex;
   uint8_t     FilterType;
   uint8_t     NbPid;                 /* Number of active Pids of the filter       */
   uint32_t    TotalPackets;          /* Number of received TS packets             */
   uint32_t    CountinuityError;      /* Number of packets with continuity break,
                                         but with valid data inside                */
   uint32_t    CorrPackets;           /* Number of corrected packets (DAB+)        */
   uint32_t    Error;                 /* Number of corrupted packets               */
};

struct DibFicMonitoring
{
   CHANNEL_HDL ChannelIndex;          /* Monitored FIC filter                       */
   FILTER_HDL  FilterIndex;
   uint32_t    CorrectCnt;
   uint32_t    ErrCnt;
};

struct DibDabPacketMonitoring
{
   CHANNEL_HDL ChannelIndex;          /* Monitored FIC filter                       */
   FILTER_HDL  FilterIndex;
   uint32_t    MscPacketGood;
   uint32_t    MscPacketsError;
   uint32_t    DataGroupGood;
   uint32_t    DataGroupError;
};

struct DibAtscMhMonitoring
{
   CHANNEL_HDL ChannelIndex;          /* Monitored FIC filter                       */
   FILTER_HDL  FilterIndex;
};


/**
 * GetDataMonitoring output information.
 */
union DibDataMonit
{
#if (DIB_MPEFEC_DATA == 1)
   struct DibDvbHMonitoring  DvbH;
#endif

#if (DIB_RAWTS_DATA == 1)
   struct DibDvbTMonitoring  DvbT;
#endif

#if (DIB_MPEIFEC_DATA == 1)
   struct DibDvbSHMonitoring DvbSH;
#endif

#if (DIB_CMMB_DATA == 1)
   struct DibCmmbMonitoring Cmmb;
#endif

#if (DIB_DAB_DATA == 1)
   struct DibDabMonitoring Dab;
#endif

#if (DIB_FIG_DATA == 1)
   struct DibFicMonitoring Fic;
#endif

#if (DIB_DABPACKET_DATA == 1)
   struct DibDabPacketMonitoring DabP;
#endif

#if (DIB_ATSCMH_DATA == 1)
   struct DibAtscMhMonitoring AtscMh;
#endif
};

/**  Global monitoring, including a per PID Data monitoring */
struct DibGlobalInfo
{
   uint32_t DriverVersion;
   uint32_t SupportedModes;         /**< Bit field of supported mode.            */
   uint16_t EmbVersions[2];         /**< Version of the embedded firmware        */
   uint16_t NumberOfDemods;         /**< Number of demods connected to the board */
   uint32_t HalVersion;             /**< Version of the HAL-software used in firmware */
   char     HalVersionExt[9];       /**< Version extension of the HAL-software used in firmware */
   uint8_t  MaxMpeServices;         /**< Maximum number of MPE streams to play   */

   uint32_t NumberOfChips;          /**< Number of chipsets */
   uint32_t NumberOfFrontends;      /**< Actual number of frontends of this board */
   FRONTEND_HDL Frontend[DIB_MAX_NB_OF_FRONTENDS];

   uint32_t NumberOfStreams;        /**< Actual number of stream interfaces of this board */
   STREAM_HDL Stream[DIB_MAX_NB_OF_STREAMS];

   uint32_t Reserved1;
   uint32_t Reserved2;
   uint32_t Reserved3;
   uint32_t Reserved4;
   uint32_t Reserved5;
};

/** Interface context */
struct DibDriverContext;

/**
 * Change Embedded or driver debug fonction.
 */
struct DibDebugMode
{
   uint32_t DriverDebugMask;        /**< Enable module traces in the driver */
   uint32_t DHelp;                  /**< DHelp: A set of two parameters: the debugging function (8bits in firefly, 16bits otherwise), and a value (nb bits : idem). ex: dh 11(rpd) 14(snr) */
   uint16_t ChipId;                 /**< ChipId for DHelp, (but not in case of firefly, always 0) */
   uint8_t  EmbeddedDebugMask;      /**< Enable module traces in the embedded firmware */
};

/** A sipsi header is 12 bytes length */
struct DibSipsiHeader
{
   uint8_t Byte[DIB_SIPSI_HEADER_LENGTH];
};

/**
 * The Sipsi Filter Feature allow the user a filter (ie drop) sipsi section whose header doesn't match certain criteria. For each
 * sipsi item, we can add one (and no more) DibSetSipsiFilter. The section is given to the host (ie not filtered) if the 2 conditions are true:
 * - (Sectionheader & BitMaskEqu) == (Value & BitMaskEqu)
 * - (SectionHeader & BitMaskXor) != (Value & BitMaskXor)
 *
 * */
struct DibSipsiFilterCfg
{
   ELEM_HDL                  ItemHdl;          /**< The handler of the element (whose type should be eSIPSI) we want to filter */
   enum DibBool              Enable;           /**< Enable or disable the filter on the item */
   struct DibSipsiHeader     BitMaskEqu;       /**< Used to compare Value and SectionHeader bits to check if they all match. */
   struct DibSipsiHeader     BitMaskXor;       /**< Used to compare Value and SectionHeader bits to check if at least one bit is different */
   struct DibSipsiHeader     Value;            /**< The field values of the SectionHeader we want to have or avoid */
   struct DibSipsiFilterCfg *pNext;            /**< Next sipsi filter if exists */
};


/**
 * Depending of the filter type, configuration of an item
 */
union DibFilters
{
   struct
   {
      uint16_t Pid;                       /**< If Ts, MpeFec, TDMB, eDab or Sipsi */
      uint16_t SubCh;                     /**< If Fig, Dab, DabIp, TDMB. */
   } Common;

#if (DIB_RAWTS_DATA == 1) || (DIB_TDMB_DATA == 1)
   struct _Ts                             /**< For raw TS frames : DVBT case */
   {
      uint16_t Pid;                       /**< PID filtered in the chip, 0x1FFF if all the stream is needed */
   } Ts;
#endif

#if (DIB_PES_DATA == 1)
   struct _PesVideo                       /**< For video PES : DVBT case */
   {
      uint16_t Pid;
   } PesVideo;

   struct _PesAudio                       /**< For audio PES : DVBT case */
   {
      uint16_t Pid;
   } PesAudio;

   struct _PesOther                       /**< For other PES : DVBT case */
   {
      uint16_t Pid;
   } PesOther;

   struct _Pcr                            /**< For PCR information : DVBT case */
   {
      uint16_t Pid;
   } Pcr;
#endif

#if (DIB_MPEFEC_DATA == 1)
   struct _MpeFec                         /**< For MPE frames with or without FEC : DVBH case */
   {
      uint16_t          Pid;                       /**< Pid : a value between 0 and 0x1FFE */
      uint16_t          MaxBurstDuration;          /**< Maximum burst duration, 0xFFFF if unknown */
      enum DibFecNbrows NbRows;                    /**< Nb rows for the FEC decoding, 0 if MPE without FEC, 256, 512, 756 or 1024 otherwise */
      enum DibPidMode   Prefetch;                  /**< Pid prefetch (1) or retrieved (0) */
   } MpeFec;
#endif

#if (DIB_SIPSI_DATA == 1)
   struct _Sipsi                          /**< Service information or Program specific information sections for DVBT, DVBH or TDMB streams */
   {
      uint16_t          Pid;                       /**< Si/Psi pid : a value between 0 and 0x1FFE that defines this si/psi*/
      enum DibBool      PidWatch;                  /**< indicates if the sipsi are retrieved during a MPE burst (1) or not (0) */
      enum DibBool      Crc;                       /**< indicates if this sipsi section includes a crc (1) or not (0) */
   } SiPsi;
#endif

#if (DIB_FIG_DATA == 1)
   struct _Fig                            /**< FIC frames : information for DAB based demodulations */
   {
      uint16_t Reserved;
   } Fig;
#endif

#if (DIB_TDMB_DATA == 1)
   struct _Tdmb                           /**< TDMB : MPEG2 TS frames on a DAB (Stream) demodulation */
   {
      uint16_t SubCh;                     /**< Index of Subchannel wished : from 0 to 63, as described in the associated FIC */
   } Tdmb;
#endif

#if (DIB_EDAB_DATA == 1)
   struct _eDab                           /**< eDAB : MPE frames on a DAB demodulation */
   {
      uint16_t Pid;                       /**< Pid concerned, with a filtre (value between 0 to 0x1FFE) */
      uint16_t SubCh;                     /**< Subchannel concerned (value between 0 and 63) */
   } eDab;
#endif

#if (DIB_DAB_DATA == 1)
   struct _Dab                            /**< Dab stream audio transmission */
   {
      uint16_t Type;                      /**< There are two types of DAB : DAB or DAB+ */
      uint16_t SubCh;                     /**< Subchannel concerned (value between 0 and 63) */
      uint8_t  UseFec;                    /**< Enable DAB+ RS (only if type is DAB+ */
   } Dab;
#endif

#if (DIB_DABPACKET_DATA == 1)
   struct _DabPacket                       /**< DAB-Packet : DAB (Packets) demodulation */
   {
      uint16_t Address;                    /**< address of the service in the subchannel */
      uint16_t SubCh;                      /**< Subchannel concerned (value between 0 and 63) */
      uint8_t  UseFec;                     /**< Fec mode packet */
      uint8_t  UseDataGroup;               /**< Mode DataGroup */
   } DabPacket;
#endif

#if (DIB_MPEIFEC_DATA == 1)
   struct _MpeIFec
   {
      uint16_t           Pid;             /**< Pid : a value between 0 and 0x1FFE    */
      uint16_t           MaxBurstDuration;/**< Max burst duration, 0xFFFF if unknown */
      enum DibFecNbrows  NbRows;          /**< Nb rows for the IFEC decoding, 256,
                                               512, 756 or 1024                      */
      enum DibPidMode    Prefetch;        /**< Pid prefetch (1) or retrieved (0)     */

      /* Service Info */
      uint8_t        EncodParal;          /**< Encoding parallelization          (B) */
      uint8_t        SpreadingFactor;     /**< Depth of the FEC spreading factor (S) */
      uint8_t        SendingDelay;        /**< Datagram burst sending delay      (D) */
      uint8_t        NbAdstColumns;       /**< Number of ADST column             (C) */
      uint8_t        MaxIFecSect;         /**< Maximum number of MPE IFEC sections
                                               per MPE-IFEC Time-Slice Burst     (R) */
   } MpeIFec;
#endif

#if (DIB_CMMB_DATA == 1)
   /* CMMB Control Information Table (time slot zero). */
   struct _CmmbCit
   {
      uint32_t TableId;
   } CmmbCit;

   /* CMMB Audio/Video/Data Service (except XPE and XPE-FEC). */
   struct _CmmbSvc
   {
      uint16_t                MfId;  /**< Multiplex Frame Id (6 bit)             */
      uint16_t                SvcId; /**< Service Id (16 bit)                    */
      enum DibCmmbServiceMode Mode;  /**< Multiplex Frame Id (0), Service Id (1) */
   } CmmbSvc;
#endif

#if (DIB_ATSCMH_DATA == 1)
   struct _AtscmhEns
   {
      uint8_t Mode;
      uint16_t Id;
   } AtscmhEns;
#endif
};

/**
 * Depending of the standard, configuration of a channel
 */
struct DibChannel
{
   enum   DibDemodType Type;                 /**< Standard of demodulation of a channel */
   struct DibTuneChan  ChannelDescriptor;    /**< Demodulator parameters of the demodulation, depending of the standard */
};

struct DibPidModeCfg
{
   ELEM_HDL          ElemHdl;
   enum DibPidMode   Mode;
};

struct DibEventModeCfg
{
   DibGetEventCallback  Callback;
   enum DibEvent        Event;
   enum DibEventMode    Mode;
};

struct DibTimeSlicing
{
   enum DibBool                  Enable;
   STREAM_HDL                    PhysicalStream;
};

struct DibCasEnable
{
   enum DibBool               Enable;
};

/**
 * DibParamConfig
 * Parameter to change and configuration
 */
union DibParamConfig
{
   struct   DibPidModeCfg     PidMode;          /**< Turn on/off prefetch mode on one PID */
   struct   DibDebugMode      DebugMode;        /**< Change a debug fonction in the sdk of in the firmware */
   struct   DibEventModeCfg   EventMode;        /**< Add or remove an event to/from event callback function notification list */
   struct   DibSipsiFilterCfg SipsiFilter;      /**< Set/Uset a filter on Sipsi Section header (12 bytes length) */
   struct   DibTimeSlicing    TimeSlicing;      /**< Turn on/off time slicing for power management */
   enum     DibHbmMode        HbmMode;          /**< Use Hbm | BBM mode in firmware */
   struct   DibCasEnable      CasEnable;        /**< Enable/disable CAS system */
};


struct DibBufContext
{
   uint32_t   BufSize;
   uint32_t   FillSize;    /* Internal Use */
   FILTER_HDL FilterIdx;
   ELEM_HDL   ItemHdl;
   uint8_t    BufId;
   uint8_t   *BufAdd;
};

/**
* Defines the data mode : IP or Callback, and in the second case the function, time out value and context needed
*/
struct DibDataBuffer
{
   enum DibDataMode DataMode;
   void            *CallbackUserCtx;
   void           (*CallbackFunc)(void *, struct DibBufContext *, enum DibDataStatus Status);
   uint32_t        Timeout;
};

/** Structure describing the Attributes of a DibStream entity: A
 * Stream entity is representing physical connection via the the
 * demodulated and decoded payload is delivered */

enum DibStreamTypes {

   eSTREAM_TYPE_HOST_DMA = 0, /* DO NOT CHANGE */
   eSTREAM_TYPE_MPEG_1,       /* MPEG bus with CLK1 */
   eSTREAM_TYPE_MPEG_2,       /* MPEG bus with CLK2 */
   eSTREAM_TYPE_CHIP_DMA,
   eSTREAM_TYPE_TX,
   eSTREAM_TYPE_TX_1,
};

struct DibFrontendPossibleInput
{
	FRONTEND_HDL Fe;
	uint32_t     Standard;

};

struct DibStreamAttributes
{
   enum DibStreamTypes Type;
   uint32_t ChipId;

   uint32_t Options; /* bit mask of DibStreamCapability */
   uint32_t Standards;

   struct DibFrontendPossibleInput PossibleDirectInputs[DIB_MAX_NB_OF_FRONTENDS];
};

/** Structure describing the attributes of a DibFrontend entity: A
 * Frontend entity is a representing a unit which tunes and
 * demodulates a signal.  When connected to a DibStream the
 * demodulated data is decoded and error-corrected.
 */
struct DibFrontendAttributes
{
   uint32_t ChipId;
   uint32_t FeId;

   uint32_t Standards;

   struct DibFrontendPossibleInput PossibleDirectInputs[DIB_MAX_NB_OF_FRONTENDS];
};

struct DibInfoRequest
{
   uint32_t Type;
   uint32_t ChipId;
};

struct DibInfoEfuse
{
   uint8_t Status;
   uint8_t IsRead;
};

struct DibInfoSdram
{
   uint8_t Status;
   uint8_t State;
   uint8_t IsUsedForCaching;
};

struct DibInfoGpio
{
   uint8_t Num;
   uint8_t Val;
   uint16_t Ratios[16];
   uint8_t Pwm[16];
   uint8_t Div;
   uint8_t Dir;
   uint8_t Pwm_mode;
};

struct DibInfoGpioConfig
{
   uint8_t            Status;
   struct DibInfoGpio Config;
};

struct DibInfoEfuseTuner
{
   uint8_t Status;
   uint8_t Level;
};


struct DibInfoTunerIdentity
{
   uint8_t Status;
   uint8_t Version;
};

struct DibInfoToggleMpeg
{
   uint8_t Status;
   uint8_t IsToggled;
};

struct DibInfoPerPeriodConfig
{
   uint8_t Status;
   enum DibPerPeriod Period;
};

struct DibInfoPerPeriod
{
   enum DibPerPeriod Period;
};

struct DibInfoData
{
   struct DibInfoEfuse        Efuse;
   struct DibInfoSdram        Sdram;
   struct DibInfoGpioConfig   Gpio;
   struct DibInfoEfuseTuner      EfuseTuner;
   struct DibInfoTunerIdentity   TunerIdentity;
   struct DibInfoToggleMpeg      ToggleMpeg;
   struct DibInfoPerPeriodConfig PerPeriod;
};

struct DibInfoSet
{
   uint32_t Type;
   uint32_t ChipId;

   union
   {
      struct DibInfoGpio  Gpio;
      struct DibInfoPerPeriod    PerPeriod;
   } Param;
};

/**
* SECTION V: functions
*/
#ifdef __cplusplus
extern "C"
{
#endif

/**
* DibOpen
*/
DIBSTATUS DibOpen(struct DibDriverContext **pContextAddr, enum DibBoardType BoardType, BOARD_HDL BoardHdl);

/**
* DibClose
*/
DIBSTATUS DibClose(struct DibDriverContext *pContext);

/**
* DibGetChannel
*/
DIBSTATUS DibGetChannel(struct  DibDriverContext     *pContext,
                                uint8_t               MinNbDemod,
                                uint8_t               MaxNbDemod,
                                uint32_t              StreamParameters,
                                /* if (StreamParameters[0:15] == eSTREAM_TYPE_MPEG_[01])
                                 *      -> StreamParameters[16]: 0 => Serial, 1 => Parallel
                                 *      -> StreamParameters[17]: 0 => 188,    1 => 204
                                 *      -> StreamParameters[18]: 0 => gated,  1 => continous
                                 * if (StreamParameters[0:15] == eSTREAM_TYPE_HOST_DMA (== 0))
                                 *      0
                                 */
                        struct  DibChannel           *pDescriptor,
                                CHANNEL_HDL          *pChannelHdl,
                                DibGetChannelCallback pCallBack,
                                void                 *pCallCtx);

/**
* DibDeleteChannel
*/
DIBSTATUS DibDeleteChannel(struct DibDriverContext *pContext,
                                  CHANNEL_HDL       ChannelHdl);

/**
* DibCreateFilter
*/
DIBSTATUS DibCreateFilter(struct DibDriverContext  *pContext,
                                 CHANNEL_HDL        ChannelHdl,
                          enum   DibDataType        DataType,
                          struct DibDataBuffer     *pDataBuf,
                                 FILTER_HDL        *pFilterHdl);

/**
* DibDeleteFilter
*/
DIBSTATUS DibDeleteFilter(struct DibDriverContext *pContext,
                                 FILTER_HDL        FilterHdl);

/**
* DibAddToFilter
*/
DIBSTATUS DibAddToFilter(struct DibDriverContext *pContext,
                                FILTER_HDL        FilterHdl,
                                uint32_t          NbElements,
                         union  DibFilters       *pFilterDescriptor,
                                ELEM_HDL         *pElemHdl);

/**
* DibRemoveFromFilter
*/
DIBSTATUS DibRemoveFromFilter(struct DibDriverContext *pContext,
                                     ELEM_HDL          ElemHdl);

/**
* DibGetSignalMonitoring
*/
DIBSTATUS DibGetSignalMonitoring(struct DibDriverContext    *pContext,
                                        CHANNEL_HDL          ChannelHdl,
                                        uint8_t             *NbDemod,
                                 union  DibDemodMonit       *pMonit,
                                        DibGetSignalCallback pCallBack,
                                        void                *pCallCtx);

/**
* DibGetGlobalMonitoring
*/
DIBSTATUS DibGetGlobalInfo(struct DibDriverContext *pContext,
                           struct DibGlobalInfo    *pGlobalInfo);

/**
* DibAbortTuneMonitChannel
*/
DIBSTATUS DibAbortTuneMonitChannel(struct DibDriverContext *pContext);

/**
* DibTuneMonitChannel
*/
DIBSTATUS DibTuneMonitChannel(struct DibDriverContext    *pContext,
                                     uint8_t              MinNbDemod,
                                     uint8_t              MaxNbDemod,
                                     uint32_t             StreamParameters,
                                     /* if (StreamParameters[0:15] == eSTREAM_TYPE_MPEG_[01])
                                      *      -> StreamParameters[16]: 0 => Serial, 1 => Parallel
                                      *      -> StreamParameters[17]: 0 => 188,    1 => 204
                                      *      -> StreamParameters[18]: 0 => gated,  1 => continous
                                      * if (StreamParameters[0:15] == eSTREAM_TYPE_HOST_DMA (== 0))
                                      *      0
                                      */
                               enum  DibDemodType         Type,
                              struct DibTuneMonit        *pMonitDescriptor,
                                     DibTuneMonitCallback pCallBack,
                                     void                *pCallCtx);

/**
* DibGetChannelDescriptor
*/
DIBSTATUS DibGetChannelDescriptor(struct DibDriverContext *pContext,
                                         CHANNEL_HDL       ChannelHdl,
                                  struct DibTuneChan      *pDescriptor);

/**
* DibRegisterBuffer
*/
DIBSTATUS DibRegisterBuffer(struct DibDriverContext *pContext,
                                   FILTER_HDL        FilterIdx,
                                   uint8_t          *BufAdd,
                                   uint32_t          BufSize,
                                   uint8_t           BufId);
/**
* DibUnregisterBuffer
*/
DIBSTATUS DibUnregisterBuffer(struct DibDriverContext *pContext,
                                     FILTER_HDL        FilterIdx,
                                     uint8_t          *BufAdd);

/**
* DibGetDataMonitoring
*/
DIBSTATUS DibGetDataMonitoring(struct DibDriverContext   *pContext,
                                      ELEM_HDL            ElemHdl,
                               union  DibDataMonit       *pMnt,
                               enum   DibBool             ClearData);

/**
* DibSetConfig
*/
DIBSTATUS DibSetConfig(struct DibDriverContext *pContext,
                         enum DibParameter      Param,
                        union DibParamConfig   *pParamConfig);

/**
* DibGetConfig
*/
DIBSTATUS DibGetConfig(struct DibDriverContext *pContext,
                         enum DibParameter      Param,
                        union DibParamConfig   *pParamConfig);


/**
 * DibRegisterEventCallback
 */
DIBSTATUS DibRegisterEventCallback(struct DibDriverContext    *pContext,
                                          DibGetEventCallback  pCallback,
                                          void                *pCallCtx,
                                          uint8_t              NbEvents,
                                     enum DibEvent            *pEvent);

/**
 * DibUnregisterEventCallback
 */
DIBSTATUS DibUnregisterEventCallback(struct DibDriverContext    *pContext,
                                            DibGetEventCallback  pCallback);


/**
* DibSendMessage
*/
DIBSTATUS DibSendMessage(struct  DibDriverContext *pContext,
                                 uint16_t          MsgType,
                                 uint16_t          MsgIndex,
                                 uint8_t          *pSendBuf,
                                 uint32_t          SendLen,
                                 uint8_t          *pRecvBuf,
                                 uint32_t         *pRecvLen);

/**
* DibRegisterMessageCallback
*/
DIBSTATUS DibRegisterMessageCallback(struct  DibDriverContext    *pContext,
                                             uint16_t             MsgType,
                                             uint8_t             *pMsgBuf,
                                             uint32_t             MsgBufLen,
                                             DibMessageCallback   pCallback,
                                             void                *pCallbackCtx);

DIBSTATUS DibUnregisterMessageCallback(struct DibDriverContext * pContext,
							 uint16_t MsgType);


/** Stream and Frontend API */
DIBSTATUS DibGetStreamAttributes(
        struct DibDriverContext *pContext,
        STREAM_HDL                  Stream,
        struct DibStreamAttributes *Attributes
);

DIBSTATUS DibGetFrontendAttributes(
        struct DibDriverContext *pContext,
        FRONTEND_HDL Fe,
        struct DibFrontendAttributes *Attributes
);

DIBSTATUS DibGetStream(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream,
        enum DibDemodType        Standard,
        uint32_t                 OutputOptions,

        /* if (Type == eSTREAM_TYPE_MPEG_[01])
         *      -> OutputOptions bit 0: 0 => Serial, 1 => Parallel
         *      -> OutputOptions bit 1: 0 => 188,    1 => 204
         * if (Type == eSTREAM_TYPE_HOST_DMA)
         *      0
         */

        enum DibPowerMode        EnPowerSaving);

DIBSTATUS DibAddFrontend(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream,
        FRONTEND_HDL             Fe,
        FRONTEND_HDL             FeOutput, /**< NULL - automatic mode, parameter is unused for now - will be used in some cases in future releases */
        uint32_t                 Force     /**< Force remove of Frontend if already in use - calls implicitly DibRemoveFrontend() */
);

DIBSTATUS DibGetChannelEx(
        struct DibDriverContext *pContext,
        STREAM_HDL               Hdl,
        struct  DibChannel      *pDescriptor,
        CHANNEL_HDL             *pChannelHdl,
        DibGetChannelCallback    pCallBack,
        void                    *pCallCtx
);

/**
* DibTuneMonitChannelEx
*/
DIBSTATUS DibTuneMonitChannelEx(struct DibDriverContext    *pContext,
                                STREAM_HDL                  Hdl,
                                struct DibTuneMonit        *pMonitDescriptor,
                                DibTuneMonitCallback        pCallBack,
                                void                       *pCallCtx);


DIBSTATUS DibGetFrontendMonit(
        struct DibDriverContext *pContext,
        FRONTEND_HDL             Fe,
        union DibDemodMonit     *Monit
);

/** removes frontend instance from stream */
DIBSTATUS DibRemoveFrontend(
        struct DibDriverContext *pContext,
        FRONTEND_HDL             Fe
);

/** Releases all resources allocated to this stream (Frontends and Channels) */
DIBSTATUS DibDeleteStream(
        struct DibDriverContext *pContext,
        STREAM_HDL               Stream
);

/** Get info */
DIBSTATUS DibGetInfo(
        struct DibDriverContext *pContext,
        struct DibInfoRequest   *pInfoRequest,
        struct DibInfoData      *pInfoData
);

/** Set info */
DIBSTATUS DibSetInfo(
        struct DibDriverContext *pContext,
        struct DibInfoSet       *pInfoSet,
        uint8_t                 *pStatus
);


#ifdef __cplusplus
}
#endif
#endif                          /* DIB_EXT_API_H */
