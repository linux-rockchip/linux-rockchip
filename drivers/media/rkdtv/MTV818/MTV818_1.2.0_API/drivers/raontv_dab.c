



#include "raontv.h"
#include "raontv_rf.h"
#include "raontv_internal.h"


#ifdef RTV_DAB_ENABLE


void rtvDAB_StandbyMode(int on)
{
	if( on )
	{ 
		RTV_REG_MAP_SEL(RF_PAGE); 
		RTV_REG_MASK_SET(0x57,0x04, 0x04);  //SW PD ALL      
	}
	else
	{	  
		RTV_REG_MAP_SEL(RF_PAGE); 
		RTV_REG_MASK_SET(0x57,0x04, 0x00);  //SW PD ALL	
	}
}

INT rtvDAB_ScanFrequency(U32 dwChFreqKHz) 
{
	return RTV_SUCCESS; // if detected channel
}

INT rtvDAB_SetFrequency(U32 dwChFreqKHz)
{
	return rtvRF_SetFrequency(RTV_TV_MODE_DAB, dwChFreqKHz);
}


INT rtvDAB_Initialize(E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType)
{

	return RTV_SUCCESS;
}


#endif
