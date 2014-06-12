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
* @file "DibDefines.h"
* @brief SDK Internal defines/structures/enum/types.
*
***************************************************************************************************/
#ifndef DIB_DEFINES_H
#define DIB_DEFINES_H

/* Debug print end of line system  */
#define CRB ""         /* CR printed BEFORE debug output */
#define CRA "\n"       /* CR printed AFTER debug output */

/**
 * Si/Psi Section header size
 */
#define DIB_SIPSI_HEADER_LENGTH       12  /**< Sipsi section have 12 bytes on it */

/**
 * List of defines and enums common to the bridge and driver
 */
typedef void *  BOARD_HDL;

#define FRONTEND_HDL_EMPTY NULL
typedef void *  FRONTEND_HDL;

typedef void *  STREAM_HDL;
typedef uint8_t CHANNEL_HDL;
typedef uint8_t FILTER_HDL;
typedef uint8_t ELEM_HDL;

enum DibDataType
{
   eTS,                          /* Raw TS Data                */
   eSIPSI,                       /* SI/PSI Sections            */
   eMPEFEC,                      /* MPE Data (with/without FEC)*/
   eFM,                          /* FM AUdio Data              */
   eANALOG,                      /* Analog Audio/Video         */
   eFIG,                         /* FIG Signalisation Data     */
   eTDMB,                        /* TDMB Video Data            */
   eDAB,                         /* DAB Audio Data             */
   eDABPACKET,                   /* DAB Mode Packet            */
   eEDAB,                        /* eDAB Data                  */
   ePESVIDEO,                    /* video PES                  */
   ePESAUDIO,                    /* audio PES                  */
   ePESOTHER,                    /* other PES                  */
   ePCR,                         /* PCR                        */
   eMPEIFEC,                     /* MPE Data (with iFEC)       */
   eCMMBMFS,                     /* CMMB All Multiplex Frames  */
   eCMMBCIT,                     /* CMMB Control Info. Tables  */
   eCMMBSVC,                     /* CMMB Service               */
   eCMMBXPE,                     /* CMMB XPE (XPE-FEC or not)  */
   eATSCMHIP,                    /* ATSCMH IP                  */
   eATSCMHFIC,                   /* ATSCMH FIC                 */
   eUNKNOWN_DATA_TYPE = 0xFF
};

enum DibEvent
{
   eDATA_LOST = 0,
   eSIPSI_LOST,
   eBURST_LOST,         /**< Entire burst was lost (DVB-H/SH) */
   eSIPSI_FILTERED,     /**< Sipsi section filtered by driver */
   eBURST_FAILED,       /**< Part of the burst was received, but could not be corrected (DVB-H/SH)             */
   eDATA_DETECTED,      /**< Signalled on first valid Data Forwarded after burst lost or burst failed (DVB-H/SH)
                             or tune (DVB-T/H/SH) */
   eMAX_NB_EVENTS       /**< Must be the last one */
};

enum DibBool
{
   eDIB_FALSE = 0,
   eDIB_TRUE  = 1
};

/* to complete */
struct DibDataLostEventConfig
{
   ELEM_HDL ItemHdl;
};

struct DibSipsiLostEventConfig
{
   ELEM_HDL ItemHdl;
};

struct DibBurstLostEventConfig
{
   ELEM_HDL ItemHdl;
};

struct DibSipsiFilterEventConfig
{
   ELEM_HDL ItemHdl;
};

union DibEventConfig
{
   struct DibDataLostEventConfig    DataLostEvent;
   struct DibSipsiLostEventConfig   SipsiLostEvent;
   struct DibBurstLostEventConfig   BurstLostEvent;
   struct DibSipsiFilterEventConfig SipsiFilterEvent;
};

enum DibBridgeInfoType
{
   eSET_SERVICE = 0          /**< Request the bridge to set/unset a service */
};

struct DibSetService
{
   uint8_t          Svc;
   uint16_t         Pid;
   ELEM_HDL         ItemHdl;
   FILTER_HDL       FilterHdl;
   uint32_t         StreamId;
   enum DibDataType DataType;
   enum DibDataMode DataMode;
};

union DibInformBridge
{
   struct DibSetService SetService;
};

enum DibInfoType
{
   eINFO_EFUSE,                        /* efuse info                */
   eINFO_SDRAM,                        /* sdram info                */
   eINFO_GPIO,                         /* gpio info                 */
};


enum DibInfoSdramState
{
   eINFO_SDRAM_NOTINIT,       /* sdram not initialized yet, presence unknown*/
   eINFO_SDRAM_PRESENT,       /* sdram present but directory not read or unvalid */
   eINFO_SDRAM_HD_VALID,      /* sdram present and directory valid */
   eINFO_SDRAM_NOT_PRESENT,   /* sdram initialization failed */
   eINFO_SDRAM_UNKNOW,        /* sdram state unknow */
};


#define DIB_INFO_MASK_EFUSE      (1 << eINFO_EFUSE)
#define DIB_INFO_MASK_SDRAM      (1 << eINFO_SDRAM)
#define DIB_INFO_MASK_GPIO       (1 << eINFO_GPIO)

/**
 * Configuration of all the different sizes
 */
#define DIB_MAX_NB_OF_CHIPS                  4  /**< Maximum number of chipsets for a board */
#define DIB_MAX_NB_OF_STREAMS                8  /**< Maximum number of stream interface on one instance of a board */
#define DIB_MAX_NB_OF_FRONTENDS              4  /**< Maximum number of frontends on one instance of a board */
#define DIB_MAX_NB_CHANNELS                  6  /**< Maximum number of channels in the system */
#define DIB_MAX_NB_FILTERS                  16  /**< Maximum number of filters in the system */
#define DIB_MAX_NB_ITEMS                    32  /**< Maximum number of items in the system */
#define DIB_MAX_NB_BUF_CONTEXT              64  /**< Maximum number of registerable buffers in the system */

#define DIB_MAX_NB_DEMODS                    DIB_MAX_NB_OF_FRONTENDS
#define DIB_NB_PERSISTENT_TS_CHANNELS        3

#define DIB_MAX_NB_SERVICES                 18

/*  How many PES service at the same time ? */
#define DIB_MAX_PES_SERVICES                 1

/*  How many PCR service at the same time ? */
#define DIB_MAX_PCR_SERVICES                 1

/*  How many MPE services at the same time ? */
#define DIB_MAX_MPE_SERVICES                 8

/* Either Data (0:1) or SiPsi-only (2) or Monit Channel (3). */
#define DIB_MAX_NB_TS_CHANNELS               4

/**
 * Reset value
 */
#define DIB_UNSET                         0xFF  /**< Item handler, Filter handler or Firefly service not used */


#endif
