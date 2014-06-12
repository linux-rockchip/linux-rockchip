#ifndef NPM_NMI310_APP_H
#define NPM_NMI310_APP_H
//#include<linux/types.h>

char NMI310_init();
void NMI310_Deinit();
void NMI310_tune(unsigned int frequency,char bandwidth);
char NMI310_lockstatus();
void NMI310_signal_strenth_quality(unsigned char *signal_quality, unsigned char *signal_strength);
void NMI310_refresh_signal_status(unsigned int frequency,char bandwidth);
int  NMI310_demod_pid_filter_ctrl(unsigned char onoff);
int  NMI310_demod_pid_filter(unsigned char id, unsigned short pid, unsigned char onoff);
#endif

