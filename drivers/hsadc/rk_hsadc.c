/* drivers/hsadc/chips/rk30_hsadc.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
*/
#include <linux/blkdev.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/delay.h>
#include <linux/irq.h>

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

#include <linux/dma-mapping.h>
#include <asm/dma.h>
#include <linux/gfp.h>
//#include <mach/dma-pl330.h>
#include <asm/scatterlist.h>

//#include <mach/cru.h>

#include <linux/dmaengine.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#endif

#include "rk_hsadc.h"

struct hsadc_client {
	int time;
	int result;
	void (*callback)(struct hsadc_client *, void *, int);
	void *callback_param;

	struct hsadc_host_t *hsadc_host;
};

struct hsadc_request {
	int result;
	void (*callback)(struct hsadc_client *, void *, int);
	void *callback_param;
	struct hsadc_client *client;
	/* Used in case of sync requests */
	struct completion completion;
};
struct hsadc_host_t;
struct hsadc_ops {
	void (*start)(struct hsadc_host_t *);
	void (*stop)(struct hsadc_host_t *);
	int (*read)(struct hsadc_host_t *);
};
	
struct hsadc_host_t {
	struct device *dev;
	int is_suspended;
	struct mutex queue_mutex;
	struct hsadc_client *cur;
	const struct hsadc_ops *ops;
	unsigned long		private[0];
};

static inline void *hsadc_priv(struct hsadc_host_t *hsadc_host)
{
	return (void *)hsadc_host->private;
}

struct rk29_hsadc_device {
	int			 		irq;
	struct clk *		clk_ahb;
	struct clk *		clk_hsadc;
	resource_size_t  reg_start;	
	void __iomem		*regs;	
	struct resource		*ioarea;
	struct dma_chan              *dmach;	
	dma_addr_t		dma_addr;
	struct resource		*dma_res;
	struct hsadc_host_t		*hsadc_host;
};

static struct hsadc_host_t *g_hsadc = NULL;

//#define RK29_HSADC_IRQ_REQUEST

//debug config
static int rk29_hsadc_debug = 1;
#define dprintk if (rk29_hsadc_debug) printk

extern int hsadc_dma_kmalloc_buf_head; 
extern int pl330_dma_getposition(struct dma_chan *chan, dma_addr_t *src, dma_addr_t *dst);


static void rk29_hsadc_stop_dma(struct hsadc_host_t *hsadc_host)
{
	struct rk29_hsadc_device *hsadc_dev  = hsadc_priv(hsadc_host);
	
	//rk29_dma_ctrl(hsadc_dev->dmach, RK29_DMAOP_STOP);
	//rk29_hsadc_dma_cleanup(hsadc_host);
	
	dmaengine_terminate_all(hsadc_dev->dmach);
}

static void rk29_hsadc_start(struct hsadc_host_t *hsadc_host)
{
	struct rk29_hsadc_device *hsadc_dev  = hsadc_priv(hsadc_host);
	unsigned int	dma_len=0;
	int ret = 0;
	dma_addr_t		dest_dma_addr;
	struct dma_slave_config slave_config;
	
        struct scatterlist sg;
	struct dma_async_tx_descriptor *desc = NULL;

#if 0
      //select hsadc_ext clock
	temp = ioread32(RK30_CRU_BASE + 0x9c);		
	temp = (temp & ~(0x30 | (0x3<< 20)));
       writel(temp | (0x2<< 4) | (0x3<< 20), RK30_CRU_BASE + 0x9c);
	dsb();	
#endif	
			
	/* HSADC control register */
	iowrite32((HSADC_ALMOST_FULL_LEVEL80 | HSADC_ALMOST_EMPTY_LEVEL32 | HSADC_SELECT_MPEG_TRANSPORT_STREAM_INPUT | HSADC_SINGLE_CHANNEL_DATA_INPUT
	| HSADC_VALID_INTERFACE_ENABLE  | HSADC_SYNC_INTERFACE_ENABLE
                   | HSADC_STORE_MODE_8BIT | HSADC_ALMOST_FULL_DMA_REQUEST | HSADC_MOST_SIGNIFICANT_BIT_NOT_NEGATION | HSADC_STORE_TO_LOW_8_10_BIT
                   | HSADC_DATA_BUS_WIDTH_8_BIT | HSADC_NOT_SELECT_GPS | HSADC_INTERFACE_UNIT_ENABLE),  hsadc_dev->regs + HSADC_CTRL);
       /* HSADC interrupt enable/mask register */
	iowrite32((HSADC_EMPTY_INTERRUPT_DISABLE | HSADC_EMPTY_INTERRUPT_DISABLE),  hsadc_dev->regs + HSADC_IER);
       hsadc_dev->dma_addr = (hsadc_dev->reg_start + HSADC_DATA);
	dest_dma_addr = virt_to_dma(NULL, (void *)hsadc_dma_kmalloc_buf_head);
	
	memset((void *)hsadc_dma_kmalloc_buf_head, 0, 1024*2048);	

#if 0
       ret = rk29_dma_devconfig(hsadc_dev->dmach, RK29_DMASRC_HW, (unsigned long )(hsadc_dev->dma_addr));	   
	 ret = rk29_dma_config(hsadc_dev->dmach, 4, 1);
        ret = rk29_dma_ctrl(hsadc_dev->dmach, RK29_DMAOP_FLUSH);
    	 ret = rk29_dma_enqueue_ring(hsadc_dev->dmach, hsadc_host, dest_dma_addr, 1024*2048/128, 128, false);  
	 ret = rk29_dma_ctrl(hsadc_dev->dmach, RK29_DMAOP_START);	
#else
//
	slave_config.direction = DMA_DEV_TO_MEM;	
	slave_config.src_addr = hsadc_dev->dma_addr;	
	slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;	
	slave_config.src_maxburst = 16;
	ret = dmaengine_slave_config(hsadc_dev->dmach, &slave_config);	
	if (ret) {
		dprintk("error in TX dma configuration."); 	
	}
//
    dma_len  = 1024*2048;
	sg_init_table(&sg, 1);
	sg_dma_len(&sg) = dma_len;
	sg_set_page(&sg, pfn_to_page(PFN_DOWN(dest_dma_addr)),
			dma_len, offset_in_page(dest_dma_addr));
	sg_dma_address(&sg) = dest_dma_addr;
	
	//desc = dmaengine_prep_slave_sg(hsadc_dev->dmach,
		//&sg, 1, DMA_DEV_TO_MEM, DMA_PREP_CONTINUE);

	desc = dmaengine_prep_dma_cyclic(hsadc_dev->dmach, 
			dest_dma_addr,
			1024*2048, 
			1024*2048/32,
			DMA_DEV_TO_MEM, DMA_PREP_INTERRUPT);	
	
	
	desc->callback = NULL;	
	//desc->callback_param = dma;
	
	dmaengine_submit(desc);
	dma_async_issue_pending(hsadc_dev->dmach);	
#endif
	dprintk("%s[%d]\n",__FUNCTION__,__LINE__);		
}

static void rk29_hsadc_stop(struct hsadc_host_t *hsadc_host)
{
	struct rk29_hsadc_device *hsadc_dev  = hsadc_priv(hsadc_host);
	/* HSADC control register */	
	iowrite32((HSADC_INTERFACE_UNIT_DISABLE),  hsadc_dev->regs + HSADC_CTRL);
       /* HSADC interrupt enable/mask register */
	iowrite32((HSADC_EMPTY_INTERRUPT_DISABLE | HSADC_FULL_INTERRUPT_DISABLE),  hsadc_dev->regs + HSADC_IER);   
}

static int rk29_hsadc_read(struct hsadc_host_t *hsadc_host)
{
	struct rk29_hsadc_device *hsadc_dev  = hsadc_priv(hsadc_host);
       dma_addr_t src, dst;
    void *addr;       

	  pl330_dma_getposition(hsadc_dev->dmach, &src, &dst);
    addr = (void *)dma_to_virt(NULL, dst);	
	   	   
	return (u32)addr;
}

//------------------------------------------
int rk29_hsadc_start_transmit(void)
{
    rk29_hsadc_start(g_hsadc);
	
    return 0;
}
EXPORT_SYMBOL(rk29_hsadc_start_transmit);

int rk29_hsadc_stop_transmit(void)
{
    int i;
    char *data_addr = (char *)hsadc_dma_kmalloc_buf_head;
    char data;
	
    rk29_hsadc_stop(g_hsadc);
    rk29_hsadc_stop_dma(g_hsadc);

    dprintk( "%s[%d]: !!!!!!!!!!!!!!the hsadc data!!!!!!!!!!!!!!!!!!\n",__FUNCTION__,__LINE__);		
    for(i = 0; i < 200; i++)
    {
        data = data_addr[i+1024];
        dprintk( "0x%x ",  data);	       
    }
	
    return 0;
}
EXPORT_SYMBOL(rk29_hsadc_stop_transmit);

int rk29_hsadc_get_cur_transmit_addr(void)
{
   int addr =0;
   
    addr = rk29_hsadc_read(g_hsadc);
	
    return addr;
}
EXPORT_SYMBOL(rk29_hsadc_get_cur_transmit_addr);
//------------------------------------------

	
#ifdef RK29_HSADC_IRQ_REQUEST	
static irqreturn_t rk29_hsadc_irq(int irq, void *data)
{
	dprintk( "%s[%d]\n",__FUNCTION__,__LINE__);	
	
	return IRQ_HANDLED;
}
#endif

static const struct hsadc_ops rk29_hsadc_ops = {
	.start		= rk29_hsadc_start,
	.stop		= rk29_hsadc_stop,
	.read		= rk29_hsadc_read,
};

static struct hsadc_host_t *hsadc_alloc_host(int extra, struct device *dev)
{
	struct hsadc_host_t *hsadc_host;
	
	hsadc_host = kzalloc(sizeof(struct hsadc_host_t) + extra, GFP_KERNEL);
	if (!hsadc_host)
		return NULL;
	hsadc_host->dev = dev;
	g_hsadc = hsadc_host;
	
	return hsadc_host;
}

static void hsadc_free_host(struct hsadc_host_t *hsadc_host)
{
	kfree(hsadc_host);
	hsadc_host = NULL;
	return;
}

static int  rk29_hsadc_probe(struct platform_device *pdev)
{
	struct hsadc_host_t *hsadc_host= NULL;
	struct rk29_hsadc_device *hsadc_dev;
	struct resource *res;
	int ret;
	struct device *dev;

	printk("%s[%d]\n",__FUNCTION__,__LINE__);	

	hsadc_host = hsadc_alloc_host(sizeof(struct rk29_hsadc_device), &pdev->dev);
	if (!hsadc_host)
		return -ENOMEM;
	
	mutex_init(&hsadc_host->queue_mutex);
	hsadc_host->dev = &pdev->dev;
	dev = &pdev->dev;
	hsadc_host->is_suspended = 0;
	hsadc_host->ops = &rk29_hsadc_ops;
	
	hsadc_dev= hsadc_priv(hsadc_host);
	hsadc_dev->hsadc_host = hsadc_host;
	
#ifdef RK29_HSADC_IRQ_REQUEST	
	//hsadc irq
	hsadc_dev->irq = platform_get_irq(pdev, 0);
	if (hsadc_dev->irq <= 0) {
		printk("failed to get hsadc irq\n");
		ret = -ENOENT;
	}

	ret = request_irq(hsadc_dev->irq, rk29_hsadc_irq, 0, pdev->name, NULL);
	if (ret < 0) {
		printk("failed to attach hsadc irq\n");
	}	
#endif	

    clk_set_parent(clk_get(dev, "clk_hsadc_out"), clk_get(NULL, "clk_hsadc_ext"));	
	//hsadc  clock
	hsadc_dev->clk_hsadc= clk_get(dev, "clk_hsadc_ext");
	if (IS_ERR(hsadc_dev->clk_hsadc)) {
		printk("failed to get hsadc_ext clock\n");
		ret = PTR_ERR(hsadc_dev->clk_hsadc);
	}
	else {
	    printk("success to get hsadc_ext clock\n");
    }
	clk_prepare_enable(hsadc_dev->clk_hsadc);	

	//hsadc ahb clock
	hsadc_dev->clk_ahb = clk_get(dev, "clk_hsadc_out");
	if (IS_ERR(hsadc_dev->clk_ahb)) {
		printk("failed to get clk_hsadc_out clock\n");
		ret = PTR_ERR(hsadc_dev->clk_ahb);
	}
	else {
	    printk("success to get clk_hsadc_out clock\n");
    }	
	clk_prepare_enable(hsadc_dev->clk_ahb);	

	//hsadc ahb clock
	hsadc_dev->clk_ahb = clk_get(dev, "hclk_hsadc");
	if (IS_ERR(hsadc_dev->clk_ahb)) {
		printk("failed to get clk_hsadc_out clock\n");
		ret = PTR_ERR(hsadc_dev->clk_ahb);
	}
	else {
	    printk("success to get hclk_hsadc clock\n");
    }	
	clk_prepare_enable(hsadc_dev->clk_ahb);		
	
       //hsadc register IO memory resource
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk("cannot find IO resource\n");
		ret = -ENOENT;
	}
	hsadc_dev->reg_start = res->start;
	hsadc_dev->ioarea = request_mem_region(res->start, (res->end - res->start) + 1,  pdev->name);
	if(hsadc_dev->ioarea == NULL) {
		printk("cannot request IO\n");
		ret = -ENXIO;
	}
	hsadc_dev->regs = ioremap(res->start, (res->end - res->start) + 1);
	if (!hsadc_dev->regs) {
		printk("cannot map IO\n");
		ret = -ENXIO;
	}
	hsadc_dev->dmach = dma_request_slave_channel(dev, "data");	
	if (!hsadc_dev->dmach)
	{
		printk( "Failed to get hsadc DMA channel\n");
	}

	
	platform_set_drvdata(pdev, hsadc_dev);
	dprintk("rk29 hsadc: driver initialized\n");

	return 0;

	iounmap(hsadc_dev->regs);

	release_mem_region(res->start,  (res->end - res->start) + 1);  //releae resource hsadc_dev->ioarea

 	clk_put(hsadc_dev->clk_hsadc);
	clk_put(hsadc_dev->clk_ahb);
			
 #ifdef RK29_HSADC_IRQ_REQUEST	
 free_irq(hsadc_dev->irq, hsadc_dev);
 #endif

	hsadc_free_host(hsadc_dev->hsadc_host);
	
	return ret;
}

static int  rk29_hsadc_remove(struct platform_device *pdev)
{
	struct rk29_hsadc_device *hsadc_dev = platform_get_drvdata(pdev);

       //rk29_dma_free(hsadc_dev->dmach, &rk29_dma_hsadc_client);
	iounmap(hsadc_dev->regs);
	release_resource(hsadc_dev->ioarea);
	kfree(hsadc_dev->ioarea);
	clk_disable(hsadc_dev->clk_ahb);
	clk_put(hsadc_dev->clk_ahb);
	clk_disable(hsadc_dev->clk_hsadc);
	clk_put(hsadc_dev->clk_hsadc);		
 #ifdef RK29_HSADC_IRQ_REQUEST		
	free_irq(hsadc_dev->irq, hsadc_dev);
 #endif	
	hsadc_free_host(hsadc_dev->hsadc_host);

	return 0;
}

#ifdef CONFIG_PM
static int rk29_hsadc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct rk29_hsadc_device *hsadc_dev = platform_get_drvdata(pdev);

	hsadc_dev->hsadc_host->is_suspended = 1;
	
	return 0;
}

static int rk29_hsadc_resume(struct platform_device *pdev)
{
	struct rk29_hsadc_device *hsadc_dev = platform_get_drvdata(pdev);

	hsadc_dev->hsadc_host->is_suspended = 0;
	
	return 0;
}

#endif

#ifdef CONFIG_OF
static const struct of_device_id rockchip_hsadc_dt_match[] = {
	{ .compatible = "rockchip-hsadc",},
	{ },
};
MODULE_DEVICE_TABLE(of, rockchip_hsadc_dt_match);
#endif /* CONFIG_OF */


static struct platform_driver rk29_hsadc_driver = {
	.driver		= {
		.name	= "rockchip-hsadc",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(rockchip_hsadc_dt_match),			
	},
	.probe		= rk29_hsadc_probe,
	.remove		= rk29_hsadc_remove,
	.suspend	= rk29_hsadc_suspend,
	.resume		= rk29_hsadc_resume,	
};
//MODULE_ALIAS("platform:rockchip-hsadc");


static int __init rk29_hsadc_init(void)
{
	printk("***rkdtv***\t%s[%d] 111\n",__FUNCTION__,__LINE__);
	return platform_driver_register(&rk29_hsadc_driver);
}

static void __exit rk29_hsadc_exit(void)
{
	printk("***rkdtv***\t%s[%d]\n",__FUNCTION__,__LINE__);
	platform_driver_unregister(&rk29_hsadc_driver);
}
int  rk29_pid_filter_ctrl(uint8_t onoff)
{

       return 0;		
}
EXPORT_SYMBOL(rk29_pid_filter_ctrl);

int rk29_pid_filter_set(uint8_t id, uint16_t pid, uint8_t onoff)
{
       return 0;	
}
EXPORT_SYMBOL(rk29_pid_filter_set);

module_init(rk29_hsadc_init);
module_exit(rk29_hsadc_exit);

MODULE_DESCRIPTION("Driver for HSADC");
MODULE_AUTHOR("aiyoujun, ayj@rock-chips.com");
MODULE_LICENSE("GPL");
