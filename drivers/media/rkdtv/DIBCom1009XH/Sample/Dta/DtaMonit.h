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
* @file "DtaMonit.h"
* @brief DiBcom Test Application - Monitoring Functions Prototypes.
*
***************************************************************************************************/
#ifndef _DTA_MONIT_H_
#define _DTA_MONIT_H_

int32_t DtaStartMonit(void);
int32_t DtaStopMonit(void);
int32_t DtaGetDvbHMonitInfo(FILE * fdlog, uint8_t full, uint32_t sample);
int32_t DtaGetDvbTMonitInfo(FILE * fdlog, uint8_t full);
int32_t DtaGetDvbSHMonitInfo(FILE * f, uint8_t display, uint32_t sample);
int32_t DtaGetCmmbMonitInfo(FILE *f, uint8_t display, uint32_t sample);
int32_t DtaGetDabMonitInfo(FILE * f, uint32_t type, uint8_t display);
void    DtaGetSHMonitInfo(uint8_t ChannelId);
void    DtaGetSignalMonitInfo(uint8_t ch, uint8_t demod, uint8_t full, uint8_t async, FILE *logFile);
void    DtaDisplaySignalMonitInfo(struct DibTotalDemodMonit *mnt, uint8_t display, FILE *logFile);
void    DtaSignalMonitLoop(uint8_t ts, uint32_t n_iter, uint8_t n_demod);
void    DtaSignalMonitLoopInFile(uint32_t ChannelIndex, uint32_t n_iter, uint32_t n_demod, char *fileName, uint32_t period);
void    DtaLogMonitStatus(FILE * f);
int32_t DtaRefreshMonitArray(void);
void    DtaSetMonitArray(uint8_t ChannelIndex);
void    DtaClearMonitArray(uint8_t ChannelIndex);
void    DtaClearAllMonitArray(void);

#endif
