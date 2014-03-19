#include <linux/delay.h>
#include <asm/io.h>
#include <mach/io.h>
#include "rk3028a_hdmi.h"
#include "rk3028a_hdmi_hw.h"
#include "rk3028a_hdmi_cec.h"

static char edid_result = 0;

static inline void delay100us(void)
{
	udelay(100);
}

static void rk3028a_hdmi_set_pwr_mode(struct rk3028a_hdmi* rk3028a_hdmi, int mode)
{
	int temp;
	
	RK3028ADBG("%s mode %d\n",__FUNCTION__, mode);
	if(rk3028a_hdmi->pwr_mode == mode)
		return; 
    switch(mode){
     case NORMAL:
//		HDMIMskReg(temp, SYS_CTRL, m_POWER, v_PWR_OFF);
//		if(rk3028a_hdmi->tmdsclk > 100000000) {
//			HDMIWrReg(PHY_DRIVER,0xcc);
//			HDMIWrReg(PHY_PRE_EMPHASIS,0x4f);
//		} else {
//			HDMIWrReg(PHY_DRIVER,0xaa);
//			HDMIWrReg(PHY_PRE_EMPHASIS,0x0f);
//		}
		HDMIWrReg(PHY_SYS_CTL,0x2d);
		HDMIWrReg(PHY_SYS_CTL,0x2c);
		HDMIWrReg(PHY_SYS_CTL,0x28);
		HDMIWrReg(PHY_SYS_CTL,0x20);
		HDMIWrReg(PHY_CHG_PWR,0x0f);
	   	HDMIWrReg(0xce, 0x00);
		HDMIWrReg(0xce, 0x01);
		HDMIMskReg(temp, SYS_CTRL, m_POWER, v_PWR_ON);
		break;
	case LOWER_PWR:
		HDMIWrReg(AV_MUTE, v_AUDIO_MUTE(0) | v_VIDEO_MUTE(0));
		HDMIWrReg(PHY_DRIVER, 0x00);
		HDMIWrReg(PHY_PRE_EMPHASIS,0x00);
		HDMIWrReg(PHY_CHG_PWR,0x00);
		HDMIWrReg(PHY_SYS_CTL,0x2f);
		HDMIMskReg(temp, SYS_CTRL, m_POWER | m_REG_CLK_SOURCE, v_PWR_OFF | v_REG_CLK_SOURCE_SYS);
		break;
	default:
	    RK3028ADBG("unkown rk3028a hdmi pwr mode %d\n",mode);
    }
	rk3028a_hdmi->pwr_mode = mode;
}


int rk3028a_hdmi_detect_hotplug(struct hdmi *hdmi)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;
	int value =	HDMIRdReg(HDMI_STATUS);
	
	RK3028ADBG( "[%s] value %02x\n", __FUNCTION__, value);
	value &= m_HOTPLUG;
	if(value == m_HOTPLUG)
		return HDMI_HPD_ACTIVED;
	else if(value)
		return HDMI_HPD_INSERT;
	else
		return HDMI_HPD_REMOVED;
}

#define HDMI_SCL_RATE            (100*1000)

int rk3028a_hdmi_read_edid(struct hdmi *hdmi, int block, unsigned char *buff)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;
	int value, ret = -1, ddc_bus_freq = 0;
	char interrupt = 0, trytime = 2;
	unsigned long flags;
	unsigned int sysclk;
	
	RK3028ADBG( "[%s] block %d\n", __FUNCTION__, block);
	spin_lock_irqsave(&rk3028a_hdmi->irq_lock, flags);
	edid_result = 0;
	spin_unlock_irqrestore(&rk3028a_hdmi->irq_lock, flags);

	sysclk = clk_get_rate(rk3028a_hdmi->hclk);
//	printk("sys clk is %u\n", sysclk);
	ddc_bus_freq = (sysclk >> 2)/HDMI_SCL_RATE; 
	HDMIWrReg(DDC_CLK_L, ddc_bus_freq & 0xFF);
	HDMIWrReg(DDC_CLK_H, (ddc_bus_freq >> 8) & 0xFF);
	
	// Enable edid interrupt
	HDMIWrReg(INTERRUPT_MASK1, m_INT_HOTPLUG | m_INT_EDID_READY);
	
	while(trytime--) {
		// Config EDID block and segment addr
		HDMIWrReg(EDID_WORD_ADDR, (block%2) * 0x80);
		HDMIWrReg(EDID_SEGMENT_POINTER, block/2);	
	
		value = 10;
		while(value--)
		{
			spin_lock_irqsave(&rk3028a_hdmi->irq_lock, flags);
			interrupt = edid_result;
			edid_result = 0;
			spin_unlock_irqrestore(&rk3028a_hdmi->irq_lock, flags);
			if(interrupt & (m_INT_EDID_READY))
				break;
			msleep(10);
		}
		RK3028ADBG( "[%s] edid read value %d\n", __FUNCTION__, value);
		if(interrupt & m_INT_EDID_READY)
		{
			for(value = 0; value < HDMI_EDID_BLOCK_SIZE; value++) 
				buff[value] = HDMIRdReg(EDID_FIFO_ADDR);
			ret = 0;
			
			RK3028ADBG( "[%s] edid read sucess\n", __FUNCTION__);
#ifdef DEBUG
			for(value = 0; value < 128; value++) {
				printk("0x%02x ,", buff[value]);
				if( (value + 1) % 16 == 0)
					printk("\n");
			}
#endif
			break;
		}else
			dev_err(hdmi->dev, "[%s] edid read error\n", __FUNCTION__);
			
		RK3028ADBG( "[%s] edid try times %d\n", __FUNCTION__, trytime);
		msleep(100);
	}
	// Disable edid interrupt
	HDMIWrReg(INTERRUPT_MASK1, m_INT_HOTPLUG);
//	msleep(100);
	return ret;
}

static void rk3028a_hdmi_config_avi(struct rk3028a_hdmi *rk3028a_hdmi, unsigned char vic, unsigned char output_color)
{
	int i;
	char info[SIZE_AVI_INFOFRAME];
	
	memset(info, 0, SIZE_AVI_INFOFRAME);
	HDMIWrReg(CONTROL_PACKET_BUF_INDEX, INFOFRAME_AVI);
	info[0] = 0x82;
	info[1] = 0x02;
	info[2] = 0x0D;	
	info[3] = info[0] + info[1] + info[2];
	info[4] = (AVI_COLOR_MODE_RGB << 5);
	info[5] = (AVI_COLORIMETRY_NO_DATA << 6) | (AVI_CODED_FRAME_ASPECT_NO_DATA << 4) | ACTIVE_ASPECT_RATE_SAME_AS_CODED_FRAME;
	info[6] = 0;
	info[7] = vic;
	info[8] = 0;

	// Calculate AVI InfoFrame ChecKsum
	for (i = 4; i < SIZE_AVI_INFOFRAME; i++)
	{
    	info[3] += info[i];
	}
	info[3] = 0x100 - info[3];
	
	for(i = 0; i < SIZE_AVI_INFOFRAME; i++)
		HDMIWrReg(CONTROL_PACKET_ADDR + i, info[i]);
}

int rk3028a_hdmi_config_video(struct hdmi *hdmi, struct hdmi_video *vpara)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;
	int value;
	struct fb_videomode *mode;
	
	RK3028ADBG( "[%s]\n", __FUNCTION__);
	if(vpara == NULL) {
		dev_err(hdmi->dev, "[%s] input parameter error\n", __FUNCTION__);
		return -1;
	}
	
	if(hdmi->ops->hdcp_power_off_cb)
		hdmi->ops->hdcp_power_off_cb();
		// Diable video and audio output
	HDMIWrReg(AV_MUTE, v_AUDIO_MUTE(1) | v_VIDEO_MUTE(1));
	
	// Input video mode is SDR RGB24bit, Data enable signal from external
	HDMIWrReg(VIDEO_CONTRL1, v_VIDEO_INPUT_FORMAT(VIDEO_INPUT_SDR_RGB444) | v_DE_EXTERNAL);
	HDMIWrReg(VIDEO_CONTRL2, v_VIDEO_INPUT_BITS(VIDEO_INPUT_8BITS) | (vpara->color_output & 0xFF));

	// Set HDMI Mode
	HDMIWrReg(HDCP_CTRL, v_HDMI_DVI(vpara->sink_hdmi));

	// Enable or disalbe color space convert
	if(vpara->color_input != vpara->color_output) {
		value = v_SOF_DISABLE | v_CSC_ENABLE;
	}
	else
		value = v_SOF_DISABLE;
	HDMIWrReg(VIDEO_CONTRL3, value);

	// Set ext video
#if 1
	HDMIWrReg(VIDEO_TIMING_CTL, 0);
	mode = (struct fb_videomode *)hdmi_vic_to_videomode(vpara->vic);
	if(mode == NULL)
	{
		dev_err(hdmi->dev, "[%s] not found vic %d\n", __FUNCTION__, vpara->vic);
		return -ENOENT;
	}
	rk3028a_hdmi->tmdsclk = mode->pixclock;
	
	if(rk3028a_hdmi->tmdsclk > 100000000) {
		HDMIWrReg(PHY_DRIVER,0xcc);
		HDMIWrReg(PHY_PRE_EMPHASIS,0x4f);
	} else {
		HDMIWrReg(PHY_DRIVER,0xaa);
		HDMIWrReg(PHY_PRE_EMPHASIS,0x0f);
	}
#else
	value = v_EXTERANL_VIDEO(1) | v_INETLACE(mode->vmode);
	if(mode->sync & FB_SYNC_HOR_HIGH_ACT)
		value |= v_HSYNC_POLARITY(1);
	if(mode->sync & FB_SYNC_VERT_HIGH_ACT)
		value |= v_VSYNC_POLARITY(1);
	HDMIWrReg(VIDEO_TIMING_CTL, value);
	
	value = mode->left_margin + mode->xres + mode->right_margin + mode->hsync_len;
	HDMIWrReg(VIDEO_EXT_HTOTAL_L, value & 0xFF);
	HDMIWrReg(VIDEO_EXT_HTOTAL_H, (value >> 8) & 0xFF);
	
	value = mode->left_margin + mode->right_margin + mode->hsync_len;
	HDMIWrReg(VIDEO_EXT_HBLANK_L, value & 0xFF);
	HDMIWrReg(VIDEO_EXT_HBLANK_H, (value >> 8) & 0xFF);
	
	value = mode->left_margin + mode->hsync_len;
	HDMIWrReg(VIDEO_EXT_HDELAY_L, value & 0xFF);
	HDMIWrReg(VIDEO_EXT_HDELAY_H, (value >> 8) & 0xFF);
	
	value = mode->hsync_len;
	HDMIWrReg(VIDEO_EXT_HDURATION_L, value & 0xFF);
	HDMIWrReg(VIDEO_EXT_HDURATION_H, (value >> 8) & 0xFF);
	
	value = mode->upper_margin + mode->yres + mode->lower_margin + mode->vsync_len;
	HDMIWrReg(VIDEO_EXT_VTOTAL_L, value & 0xFF);
	HDMIWrReg(VIDEO_EXT_VTOTAL_H, (value >> 8) & 0xFF);
	
	value = mode->upper_margin + mode->vsync_len + mode->lower_margin;
	HDMIWrReg(VIDEO_EXT_VBLANK, value & 0xFF);

	if(vpara->vic == HDMI_720x480p_60Hz_4_3 || vpara->vic == HDMI_720x480p_60Hz_16_9)
		value = 42;
	else
		value = mode->upper_margin + mode->vsync_len;

	HDMIWrReg(VIDEO_EXT_VDELAY, value & 0xFF);
	
	value = mode->vsync_len;
	HDMIWrReg(VIDEO_EXT_VDURATION, value & 0xFF);
#endif
	
	if(vpara->sink_hdmi == OUTPUT_HDMI) {
		rk3028a_hdmi_config_avi(rk3028a_hdmi, vpara->vic, vpara->color_output);
		RK3028ADBG( "[%s] sucess output HDMI.\n", __FUNCTION__);
	}
	else {
		RK3028ADBG( "[%s] sucess output DVI.\n", __FUNCTION__);	
	}
	
	if(rk3028a_hdmi->tmdsclk >= 148500000) {
		HDMIWrReg(0xe3, 0x4f);
	}
	else {
		HDMIWrReg(0xe3, 0x0f);
	}
	return 0;
}

static void rk3028a_hdmi_config_aai(struct rk3028a_hdmi *rk3028a_hdmi)
{
	int i;
	char info[SIZE_AUDIO_INFOFRAME];
	
	memset(info, 0, SIZE_AUDIO_INFOFRAME);
	
	info[0] = 0x84;
	info[1] = 0x01;
	info[2] = 0x0A;
	
	info[3] = info[0] + info[1] + info[2];	
	for (i = 4; i < SIZE_AUDIO_INFOFRAME; i++)
    	info[3] += info[i];
    	
	info[3] = 0x100 - info[3];
	
	HDMIWrReg(CONTROL_PACKET_BUF_INDEX, INFOFRAME_AAI);
	for(i = 0; i < SIZE_AUDIO_INFOFRAME; i++)
		HDMIWrReg(CONTROL_PACKET_ADDR + i, info[i]);
}

int rk3028a_hdmi_config_audio(struct hdmi *hdmi, struct hdmi_audio *audio)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;
	int rate, N, channel, mclk_fs;
	
	if(audio->channel < 3)
		channel = I2S_CHANNEL_1_2;
	else if(audio->channel < 5)
		channel = I2S_CHANNEL_3_4;
	else if(audio->channel < 7)
		channel = I2S_CHANNEL_5_6;
	else
		channel = I2S_CHANNEL_7_8;
		
	switch(audio->rate)
	{
		case HDMI_AUDIO_FS_32000:
			rate = AUDIO_32K;
			N = N_32K;
			mclk_fs = MCLK_384FS;
			break;
		case HDMI_AUDIO_FS_44100:
			rate = AUDIO_441K;
			N = N_441K;
			mclk_fs = MCLK_256FS;
			break;
		case HDMI_AUDIO_FS_48000:
			rate = AUDIO_48K;
			N = N_48K;
			mclk_fs = MCLK_256FS;
			break;
		case HDMI_AUDIO_FS_88200:
			rate = AUDIO_882K;
			N = N_882K;
			mclk_fs = MCLK_128FS;
			break;
		case HDMI_AUDIO_FS_96000:
			rate = AUDIO_96K;
			N = N_96K;
			mclk_fs = MCLK_128FS;
			break;
		case HDMI_AUDIO_FS_176400:
			rate = AUDIO_1764K;
			N = N_1764K;
			mclk_fs = MCLK_128FS;
			break;
		case HDMI_AUDIO_FS_192000:
			rate = AUDIO_192K;
			N = N_192K;
			mclk_fs = MCLK_128FS;
			break;
		default:
			dev_err(hdmi->dev, "[%s] not support such sample rate %d\n", __FUNCTION__, audio->rate);
			return -ENOENT;
	}

	//set_audio source I2S
	HDMIWrReg(AUDIO_CTRL1, 0x00); //internal CTS, disable down sample, i2s input, disable MCLK
	HDMIWrReg(AUDIO_SAMPLE_RATE, rate);
	HDMIWrReg(AUDIO_I2S_MODE, v_I2S_MODE(I2S_STANDARD) | v_I2S_CHANNEL(channel) );	
	HDMIWrReg(AUDIO_I2S_MAP, 0x00); 
	HDMIWrReg(AUDIO_I2S_SWAPS_SPDIF, 0); // no swap	
		
    //Set N value
    HDMIWrReg(AUDIO_N_H, (N >> 16) & 0x0F);
    HDMIWrReg(AUDIO_N_M, (N >> 8) & 0xFF); 
	HDMIWrReg(AUDIO_N_L, N & 0xFF);    
    rk3028a_hdmi_config_aai(rk3028a_hdmi);
    
    return 0;
}

int rk3028a_hdmi_control_output(struct hdmi *hdmi, int enable)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;
	char mutestatus = 0;
	int temp;
	
	if(enable & (HDMI_VIDEO_MUTE | HDMI_AUDIO_MUTE) ) {
		HDMIWrReg(AV_MUTE, v_AUDIO_MUTE(1) | v_VIDEO_MUTE(1));
	}
	else {
		mutestatus = HDMIRdReg(AV_MUTE);
		if(mutestatus && (m_AUDIO_MUTE | m_VIDEO_BLACK)) {
			if(enable & HDMI_AUDIO_MUTE)
				HDMIWrReg(AV_MUTE, v_AUDIO_MUTE(1) | v_VIDEO_MUTE(0));
			else
				HDMIWrReg(AV_MUTE, v_AUDIO_MUTE(0) | v_VIDEO_MUTE(0));
		}
	}
	HDMIMskReg(temp, SYS_CTRL, m_POWER, v_PWR_ON);
	HDMIMskReg(temp, SYS_CTRL, m_POWER, v_PWR_OFF);
	HDMIMskReg(temp, SYS_CTRL, m_POWER, v_PWR_ON);
	HDMIWrReg(0xce, 0x00);
	delay100us();
	HDMIWrReg(0xce, 0x01);
	return 0;
}

int rk3028a_hdmi_insert(struct hdmi *hdmi)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;
	
	RK3028ADBG("%s", __FUNCTION__);
	rk3028a_hdmi_set_pwr_mode(rk3028a_hdmi, NORMAL);
	
	return HDMI_ERROR_SUCESS;
}

int rk3028a_hdmi_removed(struct hdmi *hdmi)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;
	
	RK3028ADBG("%s", __FUNCTION__);

	//software reset hdmi to fix noise or nosound when enter early suspend or hotplug hdmi. added by sugar.
	disable_irq(rk3028a_hdmi->irq);
	rk3028a_hdmi_initial(rk3028a_hdmi);
	enable_irq(rk3028a_hdmi->irq);

	return HDMI_ERROR_SUCESS;
}


irqreturn_t hdmi_irq(int irq, void *priv)
{		
	struct rk3028a_hdmi *rk3028a_hdmi = priv;
	struct hdmi *hdmi = rk3028a_hdmi->hdmi;
	char interrupt1 = 0;
	unsigned long flags;
	
	spin_lock_irqsave(&rk3028a_hdmi->irq_lock,flags);
	interrupt1 = HDMIRdReg(INTERRUPT_STATUS1);
	HDMIWrReg(INTERRUPT_STATUS1, interrupt1);
#if 0
		RK3028ADBG( "[%s] interrupt1 %02x  \n",\
			 __FUNCTION__, interrupt1);
#endif
	if(interrupt1 & m_INT_HOTPLUG ){
		hdmi_submit_work(hdmi, HDMI_HPD_CHANGE, 10, NULL);
	}else if(interrupt1 & m_INT_EDID_READY) {
		edid_result = interrupt1;
	}
	
	if(hdmi->ops->hdcp_irq_cb)
		hdmi->ops->hdcp_irq_cb(0);
#ifdef CONFIG_CEC_RK3028A	
	CecIsr();
#endif	
	spin_unlock_irqrestore(&rk3028a_hdmi->irq_lock,flags);
	return IRQ_HANDLED;
}

void rk3028a_hdmi_initial(struct rk3028a_hdmi *rk3028a_hdmi)
{
	int temp;
	// Reset hdmi
	writel_relaxed(0x00010001,RK2928_CRU_BASE+ 0x128);
	msleep(100);
	writel_relaxed(0x00010000, RK2928_CRU_BASE + 0x128);
	
//	HDMIMskReg(temp, SYS_CTRL,m_DIGITAL_RESET | m_ANALOG_RESET, v_DIGITAL_RESET(0) | v_ANALOG_RESET(0) );
//	delay100us();
//	HDMIMskReg(temp, SYS_CTRL,m_DIGITAL_RESET | m_ANALOG_RESET, v_DIGITAL_RESET(1) | v_ANALOG_RESET(1) );

	HDMIWrReg(INTERRUPT_MASK1, m_INT_HOTPLUG);
	
	rk3028a_hdmi_set_pwr_mode(rk3028a_hdmi, LOWER_PWR);
}

int rk3028a_hdmi_enable(struct hdmi *hdmi)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;

	if(!rk3028a_hdmi->enable) {
		rk3028a_hdmi->enable = 1;
		enable_irq(rk3028a_hdmi->irq);
		hdmi_submit_work(hdmi, HDMI_HPD_CHANGE, 10, NULL);
	}
	return 0;
}

int rk3028a_hdmi_disable(struct hdmi *hdmi)
{
	struct rk3028a_hdmi *rk3028a_hdmi = hdmi->property->priv;

	if(rk3028a_hdmi->enable) {
		rk3028a_hdmi->enable = 0;
		disable_irq(rk3028a_hdmi->irq);
	}
	return 0;
}
