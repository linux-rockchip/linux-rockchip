#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/rockchip/grf.h>
#include <linux/rockchip/iomap.h>
#include "rk3288_hdmi.h"
#include "rk3288_hdmi_hw.h"

static const struct phy_mpll_config_tab PHY_MPLL_TABLE[] = {	
	//tmdsclk = (pixclk / ref_cntrl ) * (fbdiv2 * fbdiv1) / nctrl / tmdsmhl
	//opmode: 0:HDMI1.4 	1:HDMI2.0
//	|pixclock|pixrepet|colordepth|prepdiv|tmdsmhl|opmode|fbdiv2|fbdiv1|ref_cntrl|nctrl|propctrl|intctrl|gmpctrl|	
	{27000000,	0,	8,	0,	0,	0,	2,	3,	0,	3,	3,	0,	0},
	{27000000,	0,	10,	1,	0,	0,	5,	1,	0,	3,	3,	0,	0},
	{27000000,	0,	12,	2,	0,	0,	3,	3,	0,	3,	3,	0,	0},
	{27000000,	0,	16,	3,	0,	0,	2,	3,	0,	2,	5,	0,	1},
	{74250000, 	0,	8, 	0,	0,	0, 	1, 	3,	0,	2,	5,	0, 	1},
	{74250000,	0,	10,	1,	0,	0,	5,	0,	1,	1,	7,	0,	2},
	{74250000,	0,	12,	2,	0,	0,	1,	2,	0,	1,	7,	0,	2},
	{74250000,	0,	16,	3,	0,	0,	1,	3,	0,	1,	7,	0,	2},
	{148500000, 	0, 	8,  	0, 	0,	0,	1,	1,	0,	1,	0,	0,	3},
	{148500000,	0,	10,	1,	0,	0,	5,	0,	3,	0,	7,	0,	3},
	{148500000,	0,	12,	2,	0,	0,	1,	2,	1,	0,	7,	0,	3},
	{148500000,	0,	16,	3,	0,	0,	1,	1,	0,	0,	7,	0,	3},
	{297000000,	0, 	8,	0, 	0, 	0, 	1, 	0, 	0, 	0, 	0, 	0, 	3},
	{297000000,	0, 	10,	1, 	3, 	1, 	5, 	0, 	3, 	0, 	7, 	0, 	3},
	{297000000,	0, 	12,	2, 	3, 	1, 	1, 	2, 	2, 	0, 	7, 	0, 	3},
	{297000000, 	0, 	16,  	3, 	3, 	1, 	1, 	1, 	0, 	0, 	5, 	0, 	3},
	{594000000,	0, 	8, 	0, 	3, 	1, 	1, 	3, 	3, 	0, 	0, 	0, 	3},
};

static const struct phy_mpll_config_tab* get_phy_mpll_tab(unsigned int pixClock, char pixRepet, char colorDepth)
{
	int i;

	if(pixClock == 0)
		return NULL;
	HDMIDBG("%s pixClock %u pixRepet %d colorDepth %d\n", __FUNCTION__, pixClock, pixRepet, colorDepth);
	for(i = 0; i < ARRAY_SIZE(PHY_MPLL_TABLE); i++)
	{
		if((PHY_MPLL_TABLE[i].pix_clock == pixClock) && (PHY_MPLL_TABLE[i].pix_repet == pixRepet)
			&& (PHY_MPLL_TABLE[i].color_depth == colorDepth))
			return &PHY_MPLL_TABLE[i];
	}
	return NULL;
}

static void rk3288_hdmi_powerdown(struct hdmi_dev* hdmi_dev)
{
	hdmi_msk_reg(hdmi_dev, PHY_CONF0, m_PDDQ_SIG | m_TXPWRON_SIG | m_ENHPD_RXSENSE_SIG,
		v_PDDQ_SIG(1) | v_TXPWRON_SIG(0) | v_ENHPD_RXSENSE_SIG(1));
	hdmi_writel(hdmi_dev, MC_CLKDIS, 0x7f);
}

static int rk3288_hdmi_write_phy(struct hdmi_dev *hdmi_dev, int reg_addr, int val)
{
	int trytime = 2, i = 0, op_status = 0;

	while(trytime--) {
		hdmi_writel(hdmi_dev, PHY_I2CM_ADDRESS, reg_addr);
		hdmi_writel(hdmi_dev, PHY_I2CM_DATAO_1, (val >> 8) & 0xff);
		hdmi_writel(hdmi_dev, PHY_I2CM_DATAO_0, val & 0xff);
		hdmi_writel(hdmi_dev, PHY_I2CM_OPERATION, m_PHY_I2CM_WRITE);

		i = 20;
		while(i--) {
			msleep(1);
			op_status = hdmi_readl(hdmi_dev, IH_I2CMPHY_STAT0);
			if(op_status)
				hdmi_writel(hdmi_dev, IH_I2CMPHY_STAT0, op_status);

			if(op_status & (m_I2CMPHY_DONE | m_I2CMPHY_ERR)) {
				break;
			}
			msleep(4);
		}

		if(op_status & m_I2CMPHY_DONE) {
			return 0;
		}
		else {
			dev_err(hdmi_dev->hdmi->dev, "[%s] operation error,trytime=%d\n", __FUNCTION__, trytime);
		}
		msleep(100);
	}

	return -1;
}

static int rk3288_hdmi_config_phy(struct hdmi_dev *hdmi_dev)
{
	int stat = 0, i = 0;
	const struct phy_mpll_config_tab *phy_mpll = NULL;
	hdmi_msk_reg(hdmi_dev, PHY_I2CM_DIV, m_PHY_I2CM_FAST_STD, v_PHY_I2CM_FAST_STD(0));

	//power on PHY
	//hdmi_writel(hdmi_dev, PHY_CONF0, 0x1e);
	hdmi_msk_reg(hdmi_dev, PHY_CONF0, m_PDDQ_SIG | m_TXPWRON_SIG, v_PDDQ_SIG(1) | v_TXPWRON_SIG(0));

	//reset PHY
	hdmi_writel(hdmi_dev, MC_PHYRSTZ, v_PHY_RSTZ(1));
	msleep(5);
	hdmi_writel(hdmi_dev, MC_PHYRSTZ, v_PHY_RSTZ(0));

	//Set slave address as PHY GEN2 address
	hdmi_writel(hdmi_dev, PHY_I2CM_SLAVE, PHY_GEN2_ADDR);

	//config the required PHY I2C register
	phy_mpll = get_phy_mpll_tab(hdmi_dev->pixelclk, hdmi_dev->pixelrepeat - 1, hdmi_dev->colordepth);
	if(phy_mpll) {
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_OPMODE_PLLCFG, v_PREP_DIV(phy_mpll->prep_div) | v_TMDS_CNTRL(phy_mpll->tmdsmhl_cntrl) | v_OPMODE(phy_mpll->opmode) |
			v_FBDIV2_CNTRL(phy_mpll->fbdiv2_cntrl) | v_FBDIV1_CNTRL(phy_mpll->fbdiv1_cntrl) | v_REF_CNTRL(phy_mpll->ref_cntrl) | v_MPLL_N_CNTRL(phy_mpll->n_cntrl));
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_PLLCURRCTRL, v_MPLL_PROP_CNTRL(phy_mpll->prop_cntrl) | v_MPLL_INT_CNTRL(phy_mpll->int_cntrl));
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_PLLGMPCTRL, v_MPLL_GMP_CNTRL(phy_mpll->gmp_cntrl));
	}
	if(hdmi_dev->pixelclk <= 74250000) {
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_CLKSYMCTRL, v_OVERRIDE(1) | v_SLOPEBOOST(0)
			| v_TX_SYMON(1) | v_TX_TRAON(0) | v_TX_TRBON(0) | v_CLK_SYMON(1));
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_TERM_RESIS, v_TX_TERM(R100_Ohms));
	}
	else if(hdmi_dev->pixelclk <= 148500000) {
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_CLKSYMCTRL, v_OVERRIDE(1) | v_SLOPEBOOST(2)
			| v_TX_SYMON(1) | v_TX_TRAON(0) | v_TX_TRBON(0) | v_CLK_SYMON(1));
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_TERM_RESIS, v_TX_TERM(R100_Ohms));
	}
	else if(hdmi_dev->pixelclk <= 297000000) {
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_CLKSYMCTRL, v_OVERRIDE(1) | v_SLOPEBOOST(2)
			| v_TX_SYMON(1) | v_TX_TRAON(0) | v_TX_TRBON(0) | v_CLK_SYMON(1));
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_TERM_RESIS, v_TX_TERM(R100_Ohms));
	}
	else if(hdmi_dev->pixelclk > 297000000) {
		//TODO Daisen wait to add and modify
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_CLKSYMCTRL, v_OVERRIDE(1) | v_SLOPEBOOST(3)
			| v_TX_SYMON(1) | v_TX_TRAON(0) | v_TX_TRBON(1) | v_CLK_SYMON(1));
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_TERM_RESIS, v_TX_TERM(R100_Ohms));
	}
	if(hdmi_dev->pixelclk < 297000000)
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_VLEVCTRL, v_SUP_TXLVL(18) | v_SUP_CLKLVL(17));
	else
		rk3288_hdmi_write_phy(hdmi_dev, PHYTX_VLEVCTRL, v_SUP_TXLVL(14) | v_SUP_CLKLVL(13));

	rk3288_hdmi_write_phy(hdmi_dev, 0x05, 0x8000);

	//power on PHY
	hdmi_writel(hdmi_dev, PHY_CONF0, 0x2e);
	//hdmi_msk_reg(hdmi_dev, PHY_CONF0, m_PDDQ_SIG | m_TXPWRON_SIG | m_ENHPD_RXSENSE_SIG,
		//v_PDDQ_SIG(0) | v_TXPWRON_SIG(1) | v_ENHPD_RXSENSE_SIG(1));

	//check if the PHY PLL is locked
	#define PHY_TIMEOUT	10000
	while(i++ < PHY_TIMEOUT) {
		if ((i % 100) == 0) {
			stat = hdmi_readl(hdmi_dev, PHY_STAT0);
			if(stat & m_PHY_LOCK) {
				//hdmi_writel(hdmi_dev, PHY_STAT0, v_PHY_LOCK(1));
				break;
			}
		}
	}
	if((stat & m_PHY_LOCK) == 0) {
		stat = hdmi_readl(hdmi_dev, MC_LOCKONCLOCK);
		dev_err(hdmi_dev->hdmi->dev, "PHY PLL not locked: PCLK_ON=%d,TMDSCLK_ON=%d\n", (stat & m_PCLK_ON) >> 6, (stat & m_TMDSCLK_ON) >> 5);
		return -1;
	}

	return 0;
}

static int rk3288_hdmi_video_frameComposer(struct hdmi *hdmi_drv, struct hdmi_video *vpara)	//TODO Daisen wait to add support 3D
{
	struct hdmi_dev *hdmi_dev = hdmi_drv->property->priv;
	int value, vsync_pol, hsync_pol, de_pol;
	struct hdmi_video_timing *timing = NULL;
	struct fb_videomode *mode = NULL;

	
	vsync_pol = hdmi_drv->lcdc->cur_screen->pin_vsync;
	hsync_pol = hdmi_drv->lcdc->cur_screen->pin_hsync;
	de_pol = (hdmi_drv->lcdc->cur_screen->pin_den == 0) ? 1 : 0;
	
	timing = (struct hdmi_video_timing *)hdmi_vic2timing(vpara->vic);
	if(timing == NULL) {
		dev_err(hdmi_drv->dev, "[%s] not found vic %d\n", __FUNCTION__, vpara->vic);
		return -ENOENT;
	}
	mode = &(timing->mode);
	switch(vpara->color_output_depth) {
		case 8:
			hdmi_dev->tmdsclk = mode->pixclock;
			break;
		case 10:
			hdmi_dev->tmdsclk = mode->pixclock * 10 / 8;
			break;
		case 12:
			hdmi_dev->tmdsclk = mode->pixclock * 12 / 8;
			break;
		case 16:
			hdmi_dev->tmdsclk = mode->pixclock * 2;
			break;
		default:
			hdmi_dev->tmdsclk = mode->pixclock;
			break;
	}
	// Now we limit to hdmi 1.4b standard.
	if(mode->pixclock <= 340000000 && hdmi_dev->tmdsclk > 340000000)
	{
		vpara->color_output_depth = 8;
		hdmi_dev->tmdsclk = mode->pixclock;
	}
	hdmi_dev->pixelclk = mode->pixclock;
	hdmi_dev->pixelrepeat = timing->pixelrepeat;
	hdmi_dev->colordepth = vpara->color_output_depth;

	hdmi_msk_reg(hdmi_dev, A_HDCPCFG0, m_ENCRYPT_BYPASS | m_HDMI_DVI,
		v_ENCRYPT_BYPASS(1) | v_HDMI_DVI(vpara->sink_hdmi));	//cfg to bypass hdcp data encrypt
	hdmi_msk_reg(hdmi_dev, FC_INVIDCONF, m_FC_VSYNC_POL | m_FC_HSYNC_POL | m_FC_DE_POL | m_FC_HDMI_DVI | m_FC_INTERLACE_MODE,
		v_FC_VSYNC_POL(vsync_pol) | v_FC_HSYNC_POL(hsync_pol) | v_FC_DE_POL(de_pol) | v_FC_HDMI_DVI(vpara->sink_hdmi) | v_FC_INTERLACE_MODE(mode->vmode));

	hdmi_msk_reg(hdmi_dev, FC_INVIDCONF, m_FC_VBLANK, v_FC_VBLANK(0));

	value = mode->xres;
	hdmi_writel(hdmi_dev, FC_INHACTIV1, v_FC_HACTIVE1(value >> 8));
	hdmi_writel(hdmi_dev, FC_INHACTIV0, (value & 0xff));

	value = mode->yres;
	hdmi_writel(hdmi_dev, FC_INVACTIV1, v_FC_VACTIVE1(value >> 8));
	hdmi_writel(hdmi_dev, FC_INVACTIV0, (value & 0xff));

	value = mode->hsync_len + mode->left_margin + mode->right_margin;
	hdmi_writel(hdmi_dev, FC_INHBLANK1, v_FC_HBLANK1(value >> 8));
	hdmi_writel(hdmi_dev, FC_INHBLANK0, (value & 0xff));

	value = mode->vsync_len + mode->upper_margin + mode->lower_margin;
	hdmi_writel(hdmi_dev, FC_INVBLANK, (value & 0xff));

	value = mode->right_margin;
	hdmi_writel(hdmi_dev, FC_HSYNCINDELAY1, v_FC_HSYNCINDEAY1(value >> 8));
	hdmi_writel(hdmi_dev, FC_HSYNCINDELAY0, (value & 0xff));

	value = mode->lower_margin;
	hdmi_writel(hdmi_dev, FC_VSYNCINDELAY, (value & 0xff));

	value = mode->hsync_len;
	hdmi_writel(hdmi_dev, FC_HSYNCINWIDTH1, v_FC_HSYNCWIDTH1(value >> 8));
	hdmi_writel(hdmi_dev, FC_HSYNCINWIDTH0, (value & 0xff));

	value = mode->vsync_len;
	hdmi_writel(hdmi_dev, FC_VSYNCINWIDTH, (value & 0xff));

	/*Set the control period minimum duration(min. of 12 pixel clock cycles, refer to HDMI 1.4b specification)*/
	hdmi_writel(hdmi_dev, FC_CTRLDUR, 12);
	hdmi_writel(hdmi_dev, FC_EXCTRLDUR, 32);
	
#if 0

	if(hdmi_dev->tmdsclk > 340000000) {	//used for HDMI 2.0 TX	//TODO Daisen wait to modify HDCP KEEPOUT
		hdmi_msk_reg(hdmi_dev, FC_INVIDCONF, m_FC_HDCP_KEEPOUT, v_FC_HDCP_KEEPOUT(1));
		hdmi_msk_reg(hdmi_dev, FC_SCRAMBLER_CTRL, m_FC_SCRAMBLE_EN, v_FC_SCRAMBLE_EN(1));
	}
	/* spacing < 256^2 * config / tmdsClock, spacing <= 50ms
	 * worst case: tmdsClock == 25MHz => config <= 19
	 */
	hdmi_writel(hdmi_dev, FC_EXCTRLSPAC, 1);

	/*Set PreambleFilter*/
	for (i = 0; i < 3; i++) {
		value = (i + 1) * 11;
		if (i == 0)		/*channel 0*/
			hdmi_writel(hdmi_dev, FC_CH0PREAM, value);
		else if (i == 1)	/*channel 1*/
			hdmi_writel(hdmi_dev, FC_CH1PREAM, value & 0x3f);
		else if (i == 2)	/*channel 2*/
			hdmi_writel(hdmi_dev, FC_CH2PREAM, value & 0x3f);
	}
#endif

	hdmi_writel(hdmi_dev, FC_PRCONF, v_FC_PR_FACTOR(timing->pixelrepeat));
	
	return 0;
}

static int rk3288_hdmi_video_packetizer(struct hdmi_dev *hdmi_dev, struct hdmi_video *vpara)
{
	unsigned char color_depth = 0;
	unsigned char output_select = 0;
	unsigned char remap_size = 0;

	if(vpara->color_output == HDMI_COLOR_YCbCr422) {
		switch (vpara->color_output_depth) {
			case 8:
				remap_size = YCC422_16BIT;
				break;
			case 10:
				remap_size = YCC422_20BIT;
				break;
			case 12:
				remap_size = YCC422_24BIT;
				break;
			default:
				remap_size = YCC422_16BIT;
				break;
		}

		output_select = OUT_FROM_YCC422_REMAP;
		/*Config remap size for the different color Depth*/
		hdmi_msk_reg(hdmi_dev, VP_REMAP, m_YCC422_SIZE, v_YCC422_SIZE(remap_size));
	} else {
		switch (vpara->color_output_depth) {
			case 10:
				color_depth = COLOR_DEPTH_30BIT;
				output_select = OUT_FROM_PIXEL_PACKING;
				break;
			case 12:
				color_depth = COLOR_DEPTH_36BIT;
				output_select = OUT_FROM_PIXEL_PACKING;
				break;
			case 16:
				color_depth = COLOR_DEPTH_48BIT;
				output_select = OUT_FROM_PIXEL_PACKING;
				break;
			case 8:
			default:
				color_depth = COLOR_DEPTH_24BIT;
				output_select = OUT_FROM_8BIT_BYPASS;
				break;
		}

		/*Config Color Depth*/
		hdmi_msk_reg(hdmi_dev, VP_PR_CD, m_COLOR_DEPTH, v_COLOR_DEPTH(color_depth));			
	}
	
	/*Config pixel repettion*/
	hdmi_msk_reg(hdmi_dev, VP_PR_CD, m_DESIRED_PR_FACTOR, v_DESIRED_PR_FACTOR(hdmi_dev->pixelrepeat - 1));
	if (hdmi_dev->pixelrepeat > 1) {
		hdmi_msk_reg(hdmi_dev, VP_CONF, m_PIXEL_REPET_EN | m_BYPASS_SEL, v_PIXEL_REPET_EN(1) | v_BYPASS_SEL(0));
	}
	else
		hdmi_msk_reg(hdmi_dev, VP_CONF, m_PIXEL_REPET_EN | m_BYPASS_SEL, v_PIXEL_REPET_EN(0) | v_BYPASS_SEL(1));
	
	/*config output select*/
	if (output_select == OUT_FROM_PIXEL_PACKING) { /* pixel packing */
		hdmi_msk_reg(hdmi_dev, VP_CONF, m_BYPASS_EN | m_PIXEL_PACK_EN | m_YCC422_EN | m_OUTPUT_SEL,
			v_BYPASS_EN(0) | v_PIXEL_PACK_EN(1) | v_YCC422_EN(0) | v_OUTPUT_SEL(output_select));
	} else if (output_select == OUT_FROM_YCC422_REMAP) { /* YCC422 */
		hdmi_msk_reg(hdmi_dev, VP_CONF, m_BYPASS_EN | m_PIXEL_PACK_EN | m_YCC422_EN | m_OUTPUT_SEL,
			v_BYPASS_EN(0) | v_PIXEL_PACK_EN(0) | v_YCC422_EN(1) | v_OUTPUT_SEL(output_select));
	} else if (output_select == OUT_FROM_8BIT_BYPASS || output_select == 3) { /* bypass */
		hdmi_msk_reg(hdmi_dev, VP_CONF, m_BYPASS_EN | m_PIXEL_PACK_EN | m_YCC422_EN | m_OUTPUT_SEL,
			v_BYPASS_EN(1) | v_PIXEL_PACK_EN(0) | v_YCC422_EN(0) | v_OUTPUT_SEL(output_select));
	}

#if defined(HDMI_VIDEO_STUFFING)
	/* YCC422 and pixel packing stuffing*/
	hdmi_msk_reg(hdmi_dev, VP_STUFF, m_PR_STUFFING, v_PR_STUFFING(1));
	hdmi_msk_reg(hdmi_dev, VP_STUFF, m_YCC422_STUFFING | m_PP_STUFFING, v_YCC422_STUFFING(1) | v_PP_STUFFING(1));
#endif
	return 0;
}

static int rk3288_hdmi_video_sampler(struct hdmi_dev *hdmi_dev, struct hdmi_video *vpara)
{
	int map_code = 0;
	
	if (vpara->color_input == HDMI_COLOR_YCbCr422) {
		/* YCC422 mapping is discontinued - only map 1 is supported */
		switch (vpara->color_output_depth) {
		case 8:
			map_code = VIDEO_YCBCR422_8BIT;
			break;
		case 10:
			map_code = VIDEO_YCBCR422_10BIT;
			break;
		case 12:
			map_code = VIDEO_YCBCR422_12BIT;
			break;
		default:
			map_code = VIDEO_YCBCR422_8BIT;
			break;
		}
	} else {
		switch (vpara->color_output_depth) {
		case 10:
			map_code = VIDEO_RGB444_10BIT;
			break;
		case 12:
			map_code = VIDEO_RGB444_12BIT;
			break;
		case 16:
			map_code = VIDEO_RGB444_16BIT;
			break;
		case 8:
		default:
			map_code = VIDEO_RGB444_8BIT;
			break;
		}
		map_code += (vpara->color_input == HDMI_COLOR_YCbCr444) ? 8 : 0;
	}

	//Set Data enable signal from external and set video sample input mapping
	hdmi_msk_reg(hdmi_dev, TX_INVID0, m_INTERNAL_DE_GEN | m_VIDEO_MAPPING, v_INTERNAL_DE_GEN(0) | v_VIDEO_MAPPING(map_code));

#if defined(HDMI_VIDEO_STUFFING)
	hdmi_writel(hdmi_dev, TX_GYDATA0, 0x00);
	hdmi_writel(hdmi_dev, TX_GYDATA1, 0x00);
	hdmi_msk_reg(hdmi_dev, TX_INSTUFFING, m_GYDATA_STUFF, v_GYDATA_STUFF(1));
	hdmi_writel(hdmi_dev, TX_RCRDATA0, 0x00);
	hdmi_writel(hdmi_dev, TX_RCRDATA1, 0x00);
	hdmi_msk_reg(hdmi_dev, TX_INSTUFFING, m_RCRDATA_STUFF, v_RCRDATA_STUFF(1));
	hdmi_writel(hdmi_dev, TX_BCBDATA0, 0x00);
	hdmi_writel(hdmi_dev, TX_BCBDATA1, 0x00);
	hdmi_msk_reg(hdmi_dev, TX_INSTUFFING, m_BCBDATA_STUFF, v_BCBDATA_STUFF(1));
#endif
	return 0;
}

static const char coeff_csc[][24] = {
		//   G		R	    B		Bias
		//   A1    |	A2     |    A3     |	A4    |
		//   B1    |    B2     |    B3     |    B4    |
		//   C1    |    C2     |    C3     |    C4    |
	{	//CSC_RGB_0_255_TO_RGB_16_235_8BIT
		0x36, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,		//G
		0x00, 0x00, 0x36, 0xf7, 0x00, 0x00, 0x00, 0x40,		//R
		0x00, 0x00, 0x00, 0x00, 0x36, 0xf7, 0x00, 0x40,		//B
	},
	{	//CSC_RGB_0_255_TO_RGB_16_235_10BIT
		0x36, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,		//G
		0x00, 0x00, 0x36, 0xf7, 0x00, 0x00, 0x01, 0x00,		//R
		0x00, 0x00, 0x00, 0x00, 0x36, 0xf7, 0x01, 0x00,		//B
	},
#if 0
	{	//CSC_RGB_0_255_TO_ITU601_16_235_8BIT
		0x25, 0x91, 0x13, 0x23, 0x07, 0x4c, 0x00, 0x00, 	//Y
		0xe5, 0x34, 0x20, 0x00, 0xfa, 0xcc, 0x02, 0x00, 	//Cr
		0xea, 0xcd, 0xf5, 0x33, 0x20, 0x00, 0x02, 0x00, 	//Cb
	},
	{	//CSC_RGB_0_255_TO_ITU601_16_235_10BIT
		0x25, 0x91, 0x13, 0x23, 0x07, 0x4c, 0x00, 0x00, 	//Y
		0xe5, 0x34, 0x20, 0x00, 0xfa, 0xcc, 0x08, 0x00, 	//Cr
		0xea, 0xcd, 0xf5, 0x33, 0x20, 0x00, 0x08, 0x00, 	//Cb
	},
	{	//CSC_RGB_0_255_TO_ITU709_16_235_8BIT
		0x2d, 0xc6, 0x0d, 0x9b, 0x04, 0x9f, 0x00, 0x00, 	//Y
		0xe2, 0xef, 0x20, 0x00, 0xfd, 0x11, 0x02, 0x00,		//Cr
		0xe7, 0x55, 0xf8, 0xab, 0x20, 0x00, 0x02, 0x00, 	//Cb
	},
	{	//CSC_RGB_0_255_TO_ITU709_16_235_10BIT
		0x2d, 0xc6, 0x0d, 0x9b, 0x04, 0x9f, 0x00, 0x00, 	//Y
		0xe2, 0xef, 0x20, 0x00, 0xfd, 0x11, 0x08, 0x00,		//Cr
		0xe7, 0x55, 0xf8, 0xab, 0x20, 0x00, 0x08, 0x00, 	//Cb
	},
#else
	{	//CSC_RGB_0_255_TO_ITU601_16_235_8BIT
		0x20, 0x40, 0x10, 0x80, 0x06, 0x40, 0x00, 0x40,		//Y
		0xe8, 0x80, 0x1c, 0x00, 0xfb, 0x80, 0x02, 0x00,		//Cr
		0xed, 0x80, 0xf6, 0x80, 0x1c, 0x00, 0x02, 0x00,		//Cb
	},
	{	//CSC_RGB_0_255_TO_ITU601_16_235_10BIT
		0x20, 0x40, 0x10, 0x80, 0x06, 0x40, 0x01, 0x00,		//Y
		0xe8, 0x80, 0x1c, 0x00, 0xfb, 0x80, 0x08, 0x00,		//Cr
		0xed, 0x80, 0xf6, 0x80, 0x1c, 0x00, 0x08, 0x00,		//Cb
	},
	{	//CSC_RGB_0_255_TO_ITU709_16_235_8BIT
		0x27, 0x40, 0x0b, 0xc0, 0x04, 0x00, 0x00, 0x40,		//Y
		0xe6, 0x80, 0x1c, 0x00, 0xfd, 0x80, 0x02, 0x00,		//Cr
		0xea, 0x40, 0xf9, 0x80, 0x1c, 0x00, 0x02, 0x00,		//Cb
	},
	{	//CSC_RGB_0_255_TO_ITU709_16_235_10BIT
		0x27, 0x40, 0x0b, 0xc0, 0x04, 0x00, 0x01, 0x00,		//Y
		0xe6, 0x80, 0x1c, 0x00, 0xfd, 0x80, 0x08, 0x00,		//Cr
		0xea, 0x40, 0xf9, 0x80, 0x1c, 0x00, 0x08, 0x00,		//Cb
	},
#endif
		//Y		Cr	    Cb		Bias
	{	//CSC_ITU601_16_235_TO_RGB_0_255_8BIT
		0x20, 0x00, 0x69, 0x26, 0x74, 0xfd, 0x01, 0x0e, 	//G 
		0x20, 0x00, 0x2c, 0xdd, 0x00, 0x00, 0x7e, 0x9a, 	//R
		0x20, 0x00, 0x00, 0x00, 0x38, 0xb4, 0x7e, 0x3b,		//B
	},
	{	//CSC_ITU709_16_235_TO_RGB_0_255_8BIT
		0x20, 0x00, 0x71, 0x06, 0x7a, 0x02, 0x00, 0xa7, 	//G
		0x20, 0x00, 0x32, 0x64, 0x00, 0x00, 0x7e, 0x6d, 	//R
		0x20, 0x00, 0x00, 0x00, 0x3b, 0x61, 0x7e, 0x25, 	//B
	},
};

static int rk3288_hdmi_video_csc(struct hdmi_dev *hdmi_dev, struct hdmi_video *vpara)
{
	int i, mode, interpolation, decimation, csc_scale;
	const char *coeff = NULL;
	unsigned char color_depth = 0;

	if (vpara->color_input == vpara->color_output) {
		hdmi_msk_reg(hdmi_dev, MC_FLOWCTRL, m_FEED_THROUGH_OFF, v_FEED_THROUGH_OFF(0));
		return 0;
	}

	if(vpara->color_input == HDMI_COLOR_YCbCr422 && 
		vpara->color_output != HDMI_COLOR_YCbCr422 && 
		vpara->color_output != HDMI_COLOR_YCbCr420) {
		interpolation = 1;
		hdmi_msk_reg(hdmi_dev, CSC_CFG, m_CSC_INTPMODE, v_CSC_INTPMODE(interpolation));
	}

	if((vpara->color_input == HDMI_COLOR_RGB_0_255 || vpara->color_input == HDMI_COLOR_YCbCr444)
		&& vpara->color_output == HDMI_COLOR_YCbCr422) {
		decimation = 1;
		hdmi_msk_reg(hdmi_dev, CSC_CFG, m_CSC_DECIMODE, v_CSC_DECIMODE(decimation));
	}

	switch(vpara->vic)
	{
		case HDMI_720x480i_60HZ_4_3:
		case HDMI_720x576i_50HZ_4_3:
		case HDMI_720x480p_60HZ_4_3:
		case HDMI_720x576p_50HZ_4_3:
		case HDMI_720x480i_60HZ_16_9:
		case HDMI_720x576i_50HZ_16_9:
		case HDMI_720x480p_60HZ_16_9:
		case HDMI_720x576p_50HZ_16_9:
			if (vpara->color_input == HDMI_COLOR_RGB_0_255 && vpara->color_output >= HDMI_COLOR_YCbCr444) {
				mode = CSC_RGB_0_255_TO_ITU601_16_235_8BIT;
				csc_scale = 0;
			} else if (vpara->color_input >= HDMI_COLOR_YCbCr444 && vpara->color_output == HDMI_COLOR_RGB_0_255) {
				mode = CSC_ITU601_16_235_TO_RGB_0_255_8BIT;
				csc_scale = 1;
			}
			break;
		default:
			if (vpara->color_input == HDMI_COLOR_RGB_0_255 && vpara->color_output >= HDMI_COLOR_YCbCr444) {
				mode = CSC_RGB_0_255_TO_ITU709_16_235_8BIT;
				csc_scale = 0;
			} else if (vpara->color_input >= HDMI_COLOR_YCbCr444 && vpara->color_output == HDMI_COLOR_RGB_0_255) {
				mode = CSC_ITU709_16_235_TO_RGB_0_255_8BIT;
				csc_scale = 1;
			}
			break;
	}

	if ((vpara->color_input == HDMI_COLOR_RGB_0_255) && (vpara->color_output == HDMI_COLOR_RGB_16_235) ) {
		mode = CSC_RGB_0_255_TO_RGB_16_235_8BIT;
		csc_scale = 0;
	}

	switch (vpara->color_output_depth) {
		case 10:
			color_depth = COLOR_DEPTH_30BIT;
			mode += 1;
			break;
		case 12:
			color_depth = COLOR_DEPTH_36BIT;
			mode += 2;
			break;
		case 16:
			color_depth = COLOR_DEPTH_48BIT;
			mode += 3;
			break;
		case 8:
		default:
			color_depth = COLOR_DEPTH_24BIT;
			break;
	}

	coeff = coeff_csc[mode];
	for(i = 0; i < 24; i++) {
		hdmi_writel(hdmi_dev, CSC_COEF_A1_MSB + i, coeff[i]);
	}
	hdmi_msk_reg(hdmi_dev, CSC_SCALE, m_CSC_SCALE, v_CSC_SCALE(csc_scale));
	/*config CSC_COLOR_DEPTH*/
	hdmi_msk_reg(hdmi_dev, CSC_SCALE, m_CSC_COLOR_DEPTH, v_CSC_COLOR_DEPTH(color_depth));

	//enable CSC
	hdmi_msk_reg(hdmi_dev, MC_FLOWCTRL, m_FEED_THROUGH_OFF, v_FEED_THROUGH_OFF(1));
	
	return 0;
}
//i2c master reset
static void rk3288_hdmi_i2cm_reset(struct hdmi_dev *hdmi_dev)
{
	hdmi_msk_reg(hdmi_dev, I2CM_SOFTRSTZ, m_I2CM_SOFTRST, v_I2CM_SOFTRST(0));
	udelay(100);
}

static int hdmi_dev_detect_hotplug(struct hdmi *hdmi)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	u32 value = hdmi_readl(hdmi_dev, PHY_STAT0);

	HDMIDBG("[%s] reg%x value %02x\n", __FUNCTION__, PHY_STAT0, value);

	if(value & m_PHY_HPD)
		return HDMI_HPD_ACTIVED;
	else
		return HDMI_HPD_REMOVED;
}

static int hdmi_dev_read_edid(struct hdmi *hdmi, int block, unsigned char *buff)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	int i = 0, n = 0, index = 0, ret = -1, trytime = 2;
	int offset = (block % 2) * 0x80;
	int interrupt = 0;

	HDMIDBG("[%s] block %d\n", __FUNCTION__, block);

	rk3288_hdmi_i2cm_reset(hdmi_dev);
	
	//Set DDC I2C CLK which devided from DDC_CLK to 100KHz.
	hdmi_writel(hdmi_dev, I2CM_SS_SCL_HCNT_0_ADDR, 0x7a);
	hdmi_writel(hdmi_dev, I2CM_SS_SCL_LCNT_0_ADDR, 0x8d);
	hdmi_msk_reg(hdmi_dev, I2CM_DIV, m_I2CM_FAST_STD_MODE, v_I2CM_FAST_STD_MODE(STANDARD_MODE));	//Set Standard Mode

	//Enable I2C interrupt for reading edid
	hdmi_writel(hdmi_dev, IH_MUTE_I2CM_STAT0, v_SCDC_READREQ_MUTE(0) | v_I2CM_DONE_MUTE(0) | v_I2CM_ERR_MUTE(0));
	hdmi_msk_reg(hdmi_dev, I2CM_INT, m_I2CM_DONE_MASK, v_I2CM_DONE_MASK(0));
	hdmi_msk_reg(hdmi_dev, I2CM_CTLINT, m_I2CM_NACK_MASK | m_I2CM_ARB_MASK, v_I2CM_NACK_MASK(0) | v_I2CM_ARB_MASK(0));

	hdmi_writel(hdmi_dev, I2CM_SLAVE, DDC_I2C_EDID_ADDR);
	hdmi_writel(hdmi_dev, I2CM_SEGADDR, DDC_I2C_SEG_ADDR);
	hdmi_writel(hdmi_dev, I2CM_SEGPTR, block / 2);
	while(trytime--) {
		for(n = 0; n < HDMI_EDID_BLOCK_SIZE / 8; n++) {
			hdmi_writel(hdmi_dev, I2CM_ADDRESS, offset + 8 * n);
			//enable extend sequential read operation
			if(block == 0)
				hdmi_msk_reg(hdmi_dev, I2CM_OPERATION, m_I2CM_RD8, v_I2CM_RD8(1));
			else
				hdmi_msk_reg(hdmi_dev, I2CM_OPERATION, m_I2CM_RD8_EXT, v_I2CM_RD8_EXT(1));

			i = 20;
			while(i--)
			{
				msleep(1);
				interrupt = hdmi_readl(hdmi_dev, IH_I2CM_STAT0);
				if(interrupt)
					hdmi_writel(hdmi_dev, IH_I2CM_STAT0, interrupt);

				if(interrupt & (m_SCDC_READREQ | m_I2CM_DONE | m_I2CM_ERROR))
					break;
				msleep(4);
			}

			if(interrupt & m_I2CM_DONE) {
				for(index = 0; index < 8; index++) {
					buff[8 * n + index] = hdmi_readl(hdmi_dev, I2CM_READ_BUFF0 + index);
				}

				if(n == HDMI_EDID_BLOCK_SIZE / 8 - 1) {
					ret = 0;
					HDMIDBG("[%s] edid read sucess\n", __FUNCTION__);

					#ifdef DEBUG
					for(index = 0; index < 128; index++) {
						printk("0x%02x ,", buff[index]);
						if( (index + 1) % 16 == 0)
							printk("\n");
					}
					#endif
					goto exit;
				}
				continue;
			} else if((interrupt & m_I2CM_ERROR) || (i == -1)) {
				dev_err(hdmi->dev, "[%s] edid read error\n", __FUNCTION__);
				rk3288_hdmi_i2cm_reset(hdmi_dev);
				break;
			}
		}

		dev_err(hdmi->dev, "[%s] edid try times %d\n", __FUNCTION__, trytime);
		msleep(100);
	}

exit:
	//Disable I2C interrupt
	hdmi_msk_reg(hdmi_dev, IH_MUTE_I2CM_STAT0, m_I2CM_DONE_MUTE | m_I2CM_ERR_MUTE, v_I2CM_DONE_MUTE(1) | v_I2CM_ERR_MUTE(1));
	hdmi_msk_reg(hdmi_dev, I2CM_INT, m_I2CM_DONE_MASK, v_I2CM_DONE_MASK(1));
	hdmi_msk_reg(hdmi_dev, I2CM_CTLINT, m_I2CM_NACK_MASK | m_I2CM_ARB_MASK, v_I2CM_NACK_MASK(1) | v_I2CM_ARB_MASK(1));
	return ret;
}

static void hdmi_dev_config_avi(struct hdmi_dev *hdmi_dev, struct hdmi_video *vpara)
{
	unsigned char colorimetry, ext_colorimetry, aspect_ratio, y1y0;
	unsigned char rgb_quan_range = AVI_QUANTIZATION_RANGE_DEFAULT;

	//Set AVI infoFrame Data byte1
	if(vpara->color_output == HDMI_COLOR_YCbCr444)
		y1y0 = AVI_COLOR_MODE_YCBCR444;
	else if(vpara->color_output == HDMI_COLOR_YCbCr422)
		y1y0 = AVI_COLOR_MODE_YCBCR422;
	else if(vpara->color_output == HDMI_COLOR_YCbCr420)
		y1y0 = AVI_COLOR_MODE_YCBCR420;
	else
		y1y0 = AVI_COLOR_MODE_RGB;

	hdmi_msk_reg(hdmi_dev, FC_AVICONF0, m_FC_ACTIV_FORMAT | m_FC_RGC_YCC, v_FC_RGC_YCC(y1y0) | v_FC_ACTIV_FORMAT(1));

	//Set AVI infoFrame Data byte2
	switch(vpara->vic)
	{
		case HDMI_720x480i_60HZ_4_3:
		case HDMI_720x576i_50HZ_4_3:
		case HDMI_720x480p_60HZ_4_3:
		case HDMI_720x576p_50HZ_4_3:
			aspect_ratio = AVI_CODED_FRAME_ASPECT_4_3;
			colorimetry = AVI_COLORIMETRY_SMPTE_170M;
			break;
		case HDMI_720x480i_60HZ_16_9:
		case HDMI_720x576i_50HZ_16_9:
		case HDMI_720x480p_60HZ_16_9:
		case HDMI_720x576p_50HZ_16_9:
			aspect_ratio = AVI_CODED_FRAME_ASPECT_16_9;
			colorimetry = AVI_COLORIMETRY_SMPTE_170M;
			break;
		default:
			aspect_ratio = AVI_CODED_FRAME_ASPECT_16_9;
			colorimetry = AVI_COLORIMETRY_ITU709;
	}

	if(vpara->color_output_depth > 8) {
		colorimetry = AVI_COLORIMETRY_EXTENDED;
		ext_colorimetry = 6;
	}
	else if(vpara->color_output == HDMI_COLOR_RGB_16_235 || vpara->color_output == HDMI_COLOR_RGB_0_255) {
		colorimetry = AVI_COLORIMETRY_NO_DATA;
		ext_colorimetry = 0;
	}

	hdmi_writel(hdmi_dev, FC_AVICONF1, v_FC_COLORIMETRY(colorimetry) | v_FC_PIC_ASPEC_RATIO(aspect_ratio) | v_FC_ACT_ASPEC_RATIO(ACTIVE_ASPECT_RATE_SAME_AS_CODED_FRAME));

	//Set AVI infoFrame Data byte3
	hdmi_msk_reg(hdmi_dev, FC_AVICONF2, m_FC_EXT_COLORIMETRY | m_FC_QUAN_RANGE,
		v_FC_EXT_COLORIMETRY(ext_colorimetry) | v_FC_QUAN_RANGE(rgb_quan_range));

	//Set AVI infoFrame Data byte4
	hdmi_writel(hdmi_dev, FC_AVIVID, vpara->vic & 0xff);

	//Set AVI infoFrame Data byte5
	hdmi_msk_reg(hdmi_dev, FC_AVICONF3, m_FC_YQ | m_FC_CN, v_FC_YQ(YQ_LIMITED_RANGE) | v_FC_CN(CN_GRAPHICS));
}

static int hdmi_dev_config_vsi(struct hdmi *hdmi, unsigned char vic_3d, unsigned char format)
{
        int i = 0, id = 0x000c03;
	unsigned char data[3] = {0};

	struct hdmi_dev *hdmi_dev = hdmi->property->priv;

        HDMIDBG("[%s] vic %d format %d.\n", __FUNCTION__, vic_3d, format);
        
        hdmi_msk_reg(hdmi_dev, FC_DATAUTO0, m_VSD_AUTO, v_VSD_AUTO(0));
	hdmi_writel(hdmi_dev, FC_VSDIEEEID0, id & 0xff);
	hdmi_writel(hdmi_dev, FC_VSDIEEEID1, (id >> 8) & 0xff);
	hdmi_writel(hdmi_dev, FC_VSDIEEEID2, (id >> 16) & 0xff);

	data[0] = format << 5;	//PB4 --HDMI_Video_Format
	switch(format)
        {
                case HDMI_VIDEO_FORMAT_4Kx2K:
                        data[1] = vic_3d;	//PB5--HDMI_VIC
                        data[2] = 0;
                        break;
                case HDMI_VIDEO_FORMAT_3D:
			data[1] = vic_3d << 4;	//PB5--3D_Structure field
			data[2] = 0;		//PB6--3D_Ext_Data field
			break;
		default:
			data[1] = 0;
			data[2] = 0;
			break;
        }

	for (i = 0; i < 3; i++) {
		hdmi_writel(hdmi_dev, FC_VSDPAYLOAD0 + i, data[i]);
	}

//	if (auto_send) {
		hdmi_msk_reg(hdmi_dev, FC_DATAUTO0, m_VSD_AUTO, v_VSD_AUTO(1));
//	}
//	else {
//		hdmi_msk_reg(hdmi_dev, FC_DATMAN, m_VSD_MAN, v_VSD_MAN(1));
//	}

	return 0;
}

static int hdmi_dev_config_video(struct hdmi *hdmi, struct hdmi_video *vpara)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;

	HDMIDBG("%s vic %d 3dformat %d", __FUNCTION__, vpara->vic, vpara->format_3d);
		
	// force output blue
	hdmi_msk_reg(hdmi_dev, FC_DBGFORCE, m_FC_FORCEVIDEO, v_FC_FORCEVIDEO(1));
	
	if (rk3288_hdmi_video_frameComposer(hdmi, vpara) < 0)
		return -1;
	if (rk3288_hdmi_video_packetizer(hdmi_dev, vpara) < 0)
		return -1;
	//Color space convert
	if (rk3288_hdmi_video_csc(hdmi_dev, vpara) < 0)
		return -1;
	if (rk3288_hdmi_video_sampler(hdmi_dev, vpara) < 0)
		return -1;

	if (vpara->sink_hdmi == OUTPUT_HDMI) {
		hdmi_dev_config_avi(hdmi_dev, vpara);
		if ( vpara->format_3d != HDMI_3D_NONE)
                        hdmi_dev_config_vsi(hdmi, vpara->format_3d, HDMI_VIDEO_FORMAT_3D);
		#ifndef HDMI_VERSION_2
                else if ((vpara->vic > 92 && vpara->vic < 96) || (vpara->vic == 98)) {
			vpara->vic = (vpara->vic == 98) ? 4 : (96 - vpara->vic);
                        hdmi_dev_config_vsi(hdmi, vpara->vic, HDMI_VIDEO_FORMAT_4Kx2K);
                }
		#endif
                else
                        hdmi_dev_config_vsi(hdmi, vpara->vic, HDMI_VIDEO_FORMAT_NORMAL);
		dev_info(hdmi->dev, "[%s] sucess output HDMI.\n", __FUNCTION__);
	}
	else {
		dev_info(hdmi->dev, "[%s] sucess output DVI.\n", __FUNCTION__);
	}
	
	rk3288_hdmi_config_phy(hdmi_dev);
	return 0;
}

static void hdmi_dev_config_aai(struct hdmi_dev *hdmi_dev, struct hdmi_audio *audio)
{
	//Refer to CEA861-E Audio infoFrame
	//Set both Audio Channel Count and Audio Coding Type Refer to Stream Head for HDMI
	hdmi_msk_reg(hdmi_dev, FC_AUDICONF0, m_FC_CHN_CNT | m_FC_CODING_TYEP, v_FC_CHN_CNT(0) | v_FC_CODING_TYEP(0));

	//Set both Audio Sample Size and Sample Frequency Refer to Stream Head for HDMI
	hdmi_msk_reg(hdmi_dev, FC_AUDICONF1, m_FC_SAMPLE_SIZE | m_FC_SAMPLE_FREQ, v_FC_SAMPLE_SIZE(0) | v_FC_SAMPLE_FREQ(0));

	//Set Channel Allocation
	hdmi_writel(hdmi_dev, FC_AUDICONF2, 0x00);

	//Set LFEPBL¡¢DOWN-MIX INH and LSV
	hdmi_writel(hdmi_dev, FC_AUDICONF3, 0x00);
}

static int hdmi_dev_config_audio(struct hdmi *hdmi, struct hdmi_audio *audio)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	int word_length = 0, channel = 0, mclk_fs;
	unsigned int N = 0, CTS = 0;
	int rate = 0;
	HDMIDBG("%s", __FUNCTION__);
	//mute audio
	//hdmi_msk_reg(hdmi_dev, FC_AUDSCONF, m_AUD_PACK_SAMPFIT, v_AUD_PACK_SAMPFIT(0x0F));
	printk("audio->channel: %d, audio->rate: %d, audio->word_length: %d\n", audio->channel,
		audio->rate, audio->word_length);
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
			mclk_fs = FS_256;
			rate = AUDIO_32K;
			if(hdmi_dev->tmdsclk >= 594000000)
				N = N_32K_HIGHCLK;
			else if(hdmi_dev->tmdsclk >= 297000000)
				N = N_32K_MIDCLK;
			else
				N = N_32K_LOWCLK;

			CTS = CALC_CTS(N, hdmi_dev->tmdsclk/1000, 32);	//div a num to avoid the value is exceed 2^32(int)
			break;
		case HDMI_AUDIO_FS_44100:
			mclk_fs = FS_256;
			rate = AUDIO_441K;
			if(hdmi_dev->tmdsclk >= 594000000)
				N = N_441K_HIGHCLK;
			else if(hdmi_dev->tmdsclk >= 297000000)
				N = N_441K_MIDCLK;
			else
				N = N_441K_LOWCLK;

			CTS = CALC_CTS(N, hdmi_dev->tmdsclk/100, 441);
			break;
		case HDMI_AUDIO_FS_48000:
			mclk_fs = FS_256;
			rate = AUDIO_48K;
			if(hdmi_dev->tmdsclk >= 594000000)	//FS_153.6
				N = N_48K_HIGHCLK;
			else if(hdmi_dev->tmdsclk >= 297000000)
				N = N_48K_MIDCLK;
			else
				N = N_48K_LOWCLK;

			CTS = CALC_CTS(N, hdmi_dev->tmdsclk/1000, 48);
			break;
		case HDMI_AUDIO_FS_88200:
			mclk_fs = FS_256;
			rate = AUDIO_882K;
			if(hdmi_dev->tmdsclk >= 594000000)
				N = N_882K_HIGHCLK;
			else if(hdmi_dev->tmdsclk >= 297000000)
				N = N_882K_MIDCLK;
			else
				N = N_882K_LOWCLK;

			CTS = CALC_CTS(N, hdmi_dev->tmdsclk/100, 882);
			break;
		case HDMI_AUDIO_FS_96000:
			mclk_fs = FS_256;
			rate = AUDIO_96K;
			if(hdmi_dev->tmdsclk >= 594000000)	//FS_153.6
				N = N_96K_HIGHCLK;
			else if(hdmi_dev->tmdsclk >= 297000000)
				N = N_96K_MIDCLK;
			else
				N = N_96K_LOWCLK;

			CTS = CALC_CTS(N, hdmi_dev->tmdsclk/1000, 96);
			break;
		case HDMI_AUDIO_FS_176400:
			mclk_fs = FS_256;
			rate = AUDIO_1764K;
			if(hdmi_dev->tmdsclk >= 594000000)
				N = N_1764K_HIGHCLK;
			else if(hdmi_dev->tmdsclk >= 297000000)
				N = N_1764K_MIDCLK;
			else
				N = N_1764K_LOWCLK;

			CTS = CALC_CTS(N, hdmi_dev->tmdsclk/100, 1764);
			break;
		case HDMI_AUDIO_FS_192000:
			mclk_fs = FS_256;
			rate = AUDIO_192K;
			if(hdmi_dev->tmdsclk >= 594000000)	//FS_153.6
				N = N_192K_HIGHCLK;
			else if(hdmi_dev->tmdsclk >= 297000000)
				N = N_192K_MIDCLK;
			else
				N = N_192K_LOWCLK;

			CTS = CALC_CTS(N, hdmi_dev->tmdsclk/1000, 192);
			break;
		default:
			dev_err(hdmi_dev->hdmi->dev, "[%s] not support such sample rate %d\n", __FUNCTION__, audio->rate);
			return -ENOENT;
	}

	switch(audio->word_length)
	{
		case HDMI_AUDIO_WORD_LENGTH_16bit:
			word_length = I2S_16BIT_SAMPLE;
			break;
		case HDMI_AUDIO_WORD_LENGTH_20bit:
			word_length = I2S_20BIT_SAMPLE;
			break;
		case HDMI_AUDIO_WORD_LENGTH_24bit:
			word_length = I2S_24BIT_SAMPLE;
			break;
		default:
			word_length = I2S_16BIT_SAMPLE;
	}

	HDMIDBG("rate = %d, tmdsclk = %lu, N = %d, CTS = %d\n",  audio->rate, hdmi_dev->tmdsclk, N, CTS);
	/* more than 2 channels => layout 1 else layout 0 */
	hdmi_msk_reg(hdmi_dev, FC_AUDSCONF, m_AUD_PACK_LAYOUT, v_AUD_PACK_LAYOUT( (audio->channel > 2) ? 1 : 0));

	if(hdmi_dev->audiosrc == INPUT_SPDIF) {
		hdmi_msk_reg(hdmi_dev, AUD_CONF0, m_I2S_SEL, v_I2S_SEL(AUDIO_SPDIF_GPA));
		hdmi_msk_reg(hdmi_dev, AUD_SPDIF1, m_SET_NLPCM | m_SPDIF_WIDTH, v_SET_NLPCM(PCM_LINEAR) | v_SPDIF_WIDTH(word_length));
		//Mask fifo empty and full int and reset fifo
		hdmi_msk_reg(hdmi_dev, AUD_SPDIFINT, m_FIFO_EMPTY_MASK | m_FIFO_FULL_MASK, v_FIFO_EMPTY_MASK(1) | v_FIFO_FULL_MASK(1));
		hdmi_msk_reg(hdmi_dev, AUD_SPDIF0, m_SW_SAUD_FIFO_RST, v_SW_SAUD_FIFO_RST(1));
	}
	else {
		//Mask fifo empty and full int and reset fifo
		hdmi_msk_reg(hdmi_dev, AUD_INT, m_FIFO_EMPTY_MASK | m_FIFO_FULL_MASK, v_FIFO_EMPTY_MASK(1) | v_FIFO_FULL_MASK(1));
		hdmi_msk_reg(hdmi_dev, AUD_CONF0, m_SW_AUD_FIFO_RST, v_SW_AUD_FIFO_RST(1));
		hdmi_writel(hdmi_dev, MC_SWRSTZREQ, 0xF7);
		udelay(100);

		hdmi_msk_reg(hdmi_dev, AUD_CONF0, m_I2S_SEL | m_I2S_IN_EN, v_I2S_SEL(AUDIO_I2S) | v_I2S_IN_EN(channel));
		hdmi_writel(hdmi_dev, AUD_CONF1, v_I2S_MODE(I2S_STANDARD_MODE) | v_I2S_WIDTH(word_length));
	}

	hdmi_msk_reg(hdmi_dev, AUD_INPUTCLKFS, m_LFS_FACTOR, v_LFS_FACTOR(mclk_fs));

	//Set N value
	hdmi_msk_reg(hdmi_dev, AUD_N3, m_NCTS_ATOMIC_WR, v_NCTS_ATOMIC_WR(1));
	//Set CTS by manual
	hdmi_msk_reg(hdmi_dev, AUD_CTS3, m_N_SHIFT | m_CTS_MANUAL | m_AUD_CTS3,
		v_N_SHIFT(N_SHIFT_1) | v_CTS_MANUAL(1) | v_AUD_CTS3(CTS >> 16));
	hdmi_writel(hdmi_dev, AUD_CTS2, (CTS >> 8) & 0xff);
	hdmi_writel(hdmi_dev, AUD_CTS1, CTS & 0xff);
	
	hdmi_msk_reg(hdmi_dev, AUD_N3, m_AUD_N3, v_AUD_N3(N >> 16));
	hdmi_writel(hdmi_dev, AUD_N2, (N >> 8) & 0xff);
	hdmi_writel(hdmi_dev, AUD_N1, N & 0xff);

	hdmi_msk_reg(hdmi_dev, FC_AUDSCHNLS7, m_AUDIO_SAMPLE_RATE, v_AUDIO_SAMPLE_RATE(rate));

	hdmi_dev_config_aai(hdmi_dev, audio);

//	hdmi_msk_reg(hdmi_dev, AUD_CONF0, m_SW_AUD_FIFO_RST, v_SW_AUD_FIFO_RST(1));
//	hdmi_writel(hdmi_dev, MC_SWRSTZREQ, 0xF7);

    return 0;
}

static int hdmi_dev_control_output(struct hdmi *hdmi, int enable)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;

	HDMIDBG("[%s] %d\n", __FUNCTION__, enable);
	
	if(enable == HDMI_AV_UNMUTE) {
		//hdmi_msk_reg(hdmi_dev, AUD_CONF0, m_SW_AUD_FIFO_RST, v_SW_AUD_FIFO_RST(1));
		//hdmi_writel(hdmi_dev, MC_SWRSTZREQ, 0xF7);
		//unmute audio
		//hdmi_msk_reg(hdmi_dev, FC_AUDSCONF, m_AUD_PACK_SAMPFIT, v_AUD_PACK_SAMPFIT(0));
		hdmi_writel(hdmi_dev, FC_DBGFORCE, 0x00);
//		hdmi_msk_reg(hdmi_dev, FC_GCP, m_FC_SET_AVMUTE, v_FC_SET_AVMUTE(0));
	} else {
		if(enable & HDMI_VIDEO_MUTE) {
			hdmi_msk_reg(hdmi_dev, FC_DBGFORCE, m_FC_FORCEVIDEO, v_FC_FORCEVIDEO(1));
//			hdmi_msk_reg(hdmi_dev, FC_GCP, m_FC_SET_AVMUTE, v_FC_SET_AVMUTE(1));
		}
		if(enable & HDMI_AUDIO_MUTE) {
			//hdmi_msk_reg(hdmi_dev, FC_AUDSCONF, m_AUD_PACK_SAMPFIT, v_AUD_PACK_SAMPFIT(0x0F));
		}
	}
	return 0;
}

static int hdmi_dev_insert(struct hdmi *hdmi)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	
	HDMIDBG("%s", __FUNCTION__);
	hdmi_writel(hdmi_dev, MC_CLKDIS, 0x0);
	//mute audio
	//hdmi_msk_reg(hdmi_dev, FC_AUDSCONF, m_AUD_PACK_SAMPFIT, v_AUD_PACK_SAMPFIT(0x0F));
	/* report HPD state to HDCP (after configuration) */
//	hdmi_msk_reg(hdmi_dev, A_HDCPCFG0, m_RX_DETECT, v_RX_DETECT(1));
	return HDMI_ERROR_SUCESS;
}

static int hdmi_dev_remove(struct hdmi *hdmi)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	
	HDMIDBG("%s", __FUNCTION__);
	rk3288_hdmi_powerdown(hdmi_dev);
	return HDMI_ERROR_SUCESS;
}

static int hdmi_dev_enable(struct hdmi *hdmi)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;

	HDMIDBG("%s", __FUNCTION__);
	if(!hdmi_dev->enable) {
		hdmi_dev->enable = 1;
		hdmi_msk_reg(hdmi_dev, PHY_CONF0, m_ENHPD_RXSENSE_SIG, v_ENHPD_RXSENSE_SIG(1));
//		enable_irq(hdmi_dev->irq);
		if(hdmi_dev->workqueue) {
			queue_delayed_work(hdmi_dev->workqueue, &(hdmi_dev->delay_work), msecs_to_jiffies(0));
			hdmi_submit_work(hdmi, HDMI_HPD_CHANGE, 10, NULL);
		}
	}
	return 0;
}

static int hdmi_dev_disable(struct hdmi *hdmi)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;

	HDMIDBG("%s", __FUNCTION__);
	if(hdmi_dev->enable) {
		hdmi_dev->enable = 0;
		hdmi_msk_reg(hdmi_dev, PHY_CONF0, m_ENHPD_RXSENSE_SIG, v_ENHPD_RXSENSE_SIG(0));
//		disable_irq(hdmi_dev->irq);
	}
	return 0;
}

void hdmi_dev_initial(struct hdmi_dev *hdmi_dev, struct hdmi_ops *ops)
{
	if(ops) {
		ops->enable	= hdmi_dev_enable;
		ops->disable	= hdmi_dev_disable;
		ops->getStatus	= hdmi_dev_detect_hotplug;
		ops->insert	= hdmi_dev_insert;
		ops->remove	= hdmi_dev_remove;
		ops->getEdid	= hdmi_dev_read_edid;
		ops->setVideo	= hdmi_dev_config_video;
		ops->setAudio	= hdmi_dev_config_audio;
		ops->setMute	= hdmi_dev_control_output;
		ops->setVSI	= hdmi_dev_config_vsi;
	}
	
	// reset hdmi
	writel_relaxed((1 << 9) | (1 << 25), RK_CRU_VIRT + 0x01d4);
	udelay(1);
	writel_relaxed((0 << 9) | (1 << 25), RK_CRU_VIRT + 0x01d4);

	rk3288_hdmi_powerdown(hdmi_dev);
	
	//mute unnecessary interrrupt, only enable hpd
	hdmi_writel(hdmi_dev, IH_MUTE_FC_STAT0, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_FC_STAT1, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_FC_STAT2, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_AS_STAT0, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_PHY_STAT0, 0xfe);
	hdmi_writel(hdmi_dev, IH_MUTE_I2CM_STAT0, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_CEC_STAT0, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_VP_STAT0, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_I2CMPHY_STAT0, 0xff);
	hdmi_writel(hdmi_dev, IH_MUTE_AHBDMAAUD_STAT0, 0xff);
	
	hdmi_writel(hdmi_dev, PHY_MASK, 0xf1);
	
	//Force output black
	hdmi_writel(hdmi_dev, FC_DBGTMDS2, 0x00);	/*R*/
	hdmi_writel(hdmi_dev, FC_DBGTMDS1, 0x00);	/*G*/
	hdmi_writel(hdmi_dev, FC_DBGTMDS0, 0x00);	/*B*/
}

irqreturn_t hdmi_dev_irq(int irq, void *priv)
{		
	struct hdmi_dev *hdmi_dev = priv;
	struct hdmi *hdmi = hdmi_dev->hdmi;
	char phy_pol = hdmi_readl(hdmi_dev, PHY_POL0);
	char phy_status = hdmi_readl(hdmi_dev, PHY_STAT0);
	char phy_int0 = hdmi_readl(hdmi_dev, PHY_INI0);
	
	//read interrupt
	char fc_stat0 = hdmi_readl(hdmi_dev, IH_FC_STAT0);
	char fc_stat1 = hdmi_readl(hdmi_dev, IH_FC_STAT1);
	char fc_stat2 = hdmi_readl(hdmi_dev, IH_FC_STAT2);
	char aud_int = hdmi_readl(hdmi_dev, IH_AS_SATA0);
	char phy_int = hdmi_readl(hdmi_dev, IH_PHY_STAT0);
	char i2cm_int = hdmi_readl(hdmi_dev, IH_I2CM_STAT0);
	char vp_stat0 = hdmi_readl(hdmi_dev, IH_VP_STAT0);
	char phy_i2cm_int = hdmi_readl(hdmi_dev, IH_I2CMPHY_STAT0);
	char cec_int = hdmi_readl(hdmi_dev, IH_CEC_STAT0);
	
	//hdcp_int = hdmi_readl(hdmi_dev, A_APIINTSTAT);

	//clear interrupt
	hdmi_writel(hdmi_dev, IH_FC_STAT0, fc_stat0);
	hdmi_writel(hdmi_dev, IH_FC_STAT1, fc_stat1);
	hdmi_writel(hdmi_dev, IH_FC_STAT2, fc_stat2);
	hdmi_writel(hdmi_dev, IH_AS_SATA0, aud_int);
	hdmi_writel(hdmi_dev, IH_PHY_STAT0, phy_int);
	hdmi_writel(hdmi_dev, IH_VP_STAT0, vp_stat0);
//	hdmi_writel(hdmi_dev, IH_I2CM_STAT0, i2cm_int);
//	hdmi_writel(hdmi_dev, IH_I2CMPHY_STAT0, phy_i2cm_int);
	hdmi_writel(hdmi_dev, IH_CEC_STAT0, cec_int);
	
	//hdmi_writel(hdmi_dev, A_APIINTCLR, hdcp_int);

//	printk("phy_int 0x%02x i2cm_int 0x%02x phy_i2cm_int 0x%02x cec_int 0x%02x aud_int 0x%02x\n",
//		phy_int, i2cm_int, phy_i2cm_int, cec_int, aud_int);
//	printk("phy_stat is 0x%02x phy_pol 0x%02x phy_int 0x%02x\n\n", phy_status, phy_pol, hdmi_readl(hdmi_dev, PHY_INI0));
	if(phy_int0) {
		printk("phy_int 0x%02x i2cm_int 0x%02x phy_i2cm_int 0x%02x cec_int 0x%02x aud_int 0x%02x\n",
			phy_int, i2cm_int, phy_i2cm_int, cec_int, aud_int);
		printk("phy_stat is 0x%02x phy_pol 0x%02x phy_int 0x%02x\n\n", phy_status, phy_pol, hdmi_readl(hdmi_dev, PHY_INI0));
		phy_pol = (phy_pol & (~phy_int0)) | (~(phy_pol & phy_int0));
		hdmi_writel(hdmi_dev, PHY_POL0, phy_pol);
		printk("new phy pol is 0x%02x cur pol is 0x%02x\n\n", phy_pol, hdmi_readl(hdmi_dev, PHY_POL0));
		if((phy_int & m_HPD) || ((phy_int & 0x3c) == 0x3c)) {
			hdmi_submit_work(hdmi, HDMI_HPD_CHANGE, 10, NULL);
		}
	}
//	if(hdmi_readl(hdmi_dev, PHY_INI0) & 0x2) {
//		if(phy_pol & 0x02) {
//			phy_pol &= 0xf1;
//		}
//		else{
//			phy_pol |= 0x02;
//		}
//		hdmi_writel(hdmi_dev, PHY_POL0, phy_pol);
//		printk("new phy int is 0x%02x 0x%02x\n\n", phy_pol, hdmi_readl(hdmi_dev, PHY_POL0));
//	}
	//HPD or RX_SENSE
//	if((phy_int & m_HPD) || ((phy_int & 0x3c) == 0x3c)) {
//		hdmi_submit_work(hdmi, HDMI_HPD_CHANGE, 10, NULL);
//	}

	//I2CM write or read result
//	if(i2cm_int & (m_SCDC_READREQ | m_I2CM_DONE | m_I2CM_ERROR)) {
//		//spin_lock(&hdmi_drv->irq_lock);
//		hdmi_dev->i2cm_int = i2cm_int;
//		//spin_unlock(&hdmi_drv->irq_lock);
//	}
//
//	//PHY I2CM write or read result
//	if(phy_i2cm_int & (m_I2CMPHY_DONE | m_I2CMPHY_ERR)) {
//		//mutex_lock(&hdmi_dev->int_mutex);
//		hdmi_dev->phy_i2cm_int = phy_i2cm_int;
//		//mutex_unlock(&hdmi_dev->int_mutex);
//	}
	
	// Audio error
	if(aud_int) {
		printk("audio int is 0x%02x\n", aud_int);
		hdmi_msk_reg(hdmi_dev, AUD_CONF0, m_SW_AUD_FIFO_RST, v_SW_AUD_FIFO_RST(1));
		hdmi_writel(hdmi_dev, MC_SWRSTZREQ, 0xF7);
	}
	//CEC
	if(cec_int) {	//TODO Daisen wait to modify
	}

	//HDCP
//	if(hdmi_drv->hdcp_irq_cb)
//		hdmi_drv->hdcp_irq_cb(i2cm_int);

	return IRQ_HANDLED;
}


