#ifndef DIBX000_I2C_MASTER_COMMON_H
#define DIBX000_I2C_MASTER_COMMON_H

#include <adapter/databus.h>
#include <adapter/common.h>
#include <demod/dibx000_common.h>

struct dibx000_i2c_master {
#define DIB3000MC 1
#define DIB7000   2
#define DIB7000P  11
#define DIB7000MC 12
#define DIB8000   13

	uint16_t device_rev;
	uint16_t base_reg;

	enum dibx000_i2c_interface selected_interface;

	/* for the virtual gated i2c_bus */
	struct dibDataBusHost tuner_i2c_adap;
	struct dibDataBusHost gpio_1_2_i2c_adap;
	struct dibDataBusHost gpio_3_4_i2c_adap;

	struct dibDataBusHost gated_tuner_i2c_adap;
	struct dibDataBusHost gated_gpio_1_2_i2c_adap;
	struct dibDataBusHost gated_gpio_3_4_i2c_adap;
	struct dibDataBusHost gated_gpio_6_7_i2c_adap;

	struct dibDataBusClient *data_bus_client;
    struct dibDataBusClient *gate_ctrl_client;
	uint32_t current_speed;
};

extern int dibx000_init_i2c_master(struct dibx000_i2c_master *mst, uint16_t device_rev, struct dibDataBusClient *client);
extern void dibx000_reset_i2c_master(struct dibx000_i2c_master *mst);
extern void dibx000_set_i2c_master_revision(struct dibx000_i2c_master *mst, uint16_t device_rev);

extern void dibx000_exit_i2c_master(struct dibx000_i2c_master *mst);
extern struct dibDataBusHost * dibx000_get_i2c_adapter(struct dibx000_i2c_master *mst, enum dibx000_i2c_interface intf, int gating);
extern void dibx000_i2c_master_set_scl(struct dibx000_i2c_master *mst, uint16_t scl);

extern void dibx000_set_gate_control_client(struct dibx000_i2c_master *, struct dibDataBusClient *client);

extern void dibx000_deep_sleep_i2c_master(struct dibx000_i2c_master *mst);

extern const struct dibDataBusClient dibx000_mmio_data_bus_client;
extern const struct dibDataBusClient dibx000_i2c_data_bus_client;

#endif
