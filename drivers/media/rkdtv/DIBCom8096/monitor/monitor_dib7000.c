#include <adapter/component_monitor.h>
#include <monitor/monitor.h>

#if 0
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

#define val(name) { DBG("%-30s\n",name); }

#ifndef NO_DVBCHANNEL_BACKWARD_COMPAT
void dump_dvb_channel(struct dibDVBChannel *cd)
{
	switch (cd->nfft) {
		case 1:  DBG("8K   "); break;
		case 2:  DBG("4K   "); break;
		case 0:  DBG("2K   "); break;
		default: DBG("UNK  "); break;
	}

	DBG("1/%-2i  ", 32 / (1 << cd->guard));

	switch (cd->nqam) {
		case 0:  DBG("QPSK  "); break;
		case 1:  DBG("16QAM "); break;
		case 2:  DBG("64QAM "); break;
		default: DBG("UNK   "); break;
	}

	DBG("%i    ", cd->vit_hrch);
	DBG("%i    ", cd->vit_alpha);
	DBG("%i/%i ", cd->vit_code_rate_hp, cd->vit_code_rate_hp + 1);
	DBG("%i/%i ", cd->vit_code_rate_lp, cd->vit_code_rate_lp + 1);
	DBG("%s"    , cd->intlv_native ? "NATIVE" : "EXTENDED");
}
#endif

#ifdef CONFIG_STANDARD_DVBT
void dump_dvb_digital_channel(struct dibChannel *cd)
{
	switch (cd->u.dvbt.nfft) {
		case 1:  DBG("8K   "); break;
		case 2:  DBG("4K   "); break;
		case 0:  DBG("2K   "); break;
		default: DBG("UNK  "); break;
	}

	DBG("1/%-2i  ", 32 / (1 << cd->u.dvbt.guard));

	switch (cd->u.dvbt.constellation) {
		case 0:  DBG("QPSK  "); break;
		case 1:  DBG("16QAM "); break;
		case 2:  DBG("64QAM "); break;
		default: DBG("UNK   "); break;
	}

	DBG("%i    ", cd->u.dvbt.hrch);
	DBG("%i    ", cd->u.dvbt.alpha);
	DBG("%i/%i ", cd->u.dvbt.code_rate_hp, cd->u.dvbt.code_rate_hp + 1);
	DBG("%i/%i ", cd->u.dvbt.code_rate_lp, cd->u.dvbt.code_rate_lp + 1);
	DBG("%s"    , cd->u.dvbt.intlv_native ? "NATIVE" : "EXTENDED");
}

void dump_dvbsh_digital_channel(struct dibChannel *cd)
{
	switch (cd->u.dvbsh.dvb_common.nfft) {
		case 1:  DBG("8K   "); break;
		case 2:  DBG("4K   "); break;
		case 0:  DBG("2K   "); break;
		case 3:  DBG("1K   "); break;
		default: DBG("UNK  "); break;
	}

	DBG("1/%-2i  ", 32 / (1 << cd->u.dvbsh.dvb_common.guard));

	switch (cd->u.dvbsh.dvb_common.constellation) {
		case 0:  DBG("QPSK  "); break;
		case 1:  DBG("16QAM "); break;
		default: DBG("UNK   "); break;
	}

	DBG("%i    ", cd->u.dvbsh.dvb_common.hrch);
	DBG("%i    ", cd->u.dvbsh.dvb_common.alpha);

    DBG(" %s   ", DVBSH_CODERATE_TO_STRING(cd->u.dvbsh.dvb_common.code_rate_hp));
    DBG("%s"    , DVBSH_CODERATE_TO_STRING(cd->u.dvbsh.dvb_common.code_rate_lp));

}
#endif

#ifdef CONFIG_STANDARD_ISDBT
void dump_isdbt_channel(struct dibChannel *cd)
{
    uint8_t i;

    switch (cd->u.isdbt.nfft) {
		case 1:  DBG("8K     "); break;
		case 2:  DBG("4K     "); break;
		case 0:  DBG("2K     "); break;
		default: DBG("UNK    "); break;
	}

	DBG("1/%-2i      ", 32 / (1 << cd->u.isdbt.guard));
    DBG("%i       ", cd->u.isdbt.sb_mode);
    DBG("%i       ", cd->u.isdbt.partial_reception);
	for (i = 0; i<3; i++)
    {
        DBG("%.2i    ", cd->u.isdbt.layer[i].nb_segments);

        switch (cd->u.isdbt.layer[i].constellation) {
		    case 0:  DBG("QPSK   "); break;
		    case 1:  DBG("16QAM  "); break;
		    case 2:  DBG("64QAM  "); break;
            case 3 : DBG("DQPSK  "); break;
		    default: DBG("UNK    "); break;
        }
        DBG("%i/%i    ", cd->u.isdbt.layer[i].code_rate, cd->u.isdbt.layer[i].code_rate + 1);
        DBG("%i         ", cd->u.isdbt.layer[i].time_intlv);
	}
}
#endif

int channel_decoder_have_standard(struct dibChannelDecoderMonitor m[], int num, int st)
{
    do {
        num--;
        if (m[num].type == st)
            return 1;
    } while (num);
    return 0;
}

void channel_decoder_print_monitor(struct dibChannelDecoderMonitor m[], int num)
{
    int k=0;

    if (channel_decoder_have_standard(m, num, STANDARD_DVBSH)) {
        DBG("\nSH info :          SH dem lock  per            iter        CRC header ok  tc_punct_id   Select_hp     nFFT    nqam     dvb_sh \n");
        DBG("                   | SH lock    |       PS     |  overflow | pha_sh_frame |  pading_err | common_mult | hrch  | mux    |\n");
        for (k = 0; k < num; k++) {
            if (m[k].type == STANDARD_DVBSH)
                DBG("Turbo decoder %d :  %d %d       %7.5f   %-5d  %-2d %d        %d %d            %-2d %-2d         %d %d           %d %d     %d 0x%04x %d\n", k, m[k].u.sh.misc, m[k].u.sh.sh_lock, m[k].u.sh.sh_per, m[k].u.sh.sh_PS, m[k].u.sh.sh_nb_iter, m[k].u.sh.sh_overflow, m[k].u.sh.sh_crc_header_ok, m[k].u.sh.sh_pha_sh_frame, m[k].u.sh.sh_tc_punct_id, m[k].u.sh.sh_padding_err, m[k].u.sh.sh_select_hp, m[k].u.sh.sh_common_mult , m[k].u.sh.sh_nfft , m[k].u.sh.sh_hrch,  m[k].u.sh.sh_Nqam, m[k].u.sh.mux, m[k].u.sh.dvb_sh);
        }
    }

    if (channel_decoder_have_standard(m, num, STANDARD_CMMB)) {
        DBG("\nCMMB info :         slot num       per       iter/iter max     syndrome_first_iter    overflow_count   bloc_error      error flag     RS table count  RS table error count      RS per\n");
        for (k = 0; k < num; k++) {
            if (m[k].type == STANDARD_CMMB)
                DBG("CMMB decoder %d :       %2d        %7.5f       %2d/%2d                     %d               %5d              %d                %d           %d                   %d               %f\n", k, m[k].u.cmmb.sl_num, m[k].u.cmmb.per, m[k].u.cmmb.nb_iteration, m[k].u.cmmb.nb_max_iteration, m[k].u.cmmb.syndrome_first_iter, m[k].u.cmmb.overflow_cnt, m[k].u.cmmb.bloc_error, m[k].u.cmmb.err_flag, m[k].u.cmmb.rs_table_count, m[k].u.cmmb.rs_table_error_count,  m[k].u.cmmb.rs_table_count ? (double)m[k].u.cmmb.rs_table_error_count / (double)m[k].u.cmmb.rs_table_count : 0);
        }
    }

    if (channel_decoder_have_standard(m, num, STANDARD_DVBT)) {
        DBG("\n");
        DBG("\n");
        val("Viterbi syndrome");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.dvbt.viterbi_syndrome);
        DBG("\n");

        val("packet error count");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.dvbt.PacketErrors);
        DBG("\n");

        val("packet error cumul");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.dvbt.PacketErrorCount);
        DBG("\n");

        val("Bit Error Rate");
        for (k = 0; k < num; k++)
             DBG("%-15g", m[k].u.dvbt.ber);
        DBG("\n");

        val("Quasi Error Free");
        for (k = 0; k < num; k++)
            DBG("%-15s", m[k].u.dvbt.ber < 2e-4 && m[k].u.dvbt.PacketErrors == 0 ? "OK" : "*** NOT OK ***");
        DBG("\n");

        DBG("\nchannel decoder locks:     fec_frm \n");
        DBG("                       vit | fec_mpeg\n");
        for (k = 0; k < num; k++) {
            DBG("channel decoder %2d:     %d  %d  %d\n",k,
                    m[k].u.dvbt.locks.vit, m[k].u.dvbt.locks.fec_frm, m[k].u.dvbt.locks.fec_mpeg/*, m[k].u.dvbt.viterbi_syndrome, m[k].u.dvbt.PacketErrors, m[k].u.dvbt.PacketErrorCount, m[k].u.dvbt.ber*/);
            DBG("\n");
        }
    }

    if (channel_decoder_have_standard(m, num, STANDARD_ISDBT)) {
        DBG("\n");
        DBG("\n");
        val("Viterbi syndrome layer a");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.isdbt.viterbi_syndrome);
        DBG("\n");
        val("Viterbi syndrome layer b");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.isdbt.viterbi_syndrome_b);
        DBG("\n");
        val("Viterbi syndrome layer c");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.isdbt.viterbi_syndrome_c);
        DBG("\n");

        val("packet error count A");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.isdbt.PacketErrors_A);
        DBG("\n");
        val("packet error count B");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.isdbt.PacketErrors_B);
        DBG("\n");
        val("packet error count C");
        for (k = 0; k < num; k++)
            DBG("%-15i", m[k].u.isdbt.PacketErrors_C);
        DBG("\n");


        val("packet error cumul");
        for (k = 0; k < num; k++)
        DBG("%-15i", m[k].u.isdbt.PacketErrorCount);
        DBG("\n");

        val("Bit Error Rate A");
        for (k = 0; k < num; k++)
            DBG("%-15g", m[k].u.isdbt.berA);
        DBG("\n");
        val("Bit Error Rate B");
        for (k = 0; k < num; k++)
            DBG("%-15g", m[k].u.isdbt.berB);
        DBG("\n");
        val("Bit Error Rate C");
        for (k = 0; k < num; k++)
            DBG("%-15g", m[k].u.isdbt.berC);
        DBG("\n");

        /*val("Quasi Error Free");
        for (k = 0; k < num; k++)
            DBG("%-15s", m[k].u.isdbt.ber < 2e-4 && m[k].u.isdbt.PacketErrors == 0 ? "OK" : "*** NOT OK ***");
        DBG("\n");*/

        DBG("\nchannel decoder locks:            fec_mpeg(a b c) \n");
        DBG("                       vit(a b c) | \n");
        for (k = 0; k < num; k++) {
            DBG("channel decoder %2d:    %d %d %d       %d %d %d\n",k,
                    m[k].u.dvbt.locks.vit, m[k].u.isdbt.locks.vit_b, m[k].u.isdbt.locks.vit_c, m[k].u.isdbt.locks.fec_mpeg, m[k].u.isdbt.locks.fec_mpeg_b, m[k].u.isdbt.locks.fec_mpeg_c);
            DBG("\n");
        }
    }

    if (channel_decoder_have_standard(m, num, STANDARD_DAB)) {

        DBG("Viterbi syndrome : GLOBAL | FIC | MSC | SubId -> syndrome | ber \n");

        for (k = 0; k < num; k++) {
            if (m[k].type == STANDARD_DAB) {
                DBG("                   %4d   %4d  %4d    %2d    -> %4d       %f\n", m[k].u.dab.syn,m[k].u.dab.syn_fic,m[k].u.dab.syn_msc, m[k].u.dab.subc_id, m[k].u.dab.syn_subc, m[k].u.dab.ber/1e8);
            }
        }
        DBG("\nSubcId | Start addr | Size | Prev addr | Prev Size | Form | Table Index | L1punct | L2 punct |  PI1  |  PI2  | I  \n");

        for (k = 0; k < num; k++) {
            if (m[k].type == STANDARD_DAB) {
                DBG("    %2d | %3d        | %3d  | %3d       | %3d       | %1d    | %2d          | %5d   |    %4d  |%4d   |%4d   | %4d  \n", m[k].u.dab.subc_id, m[k].u.dab.subc_staddr, m[k].u.dab.subc_sz, m[k].u.dab.subc_staddr_prev, m[k].u.dab.subc_sz_prev, m[k].u.dab.subc_form, m[k].u.dab.subc_index, m[k].u.dab.subc_L1punct, m[k].u.dab.subc_L2punct, m[k].u.dab.subc_PI1, m[k].u.dab.subc_PI2, m[k].u.dab.subc_I);
            }
        }
        DBG("\n");
        DBG("MPEG_lock     DMB_Lock_frm  Fec_Lock_frm     TimeDtlv_overflow  IsMSC          Recfg      Fec_mpfail\n");
        DBG("| Fec_BER_RS  | Fec_mpcor   | Fec_ber_rscnt  | cif_counter      | Fec_rsnperr  | subc new | Fec_mpfail_cnt\n");


        for (k = 0; k < num; k++) {
            if (m[k].type == STANDARD_DAB) {
                DBG("%1d %7d     %1d %2d          %1d %3d            %1d %3d              %1d %7d      %1d %1d        %1d %7d\n", m[k].u.dab.fec_lock_mpeg, m[k].u.dab.fec_ber_rs, m[k].u.dab.dmb_lock_frm, m[k].u.dab.fec_mpcor, m[k].u.dab.fec_lock_frm, m[k].u.dab.fec_ber_rscnt, m[k].u.dab.TimeDtlv_overflow, m[k].u.dab.cifcounter, m[k].u.dab.isMSC,  m[k].u.dab.fec_ber_rsnperr, m[k].u.dab.recfg, m[k].u.dab.subc_new, m[k].u.dab.fec_mpfail, m[k].u.dab.fec_mpfail_cnt);
            }
        }
    }
}

void mac_print_monitor(struct dibMacMonitor m_mac[], struct dibDemodMonitor m_demod[], int num)
{
    int k;

        DBG("\nMAC info :  ADsample lock\n");
        DBG("            | \n");
        for (k = 0; k < num; k++) {
            DBG("Mac %d :     %d\n", k, m_mac[k].ad_sample_lock);
        }
}

int demods_have_standard(struct dibDemodMonitor m[], int num, int st)
{
    do {
        num--;
        if (m[num].cur_digital_channel.type == st)
            return 1;
    } while (num);
    return 0;
}



void dib7000_print_monitor(struct dibDemodMonitor m[], struct dibDemodStatistic s[], int stat_count, int num)
{
    int k,i;

#ifdef DIBCOM_EXTENDED_MONITORING
    if(!demods_have_standard(m, num, STANDARD_DAB)) {
        val("I adc power dBVrms");
        for (k = 0; k < num; k++)
            DBG("%-15.2f", m[k].I_adc_power);
        DBG("\n");

        val("Q adc power dBVrms");
        for (k = 0; k < num; k++)
            DBG("%-15.2f", m[k].Q_adc_power);
        DBG("\n");
    }

    val("DC offset on I");
    for (k = 0; k < num; k++)
        DBG("%-15.2f", m[k].iqc_dcoff_i);
    DBG("\n");

    val("DC offset on Q");
    for (k = 0; k < num; k++)
	DBG("%-15.2f", m[k].iqc_dcoff_q);
    DBG("\n");

    val("IQ gain mismatch (dB)");
    for (k = 0; k < num; k++)
        DBG("%-15.2g", m[k].iq_gain_mismatch);
    DBG("\n");

    val("IQ phase mismatch (degree)");
    for (k = 0; k < num; k++)
        DBG("%-15.2g", m[k].iq_phase_mismatch);
    DBG("\n");

    if (m[0].n_adc_mode <= 1 && !m[0].adc_monitor_only) {
        val("rf power dBm");
        for (k = 0; k < num; k++)
            DBG("%-15.2g", m[k].rf_power_dbm);
        DBG("\n");

        val("WBD:");
        for (k = 0; k < num; k++)
            DBG("%-15.3g", m[k].agc_wbd);
        DBG("\n");

        val("AGCglobal (norm)");
        for (k = 0; k < num; k++)
            DBG("%-15.3g", m[k].rf_power/65536.0);
        DBG("\n");

        val("AGC1 (norm)");
        for (k = 0; k < num; k++)
            DBG("%-15.3g", m[k].agc1);
        DBG("\n");

        val("AGC2 (norm)");
        for (k = 0; k < num; k++)
            DBG("%-15.3g", m[k].agc2);
        DBG("\n");
        val("AGC Split");
        for (k = 0; k < num; k++)
            DBG("%-15d", m[k].agc_split);
        DBG("\n");

        DBG("\n");
    }
#ifdef CONFIG_STANDARD_DAB
    if(demods_have_standard(m, num, STANDARD_DAB)) {
        val("AGC power");
        for (k = 0; k < num; k++)
            DBG("%-15.2f", m[k].agc_db);
        DBG("\n");

        val("Power estimation dB");
        DBG("%-15.2g", m[0].equal_signal_dB);
        DBG("\n");

        val("Demod clock Hz");
        for (k = 0; k < num; k++)
            DBG("%d", m[k].internal_clk);
        DBG("\n");

        val("small/large freq offset");
        for (k = 0; k < num; k++)
            DBG(" %-4.3g | %3d", m[k].dab.small_freqoff,m[k].dab.large_freqoff);
        DBG("\n");
    }
#endif
        val("Carrier offset in Khz");
        for (k = 0; k < num; k++)
            DBG("%-15.3g", m[k].carrier_offset);
        DBG("\n");

    if (m[0].adc_monitor_only) {
        val("ADC LNPower");
        for (k = 0; k < num; k++)
            DBG("%-15.3g", m[k].adc_lnpower);
        DBG("\n");
    }

    if (m[0].n_adc_mode == 0) {
        val("Timing offset in ppm");
        for (k = 0; k < num; k++)
            DBG("%-15.4g", m[k].timing_offset_ppm);
        DBG("\n");

#ifdef CONFIG_STANDARD_ISDBT
        if (demods_have_standard(m, num, STANDARD_ISDBT)) {
            if (!m[0].do_not_display_chandec) {
                val("Viterbi syndrome layer a");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome);
                DBG("\n");
                val("Viterbi syndrome layer b");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome_b);
                DBG("\n");
                val("Viterbi syndrome layer c");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome_c);
                DBG("\n");
            }

        }
#endif
#ifdef CONFIG_STANDARD_DVBT
        if (demods_have_standard(m, num, STANDARD_DVBT)) {
            if (!m[0].do_not_display_chandec) {
                val("Viterbi syndrome");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome);
                DBG("\n");
            }
        }
#endif
#ifdef CONFIG_STANDARD_DAB
        if (demods_have_standard(m, num, STANDARD_DAB)) {
            if (!m[0].do_not_display_chandec) {
                val("\nTransmission Mode : ");
                switch (m[0].dab.tmode) {
                    case 0 : val("I"); break;
                    case 1 : val("II"); break;
                    case 2 : val("III"); break;
                    case 3 : val("IV"); break;
                }
                DBG("\n");

                val("Viterbi syndrome GLOBAL | FIC | MSC : ");
                DBG("  %d | %d | %d ", m[0].dab.syn,m[0].dab.syn_fic,m[0].dab.syn_msc);
                DBG("\n");

                val("Viterbi syndrome SubId");
                for (k = 0; k < num; k++)
                    DBG("%d syndrome = %d", m[0].dab.syn_subid, m[0].dab.syn_subc);
                DBG("\n");
            }
        }
#endif
        val("C/N (dB)");
        for (k = 0; k < num; k++)
            DBG("%-15.4g", m[k].CoN);
        DBG("\n");

        if ( demods_have_standard(m, num, STANDARD_DVBT) || demods_have_standard(m, num, STANDARD_ISDBT) || demods_have_standard(m, num, STANDARD_DAB) ) {

#ifdef CONFIG_STANDARD_ISDBT
            if ((demods_have_standard(m, num, STANDARD_ISDBT)) && (m[0].can_display_ber_several_layers == 1))
            {
                if (!m[0].do_not_display_chandec) {
                    val("packet error count A");
                    for (k = 0; k < num; k++)
                        DBG("%-15i", m[k].PacketErrors_A);
                    DBG("\n");
                    val("packet error count B");
                    for (k = 0; k < num; k++)
                        DBG("%-15i", m[k].PacketErrors_B);
                    DBG("\n");
                    val("packet error count C");
                    for (k = 0; k < num; k++)
                        DBG("%-15i", m[k].PacketErrors_C);
                    DBG("\n");
                }
            }
            else
#endif
            if (!m[0].do_not_display_chandec) {

                val("packet error count");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].PacketErrors);
                DBG("\n");
            }

            if (!(demods_have_standard(m, num, STANDARD_ISDBT)) || (m[0].can_display_ber_several_layers != 1))
            {
                if (!m[0].do_not_display_chandec) {
                    val("packet error cumul");
                    for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].PacketErrorCount);
                    DBG("\n");
                }
            }

#ifdef CONFIG_STANDARD_ISDBT
            if ((demods_have_standard(m, num, STANDARD_ISDBT)) && (m[0].can_display_ber_several_layers == 1) && (!m[0].do_not_display_chandec))
            {
                val("Bit Error Rate A");
                for (k = 0; k < num; k++)
                    DBG("%-15g", m[k].berA);
                DBG("\n");
                val("Bit Error Rate B");
                for (k = 0; k < num; k++)
                    DBG("%-15g", m[k].berB);
                DBG("\n");
                val("Bit Error Rate C");
                for (k = 0; k < num; k++)
                    DBG("%-15g", m[k].berC);
                DBG("\n");
            }
            else
#endif
            if (!m[0].do_not_display_chandec) {

                val("Bit Error Rate");
                for (k = 0; k < num; k++)
                    DBG("%-15g", m[k].ber);
                DBG("\n");
            }
        }

        if ( (demods_have_standard(m, num, STANDARD_DVBT)) || (demods_have_standard(m, num, STANDARD_ISDBT))) {
            if (!m[0].do_not_display_chandec) {
                val("MER");
                for (k = 0; k < num; k++)
                    DBG("%-15g", m[k].mer);
                DBG("\n");

                val("Quasi Error Free");
                for (k = 0; k < num; k++)
                    DBG("%-15s", m[k].ber < 2e-4 && m[k].PacketErrors == 0 ? "OK" : "*** NOT OK ***");
                DBG("\n");
            }
        } else if (demods_have_standard(m, num, STANDARD_DAB)) {
            val("Quasi Error Free : ber<1e-4");
            DBG("%-15s", m[0].ber < 1e-4 && m[0].PacketErrors == 0 ? "OK" : "*** NOT OK ***");
            DBG("\n");
        }
#ifdef CONFIG_STANDARD_ISDBT
            if (demods_have_standard(m, num, STANDARD_ISDBT)) {
                DBG("                                            |            layer a          |           layer b           |          layer c            | \n");
                DBG("demod channel: nfft  guard  sb_mode  p_rcpt   nb_seg  modu  crate t_intlv   nb_seg  modu  crate t_intlv   nb_seg  modu  crate t_intlv\n");
                for (k=0; k < num; k++) {
                    DBG("demod %2d     : ",k);
                    dump_isdbt_channel(&m[k].cur_digital_channel);
                    DBG("\n");
                }
            }
#endif
#ifdef CONFIG_STANDARD_DVBT
            if (demods_have_standard(m, num, STANDARD_DVBT)) {
                DBG("demod channel: nfft guard nqam  hrch alpha crhp crlp intlv\n");
                for (k=0; k < num; k++) {
                    DBG("demod %2d:      ",k);
                    dump_dvb_digital_channel(&m[k].cur_digital_channel);
                    DBG("\n");
                }
            } else if (demods_have_standard(m, num, STANDARD_DVBSH)) {
                DBG("demod channel: nfft guard nqam  hrch alpha crhp      crlp\n");
                for (k=0; k < num; k++) {
                    DBG("demod %2d:      ",k);
                    dump_dvbsh_digital_channel(&m[k].cur_digital_channel);
                    DBG("\n");
                }
            }
#endif
        DBG("\n");
#ifdef CONFIG_STANDARD_ISDBT
        if (demods_have_standard(m, num, STANDARD_ISDBT)) {
            if (!m[0].do_not_display_chandec) {
                DBG("demod locks:  agc    coff        lmod4  equal      tmcc_sync   dvsy\n");
                DBG("              | corm | coff_cpil | pha3 | tmcc_dec | tmcc_data |  vit(a,b,c)  fec_mpeg(a,b,c) \n");
                for (k = 0; k < num; k++)
                    DBG("demod %2d:     %d %d    %d %d         %d %d    %d %d        %d %d         %d  %d%d%d         %d%d%d\n",k,
                            m[k].locks.agc, m[k].locks.corm, m[k].locks.coff, m[k].locks.coff_cpil, m[k].locks.lmod4, m[k].locks.pha3, m[k].locks.equal,m[k].locks.tmcc_dec, m[k].locks.tmcc_sync, m[k].locks.tmcc_data, m[k].locks.dvsy, m[k].locks.vit, m[k].locks.vit_b,m[k].locks.vit_c, m[k].locks.fec_mpeg,m[k].locks.fec_mpeg_b,m[k].locks.fec_mpeg_c);
                DBG("\n");
            }
        }
#endif
#ifdef CONFIG_STANDARD_DVBT
        if (demods_have_standard(m, num, STANDARD_DVBT)) {
            for (k = 0; k < num; k++) {
                if (!m[k].do_not_display_chandec) {
                    DBG("demod locks:  agc    coff        lmod4  equal fec_frm    tps_dec    tps_data     dvsy\n");
                    DBG("              | corm | coff_cpil | pha3 | vit | fec_mpeg | tps_sync | tps_cellid |\n");
                    break;
                }
            }
            for (k = 0; k < num; k++) {
                if (!m[k].do_not_display_chandec) {
                    DBG("demod %2d:     %d %d    %d %d         %d %d    %d %d   %d %d        %d %d        %d %d          %d\n",k,
                            m[k].locks.agc, m[k].locks.corm, m[k].locks.coff, m[k].locks.coff_cpil, m[k].locks.lmod4, m[k].locks.pha3, m[k].locks.equal, m[k].locks.vit, m[k].locks.fec_frm, m[k].locks.fec_mpeg, m[k].locks.tps_dec, m[k].locks.tps_sync, m[k].locks.tps_data, m[k].locks.tps_cellid, m[k].locks.dvsy);
                    DBG("\n");
                }
            }
        }
        else if (demods_have_standard(m, num, STANDARD_DVBSH)) {
            DBG("demod locks:  agc    coff        lmod4  equal tps_dec    tps_data     dvsy\n");
            DBG("              | corm | coff_cpil | pha3 |     | tps_sync | tps_cellid |\n");
            for (k = 0; k < num; k++)
                DBG("demod %2d:     %d %d    %d %d         %d %d    %d     %d %d        %d %d          %d\n",k,
                        m[k].locks.agc, m[k].locks.corm, m[k].locks.coff, m[k].locks.coff_cpil, m[k].locks.lmod4, m[k].locks.pha3, m[k].locks.equal,  m[k].locks.tps_dec, m[k].locks.tps_sync, m[k].locks.tps_data, m[k].locks.tps_cellid, m[k].locks.dvsy);
            DBG("\n");
        }
#endif
#ifdef CONFIG_STANDARD_DAB
         if (demods_have_standard(m, num, STANDARD_DAB)) {
            if (!m[0].do_not_display_chandec) {
                val("Nonlin quant | reverse quant | full internal : ");
                for (k = 0; k < num; k++)
                    DBG("   %dbit | %dbit | %d", m[k].dab.nb_bit_quant, m[k].dab.nb_bit_reverse_quant,m[k].dab.tdint_full_internal);
                DBG("\n");

                DBG("DAB locks:   dmb_lock_frm  ndec_tmode    fec_mpeg   dabcoff_lock\n");
                DBG("             | corm_dmb    | ndec_frame  | fec_frm  |\n");
                for (k = 0; k < num; k++)
                    DBG("demod %2d:    %d %d           %d %d           %d %d        %d\n",k,m[k].dab.dmb_lock_frm, m[k].locks.corm_lock_dmb, m[k].locks.ndec_tmode_lock, m[k].locks.ndec_frame_lock ,m[k].locks.fec_mpeg, m[k].locks.fec_frm, m[k].locks.dabcoff_lock);

                DBG("SubChan :     subc_id  recfg      cifcounter  subc_sz         sz_prev   index      L2punct    PI2\n");
                DBG("              |  isMSC | subc_new |  staddr   |   staddr_prev |   form  | L1punct  |   PI1    | I\n");

                for (k = 0; k < num; k++)
                DBG("demod %2d:    %2d  %d     %d %d       %3d %d       %d  %d          %d  %d     %d %d   %d  %d      %d %d\n",k,m[k].dab.subc_id, m[k].dab.isMSC, m[k].dab.recfg, m[k].dab.subc_new, m[k].dab.cifcounter, m[k].dab.subc_staddr, m[k].dab.subc_sz, m[k].dab.subc_staddr_prev, m[k].dab.subc_sz_prev, m[k].dab.subc_form, m[k].dab.subc_index, m[k].dab.subc_L1punct, m[k].dab.subc_L2punct, m[k].dab.subc_PI1, m[k].dab.subc_PI2, m[k].dab.subc_I);

                DBG("\nFEC id    ");
                for (i = 0 ; i < 12 ; i++)
                    DBG("%2d  ",i);
                DBG("\n");

                for (k = 0; k < num; k++) {
                    DBG("%d STATE:  ",k);
                    for (i = 0 ; i < 12 ; i++)
                        DBG("%2d  ",(m[k].dab.fec_state >> i)&0x1);
                    DBG("\n");

                    DBG("%d SubId:  ",k);
                    for (i = 0 ; i < 12 ; i++)
                        DBG("%2d  ",m[k].dab.fec_subid[i]);
                    DBG("\n\n");
                }
            } else {
                DBG("DAB locks:   signal_detect\n");
                for (k = 0; k < num; k++)
                    DBG("demod %2d:    %d\n",k, m[k].locks.coff);
            }

        }
#endif
        if (m[0].n_2dpa_monitoring) {
            val("mu_int/num_fir");
            for (k = 0; k < num; k++)
                DBG("%d/%d%15s", m[k].n_2dpa_mu_int, m[k].n_2dpa_num_fir, " ");
            DBG("\n");

            for (i = 0; i < 4; i++) {
                DBG("cti_def%d%20s",i," ");
                for (k = 0; k < num; k++)
                    DBG("%.2f (%5d)   ", m[k].cti_def[i], m[k].cti_def_raw[i]);
                DBG("\n");
            }
            DBG("\n");
        }

        val("Divin FIFO overflow");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].divin_fifo_overflow);
        DBG("\n");

        val("Divin error symbols");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].divin_nberr_sym);
        DBG("\n");

        val("Dvsy first arrived");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].dvsy_first_arrived);
        DBG("\n");

        val("Dvsy delay");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].dvsy_delay);
        DBG("\n");

        val("Divin OK (Way 0, 1)");
        for (k = 0; k < num; k++)
            DBG("%d,%d%12s",m[k].way0_ok,m[k].way1_ok,"");
        DBG("\n");

        val("Inversion"); //, DDS, (msb,lsb)");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].invspec);
        DBG("\n");

        val("p DDS Freq");
        for (k = 0; k < num; k++)
            DBG("%-15g",m[k].viewable_dds_freq);
        DBG("\n");

        if (stat_count > 0) {
            DBG("\nDiB7000 statistics each time after %d monitor iterations\n",stat_count);
            val("C/N (linear mean)");
            for (k = 0; k < num; k++)
                DBG("%-15g", s[k].CoN_lin_mean);
            DBG("\n");

            val("C/N (logarithmic mean)");
            for (k = 0; k < num; k++)
                DBG("%-15g", s[k].CoN_log_mean);
            DBG("\n");

            val("C/N (min)");
            for (k = 0; k < num; k++)
                DBG("%-15g", s[k].CoN_min);
            DBG("\n");

            val("C/N (max)");
            for (k = 0; k < num; k++)
                DBG("%-15g", s[k].CoN_max);
            DBG("\n\n");
        }
    }
#else
    if(!demods_have_standard(m, num, STANDARD_DAB)) {
        val("I adc power dBVrms");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].adc_power_i);
        DBG("\n");

        val("Q adc power dBVrms");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].adc_power_q);
        DBG("\n");
    }

    val("DC offset on I");
    for (k = 0; k < num; k++)
        DBG("0x%-13x", 0);
    DBG("\n");

    val("DC offset on Q");
    for (k = 0; k < num; k++)
	DBG("0x%-13x", 0);
    DBG("\n");

    val("IQ gain mismatch (dB)");
    for (k = 0; k < num; k++)
        DBG("0x%-13x", m[k].iq_misgain);
    DBG("\n");

    val("IQ phase mismatch (degree)");
    for (k = 0; k < num; k++)
        DBG("0x%-13x", m[k].iq_misphi);
    DBG("\n");

    if (m[0].n_adc_mode <= 1 && !m[0].adc_monitor_only) {
        val("rf power dBm");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].rf_power);
        DBG("\n");

        val("WBD:");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].agc_wbd_raw);
        DBG("\n");

        val("AGCglobal (norm)");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].rf_power);
        DBG("\n");

        val("AGC1 (norm)");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].agc1_raw);
        DBG("\n");

        val("AGC2 (norm)");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].agc2_raw);
        DBG("\n");
        val("AGC Split");
        for (k = 0; k < num; k++)
            DBG("%-15d", m[k].agc_split);
        DBG("\n");

        DBG("\n");
    }

#ifdef CONFIG_STANDARD_DAB
    if(demods_have_standard(m, num, STANDARD_DAB)) {
        val("AGC power");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].rf_power);
        DBG("\n");

        val("Power estimation dB");
        DBG("0x%-4x*2^0x%-4x", m[0].equal_signal_mant, m[0].equal_signal_exp);
        DBG("\n");

        val("Demod clock Hz");
        for (k = 0; k < num; k++)
            DBG("%d", m[k].internal_clk);
        DBG("\n");

        val("small/large freq offset");
        for (k = 0; k < num; k++)
            DBG(" 0 | %3d", m[k].dab.large_freqoff);
        DBG("\n");
    }
#endif
    val("Carrier offset");
    for (k = 0; k < num; k++)
        DBG("0x%-13x", ABS(m[k].dds_freq-m[k].p_dds_freq));
    DBG("\n");

    if (m[0].adc_monitor_only) {
        val("ADC LNPower");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].adc_lnpower_raw);
        DBG("\n");
    }

    if (m[0].n_adc_mode == 0) {
        val("timf");
        for (k = 0; k < num; k++)
            DBG("0x%-13x", m[k].timf_current);
        DBG("\n");

#ifdef CONFIG_STANDARD_ISDBT
        if (demods_have_standard(m, num, STANDARD_ISDBT)) {
            if (!m[0].do_not_display_chandec) {
                val("Viterbi syndrome layer a");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome);
                DBG("\n");
                val("Viterbi syndrome layer b");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome_b);
                DBG("\n");
                val("Viterbi syndrome layer c");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome_c);
                DBG("\n");
            }

        }
#endif
#ifdef CONFIG_STANDARD_DVBT
        if (demods_have_standard(m, num, STANDARD_DVBT)) {
            if (!m[0].do_not_display_chandec) {
                val("Viterbi syndrome");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].viterbi_syndrome);
                DBG("\n");
            }
        }
#endif
#ifdef CONFIG_STANDARD_DAB
        if (demods_have_standard(m, num, STANDARD_DAB)) {
            if (!m[0].do_not_display_chandec) {
                val("\nTransmission Mode : ");
                switch (m[0].dab.tmode) {
                    case 0 : val("I"); break;
                    case 1 : val("II"); break;
                    case 2 : val("III"); break;
                    case 3 : val("IV"); break;
                }
                DBG("\n");

                val("Viterbi syndrome GLOBAL | FIC | MSC : ");
                DBG("  %d | %d | %d ", m[0].dab.syn,m[0].dab.syn_fic,m[0].dab.syn_msc);
                DBG("\n");

                val("Viterbi syndrome SubId");
                for (k = 0; k < num; k++)
                    DBG("%d syndrome = %d", m[0].dab.syn_subid, m[0].dab.syn_subc);
                DBG("\n");
            }
        }
#endif
        val("signal");
        for (k = 0; k < num; k++)
            DBG("0x%-4x*2^0x%-4x", m[0].equal_signal_mant, m[0].equal_signal_exp);
        DBG("\n");
        val("noise");
        for (k = 0; k < num; k++)
            DBG("0x%-4x*2^0x%-4x", m[0].equal_noise_mant, m[0].equal_noise_exp);
        DBG("\n");

        if ( demods_have_standard(m, num, STANDARD_DVBT) || demods_have_standard(m, num, STANDARD_ISDBT) || demods_have_standard(m, num, STANDARD_DAB) ) {

#ifdef CONFIG_STANDARD_ISDBT
            if ((demods_have_standard(m, num, STANDARD_ISDBT)) && (m[0].can_display_ber_several_layers == 1))
            {
                if (!m[0].do_not_display_chandec) {
                    val("packet error count A");
                    for (k = 0; k < num; k++)
                        DBG("%-15i", m[k].PacketErrors_A);
                    DBG("\n");
                    val("packet error count B");
                    for (k = 0; k < num; k++)
                        DBG("%-15i", m[k].PacketErrors_B);
                    DBG("\n");
                    val("packet error count C");
                    for (k = 0; k < num; k++)
                        DBG("%-15i", m[k].PacketErrors_C);
                    DBG("\n");
                }
            }
            else
#endif
            if (!m[0].do_not_display_chandec) {

                val("packet error count");
                for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].PacketErrors);
                DBG("\n");
            }

            if (!(demods_have_standard(m, num, STANDARD_ISDBT)) || (m[0].can_display_ber_several_layers != 1))
            {
                if (!m[0].do_not_display_chandec) {
                    val("packet error cumul");
                    for (k = 0; k < num; k++)
                    DBG("%-15i", m[k].PacketErrorCount);
                    DBG("\n");
                }
            }

#ifdef CONFIG_STANDARD_ISDBT
            if ((demods_have_standard(m, num, STANDARD_ISDBT)) && (m[0].can_display_ber_several_layers == 1) && (!m[0].do_not_display_chandec))
            {
                val("Bit Error Rate A");
                for (k = 0; k < num; k++)
                    DBG("0x%-13x", m[k].ber_raw_A);
                DBG("\n");
                val("Bit Error Rate B");
                for (k = 0; k < num; k++)
                    DBG("0x%-13x", m[k].ber_raw_B);
                DBG("\n");
                val("Bit Error Rate C");
                for (k = 0; k < num; k++)
                    DBG("0x%-13x", m[k].ber_raw_C);
                DBG("\n");
            }
            else
#endif
            if (!m[0].do_not_display_chandec) {

                val("Bit Error Rate");
                for (k = 0; k < num; k++)
                    DBG("0x%-13x", m[k].ber_raw);
                DBG("\n");
            }
        }

        if ( (demods_have_standard(m, num, STANDARD_DVBT)) || (demods_have_standard(m, num, STANDARD_ISDBT))) {
            if (!m[0].do_not_display_chandec) {
                val("MER");
                for (k = 0; k < num; k++)
                    DBG("0x%-4x*2^0x%-4x", m[k].mer_mant, m[k].mer_exp);
                DBG("\n");

                val("Quasi Error Free");
                for (k = 0; k < num; k++)
                    DBG("%-15s", m[k].ber < 2e-4 && m[k].PacketErrors == 0 ? "OK" : "*** NOT OK ***");
                DBG("\n");
            }
        } else if (demods_have_standard(m, num, STANDARD_DAB)) {
            val("Quasi Error Free : ber<1e-4");
            DBG("%-15s", m[0].ber_raw < 1e4 && m[0].PacketErrors == 0 ? "OK" : "*** NOT OK ***");
            DBG("\n");
        }
#ifdef CONFIG_STANDARD_ISDBT
            if (demods_have_standard(m, num, STANDARD_ISDBT)) {
                DBG("                                            |            layer a          |           layer b           |          layer c            | \n");
                DBG("demod channel: nfft  guard  sb_mode  p_rcpt   nb_seg  modu  crate t_intlv   nb_seg  modu  crate t_intlv   nb_seg  modu  crate t_intlv\n");
                for (k=0; k < num; k++) {
                    DBG("demod %2d     : ",k);
                    dump_isdbt_channel(&m[k].cur_digital_channel);
                    DBG("\n");
                }
            }
#endif
#ifdef CONFIG_STANDARD_DVBT
            if (demods_have_standard(m, num, STANDARD_DVBT)) {
                DBG("demod channel: nfft guard nqam  hrch alpha crhp crlp intlv\n");
                for (k=0; k < num; k++) {
                    DBG("demod %2d:      ",k);
                    dump_dvb_digital_channel(&m[k].cur_digital_channel);
                    DBG("\n");
                }
            } else if (demods_have_standard(m, num, STANDARD_DVBSH)) {
                DBG("demod channel: nfft guard nqam  hrch alpha crhp      crlp\n");
                for (k=0; k < num; k++) {
                    DBG("demod %2d:      ",k);
                    dump_dvbsh_digital_channel(&m[k].cur_digital_channel);
                    DBG("\n");
                }
            }
#endif
        DBG("\n");
#ifdef CONFIG_STANDARD_ISDBT
        if (demods_have_standard(m, num, STANDARD_ISDBT)) {
            if (!m[0].do_not_display_chandec) {
                DBG("demod locks:  agc    coff        lmod4  equal      tmcc_sync   dvsy\n");
                DBG("              | corm | coff_cpil | pha3 | tmcc_dec | tmcc_data |  vit(a,b,c)  fec_mpeg(a,b,c) \n");
                for (k = 0; k < num; k++)
                    DBG("demod %2d:     %d %d    %d %d         %d %d    %d %d        %d %d         %d  %d%d%d         %d%d%d\n",k,
                            m[k].locks.agc, m[k].locks.corm, m[k].locks.coff, m[k].locks.coff_cpil, m[k].locks.lmod4, m[k].locks.pha3, m[k].locks.equal,m[k].locks.tmcc_dec, m[k].locks.tmcc_sync, m[k].locks.tmcc_data, m[k].locks.dvsy, m[k].locks.vit, m[k].locks.vit_b,m[k].locks.vit_c, m[k].locks.fec_mpeg,m[k].locks.fec_mpeg_b,m[k].locks.fec_mpeg_c);
                DBG("\n");
            }
        }
#endif
#ifdef CONFIG_STANDARD_DVBT
        if (demods_have_standard(m, num, STANDARD_DVBT)) {
            for (k = 0; k < num; k++) {
                if (!m[k].do_not_display_chandec) {
                    DBG("demod locks:  agc    coff        lmod4  equal fec_frm    tps_dec    tps_data     dvsy\n");
                    DBG("              | corm | coff_cpil | pha3 | vit | fec_mpeg | tps_sync | tps_cellid |\n");
                    break;
                }
            }
            for (k = 0; k < num; k++) {
                if (!m[k].do_not_display_chandec) {
                    DBG("demod %2d:     %d %d    %d %d         %d %d    %d %d   %d %d        %d %d        %d %d          %d\n",k,
                            m[k].locks.agc, m[k].locks.corm, m[k].locks.coff, m[k].locks.coff_cpil, m[k].locks.lmod4, m[k].locks.pha3, m[k].locks.equal, m[k].locks.vit, m[k].locks.fec_frm, m[k].locks.fec_mpeg, m[k].locks.tps_dec, m[k].locks.tps_sync, m[k].locks.tps_data, m[k].locks.tps_cellid, m[k].locks.dvsy);
                    DBG("\n");
                }
            }
        }
        else if (demods_have_standard(m, num, STANDARD_DVBSH)) {
            DBG("demod locks:  agc    coff        lmod4  equal tps_dec    tps_data     dvsy\n");
            DBG("              | corm | coff_cpil | pha3 |     | tps_sync | tps_cellid |\n");
            for (k = 0; k < num; k++)
                DBG("demod %2d:     %d %d    %d %d         %d %d    %d     %d %d        %d %d          %d\n",k,
                        m[k].locks.agc, m[k].locks.corm, m[k].locks.coff, m[k].locks.coff_cpil, m[k].locks.lmod4, m[k].locks.pha3, m[k].locks.equal,  m[k].locks.tps_dec, m[k].locks.tps_sync, m[k].locks.tps_data, m[k].locks.tps_cellid, m[k].locks.dvsy);
            DBG("\n");
        }
#endif
#ifdef CONFIG_STANDARD_DAB
         if (demods_have_standard(m, num, STANDARD_DAB)) {
            if (!m[0].do_not_display_chandec) {
                val("Nonlin quant | reverse quant | full internal : ");
                for (k = 0; k < num; k++)
                    DBG("   %dbit | %dbit | %d", m[k].dab.nb_bit_quant, m[k].dab.nb_bit_reverse_quant,m[k].dab.tdint_full_internal);
                DBG("\n");

                DBG("DAB locks:   dmb_lock_frm  ndec_tmode    fec_mpeg   dabcoff_lock\n");
                DBG("             | corm_dmb    | ndec_frame  | fec_frm  |\n");
                for (k = 0; k < num; k++)
                    DBG("demod %2d:    %d %d           %d %d           %d %d        %d\n",k,m[k].dab.dmb_lock_frm, m[k].locks.corm_lock_dmb, m[k].locks.ndec_tmode_lock, m[k].locks.ndec_frame_lock ,m[k].locks.fec_mpeg, m[k].locks.fec_frm, m[k].locks.dabcoff_lock);

                DBG("SubChan :     subc_id  recfg      cifcounter  subc_sz         sz_prev   index      L2punct    PI2\n");
                DBG("              |  isMSC | subc_new |  staddr   |   staddr_prev |   form  | L1punct  |   PI1    | I\n");

                for (k = 0; k < num; k++)
                DBG("demod %2d:    %2d  %d     %d %d       %3d %d       %d  %d          %d  %d     %d %d   %d  %d      %d %d\n",k,m[k].dab.subc_id, m[k].dab.isMSC, m[k].dab.recfg, m[k].dab.subc_new, m[k].dab.cifcounter, m[k].dab.subc_staddr, m[k].dab.subc_sz, m[k].dab.subc_staddr_prev, m[k].dab.subc_sz_prev, m[k].dab.subc_form, m[k].dab.subc_index, m[k].dab.subc_L1punct, m[k].dab.subc_L2punct, m[k].dab.subc_PI1, m[k].dab.subc_PI2, m[k].dab.subc_I);

                DBG("\nFEC id    ");
                for (i = 0 ; i < 12 ; i++)
                    DBG("%2d  ",i);
                DBG("\n");

                for (k = 0; k < num; k++) {
                    DBG("%d STATE:  ",k);
                    for (i = 0 ; i < 12 ; i++)
                        DBG("%2d  ",(m[k].dab.fec_state >> i)&0x1);
                    DBG("\n");

                    DBG("%d SubId:  ",k);
                    for (i = 0 ; i < 12 ; i++)
                        DBG("%2d  ",m[k].dab.fec_subid[i]);
                    DBG("\n\n");
                }
            } else {
                DBG("DAB locks:   signal_detect\n");
                for (k = 0; k < num; k++)
                    DBG("demod %2d:    %d\n",k, m[k].locks.coff);
            }

        }
#endif
        if (m[0].n_2dpa_monitoring) {
            val("mu_int/num_fir");
            for (k = 0; k < num; k++)
                DBG("%d/%d%15s", m[k].n_2dpa_mu_int, m[k].n_2dpa_num_fir, " ");
            DBG("\n");

            for (i = 0; i < 4; i++) {
                DBG("cti_def%d%20s",i," ");
                for (k = 0; k < num; k++)
                    DBG("     (%5d)   ", m[k].cti_def_raw[i]);
                DBG("\n");
            }
            DBG("\n");
        }

        val("Divin FIFO overflow");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].divin_fifo_overflow);
        DBG("\n");

        val("Divin error symbols");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].divin_nberr_sym);
        DBG("\n");

        val("Dvsy first arrived");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].dvsy_first_arrived);
        DBG("\n");

        val("Dvsy delay");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].dvsy_delay);
        DBG("\n");

        val("Divin OK (Way 0, 1)");
        for (k = 0; k < num; k++)
            DBG("%d,%d%12s",m[k].way0_ok,m[k].way1_ok,"");
        DBG("\n");

        val("Inversion"); //, DDS, (msb,lsb)");
        for (k = 0; k < num; k++)
            DBG("%-15d",m[k].invspec);
        DBG("\n");

        val("p DDS Freq");
        for (k = 0; k < num; k++)
            DBG("0x%-13x",m[k].p_dds_freq);
        DBG("\n");

    }
#endif
}
