#include "AW5x0x_Reg.h"
#include "AW5x0x_Drv.h"
#include <linux/string.h>
#include "AW5x0x_userpara.h"
#include "AW_UCF.h"

extern AW5x0x_UCF	AWTPCfg;
extern STRUCTCALI	AW_Cali;
extern char AW5x0x_WorkMode;
extern STRUCTNOISE		AW_Noise;

extern void AW5x0x_CLB_WriteCfg(void);
extern int AW_I2C_WriteByte(unsigned char addr, unsigned char data);
extern unsigned char AW_I2C_ReadByte(unsigned char addr);
extern unsigned char AW_I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len);
extern unsigned char AW5x0x_RAWDATACHK(void);
extern void AW_Sleep(unsigned int msec);

const STRUCTCALI Default_Cali1 = 
{
	"AWINIC TP CALI",
	AW_TXOFFSET,
	AW_RXOFFSET,
	AW_TXCAC,
	AW_RXCAC,
	AW_TXGAIN,
};

const AW5x0x_UCF Default_UCF = 
{
	AW_CHIPVER,
	AW_TX_LOCAL,	
	AW_RX_LOCAL,	
	AW_RX_ORDER,
	AW_TX_ORDER,
	AW_RX_START,
	AW_HAVE_KEY_LINE,
	AW_KeyLineValid,

	AW_MAPPING_MAX_X,                                                               
	AW_MAPPING_MAX_Y,                                                         
	                                                                                    
	AW_GainClbDeltaMin,                                                            
	AW_GainClbDeltaMax,                                                          
	AW_KeyLineDeltaMin,                                                           
	AW_KeyLineDeltaMax,                                                           
	AW_OffsetClbExpectedMin,                                                    
  	AW_OffsetClbExpectedMax,                                                      
	AW_RawDataCheckMin, 														  
	AW_RawDataCheckMax, 														  
	AW_RawDataDeviation,                                                           
	AW_CacMultiCoef,                                                              
                                                                                      
	AW_GAIN_CLB_SEPERATE,                                                            

	AW_FIRST_CALI,                                                                 

	AW_GainTestDeltaMin,                                                           
	AW_GainTestDeltaMax,                                                           
	AW_KeyLineTestDeltaMin,                                                         
	AW_KeyLineTestDeltaMax,                                                         

	AW_MULTI_SCANFREQ,                                                                
	AW_BASE_FREQ,                                                                 
	AW_FREQ_OFFSET,                                                            
	AW_WAIT_TIME,                                                               
	AW_CHAMP_CFG,                                                               
	AW_POSLEVEL_TH,                                                            
	AW_RAWDATA_DUMP_SWITCH, 														  

	AW_ESD_PROTECT,                                                               

	AW_PEAK_TH,                                                                  
	AW_GROUP_TH,                                                                 
  	AW_BIGAREA_TH,                                                                
	AW_BIGAREA_CNT,                                                               
	AW_BIGAREA_FRESHCNT,                                                              

	AW_PEAK_ROW_COMPENSATE,                                                        
	AW_PEAK_COL_COMPENSATE,                                                         
	AW_PEAK_COMPENSATE_COEF,                                                         

	AW_POINT_RELEASEHOLD,                                                           
	AW_MARGIN_RELEASEHOLD,                                                          
	AW_POINT_PRESSHOLD,                                                           
	AW_KEY_PRESSHOLD,                                                           

	AW_STABLE_DELTA_X,                                                            
	AW_STABLE_DELTA_Y,                                                              
	AW_FIRST_DELTA,                                                           

  	AW_CHARGE_PEAK_TH,                                                           
	AW_CHARGE_GROUP_TH,                                                           
	AW_CHARGE_BIGAREA_TH,                                                         
	                                                                                    
	AW_CHARGE_POINT_RELEASEHOLD,                                                   
	AW_CHARGE_MARGIN_RELEASEHOLD,                                                   
	AW_CHARGE_POINT_PRESSHOLD,                                                      
	AW_CHARGE_KEY_PRESSHOLD,                                                       
	                                                                                    
  	AW_CHARGE_PEAK_ROW_COMPENSATE,                                                     
	AW_CHARGE_PEAK_COL_COMPENSATE,                                                    
	AW_CHARGE_PEAK_COMPENSATE_COEF,                                                   
	                                                                                    
  	AW_CHARGE_STABLE_DELTA_X,                                                        
	AW_CHARGE_STABLE_DELTA_Y,                                                         
	AW_CHARGE_FIRST_DELTA, 
	AW_CHARGE_SECOND_HOLD,
	AW_CHARGE_SECOND_DELTA,

	AW_FREQ_JUMP,                                                              

	AW_ID_LOOKUP,

	AW_CACULATE_COEF,                                                               
	                                                                                    
	AW_MARGIN_COMPENSATE,                                                        
	AW_MARGIN_COMP_DATA_UP,                                                       
	AW_MARGIN_COMP_DATA_DOWN,                                                        
	AW_MARGIN_COMP_DATA_LEFT,                                                       
	AW_MARGIN_COMP_DATA_RIGHT,                                                        
	                                                                                    
  	AW_FLYING_TH,                                                               
	AW_MOVING_TH,                                                                 
	AW_MOVING_ACCELER,                                                               
	                                                                                    
	AW_LCD_NOISE_PROCESS,                                                            
	AW_LCD_NOISETH,                                                           
	                                                                                    
	AW_FALSE_PEAK_PROCESS,                                                            
	AW_FALSE_PEAK_TH,                                                            
	                                                                                    
	AW_DEBUG_LEVEL,                                                              
	                                                                                    
	AW_FAST_FRAME,                                                               
	AW_SLOW_FRAME,                                                                
	                                                                                    
	AW_MARGIN_PREFILTER,                                                             
	                                                                                    
	AW_BIGAREA_HOLDPOINT,                                                           
	AW_BASE_MODE,                                                             
	AW_WATER_REMOVE,                                                            
	AW_INT_MODE,
	                                                                                    
	AW_PROXIMITY,                                                                
	AW_PROXIMITY_LINE,                                                              
	AW_PROXIMITY_TH_HIGH,                                                           
	AW_PROXIMITY_TH_LOW,                                                           
	AW_PROXIMITY_TIME,                                                              
	AW_PROXIMITY_CNT_HIGH,                                                             
	AW_PROXIMITY_CNT_LOW,                                                            
  	AW_PROXIMITY_TOUCH_TH_HIGH,                                                      
	AW_PROXIMITY_TOUCH_TH_LOW,                                                       
	AW_PROXIMITY_PEAK_CNT_HIGH,                                                     
	AW_PROXIMITY_PEAK_CNT_LOW, 
	AW_PROXIMITY_LAST_TIME,
	AW_PROXIMITY_TOUCH_TIME,
	AW_PROXIMITY_SATUATION,                                                       
	                                                                                    
	AW_CPFREQ,                                                                   
	AW_DRVVLT,                                                                    
	AW_TCLKDLY,                                                                 
	AW_ADCCFG,                                                                  
	AW_IBCFG1,                                                                   
	AW_IBCFG2,                                                                   
	AW_LDOCFG,                                                                   
	AW_ATEST1,                                                                   
	AW_ATEST2,                                                                    
	AW_SCANTIM,                                                                  
	AW_CARRNUM,                                                                 
	AW_I2CADDR,                                                                  
	AW_I2CADDRWP,                                                                  
	AW_TXDRVCFG,                                                                  
	AW_ADCDCR,                                                                    
	AW_CCR1,                                                                   
	AW_CCR2,                                                                   
	AW_TXOFFSET_CC,  
	AW_RXOFFSET_CC,    
	AW_REGISTER_LIST, 
	AW_CAMPRSTCR															  
};                           
                                                                                      
void AW5x0x_User_Cfg1(void)                                                
{
	unsigned char i;

	AW_I2C_WriteByte(SA_PAGE,0);
	AW_I2C_WriteByte(SA_IDRST,0x55);
	AW_Sleep(5);

	if(AWTPCfg.TX_LOCAL > 21)
	{
		for(i=0;i<21;i++)
		{
			AW_I2C_WriteByte(SA_TX_INDEX0+i,AWTPCfg.TX_ORDER[i]);	//TX REVERT
		}

		AW_I2C_WriteByte(SA_PAGE,2);
		for(i=21;i<AWTPCfg.TX_LOCAL;i++)
		{
			AW_I2C_WriteByte(SA_TXINDEX21+(i-21),AWTPCfg.TX_ORDER[i]);	//TX REVERT
		}
		AW_I2C_WriteByte(SA_PAGE,0);
	}
	else
	{
		for(i=0;i<AWTPCfg.TX_LOCAL;i++)
		{
			AW_I2C_WriteByte(SA_TX_INDEX0+i,AWTPCfg.TX_ORDER[i]);	//TX REVERT
		}
	}

	AW_I2C_WriteByte(SA_TX_NUM,AWTPCfg.TX_LOCAL);
	AW_I2C_WriteByte(SA_RX_NUM,AWTPCfg.RX_LOCAL);

	if(AWTPCfg.MULTI_SCANFREQ & 1)
	{
		AW_I2C_WriteByte(SA_SCANFREQ1,AWTPCfg.BASE_FREQ);
		AW_I2C_WriteByte(SA_SCANFREQ2,AWTPCfg.BASE_FREQ);
		AW_I2C_WriteByte(SA_SCANFREQ3,AWTPCfg.BASE_FREQ);
	}
	else if(! (AWTPCfg.MULTI_SCANFREQ & 1))
	{
		AW_I2C_WriteByte(SA_SCANFREQ1,AWTPCfg.BASE_FREQ);		//3//-5
	}

	if(AWTPCfg.INT_MODE == 0)
	{
		AWTPCfg.WAIT_TIME = 0;
	}
	
	AW_I2C_WriteByte(SA_WAITTIME,AWTPCfg.WAIT_TIME);
	
	AW_I2C_WriteByte(SA_RX_START,AWTPCfg.RX_START);
	if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0)
		AW_I2C_WriteByte(SA_SCANTIM,4);		// set to 32 TX cycles mode		
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x10)
		AW_I2C_WriteByte(SA_SCANTIM,8);		// set to 64 TX cycles mode		
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x20)
	{
		AW_I2C_WriteByte(SA_SCANTIM,0x0C);	// set to 96 TX cycles mode		
		AW_I2C_WriteByte(SA_CARRNUM,96);	// set to 96 TX cycles mode		
	}
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x30)
	{
		AW_I2C_WriteByte(SA_SCANTIM,0x0C);	// set to 128 TX cycles mode 	
		AW_I2C_WriteByte(SA_CARRNUM,128);	// set to 128 TX cycles mode 	
	}
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x40)
	{
		AW_I2C_WriteByte(SA_SCANTIM,0x0C);	// set to 160 TX cycles mode 	
		AW_I2C_WriteByte(SA_CARRNUM,160);	// set to 160 TX cycles mode 	
	}
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x50)
	{
		AW_I2C_WriteByte(SA_SCANTIM,0x0C);	// set to 192 TX cycles mode 	
		AW_I2C_WriteByte(SA_CARRNUM,192);	// set to 192 TX cycles mode 	
	}
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x60)
	{
		AW_I2C_WriteByte(SA_SCANTIM,0x0C);	// set to 224 TX cycles mode 	
		AW_I2C_WriteByte(SA_CARRNUM,224);	// set to 224 TX cycles mode 	
	}
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x70)
	{
		AW_I2C_WriteByte(SA_SCANTIM,0x0C);	// set to 255 TX cycles mode 	
		AW_I2C_WriteByte(SA_CARRNUM,255);	// set to 255 TX cycles mode 	
	}
	else if((AWTPCfg.MULTI_SCANFREQ & 0xF0) == 0x80)
	{
		AW_I2C_WriteByte(SA_SCANTIM,0x0C);	// set to any TX cycles mode 	
		AW_I2C_WriteByte(SA_CARRNUM,AWTPCfg.CARRNUM);	// set to any TX cycles mode 	
	}

	if(AWTPCfg.CHIPVER == 259)
	{
		if(AWTPCfg.MULTI_SCANFREQ & 4)
			AW_I2C_WriteByte(SA_SCANTIM,(AW_I2C_ReadByte(SA_SCANTIM)|0x40));		// enable CAC CFB cycle reset
		else
			AW_I2C_WriteByte(SA_SCANTIM,(AW_I2C_ReadByte(SA_SCANTIM)&0xBF));		// disable CAC CFB cycle reset		
	}
	
	AW_I2C_WriteByte(SA_DRV_VLT, AWTPCfg.DRVVLT);

	AW_I2C_WriteByte(SA_PAGE,1);
	AW_I2C_WriteByte(SA_CHAMPCFG,AWTPCfg.CHAMP_CFG);	//
	AW_I2C_WriteByte(SA_OSCCFG1,AWTPCfg.FREQ_OFFSET);	//
	AW_I2C_WriteByte(SA_OSCCFG2,0x10);		//TRIM register
	AW_I2C_WriteByte(SA_POSLEVELTH,AWTPCfg.POSLEVEL_TH);

	AW_I2C_WriteByte(SA_CPFREQ,AWTPCfg.CPFREQ); 	
	AW_I2C_WriteByte(SA_ADCCFG,AWTPCfg.ADCCFG);
	AW_I2C_WriteByte(SA_IBCFG1,AWTPCfg.IBCFG1);
	AW_I2C_WriteByte(SA_IBCFG2,AWTPCfg.IBCFG2);
	AW_I2C_WriteByte(SA_LDOCFG,AWTPCfg.LDOCFG);

	if((AWTPCfg.CHIPVER == 256) || (AWTPCfg.CHIPVER == 259))
	{
		AW_I2C_WriteByte(SA_ATEST1,AWTPCfg.ATEST1);
		AW_I2C_WriteByte(SA_ATEST2,AWTPCfg.ATEST2);
		AW_I2C_WriteByte(SA_ADCDCR,AWTPCfg.ADCDCR);
		if(AWTPCfg.MULTI_SCANFREQ & 2)		
			AW_I2C_WriteByte(SA_CCR1,AWTPCfg.CCR1 | 0x10);
		else
			AW_I2C_WriteByte(SA_CCR1,AWTPCfg.CCR1);
		AW_I2C_WriteByte(SA_CCR2,AWTPCfg.CCR2);
		AW_I2C_WriteByte(SA_CAMPRSTCR,AWTPCfg.CAMPRSTCR);

		AW_I2C_WriteByte(SA_PAGE,2);		
		for(i=0;i<7;i++)
		{
			AW_I2C_WriteByte(SA_RXOFFSET_CC0+i,AWTPCfg.RXOFFSET_CC[i]);
		}
		for(i=0;i<13;i++)
		{
			AW_I2C_WriteByte(SA_TXOFFSET_CC0+i,AWTPCfg.TXOFFSET_CC[i]);
		}
	}

	for(i=0;i<16;i++)
	{
		if(AWTPCfg.REGISTER_LIST[i][1]!= 0)		//register != 0
		{
			AW_I2C_WriteByte(SA_PAGE,AWTPCfg.REGISTER_LIST[i][0]);
			AW_I2C_WriteByte(AWTPCfg.REGISTER_LIST[i][1],AWTPCfg.REGISTER_LIST[i][2]);
		}
		else
		{
			break;
		}
	}

	AW_I2C_WriteByte(SA_PAGE,0);
	AW5x0x_CLB_WriteCfg();	
}

void AW5x0x_User_Init(void)
{
	AW5x0x_WorkMode = DeltaMode;	//DeltaMode: chip output delta data  RawDataMode: chip output rawdata

	memcpy(&AWTPCfg,&Default_UCF,sizeof(AW5x0x_UCF));
	memcpy(&AW_Cali,&Default_Cali1,sizeof(STRUCTCALI));		//load default cali value

	AW5x0x_User_Cfg1();

	AW_Noise.FrmState = NOISE_FRM_NORMAL;
	AW_Noise.WorkFreqID = 16;
	AW_Noise.ScanFreqID = AW_Noise.WorkFreqID;
	AW_Noise.State = NOISE_LISTENING;
	AW_Noise.NoiseTh1 = 60;
	AW_Noise.JumpTh = 5;
	AW_Noise.Better_NoiseScan = 1000;
}
