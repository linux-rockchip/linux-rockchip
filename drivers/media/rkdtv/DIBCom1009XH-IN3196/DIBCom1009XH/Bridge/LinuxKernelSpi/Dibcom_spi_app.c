#include <linux/platform_device.h>
#include <linux/spi/spi.h>

#define SPI_TX_BUFFER_ERROR             -1
#define SPI_RX_BUFFER_ERROR             -1

extern int tstv_spi_set_reg(struct spi_device *spi, u8 buf[], unsigned len);
extern int  tstv_spi_read_reg(struct spi_device *spi, u8 buf[], unsigned len);
extern struct spi_device *gTSTVModuleSpi;


int DiBcom_spi_write( unsigned char *spi_tx_buf, unsigned int txcnt)
{
	
	
	if(!spi_tx_buf && txcnt)
    {
        return SPI_TX_BUFFER_ERROR;
    }
	
	if(tstv_spi_set_reg(gTSTVModuleSpi,spi_tx_buf, txcnt) != 0)
	{
		printk("TSTV:DIBCOM1009XH: set spi register failed\t%s[%d]\n", __FUNCTION__, __LINE__);
		return SPI_TX_BUFFER_ERROR;
	}
	return 0;
}


int DiBcom_spi_read(unsigned char *spi_rx_buf, int rxcnt)
{
    int i;

    if (!spi_rx_buf && rxcnt)			/* check rx buffer */
    {
        printk("TSTV:DIBCOM1009XH: read spi register failed\t%s[%d]\n",  __FUNCTION__, __LINE__);
		return SPI_RX_BUFFER_ERROR;
    }

    for (i=0 ; i<rxcnt ; i++)			/* initialize receive buffer */
	spi_rx_buf[i] = 0x0;
    if(tstv_spi_read_reg(gTSTVModuleSpi, spi_rx_buf, rxcnt) != 0)
    {
    	
		printk("TSTV:DIBCOM1009XH: read spi register failed\t%s[%d]\n", __FUNCTION__, __LINE__);
		return SPI_TX_BUFFER_ERROR;
	}
    
    return 0;
}

