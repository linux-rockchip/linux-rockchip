#ifndef ADAPTER_BUSDESCRIPTOR_H
#define ADAPTER_BUSDESCRIPTOR_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DIR_IN  0
#define DIR_OUT 1
#define CTRL_XFER 0
#define BULK_XFER 1
struct dibUSBRequest {
	uint8_t type;
	uint8_t direction;

	uint8_t endpoint;
	uint32_t  timeout;

	struct {
		uint8_t  request;
		uint8_t  request_type;
		uint16_t value;
		uint16_t index;
	} setup;

	uint16_t length;
	uint8_t *buffer;
};

struct dibBoard;
struct dibI2CAccess;
struct dibEeprom;

#define USB_SERIAL_STRING_MAX_LENGTH 30
struct dibUSBDescriptor {
	uint16_t vendor_id;
	uint16_t product_id;
	uint32_t urb_max_size;
    uint8_t  interface_number;

	int (*control_xfer) (struct dibUSBDescriptor *, uint8_t endpoint, uint8_t direction,
			uint8_t request, uint16_t idx, uint16_t val, uint8_t *data, uint16_t size);

	int (*bulk_xfer)    (struct dibUSBDescriptor *, uint8_t endpoint, uint8_t direction, uint8_t *data, uint32_t size);

	int (*msg_queue)    (struct dibUSBDescriptor *, struct dibUSBRequest *, int count);

	void (*firmware_download_cb) (struct dibUSBDescriptor *usbdescriptor, struct dibEeprom *eeprom);
	void (*firmware_download_intermediate_cb) (struct dibUSBDescriptor *usbdescriptor, struct dibBoard *board);

	char serial_string[USB_SERIAL_STRING_MAX_LENGTH];

	void *priv;
};

struct dibPCIDescriptor {
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t subsystem_vid;
	uint16_t subsystem_pid;

	int (*i2c_access) (struct dibPCIDescriptor *, struct dibI2CAccess *);

	int (*get_gpio)   (struct dibPCIDescriptor *, uint8_t num, uint8_t *dir, uint8_t *val);
	int (*set_gpio)   (struct dibPCIDescriptor *, uint8_t num, uint8_t dir, uint8_t val);

/* not used for now */
	void (*write) (struct dibPCIDescriptor *, uint32_t address, uint32_t val);
	uint32_t (*read)  (struct dibPCIDescriptor *, uint32_t address);

	void (*wait_for_event) (struct dibPCIDescriptor *,int irq_mask);

	void *priv;
};

struct dibSPPDescriptor {
	int   (*check_permission) (const struct dibSPPDescriptor *, uint16_t addr);

	uint8_t (*inb)  (const struct dibSPPDescriptor *, uint16_t addr);
	void  (*outb) (const struct dibSPPDescriptor *, uint16_t addr, uint8_t val);

	void *priv;

};

#ifdef __cplusplus
}
#endif

#endif
