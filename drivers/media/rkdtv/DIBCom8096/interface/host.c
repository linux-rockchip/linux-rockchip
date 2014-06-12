#include <adapter/common.h>
#include <adapter/busdescriptor.h>
#include <adapter/busadapter.h>

//#define DEBUG_DATA
#if 1
#define DBG(x...)	printk(KERN_DEBUG x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

struct i2c_state {
    void *priv;
};
struct dibDataBusHost* gBusI2CHost = NULL;

extern unsigned char DiBcom_i2c_write(int addr, unsigned char *i2c_tx_buf, int txcnt);
extern unsigned char DiBcom_i2c_read(int addr, unsigned char *i2c_tx_buf, int txcnt, unsigned char *i2c_rx_buf, int rxcnt);

/* transmit txcnt bytes to device */
static int I2C_backend_Write(struct i2c_state *i2c, int addr, unsigned char *i2c_tx_buf, int txcnt)
{
    int i;
    //int err;

    if (!i2c_tx_buf && txcnt)     /* check tx buffer */
        return 1;
    if(0)
    {
            printk((char *)"-W- ");	
            printk("0x%02x] ", addr);
            printk("[%d]", txcnt);
    		
            for(i=0;i<txcnt;i++)
            {
                printk(" 0x%02x ", i2c_tx_buf[i]);
            }

            if(txcnt==3)
            {
                printk(" [%d: %d]", i2c_tx_buf[0], (i2c_tx_buf[1]<<8)|i2c_tx_buf[2]);

            }
            else
            {
                printk(" [%d: %d]", (i2c_tx_buf[0]<<8)|i2c_tx_buf[1], (i2c_tx_buf[2]<<8)|i2c_tx_buf[3]);
            }
            printk("\r\n");		
        }
    addr = addr & 0xfe;

    //transmit ...
    if(DiBcom_i2c_write(addr, i2c_tx_buf, txcnt) !=DIB_RETURN_SUCCESS)
    {
        printk((char *)"-W- ");
        printk("0x%02x] ", addr);
        printk("DiBcom_i2c_write error");
        printk("\r\n");		
        return 1;
    }
    return 0;
}

/* transmit txcnt bytes to device then read rxcnt bytes */
static int I2C_backend_WriteRead(struct i2c_state *i2c, int addr,  unsigned char  *i2c_tx_buf, int txcnt,
             unsigned char *i2c_rx_buf, int rxcnt)
{
    int i;

    if (!i2c_tx_buf && txcnt)     /* check tx buffer */
        return 1;

    if (!i2c_rx_buf && rxcnt)     /* check rx buffer */
        return 1;

    for (i=0 ; i<rxcnt ; i++)     /* initialize receive buffer */
        i2c_rx_buf[i] = 0xfe;

    //addr = addr & 0xfe;

    if(DiBcom_i2c_read((unsigned char)addr, i2c_tx_buf, txcnt, i2c_rx_buf, rxcnt) !=DIB_RETURN_SUCCESS)
    {
        printk((char *)"-R- ");
        printk("0x%02x] ", addr);
        printk("DiBcom_i2c_read error");
        printk("\r\n");		

        return 1;
    }
    if(0)
    {

        printk((char *)"-R- ");
        printk("0x%02x] ", addr);

        for(i=0;i<txcnt;i++)
        {
            printk("[w 0x%02x]", i2c_tx_buf[i]);
        }
        for(i=0;i<rxcnt;i++)
        {
            printk(" [r 0x%02x]", i2c_rx_buf[i]);
        }

        if(txcnt == 1)
        {
            printk(" [%d: ", i2c_tx_buf[0]);
        printk(" %d]", (i2c_rx_buf[0]<<8)|i2c_rx_buf[1]);
        }
        else
        {
            printk(" [%d: ", (i2c_tx_buf[0]<<8)|i2c_tx_buf[1]);
            printk(" %d]", (i2c_rx_buf[0]<<8)|i2c_rx_buf[1]);	
        }
            printk("\r\n");
    }
    return 0;
}

static int host_i2c_xfer(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
    struct i2c_state *st = NULL;
    
    //DBG("TSTV:DIBCOM8076: client is %p\n", client);
    st = client->host->priv;
    msg->address = data_bus_access_device_id(client, msg);
#ifdef DEBUG_DATA
    dbgp("i2c_%s: (addr: %02x): ",msg->rx == NULL ? "wr" : "rd",data_bus_access_device_id(client, msg) >> 1);
    dbg_bufdump(msg->tx,msg->txlen);
    if (msg->rx != NULL && msg->rxlen != 0) {
        dbgp(" - ");
        dbg_bufdump(msg->rx,msg->rxlen);
    }
    dbgp("\n");
#endif

    if (msg->rx == NULL || msg->rxlen == 0) {
        // implement here the write function and return DIB_RETURN_SUCCESS in case of success
        // return DIB_RETURN_SUCCESS
        return I2C_backend_Write(st, msg->address, msg->tx, msg->txlen);
        //return DIB_RETURN_ERROR;
    }
    else {
        // implement here the read function and return DIB_RETURN_SUCCESS in case of success
        // return DIB_RETURN_SUCCESS

        //return DIB_RETURN_ERROR;
        return I2C_backend_WriteRead(st, msg->address, msg->tx, msg->txlen, msg->rx, msg->rxlen);
    }
}

void host_i2c_release(struct dibDataBusHost *i2c_adap)
{
    struct i2c_state *state = i2c_adap->priv;
    DibDbgPrint("-I-  closing I2C\n");

    MemFree(state, sizeof(struct i2c_state));
}

struct dibDataBusHost * host_i2c_interface_attach(void *priv)
{
    struct i2c_state *state = MemAlloc(sizeof(struct i2c_state));
    struct dibDataBusHost *i2c_adap = MemAlloc(sizeof(struct dibDataBusHost));

    data_bus_host_init(i2c_adap, DATA_BUS_I2C, host_i2c_xfer, state);

    state->priv = priv;

    return i2c_adap;
    goto free_mem;
free_mem:
    MemFree(state,sizeof(struct i2c_state));
    return NULL;
}

struct dibDataBusHost* rkdroid_open_i2c(void)
{
    gBusI2CHost = host_i2c_interface_attach(NULL);
    return gBusI2CHost;
}

void rkdroid_close_i2c(void)
{
    if(gBusI2CHost)
    {
        MemFree(gBusI2CHost, sizeof(struct dibDataBusHost));
        gBusI2CHost = NULL;
    }
}

