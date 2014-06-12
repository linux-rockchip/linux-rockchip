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

#include "DibExtAPI.h"
#include "stdio.h"
#include "math.h"

/*********************************************/
/*** Global Variables ************************/
struct DibGlobalInfo    globalInfo;
struct DibDriverContext *pContext = NULL;
CHANNEL_HDL             DibChannelHdl;
FILTER_HDL              IsdbtFilterHdl;
ELEM_HDL                IsdbtItemHdlList[4];
union  DibDemodMonit    Monit[4];	

#define ISDBT_BUFFER_SIZE 200000
unsigned char IsdbtDataBuffer[ISDBT_BUFFER_SIZE];
/*********************************************/

/*********************************************/
/*** Local Functions *************************/
/*********************************************/
int Open( enum DibBoardType BoardType );
int TuneSb(struct DibChannel * ChDesc);
int SbSearch(unsigned int Frequency, enum DibSpectrumBW Bw, struct DibChannel ChDesc[]);
int Monitor (void);
int ReceiveDataWithMpegTS (void);
int ReceiveDataWithFilter (void);
int IsdbtSbMpegLocked(int layer);
int GetTsbFrequencyOffset();
void printIsdbtParameters(struct DibChannel * ChDesc);

/*********************************************/
/*** Test Parameters *************************/
/*********************************************/
/*enum DibBoardType  Board       = eBOARD_NIM_1009xHx;*/
enum DibBoardType  Board      = eBOARD_NIM_10096MD4;     
uint32_t           Frequency  = 737143;
enum DibSpectrumBW Bandwidth  = eCOFDM_BW_6_0_MHZ;
enum DibStreamTypes StreamType =  eSTREAM_TYPE_HOST_DMA;
//enum DibStreamTypes StreamType =  eSTREAM_TYPE_MPEG_1;
unsigned char MpegTsMode       = 1; /* 0 => Serial, 1=> Parallel **/
unsigned char MpegTsSize       = 1; /* 0 => 188,    1=> 204 **/

unsigned short     PidList[4] = {0,110,120,130};
/*********************************************/

void printIsdbtParameters(struct DibChannel * ChDesc)
{
   int layer;
   printf("Type       = %d\n", ChDesc->Type);
   printf("RFkHz      = %d\n", ChDesc->ChannelDescriptor.RFkHz);
   printf("Bw         = %d\n", ChDesc->ChannelDescriptor.Bw);
   printf("fft        = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.Nfft);
   printf("guard      = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.Guard);
   printf("sb_mode    = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.SbMode);
   printf("partial    = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.PartialReception);
   printf("inv_spec   = %d\n", ChDesc->ChannelDescriptor.InvSpec);
   printf("sb_tot_seg = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.SbConnTotalSeg);
   printf("sb_wan_seg = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.SbWantedSeg);
   printf("sb_sub_ch  = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.SbSubchannel);

   for(layer = 0; layer < 3; layer++) {
       printf("layer %c\n", 'A' + layer);
       printf("   nb seg   = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.layer[layer].NbSegments);
       printf("   const    = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.layer[layer].Constellation);
       printf("   crate    = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.layer[layer].CodeRate);
       printf("   timeint  = %d\n", ChDesc->ChannelDescriptor.Std.Isdbt.layer[layer].TimeIntlv);
   }
}

int main(int argc, char** argv)
{
    DIBSTATUS rc;
    unsigned int i;
    int autosearch_sb = 1, autosearch_nb_sub = 0;
    struct DibChannel ChDesc[25]; /* 13+12 channel for max number of segments */
    char saisie[255];
    int idx;

    /*** Open Driver ***/
    rc = Open(Board);
    
    if (rc == DIBSTATUS_SUCCESS) {
        if(autosearch_sb == 1) {
            autosearch_nb_sub = SbSearch(Frequency, Bandwidth, &ChDesc[0]);
            if(autosearch_nb_sub > 0) {
                printf("\nISDB-Tsb Search engine finaly found %d Tsb service(s)\n", autosearch_nb_sub);
                for(i = 0 ; i< autosearch_nb_sub ; i++) {
                    printf("CHANNEL INDEX [%d] : ISDB-Tsb on RF %6dKhz SBmode = %d SubChannel ID = %d Pratial = %d\n",i,
                    ChDesc[i].ChannelDescriptor.RFkHz,
                    ChDesc[i].ChannelDescriptor.Std.Isdbt.SbMode,
                    ChDesc[i].ChannelDescriptor.Std.Isdbt.SbSubchannel,
                    ChDesc[i].ChannelDescriptor.Std.Isdbt.PartialReception);
                }

                while(1) {
                    printf("Enter the CHANNEL INDEX you want to tune on:\n");
                    fgets(saisie, 255, stdin);
                    if (sscanf(saisie, "%d", &idx) == 1)
                        if((idx >= 0) && (idx <= (autosearch_nb_sub-1)))
                            break;
                    printf("Sorry, this channel index does not exist in the list.\n");
                    
                }
                printf("You choose to tune on Tsb #%d\n",idx);
            } else {
                printf("ISDB-T SB Autosearch endded and did not found any subchannels\n");
                return 0;
            }
        }

        printIsdbtParameters(&ChDesc[idx]);
        /*** Tune Channel ***/
        rc = TuneSb(&ChDesc[idx]);

        printf("Press Key to Continue\n");
        getchar();

        if (rc == DIBSTATUS_SUCCESS) {
            for (i=0;i<100;i++) { /*** Monitor Signal ***/
                SignalMonitor();
            }
        }

        printf("Press Key to Continue\n");
        getchar();

        if(StreamType == eSTREAM_TYPE_MPEG_1) { /*** Start Data Reception in Mpeg TS mode ***/
            ReceiveDataWithMpegTS ();
        } else if(StreamType == eSTREAM_TYPE_HOST_DMA) { /*** Start Data Reception in HostDma Mode ***/
            ReceiveDataWithHostDma();
        } 
        printf("Press Key to Continue\n");
        getchar();

        /*** Close Driver ***/
        DibClose(pContext);
    }
}

/**********************************************/
/********** Driver Initialization *************/
/**********************************************/
int Open( enum DibBoardType BoardType )
{
    if(DibOpen(&pContext, BoardType, 0) != DIBSTATUS_SUCCESS) {
       printf("Open Driver Failed  \n");
       return DIBSTATUS_ERROR;
    }
    
    /*  Get driver version */
    DibGetGlobalInfo(pContext, &globalInfo);
    printf("Driver ver: %d.%d.%d\n", 
            DIB_MAJOR_VER(globalInfo.DriverVersion), 
            DIB_MINOR_VER(globalInfo.DriverVersion), 
            DIB_REVISION(globalInfo.DriverVersion));
    printf("Embedded SW ver: %d.%02d (IC: %d)\n", 
            (globalInfo.EmbVersions[0] >> 10), 
            globalInfo.EmbVersions[0] & 0x03ff, 
            globalInfo.EmbVersions[1]);
    printf("Number of demod %d \n", globalInfo.NumberOfDemods);
    
    return DIBSTATUS_SUCCESS;
}

/**********************************************/
/********** Tune Channel **********************/
/**********************************************/
int TuneSb(struct DibChannel * ChDesc)
{
    DIBSTATUS Status = DIBSTATUS_SUCCESS;
    int i;

#if 0   
    struct DibChannel  ChDesc;
    /* Initialise tune parameters */
    
    /** Physical Layer **/
    ChDesc.Type                              = eSTANDARD_ISDBT_1SEG;
    ChDesc.ChannelDescriptor.Bw		     = eCOFDM_BW_6_0_MHZ;
    
    /** Common Parameters **/
    ChDesc.ChannelDescriptor.RFkHz           = Frequency;
    ChDesc.ChannelDescriptor.InvSpec         = eDIB_SPECTRUM_AUTO;
    
    /** ISDBT Parameters **/
    ChDesc.ChannelDescriptor.Std.Isdbt.Nfft  = eDIB_FFT_AUTO;
    ChDesc.ChannelDescriptor.Std.Isdbt.Guard = eDIB_GUARD_INTERVAL_AUTO;
    
    /*** Partial Reception = AUTO ***/
    ChDesc.ChannelDescriptor.Std.Isdbt.PartialReception = 0;
    
    /*** Disable SB mode ***/
    ChDesc.ChannelDescriptor.Std.Isdbt.SbMode         = 1;
    ChDesc.ChannelDescriptor.Std.Isdbt.SbConnTotalSeg = 1;
    ChDesc.ChannelDescriptor.Std.Isdbt.SbWantedSeg    = 1;
    ChDesc.ChannelDescriptor.Std.Isdbt.SbSubchannel   = 22;
    
    /*** Initialize Layers ***/
    for (i=0;i<3;i++)
    {
        ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].NbSegments    = -1;
        ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].Constellation = eDIB_QAM_AUTO;
        ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].CodeRate      = eDIB_VIT_CODERATE_AUTO;
        ChDesc.ChannelDescriptor.Std.Isdbt.layer[i].TimeIntlv     = -1;
    }
#endif
    /* Create, initialise a channel and tune on specified frequency */
    Status = DibGetChannel(pContext, 
                           DIB_DEMOD_AUTO, 
                           DIB_DEMOD_AUTO, 
                           StreamType | (MpegTsMode << 16) | (MpegTsSize<<17), 
                           ChDesc, 
                           &DibChannelHdl, 
                           NULL, 
                           NULL);
    
    if((Status == DIBSTATUS_SUCCESS) &&  (ChDesc->ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)) {
        printf("Tune Successfull\n");
        return DIBSTATUS_SUCCESS;
    } else {
        printf("Tune Failed \n");
        return DIBSTATUS_ERROR;
    }
}

/**********************************************/
/********** Signal Monitoring *****************/
/**********************************************/
int SignalMonitor (void)
{
    DIBSTATUS Status = DIBSTATUS_SUCCESS;
    uint8_t NbDemod = 1;
    double CN;
    unsigned int MpegLockA,MpegLockB,MpegLockC;
    double BerA,BerB,BerC;
    double sig_dB, wgn_dB;

    /*** Get Signal Monitoring ***/
    Status = DibGetSignalMonitoring(pContext, 
                                       DibChannelHdl, 
                                       &NbDemod,
                                       &Monit[0], 
                                       NULL, 
                                       NULL);
    
    if (Status == DIBSTATUS_SUCCESS) {
        BerA = (double) Monit[0].Isdbt.Layer[0].FecBerRs / 1e8;
        BerB = (double) Monit[0].Isdbt.Layer[1].FecBerRs / 1e8;
        BerC = (double) Monit[0].Isdbt.Layer[2].FecBerRs / 1e8;
          
        MpegLockA=DIB_GET_ISDBT_LOCK_MPEG0(Monit[0].Isdbt.Dvb.Locks);
        MpegLockB=DIB_GET_ISDBT_LOCK_MPEG1(Monit[0].Isdbt.Dvb.Locks);
        MpegLockC=DIB_GET_ISDBT_LOCK_MPEG2(Monit[0].Isdbt.Dvb.Locks);
    
        if((Monit[0].Isdbt.Dvb.SigFp == 0) || (Monit[0].Isdbt.Dvb.WgnFp==0)) {
            CN = -100;
        } else {
            sig_dB = 10*log10((double)Monit[0].Isdbt.Dvb.SigFp) - 320*log10((double)2);
            wgn_dB = 10*log10((double)Monit[0].Isdbt.Dvb.WgnFp) - 320*log10((double)2);
            CN = sig_dB - wgn_dB;
        }

        printf("Signal Monitoring:\n");
        printf("C/N: %5.4lf\n",CN);
        printf("MPEG Lock Layer A : %d, BER: %f\n",MpegLockA, BerA);
        printf("MPEG Lock Layer B : %d, BER: %f\n",MpegLockB, BerB);
        printf("MPEG Lock Layer C : %d, BER: %f\n",MpegLockC, BerC);
    }
    return DIBSTATUS_SUCCESS;
}

/**********************************************/
/********** Mpeg Lock ISDBT CHeck *************/
/**********************************************/
int IsdbtSbMpegLocked(int layer)
{
    DIBSTATUS Status = DIBSTATUS_SUCCESS;
    uint8_t NbDemod = 1;
    unsigned int MpegLock = 0;
    uint32_t AbsFrequencyOffset, halftuningStep_Hz = 71428;

    /*** Get Signal Monitoring ***/
    Status = DibGetSignalMonitoring(pContext, 
                                       DibChannelHdl, 
                                       &NbDemod,
                                       &Monit[0], 
                                       NULL, 
                                       NULL);

    if (Status == DIBSTATUS_SUCCESS) {
        switch (layer) {
            case 0: MpegLock = DIB_GET_ISDBT_LOCK_MPEG0(Monit[0].Isdbt.Dvb.Locks);break;
            case 1: MpegLock = DIB_GET_ISDBT_LOCK_MPEG1(Monit[0].Isdbt.Dvb.Locks);break;
            case 2: MpegLock = DIB_GET_ISDBT_LOCK_MPEG2(Monit[0].Isdbt.Dvb.Locks);break;
        }
        if(MpegLock == 1)
            return DIBSTATUS_SUCCESS;
    }

    return DIBSTATUS_ERROR;
}

/**********************************************/
/********** Mpeg Lock ISDBT CHeck *************/
/**********************************************/
int GetTsbFrequencyOffset()
{
    DIBSTATUS Status = DIBSTATUS_SUCCESS;
    uint8_t NbDemod = 1;

    /*** Get Signal Monitoring ***/
    Status = DibGetSignalMonitoring(pContext, 
                                       DibChannelHdl, 
                                       &NbDemod,
                                       &Monit[0], 
                                       NULL, 
                                       NULL);

    if (Status == DIBSTATUS_SUCCESS) {
        printf("Frequency Offset = %d\n", Monit[0].Isdbt.Dvb.FrequencyOffset);
        return Monit[0].Isdbt.Dvb.FrequencyOffset;
    }

    return DIBSTATUS_ERROR;
}



/**********************************************/
/********** Data reception using MPEG TS bus***/
/**********************************************/
int ReceiveDataWithMpegTS (void)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;
    union DibFilters Filter[1];

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = NULL;
    info.CallbackUserCtx = NULL;

    /*** Create TS Filter ***/
    Status = DibCreateFilter(pContext, DibChannelHdl, eTS, &info, &IsdbtFilterHdl);

    if(Status == DIBSTATUS_SUCCESS) {
        printf(("ISDBT Raw TS Filter Create Successfull \n"));
        Filter[0].Ts.Pid = DIB_TSALLPIDS;

        /*** Add all PIDs ***/
        Status = DibAddToFilter(pContext, IsdbtFilterHdl, 1, Filter, IsdbtItemHdlList);

        if(Status == DIBSTATUS_SUCCESS) {
            printf("Add All Pids Succesfull\n");
        } else {
            printf("Add Pids Failed\n");
        }
    }

    return 0;
}

/************************************************/
/* Data reception using filter in Host DMA mode */
/************************************************/
void IsdbtDataCallback(void *dataContext, struct DibBufContext *pBuffCtx, enum DibDataStatus Status)
{
    if (Status == eREMOVED) {
        return;
    }
    printf("ISDBT MPEG TS block received, Size = %d\n",pBuffCtx->BufSize);

    /*** register again the same buffer ***/
    DibRegisterBuffer(pContext, IsdbtFilterHdl, IsdbtDataBuffer, ISDBT_BUFFER_SIZE, 0);
}

int ReceiveDataWithHostDma(void)
{
    DIBSTATUS  Status;
    struct DibDataBuffer info;
    union DibFilters Filter[4];

    info.Timeout      = DIB_TIMEOUT_IMMEDIATE;
    info.DataMode     = eCLBACK;
    info.CallbackFunc = IsdbtDataCallback;
    info.CallbackUserCtx = NULL;

    /*** Create Filter, only for monitoring ***/
    Status = DibCreateFilter(pContext, DibChannelHdl, eTS, &info, &IsdbtFilterHdl);

    if(Status == DIBSTATUS_SUCCESS) {
        printf(("ISDBT Raw TS Filter Create Successfull \n"));

        /*** Register a Buffer to receive DAB frames ***/
        Status = DibRegisterBuffer(pContext, IsdbtFilterHdl, IsdbtDataBuffer, ISDBT_BUFFER_SIZE, 0);

        if(Status == DIBSTATUS_SUCCESS) {
            Filter[0].Ts.Pid = PidList[0];
            Filter[1].Ts.Pid = PidList[1];
            Filter[2].Ts.Pid = PidList[2];
            Filter[3].Ts.Pid = PidList[3];

            /*** Start Service, only for monitoring ***/
            Status = DibAddToFilter(pContext, IsdbtFilterHdl, 4, Filter, IsdbtItemHdlList);

            if(Status == DIBSTATUS_SUCCESS) {
                printf("Add Pids Succesfull\n");
            } else {
                printf("Add Pids Failed\n");
            }
        }
    } else {
        printf(("ISDBT Filter Create Failed \n"));
    }
    return Status;
}

/**********************************************/
/**********   SB search    ********************/
/**********************************************/
void initSbSearchParam(struct DibChannel *ChDesc, unsigned int RF_seg, unsigned int subchan_id, uint8_t mode_3seg)
{
    int i = 0;

   /** Physical Layer **/
   ChDesc->Type                              = eSTANDARD_ISDBT_1SEG;
   ChDesc->ChannelDescriptor.Bw		     = eCOFDM_BW_6_0_MHZ;

   /** Common Parameters **/                 
   ChDesc->ChannelDescriptor.RFkHz           = RF_seg;
   ChDesc->ChannelDescriptor.InvSpec         = eDIB_SPECTRUM_AUTO;

   /** ISDBT Parameters **/                  
   ChDesc->ChannelDescriptor.Std.Isdbt.Nfft  = eDIB_FFT_AUTO;
   ChDesc->ChannelDescriptor.Std.Isdbt.Guard = eDIB_GUARD_INTERVAL_AUTO;

   /*** Partial Reception = AUTO ***/
   ChDesc->ChannelDescriptor.Std.Isdbt.PartialReception = mode_3seg;

   /*** Disable SB mode ***/
   ChDesc->ChannelDescriptor.Std.Isdbt.SbMode         = 1;
   ChDesc->ChannelDescriptor.Std.Isdbt.SbConnTotalSeg = 1;
   ChDesc->ChannelDescriptor.Std.Isdbt.SbWantedSeg    = 1;

   ChDesc->ChannelDescriptor.Std.Isdbt.SbSubchannel   = subchan_id;

   /*** Initialize Layers ***/
   for (i=0;i<3;i++) {
       ChDesc->ChannelDescriptor.Std.Isdbt.layer[i].NbSegments    = -1;
       ChDesc->ChannelDescriptor.Std.Isdbt.layer[i].Constellation = eDIB_QAM_AUTO;
       ChDesc->ChannelDescriptor.Std.Isdbt.layer[i].CodeRate      = eDIB_VIT_CODERATE_AUTO;
       ChDesc->ChannelDescriptor.Std.Isdbt.layer[i].TimeIntlv     = -1;
   }
}

void UpdateFreqArrays(int * rf_seg_arr, int * seg_subchan, int FrequencyOffset_KHz)
{
    int i, subchannel_offset = 0;
    for(i = 0; i < 13 ; i++) {
        subchannel_offset = FrequencyOffset_KHz/143000;

        printf("update rf_seg[%2d] from %6d to %6d and seg_subchan[%2d] from %2d to %2d\n", i, rf_seg_arr[i], rf_seg_arr[i]+(FrequencyOffset_KHz/1000), i, seg_subchan[i], seg_subchan[i]+subchannel_offset);
        rf_seg_arr[i] += (FrequencyOffset_KHz/1000);
        seg_subchan[i] += subchannel_offset;
    }
}

int SbSearch(unsigned int Frequency, enum DibSpectrumBW Bw, struct DibChannel ChDesc[])
{
    DIBSTATUS  Status;
    const unsigned int tuning_step = 143; /* 1/7MHz in KHz */
    int rf_offset_khz = 0;
    int channel_index = 0;
    int seg = 0;
    int FrequencyOffset;

    /* Arrays storage 
        In the ODD nb seg case :
        - rf_seg       -> 13 segments center frequency in Khz, array index is the segment index from 0 on the extreme left to 12 on the exteme right
        - seg_subchan  -> 13 corresponding subchannel for each odd segment's position.
     */
    int seg_rf[13]; 
    int seg_subchan[13];
    uint8_t seg_result[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

    int seg_rf_freq = 0;
    int seg_subch_id = 0;
    int restart_search =0;

    /* Try ODD 13 segments position first */
    uint8_t searchFailled = 0, odd_first = 1;
    int nb_seg = 13;

    printf("Preparing full ODD nb of segment center freq associated with their subchannel ID\n");
    for(seg = 0 ; seg < nb_seg ; seg++) {
        /* odd nb of segment computation */
        rf_offset_khz = (-18+(3*seg)) * tuning_step;
        seg_rf[seg]  = Frequency + rf_offset_khz;
        seg_subchan[seg] = 4+(3*seg);
        printf("seg_index %2d, freq = %7dkhz (%7d + %5d) subchan_id = %d\n", seg, seg_rf[seg], Frequency, rf_offset_khz, seg_subchan[seg]);
    }
    printf("Starting Tsb Search...\n");
    seg = 0;
    do {
        restart_search = 0;
        seg_rf_freq = seg_rf[seg];
        seg_subch_id = seg_subchan[seg];
        printf("-> SB search on ODD segment #%d center freq = %dkhz subch_id = %d\n", seg, seg_rf_freq, seg_subch_id);

        initSbSearchParam(&ChDesc[channel_index], seg_rf_freq, seg_subch_id, 0);
        /* Create, initialise a channel and tune on specified frequency */
        Status = DibGetChannel(pContext, 
                               DIB_DEMOD_AUTO,
                               DIB_DEMOD_AUTO,
                               StreamType | (MpegTsMode << 16) | (MpegTsSize<<17),
                               &ChDesc[channel_index],
                               &DibChannelHdl,
                               NULL, 
                               NULL);

        if((Status == DIBSTATUS_SUCCESS) &&  (ChDesc[channel_index].ChannelDescriptor.ScanStatus == DIB_SCAN_SUCCESS)) {
            usleep(1000000);
            if(IsdbtSbMpegLocked(0) == DIBSTATUS_SUCCESS) {
                FrequencyOffset = GetTsbFrequencyOffset();
                if(abs(FrequencyOffset) > (1000*tuning_step/2)) {
                    UpdateFreqArrays(&seg_rf[0], &seg_subchan[0], FrequencyOffset); /* one SEG was found, apply Rf frequency of array for next search */
                    restart_search = 1;
                    printf("Restart search with updated freq and subchannels\n");
                } else {
                    ChDesc[channel_index].ChannelDescriptor.RFkHz += FrequencyOffset/1000; /* update current frequency accuracy */
                    printIsdbtParameters(&ChDesc[channel_index]);
                    SignalMonitor();
                    channel_index++;
                    seg_result[seg] = channel_index;
                }
             } else
                initSbSearchParam(&ChDesc[channel_index], seg_rf_freq, seg_subch_id, 0);
        }

        DibDeleteChannel(pContext, DibChannelHdl);

        if(restart_search == 1) {
            channel_index = 0;
            seg = 0;
        } else {
            seg++;
        }

    } while (seg < nb_seg);

    printf("-> Tsb search results:\n");
    for (seg = 0; seg< 13; seg++)
        printf("[SEG %2d] [%6.3f MHz] [SUB %2d] [%s]\n", seg, seg_rf[seg]/1000.0, seg_subchan[seg], (seg_result[seg]!=0)?"Tsb detected":"------------");

    /* Check layer A & B mpeg lock on 3 SEG found channel */
    for (seg = 1; seg < 12; seg++) {
        if((seg_result[seg-1] == 0) && (seg_result[seg] == 1) && (seg_result[seg+1] == 0)) {
            printf("Check tune for 3 SEG channel on segment [%d %d %d] ?\n", seg-1, seg, seg+1);
            struct DibChannel test3segChDesc;
            test3segChDesc = ChDesc[seg_result[seg]-1]; /* seg_result was storing in case of success the channelindex+1 */
            initSbSearchParam(&test3segChDesc, seg_rf[seg], seg_subchan[seg], 1);
            Status = DibGetChannel(pContext, 
                               DIB_DEMOD_AUTO,
                               DIB_DEMOD_AUTO,
                               StreamType | (MpegTsMode << 16) | (MpegTsSize<<17),
                               &test3segChDesc,
                               &DibChannelHdl,
                               NULL, 
                               NULL);

            usleep(1000000);
            if((IsdbtSbMpegLocked(0) == DIBSTATUS_SUCCESS) && (IsdbtSbMpegLocked(1) == DIBSTATUS_SUCCESS)) {
                printf("3SEG tune success !\n");
                printIsdbtParameters(&test3segChDesc);
                SignalMonitor();
                ChDesc[seg_result[seg]-1] = test3segChDesc;
            } else
                printf("3SEG tune failled\n");
            DibDeleteChannel(pContext, DibChannelHdl);
        }
    }

    return channel_index; /* number of found subchannel */
}

