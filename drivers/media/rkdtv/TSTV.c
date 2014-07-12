/*
 * TSTV.c 
 *
 * Driver Abstract for all modules
 *	Copyright (C) 2009 wyr 
 *	learn to rk1000.c
 *
 *	This program is free software; you can redistribute  it and/or modify it
 *	under  the terms of  the GNU General  Public License as published by the
 *	Free Software Foundation;  either version 2 of the	License, or (at your
 *	option) any later version.
 *
 *
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/platform_data/spi-rockchip.h>

#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_gpio.h>

#include <linux/rockchip/dvfs.h>
#include <linux/rockchip/common.h>
#include <linux/rockchip/cpu.h>
#include <linux/rockchip/cru.h>
#include <linux/rockchip/grf.h>
#include <linux/rockchip/iomap.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include "demux.h"
#include "dmxdev.h"
#include "dvb_demux.h"
#include "dvb_frontend.h"
#include "dvb_net.h"
#include "dvbdev.h"



#include "spi-rockchip-core.h"



#include "TSTV.h"

#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//dtv sdk board
#ifdef CONFIG_ARCH_RK29
#define TV_POWEN_PIN         RK29_PIN6_PB3
#define TV_RESET_PIN          RK29_PIN6_PC6
#define TV_POWDN_PIN        RK29_PIN6_PD2
#else
#define TV_POWEN_PIN        gpTSTV->io_powerup.gpio
#define TV_RESET_PIN        gpTSTV->io_reset.gpio
#define TV_POWDN_PIN        gpTSTV->io_powerdown.gpio

#define GPIO_LOW   0
#define GPIO_HIGH  1
#endif

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

//#define DRV_NAME "SIANO1130_CONTROL"

#ifdef CONFIG_NMI310_CONTROL
#define NMI310_I2C_ADDR    0xc0
#define TSTV_FE_TYPE      FE_OFDM
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_NMI320_CONTROL
#define NMI320_I2C_ADDR    0x87
#define TSTV_FE_TYPE      FE_ISDB_ONESEG
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_AU8524_CONTROL
#define AU8524_I2C_ADDR    0x8e
#define TSTV_FE_TYPE      FE_ATSC//FE_OFDM//FE_ATSC
#define TSTV_SYS_TYPE    SYS_ATSC
#endif

#ifdef CONFIG_SIANO1130_CONTROL
#define SIANO1130_I2C_ADDR 0xd0
#define TSTV_FE_TYPE      FE_ISDB_ONESEG
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_DIBCOM8070_CONTROL
#define DIBCOM8076_I2C_FIRST_ADDR  0x12
#define DIBCOM8076_I2C_SECOND_ADDR 0x90
#define DIBCOM8076_I2C_THIRD_ADDR 0x84 //0xc0, dibcom8076 tuner real address 0xc0
#define TSTV_FE_TYPE      FE_ISDB_ONESEG
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_DIBCOM7090_CONTROL
#define TSTV_FE_TYPE      FE_OFDM
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_DIBCOM8096_CONTROL
//#define DIBCOM8096_I2C_FIRST_ADDR  0x12
//#define DIBCOM8096_I2C_SECOND_ADDR 0x90
//#define DIBCOM8096_I2C_THIRD_ADDR 0x84 //0xc0, dibcom8096 tuner real address 0xc0
#define TSTV_FE_TYPE      FE_ISDB_FULLSEG
#define TSTV_SYS_TYPE     SYS_ISDBT
#endif

#ifdef CONFIG_MTV818_CONTROL
#define MTV818_I2C_ADDR 0x86
#define TSTV_FE_TYPE      FE_ISDB_ONESEG
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_GX1131_CONTROL
#define GX1131_I2C_ADDR 0xD0
#define TSTV_FE_TYPE    FE_QPSK
#define TSTV_SYS_TYPE     SYS_DVBS
#endif

#ifdef CONFIG_GX1001_CONTROL
#define GX1001_I2C_ADDR 0x18
#define TSTV_FE_TYPE    FE_OFDM
#define TSTV_SYS_TYPE     SYS_DVBC
#endif

#ifdef CONFIG_ITE9133_CONTROL
#define ITE9133_I2C_ADDR (0x38 << 0)
#define TSTV_FE_TYPE      FE_OFDM
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_DIBCOM1009XH_DVB_CONTROL
#define DIBCOM1009XH_ADDR 0x80
#define TSTV_FE_TYPE      FE_OFDM
#define TSTV_SYS_TYPE     SYS_DVBT
#endif

#ifdef CONFIG_DIBCOM1009XH_ISDB_ONESEG_CONTROL 
#define DIBCOM1009XH_ADDR 0x80
#define TSTV_FE_TYPE      FE_ISDB_ONESEG
#define TSTV_SYS_TYPE     SYS_ISDBT
#endif

#ifdef CONFIG_DIBCOM1009XH_ISDB_FULLSEG_CONTROL
#define DIBCOM1009XH_ADDR 0x80
#define TSTV_FE_TYPE      FE_ISDB_FULLSEG
#define TSTV_SYS_TYPE     SYS_ISDBT
#endif


DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static const unsigned short normal_i2c[] = {
#if defined(NMI310_I2C_ADDR)
        NMI310_I2C_ADDR >> 1,
#elif defined(NMI320_I2C_ADDR)
        NMI320_I2C_ADDR >> 1,
#elif defined(SIANO1130_I2C_ADDR)
	SIANO1130_I2C_ADDR >> 1,			/* tstv module control address */
#elif (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))
        DIBCOM8076_I2C_FIRST_ADDR >> 1,
        DIBCOM8076_I2C_SECOND_ADDR >> 1,
        DIBCOM8076_I2C_THIRD_ADDR >> 1,
#elif (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
        DIBCOM8096_I2C_FIRST_ADDR >> 1,
        DIBCOM8096_I2C_SECOND_ADDR >> 1,
        //DIBCOM8096_I2C_THIRD_ADDR >> 1,
#elif defined(CONFIG_AU8524_CONTROL)
        AU8524_I2C_ADDR >> 1,              
#elif defined(MTV818_I2C_ADDR)
        MTV818_I2C_ADDR >> 1,
#elif defined(GX1131_I2C_ADDR)
        GX1131_I2C_ADDR >> 1, 
#elif defined(GX1001_I2C_ADDR)
        GX1001_I2C_ADDR >> 1,              
 #elif defined(ITE9133_I2C_ADDR)
        ITE9133_I2C_ADDR >> 1,         
#elif defined(DIBCOM1009XH_ADDR)
		DIBCOM1009XH_ADDR >> 1,

#else
		0x88 >> 1,
#endif
	I2C_CLIENT_END
};


#define HSADC_DMA_TRAN_LENGTH  (2048) 
#define HSADC_INPUT_BUFFER_NUM  1024

#define HSADC_BUFFER_HEAD_LENGTH  (3072) 

#define HSADC_SHARE_MEM_MALLOC_SIZE   (HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM+HSADC_BUFFER_HEAD_LENGTH)

#define DVB_TS_FILTER_POINTER_INTERVAL  2048//2048

#define NHWFILTERS		32

typedef struct DVB_HSADC_Data_Trans_Info 
{
	volatile unsigned int ceva_ready_flag;      /* 0: not ready,  1: ready.            */
	volatile unsigned int ceva_stop_flag;      /* 0: not stop,  1: stop.            */
	volatile unsigned int data_base_addr;       /* sdram start address for hsadc's data */
	volatile unsigned int cur_data_addr;        
	volatile unsigned int data_block_size;      /* data size of per ceva's DMA transfer */
	volatile unsigned int data_total_block_num; /* total  block number of DMA transfer */	  
	volatile unsigned int data_block_index;     /* index of cur ceva's DMA transfer, 0, 1, 2...... */


	volatile unsigned int reserved[256-7];             /* reserved for other use */	  
}DVB_HSADC_Data_Trans_Info_t;

/***********************************************************************************************************/

struct ioctrl {
	int gpio;
	int active;
};

struct TSTV {
	struct mutex lock;
	char name[32];
	
	/* dvb */
	struct dmx_frontend hw_frontend;
	struct dmx_frontend mem_frontend;
	struct dmxdev dmxdev;
	struct dvb_adapter dvb_adapter;
	struct dvb_demux demux;
	struct dvb_frontend *fe;
	struct dvb_net dvbnet;
	unsigned int full_ts_users;
	unsigned int users;

	/* i2c */
	struct mutex i2c_mutex;
	struct i2c_adapter i2c_adap;
	unsigned int i2cbug;
	
	/* timer */
	#define TSTV_TIMER_DELAY  (HZ/10)
	struct timer_list timer;

    /* workque*/
	struct work_struct 	timer_work;
	struct workqueue_struct *pWorkqueue;
	struct mutex workque_mutex;		
	
	struct ioctrl			io_powerup;
	struct ioctrl			io_powerdown;
	struct ioctrl			io_reset;		
			
	struct device		  *dev;
	struct i2c_client	*i2c_clnt;	
	struct spi_device	*spi_dev;		


	int piu_mem;
	int scu_mem;
	DVB_HSADC_Data_Trans_Info_t *hsadc_info;
	int dma_kmalloc_buf_head; //malloc buffer begin
	int dma_start_addr;//data begin

	unsigned int read_total_size;
	unsigned int ts_read_addr; 

	u8 dummy[4096];
};

struct tstv_config
{
	int dummy[4];
};

struct tstv_state {
	struct dvb_frontend demod;
	struct tstv_config *cfg;

	u8 i2c_addr;
	struct i2c_adapter *i2c_adap;

	u32 timf;

	fe_bandwidth_t current_bandwidth;

	u16 dev_id;
	
	u8 sfn_workaround_active :1;
	
	struct TSTV *pTSTV;
};

struct i2c_client *gTSTVModuleClient = NULL;
struct spi_device *gTSTVModuleSpi = NULL;

struct TSTV *gpTSTV = NULL;
EXPORT_SYMBOL(gpTSTV);
int hsadc_dma_kmalloc_buf_head; //dma 分配起始地址，因为开头有一段地址是用于通信
EXPORT_SYMBOL(hsadc_dma_kmalloc_buf_head);

extern struct TSTV_Module_t gTSTVModule;

int gHsadc_reset_flag = 0;

extern int rk29_hsadc_start_transmit(void);
extern int rk29_hsadc_stop_transmit(void);
extern int rk29_hsadc_get_cur_transmit_addr(void);

static void tstv_timer_handler( unsigned long data);
static void tstv_timer_shedule_work(struct work_struct *work);
int tstv_control_probe(struct i2c_client *client, const struct i2c_device_id *id);

#if (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))
static int dibcom_i2c_detach_client(void);
#endif // dibcomm8076

#if (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
static int dibcom_i2c_detach_client(void);
#endif // dibcomm8096



/**************************************************************************
* 函数名称:  DVB_TS_ReadData_Ptr_Estimat
* 函数描述:读写指针比较，确定读取长度
* 入口参数:   *pwrite:   写指针
              *pread:    读指针
              *buf:      读写指针指向的BUF
              unsigned long bufsize: BUF长度
* 出口参数:  无
* 返回值:   size:                                         读取长度(必须大于188个指针地址)
* 注释:    宏PID_Filter_Offset为读写指针保留间距。
**************************************************************************/
unsigned int DVB_TS_ReadData_Ptr_Estimat(u8 *pwrite, u8 *pread, u8 *buf, u32 bufsize)
{
	u32 size;
	
	if(pwrite < pread)
	{
		size = (pread - pwrite);
	
        if((bufsize - size) > (DVB_TS_FILTER_POINTER_INTERVAL + 0))
		{
			size = (bufsize - size) - DVB_TS_FILTER_POINTER_INTERVAL;
        }
        else
		{
			size = 0;
        }
	}
	else //(pwrite >= pread)
    {

		size = ( pwrite - pread);
	
		if((size) > (DVB_TS_FILTER_POINTER_INTERVAL + 0) )
		{
			size = size - DVB_TS_FILTER_POINTER_INTERVAL;
		}
		else
		{
			size =0;
		}	
	}
	return size;
}


/*
power up && reset 
 */
void tstv_power_up_and_reset(void)
{
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

//all pins to low
#ifdef TV_POWEN_PIN		
	gpio_direction_output(TV_POWEN_PIN, GPIO_LOW);
	gpio_set_value(TV_POWEN_PIN, GPIO_LOW);
#endif	

#ifdef TV_POWDN_PIN	
	gpio_direction_output(TV_POWDN_PIN, GPIO_LOW);
	gpio_set_value(TV_POWDN_PIN, GPIO_LOW);
#endif

#ifdef TV_RESET_PIN
	gpio_direction_output(TV_RESET_PIN, GPIO_LOW);
	gpio_set_value(TV_RESET_PIN, GPIO_LOW);
#endif 
//-------------------------------------------------------
//power control sequence
#ifdef TV_POWEN_PIN		
	//tstv power up
	gpio_direction_output(TV_POWEN_PIN, GPIO_HIGH);
	gpio_set_value(TV_POWEN_PIN, GPIO_HIGH);
#endif	

#ifdef TV_POWDN_PIN	
	//tstv power down
       msleep(10);	//T0:power ip time	
	gpio_direction_output(TV_POWDN_PIN, GPIO_HIGH);	   
	gpio_set_value(TV_POWDN_PIN, GPIO_HIGH);	

#endif

#ifdef TV_RESET_PIN
	//tstv reset
       msleep(50);	//T1:reset time	
	gpio_direction_output(TV_RESET_PIN, GPIO_HIGH);	   
	gpio_set_value(TV_RESET_PIN, GPIO_HIGH);	
	
       msleep(10); //T2:for device start up time
#endif 
}

/*
power down
 */
void tstv_power_down(void)
{
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

#ifdef TV_POWEN_PIN		
	//tstv power up
	gpio_direction_output(TV_POWEN_PIN, GPIO_LOW);
	gpio_set_value(TV_POWEN_PIN, GPIO_LOW);
#endif

#ifdef TV_POWDN_PIN		
	//tstv power down
	gpio_direction_output(TV_POWDN_PIN, GPIO_LOW);
	gpio_set_value(TV_POWDN_PIN, GPIO_LOW);
#endif

#ifdef TV_RESET_PIN		
	//tstv reset
	gpio_direction_output(TV_RESET_PIN, GPIO_LOW);
	gpio_set_value(TV_RESET_PIN, GPIO_LOW);
#endif
}

/*********************************************************************************/
extern int  rk29_pid_filter_ctrl(uint8_t onoff);
extern int rk29_pid_filter_set(uint8_t id, uint16_t pid, uint8_t onoff);
/****************************frontend************************/
static int tstv_start_feed(struct dvb_demux_feed *f)
{
	struct dvb_demux*dvbdmx = f->demux;
	struct TSTV *pTSTV = dvbdmx->priv;
	unsigned char switch_feed =1;
	DVB_HSADC_Data_Trans_Info_t *info;
	
	info = pTSTV->hsadc_info;	

	/* enable PID filtering */
	if (pTSTV->users++ == 0)
	{
		gTSTVModule.pid_filter_ctrl(switch_feed);
		rk29_pid_filter_ctrl(switch_feed);
		
	}

	if ((f->pid <= 0x2000) && (f->index < NHWFILTERS))
	{
		DBG("***rkdtv***\t%s[%d] add pid filter :index = %d,  pid =%d\n",__FUNCTION__,__LINE__,f->index,f->pid);
		gTSTVModule.pid_filter(f->index, f->pid, 1);
		rk29_pid_filter_set(f->index, f->pid, 1);
	}
	else if (pTSTV->full_ts_users++ == 0)
	{
		gTSTVModule.pid_filter_ctrl(0);
		rk29_pid_filter_ctrl(0);
	}

	return 0;
}


static int tstv_stop_feed(struct dvb_demux_feed *f)
{
	struct dvb_demux*dvbdmx = f->demux;
	struct TSTV *pTSTV = dvbdmx->priv;

	/* disable PID filtering */
	if (--pTSTV->users == 0) {
		gTSTVModule.pid_filter_ctrl(0);
		rk29_pid_filter_ctrl(0);
	}

	if ((f->pid <= 0x2000) && (f->index < NHWFILTERS))
	{
		DBG("***rkdtv***\t%s[%d] del pid filter :index = %d,  pid =%d\n",__FUNCTION__,__LINE__,f->index,f->pid);
		gTSTVModule.pid_filter(f->index, f->pid, 0);
		rk29_pid_filter_set(f->index, f->pid, 0);		
	}
	else if (--pTSTV->full_ts_users == 0)
	{
		gTSTVModule.pid_filter_ctrl(0);
		rk29_pid_filter_ctrl(0);
	}

	return 0;
}

static void tstv_release(struct dvb_frontend *demod)
{
	struct dib7000m_state *st = demod->demodulator_priv;
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	kfree(st);
}

static int tstv_wakeup(struct dvb_frontend *demod)
{
	struct tstv_state *st = demod->demodulator_priv;
	struct TSTV *pTSTV = st->pTSTV;
	static char test_abc[1024];
	int i;
	
	for(i = 0; i< 1024; i++)
	{
        test_abc[i] = i;
	}
	
	
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	spi_write(gTSTVModuleSpi, test_abc, 1024);

	#ifdef CONFIG_ARCH_RK30
	rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPS_CLK);
	rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_TS_SYNC);	
	rk30_mux_api_set(GPIO2B4_LCDC1DATA12_SMCADDR16_HSADCDATA9_NAME, GPIO2B_HSADC_DATA9);	
	rk30_mux_api_set(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME, GPIO2B_HSADC_DATA8);	
	rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_HSADC_DATA7);	
	rk30_mux_api_set(GPIO2C1_LCDC1DATA17_SMCBLSN0_HSADCDATA6_NAME, GPIO2C_HSADC_DATA6);	
	rk30_mux_api_set(GPIO2C2_LCDC1DATA18_SMCBLSN1_HSADCDATA5_NAME, GPIO2C_HSADC_DATA5);	
	rk30_mux_api_set(GPIO2C7_LCDC1DATA23_SPI1CSN1_HSADCDATA4_NAME, GPIO2C_HSADC_DATA4);	
	rk30_mux_api_set(GPIO2C6_LCDC1DATA22_SPI1RXD_HSADCDATA3_NAME, GPIO2C_HSADC_DATA3);	
	rk30_mux_api_set(GPIO2C5_LCDC1DATA21_SPI1TXD_HSADCDATA2_NAME, GPIO2C_HSADC_DATA2);	
	rk30_mux_api_set(GPIO2C4_LCDC1DATA20_SPI1CSN0_HSADCDATA1_NAME, GPIO2C_HSADC_DATA1);	
	rk30_mux_api_set(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME, GPIO2C_HSADC_DATA0);	
	#endif	
	
	#ifdef CONFIG_ARCH_RK3188
	// GRF_SOC_CON0 Reg
    #define  GRF_SOC_CON0_Reg             0x00a0
    #define write_grf_reg(addr, val)  __raw_writel(val, addr+RK30_GRF_BASE)
    #define read_grf_reg(addr) __raw_readl(addr+RK30_GRF_BASE)	
	iomux_set(HSADC_D8);
	iomux_set(GPIO3_B5);
	write_grf_reg(GRF_SOC_CON0_Reg, read_grf_reg(GRF_SOC_CON0_Reg)|(1<<14) | (1<<15)|(1<<30) | (1<<31));  //set hsadc iomux and hsadc clk in
	
	#endif		

	tstv_power_up_and_reset();

	if(gTSTVModule.init()==0)
	{
		printk("TSTV init faild! \t%s[%d]\n",__FUNCTION__,__LINE__);
		return -1;
	}
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);	
	
    if(gTSTVModule.frontend_init() == 0)
    {
		printk("TSTV init faild! \t%s[%d]\n",__FUNCTION__,__LINE__);
	    return -1;
    }
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	{
	    mutex_init(&pTSTV->workque_mutex);
        pTSTV->pWorkqueue= create_workqueue("tstv_workqueue");
	    INIT_WORK(&pTSTV->timer_work, tstv_timer_shedule_work);
		
		/* Register timer */
		setup_timer(&pTSTV->timer, tstv_timer_handler, (unsigned long)pTSTV);
        mod_timer(&pTSTV->timer, jiffies + TSTV_TIMER_DELAY);

		DBG("have start timer\n");
	}	

	rk29_hsadc_start_transmit();
	
	DBG("000rkdtv000\t%s[%d]\n",__FUNCTION__,__LINE__);
	
	return 0;
}

static void tstv_timer_handler(unsigned long data)
{
	struct TSTV *pTSTV =(struct TSTV *)data;
	
	queue_work(pTSTV->pWorkqueue, &pTSTV->timer_work);
    mod_timer(&pTSTV->timer, jiffies + TSTV_TIMER_DELAY);
}

static int tstv_sleep(struct dvb_frontend *demod)
{
	struct tstv_state *st = demod->demodulator_priv;
	struct TSTV *pTSTV = st->pTSTV;

	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	del_timer(&(pTSTV->timer));
	destroy_workqueue(pTSTV->pWorkqueue);	

    gTSTVModule.deinit();
	tstv_power_down();

	rk29_hsadc_stop_transmit();
	
	return 0;
}

#if 0
static int dvbfe_dvbt_bandwidth_to_num[5][2] =
{
	{ BANDWIDTH_8_MHZ, 8000},
	{ BANDWIDTH_7_MHZ, 7000},
	{ BANDWIDTH_6_MHZ, 6000},
	{ BANDWIDTH_AUTO, 0},
	{ -1, -1 }
};


static int dvblookupval(int val, int table[][2])
{
	int i =0;

	while(table[i][0] != -1) {
	    if (val == table[i][0]) {
		return table[i][1];
	    }
	    i++;
	}
	return -1;
}
#endif

static int tstv_set_frontend(struct dvb_frontend* fe)
{
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

#if ((defined(CONFIG_NMI320_CONTROL)) || defined(CONFIG_NMI310_CONTROL))// || defined(CONFIG_AU8524_CONTROL))
    tstv_power_up_and_reset();
#else

#endif

	if(gTSTVModule.frontend_init() == 0)
	{
	    printk("TSTV init faild when tune!\t%s[%d]\n",__FUNCTION__,__LINE__);
	}

	gHsadc_reset_flag = 1;
#if 0		
	if(TSTV_FE_TYPE == FE_QPSK)
	{	
		    DBG("TSTV tune frequency: %d, symbol_rate: %d, \n", fep->frequency, fep->u.qpsk.symbol_rate);
		    gTSTVModule.tune(fep->frequency,fep->u.qpsk.symbol_rate);
  }
  else
  {
    	  DBG("TSTV tune frequency: %d, bandwidth: %d, \n", fep->frequency, dvblookupval(fep->u.ofdm.bandwidth, dvbfe_dvbt_bandwidth_to_num));
    	  gTSTVModule.tune(fep->frequency, dvblookupval(fep->u.ofdm.bandwidth, dvbfe_dvbt_bandwidth_to_num));  	    
  }
#else
	if(TSTV_FE_TYPE == FE_QPSK)
	{	
		    DBG("TSTV tune frequency: %d, symbol_rate: %d, \n", fe->dtv_property_cache.frequency,  fe->dtv_property_cache.symbol_rate/1000);
		    gTSTVModule.tune(fe->dtv_property_cache.frequency, fe->dtv_property_cache.symbol_rate/1000); 
  }
  else
  {
    	  DBG("TSTV tune frequency: %d, bandwidth: %d, \n", fe->dtv_property_cache.frequency, fe->dtv_property_cache.bandwidth_hz/1000);
    	  gTSTVModule.tune(fe->dtv_property_cache.frequency, fe->dtv_property_cache.bandwidth_hz/1000);   
  }

#endif
        
	
	DBG("000rkdtv000\t%s[%d]\n",__FUNCTION__,__LINE__);

	
	return 0;
}

static int tstv_fe_get_tune_settings(struct dvb_frontend* fe, struct dvb_frontend_tune_settings *tune)
{
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	return 0;
}


static int tstv_get_frontend(struct dvb_frontend* fe)
{
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	return 0;
}


static int tstv_read_status(struct dvb_frontend *fe, fe_status_t *stat)
{
	struct tstv_state *st = fe->demodulator_priv;
	struct TSTV *pTSTV = st->pTSTV;
	DVB_HSADC_Data_Trans_Info_t *info;
	
	//DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);	
	info = pTSTV->hsadc_info;	

	if(gTSTVModule.lockstatus()>0)
	{
		//DBG("TSTV have lock\n");
		*stat = FE_HAS_LOCK;
	}
	else
	{
	    *stat = FE_TIMEDOUT;
	}
	
	return 0;
}


static int tstv_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	
	return 0;
}


static int tstv_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
  char signal_quality = 0, signal_strength = 0;
	
	//DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	gTSTVModule.signal_strenth_quality(&signal_quality, &signal_strength);
	//DBG("***rkdtv***\t%s[%d]: signal_quality = %d, signal_strength = %d \n",__FUNCTION__,__LINE__, signal_quality, signal_strength);
	
	*strength = signal_strength;
	
	*strength = 100;

	return 0;
}


static int tstv_read_snr(struct dvb_frontend* fe, u16 *snr)
{
  //  char signal_quality = 0, signal_strength = 0;	
	
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

	//gTSTVModule.signal_strenth_quality(&signal_quality, &signal_strength);
	*snr = 100;

	return 0;
}

static int tstv_read_unc_blocks(struct dvb_frontend *fe, u32 *unc)
{
	//struct dib7000m_state *state = fe->demodulator_priv;
	
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	return 0;
}


static struct dvb_frontend_ops tstv_ops = {
	.delsys = { TSTV_SYS_TYPE },	
	.info = {
		.name = "TSTV",
		.type = TSTV_FE_TYPE,
		.frequency_min		= 100,//44250000,
		.frequency_max		= 1867250000,
		.frequency_stepsize = 62500,
		.caps = FE_CAN_INVERSION_AUTO |
			FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
			FE_CAN_QPSK | FE_CAN_QAM_16 | FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
			FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO |
			FE_CAN_RECOVER |
			FE_CAN_HIERARCHY_AUTO,
	},

	.release			  = tstv_release,

	.init				  = tstv_wakeup,
	.sleep			  = tstv_sleep,

	.set_frontend		  = tstv_set_frontend,
	.get_tune_settings	  = tstv_fe_get_tune_settings,
	.get_frontend		  = tstv_get_frontend,

	.read_status		  = tstv_read_status,
	.read_ber			  = tstv_read_ber,
	.read_signal_strength = tstv_read_signal_strength,
	.read_snr			  = tstv_read_snr,
	.read_ucblocks		  = tstv_read_unc_blocks,
};

static inline struct dvb_frontend* tstv_attach(void)
{
	struct dvb_frontend *demod;
	struct tstv_state *st;
	
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	
	st = kzalloc(sizeof(struct tstv_state), GFP_KERNEL);
	if (st == NULL)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		return NULL;
	}

	demod                   = &st->demod;
	demod->demodulator_priv = st;

	/* create dvb_frontend */
	memcpy(&st->demod.ops, &tstv_ops, sizeof(struct dvb_frontend_ops));
	
	DBG("000rkdtv000\t%s[%d]\n",__FUNCTION__,__LINE__);
	return demod;
}

static int  frontend_init(struct TSTV *pTSTV)
{
	int ret;
	struct dvb_frontend *demod;
	struct tstv_state *st;
	
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

	pTSTV->fe = tstv_attach();
	if (!pTSTV->fe) {
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		return -ENODEV;
	}

	demod = pTSTV->fe;
	st	= demod->demodulator_priv;
	st->pTSTV = pTSTV;

      gHsadc_reset_flag = 0;	

	pTSTV->fe->ops.tuner_ops.set_params = NULL;

	ret = dvb_register_frontend(&pTSTV->dvb_adapter, pTSTV->fe);
	if (ret < 0) {
		if (pTSTV->fe->ops.release)
			pTSTV->fe->ops.release(pTSTV->fe);
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		return ret;
	}
	
	DBG("000rkdtv000\t%s[%d]\n",__FUNCTION__,__LINE__);
	return 0;
}

int  tstv_control_set_reg(struct i2c_client *client, u8 reg, u8 const buf[], int len)
{
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;
	int ret,count = len;
	
	char *tx_buf = (char *)kmalloc(count, GFP_KERNEL);
	if(!tx_buf)
		return -ENOMEM;

	memcpy(tx_buf, buf, count); 

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.len = count;
	msg.buf = (char *)tx_buf;
	msg.scl_rate = 150*1024;

	ret = i2c_transfer(adap, &msg, 1);
	kfree(tx_buf);

	if(ret < 0)
	{
	    printk("%s[%d]: i2c_transfer error, I2C addr: %d  \n",__FUNCTION__, __LINE__, msg.addr);	
	}

	return 0;	
}
EXPORT_SYMBOL(tstv_control_set_reg);

int  tstv_control_read_regs(struct i2c_client *client, u8 reg, u8 buf[], unsigned len)
{
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;
	int ret, count = len;
	char *rx_buf = (char *)kmalloc(count, GFP_KERNEL);
	if(!rx_buf)
		return -ENOMEM;	

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.len = count;
	msg.buf = (char *)rx_buf;
	msg.scl_rate = 150*1024;

	ret = i2c_transfer(adap,& msg, 1);
	memcpy(buf, rx_buf, count); 	
	kfree(rx_buf);

	if(ret < 0)
	{
	    printk("%s[%d]: i2c_transfer error\n",__FUNCTION__,__LINE__);	
	}	
	
	return 0;	
}
EXPORT_SYMBOL(tstv_control_read_regs);


int tstv_spi_set_reg(struct spi_device *spi, u8 buf[], unsigned len)
{
#if (defined(CONFIG_SPI_MASTER))
	 if (spi_write(spi, buf, len) != 0 )
	 {
	   printk("%s[%d]: spi_transfer error\n",__FUNCTION__,__LINE__);	
	 }
#endif

    return 0;
}
EXPORT_SYMBOL(tstv_spi_set_reg);

int tstv_spi_read_reg(struct spi_device *spi, u8 buf[], unsigned len)
{
#if (defined(CONFIG_SPI_MASTER))
	if (spi_read(spi, buf, len) != 0)
	{
	  printk("%s[%d]: spi_transfer error\n",__FUNCTION__,__LINE__);	
	}
#endif

    return 0;
}
EXPORT_SYMBOL(tstv_spi_read_reg);

#if (defined(CONFIG_SPI_MASTER) && defined(CONFIG_DIBCOM1009XH_CONTROL))
	
#else
static int tstv_control_remove(struct i2c_client *client)
{
	struct TSTV *pTSTV = gpTSTV;
	
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

       tstv_power_down();		

#ifdef  TV_POWEN_PIN
	//tstv power up 
	gpio_free(TV_POWEN_PIN);
#endif

#ifdef  TV_POWDN_PIN
	//tstv power down
	gpio_free(TV_POWDN_PIN);
#endif
	
#ifdef  TV_RESET_PIN
	//tstv reset
	gpio_free(TV_RESET_PIN);
#endif
	
	if(pTSTV)
	{
		DBG("TSTV: unloading driver\n");

		del_timer(&(pTSTV->timer));
		pTSTV->demux.dmx.remove_frontend(&pTSTV->demux.dmx, &pTSTV->mem_frontend);
		pTSTV->demux.dmx.remove_frontend(&pTSTV->demux.dmx, &pTSTV->hw_frontend);
		dvb_dmxdev_release(&pTSTV->dmxdev);
		dvb_dmx_release(&pTSTV->demux);
		if (pTSTV->fe) {
			dvb_unregister_frontend(pTSTV->fe);
			dvb_frontend_detach(pTSTV->fe);
		}
		dvb_unregister_adapter(&pTSTV->dvb_adapter);

		kfree(pTSTV);	
	}
	
#if (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))
        return dibcom_i2c_detach_client();
#elif (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
        return dibcom_i2c_detach_client();
#else
	 i2c_release_client(client);

        return 1;
#endif
}

static const struct i2c_device_id tstv_control_id[] = {
		{"tstv_control", 0},
		{ }
};


static struct i2c_driver tstv_control_driver = {
	.driver 	= {
		.name	= "tstv_control",
	},
	.id_table 	= tstv_control_id,
	.probe		= tstv_control_probe,
	.remove		= tstv_control_remove,
};
#endif

#define  MPEG_TS_PACKET_SIZE  188
#define  MPEG_TS_SYN_BYTE     0x47

/****soft irq to get data from HSADC DMA buffer **********/
/*
  timer handle ,to process TS data 
 */
static void tstv_timer_shedule_work(struct work_struct *work)
{
	struct TSTV *pTSTV = container_of(work, struct TSTV,
						timer_work);
	
	//struct TSTV *pTSTV = gpTSTV;
	DVB_HSADC_Data_Trans_Info_t *info;
	unsigned int len = 0;
	unsigned int left_size =0;
	unsigned int test_read_address =0;
    static int timer_cnt = 0;
    static int timer_cnt1 = 0;	

	mutex_lock(&pTSTV->workque_mutex);	

	info = pTSTV->hsadc_info;

	info->cur_data_addr = rk29_hsadc_get_cur_transmit_addr();

	if(timer_cnt1++ > 30)
	{
	    timer_cnt1 = 0;  
	    //printk("%s[%d] :ts_read_addr = 0x%x cur_data_addr = 0x%x info->data_base_addr = 0x%x\n",__FUNCTION__,__LINE__,pTSTV->ts_read_addr,info->cur_data_addr, info->data_base_addr);
	}
	
    if((info->cur_data_addr > (info->data_base_addr + (HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM))) || (info->cur_data_addr < info->data_base_addr))
    {
        //DBG("***tstv_timer_handler: hsadc addr error ! ***\t%s\n", __FUNCTION__);
        goto end;    
     }
       //reset hsadc data
       if(gHsadc_reset_flag == 1) 
       {
       	gHsadc_reset_flag = 0;
       	memset((void *)info->data_base_addr, 0, HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM);
	info->cur_data_addr = info->data_base_addr;
	pTSTV->ts_read_addr = info->data_base_addr;
	pTSTV->read_total_size =0;
        }   

	len = DVB_TS_ReadData_Ptr_Estimat((u8 *)info->cur_data_addr,(u8 *)pTSTV->ts_read_addr,(u8 *)info->data_base_addr,HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM);
	//DBG("pTSTV =%x LEN = %d ,cur =%x read = %x base =%x\n", gpTSTV,len,info->cur_data_addr,pTSTV->ts_read_addr,info->data_base_addr);

	if(timer_cnt++ > 10*5)
	{
	    timer_cnt = 0;
		DBG("***rkdtv available data len: %d ! ***\t%s\n",len, __FUNCTION__);
		DBG("LEN = %d ,cur =%x read = %x base =%x\n",len,info->cur_data_addr,pTSTV->ts_read_addr,info->data_base_addr);
	}
	
	//if the len is valid
	if (len > 0 && len <= (HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM) )
	{
		if((pTSTV->ts_read_addr + len) < (info->data_base_addr + (HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM)))/* <= */
		{
			//if the data not cross the end and start,just read it as whole peace
			dvb_dmx_swfilter(&pTSTV->demux,(u8 *)(pTSTV->ts_read_addr), len);
		
			//cal the next read address
			pTSTV->read_total_size += len;	
			pTSTV->ts_read_addr +=len;
		}
		else
		{
			//if the data hava cross the end and start,read in two times
			left_size = (pTSTV->ts_read_addr + len)-(info->data_base_addr + (HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM));
			
			//read the last data
			dvb_dmx_swfilter(&pTSTV->demux,(u8 *)(pTSTV->ts_read_addr), len-left_size);
	
			//read the start part data			
                        if(left_size > 0)
                        {
			    dvb_dmx_swfilter(&pTSTV->demux,(u8 *)(info->data_base_addr), left_size);
                        }
	
			//cal the next read address
			pTSTV->read_total_size += len;	
			pTSTV->ts_read_addr = info->data_base_addr + left_size;
		}

		
		//cal the next read address
		test_read_address = info->data_base_addr+pTSTV->read_total_size%(HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM);
	
		if(test_read_address != pTSTV->ts_read_addr)
			printk("cal ts_read_addr wrong, test_read_address 0x%x, pTSTV->ts_read_addr 0x%x\n\n\n", test_read_address, pTSTV->ts_read_addr);
	
		//DBG("read addr = %x\n",pTSTV->ts_read_addr);	
	}

	else if(len > (HSADC_DMA_TRAN_LENGTH*HSADC_INPUT_BUFFER_NUM))//wrong state catch
	{
		printk("%s[%d] wrong state catch: ts_read_addr = %x cur_data_addr = %x\n",__FUNCTION__,__LINE__,pTSTV->ts_read_addr,info->cur_data_addr);
	
		pTSTV->ts_read_addr = info->data_base_addr;
		pTSTV->read_total_size =0;
		printk("%s[%d] wrong state catch: cur_data_addr = %x\n",__FUNCTION__,__LINE__,info->cur_data_addr);
	}

end:
    mutex_unlock(&pTSTV->workque_mutex);	

	return;
}

#if (defined(CONFIG_SPI_MASTER) && defined(CONFIG_DIBCOM1009XH_CONTROL))

#else

#if (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))
struct i2c_client dib_i2c_client0 = {
    .driver = &tstv_control_driver,
    .name = "dib_i2c_addr0",
};
struct i2c_client dib_i2c_client1 = {
    .driver = &tstv_control_driver,
    .name = "dib_i2c_addr1",
};
//*
struct i2c_client dib_i2c_client2 = {
    .driver = &tstv_control_driver,
    .name = "dib_i2c_addr2",
};
//*/
#endif // DIBCOM8076 with two i2c address

#if (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
struct i2c_client dib_i2c_client0 = {
    .driver = &tstv_control_driver,
    .name = "dib_i2c_addr0",
};
struct i2c_client dib_i2c_client1 = {
    .driver = &tstv_control_driver,
    .name = "dib_i2c_addr1",
};
/*
struct i2c_client dib_i2c_client2 = {
    .driver = &tstv_control_driver,
    .name = "dib_i2c_addr2",
};
//*/
#endif // DIBCOM8096 with two i2c address

static int tstv_control_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
#if (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
	struct i2c_client * rc = 0;
#endif	
	struct TSTV *pTSTV = NULL;
	struct dvb_adapter *dvb_adapter;
	struct dvb_demux *dvbdemux;
	struct dmx_demux *dmx;
	int err;

	DBG("TSTV: ***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

#ifdef  TV_POWEN_PIN
	//tstv power up 
	err = gpio_request(TV_POWEN_PIN, "tstv_power_en");
	if (err) {
		gpio_free(TV_POWEN_PIN);
		printk("-------request TV_POWEN_PIN fail--------\n");
		//return -1;
	}	
#endif

#ifdef  TV_POWDN_PIN
	//tstv power down
	err = gpio_request(TV_POWDN_PIN, "tstv_power_down");
	if (err) {
		gpio_free(TV_POWDN_PIN);
		printk("-------request TV_POWDN_PIN fail--------\n");
		//return -1;
	}	
#endif
	
#ifdef  TV_RESET_PIN
	//tstv reset
	err = gpio_request(TV_RESET_PIN, "tstv_power_reset");
	if (err) {
		gpio_free(TV_RESET_PIN);
		printk("-------request TV_RESET_PIN fail--------\n");
		//return -1;
	}	
#endif	

	pTSTV = kzalloc(sizeof(struct TSTV), GFP_KERNEL);
	if (!pTSTV)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);	
		err = ENOMEM;
		goto out;
	}

	DBG("%s[%d]\n",__FUNCTION__,__LINE__);

#if (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))
        /* dibcom i2c address0 */
        dib_i2c_client0.addr = DIBCOM8076_I2C_FIRST_ADDR >> 1;
        
        rc = i2c_use_client(&dib_i2c_client0);
        if(rc == 0)
        {
            DBG("TSTV: attach dibcom8076 first device failed\n");
            goto failout;
        } 

        /* dibcom i2c address1 */
        dib_i2c_client1.addr = DIBCOM8076_I2C_SECOND_ADDR >> 1;
        
        rc = i2c_use_client(&dib_i2c_client1);
        if(rc < 0)
        {
            DBG("TSTV: attach dibcom8076 second device failed\n");
            i2c_release_client(&dib_i2c_client0);
            goto failout;
        } 

        /* dibcom i2c address2 */
//*
        dib_i2c_client2.addr = DIBCOM8076_I2C_THIRD_ADDR >> 1;
        
        rc = i2c_use_client(&dib_i2c_client2);
        if(rc == 0)
        {
            DBG("TSTV: attach dibcom8076 second device failed\n");
            i2c_release_client(&dib_i2c_client0);
            i2c_release_client(&dib_i2c_client1);
            goto failout;
        } 
//*/
#elif (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
        /* dibcom i2c address0 */
        dib_i2c_client0.addr = DIBCOM8096_I2C_FIRST_ADDR >> 1;
        
        rc = i2c_use_client(&dib_i2c_client0);
        if(rc == 0)
        {
            DBG("TSTV: attach dibcom8076 first device failed\n");
            goto failout;
        } 

        /* dibcom i2c address1 */
        dib_i2c_client1.addr = DIBCOM8096_I2C_SECOND_ADDR >> 1;

        rc = i2c_use_client(&dib_i2c_client1);
        if(rc == 0)
        {
            DBG("TSTV: attach dibcom8076 second device failed\n");
            i2c_release_client(&dib_i2c_client0);
            goto failout;
        } 

        /* dibcom i2c address2 */
/*
        dib_i2c_client2.adapter = adapter;
        dib_i2c_client2.addr = DIBCOM8096_I2C_THIRD_ADDR >> 1;
        dib_i2c_client2.mode = DIRECTMODE;
        dib_i2c_client2.Channel = I2C_CH1;
        dib_i2c_client2.speed = gTSTVModule.i2c_speed();
        dib_i2c_client2.addressBit = I2C_7BIT_ADDRESS_8BIT_REG;
        
        rc = i2c_attach_client(&dib_i2c_client2);
        if(rc < 0)
        {
            DBG("TSTV: attach dibcom8076 second device failed\n");
            i2c_detach_client(&dib_i2c_client0);
            i2c_detach_client(&dib_i2c_client1);
            goto failout;
        } 
//*/
#elif (defined(DIBCOM1009XH_ADDR))
	gTSTVModuleClient = client;
	client->addr = DIBCOM1009XH_ADDR >> 1;
#elif (defined(MTV818_I2C_ADDR))

	client->addr = MTV818_I2C_ADDR >> 1;
	gTSTVModuleClient = client;
#elif (defined(GX1131_I2C_ADDR))
	client->addr = GX1131_I2C_ADDR >> 1;
	gTSTVModuleClient = client;
#elif (defined(GX1001_I2C_ADDR))
	client->addr = GX1001_I2C_ADDR >> 1;
	gTSTVModuleClient = client;	
#elif (defined(ITE9133_I2C_ADDR))
	client->addr = ITE9133_I2C_ADDR >> 1;
	gTSTVModuleClient = client;
#elif (defined(NMI310_I2C_ADDR))
	client->addr = NMI310_I2C_ADDR >> 1;
	gTSTVModuleClient = client;
#else
	client->addr = 0xC0;
	gTSTVModuleClient = client;
	
#ifndef DRV_NAME
        tstv_control_driver.driver.name = gTSTVModule.name;
#endif
	client->driver = &tstv_control_driver;
#ifdef DRV_NAME
        strlcpy(client->name, DRV_NAME, I2C_NAME_SIZE);
#else
	strlcpy(client->name, /*DRV_NAME*/gTSTVModule.name, I2C_NAME_SIZE);
#endif

	rc = i2c_use_client(client);
	if (rc == 0)
	{

		
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		
		goto failout;
	}
#endif // #if (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))

	/*HSADC dma buffer,to store ts data*/
	pTSTV->dma_kmalloc_buf_head 	= (int)kzalloc(HSADC_SHARE_MEM_MALLOC_SIZE, GFP_DMA);
	
	if(pTSTV->dma_kmalloc_buf_head == 0)
	{

		
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		
		err = ENODEV;
		
		goto failout;
	}
	
	memset((void *)pTSTV->dma_kmalloc_buf_head, 0, HSADC_SHARE_MEM_MALLOC_SIZE);


	pTSTV->hsadc_info 		= (DVB_HSADC_Data_Trans_Info_t *)((pTSTV->dma_kmalloc_buf_head & 0xFFFFFC00) + 0x400);
	pTSTV->dma_start_addr 		= (int)pTSTV->hsadc_info+1024;
	(pTSTV->hsadc_info)->ceva_ready_flag = 0;
	pTSTV->read_total_size =0;
	
	pTSTV->hsadc_info->data_base_addr = (int)pTSTV->hsadc_info+1024;
       pTSTV->hsadc_info->data_block_size = HSADC_DMA_TRAN_LENGTH;
	pTSTV->hsadc_info->data_total_block_num = HSADC_INPUT_BUFFER_NUM;
	pTSTV->hsadc_info->data_block_index = 0;
	hsadc_dma_kmalloc_buf_head = (int)pTSTV->hsadc_info+1024;	


	DBG("TSTV: dma_kmalloc_buf_head =%x\n",pTSTV->dma_kmalloc_buf_head);
	DBG("TSTV: dma_start_addr =%x\n",pTSTV->dma_start_addr);

	/* dvb */
#ifdef DRV_NAME
        err = dvb_register_adapter(&pTSTV->dvb_adapter, DRV_NAME, THIS_MODULE, NULL, adapter_nr);
#else
	err = dvb_register_adapter(&pTSTV->dvb_adapter, /*DRV_NAME*/gTSTVModule.name, THIS_MODULE, NULL, adapter_nr);
#endif
	if (err < 0)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);	
		goto err_i2c_del_adapter;
	}

	dvb_adapter = &pTSTV->dvb_adapter;

	dvbdemux = &pTSTV->demux;
	dvbdemux->filternum = 256;
	dvbdemux->feednum = 256;
	dvbdemux->start_feed = tstv_start_feed;/*!!!NEED to add*/
	dvbdemux->stop_feed = tstv_stop_feed;/*!!!NEED to add*/
	dvbdemux->dmx.capabilities = (DMX_TS_FILTERING |
		
	DMX_SECTION_FILTERING | DMX_MEMORY_BASED_FILTERING);
	dvbdemux->priv = (void*)pTSTV;

	err = dvb_dmx_init(dvbdemux);
	if (err < 0)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);	
		goto err_dvb_unregister_adapter;
	}

	dmx = &dvbdemux->dmx;
	pTSTV->dmxdev.filternum = 256;
	pTSTV->dmxdev.capabilities = 0;

	pTSTV->hw_frontend.source = DMX_FRONTEND_0;
	pTSTV->mem_frontend.source = DMX_MEMORY_FE;
	pTSTV->dmxdev.filternum = NHWFILTERS;
	pTSTV->dmxdev.demux = dmx;

	err = dvb_dmxdev_init(&pTSTV->dmxdev, dvb_adapter);
	if (err < 0)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		goto err_dvb_dmx_release;
	}


	err = dmx->add_frontend(dmx, &pTSTV->hw_frontend);
	if (err < 0)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		goto err_dvb_dmxdev_release;
	}


	err = dmx->add_frontend(dmx, &pTSTV->mem_frontend);
	if (err < 0)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		goto err_remove_hw_frontend;
	}


	err = dmx->connect_frontend(dmx, &pTSTV->hw_frontend);
	if (err < 0)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		goto err_remove_mem_frontend;
	}


	err = frontend_init(pTSTV);
	

	if (err < 0)
	{
		printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
		goto err_disconnect_frontend;
	}

	gpTSTV = pTSTV;
	
	tstv_power_down();	

	DBG("TSTV: 000rkdtv000\t%s[%d]\n",__FUNCTION__,__LINE__);

	return 0;


err_disconnect_frontend:
	dmx->disconnect_frontend(dmx);
err_remove_mem_frontend:
	dmx->remove_frontend(dmx, &pTSTV->mem_frontend);
err_remove_hw_frontend:
	dmx->remove_frontend(dmx, &pTSTV->hw_frontend);	
err_dvb_dmxdev_release:
	dvb_dmxdev_release(&pTSTV->dmxdev);

err_dvb_dmx_release:
	dvb_dmx_release(dvbdemux);

err_dvb_unregister_adapter:
	dvb_unregister_adapter(dvb_adapter);

err_i2c_del_adapter:
#if (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))
        dibcom_i2c_detach_client();
#elif (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
        dibcom_i2c_detach_client();
#else
	i2c_release_client(client);
#endif
	kfree((void *)pTSTV->dma_kmalloc_buf_head);

failout:
#if (!defined(DIBCOM8076_I2C_FIRST_ADDR) && !defined(DIBCOM8076_I2C_SECOND_ADDR))
	kfree(client);
#elif (!defined(DIBCOM8096_I2C_FIRST_ADDR) && !defined(DIBCOM8096_I2C_SECOND_ADDR))
	kfree(client);
#endif
	kfree(pTSTV);
out:
	return err;
}


#if (defined(DIBCOM8076_I2C_FIRST_ADDR) && defined(DIBCOM8076_I2C_SECOND_ADDR))
static int dibcom_i2c_detach_client(void)
{
    int rc = 0;

    i2c_release_client(&dib_i2c_client0);

   i2c_release_client(&dib_i2c_client1);

   i2c_release_client(&dib_i2c_client2);

    return rc;
}
#endif //dibcom8076

#if (defined(DIBCOM8096_I2C_FIRST_ADDR) && defined(DIBCOM8096_I2C_SECOND_ADDR))
static int dibcom_i2c_detach_client(void)
{
    int rc = 0;

    i2c_release_client(&dib_i2c_client0);

    i2c_release_client(&dib_i2c_client1);

    return rc;
}
#endif //dibcom8096
#endif


#if (defined(CONFIG_SPI_MASTER) && defined(CONFIG_DIBCOM1009XH_CONTROL))

#ifdef CONFIG_OF
static struct dw_spi_chip *rockchip_spi_parse_dt(struct device *dev)
{
	u32 temp;
	struct dw_spi_chip *spi_chip_data; 

	spi_chip_data = devm_kzalloc(dev, sizeof(*spi_chip_data), GFP_KERNEL);
	if (!spi_chip_data) {
		dev_err(dev, "memory allocation for spi_chip_data failed\n");
		return ERR_PTR(-ENOMEM);
	}
	
	if (of_property_read_u32(dev->of_node, "poll_mode", &temp)) {
		dev_warn(dev, "fail to get poll_mode, default set 0\n");
		spi_chip_data->poll_mode = 0;
	} else {
		spi_chip_data->poll_mode = temp;
		DBG("TSTV: ***rkdtv*** spi_chip_data->poll_mode == %d \t%s[%d]\n", temp, __FUNCTION__,__LINE__);
	}

	if (of_property_read_u32(dev->of_node, "type", &temp)) {
		dev_warn(dev, "fail to get type, default set 0\n");
		spi_chip_data->type = 0;
	} else {
		spi_chip_data->type = temp;
		DBG("TSTV: ***rkdtv*** spi_chip_data->type == %d \t%s[%d]\n", temp, __FUNCTION__,__LINE__);
	}

	if (of_property_read_u32(dev->of_node, "enable_dma", &temp)) {
		dev_warn(dev, "fail to get enable_dma, default set 0\n");
		spi_chip_data->enable_dma = 0;
	} else {
		spi_chip_data->enable_dma = temp;
		DBG("TSTV: ***rkdtv*** spi_chip_data->enable_dma == %d \t%s[%d]\n", temp, __FUNCTION__,__LINE__);
	}
	

	return spi_chip_data;
}
#else
static struct spi_board_info *rockchip_spi_parse_dt(struct device *dev)
{
	return dev->platform_data;
}
#endif

static int tstv_spi_probe(struct spi_device *spi)
{
		int rc = 0;
		
		struct TSTV *pTSTV = NULL;
		struct dvb_adapter *dvb_adapter;
		struct dvb_demux *dvbdemux;
		struct dmx_demux *dmx;
		int err;
		struct device_node *dtv_np;
		enum of_gpio_flags flags;
		static struct dw_spi_chip *spi_chip_data;
		
		DBG("TSTV: ***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
		
	#ifdef CONFIG_ARCH_RK3188
	iomux_set(HSADC_D8);
	iomux_set(GPIO3_B5);	
	#endif
	
		pTSTV = kzalloc(sizeof(struct TSTV), GFP_KERNEL);
		if (!pTSTV)
		{
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);	
			rc = -ENOMEM;
			goto out;
		}
	
		DBG("%s[%d]\n",__FUNCTION__,__LINE__);
		
	if (!spi_chip_data && spi->dev.of_node) {
		spi_chip_data = rockchip_spi_parse_dt(&spi->dev);
		if (IS_ERR(spi_chip_data))
			printk("spi_chip_data error!!!!  \t%s[%d]\n",__FUNCTION__,__LINE__);	
		//return -ENOMEM;
	}
	
	spi->bits_per_word = 8;	
	spi->controller_data = spi_chip_data;
	spi->mode = SPI_MODE_3;
	//spi->max_speed_hz  = 12000000; //hz
	
		rc = spi_setup(spi);
		if (rc < 0)
			return err;
			
		gTSTVModuleSpi= spi;
		pTSTV->spi_dev = spi;

#ifdef CONFIG_OF	
	dtv_np = pTSTV->spi_dev->dev.of_node;
	  //power up
	  pTSTV->io_powerup.gpio = of_get_named_gpio_flags(dtv_np,"gpio-powerup", 0, &flags);
	  	printk("gpio_request pTSTV->io_powerup.gpio: === 0x%x!!!!!!!!!!!!!!!!!!!!!!\n",pTSTV->io_powerup.gpio);
		if (gpio_is_valid(pTSTV->io_powerup.gpio)) {
			rc = gpio_request(pTSTV->io_powerup.gpio, "dtv-gpio-powerup");
			if(!rc){
				pTSTV->io_powerup.active = !(flags & OF_GPIO_ACTIVE_LOW);
				gpio_direction_output(pTSTV->io_powerup.gpio, !(pTSTV->io_powerup.active));
			}
			else
				printk("gpio_request dtv-gpio-powerup invalid: %d\n",pTSTV->io_powerup.gpio);
		}
		else {
				printk("dtv-gpio-powerup is invalid: %d\n",pTSTV->io_powerup.gpio);
		}	
		//power down
	  pTSTV->io_powerdown.gpio = of_get_named_gpio_flags(dtv_np,"gpio-powerdown", 0, &flags);
	  printk("gpio_request pTSTV->io_powerdown.gpio: === 0x%x!!!!!!!!!!!!!!!!!!!!!!\n",pTSTV->io_powerdown.gpio);
		if (gpio_is_valid(pTSTV->io_powerdown.gpio)) {
			rc = gpio_request(pTSTV->io_powerdown.gpio, "dtv-gpio-powerdown");
			if(!rc){
				pTSTV->io_powerdown.active = !(flags & OF_GPIO_ACTIVE_LOW);
				gpio_direction_output(pTSTV->io_powerdown.gpio, !(pTSTV->io_powerdown.active));
			}
			else
				printk("gpio_request dtv-gpio-powerdown invalid: %d\n",pTSTV->io_powerdown.gpio);
		}
		else {
				printk("dtv-gpio-powerdown is invalid: %d\n",pTSTV->io_powerdown.gpio);
		}	
		//reset
	  pTSTV->io_reset.gpio = of_get_named_gpio_flags(dtv_np,"gpio-reset", 0, &flags);
	  printk("gpio_request pTSTV->io_reset.gpio: === 0x%x!!!!!!!!!!!!!!!!!!!!!!\n",pTSTV->io_reset.gpio);
		if (gpio_is_valid(pTSTV->io_reset.gpio)) {
			rc = gpio_request(pTSTV->io_reset.gpio, "dtv-gpio-reset");
			if(!rc){
				pTSTV->io_reset.active = !(flags & OF_GPIO_ACTIVE_LOW);
				gpio_direction_output(pTSTV->io_reset.gpio, !(pTSTV->io_reset.active));
			}
			else
				printk("gpio_request dtv-gpio-reset invalid: %d\n",pTSTV->io_reset.gpio);
		}
		else {
				printk("dtv-gpio-reset is invalid: %d\n",pTSTV->io_reset.gpio);
		}									
		
#endif			
  
		pTSTV->dma_kmalloc_buf_head 	= (int)kzalloc(HSADC_SHARE_MEM_MALLOC_SIZE, GFP_DMA);	
		if(pTSTV->dma_kmalloc_buf_head == 0)
		{			
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
			
			rc = -ENODEV;			
			goto failout;
		}
		
		memset((void *)pTSTV->dma_kmalloc_buf_head, 0, HSADC_SHARE_MEM_MALLOC_SIZE);
	
	
		pTSTV->hsadc_info		= (DVB_HSADC_Data_Trans_Info_t *)((pTSTV->dma_kmalloc_buf_head & 0xFFFFFC00) + 0x400);
		pTSTV->dma_start_addr		= (int)pTSTV->hsadc_info+1024;
		(pTSTV->hsadc_info)->ceva_ready_flag = 0;
		pTSTV->read_total_size =0;
		
		pTSTV->hsadc_info->data_base_addr = (int)pTSTV->hsadc_info+1024;
		   pTSTV->hsadc_info->data_block_size = HSADC_DMA_TRAN_LENGTH;
		pTSTV->hsadc_info->data_total_block_num = HSADC_INPUT_BUFFER_NUM;
		pTSTV->hsadc_info->data_block_index = 0;
		hsadc_dma_kmalloc_buf_head = (int)pTSTV->hsadc_info+1024;	
	
	
		DBG("TSTV: dma_kmalloc_buf_head =%x\n",pTSTV->dma_kmalloc_buf_head);
		DBG("TSTV: dma_start_addr =%x\n",pTSTV->dma_start_addr);
	
		/* dvb */
		#ifdef DRV_NAME		
			rc = dvb_register_adapter(&pTSTV->dvb_adapter, DRV_NAME, THIS_MODULE, NULL, adapter_nr);
		#else
			rc = dvb_register_adapter(&pTSTV->dvb_adapter, /*DRV_NAME*/gTSTVModule.name, THIS_MODULE, NULL, adapter_nr);
		#endif
		if (rc < 0)
		{
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);	
			goto err_i2c_del_adapter;
		}
	
	
		dvb_adapter = &pTSTV->dvb_adapter;
	
		dvbdemux = &pTSTV->demux;
		dvbdemux->filternum = 256;
		dvbdemux->feednum = 256;
		dvbdemux->start_feed = tstv_start_feed;/*!!!NEED to add*/
		dvbdemux->stop_feed = tstv_stop_feed;/*!!!NEED to add*/
		dvbdemux->dmx.capabilities = (DMX_TS_FILTERING |
			
		DMX_SECTION_FILTERING | DMX_MEMORY_BASED_FILTERING);
		dvbdemux->priv = (void*)pTSTV;
	
		rc = dvb_dmx_init(dvbdemux);
		if (rc < 0)
		{			
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
			
			goto err_dvb_unregister_adapter;
		}
	
	
		dmx = &dvbdemux->dmx;
		pTSTV->dmxdev.filternum = 256;
		pTSTV->dmxdev.capabilities = 0;
	
		pTSTV->hw_frontend.source = DMX_FRONTEND_0;
		pTSTV->mem_frontend.source = DMX_MEMORY_FE;
		pTSTV->dmxdev.filternum = NHWFILTERS;
		pTSTV->dmxdev.demux = dmx;
	
	
		rc = dvb_dmxdev_init(&pTSTV->dmxdev, dvb_adapter);
		if (rc < 0)
		{			
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
			
			goto err_dvb_dmx_release;
		}
	
	
		rc = dmx->add_frontend(dmx, &pTSTV->hw_frontend);
		if (rc < 0)
		{		
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
			
			goto err_dvb_dmxdev_release;
		}
	
	
		rc = dmx->add_frontend(dmx, &pTSTV->mem_frontend);
		if (rc < 0)
		{			
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
			
			goto err_remove_hw_frontend;
		}
	
	
		rc = dmx->connect_frontend(dmx, &pTSTV->hw_frontend);
		if (rc < 0)
		{
	
			
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
			
			goto err_remove_mem_frontend;
		}
	
	
		rc = frontend_init(pTSTV);
		
	
		if (rc < 0)
		{			
			printk("XXXrkdtvXXX\t%s[%d]\n",__FUNCTION__,__LINE__);
			
			goto err_disconnect_frontend;
		}

		gpTSTV = pTSTV;
		DBG("TSTV: 000rkdtv000\t%s[%d]\n",__FUNCTION__,__LINE__);
		tstv_power_down();
	
		return 0;	
	
	err_disconnect_frontend:
		dmx->disconnect_frontend(dmx);
	err_remove_mem_frontend:
		dmx->remove_frontend(dmx, &pTSTV->mem_frontend);
	err_remove_hw_frontend:
		dmx->remove_frontend(dmx, &pTSTV->hw_frontend); 
	err_dvb_dmxdev_release:
		dvb_dmxdev_release(&pTSTV->dmxdev);
	
	err_dvb_dmx_release:
		dvb_dmx_release(dvbdemux);
	
	err_dvb_unregister_adapter:
		dvb_unregister_adapter(dvb_adapter);
	
	err_i2c_del_adapter:
		
		kfree((void *)pTSTV->dma_kmalloc_buf_head);
	
	failout:
		
		kfree(pTSTV);
	out:
		return rc;

}

static int tstv_spi_remove(struct spi_device *spi)
{
	struct TSTV *pTSTV = gpTSTV;
	
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);

       tstv_power_down();	

#ifdef  TV_POWEN_PIN
	//tstv power up 
	gpio_free(TV_POWEN_PIN);
#endif

#ifdef  TV_POWDN_PIN
	//tstv power down
	gpio_free(TV_POWDN_PIN);
#endif
	
#ifdef  TV_RESET_PIN
	//tstv reset
	gpio_free(TV_RESET_PIN);
#endif		   
	
	if(pTSTV)
	{
		DBG("TSTV: unloading driver\n");

		del_timer(&(pTSTV->timer));
		pTSTV->demux.dmx.remove_frontend(&pTSTV->demux.dmx, &pTSTV->mem_frontend);
		pTSTV->demux.dmx.remove_frontend(&pTSTV->demux.dmx, &pTSTV->hw_frontend);
		dvb_dmxdev_release(&pTSTV->dmxdev);
		dvb_dmx_release(&pTSTV->demux);
		if (pTSTV->fe) {
			dvb_unregister_frontend(pTSTV->fe);
			dvb_frontend_detach(pTSTV->fe);
		}
		dvb_unregister_adapter(&pTSTV->dvb_adapter);

		kfree(pTSTV);	
	}
	return 0;
}
#endif

#ifdef CONFIG_OF
static const struct of_device_id rockchip_spi_test_dt_match[] = {
	{ .compatible = "rockchip,dtv_spi_ctrl", },
	{},
};
MODULE_DEVICE_TABLE(of, rockchip_spi_test_dt_match);

#endif /* CONFIG_OF */

static struct spi_driver tstv_spi_driver = {
	.probe = tstv_spi_probe,
	.remove = tstv_spi_remove,
	.driver = {
	    .name = "tstv-ctrl",
	    .bus	= &spi_bus_type,
	    .owner	= THIS_MODULE,
	    .of_match_table = of_match_ptr(rockchip_spi_test_dt_match),
	},
};

static int __init tstv_init(void)
{	
 
	int tmp;
	#if (defined(CONFIG_SPI_MASTER) && defined(CONFIG_DIBCOM1009XH_CONTROL))
		DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	tmp = spi_register_driver(&tstv_spi_driver);
	if (tmp != 0)
	{
		DBG("TSTV Unable to register SPI driver: %d\n", tmp);
	}
	#else
	#ifndef DRV_NAME
        tstv_control_driver.driver.name = gTSTVModule.name;
	#endif
		DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	tmp=i2c_add_driver(&tstv_control_driver);
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	#endif
	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);	
	return 0;
	
}


static void __exit tstv_exit(void)
{

	DBG("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	#if (defined(CONFIG_SPI_MASTER) && defined(CONFIG_DIBCOM1009XH_CONTROL))
		spi_unregister_driver(&tstv_spi_driver);
	#else
	#ifndef DRV_NAME
        tstv_control_driver.driver.name = gTSTVModule.name;
	#endif
	i2c_del_driver(&tstv_control_driver);
	#endif
}


module_init(tstv_init);
module_exit(tstv_exit);
/* Module information */
MODULE_DESCRIPTION("ROCKCHIP DVB moudle");
MODULE_LICENSE("GPL");
