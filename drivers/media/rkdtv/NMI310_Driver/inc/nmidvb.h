#ifndef _NMIDVB_H_
#define _NMIDVB_H_

typedef struct {
#ifdef _HAVE_FLOAT_
	double	frequency;
#else
	long frequency;
#endif
	int bandwidth;
} DVBTUNE;

typedef struct {
	int lock;
} DVBRUN;

typedef struct
{
	int					mode;				/* dvb-h = 0, dvb-t = 1 */
	int					tsmode;			/* not use = 0, parallel = 1, serial = 2 */
	int					tsclk;				/* 0:40/5, 1:20/2.5, 2:10/1.25, 3:5/0.625 */
	int					tsgatclk;			/* TS gate off output clock, 0: no, 1: yes */
	int					tsdatpol;			/* TS data polarity, 0: non-inverted, 1: inverted */  
	int					tsvalpol;			/* TS valid polarity, 0: active high, 1: active low */
	int					tssynpol;			/* TS sync polarity, 0: active high, 1:active low */
	int					chanid;
	int					npid;
	uint16_t			pid[8];
	uint8_t				fec[8];
	uint8_t				type[8];			/* 0: inactive, 1: view, 2: favorite */
	int					enfilt[8];			/* 1: enable ip filtering, 0: otherwise */
	int 					nip4;				/* nmuber of ip4 address pass in */
	uint32_t 			ip4filtadr[12];	/* ip4 filter address */
	int 					nip6;				/* number of ip6 address pass in */
	uint32_t 			ip6filtadr[8];	/* ip6 filter address */
	int					entfilt;				/* enable DVB-T filter */
	int					ntfilt;				/* number of T-filter */
	uint16_t			tfilt[16];			/* DVB-T PID filter */
	int					tsz;					/* DVB-T DMA size */
	int					tskip;				/* DVB-T don't program PID filters */
} DVBLNKCTL;

typedef struct {
#ifdef _HAVE_FLOAT_
	double snr;
#else
	long snr;
#endif
} DVBSNR;

typedef struct {
#ifdef _HAVE_FLOAT_
	double ber;
	double beri;
#else
	long ber;
#endif
} DVBBER;

typedef struct {
#ifdef _HAVE_FLOAT_
	double per;
	double peri;
#else
	long per;
#endif
} DVBPER;

typedef struct {
	uint32_t Guard;
	uint32_t Mode;
	uint32_t Rate;
	uint32_t Conste;
	uint32_t Alpha;
} DVBMODE;

typedef struct {
	int		bAGCLock;
	int 		bSyrLock;
	int		bChcTrkLock;
	int		bTpsLock;
	int		bFecLock;
#ifdef _HAVE_FLOAT_
	double	dSnr;
	double dPer;
	double dPeri;
	double dBer;
	double dBeri;
	double dFreqOffset;
	double dTimeOffset;
#endif
	uint32_t agc;
	uint32_t	uRFAGC;
	int			rssi;
	int			lnaOn;
	uint32_t	chcdelay;
	uint32_t	freqfilter;
	int			alg;
	int			vgaupper;
	int			vgalower;
	int			rssiupper;
	int			rssilower;
} DVBSTA;

typedef struct {
	int	nPid;
	uint16_t Pid[16];
} DVBTFILTER;

typedef struct {
	int nIp4;
	uint32_t ip4Filt[12];
	int nIp6;
	uint32_t ip6Filt[8];
} DVBHIPFILTER;

typedef struct {
	uint16_t Pid;
	uint8_t	  Fec;
	uint8_t	  Type;		/* inactive, view, favoriate */
	int		  enFilt;
} DVBHPID;

typedef struct {

	int nPid;
	DVBHPID aPid[8];

} DVBHPIDCTL;

typedef struct
{
#ifdef _HAVE_FLOAT_
	double fer;
#else
	long fer;
#endif
} DVBFER;
typedef struct
{
#ifdef _HAVE_FLOAT_
	double mfer;
#else
	long mfer;
#endif
} DVBMFER;

typedef struct
{
#ifdef _HAVE_FLOAT_
	double		dFer;
	double		dMFer;
#else
	long dFer;
	long dMFer;
#endif
	int			nFrame;
} DVBLNKSTA;

typedef struct
{
	int szblk;
	int nblk;
	int tail;
	int last;
	uint32_t adr;
} DVBSDIODMA;

typedef struct
{
	uint32_t pid;
	uint32_t tid;
	int fulltbl;
	int ignorecrc;
	int notidmatch;

	int pidmatch;
	int tidmatch;
} DVBPSISITBL;

typedef struct
{
	uint32_t sadr;
	int sz;
} DVBTBLREAD;

typedef struct
{
	int 		tpslock;
#ifdef _HAVE_FLOAT_
	double frequency;					/* scan frequency */
#else
	long 	frequency;
#endif
	int		bandwidth;					/* scan bandwidth */

} DVBBANDSCAN;

typedef struct
{
	int dir;
	int bit;
	int level;

} DVBGIOCTL;

typedef struct
{
	int npid;
	uint32_t pid[16];
	
} DVBSETPID;

typedef struct
{
	int tcnt;
	uint32_t tadr;
} DVBEBIDMA;

typedef struct
{
	int rssilower;
	int rssiupper;
	int vgalower;
	int vgaupper;

} DVBAGCTHRESHOLD;
/**** 
	interrupt return status defines
****/
#define DVB_ASIC_DATA_READY				0x00000001
#define DVB_ASIC_TBL_READY				0x00000002
#define DVB_ASIC_BUF_OVERFLOW			0x00000004

#endif

