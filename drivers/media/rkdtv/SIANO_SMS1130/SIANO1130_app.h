#ifndef SIANO_SMS1150_APP_H
#define SIANO_SMS1150_APP_H


char SIANO1130_Module_Init();
void SIANO1130_Module_Tune(unsigned int frequency,char bandwidth);
char SIANO1130_Module_Lockstatus();
void SIANO1130_Module_signal_strenth_quality(char *signal_quality, char *signal_strength);
#endif

