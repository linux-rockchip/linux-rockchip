/*************************************************************************/
/*                                                                       */
/* Copyright (C) 2005-2009 Siano Mobile Silicon Ltd. All rights reserved */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
#ifndef STRINGS_AND_TAGS_H
#define STRINGS_AND_TAGS_H

// (1) Define code generating macro
// Start of string definition
#define GENERATE_IDS    \
X(STR_TAG_FOR_DEBUG, "Debug Msg: %d")	\
X(STR_TAG_TEST, "Test format %04d 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X")				\
X(STR_TAG_SMS_PRINTF, "%s") /* Can be used only on nova and may remove in the future to save memory */ \
X(STR_TAG_SKIP_FIC, "Skip the first fic after tune")												\
X(STR_TAG_IDLE, "Regards from Idle %010u")															\
X(STR_TAG_TUNE, "Tune to Frequency %09dHz BW %d Crystal %09dHz")									\
X(STR_TAG_MFER1, "SNR=%u dB, MFER=%u, %d, %d, TS_PER %d%%, %u,%u,d/s=%d")									\
X(STR_TAG_MFER2, "SNR =%u dB,MFER=%u, %d, %d, TS_PER %d%%, t=%u, crl=%i, d/s=%d")					\
X(STR_TAG_BER1, "BER=%ix10^-5, %u, %u [S], FirstAddr=%u, t=%u, crl=%i, DLU_TO=%d")									\
X(STR_TAG_MFER_NA, "SNR = %u dB, MFER is NA, TS_PER %i%%, %u %u, t=%u, d/s=%i")				\
X(STR_TAG_REACQUIRE, "crl=%i, trl=%i")																\
X(STR_TAG_RSSI_MSR_TIM, "RSSI measured @0,6,12,18 dB gain is:%d %d %d %d counter is: %i %i %i %i")	\
X(STR_TAG_RSSI_MSR_TIM1, "RSSI measured @0,6,12,18 dB gain is:%d %d %d %d")	\
X(STR_TAG_AGC1, "<<AGC>>: RSSI -%d, EXLN %d , LN %d, MX %d, PG %d, LP %d, Pib %d")							\
X(STR_TAG_AGC2, "<<AGC>>: ADrms %d, LPrms %d, AGCref %d, AGCCummErr %d, AGCState %d, MaxRaw %d, Clips %d")							\
X(STR_TAG_AGC3, "<<AGC>>: Adj %d, Alt %d, LPF_BW %d, AGCAcqTime %d us")	\
X(STR_TAG_TDMB_AGC1, "RSSI -%d, EXLN %d, LN %d, MX %d, PG %d, LP %d")							\
X(STR_TAG_TDMB_AGC2, "Pib %d, ADrms %d, LPrms %d, AGCref %d, RawI %d, RawQ %d")							\
X(STR_TAG_TDMB_PRE_BER, "PRE BER = %iE-5, %u, %u")						\
X(STR_TAG_TDMB_STAT, "SNR=%udB,PER=%i%%, %u, %u, BER=%iE-5, %u, %u")							\
X(STR_TAG_TDMB_MODEM, "Frame=%d, NULL=%d,%d,%d, crl=%d, trl=%d, ssync=%d")							\
X(STR_TAG_DMB_RESYNC1, "RSSI: %d %d %d %d")							\
X(STR_TAG_DMB_RESYNC2, "Tlock=%d, WBO=%d bns, SBO=%d/16 bns, DemodFreq = %dHz")							\
X(STR_TAG_TDMB4, "<<TDMB>>: Resync !")							\
X(STR_TAG_DEBUG_RECONFIG_START, "Reconfig start for CIF %i - %i sub ch in mem")							\
X(STR_TAG_DEBUG_RECONFIG_MID, "Reconfig: new start cu = %i")							\
X(STR_TAG_DEBUG_RECONFIG_END, "Reconfig end for CIF %i after %i usec - %i sub ch in mem, %i copied")							\
X(STR_TAG_CP_AMP, "CP amp is %i")																	\
X(STR_TAG_RESYNC_STATUS, "Resyncing, Type %d, Status=%d")											\
X(STR_TAG_RESYNC_SNR, "Resyncing, Type %d, SNR=%d")													\
X(STR_TAG_RESYNC_5PARAM, "Resyncing, Type %d, cor_st=0x%x, fec_st=0x%x, SNR=%d, Slope=%d" )		\
X(STR_TAG_PID, "mpe_pid: %d")																		\
X(STR_TAG_FREQ_LIST, "frequency: %u")																\
X(STR_TAG_SIREADY, "HO_MNG_SIReady")																\
X(STR_TAG_STATEINIT, "HO_MNG_StateInit")															\
X(STR_TAG_STATE_IDEL1, "HO_MNG_StateIdle In band power = %i  SNR = %d  Analyzes = %d  Frequency = %d "  )\
X(STR_TAG_STATE_IDEL2, "HO_MNG_StateIdle Handover status, SNFLH = %d")								\
X(STR_TAG_STATE_SCAN, "HO_MNG_StateScan")															\
X(STR_TAG_STATE_SCAN2,"HO_MNG_StateScan Scan req. freq = %d")										\
X(STR_TAG_STATE_HANDOVER, "HO_MNG_StateHandover ")													\
X(STR_TAG_SCANCALLBACKFUNC, "HO_MNG_ScanCallbackFunc Analyzes = %d ")								\
X(STR_TAG_HANDOVERCALLBACKFUNC, "HO_MNG_HandoverCallbackFunc")										\
X(STR_TAG_SET_FREQ, "freq: %d bw : %d")																\
X(STR_TAG_DLU_TIMER3_ISR, "DLU_DLU_TIMER3, scanned freq = %d, SNR = %d, In band power = %d, returning to freq %d ")			\
X(STR_TAG_DLU_SET_SCANNED_REQUEST, "DLU_SCANNED_REQUEST, origin freq = %d, scanned freq = %d")		\
X(STR_TAG_DLU_SET_DEMOD_STATE, "DLU_SET_DEMOD_STATE, origin freq = %d, scanned freq = %d")			\
X(STR_TAG_MOBILITY_UPDATE, "Update Mobility to %d, m= %d") 										  	\
X(STR_TAG_FREQ_FILTER_UPDATE, "Update FreqFilter to %d, SFN = %d") 									\
X(STR_TAG_ARC_SLOP_UPDATE, "Update Arc Slope") 														\
X(STR_TAG_IFFT_RES, "IFFT results: Freq Filter = %d, SFN = %d, IsMobile = %d, m= %d, Slope=%d")		\
X(HO_TAG_SIGNAL_LOST, "Handover: Signal Lost!")														\
X(STR_TAG_DLU_DLU_SET_HANDOVER_REQUEST, "DLU_SET_HANDOVER_REQUEST to freq = %d")						\
X(STR_TAG_MOBILITY_PARAMS, "IsMobile=%d,m_MobilityIsMobile=%d,IsHigh_Low_Mobility=%d,m_IsHigh_Low_Mobility=%d") \
X(STR_TAG_ONE_SYMBOL, "One Symbol!")																	\
X(STR_TAG_RETRIALS, "Retrials counter = %d")															\
X(STR_TAG_NARROW, "MFN Filter ! ")																	\
X(STR_TAG_WIDE, "WMFN Filter ! ")																	\
X(STR_TAG_SFN, "SFN Filter ! ")																		\
X(STR_TAG_DELTA, "Delta Filter ! ")																	\
X(STR_TAG_POST_CUR, "Post Cursive ! ")																\
X(STR_TAG_PRE_CUR, "Pre Cursive ! ")																	\
X(STR_TAG_TAPS, "Taps:  %d  %d   %d")																\
X(STR_TAG_DELAYS_BINS1, "BINS:%d %d Eg: %u %u Filter = %u, PeaksNum=%u")\
X(STR_TAG_DELAYS_BINS2, "Slope r,i: %d %d ErrRat r,i: %d %d")                                        \
X(STR_TAG_CHC_COEEFS, "CHC Coeffs: %d   %d   %d")													\
X(STR_TAG_DELAYS_TE, "DelaysTe:    %d      %d")														\
X(STR_TAG_ARCSLOPE, "ARCSLOPE:    %d,COSDIFF     %d")												\
X(STR_TAG_ACQ_TIMING, "Timing: SYR = %d, COARSE = %d, PRE_IFFT = %d, IFFT1 = %d, IFFT2 = %d, 0xF6 = %d, RS_SYNC = %d, TOT = %d") \
X(STR_TAG_FAST_ACQ, "FAST ACQ: NsymsInSlice=%d,TsymsInSlice=%d,CurrS/T=%d,AVG S/T=%d, dT=%d,SnumX100=%d,Snum=%d,SnumEndOfSlice=%d") \
X(STR_TAG_PID_CHANGE, "Change PID: , PID = %u, d-Time = 0x%x, Index = %u")									\
X(STR_TAG_PID_CHANGE1, "Change PID: , Global Time = 0x%x, d-Time = 0x%x, Demod = %d")				\
X(STR_TAG_PID_CHANGE2, "PID Update HEAD: , pHead = %u, pHead-Dtime = 0x%x, pMember = %u, pMember-Dtime = 0x%x") \
X(STR_TAG_DBG_DLU_TIM, "ACT dT: %d, READ dT: %d, ACQ time: %d")										\
X(STR_TAG_SCAN_LOCK, "SCAN_MNG:lock acquired on channel %d, BW= %dMHz, SNR = %d, InBand Power = %d\n") \
X(STR_TAG_SCAN_TIMER_EXPIRED, "SCAN_MNG:Timer Expired on channel %d, BW= %dMHz\n")					\
X(STR_TAG_SCAN_START_CHAN, "SCAN_MNG:Starting Scan on channel %d, BW= %dMHz\n")						\
X(STR_TAG_DBG_SYR_OFFSET, "DBG SYR: %d,%d,%d,%d,%d,%d")	\
X(STR_TAG_DELAYS_BINS3, "BINS:%d %d MaxMobBin: %d Eg: %u %u %u Filter = %d, PeaksNum=%d")                       \
X(STR_TAG_DELAYS_BINS4, "BINS:%d %d %d Eg: %u %u %d Errs = %d, PeaksNum=%d")                         \
X(STR_TAG_DELAYS_BINS5, "SecondRelToMax: %d BINS:%d %d %d Eg: %u %u %u, PeaksNum=%d")                \
X(STR_TAG_DELAYS_BINS6, "BINS: %d, %d, %d, %d, %d Eg: %u, %u, PeaksNum=%d")                          \
X(STR_TAG_DELAYS_BINS10, "BINS:%d %d %d Eg: %u %u %u SNR = %u, PeaksNum=%u") \
X(STR_TAG_DELAYS_BINS11, "AVG Eg: %d %d %d %d %d, Peak[4]=%d, Peak[5]=%d") \
X(STR_TAG_DELAYS_BINS7, "BINS:%d %d IsMobile: %u Eg: %u %u Slope: %d Filter = %d, PeaksNum=%d")                \
X(STR_TAG_DELAYS_BINS8, "BINS:%d %d IsMobile: %u Eg: %u %u Slope: %d Errs = %d, Filter = %d")                \
X(STR_TAG_DELAYS_BINS9, "BINS:%d %d Eg: %u %u Filter = %u, PeaksNum=%u, Errors=%u, SNR=%u") \
X(STR_TAG_DVBT_STAT, "DVBT: %u, %u, PER = %u x 1E-5, Slope = %d") \
X(STR_TAG_SRVMNG_TUNE_ON,					"DVBH SrvMng: TUNE_ON")									\
X(STR_TAG_SRVMNG_RESYNC_IND,				"DVBH SrvMng: RESYNC_IND")								\
X(STR_TAG_SRVMNG_INBAND_POWER_FAILURE_IND,	"DVBH SrvMng: INBAND_POWER_FAILURE, inbandPower = %d")	\
X(STR_TAG_SRVMNG_SYNC_OK_IND,				"DVBH SrvMng: SYNC_OK_IND, signalStrength = %d")							\
X(STR_TAG_SRVMNG_TRACKING,					"DVBH SrvMng: SWITCH_TO_TRACKING, inBandPower = %d")	\
X(STR_TAG_SRVMNG_TUNE_OFF,					"DVBH SrvMng: TUNE_OFF")								\
X(STR_TAG_RESYNC_STATUS_10,                 "Resyncing, Type 10, Status=%d")						\
X(STR_TAG_RESYNC_STATUS_7,                  "Resyncing, Type 7, Status=%d")							\
X(STR_TAG_RESYNC_STATUS_8,                  "Resyncing, Type 8, Status=%d")							\
X(STR_TAG_RESYNC_STATUS_5,                  "Resyncing, Type 5, Status=%d")							\
X(STR_TAG_RESYNC_STATUS_6,					"Resyncing, Type 6, Status=%d")							\
X(STR_TAG_RESYNC_4PARAM_1, "Resyncing, 1, cor_st=0x%x, fec_st=0x%x, SNR=%d, Slope=%d" )				\
X(STR_TAG_RESYNC_4PARAM_2, "Resyncing, 2, cor_st=0x%x, fec_st=0x%x, SNR=%d, Slope=%d" )				\
X(STR_TAG_RESYNC_4PARAM_9, "Resyncing, 2, cor_st=0x%x, fec_st=0x%x, SNR=%d, Slope=%d" )				\
X(STR_TAG_DBG_SYR_OFFSET_3,                 "DBG SYR: %d,%d,%d") \
X(STR_TAG_DAB_SYNC_PHASE,                 "DAB: Sync phase: %d %d %d %d") \
X(STR_TAG_ISDBT_1, "Mode %d, GI %d, Modulation %d, WBO %d, SBO %d, CRL %d, TRL %d, CarOfs %dHz")     \
X(STR_TAG_ISDBT_2, "Frame %d, SNR %d dB, LAYER A: PRE BER %dE-5, POST BER %dE-5, TS PER %d%%, AckTime %d ms,CRL %d, TRL %d")							\
X(STR_TAG_ISDBT_3, "Frame %d, SNR %d dB, LAYER B: PRE BER %dE-5, POST BER %dE-5, TS PER %d%%, AckTime %d ms,CRL %d, TRL %d")							\
X(STR_TAG_ISDBT_4, "LAYER A: PRE BER %d, %d, POST BER %d, %d, TS PER %d,%d, TS PER ACC %d,%d")							\
X(STR_TAG_ISDBT_5, "LAYER B:PRE BER %d, %d, POST BER %d, %d, TS PER %d,%d, TS PER ACC %d,%d")							\
X(STR_TAG_ISDBT_6, "PeakVal %d, PeakIdx %d, TRLLast %d, IFFT_Te %d, SYROfs %d, SCR %d, CHCSlope %d, TRL %d")		\
X(STR_TAG_ISDBT_7, "A: CR %d MOD %d NSEG %d TIL %d B: CR %d MOD %d NSEG %d TIL %d ")     \
X(STR_TAG_ISDBT_8, "N_Peaks %d, Peak Loc %d, %d, %d, %d, Peak Width %d, %d, %d") \
X(STR_TAG_ISDBT_9, "N_Peaks %d, Peak_E %d, %d, %d, %d, Peak_S %d, %d, %d") \
X(STR_TAG_ISDBT_10, "%d %d %d %d %d %d %d %d") \
X(STR_TAG_ISDBT_11, "Fd %d, Mean %d, F_Mean %d, T_F %d, %d, %d, %d, STD %d") \
X(STR_TAG_ISDBT_12, "Peak Merged, N_Peaks = %d, Peak Loc %d, %d, %d, Peak Width %d, %d, %d, %d") \
X(STR_TAG_ISDBT_13, "IIR on Peaks: N_Peaks = %d, Peak Loc %d, %d, %d, Peak Width %d, %d, %d, IIR_const %d") \
X(STR_TAG_ECHO_MOBILITY1, "Num Of Echo=%d, Location: Echo1=%duSec, Echo2=%duSec, Echo3=%duSec, Echo4=%duSec, Echo5=%dSec") \
X(STR_TAG_ECHO_MOBILITY2, "Echo Energy [Relative]: Echo1=%d, Echo2=%d, Echo3=%d, Echo4=%d, Echo5=%d, Mobility=%d")\
X(STR_TAG_DLU_FRAME_FULL, "TS packet received while processing a previous frame pid %d") \
X(STR_TAG_DLU_NOT_INITIALIZED, "TS packet received for uninitialized pid %d, requested pid %d") \
X(STR_TAG_DLU_INVALID_RESET, "reset for uninitialized pid %d defined pid is %d") \
X(STR_TAG_DLU_SRV_ALREADY_EXISTS, "service already exists for pid %d") \
X(STR_TAG_DLU_MAX_SVC_REACHED, "max services reached") \
X(STR_TAG_DSP_FRAME_COMPLETED, "frame completed status %d param %d") \
X(STR_TAG_DLU_SMART_ERR, "pid %d smart error number %d ") \
X(STR_TAG_DLU_BAD_IDX, "pid %d smart error number %d ") \
X(STR_TAG_DLU_SMART_LOG, "smart log line %d params %d %d") \
X(STR_TAG_DLU_SMART_WARN, "smart warning line %d params %d %d") \
X(STR_TAG_ISDBT_MEM_LIMIT, "ISDB-T: TDIL internal memory limited in Layer %d")\
X(STR_TAG_CMMB_PLAYING_ADD_CHANNEL, "CmmbPlaying add channel %d" ) \
X(STR_TAG_CMMB_PLAYING_REMOVE_CHANNEL, "CmmbPlaying remove channel %d" ) \
X(STR_TAG_CMMB_PLAYING_ERROR_LOST_TIMESLOTS, "CmmbPlaying Error got pre TS lock and timeslot info did not arrive. Mask %llx" ) \
X(STR_TAG_CMMB_PLAYING_FRAME_SWITCH, "CmmbPlaying switching Bcast frames. Channel mask: %#x->%#x, Time slot mask: %#x->%#x" ) \
X(STR_TAG_CMMB_PLAYING_TIMESLOT_MISSING, "CmmbPlaying Timeslot %d is missing in TS2AHB output" ) \
X(STR_TAG_TDMB_01, "SYNC PHASE: Tlock %d, WBO %d, SBO %d, TXMODE %d, SYRAdj %d, CarrierOffset %d Hz")\
X(STR_TAG_TDMB_02, "Tune to %d Hz")\
X(STR_TAG_TDMB_03, "Modem resync, type %d")\
X(STR_TAG_TDMB_04, "OPERATIONAL PHASE: Frame %d, NullRatio %d, CRL %d, TRL %d, SYRadj %d, TDMB1 %d, TDMB2 %d, DAB %d")\
X(STR_TAG_TDMB_05, "STAT: SNR %d, PRE BER %d, %d, %dE-5, POST BER %d, %d, %dE-5")\
X(STR_TAG_TDMB_06, "STAT: TS PER %d, %d, %d %%, TS PER ACC %d, %d")\
X(STR_TAG_TDMB_07, "AGC: RSSI -%d, LNA %d, MX %d, PGA %d, LPF %d, Pib %d, ADrms %d, LPrms %d")\
X(STR_TAG_TDMB_08, "AGC: AGCref %d, ID %d, Tacq %d, Adj %d, Alt %d")\
X(STR_TAG_TDMB_09, "DEBUG_IFFT_VAL: %d,%d,%d,%d,%d,%d,%d,%d")\
X(STR_TAG_TDMB_10, "DEBUG_IFFT_END: %d")\
X(STR_TAG_TDMB_11, "DEBUG_IFFT_PEAKS: MAX_ENERGY %d, THRESHOLD %d, PEAKS_SPAN %d, PEAKS_CENTER %d")\
X(STR_TAG_CMMB_PEAK_WIDTH_TYPE1, "peak_width (u) %d NarrowFilter %d (%dusec)" )\
X(STR_TAG_CMMB_PEAK_WIDTH_TYPE2, "peak_width (u) %d Filter delta" )\
X(STR_TAG_CMMB_WBO_FOUNDED, "WBO offset=%d!=0" )\
X(STR_TAG_CMMB_WBO_INFO, "WBO offset=%d prbs_idx	%d WBO_ABS_ACC %d" )\
X(STR_TAG_CMMB_WBO_FAILED, "WBO FAILED	- Resyncing"  )\
X(STR_TAG_CMMB_WBO, "WBO[%d] too late")\
X(STR_TAG_CMMB_WAKE_UP_COLD, "DEMOD_WAKE_UP_FIRST_TIME -	COLD INIT")\
X(STR_TAG_CMMB_WAKE_UP_WARM, "DEMOD_WAKE_UP_FIRST_TIME -	WARM INIT")\
X(STR_TAG_CMMB_SYR_LOCKED, "SYR Locked, magnitude=%d")\
X(STR_TAG_CMMB_SYR_LOW, "SYR too low, restarting sm")\
X(STR_TAG_CMMB_SBO, "SBO=%d")\
X(STR_TAG_CMMB_FAST_REAC, "FAST_REACQUISITION: ts=%d,	snum=%d,HW snum=%d")\
X(STR_TAG_CMMB_SDU_RATIO1, "SDU max/min=%d/%d=%d,snum=%d,fnum=%d")\
X(STR_TAG_CMMB_SDU_RATIO2, "SDU max/min=%d/%d=%d")\
X(STR_TAG_CMMB_BEACONE_FOUND, "Beacone Found")\
X(STR_TAG_CMMB_BEACONE_TIMEOUT, "timeOut, restarting")\
X(STR_TAG_CMMB_RF_OFF, "Turn Off RF trl=%d, crl=%d")\
X(STR_TAG_CMMB_BDIL_FINISH, "BDIL Finish")\
X(STR_TAG_CMMB_DEMOD_RESYNC, "Demod Resyncing")\
X(STR_TAG_CMMB_ADC_INFO, "ADC_deviation %d ADC_newFreq %d NewNominalRate %d CarrierOffset %d NOM_FREQ %d")\
X(STR_TAG_CMMB_PRBS_FINISH, "Finish Finding all PRBS path %d")\
X(STR_TAG_CMMB_PRBS_ERR, "prbs[%d]!=%d (err)")\
X(STR_TAG_CMMB_TS_ERR, "TS error, expected	%d,	got	%d, snum=%d")\
X(STR_TAG_CMMB_PRBS, "prbs[%d]=%d (-1)")\
X(STR_TAG_CMMB_FAST_REAC_START, "Start Fast Reacq")\
X(STR_TAG_CMMB_CLK_OFF, "TURN OFF FDU and FECU first block CLOCKS")\
X(STR_TAG_CMMB_SNR_INFO, "SNR=%d.%02d/%d.%02d/%d.%02d/%d (CP/IIE/FDU%dCP/CSI)")\
X(STR_TAG_CMMB_TRL_INFO1, "TRL_NomFreqResetCounter %d TRL_StillConverged %d")\
X(STR_TAG_CMMB_TRL_INFO2, "NR %d TRL_STEP_SIZE %d bias %d NR_By_FREQ %d Real Period %d")\
X(STR_TAG_CMMB_TRL_INFO3, "Init TRL Nominal Rate %d")\
X(STR_TAG_CMMB_IFFT_INFO1, "TRL_STEP_SIZE %d Total_Time_in_Te %d Delta_Time %d, DeltaACC %d")\
X(STR_TAG_CMMB_IFFT_INFO2, "Threshold %d   numberOfRelevantPeaks %d")\
X(STR_TAG_CMMB_IFFT_INFO3, "max_energy = %d")\
X(STR_TAG_CMMB_IFFT_INFO4, "Check IFFT time; %u,%u,%u,%u, flag=%u")\
X(STR_TAG_CMMB_MULTI_PRBS, "g_MultiPRBS_flag = %d")\
X(STR_TAG_CMMB_LDPC_CYCLES, "LDPC %d %d %d %d ptr = %d")\
X(STR_TAG_CMMB_AGC0, "IntScanCalcPinband: lna %d mix %d pga %d pib %d")\
X(STR_TAG_CMMB_AGC1, "AGC new INT support - CLIP detector enabled")\
X(STR_TAG_CMMB_AGC2, "INT: adc_rms %d, lpf_rms %d agc_ref %d lpf_bw %d")\
X(STR_TAG_CMMB_AGC3, "<<AGC>>: RSSI -%d, LNA %d, MX %d, PGA %d, LPF %d, Pib %d, ADrms %d, LPrms %d")\
X(STR_TAG_CMMB_AGC4, "<<AGC %d>>: AGCref %d, acumm_err %d DC_i %d DC_q %d id %d")\
X(STR_TAG_CMMB_AGC5, "EX LNA -- > Before ON SmAGC[Path].p_in %d")\
X(STR_TAG_CMMB_AGC6, "New LNA-MIX Algo:, p_in %d, lna_gain %d, mix_gain %d")\
X(STR_TAG_CMMB_AGC7, "New LNA-MIX Algo:, pin_lna_total %d,pin_lna_inband %d, lna_gain %d, pin_mix_total %d, pin_mix_inband %d, mix_gain %d")\
X(STR_TAG_CMMB_AGC8, "INT SCAN: (-3,-2,-1,0,1,2,3) %d, %d, %d, %d, %d, %d, %d")\
X(STR_TAG_CMMB_AGC9, "SmAGC[Path].TOTAL_adc_clip_results %d New AGC_ref = %d")\
X(STR_TAG_CMMB_AGC10, "DCC OFF")\
X(STR_TAG_CMMB_AGC11, "DCC ON")\
X(STR_TAG_CMMB_AGC12, "LPF ERROR")\
X(STR_TAG_CMMB_WBO_THR, "cmmb_demod_sync_db.WBO_thresh = %d")\
X(STR_TAG_CMMB_TX_ERR, "Tx err %i override by %i !!!")\
X(STR_TAG_CMMB_AGC_ADC_CLOCK, "ADC clock = %d,%d, NR=%d")\
X(STR_TAG_CMMB_AGC_MOBILITY0, "Mobility changed to static!")\
X(STR_TAG_CMMB_AGC_MOBILITY1, "Mobility changed to mobile!")\
X(STR_TAG_CMMB_AGC_MOBILITY2, "MD: Fd = %d, Mean = %d, Filtered = %d, TIME_Filter = %d, STD = %d std2mean_iir %d")\
X(STR_TAG_CMMB_IFFT_INFO5, "Max Peak Idx = %d , TRL_res = %d New Nominal Rate = %d Center_of_band = %d")\
X(STR_TAG_CMMB_IFFT_INFO6, "SYR Offset = %d ,SCR_Slope = %d, CHC_arc_slope = %d, Ifft_Offset = %d")\
X(STR_TAG_CMMB_IFFT_INFO7, "%d %d %d %d %d %d %d %d")\
X(STR_TAG_CMMB_IFFT_INFO8, "IFFT DUMP done %d,%d,%d,%d,%d,%d")\
X(STR_TAG_CMMB_IFFT_INFO9, "SYR FIRST TIME: Max Peak Idx = %d , TRL_res = %d New Nominal Rate = %d Center_of_band = %d")\
X(STR_TAG_CMMB_IFFT_INFO10, "IFFT ERROR")\
X(STR_TAG_CMMB_IFFT_INFO11, "IFFT_1ST_TONE = %d")\
X(STR_TAG_CMMB_FAST_REAC1, "Sleep time %d")\
X(STR_TAG_CMMB_ADC_INFO1, "mean_real %d mean_imag %d variance_r %d variance_i %d STD_r %d STD_i %d")\
X(STR_TAG_CMMB_ADC_INFO2, "Found ADC Harmonies InBand diff %d freq=%d i=%d  j=%d adc_clock_bias=%d")\
X(STR_TAG_CMMB_ADC_INFO3, "Found SYS Harmonies InBand diff %d freq=%d i=%d  j=%d sys_clock_bias=%d")\
X(STR_TAG_CMMB_ADC_INFO4, "Freq=%llu,BW=%d,SYSclk=%d,ADCclk=%d,clock_bias=%d")\
X(STR_TAG_CMMB_IFFT_INFO12, "TRL idx overflow %d")\
X(STR_TAG_CMMB_IFFT_INFO13, "IFFT: Too much peaks - %d")\
X(STR_TAG_CMMB_IFFT_INFO14, "IFFT: Peak Separated")\
X(STR_TAG_CMMB_IFFT_INFO15, "IFFT: Found Zero Peaks")\
X(STR_TAG_CMMB_IFFT_INFO16, "IFFT: Doppler Peak 128")\
X(STR_TAG_CMMB_IFFT_INFO17, "Discard peak - idx = %d Energy %d")\
X(STR_TAG_CMMB_IFFT_INFO18, "IFFT: Too many peaks to merge - %d")\
X(STR_TAG_CMMB_IFFT_INFO19, "Max Acc=%i, Max Idx=%i, Max Idx AVG=%i")\
X(STR_TAG_CMMB_IFFT_INFO20, "IIR PeaksNum %d, Loc %d,%d,%d,%d, Width %d,%d,%d,%d")\
X(STR_TAG_CMMB_IFFT_INFO21, "Peak Overflow : %d")\
X(STR_TAG_CMMB_DUMP1, "DUMP DONE Frequency %d")\
X(STR_TAG_CMMB_DB_RST, "DB RST" )\
X(STR_TAG_CMMB_SFR_ERR, "sfr Err!")\
X(STR_TAG_CMMB_UNKNOWN, "Unknown table type 0x%x")\
X(STR_TAG_CMMB_ERR, "Err! Data inactive ch" )\
X(STR_TAG_CMMB_DEM_ERR, "Demux Err - header bad" )\
X(STR_TAG_CMMB_DEM_WAR, "Demux warn - expected ChId %d, got %d")\
X(STR_TAG_CMMB_DATA, "Data frame(%d)" )\
X(STR_TAG_CMMB_CTL_FRM, "Ctrl frame(0)" )\
X(STR_TAG_CMMB_SPECIAL, "Special channel! not supported" )\
X(STR_TAG_CMMB_ERR_NO_ACTIVE_CH, "Error - no active ch!!" )\
X(STR_TAG_CMMB_RS_FIX, "RS fix Bytes %d Rows %d, Avg b/l %d" )\
X(STR_TAG_CMMB_RS_BAD_ROWS, "RS bad %d/%d rows" )\
X(STR_TAG_CMMB_RS_BAD_SEC, "RS Bad second" )\
X(STR_TAG_CMMB_RS_LDPC_RESYNC, "Software resync - either RS or LDPC criteria" )\
X(STR_TAG_CMMB_RS_BUSY, " RS is busy - waiting..."  )\
X(STR_TAG_CMMB_TS, "TS %d LDPC not cvrg %d avg cyc %d")\
X(STR_TAG_CMMB_LDPC_RESYNC, "LDPC resync" )\
X(STR_TAG_CMMB_CHUNK, "First chunk in bCast frame not first in RS table (ts=%d) - discarding...")\
X(STR_TAG_CMMB_SSL_WITH_ERR, "SSL with errors. max errors in a row=%d number of FEC comumns=%d\n")\
X(STR_TAG_CMMB_MIN_RS_TABLE, "Min RS table size changed from %d to %d")\
X(STR_TAG_CMMB_DLUMGR_ADD, "DluMgr add ch %d TSs %d-%d RSCR %d INTR %d LDPC_CR %d CONST %d SCR %d")\
X(STR_TAG_CMMB_DLUMGR_REM, "DluMgr Rem ch %d number of active channels %d" )\
X(STR_TAG_CMMB_CLOSING_EMPTY_SSL, "Closing empty SSL!")\
X(STR_TAG_CMMB_OUT_OF_SPACE, "Ssl ran out of space!")\
X(STR_TAG_CMMB_SSL_NOT_TERM, "Ssl not terminated! Maximum SSL Entries=%d")\
X(STR_TAG_CMMB_INVALID_CHUNK, "Invalid chunk!" )\
X(STR_TAG_CMMB_ERR_NO_CHANNEL_INFO, "ERR - no channel info" )\
X(STR_TAG_CMMB_UNEXPECTED, "UNEXPECTED - Service info changed" )\
X(STR_TAG_CMMB_REQ_FREQ, "Req freq %d, cur freq %d")\
X(STR_TAG_CMMB_START_SER, "Start ser %#x L/N %d/%d")\
X(STR_TAG_CMMB_CHANGE_FREQ, "CmmbMgrChangeFreqIfPossible Retcode %d" )\
X(STR_TAG_CMMB_ST_SER, "Start Service handle %d subframe number %d")\
X(STR_TAG_CMMB_STOP_SER, "Stop ser hdl %d")\
X(STR_TAG_CMMB_TUNE, "Tune %d")\
X(STR_TAG_CMMB_NOT_TUNED, "Not Tuned Yet")\
X(STR_TAG_CMMB_DEM_RESYNC, "Demod resync" )\
X(STR_TAG_CMMB_SCAN_PROG, "Scan progress: %u IsGood %d IsLast %d")\
X(STR_TAG_CMMB_START_CH, "Start ch #%x")\
X(STR_TAG_CMMB_STOP_CH, "Stop ch #%x")\
X(STR_TAG_CMMB_SCAN_START, "Scan start" )\
X(STR_TAG_CMMB_START_CTRL_INFO, "Start ctrl info L/N %d/%d")\
X(STR_TAG_CMMB_OUT_FMT, "Out fmt %d")\
X(STR_TAG_CMMB_CMMB_MGR_ERR, "-- CmmbMgr ERROR! %d --")\
X(STR_TAG_CMMB_DEM_FULL_LOCK, "Demod full lock. Lock time %d us")\
X(STR_TAG_CMMB_UNHANDLED_EVT, "Unhandled demod event!" )\
X(STR_TAG_CMMB_TOT_BUF_SIZE, "Total ts2ahb buffering size %d")\
X(STR_TAG_CMMB_FIFO_OVERFLOW, "TS2AHB BIT fifo overflow")\
X(STR_TAG_CMMB_NEW_Q, "New q. depth - %d/%d")\
X(STR_TAG_CMMB_ERROR_NO_PENDING, "Error - no pending TS")\
X(STR_TAG_CMMB_TIMESLOT_MASK, "timeslot mask=%d")\
X(STR_TAG_CMMB_CH_LTW, "ch%d: LTW %d LNC %d RTR %d RBR %d PLB %d FHG %d FHB %d ")\
X(STR_TAG_CMMB_FADD_AD_HOC_CHANNEL0, "Added Ad Hoc channel 0. Error=%x")\
X(STR_TAG_CMMB_REMOVE_PERMANENT_CHANNEL0, "Removed channel 0 due to addition of channel %d. Error=%x")\
X(STR_TAG_CMMB_ADD_PERMANENT_CHANNEL0, "No active channels after remove channel. adding channel 0")\
X(STR_TAG_CMMB_CLEAR_AD_HOC_CHANNEL0, "Clearing Ad Hoc channel 0")\
X(STR_TAG_CMMB_SET_PERIODIC_STATS, "Set periodic statistics %d")\
X(STR_TAG_CMMB_CTRL_TBL_RES, "Control Table Res: id %d, size %d" )\
X(STR_TAG_CMMB_MSGS_AFTER_0_IN_TX_Q, "Warning - there are pending messages after 0 msg in TX queue" )\
X(STR_TAG_CMMB_SYS_NOT_DIV, "Error - ADC clock (%u) is not a multiple of sys (%u).")\
X(STR_TAG_CMMB_FLUSHING_MSG, "Flushing message (index %d)")\
X(STR_TAG_CMMB_TABLE_INJECTION, "Injected table id=0x%x, length=%d error=%0x%x")\
X(STR_TAG_CMMB_PLAYING_FULL_BCAST_FRAME_ELAPSED, "Full broadcast frame elapled. gap=%d" )\
X(STR_TAG_CMMB_PLAYING_CLEAR_CHANNEL, "Clear channel %d" )\
X(STR_TAG_CMMB_DATA_ON_RESYNC, "TS2AHB data on resync" )\
X(STR_TAG_CMMB_PLAYING_ADD_CHANNEL_PARAMS, "CmmbPlaying channel start on %08X closest possible addition %08X now=%08X skip=%d" )\
X(STR_TAG_CMMB_PLAYING_CLEAR_SKIP, "Clear skip flag for new channel %d" )\
X(STR_TAG_CMMB_LOG_REGULATOR_DROPPED_LOGS, "Logs regualtor dropped logs!" )\
X(STR_TAG_CMMB_LOG_BUFFER_OVERRUN, "Log buffer overrun!" )\
X(STR_TAG_CMMB_LOG_DISCARDED_BUSY, "Log when other log is busy discarded!" )\
X(STR_TAG_CMMB_CSCT_SERVICES_ERR, "CSCT error - number of services %d is larger than the space left (left: %d svsc)")\
X(STR_TAG_CMMB_IN_INTERRUPT, "Function called from ISR!!!")\
X(STR_TAG_CMMB_OUTPUT_WHILE_DEMOD_ACTIVE, "Warning - UnBlocked! (%d/%d) (%d)")\
X(STR_TAG_CMMB_SHORT_SLEEP, "Wrong short sleep calculation cycles=%d new cycles=%d diff=%d path %d")\
X(STR_TAG_CMMB_IFFT_ALGO0, "IFFT configure IFFT HW path %d PRBS %d")\
X(STR_TAG_CMMB_IFFT_ALGO1, "Finish IFFT on path %d")\
X(STR_TAG_CMMB_OPERATIONAL0, "Switch to OPERATIONAL path %d, other is OPERATIONAL")\
X(STR_TAG_CMMB_OPERATIONAL1, "Switch to OPERATIONAL path %d, other is WAIT_OPERATIONAL")\
X(STR_TAG_CMMB_OPERATIONAL2, "Switch to OPERATIONAL path %d, other is NeedsFallOver")\
X(STR_TAG_CMMB_ADC_OFF, "ADC off")\
X(STR_TAG_CMMB_FAST_REAC2, "TimeUsec is zero!!")\
X(STR_TAG_CMMB_FAST_REAC3, "TsDiff is zero !!!!!")\
X(STR_TAG_CMMB_FAST_REAC4, "Next TS time is in the past!!")\
X(STR_TAG_CMMB_PATH_FALLOVER, "path %d is out of sync")\
X(STR_TAG_CMMB_OPEN_BLOCKER, "Open Blocker ts %d")\
X(STR_TAG_CMMB_TRL_CONVERGED, "CONVERGED on path %d")\
X(STR_TAG_CMMB_CTRL_TBL_CRC_ERR, "Control table CRC error! Discarding. Table ID: %d")\
X(STR_TAG_CMMB_ZERO_TIMESLOTS, "Zero number of playing timeslots!")\
X(STR_TAG_CMMB_DISCARD_CHUNK, "Discarding TS2AHB chunk!" )\
X(STR_TAG_CMMB_LDPC_BAD_SEC, "LDPC Bad second" )\
X(STR_TAG_CMMB_CTRL_INFO_CHANGE, "Control info changed")\
X(STR_TAG_CMMB_ADC_SLEEPING_ERROR, "GOING TO SLEEP BECAUSE OF ADC OFF!!!!!!!!!!")\
X(STR_TAG_CMMB_UNEXPECTED_DEMOD_STOPPED, "Error: demod stopped on unexpected tune state %d")\
X(STR_TAG_CMMB_RSSI_SAMPLE, "RSSI Sample: num=%d sample=%d  set to %d tune state=%d")\
X(STR_TAG_CMMB_MRR_SET_GPIO, "CMMB Mgr set GPIO=%d IsOn=%d")\













/* 	X(STR_TAG_DLU_DLU_TIMER2_ISR, "DLU_TIMER2_ISR ho finish moved back from freq %d to %d")				\
X(STR_TAG_HO_TIMER, "HO Timer Expierd")																\
X(STR_TAG_HO_T, "timer : %d")																		\
X(STR_TAG_HO_TIMER2, "SNR: %d  IN P: %i")															\
X(STR_TAG_MOBILITY_PARAMS, "Ratio_r=%d,Ratio_i=%d,slope_r=%d,slope_i=%d") \

End of string definition */

// (2) Define X-Macro for generating enum members
#define X(id, idString) id,

typedef enum
{
    // To be on the safe side,
    // force ID's to start at 0
    STR_TAG_ID_LOWERBOUND = -1,
    GENERATE_IDS
    STR_NUMBER_OF_TAGS
} STRING_TAGS_E;

#endif /*#ifndef STRINGS_AND_TAGS_H*/


