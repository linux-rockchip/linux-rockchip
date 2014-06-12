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
* @file "DibDriverDragonflyStd.h"
* @brief Dragonfly Driver specific functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DRAGONFLY_STD_H
#define DIB_DRIVER_DRAGONFLY_STD_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */

/*************************************************************/
/*** DVB                                                  ****/
/*************************************************************/
#if (DIB_DVB_STD == 1)
   void      DibDriverDragonflyProcessMsgMonitDemodDvb(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo);
   DIBSTATUS DibDriverDragonflyGetChannelInfoDvb(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl);
   DIBSTATUS DibDriverDragonflySetMsgTuneDvb(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg);
   uint8_t   DibDriverDragonflyGetTimeoutDvb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId);
   void DibDriverDragonflyCheckMonitDvb(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl);
#else                                         
   void      DibDriverDragonflyProcessMsgMonitDemodDvb(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo) {;}
   DIBSTATUS DibDriverDragonflyGetChannelInfoDvb(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl) {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflySetMsgTuneDvb(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg) {return DIBSTATUS_ERROR;}
   uint8_t   DibDriverDragonflyGetTimeoutDvb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId) {return 0;}
   void DibDriverDragonflyCheckMonitDvb(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl) {}
#endif

/*************************************************************/
/*** DVB-SH                                               ****/
/*************************************************************/
#if (DIB_DVBSH_STD == 1)
   void      DibDriverDragonflyProcessMsgMonitDemodDvbSh(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo);
   DIBSTATUS DibDriverDragonflyGetChannelInfoDvbSh(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl);
   DIBSTATUS DibDriverDragonflySetMsgTuneDvbSh(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg);
   uint8_t   DibDriverDragonflyGetTimeoutDvbSh(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId);
#else                                         
   void      DibDriverDragonflyProcessMsgMonitDemodDvbSh(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo) {;}
   DIBSTATUS DibDriverDragonflyGetChannelInfoDvbSh(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl)  {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflySetMsgTuneDvbSh(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg) {return DIBSTATUS_ERROR;}
   uint8_t   DibDriverDragonflyGetTimeoutDvbSh(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId) {return 0;}
#endif

/*************************************************************/
/*** DAB                                                  ****/
/*************************************************************/
#if (DIB_DAB_STD == 1)
   void      DibDriverDragonflyProcessMsgMonitDemodDab(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo);
   DIBSTATUS DibDriverDragonflySetMsgTuneDab(struct DibTuneChan * pTc, struct MsgCreateChannel * pMsg);   
   DIBSTATUS DibDriverDragonflyGetChannelInfoDab(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl);
   uint8_t   DibDriverDragonflyGetTimeoutDab(struct DibTotalDemodMonit * pTotalMonitInfo, uint32_t TmpDemId);
   void DibDriverDragonflyCheckMonitDab(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl);
#else
   void      DibDriverDragonflyProcessMsgMonitDemodDab(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo) {;}
   DIBSTATUS DibDriverDragonflySetMsgTuneDab(struct DibTuneChan * pTc, struct MsgCreateChannel * pMsg) {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflyGetChannelInfoDab(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl)  {return DIBSTATUS_ERROR;}
   uint8_t   DibDriverDragonflyGetTimeoutDab(struct DibTotalDemodMonit * pTotalMonitInfo, uint32_t TmpDemId) {return 0;}
   void DibDriverDragonflyCheckMonitDab(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl) {}
#endif

/*************************************************************/
/*** CMMB                                                 ****/
/*************************************************************/
#if (DIB_CMMB_STD == 1)
   void DibDriverDragonflyProcessMsgMonitDemodCmmb(struct DibDriverContext *pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit *pTotalMonitInfo);
   DIBSTATUS DibDriverDragonflySetMsgTuneCmmb(struct DibTuneChan *pTc, struct MsgCreateChannel *pMsg);
   DIBSTATUS DibDriverDragonflyGetChannelInfoCmmb(struct DibDriverContext *pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl);
   uint8_t DibDriverDragonflyGetTimeoutCmmb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId);
#else
   void DibDriverDragonflyProcessMsgMonitDemodCmmb(struct DibDriverContext *pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit *pTotalMonitInfo) {;}
   DIBSTATUS DibDriverDragonflySetMsgTuneCmmb(struct DibTuneChan *pTc, struct MsgCreateChannel *pMsg) {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflyGetChannelInfoCmmb(struct DibDriverContext *pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl) {return DIBSTATUS_ERROR;}
   uint8_t   DibDriverDragonflyGetTimeoutCmmb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId) {return 0;}
#endif /* DIB_CMMB_STD */

/*************************************************************/
/*** ISDBT                                                ****/
/*************************************************************/
#if (DIB_ISDBT_STD == 1)
   void      DibDriverDragonflyProcessMsgMonitDemodIsdbt(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo);
   DIBSTATUS DibDriverDragonflyGetChannelInfoIsdbt(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl);
   DIBSTATUS DibDriverDragonflyGetChannelInfoIsdbtOneSeg(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl);
   DIBSTATUS DibDriverDragonflySetMsgTuneIsdbt(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg);
   DIBSTATUS DibDriverDragonflySetMsgTuneIsdbtOneSeg(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg);
   uint8_t   DibDriverDragonflyGetTimeoutIsdbt(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId);
   void DibDriverDragonflyCheckMonitIsdbt(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl);
#else
   void      DibDriverDragonflyProcessMsgMonitDemodIsdbt(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo) {;}
   DIBSTATUS DibDriverDragonflyGetChannelInfoIsdbt(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl) {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflyGetChannelInfoIsdbtOneSeg(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl) {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflySetMsgTuneIsdbt(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg) {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflySetMsgTuneIsdbtOneSeg(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg) {return DIBSTATUS_ERROR;}
   uint8_t   DibDriverDragonflyGetTimeoutIsdbt(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId) {return 0;}
   void DibDriverDragonflyCheckMonitIsdbt(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl) {}
#endif

/*************************************************************/
/*** CTTB                                                ****/
/*************************************************************/
#if (DIB_CTTB_STD == 1)
   void      DibDriverDragonflyProcessMsgMonitDemodCttb(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo);
   DIBSTATUS DibDriverDragonflyGetChannelInfoCttb(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl);
   DIBSTATUS DibDriverDragonflySetMsgTuneCttb(struct DibTuneChan * pTc, struct MsgCreateChannel * pMsg);
   uint8_t   DibDriverDragonflyGetTimeoutCttb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId);
   void DibDriverDragonflyCheckMonitCttb(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl);
#else
   void      DibDriverDragonflyProcessMsgMonitDemodCttb(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * msgIn, struct DibTotalDemodMonit * pTotalMonitInfo) {;}
   DIBSTATUS DibDriverDragonflyGetChannelInfoCttb(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * msgIn, uint16_t ChHdl) {return DIBSTATUS_ERROR;}
   DIBSTATUS DibDriverDragonflySetMsgTuneCttb(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg) {return DIBSTATUS_ERROR;}
   uint8_t   DibDriverDragonflyGetTimeoutCttb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId) {return 0;}
   void DibDriverDragonflyCheckMonitCttb(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl) {}
#endif

#endif  /* DIB_DRIVER_IF_H */
