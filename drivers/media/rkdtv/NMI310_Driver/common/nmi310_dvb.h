#ifndef NMI310_DVB_H
#define NMI310_DVB_H
#include<linux/types.h>
void nmi_delay(uint32_t msec);
unsigned int nmi_get_tick(void);
uint32_t rReg32(uint32_t adr);
void wReg32(uint32_t adr, uint32_t val);
void dvb_t_filter_remove_pid(uint32_t pid);
void dvb_t_filter_add_pid(uint32_t pid);
#endif/*NMI310_DVB_H*/
