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
* @file "DtaCmds.h"
* @brief DiBcom Test Application - Commands Functions Prototypes.
*
***************************************************************************************************/
#ifndef _DTA_CMDS_H_
#define _DTA_CMDS_H_


struct DtaStream
{
   STREAM_HDL                 Hdl;
   uint32_t                   Used;
   enum DibDemodType          Std;
   uint32_t                   Options;
   struct DibStreamAttributes Attr;
};

struct DtaFrontend
{
   FRONTEND_HDL                 Hdl;
   uint32_t                     StreamNum;
   uint32_t                     Used;
   struct DibFrontendAttributes Attr;
};

struct DtaChannel
{
   struct DibChannel            Info;
   uint32_t                     Used;
   CHANNEL_HDL                  ChannelHdl;
   struct DibTotalDemodMonit    ChannelMonit;   /* for signal monitoring */
   uint32_t                     NbReliable;
   uint32_t                     NbTimeout;
   uint32_t                     StreamNum;
   int16_t                      ActualPowerUpTimeMs;
};

struct DtaFilter
{
   enum DibDataType DataType;
   uint32_t         ParentChannel;
   uint32_t         Used;
   FILTER_HDL       FilterHdl;
};

struct DtaItem
{
   union DibFilters Config;
   uint32_t         ParentFilter;
   uint32_t         Used;
   ELEM_HDL         ItemHdl;
};

#define MAX_TS_MONIT_CTX      2
#define MAX_PID_BY_CTX       10 

struct DtaTsInfo
{
   uint32_t         BufferError;
   uint32_t         CorruptedPacketsCount;
   uint32_t         ErrorIndicFlag;
   uint32_t         DiscontinuitiesCount;
   uint32_t         TramesOk;
   uint32_t         NullPackets;
   uint16_t         PidsVal[MAX_PID_BY_CTX];
   uint32_t         PidsCCCount[MAX_PID_BY_CTX];
};

extern struct DtaChannel ChannelArray[DIB_MAX_NB_CHANNELS];
extern struct DtaFilter  FilterArray[DIB_MAX_NB_FILTERS];
extern struct DtaItem    ItemArray[DIB_MAX_NB_ITEMS];

int32_t DtaInit(void);
int32_t DtaInitSoft(enum DibBoardType BoardType, BOARD_HDL BoardHdl);
int32_t DtaInitSoftFromFlash(enum DibBoardType BoardType, BOARD_HDL BoardHdl);
int32_t DtaDeInit(void);
int32_t DtaTestEmbVersion(uint32_t);
void    DtaDisplayLayoutInfo(void);

int32_t DtaSetGlobalPowerMode(enum DibBool TimeSlicing);
int32_t DtaSetPowerMode(enum DibBool TimeSlicing, uint32_t StreamNum);
int32_t DtaGetPowerMode(uint32_t StreamNum);

int32_t DtaSetHbmMode(enum DibHbmMode HbmMode);
int32_t DtaSetPrefetch(ELEM_HDL ElemHdl, uint16_t Prefetch);
int32_t DtaTestReinit(enum DibBoardType BoardType);


void DtaSetDriverDebugMask(uint32_t Mask);
void DtaSetDmsk(uint8_t Mask);
void DtaSetDhelp(uint16_t i, uint16_t v, uint16_t ch);
void DtaMpeMnt(void);
void DtaHbmProf(void);
void DtaTsMonitInit();
void DtaTsMonitStart(uint32_t ctx);
void DtaTsMonitStop(void);
void DtaTsMonitFree(uint32_t ctx);
void DtaTsRecordEnable(uint8_t enable);
void DtaLogTsStatus(FILE * f);

void DtaDebugReadReg16(uint32_t Addr);
void DtaDebugReadReg32(uint32_t Addr);
void DtaDebugWriteReg16(uint32_t Addr, uint32_t Data);
void DtaDebugWriteReg32(uint32_t Addr, uint32_t Data);
void DtaDebugReadBuf32(uint32_t Addr, uint32_t size);

void DtaDebugReadRegInternal(uint32_t Addr);
void DtaDebugWriteRegInternal(uint32_t Addr, uint32_t Val);
void DtaDebugDumpReg(void);
void DtaDebugReadI2cTuner(uint16_t Addr);
void DtaDebugWriteI2cTuner(uint16_t Addr);
void DtaDebugWriteI2cTunerReg(uint8_t Addr, uint8_t Data);

DIBSTATUS DtaGetChannel(uint32_t ChannelId, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType TypeDemod, uint32_t Frequency, uint16_t BW, uint8_t Async, uint8_t Flag);
DIBSTATUS DtaGetChannelEx(uint32_t ChannelId, uint32_t StreamNum, struct DibTuneChan *pChannelInfo, uint8_t Async, uint8_t Flag);
 DIBSTATUS DtaGetWkDvbChannel(uint32_t ChannelId, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType TypeDemod, uint32_t Frequency, enum DibSpectrumBW BW, uint8_t Async, uint8_t Flag, enum DibSpectrumInv InvSpec, enum DibFFTSize Nfft, enum DibGuardIntervalSize Guard, enum DibConstellation Nqam, enum DibIntlvNative IntlvNative, enum DibVitHrch VitHrch, enum DibVitPriority VitSelectHp, enum DibVitAlpha VitAlpha, enum DibVitCoderate VitCodeRateHp, enum DibVitCoderate VitCodeRateLp);


DIBSTATUS DtaGetStream(uint32_t StreamNum, uint32_t Std, uint32_t Options, uint32_t EnPowerSaving);
DIBSTATUS DtaDeleteStream(uint32_t StreamNum);
DIBSTATUS DtaDeleteChannel(uint32_t ChannelId);
DIBSTATUS DtaGetFrontend(uint32_t StreamNum, uint32_t FeNum, uint32_t OutputFeNum, uint32_t Force);
DIBSTATUS DtaDeleteFrontend(uint32_t FeNum);
DIBSTATUS DtaCreateFilter(uint32_t FilterId, uint32_t ChannelId, enum DibDataType DataType, uint8_t async);
DIBSTATUS DtaDeleteFilter(uint32_t FilterId);
DIBSTATUS DtaAddToFilter(uint32_t ItemId, uint32_t FilterId, union DibFilters *pFilterDescriptor, int32_t filter_type);
DIBSTATUS DtaRemoveFromFilter(uint32_t ItemId);
void      DtaRemovePidCountersByFilter(uint32_t FilterType);


void DtaSetEventCallback(enum DibEvent Event, uint32_t Add);

#if (DEBUG_LIB == 1)
void DtaTestDfly(void);
#endif

void DtaInitInternal(void);
void DtaShowInternal(void);

extern struct DibGlobalInfo         globalInfo;


extern struct DtaChannel  ChannelArray[DIB_MAX_NB_CHANNELS];
extern struct DtaFilter   FilterArray[DIB_MAX_NB_FILTERS];
extern struct DtaItem     ItemArray[DIB_MAX_NB_ITEMS];
extern struct DtaStream   StreamArray[DIB_MAX_NB_OF_STREAMS];
extern struct DtaFrontend FrontendArray[DIB_MAX_NB_OF_FRONTENDS];

#define DTA_TINY_DISPLAY      0x00
#define DTA_SHORT_DISPLAY     0x01
#define DTA_FULL_DISPLAY      0x02
#define DTA_CSV_DISPLAY       0x03

#endif
