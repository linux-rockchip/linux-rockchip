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
* @file "DtaScan.h"
* @brief DiBcom Test Application - Scan/Tune Functions Prototypes.
*
***************************************************************************************************/
#ifndef _DTA_SCAN_H_
#define _DTA_SCAN_H_

#define SCAN_FLAG_AUTOSEARCH      0x01 /* Tune with phy param unknown */
#define SCAN_FLAG_PLAIN_TUNE      0x02 /* Tune with phy param to find in the cache */
#define SCAN_FLAG_FAST_SEARCH     0x03 /* Fast tune with phy param known */
#define SCAN_FLAG_MANUAL_TUNE     0x04 /* Tune with phy param given as input parameters */
#define SCAN_FLAG_ALWAYS_TUNE_SUCCESS  0x05 /* Signal lock only */

#define DTA_MAX_FREQUENCIES         48
#define DTA_FREQUENCY_EMPTY          0
#define DTA_BANDWIDTH_EMPTY          0
#define DTA_NO_INDEX               (-1)


int32_t DtaTuneChannel(enum DibDemodType Type, uint32_t bw, uint32_t rf, uint8_t flag, uint8_t async, uint32_t PhyInput);
int32_t DtaTuneChannelEx(uint32_t StreamNum, uint32_t bw, uint32_t rf, uint8_t flag, uint8_t async);
int32_t DtaStartScan(enum DibDemodType Type, uint32_t FreqStart, uint32_t FreqEnd, uint32_t bw);
void    DtaInitTuneArray(void);
int32_t DtaRefreshFreqScan(void);
void    DtaLogScanStatus(FILE *f);
int32_t DtaSetTuneChan(struct DibTuneChan *tc);
void DtaShowChannelParams(struct DibTuneChan *cd, enum DibDemodType Type, uint8_t display);
int32_t GetTuneChan(uint32_t rf, uint32_t bw, struct DibTuneChan *dest);

#endif
