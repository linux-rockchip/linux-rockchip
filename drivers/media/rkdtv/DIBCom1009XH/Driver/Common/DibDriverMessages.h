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
* @file "DibDriverMessages.h"
* @brief Generic Message Handlers prototypes.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_MESSAGES_H
#define DIB_DRIVER_MESSAGES_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriver.h"

#define CHAR_BUF_TO_INT32( ptr, pos )   ( (ptr[pos]<<24) + (ptr[pos+1]<<16) + (ptr[pos+2]<<8) + (ptr[pos+3]) )
#define CHAR_BUF_TO_SHORT16( ptr, pos ) ( (ptr[pos]<<8) + (ptr[pos+1]) )

/* Format (must match with DibBridgeCommon.h) */
#define FORMAT_MPE         1
#define FORMAT_SIPSI       2
#define FORMAT_RAWTS       3
#define FORMAT_PES_VIDEO   4
#define FORMAT_PES_AUDIO   5
#define FORMAT_PES_OTHER   6
#define FORMAT_PCR         7
#define FORMAT_FRG         8
#define FORMAT_LAST_FRG    9

/* PID operations */
#define REQ_DEL_PID        0
#define REQ_ADD_PID        1
#define REQ_MOD_PID        2
#define REQ_PID_MAX        3


/**************************** common functions ***************************/
void      IntDriverParseTpsRegisters(struct DibDriverContext *pContext, uint16_t *TpsRegisters, struct DibDvbTuneChan *pstc);
DIBSTATUS DibDriverSendMessage(struct DibDriverContext *pContext, uint32_t *Data, uint32_t Nb);


/**************************** chip specific functions ***************************/

/*----------------------------- Handler to receive messages -----------------------------------*/
#define  DibDriverProcessMessage(pContext, Size, Data)         pContext->ChipOps.ProcessMessage(pContext, Size, Data)

/*--------------------------- Operations for sending messages ---------------------------------*/
#define  DibDriverSetDebugHelp(pContext, i, v, ch)             pContext->ChipOps.SetDebugHelp(pContext, i, v, ch)
#define  DibDriverApbWrite(pContext, adr, Data)                pContext->ChipOps.ApbWrite(pContext, adr, Data)
#define  DibDriverApbRead(pContext, adr)                       pContext->ChipOps.ApbRead(pContext, adr)
#define  DibDriverI2CWrite(pContext, I2cAdr, txbuf, nbtx)      pContext->ChipOps.I2CWrite(pContext, I2cAdr, txbuf, nbtx)
#define  DibDriverI2CRead(pContext, I2cAdr, txbuf, nbtx, rxbuf, nbrx)                              \
                                                               pContext->ChipOps.I2CRead(pContext, I2cAdr, txbuf, nbtx, rxbuf, nbrx)
#define  DibDriverInitDemod(pContext, FeId)                    pContext->ChipOps.InitDemod(pContext, FeId)
#define  DibDriverGetMonit(pContext, TsChannelNb)              pContext->ChipOps.GetMonit(pContext, TsChannelNb)
#define  DibDriverGetEmbeddedInfo(pContext)                    pContext->ChipOps.GetEmbeddedInfo(pContext)
#define  DibDriverGetLayoutInfo(pContext)                      pContext->ChipOps.GetLayoutInfo(pContext)
#define  DibDriverGetStreamInfo(pContext, StreamNum)           pContext->ChipOps.GetStreamInfo(pContext, StreamNum)
#define  DibDriverGetFrontendInfo(pContext, FeNum)             pContext->ChipOps.GetFrontendInfo(pContext, FeNum)
#define  DibDriverSetCfgGpio(pContext, pConfig, FeId)          pContext->ChipOps.SetCfgGpio(pContext, pConfig, FeId)
#define  DibDriverSubbandSelect(pContext, pConfig, FeId)       pContext->ChipOps.SubbandSelect(pContext, pConfig, FeId)
#define  DibDriverSetHbm(pContext, buffering_mode)             pContext->ChipOps.SetHbm(pContext, buffering_mode)
#define  DibDriverEnableCas(pContext, Enable)                  pContext->ChipOps.EnableCas(pContext, Enable)
#define  DibDriverSpecifAbortTuneMonitChannel(pContext)        pContext->ChipOps.AbortTuneMonitChannel(pContext)
#define  DibDriverSpecifTuneMonitChannel(pContext, channel_hdl, MinNbDemod, MaxNbDemod, StreamParameters, pstc)   pContext->ChipOps.TuneMonitChannel(pContext, channel_hdl, MinNbDemod, MaxNbDemod, StreamParameters, pstc)
#define  DibDriverSpecifGetChannel(pContext, MinNbDemod, MaxNbDemod, StreamParameters, pDescriptor, pChannelHdl) \
                                                               pContext->ChipOps.GetChannel(pContext, MinNbDemod, MaxNbDemod, StreamParameters, pDescriptor, pChannelHdl)
#define  DibDriverSpecifDeleteChannel(pContext, ChannelHdl)    pContext->ChipOps.DeleteChannel(pContext, ChannelHdl)
#define  DibDriverSpecifCreateFilter(pContext, ChannelHdl, DataType, pDataBuf, pFilterHdl)         \
                                                               pContext->ChipOps.CreateFilter(pContext, ChannelHdl, DataType, pDataBuf, pFilterHdl)
#define  DibDriverSpecifDeleteFilter(pContext, FilterHdl)      pContext->ChipOps.DeleteFilter(pContext, FilterHdl)
#define  DibDriverSpecifAddItem(pContext, FilterHdl, pFilterDesc, pElemHdl)                        \
                                                               pContext->ChipOps.AddItem(pContext, FilterHdl, pFilterDesc, pElemHdl)
#define  DibDriverSpecifRemoveItem(pContext, pElemHdl)         pContext->ChipOps.RemoveItem(pContext, pElemHdl)
#define  DibDriverSetPowerManagement(pContext, TimeSlicing, Str)    pContext->ChipOps.SetPowerManagement(pContext, TimeSlicing, Str)
#define  DibDriverGetPowerManagement(pContext, Str, EnAddr)    pContext->ChipOps.GetPowerManagement(pContext, Str, EnAddr)
#define  DibDriverSetPidMode(pContext, pPidMode)               pContext->ChipOps.SetPidMode(pContext, pPidMode)
#define  DibDriverDwldSlaveFw(pContext, FeId)                  pContext->ChipOps.DwldSlaveFw(pContext, FeId)
#define  DibDriverClearMonit(pContext, ItemHdl)                pContext->ChipOps.ClearMonit(pContext, ItemHdl)

#define  DibDriverSetCfg(pContext, Cfg)                        pContext->ChipOps.SetCfg(pContext, Cfg)
#define  DibDriverDeInitFirmware(pContext)                     pContext->ChipOps.DeInitFirmware(pContext)

#define  DibDriverSendMsgApi(pContext, MsgType, MsgIndex, pSendBuf, SendLen, pRecvBuf, pRecvLen) \
                                                               pContext->ChipOps.SendMsgApi(pContext, MsgType, MsgIndex, pSendBuf, SendLen, pRecvBuf, pRecvLen)

#define  DibDriverEnableMsgApiUp(pContext, MsgType, En)        pContext->ChipOps.EnableMsgApiUp(pContext, MsgType, En)
#define  DibDriverSpecifGetStream(pContext, pStream, Std, Options, EnPowerSaving) \
                                                               pContext->ChipOps.GetStream(pContext, pStream, Std, Options, EnPowerSaving)
#define  DibDriverSpecifDeleteStream(pContext, pStream)        pContext->ChipOps.DeleteStream(pContext, pStream)
#define  DibDriverSpecifAddFrontend(pContext, pStream, pFe, pOutputFe) \
                                                               pContext->ChipOps.AddFrontend(pContext, pStream, pFe, pOutputFe)
#define  DibDriverSpecifRemoveFrontend(pContext, pFe)          pContext->ChipOps.RemoveFrontend(pContext, pFe)

#if (DIB_INTERNAL_DEBUG == 1)
#define  DibDriverSpecifSubmitBlock(pContext, pBlock)          pContext->ChipOps.SubmitBlock(pContext, pBlock)
#endif
#define  DibDriverSpecifGetChannelEx(pContext, pStream, pDescriptor, pChannelHdl) \
                                                               pContext->ChipOps.GetChannelEx(pContext, pStream, pDescriptor, pChannelHdl)
#define  DibDriverSpecifTuneMonitChannelEx(pContext, pStream, channel_hdl, pstc)  pContext->ChipOps.TuneMonitChannelEx(pContext, pStream, channel_hdl, pstc)

#if ((WRITE_FLASH_SUPPORT == eWRFL_STATIC) || (WRITE_FLASH_SUPPORT == eWRFL_FILE))
#define  DibDriverProgramFlash(pContext, filename)             pContext->ChipOps.ProgramFlash(pContext, filename)
#endif

#endif /*  DIB_DRIVER_MESSAGES_H */
