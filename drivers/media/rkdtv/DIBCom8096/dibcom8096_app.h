#ifndef DIB8096_XXXXXXXX
#define DIB8096_XXXXXXXX

#include <include/adapter/8096_tuner.h>
#include <include/adapter/8096_demod.h>

extern struct dibI2CAdapter * open_dibcom_i2c();

char DVB_Module_Init();
void DVB_Module_Tune(unsigned int frequency,char bandwidth);
char DVB_Module_Lockstatus();
void DVB_Module_signal_strenth_quality(unsigned char *signal_quality, unsigned char *signal_strength);
void  DVB_demod_refresh_signal_status(unsigned int frequency,char bandwidth);
void DVB_demod_turn_with_no_delay(unsigned int frequency,char bandwidth);
void  DVB_demod_refresh_signal_status(unsigned int frequency,char bandwidth);

#endif

