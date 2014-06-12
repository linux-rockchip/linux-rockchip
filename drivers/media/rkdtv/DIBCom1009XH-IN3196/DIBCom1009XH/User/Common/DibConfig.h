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

/***********************************************************************************************//**
 * @file "DibConfig.h"
 * @brief SDK specific build configuration (standard selection, parameter checking, IC support).
 *
 ***************************************************************************************************/
#ifndef DIB_CONFIG_H
#define DIB_CONFIG_H

/*************************************************************/
/*** IC SELECTION                                         ****/
/*************************************************************/
/* If only one type of IC is used then remove the other to reduce
   code size */
#define     USE_FIREFLY   0     /* DIB9080 / DIB9090 */
#define     USE_DRAGONFLY 1      /* DIB19088 / DIB29098 */
#define     mSDK   0
#define DRIVER_AND_BRIDGE_MERGED 1
#define _BCAS_

/*************************************************************/
/*** BOARD SELECTION                                      ****/
/*************************************************************/
#if (mSDK == 0)
#define     DIB_BOARD_SDIO7078         0
#define     DIB_BOARD_SDIO7070         0
#define     DIB_BOARD_SDIO9080         0
#define     DIB_BOARD_STK7078          0
#define     DIB_BOARD_NIM9080MD        0
#define     DIB_BOARD_STK7078MD4       0
#define     DIB_BOARD_SDIO9090         0
#define     DIB_BOARD_NIM9090MD        0
#define     DIB_BOARD_SDIO19088        0
#define     DIB_BOARD_NIM29098         0
#define     DIB_BOARD_STK29098MD4      0
#define     DIB_BOARD_MOD29098SHA      0
#define     DIB_BOARD_NIM9090H         0
#define     DIB_BOARD_NIM1009xHx       1
#define     DIB_BOARD_NIM10096MD2      0
#define     DIB_BOARD_NIM10096MD3      0
#define     DIB_BOARD_NIM10096MD4      0
#define     DIB_BOARD_NIM3009xHx       0
#define     DIB_BOARD_NIM30092MD2      0
#define     DIB_BOARD_NIM30092MD3      0
#define     DIB_BOARD_NIM30092MD4      0
#define     DIB_BOARD_NIM3006X         0

#else

#define     DIB_BOARD_SDIO7078         0
#define     DIB_BOARD_SDIO7070         0
#define     DIB_BOARD_SDIO9080         0
#define     DIB_BOARD_STK7078          0
#define     DIB_BOARD_NIM9080MD        0
#define     DIB_BOARD_STK7078MD4       0
#define     DIB_BOARD_SDIO9090         0
#define     DIB_BOARD_NIM9090MD        0
#define     DIB_BOARD_SDIO19088        0
#define     DIB_BOARD_NIM29098         1
#define     DIB_BOARD_STK29098MD4      0
#define     DIB_BOARD_MOD29098SHA      0
#define     DIB_BOARD_NIM9090H         0
#define     DIB_BOARD_NIM1009xHx       1
#define     DIB_BOARD_NIM10096MD2      1
#define     DIB_BOARD_NIM10096MD3      1
#define     DIB_BOARD_NIM10096MD4      1
#define     DIB_BOARD_NIM3009xHx       1
#define     DIB_BOARD_NIM30092MD2      1
#define     DIB_BOARD_NIM30092MD3      1
#define     DIB_BOARD_NIM30092MD4      1
#endif


/* Check IC and board match */
#if(USE_DRAGONFLY == 0)
#if((DIB_BOARD_SDIO19088 == 1) || (DIB_BOARD_NIM29098 == 1) || (DIB_BOARD_NIM29090D == 1) || (DIB_BOARD_STK29098MD4 == 1) || (DIB_BOARD_MOD29098SHA == 1) || (DIB_BOARD_NIM1009xHx == 1) || (DIB_BOARD_NIM10096MD2 == 1) || (DIB_BOARD_NIM10096MD3 == 1) || (DIB_BOARD_NIM10096MD4 == 1) || (DIB_BOARD_NIM3009xHx == 1) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30096MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1))
#error IC selection is incompatible board selection.
#endif
#endif

#if(USE_FIREFLY == 0)
#if((DIB_BOARD_SDIO7078 == 1)  || (DIB_BOARD_SDIO7070 == 1)   || (DIB_BOARD_SDIO9080 == 1) || (DIB_BOARD_STK7078 == 1)  || \
    (DIB_BOARD_NIM9080MD == 1) || (DIB_BOARD_STK7078MD4 == 1) || (DIB_BOARD_SDIO9090 == 1) || (DIB_BOARD_NIM9090MD == 1) || \
    (DIB_BOARD_NIM9090H == 1))
#error IC selection is incompatible board selection.
#endif
#endif

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
#define     DIB_ANALOD_STD   0   /* Not yet supported    */
#define     DIB_ATSC_STD     0   /* Not yet supported    */
#define     DIB_FM_STD       0   /* Not yet supported    */
#define     DIB_CMMB_STD     1   /* CMMB                 */
#define     DIB_CTTB_STD     1   /* CTTB                 */
#else
#define     DIB_DVB_STD      1   /* DVB-T/DVB-H          */
#define     DIB_DVBSH_STD    0   /* DVB-SH               */
#define     DIB_DAB_STD      0   /* DAB/TDMB/DABPacket/EDAB  */
#define     DIB_ISDBT_STD    0   /* Not yet supported    */
#define     DIB_ANALOD_STD   0   /* Not yet supported    */
#define     DIB_ATSC_STD     0   /* Not yet supported    */
#define     DIB_FM_STD       0   /* Not yet supported    */
#define     DIB_CMMB_STD     0   /* CMMB                 */
#define     DIB_CTTB_STD     0   /* CTTB                 */
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
#define     DIB_DABPACKET_DATA  1   /* Dab Mode Packet                        */
#define     DIB_DAB_DATA        1   /* Dab Mode Stream - Audio (DAB, DAB+)    */
#define     DIB_EDAB_DATA       0   /* Not yet supported                      */
#define     DIB_TDMB_DATA       1   /* Dab Mode Stream - Data 5TDMB)          */
#define     DIB_CMMB_DATA       1   /* CMMB                                   */
#else
#define     DIB_RAWTS_DATA      0   /* DVB-T / DVB-H / DVB-SH / TDMB / ISDB-T */
#define     DIB_PES_DATA        0   /* DVB-T                                  */
#define     DIB_SIPSI_DATA      0   /* DVB-T / DVB-H / DVB-SH                 */
#define     DIB_MPEFEC_DATA     0   /* DVB-H / DVB-SH                         */
#define     DIB_MPEIFEC_DATA    0   /* DVB-SH                                 */
#define     DIB_FM_DATA         0   /* FM                                     */
#define     DIB_FIG_DATA        0   /* FIC                                    */
#define     DIB_DABPACKET_DATA  0   /* Dab Mode Packet                        */
#define     DIB_DAB_DATA        0   /* Dab Mode Stream - Audio (DAB, DAB+)    */
#define     DIB_EDAB_DATA       0   /* Not yet supported                      */
#define     DIB_TDMB_DATA       0   /* Dab Mode Stream - Data 5TDMB)          */
#define     DIB_CMMB_DATA       0   /* CMMB                                   */
#endif


/* Check data and standard match */


/*************************************************************/
/*** INTERNAL                                             ****/
/*************************************************************/

#define     eWRFL_NONE              0   /* do not support write to flash */ 
#define     eWRFL_FILE              1   /* write flash image from file   */ 
#define     eWRFL_STATIC            2   /* write flash image from static array */ 

#define     BUILD_SDK               1   /* Must be set to 1 */
#define     EMBEDDED_FLASH_SUPPORT  0   /* 1 if boot from spi flash embedded is supported */
#if (EMBEDDED_FLASH_SUPPORT == 1)
   #define    WRITE_FLASH_SUPPORT   eWRFL_NONE /* convenient for integration : eWRFL_FILE */
#else
   #define    WRITE_FLASH_SUPPORT   eWRFL_NONE   
#endif
#define     DIB_INTERNAL_DEBUG      0   /* Internal use only */ /* NDEV */

#endif  /* DIB_CONFIG_H */
