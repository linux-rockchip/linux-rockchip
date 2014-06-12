#include "adapter/common.h"
#include "adapter/databus.h"

//#define DEBUG_DATA
#define DEBUG_TYPE (DATA_BUS_MMIO_SRAM_LIKE|DATA_BUS_DIBSPI)
//#define DEBUG_TYPE DATA_BUS_I2C

int data_bus_host_init(struct dibDataBusHost *a, uint16_t type, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *), void *priv)
{
	if (a == NULL)
		return DIB_RETURN_ERROR;

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
	if (!(type & DATA_BUS_I2C_COMPAT))
#endif
    if (func == NULL)
        return DIB_RETURN_ERROR;

    DibZeroMemory(a, sizeof(struct dibDataBusHost));

	DibInitLock(&a->lock);

	a->type = type;
	a->xfer = func;
	a->priv = priv;

	return DIB_RETURN_SUCCESS;
}

int data_bus_host_interface_init(struct dibDataBusHost *a, uint16_t type, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *), void *priv, uint8_t intf)
{
    if (data_bus_host_init(a, type, func, priv) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;
    a->interface_id = intf;
    return DIB_RETURN_SUCCESS;
}

#ifdef CONFIG_BUILD_HOST
int data_bus_host_interface_reinit(struct dibDataBusHost *a, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *), void *priv, uint8_t intf)
{
	if (a == NULL)
		return DIB_RETURN_ERROR;

// There is an issue with MS windows / driver verifier :
// no function exist to un-init a lock : we can not prevent driver verifier
// from complaining about multiple initialization of the lock
#ifdef _WIN32

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
	if (!(a->type & DATA_BUS_I2C_COMPAT))
#endif
    if (func == NULL)
        return DIB_RETURN_ERROR;

	a->xfer = func;
	a->priv = priv;
    a->interface_id = intf;
	a->gate_keeper = 0;
	// Keep original value
	//	a->root_client = 0;

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
	a->xfer_i2c_compat = 0;
#endif

#else
    uint16_t type = a->type;
    struct dibDataBusClient *root_client = a->root_client;

    data_bus_host_exit(a);
    if (data_bus_host_init(a, type, func, priv) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;
    a->interface_id = intf;
	a->root_client = root_client;
#endif

	return DIB_RETURN_SUCCESS;
}
#endif

#ifndef NO_I2C_GATED
int data_bus_host_init_gated(struct dibDataBusHost *host, uint16_t type, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *),
	struct dibDataBusClient *root, void *priv, uint8_t intf)
{
	int ret = data_bus_host_init(host, type | DATA_BUS_GATED, func, priv);
	host->root_client = root;
    host->interface_id = intf;
	return ret;
}
#endif

void data_bus_host_reset(struct dibDataBusHost *host)
{
    if (host)
        host->gate_keeper = NULL;
}

void data_bus_host_exit(struct dibDataBusHost *adap)
{
	DibFreeLock(&adap->lock);
}

int data_bus_client_init(struct dibDataBusClient *client, const struct dibDataBusClient *tmpl, struct dibDataBusHost *bus)
{
	memcpy(client, tmpl, sizeof(*client));
	data_bus_client_set_data_bus(client, bus);
	if (data_bus_host_type(bus) & DATA_BUS_I2C)
		data_bus_client_set_speed(client, 340); /* default speed inherited from I2CAdapter */
	return DIB_RETURN_SUCCESS;
}

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
static int i2c_transfer_compat(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	struct dibI2CAccess dmsg;
	dmsg.addr    = (uint8_t) data_bus_access_device_id(client, msg);
	dmsg.scl_khz = (uint16_t) client->speed;
	dmsg.tx      = msg->tx;
	dmsg.txlen   = (uint8_t) msg->txlen;
	dmsg.rx      = msg->rx;
	dmsg.rxlen   = (uint8_t) msg->rxlen;
	return client->host->xfer_i2c_compat(client->host, &dmsg);
}
#endif

static int data_bus_transfer_common(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	struct dibDataBusHost *host = client->host;
	int ret;

#ifndef NO_I2C_GATED
	/* if the message is for a GATED (virtual) host */
	if (host->type & DATA_BUS_GATED) {

		/* if it is the first transfer call set the source client into the message
		 * - to keep the device information (speed, id) */
		if (msg->first_client == NULL)
			msg->first_client = client;

		/* and call the transfer of the client of the gated bus (only a gated bus has it */
		return data_bus_transfer(host->root_client, msg, 1);
	}
#endif

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
	if (host->type & DATA_BUS_I2C_COMPAT)
		ret = i2c_transfer_compat(client, msg);
	else
#endif
        ret = host->xfer(client, msg);

#ifdef DEBUG_DATA
    if (host->type & DEBUG_TYPE) {
        dbgp("%s %04x %08x %5d %04x ", msg->rx == NULL ? "wr" : "rd", client->host->type, client->device_id, client->speed, msg->mode);
        if (msg->first_client)
            dbgp("%08x %5d ", msg->first_client->device_id, msg->first_client->speed);
        else
            dbgp("               ");

        dbgp("%08x ", msg->address);

        dbg_bufdump(msg->tx, msg->txlen);

        if (msg->rx != NULL && msg->rxlen != 0) {
            dbgp(" - ");
            dbg_bufdump(msg->rx, msg->rxlen);
        }
        dbgp("\n");
    }
#endif
	return ret;
}

int data_bus_transfer_unprotected(struct dibDataBusClient *client, struct dibDataBusAccess *msg, int num)
{
	int i,ret = DIB_RETURN_SUCCESS;
	for (i = 0; i < num; i++)
		if ((ret = data_bus_transfer_common(client, &msg[i])) != DIB_RETURN_SUCCESS)
			break;
	return ret;
}

int data_bus_transfer(struct dibDataBusClient *client, struct dibDataBusAccess *msg, int num)
{
	int i, ret = DIB_RETURN_SUCCESS;
#ifndef CONFIG_BUILD_LEON
	struct dibDataBusHost   *host = client->host;
#endif

	DibAcquireLock(&host->lock);
	for (i = 0; i < num; i++) {
#ifndef NO_I2C_GATED
        struct dibDataBusClient *gate = host->gate_keeper;

/*		dbgp("message: %d %p==%p %d==%d\n", msg->for_virtual_bus, gk,
			msg->gate_keeper, gk != NULL ? gk->current_device_id : 0,
			msg->gate_keeper != NULL ? client->device_id : 0);
*/

		/* close the gate if the message is not for a gated (virtual) bus but there is currently a gate open */
		if (msg->first_client == NULL &&
			gate != NULL) {

			/* set gate_keeper to NULL before the current gate is closed - closing the gate is done by the same transfer-type maybe */
			host->gate_keeper = NULL;

			if (gate->gk_gate_ctrl)
				gate->gk_gate_ctrl(gate, 0, 0);
		}

		/* this message goes to a device behind the gate */
		if (msg->first_client != NULL) {

			/* there is a gate keeper currently open */
			/* and this message is going through another gatekeeper */
			/* or the message goes to another i2c-address of the same gate */

			if ( gate != NULL &&
				(gate != client ||
				(gate->gk_current_device_id != msg->first_client->device_id)) ) {

				/* set gate_keeper to NULL before the current gate is closed - closing the gate is done via i2c, too */
				host->gate_keeper = NULL;

				/* close the previous gate */
				if (gate->gk_gate_ctrl)
					gate->gk_gate_ctrl(gate, 0, 0);

				/* open the new gate */
				if (client->gk_gate_ctrl)
					client->gk_gate_ctrl(client, msg->first_client, 1);

				/* this is now the current gate keeper */
				host->gate_keeper = client;
			}

			/* there is no gate open at the moment */
			if (gate == NULL) {
				if (client->gk_gate_ctrl)
					client->gk_gate_ctrl(client, msg->first_client, 1);

				/* this is now the current gate keeper */
				host->gate_keeper = client;
			}
		}
#endif
		if ((ret = data_bus_transfer_common(client, &msg[i])) != DIB_RETURN_SUCCESS)
			break;
#ifndef NO_I2C_GATED
        if ((data_bus_client_get_gk_prop(client) & CLOSE_GATE_AFTER_EACH_TRANSACTION) && (msg->first_client != NULL)) {
            /* close the gate */
            if (client->gk_gate_ctrl)
                client->gk_gate_ctrl(client, msg->first_client, 0);

            gate = NULL;
        }
#endif
    }
	DibReleaseLock(&host->lock);

	return ret;
}


int data_bus_client_write8(struct dibDataBusClient *client, uint32_t addr, uint8_t v)
{
    return data_bus_client_write_attr(client, addr, DATA_BUS_ACCESS_MODE_8BIT, &v, 1);
}


int data_bus_client_write16_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t v, uint16_t attr)
{
    uint8_t b[2];
    b[0] = (uint8_t) ((v >> 8) & 0xff);
    b[1] = (uint8_t) ( v       & 0xff);
    attr &= ~DATA_BUS_ACCESS_MODE_MASK;
    return data_bus_client_write_attr(client, addr, attr | DATA_BUS_ACCESS_MODE_16BIT, b, 2);
}

int data_bus_client_write16(struct dibDataBusClient *client, uint32_t addr, uint16_t v)
{
    return data_bus_client_write16_attr(client, addr, v, 0);
}

int data_bus_client_write32(struct dibDataBusClient *client, uint32_t addr, uint32_t v)
{
    uint8_t b[4];
    b[0] = (uint8_t) ((v >> 24) & 0xff);
    b[1] = (uint8_t) ((v >> 16) & 0xff);
    b[2] = (uint8_t) ((v >>  8) & 0xff);
    b[3] = (uint8_t) ( v        & 0xff);
    return data_bus_client_write_attr(client, addr, DATA_BUS_ACCESS_MODE_32BIT, b, 4);
}

uint8_t data_bus_client_read8(struct dibDataBusClient *client, uint32_t addr)
{
    uint8_t v;
    if (data_bus_client_read_attr(client, addr, DATA_BUS_ACCESS_MODE_8BIT, &v, 1) != DIB_RETURN_SUCCESS)
        return 0;
    return v;
}

uint16_t data_bus_client_read16_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr)
{
    uint16_t b;
    if (data_bus_client_read_multi16_attr(client, addr, (attr & ~DATA_BUS_ACCESS_MODE_MASK) | DATA_BUS_ACCESS_MODE_16BIT, &b, 1) != DIB_RETURN_SUCCESS)
        return 0;
    return b;
}

uint16_t data_bus_client_read16(struct dibDataBusClient *client, uint32_t addr)
{
    return data_bus_client_read16_attr(client, addr, 0);
}

uint32_t data_bus_client_read32(struct dibDataBusClient *client, uint32_t addr)
{
    uint32_t b;
    if (data_bus_client_read_multi32_attr(client, addr, DATA_BUS_ACCESS_MODE_32BIT, &b, 1) != DIB_RETURN_SUCCESS)
        return 0;
    return b;
}

int data_bus_client_read_multi16(struct dibDataBusClient *client, uint32_t addr, uint16_t *b, uint32_t nb)
{
    return data_bus_client_read_multi16_attr(client, addr, DATA_BUS_ACCESS_MODE_16BIT, b, nb);
}

int data_bus_client_read_multi16_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint16_t *b, uint32_t nb)
{
    uint8_t *d;
    uint16_t val;
    uint32_t i;

    if (data_bus_client_read_attr(client, addr, (attr & ~DATA_BUS_ACCESS_MODE_MASK) | DATA_BUS_ACCESS_MODE_16BIT, (uint8_t *) b, nb * 2) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;

    d = (uint8_t *) b;
    for (i = 0; i < nb; i++) {
        val = b[i];
        *d++ = val >> 8;
        *d++ = val & 0xff;
    }

    return DIB_RETURN_SUCCESS;
}

int data_bus_client_read_multi32(struct dibDataBusClient *client, uint32_t addr, uint32_t *b, uint32_t nb)
{
    return data_bus_client_read_multi32_attr(client, addr, DATA_BUS_ACCESS_MODE_32BIT, b, nb);
}

int data_bus_client_read_multi32_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint32_t *b, uint32_t nb)
{
    uint8_t *d;
    uint32_t val;
    uint32_t i;

    if (data_bus_client_read_attr(client, addr, (attr & ~DATA_BUS_ACCESS_MODE_MASK) | DATA_BUS_ACCESS_MODE_32BIT, (uint8_t *) b, (uint16_t) (nb * 4)) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;

    d = (uint8_t *) b;
    for (i = 0; i < nb; i++) {
        val = b[i];
        *d++ = val >> 24;
        *d++ = val >> 16;
        *d++ = val >> 8;
        *d++ = val & 0xff;
    }

    return DIB_RETURN_SUCCESS;
}

int data_bus_client_write_multi32(struct dibDataBusClient *client, uint32_t addr, uint32_t *b, uint32_t nb)
{
    return data_bus_client_write_multi32_attr(client, addr, DATA_BUS_ACCESS_MODE_32BIT, b, nb);
}

int data_bus_client_write_multi32_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint32_t *b, uint32_t nb)
{
    uint8_t *d;
    uint32_t val;
    uint32_t i;

    d = (uint8_t *) b;
    for (i = 0; i < nb; i++) {
        val = b[i];
        *d++ = val >> 24;
        *d++ = val >> 16;
        *d++ = val >> 8;
        *d++ = val & 0xff;
    }

    if (data_bus_client_write_attr(client, addr, (attr & ~DATA_BUS_ACCESS_MODE_MASK) | DATA_BUS_ACCESS_MODE_32BIT, (uint8_t *) b, (uint32_t) (nb * 4)) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;

    return DIB_RETURN_SUCCESS;
}

int data_bus_client_write_multi16(struct dibDataBusClient *client, uint32_t addr, uint16_t *b, uint32_t nb)
{
    return data_bus_client_write_multi16_attr(client, addr, DATA_BUS_ACCESS_MODE_16BIT, b, nb);
}

int data_bus_client_write_multi16_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint16_t *b, uint32_t nb)
{
    uint8_t *d;
    uint16_t val;
    uint32_t i;

    d = (uint8_t *) b;
    for (i = 0; i < nb; i++) {
        val = b[i];
        *d++ = val >> 8;
        *d++ = val & 0xff;
    }

    if (data_bus_client_write_attr(client, addr, (attr & ~DATA_BUS_ACCESS_MODE_MASK) | DATA_BUS_ACCESS_MODE_16BIT, (uint8_t *) b, (uint32_t) (nb * 2)) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;

    return DIB_RETURN_SUCCESS;
}

/* common for dibcom tuners */
static int data_bus_8_16_data_read(struct dibDataBusClient *client, uint32_t address, uint16_t attribute, const uint8_t *tx, uint32_t txlen, uint8_t *b, uint32_t len)
{
	struct dibDataBusAccess acc;
	INIT_DATA_BUS_ACCESS_FOR_READ(acc, address, NULL, 0, b, len, DATA_BUS_ACCESS_MODE_16BIT | attribute);
	return data_bus_transfer(client, &acc, 1);
}

static int data_bus_8_16_data_write(struct dibDataBusClient *client, uint32_t address, uint16_t attribute, const uint8_t *b, uint32_t len)
{
	struct dibDataBusAccess acc;
	INIT_DATA_BUS_ACCESS_FOR_WRITE(acc, address, b, len, DATA_BUS_ACCESS_MODE_16BIT | attribute);
	return data_bus_transfer(client, &acc, 1);
}

uint32_t data_bus_client_get_speed(struct dibDataBusClient *client, struct dibDataBusAccess * msg)
{
	if (msg != NULL && msg->first_client != 0)
	{
		struct dibDataBusClient * cur_client = msg->first_client;
		uint32_t min_speed = client->speed;
		while (cur_client != NULL) {
			if (cur_client->speed < min_speed)
				min_speed = cur_client->speed;
			cur_client = cur_client->host->root_client;
		}
		return min_speed;
	}
	else
		 return client->speed;
}

static int data_bus_data_write_0wire(struct dibDataBusClient *client, uint32_t address, uint16_t attribute, const uint8_t *b, uint32_t len)
{
	struct dibDataBusAccess acc;
	uint8_t buf[255];
	//0wire is special
	buf[0] = (uint8_t) address;
	memcpy(&buf[1], b, len);
	INIT_DATA_BUS_ACCESS_FOR_WRITE(acc, 0, buf, len+1, DATA_BUS_ACCESS_MODE_16BIT);
	return data_bus_transfer(client, &acc, 1);
}

const struct dibDataBusClient data_bus_client_template_8b_16b = {
    NULL,

    8,
    16,

    data_bus_8_16_data_read,
    data_bus_8_16_data_write,
};

const struct dibDataBusClient data_bus_client_template_0wire = {
	NULL,

	0,
	16,

	NULL,
	data_bus_data_write_0wire,
};


//warning TODO choose another name for this define, best case : use "no i2c to the tuner" see dib0090.c for the same used flag
//#ifndef CONFIG_BUILD_LEON
static int data_bus_data_read_i2c(struct dibDataBusClient *client, uint32_t address, uint16_t attribute, const uint8_t *tx, uint32_t txlen, uint8_t *b, uint32_t len)
{
	struct dibDataBusAccess acc;
	uint8_t addr = (uint8_t) address;
	INIT_DATA_BUS_ACCESS_FOR_READ(acc, 0, &addr, 1, b, len, DATA_BUS_ACCESS_MODE_16BIT);
	return data_bus_transfer(client, &acc, 1);
}

static int data_bus_data_write_i2c(struct dibDataBusClient *client, uint32_t address, uint16_t attribute, const uint8_t *b, uint32_t len)
{
	struct dibDataBusAccess acc;
	uint8_t buf[255];
	buf[0] = (uint8_t) address;
	memcpy(&buf[1], b, len);
	INIT_DATA_BUS_ACCESS_FOR_WRITE(acc, 0, buf, len+1, DATA_BUS_ACCESS_MODE_16BIT);
	return data_bus_transfer(client, &acc, 1);
}

const struct dibDataBusClient data_bus_client_template_i2c_8b_16b = {
	NULL,

	8,
	16,

	data_bus_data_read_i2c,
	data_bus_data_write_i2c,
};
//#endif

int data_bus_client_access(struct dibDataBusClient *client, struct dibDataBusAccess *acc)
{
    if (acc->rx == NULL || acc->rxlen == 0)
        return data_bus_client_write_attr(client, acc->address, acc->mode, acc->tx, acc->txlen);
    else
        return data_bus_client_read_attr(client, acc->address, acc->mode, acc->rx, acc->rxlen);
}

static int data_bus_client_clone_read(struct dibDataBusClient *c, uint32_t address, uint16_t attribute, const uint8_t *tx, uint32_t txlen, uint8_t *b, uint32_t len)
{
    return data_bus_client_read_attr(c->clone_src, address, attribute, b, len);
}

static int data_bus_client_clone_write(struct dibDataBusClient *c, uint32_t address, uint16_t attribute, const uint8_t *b, uint32_t len)
{
    return data_bus_client_write_attr(c->clone_src, address, attribute, b, len);
}

void data_bus_client_clone(struct dibDataBusClient *c, struct dibDataBusClient *src)
{
    memcpy(c, src, sizeof(*c));
    c->read = data_bus_client_clone_read;
    c->write = data_bus_client_clone_write;
    c->clone_src = src;
}

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
/* backward compat */
int i2c_adapter_init(struct dibDataBusHost *bus, const struct dibI2CMasterXfer *xfer)
{
	/* when using i2c_adapter_init priv will be set outside */
	int ret = data_bus_host_init(bus, DATA_BUS_I2C | DATA_BUS_I2C_COMPAT, NULL, NULL);
	if (ret == DIB_RETURN_SUCCESS)
		bus->xfer_i2c_compat = xfer->master_xfer;
	return ret;
}

int dib_i2c_transfer(struct dibDataBusHost *bus, struct dibI2CAccess *msg) //jan
{
	struct dibDataBusClient client;
	struct dibDataBusAccess dmsg;

	DibZeroMemory(&client, sizeof(client));
	DibZeroMemory(&dmsg,   sizeof(dmsg));

	data_bus_client_set_data_bus(&client, bus);
	data_bus_client_set_speed(&client, msg->scl_khz);
	data_bus_client_set_device_id(&client, msg->addr);

	dmsg.tx = msg->tx;
	dmsg.txlen = msg->txlen;
	dmsg.rx = msg->rx;
	dmsg.rxlen = msg->rxlen;
	dmsg.mode = DATA_BUS_ACCESS_MODE_8BIT;

	return data_bus_transfer(&client, &dmsg, 1);
}

int i2c_transfer_to_control_a_gate(struct dibDataBusHost *bus, struct dibI2CAccess *msg)
{
	struct dibDataBusClient client;
	struct dibDataBusAccess dmsg;

	DibZeroMemory(&client, sizeof(client));
	DibZeroMemory(&dmsg,   sizeof(dmsg));

	client.host  = bus;
	client.device_id = msg->addr;
	client.speed     = msg->scl_khz;

	dmsg.tx = msg->tx;
	dmsg.txlen = msg->txlen;
	dmsg.rx = msg->rx;
	dmsg.rxlen = msg->rxlen;
	dmsg.mode = DATA_BUS_ACCESS_MODE_8BIT;

	return data_bus_transfer_unprotected(&client, &dmsg, 1);
}
#endif // NO_I2C_ADAPTER_BACKWARD_COMPAT
