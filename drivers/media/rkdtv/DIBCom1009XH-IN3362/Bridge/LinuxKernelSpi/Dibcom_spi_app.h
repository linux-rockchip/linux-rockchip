#ifndef __DIB_SPI_HHHHHHHHHHHHH
#define __DIB_SPI_HHHHHHHHHHHH

//////////////////////////////////////
//include the .h file here
//dibcom write
int DiBcom_spi_write( unsigned char *i2c_tx_buf, int txcnt);

//dibcom before read , should write the register address first (write without stop)
int DiBcom_spi_read( unsigned char  *i2c_rx_buf, int rxcnt);
#endif

