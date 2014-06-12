#ifndef SCAN_H
#define SCAN_H
#include <adapter/frontend.h>
struct scan_config {
    const char *filename;
    const char *hw_str;
    const char *sw_str;
    const char *comment_str;
    uint8_t nb_monitoring_loop;

    /* for the mode 0 */
    uint32_t freq;

    /* for the mode > 2 */
    uint32_t *freq_list;
    char **freq_name;
    uint8_t freq_nb;
};

/**
  * This function is done to scan and stores the results in a file
  *
  * mode: the mode of scan
  * mode = 0: tune 20 times the frequency cfg->freq
  * mode = 1: scan the whole UHF band
  * mode = 2: scan a predifined frequency list (the frequency list for Sao-Paulo, Brazil
  * mode > 2: scan the frequency list given in the cfg structure
  * freq: the frequency used in mode=0
  * cfg: a configuration structure
  *   cfg->filename: the name of the file where to store the results
  *   cfg->hw_str: a string describing the hardware
  *   cfg->sw_str: a string giving the software version
  *   cfg->comment_str: a string to add some comments
  *   cfg->nb_monitoring_loop: the number of monitoring loop to be performed
  *   only for the mode = 0
  *   cfg->freq: the frequency used
  *   only for the mode > 2
  *   cfg->freq_list: the list of frequencies
  *   cfg->freq_name: the channel name for each frequencies
  *   cfg->FreqNb: the number of frequencies to be tested
  */
extern void scanmonitor(struct dibFrontend *fe, uint8_t mode, struct scan_config *cfg);

extern void scanstatus(struct dibFrontend *fe, uint8_t mode, struct scan_config *cfg);

#endif

