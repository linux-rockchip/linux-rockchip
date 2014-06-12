#include <demod/dibx000_i2c_common.h>

// tuner_bank_select
// 0 = tuner, 1 = GPIO1,2, 2 = GPIO3,4
// P_i2c_mst_bank_sel

/* 3000MC/P
   n_ti2c_data                0             0     16    0     0     1     768   15    0
   n_ti2c_ctrl                0             0     16    0     0     1     769   15    0
   n_ti2c_status              0             0     16    0     0     1     770   15    0
   P_ti2c_cfg                 175           0     16    0     0     1     771   15    0
 */

/* 7000
   P_i2c_mst_data             0             0     16    0     0     1     768   15    0
   P_i2c_mst_ctrl             0             0     16    0     0     1     769   15    0
   n_i2c_mst_status           0             0     16    0     1     1     770   15    0
   P_i2c_mst_cfg              175           0     10    0     0     1     771   9     0
   P_i2c_mst_bank_sel         0             0     2     0     0     1     772   1     0
 */

/* 7000MC
   P_i2c_mst_data             0             0     16    0     0     1     768   15    0
   P_i2c_mst_ctrl             0             0     16    0     0     1     769   15    0
   n_i2c_mst_status           0             0     16    0     1     1     770   15    0
   P_i2c_mst_cfg              175           0     10    0     0     1     771   9     0
   P_i2c_mst_bank_sel         0             0     2     0     0     1     772   1     0
 */

/* 7000P
   P_i2c_mst_data             0             0     16    0     0     1     1024  15    0
   P_i2c_mst_ctrl             0             0     16    0     0     1     1025  15    0
   n_i2c_mst_status           0             0     16    0     1     1     1026  15    0
   P_i2c_mst_cfg              175           0     10    0     0     1     1027  9     0
   P_i2c_mst_bank_sel         0             0     2     0     0     1     1028  1     0
 */

/*
   st_rdata_apb = 0;
   st_rdata_apb.setbit(8     , st_rest.q()        );
   st_rdata_apb.setbit(7     , st_ack.q()         );
   st_rdata_apb.setbit(6     , st_irq.q()         );
   st_rdata_apb.setrange(5,3 , st_apb_bytecnt.q() );
   st_rdata_apb.setrange(2,0 , st_i2c_bytecnt.q() );
 */

DEBUG_OBJECT_STATIC(dibx000_dbg, DEBUG_BUS, "DiBx000-I2CM");

#define dibx000_write_word_gate(cl, reg, val) \
    data_bus_client_write16_attr(cl, reg, val, DATA_BUS_ACCESS_MODE_GATE_CTRL)
#define dibx000_write_word(mst, reg, val) data_bus_client_write16(mst->data_bus_client, reg, val)
#define dibx000_read_word(msg, reg)       data_bus_client_read16(mst->data_bus_client, reg)


#ifdef CONFIG_BUILD_HOST
static int dibx000_is_i2c_done(struct dibx000_i2c_master *mst)
{
    int i = 100; // max_i2c_polls;
    uint16_t status;

    while (((status = dibx000_read_word(mst, mst->base_reg + 2)) & 0x0100) == 0 && --i > 0);

    /* i2c timed out */
    if (i == 0)
        return DIB_RETURN_ERROR;

    /* no acknowledge */
    if ((status & 0x0080) == 0)
        return DIB_RETURN_ERROR;

    return DIB_RETURN_SUCCESS;
}

static int dibx000_master_i2c_write(struct dibx000_i2c_master *mst, struct dibDataBusClient *client, struct dibDataBusAccess *msg, uint8_t stop)
{
    uint16_t data;
    uint16_t da;
    uint16_t i;
    uint16_t txlen = msg->txlen, len;
    const uint8_t *b = msg->tx;

    while (txlen) {
        dibx000_read_word(mst, mst->base_reg + 2);   // reset fifo ptr

        len = txlen > 8 ? 8 : txlen;
        for (i = 0; i < len; i += 2) {
            data = *b++ << 8;
            if (i+1 < len)
                data |= *b++;
            dibx000_write_word(mst, mst->base_reg, data);
        }
        da = ((((uint8_t) data_bus_access_device_id(client, msg)) >> 1) << 9) | // addr
             (1           << 8) | // master
             (1           << 7) | // rq
             (0           << 6) | // stop
             (0           << 5) | // start
             ((len & 0x7) << 2) | // nb 8 bytes == 0 here
             (0           << 1) | // rw
             (0           << 0);  // irqen

        if (txlen == msg->txlen)
            da |= 1 << 5; /* start */

        if (txlen-len == 0 && stop)
            da |= 1 << 6; /* stop */

        dibx000_write_word(mst, mst->base_reg+1, da);

        if (dibx000_is_i2c_done(mst) != DIB_RETURN_SUCCESS)
            return DIB_RETURN_ERROR;
        txlen -= len;
    }

    return DIB_RETURN_SUCCESS;
}

static int dibx000_master_i2c_read(struct dibx000_i2c_master *mst, struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
    uint16_t da;
    int i;
    uint8_t *b = msg->rx;
    uint16_t rxlen = msg->rxlen, len;

    dibx000_read_word(mst,mst->base_reg + 2);   // reset fifo ptr

    // wr data
    if (msg->txlen != 0)
        if ((i = dibx000_master_i2c_write(mst, client, msg,0)) != DIB_RETURN_SUCCESS)
            return i;

    while (rxlen) {
        len = rxlen > 8 ? 8 : rxlen;
        da = ((((uint8_t) data_bus_access_device_id(client, msg)) >> 1) << 9) | // addr
             (1           << 8) | // master
             (1           << 7) | // rq
             (0           << 6) | // stop
             (0           << 5) | // start
             ((len & 0x7) << 2) | // nb
             (1           << 1) | // rw
             (0           << 0);  // irqen

        if (rxlen == msg->rxlen)
            da |= 1 << 5; /* start */

        if (rxlen-len == 0)
            da |= 1 << 6; /* stop */
        dibx000_write_word(mst, mst->base_reg+1, da);

        if (dibx000_is_i2c_done(mst) != DIB_RETURN_SUCCESS)
            return DIB_RETURN_ERROR;

        rxlen -= len;

        while (len) {
            da = dibx000_read_word(mst, mst->base_reg);
            *b++ = (da >> 8) & 0xff;
            len--;
            if (len >= 1) {
                *b++ =  da   & 0xff;
                len--;
            }
        }
    }

    return DIB_RETURN_SUCCESS;
}

static int dibx000_i2c_select_interface(struct dibx000_i2c_master *mst, uint8_t intf, uint8_t gate)
{
    if (mst->device_rev > DIB3000MC && mst->selected_interface != intf) {
#if 0
        dbgpl(&dibx000_dbg, "selecting interface: %d %d",intf, gate);
#endif
        mst->selected_interface = intf;
        if (gate)
            return dibx000_write_word_gate(mst->gate_ctrl_client, mst->base_reg + 4, intf);
        else
            return dibx000_write_word(mst, mst->base_reg + 4, intf);
    }
    return 0;
}

static int dibx000_i2c_master_xfer(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
    struct dibx000_i2c_master *mst = client->host->priv;
    /* works only for 60MHz demod clock for now TODO check for DiB8000 */
    if (mst->current_speed != data_bus_client_get_speed(client, msg)) {
        uint16_t speed = (uint16_t)data_bus_client_get_speed(client, msg);
        if (mst->device_rev < DIB7000MC && speed < 235) /* DiB7000P and below has a bug in the i2c-master, cannot be slower than 235 */
            speed = 235;
#if 0
        dbgpl(&dibx000_dbg, "switching to %d kHz for SCL on DiBx000 I2C master (%d for 0x%02x, %p)",
                speed,
                60000 / speed,
                data_bus_access_device_id(client, msg), mst);
#endif
        dibx000_write_word(mst, mst->base_reg + 3, (uint16_t)(60000 / speed));
        mst->current_speed = speed;
    }

    dibx000_i2c_select_interface(mst, client->host->interface_id, 0);

    if (msg->rx == NULL && msg->rxlen == 0)
        return dibx000_master_i2c_write(mst, client, msg, 1);
    else
        return dibx000_master_i2c_read(mst, client, msg);
}

static int dibx000_i2c_gate_ctrl(struct dibDataBusClient *gk, struct dibDataBusClient *client, int onoff)
{
    struct dibx000_i2c_master *mst = gk->gk_priv;
    uint16_t val;

#if 0
    if (onoff)
        dbgpl(&dibx000_dbg, "opening gate for %p (%x)- on i2c_address %x", mst, (gk->device_id), client->device_id);
    else
        dbgpl(&dibx000_dbg, "closing gate for %p", mst);
#endif

    if (client != NULL && onoff) {
        /* select the right port when we open up */
        dibx000_i2c_select_interface(mst, client->host->interface_id, 1);
        val = (((uint8_t) client->device_id)>> 1) << 8; // bit 7 = use master or not, if 0, the gate is open
        gk->gk_current_device_id = client->device_id;
    } else {
        val = 1 << 7;
        gk->gk_current_device_id = 0;
    }

    if (mst->device_rev > DIB7000)
        val <<= 1;

    return dibx000_write_word_gate(mst->gate_ctrl_client, mst->base_reg + 1, val);
}

struct dibDataBusHost * dibx000_get_i2c_adapter(struct dibx000_i2c_master *mst, enum dibx000_i2c_interface intf, int gating)
{
    struct dibDataBusHost *i2c = NULL;

    if (mst->device_rev == DIB7000 && !gating)
        return NULL;

    switch (intf) {
        case DIBX000_I2C_INTERFACE_TUNER:
            if (gating)
                i2c = &mst->gated_tuner_i2c_adap;
            else
                i2c = &mst->tuner_i2c_adap;
            break;
        case DIBX000_I2C_INTERFACE_GPIO_1_2:
            if (gating)
                i2c = &mst->gated_gpio_1_2_i2c_adap;
            else
                i2c = &mst->gpio_1_2_i2c_adap;
            break;
        case DIBX000_I2C_INTERFACE_GPIO_3_4:
            if (gating)
                i2c = &mst->gated_gpio_3_4_i2c_adap;
            else
                i2c = &mst->gpio_3_4_i2c_adap;
            break;
        case DIBX000_I2C_INTERFACE_GPIO_6_7:
            if (gating) {
                i2c = &mst->gated_gpio_6_7_i2c_adap; 
                break;
            }
        default:
            dbgpl(&dibx000_dbg, "incorrect interface selected");
            return NULL;
    }

    return i2c;
}

void dibx000_reset_i2c_master(struct dibx000_i2c_master *mst)
{
    /* initialize the i2c-master by closing the gate */
    dibx000_i2c_gate_ctrl(mst->data_bus_client, NULL, 0); // mst->data_bus_client->gk_gate_ctrl(mst->data_bus_client, NULL, 0);
    mst->selected_interface = 0xff;                       // the first time force a select of the I2C
    dibx000_i2c_select_interface(mst, DIBX000_I2C_INTERFACE_TUNER, 0);
}

static void dibx000_reinit(struct dibx000_i2c_master *mst, struct  dibDataBusClient *client)
{
    mst->selected_interface = 0xff; /* the selected interface is unknown now as we powered down the hardware */
    mst->current_speed = 0;

	data_bus_host_interface_reinit(&mst->tuner_i2c_adap,    dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_TUNER);
    data_bus_host_interface_reinit(&mst->gpio_1_2_i2c_adap, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_GPIO_1_2);
    data_bus_host_interface_reinit(&mst->gpio_3_4_i2c_adap, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_GPIO_3_4);
    data_bus_host_interface_reinit(&mst->gated_tuner_i2c_adap,    dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_TUNER);
    data_bus_host_interface_reinit(&mst->gated_gpio_1_2_i2c_adap, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_GPIO_1_2);
    data_bus_host_interface_reinit(&mst->gated_gpio_3_4_i2c_adap, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_GPIO_3_4);
    data_bus_host_interface_reinit(&mst->gated_gpio_6_7_i2c_adap, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_GPIO_6_7);
}

void dibx000_deep_sleep_i2c_master(struct dibx000_i2c_master *mst)
{
    dibx000_reinit(mst, mst->data_bus_client);
}

void dibx000_set_i2c_master_revision(struct dibx000_i2c_master *mst, uint16_t device_rev)
{
    mst->device_rev = device_rev;
    if (device_rev == DIB7000P || device_rev == DIB8000)
        mst->base_reg = 1024;
    else
        mst->base_reg = 768;
}

int dibx000_init_i2c_master(struct dibx000_i2c_master *mst, uint16_t device_rev, struct dibDataBusClient *client)
{
    mst->selected_interface = 0xff; /* the selected interface is unknown now as we powered down the hardware */
    mst->current_speed = 0;

	data_bus_host_interface_init(&mst->tuner_i2c_adap,    DATA_BUS_I2C, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_TUNER);
    data_bus_host_interface_init(&mst->gpio_1_2_i2c_adap, DATA_BUS_I2C, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_GPIO_1_2);
    data_bus_host_interface_init(&mst->gpio_3_4_i2c_adap, DATA_BUS_I2C, dibx000_i2c_master_xfer, mst, DIBX000_I2C_INTERFACE_GPIO_3_4);
    data_bus_host_init_gated(&mst->gated_tuner_i2c_adap,    DATA_BUS_I2C, dibx000_i2c_master_xfer, client, mst, DIBX000_I2C_INTERFACE_TUNER);
    data_bus_host_init_gated(&mst->gated_gpio_1_2_i2c_adap, DATA_BUS_I2C, dibx000_i2c_master_xfer, client, mst, DIBX000_I2C_INTERFACE_GPIO_1_2);
    data_bus_host_init_gated(&mst->gated_gpio_3_4_i2c_adap, DATA_BUS_I2C, dibx000_i2c_master_xfer, client, mst, DIBX000_I2C_INTERFACE_GPIO_3_4);
    data_bus_host_init_gated(&mst->gated_gpio_6_7_i2c_adap, DATA_BUS_I2C, dibx000_i2c_master_xfer, client, mst, DIBX000_I2C_INTERFACE_GPIO_6_7);

    /* make the data_bus_client also a gatekeeper */
    client->gk_gate_ctrl         = dibx000_i2c_gate_ctrl;
    client->gk_current_device_id = 0;
    client->gk_priv              = mst;

    mst->data_bus_client = client; /* the client to control this master */
    mst->gate_ctrl_client = client; /* the client to control the gate open and close - by default it is the same */

    dibx000_set_i2c_master_revision(mst, device_rev);
    return DIB_RETURN_SUCCESS;
}

void dibx000_set_gate_control_client(struct dibx000_i2c_master *mst, struct dibDataBusClient *cl)
{
    mst->gate_ctrl_client = cl;
}

void dibx000_exit_i2c_master(struct dibx000_i2c_master *mst)
{
    data_bus_host_exit(&mst->tuner_i2c_adap);
    data_bus_host_exit(&mst->gpio_1_2_i2c_adap);
    data_bus_host_exit(&mst->gpio_3_4_i2c_adap);

    data_bus_host_exit(&mst->gated_tuner_i2c_adap);
    data_bus_host_exit(&mst->gated_gpio_1_2_i2c_adap);
    data_bus_host_exit(&mst->gated_gpio_3_4_i2c_adap);
    data_bus_host_exit(&mst->gated_gpio_6_7_i2c_adap);
}

#endif
/* format of a register address access to the appropriate way of accessing it using the selected bus
 * is common for all dibx000XX demodulators */

#if 0
static void check_reg(struct dibDataBusClient *data, uint32_t addr)
{
    uint8_t rb[2];
    data_bus_client_read(data, 771, rb, 2);
    dbgp("after %04x id is %04x\n", addr, (rb[0] << 8) | rb[1]);
}
#endif

#ifndef CONFIG_DEMOD_I2C_CHUNK_SIZE
#define CONFIG_DEMOD_I2C_CHUNK_SIZE 125
#endif

static int dibx000_data_bus_i2c_read(struct dibDataBusClient *data, uint32_t address, uint16_t attribute, const uint8_t *tx, uint32_t txlen, uint8_t *b, uint32_t len)
{
    uint8_t wb[2];
    struct dibDataBusAccess acc;
    uint32_t chunk_size = CONFIG_DEMOD_I2C_CHUNK_SIZE > 0 ? CONFIG_DEMOD_I2C_CHUNK_SIZE : len;
    uint32_t l;
    uint8_t ret = DIB_RETURN_SUCCESS;

    if (chunk_size % 2)
        chunk_size++;

    /* if the user did not specify a mode, we force 16bit */
    if (!(attribute & DATA_BUS_ACCESS_MODE_MASK))
        attribute |= DATA_BUS_ACCESS_MODE_16BIT;

    do {
        wb[0] = (uint8_t)((address >> 8) & 0x0f);
        wb[1] = (uint8_t)((address     ) & 0xff);

        if (attribute & DATA_BUS_ACCESS_MODE_8BIT)
            wb[0] |= (1 << 5);
        if (attribute & DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT) // if increment is not requested
            wb[0] |= (1 << 4);

        l = len < chunk_size ? len : chunk_size;
        INIT_DATA_BUS_ACCESS_FOR_READ(acc, 0, wb, 2, b, (uint16_t)l, attribute);

        b += l;
        len -= (unsigned short)l;

        if (attribute & DATA_BUS_ACCESS_MODE_GATE_CTRL)
            ret = (unsigned char)data_bus_transfer_unprotected(data, &acc, 1);
        else
            ret = (unsigned char)data_bus_transfer(data, &acc, 1);

        // if needed increment the address, depending on the address size
        if (!(attribute & DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT))
            address += l/2;
    } while (ret == DIB_RETURN_SUCCESS && len);
    return ret;
}

static int dibx000_data_bus_i2c_write(struct dibDataBusClient *data, uint32_t address, uint16_t attribute, const uint8_t *b, uint32_t len)
{
    uint8_t wb[255];
    struct dibDataBusAccess acc;
    uint32_t chunk_size = CONFIG_DEMOD_I2C_CHUNK_SIZE > 0 ? CONFIG_DEMOD_I2C_CHUNK_SIZE : len;
    uint32_t l;
    uint8_t ret = DIB_RETURN_SUCCESS;

    if (chunk_size > 252)
        chunk_size = 252; // avoid wb buffer overflow

    if (chunk_size % 2)
        chunk_size++;

    /* if the user did not specify a mode, we assume 16bit */
    if (!(attribute & DATA_BUS_ACCESS_MODE_MASK))
        attribute |= DATA_BUS_ACCESS_MODE_16BIT;

    do {
        wb[0] = (uint8_t)((address >> 8) & 0x0f);
        wb[1] = (uint8_t)((address     ) & 0xff);

        if (address == 0xffff)
            wb[0] = 0xff;

        if (attribute & DATA_BUS_ACCESS_MODE_8BIT)
            wb[0] |= (1 << 5);
        if (attribute & DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT) // if increment is not requested
            wb[0] |= (1 << 4);

        l = len < chunk_size ? len : chunk_size;

        INIT_DATA_BUS_ACCESS_FOR_WRITE(acc, 0, wb, (uint16_t)l+2, attribute);

        memcpy(&wb[2], b, l);

        b += l;
        len -= (unsigned short)l;

        if (attribute & DATA_BUS_ACCESS_MODE_GATE_CTRL)
            ret = (unsigned char)data_bus_transfer_unprotected(data, &acc, 1);
        else
            ret = (unsigned char)data_bus_transfer(data, &acc, 1);

        // if needed increment the address, depending on the address size
        if (!(attribute & DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT))
            address += l/2;

    } while (ret == DIB_RETURN_SUCCESS && len);
    return ret;
}

const struct dibDataBusClient dibx000_i2c_data_bus_client = {
    NULL,

    12,
    16,

    dibx000_data_bus_i2c_read,
    dibx000_data_bus_i2c_write,

    0,

    100,
};

#ifdef CONFIG_BUILD_HOST
static int dibx000_data_bus_mmio_read(struct dibDataBusClient *data, uint32_t address, uint16_t attribute, const uint8_t *tx, uint32_t txlen, uint8_t *b, uint32_t len)
{
    struct dibDataBusAccess acc;

    if (attribute & DATA_BUS_ACCESS_MODE_8BIT) // byte mode
        address |= (1 << 13);
    if (!(attribute & DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT)) // if increment is requested
        address |= (1 << 12);

    /* if the user did not specify a mode, we force 16bit */
    if ((attribute & DATA_BUS_ACCESS_MODE_MASK) == 0)
        attribute |= DATA_BUS_ACCESS_MODE_16BIT;

    INIT_DATA_BUS_ACCESS_FOR_READ(acc, address, NULL, 0, b, len, attribute);
    return data_bus_transfer(data, &acc, 1);
}

static int dibx000_data_bus_mmio_write(struct dibDataBusClient *data, uint32_t address, uint16_t attribute, const uint8_t *b, uint32_t len)
{
    struct dibDataBusAccess acc;

    if (attribute & DATA_BUS_ACCESS_MODE_8BIT) // byte mode
        address |= (1 << 13);
    if (!(attribute & DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT)) // if increment is requested
        address |= (1 << 12);

    /* if the user did not specify a mode, we force 16bit */
    if ((attribute & DATA_BUS_ACCESS_MODE_MASK) == 0)
        attribute |= DATA_BUS_ACCESS_MODE_16BIT;

    INIT_DATA_BUS_ACCESS_FOR_WRITE(acc, address, b, len, attribute);

    return data_bus_transfer(data, &acc, 1);
}

const struct dibDataBusClient dibx000_mmio_data_bus_client = {
    NULL,

    12,
    16,

    dibx000_data_bus_mmio_read,
    dibx000_data_bus_mmio_write,
};
#endif
