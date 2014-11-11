#include <linux/delay.h>
#include "../hdmi-cec.h"
#include "rk3288_hdmi.h"
#include "rk3288_hdmi_hw.h"

/* static wait_queue_head_t	wait;*/
static int init = 1;
void rk3288_hdmi_cec_isr(struct hdmi_dev *hdmi_dev, char cec_int)
{
	CECDBG("%s cec 0x%x\n", __func__, cec_int);
	if (cec_int & m_EOM)
		hdmi_cec_submit_work(EVENT_RX_FRAME, 0, NULL);
	if (cec_int & m_DONE)
		CECDBG("send frame success\n");
}

static int rk3288_hdmi_cec_readframe(struct hdmi *hdmi,
					struct cec_framedata *frame)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	int i, count;
	char *data = (char *)frame;

	if (frame == NULL)
		return -1;
	count = hdmi_readl(hdmi_dev, CEC_RX_CNT);
	CECDBG("%s count %d\n", __func__, count);
	for (i = 0; i < count; i++) {
		data[i] = hdmi_readl(hdmi_dev, CEC_RX_DATA0 + i);
		CECDBG("%02x\n", data[i]);
	}
	hdmi_writel(hdmi_dev, CEC_LOCK, 0x0);
	return 0;
}


void rk3288_hdmi_cec_setcecla(struct hdmi *hdmi, int ceclgaddr)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	short val;

	if (ceclgaddr < 0 || ceclgaddr > 16)
		return;
	val = 1 << ceclgaddr;
	hdmi_writel(hdmi_dev, CEC_ADDR_L, val & 0xff);
	hdmi_writel(hdmi_dev, CEC_ADDR_H, val>>8);
}

static int rk3288_hdmi_cec_sendframe(struct hdmi *hdmi,
					struct cec_framedata *frame)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;
	int i, interrupt;

	CECDBG("TX srcDestAddr %02x opcode %02x ",
		 frame->srcDestAddr, frame->opcode);
	if (frame->argCount) {
		CECDBG("args:");
		for (i = 0; i < frame->argCount; i++)
			CECDBG("%02x ", frame->args[i]);

	}
	CECDBG("\n");
	if ((frame->srcDestAddr & 0x0f) == ((frame->srcDestAddr >> 4) & 0x0f)) {
		/*it is a ping command*/
		hdmi_writel(hdmi_dev, CEC_TX_DATA0, frame->srcDestAddr);
		hdmi_writel(hdmi_dev, CEC_TX_CNT, 1);
	} else {
		hdmi_writel(hdmi_dev, CEC_TX_DATA0, frame->srcDestAddr);
		hdmi_writel(hdmi_dev, CEC_TX_DATA0 + 1, frame->opcode);
		for (i = 0; i < frame->argCount; i++)
			hdmi_writel(hdmi_dev,
					CEC_TX_DATA0 + 2 + i, frame->args[i]);
		hdmi_writel(hdmi_dev, CEC_TX_CNT, frame->argCount + 2);
	}
	/*Start TX*/
	hdmi_msk_reg(hdmi_dev, CEC_CTRL, m_CEC_SEND, v_CEC_SEND(1));
	i = 20;
	while (i--) {
		udelay(1000);
		interrupt = hdmi_readl(hdmi_dev, IH_CEC_STAT0);
		if (interrupt & (m_ERR_INITIATOR | m_ARB_LOST |
					m_NACK | m_DONE)) {
			hdmi_writel(hdmi_dev, IH_CEC_STAT0,
					interrupt & (m_ERR_INITIATOR |
					m_ARB_LOST | m_NACK | m_DONE));
			break;
		}
	}
	CECDBG("%s interrupt 0x%02x\n", __func__, interrupt);
	if (interrupt & m_DONE)
		return 0;
	else if (interrupt & m_NACK)
		return 1;
	else
		return  -1;
}

void rk3288_hdmi_cec_init(struct hdmi *hdmi)
{
	struct hdmi_dev *hdmi_dev = hdmi->property->priv;

	if (!hdmi->cecenable) {
		hdmi_writel(hdmi_dev, CEC_MASK, 0x7f);
		return;
	}
	if ( init) {
		hdmi_cec_init(hdmi, rk3288_hdmi_cec_sendframe,
			      rk3288_hdmi_cec_readframe,
			      rk3288_hdmi_cec_setcecla);
		init = 0;
		/* init_waitqueue_head(&wait); */
	}
	hdmi_writel(hdmi_dev, CEC_MASK, 0x00);
	hdmi_writel(hdmi_dev, IH_MUTE_CEC_STAT0, m_ERR_INITIATOR |
		m_ARB_LOST | m_NACK | m_DONE);
	CECDBG("%s", __func__);
}
