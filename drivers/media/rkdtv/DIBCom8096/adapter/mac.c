#include <adapter/mac.h>

void mac_init(struct dibMac *target, const struct dibMac *src, void *priv)
{
    memcpy(target, src, sizeof(*target));
    target->priv = priv;
}

int mac_reset(struct dibMac *mac)
{
    if (mac && mac->ops.reset)
        return mac->ops.reset(mac);
    return DIB_RETURN_SUCCESS;
}

int mac_wakeup(struct dibMac *mac)
{
    if (mac && mac->ops.wakeup)
        return mac->ops.wakeup(mac);
    return DIB_RETURN_SUCCESS;
}

int mac_sleep(struct dibMac *mac, uint8_t deep)
{
    if (mac && mac->ops.sleep)
        return mac->ops.sleep(mac, deep);
    return DIB_RETURN_SUCCESS;
}

int mac_set_gpio(struct dibMac *mac, uint8_t num, uint8_t dir, uint8_t val)
{
    if (mac && mac->ops.set_gpio)
        return mac->ops.set_gpio(mac, num, dir, val);
    return DIB_RETURN_SUCCESS;
}

#ifdef CONFIG_BUILD_HOST
int mac_set_frequency(struct dibMac * mac, uint32_t freq_ref, uint32_t freq_out, uint32_t select_dll_id, uint16_t divider)
{
    if (mac && mac->ops.set_frequency)
        return mac->ops.set_frequency(mac, freq_ref, freq_out, select_dll_id, divider);
    return DIB_RETURN_SUCCESS;
}
#endif

int mac_set_output_mode(struct dibMac *mac, uint16_t mode)
{
    if (mac != NULL && mac->ops.set_output_mode)
        return mac->ops.set_output_mode(mac, mode);
    return DIB_RETURN_SUCCESS;
}

#ifdef CONFIG_BUILD_HOST
int mac_download_code(struct dibMac * mac, uint8_t *code, uint32_t size, uint8_t encrypted, uint32_t jump_addr)
{
    if (mac != NULL && mac->ops.download_code)
        return mac->ops.download_code(mac, code, size, encrypted, jump_addr);
    return DIB_RETURN_SUCCESS;
}
#endif
void mac_release(struct dibMac *mac)
{
    if (mac != NULL && mac->ops.release)
        mac->ops.release(mac);
}

int mac_set_input_mode(struct dibMac *mac, uint16_t mode)
{
    if (mac != NULL && mac->ops.set_input_mode)
        return mac->ops.set_input_mode(mac, mode);
    return DIB_RETURN_SUCCESS;
}
#ifdef CONFIG_BUILD_HOST
int mac_get_monitoring(struct dibMac *mac, struct dibMacMonitor *m_mac , struct dibDemodMonitor *m_demod)
{
    if (mac != NULL && mac->ops.get_monitoring)
        return mac->ops.get_monitoring(mac, m_mac, m_demod);
    return DIB_RETURN_SUCCESS;
}
#endif
