#ifndef ADAPTER_BUSADAPTER_H
#define ADAPTER_BUSADAPTER_H

#include <adapter/databus.h>

#ifdef __cplusplus
extern "C" {
#endif


struct dibBoard;
struct dibI2CAccess;

/* generic BusAdapter */

struct dibStreamingRequest {
    uint8_t id;
    uint8_t onoff;
    int   mode;
    int   video_standard;
};

struct dibPwmRequest {
    uint8_t demod_address;
    uint8_t pwm_enable;
    uint8_t get_status_enable;
    uint8_t use_force_value;
    uint16_t register_address;
    uint16_t pwm_forced_value;
};

struct dibBusAdapter;


#define BUS_ADAPTER_SERIAL_STRING_MAX_LENGTH 40
struct dibBusAdapterInfo {
	const char name[MAX_INFO_NAME_LENGTH];

	struct {
		int (*wakeup)        (struct dibBusAdapter *);
		int (*sleep)         (struct dibBusAdapter *);

		int (*get_gpio)      (struct dibBusAdapter *, uint8_t num, uint8_t *val, uint8_t *dir);

		int (*set_gpio)      (struct dibBusAdapter *, uint8_t num, uint8_t val, uint8_t dir);
		int (*set_streaming) (struct dibBusAdapter *, struct dibStreamingRequest *);
		int (*set_pwm)       (struct dibBusAdapter *, struct dibPwmRequest *);
        int (*set_scaling)   (struct dibBusAdapter *, struct dibVideoConfig *video_config, struct dibVideoTransitions *video_transitions);

		int (*set_pll)       (struct dibBusAdapter *, uint32_t freq_khz, uint8_t clock_gpio3);

		int (*memory_read)   (struct dibBusAdapter *, uint32_t addr, uint8_t mode, uint32_t cnt, uint8_t *buffer);
		int (*memory_write)  (struct dibBusAdapter *, uint32_t addr, uint8_t mode, uint32_t cnt, uint8_t *buffer);

		int (*set_def_data)  (struct dibBusAdapter *, uint16_t type);

        uint32_t (*streaming_mode) (struct dibBusAdapter *, uint32_t);

        struct dibDataBusHost * (*get_data_bus_host) (struct dibBusAdapter *, uint16_t type, uint16_t idx);

		void (*release)      (struct dibBusAdapter *);

	} ops;

};

struct dibBusAdapter
{
	struct dibBusAdapterInfo info;
	struct dibBoard          *board;

	char serial_string[BUS_ADAPTER_SERIAL_STRING_MAX_LENGTH];

	struct dibDataBusHost *i2c_adap;
	struct dibDataBusHost *default_data_bus;

	struct dibDataBusClient data_bus_client;

	void *priv;
};

extern int bus_adapter_sleep(struct dibBusAdapter *);
extern int bus_adapter_wakeup(struct dibBusAdapter *);

extern void bus_adapter_gpio_function(struct dibBusAdapter *bus, struct dibGPIOFunction *f);

extern const char * bus_adapter_name(struct dibBusAdapter *);

#define bus_adapter_get_gpio(b,n,v,d) \
	((b) != NULL && (b)->info.ops.get_gpio != NULL      ? (b)->info.ops.get_gpio(b,n,v,d) : DIB_RETURN_ERROR)

#define bus_adapter_set_gpio(b,n,v,d) \
	((b) != NULL && (b)->info.ops.set_gpio != NULL      ? (b)->info.ops.set_gpio(b,n,v,d) : DIB_RETURN_ERROR)

#define bus_adapter_set_streaming(b,s) \
	((b) != NULL && (b)->info.ops.set_streaming != NULL ? (b)->info.ops.set_streaming(b,s) : DIB_RETURN_ERROR)

#define bus_adapter_set_pwm(b,p) \
	((b) != NULL && (b)->info.ops.set_pwm != NULL       ? (b)->info.ops.set_pwm(b,p) : DIB_RETURN_ERROR)

#define bus_adapter_set_scaling(b,c,t) \
	((b) != NULL && (b)->info.ops.set_scaling != NULL   ? (b)->info.ops.set_scaling(b,c,t) : DIB_RETURN_ERROR)

#define bus_adapter_set_pll(b, f, gp3) \
	((b) != NULL && (b)->info.ops.set_pll != NULL       ? (b)->info.ops.set_pll(b,f,gp3) : DIB_RETURN_ERROR)

#define bus_adapter_memory_write(b, a, bytes, cnt, buf) \
	((b) != NULL && (b)->info.ops.memory_write != NULL  ? (b)->info.ops.memory_write(b,a,bytes,cnt,buf) : DIB_RETURN_ERROR)

#define bus_adapter_memory_read(b, a, bytes, cnt, buf) \
	((b) != NULL && (b)->info.ops.memory_read != NULL   ? (b)->info.ops.memory_read(b,a,bytes,cnt,buf) : DIB_RETURN_ERROR)

#define bus_adapter_set_default_data_bus(b, t) \
	((b) != NULL && (b)->info.ops.set_def_data != NULL   ? (b)->info.ops.set_def_data(b, t) : DIB_RETURN_ERROR)

extern struct dibDataBusHost * bus_adapter_get_default_data_bus(struct dibBusAdapter *);

extern struct dibDataBusHost * bus_adapter_get_data_bus(struct dibBusAdapter *, uint16_t type, uint16_t idx);

/* a bus_adapter knows for which mode of an adapter he needs/can handle which output mode */
extern uint32_t bus_adapter_streaming_mode(struct dibBusAdapter *, uint32_t mode);

#define bus_adapter_release(b) \
	do { if ((b) != NULL && (b)->info.ops.release != NULL) (b)->info.ops.release(b); } while (0)

	
#ifdef __cplusplus
}
#endif

#endif
