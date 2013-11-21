#include <linux/delay.h>
#include "rk616_hdmi.h"
#include "rk616_hdmi_hw.h"
#include "rk616_hdcp.h"

void rk616_set_colorbar(int enable)
{
        static int display_mask = 0;
        int reg_value;
        if (enable) {
                if (!display_mask) {
                        if (hdmi->tmdsclk <= (HDMI_SYS_FREG_CLK << 2)) {
	                        hdmi_readl(SYS_CTRL, &reg_value);
                                hdmi_msk_reg(SYS_CTRL, m_REG_CLK_SOURCE, v_REG_CLK_SOURCE_SYS);  
        	                hdmi_writel(HDMI_COLORBAR, 0x00);
                                hdmi_writel(SYS_CTRL, reg_value);
                        } else {
        	                hdmi_writel(HDMI_COLORBAR, 0x00);
                        }

                        display_mask = 1;
                }
        } else {
                if (display_mask) {

                        if (hdmi->tmdsclk <= (HDMI_SYS_FREG_CLK << 2)) {
	                        hdmi_readl(SYS_CTRL, &reg_value);
                                hdmi_msk_reg(SYS_CTRL, m_REG_CLK_SOURCE, v_REG_CLK_SOURCE_SYS);  
                                hdmi_writel(HDMI_COLORBAR, 0x10);
                                hdmi_writel(SYS_CTRL, reg_value);
                        } else {
                                hdmi_writel(HDMI_COLORBAR, 0x10);
                        }

                        display_mask = 0;
                }
        }
}

void rk616_hdcp_disable(void)
{
	// Diable HDCP Interrupt
	hdmi_writel(HDCP_INT_MASK1, 0x00);
	// Stop and Reset HDCP
	hdmi_msk_reg(HDCP_CTRL1, m_ENCRYPT_ENABLE | m_AUTH_STOP | m_HDCP_RESET, 
		v_ENCRYPT_ENABLE(0) | v_AUTH_STOP(1) | v_HDCP_RESET(1) );
}

int 	rk616_hdcp_key_check(struct hdcp_keys *key)
{
	int i = 0;
       	
	DBG("HDCP: check hdcp key\n");
	//check 40 private key 
	for(i = 0; i < HDCP_PRIVATE_KEY_SIZE; i++){
		if(key->DeviceKey[i] != 0x00)
			return HDCP_KEY_VALID;
	}
	//check aksv
   	for(i = 0; i < 5; i++){
		if(key->KSV[i] != 0x00)
			return HDCP_KEY_VALID;
	}

	return HDCP_KEY_INVALID;		
}

int	rk616_hdcp_load_key2mem(struct hdcp_keys *key)
{
	int i;
	DBG("HDCP: rk616_hdcp_load_key2mem start");
	// Write 40 private key
	for(i = 0; i < HDCP_PRIVATE_KEY_SIZE; i++)
		hdmi_writel(HDCP_KEY_FIFO, key->DeviceKey[i]);
	
	// Write 1st aksv
	for(i = 0; i < 5; i++)
		hdmi_writel(HDCP_KEY_FIFO, key->KSV[i]);
		
	// Write 2nd aksv
	for(i = 0; i < 5; i++)
		hdmi_writel(HDCP_KEY_FIFO, key->KSV[i]);
	DBG("HDCP: rk616_hdcp_load_key2mem end");
	return HDCP_OK;
}

int	rk616_hdcp_start_authentication(void)
{
	int temp;
	int retry = 0;

	if(hdcp->keys == NULL) {
		printk(KERN_ERR "HDCP: key is not loaded\n");
		return HDCP_KEY_ERR;
	}
	
	if(rk616_hdcp_key_check(hdcp->keys) == HDCP_KEY_INVALID){
		printk(KERN_ERR "loaded HDCP key is incorrect\n");
		return HDCP_KEY_ERR;
	}	

        if (hdmi->tmdsclk > (HDMI_SYS_FREG_CLK << 2)) {
        	// Select TMDS CLK to configure regs
	        hdmi_msk_reg(SYS_CTRL, m_REG_CLK_SOURCE, v_REG_CLK_SOURCE_TMDS);
        }

	hdmi_readl(HDCP_KEY_STATUS,&temp);
	while( ( temp & m_KEY_READY) == 0 ) {
		if(retry > 1000) {
			printk(KERN_ERR "HDCP: loaded key error\n");
			return HDCP_KEY_ERR;
		}
		rk616_hdcp_load_key2mem(hdcp->keys);
		msleep(1);
		hdmi_readl(HDCP_KEY_STATUS,&temp);
                retry++;
	}
        
        // Config DDC bus clock: ddc_clk = reg_clk/4*(reg 0x4c 0x4b)
        retry = hdmi->tmdsclk/(HDCP_DDC_CLK << 2);
        hdmi_writel(DDC_CLK_L, retry & 0xFF);
        hdmi_writel(DDC_CLK_H, (retry >> 8) & 0xFF);
 
	hdmi_writel(HDCP_CTRL2, 0x77);
	
	//Enable interrupt
	hdmi_writel(HDCP_INT_MASK1, m_INT_HDCP_ERR | m_INT_BKSV_READY | m_INT_BKSV_UPDATE | m_INT_AUTH_SUCCESS | m_INT_AUTH_READY);
	 hdmi_writel(HDCP_INT_MASK2, 0x00);

	//Start authentication
	hdmi_msk_reg(HDCP_CTRL1, m_AUTH_START | m_ENCRYPT_ENABLE | m_ADVANED_ENABLE, v_AUTH_START(1) | v_ENCRYPT_ENABLE(1) | v_ADVANED_ENABLE(0));
	

        if (hdmi->tmdsclk <= (HDMI_SYS_FREG_CLK << 2)) {
                hdmi_msk_reg(SYS_CTRL, m_REG_CLK_SOURCE, v_REG_CLK_SOURCE_TMDS);
        }
	return HDCP_OK;
}

int	rk616_hdcp_stop_authentication(void)
{
        hdmi_msk_reg(SYS_CTRL, m_REG_CLK_SOURCE, v_REG_CLK_SOURCE_SYS);  
        hdmi_writel(DDC_CLK_L, 0x1c);
	hdmi_writel(DDC_CLK_H, 0x00);
	hdmi_writel(HDCP_CTRL2, 0x08);
	hdmi_writel(HDCP_INT_MASK2, 0x06);
	hdmi_writel(HDCP_CTRL1, 0x02);
        return 0;
	//hdmi_writel(HDCP_CTRL1, 0x0a);
}

#if 0
int	rk616_hdcp_check_bksv(void)
{
	int i, j;
	int temp = 0, bksv[5];
	char *invalidkey;
	
	for(i = 0; i < 5; i++) {
		hdmi_readl(HDCP_KSV_BYTE0 + (4 - i), &temp);
		bksv[i] = temp & 0xFF;
	}
	DBG("bksv is 0x%02x%02x%02x%02x%02x", bksv[0], bksv[1], bksv[2], bksv[3], bksv[4]);
	
	temp = 0; 	
	for (i = 0; i < 5; i++)
	{
    	for (j = 0; j < 8; j++)
    	{
    		if (bksv[i] & 0x01)
    		{
        		temp++;
    		}
    		bksv[i] >>= 1;
    	}
 	}
 	if (temp != 20)
    	return HDCP_KSV_ERR;
	
	for(i = 0; i < hdcp->invalidkey; i++)
	{
		invalidkey = hdcp->invalidkeys + i *5;
		if(memcmp(bksv, invalidkey, 5) == 0) {
			printk(KERN_ERR "HDCP: BKSV was revocated!!!\n");
			hdmi_msk_reg(HDCP_CTRL1, m_BKSV_INVALID | m_ENCRYPT_ENABLE, v_BKSV_INVALID(1) | v_ENCRYPT_ENABLE(1));
			return HDCP_KSV_ERR;
		}
	}
	hdmi_msk_reg(HDCP_CTRL1, m_BKSV_VALID | m_ENCRYPT_ENABLE, v_BKSV_VALID(1) | v_ENCRYPT_ENABLE(1));
	return HDCP_OK;
}
#endif

int rk616_hdcp_error(int value)
{
        if (value && 0x80) {
                printk("Timed out waiting for downstream repeater\n");

        } else if (value && 0x40) {
                printk("Too many devices connected to repeater tree\n");

        } else if (value && 0x20) {
                printk("SHA-1 hash check of BKSV list failed\n");

        } else if (value && 0x10) {
                printk("SHA-1 hash check of BKSV list failed\n");

        } else if (value && 0x08) {
                printk("DDC channels no acknowledge\n");

        } else if (value && 0x04) {
                printk("Pj mismatch\n");

        } else if (value && 0x02) {
                printk("Ri mismatch\n");

        } else if (value && 0x01) {
                printk("Bksv is wrong\n");

        } else {
                return 0;
        }
        return 1;
}

void rk616_hdcp_interrupt(char *status1, char *status2)
{
	int interrupt1 = 0;
	int interrupt2 = 0;
	int temp =0;
	hdmi_readl(HDCP_INT_STATUS1,&interrupt1);
	hdmi_readl(HDCP_INT_STATUS2,&interrupt2);
	if(interrupt1) {
		hdmi_writel(HDCP_INT_STATUS1, interrupt1);
		if(interrupt1 & m_INT_HDCP_ERR){
			hdmi_readl(HDCP_ERROR,&temp);
			printk(KERN_INFO "HDCP: Error reg 0x65 = 0x%02x\n", temp);
                        rk616_hdcp_error(temp); 
	                hdmi_writel(HDCP_ERROR, 0x00);
		}
	}
	if(interrupt2)
		hdmi_writel(HDCP_INT_STATUS2, interrupt2);
	
	*status1 = interrupt1;
	*status2 = interrupt2;

        hdmi_readl(HDCP_ERROR, &temp);
        DBG("HDCP: Error reg 0x65 = 0x%02x\n", temp);
}

