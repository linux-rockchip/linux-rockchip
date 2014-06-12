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
* @file "DtaSiPsi.h"
* @brief DiBcom Test Application - SI PSI Functions Prototypes.
*
***************************************************************************************************/
#ifndef _DTA_SIPSI_H_
#define _DTA_SIPSI_H_

int32_t DtaStartSiPsi(void);
int32_t DtaStopSiPsi(void);
void    DtaResetAllSiPsiCounters(uint32_t ChannelId);
void    DtaAddSiPsiPidCounter(int32_t Pid, uint32_t ItemId);
void    DtaIncSiPsiOkCounter(int32_t Pid, uint32_t ItemId);
void    DtaIncSiPsiKoCounter(int32_t Pid, uint32_t ItemId);
void    DtaRemoveBuffersFromSipsiItem(int32_t Pid, uint32_t ItemId);
void    DtaReadSiPsiSection(uint32_t ItemId, uint32_t timeout);
void    DtaResetSiPsiCounters(uint32_t ChannelId);
void    DtaLogSiPsiStatus(FILE * f);
void    DtaSiPsiCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status);

#endif
