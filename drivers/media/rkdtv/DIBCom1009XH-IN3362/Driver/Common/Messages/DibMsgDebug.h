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

#ifndef __MSG_TYPE_DEBUG__
#define __MSG_TYPE_DEBUG__

#ifdef __cplusplus
#define EXTERN  extern "C"
#else
#define EXTERN
#endif


/* Max message length */
#define MAX_MSG_DEBUG_BITS             480

/* Input message identifiers */
#define IN_MSG_DEBUG_ACK_INDICATION    0
#define IN_MSG_OCTO_READ_MEM_INDICATION 1
#define IN_MSG_REGISTER_ACCESS_ACK     2
#define IN_MSG_REGISTER_ENABLE_PERIODIC_READ_ACK 3
#define IN_MSG_FRONTEND_GET_INFO_IND   4
#define IN_MSG_GET_NUMBER_CHIP_IND     6
#define IN_MSG_PER_READ_VALUE          7
#define IN_MSG_GET_CHIP_INFO_IND       9
#define IN_MSG_GET_SDK_INFO_IND        10
#define IN_MSG_MONITOR_DLL             12
#define IN_MSG_GET_MEM_BLOCK_TABLE_IND 13
#define MAX_MSG_DEBUG_IN               14

/* Output message identifiers */
#define OUT_MSG_OCTO_SETTRACELEVEL     0
#define OUT_MSG_OCTO_ENABLE_DUMP       1
#define OUT_MSG_OCTO_ENABLE_GET        2
#define OUT_MSG_OCTO_RESET             3
#define OUT_MSG_OCTO_INITIALIZE_DESC   4
#define OUT_MSG_OCTO_WRITE_MEM         5
#define OUT_MSG_OCTO_READ_MEM          6
#define OUT_MSG_REGISTER_ACCESS        7
#define OUT_MSG_REGISTER_ENABLE_PERIODIC_READ 8
#define OUT_MSG_REGISTER_DISABLE_PERIODIC_READ 9
#define OUT_MSG_FRONTEND_GET_INFO      10
#define OUT_MSG_ACQUISITION_MODE_CTRL  11
#define OUT_MSG_TUNER_AGC_FREEZE       12
#define OUT_MSG_GET_NUMBER_CHIP        14
#define OUT_MSG_GET_CHIP_INFO          16
#define OUT_MSG_GET_SDK_INFO           17
#define OUT_MSG_SET_CLOCK_FREQUENCY    18
#define OUT_MSG_MONITOR_DLL            19
#define OUT_MSG_GET_MEM_BLOCK_TABLE    20
#define MAX_MSG_DEBUG_OUT              21



#include "DibMsgTypes.h"
#include "DibMsgGeneric.h"
#include "DibMsgFields.h"



/* Generic acknowledge for Debug messages */
struct MsgDebugAcknowledge
{
   struct MsgHeader Head;
   uint8_t idMsgOrigin;
};
/*------------------------------MsgOctoSetTraceLevel-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_OCTO_SETTRACELEVEL
 */
struct MsgOctoSetTraceLevel
{
   struct MsgHeader Head;
   int16_t Level;
};/*------------------------------MsgOctoEnableDump-----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */
struct MsgOctoEnableDump
{
   struct MsgHeader Head;
   uint8_t Enable;
};
/*------------------------------MsgOctoEnableGet----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */
struct MsgOctoEnableGet
{
   struct MsgHeader Head;
   uint8_t Enable;
};
struct MsgOctoReset
{
   struct MsgHeader Head;
};
enum OTTransferType
{
   eOT_MPEGTS = 0,
   eOT_DMA = 1,
};
struct MsgOctoInitializeDesc
{
   struct MsgHeader Head;
   enum OTTransferType TransType;
};
struct MsgOctoWriteMem
{
   struct MsgHeader Head;
   uint16_t Address;
   int16_t Value;
};
struct MsgOctoReadMem
{
   struct MsgHeader Head;
   uint16_t Address;
};
struct MsgOctoReadMemIndication
{
   struct MsgHeader Head;
   int16_t Value;
};
/* Access one "Net"/Register */
struct MsgRegisterAccess
{
   struct MsgHeader Head;
   uint8_t ReadWrite;
   uint32_t Address;
   uint8_t Offset;
   uint8_t Bits;
   uint8_t AccessType;
   uint32_t Msb;
   uint32_t Lsb;
};
/* This message is sent when the write or read is done
 * Msb, Lsb are used for the value read even if a write was done
 */
struct MsgRegisterAccessAck
{
   struct MsgHeader Head;
   uint32_t Msb;
   uint32_t Lsb;
   uint32_t TimeStamp;
};
struct MsgRegisterEnablePeriodicgRead
{
   struct MsgHeader Head;
   uint32_t Address;
   uint16_t ByteCount;
   uint32_t IntervalMsec;
   uint8_t AccessType;
};
/* this define is used in the ID-field of MsgRegisterEnablePeriodicReadAck and MsgRegisterDisablePeriodicRead */
#define REGISTER_PERIODIC_READ_ID_FAILED_OR_ALL 65535

struct MsgRegisterEnablePeriodicReadAck
{
   struct MsgHeader Head;
   uint16_t Id;
   uint32_t Address;
};
struct MsgRegisterDisablePeriodicRead
{
   struct MsgHeader Head;
   uint16_t Id;
};
struct MsgFrontendGetInfo
{
   struct MsgHeader Head;
};
struct MsgFrontendGetInfoIndication
{
   struct MsgHeader Head;
   uint32_t bandwidth_kHz;
   uint8_t standard;
   uint8_t FFT;
   uint32_t sampling_frequency_hz;
   uint8_t front_end;
};
struct MsgGetNumberOfChips
{
   struct MsgHeader Head;
};
struct MsgGetNumberOfChipsIndication
{
   struct MsgHeader Head;
   uint8_t Nb;
};
struct MsgPeriodicReadingValue
{
   struct MsgHeader Head;
   uint8_t Size;
   uint32_t Address;
   uint32_t TimeStamp;
   uint8_t Buffer[8];
};
enum AcquisitionType
{
   ACQUISITION_MODE_ADC = 0,
   ACQUISITION_MODE_ANALOG_FE = 1,
   ACQUISITION_MODE_OCTOPUS = 2,
   ACQUISITION_MODE_REFLEX = 3,
   ACQUISITION_MODE_DIVERSITY = 4,
};
struct MsgAcquisitionModeCtrl
{
   struct MsgHeader Head;
   uint8_t Enable;
   enum AcquisitionType Type;
/* Passthru enables the forwarding of the data received from a slave-device (mode as defined in Type) on this device */
   uint8_t Passthru;
};
struct MsgTunerAgcFreeze
{
   struct MsgHeader Head;
   uint8_t Enable;
   uint16_t AntennaId;
};
struct MsgGetChipInfoIndication
{
   struct MsgHeader Head;
   uint32_t TimeStamp;
   uint16_t ChipVer;
   uint32_t HalVer;
   uint32_t HalVerExt[8];
   uint32_t TunerVers;
   uint8_t NbTuner;
   uint32_t OffsetTuner;
   uint32_t OffsetSPAL;
};
struct MsgGetChipInfo
{
   struct MsgHeader Head;
};
struct MsgSDKInfoIndication
{
   struct MsgHeader Head;
   uint32_t SDKVer;
   uint8_t Nb;
};
struct MsgSDKInfo
{
   struct MsgHeader Head;
};
enum ClockFrequencyType
{
   CLOCK_FREQUENCY_OCTOPUS = 0,
   CLOCK_FREQUENCY_REFLEX = 1,
};
struct OCTOPUSClockParameters
{
   uint32_t OctoClk;
};
struct REFLEXCLockParameters
{
   uint32_t FemtoClk;
   uint32_t CceClk;
};
union ClockFrequencyParameters
{
   struct OCTOPUSClockParameters Octopus;
   struct REFLEXCLockParameters Reflex;
};
struct MsgSetClockFrequency
{
   struct MsgHeader Head;
   enum ClockFrequencyType Type;
   union ClockFrequencyParameters Param;
};
struct MsgMonitorDll
{
   struct MsgHeader Head;
   uint32_t Ifreq;
   uint32_t o_delay;
};
struct MsgGetMemBlockTable
{
   struct MsgHeader Head;
   uint8_t Enable;
};
struct MemBlockParameters
{
   uint32_t MaxIdx;
   uint32_t BaseAddress;
   uint32_t SmallBlockSize;
   uint32_t LargeBlockSize;
   uint8_t AllocatedBlocks[32];
};
struct MsgGetMemBlockTableAck
{
   struct MsgHeader Head;
   uint32_t Chip;
   uint32_t Time;
   struct MemBlockParameters Data;
};






/* Generic acknowledge for Debug messages */
#define     MsgDebugAcknowledgeBits 40
#define     MsgDebugAcknowledgePackInit(s, ctx)  SerialBufRestart(ctx); MsgDebugAcknowledgePack(s, ctx);
#define     MsgDebugAcknowledgeUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgDebugAcknowledgeUnpack(ctx, s);
EXTERN void MsgDebugAcknowledgePack(struct MsgDebugAcknowledge *s, struct SerialBuf *ctx);
EXTERN void MsgDebugAcknowledgeUnpack(struct SerialBuf *ctx, struct MsgDebugAcknowledge *s);

/*------------------------------MsgOctoSetTraceLevel-----------------------------------*/
/**
 Msg to have a new stream
 OUT_MSG_OCTO_SETTRACELEVEL
 */
#define     MsgOctoSetTraceLevelBits 48
#define     MsgOctoSetTraceLevelPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoSetTraceLevelPack(s, ctx);
#define     MsgOctoSetTraceLevelUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoSetTraceLevelUnpack(ctx, s);
EXTERN void MsgOctoSetTraceLevelPack(struct MsgOctoSetTraceLevel *s, struct SerialBuf *ctx);
EXTERN void MsgOctoSetTraceLevelUnpack(struct SerialBuf *ctx, struct MsgOctoSetTraceLevel *s);
/*------------------------------MsgOctoEnableDump-----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */
#define     MsgOctoEnableDumpBits 40
#define     MsgOctoEnableDumpPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoEnableDumpPack(s, ctx);
#define     MsgOctoEnableDumpUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoEnableDumpUnpack(ctx, s);
#define     MsgOctoEnableDumpPack(s, ctx)   MsgDebugAcknowledgePack((struct MsgDebugAcknowledge *)(s), ctx)
#define     MsgOctoEnableDumpUnpack(ctx, s)   MsgDebugAcknowledgeUnpack(ctx, (struct MsgDebugAcknowledge *)(s))

/*------------------------------MsgOctoEnableGet----------------------------------*/
/**
 OUT_MSG_OCTO_ENABLE_DUMP
 */
#define     MsgOctoEnableGetBits 40
#define     MsgOctoEnableGetPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoEnableGetPack(s, ctx);
#define     MsgOctoEnableGetUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoEnableGetUnpack(ctx, s);
#define     MsgOctoEnableGetPack(s, ctx)   MsgDebugAcknowledgePack((struct MsgDebugAcknowledge *)(s), ctx)
#define     MsgOctoEnableGetUnpack(ctx, s)   MsgDebugAcknowledgeUnpack(ctx, (struct MsgDebugAcknowledge *)(s))

#define     MsgOctoResetBits 32
#define     MsgOctoResetPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoResetPack(s, ctx);
#define     MsgOctoResetUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoResetUnpack(ctx, s);
#define     MsgOctoResetPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgOctoResetUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))


#define     MsgOctoInitializeDescBits 34
#define     MsgOctoInitializeDescPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoInitializeDescPack(s, ctx);
#define     MsgOctoInitializeDescUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoInitializeDescUnpack(ctx, s);
EXTERN void MsgOctoInitializeDescPack(struct MsgOctoInitializeDesc *s, struct SerialBuf *ctx);
EXTERN void MsgOctoInitializeDescUnpack(struct SerialBuf *ctx, struct MsgOctoInitializeDesc *s);

#define     MsgOctoWriteMemBits 64
#define     MsgOctoWriteMemPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoWriteMemPack(s, ctx);
#define     MsgOctoWriteMemUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoWriteMemUnpack(ctx, s);
EXTERN void MsgOctoWriteMemPack(struct MsgOctoWriteMem *s, struct SerialBuf *ctx);
EXTERN void MsgOctoWriteMemUnpack(struct SerialBuf *ctx, struct MsgOctoWriteMem *s);

#define     MsgOctoReadMemBits 48
#define     MsgOctoReadMemPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoReadMemPack(s, ctx);
#define     MsgOctoReadMemUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoReadMemUnpack(ctx, s);
EXTERN void MsgOctoReadMemPack(struct MsgOctoReadMem *s, struct SerialBuf *ctx);
EXTERN void MsgOctoReadMemUnpack(struct SerialBuf *ctx, struct MsgOctoReadMem *s);

#define     MsgOctoReadMemIndicationBits 48
#define     MsgOctoReadMemIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgOctoReadMemIndicationPack(s, ctx);
#define     MsgOctoReadMemIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgOctoReadMemIndicationUnpack(ctx, s);
#define     MsgOctoReadMemIndicationPack(s, ctx)   MsgOctoSetTraceLevelPack((struct MsgOctoSetTraceLevel *)(s), ctx)
#define     MsgOctoReadMemIndicationUnpack(ctx, s)   MsgOctoSetTraceLevelUnpack(ctx, (struct MsgOctoSetTraceLevel *)(s))

/* Access one "Net"/Register */
#define     MsgRegisterAccessBits 147
#define     MsgRegisterAccessPackInit(s, ctx)  SerialBufRestart(ctx); MsgRegisterAccessPack(s, ctx);
#define     MsgRegisterAccessUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRegisterAccessUnpack(ctx, s);
EXTERN void MsgRegisterAccessPack(struct MsgRegisterAccess *s, struct SerialBuf *ctx);
EXTERN void MsgRegisterAccessUnpack(struct SerialBuf *ctx, struct MsgRegisterAccess *s);

/* This message is sent when the write or read is done
 * Msb, Lsb are used for the value read even if a write was done
 */
#define     MsgRegisterAccessAckBits 128
#define     MsgRegisterAccessAckPackInit(s, ctx)  SerialBufRestart(ctx); MsgRegisterAccessAckPack(s, ctx);
#define     MsgRegisterAccessAckUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRegisterAccessAckUnpack(ctx, s);
EXTERN void MsgRegisterAccessAckPack(struct MsgRegisterAccessAck *s, struct SerialBuf *ctx);
EXTERN void MsgRegisterAccessAckUnpack(struct SerialBuf *ctx, struct MsgRegisterAccessAck *s);

#define     MsgRegisterEnablePeriodicgReadBits 120
#define     MsgRegisterEnablePeriodicgReadPackInit(s, ctx)  SerialBufRestart(ctx); MsgRegisterEnablePeriodicgReadPack(s, ctx);
#define     MsgRegisterEnablePeriodicgReadUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRegisterEnablePeriodicgReadUnpack(ctx, s);
EXTERN void MsgRegisterEnablePeriodicgReadPack(struct MsgRegisterEnablePeriodicgRead *s, struct SerialBuf *ctx);
EXTERN void MsgRegisterEnablePeriodicgReadUnpack(struct SerialBuf *ctx, struct MsgRegisterEnablePeriodicgRead *s);

/* this define is used in the ID-field of MsgRegisterEnablePeriodicReadAck and MsgRegisterDisablePeriodicRead */

#define     MsgRegisterEnablePeriodicReadAckBits 80
#define     MsgRegisterEnablePeriodicReadAckPackInit(s, ctx)  SerialBufRestart(ctx); MsgRegisterEnablePeriodicReadAckPack(s, ctx);
#define     MsgRegisterEnablePeriodicReadAckUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRegisterEnablePeriodicReadAckUnpack(ctx, s);
EXTERN void MsgRegisterEnablePeriodicReadAckPack(struct MsgRegisterEnablePeriodicReadAck *s, struct SerialBuf *ctx);
EXTERN void MsgRegisterEnablePeriodicReadAckUnpack(struct SerialBuf *ctx, struct MsgRegisterEnablePeriodicReadAck *s);

#define     MsgRegisterDisablePeriodicReadBits 48
#define     MsgRegisterDisablePeriodicReadPackInit(s, ctx)  SerialBufRestart(ctx); MsgRegisterDisablePeriodicReadPack(s, ctx);
#define     MsgRegisterDisablePeriodicReadUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRegisterDisablePeriodicReadUnpack(ctx, s);
#define     MsgRegisterDisablePeriodicReadPack(s, ctx)   MsgOctoReadMemPack((struct MsgOctoReadMem *)(s), ctx)
#define     MsgRegisterDisablePeriodicReadUnpack(ctx, s)   MsgOctoReadMemUnpack(ctx, (struct MsgOctoReadMem *)(s))

#define     MsgFrontendGetInfoBits 32
#define     MsgFrontendGetInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgFrontendGetInfoPack(s, ctx);
#define     MsgFrontendGetInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgFrontendGetInfoUnpack(ctx, s);
#define     MsgFrontendGetInfoPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgFrontendGetInfoUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

#define     MsgFrontendGetInfoIndicationBits 117
#define     MsgFrontendGetInfoIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgFrontendGetInfoIndicationPack(s, ctx);
#define     MsgFrontendGetInfoIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgFrontendGetInfoIndicationUnpack(ctx, s);
EXTERN void MsgFrontendGetInfoIndicationPack(struct MsgFrontendGetInfoIndication *s, struct SerialBuf *ctx);
EXTERN void MsgFrontendGetInfoIndicationUnpack(struct SerialBuf *ctx, struct MsgFrontendGetInfoIndication *s);

#define     MsgGetNumberOfChipsBits 32
#define     MsgGetNumberOfChipsPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetNumberOfChipsPack(s, ctx);
#define     MsgGetNumberOfChipsUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetNumberOfChipsUnpack(ctx, s);
#define     MsgGetNumberOfChipsPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgGetNumberOfChipsUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

#define     MsgGetNumberOfChipsIndicationBits 40
#define     MsgGetNumberOfChipsIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetNumberOfChipsIndicationPack(s, ctx);
#define     MsgGetNumberOfChipsIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetNumberOfChipsIndicationUnpack(ctx, s);
#define     MsgGetNumberOfChipsIndicationPack(s, ctx)   MsgDebugAcknowledgePack((struct MsgDebugAcknowledge *)(s), ctx)
#define     MsgGetNumberOfChipsIndicationUnpack(ctx, s)   MsgDebugAcknowledgeUnpack(ctx, (struct MsgDebugAcknowledge *)(s))

#define     MsgPeriodicReadingValueBits 168
#define     MsgPeriodicReadingValuePackInit(s, ctx)  SerialBufRestart(ctx); MsgPeriodicReadingValuePack(s, ctx);
#define     MsgPeriodicReadingValueUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgPeriodicReadingValueUnpack(ctx, s);
EXTERN void MsgPeriodicReadingValuePack(struct MsgPeriodicReadingValue *s, struct SerialBuf *ctx);
EXTERN void MsgPeriodicReadingValueUnpack(struct SerialBuf *ctx, struct MsgPeriodicReadingValue *s);


#define     MsgAcquisitionModeCtrlBits 42
#define     MsgAcquisitionModeCtrlPackInit(s, ctx)  SerialBufRestart(ctx); MsgAcquisitionModeCtrlPack(s, ctx);
#define     MsgAcquisitionModeCtrlUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgAcquisitionModeCtrlUnpack(ctx, s);
EXTERN void MsgAcquisitionModeCtrlPack(struct MsgAcquisitionModeCtrl *s, struct SerialBuf *ctx);
EXTERN void MsgAcquisitionModeCtrlUnpack(struct SerialBuf *ctx, struct MsgAcquisitionModeCtrl *s);

#define     MsgTunerAgcFreezeBits 49
#define     MsgTunerAgcFreezePackInit(s, ctx)  SerialBufRestart(ctx); MsgTunerAgcFreezePack(s, ctx);
#define     MsgTunerAgcFreezeUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgTunerAgcFreezeUnpack(ctx, s);
EXTERN void MsgTunerAgcFreezePack(struct MsgTunerAgcFreeze *s, struct SerialBuf *ctx);
EXTERN void MsgTunerAgcFreezeUnpack(struct SerialBuf *ctx, struct MsgTunerAgcFreeze *s);

#define     MsgGetChipInfoIndicationBits 280
#define     MsgGetChipInfoIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetChipInfoIndicationPack(s, ctx);
#define     MsgGetChipInfoIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetChipInfoIndicationUnpack(ctx, s);
EXTERN void MsgGetChipInfoIndicationPack(struct MsgGetChipInfoIndication *s, struct SerialBuf *ctx);
EXTERN void MsgGetChipInfoIndicationUnpack(struct SerialBuf *ctx, struct MsgGetChipInfoIndication *s);

#define     MsgGetChipInfoBits 32
#define     MsgGetChipInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetChipInfoPack(s, ctx);
#define     MsgGetChipInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetChipInfoUnpack(ctx, s);
#define     MsgGetChipInfoPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgGetChipInfoUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))

#define     MsgSDKInfoIndicationBits 72
#define     MsgSDKInfoIndicationPackInit(s, ctx)  SerialBufRestart(ctx); MsgSDKInfoIndicationPack(s, ctx);
#define     MsgSDKInfoIndicationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSDKInfoIndicationUnpack(ctx, s);
EXTERN void MsgSDKInfoIndicationPack(struct MsgSDKInfoIndication *s, struct SerialBuf *ctx);
EXTERN void MsgSDKInfoIndicationUnpack(struct SerialBuf *ctx, struct MsgSDKInfoIndication *s);

#define     MsgSDKInfoBits 32
#define     MsgSDKInfoPackInit(s, ctx)  SerialBufRestart(ctx); MsgSDKInfoPack(s, ctx);
#define     MsgSDKInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSDKInfoUnpack(ctx, s);
#define     MsgSDKInfoPack(s, ctx)   MsgHeaderPack((struct MsgHeader *)(s), ctx)
#define     MsgSDKInfoUnpack(ctx, s)   MsgHeaderUnpack(ctx, (struct MsgHeader *)(s))


#define     OCTOPUSClockParametersBits 32
#define     OCTOPUSClockParametersPackInit(s, ctx)  SerialBufRestart(ctx); OCTOPUSClockParametersPack(s, ctx);
#define     OCTOPUSClockParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); OCTOPUSClockParametersUnpack(ctx, s);
EXTERN void OCTOPUSClockParametersPack(struct OCTOPUSClockParameters *s, struct SerialBuf *ctx);
EXTERN void OCTOPUSClockParametersUnpack(struct SerialBuf *ctx, struct OCTOPUSClockParameters *s);

#define     REFLEXCLockParametersBits 64
#define     REFLEXCLockParametersPackInit(s, ctx)  SerialBufRestart(ctx); REFLEXCLockParametersPack(s, ctx);
#define     REFLEXCLockParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); REFLEXCLockParametersUnpack(ctx, s);
EXTERN void REFLEXCLockParametersPack(struct REFLEXCLockParameters *s, struct SerialBuf *ctx);
EXTERN void REFLEXCLockParametersUnpack(struct SerialBuf *ctx, struct REFLEXCLockParameters *s);

#define     ClockFrequencyParametersBits 64
#define     ClockFrequencyParametersPackInit(s, id, ctx)  SerialBufRestart(ctx); ClockFrequencyParametersPack(s, id, ctx);
#define     ClockFrequencyParametersUnpackInit(ctx, id, s)  SerialBufRestart(ctx); ClockFrequencyParametersPack(ctx, id, s);
EXTERN void ClockFrequencyParametersPack(union ClockFrequencyParameters *s, int id, struct SerialBuf *ctx);
EXTERN void ClockFrequencyParametersUnpack(struct SerialBuf *ctx, int id, union ClockFrequencyParameters *s);

#define     MsgSetClockFrequencyBits 104
#define     MsgSetClockFrequencyPackInit(s, ctx)  SerialBufRestart(ctx); MsgSetClockFrequencyPack(s, ctx);
#define     MsgSetClockFrequencyUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSetClockFrequencyUnpack(ctx, s);
EXTERN void MsgSetClockFrequencyPack(struct MsgSetClockFrequency *s, struct SerialBuf *ctx);
EXTERN void MsgSetClockFrequencyUnpack(struct SerialBuf *ctx, struct MsgSetClockFrequency *s);

#define     MsgMonitorDllBits 96
#define     MsgMonitorDllPackInit(s, ctx)  SerialBufRestart(ctx); MsgMonitorDllPack(s, ctx);
#define     MsgMonitorDllUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgMonitorDllUnpack(ctx, s);
EXTERN void MsgMonitorDllPack(struct MsgMonitorDll *s, struct SerialBuf *ctx);
EXTERN void MsgMonitorDllUnpack(struct SerialBuf *ctx, struct MsgMonitorDll *s);

#define     MsgGetMemBlockTableBits 33
#define     MsgGetMemBlockTablePackInit(s, ctx)  SerialBufRestart(ctx); MsgGetMemBlockTablePack(s, ctx);
#define     MsgGetMemBlockTableUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetMemBlockTableUnpack(ctx, s);
EXTERN void MsgGetMemBlockTablePack(struct MsgGetMemBlockTable *s, struct SerialBuf *ctx);
EXTERN void MsgGetMemBlockTableUnpack(struct SerialBuf *ctx, struct MsgGetMemBlockTable *s);

#define     MemBlockParametersBits 384
#define     MemBlockParametersPackInit(s, ctx)  SerialBufRestart(ctx); MemBlockParametersPack(s, ctx);
#define     MemBlockParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); MemBlockParametersUnpack(ctx, s);
EXTERN void MemBlockParametersPack(struct MemBlockParameters *s, struct SerialBuf *ctx);
EXTERN void MemBlockParametersUnpack(struct SerialBuf *ctx, struct MemBlockParameters *s);

#define     MsgGetMemBlockTableAckBits 480
#define     MsgGetMemBlockTableAckPackInit(s, ctx)  SerialBufRestart(ctx); MsgGetMemBlockTableAckPack(s, ctx);
#define     MsgGetMemBlockTableAckUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGetMemBlockTableAckUnpack(ctx, s);
EXTERN void MsgGetMemBlockTableAckPack(struct MsgGetMemBlockTableAck *s, struct SerialBuf *ctx);
EXTERN void MsgGetMemBlockTableAckUnpack(struct SerialBuf *ctx, struct MsgGetMemBlockTableAck *s);



#endif /*__MSG_TYPE_DEBUG__*/


