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

/* set environment-variable DTABATCH_STDOUT=1 to force output to STDOUT instead of a temporary file
 * set environment-variable DTABATCH_PREFIX="dta: " to have each line prefixed with "dta: "
 *
 * usage: e.g.: LD_PRELOAD=Sample/LinuxDriverLibrary/libDibExpApiToBatchWriter.so Sample/Dta/OS_Linux/dta 22 */

#include "DibExtAPI.h"

#include "DibDriver.h"
#include "DibDriverIf.h"
#include "DibDriverTargetTypes.h"
#include "DibDriverCtx.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define __USE_GNU
#include <dlfcn.h>

static int __o    = -1;
static char __n[] = "dta-batch-log-XXXXXX";

#define info(args...) printf("DtaBatchWriter: " args)

/***** !!! This code does not handle multi-board-instances !!! *****/

#define CREATE_OVERRIDE(func_name, args_decl, args_call, code, postcall_code...) \
    DIBSTATUS func_name args_decl \
    { \
        static DIBSTATUS (*func) args_decl = NULL; \
        if (!func) \
            func = dlsym(RTLD_NEXT, # func_name); \
        code \
        DIBSTATUS __ret__; \
        __ret__ = func args_call; \
        postcall_code \
        return __ret__; \
    }

#define CREATE_OVERRIDE_DEFAULT(func_name, args_decl, args_call) \
    CREATE_OVERRIDE(func_name, args_decl, args_call, { dprintf(__o, "# not yet implemented '%s'\n", # func_name); } )

/* Context */
static struct {
    enum DibDataType FilterDataType[DIB_MAX_NB_FILTERS];
} Ctx = {
    .FilterDataType = { eUNKNOWN_DATA_TYPE },
};

/* Helper function */
static int isNewline = 1;
#define dprintf(fd, fmt, args...) do { \
    char *__prefix = getenv("DTABATCH_PREFIX"); \
    if (isNewline && __prefix) { \
        dprintf(fd, "%s", __prefix); \
        isNewline = 0; \
    } \
    dprintf(fd, fmt, args); \
    if (strchr(fmt, '\n') != NULL) \
        isNewline = 1; \
    \
} while (0)


static int streamHdlToNum(struct DibDriverContext *c, STREAM_HDL *s)
{
    int i;
    for (i = 0; i < DIB_MAX_NB_OF_STREAMS; i++)
        if ((STREAM_HDL)&c->Stream[i] == s)
            return i;
    return -1;
}

static int feHdlToNum(struct DibDriverContext *c, FRONTEND_HDL *f)
{
    int i;
    for (i = 0; i < DIB_MAX_NB_OF_FRONTENDS; i++)
        if ((FRONTEND_HDL)&c->Frontend[i] == f)
            return i;
    return -1;
}

/* actual overrides */
CREATE_OVERRIDE(DibOpen, (struct DibDriverContext **pContextAddr, enum DibBoardType BoardType, BOARD_HDL BoardHdl), (pContextAddr, BoardType, BoardHdl),
{
    if (__o != -1) /* not the first DibOpen-call in this session */
        info("reinit call not implemented yet\n");

    if (getenv("DTABATCH_STDOUT") != NULL) {
        __o = STDOUT_FILENO;
        info("print batch to STDOUT by user request\n");
    } else {
        __o = mkstemp(__n);
        if (__o == -1)
            __o = STDOUT_FILENO;
        info("writing to %s\n", __n);
    }
})

CREATE_OVERRIDE(DibClose, (struct DibDriverContext *pContext), (pContext),
{
    dprintf(__o, "# %s # DibClose was called\n", "quit");
    if (__o != STDOUT_FILENO)
        close(__o);
    info("DtaBatchWriter: closing %s\n", __n);
})

CREATE_OVERRIDE_DEFAULT(DibGetChannel,
    (struct  DibDriverContext *pContext, uint8_t MinNbDemod,
     uint8_t MaxNbDemod, uint32_t StreamParameters, struct  DibChannel *pDescriptor,
     CHANNEL_HDL *pChannelHdl, DibGetChannelCallback pCallBack, void *pCallCtx),
    (pContext, MinNbDemod, MaxNbDemod, StreamParameters, pDescriptor, pChannelHdl, pCallBack, pCallCtx)
)
/* if (StreamParameters[0:15] == eSTREAM_TYPE_MPEG_[01])
                                 *      -> StreamParameters[16]: 0 => Serial, 1 => Parallel
                                 *      -> StreamParameters[17]: 0 => 188,    1 => 204
                                 *      -> StreamParameters[18]: 0 => gated,  1 => continous
                                 * if (StreamParameters[0:15] == eSTREAM_TYPE_HOST_DMA (== 0))
                                 *      0
                                 */

CREATE_OVERRIDE(DibDeleteChannel,
        (struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl),
        (pContext, ChannelHdl),
{
    dprintf(__o, "dch %d\n", ChannelHdl);
})

CREATE_OVERRIDE(DibCreateFilter,
    (struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl,
            enum DibDataType DataType, struct DibDataBuffer *pDataBuf, FILTER_HDL *pFilterHdl),
    (pContext, ChannelHdl, DataType, pDataBuf, pFilterHdl),
    {},
{
    dprintf(__o, "gfi %d %d %d\n", ChannelHdl, DataType, *pFilterHdl); /* pFilterHdl gives us a good enough filter-id,
                                                                          maybe does not match with user-application's ID */
    Ctx.FilterDataType[*pFilterHdl] = DataType;
})

CREATE_OVERRIDE(DibDeleteFilter, (struct DibDriverContext *pContext, FILTER_HDL FilterHdl), (pContext, FilterHdl),
{
    dprintf(__o, "dfi %d\n", FilterHdl);
    Ctx.FilterDataType[FilterHdl] = eUNKNOWN_DATA_TYPE;
})

CREATE_OVERRIDE(DibAddToFilter,
        (struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint32_t NbElements, union DibFilters *pFilterDescriptor, ELEM_HDL *pElemHdl),
        (pContext, FilterHdl, NbElements, pFilterDescriptor, pElemHdl),
        {},
{
    if (NbElements != 1)
        dprintf(__o, "# atf with NbElements != 1 will not work. Is %d\n", NbElements);
    dprintf(__o, "atf %d ", FilterHdl);
    switch (Ctx.FilterDataType[FilterHdl]) {
    case eTS:
    case eLOWSPEEDTS:
        dprintf(__o, "%d ", pFilterDescriptor->Ts.Pid);
        break;
    case eSIPSI:
        dprintf(__o, "%d %d %d ",
            pFilterDescriptor->SiPsi.Pid,
            pFilterDescriptor->SiPsi.PidWatch,
            pFilterDescriptor->SiPsi.Crc);
        break;

    case eTDMB:
        dprintf(__o, "%d ", pFilterDescriptor->Tdmb.SubCh);
        break;

#if (DIB_EDAB_DATA == 1)
    case eEDAB:
        dprintf(__o, "%d %d ", pFilterDescriptor->eDab.Pid, pFilterDescriptor->eDab.SubCh);
        break;
#endif

    case eDAB:
        dprintf(__o, "%d %d ", pFilterDescriptor->Dab.SubCh, pFilterDescriptor->Dab.Type);
        if (pFilterDescriptor->Dab.Type == 1)
            dprintf(__o, "%d ", pFilterDescriptor->Dab.UseFec);
        break;

    case eDABPACKET:
        dprintf(__o, "%d %d %d %d ", pFilterDescriptor->DabPacket.SubCh, pFilterDescriptor->DabPacket.Address, pFilterDescriptor->DabPacket.UseFec, pFilterDescriptor->DabPacket.UseDataGroup);
        break;

    case eFIG:
        break;

    case eCMMBSVC:
        dprintf(__o, "%d %d ",
                pFilterDescriptor->CmmbSvc.Mode,
                pFilterDescriptor->CmmbSvc.Mode == eMF_ID ?
                    pFilterDescriptor->CmmbSvc.MfId : pFilterDescriptor->CmmbSvc.SvcId);
        break;
    case eATSCMHIP:
    case eATSCMHFIC:
        dprintf(__o, "%d %d ", pFilterDescriptor->AtscmhEns.Mode, pFilterDescriptor->AtscmhEns.Id);
        break;

    case eFM:
    case eANALOG:
        break;

    default:
        dprintf(__o, " # Datatype %d is not handled yet # ", Ctx.FilterDataType[FilterHdl]);
        break;
    }
    dprintf(__o, "%d\n", *pElemHdl);
#if 0
#if (DIB_PES_DATA == 1)
      case ePESVIDEO:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.PesVideo.Pid = var;
         break;
      case ePESAUDIO:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.PesAudio.Pid = var;
         break;
      case ePESOTHER:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.PesOther.Pid = var;
         break;
      case ePCR:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.Pcr.Pid = var;
         break;
#endif

#if (DIB_MPEFEC_DATA == 1)
      case eMPEFEC:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.MpeFec.Pid = var;

         DtaGetParam(&var, "Nb rows", batch_mode, fb);
         FilDesc.MpeFec.NbRows = (enum DibFecNbrows)var;

         DtaGetParam(&var, "max burst duration", batch_mode, fb);
         FilDesc.MpeFec.MaxBurstDuration = var;

         if(FilDesc.MpeFec.MaxBurstDuration == 0)
            FilDesc.MpeFec.MaxBurstDuration = DIB_MAX_BURST_TIME_UNKNOWN;

         FilDesc.MpeFec.Prefetch = eACTIVE;
         break;
#endif

#if (DIB_MPEIFEC_DATA == 1)
      case eMPEIFEC:
         DtaGetParam(&var, "Pid", batch_mode, fb);
         FilDesc.MpeIFec.Pid = var;

         DtaGetParam(&var, "Nb rows", batch_mode, fb);
         FilDesc.MpeIFec.NbRows = (enum DibFecNbrows) var;

         DtaGetParam(&var, "Encoding Parallelization (B)", batch_mode, fb);
         FilDesc.MpeIFec.EncodParal = var;

         DtaGetParam(&var, "Spreading Factor (S)", batch_mode, fb);
         FilDesc.MpeIFec.SpreadingFactor = var;

         DtaGetParam(&var, "Datagram Burst Sending Delay (D)", batch_mode, fb);
         FilDesc.MpeIFec.SendingDelay = var;

         DtaGetParam(&var, "Adst Columns (C)", batch_mode, fb);
         FilDesc.MpeIFec.NbAdstColumns = var;

         DtaGetParam(&var, "Max IFEC Sections (R)", batch_mode, fb);
         FilDesc.MpeIFec.MaxIFecSect = var;

         DtaGetParam(&var, "max burst duration", batch_mode, fb);
         FilDesc.MpeIFec.MaxBurstDuration = var;

         if(FilDesc.MpeIFec.MaxBurstDuration == 0)
            FilDesc.MpeIFec.MaxBurstDuration = DIB_MAX_BURST_TIME_UNKNOWN;

         FilDesc.MpeIFec.Prefetch = eACTIVE;
         break;
#endif
#endif

})

CREATE_OVERRIDE_DEFAULT(DibRemoveFromFilter, (struct DibDriverContext *pContext, ELEM_HDL ElemHdl), (pContext, ElemHdl) )

CREATE_OVERRIDE(DibGetSignalMonitoring,
    (struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, uint8_t *NbDemod, union DibDemodMonit *pMonit, DibGetSignalCallback pCallBack, void *pCallCtx),
    (pContext, ChannelHdl, NbDemod, pMonit, pCallBack, pCallCtx),
{
    dprintf(__o, "mnt 0 1 %d\n", ChannelHdl);
}
)

CREATE_OVERRIDE_DEFAULT(DibGetGlobalInfo,
    (struct DibDriverContext *pContext, struct DibGlobalInfo *pGlobalInfo),
    (pContext, pGlobalInfo)
)

CREATE_OVERRIDE_DEFAULT(DibAbortTuneMonitChannel, (struct DibDriverContext *pContext), (pContext) )

CREATE_OVERRIDE_DEFAULT(DibTuneMonitChannel,
    (struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, enum DibDemodType Type, struct DibTuneMonit *pMonitDescriptor, DibTuneMonitCallback pCallBack, void *pCallCtx),
    (pContext, MinNbDemod, MaxNbDemod, StreamParameters, Type, pMonitDescriptor, pCallBack, pCallCtx)
)

CREATE_OVERRIDE_DEFAULT(DibGetChannelDescriptor,
    (struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, struct DibTuneChan *pDescriptor),
    (pContext, ChannelHdl, pDescriptor)
)

DIBSTATUS DibRegisterBuffer(struct DibDriverContext *pContext,
                                   FILTER_HDL        FilterIdx,
                                   uint8_t          *BufAdd,
                                   uint32_t          BufSize,
                                   uint8_t           BufId);
DIBSTATUS DibUnregisterBuffer(struct DibDriverContext *pContext,
                                     FILTER_HDL        FilterIdx,
                                     uint8_t          *BufAdd);

DIBSTATUS DibGetDataMonitoring(struct DibDriverContext   *pContext,
                                      ELEM_HDL            ElemHdl,
                               union  DibDataMonit       *pMnt,
                               enum   DibBool             ClearData);

DIBSTATUS DibSetConfig(struct DibDriverContext *pContext,
                         enum DibParameter      Param,
                        union DibParamConfig   *pParamConfig);

DIBSTATUS DibGetConfig(struct DibDriverContext *pContext,
                         enum DibParameter      Param,
                        union DibParamConfig   *pParamConfig);


DIBSTATUS DibRegisterEventCallback(struct DibDriverContext    *pContext,
                                          DibGetEventCallback  pCallback,
                                          void                *pCallCtx,
                                          uint8_t              NbEvents,
                                     enum DibEvent            *pEvent);

DIBSTATUS DibUnregisterEventCallback(struct DibDriverContext    *pContext,
                                            DibGetEventCallback  pCallback);


DIBSTATUS DibSendMessage(struct  DibDriverContext *pContext,
                                 uint16_t          MsgType,
                                 uint16_t          MsgIndex,
                                 uint8_t          *pSendBuf,
                                 uint32_t          SendLen,
                                 uint8_t          *pRecvBuf,
                                 uint32_t         *pRecvLen);

DIBSTATUS DibRegisterMessageCallback(struct  DibDriverContext    *pContext,
                                             uint16_t             MsgType,
                                             uint8_t             *pMsgBuf,
                                             uint32_t             MsgBufLen,
                                             DibMessageCallback   pCallback,
                                             void                *pCallbackCtx);

DIBSTATUS DibGetStreamAttributes(
        struct DibDriverContext *pContext,
        STREAM_HDL                  Stream,
        struct DibStreamAttributes *Attributes
);

DIBSTATUS DibGetFrontendAttributes(
        struct DibDriverContext *pContext,
        FRONTEND_HDL Fe,
        struct DibFrontendAttributes *Attributes
);

CREATE_OVERRIDE(DibGetStream,
        (struct DibDriverContext *pContext, STREAM_HDL Stream, enum DibDemodType Standard, uint32_t OutputOptions, enum DibPowerMode EnPowerSaving),
        (pContext, Stream, Standard, OutputOptions, EnPowerSaving),
{
    dprintf(__o, "gstr %d %d ", streamHdlToNum(pContext, Stream), Standard);
    struct DibStream *s = (struct DibStream *) Stream;

    if (s->Attributes.Type > 0) /* everything except SRAM */
        dprintf(__o, "%d %d ", OutputOptions & 0x1, !!(OutputOptions & 0x2));

    dprintf(__o, "%d\n", EnPowerSaving);
})

CREATE_OVERRIDE(DibAddFrontend,
        (struct DibDriverContext *pContext, STREAM_HDL Stream, FRONTEND_HDL Fe, FRONTEND_HDL FeOutput, uint32_t Force),
        (pContext, Stream, Fe, FeOutput, Force),
{
    dprintf(__o, "gfe %d %d\n", streamHdlToNum(pContext, Stream), feHdlToNum(pContext, Fe));
})

CREATE_OVERRIDE(DibGetChannelEx,
    (struct DibDriverContext *pContext, STREAM_HDL Hdl, struct DibChannel *pDescriptor, CHANNEL_HDL *pChannelHdl, DibGetChannelCallback pCallBack, void *pCallCtx),
    (pContext, Hdl, pDescriptor, pChannelHdl, pCallBack, pCallCtx),
{
    dprintf(__o, "gch %d %d 0 %d 0 0 0\n", pDescriptor->ChannelDescriptor.RFkHz,
        pDescriptor->ChannelDescriptor.Bw / 10, streamHdlToNum(pContext, Hdl));
})

CREATE_OVERRIDE_DEFAULT(DibTuneMonitChannelEx,
    (struct DibDriverContext *pContext, STREAM_HDL Hdl, struct DibTuneMonit *pMonitDescriptor, DibTuneMonitCallback pCallBack, void *pCallCtx),
    (pContext, Hdl, pMonitDescriptor, pCallBack, pCallCtx)
)

CREATE_OVERRIDE_DEFAULT(DibGetFrontendMonit,
    (struct DibDriverContext *pContext, FRONTEND_HDL Fe, union DibDemodMonit *Monit),
    (pContext, Fe, Monit)
)

CREATE_OVERRIDE(DibRemoveFrontend,
        (struct DibDriverContext *pContext, FRONTEND_HDL Frontend),
        (pContext, Frontend),
{
    dprintf(__o, "dfe %d\n", feHdlToNum(pContext, Frontend));
})

CREATE_OVERRIDE(DibDeleteStream,
        (struct DibDriverContext *pContext, STREAM_HDL Stream),
        (pContext, Stream),
{
    dprintf(__o, "dstr %d\n", streamHdlToNum(pContext, Stream));
})


DIBSTATUS DibGetInfo(
        struct DibDriverContext *pContext,
        struct DibInfoRequest   *pInfoRequest,
        struct DibInfoData      *pInfoData
);

DIBSTATUS DibSetInfo(
        struct DibDriverContext *pContext,
        struct DibInfoSet       *pInfoSet,
        uint8_t                 *pStatus
);
