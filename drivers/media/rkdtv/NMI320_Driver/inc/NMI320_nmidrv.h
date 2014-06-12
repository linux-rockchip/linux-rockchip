#ifndef _NMI320_NMIDRV_DRV_H_
#define _NMI320_NMIDRV_DRV_H_

//#include "include.h"

/********************************************
	Internat Data Types
********************************************/

#include<linux/types.h> 
typedef double REAL;

#if 0

typedef   char int8_t;
typedef   short int16_t;
typedef   long int32_t;
typedef   long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

#ifndef _HAVE_FLOAT_
typedef unsigned long long 		uint64_t;
#endif
#endif



/********************************************
	ISDBT
********************************************/
#define _NMI_USE_ISDBT_

/********************************************
	Float/Fixed
********************************************/
//#define _HAVE_FLOAT_	


typedef enum {
	DVB = 1,
	ISDBT
} NMITV;


/********************************************
	Debug Flags
********************************************/
#define _INIT_			0x1
#define _ERR_			0x2
#define _WARN_			0x4
#define _FUNC_			0x8
#define _INFO_			0x10
#define _INTR_			0x20
#define _VERB_			0x40


/********************************************
	Bus Type Defines
********************************************/
typedef enum {
	_I2C_ = 1,
	_SPI1_,
	_SPI_,
	_USB_,
	_SDIO_,
	_EBI_,
} NMIBUSTYPE;

typedef enum {
	_EVAL_BOARD_ = 1,
	_USB_SDIO_DONGLE_,
	_USB_DONGLE_,
	_YUAN_DONGLE_,
} NMIBOARDTYPE;

typedef struct {
	int (*nmi_bus_init)(void *);
	void (*nmi_bus_deinit)(void *);
	int (*nmi_bus_read)(uint32_t, uint8_t *, uint32_t);
	int (*nmi_bus_write)(uint32_t, uint8_t *, uint32_t);
} NMIBUSVTBL;


typedef struct {
	int (*nmi_write_reg)(uint32_t, uint8_t *, uint32_t);
	int (*nmi_read_reg)(uint32_t, uint8_t *, uint32_t);
	void (*nmi_bus_mode)(uint8_t);
	void (*nmi_delay)(uint32_t);
	uint32_t (*nmi_get_tick)(void);
	void (*nmi_log)(char *);
	int (*nmi_dma_read)(void *);
} NMIHLPVTBL;

/********************************************
	Driver 
********************************************/
typedef struct {
	NMITV			dtvtype;
	NMIBUSTYPE 		bustype;
	uint32_t		dbgflag;
	int				crystalindex;
	int				tstype;
	NMIBOARDTYPE	boardtype;
	NMIHLPVTBL		tbl;
} NMICMN;


/********************************************
	Bus Functions
********************************************/
typedef int (*NMI_BUS_INIT) (void *);
typedef void (*NMI_BUS_DEINIT) (void *);
typedef int (*NMI_BUS_READ) (int, unsigned char*, int);
typedef int (*NMI_BUS_WRITE) (unsigned int, unsigned char *, unsigned int);



/********************************************
	Function VTable
********************************************/
typedef struct {
	int (*nmi_config_tuner)(void *);
	int (*nmi_config_demod)(void *);
	void (*nmi_config_mac)(void *);
	void (*nmi_get_snr)(void *);
	void (*nmi_get_ber)(void *);
	void (*nmi_get_per)(void *);
	void (*nmi_track_agc)(void *);
	uint32_t  (*nmi_get_chipid)(void);
	void (*nmi_rst_ber)(void);
	void (*nmi_rst_per)(void);
	void (*nmi_rf_write_reg)(uint32_t, uint8_t);
	unsigned char  (*nmi_rf_read_reg)(uint32_t);
	void (*nmi_soft_reset)(void);
	void (*nmi_fec_lock)(void *);
	void (*nmi_handle_intr)(void *);
	void (*nmi_get_freq_offset)(void *);
	void (*nmi_get_time_offset)(void *);
	int (*nmi_agc_lock)(void);
	int (*nmi_symbol_lock)(void);
	void (*nmi_get_rfagcgain)(void *);
	void (*nmi_set_rfagcgain)(void *);
	uint32_t (*nmi_get_vgagain)(void);
	// new functions
	int (*nmi_add_new_pid) (void *);
	void (*nmi_enable_pid_filter)(uint8_t);
	void (*nmi_enable_encrypt)(void *);
	void (*nmi_demod_enable)(uint8_t);
	unsigned char (*nmi_get_subchannel)(void *);
	void (*nmi_RF_powerdown)(void);
	void (*nmi_RF_powerup)(void);
	void (*nmi_enable_dma)(void *);
	void (*nmi_config_TS_output)(void *);
	void (*nmi_master_reset)(void);
	void (*nmi_start_isdbt)(void);
	void (*nmi_stop_isdbt)(void);
	void (*nmi_preempt_trackagc)(void);
	void (*nmi_get_signal_rssi)(void *);
	void (*nmi_get_ber_b4viterbi)(void *);
	void (*nmi_set_new_ber_timer)(void *);
	// 
	int (*nmi_tmc_lock)(void);
	void (*nmi_set_segment)(int);
	void (*nmi_check_soft_reset)(void);
	void (*nmi_get_channel_length)(void *);
	void (*nmi_config_extLNA_GPIO)(uint8_t);
	void (*nmi_get_coarsefreqoffset)(void *p);
	int (*nmi_scan_frequency)(void *p);
	void (*nmi_check_DRAM)(void);
	//void (*nmi_handle_intr)(void *);
} NMIDTVVTBL;


typedef struct {
	NMITV		dtvtype;
	NMIBUSTYPE 	bustype;
	uint32_t	dbg;
	int			CrystalIndex;
	int			BoardType;
	int			TSType;
	NMIHLPVTBL 	hlp;
	NMIDTVVTBL 	dtv;
} NMICHIP;


/********************************************
	ISDBT
********************************************/
typedef struct {
	int32_t	freq;
	int32_t subchannel;
} ISDBTTUNE;

typedef struct {
#ifdef _HAVE_FLOAT_
	double snr_a;
	double snr_b;
	double dmp_snr;
#else
	int32_t snr_a;
	int32_t snr_b;
	int32_t dmp_snr;
#endif
} ISDBTSNR;

typedef struct {
#ifdef _HAVE_FLOAT_
	double ber_a;
	double ber_b;
#else
	int32_t ber_a;
	int32_t ber_b;
#endif
} ISDBTBER;

typedef struct {
#ifdef _HAVE_FLOAT_
	double ber_a;
	double ber_b;
#else
	int32_t ber_a;
	int32_t ber_b;
#endif
	uint8_t overflow_bera;
	uint8_t overflow_berb;
}ISDBTBER_B4VITERBI;

typedef struct {
	uint32_t bertimer_a;
	uint32_t bertimer_b;
}ISDBT_BERTIMER;

typedef struct {
#ifdef _HAVE_FLOAT_
	double per_a;
	double per_b;
#else
	int32_t per_a;
	int32_t per_b;
#endif
} ISDBTPER;

typedef struct {
	int32_t fec_lock_a;
	int32_t fec_lock_b;
} ISDBTFEC;

typedef struct {
#ifdef _HAVE_FLOAT_
	double channelLength;
#else
	int32_t channelLength;
#endif
}ISDBTCHLEN;

typedef struct _ISDBT_3SEG_FREQ_ {
#ifdef _HAVE_FLOAT_
	double SideFreq1;
	double CentreFreq;
	double SideFreq2;
#else
	uint32_t SideFreq1;
	uint32_t CentreFreq;
	uint32_t SideFreq2;
#endif
}FREQ_3SEG,*PFREQ_3SEG;

typedef struct _ISDBT_3SEG_CHANNEL_ {
	uint8_t SideChannel1;
	uint8_t CenterChannel;
	uint8_t SideChannel2;
}CHANNEL_3SEG,*PCHANNEL_3SEG;

typedef struct _ISDBT_AGC_STATUS_ {
	int32_t ExtLNA;
	int32_t RSSI;
	int32_t RfAgcCurrent;
	int32_t IfVGAGain;
}ISDBTAGC;

typedef struct _TSO_CONFIG_ {
	uint8_t		ts_enable;
	uint8_t		ts_type;
	uint8_t		ts_clkrate;
	uint8_t		ts_gatedclk;
	uint8_t		ts_cfg;
	uint8_t		rs_error_intr_enable;
	uint8_t		rsvd;
} TSO_CONFIG, *PTSO_CONFIG;

typedef struct _PID_CONFIG_ {
	uint32_t		pid_num;
	uint8_t		rst_pid_table_index;
} PID_CONFIG,*PPID_CONFIG;

typedef struct _DMA_CONFIG_ {
	uint8_t		dma_enable;
	uint8_t		block_size;
	uint8_t		dma_cfg;
} DMA_CONFIG, *PDMA_CONFIG;

typedef struct {
	int32_t wrap;
} ISDBTSPIDMA;

typedef struct {
	int32_t wrap;
	int32_t szblk;
	int32_t nblk;
	int32_t size;
	int32_t last;
	uint32_t adr;
} ISDBTSDIODMA;


typedef struct _ENCRYPT_CONFIG_ {
	uint8_t		enable_config;
	uint8_t		enable_encrypt;
	uint8_t		key_select;
	uint32_t	key_w0;
	uint32_t	key_w1;
	uint32_t	key_w2;
	uint32_t	key_w3;
} ENCRYPT_CONFIG,*PENCRYPT_CONFIG;

typedef enum {
	_UHF_HIGH_GAIN_ = 1,
	_UHF_LOW_GAIN_,
	_VHF_HIGH_GAIN_,
	_VHF_LOW_GAIN_,
}EXT_LNA_GAIN;

typedef enum {
	_CH13_45_UHF_HIGH_GAIN_ = 1,
	_CH13_45_UHF_LOW_GAIN_,
	_CH46_62_UHF_HIGH_GAIN_,
	_CH46_62_UHF_LOW_GAIN_,
	_YUAN_VHF_HIGH_GAIN_,
	_YUAN_VHF_LOW_GAIN_,
}YUAN_EXT_LNA_GAIN;

typedef enum {
	_12_MHz	= 0,
	_13_MHz,
	_19_2_MHz,
	_19_68_MHz,
	_19_8_MHz,
	_26_MHz,
	_27_MHz,
	_32_MHz,
}CRYSTAL_FREQ_SELECT;

typedef enum {
	TS_PARALLEL		= 0,
	TS_SERIAL,
}TS_TYPE;

//Crystal frequencies currently supported
#define CFREQ_12_0_0	12
#define CFREQ_13_0_0	13
#define CFREQ_19_2_0	192
#define CFREQ_19_68_0	1968
#define CFREQ_19_8_0	198
#define CFREQ_26_0_0	26
#define CFREQ_27_0_0	27
#define CFREQ_32_0_0    32

//This is changed depending on the real crystal frequency
#define CRYSTAL_FREQUENCY CFREQ_26_0_0

#if   (CRYSTAL_FREQUENCY == CFREQ_12_0_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0xB2F4FC08)

#elif (CRYSTAL_FREQUENCY == CFREQ_13_0_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0xA530E8A5)

#elif (CRYSTAL_FREQUENCY == CFREQ_19_2_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0xDFB23B09)

#elif (CRYSTAL_FREQUENCY == CFREQ_19_68_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0xDA3D7E48)

#elif (CRYSTAL_FREQUENCY == CFREQ_19_8_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0xD8EAE3EA)

#elif (CRYSTAL_FREQUENCY == CFREQ_26_0_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0xA530E8A5)

#elif (CRYSTAL_FREQUENCY == CFREQ_27_0_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0x9F12A723)

#elif (CRYSTAL_FREQUENCY == CFREQ_32_0_0)
#define _FRQUENCY_CONVERSION_
#define CRYSTAL_BINARY_FACTOR  (0x8637BD06)
#endif

#define RF_CALIBRATION_VALUE	48



#endif

