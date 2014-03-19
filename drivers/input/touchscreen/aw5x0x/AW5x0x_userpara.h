#ifndef AW5X0X_USERPARA_H

#define AW5X0X_USERPARA_H

typedef struct {
	unsigned short  CHIPVER;
	unsigned char	TX_LOCAL;	//					15		//TX number of TP
	unsigned char	RX_LOCAL;	//					10		//RX number of TP
	unsigned char	RX_ORDER[14];	// RX mapping in inverted order
	unsigned char	TX_ORDER[26];	// TX ORDER
	unsigned char	RX_START;	//RX START LINE
	unsigned char	HAVE_KEY_LINE;	// 0: no KEY line, 1: have key line on TX line TX_LOCAL-1
	unsigned char	KeyLineValid[16];

	unsigned short	MAPPING_MAX_X;	//   320
	unsigned short	MAPPING_MAX_Y;	//   460

	unsigned short	GainClbDeltaMin;	// Expected minimum delta for GAIN calibration
	unsigned short	GainClbDeltaMax;	// Expected maximum delta for GAIN calibration
	unsigned short	KeyLineDeltaMin;
	unsigned short	KeyLineDeltaMax;
	unsigned short	OffsetClbExpectedMin;	// Expected minimum data for OFFSET calibration
	unsigned short	OffsetClbExpectedMax;	// Expected minimum data for OFFSET calibration
	unsigned short	RawDataCheckMin;
	unsigned short	RawDataCheckMax;
	unsigned short	RawDataDeviation;	// Maximum deviation in a frame
	unsigned short	CacMultiCoef;

	unsigned char	GAIN_CLB_SEPERATE;

	unsigned char 	FIRST_CALI;

	unsigned short	GainTestDeltaMin;	// Expected minimum delta for GAIN calibration
	unsigned short	GainTestDeltaMax;	// Expected maximum delta for GAIN calibration
	unsigned short	KeyLineTestDeltaMin;
	unsigned short	KeyLineTestDeltaMax;

	unsigned char	MULTI_SCANFREQ;
	unsigned char	BASE_FREQ;
	unsigned char	FREQ_OFFSET;
	unsigned char	WAIT_TIME;
	unsigned char	CHAMP_CFG;
	unsigned char	POSLEVEL_TH;
	unsigned char	RAWDATA_DUMP_SWITCH;

	unsigned char	ESD_PROTECT;

	unsigned char	PEAK_TH;
	unsigned char	GROUP_TH;
	unsigned char	BIGAREA_TH;
	unsigned char	BIGAREA_CNT;
	unsigned char	BIGAREA_FRESHCNT;

	unsigned char	PEAK_ROW_COMPENSATE;
	unsigned char	PEAK_COL_COMPENSATE;
	unsigned char	PEAK_COMPENSATE_COEF;

	unsigned char	POINT_RELEASEHOLD;
	unsigned char	MARGIN_RELEASEHOLD;
	unsigned char	POINT_PRESSHOLD;
	unsigned char	KEY_PRESSHOLD;

	unsigned char	STABLE_DELTA_X;
	unsigned char	STABLE_DELTA_Y;
	unsigned char	FIRST_DELTA;


	unsigned char	CHARGE_PEAK_TH;
	unsigned char	CHARGE_GROUP_TH;
	unsigned char	CHARGE_BIGAREA_TH;

	unsigned char	CHARGE_PEAK_ROW_COMPENSATE;
	unsigned char	CHARGE_PEAK_COL_COMPENSATE;
	unsigned char	CHARGE_PEAK_COMPENSATE_COEF;

	unsigned char	CHARGE_POINT_RELEASEHOLD;
	unsigned char	CHARGE_MARGIN_RELEASEHOLD;
	unsigned char	CHARGE_POINT_PRESSHOLD;
	unsigned char	CHARGE_KEY_PRESSHOLD;

	unsigned char	CHARGE_STABLE_DELTA_X;
	unsigned char	CHARGE_STABLE_DELTA_Y;
	unsigned char	CHARGE_FIRST_DELTA;
	unsigned char	CHARGE_SECOND_HOLD;
	unsigned char	CHARGE_SECOND_DELTA;

	unsigned char	FREQ_JUMP;
	unsigned char	ID_LOOKUP;

	unsigned char	CACULATE_COEF;

	unsigned char	MARGIN_COMPENSATE;
	unsigned char	MARGIN_COMP_DATA_UP;
	unsigned char	MARGIN_COMP_DATA_DOWN;
	unsigned char	MARGIN_COMP_DATA_LEFT;
	unsigned char	MARGIN_COMP_DATA_RIGHT;

	unsigned short  FLYING_TH;
	unsigned short 	MOVING_TH;
	unsigned short 	MOVING_ACCELER;

	unsigned char	LCD_NOISE_PROCESS;
	unsigned char	LCD_NOISETH;
	
	unsigned char	FALSE_PEAK_PROCESS;
	unsigned char	FALSE_PEAK_TH;
	
	unsigned char	DEBUG_LEVEL;
	
	unsigned char	FAST_FRAME;
	unsigned char	SLOW_FRAME;

	unsigned char	MARGIN_PREFILTER;
	
	unsigned char	BIGAREA_HOLDPOINT;
	unsigned char	BASE_MODE;
	unsigned char	WATER_REMOVE;
	unsigned char	INT_MODE;

		
	unsigned char	PROXIMITY;
	unsigned char	PROXIMITY_LINE;
	unsigned char	PROXIMITY_TH_HIGH;
	unsigned char	PROXIMITY_TH_LOW;
	unsigned char	PROXIMITY_TIME;
	unsigned char	PROXIMITY_CNT_HIGH;
	unsigned char	PROXIMITY_CNT_LOW;
	unsigned char	PROXIMITY_TOUCH_TH_HIGH;
	unsigned char	PROXIMITY_TOUCH_TH_LOW;
	unsigned char	PROXIMITY_PEAK_CNT_HIGH;
	unsigned char	PROXIMITY_PEAK_CNT_LOW;
	unsigned char	PROXIMITY_LAST_TIME;
	unsigned char	PROXIMITY_TOUCH_TIME;
	unsigned char	PROXIMITY_SATUATION;


	unsigned char	CPFREQ;
	unsigned char	DRVVLT;
	unsigned char	TCLKDLY;
	unsigned char	ADCCFG;
	unsigned char	IBCFG1;
	unsigned char	IBCFG2;
	unsigned char	LDOCFG;
	unsigned char	ATEST1;
	unsigned char	ATEST2;
	unsigned char	SCANTIM;
	unsigned char	CARRNUM;
	unsigned char	I2CADDR;
	unsigned char	I2CADDRWP;
	unsigned char	TXDRVCFG;
	unsigned char	ADCDCR;
	unsigned char	CCR1;
	unsigned char	CCR2;
	unsigned char	TXOFFSET_CC[13];
	unsigned char	RXOFFSET_CC[7];
	
	unsigned char	REGISTER_LIST[16][3];
	unsigned char	CAMPRSTCR;
	
}AW5x0x_UCF;

void AW5x0x_User_Init(void);
void AW5x0x_User_Cfg1(void);

#endif
