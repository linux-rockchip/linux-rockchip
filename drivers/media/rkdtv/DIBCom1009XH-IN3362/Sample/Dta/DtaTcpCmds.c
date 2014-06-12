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

#ifdef DTA_TCP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "DibConfig.h"
#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"

#include "DtaCmds.h"
#include "DtaScan.h"
#include "DtaTcp.h"
#define dbgp(args...) fprintf(stderr, args)
#define max(a,b) ((a)>(b)?(a):(b))

uint8_t monitDta(struct DibDtaDebugPlatformInstance *instance,struct DibTotalDemodMonit *mnt) {
  union DibDemodMonit      *m=NULL;
  DIBSTATUS                 rc = DIBSTATUS_SUCCESS;
  DibGetSignalCallback      CallBack = NULL;
  uint8_t                   MaxNbDemod = DIB_UNSET;
  void                     *CallCtx = NULL;
  uint8_t                   ChannelHdl = ChannelArray[0].ChannelHdl;   
  uint8_t DemodNb=4;
  struct DibDriverContext *ctx =instance->parentCtx->DibDrvCtx;
  m = &ChannelArray[0].ChannelMonit.DemodMonit[0];
  memset(m, 0, sizeof(union DibDemodMonit)*DIB_MAX_NB_DEMODS);   
  
rc = DibGetSignalMonitoring(ctx, ChannelHdl, &DemodNb, m, CallBack, CallCtx);
  memcpy(&(mnt->DemodMonit[0]), m, sizeof(union DibDemodMonit)*DemodNb);
  mnt->NbDemods     = DemodNb;
  mnt->ChannelIndex = ChannelHdl;
  mnt->Type = ChannelArray[0].Info.Type;
  return rc;
}

int snr(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar)
{
  uint8_t ret;
  char res[256];
  uint8_t DemId=i->parentCtx->curChipId;    
  struct DibTotalDemodMonit *pMnt;
  struct DibTotalDemodMonit mnt;
  double sig_dB[DIB_MAX_NB_DEMODS], wgn_dB[DIB_MAX_NB_DEMODS];
  dbgp("current demodID = %d\n",i->parentCtx->curChipId);
  if (nbPar<1) { 
    dbgp("Not enough parameters to perform the command\n");
    DibDtaWriteRaw(i, "NOK", 3);
    return -1;
  }
  
  pMnt= &mnt;
  ret=monitDta(i,pMnt);
  switch (pMnt->Type)
  {
    case eSTANDARD_CTTB:
      sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Cttb.Dvb.SigFp) - 320 * log10(2);
      wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Cttb.Dvb.WgnFp) - 320 * log10(2);
      break;
    case eSTANDARD_DVB:
    case eSTANDARD_DVB | eFAST:
    case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
      sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dvb.SigFp) - 320 * log10(2);
      wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Dvb.WgnFp) - 320 * log10(2);
      break;           
    case eSTANDARD_ISDBT:
    case eSTANDARD_ISDBT_1SEG:
    case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
    case eSTANDARD_ISDBT_1SEG | eALWAYS_TUNE_SUCCESS:
      sig_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Isdbt.Dvb.SigFp) - 320 * log10(2);
      wgn_dB[DemId] = 10 * log10(pMnt->DemodMonit[DemId].Isdbt.Dvb.WgnFp) - 320 * log10(2);
      break;
    default:
      dbgp("No monitoring for standard %d\n", pMnt->Type);
      break;
  }
  sprintf(res,"%f", sig_dB[DemId] - wgn_dB[DemId]);
  
  if (ret==DIBSTATUS_SUCCESS)
    DibDtaWriteRaw(i, res, strlen(res));
  else
    DibDtaWriteRaw(i, "NOK", 3);
  return 1;
}

int adc(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar)
{
  uint8_t ret;
  uint8_t DemId=i->parentCtx->curChipId;
  char res[256];

  
  struct DibTotalDemodMonit *pMnt;
  struct DibTotalDemodMonit mnt;
  double tmp_pow[DIB_MAX_NB_DEMODS], ad_pow_dB[DIB_MAX_NB_DEMODS];
  
  if (nbPar<1) { 
    dbgp("Not enough parameters to perform the command\n");
    DibDtaWriteRaw(i, "NOK", 3);
    return -1;
  }  
  
  pMnt= &mnt;
  ret=monitDta(i,pMnt);
  
  switch (pMnt->Type)
  {
    case eSTANDARD_CTTB:
      tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Cttb.Dvb.AgcPower);
      tmp_pow[DemId]  /= (double)(1 << 20);
      ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
      break;
    case eSTANDARD_DVB:
    case eSTANDARD_DVB | eFAST:
    case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
      tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Dvb.AgcPower);
      tmp_pow[DemId]  /= (double)(1 << 20);
      ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
      break;           
    case eSTANDARD_ISDBT:
    case eSTANDARD_ISDBT_1SEG:
    case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
    case eSTANDARD_ISDBT_1SEG | eALWAYS_TUNE_SUCCESS:
      tmp_pow[DemId]   = (double)(pMnt->DemodMonit[DemId].Isdbt.Dvb.AgcPower);
      tmp_pow[DemId]  /= (double)(1 << 20);
      ad_pow_dB[DemId] = 10 * log10(tmp_pow[DemId]);
      break;
    default:
      dbgp("No monitoring for standard %d\n", pMnt->Type);
      break;
  }
  sprintf(res,"%f", ad_pow_dB[DemId]);
  
  if (ret==DIBSTATUS_SUCCESS)
    DibDtaWriteRaw(i, res, strlen(res));
  else
    DibDtaWriteRaw(i, "NOK", 3);
  return 1;
}

int berPer(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar)
{
  uint8_t ret;
  char res[256];
  uint8_t DemId=i->parentCtx->curChipId;    
  struct DibTotalDemodMonit *pMnt;
  struct DibTotalDemodMonit mnt;
  double berPer;
  uint32_t per;
  
  if (nbPar<1) { 
    dbgp("Not enough parameters to perform the command\n");
    DibDtaWriteRaw(i, "NOK", 3);
    return -1;
  }  
  
  pMnt= &mnt;
  ret=monitDta(i,pMnt);
  
  switch (pMnt->Type)    {
    case eSTANDARD_CTTB:
      per = pMnt->DemodMonit[0].Cttb.Per;
      if((per&0xffff) == 0)
        berPer = 0;
      else
        berPer = (float)((per&0xffff0000) >> 16)/(float)(per&0xffff);
      
      break;
    case eSTANDARD_DVB:
    case eSTANDARD_DVB | eFAST:
    case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
      berPer=((double)pMnt->DemodMonit[DemId].Dvb.FecBerRs) / ((double)100000000);
      break;           
    case eSTANDARD_ISDBT:
    case eSTANDARD_ISDBT_1SEG:
    case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
    case eSTANDARD_ISDBT_1SEG | eALWAYS_TUNE_SUCCESS:
        berPer=((double)pMnt->DemodMonit[DemId].Isdbt.Layer[0].FecBerRs) / ((double)100000000);
      break;
    default:
      dbgp("No monitoring for standard %d\n", pMnt->Type);
      break;
  }
  sprintf(res,"%f", berPer*1.0);
  
  if (ret==DIBSTATUS_SUCCESS)
    DibDtaWriteRaw(i, res, strlen(res));
  else
    DibDtaWriteRaw(i, "NOK", 3);
  return 1;
}

int streamId(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar) {
  uint8_t ret;
  
  if (nbPar<1) { 
    dbgp("Not enough parameters to perform the command\n");
    DibDtaWriteRaw(i, "NOK", 3);
    return -1;
  } 
  i->parentCtx->curStream=atoi(Par[1]);
  dbgp("The Stream monitored is %d\n",i->parentCtx->curStream);
  if (ret==DIBSTATUS_SUCCESS)
    DibDtaWriteRaw(i, "OK",2);
  else
    DibDtaWriteRaw(i, "NOK", 3);
  return 1;
}

int chipId(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar) {
  uint8_t ret;

  if (nbPar<1) { 
    dbgp("Not enough parameters to perform the command\n");
    DibDtaWriteRaw(i, "NOK", 3);
    return -1;
  } 
  i->parentCtx->curChipId=atoi(Par[1]);
  dbgp("The Chip monitored is %d\n",i->parentCtx->curChipId);
  if (ret==DIBSTATUS_SUCCESS)
    DibDtaWriteRaw(i, "OK",2);
  else
    DibDtaWriteRaw(i, "NOK", 3);
  return 1;
}

int mpeg(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar)
{
  uint8_t ret;
  char res[256];
  uint8_t DemId=i->parentCtx->curChipId;    
  struct DibTotalDemodMonit *pMnt;
  struct DibTotalDemodMonit mnt;
  uint8_t mpegVitLock;;
  if (nbPar<1) { 
    dbgp("Not enough parameters to perform the command\n");
    DibDtaWriteRaw(i, "NOK", 3);
    return -1;
  }  
  
  pMnt= &mnt;
  ret =monitDta(i,pMnt);
  
  switch (pMnt->Type)    {
    case eSTANDARD_DVB:
    case eSTANDARD_DVB | eFAST:
    case eSTANDARD_DVB | eALWAYS_TUNE_SUCCESS:
      mpegVitLock=DIB_GET_FEC_LOCK_MPEG(pMnt->DemodMonit[DemId].Dvb.Locks);
      break;           
    case eSTANDARD_ISDBT:
    case eSTANDARD_ISDBT_1SEG:
    case eSTANDARD_ISDBT | eALWAYS_TUNE_SUCCESS:
    case eSTANDARD_ISDBT_1SEG | eALWAYS_TUNE_SUCCESS:
      mpegVitLock=DIB_GET_ISDBT_LOCK_VIT0(pMnt->DemodMonit[DemId].Isdbt.Dvb.Locks);
      break;
    default:
      printf("No monitoring for standard %d\n", pMnt->Type);
      break;
  }
  sprintf(res,"%f", mpegVitLock*1.0);
  
  if (ret==DIBSTATUS_SUCCESS)
    DibDtaWriteRaw(i, res, strlen(res));
  else
    DibDtaWriteRaw(i, "NOK", 3);
  return 1;
}

int getChannel(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar)
{
  uint32_t Rf;
  uint8_t Bw,Std;
  uint8_t ret;
  if (nbPar<3) { 
    dbgp("Not enough parameters to perform the command\n");
    DibDtaWriteRaw(i, "NOK", 3);
    return -1;
  }
    
  Rf = atoi (Par[1]);
  Bw = atoi (Par[2]);
  Std = atoi(Par[3]);
  ret=DtaGetChannel(0, DIB_DEMOD_AUTO, 0, 0, (enum DibDemodType)Std, Rf, Bw*10, 0, SCAN_FLAG_AUTOSEARCH);
  if (ret==DIBSTATUS_SUCCESS)
    DibDtaWriteRaw(i, "OK", 2);
  else
    DibDtaWriteRaw(i, "NOK", 3);
  return 1;
}

int getFrontend(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar)
{
    uint8_t StreamNum,Std,FeNum;
    uint8_t ret;
    if (nbPar<3) { 
        dbgp("Not enough parameters to perform the command\n");
        DibDtaWriteRaw(i, "NOK", 3);
        return -1;
    }
    StreamNum = atoi (Par[1]);
    Std = atoi (Par[2]);
    FeNum = atoi (Par[3]);
    DtaDeleteFrontend(FeNum);
    DtaDeleteStream(StreamNum);
    DtaGetStream(StreamNum, Std, 0, 0);
    DtaGetFrontend(StreamNum, FeNum, -1 /*FeOutputNum*/, 0/*Force*/);
    if (ret==DIBSTATUS_SUCCESS)
        DibDtaWriteRaw(i, "OK", 2);
    else
        DibDtaWriteRaw(i, "NOK", 3);
    return 1;
}

int tuneFrontend(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar)
{
    uint8_t StreamNum;
    uint32_t Rf;
    uint8_t Bw;
    uint8_t ret;
    struct DibTuneChan TuneChan;
    uint32_t Type;
    
    
    if (nbPar<3) { 
        dbgp("Not enough parameters to perform the command\n");
        DibDtaWriteRaw(i, "NOK", 3);
        return -1;
    }
    Rf = atoi (Par[1]);
    Bw = atoi (Par[2]);
    StreamNum = atoi (Par[3]);   
    
    Type = StreamArray[StreamNum].Std;
    
    TuneChan.Bw      = Bw*10;
    TuneChan.RFkHz   = Rf;
    TuneChan.InvSpec = -1;
    memset(&TuneChan.Std, -1, sizeof(TuneChan.Std));
 
    DtaDeleteChannel(0); 
    ret=DtaGetChannelEx(0, StreamNum, &TuneChan, 0, SCAN_FLAG_AUTOSEARCH);
    if (ret==DIBSTATUS_SUCCESS)
        DibDtaWriteRaw(i, "OK", 2);
    else
        DibDtaWriteRaw(i, "NOK", 3);
    return 1;
}
#endif
