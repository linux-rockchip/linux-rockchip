#ifndef DATABUS_H
#define DATABUS_H

#include "../platform/platform.h"
#include "../platform/platform_linux_kernel.h"


#ifdef __cplusplus
extern "C" {
#endif

struct dibDataBusClient;
struct dibDataBusAccess;

/* backward compat */
struct dibI2CAccess;
//typedef int DIB_LOCK;

struct dibDataBusHost {
#define DATA_BUS_TYPE_MASK       0x7fff
#define DATA_BUS_I2C             0x0001
#define DATA_BUS_MMIO_SRAM_LIKE  0x0002
#define DATA_BUS_SDIO            0x0004
#define DATA_BUS_DIBCTRL_IF      0x0008
#define DATA_BUS_MESSAGE_PROXY   0x0010
#define DATA_BUS_APB             0x0020
#define DATA_BUS_AHB             0x0040
#define DATA_BUS_0WIRE           0x0080
#define DATA_BUS_1WIRE           0x0100
#define DATA_BUS_DIBSPI          0x0200
#define DATA_BUS_GATED           0x8000

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
#define DATA_BUS_I2C_COMPAT      0x4000
#endif


	/**
	 * type field keeps the type of this DataBusHost
	 */
	uint16_t type;

	/**
	 * function which is called by "data_bus_transfer" when a client wants to do an access. Implementers have to give this function at "data_bus_host_init".
	 */
	int (*xfer) (struct dibDataBusClient *, struct dibDataBusAccess *);

	/* fields used when this DataBusHost there is a DATA_BUS_GATED somewhere (below) */

	/**
	 * This field keeps the client which is currently having the gate open (to close it later). Used internally for gate-mechanism.
	 */
	struct dibDataBusClient *gate_keeper;

	/**
	 * This field contains the root client which is hosting this gated host. Used internally for the gate-mechanism.
	 */
	struct dibDataBusClient *root_client;

	/**
	 * A Mutex used to protect multi-threaded accesses from each other. Used internally by "data_bus_transfer".
	 */
	DIB_LOCK lock;

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
	/* only kept for backward compatibility - do not use for new DataBus-implementations */
	int (*xfer_i2c_compat) (struct dibDataBusHost *, struct dibI2CAccess *);
#endif

	/* this field is used to distinguish the phy interface used by the controller of this bus-host - mandatory for gated hosts */
    uint8_t interface_id;

	void *priv;
};

//! Retrieve the type of a DataBusHost
/**
 * Calling this function retrieves the type of the DataBusHost. Used
 * by component driver to choose the correct client.
 *
 * \param d pointer to a valid dibDataBusHost
 * \return type of the given DataBusHost
 */
#define data_bus_host_type(d) ((d)->type & DATA_BUS_TYPE_MASK)

//! Initialize a DataBusHost

/**
 * Calling this function will initialize the dibDataBusHost. Users
 * need to call this function to create a valid DataBusHost which then
 * can be used to attach component-drivers.
 *
 * \param host pointer to valid struct dibDataBusHost to be initialized
 * \param type the type of the DataBusHost
 * \param func address of a function which is called when an attached client wants to do a transfer
 * \param priv pointer pointing to private memory. When receiving a call to "func" the DataBusHost implementer has this memory available in "client->host->priv".
 * \return DIB_RETURN_SUCCESS in case of success otherwise DIB_RETURN_ERROR
 */
extern int data_bus_host_init(struct dibDataBusHost *host, uint16_t type, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *), void *priv);

/**
 * Calling this function will initialize the dibDataBusHost. Users
 * need to call this function to create a valid DataBusHost which then
 * can be used to attach component-drivers. If the real controller is
 * able to handle more than one interface, call this function set the
 * interface ID.
 *
 * \param host pointer to valid struct dibDataBusHost to be initialized
 * \param type the type of the DataBusHost
 * \param func address of a function which is called when an attached client wants to do a transfer
 * \param priv pointer pointing to private memory. When receiving a call to "func" the DataBusHost implementer has this memory available in "client->host->priv".
 * \param intf interface ID used by this host
 * \return DIB_RETURN_SUCCESS in case of success otherwise DIB_RETURN_ERROR
 */
extern int data_bus_host_interface_init(struct dibDataBusHost *host, uint16_t type, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *), void *priv, uint8_t intf);


extern int data_bus_host_interface_reinit(struct dibDataBusHost *host, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *), void *priv, uint8_t intf);

//! Initialize a gated DataBusHost
/**
 * An overloaded call of data_bus_host_init used for DataBusses which
 * are gated (e.g. the tuner behind a demodulator). Parameters are
 * identical to data_bus_host_init except the caller has to give an
 * additional client which gives access to the bus before the gated
 * bus.
 *
 * A gated bus must have an interface ID inside in order to select the
 * right interface when activating the gate.
 *
 * \param host pointer to valid struct dibDataBusHost to be initialized
 * \param type the type of the DataBusHost
 * \param func address of a function which is called when an attached client wants to do a transfer
 * \param root valid dibDataBusClient giving access to the root-bus.
 * \param priv pointer pointing to private memory. When receiving a call to "func" the DataBusHost implementer has this memory available in "client->host->priv".
 * \param intf interface ID used by this gated host
 * \return DIB_RETURN_SUCCESS in case of success otherwise DIB_RETURN_ERROR
 */
extern int data_bus_host_init_gated(struct dibDataBusHost *host, uint16_t type, int (*func)(struct dibDataBusClient *, struct dibDataBusAccess *),
	struct dibDataBusClient *root, void *priv, uint8_t intf);

//! Deinitialize a DataBusHost
/**
 * Deinitializing and cleaning up the DataBusHost at the end of its lifetime.
 *
 * \param host valid and previously initialized DataBusHost
 */
extern void data_bus_host_exit(struct dibDataBusHost *host);

//! Reset a DataBusHost
/**
 * After going to deep-sleep it is useful to reset the software representation of a DataBus, because a possibly open gate is not open any longer.
 *
 * \param host valid and previously initialized DataBusHost
 */
extern void data_bus_host_reset(struct dibDataBusHost *host);

//! Doing a transfer/access
/**
 * For example component drivers are calling this function when they
 * want to do an Access to their corresponding hardware. It is very
 * important to issue accesses using this function (even for
 * application specific components), because this function is taking
 * care about 2 important things: It opens and closes a gated in case
 * a gated transfer is used and it locks each transfer with a Mutex in
 * case it is used in a threaded environment.
 *
 * \param client is the client of the component
 * \param access one or more access representing one or more transfer (depending on the type of the corresponding host)
 * \param num number of DataBusAccesses given
 * \return DIB_RETURN_SUCCESS in case of success otherwise DIB_RETURN_ERROR
 */
extern int data_bus_transfer(struct dibDataBusClient *client, struct dibDataBusAccess *, int num);

//! Doing an access unprotected
/**
 * calling this will do basicly the same as "data_bus_transfer" but
 * unprotected. It neither opens/closes the gate nor does it use the
 * mutex. Don't use this function except for a message to open and
 * close a gated.
 *
 * \param client is the client of the component
 * \param access one or more access representing one or more transfer (depending on the type of the corresponding host)
 * \param num number of DataBusAccesses given
 * \return DIB_RETURN_SUCCESS in case of success otherwise DIB_RETURN_ERROR
 */
extern int data_bus_transfer_unprotected(struct dibDataBusClient *client, struct dibDataBusAccess *, int num);

struct dibDataBusAccess {
	/* memory address to be accessed */
	uint32_t address;

	/* buffer to be written to the address. Can be NULL in a read access */
	const uint8_t *tx;
	uint32_t txlen;

	/* buffer to be read to the address. Must be NULL in a write access */
	uint8_t *rx;
	uint32_t rxlen;

#define DATA_BUS_ACCESS_MODE_MASK                 0x0007
#define DATA_BUS_ACCESS_MODE_8BIT                 0x0001
#define DATA_BUS_ACCESS_MODE_16BIT                0x0002
#define DATA_BUS_ACCESS_MODE_32BIT                0x0004
#define DATA_BUS_ACCESS_MODE_NO_SYNC              0x0008
#define DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT 0x0010
#define DATA_BUS_ACCESS_MODE_GATE_CTRL            0x0020
	uint16_t mode;

    const uint8_t *tx_addr;
    uint8_t       tx_addr_len;

	/* in case of a virtual/gated bus this field keeps the original client */
	struct dibDataBusClient *first_client;
};

//! Initialize a DataBusAccess to do a write
/**
 * Use this define to initialize the DataBusAccess for a write.
 * Depending on the type of the Host each field has a different
 * meaning.
 *
 * \param acc instance of a dibDataBusAccess
 * \param _addr address of the register (in case of memory access transfers)
 * \param _tx byte-buffer to send to the device
 * \param _txlen length of _tx
 * \param _mode mode of the transfer - gives information about the nature of the transfer
 */
#define INIT_DATA_BUS_ACCESS_FOR_WRITE(acc, _addr, _tx, _txlen, _mode) do { \
	DibZeroMemory(&acc, sizeof(struct dibDataBusAccess)); \
	acc.address = _addr; \
	acc.tx      = _tx; \
	acc.txlen   = _txlen; \
	acc.mode    = _mode; \
} while (0)

//! Initialize a DataBusAccess to do a read
/**
 * Use this define to initialize the DataBusAccess for a read.
 * Depending on the type of the Host each field has a different
 * meaning.
 *
 * \param acc instance of a dibDataBusAccess
 * \param _addr address of the register (in case of memory access transfers)
 * \param _tx byte-buffer to send to the device (to initialize the communication) (can be NULL)
 * \param _txlen length of _tx (can be 0)
 * \param _rx byte-buffer where the Host will/has to put the received data
 * \param _rxlen length of _rx
 * \param _mode mode of the transfer - gives information about the nature of the transfer
 */
#define INIT_DATA_BUS_ACCESS_FOR_READ(acc, _addr, _tx, _txlen, _rx, _rxlen, _mode) do { \
	DibZeroMemory(&acc, sizeof(struct dibDataBusAccess)); \
	acc.address = _addr; \
	acc.tx      = _tx; \
	acc.txlen   = _txlen; \
	acc.rx      = _rx; \
	acc.rxlen   = _rxlen; \
	acc.mode    = _mode; \
} while (0)

//!
/** Retrieve the device ID (Device address)
 * Calling this function will retrieve the device id or address. The
 * client has to be provided also to retrieve the correct address in
 * case of using gated-transfer.
 *
 * \param client valid pointer to a DataBusClient
 * \param acc valid pointer to a DataBusAccess
 */
#define data_bus_access_device_id(client, acc) ((acc)->first_client != NULL ? (acc)->first_client->device_id : (client)->device_id)

/* will be provided by each component using a dibDataBusHost */
struct dibDataBusClient {
	struct dibDataBusHost *host;

	uint8_t address_size;          /* in bits */
	uint8_t value_size;            /* default, in bits, use data_bus_client_get_value_size() to access */

	/* attribute is using DATA_BUS_ACCESS_MODE_* */
	int (*read)  (struct dibDataBusClient *, uint32_t address, uint16_t attribute, const uint8_t *tx, uint32_t txlen, uint8_t *b, uint32_t len);
	int (*write) (struct dibDataBusClient *, uint32_t address, uint16_t attribute, const uint8_t *b, uint32_t len);

/* contains the device address on the bus if applicable
 * e.g. the I2C-address on an I2C-Bus */
	uint32_t device_id;

/* can be used to give an indicator to the DataBusAdapter what can be the speed of talking to the device
 * - in DIBCTRL/I2C it is the scl in kHz
 * - in MMIO is the high = 1 or low = 0
 */
	uint32_t speed;

/* in case the size of the data in this client is not constant (hence
 * depending on the address), this function returns the good size
 * depending on the address in bits */
	uint8_t (*value_size_by_addr) (struct dibDataBusClient *, uint32_t);

/* user defined field to store the status of this client */
	uint32_t status;

/* gate keeper fields in case there is an gated client after this device */
	int (*gk_gate_ctrl) (struct dibDataBusClient *, struct dibDataBusClient *first, int);
	uint32_t gk_current_device_id;
	void *gk_priv;

/* this field takes the original dataBusClient where this one is a clone of */
    struct dibDataBusClient *clone_src;

#define CLOSE_GATE_AFTER_EACH_TRANSACTION   (1<<0)
    uint32_t gk_prop;
};

/* write/read functions */

#define data_bus_client_write(d, a, b, l) \
	((d) != NULL && (d)->write != NULL ? (d)->write(d, a, 0, b, l) : DIB_RETURN_ERROR)
#define data_bus_client_read(d, a, b, l) \
	((d) != NULL && (d)->read  != NULL ? (d)->read(d, a, 0, NULL, 0, b, l)  : DIB_RETURN_ERROR)
#define data_bus_client_read_ex(d, a, tx, txlen, b, l) \
	((d) != NULL && (d)->read  != NULL ? (d)->read(d, a, 0, tx, txlen, b, l)  : DIB_RETURN_ERROR)

#define data_bus_client_write_attr(d, a, attr, b, l) \
	((d) != NULL && (d)->write != NULL ? (d)->write(d, a, attr, b, l) : DIB_RETURN_ERROR)
#define data_bus_client_read_attr(d, a, attr, b, l) \
	((d) != NULL && (d)->read  != NULL ? (d)->read(d, a, attr, NULL, 0, b, l)  : DIB_RETURN_ERROR)

extern uint8_t data_bus_client_read8(struct dibDataBusClient *, uint32_t addr);
extern int data_bus_client_write8(struct dibDataBusClient *, uint32_t addr, uint8_t v);

extern uint16_t data_bus_client_read16(struct dibDataBusClient *, uint32_t addr);
extern int data_bus_client_write16(struct dibDataBusClient *, uint32_t addr, uint16_t v);

extern uint16_t data_bus_client_read16_attr(struct dibDataBusClient *, uint32_t addr, uint16_t attr);
extern int data_bus_client_write16_attr(struct dibDataBusClient *, uint32_t addr, uint16_t v, uint16_t attr);

extern uint32_t data_bus_client_read32(struct dibDataBusClient *, uint32_t addr);
extern int data_bus_client_write32(struct dibDataBusClient *, uint32_t addr, uint32_t v);

extern int data_bus_client_read_multi32(struct dibDataBusClient *client, uint32_t addr, uint32_t *b, uint32_t nb);
extern int data_bus_client_write_multi32(struct dibDataBusClient *client, uint32_t addr, uint32_t *b, uint32_t nb);
extern int data_bus_client_write_multi32_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint32_t *b, uint32_t nb);
extern int data_bus_client_read_multi32_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint32_t *b, uint32_t nb);

extern int data_bus_client_access(struct dibDataBusClient *client, struct dibDataBusAccess *acc);
extern int data_bus_client_read_multi16_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint16_t *b, uint32_t nb);
extern int data_bus_client_read_multi16(struct dibDataBusClient *client, uint32_t addr, uint16_t *b, uint32_t nb);

extern int data_bus_client_write_multi16_attr(struct dibDataBusClient *client, uint32_t addr, uint16_t attr, uint16_t *b, uint32_t nb);
extern int data_bus_client_write_multi16(struct dibDataBusClient *client, uint32_t addr, uint16_t *b, uint32_t nb);

extern const struct dibDataBusClient data_bus_client_template_8b_;
extern const struct dibDataBusClient data_bus_client_template_8b_16b;
extern const struct dibDataBusClient data_bus_client_template_i2c_8b_16b;
extern const struct dibDataBusClient data_bus_client_template_0wire;

//! Initialize a DataBusClient
/**
 * A component driver is calling this function to initialize the DataBusClient.
 *
 * \param client pointer to a valid dibDataBusClient
 * \param tmpl   pointer to a valid template dibDataBusClient
 * \param bus    dibDataBusHost given by the initialization of the component
 */
extern int data_bus_client_init(struct dibDataBusClient *client, const struct dibDataBusClient *tmpl, struct dibDataBusHost *bus);


extern void data_bus_client_clone(struct dibDataBusClient *, struct dibDataBusClient *src);

//! Assign a DataBusHost
/**
 * Assign a dibDataBusHost to a client
 *
 * \param c valid dibDataBusClient
 * \param _host valid dibDataBusHost
 */
#define data_bus_client_set_data_bus(c, _host)    ((c)->host = _host)

//! Retrieve the DataBusHost of a client
/**
 *
 * \param c valid dibDataBusClient
 */
#define data_bus_client_get_data_bus(c) ((c)->host)

//! Set the device ID of a client
/**
 * Calling this function will set a (new) device ID for this client.
 * Device IDs are today only used in case of I2C and they represent
 * the I2C address in 8Bit notation
 *
 * \param c valid dibDataBusClient
 * \param dev_id new device ID
 */
#define data_bus_client_set_device_id(c, dev_id) (c)->device_id = dev_id

//! Get the device ID of a client
/**
 * Retrieve the device ID of a client. Don't use this function from inside
 * the transfer call of the host, use "data_bus_access_device_id" instead.
 *
 * \param c valid dibDataBusClient
 * \return the device of this DataBusClient
 */
#define data_bus_client_get_device_id(c)         ((c)->device_id)

//! Set the speed of a client
/**
 * Depending on the configuration of a component a different
 * clock-speed can be used to communicate with the device. A
 * component-driver can use this function to set a new speed.
 *
 * The meaning of the speed value depends also of the type of the DataBusHost.
 *
 * \param c valid dibDataBusClient
 * \param _speed number representing the speed
 */
#define data_bus_client_set_speed(c, _speed)     (c)->speed = _speed

//! Get the speed of a client
/**
 * In the transfer function a user wants to know what speed he can use
 * to talk to the hardware. This function can be used to retrieve this
 * information.
 *
 * \param c valid dibDataBusClient
 * \param msg dibDataBusAccess (optional)
 * \return the speed of the dataBusClient or the the real client of the gated Bus
 */
extern uint32_t data_bus_client_get_speed(struct dibDataBusClient *c, struct dibDataBusAccess *msg);
//((msg) != NULL && (msg)->first_client != 0 ? (msg)->first_client->speed : (c)->speed)

#define data_bus_client_set_gk_prop(c, p) (c)->gk_prop |= (p);
#define data_bus_client_get_gk_prop(c) ((c) != NULL ? (c)->gk_prop : 0)

#define data_bus_client_get_value_size(c, a) (((c) != NULL && (c)->value_size_by_addr != NULL) ? (c)->value_size_by_addr(c, a) : (c)->value_size)

#ifndef NO_I2C_ADAPTER_BACKWARD_COMPAT
/* folloing functions/structure are provided for backward compatibility */
#define dibI2CAdapter    dibDataBusHost
#define dibI2CGateKeeper dibDataBusClient

struct dibI2CAccess;

struct dibI2CMasterXfer
{
	int (*master_xfer) (struct dibDataBusHost *, struct dibI2CAccess *);
};

struct dibI2CAccess
{
	uint8_t addr;

	const uint8_t *tx;
	uint8_t txlen;

	uint8_t *rx;
	uint8_t rxlen;

	uint16_t scl_khz;

	int for_virtual_bus;
	struct dibDataBusClient *gate_keeper;
};

#define INIT_I2C_ACCESS_FOR_WRITE(acc,_addr,_tx,_txlen) \
	DibZeroMemory(&acc,sizeof(struct dibI2CAccess)); \
	acc.addr    = _addr; \
	acc.tx      = _tx; \
	acc.txlen   = _txlen; \
	acc.scl_khz = 340;

#define INIT_I2C_ACCESS_FOR_READ(acc,_addr,_tx,_txlen,_rx,_rxlen) \
	DibZeroMemory(&acc,sizeof(struct dibI2CAccess)); \
	acc.addr  = _addr; \
	acc.tx    = _tx; \
	acc.txlen = _txlen; \
	acc.rx    = _rx; \
	acc.rxlen = _rxlen; \
	acc.scl_khz = 340;

#define dibComponentRegisterAccess dibDataBusClient
#define INIT_COMPONENT_REGISTER_ACCESS(a,_reg_size,_val_size,_read,_write,_priv)

extern int  i2c_adapter_init(struct dibDataBusHost *, const struct dibI2CMasterXfer *);
#define i2c_adapter_exit data_bus_host_exit

#define i2c_adapter_wakeup(a) (DIB_RETURN_SUCCESS)
#define i2c_adapter_sleep(a)  (DIB_RETURN_SUCCESS)

extern int dib_i2c_transfer (struct dibDataBusHost *, struct dibI2CAccess *);
extern int i2c_transfer_to_control_a_gate(struct dibDataBusHost *, struct dibI2CAccess *);

#endif // NO_I2C_ADAPTER_BACKWARD_COMPAT

#ifdef __cplusplus
}
#endif

#endif
