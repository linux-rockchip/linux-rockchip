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
* @file "DibBridgeCommon.h"
* @brief Bridge functionality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_COMMON_H
#define DIB_BRIDGE_COMMON_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/************************************************************/
/* Include Target Specific                                  */
/************************************************************/
#include "../../User/Common/DibStatus.h"
#include "DibBridgeConstants.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTargetTypes.h"
#include "DibBridgeTargetCommon.h"
#include "../../User/Common/DibDefines.h"
#include "DibMsgFields.h"

/* Optimised Mode Selected */
#if(DEBUG_MODE == NO_DEBUG)

/* No logs */
#define DIB_DEBUG(_MASK_, _MSG_)

/* Remove Asserts */
#ifdef DIB_ASSERT
#undef DIB_ASSERT
#define DIB_ASSERT(foo)
#endif

#else

/* Debug Verbose Selected */
#define DIB_DEBUG(_MASK_, _MSG_) if(pContext->BridgeDebugMask & _MASK_) { DibBridgeTargetLogFunction _MSG_ ; }

/* Leave assert Implementation to Target */
#endif

/** Define Bridge2Driver mailbox size. Able to store until 10 Big sipsi sections of 4K */
#define UPMESSAGE_MAILBOX_SIZE  ((4096+4)*10)

/** Define the maximum size of the information contained in the message */
#define UPMESSAGE_MAX_SIZE              4096

/* the type returned from GetMsgFromQueue is flagged with last bit set to 1 if queue is empty */
#define UPMESSAGE_BIT_EMPTY          (1<<15)
#define UPMESSAGE_MASK_TYPE         (0x7FFF)

/** Bridge2Driver circular mailbox size. */
#if (UPMESSAGE_MAILBOX_SIZE & 0x3)
#error "Mailbox size must be 4 bytes aligned"
#endif

/** Required message size for internal messages. */
#define UPMESSAGE_MAILBOX_MIN_SIZE  1024

#if UPMESSAGE_MAILBOX_SIZE <= UPMESSAGE_MAILBOX_MIN_SIZE
#error "Mailbox size should at least be 1024"
#endif

#define MAX_DATA_MSG            50 /* MAX number of queued low priority data msg */
#define MAX_MPE_BUFFER          (32+1)
#define MAX_Q_BUFF              (20)

#if (DEMOD_TRACE == 1)
#define DTRACE_BUFF_SIZE        ((152 + 3) * 2) /* 152 registers, DWORD timestamp, BYTE state */
#endif

#define SIPSI_BUFF_SIZE         0x1000
#define HBM_SLICE_SIZE          0x2000
#define HBM_SLICE_LINES         (HBM_SLICE_SIZE / 256) /**< Number of lines(256 bytes) max in Slice buffer. Firefly limit */
#define MAX_MSG 32
#define MAX_MPE_FRAME_SIZE      (1024 * 255)  /* to indicate the item (callback Mode) */
#define MAX_MPE_NO_FEC_SIZE     (1024 * 191)  /* to indicate the item (callback Mode) */
#define TP_SIZE                 188
#define RAWTS_BUFFER_SIZE       (265 * 188)   /* to indicate the item (callback Mode) */

#define RAWTS_NB_BUFFER         2
#define RAWTS_TIMEOUT_DATA      100 

#define CMMB_RSM_MAX_SIZE       103680

/* Format */
/* FIREFLY: from FORMAT_MPE to FORMAT_DTRACE, do not modify the order */
#define FORMAT_MPE                     1
#define FORMAT_SIPSI                   2
#define FORMAT_RAWTS                   3
#define FORMAT_PES_VIDEO               4
#define FORMAT_PES_AUDIO               5
#define FORMAT_PES_OTHER               6
#define FORMAT_PCR                     7
#define FORMAT_FRG                     8
#define FORMAT_LAST_FRG                9
#define FORMAT_MPE_CORR                10

#define FORMAT_SLICE_REQ               11
#define FORMAT_SLICE_DONE              12
#define FORMAT_FLUSH_SVC               13
#define FORMAT_DTRACE                  14
#define FORMAT_FIG                     15
#define FORMAT_MSC                     16
#define FORMAT_CAV1                    17
#define FORMAT_CAV2                    18
#define FORMAT_SH_FRG_DATA             19
#define FORMAT_SH_FRG_IFEC             20
#define FORMAT_SH_PATTERN_DATA         21
#define FORMAT_SH_PATTERN_IFEC         22
#define FORMAT_SH_ALLOC                23
#define FORMAT_SH_FILL_DATA            24
#define FORMAT_SH_FILL_IFEC            25
#define FORMAT_MSCPLUS                 28
#define FORMAT_CMMB_RSM                29
#define FORMAT_MSCPACKET               31

#define PREFETCH_DATA                  0x20


typedef char   DIBDMA;

#define DIB_NO_IRQ              0         /**< No message available at bridge level */
#define DIB_NO_DMA              1         /**< No Dma have been started. */
#define DIB_DMA_DONE            2         /**< A fake dma has been done and is now finished */
#define DIB_DMA_PENDING         3         /**< A real dma has been started and we are waiting for an interrupt */
#define DIB_DMA_DEFERRED        4         /**< A dma request is wanted but should be deferred in the bottom half handler */
#define DIB_DEV_FAILED          5         /**< Device is no longer responding */

#define BUF_RDY                 0   /**< The buffer is ready for the next burst */
#define BUF_RECEPTION           1   /**< Buffer reception is started, a MPE buffer has been allocated. */
#define BUF_CORR_PENDING        2   /**< Correction is needed and we have received the last fragment. */
#define BUF_COMPLETE            3   /**< Frame is complete, correction (if needed) is finished. */


struct DibBridgeDmaFlags
{
   uint8_t           BlockId;
   enum DibBool      FirstFrag;
   uint8_t           FrameId;
   ELEM_HDL          ItemHdl;
   enum DibBool      LastFrag;
   uint8_t           NbRows;
   uint8_t           BlockType;
   uint8_t           Type;
   uint8_t           Prefetch;
};

struct DibBridgeRecordDmaFlags
{
   struct DibBridgeDmaFlags DmaFlags;
   uint8_t * Add;
   uint32_t Begin;
   uint32_t Shift;
   uint32_t Len;
   uint8_t Index;
};

struct DibBridgeServiceCtx
{
   /* This "define" directive should be present in DibBridgeTargetCommon.h */
#if (DIB_BRIDGE_TARGET_CTX == 1)
   struct DibBridgeTargetServiceCtx BridgeTargetServiceCtx;
#endif
   uint8_t  Cpt[32];

#if ((DIB_MPEFEC_DATA == 1) || (DIB_MPEIFEC_DATA == 1))
   uint8_t *MpeBufRx;
   uint8_t *MpeBufCor; 
   uint8_t  MpeRxState; /*ib reception mpe_buffer state */
   uint8_t *MpeFrag;
   uint32_t BufCorLen;
   uint16_t SkipR;                /**< Number of Rows in the compressed table */
   uint8_t  SkipC;                /**< Number of Columns in the compressed table */
   uint16_t FecOffset;
#endif

   uint32_t CcFailCnt;
   uint32_t ErrCnt;
   uint32_t DataLenRx;
   uint16_t CurCc;
   uint8_t  FilterParent;         /**< Parent Filter of the Item */
   uint32_t SliceLen;

#if (DIB_DAB_DATA == 1)
   uint32_t NbMaxFrames;
#endif

#if (DIB_MPEIFEC_DATA == 1)
   uint32_t GlobalSize;
   uint8_t *ProcBuf[2];
   uint8_t *EncodeAdd[64][2];
   uint32_t EncodLenRx[64][2];
   uint8_t *MpeBurst[32];
   struct   DibBridgeRecordDmaFlags RecordMsg[96];      /** 191*1024/4ko */
   uint8_t  FreeRecordMsg;
#endif

#if (DIB_CMMB_DATA == 1)
   uint8_t *MultiplexFrame; /**< Buffer for current Multiplex Frame. */
   uint32_t Size;           /**< Allocated size.                     */
   uint8_t PhyFrameId;      /**< Broadcast Channel Frame identifier. */
   uint32_t WrOffset;       /**< Number of bytes received so far. */
#endif /* DIB_CMMB_DATA */
};

#if DIB_CHECK_RAWTS_DATA == 1
struct CheckRawTs
{
   uint8_t          PidsCc[8192];
   uint8_t          PidsOk[8192 / 8];
   uint32_t         CorruptedPacketsCount;
   uint32_t         ErrorIndicFlag;
   uint32_t         DiscontinuitiesCount;
   uint32_t         TotalNbPackets;
   uint32_t         NbNullPackets;
   uint32_t         TotalNbPid;
};
#endif

struct DibBridgeFilterCtx
{
   enum DibDataType DataType;    /**< Type of the filter. */
   enum DibDataMode DataMode;    /**< Data delivery mode (callback or IP). if Filter not allocated, it is eUNSET. In that case ItemCount=0 and DataType=UNKNOW*/
   uint32_t         StreamId;    /**< Stream identifier linked to this filter. */
   uint8_t          ItemCount;   /**< Number of items linked to this filter. */

#if DIB_CHECK_RAWTS_DATA == 1
   struct CheckRawTs CheckRawTs;
#endif
};

struct DibBridgeDmaCtx
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
   struct DibBridgeDmaFlags DmaFlags;
};

struct DibBridgeDataMsg
{
   uint32_t Min;
   uint32_t Max;
   uint32_t Add;
   uint32_t Len;
   struct DibBridgeDmaFlags Flags;
};

struct DibBridgeRawDataCtx
{
   uint8_t    *BufList[RAWTS_NB_BUFFER];
   uint8_t     ReadPtr;
   uint8_t     WritePtr;
   FILTER_HDL  TheFireflyRawTsFilter;           /**< firefly handle rawts services in a unique rawts buffer */
} ;

/** MsgQueue used to send msg from Bridge to Driver. Warning: each MsgQueue must be 32 bits aligned because
 * the message header is directly mapped into the circular message queue buffer. The message can be splited into
 * 2 parts in the message queue, but by units of 4 bytes, so that message header (Size and Type) are always together. */
struct DibBridgeMsgQueue
{
   uint16_t Size;             /**< Size (can be any value less than UPMESSAGE_MAX_SIZE) of the buffer field in bytes. */
   uint16_t Type;             /**< Type of message */
};


/** Calculate the size of the message size. Must be used to increment pointer in MsgQueue */
#define DIB_BRIDGE_MESSAGE_SIZE(Size) ( (((Size)+3)&(~3)) + sizeof(struct DibBridgeMsgQueue))

/** Buffer descriptor used for mpe_buffer */
struct DibBridgeBufDesc
{
   uint32_t Size;    /* allocated Buffer Size */
   uint8_t *Data;    /* Buffer */
   uint8_t  ServId;  /* associated service id */
};

struct DibBridgeFireflyService
{
   ELEM_HDL ItemIndex;
};

struct DibBridgeDataInfoUp
{
   uint8_t * pBuff;
   FILTER_HDL FilterIndex;
};

struct DibDataInfoUp
{
   uint32_t Length;
   ELEM_HDL ItemIndex;
   uint8_t Flags;
};

struct DibEventInfoUp
{
   enum DibEvent        EventType;
   union DibEventConfig EventConfig;
};

enum DibBridgeHostIfMode
{
   eNONE,     
   eSRAM,     /* Bridge is communicating which chipset using SRAM interface. */
   eSDIO,     /* Bridge is communicating which chipset using SDIO interface (SD1, SD2, SD4 or SPI). */
   eI2C,       /* Bridge is communicating which chipset using I2C interface. */
   eSPI       /* Bridge is communicating which chipset using DIB SPI interface. */
};


/*********************** Specific chip operations ***************************/
struct DibBridgeContext;
struct DibBridgeChipOperations 
{
   DIBSTATUS   (* SetupDma)           (struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);
   DIBDMA      (* RequestDma)         (struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);
   uint8_t     (* GetArch)            (struct DibBridgeContext *pContext);
   DIBSTATUS   (* PreFormat)          (struct DibBridgeContext *pContext, uint8_t ByteMode,
                                       uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * Buf, uint32_t Cnt);
   DIBSTATUS   (* PostFormat)         (struct DibBridgeContext *pContext, uint8_t ByteMode,
                                       uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * Buf, uint32_t Cnt);
   uint32_t    (* IncrementFormattedAddress)  (struct DibBridgeContext *pContext, uint32_t InFmrAddr, int32_t Offset);
   uint32_t    (* AssembleSlice)      (struct DibBridgeContext *pContext,  struct DibBridgeDmaFlags *pFlags, uint32_t offs, uint32_t len);
   DIBSTATUS   (* SignalBufFail)      (struct DibBridgeContext *pContext, struct DibBridgeDmaFlags * pFlags, uint8_t Flush);
   DIBDMA      (* ProcessIrq)         (struct DibBridgeContext *pContext);
   DIBSTATUS   (* ProcessDma)         (struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx);
   DIBSTATUS   (* SendMsg)            (struct DibBridgeContext *pContext, uint32_t * Data, uint32_t len);
   DIBSTATUS   (* SendAck)            (struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint8_t failed);
   DIBSTATUS   (* ChipsetInit)        (struct DibBridgeContext *pContext);
   void        (* ChipsetDeinit)      (struct DibBridgeContext *pContext);

#if (DIB_CHECK_DATA == 1)
   void        (* ClearCheckStats)    (struct DibBridgeContext *pContext, uint32_t *buf);
   void        (* ForwardCheckStats)  (struct DibBridgeContext *pContext, ELEM_HDL Item);
#endif

   void        (*SetService)          (struct DibBridgeContext *pContext, uint8_t Svc, ELEM_HDL ItemHdl, FILTER_HDL FilterHdl, enum DibDataType DataType, enum DibDataMode DataMode);

#if (DIB_BRIDGE_HBM_PROFILER == 1)
   DIBSTATUS   (*HbmProfiler)         (struct DibBridgeContext *pContext, uint8_t idx, uint8_t page, uint8_t LastFrag);
#endif

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))
   int32_t     (* TestBasicRead)      (struct DibBridgeContext *pContext);
   int32_t     (* TestRegister)       (struct DibBridgeContext *pContext);
   int32_t     (* TestInternalRam)    (struct DibBridgeContext *pContext);
   int32_t     (* TestExternalRam)    (struct DibBridgeContext *pContext);
   uint32_t    (* GetRamAddr)         (struct DibBridgeContext *pContext);
#endif
};

#if USE_DRAGONFLY == 1
/* TODO: move it into specific part */
struct DibBridgeDragonflyRegisters
{
   uint32_t JedecAddr;                     /**< Address where to read the Jedec number                              */
   uint32_t JedecValue;                    /**< Value to be read in the Jedec address                               */
   uint32_t MacMbxSize;                    /**< The size of the MAC mailbox                                         */
   uint32_t MacMbxStart;                   /**< The start address of the MAC mailbox                                */
   uint32_t MacMbxEnd;                     /**< The end address + 4 of the MAC mailbox                              */
   uint32_t HostMbxSize;                   /**< The size of the HOST mailbox                                        */
   uint32_t HostMbxStart;                  /**< The start address of the HOST mailbox                               */
   uint32_t HostMbxEnd;                    /**< The end address + 4 of the HOST mailbox                             */
   uint32_t HostMbxRdPtrReg;               /**< The Register that own the current Read pointer in HOST mailbox      */
   uint32_t HostMbxWrPtrReg;               /**< The Register that own the current Write pointer in HOST mailbox     */
   uint32_t MacMbxRdPtrReg;                /**< The Register that own the current Read pointer in MAC mailbox       */
   uint32_t MacMbxWrPtrReg;                /**< The Register that own the current Write pointer in MAC mailbox      */
};
#endif

struct DibBridgeIpCallback 
{
   void   (* DataMpegTs)         (struct DibBridgeContext *pContext, ELEM_HDL Type, uint8_t *Buffer, uint32_t Size);
   void   (* DataMpe)            (struct DibBridgeContext *pContext, ELEM_HDL Type, uint8_t *Buffer, uint32_t Size);
   void   (* DataCmmb)           (struct DibBridgeContext *pContext, ELEM_HDL Type, uint8_t *Buffer, uint32_t Size);
   void   (* DataTdmb)           (struct DibBridgeContext *pContext, ELEM_HDL Type, uint8_t *Buffer, uint32_t Size);
   void   (* DataDab)            (struct DibBridgeContext *pContext, ELEM_HDL Type, uint8_t *Buffer, uint32_t Size);
   void   (* DataDabPlus)        (struct DibBridgeContext *pContext, ELEM_HDL Type, uint8_t *Buffer, uint32_t Size);
};


struct DibBridgeContext 
{
   /* Target access protection */
   DIB_LOCK DibTargetAccessLock;

   /* chip specific operations for the bridge */
   struct DibBridgeChipOperations BridgeChipOps;

#if (DIB_MPEFEC_DATA == 1)
   struct DibBridgeBufDesc    MpeBuffers[MAX_MPE_BUFFER];/**< Mpe Buffer Pool with various allocated sizes. */
#endif

   struct DibBridgeServiceCtx ItSvc[DIB_MAX_NB_ITEMS];       /**< Item Information needed by the bridge */
   struct DibBridgeDmaCtx     DmaCtx;                    /**< THE Dma transfert context */
   uint8_t                   *SliceBuf;                  /**< 8Ko Buffer to assemble a slice (B2F) or get corr slice (F2B) */
   uint8_t                   *SiPsiBuf;                  /**< Buffer in which SIPSI fragments (F2B) are copied, before queueing */
   struct DibBridgeDataInfoUp DataInfoUp[MAX_Q_BUFF];    /**< B2D buffers queue for callback mode. dequeued on sys_read() */
   uint8_t                    NextFreeWrBuff;            /**< Next buffer to fill */
   uint8_t                    NextFreeRdBuff;            /**< Next buffer to read */

   uint8_t                    DevFailed;

#if (DEMOD_TRACE == 1)
   uint8_t                   *DemodTraceBuf;
#endif

   struct DibBridgeRawDataCtx RawDataCtx;
   uint32_t                   BridgeDebugMask;
   uint8_t                    IrqEnabled;
   uint8_t                    ResetApb;                  /**< true if the last access was to apb demod registers. Workaround */
   uint32_t                   DibChip;                   /**< Dibcom Chip insided the board */
   BOARD_HDL                  BoardHdl;                  /**< Board handler to distinguish between several boards */

   /** Data path Type in function of the filter Type */
   struct DibBridgeFilterCtx  FilterInfo[DIB_MAX_NB_FILTERS];

   /** concern only the firefly */
   struct DibBridgeFireflyService FireflyInfo[DIB_MAX_NB_SERVICES];

   /** concern only dragonfly based */
#if USE_DRAGONFLY == 1
   struct DibBridgeDragonflyRegisters   DragonflyRegisters;
   uint32_t                   TxBuffer[30];
   struct SerialBuf           TxSerialBuf;
   struct SerialBuf           RxSerialBuf;
#endif
   uint32_t * HostBuffer;
   uint32_t RxOffset;
   uint32_t RxCnt;

   /** message queue. Used for every messages + SIPSI fragments (max 4096) delivery to the driver.*/
#if (DRIVER_AND_BRIDGE_MERGED == 0)
   uint32_t                   MsgQueue[UPMESSAGE_MAILBOX_SIZE/4];
   DIB_LOCK                   MsgQueueLock;
   DIB_EVENT                  MsgQueueEvent;
   volatile uint32_t          MsgQueueHead;
   volatile uint32_t          MsgQueueTail;
   volatile uint32_t          MsgQueueInitialized;
#endif
   
   enum DibBridgeHostIfMode   HostIfMode;                /**< Host Interface Type */

   struct DibBridgeIpCallback IpCallback;

   /* This "define" directive should be present in DibBridgeTargetCommon.h */
#if (DIB_BRIDGE_TARGET_CTX == 1)
   struct DibBridgeTargetCtx  BridgeTargetCtx;           /**< Target communication (Usb, Sdio, Sram,...) Specific info */
#endif
};

#endif
