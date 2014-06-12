#ifndef ADAPTER_MAC_H
#define ADAPTER_MAC_H

#include "common.h"
#include "databus.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dibMacMonitor;
struct dibDemodMonitor;

#define DLL_SH  1
#define DLL_MAC 2

//! generic description of a Mac
/**
 * This struct describes a MAC in a generic way. Each MAC-driver has to implement one.
 */
struct dibMac {
    const char name[MAX_INFO_NAME_LENGTH]; /*! Keeps a human-readable name as a string. */
    struct {
        int (*reset) (struct dibMac *);

        int (*wakeup) (struct dibMac *);
        int (*sleep) (struct dibMac *, uint8_t deep);

        int (*set_gpio) (struct dibMac *, uint8_t num, uint8_t dir, uint8_t val);

        int (*set_frequency) (struct dibMac * fe, uint32_t freq_ref, uint32_t freq_out, uint32_t select_dll_id, uint16_t divider);

        int (*release) (struct dibMac *);

        int (*set_output_mode)(struct dibMac *, int);
        int (*set_input_mode)(struct dibMac *, int);
        int (*get_monitoring)(struct dibMac *, struct dibMacMonitor *, struct dibDemodMonitor *);

        int (*download_code)(struct dibMac *, uint8_t *code, uint32_t size, uint8_t encrypted, uint32_t jump_address);   
    } ops;

    struct dibDataBusClient data_bus_client; /*! structure the mac uses to read from and write to itself (how to access the register) */
    void *priv;
};

extern void mac_init(struct dibMac *, const struct dibMac *, void *priv);

#define mac_get_name(mac) mac->name

/* stable API for dibMacOps - use this instead of the function pointers directly */
#define FE_MAC_FUNC_CHECK(obj, func)  FE_FUNC_CHECK(obj, mac, func)
#ifndef FE_MAC_FUNC_CALL
#define FE_MAC_FUNC_CALL(obj, func)   FE_FUNC_CALL(obj, mac, func)
#endif

//! Reset a mac
extern int mac_reset(struct dibMac *);

//! Wake up a mac
/**
 * Has to be called after sleep and before each tune - to put the mac into a defined state
 *
 * \param d pointer to a valid struct dibFrontend
 */
extern int mac_wakeup(struct dibMac *);

//! Software-suspend a mac.
/**
 * Put the mac to sleep per software. Note that the power consumption is
 * still higher than doing it by hardware. Temporary low-power-mode during
 * runtime.
 *
 * \param d pointer to a valid struct dibMac
 */
extern int mac_sleep(struct dibMac *, uint8_t deep);


//! Set output mode of the MAC.
/**
 * Set up the output_mode of a demod. Valid modes can be found in common.h . (e.g. OUTMODE_DIVERSITY, OUTMODE_MPEG2_SERIAL).
 *
 * \param d pointer to a valid struct dibMac
 * \param m output mode (default: OUTMODE_MPEG2_FIFO)
 */
extern int mac_set_output_mode(struct dibMac *, uint16_t mode);

//! Set intput mode of the MAC.
/**
 * Set up the intput_mode of a MAC.
 *
 * \param d pointer to a valid struct dibMac
 * \param m intput mode onoff
 */
extern int mac_set_input_mode(struct dibMac *, uint16_t mode);

//! Manipulate the GPIO of a mac
/**
 * Use this function to manipulate the state of a GPIO of the chip.
 *
 * \param d pointer to a valid struct dibMac
 * \param num number of GPIO to control (if in doubt do not touch at least GPIO1,2 and GPIO3,4)
 * \param dir GPIO direction 0 = OUT, 1 = IN
 * \param val state of the GPIO (valid only in direction out) -> 0 = low, 1 = high
 */
extern int mac_set_gpio(struct dibMac *, uint8_t num, uint8_t dir, uint8_t val);

extern int mac_set_frequency(struct dibMac * mac, uint32_t freq_ref, uint32_t freq_out, uint32_t select_dll_id, uint16_t divider);

/* for monitoring */
extern int mac_get_monitoring(struct dibMac *mac, struct dibMacMonitor *m_mac, struct dibDemodMonitor *m_demod);

/* for downloading a firmware */
extern int mac_download_code(struct dibMac * mac, uint8_t *code, uint32_t size, uint8_t encrypted, uint32_t jump_addr);

//! Free mac memory
/**
 * Calling the function cleans up and frees private memory.
 *
 * \param d pointer to a valid struct dibMac
 */
extern void mac_release(struct dibMac *);

#define mac_get_data_bus_client(d) ((d) != NULL ? &(d)->data_bus_client : NULL)

#ifdef __cplusplus
}
#endif

#endif
