/**
 * CopyRight: all rights reserved by rockchips fuzhou inc. 
 * 07.26.2010
 */

/**
 * adapt all modules 
 */

#ifndef TS_TV_H
#define TS_TV_H

struct TSTV_Module_t{
    const char* name;
    
    char (*init)(void);
    void (*deinit)(void);
    void (*tune)(unsigned int frequency, unsigned int bandwidth);
    char (*lockstatus)(void);
    int (*pid_filter)(unsigned char id, unsigned short pid, unsigned char onoff);
    int (*pid_filter_ctrl)(unsigned char onoff);
    void (*signal_strenth_quality)(char* signal_quality, char* signal_strength); 
    int (*i2c_addr)(void);
    unsigned short (*i2c_speed)(void);
    char (*frontend_init)(void);
    //unsigned int(*ts_readdata_ptr_estimat)(unsigned char* pwrite, unsigned char* pread, unsigned char* buf, unsigned int bufsize);
};

#endif //TS_TV_H

