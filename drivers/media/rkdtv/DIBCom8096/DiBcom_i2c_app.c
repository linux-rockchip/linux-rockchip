#include <linux/i2c.h>

#include <include/adapter/common.h>
#include "DiBcom_i2c_app.h"
//#include <../../../../../include/asm/arch/api_i2c.h>
//#include <linux/spi/spi.h>

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

#define I2C_TX_BUFFER_ERROR             DIB_RETURN_ERROR
#define I2C_RX_BUFFER_ERROR             DIB_RETURN_ERROR
#define I2C_NACK_ERROR                  DIB_RETURN_ERROR
#define I2C_DEV_ADR_NACK_ERROR          DIB_RETURN_ERROR
#define I2C_WR_NACK_ERROR               DIB_RETURN_ERROR



/*i2c address*/
#define DIB_I2C_ADDR0 0x12
#define DIB_I2C_ADDR1 0x90 /*virtual address*/
//#define DIB_I2C_ADDR2 0xc0

/*i2c interface*/
extern struct i2c_client *gTSTVModuleClient;

extern int  tstv_control_set_reg(struct i2c_client *client, unsigned char reg, unsigned char const buf[], int len);
extern int  tstv_control_read_regs(struct i2c_client *client, unsigned char reg, unsigned char buf[], unsigned len);

unsigned char DiBcom_i2c_write(int addr, unsigned char *i2c_tx_buf, int txcnt)
{
    if(!i2c_tx_buf && txcnt)
    {
        return I2C_TX_BUFFER_ERROR;
    }

    if(addr == DIB_I2C_ADDR0)
    {
        //gTSTVModuleClient = &dib_i2c_client0;
    }
    else if(addr == DIB_I2C_ADDR1)
    {
        //gTSTVModuleClient = &dib_i2c_client1;
    }
    else
    {
       
    }


	gTSTVModuleClient->addr = addr >> 1;
	
    //gTSTVModuleClient->mode = DIRECTMODE; //for dibcom8076
    if(tstv_control_set_reg(gTSTVModuleClient, addr, i2c_tx_buf, txcnt) != 0)
    {
        printk("TSTV:DIBCOM8096: set i2c register failed\t%s[%d]\n", __FUNCTION__, __LINE__);
        return I2C_TX_BUFFER_ERROR;
    }
    else
    {
        //DBG("TSTV:DIBCOM8076: set i2c register success, addr 0x%x\t%s[%d]\n", addr, __FUNCTION__, __LINE__);
        return 0;
    }
}
//EXPORT_SYMBOL(DiBcom_i2c_write);

unsigned char DiBcom_i2c_read(int addr, unsigned char *i2c_tx_buf, int txcnt, unsigned char *i2c_rx_buf, int rxcnt)
{
    int i;
	
    if (!i2c_tx_buf && txcnt)			/* check tx buffer */
    {
        printk("TSTV:DIBCOM8096: read i2c register failed, i2c_tx_buf %p, txcnt %d \t%s[%d]\n", i2c_tx_buf, txcnt, __FUNCTION__, __LINE__);
	return I2C_TX_BUFFER_ERROR;
    }

    if (!i2c_rx_buf && rxcnt)			/* check rx buffer */
    {
        printk("TSTV:DIBCOM8096: read i2c register failed, i2c_rx_buf %p, rxcnt %d \t%s[%d]\n", i2c_rx_buf, rxcnt, __FUNCTION__, __LINE__);
	return I2C_RX_BUFFER_ERROR;
    }

    if(!i2c_tx_buf && txcnt)
    {
        return I2C_TX_BUFFER_ERROR;
    }

    if(addr == DIB_I2C_ADDR0)
    {
        //gTSTVModuleClient = &dib_i2c_client0;
    }
    else if(addr == DIB_I2C_ADDR1)
    {
        //gTSTVModuleClient = &dib_i2c_client1;
    }
    else
    {
        
    }
	
	gTSTVModuleClient->addr = addr >> 1;
	

    for (i=0 ; i<rxcnt ; i++)			/* initialize receive buffer */
	i2c_rx_buf[i] = 0xfe;
    
    if(txcnt > 0)
    {
            //gTSTVModuleClient->mode = DIRECTMODE; //for dibcom8096 tuner
            addr |= 1;
            tstv_control_set_reg(gTSTVModuleClient, addr, i2c_tx_buf, txcnt);
            addr |= 1;
            tstv_control_read_regs(gTSTVModuleClient, addr, i2c_rx_buf, rxcnt);
    }
/*
    DBG("TSTV:DIBCOM8076: read i2c register, i2c_rx_buf %p, rxcnt %d, txcnt %d, addr 0x%x\t%s[%d]\n", i2c_rx_buf, rxcnt, txcnt, addr, __FUNCTION__, __LINE__);
    DBG("TX:");
    for(i = 0; i < txcnt; i++)
        DBG(" 0x%x", i2c_tx_buf[i]);
    DBG(" \n");
    DBG("RX:");
    for(i = 0; i < rxcnt; i++)
        DBG(" 0x%x", i2c_rx_buf[i]);
    DBG("\n");
//*/
    return 0;
}
//EXPORT_SYMBOL(DiBcom_i2c_read);

#if 0 //test code
void test_dibcom_7070()
{
    unsigned short i=0, sub;
    unsigned char i2c_tx_buf[4];
    unsigned char i2c_rx_buf[2];

    sub = 768;
    i2c_tx_buf[0] = (sub >> 8) & 0xff;
    i2c_tx_buf[1] = sub  & 0xff;
    for(i = 1; i< 4 ; i++)
    {
        i2c_rx_buf[2] = 0xfe;
        i2c_rx_buf[3] = 0xfe;

        if((DiBcom_i2c_read(0x12, i2c_tx_buf, 2,i2c_rx_buf, 2)))
        {
            DBG("write error\n");
            return ;
        }
        
        DBG("i2c_tx_buf[0]=0x%02x, i2c_tx_buf[1]=0x%02x,i2c_rx_buf[0]=0x%02x,i2c_rx_buf[1]=0x%02x\n",i2c_tx_buf[0], i2c_tx_buf[1],i2c_rx_buf[0],i2c_rx_buf[1]);
    }
   
    sub = 770;
    i2c_tx_buf[0] = (sub >> 8) & 0xff;
    i2c_tx_buf[1] = sub  & 0xff;
    for(i = 1; i< 11 ; i++)
    {
        i2c_tx_buf[2] = 0;
        i2c_tx_buf[3] = i;

        DBG("i2c_tx_buf[0]=0x%02x, i2c_tx_buf[1]=0x%02x,i2c_tx_buf[2]=0x%02x,i2c_tx_buf[3]=0x%02x\n",i2c_tx_buf[0], i2c_tx_buf[1],i2c_tx_buf[2],i2c_tx_buf[3]);
        if((DiBcom_i2c_write(0x12, i2c_tx_buf, 4)))
        {
            DBG("write error\n");
            return ;
        }
        if(DiBcom_i2c_read(0x12, i2c_tx_buf, 2, i2c_rx_buf, 2))
        {
            DBG("read write error\n");
            return ;
        }        
        DBG("i2c_tx_buf[0]=0x%02x, i2c_tx_buf[1]=0x%02x,i2c_rx_buf[0]=0x%02x,i2c_rx_buf[1]=0x%02x\n",i2c_tx_buf[0], i2c_tx_buf[1],i2c_rx_buf[0],i2c_rx_buf[1]);
    }   
}
#endif
