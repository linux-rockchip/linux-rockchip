
#include <linux/time.h>
#include "adapter/frontend.h"
#include "platform/platform.h"
#include "platform/platform_linux_kernel.h"



int feid_to_chid(struct dibChannel *ch, int feid)
{
    int i;
    for (i = 0; i< MAX_NB_OF_FRONTENDS_IN_CHAIN; i++) {
        if (ch->chid_to_feid[i] == feid) {
            return i;
        }
    }
    return -1;
}

#ifdef CONFIG_STANDARD_DVBT
static void init_dvbt_channel(struct dibChannel *ch)
{
    ch->bandwidth_kHz = 8000;

    ch->u.dvbt.nfft               = FFT_AUTO;
    ch->u.dvbt.guard              = GUARD_INTERVAL_AUTO;
    ch->u.dvbt.spectrum_inversion = -1;
    ch->u.dvbt.constellation      = QAM_AUTO;
    ch->u.dvbt.intlv_native       = INTLV_NATIVE_AUTO;
    ch->u.dvbt.hrch               = VIT_HRCH_AUTO;
    ch->u.dvbt.select_hp          = VIT_PRIORITY_AUTO;
    ch->u.dvbt.alpha              = VIT_ALPHA_AUTO;
    ch->u.dvbt.code_rate_hp       = VIT_CODERATE_AUTO;
    ch->u.dvbt.code_rate_lp       = VIT_CODERATE_AUTO;

}

#ifdef CONFIG_BUILD_HOST
static uint8_t digital_dvbt_channels_equal(struct dibChannel* ch1, struct dibChannel* ch2)
{
    return (ch1->u.dvbt.nfft == ch2->u.dvbt.nfft) && (ch1->u.dvbt.guard == ch2->u.dvbt.guard) &&
        (ch1->u.dvbt.constellation == ch2->u.dvbt.constellation) && (ch1->u.dvbt.intlv_native == ch2->u.dvbt.intlv_native) &&
        (ch1->u.dvbt.hrch == ch2->u.dvbt.hrch) && (ch1->u.dvbt.alpha == ch2->u.dvbt.alpha) &&
        (ch1->u.dvbt.code_rate_hp == ch2->u.dvbt.code_rate_hp &&
        ((ch1->u.dvbt.hrch == 1 && (ch1->u.dvbt.code_rate_lp == ch2->u.dvbt.code_rate_lp)) || ch1->u.dvbt.hrch != 1));
}
#endif


static void init_dvbsh_channel(struct dibChannel *ch)
{
    ch->bandwidth_kHz = 8000;

    ch->u.dvbsh.dvb_common.nfft               = FFT_AUTO;
    ch->u.dvbsh.dvb_common.guard              = GUARD_INTERVAL_AUTO;
    ch->u.dvbsh.dvb_common.spectrum_inversion = -1;
    ch->u.dvbsh.dvb_common.constellation      = QAM_AUTO;
    ch->u.dvbsh.dvb_common.intlv_native        = 0;
    ch->u.dvbsh.dvb_common.hrch               = VIT_HRCH_AUTO;
    ch->u.dvbsh.dvb_common.select_hp          = VIT_PRIORITY_AUTO;
    ch->u.dvbsh.dvb_common.alpha              = VIT_ALPHA_AUTO;
    ch->u.dvbsh.dvb_common.code_rate_hp       = TURBO_CODERATE_AUTO;
    ch->u.dvbsh.dvb_common.code_rate_lp       = TURBO_CODERATE_AUTO;
    ch->u.dvbsh.dvb_sh             = 1;
    ch->u.dvbsh.common_mult        = 0;
    ch->u.dvbsh.nof_late_taps      = 0;
    ch->u.dvbsh.nof_slices         = 0;
    ch->u.dvbsh.slice_distance     = 0;
    ch->u.dvbsh.non_late_incr      = 0;

}

#ifdef CONFIG_BUILD_HOST
static uint8_t digital_dvbsh_channels_equal(struct dibChannel* ch1, struct dibChannel* ch2)
{
    return (ch1->u.dvbsh.dvb_common.nfft          == ch2->u.dvbsh.dvb_common.nfft) &&
           (ch1->u.dvbsh.dvb_common.guard         == ch2->u.dvbsh.dvb_common.guard) &&
           (ch1->u.dvbsh.dvb_common.constellation == ch2->u.dvbsh.dvb_common.constellation) &&
           (ch1->u.dvbsh.dvb_common.intlv_native  == ch2->u.dvbsh.dvb_common.intlv_native) &&
           (ch1->u.dvbsh.dvb_common.hrch          == ch2->u.dvbsh.dvb_common.hrch) &&
           (ch1->u.dvbsh.dvb_common.alpha         == ch2->u.dvbsh.dvb_common.alpha) &&
           (ch1->u.dvbsh.dvb_sh                   == ch2->u.dvbsh.dvb_sh) &&
           (ch1->u.dvbsh.common_mult              == ch2->u.dvbsh.common_mult) &&
           (ch1->u.dvbsh.nof_late_taps            == ch2->u.dvbsh.nof_late_taps) &&
           (ch1->u.dvbsh.nof_slices               == ch2->u.dvbsh.nof_slices) &&
           (ch1->u.dvbsh.slice_distance           == ch2->u.dvbsh.slice_distance) &&
           (ch1->u.dvbsh.non_late_incr            == ch2->u.dvbsh.non_late_incr) &&
           (ch1->u.dvbsh.dvb_common.code_rate_hp  == ch2->u.dvbsh.dvb_common.code_rate_hp &&
           ((ch1->u.dvbsh.dvb_common.hrch == 1 && (ch1->u.dvbsh.dvb_common.code_rate_lp == ch2->u.dvbsh.dvb_common.code_rate_lp)) || ch1->u.dvbsh.dvb_common.hrch != 1));
}
#endif
#endif

#ifdef CONFIG_STANDARD_DAB

void set_tdmb_dab_channel_reconfig(struct dibChannel *ch) {
    ch->u.dab.reconfig = 1;
}

void set_tdmb_dab_channel_occurence_change(struct dibChannel *ch, uint8_t cif_occurence_change) {
    ch->u.dab.cif_occurence_change = cif_occurence_change;
}

int set_tdmb_channel_fec_state(struct dibChannel *ch, uint8_t subChid, uint8_t onoff) {
    uint8_t sub;

    for (sub=0 ; sub < ch->u.dab.nb_active_sub ; sub++) {
        if ((ch->u.dab.subChannel[sub].is_ts) && (ch->u.dab.subChannel[sub].subChid == subChid)) {
            ch->u.dab.subChannel[sub].requested_fec = onoff;
            return DIB_RETURN_SUCCESS;
        }
    }
    return DIB_RETURN_ERROR;
}

int set_fig01_02_to_channel(struct dibChannel *ch, uint8_t *fig0_1_buff, uint8_t *fig0_2_buff)
{
#ifdef CONFIG_STANDARD_DAB
    uint8_t  ptr1 = 1, ptr2 = 1, sub_index = 0;
    uint16_t sub = 0, i = 0;
    uint16_t start_addr_table[DAB_MAX_SUBC];
    uint16_t start, swap, tmp;

    DibZeroMemory(start_addr_table, DAB_MAX_SUBC*sizeof(uint16_t));
    DibZeroMemory(ch->u.dab.subChannel, DAB_MAX_SUBC*sizeof(struct dibDABsubChannel));

    /* store all start adress like it comes in the buffer into start_addr_table */
    do {
        start_addr_table[sub] = ( (*(fig0_1_buff+ptr1)&0x3) << 8) + (*(fig0_1_buff+ptr1+1));
        if (*(fig0_1_buff+ptr1+2)&0x80)
            ptr1+= 4;
        else
            ptr1+= 3;
        sub++;

    } while (ptr1 < *fig0_1_buff);

#if 0
  /* debug before bubble sort*/
   for(i=0 ; i<sub ; i++)
        dbgpl(NULL, "before bubble sort start_addr_table[%d] = %d",i, start_addr_table[i]);
#endif

    /* Bubble sorting of the start_addr_table by start_addr ASC */
    do {
        swap = 0;
        for(i=0 ; i<sub ; i++) {
            if(start_addr_table[i+1] < start_addr_table[i] && (i+1 < sub)) {
                tmp = start_addr_table[i];
                start_addr_table[i] = start_addr_table[i+1];
                start_addr_table[i+1] = tmp;
                swap++;
            }
        }
    } while(swap > 0);

#if 0
    /* after bubble sort*/
   for(i=0 ; i<sub ; i++)
        dbgpl(NULL, "After bubble sortstart_addr_table[%d] = %d",i, start_addr_table[i]);
#endif

    ptr1 = 1;
    do {
        start = (( (*(fig0_1_buff+ptr1)&0x3) << 8) + (*(fig0_1_buff+ptr1+1)));
        dbgpl(NULL, "---------------- subchanid %d start_address=%d",( (*(fig0_1_buff+ptr1) & 252) >> 2), start);

        /* take the required index in ch->u.dab.subChannel[sub_index] of the current subchannel */
        for(i=0 ; i < sub; i++) {
            if(start_addr_table[i] == start) {
                dbgpl(NULL, "start %d at position = %d",start,i);
                sub_index = i;
                break;
            }
        }

        ch->u.dab.subChannel[sub_index].start_address = start;
        ch->u.dab.subChannel[sub_index].subChid  = ( (*(fig0_1_buff+ptr1) & 252) >> 2);

        if (*(fig0_1_buff+ptr1+2)&0x80) {
            ch->u.dab.subChannel[sub_index].option           = ( (*(fig0_1_buff+ptr1+2) & 112) >> 4);
            ch->u.dab.subChannel[sub_index].protection_level = ( (*(fig0_1_buff+ptr1+2) &  12) >> 2);
            ch->u.dab.subChannel[sub_index].sub_size             = ( (*(fig0_1_buff+ptr1+2) &   3) << 8) + *(fig0_1_buff+ptr1+3);
            ch->u.dab.subChannel[sub_index].form             = 1;
            ptr1+= 4;
        } else {
            ch->u.dab.subChannel[sub_index].table_index  = (*(fig0_1_buff+ptr1+2) & 63);
            ch->u.dab.subChannel[sub_index].form             = 0;
            ptr1+= 3;
        }

        /* TS sub channel analyse with fig0_2_buff, lookup subid over full fig0_2_buff in the case this fig02 does not store the sub_id in the same order */
        do {
            if((ch->u.dab.subChannel[sub_index].subChid == (((*(fig0_2_buff+ptr2+6) >> 2) & 0x3f))) && ((*(fig0_2_buff + ptr2 + 5) &0x80) >> 7 == 0)){
                dbgpl(NULL, "SubChanId: %d has been found in fig0_2",ch->u.dab.subChannel[sub_index].subChid);
                if(((*(fig0_2_buff+ptr2+5)&0x1f) == 24) && ((*(fig0_2_buff + ptr2 + 5) &0xc0) >> 6 == 1)) {
                    ch->u.dab.subChannel[sub_index].is_ts = 1;
                    dbgpl(NULL, "SubChanId: %d in fig0_2 is TS ! (DSCTy = %d)",(((*(fig0_2_buff+ptr2+6)) >> 2) & 0x3f), (*(fig0_2_buff+ptr2+5)&0x1f));
                } else {
                    ch->u.dab.subChannel[sub_index].is_ts = 0;
                    dbgpl(NULL, "SubChanId: %d in fig0_2 is not TS ! (DSCTy = %d)",(((*(fig0_2_buff+ptr2+6)) >> 2) & 0x3f), (*(fig0_2_buff+ptr2+5)&0x1f));
                }
                break;
            }
            /* else
                dbgpl(NULL, "SubChanId: %d was not found in fig0_2",ch->u.dab.subChannel[sub_index].subChid); */

            ptr2+=7;
        } while(ptr2 < *fig0_2_buff);
        ptr2 = 1;

        ch->u.dab.subChannel[sub_index].addr_URAM = 0;

/* for test only
        ch->u.dab.cif_occurence_change = 127;
        if(ch->u.dab.subChannel[sub_index].is_ts && ch->u.dab.subChannel[sub_index].subChid == 6)
            ch->u.dab.subChannel[sub_index].requested_fec = 1;
*/

    } while (ptr1 < *fig0_1_buff);

    ch->u.dab.nb_active_sub = sub;
    dbgpl(NULL, "nb sub channel in the FIC = %d",ch->u.dab.nb_active_sub);

    return DIB_RETURN_SUCCESS;
#else
    return DIB_RETURN_SUCCESS;
#endif

}

static void init_dab_channel(struct dibChannel *ch)
{
    ch->u.dab.tmode = DAB_TMODE_AUTO;
    ch->u.dab.spectrum_inversion = 0;
    ch->bandwidth_kHz = 5000;
    ch->u.dab.nb_active_sub = 0;
    ch->u.dab.cif_occurence_change = -1;
    ch->u.dab.reconfig = 0;

    DibZeroMemory(ch->u.dab.subChannel, sizeof(ch->u.dab.subChannel));
}

#ifdef CONFIG_BUILD_HOST
#ifndef CONFIG_BUILD_BDA
/*
* The followinf function is doing almost the reverse job of set_fig01_02_to_channel.
* The following function creates an outup binary file doc/chan_desc/fic_gen.bin formated as if the MAC was given it after having decoded the real stream.
* This file is usefull to tune from the host env (without embedded) to allow channel decoder framde demultiplexer configuration.
* The binary out data are formating as follow: [size of FIG01 info][FIG01 subchan #0 bytes][FIG01 subchan #1 bytes]...[size of FIG02 info][FIG02 subchan #0 bytes]...
*/
void generate_binary_FIC_file_from_dabChannel(void) {

    struct dibChannel channel;
    int total_buff_size = 0, nb_data_byte_01 = 0, nb_data_byte_02 = 0, sub = 0;
    FILE * f = fopen("doc/chan_desc/fic_gen.bin", "w");
    uint8_t *buff, *pt;
    uint16_t buf, i;

    /* Initialise the dab channel subchannels definition as they are in the stream you want work on. */
    channel.u.dab.nb_active_sub = 1;

    channel.u.dab.subChannel[0].subChid          = 0;
    channel.u.dab.subChannel[0].start_address    = 0;
    channel.u.dab.subChannel[0].sub_size         = 288;
    channel.u.dab.subChannel[0].form             = 1;
    channel.u.dab.subChannel[0].table_index      = 0;
    channel.u.dab.subChannel[0].option           = 0;
    channel.u.dab.subChannel[0].protection_level = 2;
    channel.u.dab.subChannel[0].is_ts            = 1;
    channel.u.dab.subChannel[0].subc_new         = 0;

    channel.u.dab.subChannel[1].subChid          = 0;
    channel.u.dab.subChannel[1].start_address    = 450;
    channel.u.dab.subChannel[1].sub_size         = 408;
    channel.u.dab.subChannel[1].form             = 1; /* EEP = 1, UEP  = 0*/
    channel.u.dab.subChannel[1].table_index      = 0; /* need to be UEP */
    channel.u.dab.subChannel[1].option           = 0;
    channel.u.dab.subChannel[1].protection_level = 2;
    channel.u.dab.subChannel[1].is_ts            = 1;
    channel.u.dab.subChannel[1].subc_new         = 0;

    channel.u.dab.subChannel[2].subChid          = 3;
    channel.u.dab.subChannel[2].start_address    = 288;
    channel.u.dab.subChannel[2].sub_size         = 192;
    channel.u.dab.subChannel[2].form             = 0;
    channel.u.dab.subChannel[2].table_index      = 45;
    channel.u.dab.subChannel[2].option           = 0;
    channel.u.dab.subChannel[2].protection_level = 0;
    channel.u.dab.subChannel[2].is_ts            = 0;
    channel.u.dab.subChannel[2].subc_new         = 0;

    /* Compute len of full binary output buffer/file */
    for(sub = 0 ; sub < channel.u.dab.nb_active_sub ; sub++) {
        /* fig 01 len */
        if(channel.u.dab.subChannel[sub].form == 1)
            nb_data_byte_01+=4;
        else
            nb_data_byte_01+=3;

        nb_data_byte_02+=7; /* fig 02 len */
    }
    total_buff_size = (1 + nb_data_byte_01 + 1 + nb_data_byte_02); /* Additianal 2 bytes for size signalisation of FIG01 and FIG02 */
    sub =0;

    buff = malloc(total_buff_size * sizeof(uint8_t)); /* memory allocation */
    pt = buff;

    *pt = nb_data_byte_01; /* First byte is the len of the buffer */
    pt++;

    /* FIG 0/1 loop */
    while (sub < channel.u.dab.nb_active_sub) {
        buf = ((channel.u.dab.subChannel[sub].subChid & 0x3f) << 10) | (channel.u.dab.subChannel[sub].start_address & 0x3ff); /* first byte*/
        *pt = (buf >> 8)&0xff;
        pt++;
        *pt = buf&0xff;
        pt++;
        //printf("sub[%d] = subid = %d , byte1 = 0x%01x byte2 = 0x%01x\n", sub, channel.u.dab.subChannel[sub].subChid, (buf>>8)&0xff, buf&0xff);

        if(channel.u.dab.subChannel[sub].form == 1) {
            //printf("sub[%d] form = %d option = %d protect = %d size = %d\n", sub, channel.u.dab.subChannel[sub].form ,channel.u.dab.subChannel[sub].option, channel.u.dab.subChannel[sub].protection_level, channel.u.dab.subChannel[sub].sub_size);
            buf = ((channel.u.dab.subChannel[sub].form & 0x1) << 15) | ((channel.u.dab.subChannel[sub].option & 0x7) << 12) | ((channel.u.dab.subChannel[sub].protection_level & 0x3) << 10) | (channel.u.dab.subChannel[sub].sub_size &0x3ff);
            *pt = (buf >> 8)&0xff;
            pt++;
            *pt = buf&0xff;
            //printf("sub[%d] EEP subid = %d , byte3 = 0x%01x byte4 = 0x%01x\n", sub, channel.u.dab.subChannel[sub].subChid, (buf>>8)&0xff, buf&0xff);

        } else {
            *pt = ((channel.u.dab.subChannel[sub].form &0x1) << 7) | (0 << 6) | (channel.u.dab.subChannel[sub].table_index &0x3f);
            //printf("sub[%d] UEP  subid = %d , byte3 = 0x%01x\n", sub, channel.u.dab.subChannel[sub].subChid, *pt);
        }
        pt++;

        buf = 0;
        sub++;
    }

    /* FIG 0/2 loop */
    *pt = nb_data_byte_02; /* len of buf fig02 */
    //printf("FIG 02 size byte = 0x%01x\n", *pt);
    pt++;
    sub = 0;
    while (sub < channel.u.dab.nb_active_sub) {
        //printf(" id %d", channel.u.dab.subChannel[sub].subChid);

        for(i = 0 ; i< 5 ; i++) { /* write 0x0 on first 5 bytes */
            *pt = 0x0;
            pt++;
        }

        if(channel.u.dab.subChannel[sub].is_ts)
            *pt = (0x1 << 6) | 24; /* TMid = 01 & Dstcy = 24 */
        else
            *pt = 0;
        pt++;

        *pt = (channel.u.dab.subChannel[sub].subChid & 0x3f) << 2;
        pt++;
        sub++;
    }

    /* write the binary file now */
    dbgpl(NULL, "Writing output binary FIC file in doc/chan_desc/");
    fwrite(buff, sizeof(uint8_t), total_buff_size, f);
    fclose(f);
    free(buff);
}
#endif

static uint8_t digital_dab_channels_equal(struct dibChannel* ch1, struct dibChannel* ch2)
{
    uint8_t ret;
    int i;
    ret = ( (ch1->u.dab.tmode == ch2->u.dab.tmode) &&
            (ch1->u.dab.nb_active_sub == ch2->u.dab.nb_active_sub) &&
            (ch1->u.dab.spectrum_inversion  == ch2->u.dab.spectrum_inversion) &&
            (ch1->u.dab.cif_occurence_change == ch2->u.dab.cif_occurence_change) &&
            (ch1->u.dab.reconfig == ch2->u.dab.reconfig));

    for (i = 0; i < 63; i++) {
        ret += (ch1->u.dab. subChannel[i].subChid == ch2->u.dab.subChannel[i].subChid) &&
            (ch1->u.dab.subChannel[i].start_address   == ch2->u.dab.subChannel[i].start_address) &&
            (ch1->u.dab.subChannel[i].sub_size        == ch2->u.dab.subChannel[i].sub_size) &&
            (ch1->u.dab.subChannel[i].form            == ch2->u.dab.subChannel[i].form) &&
            (ch1->u.dab.subChannel[i].table_index     == ch2->u.dab.subChannel[i].table_index) &&
            (ch1->u.dab.subChannel[i].option          == ch2->u.dab.subChannel[i].option) &&
            (ch1->u.dab.subChannel[i].protection_level== ch2->u.dab.subChannel[i].protection_level) &&
            (ch1->u.dab.subChannel[i].is_ts           == ch2->u.dab.subChannel[i].is_ts) &&
            (ch1->u.dab.subChannel[i].subc_new        == ch2->u.dab.subChannel[i].subc_new) &&
            (ch1->u.dab.subChannel[i].addr_URAM       == ch2->u.dab.subChannel[i].addr_URAM);
    }
    return ret;
}
#endif

#endif

#ifdef CONFIG_STANDARD_ISDBT
static void init_isdbt_channel(struct dibChannel *ch)
{
    int i=0;

    ch->bandwidth_kHz = 6000;

    ch->u.isdbt.sb_mode             = 0;
    ch->u.isdbt.partial_reception   = 0;
    ch->u.isdbt.nfft                = FFT_AUTO;
    ch->u.isdbt.guard               = GUARD_INTERVAL_AUTO;
    ch->u.isdbt.spectrum_inversion  = 0;
    ch->u.isdbt.sb_conn_total_seg   = 13;
    ch->u.isdbt.sb_wanted_seg       = 0;
    ch->u.isdbt.sb_subchannel       = -1;

    for (i = 0; i < 3; i++) {
        ch->u.isdbt.layer[i].constellation = QAM_AUTO;
        ch->u.isdbt.layer[i].code_rate     = VIT_CODERATE_AUTO;
        ch->u.isdbt.layer[i].time_intlv    = -1;
        ch->u.isdbt.layer[i].nb_segments   = i == 0 ? 13 : 0;
    }
}

static uint8_t digital_isdbt_channels_equal(struct dibChannel* ch1, struct dibChannel* ch2)
{
    uint8_t ret;
    int i;
    ret = (ch1->u.isdbt.sb_mode == ch2->u.isdbt.sb_mode) && (ch1->u.isdbt.partial_reception == ch2->u.isdbt.partial_reception) &&
            (ch1->u.isdbt.nfft == ch2->u.isdbt.nfft) && (ch1->u.isdbt.guard == ch2->u.isdbt.guard) &&
            (ch1->u.isdbt.spectrum_inversion  == ch2->u.isdbt.spectrum_inversion) && (ch1->u.isdbt.sb_wanted_seg == ch2->u.isdbt.sb_wanted_seg ) &&
            (ch1->u.isdbt.sb_conn_total_seg == ch2->u.isdbt.sb_conn_total_seg) && (ch1->u.isdbt.sb_subchannel == ch2->u.isdbt.sb_subchannel);

    for (i = 0; i < 3; i++) {
        ret += (ch1->u.isdbt.layer[i].constellation == ch2->u.isdbt.layer[i].constellation) &&
                (ch1->u.isdbt.layer[i].time_intlv == ch2->u.isdbt.layer[i].time_intlv) &&
                (ch1->u.isdbt.layer[i].code_rate == ch2->u.isdbt.layer[i].code_rate) &&
                (ch1->u.isdbt.layer[i].nb_segments == ch2->u.isdbt.layer[i].nb_segments);
    }
    return ret;
}
#endif

void INIT_CHANNEL(struct dibChannel *ch, uint8_t type)
{
    channel_init(ch, type);
}

uint8_t DIGITAL_CHANNELS_EQUAL(struct dibChannel* ch1, struct dibChannel* ch2)
{
    if ((ch1->RF_kHz != ch2->RF_kHz) || (ch1->bandwidth_kHz != ch2->bandwidth_kHz) || (ch1->type != ch2->type))
        return 0;

    switch (ch1->type) {
#ifdef CONFIG_STANDARD_DVBT
        case STANDARD_DVBT: return digital_dvbt_channels_equal(ch1, ch2);
        case STANDARD_DVBSH: return digital_dvbsh_channels_equal(ch1, ch2);
#endif
#ifdef CONFIG_STANDARD_DAB
        case STANDARD_DAB: return digital_dab_channels_equal(ch1, ch2);
#endif
#ifdef CONFIG_STANDARD_ISDBT
        case STANDARD_ISDBT: return digital_isdbt_channels_equal(ch1, ch2);
#endif
        default: return 0;
    }
}

#ifdef CONFIG_BUILD_HOST
uint8_t channel_equal(struct dibChannel *ch1, struct dibChannel *ch2)
{
   if ((ch1->RF_kHz != ch2->RF_kHz) || (ch1->bandwidth_kHz != ch2->bandwidth_kHz) || (ch1->type != ch2->type))
      return 0;

   switch (ch1->type) {
#ifdef CONFIG_STANDARD_DVBT
      case STANDARD_DVBT: return digital_dvbt_channels_equal(ch1, ch2);
      case STANDARD_DVBSH: return digital_dvbsh_channels_equal(ch1, ch2);
#endif
#ifdef CONFIG_STANDARD_DAB
      case STANDARD_DAB: return digital_dab_channels_equal(ch1, ch2);
#endif
#ifdef CONFIG_STANDARD_ISDBT
      case STANDARD_ISDBT: return digital_isdbt_channels_equal(ch1, ch2);
#endif
      default: return 0;
   }
}
#endif

#ifdef CONFIG_STANDARD_CMMB
void init_cmmb_channel(struct dibChannel *ch) {
    int i;
    for (i=0; i<40; i++)
        ch->u.cmmb.ts_array[i].parameters.constellation = QAM_QPSK;
}
#endif

#ifdef CONFIG_STANDARD_ATSC
void init_atsc_channel(struct dibChannel *ch) {
    ch->bandwidth_kHz             = 6000;
	ch->u.atsc.modulation         = QAM_AUTO;
    ch->u.atsc.spectrum_inversion = 0;
}
#endif

void channel_init(struct dibChannel *ch, uint8_t type)
{
    int i;
    DibZeroMemory(ch, sizeof(struct dibChannel));
    ch->type   = type;

    ch->context.status               = CHANNEL_STATUS_PARAMETERS_UNKNOWN;
    ch->context.tune_time_estimation[TUNE_TIME_LOCKED] = channel_get_default_tune_time(ch->type);
    ch->context.tune_time_estimation[TUNE_TIME_DATA] = ch->context.tune_time_estimation[TUNE_TIME_LOCKED];

    for (i = 0; i< MAX_NB_OF_FRONTENDS_IN_CHAIN; i++)
        ch->chid_to_feid[i] = -1;

    switch (type) {
#ifdef CONFIG_STANDARD_DVBT
        case STANDARD_DVBT:  init_dvbt_channel(ch); break;
        case STANDARD_DVBSH:  init_dvbsh_channel(ch); break;
#endif
#ifdef CONFIG_STANDARD_DAB
        case STANDARD_DAB:   init_dab_channel(ch); break;
#endif
#ifdef CONFIG_STANDARD_ISDBT
        case STANDARD_ISDBT: init_isdbt_channel(ch); break;
#endif
#ifdef CONFIG_STANDARD_CMMB
        case STANDARD_CMMB: init_cmmb_channel(ch); break;
#endif
#ifdef CONFIG_STANDARD_ATSC
        case STANDARD_ATSC: init_atsc_channel(ch); break;
#endif
    }
}

uint16_t channel_get_default_tune_time(uint32_t type)
{
    switch(type) {
        default:
        case STANDARD_DVBT:
        case STANDARD_DVBSH:
            return 300; /* 30ms */
            break;
    }
}

uint8_t channel_get_status(struct dibChannel *ch)
{
    return ch->context.status;
}

uint32_t channel_get_tune_time_estimation(struct dibChannel *ch, int tune_time_type)
{
    return ch->context.tune_time_estimation[tune_time_type];
}

const uint16_t * channel_get_tps_info(struct dibChannel *ch)
{
    return ch->context.tps;
}

int channel_tps_available(struct dibChannel *ch)
{
    return ch->context.tps_available;
}

int channel_monitoring_available(struct dibChannel *ch, int id)
{
#if 0
    int monit_ready;
    uint32_t t = channel_frontend_monitoring(ch, id)->timestamp;
    monit_ready = (t != 0) && (systime() - t) < MAX_MONITORING_AGE;
    dbgpl(NULL, "monitoring %sready on frontend %d", monit_ready?" ":"not ", id);
    return monit_ready;
#else
	struct dibChannelMonitor * mon=channel_frontend_monitoring(ch, id);
	if (mon == NULL)
		return 0;
    return (mon->timestamp != 0) && (systime() - mon->timestamp) < MAX_MONITORING_AGE;
#endif
}

struct dibChannelFEInfo * channel_frontend_info(struct dibChannel *ch, int id)
{
    return &ch->fe_info[feid_to_chid(ch, id)];
}

struct dibChannelMonitor * channel_frontend_monitoring(struct dibChannel *ch, int id)
{
	int feid = feid_to_chid(ch, id);

	if (feid < 0)
		return NULL;
    return &ch->fe_monitor[feid];
}

int channel_standard_to_mode(struct dibChannel *ch)
{
    int mode;
    switch (ch->type) {
    case STANDARD_ISDBT  : mode = MODE_ISDBT;    break;
    case STANDARD_DAB    : mode = MODE_DAB_TDMB; break;
    case STANDARD_ATSC   : mode = MODE_ATSC;     break;
    case STANDARD_ANALOG : mode = MODE_ANALOG;   break;
    case STANDARD_DVBSH  : mode = MODE_DVBSH;    break;
    case STANDARD_CMMB   : mode = MODE_CMMB;     break;
    default :
    case STANDARD_DVBT   : mode = MODE_DVBT;     break;
    }
    return mode;
}

//#ifdef BUILD_LEGACY_DIB_ADAPTER
int channel_frequency_band(uint32_t freq_kHz)
{
    if (freq_kHz <= 170000)
        return BAND_CBAND;
    if (freq_kHz <= 115000)
        return BAND_FM;
    if (freq_kHz <= 380000)
        return BAND_VHF;
    if (freq_kHz <= 863000)
        return BAND_UHF;
    if (freq_kHz <= 2000000)
        return BAND_LBAND;
    return BAND_SBAND;
}
//#endif

#ifndef NO_DVBCHANNEL_BACKWARD_COMPAT
#ifdef CONFIG_STANDARD_DVBT
void CONVERT_DVB_TO_DIGITAL_CHANNEL(struct dibDVBChannel *dvbch, struct dibChannel *digitch)
{
    INIT_CHANNEL(digitch, STANDARD_DVBT);

    digitch->RF_kHz =(dvbch)->RF_kHz; \
	digitch->bandwidth_kHz =BW_INDEX_TO_KHZ((dvbch)->Bw);  \
	digitch->u.dvbt.nfft =(int8_t)(dvbch)->nfft; \
	digitch->u.dvbt.guard =(int8_t)(dvbch)->guard; \
	digitch->u.dvbt.spectrum_inversion =(int8_t)(dvbch)->invspec; \
	digitch->u.dvbt.constellation =(int8_t)(dvbch)->nqam; \
	digitch->u.dvbt.intlv_native =(int8_t)(dvbch)->intlv_native; \
	digitch->u.dvbt.hrch =(int8_t)(dvbch)->vit_hrch; \
	digitch->u.dvbt.select_hp =(int8_t)(dvbch)->vit_select_hp; \
	digitch->u.dvbt.alpha =(int8_t)(dvbch)->vit_alpha; \
	digitch->u.dvbt.code_rate_hp =(int8_t)(dvbch)->vit_code_rate_hp; \
	digitch->u.dvbt.code_rate_lp =(int8_t)(dvbch)->vit_code_rate_lp; \
}

void CONVERT_DIGITAL_TO_DVB_CHANNEL(struct dibChannel *digitch, struct dibDVBChannel *dvbch)
{
    dvbch->RF_kHz           = (digitch)->RF_kHz; \
	dvbch->Bw               = (int8_t)BW_KHZ_TO_INDEX((digitch)->bandwidth_kHz); \
	dvbch->nfft             = (digitch)->u.dvbt.nfft; \
	dvbch->guard            = (digitch)->u.dvbt.guard; \
	dvbch->invspec          = (digitch)->u.dvbt.spectrum_inversion; \
	dvbch->nqam             = (digitch)->u.dvbt.constellation; \
	dvbch->intlv_native     = (digitch)->u.dvbt.intlv_native; \
	dvbch->vit_hrch         = (digitch)->u.dvbt.hrch; \
	dvbch->vit_select_hp    = (digitch)->u.dvbt.select_hp; \
	dvbch->vit_alpha        = (digitch)->u.dvbt.alpha; \
	dvbch->vit_code_rate_hp = (digitch)->u.dvbt.code_rate_hp; \
	dvbch->vit_code_rate_lp = (digitch)->u.dvbt.code_rate_lp; \
}
#endif
#endif
