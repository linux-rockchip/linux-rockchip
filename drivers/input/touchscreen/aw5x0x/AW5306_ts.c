/**************************************************************************
*  AW5x0x_ts.c
* 
*  AW5x0x rockchip sample code version 1.0
* 
*  Create Date : 2013/11/27
* 
*  Modify Date : 
*
*  Create by   : wuhaijun
* 
**************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
    #include <linux/pm.h>
    #include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/async.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <linux/irq.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/proc_fs.h>

#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>

#include <asm/gpio.h>

#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#include <linux/input/mt.h>
#include <mach/iomux.h>
#include <asm/io.h>

#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <linux/input/mt.h>
#include <mach/iomux.h>
#include <mach/board.h>


//*************************TouchScreen Work Part*****************************
#define AW5306_I2C_NAME "aw5306_ts"

//define resolution of the touchscreen
#define TOUCH_MAX_HEIGHT 	800			
#define TOUCH_MAX_WIDTH		480
//#define STOP_IRQ_TYPE                     // if define then   no stop irq in irq_handle   kuuga add 1202S
//#define REFRESH 0     //0~0x64   Scan rate = 10000/(100+REFRESH)//define resolution of the LCD

//#define INTMODE

#define CHARGE_DETECT

#define AUTO_RUDUCEFRAME
#define AW5X0X_I2C_SPEED (200*1000)
#define AW5x0x_I2C_NAME "aw5306_ts"

struct i2c_client * this_client=NULL;


#include "AW5x0x_Drv.h"
#include "AW5x0x_userpara.h"

#ifdef CONFIG_HAS_EARLYSUSPEND
static void AW5x0x_ts_early_suspend(struct early_suspend *h);
static void AW5x0x_ts_late_resume(struct early_suspend *h);
#endif

extern char AW5x0x_CLB();
extern void AW5x0x_CLB_GetCfg();
extern STRUCTCALI       AW_Cali;
extern AW5x0x_UCF   AWTPCfg;
extern STRUCTBASE		AW_Base;
extern short	Diff[NUM_TX][NUM_RX];
extern short	adbDiff[NUM_TX][NUM_RX];
extern short	AWDeltaData[32];
extern unsigned char TestErrorCode;

char	AW_CALI_FILENAME[50] = {0,};
char	AW_UCF_FILENAME[50] = {0,};


static unsigned char suspend_flag=0; //0: sleep out; 1: sleep in
static short tp_idlecnt = 0;
static char tp_SlowMode = 1;
static char AWTP_cali_st = 0;
static char AWTP_test_st = 0;

struct ts_event {
	int	x[5];
	int	y[5];
	int	pressure;
	int touch_ID[5];
	int touch_point;
	int pre_point;
};


struct AW5x0x_ts_data {
	struct input_dev	*input_dev;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct work_struct 	Charge_Detectwork;
#ifdef SMART_READ_DATA
	struct work_struct 	read_data_work;
#endif
	struct workqueue_struct *ts_workqueue;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
	struct timer_list touch_timer;
	struct timer_list charge_detect;
	int irq;
  char phys[32];
  char bad_data;
  int use_irq;
	int	reset_gpio;
	int irq_gpio;
	int charge_status;
	int (*get_system_charge_status)(void); //get system charge status
};


static void ctp_enable_irq(void)
{
	struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(this_client);
	
	enable_irq(AW5x0x_ts->irq);
	return;
	
}

/**
 * ctp_disable_irq - 
 *
 */
static void ctp_disable_irq(void)
{
	struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(this_client);
	
	disable_irq_nosync(AW5x0x_ts->irq);
	return;
}


int AW_nvram_read(char *filename, char *buf, ssize_t len, int offset)
{	
    struct file *fd;
    int retLen = -1;
    
    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    fd = filp_open(filename, O_RDONLY, 0);
    
    if(IS_ERR(fd)) {
        printk("[AW5x0x][nvram_read] : failed to open!!\n");
        return -1;
    }
    do{
        if ((fd->f_op == NULL) || (fd->f_op->read == NULL))
    		{
            printk("[AW5x0x][nvram_read] : file can not be read!!\n");
            break;
    		} 
    		
        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        		    if(fd->f_op->llseek(fd, offset, 0) != offset) {
						      printk("[AW5x0x][nvram_read] : failed to seek!!\n");
					         break;
        		    }
        	  } else {
        		    fd->f_pos = offset;
        	  }
        }    		
        
    		retLen = fd->f_op->read(fd,buf,len,&fd->f_pos);			
    		
    }while(false);
    
    filp_close(fd, NULL);
    
    set_fs(old_fs);
    
    return retLen;
}

int AW_nvram_write(char *filename, char *buf, ssize_t len, int offset)
{	
    struct file *fd;
    int retLen = -1;
        
    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    fd = filp_open(filename, O_WRONLY|O_CREAT, 0666);
    
    if(IS_ERR(fd)) {
        printk("[AW5x0x][nvram_write] : failed to open!!\n");
        return -1;
    }
    do{
        if ((fd->f_op == NULL) || (fd->f_op->write == NULL))
    		{
            printk("[AW5x0x][nvram_write] : file can not be write!!\n");
            break;
    		} /* End of if */
    		
        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        	    if(fd->f_op->llseek(fd, offset, 0) != offset) {
				            printk("[AW5x0x][nvram_write] : failed to seek!!\n");
                    break;
                }
            } else {
                fd->f_pos = offset;
            }
        }       		
        
        retLen = fd->f_op->write(fd,buf,len,&fd->f_pos);			
    		
    }while(false);
    
    filp_close(fd, NULL);
    
    set_fs(old_fs);
    
    return retLen;
}

int AW_I2C_WriteByte(u8 addr, u8 para)
{
	int ret;
	u8 buf[3];


	buf[0] = addr;
	buf[1] = para;

	struct i2c_msg msg[] = {
		{
			.scl_rate = AW5X0X_I2C_SPEED,
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		},
	};

	ret = i2c_transfer(this_client->adapter, msg, 1);

	return ret;
}

unsigned char AW_I2C_ReadByte(u8 addr)
{
	int ret;
	u8 buf[2] = {0};

	buf[0] = addr;
	struct i2c_msg msgs[] = {
		{
			.scl_rate = AW5X0X_I2C_SPEED,
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.scl_rate = AW5X0X_I2C_SPEED,
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);

	return buf[0];
  
}

unsigned char AW_I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len)
{
	int ret,i;
	u8 rdbuf[512] = {0};

	rdbuf[0] = addr;
	struct i2c_msg msgs[] = {
		{
			.scl_rate = AW5X0X_I2C_SPEED,
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rdbuf,
		},
		{
			.scl_rate = AW5X0X_I2C_SPEED,
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= len,
			.buf	= rdbuf,
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	for(i = 0; i < len; i++)
	{
		buf[i] = rdbuf[i];
	}

    return ret;
}

unsigned char AW_I2C_WriteXByte( unsigned char *buf, unsigned char addr, unsigned short len)
{
	int ret,i;
	u8 wdbuf[512] = {0};

	wdbuf[0] = addr;
	for(i = 0; i < len; i++)
	{
		wdbuf[i+1] = buf[i];
	}
	struct i2c_msg msgs[] = {
		{
			.scl_rate = AW5X0X_I2C_SPEED,
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= len+1,
			.buf	= wdbuf,
		}
	};

	ret = i2c_transfer(this_client->adapter, msgs, 1);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

    return ret;
}

void AW_Sleep(unsigned int msec)
{
	msleep(msec);
}

static ssize_t AW5x0x_get_Cali(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_set_Cali(struct device* cd,struct device_attribute *attr, char *buf, size_t count);
static ssize_t AW5x0x_get_reg(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_write_reg(struct device* cd,struct device_attribute *attr, char *buf, size_t count);
static ssize_t AW5x0x_get_Base(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_get_Diff(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_get_adbBase(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_get_adbDiff(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_get_FreqScan(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_Set_FreqScan(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);
static ssize_t AW5x0x_GetUcf(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_GetCaliSt(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_GetTestSt(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_GetTest(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5x0x_SetTest(struct device* cd,struct device_attribute *attr, char *buf, size_t count);


static DEVICE_ATTR(cali,  S_IRUGO | S_IWUGO, AW5x0x_get_Cali,    AW5x0x_set_Cali);
static DEVICE_ATTR(readreg,  S_IRUGO | S_IWUGO, AW5x0x_get_reg,    AW5x0x_write_reg);
static DEVICE_ATTR(base,  S_IRUGO | S_IWUSR, AW5x0x_get_Base,    NULL);
static DEVICE_ATTR(diff, S_IRUGO | S_IWUSR, AW5x0x_get_Diff,    NULL);
static DEVICE_ATTR(adbbase,  S_IRUGO | S_IWUSR, AW5x0x_get_adbBase,    NULL);
static DEVICE_ATTR(adbdiff, S_IRUGO | S_IWUSR, AW5x0x_get_adbDiff,    NULL);
static DEVICE_ATTR(freqscan, S_IRUGO | S_IWUGO, AW5x0x_get_FreqScan,    AW5x0x_Set_FreqScan);
static DEVICE_ATTR(getucf, S_IRUGO | S_IWUSR, AW5x0x_GetUcf,    NULL);
static DEVICE_ATTR(cali_st, 	S_IRUGO | S_IWUGO, AW5x0x_GetCaliSt,    NULL);
static DEVICE_ATTR(test_st, 	S_IWUGO | S_IRUGO, AW5x0x_GetTestSt, NULL);
static DEVICE_ATTR(test, 	S_IWUGO | S_IRUGO, AW5x0x_GetTest,	AW5x0x_SetTest);


static ssize_t AW5x0x_get_Cali(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len,"AWINIC RELEASE CODE VER = %d\n", Release_Ver);
	
	len += snprintf(buf+len, PAGE_SIZE-len,"*****AW5x0x Calibrate data*****\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"TXOFFSET:");
	
	for(i=0;i<11;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXOFFSET[i]);
	}
	
	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXOFFSET:");

	for(i=0;i<6;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXOFFSET[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXCAC:");

	for(i=0;i<21;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXCAC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXCAC:");

	for(i=0;i<12;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXCAC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXGAIN:");

	for(i=0;i<21;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXGAIN[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXCC:");

	for(i=0;i<13;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXCC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXCC:");

	for(i=0;i<7;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXCC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");

	for(i=0;i<AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d ", AW_Cali.SOFTOFFSET[i][j]);
		}
		len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	}
	return len;
	
}

static ssize_t AW5x0x_set_Cali(struct device* cd,struct device_attribute *attr, char *buf, size_t count)
{
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);
	
	unsigned long on_off = simple_strtoul(buf, NULL, 10);

	AWTP_cali_st = 0;

	if(on_off == 1)
	{
	#ifdef INTMODE
		ctp_disable_irq();
		AW5x0x_Sleep();
		suspend_flag = 1;
		AW_Sleep(50);

		TP_Force_Calibration();

		AW5x0x_TP_Reinit();
		ctp_enable_irq();
		suspend_flag = 0;
		
	#else
		suspend_flag = 1;
		AW_Sleep(50);
		
		TP_Force_Calibration();
		
		AW5x0x_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;

		mod_timer(&data->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
	#endif
	}
	
	AWTP_cali_st = 1;

	return count;
}


static ssize_t AW5x0x_get_adbBase(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "base: \n");
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]);
		}
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}
	
	return len;
}

static ssize_t AW5x0x_get_Base(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	*(buf+len) = AWTPCfg.TX_LOCAL;
	len++;
	*(buf+len) = AWTPCfg.RX_LOCAL;
	len++;
	
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			*(buf+len) = (char)(((AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]) & 0xFF00)>>8);
			len++;
			*(buf+len) = (char)((AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]) & 0x00FF);
			len++;
		}
	}
	return len;

}

static ssize_t AW5x0x_get_adbDiff(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "Diff: \n");
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",adbDiff[i][j]);
		}
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}
	
	return len;
}

static ssize_t AW5x0x_get_Diff(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	*(buf+len) = AWTPCfg.TX_LOCAL;
	len++;
	*(buf+len) = AWTPCfg.RX_LOCAL;
	len++;
	
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			*(buf+len) = (char)((adbDiff[i][j] & 0xFF00)>>8);
			len++;
			*(buf+len) = (char)(adbDiff[i][j] & 0x00FF);
			len++;
		}
	}
	return len;
}

static ssize_t AW5x0x_get_FreqScan(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i;
	ssize_t len = 0;

	for(i=0;i< 32;i++)
	{
		//*(buf+len) = (char)((AWDeltaData[i] & 0xFF00)>>8);
		//len++;
		//*(buf+len) = (char)(AWDeltaData[i] & 0x00FF);
		//len++;
		len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",AWDeltaData[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	return len;
}

static ssize_t AW5x0x_Set_FreqScan(struct device* cd, struct device_attribute *attr,
		       const char* buf, size_t len)
{
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);
	unsigned long Basefreq = simple_strtoul(buf, NULL, 10);

	if(Basefreq < 16)
	{
	#ifdef INTMODE
		ctp_disable_irq();
		AW5x0x_Sleep();
		suspend_flag = 1;
		AW_Sleep(50);

		FreqScan(Basefreq);

		AW5x0x_TP_Reinit();
		ctp_enable_irq();
		suspend_flag = 0;
	#else
		suspend_flag = 1;
		AW_Sleep(200);

		FreqScan(Basefreq);

		AW5x0x_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;

		mod_timer(&data->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
	#endif
	}
	return len;
}

static ssize_t AW5x0x_get_reg(struct device* cd,struct device_attribute *attr, char* buf)
{
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);
	u8 reg_val[128];
	ssize_t len = 0;
	u8 i;

	if(suspend_flag != 1)
	{
	#ifdef INTMODE
		ctp_disable_irq();
		AW5x0x_Sleep();
		suspend_flag = 1;
		AW_Sleep(50);

		AW_I2C_ReadXByte(reg_val,0,127);
		
		AW5x0x_TP_Reinit();
		ctp_enable_irq();
		suspend_flag = 0;
	#else
		suspend_flag = 1;
		
		AW_Sleep(50);
				
		AW_I2C_ReadXByte(reg_val,0,127);

		AW5x0x_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		
		mod_timer(&data->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
	#endif
	}
	else
	{
		AW_I2C_ReadXByte(reg_val,0,127);
	}
	
	for(i=0;i<0x7F;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "reg%02X = 0x%02X, ", i,reg_val[i]);
	}

	return len;

}

static ssize_t AW5x0x_write_reg(struct device* cd,struct device_attribute *attr, char *buf, size_t count)
{
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);
	int databuf[2];
	
	if(2 == sscanf(buf, "%d %d", &databuf[0], &databuf[1]))
	{ 
		if(suspend_flag != 1)
		{
		#ifdef INTMODE
			ctp_disable_irq();
			AW5x0x_Sleep();
			suspend_flag = 1;
			AW_Sleep(50);

			AW_I2C_WriteByte((u8)databuf[0],(u8)databuf[1]);
			
			AW5x0x_TP_Reinit();
			ctp_enable_irq();
			suspend_flag = 0;
		#else
			suspend_flag = 1;
			AW_Sleep(50);
			
			AW_I2C_WriteByte((u8)databuf[0],(u8)databuf[1]);

			AW5x0x_TP_Reinit();
			tp_idlecnt = 0;
			tp_SlowMode = 0;
			suspend_flag = 0;
			mod_timer(&data->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
		#endif
		}
		else
		{
			AW_I2C_WriteByte((u8)databuf[0],(u8)databuf[1]);
		}
	}
	else
	{
		printk("invalid content: '%s', length = %d\n", buf, count);
	}
	return count; 
}

static ssize_t AW5x0x_GetUcf(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char ret,i,j;
	short count;
	ssize_t len = 0;
	
	len += snprintf(buf+len, PAGE_SIZE-len,"*****AW5x0x UCF DATA*****\n");
	
	len += snprintf(buf+len, PAGE_SIZE-len,"PAGE1:\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,\n",AWTPCfg.CHIPVER);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,\n",AWTPCfg.TX_LOCAL,AWTPCfg.RX_LOCAL);
	len += snprintf(buf+len, PAGE_SIZE-len,"(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d}\n",
		AWTPCfg.TX_ORDER[0],AWTPCfg.TX_ORDER[1],AWTPCfg.TX_ORDER[2],AWTPCfg.TX_ORDER[3],AWTPCfg.TX_ORDER[4],
		AWTPCfg.TX_ORDER[5],AWTPCfg.TX_ORDER[6],AWTPCfg.TX_ORDER[7],AWTPCfg.TX_ORDER[8],AWTPCfg.TX_ORDER[9],
		AWTPCfg.TX_ORDER[10],AWTPCfg.TX_ORDER[11],AWTPCfg.TX_ORDER[12],AWTPCfg.TX_ORDER[13],AWTPCfg.TX_ORDER[14],
		AWTPCfg.TX_ORDER[15],AWTPCfg.TX_ORDER[16],AWTPCfg.TX_ORDER[17],AWTPCfg.TX_ORDER[19],AWTPCfg.TX_ORDER[19],
		AWTPCfg.TX_ORDER[20],AWTPCfg.TX_ORDER[21],AWTPCfg.TX_ORDER[22],AWTPCfg.TX_ORDER[23],AWTPCfg.TX_ORDER[24],
		AWTPCfg.TX_ORDER[25]);
	len += snprintf(buf+len, PAGE_SIZE-len,"{%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d},\n",
					AWTPCfg.RX_ORDER[0],AWTPCfg.RX_ORDER[1],AWTPCfg.RX_ORDER[2],AWTPCfg.RX_ORDER[3],
					AWTPCfg.RX_ORDER[4],AWTPCfg.RX_ORDER[5],AWTPCfg.RX_ORDER[6],AWTPCfg.RX_ORDER[7],
					AWTPCfg.RX_ORDER[8],AWTPCfg.RX_ORDER[9],AWTPCfg.RX_ORDER[10],AWTPCfg.RX_ORDER[11],
					AWTPCfg.RX_ORDER[12],AWTPCfg.RX_ORDER[13]);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,\n",AWTPCfg.RX_START,AWTPCfg.HAVE_KEY_LINE);
	len += snprintf(buf+len, PAGE_SIZE-len,"{%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d},\n",
		AWTPCfg.KeyLineValid[0],AWTPCfg.KeyLineValid[1],AWTPCfg.KeyLineValid[2],AWTPCfg.KeyLineValid[3],
		AWTPCfg.KeyLineValid[4],AWTPCfg.KeyLineValid[5],AWTPCfg.KeyLineValid[6],AWTPCfg.KeyLineValid[7],
		AWTPCfg.KeyLineValid[8],AWTPCfg.KeyLineValid[9],AWTPCfg.KeyLineValid[10],AWTPCfg.KeyLineValid[11]);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,\n",AWTPCfg.MAPPING_MAX_X,AWTPCfg.MAPPING_MAX_Y);
	
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,\n",AWTPCfg.GainClbDeltaMin,AWTPCfg.GainClbDeltaMax,
		AWTPCfg.RawDataDeviation,AWTPCfg.GainClbDeltaMin);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,\n",AWTPCfg.KeyLineDeltaMin,AWTPCfg.KeyLineDeltaMax,
		AWTPCfg.CacMultiCoef,AWTPCfg.GainTestDeltaMax);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,\n",AWTPCfg.OffsetClbExpectedMin,AWTPCfg.OffsetClbExpectedMax,
		AWTPCfg.GAIN_CLB_SEPERATE,AWTPCfg.KeyLineTestDeltaMin);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,\n",AWTPCfg.RawDataCheckMin,AWTPCfg.RawDataCheckMax,
		AWTPCfg.FIRST_CALI,AWTPCfg.KeyLineTestDeltaMax);

	len += snprintf(buf+len, PAGE_SIZE-len,"\nPAGE2:\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,0x%x,0x%x,\n",AWTPCfg.MULTI_SCANFREQ,AWTPCfg.BASE_FREQ,
		AWTPCfg.FREQ_OFFSET,AWTPCfg.WAIT_TIME);
	len += snprintf(buf+len, PAGE_SIZE-len,"0x%x,0x%x,%d,%d\n",AWTPCfg.CHAMP_CFG,AWTPCfg.POSLEVEL_TH,
		AWTPCfg.RAWDATA_DUMP_SWITCH,AWTPCfg.ESD_PROTECT);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,\n",AWTPCfg.PEAK_TH,AWTPCfg.GROUP_TH,AWTPCfg.BIGAREA_TH,AWTPCfg.BIGAREA_CNT);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,\n",AWTPCfg.BIGAREA_FRESHCNT,AWTPCfg.PEAK_ROW_COMPENSATE,
		AWTPCfg.PEAK_COL_COMPENSATE,AWTPCfg.PEAK_COMPENSATE_COEF);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,\n",AWTPCfg.POINT_RELEASEHOLD,AWTPCfg.MARGIN_RELEASEHOLD,
		AWTPCfg.POINT_PRESSHOLD,AWTPCfg.KEY_PRESSHOLD);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,\n",AWTPCfg.STABLE_DELTA_X,AWTPCfg.STABLE_DELTA_Y,AWTPCfg.FIRST_DELTA);
	len += snprintf(buf+len, PAGE_SIZE-len,"CHARGE:\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.CHARGE_PEAK_TH,AWTPCfg.CHARGE_GROUP_TH,AWTPCfg.CHARGE_BIGAREA_TH);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.CHARGE_PEAK_ROW_COMPENSATE,AWTPCfg.CHARGE_PEAK_COL_COMPENSATE,
		AWTPCfg.CHARGE_PEAK_COMPENSATE_COEF);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.CHARGE_POINT_RELEASEHOLD,AWTPCfg.CHARGE_MARGIN_RELEASEHOLD,
		AWTPCfg.CHARGE_POINT_PRESSHOLD);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.CHARGE_KEY_PRESSHOLD,AWTPCfg.CHARGE_STABLE_DELTA_X,
		AWTPCfg.CHARGE_STABLE_DELTA_Y);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.CHARGE_FIRST_DELTA,AWTPCfg.CHARGE_SECOND_HOLD,AWTPCfg.CHARGE_SECOND_DELTA);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d\n",AWTPCfg.FREQ_JUMP,AWTPCfg.ID_LOOKUP);

	len += snprintf(buf+len, PAGE_SIZE-len,"\nPAGE3:\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,\n",AWTPCfg.CACULATE_COEF);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d,%d,\n",AWTPCfg.MARGIN_COMPENSATE,AWTPCfg.MARGIN_COMP_DATA_UP,
		AWTPCfg.MARGIN_COMP_DATA_DOWN,AWTPCfg.MARGIN_COMP_DATA_LEFT,AWTPCfg.MARGIN_COMP_DATA_RIGHT);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,\n",AWTPCfg.FLYING_TH,AWTPCfg.MOVING_TH,AWTPCfg.MOVING_ACCELER);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,\n",AWTPCfg.LCD_NOISE_PROCESS,AWTPCfg.LCD_NOISETH);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,\n",AWTPCfg.FALSE_PEAK_PROCESS,AWTPCfg.FALSE_PEAK_TH);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,\n",AWTPCfg.DEBUG_LEVEL,AWTPCfg.FAST_FRAME,AWTPCfg.SLOW_FRAME);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,\n",AWTPCfg.MARGIN_PREFILTER,AWTPCfg.BIGAREA_HOLDPOINT);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.BASE_MODE,AWTPCfg.WATER_REMOVE,AWTPCfg.INT_MODE);
	len += snprintf(buf+len, PAGE_SIZE-len,"PROXIMITY:\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,\n",AWTPCfg.PROXIMITY);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.PROXIMITY_LINE,AWTPCfg.PROXIMITY_TH_HIGH,AWTPCfg.PROXIMITY_TH_LOW);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.PROXIMITY_TIME,AWTPCfg.PROXIMITY_CNT_HIGH,AWTPCfg.PROXIMITY_CNT_LOW);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d,%d\n",AWTPCfg.PROXIMITY_TOUCH_TH_HIGH,AWTPCfg.PROXIMITY_TOUCH_TH_LOW,
				AWTPCfg.PROXIMITY_PEAK_CNT_HIGH,AWTPCfg.PROXIMITY_PEAK_CNT_LOW);
	len += snprintf(buf+len, PAGE_SIZE-len,"%d,%d,%d\n",AWTPCfg.PROXIMITY_LAST_TIME,AWTPCfg.PROXIMITY_TOUCH_TIME,AWTPCfg.PROXIMITY_SATUATION);

	return len;

}

static ssize_t AW5x0x_GetCaliSt(struct device* cd,struct device_attribute *attr, char* buf)
{
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len,  "%d",AWTP_cali_st);
	return len;

}

static ssize_t AW5x0x_GetTestSt(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char ret,i,j;
	short count;
	ssize_t len = 0;

	if(TestErrorCode == 0)
	{
		len += snprintf(buf+len,PAGE_SIZE-len,"test OK");
	}
	else
	{
		len += snprintf(buf+len,PAGE_SIZE-len,"test FAIL%d",TestErrorCode);
	}

	return len;

}

static ssize_t AW5x0x_GetTest(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char ret,i,j;
	short count;
	ssize_t len = 0;
	
	len += snprintf(buf+len, PAGE_SIZE-len,  "%d",AWTP_test_st);
	return len;
	
}


static ssize_t AW5x0x_SetTest(struct device* cd,struct device_attribute *attr, char *buf, size_t count)
{
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
	AWTP_test_st = 0;	// set "0" to test_st indicates the test is running

	if(on_off == 1)
	{	
	#ifdef INTMODE
		ctp_disable_irq();
		AW5x0x_Sleep();
		suspend_flag = 1;
		AW_Sleep(50);
		AW5x0x_TP_Test();

		AW5x0x_TP_Reinit();
		ctp_enable_irq(); 
		suspend_flag = 0;
	#else
		suspend_flag = 1;
		AW_Sleep(50);
		AW5x0x_TP_Test();

		AW5x0x_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;

		mod_timer(&data->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
	#endif
	}

	AWTP_test_st = 1;	// set "1" to test_st indicates the test is finished
	
	return count;
}


static int AW5x0x_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	//TS_DBG("%s", __func__);
	
	err = device_create_file(dev, &dev_attr_cali);
	err = device_create_file(dev, &dev_attr_readreg);
	err = device_create_file(dev, &dev_attr_base);
	err = device_create_file(dev, &dev_attr_diff);
	err = device_create_file(dev, &dev_attr_adbbase);
	err = device_create_file(dev, &dev_attr_adbdiff);
	err = device_create_file(dev, &dev_attr_freqscan);
	err = device_create_file(dev, &dev_attr_getucf);
	err = device_create_file(dev, &dev_attr_cali_st);
	err = device_create_file(dev, &dev_attr_test_st);
	err = device_create_file(dev, &dev_attr_test);
	return err;
}

static void AW5x0x_ts_release(void)
{
	short tmp;
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);

	for(tmp=0; tmp< 5; tmp++)
	{
		input_mt_slot(data->input_dev, tmp);//按序号上???		//input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, false);
	}
	
	input_sync(data->input_dev);
	return;

}
static void AW5x0x_report_multitouch(void)
{
	char i;
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	int down_table = 0 ,up_table = 1;      
     
#ifdef TOUCH_KEY_SUPPORT
	if(1 == key_tp){
		return;
	}
#endif
	switch(event->touch_point) {
	case 5:
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
		input_mt_slot(data->input_dev, 4);
		input_report_abs(data->input_dev,ABS_MT_TRACKING_ID,4);
              down_table |= 1<<4;   
		input_report_abs(data->input_dev,  ABS_MT_TRACKING_ID, event->touch_ID[4]);

		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[4]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[4]);			
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);

	case 4:
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
		input_mt_slot(data->input_dev, 3);
		input_report_abs(data->input_dev,ABS_MT_TRACKING_ID,3);
                down_table |= 1<<3;   
		input_report_abs(data->input_dev,  ABS_MT_TRACKING_ID, event->touch_ID[3]);
		
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[3]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[3]);			
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		
	case 3:
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
		input_mt_slot(data->input_dev, 2);
		input_report_abs(data->input_dev,ABS_MT_TRACKING_ID,2);
                down_table |= 1<<2;   
		input_report_abs(data->input_dev,  ABS_MT_TRACKING_ID, event->touch_ID[2]);
		
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[2]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[2]);			
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		
	case 2:
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
		input_mt_slot(data->input_dev, 1);
		input_report_abs(data->input_dev,ABS_MT_TRACKING_ID,1);
                down_table |= 1<<1;   
		input_report_abs(data->input_dev,  ABS_MT_TRACKING_ID, event->touch_ID[1]);
		
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[1]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[1]);			
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
	case 1:
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
		input_mt_slot(data->input_dev, 0);
		input_report_abs(data->input_dev,ABS_MT_TRACKING_ID,0);
                down_table |= 1<<0;   
		input_report_abs(data->input_dev,  ABS_MT_TRACKING_ID, event->touch_ID[0]);
		
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[0]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[0]);			
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		
	//	printk("===x1 = %d,y1 = %d ====\n",event->x[0],event->y[0]);
		break;
	default:
//		print_point_info("==touch_point default =\n");
		break;
	}


  for (i =0;i<5;i++)
      {
        if (((~down_table)&1<<i)&&!(up_table&1<<i))
           {
            input_mt_slot(data->input_dev,i);
            input_report_abs(data->input_dev,ABS_MT_TRACKING_ID,-1);
           }
       }
  up_table = ~down_table;

	
	input_sync(data->input_dev);
	
	event->pre_point = event->touch_point;
	dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__,
		event->x[0], event->y[0], event->x[1], event->y[1]);
	return;
}

static int AW5x0x_read_data(void)
{
	struct AW5x0x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	 int Pevent;
    int i = 0;
	static int flag_realse = 2;

	if(AW5x0x_TouchProcess())
	{
		event->touch_point = AW5x0x_GetPointNum();

		for(i=0;i<event->touch_point;i++)
		{
			AW5x0x_GetPoint(&event->x[i],&event->y[i],&event->touch_ID[i],&Pevent,i);
			swap(event->x[i], event->y[i]);
			if(AWTPCfg.DEBUG_LEVEL > 0)
			{
				printk("point%d = %d,%d,%d \n",i,event->x[i],event->y[i],event->touch_ID[i] );
			}
		}
	    	
		if (event->touch_point == 0) 
		{
			if(tp_idlecnt <= AWTPCfg.FAST_FRAME*5)
			{
				tp_idlecnt++;
			}
			if(tp_idlecnt > AWTPCfg.FAST_FRAME*5)
			{
				tp_SlowMode = 1;
			}
			
			if (event->pre_point != 0)
			{
			    AW5x0x_ts_release();
				if(flag_realse--==0)
					event->pre_point = 0;
			}
			return 1; 
		}
		else
		{
			flag_realse = 2; 
			tp_SlowMode = 0;
			tp_idlecnt = 0;
			event->pre_point = event->touch_point; 
			event->pressure = 20;

			AW5x0x_report_multitouch();
			return 0;
		}
	}
	else
	{
		return 1;
	}
}


#ifdef CHARGE_DETECT
#define AW_CHARGE_FILENAME	"sys/class/power_supply/battery/status"
void AW5x0x_charge_detect(void)
{
	char charge_flag=0;
	struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(this_client);
	if(AW5x0x_ts&&AW5x0x_ts->get_system_charge_status){
		if(AW5x0x_ts->get_system_charge_status()){
			charge_flag=1;
		}else{
			charge_flag=0;
		}
	}else{
		char batstatus[20];
	  memset(batstatus,0,20);
	  AW_nvram_read(AW_CHARGE_FILENAME,batstatus,20,0);
    if(memcmp(batstatus,"Charging",8) == 0 || memcmp(batstatus,"Full",4) == 0){
		  charge_flag=1;
	  }
	  else{
		  charge_flag=0;
	  }
	}
	if(AW5x0x_ts->charge_status!=charge_flag){
	 if(charge_flag){
		 printk("charging");
		 AW5x0x_ChargeMode(1);
	 }else{
		 printk("not charging");
		 AW5x0x_ChargeMode(0);
	 }
	 AW5x0x_ts->charge_status=charge_flag;
	}
}

static void AW5x0x_charge_polling(unsigned long arg)
{
	struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(this_client);

	if(suspend_flag != 1)		
	{
		queue_work(AW5x0x_ts->ts_workqueue, &AW5x0x_ts->Charge_Detectwork);
		if(AW5x0x_ts->get_system_charge_status)//hardware detect
		   mod_timer(&AW5x0x_ts->charge_detect,jiffies + HZ*1);
		else
			 mod_timer(&AW5x0x_ts->charge_detect,jiffies + HZ*3);
	}
	return ;
}
#endif

static void AW5x0x_ts_pen_irq_work(struct work_struct *work)
{
#ifdef INTMODE
	if(suspend_flag != 1)
	{
		ctp_enable_irq();
	}
#endif
  if(suspend_flag != 1){
	  AW5x0x_read_data();
	}
}


#ifdef INTMODE
static irqreturn_t AW5x0x_ts_interrupt(int irq, void *dev_id)
{
	struct AW5x0x_ts_data *AW5x0x_ts = dev_id;
	int reg_val;
		
	ctp_disable_irq();		
	if (!work_pending(&AW5x0x_ts->pen_event_work)) 
	{
		queue_work(AW5x0x_ts->ts_workqueue, &AW5x0x_ts->pen_event_work);
	}

	return IRQ_HANDLED;
}
#endif

#ifdef SMART_READ_DATA
int AW5x0x_write_detect_data(void)
{
	int ret;
	char data_buf[10] = {"AWINIC"};

	ret = AW_nvram_write(AW_DATA_FILENAME,(char *)data_buf,sizeof(data_buf),0);
	if(ret == -1)
	{
		printk("AW5306 fail to write data_buf file! \n");
	}
	else
	{
		printk("AW5306 !!! success !to write data_buf file! \n");
		need_read_data = 0;
	}
}
#endif

void AW5x0x_tpd_polling(void)
 {
	struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(this_client);

#ifdef SMART_READ_DATA
	if(need_read_data)
	{
		queue_work(AW5x0x_ts->ts_workqueue,&AW5x0x_ts->read_data_work);
		mod_timer(&AW5x0x_ts->touch_timer,jiffies + HZ);
		return;
	}
#endif
	
	if (!work_pending(&AW5x0x_ts->pen_event_work)) 
	{
		queue_work(AW5x0x_ts->ts_workqueue, &AW5x0x_ts->pen_event_work);
	}

#ifdef INTMODE 
	ctp_enable_irq();
	return HRTIMER_NORESTART;
#else
	if(suspend_flag != 1)
	{
#ifdef AUTO_RUDUCEFRAME
		if(tp_SlowMode){  	
			mod_timer(&AW5x0x_ts->touch_timer,jiffies + HZ/AWTPCfg.SLOW_FRAME);
		}
		else{
			mod_timer(&AW5x0x_ts->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
		}
#else
		mod_timer(&AW5x0x_ts->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
#endif
	}
#endif
 }


//停用设备
static int AW5x0x_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(this_client);

#ifdef INTMODE
	if(suspend_flag != 1)
	{
		suspend_flag = 1;
		msleep(50);
		AW5x0x_Sleep();
		ctp_disable_irq();
	}
#else
	if(suspend_flag != 1)
	{
		printk("AW5x0x SLEEP!!!");
		suspend_flag = 1;
		AW5x0x_Sleep(); 
	}  
#endif
}

extern void AW5x0x_User_Cfg1(void);
static int AW5x0x_ts_resume(struct i2c_client *client)
{
	struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(this_client);

#ifdef INTMODE
	if(suspend_flag != 0)
	{
		AW5x0x_User_Cfg1();
		AW5x0x_TP_Reinit();
		suspend_flag = 0;
		mod_timer(&AW5x0x_ts->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);
#ifdef CHARGE_DETECT
		mod_timer(&AW5x0x_ts->charge_detect,jiffies + HZ*3);
#endif

	}
#else
	if(suspend_flag != 0)
	{
		AW5x0x_User_Cfg1();
		AW5x0x_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		printk("AW5x0x WAKE UP!!!");
    mod_timer(&AW5x0x_ts->touch_timer,jiffies + HZ/AWTPCfg.FAST_FRAME);  
#ifdef CHARGE_DETECT
		mod_timer(&AW5x0x_ts->charge_detect,jiffies + HZ*3);
#endif
	}
#endif
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void AW5x0x_ts_early_suspend(struct early_suspend *h)
{
	struct AW5x0x_ts_data *AW5x0x_ts;
	AW5x0x_ts = container_of(h, struct AW5x0x_ts_data, early_suspend);
	AW5x0x_ts_suspend(NULL, PMSG_SUSPEND);
}

static void AW5x0x_ts_late_resume(struct early_suspend *h)
{
	struct AW5x0x_ts_data *AW5x0x_ts;
	AW5x0x_ts = container_of(h, struct AW5x0x_ts_data, early_suspend);
	AW5x0x_ts_resume(NULL);
}
#endif
 

/*******************************************************	
Function:
	Touch-screen detection function
	Called when the registration drive (required for a corresponding client);
	For IO, interrupts and other resources to apply; equipment registration; touch screen initialization, etc.
Parameters:
	client: the device structure to be driven
	id: device ID
return:
	Results of the implementation code, 0 for normal execution
********************************************************/
static int AW5x0x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	int retry=0;
	char test_data = 1;
	char buf[2]={0};   
	unsigned char read_data[2] = {0,0 };  
	unsigned char reg_value; 
	struct AW5x0x_ts_data *AW5x0x_ts;
	struct aw5x0x_platform_data *pdata=NULL;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		dev_err(&client->dev, "Must have I2C_FUNC_I2C.\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	
	AW5x0x_ts = kzalloc(sizeof(*AW5x0x_ts), GFP_KERNEL);
	if (AW5x0x_ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	
  pdata=client->dev.platform_data;
  if(pdata&&pdata->init_platform_hw){
  	if(pdata->init_platform_hw()<0)
  	  goto err_hw_init_failed;
  }
	AW5x0x_ts->reset_gpio=pdata->reset_gpio;
  AW5x0x_ts->irq_gpio=pdata->irq_gpio;
  #ifndef INTMODE  //avoid gpio output mode
  if(AW5x0x_ts->irq_gpio!=INVALID_GPIO){
  	if(gpio_request(AW5x0x_ts->irq_gpio,"aw5x0x_irq_pin")==0){
  		gpio_direction_input(AW5x0x_ts->irq_gpio);
  	}
  }
  #endif
	if(pdata&&pdata->get_system_charge_status)
		AW5x0x_ts->get_system_charge_status=pdata->get_system_charge_status;
	else
		AW5x0x_ts->get_system_charge_status=NULL;
	AW5x0x_ts->charge_status=0x02;//esure 执行
	
  this_client = client;
	printk("client addr1 = %x", client->addr);

	reg_value = AW_I2C_ReadByte(0x01);
	printk("[gll]reg_value = %x", reg_value);
	if(reg_value != 0xB8)
	{
		client->addr = 0x39;
		dev_err(&client->dev, "AW5x0x_ts_probe: CHIP ID NOT CORRECT\n");
		goto err_i2c_failed;
	}
	i2c_set_clientdata(client, AW5x0x_ts);
	
	INIT_WORK(&AW5x0x_ts->pen_event_work, AW5x0x_ts_pen_irq_work);
	INIT_WORK(&AW5x0x_ts->Charge_Detectwork, AW5x0x_charge_detect);
	AW5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!AW5x0x_ts->ts_workqueue) {
		ret = -ESRCH;
		goto exit_create_singlethread;
	}

	AW5x0x_ts->input_dev = input_allocate_device();
	if (AW5x0x_ts->input_dev == NULL) 
	{
		ret = -ENOMEM;
		dev_dbg(&client->dev,"AW5x0x_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}


/*
	input_set_abs_params(ts->input_dev, ABS_X, 0,  ts->abs_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, ts->abs_y_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
*/	

	__set_bit(INPUT_PROP_DIRECT, AW5x0x_ts->input_dev->propbit);
	__set_bit(EV_ABS, AW5x0x_ts->input_dev->evbit);
	
	input_mt_init_slots(AW5x0x_ts->input_dev, 5);
	input_set_abs_params(AW5x0x_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(AW5x0x_ts->input_dev, ABS_MT_POSITION_X, 0, TOUCH_MAX_HEIGHT, 0, 0);
	input_set_abs_params(AW5x0x_ts->input_dev, ABS_MT_POSITION_Y, 0, TOUCH_MAX_WIDTH, 0, 0);	
	
	

  sprintf(AW5x0x_ts->phys, "input/ts");
	AW5x0x_ts->input_dev->name = AW5x0x_I2C_NAME;
	AW5x0x_ts->input_dev->phys = AW5x0x_ts->phys;
	AW5x0x_ts->input_dev->id.bustype = BUS_I2C;
	AW5x0x_ts->input_dev->dev.parent = &client->dev;

	ret = input_register_device(AW5x0x_ts->input_dev);
	if (ret) {
		dev_err(&client->dev,"Probe: Unable to register %s input device\n", AW5x0x_ts->input_dev->name);
		goto err_input_register_device_failed;
	}
	AW5x0x_ts->bad_data = 0;

#ifdef CONFIG_HAS_EARLYSUSPEND
	AW5x0x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;//EARLY_SUSPEND_LEVEL_BLANK_SCREEN +1;
	AW5x0x_ts->early_suspend.suspend = AW5x0x_ts_early_suspend;
	AW5x0x_ts->early_suspend.resume = AW5x0x_ts_late_resume;
	register_early_suspend(&AW5x0x_ts->early_suspend);
#endif

	dev_info(&client->dev,"Start %s in %s mode,Driver Modify Date:2012-01-05\n", 
	AW5x0x_ts->input_dev->name, AW5x0x_ts->use_irq ? "interrupt" : "polling");

	AW5x0x_create_sysfs(client);

	memcpy(AW_CALI_FILENAME,"/data/tpcali",12);
	memcpy(AW_UCF_FILENAME,"/data/AWTPucf",13);
#ifdef SMART_READ_DATA
		memcpy(AW_DATA_FILENAME,"/data/AWdetect",14);
#endif
  
	AW5x0x_TP_Init();  

	AW5x0x_ts->touch_timer.function = AW5x0x_tpd_polling;
	AW5x0x_ts->touch_timer.data = AW5x0x_ts;
	init_timer(&AW5x0x_ts->touch_timer);
	AW5x0x_ts->touch_timer.expires = jiffies + HZ*10;
	add_timer(&AW5x0x_ts->touch_timer);
#ifdef CHARGE_DETECT
	AW5x0x_ts->charge_detect.function = AW5x0x_charge_polling;
	AW5x0x_ts->charge_detect.data = AW5x0x_ts;
	init_timer(&AW5x0x_ts->charge_detect);
	AW5x0x_ts->charge_detect.expires = jiffies + HZ*15;
	add_timer(&AW5x0x_ts->charge_detect);
#endif

#ifdef INTMODE
	rc=  request_irq(AW5x0x_ts->irq, AW5x0x_ts_interrupt, IRQF_TRIGGER_FALLING, client->name, AW5x0x_ts);
	if (rc < 0) {
		printk( "AW5x0x_probe: request irq failed\n");
		goto error_req_irq_fail;
	}
#endif
  suspend_flag=0;
  
	printk("AW5x0x_ts  probe ok .........\n");
	
	return 0;

error_req_irq_fail:
    free_irq(AW5x0x_ts->irq, AW5x0x_ts);	
err_input_register_device_failed:
	input_free_device(AW5x0x_ts->input_dev);

err_input_dev_alloc_failed:
	i2c_set_clientdata(client, NULL);
exit_create_singlethread:
err_i2c_failed:	
err_hw_init_failed:
	kfree(AW5x0x_ts);	
err_alloc_data_failed:
err_check_functionality_failed:
err_create_proc_entry:

	return ret;
}
                                                                                                                       
/*******************************************************	
Function:
	Drive the release of resources
Parameters:
	client: the device structure
return:
	Results of the implementation code, 0 for normal execution
********************************************************/
static int AW5x0x_ts_remove(struct i2c_client *client)
{
	struct AW5x0x_ts_data *AW5x0x_ts = i2c_get_clientdata(client);
	
	pr_info("==AW5x0x_ts_remove=\n");
#ifdef INTMODE
	free_irq(SW_INT_IRQNO_PIO, AW5x0x_ts);
#else
	del_timer(&AW5x0x_ts->touch_timer);
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&AW5x0x_ts->early_suspend);
#endif
	input_unregister_device(AW5x0x_ts->input_dev);
	input_free_device(AW5x0x_ts->input_dev);
	cancel_work_sync(&AW5x0x_ts->pen_event_work);
	destroy_workqueue(AW5x0x_ts->ts_workqueue);
	kfree(AW5x0x_ts);
    
	i2c_set_clientdata(client, NULL);
	//ctp_ops.free_platform_resource();

	return 0;

}

                                                                                                                             
//可用于该驱动???设备名—设备ID 列表                                                                              
//only one client                                                                                                               
static const struct i2c_device_id AW5x0x_ts_id[] = {                                                                            
	{ AW5x0x_I2C_NAME, 0 },                                                                                                       
	{ }                                                                                                                           
};                                                                                                                              
                                                                                                                                
//设备驱动结构???                                                                                                         
static struct i2c_driver AW5x0x_ts_driver = {                                                                                   
	.probe		= AW5x0x_ts_probe,                                                                                                  
	.remove		= AW5x0x_ts_remove,                                                                                                 
#ifndef CONFIG_HAS_EARLYSUSPEND                                                                                                 
	.suspend	= AW5x0x_ts_suspend,                                                                                                
	.resume		= AW5x0x_ts_resume,                                                                                                 
#endif                                                                                                                          
	.id_table	= AW5x0x_ts_id,                                                                                                     
	.driver = {                                                                                                                   
		.name	= AW5x0x_I2C_NAME,                                                                                                    
		.owner = THIS_MODULE,                                                                                                       
	},                                                                                                                            
}; 

/*******************************************************	
功能???	驱动加载函数
return???	执行结果码，0表示正常执行
********************************************************/
static int __devinit AW5x0x_ts_init(void)
{
	int ret;
	ret=i2c_add_driver(&AW5x0x_ts_driver);
	printk("[gll] ret = %d\n",ret);
	return ret; 
}

/*******************************************************	
功能???	驱动卸载函数
参数???	client：设备结构体
********************************************************/
static void __exit AW5x0x_ts_exit(void)
{
	printk(KERN_ALERT "Touchscreen driver of guitar exited.\n");
	i2c_del_driver(&AW5x0x_ts_driver);
}


late_initcall(AW5x0x_ts_init); 
module_exit(AW5x0x_ts_exit);

MODULE_DESCRIPTION("AW5x0x Touchscreen Driver");
MODULE_LICENSE("GPL");
               

