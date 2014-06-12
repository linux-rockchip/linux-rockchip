#ifndef NPM_NMI320_APP_H
#define NPM_NMI320_APP_H


char DVB_Module_Init();
void DVB_Module_Tune(unsigned int frequency,char bandwidth);
char DVB_Module_Lockstatus();
void DVB_Module_signal_strenth_quality(char *signal_quality, char *signal_strength);
#endif

