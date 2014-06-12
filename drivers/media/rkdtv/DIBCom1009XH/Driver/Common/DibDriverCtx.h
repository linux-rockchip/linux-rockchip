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
* @file "DibDriverCtx.h"
* @brief Generic Driver context structures definition.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_CTX_H
#define DIB_DRIVER_CTX_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define DIB_MICROCODE_A_KEY 0x1234
#define DIB_MICROCODE_B_KEY 0x1234

/*  This file REQUIRES some types to have been defined */
/*  That can be done through DibDriverTargetTypes.h */
/*#ifndef DIB_DRIVER_TARGET_TYPES_H
#error DibDriverCtx.h requires DibDriverTargetTypes.h
#endif*/
#include "../Driver2Bridge/DibDriverTargetTypes.h"
#include "DibDriverIf.h"

/*#ifndef DIB_DRIVER_IF_H
#error DibDriverCtx.h requires DibDriverIf.h
#endif*/

#include "DibBoardParameters.h"
#include "DibDriverTarget.h"
#include "DibDriverIntDbg.h"
#include "DibMsgFields.h"
#include "DibStruct.h"

#if (USE_DRAGONFLY == 1)
#include "DibDriverDragonflyIntDbg.h"
#endif

#if (DIB_INTERNAL_MONITORING == 1)
#include "DibDriverDebug.h"
#endif

#define PID_FILTER_EMPTY    0xFFFF

/* Possible interrupted while the driver is waiting for an MsgAckEvent: */
#define DIB_INTERRUPT_REQUEST_BINO     0

/**
 * DibDriverHardwareDesc
 */
struct DibDriverHardwareDesc
{
   uint8_t Diversity[MAX_NUMBER_OF_FRONTEND];
   uint8_t DemodI2CAdd[MAX_NUMBER_OF_FRONTEND];
   uint8_t TunerI2CAdd[MAX_NUMBER_OF_FRONTEND];
   struct  DibDemodBoardConfig *BoardConfig;
#if (USE_DRAGONFLY == 1)
   struct  dibBoard            *pBoard;
   struct  dibFrontend         *pFe;
#endif
};

/**
 * DibDriverMonitChannelCtxt
 */
struct DibDriverMonitChannelCtxt
{
   /* Event waited upon when we tune or request channel information */
   DIB_EVENT            MonitEvent;

   /* Each TSChannel has a persistent monitoring, and keeps Data about its channel */
   struct DibTuneMonit  TuneMonit;

   DibTuneMonitCallback pTuneMonitCb;
   void                *pTuneMonitCtx;
};

/**
 * DibDriverDmaCtx
 * WARNING: every modification in DibBridgeDmaCtx must be reported into this structure to ensure driver/bridge coherency and
 * GPL constraints.
 */
struct DibDriverDmaCtx
{
   uint32_t                 DmaLen;       /* Number of bytes to transfert              */
   uint32_t                 DmaSize;      /* Internal use: real DMA length             */
   uint32_t                 ChipBaseMin;  /*                                           */
   uint32_t                 ChipBaseMax;  /*                                           */
   uint32_t                 ChipAddr;     /* Address in the chip                       */
   uint32_t                 FmtChipAddr;  /* Internal use: formatted address           */
   uint8_t                 *pHostAddr;    /* Address in the Host                       */
   uint8_t                  Dir;          /* DIBDRIVER_DMA_READ or DIBDRIVER_DMA_WRITE */
   uint8_t                  Mode;         /* DIBBRIGDE_BIT_MODE_8, 16 or 32            */
};

/**
 * DVBH Monitoring
*/
struct DvbHMonit
{
   /*  Tables */
   uint32_t TotalTables;
   uint32_t ErrorTablesBeforeFec;
   uint32_t ErrorTablesAfterFec;

   /*  Data */
   uint8_t  MpePadding;
   uint8_t  MpePuncturing;

   /*  Time */
   uint16_t DeltaTStream;
   uint16_t DeltaTDetected;
   uint16_t BurstTimeStream;
   uint16_t BurstTimeDetected;
   uint16_t PowerupTime;

   uint32_t CcFailCnt;
   uint32_t ErrCnt;
};

struct DvbSHMonit
{
   /* ADST Tables Info */
   uint32_t AdstTotalTables;    
   uint32_t AdstTablesLost;     
   uint32_t AdstTablesCorrected;
                                
   uint32_t AdstTablesFailed;   
   uint32_t AdstTablesGood;     
                                

   /* ADT Tables Info  */
   uint32_t AdtTotalTables;     
                                
   uint32_t AdtTablesCorrected; 
   uint32_t AdtTablesFailed;    
   uint32_t AdtTablesGood;      
                                
                                
   /* Burst Info */
   uint16_t DeltaTStream;         
   uint16_t DeltaTDetected;       
   uint16_t BurstStream;          
   uint16_t BurstDetected;        
   uint16_t PowerUpTime;

   uint32_t CcFailCnt;
   uint32_t ErrCnt;
};

/**
 * DVBT Monitoring
*/
struct DvbTMonit
{
   /* Data Monitoring from embedded */
   uint32_t TotalPackets;
   uint32_t CcFailCnt;
   uint32_t ErrCnt;
   /* Check RawTs data from bridge */
   uint32_t CheckTotalPackets;
   uint32_t CheckCcFailCnt;
   uint32_t CheckErrCnt;
};

/**
 * Mpe service context
 */
struct DibDriverMpeServiceCtx
{
   uint32_t RsNumRows;
   uint32_t MaxBurstDuration;
   uint16_t Pid;
   uint8_t  OnlyIfPrioritaryChannel;

   struct DvbHMonit DvbHMonitoring;
};

/**
 * Mpe-IFec service context
 */
struct DibDriverMpeIFecServiceCtx
{
   uint32_t RsNumRows;
   uint32_t MaxBurstDuration;
   uint16_t Pid;
   uint8_t  OnlyIfPrioritaryChannel;

   struct DvbSHMonit DvbSHMonitoring;
};

/**
 * SIPSI service context
 */
struct DibDriverSipsiServiceCtx
{
   /* SIPSI filtering of section header */
   struct DibSipsiFilterCfg *pFilterSipsi;

   int32_t  Nb;
   uint8_t *Table;
};

/**
 * CMMB monitoring.
 */
struct CmmbMonit
{
   /* LDPC blocks. */
   uint32_t TotalLdpcBlocks;
   uint32_t ErrorLdpcBlocks;

   /* RS matrices. */
   uint32_t TotalRsMatrices;
   uint32_t ErrorRsMatricesBeforeRs;
   uint32_t ErrorRsMatricesAfterRs;

   /* Multiplex Frames. */
   uint32_t TotalMf;
   uint32_t ErrorMfBeforeRs;
   uint32_t ErrorMfAfterRs;

   /* Multiplex payload checker */
   uint32_t ErrorMf;
   uint32_t DiscountMf;
};

/**
 * CMMB service context.
 */
struct DibDriverCmmbServiceCtx
{
   uint8_t *MultiplexFrame;
   uint32_t Size;
   struct CmmbMonit CmmbMonitoring;
};

struct FicMonit
{
   uint32_t    CorrectCnt;
   uint32_t    ErrCnt;
};

/**
 * DAB FIG Service context.
 */
struct DibDriverFigServiceCtx
{
   /* monitoring */
   struct FicMonit FicMonitoring;
};

/**
 * DAB TDMB Service context.
 */
struct DibDriverTdmbServiceCtx
{
   uint32_t SubId;   
};

struct DabMonit
{
   uint32_t    TotalPackets;          /* Number of received TS packets             */
   uint32_t    Error;                 /* Number of corrupted packets               */
   /* Dab Type 0 specific */
   uint32_t    CountinuityError;      /* Number of packets with continuity break,
                                         but with valid data inside                */
   /* Dab Type 1 specific */
   uint32_t    CorrectedPackets;
};

/**
 * DAB Audio Service context.
 */
struct DibDriverDabAudioServiceCtx
{
   uint32_t SubId;   
   uint32_t Fec;   
   /* Data monitoring */
   struct DabMonit DabMonitoring;
};

struct DabPacketMonit
{
   uint32_t MscPacketGood;
   uint32_t MscPacketsError;
   uint32_t DataGroupGood;
   uint32_t DataGroupError;
};

/**
 * DAB Packet Service context.
 */
struct DibDriverDabPacketServiceCtx
{
   uint32_t SubId;   
   uint32_t Address;
   uint32_t Fec;
   uint32_t DataGroup;   
   /* Data Monitoring */
   struct DabPacketMonit DabPacketMonitoring;
};

union DibDriverItemCtx
{
   struct DibDriverSipsiServiceCtx     *pSipsiService;
   struct DibDriverMpeServiceCtx       *pMpeService;
   struct DibDriverMpeIFecServiceCtx   *pMpeIFecService;
   struct DibDriverCmmbServiceCtx      *pCmmbService;
   struct DibDriverFigServiceCtx       *pFigService;
   struct DibDriverTdmbServiceCtx      *pTdmbService;
   struct DibDriverDabAudioServiceCtx  *pDabAudioService;
   struct DibDriverDabPacketServiceCtx *pDabPacketService;
};

struct DibDriverChannel
{
   enum DibDemodType Type;

   /* Channel Descriptor Context (get channel) */
   struct DibTuneChan        ChannelDescriptor;
   DIB_EVENT                 ScanEvent;

   /* Monit Channel Context (tune monit)  */
   struct DibDriverMonitChannelCtxt MonitContext;

   /* Monit Channel Context (get signal monit)  */
   struct DibTotalDemodMonit ChannelMonit;
   DibGetSignalCallback      pGetSignalCb;
   void                     *pGetSignalCtx;
   /* In asynchrone mode, remember the number of desired demods */
   uint8_t                   NbDemodsUser;

   /* Info related to Timing Offset and LNA status. */
   uint8_t                   LnaStatus[MAX_NUMBER_OF_FRONTEND];

   uint8_t                   MinNbDemod;
   uint8_t                   MaxNbDemod;
   uint32_t                  StreamParameters;
   enum DibBool              IsDataChannel;  /* for each channel associated with a same adapter, one and only one must be active */
   uint8_t                   InUse;
   FILTER_HDL                FirstFilter;     /** address (in the global variable) of the first filter of this channel (init : 0xFF) */
   uint8_t                   Temporary;     /** temporary channel will be deleted at the end (dragonfly_based case) */
   struct DibStream         *pStream;
};

struct DibDriverFilter
{
   /*  Reconstructed SI/PSI storage and associated Lock */
   uint8_t              NbActivePids;       /** Number of raw ts active pids */

   enum DibDataType     TypeFilter;         /** Type of filter (SIPSI, Data, FIC, ...) */
   uint8_t              FirstBuf;
   CHANNEL_HDL          ParentChannel;      /** address of the associated channel of this filter */
   FILTER_HDL           NextFilter;         /** address of the next filter of the same channel */
   ELEM_HDL             FirstItem;          /** address of the first item of this filter */
   struct DibDataBuffer DataInfo;
   uint8_t              FlushActive;

   /** DVB-T/TDMB monitoring */
   struct DvbTMonit     DvbTMonitoring;
};

struct DibDriverItem
{
   union DibFilters        Config;         /** Config of this filter, depending of the concerned Type of filter */
   FILTER_HDL              ParentFilter;   /** address of the associated filter of this item */
   ELEM_HDL                NextItem;       /** address of the next item of the same filter */
   union DibDriverItemCtx  ItemCtx;        /** Item specific context */

   uint8_t                 FlushAcq;

#if (USE_FIREFLY == 1)
   uint8_t  ServId;
#endif
   ELEM_HDL                AliasItem;     /** Item aliasing at SDK level, if not supported at firmware level */
};


/**
*  Firefly Context
*/
struct DibDriverChipCtx
{
   uint32_t APBBridgeAddressRead;
   uint32_t APBBridgeDataRead;

   uint8_t  I2CBridgeDataRead[I2C_BRIDGE_FRAME_SIZE];

   /*  Embedded versions */
   uint16_t  EmbVersions[2];
   uint32_t  SupportedModes;
   DIB_EVENT GlobalMonitEvent;

   uint32_t FwDwldData;

   uint32_t HalVersion;
   char     HalVersionExt[9];
};

/**
* Firefly Service Info
*/
struct DibDriverFireflyService
{
   uint16_t TsChannelNumber;        /* Points to a TS channel no. */
   uint16_t ValidPid;
   uint32_t ItemIndex;
};

struct DibDriverBufCtxList
{
   struct DibBufContext Ctx;
   uint8_t              NextBuf;
};

struct DibDriverDataInfoUp
{
   uint32_t Length;
   ELEM_HDL ItemIndex;
   uint8_t  Flags;
};

struct DibDriverEventInfoUp
{
   enum DibEvent        EventType;
   union DibEventConfig EventConfig;
};

struct DibGetEventCallbackDescriptor
{
   DibGetEventCallback   List;
   void                * pContext;
};

struct DibDriverApiMsgCtx
{
   DIBSTATUS             Status;
   uint32_t            * pSize;           /* Buffer size in bytes */
   uint8_t             * pBuffer;         /* Answer buffer */
};

struct DibDriverApiMsgCbCtx
{
   uint32_t              BufferSize;      /* Buffer size in bytes */
   uint8_t             * pBuffer;         /* Answer buffer */
   DibMessageCallback    pCallback;       /* Registered callback */
   void *              * pCallbackCtx;    /* Callback context */
};

/* These fields are filled in driver context uppon REQUEST_BINO_DWNLD message reception to process
 * in the user context of GetChannel or TuneMonit to the octocode download procedure */
struct DibDriverDragonflyFirmwareDwnld
{
   char     FirmwareName[FIRMWARE_NAME_LEN];
   uint32_t Address;
   uint32_t MaxLength;
   uint32_t LengthAck;
   uint32_t FileOffset;
   uint8_t  Raw;
};

struct DibStream
{
   struct DibStreamAttributes Attributes;

   /* private */
   uint32_t                   StreamId;         /** unique ID of this frontend in the system, that is the StreamNum */
   uint8_t                    Used;             /** True if allocated sucessfully */
   uint8_t                    AutoRemove;       /** the stream must be removed once last channel is removed (bw compat) */
   uint8_t                    NbConnFrontends;  /** Number of linked inputs */
   uint8_t                    ChipTimeSlicing;  /** Is TimeSlicing enabled ? */
   uint32_t                   Options;          /** selected options */
   enum DibDemodType          Std;              /** selected standard */
   struct DibFrontend *       pInput;           /** Linked list of connected frontends */
};

struct DibFrontend
{
   struct DibFrontendAttributes Attributes;

   /* private */
   uint32_t             FeId;       /* unique ID of this frontend in the system */
   struct DibStream *   pStream;    /* NULL is not active otherwise the DibStream this Frontend is plugged to */
   struct DibFrontend * pInput;     /* Linked list of frontends */
};

struct DibChipOperations
{
   /*----------------------------- Mac initialisation --------------------------------------------*/
   void      (*MacResetCpu)                        (struct DibDriverContext * pContext);
   void      (*MacInitCpu)                         (struct DibDriverContext * pContext);
   void      (*MacStartCpu)                        (struct DibDriverContext * pContext);
   DIBSTATUS (*MacInitMailbox)                     (struct DibDriverContext * pContext);
   DIBSTATUS (*MacUploadMicrocode)                 (struct DibDriverContext * pContext);
   DIBSTATUS (*MacWaitForFirmware)                 (struct DibDriverContext * pContext);
   uint32_t  (*MacTestRamInterface)                (struct DibDriverContext * pContext);
   void      (*MacEnableClearIrq)                  (struct DibDriverContext * pContext);

   /*----------------------------- Register interface --------------------------------------------*/
   void      (*RegisterIfInit)                     (struct DibDriverContext * pContext);
   uint16_t  (*ReadDemod)                          (struct DibDriverContext * pContext, uint8_t DemId,  uint32_t SubAdd);
   DIBSTATUS (*WriteDemod)                         (struct DibDriverContext * pContext, uint8_t DemId,  uint32_t SubAdd, uint16_t Value);

   /*----------------------------- Frontend initialisation ---------------------------------------*/
   DIBSTATUS (*FrontendInit)                       (struct DibDriverContext * pContext);
   uint32_t  (*FrontendRfTotalGain)                (uint8_t LnaStatus, uint32_t agc_global, struct DibDriverContext * pContext);

   /*----------------------------- Packet demux for sipsi ----------------------------------------*/
   DIBSTATUS  (*GetSiPsiInfo)                      (struct DibDriverContext * pContext, uint8_t * pPacketAddr, int32_t * pPacketSize, uint32_t * pItemIdx, uint8_t  * StartSection, uint8_t * EndSection, uint8_t * CrcResult, uint8_t *pHeaderLen, uint8_t *pCrcLen);


   /*----------------------------- Handler to receive messages -----------------------------------*/
   void       (*ProcessMessage)                    (struct DibDriverContext *pContext, uint32_t Size, uint16_t *pData);

   /*--------------------------- Operations for sending messages ---------------------------------*/
   DIBSTATUS   (*SetDebugHelp)                     (struct DibDriverContext *pContext, uint16_t i, uint16_t v, uint16_t ch);
   DIBSTATUS   (*ApbRead)                          (struct DibDriverContext *pContext, uint32_t adr);
   DIBSTATUS   (*ApbWrite)                         (struct DibDriverContext *pContext, uint32_t adr, uint32_t Data);
   DIBSTATUS   (*I2CWrite)                         (struct DibDriverContext *pContext, uint8_t I2cAdr, uint8_t * txbuf, uint8_t nbtx);
   DIBSTATUS   (*AbortTuneMonitChannel)            (struct DibDriverContext *pContext);
   DIBSTATUS   (*TuneMonitChannel)                 (struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl,  uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibTuneMonit * pTuneMonit);
   DIBSTATUS   (*TuneMonitChannelEx)               (struct DibDriverContext *pContext, struct DibStream * pStream, CHANNEL_HDL ChannelHdl, struct DibTuneMonit * pTuneMonit);
   DIBSTATUS   (*InitDemod)                        (struct DibDriverContext *pContext, uint16_t FeId);
   DIBSTATUS   (*GetMonit)                         (struct DibDriverContext *pContext, uint8_t TsChannelNb);
   DIBSTATUS   (*SubbandSelect)                    (struct DibDriverContext *pContext, struct DibDemodSubbandSelection *pConfig, uint16_t FeId);
   DIBSTATUS   (*SetCfgGpio)                       (struct DibDriverContext *pContext, struct DibDemodGpioFn *pConfig, uint16_t FeId);
   DIBSTATUS   (*GetEmbeddedInfo)                  (struct DibDriverContext *pContext);
   DIBSTATUS   (*SetHbm)                           (struct DibDriverContext *pContext, uint8_t buffering_mode);
   DIBSTATUS   (*EnableCas)                        (struct DibDriverContext *pContext, enum DibBool Enable);
   DIBSTATUS   (*I2CRead)                          (struct DibDriverContext *pContext, uint8_t I2cAdr, uint8_t * txbuf, uint8_t nbtx, uint8_t * rxbuf, uint8_t nbrx);
   DIBSTATUS   (*GetChannel)                       (struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibChannel *pDescriptor, CHANNEL_HDL * pChannelHdl);
   DIBSTATUS   (*DeleteChannel)                    (struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl);
   DIBSTATUS   (*CreateFilter)                     (struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer * pDataBuf, FILTER_HDL * pFilterHdl);
   DIBSTATUS   (*DeleteFilter)                     (struct DibDriverContext *pContext, FILTER_HDL FilterHdl);
   DIBSTATUS   (*AddItem)                          (struct DibDriverContext *pContext, FILTER_HDL FilterHdl, union DibFilters * pFilterDesc, ELEM_HDL * pElemHdl);
   DIBSTATUS   (*RemoveItem)                       (struct DibDriverContext *pContext, ELEM_HDL ElemHdl);
   DIBSTATUS   (*SetPowerManagement)               (struct DibDriverContext *pContext, enum DibBool TimeSlicing, STREAM_HDL StreamHdl);
   DIBSTATUS   (*GetPowerManagement)               (struct DibDriverContext *pContext, STREAM_HDL StreamHdl, enum DibBool * Enable);
   DIBSTATUS   (*SetPidMode)                       (struct DibDriverContext *pContext, struct DibPidModeCfg * pPidInfo);
   DIBSTATUS   (*DwldSlaveFw)                      (struct DibDriverContext *pContext, uint16_t FeId);
   DIBSTATUS   (*ClearMonit)                       (struct DibDriverContext *pContext, ELEM_HDL ItemHdl);
   DIBSTATUS   (*SetCfg)                           (struct DibDriverContext *pContext, uint32_t Cfg);
   DIBSTATUS   (*DeInitFirmware)                   (struct DibDriverContext *pContext);
   DIBSTATUS   (*SendMsgApi)                       (struct DibDriverContext *pContext, uint16_t MsgType, uint16_t MsgIndex, uint8_t *pSendBuf, uint32_t SendLen, uint8_t *pRecvBuf, uint32_t * pRecvLen);
   DIBSTATUS   (*EnableMsgApiUp)                   (struct DibDriverContext *pContext, uint16_t MsgType, enum DibBool Enable);
   DIBSTATUS   (*GetStreamInfo)                    (struct DibDriverContext *pContext, uint32_t StreamNum);
   DIBSTATUS   (*GetLayoutInfo)                    (struct DibDriverContext *pContext);
   DIBSTATUS   (*GetFrontendInfo)                  (struct DibDriverContext *pContext, uint32_t FeNum);
   DIBSTATUS   (*GetStream)                        (struct DibDriverContext *pContext, struct DibStream * pStream, enum DibDemodType Std, uint32_t Options, enum DibPowerMode EnPowerSaving);
   DIBSTATUS   (*DeleteStream)                     (struct DibDriverContext *pContext, struct DibStream * pStream);
   DIBSTATUS   (*AddFrontend)                      (struct DibDriverContext *pContext, struct DibStream * pStream, struct DibFrontend * pFe, struct DibFrontend * pOutputFe);
   DIBSTATUS   (*RemoveFrontend)                   (struct DibDriverContext *pContext, struct DibFrontend * pFe);
#if ((WRITE_FLASH_SUPPORT == eWRFL_STATIC) || (WRITE_FLASH_SUPPORT == eWRFL_FILE))
   DIBSTATUS   (*ProgramFlash)                     (struct DibDriverContext *pContext, char* filename);
#endif

   /*--------------------------- Internal Debug Operations ---------------------------------*/
#if (DIB_INTERNAL_DEBUG == 1)
   DIBSTATUS   (*SubmitBlock)                      (struct DibDriverContext *pContext, struct DibBlock *pBlock);
#endif
   DIBSTATUS   (*GetChannelEx)                     (struct DibDriverContext *pContext, struct DibStream * pStream, struct DibChannel *pDescriptor, CHANNEL_HDL * pChannelHdl);
};


struct DibDriverContext
{
   /* device open/not open */
   enum DibBool                            Open;
   enum DibBool                            TargetInit;

   /* Hardware Descriptor. */
   struct DibDriverHardwareDesc            Hd;
   BOARD_HDL                               BoardHdl;
   uint32_t                                DibChip;
   enum DibBoardType                       BoardType;

   /* Chipset specific operations */
   struct DibChipOperations                ChipOps;

   /* Message context */
   struct SerialBuf                        TxSerialBuf;
   struct SerialBuf                        RxSerialBuf;
   uint32_t                                * TxBuffer;

   /* TS Channel Contexts : firefly case */
   DIB_LOCK UserLock;
   DIB_LOCK MonitLock;
   DIB_LOCK ChannelLock;
   DIB_LOCK SignalMonitLock;
   DIB_LOCK BridgeRWLock;

   DIB_EVENT                               MsgAckEvent;
   uint8_t                                 MsgAckStatus;
   uint8_t                                 TuneStatus;
   uint8_t                                 MsgAckIntType; /* if MsgAckStatus == INTERRUPTED */

   /* Channel Contexts : dragonfly case */
   struct DibDriverChannel                 ChannelInfo[DIB_MAX_NB_CHANNELS];
   struct DibDriverFilter                  FilterInfo[DIB_MAX_NB_FILTERS];
   struct DibDriverItem                    ItemInfo[DIB_MAX_NB_ITEMS];

   struct DibDriverBufCtxList              CallbackBufCtx[DIB_MAX_NB_BUF_CONTEXT];
   DIB_EVENT                               BridgeRWEvent;
   DIB_EVENT                               SvcChangeEvent;
   DIB_EVENT                               FWDwldEvent;

   /* Message API */
   struct DibDriverApiMsgCtx               ApiMsgCtx;    /* MsgApi synchronous answer context */
   struct DibDriverApiMsgCbCtx             ApiMsgCbCtx;  /* MsgApi asynchronous callback context */

   uint8_t                                 FireflyDiversityActiv;

   struct DibDriverChipCtx                 ChipContext;

#if (DIB_INTERNAL_DEBUG == 1)
   #if (DIB_CMMB_DATA == 1)
      struct DibDriverChannelDecoder       ChannelDecoder;
   #endif

   ProcessIntDbgMsgCb                      pIntDbgProcessMsgCb;
#endif

#if (USE_FIREFLY == 1)
   /* firefly specific structures */
   uint8_t                                 FireflyChannel[DIB_MAX_NB_TS_CHANNELS];   /* TsCh  => ChannelIndex convertion */
   struct DibDriverFireflyService          FireflyService[DIB_MAX_NB_SERVICES];      /* SvcNb => ItemIndex convertion    */
   enum DibBool                            FireflyChipTimeSlicing;                   /* Is TS enabled ? */
#endif

#if USE_DRAGONFLY == 1
   struct DibDriverDragonflyRegisters      DragonflyRegisters;
   struct DibDriverDragonflyFirmwareDwnld  DragonflyFirmware;
#endif

   uint32_t                                DriverDebugMask;

   struct DibGetEventCallbackDescriptor    EventCallbackDesc[eMAX_NB_EVENTS];
   uint8_t                                 EventTypeCallbackIndex[eMAX_NB_EVENTS];

   struct DibStream                        Stream[DIB_MAX_NB_OF_STREAMS];
   struct DibFrontend                      Frontend[DIB_MAX_NB_OF_FRONTENDS];
   uint32_t                                NbStreams;
   uint32_t                                NbFrontends;
   uint32_t                                NbChips;

#if (DIB_DRIVER_TARGET_CTX == 1)
   struct DibDriverTargetCtx               DriverTargetCtx;
#endif
#if (DIB_INTERNAL_MONITORING == 1)
   struct DibDriverDebugContext            DebugCtx;
#endif

};

#endif /*  DIB_DRIVER_CTX_H */
