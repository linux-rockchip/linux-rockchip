#include <adapter/frontend.h>
#include <sip/dibx090p.h>
#include "dibx090p_priv.h"

//#define CONFIG_SIP_DibX090P_DEBUG

static const struct dibDebugObject dibx090p_dbg = {
    DEBUG_SIP,
    "DiBX090P_CTRL"
};

static void dibx090p_get_version(struct dibFrontend *fe)
{
    struct dibx090p_state *state = fe->sip->priv;
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);

    state->version = data_bus_client_read16(client, 897);
}

extern int dibx090p_update_lna(struct dibFrontend *fe, uint16_t agc_global)
{
    struct dibx090p_state *st = fe->sip->priv;

    if (st->cfg->update_lna)
        return st->cfg->update_lna(fe, agc_global);
    return 0;
}

extern int dibx090p_tuner_sleep(struct dibFrontend *fe, int onoff)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    struct dibx090p_state *state= fe->sip->priv;
    uint16_t en_cur_state;

    dbgpl(&dibx090p_dbg, "sleep dibx090p: %d", onoff);

    en_cur_state = data_bus_client_read16(client, 1922);
#ifdef CONFIG_SIP_Dibx090p_DEBUG
    dbgpl(&dibx090p_dbg, "***Current state is %x",en_cur_state);
#endif

    if (en_cur_state > 0xff) { //LNAs and MIX are ON and therefore it is a valid configuration
#ifdef CONFIG_SIP_Dibx090p_DEBUG
        dbgpl(&dibx090p_dbg, "***Wakeup state Stored TUNER_EN=%x",en_cur_state);
#endif
        state->tuner_enable = en_cur_state ;
    }

    if(onoff)
        en_cur_state &= 0x00ff; //Mask to be applied
    else {
        if (state->tuner_enable != 0)
            en_cur_state = state->tuner_enable;
#ifdef CONFIG_SIP_Dibx090p_DEBUG
        else
            dbgpl(&dibx090p_dbg, "***Tuner_enable is empty Bypass");
#endif
    }

    data_bus_client_write16(client, 1922, en_cur_state);

    return DIB_RETURN_SUCCESS;
}

extern int dibx090p_agc_restart(struct dibFrontend *fe, uint8_t restart)
{
    //dbgpl(&dibx090p_dbg, "AGC restart callback: %d", restart);
    //dib0090_ctrl_agc_filter(fe, restart);
    if(restart == 0) /* before AGC startup */
        dib0090_set_dc_servo(fe, 1);
      
    return DIB_RETURN_SUCCESS;
}

extern int dibx090p_get_dc_power(struct dibFrontend *fe, uint8_t IQ)
{
    struct dibx090p_state *state = fe->sip->priv;
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    int val = 0;
    uint16_t n_iqc_dcoff_i, n_iqc_dcoff_q;

    if(state->version == 0)
        dibx090p_get_version(fe);

    if(state->version == 0x8090) {
        n_iqc_dcoff_i = 403;
        n_iqc_dcoff_q = 404;
    } else if(state->version == 0x7090) {
        n_iqc_dcoff_i = 407;
        n_iqc_dcoff_q = 408;
    }

    switch (IQ) {
        case 1: val = data_bus_client_read16(client, n_iqc_dcoff_i); break; // Iqc_dcoffI Channel
        case 0: val = data_bus_client_read16(client, n_iqc_dcoff_q); break; //Iqc_dcoffQ channel
    }
    if (val  & 0x200) //10 bits signed
        val -= 1024;

  return val;
}

extern void dibx090p_release(struct dibFrontend *fe)
{
    struct dibx090p_state *state = fe->sip->priv;
    MemFree(state,sizeof(struct dibx090p_state));
}

static int map_addr_to_serpar_number(struct dibDataBusAccess * acc) {
    if((acc->address <= 15))
        acc->address -= 1;
    else if(acc->address == 17)
        acc->address = 15;
    else if(acc->address == 16)
        acc->address = 17;
    else if(acc->address == 19)
        acc->address = 16;
    else if(acc->address >= 21 && acc->address <=25)
        acc->address -= 3;
    else if(acc->address == 28)
        acc->address = 23;
    else if(acc->address == 99)
        acc->address = 99;
    else {
#ifdef CONFIG_SIP_Dibx090p_DEBUG
        dbgpl(&dibx090p_dbg, "WARNING : Address %d can not be converted to serpar number.",acc->address);
#endif
        return DIB_RETURN_ERROR;
    }
    return DIB_RETURN_SUCCESS;
}

void dibx090p_dump_tuner_dig(struct dibFrontend *fe)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    int reg;

    for(reg = 1920 ; reg < 1949 ; reg ++)
        dbgpl(&dibx090p_dbg, "Reg %d = %d", reg, data_bus_client_read16(client, reg));
}

static void dibx090p_tuner_write_serpar(struct dibFrontend *fe, uint16_t serpar_num, uint16_t data_to_write)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint8_t n_overflow = 1;
    uint16_t i = 1000;
    while (n_overflow == 1 && i) {
        n_overflow = (data_bus_client_read16(client, 1984)>>1)&0x1; // DIB_PARAM_n_overflow
        i--;
        if (i == 0)
            dbgpl(&dibx090p_dbg, "Tuner ITF: write busy (overflow)");
    }
    data_bus_client_write16(client, 1985, (1<<6) | (serpar_num&0x3f)); // DIB_PARAM_n_rw = 1 & serpar addr
    data_bus_client_write16(client, 1986, data_to_write);
}

extern uint16_t dibx090p_tuner_read_serpar(struct dibFrontend *fe, uint16_t serpar_num)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint8_t n_overflow = 1, n_empty = 1;
    uint16_t i = 1000;
    
    while (n_overflow == 1 && i) {
        n_overflow = (data_bus_client_read16(client, 1984)>>1)&0x1; // DIB_PARAM_n_overflow
        i--;
        if (i == 0)
            dbgpl(&dibx090p_dbg, "TunerITF: read busy (overflow)");
    }
    data_bus_client_write16(client, 1985, (0<<6) | (serpar_num&0x3f));
    
    i = 1000;
    while (n_empty == 1 && i) {
        n_empty = data_bus_client_read16(client, 1984)&0x1; // DIB_PARAM_n_empty
        i--;
        if (i == 0)
            dbgpl(&dibx090p_dbg, "TunerITF: read busy (empty)");
    }
    
    return data_bus_client_read16(client, 1987);
}

static int dibx090p_tuner_rw_serpar(struct dibFrontend *fe, struct dibDataBusAccess *acc)
{
    uint16_t word;
    uint16_t data_to_write;

    if(map_addr_to_serpar_number(acc) != DIB_RETURN_SUCCESS) /* Tuner regs to ignore : DIG_CFG, CTRL_RF_LT, PLL_CFG, PWM1_REG, PWM2_REG, ADCCLK, DIG_CFG_3; SLEEP_EN... */
        return DIB_RETURN_ERROR;
    if (acc->rxlen == 0 || acc->rx == NULL) { /* write */
        data_to_write = (uint16_t)((acc->tx[0] << 8) | acc->tx[1]);
        dibx090p_tuner_write_serpar(fe, acc->address, data_to_write);

#ifdef CONFIG_SIP_Dibx090p_DEBUG
        /* Read back */
        word = dibx090p_tuner_read_serpar(fe, acc->address);
        if(word != data_to_write) {
            dbgpl(&dibx090p_dbg, "W/R SERPAR ERROR @ Num %d : W 0x%04x, R 0x%04x  ",acc->address, (uint16_t)((acc->tx[0] << 8) | acc->tx[1]), word);
            return DIB_RETURN_ERROR;
        }
#endif

    } else { /* read */
        word = dibx090p_tuner_read_serpar(fe, acc->address);
        acc->rx[0] = (word >> 8) & 0xff;
        acc->rx[1] = (word     ) & 0xff;
    }
    return DIB_RETURN_SUCCESS;
}

#if 0
void dibx090p_dump_tuner_dig(struct dibFrontend *fe)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    int reg;

    for(reg = 1920 ; reg < 1948 ; reg ++)
        dbgpl(&dibx090p_dbg, "Reg %d = %d", reg, data_bus_client_read16(client, reg));

    for(reg = 1984 ; reg < 1987 ; reg ++)
        dbgpl(&dibx090p_dbg, "Reg %d = %d", reg, data_bus_client_read16(client, reg));
}
#endif

static int dibx090p_rw_on_apb(struct dibFrontend *fe, struct dibDataBusAccess *acc, uint16_t apb_address)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint16_t word;

    acc->address = apb_address;
    if (acc->rxlen == 0 || acc->rx == NULL) /* write */
        return data_bus_client_write16(client, apb_address, ((acc->tx[0]<<8) | (acc->tx[1])));

    word = data_bus_client_read16(client, apb_address);
    acc->rx[0] = (word >> 8) & 0xff;
    acc->rx[1] = (word     ) & 0xff;
    return DIB_RETURN_SUCCESS;
}

void dibx090p_dump_serpar(struct dibFrontend *fe, struct dibDataBusAccess *acc)
{
    uint16_t word;
    if(acc->address == 99) {
        int i;
        for (i = 0 ; i < 24 ; i++) {
            word = dibx090p_tuner_read_serpar(fe, i);
            acc->rx[0] = (word >> 8) & 0xff;
            acc->rx[1] = (word     ) & 0xff;
            dbgpl(&dibx090p_dbg, "separ %d = 0x%04x", i, word);
        }
    }
}

int dibx090p_tuner_xfer(struct dibDataBusClient *tuner_client, struct dibDataBusAccess *acc)
{
    struct dibx090p_state *state = tuner_client->host->priv;
    struct dibDataBusClient * client = demod_get_data_bus_client(state->fe);
    struct dibFrontend *fe = state->fe;

    uint16_t apb_address = 0, word;
    int i = 0;

    uint16_t P_sad_addr, n_sad_data0_addr;

    if(state->version == 0)
        dibx090p_get_version(state->fe);

    if(state->version == 0x8090) {
        P_sad_addr = 921;
        n_sad_data0_addr = 924;
    } else if(state->version == 0x7090) {
        P_sad_addr = 72;
        n_sad_data0_addr = 384;
    }

    switch(acc->address) {
        case IQ_ADC_CTRL : apb_address = 1920; break;
        case CTRL_CRYSTAL: apb_address = 1921; break;
	case TUNER_EN    : apb_address = 1922; break;
        case DIG_CFG_RO  : apb_address = 1923; break;
        case CALIBRATE   : apb_address = 1924; break;
        case BB_RAMP1    : apb_address = 1926; break;
        case BB_RAMP2    : apb_address = 1927; break;
        case BB_RAMP3    : apb_address = 1928; break;
        case BB_RAMP4    : apb_address = 1929; break;
        case BB_RAMP5    : apb_address = 1930; break;
        case BB_RAMP6    : apb_address = 1931; break;
        case BB_RAMP7    : apb_address = 1932; break;
        case RF_RAMP1    : apb_address = 1935; break;
        case RF_RAMP2    : apb_address = 1936; break;
        case RF_RAMP3    : apb_address = 1937; break;
        case RF_RAMP4    : apb_address = 1938; break;
        case RF_RAMP5    : apb_address = 1939; break;
        case RF_RAMP6    : apb_address = 1940; break;
        case RF_RAMP7    : apb_address = 1941; break;
        case RF_RAMP8    : apb_address = 1942; break;
        case RF_RAMP9    : apb_address = 1943; break;
        case GAIN4_1     : apb_address = 1944; break;
        case GAIN4_2     : apb_address = 1945; break;
        case CAPTRIM_CFG : apb_address = 1948; break;
	case EFUSE_1     : apb_address = (state->version == 0x8090)? 936 : 914; break;
	case EFUSE_2     : apb_address = (state->version == 0x8090)? 937 : 915; break;
	case EFUSE_3     : apb_address = (state->version == 0x8090)? 938 : 917; break;
	case EFUSE_4     : apb_address = (state->version == 0x8090)? 939 : 916; break;
        case ADCVAL :
            i = ((data_bus_client_read16(client, P_sad_addr) >> 12)&0x3); /* get sad sel request */
            word = data_bus_client_read16(client, n_sad_data0_addr+i);
            //dbgpl(&dibx090p_dbg, "sad[%d] %d %f (%d , %f)10bits ",i, word, word/4096.0, word>>2, (word>>2)/2048.0); /* 12bits ADC */
            acc->rx[0] = (word >> 8) & 0xff;
            acc->rx[1] = (word     ) & 0xff;
            return DIB_RETURN_SUCCESS;

	case VGA_MODE:
            if (acc->rxlen == 0 || acc->rx == NULL) { /* write */
                //Place the Sel in the Sad register
                word = (uint16_t)((acc->tx[0] << 8) | acc->tx[1]);
                word &= 0x3; //in the VGAMODE Sel are located on bit 0/1
                //dbgpl(&dibx090p_dbg, "Sel = %d",word);
                word = (data_bus_client_read16(client, P_sad_addr) & ~(3<<12)) | (word<<12); //Mask bit 12,13
                //dbgpl(&dibx090p_dbg, "to be written %x",word);
                data_bus_client_write16(client, P_sad_addr, word); /* Set the proper input */
                return DIB_RETURN_SUCCESS;
        }
    }

    if(apb_address != 0) /* R/W acces via APB */
        return dibx090p_rw_on_apb(fe, acc, apb_address);
    return dibx090p_tuner_rw_serpar(fe, acc); /* R/W access via SERPAR  */
}

static void dibx090p_host_bus_drive(struct dibFrontend *fe, uint8_t drive)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint16_t reg;
    drive &= 0x7;

    /* drive host bus 2, 3, 4 */
    reg = data_bus_client_read16(client, 1798) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    data_bus_client_write16(client, 1798, reg);

    /* drive host bus 5,6 */
    reg = data_bus_client_read16(client, 1799) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    data_bus_client_write16(client, 1799, reg);

    /* drive host bus 7, 8, 9 */
    reg = data_bus_client_read16(client, 1800) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    data_bus_client_write16(client, 1800, reg);

    /* drive host bus 10, 11 */
    reg = data_bus_client_read16(client, 1801) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    data_bus_client_write16(client, 1801, reg);

    /* drive host bus 12, 13, 14 */
    reg = data_bus_client_read16(client, 1802) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    data_bus_client_write16(client, 1802, reg);
}

static uint32_t dibx090p_calcSyncFreq(uint32_t P_Kin, uint32_t P_Kout, uint32_t insertExtSynchro, uint32_t syncSize)
{
    uint32_t quantif = 3;
    uint32_t nom = (insertExtSynchro*P_Kin+syncSize);
    uint32_t denom = P_Kout;
    uint32_t syncFreq = ((nom<<quantif) / denom );

    if ((syncFreq & ((1<<quantif)-1)) != 0)
       syncFreq = (syncFreq>>quantif) +1;
    else
       syncFreq = (syncFreq>>quantif);

    if(syncFreq != 0)
        syncFreq = syncFreq - 1;

    return syncFreq;
}

static void dibx090p_cfg_DibTx(struct dibFrontend *fe, uint32_t P_Kin, uint32_t P_Kout, uint32_t insertExtSynchro, uint32_t synchroMode, uint32_t syncWord, uint32_t syncSize)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    //uint16_t rx_copy_buff[22];
    dbgpl(&dibx090p_dbg, "Configure DibStream Tx");

    /*HW bug workarround : Dibstream TX/RX bus addr discrimination issue */
    //data_bus_client_read_multi16(client, 1536, rx_copy_buff, 22); /* saving a copy of dibstream Rx config */

    data_bus_client_write16(client, 1615, 1);                // dibstream_tx_n_cfg_restart     = 1
    data_bus_client_write16(client, 1603, P_Kin);            // dibstream_tx_n_Kin             = P_Kin
    data_bus_client_write16(client, 1605, P_Kout);           // dibstream_tx_n_Kout            = P_Kout
    data_bus_client_write16(client, 1606, insertExtSynchro); // dibstream_tx_n_start_out_gene  = insertExtSynchro
    data_bus_client_write16(client, 1608, synchroMode);      // dibstream_tx_n_insert_on_start = synchroMode
    data_bus_client_write32(client, 1609, syncWord);         // dibstream_tx_n_sync_word       = syncWord
    data_bus_client_write16(client, 1612, syncSize);         // dibstream_tx_n_sync_size       = syncSize
    data_bus_client_write16(client, 1615, 0);                // dibstream_tx_n_cfg_restart     = 0

    /*HW bug workarround : Dibstream TX/RX bus addr discrimination issue */
    //data_bus_client_write_multi16(client, 1536, rx_copy_buff, 22); /* rewriting Rx config */
}


static void dibx090p_cfg_DibRx(struct dibFrontend *fe, uint32_t P_Kin, uint32_t P_Kout, uint32_t synchroMode, uint32_t insertExtSynchro, uint32_t syncWord, uint32_t syncSize, uint32_t dataOutRate)
{
   struct dibDataBusClient * client = demod_get_data_bus_client(fe);
   uint32_t syncFreq;

   dbgpl(&dibx090p_dbg, "Configure DibStream Rx synchroMode = %d", synchroMode);

   if((P_Kin != 0) && (P_Kout != 0)) {
       syncFreq = dibx090p_calcSyncFreq(P_Kin,P_Kout,insertExtSynchro,syncSize);
       data_bus_client_write16(client, 1542, syncFreq);         // dibstream_rx_n_sync_freq = syncFreq
    }

   data_bus_client_write16(client, 1554, 1);                // dibstream_rx_n_cfg_restart     = 1
   data_bus_client_write16(client, 1536, P_Kin);            // dibstream_rx_n_Kin             = P_Kin
   data_bus_client_write16(client, 1537, P_Kout);           // dibstream_rx_n_Kout            = P_Kout
   data_bus_client_write16(client, 1539, synchroMode);      // dibstream_rx_n_check_on_start  = synchroMode
   data_bus_client_write32(client, 1540, syncWord);         // dibstream_rx_n_sync_word       = syncWord
   data_bus_client_write16(client, 1543, syncSize);         // dibstream_rx_n_sync_size       = syncSize
   data_bus_client_write16(client, 1544, dataOutRate);      // dibstream_rx_n_dataout_rate    = dataOutRate
   data_bus_client_write16(client, 1554, 0);                // dibstream_rx_n_cfg_restart     = 0
}

static void dibx090p_enMpegMux(struct dibFrontend *fe, int onoff)
{
    struct dibx090p_state * state = fe->sip->priv;
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint16_t reg_1287;

    if(state->version == 0)
        dibx090p_get_version(fe);

    reg_1287 = data_bus_client_read16(client, 1287);

    switch (onoff) {
        case 1:
            if(state->version == 0x8090)
                reg_1287 &= ~(1<<8);
            else if (state->version == 0x7090)
                reg_1287 &= ~(1<<7);
        break;
        case 0:
            if(state->version == 0x8090)
                reg_1287 |= (1<<8);
            else if (state->version == 0x7090)
                reg_1287 |= (1<<7);
        break;
    }

    data_bus_client_write16(client, 1287, reg_1287);
}

static void dibx090p_configMpegMux(struct dibFrontend *fe, uint16_t pulseWidth, uint16_t enSerialMode, uint16_t enSerialClkDiv2)
{
    struct dibx090p_state *state = fe->sip->priv;
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint16_t reg_1287;

    if(state->version == 0)
        dibx090p_get_version(fe);

    dbgpl(&dibx090p_dbg, "Enable Mpeg mux");

    dibx090p_enMpegMux(fe, 0);

    if((enSerialMode == 1) && (state->input_mode_mpeg == 1)) /* If the input mode is MPEG do not divide the serial clock */
        enSerialClkDiv2 = 0;

    reg_1287 = data_bus_client_read16(client, 1287);
    if(state->version == 0x8090)
        reg_1287 = ((pulseWidth & 0x1f) << 3) | ((enSerialMode & 0x1) << 2) | (enSerialClkDiv2 & 0x1);
    else if (state->version == 0x7090)
        reg_1287 = ((pulseWidth & 0x1f) << 2) | ((enSerialMode & 0x1) << 1) | (enSerialClkDiv2 & 0x1);

    data_bus_client_write16(client, 1287, reg_1287);

    dibx090p_enMpegMux(fe, 1);
}

extern void dibx090p_setDibTxMux(struct dibFrontend *fe, int mode)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint16_t reg_1288 = data_bus_client_read16(client, 1288) & ~(0x7 << 7);
    switch (mode) {
        case MPEG_ON_DIBTX:
            dbgpl(&dibx090p_dbg, "Fe %d : SET MPEG ON DIBSTREAM TX", fe->id);
            dibx090p_cfg_DibTx(fe, 8,5,0,0,0,0);
            reg_1288 |= (1<<9); break;
        case DIV_ON_DIBTX:
            dbgpl(&dibx090p_dbg, "Fe %d : SET DIV_OUT ON DIBSTREAM TX", fe->id);
            dibx090p_cfg_DibTx(fe, 5,5,0,0,0,0);
            reg_1288 |= (1<<8); break;
        case ADC_ON_DIBTX:
            dbgpl(&dibx090p_dbg, "Fe %d : SET ADC_OUT ON DIBSTREAM TX", fe->id);
            dibx090p_cfg_DibTx(fe, 20,5,10,0,0,0);
            reg_1288 |= (1<<7); break;
        default:
        break;
    }
    data_bus_client_write16(client, 1288, reg_1288);
}

extern void dibx090p_setHostBusMux(struct dibFrontend *fe, int mode)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint16_t reg_1288 = data_bus_client_read16(client, 1288) & ~(0x7 << 4);
    switch (mode) {
        case DEMOUT_ON_HOSTBUS:
            dbgpl(&dibx090p_dbg, "Fe %d : SET DEM OUT OLD INTERF ON HOST BUS", fe->id);
            dibx090p_enMpegMux(fe, 0);
            reg_1288 |= (1<<6);
            break;
        case DIBTX_ON_HOSTBUS:
            dbgpl(&dibx090p_dbg, "Fe %d : SET DIBSTREAM TX ON HOST BUS", fe->id);
            dibx090p_enMpegMux(fe, 0);
            reg_1288 |= (1<<5);
            break;
        case MPEG_ON_HOSTBUS:
            dbgpl(&dibx090p_dbg, "Fe %d : SET MPEG MUX ON HOST BUS", fe->id);
            reg_1288 |= (1<<4);
            break;
        default:
        break;
    }
    data_bus_client_write16(client, 1288, reg_1288);
}

int dibx090p_set_diversity_in(struct dibFrontend *fe, int onoff)
{
    struct dibx090p_state *state = fe->sip->priv;
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    uint16_t reg_1287;

    switch (onoff) {
        case 0: /* only use the internal way - not the diversity input */
            dbgpl(&dibx090p_dbg, "fe #%d : set_diversity_in mode OFF : by default Enable Mpeg INPUT", fe->id);
            dibx090p_cfg_DibRx(fe, 8,5,0,0,0,8,0); /*outputRate = 8 */

            /* Do not divide the serial clock of MPEG MUX in SERIAL MODE in case input mode MPEG is used */
            reg_1287 = data_bus_client_read16(client, 1287);
            if((reg_1287 & 0x1) == 1) { /* enSerialClkDiv2 == 1 ? */
                reg_1287 &= ~0x1; /* force enSerialClkDiv2 = 0 */
                data_bus_client_write16(client, 1287, reg_1287);
            }
            state->input_mode_mpeg = 1;
            break;
        case 1: /* both ways */
        case 2: /* only the diversity input */
            dbgpl(&dibx090p_dbg, "fe #%d : set_diversity_in ON : Enable diversity INPUT", fe->id);
            dibx090p_cfg_DibRx(fe, 5,5,0,0,0,0,0);
            state->input_mode_mpeg = 0;
            break;
    }

    state->set_diversity_in(fe, onoff);
    return DIB_RETURN_SUCCESS;
}

int dibx090p_set_output_mode(struct dibFrontend *fe, int mode)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    struct dibx090p_state *state = fe->sip->priv;

    uint16_t outreg, smo_mode, fifo_threshold;
    uint8_t prefer_mpeg_mux_use = 1;
    int ret = 0;
    uint16_t P_smo_mode_addr;

    if(state->version == 0)
        dibx090p_get_version(fe);

    if(state->version == 0x8090)
        P_smo_mode_addr = 299;
    else if (state->version == 0x7090)
        P_smo_mode_addr = 235;

    dibx090p_host_bus_drive(fe,1);

    fifo_threshold = 1792;
    smo_mode = (data_bus_client_read16(client, P_smo_mode_addr)& 0x0050) | (1 << 1);
    outreg   = data_bus_client_read16(client, 1286)&~((1<<10) | (0x7<<6) | (1<<1));

    switch (mode) {
        case OUTPUT_MODE_OFF:
                outreg = 0;
            break;

        case OUTPUT_MODE_TS_SERIAL:
            if(prefer_mpeg_mux_use) {
                dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode TS_SERIAL using Mpeg Mux", fe->id);
                dibx090p_configMpegMux(fe, 3, 1, 1);
                dibx090p_setHostBusMux(fe, MPEG_ON_HOSTBUS);
            } else {/* Use Smooth block */
                dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode TS_SERIAL using Smooth bloc", fe->id);
                dibx090p_setHostBusMux(fe, DEMOUT_ON_HOSTBUS);
                outreg |= (2<<6) | (0 << 1);
            }
            break;

        case OUTPUT_MODE_TS_PARALLEL_GATED:
            if(prefer_mpeg_mux_use) {
                dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode TS_PARALLEL_GATED using Mpeg Mux", fe->id);
                dibx090p_configMpegMux(fe, 2, 0, 0);
                dibx090p_setHostBusMux(fe, MPEG_ON_HOSTBUS);
            } else { /* Use Smooth block */
                dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode TS_PARALLEL_GATED using Smooth block", fe->id);
                dibx090p_setHostBusMux(fe, DEMOUT_ON_HOSTBUS);
                outreg |= (0<<6);
            }
            break;

    	case OUTPUT_MODE_TS_PARALLEL_CONT: /* Using Smooth block only */
                dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode TS_PARALLEL_CONT using Smooth block", fe->id);
                dibx090p_setHostBusMux(fe, DEMOUT_ON_HOSTBUS);
                outreg|= (1<<6);
            break;

    	case OUTPUT_MODE_TS_FIFO: /* Using Smooth block because not supported by new Mpeg Mux bloc */
                dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode TS_FIFO using Smooth block", fe->id);
                dibx090p_setHostBusMux(fe, DEMOUT_ON_HOSTBUS);
                outreg |= (5<<6);
                smo_mode |= (3 << 1);
                fifo_threshold = 512;
            break;

    	case OUTPUT_MODE_DIVERSITY:
            dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode MODE_DIVERSITY", fe->id);
            dibx090p_setDibTxMux(fe, DIV_ON_DIBTX);
            dibx090p_setHostBusMux(fe, DIBTX_ON_HOSTBUS);
            break;

    	case OUTPUT_MODE_ANALOG_ADC:
            dbgpl(&dibx090p_dbg, "Fe%d : Sipx090p setting output mode MODE_ANALOG_ADC", fe->id);
            dibx090p_setDibTxMux(fe, ADC_ON_DIBTX);
            dibx090p_setHostBusMux(fe, DIBTX_ON_HOSTBUS);
            break;
    }

    if (mode != OUTPUT_MODE_OFF)
        outreg |= (1<<10);

    dbgpl(&dibx090p_dbg, "Fe%d : output_mpeg2_in_188_bytes = %d", fe->id, state->cfg->output_mpeg2_in_188_bytes);
    if (state->cfg->output_mpeg2_in_188_bytes)
        smo_mode |= (1 << 5);

    ret |= data_bus_client_write16(client, P_smo_mode_addr, smo_mode);
    ret |= data_bus_client_write16(client, P_smo_mode_addr + 1, fifo_threshold); /* synchronous fread */
    ret |= data_bus_client_write16(client, 1286, outreg);

    return ret;
}

#ifdef BIST_TEST
extern int dibx090p_test_bist(struct dibFrontend *fe, unsigned int cmdCfg, unsigned int targetMem, unsigned int algoType, unsigned int algoSeq, unsigned int addrCfg, unsigned int dataCfg)
{
   struct dibDataBusClient * client = demod_get_data_bus_client(fe);
   unsigned int dataToWrite;
   unsigned int dataToRead;
   unsigned int addr;

   data_bus_client_write16(client, 0x306, 0); //P_force_pwdn_dem,
   data_bus_client_write16(client, 0x302, 0); //P_restart_dem

   data_bus_client_write16(client, 0x396, 1); //DIB_WR(P_enClk_bist,1);
   data_bus_client_write16(client, 0x382, 1); //DIB_WR(P_restart_bist,1);
   data_bus_client_write16(client, 0x382, 0); //DIB_WR(P_restart_bist,0);

   /***** BIST PROG 2 *****/
   dataToWrite = (targetMem << 9) |  // all rams if 127
                 (        1 << 4) |  // bistmode : enable bist mode
                 (   cmdCfg << 0) ;  // command

   data_bus_client_write16(client, 0x385, dataToWrite); //DIB_WR_BUF(DIB_ADDR(P_bist_prog2),&dataToWrite,1,16,0);


   /***** BIST PROG *****/
   dataToWrite = (algoType << 11) |  // 1  std algo
                 ( addrCfg <<  9) |  // 0  adrcfg (NA bit 11 = 1);
                 ( dataCfg <<  5) |  // 0  datacfg(background select)
                 ( algoSeq <<  0) ;  // 1  4N
   data_bus_client_write16(client, 0x384, dataToWrite); //DIB_WR_BUF(DIB_ADDR(P_bist_prog),&dataToWrite,1,16,0);


   /***** BIST RES *****/
   dataToRead = 0;
   while ((dataToRead & 0x1) != 1) {
      dataToRead = data_bus_client_read16(client, 0x386); //DIB_RD_BUF(DIB_ADDR(M_bist_res),&dataToRead,1,16,0);
   }

   if ( ((dataToRead >> 1) & 0x1) == 1)
   {
      dbgpl(NULL,"BIST ERROR DETECTED");
      return 1;
   }
   else
   {
      dbgpl(NULL,"BIST DONE WITHOUT ERROR");
      return 0;
   }

}
#endif

