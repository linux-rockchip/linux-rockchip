#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include "scan.h"
#include "sample.h"
#include "interface/host.h"

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include "monitor/monitor.h"

static void store_monitor(struct dibChannel *ch,struct dibFrontend *fe, struct dibDemodMonitor *pmnt, uint8_t store, char* name, struct scan_config *cfg);
static void store_scanstatus(struct dibChannel *ch, int status,char* name,struct scan_config *cfg);
static void init_monitor(struct scan_config *cfg);
static void init_scanstatus(struct scan_config *cfg);
static void  fill_freq_list(uint32_t *freq_list, uint8_t *nb, uint8_t mode, uint32_t freq, char **freq_name);

void scanmonitor(struct dibFrontend *fe, uint8_t mode, struct scan_config *cfg)
{
    struct dibChannel ch;
    uint8_t i, j, freq_nb;
    double avgcn;
    struct dibDemodMonitor monit;
    uint32_t *freq_list;
    char **freq_name;

    if (mode == 0) {
        freq_list = DibMemAlloc(20*sizeof(unsigned int));
        freq_name = DibMemAlloc(20*sizeof(char *));
        for (i=0; i<20; i++)
            freq_name[i] = DibMemAlloc(100*sizeof(char));
    } else if (mode == 1) {
        freq_list = DibMemAlloc(60*sizeof(unsigned int));
        freq_name = DibMemAlloc(60*sizeof(char *));
        for (i=0; i<60; i++)
            freq_name[i] = DibMemAlloc(100*sizeof(char));
    } else if (mode == 2) {
        freq_list = DibMemAlloc(20*sizeof(unsigned int));
        freq_name = DibMemAlloc(20*sizeof(char *));
        for (i=0; i<20; i++)
            freq_name[i] = DibMemAlloc(100*sizeof(char));
    } else {
        freq_list = cfg->freq_list;
        freq_name = cfg->freq_name;
        freq_nb = cfg->freq_nb;
    }

    init_monitor(cfg);
    fill_freq_list(freq_list,&freq_nb, mode, cfg->freq, freq_name);

    for (j=0;j<freq_nb;j++) {
       avgcn = 0;
       INIT_CHANNEL(&ch, STANDARD_ISDBT);
       ch.RF_kHz = freq_list[j];
       ch.bandwidth_kHz = 6000;

       tune_diversity(fe, 1, &ch);
       printf("%d Khz => Status: %d\n", freq_list[j], frontend_get_status(&fe[0]));

       DibMSleep(1);

       for (i=0;i<cfg->nb_monitoring_loop;i++) {
         demod_get_monitoring(fe, &monit);
         store_monitor(&ch, fe, &monit, 0, freq_name[j], cfg);
         avgcn+=  monit.CoN;
         DibMSleep(100);
       }
       demod_get_monitoring(fe, &monit);
       avgcn +=  monit.CoN;
       avgcn  = avgcn / (cfg->nb_monitoring_loop+1);
       monit.CoN = avgcn;
       printf("AVG cn: %f\n", monit.CoN);
       store_monitor(&ch, fe, &monit, 1, freq_name[j], cfg);
    }

    if (mode <= 2) {
        free(freq_list);
        for (i=0; i<20; i++)
            free(freq_name[i]);
        free(freq_name);
    }
}

void scanstatus(struct dibFrontend *fe, uint8_t mode, struct scan_config *cfg)
{
    struct dibChannel ch;
    uint8_t i, j, freq_nb;
    uint32_t *freq_list;
    uint32_t *freq_status;
    char **freq_name;

    if (mode == 0) {
        freq_list = DibMemAlloc(20*sizeof(unsigned int));
        freq_status = DibMemAlloc(20*sizeof(unsigned int));
        freq_name = DibMemAlloc(20*sizeof(char *));
        for (i=0; i<20; i++)
            freq_name[i] = DibMemAlloc(100*sizeof(char));
    } else if (mode == 1) {
        freq_list = DibMemAlloc(60*sizeof(unsigned int));
        freq_status = DibMemAlloc(60*sizeof(unsigned int));
        freq_name = DibMemAlloc(60*sizeof(char *));
        for (i=0; i<60; i++)
            freq_name[i] = DibMemAlloc(100*sizeof(char));
    } else if (mode == 2) {
        freq_list = DibMemAlloc(20*sizeof(unsigned int));
        freq_status = DibMemAlloc(60*sizeof(unsigned int));
        freq_name = DibMemAlloc(20*sizeof(char *));
        for (i=0; i<20; i++)
            freq_name[i] = DibMemAlloc(100*sizeof(char));
    } else {
        freq_list = cfg->freq_list;
        freq_name = cfg->freq_name;
        freq_nb = cfg->freq_nb;
    }

    init_scanstatus(cfg);
    fill_freq_list(freq_list,&freq_nb, mode, cfg->freq, freq_name);

    for (j=0;j<freq_nb;j++) 
            freq_status[j]=0;

    for (i=0;i<cfg->nb_monitoring_loop;i++) {
       for (j=0;j<freq_nb;j++) {
          INIT_CHANNEL(&ch, STANDARD_ISDBT);
          ch.RF_kHz = freq_list[j];
          ch.bandwidth_kHz = 6000;

          tune_diversity(fe, 1, &ch);
          printf("%d Khz => Status: %d\n", freq_list[j], frontend_get_status(&fe[0]));
          if (frontend_get_status(&fe[0]) == FE_STATUS_DATA_LOCKED)
          {
             freq_status[j]++;
          }
       }
    }
    for (j=0;j<freq_nb;j++) {
       ch.RF_kHz = freq_list[j];
       store_scanstatus(&ch,freq_status[j],freq_name[j],cfg);
    }

    if (mode <= 2) {
        free(freq_list);
        for (i=0; i<20; i++)
            free(freq_name[i]);
        free(freq_name);
    }
}

static void init_scanstatus(struct scan_config *cfg)
{
  FILE *fd;
  fd = fopen(cfg->filename, "a");
  fprintf(fd, "SW,HW,Comment,Freq,Name,");
  fprintf(fd, "Tune Status");
  fprintf(fd, "\n");
  fclose(fd);
}

static void init_monitor(struct scan_config *cfg)
{
  FILE *fd;
  fd = fopen(cfg->filename, "a");
  fprintf(fd, "SW,HW,Comment,Freq,Name,Tune,Video,");
  fprintf(fd, "C/N,AGC Global,AGC RF,AGC BB, WBD, SPLIT,IPow,QPow,Freqoffset,Timingoffset,");
  fprintf(fd, "LockA,LockB,LockC,SynA,SynB,SynC,PerA,PerB,PerC,BerA,BerB,BerC,");
  fprintf(fd, "\n");
  fclose(fd);
}

static void store_scanstatus(struct dibChannel *ch, int status,char* name,struct scan_config *cfg)
{
  FILE *fd;
    fd = fopen(cfg->filename,"a");
  fprintf(fd, "%s,%s,%s,", cfg->sw_str, cfg->hw_str, cfg->comment_str);
  fprintf(fd, "%d,%s,",ch->RF_kHz, name);
  fprintf(fd, "%d",status);
  fprintf(fd, "\n");
  fclose(fd);
}
static void store_monitor(struct dibChannel *ch,struct dibFrontend *fe, struct dibDemodMonitor *pmnt, uint8_t store, char* name,struct scan_config *cfg) 
{
  FILE *fd;

  if (store == 1)
    fd = fopen(cfg->filename,"a");
  else
    fd = stdout;

  fprintf(fd, "%s,%s,%s,", cfg->sw_str, cfg->hw_str, cfg->comment_str);
  fprintf(fd, "%d,%s,",ch->RF_kHz, name);

  if ((pmnt->locks.fec_mpeg_b == 1) && (pmnt->PacketErrors_B == 0))
    fprintf(fd, "%d,G,",pmnt->locks.fec_mpeg);
  else if ((pmnt->locks.fec_mpeg_b == 1) && (pmnt->PacketErrors_B < 500))
    fprintf(fd, "%d,M,",pmnt->locks.fec_mpeg);
  else
    fprintf(fd, "%d,B,",pmnt->locks.fec_mpeg);

              
  fprintf(fd, "%5.4g,%5.2g,%5.4g,%5.4g,%5.4g,%5d,%5.2f,%5.2f,%5.3g,%5.4g,",
              pmnt->CoN,
              pmnt->rf_power/65536.0,
              pmnt->agc1,
              pmnt->agc2,
              pmnt->agc_wbd,
              pmnt->agc_split,
              pmnt->I_adc_power,
              pmnt->Q_adc_power,
              pmnt->carrier_offset,
              pmnt->timing_offset_ppm);
  fprintf(fd, "%d,%d,%d,%i,%i,%i,%i,%i,%i,%g,%g,%g",
              pmnt->locks.fec_mpeg,
              pmnt->locks.fec_mpeg_b,
              pmnt->locks.fec_mpeg_c,
              pmnt->viterbi_syndrome,
              pmnt->viterbi_syndrome_b,
              pmnt->viterbi_syndrome_c,
              pmnt->PacketErrors_A,
              pmnt->PacketErrors_B,
              pmnt->PacketErrors_C,
              pmnt->berA,
              pmnt->berB,
              pmnt->berC);

  fprintf(fd, "\n");

  if (store == 1)
    fclose(fd);
}

static void  fill_freq_list(uint32_t *freq_list, uint8_t *nb, uint8_t mode, uint32_t freq, char **freq_name)
{
  uint8_t i;

  if (mode == 0) {
    *nb = 20;
    for (i = 0; i < 20; i++) {
      freq_list[i] = freq;
      sprintf(freq_name[i], "%s", "");
    }
  }
  else if (mode == 1) {
    for (i = 0; i < 60; i++) {
      freq_list[i] = 473143 + 6000*i;
      sprintf(freq_name[i], "%s", "");
    }
    *nb = 50;
  }
  else if (mode == 2) {
      freq_list[0] = 533143;
      freq_list[1] = 557143;
      freq_list[2] = 497143;
      freq_list[3] = 509143;
      freq_list[4] = 563143;
      freq_list[5] = 491143;
      freq_list[6] = 527143;
      freq_list[7] = 479143;
      freq_list[8] = 521143;
      freq_list[9] = 569143;
      freq_list[10] = 575143;
      freq_list[11] = 587143;
      freq_list[12] = 623143;
      freq_list[13] = 647143;
      freq_list[14] = 671143;
      freq_list[15] = 749143;
      freq_list[16] = 755143;
      freq_list[17] = 767143;
      freq_list[18] = 773142;

      sprintf(freq_name[0],"%s","Cultura");
      sprintf(freq_name[1],"%s","SBT");
      sprintf(freq_name[2],"%s","Globo");
      sprintf(freq_name[3],"%s","Record");
      sprintf(freq_name[4],"%s","RedeTv");
      sprintf(freq_name[5],"%s","Gazeta");
      sprintf(freq_name[6],"%s","Band");
      sprintf(freq_name[7],"%s","MegaTv");
      sprintf(freq_name[8],"%s","Canal21");
      sprintf(freq_name[9],"%s","RIT");
      sprintf(freq_name[10],"%s","MTV");
      sprintf(freq_name[11],"%s","Abril");
      sprintf(freq_name[12],"%s","RedeVida");
      sprintf(freq_name[13],"%s","RecordNews");
      sprintf(freq_name[14],"%s","Ngt");
      sprintf(freq_name[15],"%s","Mackenzie");
      sprintf(freq_name[16],"%s","TVCamara");
      sprintf(freq_name[17],"%s","TVBrazil");
      sprintf(freq_name[18],"%s","TVJustica");

      *nb=19;
  }
}

