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

#ifndef __MSG_TYPE_STRUCT__
#define __MSG_TYPE_STRUCT__

#ifdef __cplusplus
#define EXTERN  extern "C"
#else
#define EXTERN
#endif


/* Max message length */
#define MAX_MSG_STRUCT_BITS            17448

/* Input message identifiers */
#define MAX_MSG_STRUCT_IN              1

/* Output message identifiers */
#define MAX_MSG_STRUCT_OUT             1



#include "DibMsgTypes.h"
#include "DibMsgGeneric.h"
#include "DibExtAPI.h"

/*----------------------------------------------------------
 * Constants
 *---------------------------------------------------------*/
#define MAX_PRINT_BYTES                200
#define INPUT_LIST_MAX                 24
#define MAX_ANTENNAS                   4
#define MAX_STREAMS                    10

#define ACK_KO                         0
#define ACK_OK                         1
#define NO_ACK                         2

#define CHIP_MODEL_SPITS               0
#define CHIP_MODEL_DMA                 1
#define CHIP_MODEL_TX_DIV              2
#define CHIP_MODEL_MPEG1_TX_DIV        3

#define DIB_1009X                      0
#define DIB_20066                      1
#define DIB_3006X                      2
#define DIB_3009X                      3

#define HOST_IF_SRAM                   0
#define HOST_IF_SPI                    1
#define HOST_IF_SDIO                   2
#define HOST_IF_I2C                    3
#define HOST_IF_DIBCTRL                4


#define DAB_MAX_SUBC                   64
#define GPIO_FUNC_INIT                 0
#define GPIO_FUNC_TUNER_ON             1
#define GPIO_FUNC_TUNER_OFF            2
#define GPIO_FUNC_LNA_ON               3
#define GPIO_FUNC_LNA_OFF              4
#define GPIO_FUNC_LNA_BYPASS           5
#define GPIO_FUNC_MAX                  6

/* Error Flag 8bits fields      */
#define DFERR_FLAG_MASK                255
#define DFERR_CRITICAL                 1
#define DFERR_MINOR                    2
#define DFERR_FATAL                    4

/* General Error Codes : 0-99     */
#define DFERR_UNSUFFICIENT_MEMORY      1
#define DFERR_UNSUFFICIENT_RESOURCES   2
#define DFERR_OVERFLOW                 3
#define DFERR_HW_NOT_RESPONDING        4

/* Stream If Block Error Type 100-119 */
#define DFERR_TSQ_OVERFLOW             100
#define DFERR_QSDMA_CTX_FULL           101

/* Fec Block Error Type       120-139 */
#define DFERR_QFEC_CTX_FULL            120

/* Dma Block Error Type       130-149 */
#define DFERR_QMDMA_CTX_FULL           130
#define DFERR_QGDMA_CTX_FULL           131

/* Data Error Type            200-250 */
#define DFERR_IFEC_FILE_LOOP           200
#define DFERR_IFEC_RESOURCES           201
#define DFERR_IFEC_MISEVAL             202

/* Test Error Type            500-700 */
#define DFERR_TEST_URAM_DMA            500

#define INTERNAL_WRITE                 1
#define INTERNAL_READ                  0

#define POWER_SAVING_OFF               0
#define POWER_SAVING_ON                1

#define FIRMWARE_NAME_LEN              44

/* Option field
 * UNCOMPRESS    : the firmware ask for compressed file
 * TEST          : for sdk only, the download is from the chipset initiative
 * RAW           : for sdk only, the download file is on the file system.
 */

#define REQUEST_FIRMWARE_OPTION_UNCOMPRESS 1
#define REQUEST_FIRMWARE_OPTION_TEST   2
#define REQUEST_FIRMWARE_OPTION_RAW    4
#define REQUEST_FIRMWARE_OPTION_MASK   7

#define SLAVE_ACCESS_MAX               16

/*----------------------------------------------------------
 * Enumerations
 *---------------------------------------------------------*/

/* TuneStatus enumeration */
enum TuneStatus
{
   TUNE_FAILED = 0,
   TUNE_TIMED_OUT = -1,
   TUNE_TIME_TOO_SHORT = -2,
   TUNE_PENDING = -3,
   TUNE_STD_SUCCESS = -4,
   TUNE_FFT_SUCCESS = -5,
   TUNE_DEMOD_SUCCESS = -6,
   TUNE_RETRY = -7,
   TUNE_LOCKED = -8,
};
/* antenna general tune state */
enum AdapterState
{
   POWER_OFF = 0,
   POWER_UP = 1,
   POWER_ON = 2,
   POWER_DOWN = 3,
};
enum RestartTune
{
   RESTART_TUNE_FROM_TUNER = 0,
   RESTART_TUNE_FROM_AGC = 1,
   RESTART_TUNE_FROM_DEMOD = 2,
};
enum TuneOption
{
   TUNE_STANDARD = 0,
   TUNE_SCAN_FAST = 1,
   TUNE_AUTO = 2,
};


/*----------------------------------------------------------
 * exchange structures
 *---------------------------------------------------------*/
struct BasicLayoutInfo
{
   uint8_t chipId;
   uint8_t chipModel;
   uint8_t hostInterface;
   uint8_t notReset;
   uint8_t package;
   int8_t flashchip;
   int8_t scchip;
};

struct GpioFunction
{
   uint32_t FuncName;
   uint32_t Mask;
   uint32_t Direction;
   uint32_t Value;
};

struct InputList
{
   uint32_t FeId;
   uint32_t StreamId;
   uint32_t FeStandards;
};
struct StreamInfo
{
   uint8_t ChipId;
   uint8_t StreamType;
   uint8_t NumOfInputs;
   uint8_t PowerMode;
   uint32_t StreamId;
   uint32_t Options;
   uint32_t SupportedStandards;
   struct InputList DirectInputs[24];
};
struct FrontendInfo
{
   uint8_t ChipId;
   uint8_t NumOfInputs;
   uint32_t FeId;
   uint32_t SupportedStandards;
   struct InputList DirectInputs[24];
};
struct LayoutInfo
{
   uint8_t MaxNbChips;
   uint8_t MaxNbFrontends;
   uint8_t MaxNbStreams;
   struct StreamInfo StreamInfo[10];
   struct FrontendInfo FeInfo[4];
};
struct CMMBmfMapping
{
   uint8_t MsfNb;
   uint8_t MfId;
   uint16_t ServId[15];
};
struct CMMBmapping
{
   uint8_t LastMfNum;
   uint8_t NbMf;
   uint8_t End;
   struct CMMBmfMapping Mapping[6];
};

struct DVBChannelParameters
{
   int8_t Fft;
   int8_t Guard;
   int8_t Constellation;
   int8_t NativeIntlv;
   int8_t InvSpect;
};
struct DVBTChannelParameters
{
   struct DVBChannelParameters Dvb;
   int8_t ViterbiSelectHp;
   int8_t ViterbiCodeRateHp;
   int8_t ViterbiCodeRateLp;
   int8_t ViterbiAlpha;
   int8_t ViterbiHierach;
};
struct DVBTChannelDecoderInfo
{
   uint16_t Quality;
   uint16_t FastQuality;
   uint32_t Ber;
   uint32_t Per;
};
struct DVBSHChannelParameters
{
   struct DVBChannelParameters Dvb;
   int8_t SelectHp;
   int8_t CodeRateHp;
   int8_t CodeRateLp;
   int8_t Alpha;
   int8_t Hierach;
   uint8_t TurboLateTaps;
   uint8_t TurboNonLateIncr;
   uint8_t TurboCommonMult;
   uint8_t TurboNbSlices;
   uint8_t TurboSliceDist;
};
struct DVBSHChannelDecoderInfo
{
   uint16_t Quality;
   uint16_t FastQuality;
   uint32_t Per;
};

struct ISDBTLayerParameter
{
   int8_t Constellation;
   int8_t CodeRate;
   int8_t NbSegments;
   int8_t TimeIntlv;
};
struct ISDBTChannelParameters
{
   int8_t Fft;
   int8_t Guard;
   int8_t SbMode;
   int8_t PartialReception;
   int8_t InvSpect;
   int8_t SbConTotSeg;
   int8_t SbWantedSeg;
   int8_t SbSubChannel;
   struct ISDBTLayerParameter Layer[3];
};
struct ISDBTChannelDecoderInfo
{
   uint16_t Quality;
   uint16_t FastQuality;
   uint32_t Ber;
   uint32_t Per;
};
struct DABSubChannel
{
   uint8_t Id;
   uint16_t StartAddress;
   uint16_t SubSize;
   uint8_t Form;
   uint8_t TableIndex;
   uint8_t Option;
   uint8_t ProtectionLevel;
   uint8_t IsTs;
   uint8_t SubChNew;
   uint16_t AddrRAM;
   uint8_t RequestedFec;
};
struct DABSubChannelCanalInfo
{
   uint16_t StartAddress;
   uint16_t SubSize;
   uint16_t PI1;
   uint16_t PI2;
   uint16_t I;
   uint16_t L1Punct;
   uint16_t L2Punct;
   uint8_t Id;
   uint8_t Form;
   uint8_t Index;
};
struct DABChannelParameters
{
   int8_t TMode;
   uint8_t NbActiveSubCh;
   uint8_t Reconfig;
   int16_t CifOccurrenceChange;
   struct DABSubChannel SubChannel[64];
};
struct DABChannelShortParameters
{
   int8_t TMode;
   uint8_t NbActiveSubCh;
};
struct DABLocks
{
   uint8_t TdIntFullInternal;
   
   uint8_t Mpeg;
   uint8_t Frm;
   uint8_t Dmb;
   
   uint8_t NDecFrm;
   uint8_t NDecTMode;
   uint8_t CoffDab;
   uint8_t CoffDmb;
   
};
struct DABChannelDecoderInfo
{
   uint16_t Quality;
   uint16_t FastQuality;
   uint32_t Ber;
   uint32_t Per;
   uint32_t TdmbBer;
};

struct CMMBTimeSlot
{
   int8_t RsCodeRate;
   int8_t IntlvMode;
   int8_t LdpcCodeRate;
   int8_t Constellation;
   int8_t Scrambling;
   int8_t MfId;
};
struct CMMBChannelParameters
{
   int8_t Fft;
   struct CMMBTimeSlot TsArray[40];
   int16_t configFrame;
};
struct CMMBChannelDecoderInfo
{
   uint16_t Quality;
   uint16_t FastQuality;
   uint32_t Per;
};
struct CTTBChannelParameters
{
   int8_t Fft;
   int8_t Constellation;
   int8_t Guard;
   int8_t LdpcCodeRate;
   int8_t Intlv;
   int8_t ConstantPN;
};
struct CTTBChannelDecoderInfo
{
   uint16_t Quality;
   uint16_t FastQuality;
   uint32_t Per;
   uint32_t LdpcSyndrome;
   uint32_t LdpcSyndromeFirstIter;
   uint8_t LdpcNbIter;
   uint8_t LdpcFlags;
};
struct ATSCChannelParameters
{
   uint32_t dummy;
};
struct ATSCChannelDecoderInfo
{
   uint32_t Ber;
   uint32_t Per;
};
struct ATSCMHChannelDecoderInfo
{
   uint32_t Ber;
   uint32_t Per;
};

union ChannelParameters
{
   struct DVBTChannelParameters Dvbt;
   struct ISDBTChannelParameters Isdbt;
   struct DABChannelParameters Dab;
   struct DVBSHChannelParameters Dvbsh;
   struct CMMBChannelParameters Cmmb;
   struct CTTBChannelParameters Cttb;
   struct ATSCChannelParameters Atsc;
};

union TuneChannelParameters
{
   struct DVBTChannelParameters Dvbt;
   struct ISDBTChannelParameters Isdbt;
   struct DABChannelShortParameters Dab;
   struct DVBSHChannelParameters Dvbsh;
   struct CMMBChannelParameters Cmmb;
   struct CTTBChannelParameters Cttb;
   struct ATSCChannelParameters Atsc;
};
union ChannelDecoderInfo
{
   struct DVBTChannelDecoderInfo Dvbt;
   struct ISDBTChannelDecoderInfo Isdbt[3];
   struct DABChannelDecoderInfo Dab;
   struct DVBSHChannelDecoderInfo Dvbsh;
   struct CMMBChannelDecoderInfo Cmmb;
   struct CTTBChannelDecoderInfo Cttb;
   struct ATSCChannelDecoderInfo Atsc;
   struct ATSCMHChannelDecoderInfo AtscMh;
};
#define TRANSMITTER_ID_UNAVAILABLE     0xffffffff

struct SignalMonitoring
{
   uint8_t Standard;
   uint8_t MonitStatus;
   uint8_t LastDemod;
   uint32_t AdcPower;
   uint16_t AgcGlobal;
   uint16_t AgcRf;
   uint16_t AgcBb;
   uint16_t AgcWbd;
   uint16_t AgcSplitOffset;
   uint16_t Locks;
   uint32_t EqualNoise;
   uint32_t EqualSignal;
   uint32_t SystemNoise;
   uint32_t CombNoise;
   uint32_t CombSignal;
   int32_t TimingOffset;
   int32_t FrequencyOffset;
   uint32_t TransmitterId;
   union ChannelDecoderInfo ChanDec;
};
struct ChannelDesc
{
   uint32_t Frequency;
   uint16_t Bandwidth;
   uint16_t Standard;

   union ChannelParameters Param;
};
struct TuneChannelDesc
{
   uint32_t Frequency;
   uint16_t Bandwidth;
   uint16_t Standard;

   union TuneChannelParameters Param;
};

struct TSItemParameters
{
   uint16_t Pid;
};
struct SIPSIItemParameters
{
   uint16_t Pid;
   uint8_t PidWatch;
   uint8_t PidCrc;
};
struct MPEFECItemParameters
{
   uint16_t Pid;
   uint8_t Prefetch;
   uint16_t NbRows;
   uint16_t MaxBurstDuration;
};
struct FIGItemParameters
{
   uint32_t RFU;
};
struct TDMBItemParameters
{
   uint16_t Pid;
   uint16_t SubCh;
};
struct DABItemParameters
{
   uint16_t SubCh;
   uint16_t Type;
   uint8_t Fec;
};
struct DABPacketItemParameters
{
   uint16_t SubCh;
   uint16_t Address;
   uint8_t Fec;
   uint8_t DataGroup;
};
struct PESVItemParameters
{
   uint32_t RFU;
};
struct PESAItemParameters
{
   uint32_t RFU;
};
struct PESOItemParameters
{
   uint32_t RFU;
};
struct PCRItemParameters
{
   uint32_t RFU;
};
struct MPEIFECItemParameters
{
   uint16_t Pid;
   uint8_t Prefetch;
   uint16_t NbRows;
   uint16_t MaxBurstDuration;
   uint8_t D;
   uint8_t B;
   uint8_t C;
   uint8_t R;
   uint8_t S;
};
struct CMMBMFSItemParameters
{
   uint32_t RFU;
};
struct CMMBCITItemParameters
{
   uint16_t TableId;
};
struct CMMBSVCItemParameters
{
   uint16_t Mode;
   uint16_t MfId;
   uint16_t SvcId;
};
struct CMMBXPEItemParameters
{
   uint32_t RFU;
};
struct ATSCMHIPItemParameters
{
   uint8_t Mode;
   uint16_t Id;
};
struct LOWSPEEDTSItemParameters
{
   uint16_t Pid;
};
union ItemParameters
{
   struct TSItemParameters Ts;
   struct SIPSIItemParameters Sipsi;
   struct MPEFECItemParameters Mpefec;
   struct FIGItemParameters Fig;
   struct TDMBItemParameters Tdmb;
   struct DABItemParameters Dab;
   struct DABPacketItemParameters DabPacket;
   struct PESVItemParameters PesV;
   struct PESAItemParameters PesA;
   struct PESOItemParameters PesO;
   struct PCRItemParameters Pcr;
   struct MPEIFECItemParameters Mpeifec;
   struct CMMBMFSItemParameters CmmbMfs;
   struct CMMBCITItemParameters CmmbCit;
   struct CMMBSVCItemParameters CmmbSvc;
   struct CMMBXPEItemParameters CmmbXpe;
   struct ATSCMHIPItemParameters AtscmhFic;
   struct ATSCMHIPItemParameters AtscmhIp;
   struct LOWSPEEDTSItemParameters LowSpeedTs;
};
struct EFUSEInfoParameters
{
   uint8_t IsRead;
};
struct SDRAMInfoParameters
{
   uint8_t State;
   uint8_t IsUsedForCaching;
};
struct GPIOInfoParameters
{
   uint8_t Num;
   uint8_t Val;
   uint16_t Ratios[16];
   uint8_t Pwm[16];
   uint8_t Div;
   uint8_t Dir;
   uint8_t Pwm_mode;
};
struct EFUSETUNERInfoParameters
{
   uint8_t Level;
};
struct TUNERINDENTITYInfoParameters
{
   uint8_t Version;
};
struct TOGGLEMPEGInfoParameters
{
   uint8_t IsToggled;
};
struct PERPERIODInfoParameters
{
   uint8_t Period;
};
union InfoParameters
{
   struct EFUSEInfoParameters Efuse;
   struct SDRAMInfoParameters Sdram;
   struct GPIOInfoParameters Gpio;
   struct EFUSETUNERInfoParameters EfuseTuner;
   struct TUNERINDENTITYInfoParameters TunerIdentity;
   struct TOGGLEMPEGInfoParameters ToggleMpeg;
   struct PERPERIODInfoParameters PerPeriod;
};




/*----------------------------------------------------------
 * Constants
 *---------------------------------------------------------*/







/* Error Flag 8bits fields      */

/* General Error Codes : 0-99     */

/* Stream If Block Error Type 100-119 */

/* Fec Block Error Type       120-139 */

/* Dma Block Error Type       130-149 */

/* Data Error Type            200-250 */

/* Test Error Type            500-700 */




/* Option field
 * UNCOMPRESS    : the firmware ask for compressed file
 * TEST          : for sdk only, the download is from the chipset initiative
 * RAW           : for sdk only, the download file is on the file system.
 */



/*----------------------------------------------------------
 * Enumerations
 *---------------------------------------------------------*/

/* TuneStatus enumeration */

/* antenna general tune state */





/*----------------------------------------------------------
 * exchange structures
 *---------------------------------------------------------*/
#define     BasicLayoutInfoBits 38
#define     BasicLayoutInfoPackInit(s, ctx)  SerialBufRestart(ctx); BasicLayoutInfoPack(s, ctx);
#define     BasicLayoutInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); BasicLayoutInfoUnpack(ctx, s);
EXTERN void BasicLayoutInfoPack(struct BasicLayoutInfo *s, struct SerialBuf *ctx);
EXTERN void BasicLayoutInfoUnpack(struct SerialBuf *ctx, struct BasicLayoutInfo *s);


#define     GpioFunctionBits 128
#define     GpioFunctionPackInit(s, ctx)  SerialBufRestart(ctx); GpioFunctionPack(s, ctx);
#define     GpioFunctionUnpackInit(ctx, s)  SerialBufRestart(ctx); GpioFunctionUnpack(ctx, s);
EXTERN void GpioFunctionPack(struct GpioFunction *s, struct SerialBuf *ctx);
EXTERN void GpioFunctionUnpack(struct SerialBuf *ctx, struct GpioFunction *s);


#define     InputListBits 48
#define     InputListPackInit(s, ctx)  SerialBufRestart(ctx); InputListPack(s, ctx);
#define     InputListUnpackInit(ctx, s)  SerialBufRestart(ctx); InputListUnpack(ctx, s);
EXTERN void InputListPack(struct InputList *s, struct SerialBuf *ctx);
EXTERN void InputListUnpack(struct SerialBuf *ctx, struct InputList *s);

#define     StreamInfoBits 1256
#define     StreamInfoPackInit(s, ctx)  SerialBufRestart(ctx); StreamInfoPack(s, ctx);
#define     StreamInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); StreamInfoUnpack(ctx, s);
EXTERN void StreamInfoPack(struct StreamInfo *s, struct SerialBuf *ctx);
EXTERN void StreamInfoUnpack(struct SerialBuf *ctx, struct StreamInfo *s);

#define     FrontendInfoBits 1216
#define     FrontendInfoPackInit(s, ctx)  SerialBufRestart(ctx); FrontendInfoPack(s, ctx);
#define     FrontendInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); FrontendInfoUnpack(ctx, s);
EXTERN void FrontendInfoPack(struct FrontendInfo *s, struct SerialBuf *ctx);
EXTERN void FrontendInfoUnpack(struct SerialBuf *ctx, struct FrontendInfo *s);

#define     LayoutInfoBits 17448
#define     LayoutInfoPackInit(s, ctx)  SerialBufRestart(ctx); LayoutInfoPack(s, ctx);
#define     LayoutInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); LayoutInfoUnpack(ctx, s);
EXTERN void LayoutInfoPack(struct LayoutInfo *s, struct SerialBuf *ctx);
EXTERN void LayoutInfoUnpack(struct SerialBuf *ctx, struct LayoutInfo *s);

#define     CMMBmfMappingBits 256
#define     CMMBmfMappingPackInit(s, ctx)  SerialBufRestart(ctx); CMMBmfMappingPack(s, ctx);
#define     CMMBmfMappingUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBmfMappingUnpack(ctx, s);
EXTERN void CMMBmfMappingPack(struct CMMBmfMapping *s, struct SerialBuf *ctx);
EXTERN void CMMBmfMappingUnpack(struct SerialBuf *ctx, struct CMMBmfMapping *s);

#define     CMMBmappingBits 1560
#define     CMMBmappingPackInit(s, ctx)  SerialBufRestart(ctx); CMMBmappingPack(s, ctx);
#define     CMMBmappingUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBmappingUnpack(ctx, s);
EXTERN void CMMBmappingPack(struct CMMBmapping *s, struct SerialBuf *ctx);
EXTERN void CMMBmappingUnpack(struct SerialBuf *ctx, struct CMMBmapping *s);


#define     DVBChannelParametersBits 40
#define     DVBChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); DVBChannelParametersPack(s, ctx);
#define     DVBChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); DVBChannelParametersUnpack(ctx, s);
EXTERN void DVBChannelParametersPack(struct DVBChannelParameters *s, struct SerialBuf *ctx);
EXTERN void DVBChannelParametersUnpack(struct SerialBuf *ctx, struct DVBChannelParameters *s);

#define     DVBTChannelParametersBits 80
#define     DVBTChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); DVBTChannelParametersPack(s, ctx);
#define     DVBTChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); DVBTChannelParametersUnpack(ctx, s);
EXTERN void DVBTChannelParametersPack(struct DVBTChannelParameters *s, struct SerialBuf *ctx);
EXTERN void DVBTChannelParametersUnpack(struct SerialBuf *ctx, struct DVBTChannelParameters *s);

#define     DVBTChannelDecoderInfoBits 96
#define     DVBTChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); DVBTChannelDecoderInfoPack(s, ctx);
#define     DVBTChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); DVBTChannelDecoderInfoUnpack(ctx, s);
EXTERN void DVBTChannelDecoderInfoPack(struct DVBTChannelDecoderInfo *s, struct SerialBuf *ctx);
EXTERN void DVBTChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct DVBTChannelDecoderInfo *s);

#define     DVBSHChannelParametersBits 106
#define     DVBSHChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); DVBSHChannelParametersPack(s, ctx);
#define     DVBSHChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); DVBSHChannelParametersUnpack(ctx, s);
EXTERN void DVBSHChannelParametersPack(struct DVBSHChannelParameters *s, struct SerialBuf *ctx);
EXTERN void DVBSHChannelParametersUnpack(struct SerialBuf *ctx, struct DVBSHChannelParameters *s);

#define     DVBSHChannelDecoderInfoBits 64
#define     DVBSHChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); DVBSHChannelDecoderInfoPack(s, ctx);
#define     DVBSHChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); DVBSHChannelDecoderInfoUnpack(ctx, s);
EXTERN void DVBSHChannelDecoderInfoPack(struct DVBSHChannelDecoderInfo *s, struct SerialBuf *ctx);
EXTERN void DVBSHChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct DVBSHChannelDecoderInfo *s);


#define     ISDBTLayerParameterBits 19
#define     ISDBTLayerParameterPackInit(s, ctx)  SerialBufRestart(ctx); ISDBTLayerParameterPack(s, ctx);
#define     ISDBTLayerParameterUnpackInit(ctx, s)  SerialBufRestart(ctx); ISDBTLayerParameterUnpack(ctx, s);
EXTERN void ISDBTLayerParameterPack(struct ISDBTLayerParameter *s, struct SerialBuf *ctx);
EXTERN void ISDBTLayerParameterUnpack(struct SerialBuf *ctx, struct ISDBTLayerParameter *s);

#define     ISDBTChannelParametersBits 96
#define     ISDBTChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); ISDBTChannelParametersPack(s, ctx);
#define     ISDBTChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); ISDBTChannelParametersUnpack(ctx, s);
EXTERN void ISDBTChannelParametersPack(struct ISDBTChannelParameters *s, struct SerialBuf *ctx);
EXTERN void ISDBTChannelParametersUnpack(struct SerialBuf *ctx, struct ISDBTChannelParameters *s);

#define     ISDBTChannelDecoderInfoBits 96
#define     ISDBTChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); ISDBTChannelDecoderInfoPack(s, ctx);
#define     ISDBTChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); ISDBTChannelDecoderInfoUnpack(ctx, s);
#define     ISDBTChannelDecoderInfoPack(s, ctx)   DVBTChannelDecoderInfoPack((struct DVBTChannelDecoderInfo *)(s), ctx)
#define     ISDBTChannelDecoderInfoUnpack(ctx, s)   DVBTChannelDecoderInfoUnpack(ctx, (struct DVBTChannelDecoderInfo *)(s))

#define     DABSubChannelBits 37
#define     DABSubChannelPackInit(s, ctx)  SerialBufRestart(ctx); DABSubChannelPack(s, ctx);
#define     DABSubChannelUnpackInit(ctx, s)  SerialBufRestart(ctx); DABSubChannelUnpack(ctx, s);
EXTERN void DABSubChannelPack(struct DABSubChannel *s, struct SerialBuf *ctx);
EXTERN void DABSubChannelUnpack(struct SerialBuf *ctx, struct DABSubChannel *s);

#define     DABSubChannelCanalInfoBits 136
#define     DABSubChannelCanalInfoPackInit(s, ctx)  SerialBufRestart(ctx); DABSubChannelCanalInfoPack(s, ctx);
#define     DABSubChannelCanalInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); DABSubChannelCanalInfoUnpack(ctx, s);
EXTERN void DABSubChannelCanalInfoPack(struct DABSubChannelCanalInfo *s, struct SerialBuf *ctx);
EXTERN void DABSubChannelCanalInfoUnpack(struct SerialBuf *ctx, struct DABSubChannelCanalInfo *s);

#define     DABChannelParametersBits 2384
#define     DABChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); DABChannelParametersPack(s, ctx);
#define     DABChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); DABChannelParametersUnpack(ctx, s);
EXTERN void DABChannelParametersPack(struct DABChannelParameters *s, struct SerialBuf *ctx);
EXTERN void DABChannelParametersUnpack(struct SerialBuf *ctx, struct DABChannelParameters *s);

#define     DABChannelShortParametersBits 16
#define     DABChannelShortParametersPackInit(s, ctx)  SerialBufRestart(ctx); DABChannelShortParametersPack(s, ctx);
#define     DABChannelShortParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); DABChannelShortParametersUnpack(ctx, s);
EXTERN void DABChannelShortParametersPack(struct DABChannelShortParameters *s, struct SerialBuf *ctx);
EXTERN void DABChannelShortParametersUnpack(struct SerialBuf *ctx, struct DABChannelShortParameters *s);

#define     DABLocksBits 16
#define     DABLocksPackInit(s, ctx)  SerialBufRestart(ctx); DABLocksPack(s, ctx);
#define     DABLocksUnpackInit(ctx, s)  SerialBufRestart(ctx); DABLocksUnpack(ctx, s);
EXTERN void DABLocksPack(struct DABLocks *s, struct SerialBuf *ctx);
EXTERN void DABLocksUnpack(struct SerialBuf *ctx, struct DABLocks *s);

#define     DABChannelDecoderInfoBits 128
#define     DABChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); DABChannelDecoderInfoPack(s, ctx);
#define     DABChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); DABChannelDecoderInfoUnpack(ctx, s);
EXTERN void DABChannelDecoderInfoPack(struct DABChannelDecoderInfo *s, struct SerialBuf *ctx);
EXTERN void DABChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct DABChannelDecoderInfo *s);


#define     CMMBTimeSlotBits 27
#define     CMMBTimeSlotPackInit(s, ctx)  SerialBufRestart(ctx); CMMBTimeSlotPack(s, ctx);
#define     CMMBTimeSlotUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBTimeSlotUnpack(ctx, s);
EXTERN void CMMBTimeSlotPack(struct CMMBTimeSlot *s, struct SerialBuf *ctx);
EXTERN void CMMBTimeSlotUnpack(struct SerialBuf *ctx, struct CMMBTimeSlot *s);

#define     CMMBChannelParametersBits 1104
#define     CMMBChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); CMMBChannelParametersPack(s, ctx);
#define     CMMBChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBChannelParametersUnpack(ctx, s);
EXTERN void CMMBChannelParametersPack(struct CMMBChannelParameters *s, struct SerialBuf *ctx);
EXTERN void CMMBChannelParametersUnpack(struct SerialBuf *ctx, struct CMMBChannelParameters *s);

#define     CMMBChannelDecoderInfoBits 64
#define     CMMBChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); CMMBChannelDecoderInfoPack(s, ctx);
#define     CMMBChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBChannelDecoderInfoUnpack(ctx, s);
#define     CMMBChannelDecoderInfoPack(s, ctx)   DVBSHChannelDecoderInfoPack((struct DVBSHChannelDecoderInfo *)(s), ctx)
#define     CMMBChannelDecoderInfoUnpack(ctx, s)   DVBSHChannelDecoderInfoUnpack(ctx, (struct DVBSHChannelDecoderInfo *)(s))

#define     CTTBChannelParametersBits 48
#define     CTTBChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); CTTBChannelParametersPack(s, ctx);
#define     CTTBChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); CTTBChannelParametersUnpack(ctx, s);
EXTERN void CTTBChannelParametersPack(struct CTTBChannelParameters *s, struct SerialBuf *ctx);
EXTERN void CTTBChannelParametersUnpack(struct SerialBuf *ctx, struct CTTBChannelParameters *s);

#define     CTTBChannelDecoderInfoBits 144
#define     CTTBChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); CTTBChannelDecoderInfoPack(s, ctx);
#define     CTTBChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); CTTBChannelDecoderInfoUnpack(ctx, s);
EXTERN void CTTBChannelDecoderInfoPack(struct CTTBChannelDecoderInfo *s, struct SerialBuf *ctx);
EXTERN void CTTBChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct CTTBChannelDecoderInfo *s);

#define     ATSCChannelParametersBits 32
#define     ATSCChannelParametersPackInit(s, ctx)  SerialBufRestart(ctx); ATSCChannelParametersPack(s, ctx);
#define     ATSCChannelParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); ATSCChannelParametersUnpack(ctx, s);
EXTERN void ATSCChannelParametersPack(struct ATSCChannelParameters *s, struct SerialBuf *ctx);
EXTERN void ATSCChannelParametersUnpack(struct SerialBuf *ctx, struct ATSCChannelParameters *s);

#define     ATSCChannelDecoderInfoBits 64
#define     ATSCChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); ATSCChannelDecoderInfoPack(s, ctx);
#define     ATSCChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); ATSCChannelDecoderInfoUnpack(ctx, s);
EXTERN void ATSCChannelDecoderInfoPack(struct ATSCChannelDecoderInfo *s, struct SerialBuf *ctx);
EXTERN void ATSCChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct ATSCChannelDecoderInfo *s);

#define     ATSCMHChannelDecoderInfoBits 64
#define     ATSCMHChannelDecoderInfoPackInit(s, ctx)  SerialBufRestart(ctx); ATSCMHChannelDecoderInfoPack(s, ctx);
#define     ATSCMHChannelDecoderInfoUnpackInit(ctx, s)  SerialBufRestart(ctx); ATSCMHChannelDecoderInfoUnpack(ctx, s);
#define     ATSCMHChannelDecoderInfoPack(s, ctx)   ATSCChannelDecoderInfoPack((struct ATSCChannelDecoderInfo *)(s), ctx)
#define     ATSCMHChannelDecoderInfoUnpack(ctx, s)   ATSCChannelDecoderInfoUnpack(ctx, (struct ATSCChannelDecoderInfo *)(s))


#define     ChannelParametersBits 2384
#define     ChannelParametersPackInit(s, id, ctx)  SerialBufRestart(ctx); ChannelParametersPack(s, id, ctx);
#define     ChannelParametersUnpackInit(ctx, id, s)  SerialBufRestart(ctx); ChannelParametersPack(ctx, id, s);
EXTERN void ChannelParametersPack(union ChannelParameters *s, int id, struct SerialBuf *ctx);
EXTERN void ChannelParametersUnpack(struct SerialBuf *ctx, int id, union ChannelParameters *s);


#define     TuneChannelParametersBits 1104
#define     TuneChannelParametersPackInit(s, id, ctx)  SerialBufRestart(ctx); TuneChannelParametersPack(s, id, ctx);
#define     TuneChannelParametersUnpackInit(ctx, id, s)  SerialBufRestart(ctx); TuneChannelParametersPack(ctx, id, s);
EXTERN void TuneChannelParametersPack(union TuneChannelParameters *s, int id, struct SerialBuf *ctx);
EXTERN void TuneChannelParametersUnpack(struct SerialBuf *ctx, int id, union TuneChannelParameters *s);

#define     ChannelDecoderInfoBits 288
#define     ChannelDecoderInfoPackInit(s, id, ctx)  SerialBufRestart(ctx); ChannelDecoderInfoPack(s, id, ctx);
#define     ChannelDecoderInfoUnpackInit(ctx, id, s)  SerialBufRestart(ctx); ChannelDecoderInfoPack(ctx, id, s);
EXTERN void ChannelDecoderInfoPack(union ChannelDecoderInfo *s, int id, struct SerialBuf *ctx);
EXTERN void ChannelDecoderInfoUnpack(struct SerialBuf *ctx, int id, union ChannelDecoderInfo *s);


#define     SignalMonitoringBits 686
#define     SignalMonitoringPackInit(s, ctx)  SerialBufRestart(ctx); SignalMonitoringPack(s, ctx);
#define     SignalMonitoringUnpackInit(ctx, s)  SerialBufRestart(ctx); SignalMonitoringUnpack(ctx, s);
EXTERN void SignalMonitoringPack(struct SignalMonitoring *s, struct SerialBuf *ctx);
EXTERN void SignalMonitoringUnpack(struct SerialBuf *ctx, struct SignalMonitoring *s);

#define     ChannelDescBits 2437
#define     ChannelDescPackInit(s, ctx)  SerialBufRestart(ctx); ChannelDescPack(s, ctx);
#define     ChannelDescUnpackInit(ctx, s)  SerialBufRestart(ctx); ChannelDescUnpack(ctx, s);
EXTERN void ChannelDescPack(struct ChannelDesc *s, struct SerialBuf *ctx);
EXTERN void ChannelDescUnpack(struct SerialBuf *ctx, struct ChannelDesc *s);

#define     TuneChannelDescBits 1157
#define     TuneChannelDescPackInit(s, ctx)  SerialBufRestart(ctx); TuneChannelDescPack(s, ctx);
#define     TuneChannelDescUnpackInit(ctx, s)  SerialBufRestart(ctx); TuneChannelDescUnpack(ctx, s);
EXTERN void TuneChannelDescPack(struct TuneChannelDesc *s, struct SerialBuf *ctx);
EXTERN void TuneChannelDescUnpack(struct SerialBuf *ctx, struct TuneChannelDesc *s);


#define     TSItemParametersBits 16
#define     TSItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); TSItemParametersPack(s, ctx);
#define     TSItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); TSItemParametersUnpack(ctx, s);
EXTERN void TSItemParametersPack(struct TSItemParameters *s, struct SerialBuf *ctx);
EXTERN void TSItemParametersUnpack(struct SerialBuf *ctx, struct TSItemParameters *s);

#define     SIPSIItemParametersBits 18
#define     SIPSIItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); SIPSIItemParametersPack(s, ctx);
#define     SIPSIItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); SIPSIItemParametersUnpack(ctx, s);
EXTERN void SIPSIItemParametersPack(struct SIPSIItemParameters *s, struct SerialBuf *ctx);
EXTERN void SIPSIItemParametersUnpack(struct SerialBuf *ctx, struct SIPSIItemParameters *s);

#define     MPEFECItemParametersBits 48
#define     MPEFECItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); MPEFECItemParametersPack(s, ctx);
#define     MPEFECItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); MPEFECItemParametersUnpack(ctx, s);
EXTERN void MPEFECItemParametersPack(struct MPEFECItemParameters *s, struct SerialBuf *ctx);
EXTERN void MPEFECItemParametersUnpack(struct SerialBuf *ctx, struct MPEFECItemParameters *s);

#define     FIGItemParametersBits 32
#define     FIGItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); FIGItemParametersPack(s, ctx);
#define     FIGItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); FIGItemParametersUnpack(ctx, s);
#define     FIGItemParametersPack(s, ctx)   ATSCChannelParametersPack((struct ATSCChannelParameters *)(s), ctx)
#define     FIGItemParametersUnpack(ctx, s)   ATSCChannelParametersUnpack(ctx, (struct ATSCChannelParameters *)(s))

#define     TDMBItemParametersBits 32
#define     TDMBItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); TDMBItemParametersPack(s, ctx);
#define     TDMBItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); TDMBItemParametersUnpack(ctx, s);
EXTERN void TDMBItemParametersPack(struct TDMBItemParameters *s, struct SerialBuf *ctx);
EXTERN void TDMBItemParametersUnpack(struct SerialBuf *ctx, struct TDMBItemParameters *s);

#define     DABItemParametersBits 33
#define     DABItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); DABItemParametersPack(s, ctx);
#define     DABItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); DABItemParametersUnpack(ctx, s);
EXTERN void DABItemParametersPack(struct DABItemParameters *s, struct SerialBuf *ctx);
EXTERN void DABItemParametersUnpack(struct SerialBuf *ctx, struct DABItemParameters *s);

#define     DABPacketItemParametersBits 34
#define     DABPacketItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); DABPacketItemParametersPack(s, ctx);
#define     DABPacketItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); DABPacketItemParametersUnpack(ctx, s);
EXTERN void DABPacketItemParametersPack(struct DABPacketItemParameters *s, struct SerialBuf *ctx);
EXTERN void DABPacketItemParametersUnpack(struct SerialBuf *ctx, struct DABPacketItemParameters *s);

#define     PESVItemParametersBits 32
#define     PESVItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); PESVItemParametersPack(s, ctx);
#define     PESVItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); PESVItemParametersUnpack(ctx, s);
#define     PESVItemParametersPack(s, ctx)   ATSCChannelParametersPack((struct ATSCChannelParameters *)(s), ctx)
#define     PESVItemParametersUnpack(ctx, s)   ATSCChannelParametersUnpack(ctx, (struct ATSCChannelParameters *)(s))

#define     PESAItemParametersBits 32
#define     PESAItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); PESAItemParametersPack(s, ctx);
#define     PESAItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); PESAItemParametersUnpack(ctx, s);
#define     PESAItemParametersPack(s, ctx)   ATSCChannelParametersPack((struct ATSCChannelParameters *)(s), ctx)
#define     PESAItemParametersUnpack(ctx, s)   ATSCChannelParametersUnpack(ctx, (struct ATSCChannelParameters *)(s))

#define     PESOItemParametersBits 32
#define     PESOItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); PESOItemParametersPack(s, ctx);
#define     PESOItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); PESOItemParametersUnpack(ctx, s);
#define     PESOItemParametersPack(s, ctx)   ATSCChannelParametersPack((struct ATSCChannelParameters *)(s), ctx)
#define     PESOItemParametersUnpack(ctx, s)   ATSCChannelParametersUnpack(ctx, (struct ATSCChannelParameters *)(s))

#define     PCRItemParametersBits 32
#define     PCRItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); PCRItemParametersPack(s, ctx);
#define     PCRItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); PCRItemParametersUnpack(ctx, s);
#define     PCRItemParametersPack(s, ctx)   ATSCChannelParametersPack((struct ATSCChannelParameters *)(s), ctx)
#define     PCRItemParametersUnpack(ctx, s)   ATSCChannelParametersUnpack(ctx, (struct ATSCChannelParameters *)(s))

#define     MPEIFECItemParametersBits 80
#define     MPEIFECItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); MPEIFECItemParametersPack(s, ctx);
#define     MPEIFECItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); MPEIFECItemParametersUnpack(ctx, s);
EXTERN void MPEIFECItemParametersPack(struct MPEIFECItemParameters *s, struct SerialBuf *ctx);
EXTERN void MPEIFECItemParametersUnpack(struct SerialBuf *ctx, struct MPEIFECItemParameters *s);

#define     CMMBMFSItemParametersBits 32
#define     CMMBMFSItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); CMMBMFSItemParametersPack(s, ctx);
#define     CMMBMFSItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBMFSItemParametersUnpack(ctx, s);
#define     CMMBMFSItemParametersPack(s, ctx)   ATSCChannelParametersPack((struct ATSCChannelParameters *)(s), ctx)
#define     CMMBMFSItemParametersUnpack(ctx, s)   ATSCChannelParametersUnpack(ctx, (struct ATSCChannelParameters *)(s))

#define     CMMBCITItemParametersBits 16
#define     CMMBCITItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); CMMBCITItemParametersPack(s, ctx);
#define     CMMBCITItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBCITItemParametersUnpack(ctx, s);
#define     CMMBCITItemParametersPack(s, ctx)   TSItemParametersPack((struct TSItemParameters *)(s), ctx)
#define     CMMBCITItemParametersUnpack(ctx, s)   TSItemParametersUnpack(ctx, (struct TSItemParameters *)(s))

#define     CMMBSVCItemParametersBits 48
#define     CMMBSVCItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); CMMBSVCItemParametersPack(s, ctx);
#define     CMMBSVCItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBSVCItemParametersUnpack(ctx, s);
EXTERN void CMMBSVCItemParametersPack(struct CMMBSVCItemParameters *s, struct SerialBuf *ctx);
EXTERN void CMMBSVCItemParametersUnpack(struct SerialBuf *ctx, struct CMMBSVCItemParameters *s);

#define     CMMBXPEItemParametersBits 32
#define     CMMBXPEItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); CMMBXPEItemParametersPack(s, ctx);
#define     CMMBXPEItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); CMMBXPEItemParametersUnpack(ctx, s);
#define     CMMBXPEItemParametersPack(s, ctx)   ATSCChannelParametersPack((struct ATSCChannelParameters *)(s), ctx)
#define     CMMBXPEItemParametersUnpack(ctx, s)   ATSCChannelParametersUnpack(ctx, (struct ATSCChannelParameters *)(s))

#define     ATSCMHIPItemParametersBits 19
#define     ATSCMHIPItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); ATSCMHIPItemParametersPack(s, ctx);
#define     ATSCMHIPItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); ATSCMHIPItemParametersUnpack(ctx, s);
EXTERN void ATSCMHIPItemParametersPack(struct ATSCMHIPItemParameters *s, struct SerialBuf *ctx);
EXTERN void ATSCMHIPItemParametersUnpack(struct SerialBuf *ctx, struct ATSCMHIPItemParameters *s);

#define     LOWSPEEDTSItemParametersBits 16
#define     LOWSPEEDTSItemParametersPackInit(s, ctx)  SerialBufRestart(ctx); LOWSPEEDTSItemParametersPack(s, ctx);
#define     LOWSPEEDTSItemParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); LOWSPEEDTSItemParametersUnpack(ctx, s);
#define     LOWSPEEDTSItemParametersPack(s, ctx)   TSItemParametersPack((struct TSItemParameters *)(s), ctx)
#define     LOWSPEEDTSItemParametersUnpack(ctx, s)   TSItemParametersUnpack(ctx, (struct TSItemParameters *)(s))

#define     ItemParametersBits 80
#define     ItemParametersPackInit(s, id, ctx)  SerialBufRestart(ctx); ItemParametersPack(s, id, ctx);
#define     ItemParametersUnpackInit(ctx, id, s)  SerialBufRestart(ctx); ItemParametersPack(ctx, id, s);
EXTERN void ItemParametersPack(union ItemParameters *s, int id, struct SerialBuf *ctx);
EXTERN void ItemParametersUnpack(struct SerialBuf *ctx, int id, union ItemParameters *s);

#define     EFUSEInfoParametersBits 1
#define     EFUSEInfoParametersPackInit(s, ctx)  SerialBufRestart(ctx); EFUSEInfoParametersPack(s, ctx);
#define     EFUSEInfoParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); EFUSEInfoParametersUnpack(ctx, s);
EXTERN void EFUSEInfoParametersPack(struct EFUSEInfoParameters *s, struct SerialBuf *ctx);
EXTERN void EFUSEInfoParametersUnpack(struct SerialBuf *ctx, struct EFUSEInfoParameters *s);

#define     SDRAMInfoParametersBits 9
#define     SDRAMInfoParametersPackInit(s, ctx)  SerialBufRestart(ctx); SDRAMInfoParametersPack(s, ctx);
#define     SDRAMInfoParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); SDRAMInfoParametersUnpack(ctx, s);
EXTERN void SDRAMInfoParametersPack(struct SDRAMInfoParameters *s, struct SerialBuf *ctx);
EXTERN void SDRAMInfoParametersUnpack(struct SerialBuf *ctx, struct SDRAMInfoParameters *s);

#define     GPIOInfoParametersBits 416
#define     GPIOInfoParametersPackInit(s, ctx)  SerialBufRestart(ctx); GPIOInfoParametersPack(s, ctx);
#define     GPIOInfoParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); GPIOInfoParametersUnpack(ctx, s);
EXTERN void GPIOInfoParametersPack(struct GPIOInfoParameters *s, struct SerialBuf *ctx);
EXTERN void GPIOInfoParametersUnpack(struct SerialBuf *ctx, struct GPIOInfoParameters *s);

#define     EFUSETUNERInfoParametersBits 8
#define     EFUSETUNERInfoParametersPackInit(s, ctx)  SerialBufRestart(ctx); EFUSETUNERInfoParametersPack(s, ctx);
#define     EFUSETUNERInfoParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); EFUSETUNERInfoParametersUnpack(ctx, s);
EXTERN void EFUSETUNERInfoParametersPack(struct EFUSETUNERInfoParameters *s, struct SerialBuf *ctx);
EXTERN void EFUSETUNERInfoParametersUnpack(struct SerialBuf *ctx, struct EFUSETUNERInfoParameters *s);

#define     TUNERINDENTITYInfoParametersBits 8
#define     TUNERINDENTITYInfoParametersPackInit(s, ctx)  SerialBufRestart(ctx); TUNERINDENTITYInfoParametersPack(s, ctx);
#define     TUNERINDENTITYInfoParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); TUNERINDENTITYInfoParametersUnpack(ctx, s);
#define     TUNERINDENTITYInfoParametersPack(s, ctx)   EFUSETUNERInfoParametersPack((struct EFUSETUNERInfoParameters *)(s), ctx)
#define     TUNERINDENTITYInfoParametersUnpack(ctx, s)   EFUSETUNERInfoParametersUnpack(ctx, (struct EFUSETUNERInfoParameters *)(s))

#define     TOGGLEMPEGInfoParametersBits 1
#define     TOGGLEMPEGInfoParametersPackInit(s, ctx)  SerialBufRestart(ctx); TOGGLEMPEGInfoParametersPack(s, ctx);
#define     TOGGLEMPEGInfoParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); TOGGLEMPEGInfoParametersUnpack(ctx, s);
#define     TOGGLEMPEGInfoParametersPack(s, ctx)   EFUSEInfoParametersPack((struct EFUSEInfoParameters *)(s), ctx)
#define     TOGGLEMPEGInfoParametersUnpack(ctx, s)   EFUSEInfoParametersUnpack(ctx, (struct EFUSEInfoParameters *)(s))

#define     PERPERIODInfoParametersBits 8
#define     PERPERIODInfoParametersPackInit(s, ctx)  SerialBufRestart(ctx); PERPERIODInfoParametersPack(s, ctx);
#define     PERPERIODInfoParametersUnpackInit(ctx, s)  SerialBufRestart(ctx); PERPERIODInfoParametersUnpack(ctx, s);
#define     PERPERIODInfoParametersPack(s, ctx)   EFUSETUNERInfoParametersPack((struct EFUSETUNERInfoParameters *)(s), ctx)
#define     PERPERIODInfoParametersUnpack(ctx, s)   EFUSETUNERInfoParametersUnpack(ctx, (struct EFUSETUNERInfoParameters *)(s))

#define     InfoParametersBits 20
#define     InfoParametersPackInit(s, id, ctx)  SerialBufRestart(ctx); InfoParametersPack(s, id, ctx);
#define     InfoParametersUnpackInit(ctx, id, s)  SerialBufRestart(ctx); InfoParametersPack(ctx, id, s);
EXTERN void InfoParametersPack(union InfoParameters *s, int id, struct SerialBuf *ctx);
EXTERN void InfoParametersUnpack(struct SerialBuf *ctx, int id, union InfoParameters *s);



#endif /*__MSG_TYPE_STRUCT__*/


