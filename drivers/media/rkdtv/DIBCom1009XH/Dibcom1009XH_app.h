#ifndef _DIB1009XH_XXXXXXXX
#define _DIB1009XH_XXXXXXXX

char DVB_Module_Init();
void DVB_Module_Tune(unsigned int frequency_KHz, unsigned int  bandwidth_KHz);
char DVB_Module_Lockstatus();
void DVB_Module_signal_strenth_quality(unsigned char *signal_quality, unsigned char *signal_strength);
void  DVB_demod_refresh_signal_status(unsigned int frequency,char bandwidth);
void DVB_demod_turn_with_no_delay(unsigned int frequency,char bandwidth);
void  DVB_demod_refresh_signal_status(unsigned int frequency,char bandwidth);

#endif

