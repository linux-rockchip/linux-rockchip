

#ifndef __COMBO_MT66XX_H__
#define __COMBO_MT66xx_H__

struct mtk_wmt_platform_data {
	/* GPIO pin config */
	int pmu;
	int rst;
	int bgf_int;
	int urt_cts;
	int rtc;
	int gps_sync;
	int gps_lna;

	/* kernel launcher parameter. */
	
};

struct mtk_sdio_eint_platform_data {
	/* used for sdio eint. */
	int sdio_eint;
};

#endif
