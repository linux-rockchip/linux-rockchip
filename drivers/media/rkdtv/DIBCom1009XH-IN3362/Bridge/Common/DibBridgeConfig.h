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
* @file "DibBridgeConfig.h"
* @brief Bridge functionality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_CONFIG_H
#define DIB_BRIDGE_CONFIG_H


/*************************************************************/
/*** IC SELECTION                                         ****/
/*************************************************************/
/* If only one type of IC is used then remove the other to reduce
   code size */
#define     USE_FIREFLY   0      /* DIB7070 / DIB7078 / DIB9080 / DIB9090 */ // by infospace, dckim : default = 1
#define     USE_DRAGONFLY 1      /* DIB19088 / DIB29098 */
#define     mSDK       0
//#define DRIVER_AND_BRIDGE_MERGED	1 // by dckim, infospace
/*************************************************************/
/*************************************************************/

#define     ENG3_COMPATIBILITY  1

/*************************************************************/
/*** PARAMETER CHECKING                                   ****/
/*************************************************************/
/* If full application has been validated this option can be
   removed and therfore reduce code size and optimise speed */
#define     DIB_PARAM_CHECK   1

/*************************************************************/
/*** STANDARD SELECTION                                   ****/
/*************************************************************/
/* Be careful changing the standard will also change the size
   of the structures passed to the SDK API - be sure they are in
   sync */
/* Demodulation standard */
#if (mSDK == 0)
#define     DIB_DVB_STD      1   /* DVB-T/DVB-H          */
#define     DIB_DVBSH_STD    1   /* DVB-SH               */
#define     DIB_DAB_STD      1   /* DAB/TDMB/DABPacket/EDAB  */
#define     DIB_ISDBT_STD    1   /* Not yet supported    */
#define     DIB_ANALOG_STD   0   /* Not yet supported    */
#define     DIB_ATSC_STD     1   /* ATSC                 */
#define     DIB_FM_STD       0   /* Not yet supported    */
#define     DIB_CMMB_STD     1   /* CMMB                 */
#define     DIB_CTTB_STD     1   /* CTTB                 */
#define     DIB_ATSCMH_STD   1   /* ATSC                 */
#else
#define     DIB_DVB_STD      1   /* DVB-T/DVB-H          */
#define     DIB_DVBSH_STD    0   /* DVB-SH               */
#define     DIB_DAB_STD      0   /* DAB/TDMB/DABPacket/EDAB  */
#define     DIB_ISDBT_STD    0   /* Not yet supported    */
#define     DIB_ANALOG_STD   0   /* Not yet supported    */
#define     DIB_ATSC_STD     1   /* Not yet supported    */
#define     DIB_FM_STD       0   /* Not yet supported    */
#define     DIB_CTTB_STD     0   /* CTTB                 */
#define     DIB_CMMB_STD     0   /* CMMB                 */
#define     DIB_ATSCMH_STD   1   /* Not yet supported    */
#endif


/*************************************************************/
/*** DATA TYPE SELECTION                                  ****/
/*************************************************************/
/* Be careful changing the standard will also change the size
   of the structures passed to the SDK API - be sure they are in
   sync */
/* Data Type */
#if (mSDK == 0)
#define     DIB_RAWTS_DATA      1   /* DVB-T / DVB-H / DVB-SH / TDMB / ISDB-T */
#define     DIB_PES_DATA        1   /* DVB-T                                  */
#define     DIB_SIPSI_DATA      1   /* DVB-T / DVB-H / DVB-SH                 */
#define     DIB_MPEFEC_DATA     1   /* DVB-H / DVB-SH                         */
#define     DIB_MPEIFEC_DATA    1   /* DVB-SH                                 */
#define     DIB_FM_DATA         0   /* FM                                     */
#define     DIB_FIG_DATA        1   /* FIC                                    */
#define     DIB_DABPACKET_DATA  0   /* DAB Mode Packet                        */
#define     DIB_DAB_DATA        0   /* DAB Mode Stream - Audio (DAB, DAB+)    */
#define     DIB_EDAB_DATA       0   /* Not yet supported                      */
#define     DIB_TDMB_DATA       0   /* TDMB (i.e DAB Mode Stream Data)        */
#define     DIB_CMMB_DATA       0   /* CMMB                                   */
#else
#define     DIB_RAWTS_DATA      0   /* DVB-T / DVB-H / DVB-SH / TDMB / ISDB-T */
#define     DIB_PES_DATA        0   /* DVB-T                                  */
#define     DIB_SIPSI_DATA      0   /* DVB-T / DVB-H / DVB-SH                 */
#define     DIB_MPEFEC_DATA     0   /* DVB-H / DVB-SH                         */
#define     DIB_MPEIFEC_DATA    0   /* DVB-SH                                 */
#define     DIB_FM_DATA         0   /* FM                                     */
#define     DIB_FIG_DATA        0   /* FIC                                    */
#define     DIB_DABPACKET_DATA  0   /* DAB Mode Packet                        */
#define     DIB_DAB_DATA        0   /* DAB Mode Stream - Audio (DAB, DAB+)    */
#define     DIB_EDAB_DATA       0   /* Not yet supported                      */
#define     DIB_TDMB_DATA       0   /* TDMB (i.e DAB Mode Stream Data)        */
#define     DIB_CMMB_DATA       0   /* CMMB                                   */
#endif


/*************************************************************/
/*** INTERNAL                                             ****/
/*************************************************************/
#define     BUILD_SDK        1   /* Must be set to 1 */


/************************************************************/
/* Possible Configuration Values                            */
/************************************************************/
/** Specific Debug Print Levels (DEBUG_MODE)    */
#define NO_DEBUG                    0
#define DEBUG_PRINT                 1

/* Debug print end of line system  */
#define CRB ""         /* CR printed BEFORE debug output */
#define CRA "\n"       /* CR printed AFTER debug output */

#define DIB_FREE_UNUSED_MPE_BUF     0

/*************************************************************/
/*** Debug Flags                                          ****/
/*************************************************************/
/* Activates debug logs, DIB_ASSERT and dumps. Once application is
   validated, can be disabled for faster and more compact
   code.                                                    */
#define DEBUG_MODE                  DEBUG_PRINT

#define DIB_CHECK_DATA     ( DIB_CHECK_IP_DATA     \
                           | DIB_CHECK_RTP_DATA    \
                           | DIB_CHECK_RAWTS_DATA  \
                           | DIB_CHECK_PES_DATA    \
                           | DIB_CHECK_PCR_DATA    \
                           | DIB_CHECK_FIG_DATA    \
                           | DIB_CHECK_MSC_DATA    \
                           | DIB_CHECK_CAV_DATA    )

/** Check IP header and Pattern if well known (killer stream) */
/** Shouldn't be used with RTP checker */
#define DIB_CHECK_IP_DATA           0

/** Check RTP Continuity for RTP type 96 (Video) or 97 (Audio). (For real streams) */
/** Shouldn't be used with IP checker */
#define DIB_CHECK_RTP_DATA          0

/** Check TS packet continuity counter for each Pid. First TS is handled. */
#define DIB_CHECK_RAWTS_DATA        0

/** Check audio and other PES data */
#define DIB_CHECK_PES_DATA          0

/** Display received PCR field for manual check */
#define DIB_CHECK_PCR_DATA          0

/** Check FIG data */
#define DIB_CHECK_FIG_DATA          0

/** Check MSC data */
#define DIB_CHECK_MSC_DATA          0

/** Check CAV data */
#define DIB_CHECK_CAV_DATA          0

/** Check CMMB data */
#define DIB_CHECK_CMMB_DATA         0

/** Basic (Chip ID..) and Register tests, in low speed mode. PORT_LOG (and NDEBUG in kernel) must be enabled. */
#define DIB_BRIDGE_TESTIF_PREINIT   0

/** Full tests in high speed mode: Chip ID, Registers, Int and Ext Ram, HostIf. PORT_LOG (and NDEBUG in kernel) must be enabled */
#define DIB_BRIDGE_TESTIF_POSTINIT  0

/** Monitoring of the duration of H<->M exchanges, memory available, and table corruption */
#define DIB_BRIDGE_HBM_PROFILER     0

/** Forward from firmware to driver demodulation trace buffers for debugging */
#define DEMOD_TRACE                 0

#define INTERNAL_URAM_CHECK		0


/*************************************************************/
/*** Internal Debug Flags                                 ****/
/*************************************************************/
/** test if transfered Table is the same as internal Table */
#define TEST_TRANSFERT              0

/* Output Data to file for debugging */
#define DIB_CHECK_DATA_IN_FILE      0

/* Add discontinuities and errors in data checkers */
#define DIB_CHECK_MONITORING        0

/* enable test of the hook dma using message GET_VERSION */
#define TEST_DMA                    0

/* debug the Buffer transfered to see the erronous bytes */
#define PRINT_BUFFER                0

/* Internal Use Only */
#define SUPPORT_CAV                 0

#endif
