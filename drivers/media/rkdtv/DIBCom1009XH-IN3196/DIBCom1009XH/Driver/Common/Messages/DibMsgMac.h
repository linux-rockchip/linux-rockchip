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

#ifndef __MSG_TYPE_MAC__
#define __MSG_TYPE_MAC__

#ifdef __cplusplus
#define EXTERN  extern "C"
#else
#define EXTERN
#endif


/* Max message length */
#define MAX_MSG_MAC_BITS               2501

/* Input message identifiers */
#define IN_MSG_MPE_MONITOR             5
#define IN_MSG_RAWTS_MONITOR           6
#define IN_MSG_VERSION                 9
#define IN_MSG_MONIT_DEMOD             11
#define IN_MSG_ERROR                   12
#define IN_MSG_EVENT                   14
#define IN_MSG_PRINT                   17
#define IN_MSG_ACK_CREATE_CH           18
#define IN_MSG_ACK_DEL_CH              19
#define IN_MSG_ACK_CREATE_FILT         20
#define IN_MSG_ACK_DEL_FILT            21
#define IN_MSG_ACK_ADD_TO_FILT         22
#define IN_MSG_MPE_IFEC_MONITOR        23
#define IN_MSG_INTERNAL_ACCESS         24
#define IN_MSG_ACK_INIT                25
#define IN_MSG_HRFX_SET_CONFIG         27
#define IN_MSG_HRFX_SET_SVCPARAM       28
#define IN_MSG_HRFX_INIT               29
#define IN_MSG_WAVE                    30
#define IN_MSG_INT_DBG                 31
#define IN_MSG_PROFILE                 32
#define IN_MSG_CMMB_RSM_MONITOR        33
#define IN_MSG_HRFX_STEP               34
#define IN_MSG_DOWNLOAD_IND            35
#define IN_MSG_API_ANS                 36
#define IN_MSG_FIC_MONITOR             37
#define IN_MSG_MSCPACKET_MONITOR       38
#define IN_MSG_MSCPLUS_MONITOR         39
#define IN_MSG_ACK_CAS_EN              40
#define IN_MSG_CMMB_MF_MONITOR         41
#define IN_MSG_ACK_API_UP_EN           42
#define IN_MSG_API_UP                  43
#define IN_MSG_STREAM_INFO             44
#define IN_MSG_FRONTEND_INFO           45
#define IN_MSG_LAYOUT_INFO             46
#define IN_MSG_UDIBADAPTER_CFG_ACK     48
#define IN_MSG_HOCTO_INIT              49
#define IN_MSG_HOCTO_RW_OCTOPUS        50
#define IN_MSG_ACK_GET_STREAM          51
#define IN_MSG_ACK_DEL_STREAM          52
#define IN_MSG_ACK_ADD_FRONTEND        53
#define IN_MSG_ACK_DEL_FRONTEND        54
#define IN_MSG_WRITE_SLAVE_REP         55
#define IN_MSG_PRINT_BUFFER            56
#define IN_MSG_ADD_SLAVE_CONFIRM       57
#define IN_MSG_UPDATE_CHANNEL_IND      58
#define IN_MSG_TUNE_STATUS_CFN         59
#define IN_MSG_READ_SLAVE_REP          60
#define IN_MSG_SET_BASIC_LAYOUT_INFO   62
#define IN_MSG_CMMB_MAP_SLAVE          63
#define IN_MSG_STOP_MAC_HANDOVER       64
#define IN_MSG_START_MAC_HANDOVER      65
#define IN_MSG_WAKEUP_MAC_HANDOVER     66
#define IN_MSG_UPDATE_CHANNEL_CFN      67
#define IN_MSG_FLASH_PROGRAM_DONE      68
#define IN_MSG_INFO_DATA               69
#define IN_MSG_INFO_SET_STATUS         70
#define MAX_MSG_MAC_IN                 71

/* Output message identifiers */
#define OUT_MSG_REQ_VERSION            2
#define OUT_MSG_MONIT_DEMOD            8
#define OUT_MSG_CONF_GPIO              9
#define OUT_MSG_DEBUG_HELP             10
#define OUT_MSG_ENABLE_TIME_SLICE      12
#define OUT_MSG_REQ_CREATE_CH          20
#define OUT_MSG_REQ_DEL_CH             21
#define OUT_MSG_REQ_CREATE_FILT        22
#define OUT_MSG_REQ_DEL_FILT           23
#define OUT_MSG_REQ_ADD_TO_FILT        24
#define OUT_MSG_REQ_REM_TO_FILT        25
#define OUT_MSG_INTERNAL_ACCESS        26
#define OUT_MSG_ABORT_TUNE_MONIT       28
#define OUT_MSG_SET_PID_MODE           29
#define OUT_MSG_INT_DBG                30
#define OUT_MSG_SET_HBM                31
#define OUT_MSG_API_REQ                32
#define OUT_MSG_CAS_EN                 33
#define OUT_MSG_API_UP_EN              34
#define OUT_MSG_API_UP_ACK             35
#define OUT_MSG_GET_STREAM_INFO        37
#define OUT_MSG_GET_FRONTEND_INFO      38
#define OUT_MSG_GET_STREAM             39
#define OUT_MSG_DEL_STREAM             40
#define OUT_MSG_ADD_FRONTEND           41
#define OUT_MSG_DEL_FRONTEND           42
#define OUT_MSG_GET_LAYOUT_INFO        43
#define OUT_MSG_UDIBADAPTER_OFF        46
#define OUT_MSG_OCTO_INIT              47
#define OUT_MSG_OCTO_RW_OCTOPUS        48
#define OUT_MSG_WRITE_SLAVE_REQ        49
#define OUT_MSG_SET_BASIC_LAYOUT_INFO  50
#define OUT_MSG_ADD_SLAVE_DEVICE       51
#define OUT_MSG_UPDATE_CHANNEL_IND     52
#define OUT_MSG_SLEEP_IND              53
#define OUT_MSG_OCTO_UTILS_CHECK_SUM   54
#define OUT_MSG_TUNE_IND               55
#define OUT_MSG_DOWNLOAD_CFN           56
#define OUT_MSG_READ_SLAVE_REQ         58
#define OUT_MSG_CMMB_MAP_MASTER        59
#define OUT_MSG_DUMP                   60
#define OUT_MSG_START_MAC_HANDOVER     61
#define OUT_MSG_STOP_MAC_HANDOVER      62
#define OUT_MSG_WAKEUP_MAC_HANDOVER    63
#define OUT_MSG_FLASH_PROGRAM          64
#define OUT_MSG_INFO_REQUEST           65
#define OUT_MSG_INFO_SET               66
#define MAX_MSG_MAC_OUT                67




#include "DibMsgTypes.h"
#include "DibMsgGeneric.h"
#include "DibMsgFields.h"
#include "DibMsgMacIf.h"
#include "DibStruct.h"
#include "DibExtAPI.h"



/*----------------------------------------------------------
 * messages
 * every msg should start by MsgXXX, have a MsgHeader, and define IN_MSG_ or OUT_MSG_ values
 *---------------------------------------------------------*/

/*------------------------------MsgDump-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_DUMP
 */
struct MsgDump
{
   struct MsgHeader Head;
};/*------------------------------MsgGetStream-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_GET_STREAM.
 */
struct MsgGetStream
{
   struct MsgHeader Head;
   uint32_t OutputOptions;
   uint32_t Standard;
   uint16_t StreamId;
   uint8_t EnableTimeSlicing;
};
/*------------------------------MsgDeleteStream-----------------------------------*/
/**
 Msg to delete a stream
 OUT_MSG_DEL_STREAM.
 */
struct MsgDeleteStream
{
   struct MsgHeader Head;
   uint16_t StreamId;
};
/*------------------------------MsgAddFrontend-------------------------------------*/
/**
 Msg to add a frontend to a stream
 OUT_MSG_ADD_FRONTEND.
 */
struct MsgAddFrontend
{
   struct MsgHeader Head;
   uint16_t FrontendId;
   uint16_t StreamId;
   int16_t OutputFrontendId;
};
/*------------------------------MsgRemoveFrontend-----------------------------------*/
/**
 Msg to remove a frontend to a stream
 OUT_MSG_DEL_FRONTEND.
 */
struct MsgRemoveFrontend
{
   struct MsgHeader Head;
   uint16_t FrontendId;
   uint16_t StreamId;
};
/*--------------------------- MsgCreateChannel --------------------------- */

struct MsgCreateChannel
{
   struct MsgHeader Head;

   uint8_t ChannelId;
   uint8_t TuneMonit;
   uint16_t StreamId;
   struct ChannelDesc Desc;

};
/*----------------------------- MsgTuneInd ------------------------------ */

struct MsgTuneIndication
{
   struct MsgHeader Head;

   uint16_t StreamId;
   uint8_t ChannelId;
   uint8_t FastScan;
   uint8_t TuneMonit;
};
/*--------------------------- MsgAckCreateChannel --------------------------- */

struct MsgAckCreateChannel
{
   struct MsgHeader Head;

   uint8_t ChannelId;
   uint8_t AckOk;
};/*--------------------------- MsgCmmbMappingExchg -------------------------- */

struct MsgCmmbMapMaster
{
   struct MsgHeader Head;
   uint8_t Status;
   uint8_t Initiator;
   uint8_t ChannelId;
   struct CMMBmapping MappingExch;
};
struct MsgCmmbMapSlave
{
   struct MsgHeader Head;
   uint8_t Status;
   uint8_t Initiator;
   uint8_t ChannelId;
   struct CMMBmapping MappingExch;
};
/*------------------------- MsgUpdateChannelInd --------------------------- */

struct MsgUpdateChannelIndication
{
   struct MsgHeader Head;

   uint16_t StreamId;
   uint8_t ChannelId;
   struct ChannelDesc Desc;

};
/*--------------------------- MsgTuneStatusConfirm --------------------------- */
struct MsgTuneStatusConfirm
{
   struct MsgHeader Head;

   uint16_t FrontendId;
   uint8_t ChannelId;
   enum TuneStatus Status;
   enum AdapterState State;
};
/*--------------------------- MsgSleepIndication --------------------------- */

struct MsgSleepIndication
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t ChannelId;
};
/*-------------------------MsgDeleteChannel-----------------------------------*/
/**
 Msg request the suppression of a channel
 OUT_MSG_REQ_DEL_CH.
 */

struct MsgDeleteChannel
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t ChannelId;
};
/*-------------------------MsgCreateFilter-----------------------------------*/
/**
 Msg request the creation of a new filter
 OUT_MSG_REQ_CREATE_FILT.
 */

struct MsgCreateFilter
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t ChannelId;
   uint8_t FilterId;
   uint8_t Type;
};
/*-------------------------MsgDeleteFilter-----------------------------------*/
/**
 Msg request the suppression of a filter
 OUT_MSG_REQ_DEL_FILT.
 */

struct MsgDeleteFilter
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t FilterId;
};
/*-------------------------MsgCreateItem -----------------------------------*/
struct MsgCreateItem
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t ItemId;
   uint8_t FilterType;
   uint8_t FilterId;
   union ItemParameters Param;
};
/*-------------------------MsgDeleteItem----------------------------------------*/
/**
 Msg request the suppression of a filter's item
 OUT_MSG_REQ_REM_TO_FILT
 */

struct MsgDeleteItem
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t ItemId;
};
/*--------------------------MsgAcknowledgeApi----------------------------------*/
/**
 Acknowledge API request.
 MSG_ACK_API identifier
 IN_MSG_ACK_DEL_CH, IN_MSG_ACK_CREATE_FILT, IN_MSG_ACK_DEL_FILT,
 IN_MSG_ACK_ADD_TO_FILT, IN_MSG_ACK_FREE_ITEM, IN_MSG_ACK_CAS_EN,
 IN_MSG_ACK_API_UP_EN
 */

struct MsgAcknowledgeApi
{
   struct MsgHeader Head;
   uint16_t Handler;
   uint16_t Success;
};
/*------------------------------- MsgSetHbm ------------------------------ */

struct MsgSetHbm
{
   struct MsgHeader Head;
   uint32_t Mode;
};
/* ----------------------------- MsgRawTSMonitor-------------------------- */
/**
 */
struct MsgRawTSMonitor
{
   struct MsgHeader Head;
   uint32_t TotalPackets;
   uint32_t ErrorPackets;
   uint32_t CCPackets;
   uint32_t ItemHdl;
};
/* ----------------------------- MsgMpeMonitor ------------------------------ */
/**
  Firmware send MPE monitoring information after frame reception is complete
  Be careful : do not change the order of the following information
*/
struct MsgMpeMonitor
{
   struct MsgHeader Head;
   uint32_t NbRxBurst;
   uint32_t NbErrBeforeFec;
   uint32_t NbErrAfterFec;
   uint32_t NbErrTableBeforeFec;
   uint32_t NbErrTableAfterFec;
   uint32_t NbTableMissed;
   uint32_t BurstStartDate;
   uint32_t BurstEndDate;
   uint32_t DeltatMin;
   uint32_t DeltatMax;
   uint32_t FrameDuration;
   uint32_t PowerUpDuration;
   uint8_t Padding;
   uint8_t Puncturing;
   uint8_t ItemHdl;
   uint8_t FilterId;
   uint32_t FrameStatus;
};
/*-------------------------------MsgMpeIfecMonitor------------------------------*/
/**
 Firmware send IFEC monitoring information (DVBSH standard)
 */
struct MsgMpeIfecMonitor
{
   struct MsgHeader Head;

   /** ADT (ie. table) monitoring */
   uint32_t NbADTTables;
   uint32_t NbADTTablesCorrected;
   uint32_t NbADTTablesFailed;
   uint32_t NbADTTablesGood;

   /** ADST (ie. burst) monitoring */
   uint32_t NbADSTTables;
   uint32_t NbADSTTablesLost;
   uint32_t NbADSTTablesCorrected;
   uint32_t NbADSTTablesFailed;
   uint32_t NbADSTTablesGood;

   uint32_t BurstStartDate;
   uint32_t BurstEndDate;
   uint32_t DeltatMin;
   uint32_t DeltatMax;
   uint32_t FrameDuration;
   uint32_t PowerUpDuration;

   uint16_t TableIndex;
   uint8_t BurstIndex;
   uint8_t ItemHdl;
   uint32_t FrameStatus;
};
/*--------------------------------MsgFicMonitor----------------------------------*/
 /**
  Fic avalaible packets is tested with the 16bits CRC
  */
struct MsgFicMonitor
{
   struct MsgHeader Head;
   uint32_t ItemHdl;
   uint32_t ErrorPackets;
   uint32_t AvalaiblePackets;
};
/*-------------------------------MsgMscPacketMonitor------------------------------*/
/**
 Firmware send Msc Packet monitoring information
 */
struct MsgMscPacketMonitor
{
   struct MsgHeader Head;
   uint32_t ItemHdl;
   uint32_t MscPacketsAvail;
   uint32_t MscPacketsError;
   uint32_t DataGroupAvail;
   uint32_t DataGroupError;
};
/*-------------------------------MsgMscPlusMonitor------------------------------*/
/**
 Firmware send DAB+ monitoring information
 */
struct MsgMscPlusMonitor
{
   struct MsgHeader Head;
   uint32_t ItemHdl;
   uint32_t AvalaibleSuperFrame;
   uint32_t CorrectedSuperFrame;
   uint32_t FailedsuperFrame;
   uint32_t NbFrameWoSynchro;
};
/* ---------------------------- MsgGetSignalMonit --------------------------------*/

struct MsgGetSignalMonit
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t ChannelId;
   uint8_t DemodId;
};
/* ---------------------------- MsgAckGetSignalMonit -----------------------------*/

struct MsgAckGetSignalMonit
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t ChannelId;
   uint8_t DemodId;
   struct SignalMonitoring Mon;
};

/* --------------------------------- MsgError -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */


struct MsgError
{
   struct MsgHeader Head;

   uint16_t ErrorCode;
   uint16_t ErrorFlag;
   uint16_t Param[4];
};

/* --------------------------------- MsgVersion -----------------------------------*/
/*
 * Description : Host request the version of the firmware
 */
struct MsgVersion
{
   struct MsgHeader Head;

   uint16_t SoftVer;
   uint16_t ChipVer;
   uint32_t HalVer;
   uint32_t HalVerExt[8];
};
/* ----------------------------------- MsgEvent -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */
struct MsgEvent
{
   struct MsgHeader Head;

   uint8_t ItemId;
   uint8_t EventId;
};
/* -------------------------------- MsgCmmbRsmMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB LDPC and RS monitoring information at end of RS matrix.
 */
struct MsgCmmbRsmMonitor
{
   struct MsgHeader Head;

   /* General Info */
   uint8_t NbFecErr;
   uint8_t ItemId;
   uint8_t Status;

   /* LDPC block error rate. */
   uint32_t TotalLdpcBlocks;
   uint32_t ErrorLdpcBlocks;

   /* RS matrix error rate. */
   uint32_t TotalRsMatrices;
   uint32_t ErrorRsMatricesBeforeRs;
   uint32_t ErrorRsMatricesAfterRs;
};

/* ----------------------------- MsgCmmbMfMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB MF information at end of each MF.
 */
struct MsgCmmbMfMonitor
{
   struct MsgHeader Head;

   /* Multiplex frame info. */
   uint8_t Status;
   uint8_t ItemId;

   /* Multiplex Frames. */
   uint32_t TotalMf;
   uint32_t ErrorMfBeforeRs;
   uint32_t ErrorMfAfterRs;
};

/* ----------------------------- MsgDebugHelp -----------------------------*/
/*
 * Set a Debug function of the embedded firmware.
 * Func:
 *    - 0: memory dumping
 *    - 1: ldpc discarder
 *    - 11: Enable Real Time Discarder. Value is the SNR
 *    - 12: dvb-sh discarded
 */
struct MsgDebugHelp
{
   struct MsgHeader Head;

   uint32_t Funct;
   uint32_t Value;
};

/* ----------------------------- MsgPrint -----------------------------*/
/*
 * Message used to send a string to the host
 */
struct MsgPrint
{
   struct MsgHeader Head;

   char Data[200];
};
/* ----------------------------- MsgEnableTimeSlice -----------------------------*/
/*
 * enable/disable time-slicing for a dedicated stream
 */
struct MsgEnableTimeSlice
{
   struct MsgHeader Head;
   uint16_t StreamId;
   uint8_t Enable;
};
/* --------------------------- MsgAbortTuneMonit ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */
struct MsgAbortTuneMonit
{
   struct MsgHeader Head;
};
/* --------------------------- MsgSetPidMode ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */
struct MsgSetPidMode
{
   struct MsgHeader Head;
   uint8_t ItemId;
   uint8_t Prefetch;
};

/* --------------------------- MsgCasEnable ---------------------------*/
/*
 * Enable/disable CAS system
 */
struct MsgCasEnable
{
   struct MsgHeader Head;
   uint8_t Enable;
};

/* ------------------------ MsgAHBAccess ----------------------------*/
/*
 * Internal access done by firmware but requested by the host
 */
struct MsgAHBAccess
{
   struct MsgHeader Head;
   uint32_t Addr;
   uint32_t Value;
   uint8_t Mode;
};
/* ------------------------ MsgHostReflexInit ----------------------------*/
/*
 * Address in emb of reflex context needed for emulation
 */
struct MsgHostReflexInit
{
   struct MsgHeader Head;
   uint32_t HostReflexContextAddress;
};
/* ------------------------ MsgHostReflexStep ----------------------------*/
/*
 * Command to advance one step in emulation
 */
struct MsgHostReflexStep
{
   struct MsgHeader Head;
};
/* ------------------------ MsgHostReflexSetTsConfig ----------------------------*/
/*
 * Each time time slot configuration changes, warn the host
 */
struct MsgHostReflexSetTsConfig
{
   struct MsgHeader Head;
   uint32_t Configuration;
   uint32_t TimeSlotMap_0_31;
   uint32_t ParamIdFourTimeSlots[10];
};
/* ------------------------ MsgHostReflexSetSvcParam ----------------------------*/
/*
 * Each time service parameters changes, warn the host
 */
struct MsgHostReflexSetSvcParam
{
   struct MsgHeader Head;
   uint32_t Parameters;
   uint32_t BaseAddress0;
   uint32_t BaseAddress1;
};
/* --------------------------- MsgGpioConfig ------------------------------*/

struct MsgGpioConfig
{
   struct MsgHeader Head;
   uint32_t NbFuncs;
   uint32_t FeId;
   struct GpioFunction Func[6];
};
/* --------------------------- MsgLayoutConfig ------------------------------*/
struct MsgLayoutConfig
{
   struct MsgHeader Head;
   uint32_t Cfg;
   uint32_t LnaConfig;
   uint16_t GpioDefaultDir[2];
   uint16_t GpioDefaultVal[2];
   uint16_t CurTrig;
   uint16_t MaxTrig;
   uint32_t FeId;
   uint32_t GpioNum;
   uint32_t GpioDir;
   uint32_t GpioVal;
   uint32_t FeIdMask;
   uint32_t TuneStateMask;
   uint32_t FreqRangeMhz[2];
   uint32_t ThresholdVal;
   uint8_t Condition;
   uint8_t OutsideBand;
   uint8_t ThresholdDir;
};

/* --------------------------- MsgAckLayoutConfig ------------------------------*/

struct MsgAckLayoutConfig
{
   struct MsgHeader Head;
   uint32_t Status;
};
/* --------------------------- MsgAckLayoutReset ------------------------------*/

struct MsgLayoutReset
{
   struct MsgHeader Head;
};
/* ----------------------------- MsgGetLayoutInfo -----------------------------*/
/*
 * Request information concerning the layout
 */
struct MsgGetLayoutInfo
{
   struct MsgHeader Head;
};
/* ----------------------------- MsgLayoutInfo --------------------------------*/
/*
 * Reply information concerning the layout
 */
struct MsgLayoutInfo
{
   struct MsgHeader Head;
   uint32_t MaxNbChips;
   uint32_t MaxNbStreams;
   uint32_t MaxNbFrontends;
};
/* ----------------------------- MsgGetFrontendInfo ----------------------------*/
/*
 * Request information concerning a specific frontend
 */
struct MsgGetFrontendInfo
{
   struct MsgHeader Head;
   uint8_t FeNum;
};
/* ----------------------------- MsgFrontendInfo -------------------------------*/
/*
 * Request information concerning a specific frontend
 */
struct MsgFrontendInfo
{
   struct MsgHeader Head;
   uint8_t FeNum;
   struct FrontendInfo FeInfo;
};
/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Request information concerning a specific stream
 */
struct MsgGetStreamInfo
{
   struct MsgHeader Head;
   uint32_t StreamNum;
};
/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Reply information concerning a stream
 */

struct MsgStreamInfo
{
   struct MsgHeader Head;
   uint8_t StreamNum;
   struct StreamInfo StreamInfo;
};

/* ----------------------------- MsgDownloadIndication ------------------------------*/
/*
 * Request for downloading a file
 */
struct MsgDownloadIndication
{
   struct MsgHeader Head;
   char FirmwareName[44];
   uint32_t AddrMsb;
   uint32_t AddrLsb;
   uint32_t LengthAckMsb;
   uint32_t LengthAckLsb;
   uint32_t FileOffset;
   uint32_t MaxLength;
   uint32_t Options;
};

/* ----------------------------- MsgDownloadConfirm ------------------------------*/

/*
 * confirmation of file download
 */
struct MsgDownloadConfirm
{
   struct MsgHeader Head;
   uint32_t LengthAckMsb;
   uint32_t LengthAckLsb;
   uint32_t Length;
   uint8_t Complete;
   uint8_t WithCrc;
};
/* ----------------------------- MsgApiRequest ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer to the firmware and reecive one in return.
 * Each MsgApiRequest must be acknowledged before sending a new one.
 */


struct MsgApiRequest
{
   struct MsgHeader Head;
   uint16_t ReqId;
   uint16_t ReqType;
   uint32_t TxLen;
   uint32_t TxAddr;
};
/* ----------------------------- MsgApiAnswer ------------------------------*/
/*
 * This is a reply to the MsgApiRequest, with the possibility to allow buffer retreival fron embedded buffer to
 * host buffer.
 */
struct MsgApiAnswer
{
   struct MsgHeader Head;
   uint32_t Status;
   uint32_t RxLen;
   uint32_t RxAddr;
};
/* ----------------------------- MsgApiUpEnable ------------------------------*/
/*
 * This message is used to enable/disable the fact that the firmware can send msg api information to the user
 * without any request from it. In that case, the user is notified from the sdk through a message callback mechanism, with the
 * possibility to get also data buffers.
 */
struct MsgApiUpEnable
{
   struct MsgHeader Head;
   uint16_t ReqType;
   uint16_t Enable;
};
/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer from the firwmare to the host is permitted (see MsgApiUpEnable)
 * Each MsgApiUp must be acknoledged by a MsgApiUpAck whose role is to free buffer allocated at RxAddr.
 */
struct MsgApiUp
{
   struct MsgHeader Head;
   uint16_t ReqId;
   uint16_t ReqType;
   uint32_t RxLen;
   uint32_t RxAddr;
};
/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * This message is used to free the memory allocated by firmware when sending the MsgApiUp message
 */
struct MsgApiUpAck
{
   struct MsgHeader Head;
   uint32_t Addr;
};
/* ------------------------------ MsgOctoInit --------------------------*/


struct MsgOctoInit
{
   struct MsgHeader GlobalHeader;
   struct MsgHeader Header;
};
/* ------------------------- MsgOctoUtilsCheckSum ----------------------*/
struct MsgOctoUtilsCheckSum
{
   struct MsgHeader Head;
   struct MsgHeader InternalHead;
   uint32_t Hello;
};

/* --------------------------- MsgOctoRwOctopus --------------------------*/
struct MsgOctoRwOctopus
{
   struct MsgHeader GlobalHeader;
   struct MsgHeader Header;
   uint32_t Config;
   uint32_t Size;
   uint32_t Last;
};
/* --------------------------- MsgHostOctoInit --------------------------*/
/*
 * Indicates the host where to find the data buffer inside chipset memory
 */
struct MsgHostOctoInit
{
   struct MsgHeader GlobalHeader;
   struct MsgHeader Header;
   uint32_t HostOctopusDataAddress;
};
/* --------------------------- MsgHostOctoRwOctopus ----------------------*/
struct MsgHostOctoRwOctopus
{
   struct MsgHeader GlobalHeader;
   struct MsgHeader Header;
};
/* --------------------------- MsgWave -----------------------------------*/
struct MsgWave
{
   struct MsgHeader Header;
};
/* --------------------------- MsgIntDebug -------------------------------*/
struct MsgIntDebug
{
   struct MsgHeader Header;
};
/* --------------------------- MsgProfile --------------------------------*/
struct MsgProfile
{
   struct MsgHeader Header;
};

/* --------------------------- debugCmd ----------------------------------*/

/* Mode */
struct MsgSlaveData
{
   struct MsgHeader Head;
   uint8_t ByteMode;
   uint8_t Incr;
   uint16_t Len;
   uint32_t Addr;
   uint8_t Buffer[16];
};
struct MsgSlaveControl
{
   struct MsgHeader Head;
   uint8_t ByteMode;
   uint8_t Incr;
   uint16_t Len;
   uint32_t Addr;
};
/* MsgSetBasicLayoutInfo */
struct MsgSetBasicLayoutInfo
{
   struct MsgHeader Head;
   struct BasicLayoutInfo info;
};
/* --------------------------- MsgAddSlaveIndication ----------------------------------*/
struct MsgAddSlaveDevice
{
   struct MsgHeader Head;
   struct BasicLayoutInfo info;
   uint8_t deviceAddress;
   uint16_t gpioInterface;
};
/* --------------------------- MsgAddSlaveConfirm ----------------------------------*/
struct MsgAddSlaveConfirm
{
   struct MsgHeader Head;
   int32_t status;
};
/* ---------------------------- MsgStopMacHandover --------------------------------*/
struct MsgStopMacHandover
{
   struct MsgHeader Head;
   uint8_t ChannelId;
};
/* ---------------------------- MsgStartMacHandover --------------------------------*/
struct MsgStartMacHandover
{
   struct MsgHeader Head;
   uint32_t Pattern;
   uint8_t ChannelId;
};
/* ---------------------------- MsgWakeUpMacHandover --------------------------------*/
struct MsgWakeUpMacHandover
{
   struct MsgHeader Head;
   uint32_t StreamId;
   uint32_t OldChipId;
   uint32_t NewChipId;
};
/*------------------------------MsgFlashProgram-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_FLASH_PROGRAM.
 */
struct MsgFlashProgram
{
   struct MsgHeader Head;
   uint32_t NameLength;
   char Name[44];
};

/*---------------------------MsgFlashProgramDone-----------------------------------*/
/**
 Msg to have a new stream
 IN_MSG_FLASH_PROGRAM_DONE.
 */
struct MsgFlashProgramDone
{
   struct MsgHeader Head;
   int Status;
   uint32_t Size;
};
/* ----------------------------- MsgGetInfo -----------------------------*/
/*
 * Request information
 */
struct MsgInfoRequest
{
   struct MsgHeader Head;
   uint32_t Type;
};
/*
 * Reply information data
 */
struct MsgInfoData
{
   struct MsgHeader Head;
   uint32_t Type;
   uint8_t Status;
   union InfoParameters Param;
};
/* ----------------------------- MsgSetInfo -----------------------------*/
/*
 * Set information
 */
struct MsgInfoSet
{
   struct MsgHeader Head;
   uint32_t Type;
   union InfoParameters Param;
};
/*
 * Get set information status
 */
struct MsgInfoSetStatus
{
   struct MsgHeader Head;
   uint8_t Status;
};






/*----------------------------------------------------------
 * messages
 * every msg should start by MsgXXX, have a MsgHeader, and define IN_MSG_ or OUT_MSG_ values
 *---------------------------------------------------------*/

/*------------------------------MsgDump-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_DUMP
 */
#define     MsgDumpBits 32
#define     MsgDumpPackInit(s, ctx)  SerialBufRestart(ctx); MsgDumpPack(s, ctx);
#define     MsgDumpUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDumpUnpack(ctx, s);
#define     MsgDumpPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgDumpUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))
/*------------------------------MsgGetStream-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_GET_STREAM.
 */
#define     MsgGetStreamBits 114
#define     MsgGetStreamPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetStreamPack(s, ctx);
#define     MsgGetStreamUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetStreamUnpack(ctx, s);
EXTERN void MsgGetStreamPack(struct MsgGetStream *s, struct SerialBuf *ctx);
EXTERN void MsgGetStreamUnpack(struct SerialBuf *ctx, struct MsgGetStream *s);

/*------------------------------MsgDeleteStream-----------------------------------*/
/**
 Msg to delete a stream
 OUT_MSG_DEL_STREAM.
 */
#define     MsgDeleteStreamBits 48
#define     MsgDeleteStreamPackInit(s, ctx)  SerialBufRestart(ctx); MsgDeleteStreamPack(s, ctx);
#define     MsgDeleteStreamUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDeleteStreamUnpack(ctx, s);
EXTERN void MsgDeleteStreamPack(struct MsgDeleteStream *s, struct SerialBuf *ctx);
EXTERN void MsgDeleteStreamUnpack(struct SerialBuf *ctx, struct MsgDeleteStream *s);

/*------------------------------MsgAddFrontend-------------------------------------*/
/**
 Msg to add a frontend to a stream
 OUT_MSG_ADD_FRONTEND.
 */
#define     MsgAddFrontendBits 80
#define     MsgAddFrontendPackInit(s, ctx)  SerialBufRestart(ctx); MsgAddFrontendPack(s, ctx);
#define     MsgAddFrontendUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAddFrontendUnpack(ctx, s);
EXTERN void MsgAddFrontendPack(struct MsgAddFrontend *s, struct SerialBuf *ctx);
EXTERN void MsgAddFrontendUnpack(struct SerialBuf *ctx, struct MsgAddFrontend *s);

/*------------------------------MsgRemoveFrontend-----------------------------------*/
/**
 Msg to remove a frontend to a stream
 OUT_MSG_DEL_FRONTEND.
 */
#define     MsgRemoveFrontendBits 64
#define     MsgRemoveFrontendPackInit(s, ctx)  SerialBufRestart(ctx); MsgRemoveFrontendPack(s, ctx);
#define     MsgRemoveFrontendUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRemoveFrontendUnpack(ctx, s);
EXTERN void MsgRemoveFrontendPack(struct MsgRemoveFrontend *s, struct SerialBuf *ctx);
EXTERN void MsgRemoveFrontendUnpack(struct SerialBuf *ctx, struct MsgRemoveFrontend *s);

/*--------------------------- MsgCreateChannel --------------------------- */

#define     MsgCreateChannelBits 2501
#define     MsgCreateChannelPackInit(s, ctx)  SerialBufRestart(ctx); MsgCreateChannelPack(s, ctx);
#define     MsgCreateChannelUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCreateChannelUnpack(ctx, s);
EXTERN void MsgCreateChannelPack(struct MsgCreateChannel *s, struct SerialBuf *ctx);
EXTERN void MsgCreateChannelUnpack(struct SerialBuf *ctx, struct MsgCreateChannel *s);

/*----------------------------- MsgTuneInd ------------------------------ */

#define     MsgTuneIndicationBits 58
#define     MsgTuneIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgTuneIndicationPack(s, ctx);
#define     MsgTuneIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgTuneIndicationUnpack(ctx, s);
EXTERN void MsgTuneIndicationPack(struct MsgTuneIndication *s, struct SerialBuf *ctx);
EXTERN void MsgTuneIndicationUnpack(struct SerialBuf *ctx, struct MsgTuneIndication *s);

/*--------------------------- MsgAckCreateChannel --------------------------- */

#define     MsgAckCreateChannelBits 42
#define     MsgAckCreateChannelPackInit(s, ctx)  SerialBufRestart(ctx); MsgAckCreateChannelPack(s, ctx);
#define     MsgAckCreateChannelUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAckCreateChannelUnpack(ctx, s);
EXTERN void MsgAckCreateChannelPack(struct MsgAckCreateChannel *s, struct SerialBuf *ctx);
EXTERN void MsgAckCreateChannelUnpack(struct SerialBuf *ctx, struct MsgAckCreateChannel *s);
/*--------------------------- MsgCmmbMappingExchg -------------------------- */

#define     MsgCmmbMapMasterBits 1616
#define     MsgCmmbMapMasterPackInit(s, ctx)  SerialBufRestart(ctx); MsgCmmbMapMasterPack(s, ctx);
#define     MsgCmmbMapMasterUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCmmbMapMasterUnpack(ctx, s);
EXTERN void MsgCmmbMapMasterPack(struct MsgCmmbMapMaster *s, struct SerialBuf *ctx);
EXTERN void MsgCmmbMapMasterUnpack(struct SerialBuf *ctx, struct MsgCmmbMapMaster *s);

#define     MsgCmmbMapSlaveBits 1616
#define     MsgCmmbMapSlavePackInit(s, ctx)  SerialBufRestart(ctx); MsgCmmbMapSlavePack(s, ctx);
#define     MsgCmmbMapSlaveUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCmmbMapSlaveUnpack(ctx, s);
#define     MsgCmmbMapSlavePack(s, ctx)   MsgCmmbMapMasterPack((struct MsgCmmbMapMaster *)(s), ctx)
#define     MsgCmmbMapSlaveUnpack(ctx, s)   MsgCmmbMapMasterUnpack(ctx, (struct MsgCmmbMapMaster *)(s))

/*------------------------- MsgUpdateChannelInd --------------------------- */

#define     MsgUpdateChannelIndicationBits 2493
#define     MsgUpdateChannelIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgUpdateChannelIndicationPack(s, ctx);
#define     MsgUpdateChannelIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgUpdateChannelIndicationUnpack(ctx, s);
EXTERN void MsgUpdateChannelIndicationPack(struct MsgUpdateChannelIndication *s, struct SerialBuf *ctx);
EXTERN void MsgUpdateChannelIndicationUnpack(struct SerialBuf *ctx, struct MsgUpdateChannelIndication *s);

/*--------------------------- MsgTuneStatusConfirm --------------------------- */
#define     MsgTuneStatusConfirmBits 72
#define     MsgTuneStatusConfirmPackInit(s, ctx)  SerialBufRestart(ctx); MsgTuneStatusConfirmPack(s, ctx);
#define     MsgTuneStatusConfirmUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgTuneStatusConfirmUnpack(ctx, s);
EXTERN void MsgTuneStatusConfirmPack(struct MsgTuneStatusConfirm *s, struct SerialBuf *ctx);
EXTERN void MsgTuneStatusConfirmUnpack(struct SerialBuf *ctx, struct MsgTuneStatusConfirm *s);

/*--------------------------- MsgSleepIndication --------------------------- */

#define     MsgSleepIndicationBits 56
#define     MsgSleepIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgSleepIndicationPack(s, ctx);
#define     MsgSleepIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSleepIndicationUnpack(ctx, s);
EXTERN void MsgSleepIndicationPack(struct MsgSleepIndication *s, struct SerialBuf *ctx);
EXTERN void MsgSleepIndicationUnpack(struct SerialBuf *ctx, struct MsgSleepIndication *s);

/*-------------------------MsgDeleteChannel-----------------------------------*/
/**
 Msg request the suppression of a channel
 OUT_MSG_REQ_DEL_CH.
 */

#define     MsgDeleteChannelBits 56
#define     MsgDeleteChannelPackInit(s, ctx)  SerialBufRestart(ctx); MsgDeleteChannelPack(s, ctx);
#define     MsgDeleteChannelUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDeleteChannelUnpack(ctx, s);
#define     MsgDeleteChannelPack(s, ctx)   MsgSleepIndicationPack((struct MsgSleepIndication *)(s), ctx)
#define     MsgDeleteChannelUnpack(ctx, s)   MsgSleepIndicationUnpack(ctx, (struct MsgSleepIndication *)(s))

/*-------------------------MsgCreateFilter-----------------------------------*/
/**
 Msg request the creation of a new filter
 OUT_MSG_REQ_CREATE_FILT.
 */

#define     MsgCreateFilterBits 72
#define     MsgCreateFilterPackInit(s, ctx)  SerialBufRestart(ctx); MsgCreateFilterPack(s, ctx);
#define     MsgCreateFilterUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCreateFilterUnpack(ctx, s);
EXTERN void MsgCreateFilterPack(struct MsgCreateFilter *s, struct SerialBuf *ctx);
EXTERN void MsgCreateFilterUnpack(struct SerialBuf *ctx, struct MsgCreateFilter *s);

/*-------------------------MsgDeleteFilter-----------------------------------*/
/**
 Msg request the suppression of a filter
 OUT_MSG_REQ_DEL_FILT.
 */

#define     MsgDeleteFilterBits 56
#define     MsgDeleteFilterPackInit(s, ctx)  SerialBufRestart(ctx); MsgDeleteFilterPack(s, ctx);
#define     MsgDeleteFilterUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDeleteFilterUnpack(ctx, s);
#define     MsgDeleteFilterPack(s, ctx)   MsgSleepIndicationPack((struct MsgSleepIndication *)(s), ctx)
#define     MsgDeleteFilterUnpack(ctx, s)   MsgSleepIndicationUnpack(ctx, (struct MsgSleepIndication *)(s))

/*-------------------------MsgCreateItem -----------------------------------*/
#define     MsgCreateItemBits 152
#define     MsgCreateItemPackInit(s, ctx)  SerialBufRestart(ctx); MsgCreateItemPack(s, ctx);
#define     MsgCreateItemUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCreateItemUnpack(ctx, s);
EXTERN void MsgCreateItemPack(struct MsgCreateItem *s, struct SerialBuf *ctx);
EXTERN void MsgCreateItemUnpack(struct SerialBuf *ctx, struct MsgCreateItem *s);

/*-------------------------MsgDeleteItem----------------------------------------*/
/**
 Msg request the suppression of a filter's item
 OUT_MSG_REQ_REM_TO_FILT
 */

#define     MsgDeleteItemBits 56
#define     MsgDeleteItemPackInit(s, ctx)  SerialBufRestart(ctx); MsgDeleteItemPack(s, ctx);
#define     MsgDeleteItemUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDeleteItemUnpack(ctx, s);
#define     MsgDeleteItemPack(s, ctx)   MsgSleepIndicationPack((struct MsgSleepIndication *)(s), ctx)
#define     MsgDeleteItemUnpack(ctx, s)   MsgSleepIndicationUnpack(ctx, (struct MsgSleepIndication *)(s))

/*--------------------------MsgAcknowledgeApi----------------------------------*/
/**
 Acknowledge API request.
 MSG_ACK_API identifier
 IN_MSG_ACK_DEL_CH, IN_MSG_ACK_CREATE_FILT, IN_MSG_ACK_DEL_FILT,
 IN_MSG_ACK_ADD_TO_FILT, IN_MSG_ACK_FREE_ITEM, IN_MSG_ACK_CAS_EN,
 IN_MSG_ACK_API_UP_EN
 */

#define     MsgAcknowledgeApiBits 64
#define     MsgAcknowledgeApiPackInit(s, ctx)  SerialBufRestart(ctx); MsgAcknowledgeApiPack(s, ctx);
#define     MsgAcknowledgeApiUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAcknowledgeApiUnpack(ctx, s);
#define     MsgAcknowledgeApiPack(s, ctx)   MsgRemoveFrontendPack((struct MsgRemoveFrontend *)(s), ctx)
#define     MsgAcknowledgeApiUnpack(ctx, s)   MsgRemoveFrontendUnpack(ctx, (struct MsgRemoveFrontend *)(s))

/*------------------------------- MsgSetHbm ------------------------------ */

#define     MsgSetHbmBits 64
#define     MsgSetHbmPackInit(s, ctx)  SerialBufRestart(ctx); MsgSetHbmPack(s, ctx);
#define     MsgSetHbmUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSetHbmUnpack(ctx, s);
EXTERN void MsgSetHbmPack(struct MsgSetHbm *s, struct SerialBuf *ctx);
EXTERN void MsgSetHbmUnpack(struct SerialBuf *ctx, struct MsgSetHbm *s);

/* ----------------------------- MsgRawTSMonitor-------------------------- */
/**
 */
#define     MsgRawTSMonitorBits 160
#define     MsgRawTSMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgRawTSMonitorPack(s, ctx);
#define     MsgRawTSMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRawTSMonitorUnpack(ctx, s);
EXTERN void MsgRawTSMonitorPack(struct MsgRawTSMonitor *s, struct SerialBuf *ctx);
EXTERN void MsgRawTSMonitorUnpack(struct SerialBuf *ctx, struct MsgRawTSMonitor *s);

/* ----------------------------- MsgMpeMonitor ------------------------------ */
/**
  Firmware send MPE monitoring information after frame reception is complete
  Be careful : do not change the order of the following information
*/
#define     MsgMpeMonitorBits 480
#define     MsgMpeMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgMpeMonitorPack(s, ctx);
#define     MsgMpeMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgMpeMonitorUnpack(ctx, s);
EXTERN void MsgMpeMonitorPack(struct MsgMpeMonitor *s, struct SerialBuf *ctx);
EXTERN void MsgMpeMonitorUnpack(struct SerialBuf *ctx, struct MsgMpeMonitor *s);

/*-------------------------------MsgMpeIfecMonitor------------------------------*/
/**
 Firmware send IFEC monitoring information (DVBSH standard)
 */
#define     MsgMpeIfecMonitorBits 576
#define     MsgMpeIfecMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgMpeIfecMonitorPack(s, ctx);
#define     MsgMpeIfecMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgMpeIfecMonitorUnpack(ctx, s);
EXTERN void MsgMpeIfecMonitorPack(struct MsgMpeIfecMonitor *s, struct SerialBuf *ctx);
EXTERN void MsgMpeIfecMonitorUnpack(struct SerialBuf *ctx, struct MsgMpeIfecMonitor *s);

/*--------------------------------MsgFicMonitor----------------------------------*/
 /**
  Fic avalaible packets is tested with the 16bits CRC
  */
#define     MsgFicMonitorBits 128
#define     MsgFicMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgFicMonitorPack(s, ctx);
#define     MsgFicMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgFicMonitorUnpack(ctx, s);
EXTERN void MsgFicMonitorPack(struct MsgFicMonitor *s, struct SerialBuf *ctx);
EXTERN void MsgFicMonitorUnpack(struct SerialBuf *ctx, struct MsgFicMonitor *s);

/*-------------------------------MsgMscPacketMonitor------------------------------*/
/**
 Firmware send Msc Packet monitoring information
 */
#define     MsgMscPacketMonitorBits 192
#define     MsgMscPacketMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgMscPacketMonitorPack(s, ctx);
#define     MsgMscPacketMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgMscPacketMonitorUnpack(ctx, s);
EXTERN void MsgMscPacketMonitorPack(struct MsgMscPacketMonitor *s, struct SerialBuf *ctx);
EXTERN void MsgMscPacketMonitorUnpack(struct SerialBuf *ctx, struct MsgMscPacketMonitor *s);

/*-------------------------------MsgMscPlusMonitor------------------------------*/
/**
 Firmware send DAB+ monitoring information
 */
#define     MsgMscPlusMonitorBits 192
#define     MsgMscPlusMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgMscPlusMonitorPack(s, ctx);
#define     MsgMscPlusMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgMscPlusMonitorUnpack(ctx, s);
#define     MsgMscPlusMonitorPack(s, ctx)   MsgMscPacketMonitorPack((struct MsgMscPacketMonitor *)(s), ctx)
#define     MsgMscPlusMonitorUnpack(ctx, s)   MsgMscPacketMonitorUnpack(ctx, (struct MsgMscPacketMonitor *)(s))

/* ---------------------------- MsgGetSignalMonit --------------------------------*/

#define     MsgGetSignalMonitBits 64
#define     MsgGetSignalMonitPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetSignalMonitPack(s, ctx);
#define     MsgGetSignalMonitUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetSignalMonitUnpack(ctx, s);
EXTERN void MsgGetSignalMonitPack(struct MsgGetSignalMonit *s, struct SerialBuf *ctx);
EXTERN void MsgGetSignalMonitUnpack(struct SerialBuf *ctx, struct MsgGetSignalMonit *s);

/* ---------------------------- MsgAckGetSignalMonit -----------------------------*/

#define     MsgAckGetSignalMonitBits 718
#define     MsgAckGetSignalMonitPackInit(s, ctx)  SerialBufRestart(ctx); MsgAckGetSignalMonitPack(s, ctx);
#define     MsgAckGetSignalMonitUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAckGetSignalMonitUnpack(ctx, s);
EXTERN void MsgAckGetSignalMonitPack(struct MsgAckGetSignalMonit *s, struct SerialBuf *ctx);
EXTERN void MsgAckGetSignalMonitUnpack(struct SerialBuf *ctx, struct MsgAckGetSignalMonit *s);


/* --------------------------------- MsgError -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */


#define     MsgErrorBits 192
#define     MsgErrorPackInit(s, ctx)  SerialBufRestart(ctx); MsgErrorPack(s, ctx);
#define     MsgErrorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgErrorUnpack(ctx, s);
EXTERN void MsgErrorPack(struct MsgError *s, struct SerialBuf *ctx);
EXTERN void MsgErrorUnpack(struct SerialBuf *ctx, struct MsgError *s);


/* --------------------------------- MsgVersion -----------------------------------*/
/*
 * Description : Host request the version of the firmware
 */
#define     MsgVersionBits 160
#define     MsgVersionPackInit(s, ctx)  SerialBufRestart(ctx); MsgVersionPack(s, ctx);
#define     MsgVersionUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgVersionUnpack(ctx, s);
EXTERN void MsgVersionPack(struct MsgVersion *s, struct SerialBuf *ctx);
EXTERN void MsgVersionUnpack(struct SerialBuf *ctx, struct MsgVersion *s);

/* ----------------------------------- MsgEvent -----------------------------------*/
/*
 * Description : Firmware sends event notification
 */
#define     MsgEventBits 48
#define     MsgEventPackInit(s, ctx)  SerialBufRestart(ctx); MsgEventPack(s, ctx);
#define     MsgEventUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgEventUnpack(ctx, s);
EXTERN void MsgEventPack(struct MsgEvent *s, struct SerialBuf *ctx);
EXTERN void MsgEventUnpack(struct SerialBuf *ctx, struct MsgEvent *s);

/* -------------------------------- MsgCmmbRsmMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB LDPC and RS monitoring information at end of RS matrix.
 */
#define     MsgCmmbRsmMonitorBits 224
#define     MsgCmmbRsmMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgCmmbRsmMonitorPack(s, ctx);
#define     MsgCmmbRsmMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCmmbRsmMonitorUnpack(ctx, s);
EXTERN void MsgCmmbRsmMonitorPack(struct MsgCmmbRsmMonitor *s, struct SerialBuf *ctx);
EXTERN void MsgCmmbRsmMonitorUnpack(struct SerialBuf *ctx, struct MsgCmmbRsmMonitor *s);


/* ----------------------------- MsgCmmbMfMonitor -----------------------------*/
/*
 * Description : Firmware sends CMMB MF information at end of each MF.
 */
#define     MsgCmmbMfMonitorBits 138
#define     MsgCmmbMfMonitorPackInit(s, ctx)  SerialBufRestart(ctx); MsgCmmbMfMonitorPack(s, ctx);
#define     MsgCmmbMfMonitorUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCmmbMfMonitorUnpack(ctx, s);
EXTERN void MsgCmmbMfMonitorPack(struct MsgCmmbMfMonitor *s, struct SerialBuf *ctx);
EXTERN void MsgCmmbMfMonitorUnpack(struct SerialBuf *ctx, struct MsgCmmbMfMonitor *s);


/* ----------------------------- MsgDebugHelp -----------------------------*/
/*
 * Set a Debug function of the embedded firmware.
 * Func:
 *    - 0: memory dumping
 *    - 1: ldpc discarder
 *    - 11: Enable Real Time Discarder. Value is the SNR
 *    - 12: dvb-sh discarded
 */
#define     MsgDebugHelpBits 64
#define     MsgDebugHelpPackInit(s, ctx)  SerialBufRestart(ctx); MsgDebugHelpPack(s, ctx);
#define     MsgDebugHelpUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDebugHelpUnpack(ctx, s);
EXTERN void MsgDebugHelpPack(struct MsgDebugHelp *s, struct SerialBuf *ctx);
EXTERN void MsgDebugHelpUnpack(struct SerialBuf *ctx, struct MsgDebugHelp *s);


/* ----------------------------- MsgPrint -----------------------------*/
/*
 * Message used to send a string to the host
 */
#define     MsgPrintBits 1632
#define     MsgPrintPackInit(s, ctx)  SerialBufRestart(ctx); MsgPrintPack(s, ctx);
#define     MsgPrintUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgPrintUnpack(ctx, s);
EXTERN void MsgPrintPack(struct MsgPrint *s, struct SerialBuf *ctx);
EXTERN void MsgPrintUnpack(struct SerialBuf *ctx, struct MsgPrint *s);

/* ----------------------------- MsgEnableTimeSlice -----------------------------*/
/*
 * enable/disable time-slicing for a dedicated stream
 */
#define     MsgEnableTimeSliceBits 56
#define     MsgEnableTimeSlicePackInit(s, ctx)  SerialBufRestart(ctx); MsgEnableTimeSlicePack(s, ctx);
#define     MsgEnableTimeSliceUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgEnableTimeSliceUnpack(ctx, s);
#define     MsgEnableTimeSlicePack(s, ctx)   MsgSleepIndicationPack((struct MsgSleepIndication *)(s), ctx)
#define     MsgEnableTimeSliceUnpack(ctx, s)   MsgSleepIndicationUnpack(ctx, (struct MsgSleepIndication *)(s))

/* --------------------------- MsgAbortTuneMonit ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */
#define     MsgAbortTuneMonitBits 32
#define     MsgAbortTuneMonitPackInit(s, ctx)  SerialBufRestart(ctx); MsgAbortTuneMonitPack(s, ctx);
#define     MsgAbortTuneMonitUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAbortTuneMonitUnpack(ctx, s);
#define     MsgAbortTuneMonitPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgAbortTuneMonitUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* --------------------------- MsgSetPidMode ---------------------------*/
/*
 * Abort a tune monit request on a user stream
 */
#define     MsgSetPidModeBits 41
#define     MsgSetPidModePackInit(s, ctx)  SerialBufRestart(ctx); MsgSetPidModePack(s, ctx);
#define     MsgSetPidModeUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSetPidModeUnpack(ctx, s);
EXTERN void MsgSetPidModePack(struct MsgSetPidMode *s, struct SerialBuf *ctx);
EXTERN void MsgSetPidModeUnpack(struct SerialBuf *ctx, struct MsgSetPidMode *s);


/* --------------------------- MsgCasEnable ---------------------------*/
/*
 * Enable/disable CAS system
 */
#define     MsgCasEnableBits 33
#define     MsgCasEnablePackInit(s, ctx)  SerialBufRestart(ctx); MsgCasEnablePack(s, ctx);
#define     MsgCasEnableUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgCasEnableUnpack(ctx, s);
EXTERN void MsgCasEnablePack(struct MsgCasEnable *s, struct SerialBuf *ctx);
EXTERN void MsgCasEnableUnpack(struct SerialBuf *ctx, struct MsgCasEnable *s);


/* ------------------------ MsgAHBAccess ----------------------------*/
/*
 * Internal access done by firmware but requested by the host
 */
#define     MsgAHBAccessBits 97
#define     MsgAHBAccessPackInit(s, ctx)  SerialBufRestart(ctx); MsgAHBAccessPack(s, ctx);
#define     MsgAHBAccessUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAHBAccessUnpack(ctx, s);
EXTERN void MsgAHBAccessPack(struct MsgAHBAccess *s, struct SerialBuf *ctx);
EXTERN void MsgAHBAccessUnpack(struct SerialBuf *ctx, struct MsgAHBAccess *s);

/* ------------------------ MsgHostReflexInit ----------------------------*/
/*
 * Address in emb of reflex context needed for emulation
 */
#define     MsgHostReflexInitBits 64
#define     MsgHostReflexInitPackInit(s, ctx)  SerialBufRestart(ctx); MsgHostReflexInitPack(s, ctx);
#define     MsgHostReflexInitUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHostReflexInitUnpack(ctx, s);
#define     MsgHostReflexInitPack(s, ctx)   MsgSetHbmPack((struct MsgSetHbm *)(s), ctx)
#define     MsgHostReflexInitUnpack(ctx, s)   MsgSetHbmUnpack(ctx, (struct MsgSetHbm *)(s))

/* ------------------------ MsgHostReflexStep ----------------------------*/
/*
 * Command to advance one step in emulation
 */
#define     MsgHostReflexStepBits 32
#define     MsgHostReflexStepPackInit(s, ctx)  SerialBufRestart(ctx); MsgHostReflexStepPack(s, ctx);
#define     MsgHostReflexStepUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHostReflexStepUnpack(ctx, s);
#define     MsgHostReflexStepPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgHostReflexStepUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* ------------------------ MsgHostReflexSetTsConfig ----------------------------*/
/*
 * Each time time slot configuration changes, warn the host
 */
#define     MsgHostReflexSetTsConfigBits 416
#define     MsgHostReflexSetTsConfigPackInit(s, ctx)  SerialBufRestart(ctx); MsgHostReflexSetTsConfigPack(s, ctx);
#define     MsgHostReflexSetTsConfigUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHostReflexSetTsConfigUnpack(ctx, s);
EXTERN void MsgHostReflexSetTsConfigPack(struct MsgHostReflexSetTsConfig *s, struct SerialBuf *ctx);
EXTERN void MsgHostReflexSetTsConfigUnpack(struct SerialBuf *ctx, struct MsgHostReflexSetTsConfig *s);

/* ------------------------ MsgHostReflexSetSvcParam ----------------------------*/
/*
 * Each time service parameters changes, warn the host
 */
#define     MsgHostReflexSetSvcParamBits 128
#define     MsgHostReflexSetSvcParamPackInit(s, ctx)  SerialBufRestart(ctx); MsgHostReflexSetSvcParamPack(s, ctx);
#define     MsgHostReflexSetSvcParamUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHostReflexSetSvcParamUnpack(ctx, s);
#define     MsgHostReflexSetSvcParamPack(s, ctx)   MsgFicMonitorPack((struct MsgFicMonitor *)(s), ctx)
#define     MsgHostReflexSetSvcParamUnpack(ctx, s)   MsgFicMonitorUnpack(ctx, (struct MsgFicMonitor *)(s))

/* --------------------------- MsgGpioConfig ------------------------------*/

#define     MsgGpioConfigBits 832
#define     MsgGpioConfigPackInit(s, ctx)  SerialBufRestart(ctx); MsgGpioConfigPack(s, ctx);
#define     MsgGpioConfigUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGpioConfigUnpack(ctx, s);
EXTERN void MsgGpioConfigPack(struct MsgGpioConfig *s, struct SerialBuf *ctx);
EXTERN void MsgGpioConfigUnpack(struct SerialBuf *ctx, struct MsgGpioConfig *s);

/* --------------------------- MsgLayoutConfig ------------------------------*/
#define     MsgLayoutConfigBits 547
#define     MsgLayoutConfigPackInit(s, ctx)  SerialBufRestart(ctx); MsgLayoutConfigPack(s, ctx);
#define     MsgLayoutConfigUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgLayoutConfigUnpack(ctx, s);
EXTERN void MsgLayoutConfigPack(struct MsgLayoutConfig *s, struct SerialBuf *ctx);
EXTERN void MsgLayoutConfigUnpack(struct SerialBuf *ctx, struct MsgLayoutConfig *s);


/* --------------------------- MsgAckLayoutConfig ------------------------------*/

#define     MsgAckLayoutConfigBits 64
#define     MsgAckLayoutConfigPackInit(s, ctx)  SerialBufRestart(ctx); MsgAckLayoutConfigPack(s, ctx);
#define     MsgAckLayoutConfigUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAckLayoutConfigUnpack(ctx, s);
#define     MsgAckLayoutConfigPack(s, ctx)   MsgSetHbmPack((struct MsgSetHbm *)(s), ctx)
#define     MsgAckLayoutConfigUnpack(ctx, s)   MsgSetHbmUnpack(ctx, (struct MsgSetHbm *)(s))

/* --------------------------- MsgAckLayoutReset ------------------------------*/

#define     MsgLayoutResetBits 32
#define     MsgLayoutResetPackInit(s, ctx)  SerialBufRestart(ctx); MsgLayoutResetPack(s, ctx);
#define     MsgLayoutResetUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgLayoutResetUnpack(ctx, s);
#define     MsgLayoutResetPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgLayoutResetUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* ----------------------------- MsgGetLayoutInfo -----------------------------*/
/*
 * Request information concerning the layout
 */
#define     MsgGetLayoutInfoBits 32
#define     MsgGetLayoutInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetLayoutInfoPack(s, ctx);
#define     MsgGetLayoutInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetLayoutInfoUnpack(ctx, s);
#define     MsgGetLayoutInfoPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgGetLayoutInfoUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* ----------------------------- MsgLayoutInfo --------------------------------*/
/*
 * Reply information concerning the layout
 */
#define     MsgLayoutInfoBits 128
#define     MsgLayoutInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgLayoutInfoPack(s, ctx);
#define     MsgLayoutInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgLayoutInfoUnpack(ctx, s);
#define     MsgLayoutInfoPack(s, ctx)   MsgFicMonitorPack((struct MsgFicMonitor *)(s), ctx)
#define     MsgLayoutInfoUnpack(ctx, s)   MsgFicMonitorUnpack(ctx, (struct MsgFicMonitor *)(s))

/* ----------------------------- MsgGetFrontendInfo ----------------------------*/
/*
 * Request information concerning a specific frontend
 */
#define     MsgGetFrontendInfoBits 40
#define     MsgGetFrontendInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetFrontendInfoPack(s, ctx);
#define     MsgGetFrontendInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetFrontendInfoUnpack(ctx, s);
EXTERN void MsgGetFrontendInfoPack(struct MsgGetFrontendInfo *s, struct SerialBuf *ctx);
EXTERN void MsgGetFrontendInfoUnpack(struct SerialBuf *ctx, struct MsgGetFrontendInfo *s);

/* ----------------------------- MsgFrontendInfo -------------------------------*/
/*
 * Request information concerning a specific frontend
 */
#define     MsgFrontendInfoBits 1256
#define     MsgFrontendInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgFrontendInfoPack(s, ctx);
#define     MsgFrontendInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgFrontendInfoUnpack(ctx, s);
EXTERN void MsgFrontendInfoPack(struct MsgFrontendInfo *s, struct SerialBuf *ctx);
EXTERN void MsgFrontendInfoUnpack(struct SerialBuf *ctx, struct MsgFrontendInfo *s);

/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Request information concerning a specific stream
 */
#define     MsgGetStreamInfoBits 64
#define     MsgGetStreamInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetStreamInfoPack(s, ctx);
#define     MsgGetStreamInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetStreamInfoUnpack(ctx, s);
#define     MsgGetStreamInfoPack(s, ctx)   MsgSetHbmPack((struct MsgSetHbm *)(s), ctx)
#define     MsgGetStreamInfoUnpack(ctx, s)   MsgSetHbmUnpack(ctx, (struct MsgSetHbm *)(s))

/* ----------------------------- MsgGetStreamInfo ------------------------------*/
/*
 * Reply information concerning a stream
 */

#define     MsgStreamInfoBits 1296
#define     MsgStreamInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgStreamInfoPack(s, ctx);
#define     MsgStreamInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgStreamInfoUnpack(ctx, s);
EXTERN void MsgStreamInfoPack(struct MsgStreamInfo *s, struct SerialBuf *ctx);
EXTERN void MsgStreamInfoUnpack(struct SerialBuf *ctx, struct MsgStreamInfo *s);


/* ----------------------------- MsgDownloadIndication ------------------------------*/
/*
 * Request for downloading a file
 */
#define     MsgDownloadIndicationBits 576
#define     MsgDownloadIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgDownloadIndicationPack(s, ctx);
#define     MsgDownloadIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDownloadIndicationUnpack(ctx, s);
EXTERN void MsgDownloadIndicationPack(struct MsgDownloadIndication *s, struct SerialBuf *ctx);
EXTERN void MsgDownloadIndicationUnpack(struct SerialBuf *ctx, struct MsgDownloadIndication *s);


/* ----------------------------- MsgDownloadConfirm ------------------------------*/

/*
 * confirmation of file download
 */
#define     MsgDownloadConfirmBits 126
#define     MsgDownloadConfirmPackInit(s, ctx)  SerialBufRestart(ctx); MsgDownloadConfirmPack(s, ctx);
#define     MsgDownloadConfirmUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDownloadConfirmUnpack(ctx, s);
EXTERN void MsgDownloadConfirmPack(struct MsgDownloadConfirm *s, struct SerialBuf *ctx);
EXTERN void MsgDownloadConfirmUnpack(struct SerialBuf *ctx, struct MsgDownloadConfirm *s);

/* ----------------------------- MsgApiRequest ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer to the firmware and reecive one in return.
 * Each MsgApiRequest must be acknowledged before sending a new one.
 */


#define     MsgApiRequestBits 128
#define     MsgApiRequestPackInit(s, ctx)  SerialBufRestart(ctx); MsgApiRequestPack(s, ctx);
#define     MsgApiRequestUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgApiRequestUnpack(ctx, s);
EXTERN void MsgApiRequestPack(struct MsgApiRequest *s, struct SerialBuf *ctx);
EXTERN void MsgApiRequestUnpack(struct SerialBuf *ctx, struct MsgApiRequest *s);

/* ----------------------------- MsgApiAnswer ------------------------------*/
/*
 * This is a reply to the MsgApiRequest, with the possibility to allow buffer retreival fron embedded buffer to
 * host buffer.
 */
#define     MsgApiAnswerBits 128
#define     MsgApiAnswerPackInit(s, ctx)  SerialBufRestart(ctx); MsgApiAnswerPack(s, ctx);
#define     MsgApiAnswerUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgApiAnswerUnpack(ctx, s);
#define     MsgApiAnswerPack(s, ctx)   MsgFicMonitorPack((struct MsgFicMonitor *)(s), ctx)
#define     MsgApiAnswerUnpack(ctx, s)   MsgFicMonitorUnpack(ctx, (struct MsgFicMonitor *)(s))

/* ----------------------------- MsgApiUpEnable ------------------------------*/
/*
 * This message is used to enable/disable the fact that the firmware can send msg api information to the user
 * without any request from it. In that case, the user is notified from the sdk through a message callback mechanism, with the
 * possibility to get also data buffers.
 */
#define     MsgApiUpEnableBits 64
#define     MsgApiUpEnablePackInit(s, ctx)  SerialBufRestart(ctx); MsgApiUpEnablePack(s, ctx);
#define     MsgApiUpEnableUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgApiUpEnableUnpack(ctx, s);
#define     MsgApiUpEnablePack(s, ctx)   MsgRemoveFrontendPack((struct MsgRemoveFrontend *)(s), ctx)
#define     MsgApiUpEnableUnpack(ctx, s)   MsgRemoveFrontendUnpack(ctx, (struct MsgRemoveFrontend *)(s))

/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * User message encapsulation. Possibility to send a buffer from the firwmare to the host is permitted (see MsgApiUpEnable)
 * Each MsgApiUp must be acknoledged by a MsgApiUpAck whose role is to free buffer allocated at RxAddr.
 */
#define     MsgApiUpBits 128
#define     MsgApiUpPackInit(s, ctx)  SerialBufRestart(ctx); MsgApiUpPack(s, ctx);
#define     MsgApiUpUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgApiUpUnpack(ctx, s);
#define     MsgApiUpPack(s, ctx)   MsgApiRequestPack((struct MsgApiRequest *)(s), ctx)
#define     MsgApiUpUnpack(ctx, s)   MsgApiRequestUnpack(ctx, (struct MsgApiRequest *)(s))

/* ----------------------------- MsgApiUp ------------------------------*/
/*
 * This message is used to free the memory allocated by firmware when sending the MsgApiUp message
 */
#define     MsgApiUpAckBits 64
#define     MsgApiUpAckPackInit(s, ctx)  SerialBufRestart(ctx); MsgApiUpAckPack(s, ctx);
#define     MsgApiUpAckUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgApiUpAckUnpack(ctx, s);
#define     MsgApiUpAckPack(s, ctx)   MsgSetHbmPack((struct MsgSetHbm *)(s), ctx)
#define     MsgApiUpAckUnpack(ctx, s)   MsgSetHbmUnpack(ctx, (struct MsgSetHbm *)(s))

/* ------------------------------ MsgOctoInit --------------------------*/


#define     MsgOctoInitBits 64
#define     MsgOctoInitPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoInitPack(s, ctx);
#define     MsgOctoInitUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoInitUnpack(ctx, s);
EXTERN void MsgOctoInitPack(struct MsgOctoInit *s, struct SerialBuf *ctx);
EXTERN void MsgOctoInitUnpack(struct SerialBuf *ctx, struct MsgOctoInit *s);

/* ------------------------- MsgOctoUtilsCheckSum ----------------------*/
#define     MsgOctoUtilsCheckSumBits 96
#define     MsgOctoUtilsCheckSumPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoUtilsCheckSumPack(s, ctx);
#define     MsgOctoUtilsCheckSumUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoUtilsCheckSumUnpack(ctx, s);
EXTERN void MsgOctoUtilsCheckSumPack(struct MsgOctoUtilsCheckSum *s, struct SerialBuf *ctx);
EXTERN void MsgOctoUtilsCheckSumUnpack(struct SerialBuf *ctx, struct MsgOctoUtilsCheckSum *s);


/* --------------------------- MsgOctoRwOctopus --------------------------*/
#define     MsgOctoRwOctopusBits 160
#define     MsgOctoRwOctopusPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoRwOctopusPack(s, ctx);
#define     MsgOctoRwOctopusUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoRwOctopusUnpack(ctx, s);
EXTERN void MsgOctoRwOctopusPack(struct MsgOctoRwOctopus *s, struct SerialBuf *ctx);
EXTERN void MsgOctoRwOctopusUnpack(struct SerialBuf *ctx, struct MsgOctoRwOctopus *s);

/* --------------------------- MsgHostOctoInit --------------------------*/
/*
 * Indicates the host where to find the data buffer inside chipset memory
 */
#define     MsgHostOctoInitBits 96
#define     MsgHostOctoInitPackInit(s, ctx)  SerialBufRestart(ctx); MsgHostOctoInitPack(s, ctx);
#define     MsgHostOctoInitUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHostOctoInitUnpack(ctx, s);
#define     MsgHostOctoInitPack(s, ctx)   MsgOctoUtilsCheckSumPack((struct MsgOctoUtilsCheckSum *)(s), ctx)
#define     MsgHostOctoInitUnpack(ctx, s)   MsgOctoUtilsCheckSumUnpack(ctx, (struct MsgOctoUtilsCheckSum *)(s))

/* --------------------------- MsgHostOctoRwOctopus ----------------------*/
#define     MsgHostOctoRwOctopusBits 64
#define     MsgHostOctoRwOctopusPackInit(s, ctx)  SerialBufRestart(ctx); MsgHostOctoRwOctopusPack(s, ctx);
#define     MsgHostOctoRwOctopusUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgHostOctoRwOctopusUnpack(ctx, s);
#define     MsgHostOctoRwOctopusPack(s, ctx)   MsgOctoInitPack((struct MsgOctoInit *)(s), ctx)
#define     MsgHostOctoRwOctopusUnpack(ctx, s)   MsgOctoInitUnpack(ctx, (struct MsgOctoInit *)(s))

/* --------------------------- MsgWave -----------------------------------*/
#define     MsgWaveBits 32
#define     MsgWavePackInit(s, ctx)  SerialBufRestart(ctx); MsgWavePack(s, ctx);
#define     MsgWaveUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgWaveUnpack(ctx, s);
#define     MsgWavePack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgWaveUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* --------------------------- MsgIntDebug -------------------------------*/
#define     MsgIntDebugBits 32
#define     MsgIntDebugPackInit(s, ctx)  SerialBufRestart(ctx); MsgIntDebugPack(s, ctx);
#define     MsgIntDebugUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgIntDebugUnpack(ctx, s);
#define     MsgIntDebugPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgIntDebugUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

/* --------------------------- MsgProfile --------------------------------*/
#define     MsgProfileBits 32
#define     MsgProfilePackInit(s, ctx)  SerialBufRestart(ctx); MsgProfilePack(s, ctx);
#define     MsgProfileUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgProfileUnpack(ctx, s);
#define     MsgProfilePack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgProfileUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))


/* --------------------------- debugCmd ----------------------------------*/

/* Mode */
#define     MsgSlaveDataBits 217
#define     MsgSlaveDataPackInit(s, ctx)  SerialBufRestart(ctx); MsgSlaveDataPack(s, ctx);
#define     MsgSlaveDataUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSlaveDataUnpack(ctx, s);
EXTERN void MsgSlaveDataPack(struct MsgSlaveData *s, struct SerialBuf *ctx);
EXTERN void MsgSlaveDataUnpack(struct SerialBuf *ctx, struct MsgSlaveData *s);

#define     MsgSlaveControlBits 89
#define     MsgSlaveControlPackInit(s, ctx)  SerialBufRestart(ctx); MsgSlaveControlPack(s, ctx);
#define     MsgSlaveControlUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSlaveControlUnpack(ctx, s);
EXTERN void MsgSlaveControlPack(struct MsgSlaveControl *s, struct SerialBuf *ctx);
EXTERN void MsgSlaveControlUnpack(struct SerialBuf *ctx, struct MsgSlaveControl *s);

/* MsgSetBasicLayoutInfo */
#define     MsgSetBasicLayoutInfoBits 70
#define     MsgSetBasicLayoutInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgSetBasicLayoutInfoPack(s, ctx);
#define     MsgSetBasicLayoutInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSetBasicLayoutInfoUnpack(ctx, s);
EXTERN void MsgSetBasicLayoutInfoPack(struct MsgSetBasicLayoutInfo *s, struct SerialBuf *ctx);
EXTERN void MsgSetBasicLayoutInfoUnpack(struct SerialBuf *ctx, struct MsgSetBasicLayoutInfo *s);

/* --------------------------- MsgAddSlaveIndication ----------------------------------*/
#define     MsgAddSlaveDeviceBits 94
#define     MsgAddSlaveDevicePackInit(s, ctx)  SerialBufRestart(ctx); MsgAddSlaveDevicePack(s, ctx);
#define     MsgAddSlaveDeviceUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAddSlaveDeviceUnpack(ctx, s);
EXTERN void MsgAddSlaveDevicePack(struct MsgAddSlaveDevice *s, struct SerialBuf *ctx);
EXTERN void MsgAddSlaveDeviceUnpack(struct SerialBuf *ctx, struct MsgAddSlaveDevice *s);

/* --------------------------- MsgAddSlaveConfirm ----------------------------------*/
#define     MsgAddSlaveConfirmBits 64
#define     MsgAddSlaveConfirmPackInit(s, ctx)  SerialBufRestart(ctx); MsgAddSlaveConfirmPack(s, ctx);
#define     MsgAddSlaveConfirmUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAddSlaveConfirmUnpack(ctx, s);
EXTERN void MsgAddSlaveConfirmPack(struct MsgAddSlaveConfirm *s, struct SerialBuf *ctx);
EXTERN void MsgAddSlaveConfirmUnpack(struct SerialBuf *ctx, struct MsgAddSlaveConfirm *s);

/* ---------------------------- MsgStopMacHandover --------------------------------*/
#define     MsgStopMacHandoverBits 40
#define     MsgStopMacHandoverPackInit(s, ctx)  SerialBufRestart(ctx); MsgStopMacHandoverPack(s, ctx);
#define     MsgStopMacHandoverUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgStopMacHandoverUnpack(ctx, s);
#define     MsgStopMacHandoverPack(s, ctx)   MsgGetFrontendInfoPack((struct MsgGetFrontendInfo *)(s), ctx)
#define     MsgStopMacHandoverUnpack(ctx, s)   MsgGetFrontendInfoUnpack(ctx, (struct MsgGetFrontendInfo *)(s))

/* ---------------------------- MsgStartMacHandover --------------------------------*/
#define     MsgStartMacHandoverBits 72
#define     MsgStartMacHandoverPackInit(s, ctx)  SerialBufRestart(ctx); MsgStartMacHandoverPack(s, ctx);
#define     MsgStartMacHandoverUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgStartMacHandoverUnpack(ctx, s);
EXTERN void MsgStartMacHandoverPack(struct MsgStartMacHandover *s, struct SerialBuf *ctx);
EXTERN void MsgStartMacHandoverUnpack(struct SerialBuf *ctx, struct MsgStartMacHandover *s);

/* ---------------------------- MsgWakeUpMacHandover --------------------------------*/
#define     MsgWakeUpMacHandoverBits 128
#define     MsgWakeUpMacHandoverPackInit(s, ctx)  SerialBufRestart(ctx); MsgWakeUpMacHandoverPack(s, ctx);
#define     MsgWakeUpMacHandoverUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgWakeUpMacHandoverUnpack(ctx, s);
#define     MsgWakeUpMacHandoverPack(s, ctx)   MsgFicMonitorPack((struct MsgFicMonitor *)(s), ctx)
#define     MsgWakeUpMacHandoverUnpack(ctx, s)   MsgFicMonitorUnpack(ctx, (struct MsgFicMonitor *)(s))

/*------------------------------MsgFlashProgram-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_FLASH_PROGRAM.
 */
#define     MsgFlashProgramBits 416
#define     MsgFlashProgramPackInit(s, ctx)  SerialBufRestart(ctx); MsgFlashProgramPack(s, ctx);
#define     MsgFlashProgramUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgFlashProgramUnpack(ctx, s);
EXTERN void MsgFlashProgramPack(struct MsgFlashProgram *s, struct SerialBuf *ctx);
EXTERN void MsgFlashProgramUnpack(struct SerialBuf *ctx, struct MsgFlashProgram *s);


/*---------------------------MsgFlashProgramDone-----------------------------------*/
/**
 Msg to have a new stream
 IN_MSG_FLASH_PROGRAM_DONE.
 */
#define     MsgFlashProgramDoneBits 96
#define     MsgFlashProgramDonePackInit(s, ctx)  SerialBufRestart(ctx); MsgFlashProgramDonePack(s, ctx);
#define     MsgFlashProgramDoneUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgFlashProgramDoneUnpack(ctx, s);
EXTERN void MsgFlashProgramDonePack(struct MsgFlashProgramDone *s, struct SerialBuf *ctx);
EXTERN void MsgFlashProgramDoneUnpack(struct SerialBuf *ctx, struct MsgFlashProgramDone *s);

/* ----------------------------- MsgGetInfo -----------------------------*/
/*
 * Request information
 */
#define     MsgInfoRequestBits 64
#define     MsgInfoRequestPackInit(s, ctx)  SerialBufRestart(ctx); MsgInfoRequestPack(s, ctx);
#define     MsgInfoRequestUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgInfoRequestUnpack(ctx, s);
#define     MsgInfoRequestPack(s, ctx)   MsgSetHbmPack((struct MsgSetHbm *)(s), ctx)
#define     MsgInfoRequestUnpack(ctx, s)   MsgSetHbmUnpack(ctx, (struct MsgSetHbm *)(s))

/*
 * Reply information data
 */
#define     MsgInfoDataBits 92
#define     MsgInfoDataPackInit(s, ctx)  SerialBufRestart(ctx); MsgInfoDataPack(s, ctx);
#define     MsgInfoDataUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgInfoDataUnpack(ctx, s);
EXTERN void MsgInfoDataPack(struct MsgInfoData *s, struct SerialBuf *ctx);
EXTERN void MsgInfoDataUnpack(struct SerialBuf *ctx, struct MsgInfoData *s);

/* ----------------------------- MsgSetInfo -----------------------------*/
/*
 * Set information
 */
#define     MsgInfoSetBits 84
#define     MsgInfoSetPackInit(s, ctx)  SerialBufRestart(ctx); MsgInfoSetPack(s, ctx);
#define     MsgInfoSetUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgInfoSetUnpack(ctx, s);
EXTERN void MsgInfoSetPack(struct MsgInfoSet *s, struct SerialBuf *ctx);
EXTERN void MsgInfoSetUnpack(struct SerialBuf *ctx, struct MsgInfoSet *s);

/*
 * Get set information status
 */
#define     MsgInfoSetStatusBits 40
#define     MsgInfoSetStatusPackInit(s, ctx)  SerialBufRestart(ctx); MsgInfoSetStatusPack(s, ctx);
#define     MsgInfoSetStatusUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgInfoSetStatusUnpack(ctx, s);
#define     MsgInfoSetStatusPack(s, ctx)   MsgGetFrontendInfoPack((struct MsgGetFrontendInfo *)(s), ctx)
#define     MsgInfoSetStatusUnpack(ctx, s)   MsgGetFrontendInfoUnpack(ctx, (struct MsgGetFrontendInfo *)(s))


#endif /*__MSG_TYPE_MAC__*/


