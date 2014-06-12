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



#include "DibStruct.h"

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
void BasicLayoutInfoPack(struct BasicLayoutInfo *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 4, s->chipId);
   SerialBufWriteField(ctx, 8, s->chipModel);
   SerialBufWriteField(ctx, 8, s->hostInterface);
   SerialBufWriteField(ctx, 4, s->notReset);
   SerialBufWriteField(ctx, 8, s->package);
   SerialBufWriteField(ctx, 3, s->flashchip);
   SerialBufWriteField(ctx, 3, s->scchip);
}


void GpioFunctionPack(struct GpioFunction *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 32, s->FuncName);
   SerialBufWriteField(ctx, 32, s->Mask);
   SerialBufWriteField(ctx, 32, s->Direction);
   SerialBufWriteField(ctx, 32, s->Value);
}


void InputListPack(struct InputList *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->FeId);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 16, s->FeStandards);
}

void StreamInfoPack(struct StreamInfo *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->ChipId);
   SerialBufWriteField(ctx, 8, s->StreamType);
   SerialBufWriteField(ctx, 8, s->NumOfInputs);
   SerialBufWriteField(ctx, 2, s->PowerMode);
   SerialBufWriteField(ctx, 16, s->StreamId);
   SerialBufWriteField(ctx, 30, s->Options);
   SerialBufWriteField(ctx, 32, s->SupportedStandards);
   for(i = 0; i < 24;i++ )
   {
      InputListPack(&s->DirectInputs[i], ctx);
   }
}

void FrontendInfoPack(struct FrontendInfo *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->ChipId);
   SerialBufWriteField(ctx, 8, s->NumOfInputs);
   SerialBufWriteField(ctx, 16, s->FeId);
   SerialBufWriteField(ctx, 32, s->SupportedStandards);
   for(i = 0; i < 24;i++ )
   {
      InputListPack(&s->DirectInputs[i], ctx);
   }
}

void LayoutInfoPack(struct LayoutInfo *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->MaxNbChips);
   SerialBufWriteField(ctx, 8, s->MaxNbFrontends);
   SerialBufWriteField(ctx, 8, s->MaxNbStreams);
   for(i = 0; i < 10;i++ )
   {
      StreamInfoPack(&s->StreamInfo[i], ctx);
   }
   for(i = 0; i < 4;i++ )
   {
      FrontendInfoPack(&s->FeInfo[i], ctx);
   }
}

void CMMBmfMappingPack(struct CMMBmfMapping *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->MsfNb);
   SerialBufWriteField(ctx, 8, s->MfId);
   for(i = 0; i < 15;i++ )
   {
      SerialBufWriteField(ctx, 16, s->ServId[i]);
   }
}

void CMMBmappingPack(struct CMMBmapping *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->LastMfNum);
   SerialBufWriteField(ctx, 8, s->NbMf);
   SerialBufWriteField(ctx, 8, s->End);
   for(i = 0; i < 6;i++ )
   {
      CMMBmfMappingPack(&s->Mapping[i], ctx);
   }
}


void DVBChannelParametersPack(struct DVBChannelParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 8, s->Fft);
   SerialBufWriteField(ctx, 8, s->Guard);
   SerialBufWriteField(ctx, 8, s->Constellation);
   SerialBufWriteField(ctx, 8, s->NativeIntlv);
   SerialBufWriteField(ctx, 8, s->InvSpect);
}

void DVBTChannelParametersPack(struct DVBTChannelParameters *s, struct SerialBuf *ctx)
{
   DVBChannelParametersPack(&s->Dvb, ctx);
   SerialBufWriteField(ctx, 8, s->ViterbiSelectHp);
   SerialBufWriteField(ctx, 8, s->ViterbiCodeRateHp);
   SerialBufWriteField(ctx, 8, s->ViterbiCodeRateLp);
   SerialBufWriteField(ctx, 8, s->ViterbiAlpha);
   SerialBufWriteField(ctx, 8, s->ViterbiHierach);
}

void DVBTChannelDecoderInfoPack(struct DVBTChannelDecoderInfo *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Quality);
   SerialBufWriteField(ctx, 16, s->FastQuality);
   SerialBufWriteField(ctx, 32, s->Ber);
   SerialBufWriteField(ctx, 32, s->Per);
}



void DVBSHChannelParametersPack(struct DVBSHChannelParameters *s, struct SerialBuf *ctx)
{
   DVBChannelParametersPack(&s->Dvb, ctx);
   SerialBufWriteField(ctx, 2, s->SelectHp);
   SerialBufWriteField(ctx, 8, s->CodeRateHp);
   SerialBufWriteField(ctx, 8, s->CodeRateLp);
   SerialBufWriteField(ctx, 8, s->Alpha);
   SerialBufWriteField(ctx, 6, s->Hierach);
   SerialBufWriteField(ctx, 6, s->TurboLateTaps);
   SerialBufWriteField(ctx, 6, s->TurboNonLateIncr);
   SerialBufWriteField(ctx, 6, s->TurboCommonMult);
   SerialBufWriteField(ctx, 8, s->TurboNbSlices);
   SerialBufWriteField(ctx, 8, s->TurboSliceDist);
}

void DVBSHChannelDecoderInfoPack(struct DVBSHChannelDecoderInfo *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Quality);
   SerialBufWriteField(ctx, 16, s->FastQuality);
   SerialBufWriteField(ctx, 32, s->Per);
}


void ISDBTLayerParameterPack(struct ISDBTLayerParameter *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 5, s->Constellation);
   SerialBufWriteField(ctx, 5, s->CodeRate);
   SerialBufWriteField(ctx, 5, s->NbSegments);
   SerialBufWriteField(ctx, 4, s->TimeIntlv);
}

void ISDBTChannelParametersPack(struct ISDBTChannelParameters *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->Fft);
   SerialBufWriteField(ctx, 8, s->Guard);
   SerialBufWriteField(ctx, 2, s->SbMode);
   SerialBufWriteField(ctx, 2, s->PartialReception);
   SerialBufWriteField(ctx, 2, s->InvSpect);
   SerialBufWriteField(ctx, 5, s->SbConTotSeg);
   SerialBufWriteField(ctx, 5, s->SbWantedSeg);
   SerialBufWriteField(ctx, 7, s->SbSubChannel);
   for(i = 0; i < 3;i++ )
   {
      ISDBTLayerParameterPack(&s->Layer[i], ctx);
   }
}


void DABSubChannelPack(struct DABSubChannel *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 6, s->Id);
   SerialBufWriteField(ctx, 10, s->StartAddress);
   SerialBufWriteField(ctx, 10, s->SubSize);
   SerialBufWriteField(ctx, 1, s->Form);
   SerialBufWriteField(ctx, 6, s->TableIndex);
   SerialBufWriteField(ctx, 1, s->Option);
   SerialBufWriteField(ctx, 2, s->ProtectionLevel);
   SerialBufWriteField(ctx, 1, s->IsTs);
   SerialBufWriteField(ctx, 0, s->SubChNew);
   SerialBufWriteField(ctx, 0, s->AddrRAM);
   SerialBufWriteField(ctx, 0, s->RequestedFec);
}

void DABSubChannelCanalInfoPack(struct DABSubChannelCanalInfo *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->StartAddress);
   SerialBufWriteField(ctx, 16, s->SubSize);
   SerialBufWriteField(ctx, 16, s->PI1);
   SerialBufWriteField(ctx, 16, s->PI2);
   SerialBufWriteField(ctx, 16, s->I);
   SerialBufWriteField(ctx, 16, s->L1Punct);
   SerialBufWriteField(ctx, 16, s->L2Punct);
   SerialBufWriteField(ctx, 8, s->Id);
   SerialBufWriteField(ctx, 8, s->Form);
   SerialBufWriteField(ctx, 8, s->Index);
}

void DABChannelParametersPack(struct DABChannelParameters *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->TMode);
   SerialBufWriteField(ctx, 8, s->NbActiveSubCh);
   SerialBufWriteField(ctx, 0, s->Reconfig);
   SerialBufWriteField(ctx, 0, s->CifOccurrenceChange);
   for(i = 0; i < 64;i++ )
   {
      DABSubChannelPack(&s->SubChannel[i], ctx);
   }
}

void DABChannelShortParametersPack(struct DABChannelShortParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 8, s->TMode);
   SerialBufWriteField(ctx, 8, s->NbActiveSubCh);
}

void DABLocksPack(struct DABLocks *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 1, s->TdIntFullInternal);
   SerialBufWriteField(ctx, 5, 0);
   SerialBufWriteField(ctx, 1, s->Mpeg);
   SerialBufWriteField(ctx, 1, s->Frm);
   SerialBufWriteField(ctx, 1, s->Dmb);
   SerialBufWriteField(ctx, 2, 0);
   SerialBufWriteField(ctx, 1, s->NDecFrm);
   SerialBufWriteField(ctx, 1, s->NDecTMode);
   SerialBufWriteField(ctx, 1, s->CoffDab);
   SerialBufWriteField(ctx, 1, s->CoffDmb);
   SerialBufWriteField(ctx, 1, 0);
}

void DABChannelDecoderInfoPack(struct DABChannelDecoderInfo *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Quality);
   SerialBufWriteField(ctx, 16, s->FastQuality);
   SerialBufWriteField(ctx, 32, s->Ber);
   SerialBufWriteField(ctx, 32, s->Per);
   SerialBufWriteField(ctx, 32, s->TdmbBer);
}


void CMMBTimeSlotPack(struct CMMBTimeSlot *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 3, s->RsCodeRate);
   SerialBufWriteField(ctx, 3, s->IntlvMode);
   SerialBufWriteField(ctx, 4, s->LdpcCodeRate);
   SerialBufWriteField(ctx, 5, s->Constellation);
   SerialBufWriteField(ctx, 4, s->Scrambling);
   SerialBufWriteField(ctx, 8, s->MfId);
}

void CMMBChannelParametersPack(struct CMMBChannelParameters *s, struct SerialBuf *ctx)
{
   int i;
   SerialBufWriteField(ctx, 8, s->Fft);
   for(i = 0; i < 40;i++ )
   {
      CMMBTimeSlotPack(&s->TsArray[i], ctx);
   }
   SerialBufWriteField(ctx, 16, s->configFrame);
}


void CTTBChannelParametersPack(struct CTTBChannelParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 8, s->Fft);
   SerialBufWriteField(ctx, 8, s->Constellation);
   SerialBufWriteField(ctx, 8, s->Guard);
   SerialBufWriteField(ctx, 8, s->LdpcCodeRate);
   SerialBufWriteField(ctx, 8, s->Intlv);
   SerialBufWriteField(ctx, 8, s->ConstantPN);
}

void CTTBChannelDecoderInfoPack(struct CTTBChannelDecoderInfo *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Quality);
   SerialBufWriteField(ctx, 16, s->FastQuality);
   SerialBufWriteField(ctx, 32, s->Per);
   SerialBufWriteField(ctx, 32, s->LdpcSyndrome);
   SerialBufWriteField(ctx, 32, s->LdpcSyndromeFirstIter);
   SerialBufWriteField(ctx, 8, s->LdpcNbIter);
   SerialBufWriteField(ctx, 8, s->LdpcFlags);
}


void ChannelParametersPack(union ChannelParameters *s, int id, struct SerialBuf *ctx)
{
   if(0) {}
   else if(id == eSTANDARD_DVB)
   {
      DVBTChannelParametersPack(&s->Dvbt, ctx);
   }
   else if(id == eSTANDARD_ISDBT)
   {
      ISDBTChannelParametersPack(&s->Isdbt, ctx);
   }
   else if(id == eSTANDARD_DAB)
   {
      DABChannelParametersPack(&s->Dab, ctx);
   }
   else if(id == eSTANDARD_DVBSH)
   {
      DVBSHChannelParametersPack(&s->Dvbsh, ctx);
   }
   else if(id == eSTANDARD_CMMB)
   {
      CMMBChannelParametersPack(&s->Cmmb, ctx);
   }
   else if(id == eSTANDARD_CTTB)
   {
      CTTBChannelParametersPack(&s->Cttb, ctx);
   }
}


void TuneChannelParametersPack(union TuneChannelParameters *s, int id, struct SerialBuf *ctx)
{
   if(0) {}
   else if(id == eSTANDARD_DVB)
   {
      DVBTChannelParametersPack(&s->Dvbt, ctx);
   }
   else if(id == eSTANDARD_ISDBT)
   {
      ISDBTChannelParametersPack(&s->Isdbt, ctx);
   }
   else if(id == eSTANDARD_DAB)
   {
      DABChannelShortParametersPack(&s->Dab, ctx);
   }
   else if(id == eSTANDARD_DVBSH)
   {
      DVBSHChannelParametersPack(&s->Dvbsh, ctx);
   }
   else if(id == eSTANDARD_CMMB)
   {
      CMMBChannelParametersPack(&s->Cmmb, ctx);
   }
   else if(id == eSTANDARD_CTTB)
   {
      CTTBChannelParametersPack(&s->Cttb, ctx);
   }
}

void ChannelDecoderInfoPack(union ChannelDecoderInfo *s, int id, struct SerialBuf *ctx)
{
   if(0) {}
   else if(id == eSTANDARD_DVB)
   {
      DVBTChannelDecoderInfoPack(&s->Dvbt, ctx);
   }
   else if(id == eSTANDARD_ISDBT)
   {
      int i;
      for(i = 0; i < 3;i++ )
   {
      ISDBTChannelDecoderInfoPack(&s->Isdbt[i], ctx);
   }
   }
   else if(id == eSTANDARD_DAB)
   {
      DABChannelDecoderInfoPack(&s->Dab, ctx);
   }
   else if(id == eSTANDARD_DVBSH)
   {
      DVBSHChannelDecoderInfoPack(&s->Dvbsh, ctx);
   }
   else if(id == eSTANDARD_CMMB)
   {
      CMMBChannelDecoderInfoPack(&s->Cmmb, ctx);
   }
   else if(id == eSTANDARD_CTTB)
   {
      CTTBChannelDecoderInfoPack(&s->Cttb, ctx);
   }
}


void SignalMonitoringPack(struct SignalMonitoring *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 8, s->Standard);
   SerialBufWriteField(ctx, 7, s->MonitStatus);
   SerialBufWriteField(ctx, 1, s->LastDemod);
   SerialBufWriteField(ctx, 32, s->AdcPower);
   SerialBufWriteField(ctx, 16, s->AgcGlobal);
   SerialBufWriteField(ctx, 16, s->AgcRf);
   SerialBufWriteField(ctx, 16, s->AgcBb);
   SerialBufWriteField(ctx, 16, s->AgcWbd);
   SerialBufWriteField(ctx, 16, s->AgcSplitOffset);
   SerialBufWriteField(ctx, 16, s->Locks);
   SerialBufWriteField(ctx, 32, s->EqualNoise);
   SerialBufWriteField(ctx, 32, s->EqualSignal);
   SerialBufWriteField(ctx, 32, s->CombNoise);
   SerialBufWriteField(ctx, 32, s->CombSignal);
   SerialBufWriteField(ctx, 31, s->TimingOffset);
   SerialBufWriteField(ctx, 31, s->FrequencyOffset);
   SerialBufWriteField(ctx, 32, s->TransmitterId);
   ChannelDecoderInfoPack(&s->ChanDec, s->Standard, ctx);
}

void ChannelDescPack(struct ChannelDesc *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 32, s->Frequency);
   SerialBufWriteField(ctx, 16, s->Bandwidth);
   SerialBufWriteField(ctx, 5, s->Standard);

   ChannelParametersPack(&s->Param, s->Standard, ctx);
}

void TuneChannelDescPack(struct TuneChannelDesc *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 32, s->Frequency);
   SerialBufWriteField(ctx, 16, s->Bandwidth);
   SerialBufWriteField(ctx, 5, s->Standard);

   TuneChannelParametersPack(&s->Param, s->Standard, ctx);
}


void TSItemParametersPack(struct TSItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Pid);
}

void SIPSIItemParametersPack(struct SIPSIItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Pid);
   SerialBufWriteField(ctx, 1, s->PidWatch);
   SerialBufWriteField(ctx, 1, s->PidCrc);
}

void MPEFECItemParametersPack(struct MPEFECItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Pid);
   SerialBufWriteField(ctx, 1, s->Prefetch);
   SerialBufWriteField(ctx, 15, s->NbRows);
   SerialBufWriteField(ctx, 16, s->MaxBurstDuration);
}

void FIGItemParametersPack(struct FIGItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 32, s->RFU);
}

void TDMBItemParametersPack(struct TDMBItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Pid);
   SerialBufWriteField(ctx, 16, s->SubCh);
}

void DABItemParametersPack(struct DABItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->SubCh);
   SerialBufWriteField(ctx, 16, s->Type);
   SerialBufWriteField(ctx, 1, s->Fec);
}

void DABPacketItemParametersPack(struct DABPacketItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->SubCh);
   SerialBufWriteField(ctx, 16, s->Address);
   SerialBufWriteField(ctx, 1, s->Fec);
   SerialBufWriteField(ctx, 1, s->DataGroup);
}





void MPEIFECItemParametersPack(struct MPEIFECItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Pid);
   SerialBufWriteField(ctx, 1, s->Prefetch);
   SerialBufWriteField(ctx, 12, s->NbRows);
   SerialBufWriteField(ctx, 16, s->MaxBurstDuration);
   SerialBufWriteField(ctx, 8, s->D);
   SerialBufWriteField(ctx, 6, s->B);
   SerialBufWriteField(ctx, 8, s->C);
   SerialBufWriteField(ctx, 7, s->R);
   SerialBufWriteField(ctx, 6, s->S);
}



void CMMBSVCItemParametersPack(struct CMMBSVCItemParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 16, s->Mode);
   SerialBufWriteField(ctx, 16, s->MfId);
   SerialBufWriteField(ctx, 16, s->SvcId);
}



void ItemParametersPack(union ItemParameters *s, int id, struct SerialBuf *ctx)
{
   if(0) {}
   else if(id == eTS)
   {
      TSItemParametersPack(&s->Ts, ctx);
   }
   else if(id == eSIPSI)
   {
      SIPSIItemParametersPack(&s->Sipsi, ctx);
   }
   else if(id == eMPEFEC)
   {
      MPEFECItemParametersPack(&s->Mpefec, ctx);
   }
   else if(id == eFIG)
   {
      FIGItemParametersPack(&s->Fig, ctx);
   }
   else if(id == eTDMB)
   {
      TDMBItemParametersPack(&s->Tdmb, ctx);
   }
   else if(id == eDAB)
   {
      DABItemParametersPack(&s->Dab, ctx);
   }
   else if(id == eDABPACKET)
   {
      DABPacketItemParametersPack(&s->DabPacket, ctx);
   }
   else if(id == ePESVIDEO)
   {
      PESVItemParametersPack(&s->PesV, ctx);
   }
   else if(id == ePESAUDIO)
   {
      PESAItemParametersPack(&s->PesA, ctx);
   }
   else if(id == ePESOTHER)
   {
      PESOItemParametersPack(&s->PesO, ctx);
   }
   else if(id == ePCR)
   {
      PCRItemParametersPack(&s->Pcr, ctx);
   }
   else if(id == eMPEIFEC)
   {
      MPEIFECItemParametersPack(&s->Mpeifec, ctx);
   }
   else if(id == eCMMBMFS)
   {
      CMMBMFSItemParametersPack(&s->CmmbMfs, ctx);
   }
   else if(id == eCMMBCIT)
   {
      CMMBCITItemParametersPack(&s->CmmbCit, ctx);
   }
   else if(id == eCMMBSVC)
   {
      CMMBSVCItemParametersPack(&s->CmmbSvc, ctx);
   }
   else if(id == eCMMBXPE)
   {
      CMMBXPEItemParametersPack(&s->CmmbXpe, ctx);
   }
   else if(id == eATSCMHIP)
   {
      ATSCMHIPItemParametersPack(&s->AtscmhEns, ctx);
   }
}


void EFUSEInfoParametersPack(struct EFUSEInfoParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 1, s->IsRead);
}

void SDRAMInfoParametersPack(struct SDRAMInfoParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 8, s->State);
   SerialBufWriteField(ctx, 1, s->IsUsedForCaching);
}

void GPIOInfoParametersPack(struct GPIOInfoParameters *s, struct SerialBuf *ctx)
{
   SerialBufWriteField(ctx, 4, s->Num);
   SerialBufWriteField(ctx, 8, s->Val);
   SerialBufWriteField(ctx, 8, s->Dir);
}

void InfoParametersPack(union InfoParameters *s, int id, struct SerialBuf *ctx)
{
   if(0) {}
   else if(id == eINFO_EFUSE)
   {
      EFUSEInfoParametersPack(&s->Efuse, ctx);
   }
   else if(id == eINFO_SDRAM)
   {
      SDRAMInfoParametersPack(&s->Sdram, ctx);
   }
   else if(id == eINFO_GPIO)
   {
      GPIOInfoParametersPack(&s->Gpio, ctx);
   }
}





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
void BasicLayoutInfoUnpack(struct SerialBuf *ctx, struct BasicLayoutInfo *s)
{
   s->chipId                      = SerialBufReadField(ctx, 4, 0);
   s->chipModel                   = SerialBufReadField(ctx, 8, 0);
   s->hostInterface               = SerialBufReadField(ctx, 8, 0);
   s->notReset                    = SerialBufReadField(ctx, 4, 0);
   s->package                     = SerialBufReadField(ctx, 8, 0);
   s->flashchip                   = SerialBufReadField(ctx, 3, 0);
   s->scchip                      = SerialBufReadField(ctx, 3, 0);
}


void GpioFunctionUnpack(struct SerialBuf *ctx, struct GpioFunction *s)
{
   s->FuncName                    = SerialBufReadField(ctx, 32, 0);
   s->Mask                        = SerialBufReadField(ctx, 32, 0);
   s->Direction                   = SerialBufReadField(ctx, 32, 0);
   s->Value                       = SerialBufReadField(ctx, 32, 0);
}


void InputListUnpack(struct SerialBuf *ctx, struct InputList *s)
{
   s->FeId                        = SerialBufReadField(ctx, 16, 0);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->FeStandards                 = SerialBufReadField(ctx, 16, 0);
}

void StreamInfoUnpack(struct SerialBuf *ctx, struct StreamInfo *s)
{
   int i;
   s->ChipId                      = SerialBufReadField(ctx, 8, 0);
   s->StreamType                  = SerialBufReadField(ctx, 8, 0);
   s->NumOfInputs                 = SerialBufReadField(ctx, 8, 0);
   s->PowerMode                   = SerialBufReadField(ctx, 2, 0);
   s->StreamId                    = SerialBufReadField(ctx, 16, 0);
   s->Options                     = SerialBufReadField(ctx, 30, 0);
   s->SupportedStandards          = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 24;i++ )
   {
      InputListUnpack(ctx, &s->DirectInputs[i]);
   }
}

void FrontendInfoUnpack(struct SerialBuf *ctx, struct FrontendInfo *s)
{
   int i;
   s->ChipId                      = SerialBufReadField(ctx, 8, 0);
   s->NumOfInputs                 = SerialBufReadField(ctx, 8, 0);
   s->FeId                        = SerialBufReadField(ctx, 16, 0);
   s->SupportedStandards          = SerialBufReadField(ctx, 32, 0);
   for(i = 0; i < 24;i++ )
   {
      InputListUnpack(ctx, &s->DirectInputs[i]);
   }
}

void LayoutInfoUnpack(struct SerialBuf *ctx, struct LayoutInfo *s)
{
   int i;
   s->MaxNbChips                  = SerialBufReadField(ctx, 8, 0);
   s->MaxNbFrontends              = SerialBufReadField(ctx, 8, 0);
   s->MaxNbStreams                = SerialBufReadField(ctx, 8, 0);
   for(i = 0; i < 10;i++ )
   {
      StreamInfoUnpack(ctx, &s->StreamInfo[i]);
   }
   for(i = 0; i < 4;i++ )
   {
      FrontendInfoUnpack(ctx, &s->FeInfo[i]);
   }
}

void CMMBmfMappingUnpack(struct SerialBuf *ctx, struct CMMBmfMapping *s)
{
   int i;
   s->MsfNb                       = SerialBufReadField(ctx, 8, 0);
   s->MfId                        = SerialBufReadField(ctx, 8, 0);
   for(i = 0; i < 15;i++ )
   {
      s->ServId[i]                   = SerialBufReadField(ctx, 16, 0);
   }
}

void CMMBmappingUnpack(struct SerialBuf *ctx, struct CMMBmapping *s)
{
   int i;
   s->LastMfNum                   = SerialBufReadField(ctx, 8, 0);
   s->NbMf                        = SerialBufReadField(ctx, 8, 0);
   s->End                         = SerialBufReadField(ctx, 8, 0);
   for(i = 0; i < 6;i++ )
   {
      CMMBmfMappingUnpack(ctx, &s->Mapping[i]);
   }
}


void DVBChannelParametersUnpack(struct SerialBuf *ctx, struct DVBChannelParameters *s)
{
   s->Fft                         = SerialBufReadField(ctx, 8, 1);
   s->Guard                       = SerialBufReadField(ctx, 8, 1);
   s->Constellation               = SerialBufReadField(ctx, 8, 1);
   s->NativeIntlv                 = SerialBufReadField(ctx, 8, 1);
   s->InvSpect                    = SerialBufReadField(ctx, 8, 1);
}

void DVBTChannelParametersUnpack(struct SerialBuf *ctx, struct DVBTChannelParameters *s)
{
   DVBChannelParametersUnpack(ctx, &s->Dvb);
   s->ViterbiSelectHp             = SerialBufReadField(ctx, 8, 1);
   s->ViterbiCodeRateHp           = SerialBufReadField(ctx, 8, 1);
   s->ViterbiCodeRateLp           = SerialBufReadField(ctx, 8, 1);
   s->ViterbiAlpha                = SerialBufReadField(ctx, 8, 1);
   s->ViterbiHierach              = SerialBufReadField(ctx, 8, 1);
}

void DVBTChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct DVBTChannelDecoderInfo *s)
{
   s->Quality                     = SerialBufReadField(ctx, 16, 0);
   s->FastQuality                 = SerialBufReadField(ctx, 16, 0);
   s->Ber                         = SerialBufReadField(ctx, 32, 0);
   s->Per                         = SerialBufReadField(ctx, 32, 0);
}



void DVBSHChannelParametersUnpack(struct SerialBuf *ctx, struct DVBSHChannelParameters *s)
{
   DVBChannelParametersUnpack(ctx, &s->Dvb);
   s->SelectHp                    = SerialBufReadField(ctx, 2, 1);
   s->CodeRateHp                  = SerialBufReadField(ctx, 8, 1);
   s->CodeRateLp                  = SerialBufReadField(ctx, 8, 1);
   s->Alpha                       = SerialBufReadField(ctx, 8, 1);
   s->Hierach                     = SerialBufReadField(ctx, 6, 1);
   s->TurboLateTaps               = SerialBufReadField(ctx, 6, 0);
   s->TurboNonLateIncr            = SerialBufReadField(ctx, 6, 0);
   s->TurboCommonMult             = SerialBufReadField(ctx, 6, 0);
   s->TurboNbSlices               = SerialBufReadField(ctx, 8, 0);
   s->TurboSliceDist              = SerialBufReadField(ctx, 8, 0);
}

void DVBSHChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct DVBSHChannelDecoderInfo *s)
{
   s->Quality                     = SerialBufReadField(ctx, 16, 0);
   s->FastQuality                 = SerialBufReadField(ctx, 16, 0);
   s->Per                         = SerialBufReadField(ctx, 32, 0);
}


void ISDBTLayerParameterUnpack(struct SerialBuf *ctx, struct ISDBTLayerParameter *s)
{
   s->Constellation               = SerialBufReadField(ctx, 5, 1);
   s->CodeRate                    = SerialBufReadField(ctx, 5, 1);
   s->NbSegments                  = SerialBufReadField(ctx, 5, 1);
   s->TimeIntlv                   = SerialBufReadField(ctx, 4, 1);
}

void ISDBTChannelParametersUnpack(struct SerialBuf *ctx, struct ISDBTChannelParameters *s)
{
   int i;
   s->Fft                         = SerialBufReadField(ctx, 8, 1);
   s->Guard                       = SerialBufReadField(ctx, 8, 1);
   s->SbMode                      = SerialBufReadField(ctx, 2, 1);
   s->PartialReception            = SerialBufReadField(ctx, 2, 1);
   s->InvSpect                    = SerialBufReadField(ctx, 2, 1);
   s->SbConTotSeg                 = SerialBufReadField(ctx, 5, 1);
   s->SbWantedSeg                 = SerialBufReadField(ctx, 5, 1);
   s->SbSubChannel                = SerialBufReadField(ctx, 7, 1);
   for(i = 0; i < 3;i++ )
   {
      ISDBTLayerParameterUnpack(ctx, &s->Layer[i]);
   }
}


void DABSubChannelUnpack(struct SerialBuf *ctx, struct DABSubChannel *s)
{
   s->Id                          = SerialBufReadField(ctx, 6, 0);
   s->StartAddress                = SerialBufReadField(ctx, 10, 0);
   s->SubSize                     = SerialBufReadField(ctx, 10, 0);
   s->Form                        = SerialBufReadField(ctx, 1, 0);
   s->TableIndex                  = SerialBufReadField(ctx, 6, 0);
   s->Option                      = SerialBufReadField(ctx, 1, 0);
   s->ProtectionLevel             = SerialBufReadField(ctx, 2, 0);
   s->IsTs                        = SerialBufReadField(ctx, 1, 0);
   s->SubChNew                    = SerialBufReadField(ctx, 0, 0);
   s->AddrRAM                     = SerialBufReadField(ctx, 0, 0);
   s->RequestedFec                = SerialBufReadField(ctx, 0, 0);
}

void DABSubChannelCanalInfoUnpack(struct SerialBuf *ctx, struct DABSubChannelCanalInfo *s)
{
   s->StartAddress                = SerialBufReadField(ctx, 16, 0);
   s->SubSize                     = SerialBufReadField(ctx, 16, 0);
   s->PI1                         = SerialBufReadField(ctx, 16, 0);
   s->PI2                         = SerialBufReadField(ctx, 16, 0);
   s->I                           = SerialBufReadField(ctx, 16, 0);
   s->L1Punct                     = SerialBufReadField(ctx, 16, 0);
   s->L2Punct                     = SerialBufReadField(ctx, 16, 0);
   s->Id                          = SerialBufReadField(ctx, 8, 0);
   s->Form                        = SerialBufReadField(ctx, 8, 0);
   s->Index                       = SerialBufReadField(ctx, 8, 0);
}

void DABChannelParametersUnpack(struct SerialBuf *ctx, struct DABChannelParameters *s)
{
   int i;
   s->TMode                       = SerialBufReadField(ctx, 8, 1);
   s->NbActiveSubCh               = SerialBufReadField(ctx, 8, 0);
   s->Reconfig                    = SerialBufReadField(ctx, 0, 0);
   s->CifOccurrenceChange         = SerialBufReadField(ctx, 0, 1);
   for(i = 0; i < 64;i++ )
   {
      DABSubChannelUnpack(ctx, &s->SubChannel[i]);
   }
}

void DABChannelShortParametersUnpack(struct SerialBuf *ctx, struct DABChannelShortParameters *s)
{
   s->TMode                       = SerialBufReadField(ctx, 8, 1);
   s->NbActiveSubCh               = SerialBufReadField(ctx, 8, 0);
}

void DABLocksUnpack(struct SerialBuf *ctx, struct DABLocks *s)
{
   s->TdIntFullInternal           = SerialBufReadField(ctx, 1, 0);
   SerialBufReadField(ctx, 5, 0);
   s->Mpeg                        = SerialBufReadField(ctx, 1, 0);
   s->Frm                         = SerialBufReadField(ctx, 1, 0);
   s->Dmb                         = SerialBufReadField(ctx, 1, 0);
   SerialBufReadField(ctx, 2, 0);
   s->NDecFrm                     = SerialBufReadField(ctx, 1, 0);
   s->NDecTMode                   = SerialBufReadField(ctx, 1, 0);
   s->CoffDab                     = SerialBufReadField(ctx, 1, 0);
   s->CoffDmb                     = SerialBufReadField(ctx, 1, 0);
   SerialBufReadField(ctx, 1, 0);
}

void DABChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct DABChannelDecoderInfo *s)
{
   s->Quality                     = SerialBufReadField(ctx, 16, 0);
   s->FastQuality                 = SerialBufReadField(ctx, 16, 0);
   s->Ber                         = SerialBufReadField(ctx, 32, 0);
   s->Per                         = SerialBufReadField(ctx, 32, 0);
   s->TdmbBer                     = SerialBufReadField(ctx, 32, 0);
}


void CMMBTimeSlotUnpack(struct SerialBuf *ctx, struct CMMBTimeSlot *s)
{
   s->RsCodeRate                  = SerialBufReadField(ctx, 3, 1);
   s->IntlvMode                   = SerialBufReadField(ctx, 3, 1);
   s->LdpcCodeRate                = SerialBufReadField(ctx, 4, 1);
   s->Constellation               = SerialBufReadField(ctx, 5, 1);
   s->Scrambling                  = SerialBufReadField(ctx, 4, 1);
   s->MfId                        = SerialBufReadField(ctx, 8, 1);
}

void CMMBChannelParametersUnpack(struct SerialBuf *ctx, struct CMMBChannelParameters *s)
{
   int i;
   s->Fft                         = SerialBufReadField(ctx, 8, 1);
   for(i = 0; i < 40;i++ )
   {
      CMMBTimeSlotUnpack(ctx, &s->TsArray[i]);
   }
   s->configFrame                 = SerialBufReadField(ctx, 16, 1);
}


void CTTBChannelParametersUnpack(struct SerialBuf *ctx, struct CTTBChannelParameters *s)
{
   s->Fft                         = SerialBufReadField(ctx, 8, 1);
   s->Constellation               = SerialBufReadField(ctx, 8, 1);
   s->Guard                       = SerialBufReadField(ctx, 8, 1);
   s->LdpcCodeRate                = SerialBufReadField(ctx, 8, 1);
   s->Intlv                       = SerialBufReadField(ctx, 8, 1);
   s->ConstantPN                  = SerialBufReadField(ctx, 8, 1);
}

void CTTBChannelDecoderInfoUnpack(struct SerialBuf *ctx, struct CTTBChannelDecoderInfo *s)
{
   s->Quality                     = SerialBufReadField(ctx, 16, 0);
   s->FastQuality                 = SerialBufReadField(ctx, 16, 0);
   s->Per                         = SerialBufReadField(ctx, 32, 0);
   s->LdpcSyndrome                = SerialBufReadField(ctx, 32, 0);
   s->LdpcSyndromeFirstIter       = SerialBufReadField(ctx, 32, 0);
   s->LdpcNbIter                  = SerialBufReadField(ctx, 8, 0);
   s->LdpcFlags                   = SerialBufReadField(ctx, 8, 0);
}


void ChannelParametersUnpack(struct SerialBuf *ctx,  int id, union ChannelParameters *s)
{
   if(0) {}
   else if(id == eSTANDARD_DVB)
   {
      DVBTChannelParametersUnpack(ctx, &s->Dvbt);
   }
   else if(id == eSTANDARD_ISDBT)
   {
      ISDBTChannelParametersUnpack(ctx, &s->Isdbt);
   }
   else if(id == eSTANDARD_DAB)
   {
      DABChannelParametersUnpack(ctx, &s->Dab);
   }
   else if(id == eSTANDARD_DVBSH)
   {
      DVBSHChannelParametersUnpack(ctx, &s->Dvbsh);
   }
   else if(id == eSTANDARD_CMMB)
   {
      CMMBChannelParametersUnpack(ctx, &s->Cmmb);
   }
   else if(id == eSTANDARD_CTTB)
   {
      CTTBChannelParametersUnpack(ctx, &s->Cttb);
   }
}


void TuneChannelParametersUnpack(struct SerialBuf *ctx,  int id, union TuneChannelParameters *s)
{
   if(0) {}
   else if(id == eSTANDARD_DVB)
   {
      DVBTChannelParametersUnpack(ctx, &s->Dvbt);
   }
   else if(id == eSTANDARD_ISDBT)
   {
      ISDBTChannelParametersUnpack(ctx, &s->Isdbt);
   }
   else if(id == eSTANDARD_DAB)
   {
      DABChannelShortParametersUnpack(ctx, &s->Dab);
   }
   else if(id == eSTANDARD_DVBSH)
   {
      DVBSHChannelParametersUnpack(ctx, &s->Dvbsh);
   }
   else if(id == eSTANDARD_CMMB)
   {
      CMMBChannelParametersUnpack(ctx, &s->Cmmb);
   }
   else if(id == eSTANDARD_CTTB)
   {
      CTTBChannelParametersUnpack(ctx, &s->Cttb);
   }
}

void ChannelDecoderInfoUnpack(struct SerialBuf *ctx,  int id, union ChannelDecoderInfo *s)
{
   if(0) {}
   else if(id == eSTANDARD_DVB)
   {
      DVBTChannelDecoderInfoUnpack(ctx, &s->Dvbt);
   }
   else if(id == eSTANDARD_ISDBT)
   {
      int i;
      for(i = 0; i < 3;i++ )
   {
      ISDBTChannelDecoderInfoUnpack(ctx, &s->Isdbt[i]);
   }
   }
   else if(id == eSTANDARD_DAB)
   {
      DABChannelDecoderInfoUnpack(ctx, &s->Dab);
   }
   else if(id == eSTANDARD_DVBSH)
   {
      DVBSHChannelDecoderInfoUnpack(ctx, &s->Dvbsh);
   }
   else if(id == eSTANDARD_CMMB)
   {
      CMMBChannelDecoderInfoUnpack(ctx, &s->Cmmb);
   }
   else if(id == eSTANDARD_CTTB)
   {
      CTTBChannelDecoderInfoUnpack(ctx, &s->Cttb);
   }
}


void SignalMonitoringUnpack(struct SerialBuf *ctx, struct SignalMonitoring *s)
{
   s->Standard                    = SerialBufReadField(ctx, 8, 0);
   s->MonitStatus                 = SerialBufReadField(ctx, 7, 0);
   s->LastDemod                   = SerialBufReadField(ctx, 1, 0);
   s->AdcPower                    = SerialBufReadField(ctx, 32, 0);
   s->AgcGlobal                   = SerialBufReadField(ctx, 16, 0);
   s->AgcRf                       = SerialBufReadField(ctx, 16, 0);
   s->AgcBb                       = SerialBufReadField(ctx, 16, 0);
   s->AgcWbd                      = SerialBufReadField(ctx, 16, 0);
   s->AgcSplitOffset              = SerialBufReadField(ctx, 16, 0);
   s->Locks                       = SerialBufReadField(ctx, 16, 0);
   s->EqualNoise                  = SerialBufReadField(ctx, 32, 0);
   s->EqualSignal                 = SerialBufReadField(ctx, 32, 0);
   s->CombNoise                   = SerialBufReadField(ctx, 32, 0);
   s->CombSignal                  = SerialBufReadField(ctx, 32, 0);
   s->TimingOffset                = SerialBufReadField(ctx, 31, 1);
   s->FrequencyOffset             = SerialBufReadField(ctx, 31, 1);
   s->TransmitterId               = SerialBufReadField(ctx, 32, 0);
   ChannelDecoderInfoUnpack(ctx, s->Standard, &s->ChanDec);
}

void ChannelDescUnpack(struct SerialBuf *ctx, struct ChannelDesc *s)
{
   s->Frequency                   = SerialBufReadField(ctx, 32, 0);
   s->Bandwidth                   = SerialBufReadField(ctx, 16, 0);
   s->Standard                    = SerialBufReadField(ctx, 5, 0);

   ChannelParametersUnpack(ctx, s->Standard, &s->Param);
}

void TuneChannelDescUnpack(struct SerialBuf *ctx, struct TuneChannelDesc *s)
{
   s->Frequency                   = SerialBufReadField(ctx, 32, 0);
   s->Bandwidth                   = SerialBufReadField(ctx, 16, 0);
   s->Standard                    = SerialBufReadField(ctx, 5, 0);

   TuneChannelParametersUnpack(ctx, s->Standard, &s->Param);
}


void TSItemParametersUnpack(struct SerialBuf *ctx, struct TSItemParameters *s)
{
   s->Pid                         = SerialBufReadField(ctx, 16, 0);
}

void SIPSIItemParametersUnpack(struct SerialBuf *ctx, struct SIPSIItemParameters *s)
{
   s->Pid                         = SerialBufReadField(ctx, 16, 0);
   s->PidWatch                    = SerialBufReadField(ctx, 1, 0);
   s->PidCrc                      = SerialBufReadField(ctx, 1, 0);
}

void MPEFECItemParametersUnpack(struct SerialBuf *ctx, struct MPEFECItemParameters *s)
{
   s->Pid                         = SerialBufReadField(ctx, 16, 0);
   s->Prefetch                    = SerialBufReadField(ctx, 1, 0);
   s->NbRows                      = SerialBufReadField(ctx, 15, 0);
   s->MaxBurstDuration            = SerialBufReadField(ctx, 16, 0);
}

void FIGItemParametersUnpack(struct SerialBuf *ctx, struct FIGItemParameters *s)
{
   s->RFU                         = SerialBufReadField(ctx, 32, 0);
}

void TDMBItemParametersUnpack(struct SerialBuf *ctx, struct TDMBItemParameters *s)
{
   s->Pid                         = SerialBufReadField(ctx, 16, 0);
   s->SubCh                       = SerialBufReadField(ctx, 16, 0);
}

void DABItemParametersUnpack(struct SerialBuf *ctx, struct DABItemParameters *s)
{
   s->SubCh                       = SerialBufReadField(ctx, 16, 0);
   s->Type                        = SerialBufReadField(ctx, 16, 0);
   s->Fec                         = SerialBufReadField(ctx, 1, 0);
}

void DABPacketItemParametersUnpack(struct SerialBuf *ctx, struct DABPacketItemParameters *s)
{
   s->SubCh                       = SerialBufReadField(ctx, 16, 0);
   s->Address                     = SerialBufReadField(ctx, 16, 0);
   s->Fec                         = SerialBufReadField(ctx, 1, 0);
   s->DataGroup                   = SerialBufReadField(ctx, 1, 0);
}





void MPEIFECItemParametersUnpack(struct SerialBuf *ctx, struct MPEIFECItemParameters *s)
{
   s->Pid                         = SerialBufReadField(ctx, 16, 0);
   s->Prefetch                    = SerialBufReadField(ctx, 1, 0);
   s->NbRows                      = SerialBufReadField(ctx, 12, 0);
   s->MaxBurstDuration            = SerialBufReadField(ctx, 16, 0);
   s->D                           = SerialBufReadField(ctx, 8, 0);
   s->B                           = SerialBufReadField(ctx, 6, 0);
   s->C                           = SerialBufReadField(ctx, 8, 0);
   s->R                           = SerialBufReadField(ctx, 7, 0);
   s->S                           = SerialBufReadField(ctx, 6, 0);
}



void CMMBSVCItemParametersUnpack(struct SerialBuf *ctx, struct CMMBSVCItemParameters *s)
{
   s->Mode                        = SerialBufReadField(ctx, 16, 0);
   s->MfId                        = SerialBufReadField(ctx, 16, 0);
   s->SvcId                       = SerialBufReadField(ctx, 16, 0);
}



void ItemParametersUnpack(struct SerialBuf *ctx,  int id, union ItemParameters *s)
{
   if(0) {}
   else if(id == eTS)
   {
      TSItemParametersUnpack(ctx, &s->Ts);
   }
   else if(id == eSIPSI)
   {
      SIPSIItemParametersUnpack(ctx, &s->Sipsi);
   }
   else if(id == eMPEFEC)
   {
      MPEFECItemParametersUnpack(ctx, &s->Mpefec);
   }
   else if(id == eFIG)
   {
      FIGItemParametersUnpack(ctx, &s->Fig);
   }
   else if(id == eTDMB)
   {
      TDMBItemParametersUnpack(ctx, &s->Tdmb);
   }
   else if(id == eDAB)
   {
      DABItemParametersUnpack(ctx, &s->Dab);
   }
   else if(id == eDABPACKET)
   {
      DABPacketItemParametersUnpack(ctx, &s->DabPacket);
   }
   else if(id == ePESVIDEO)
   {
      PESVItemParametersUnpack(ctx, &s->PesV);
   }
   else if(id == ePESAUDIO)
   {
      PESAItemParametersUnpack(ctx, &s->PesA);
   }
   else if(id == ePESOTHER)
   {
      PESOItemParametersUnpack(ctx, &s->PesO);
   }
   else if(id == ePCR)
   {
      PCRItemParametersUnpack(ctx, &s->Pcr);
   }
   else if(id == eMPEIFEC)
   {
      MPEIFECItemParametersUnpack(ctx, &s->Mpeifec);
   }
   else if(id == eCMMBMFS)
   {
      CMMBMFSItemParametersUnpack(ctx, &s->CmmbMfs);
   }
   else if(id == eCMMBCIT)
   {
      CMMBCITItemParametersUnpack(ctx, &s->CmmbCit);
   }
   else if(id == eCMMBSVC)
   {
      CMMBSVCItemParametersUnpack(ctx, &s->CmmbSvc);
   }
   else if(id == eCMMBXPE)
   {
      CMMBXPEItemParametersUnpack(ctx, &s->CmmbXpe);
   }
   else if(id == eATSCMHIP)
   {
      ATSCMHIPItemParametersUnpack(ctx, &s->AtscmhEns);
   }
}


void EFUSEInfoParametersUnpack(struct SerialBuf *ctx, struct EFUSEInfoParameters *s)
{
   s->IsRead                      = SerialBufReadField(ctx, 1, 0);
}

void SDRAMInfoParametersUnpack(struct SerialBuf *ctx, struct SDRAMInfoParameters *s)
{
   s->State                       = SerialBufReadField(ctx, 8, 0);
   s->IsUsedForCaching            = SerialBufReadField(ctx, 1, 0);
}

void GPIOInfoParametersUnpack(struct SerialBuf *ctx, struct GPIOInfoParameters *s)
{
   s->Num                         = SerialBufReadField(ctx, 4, 0);
   s->Val                         = SerialBufReadField(ctx, 8, 0);
   s->Dir                         = SerialBufReadField(ctx, 8, 0);
}

void InfoParametersUnpack(struct SerialBuf *ctx,  int id, union InfoParameters *s)
{
   if(0) {}
   else if(id == eINFO_EFUSE)
   {
      EFUSEInfoParametersUnpack(ctx, &s->Efuse);
   }
   else if(id == eINFO_SDRAM)
   {
      SDRAMInfoParametersUnpack(ctx, &s->Sdram);
   }
   else if(id == eINFO_GPIO)
   {
      GPIOInfoParametersUnpack(ctx, &s->Gpio);
   }
}



