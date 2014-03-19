/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/kfifo.h>

#include "rk_ipc.h"
#include "rk_ipc_mbox.h"

#define DRV_NAME	"rk3190-mbox"

/* RK3190 Mailbox Registers */
#define MBOX_A2B_INTEN		0x00
#define MBOX_A2B_STATUS		0x04
#define MBOX_A2B_CMD0		0x08
#define MBOX_A2B_DATA0		0x0c
#define MBOX_A2B_CMD1		0x10
#define MBOX_A2B_DATA1		0x14
#define MBOX_A2B_CMD2		0x18
#define MBOX_A2B_DATA2		0x1c
#define MBOX_A2B_CMD3		0x20
#define MBOX_A2B_DATA3		0x24

#define MBOX_B2A_INTEN		0x28
#define MBOX_B2A_STATUS		0x2c
#define MBOX_B2A_CMD0		0x30
#define MBOX_B2A_DATA0		0x34
#define MBOX_B2A_CMD1		0x38
#define MBOX_B2A_DATA1		0x3c
#define MBOX_B2A_CMD2		0x40
#define MBOX_B2A_DATA2		0x44
#define MBOX_B2A_CMD3		0x48
#define MBOX_B2A_DATA3		0x4c


#define MBOX_CHx_CMD_OUT(x)		(MBOX_A2B_CMD0 + ((x) << 3))
#define MBOX_CHx_DATA_OUT(x)	(MBOX_A2B_DATA0 + ((x) << 3))
#define MBOX_CHx_CMD_IN(x)		(MBOX_B2A_CMD0 + ((x) << 3))
#define MBOX_CHx_DATA_IN(x)		(MBOX_B2A_DATA0 + ((x) << 3))

#define MBOX_CH0_CMD_OUT	MBOX_A2B_CMD0
#define MBOX_CH0_DATA_OUT	MBOX_A2B_DATA0
#define MBOX_CH1_CMD_OUT	MBOX_A2B_CMD1
#define MBOX_CH1_DATA_OUT	MBOX_A2B_DATA1
#define MBOX_CH2_CMD_OUT	MBOX_A2B_CMD2
#define MBOX_CH2_DATA_OUT	MBOX_A2B_DATA2
#define MBOX_CH3_CMD_OUT	MBOX_A2B_CMD3
#define MBOX_CH3_DATA_OUT	MBOX_A2B_DATA3

#define MBOX_CH0_CMD_IN		MBOX_B2A_CMD0
#define MBOX_CH0_DATA_IN	MBOX_B2A_DATA0
#define MBOX_CH1_CMD_IN		MBOX_B2A_CMD1
#define MBOX_CH1_DATA_IN	MBOX_B2A_DATA1
#define MBOX_CH2_CMD_IN		MBOX_B2A_CMD2
#define MBOX_CH2_DATA_IN	MBOX_B2A_DATA2
#define MBOX_CH3_CMD_IN		MBOX_B2A_CMD3
#define MBOX_CH3_DATA_IN	MBOX_B2A_DATA3

#define MBOX_INTEN			MBOX_B2A_INTEN
#define MBOX_INTSTAT		MBOX_B2A_STATUS

#define RMT_MBOX_INTSTAT	MBOX_A2B_STATUS
#define RMT_MBOX_INTEN		MBOX_A2B_INTEN


#define MBOX_CHAN_NUM		4
#define MBOX_CHAN_MASKBITS	((1 << MBOX_CHAN_NUM) - 1)

/*-----------------------------------------------------*/

struct rk3190_mbox {
	struct ipc_mbox mbox;

	int irq[MBOX_CHAN_NUM];
	void __iomem *base;
	int usecount;

	struct blocking_notifier_head        notifier;

	unsigned long	chan_free_bitmask;
	
	u32 send_seqno;
	u32 recv_seqno;

	struct kfifo	in_fifo;		// receive FIFO

	struct work_struct work;

	struct workqueue_struct *mboxd;

	spinlock_t	lock;
};

static struct rk3190_mbox *hw_mbox;


static inline void rk3190_mbox_writel(struct rk3190_mbox *pmb, int offset, u32 val)
{
	writel(val, pmb->base + offset);
}

static inline u32 rk3190_mbox_readl(struct rk3190_mbox *pmb, int offset)
{
	return readl(pmb->base + offset);
}

static inline void mbox_chan_set_free(struct rk3190_mbox* pmb, int ch)
{
	set_bit(ch, &pmb->chan_free_bitmask);
}

static inline void mbox_chan_set_busy(struct rk3190_mbox* pmb, int ch)
{
	clear_bit(ch, &pmb->chan_free_bitmask);
}

static inline int mbox_chan_is_free(struct rk3190_mbox *pmb, int ch)
{
	return test_bit(ch, &pmb->chan_free_bitmask);
}

static inline void mbox_chan_write(struct rk3190_mbox *pmb, int ch, u32 cmd, u32 data)
{
	rk3190_mbox_writel(pmb, MBOX_CHx_CMD_OUT(ch), cmd);
	rk3190_mbox_writel(pmb, MBOX_CHx_DATA_OUT(ch), data);

	mbox_chan_set_busy(pmb, ch);
}

static inline u32 mbox_chan_read_cmd(struct rk3190_mbox *pmb, int ch)
{
	return rk3190_mbox_readl(pmb, MBOX_CHx_CMD_IN(ch));
}

static inline u32 mbox_chan_read_data(struct rk3190_mbox* pmb, int ch)
{
	return rk3190_mbox_readl(pmb, MBOX_CHx_DATA_IN(ch));
}

static int mbox_chan_available(struct rk3190_mbox *pmb)
{
	return (pmb->chan_free_bitmask == 0) ? 0 : 1;
}

static int mbox_chan_update_available(struct rk3190_mbox* pmb)
{
	if (pmb->chan_free_bitmask == 0) {
		unsigned long rmt_intstat, rmt_inten;
		unsigned long avail;

		rmt_intstat = rk3190_mbox_readl(pmb, RMT_MBOX_INTSTAT);
		rmt_inten = rk3190_mbox_readl(pmb, RMT_MBOX_INTEN);
		avail = ~rmt_intstat & rmt_inten;

		if (avail)
			pmb->chan_free_bitmask = avail & MBOX_CHAN_MASKBITS;
	}

	return mbox_chan_available(pmb);
}

static irqreturn_t rk3190_mbox_interrupt(int irq, void *p)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)p;
	unsigned long flags;
	spin_lock_irqsave(&pmb->lock, flags);
	while (1) { 
			unsigned long status, clear = 0;
			u32 msg_array[MBOX_CHAN_NUM] = {0};
			int msg_num = 0;
			int ch = 0;
	
			// get IRQ status
			status = rk3190_mbox_readl(pmb, MBOX_INTSTAT) & MBOX_CHAN_MASKBITS;
	
			if (status == 0) {
				break;
			}
	
			do {
				if (status & 0x1) {
					u32 seqno;
					u32 index;
	
					seqno = mbox_chan_read_data(pmb, ch);
					index = seqno - pmb->recv_seqno;
	
					if (index < MBOX_CHAN_NUM) {
						msg_array[index] = mbox_chan_read_cmd(pmb, ch);
						msg_num++;
	
					} else {
						WARN(1, "Mailbox Recieve Message sequence number error!\n");
						//reset seqno 
						pmb->send_seqno = 0;
						pmb->recv_seqno = seqno;
						index = seqno - pmb->recv_seqno;
						msg_array[index] = mbox_chan_read_cmd(pmb, ch);
						msg_num++;
					}
	
					clear |= (1 << ch); 
				}
	
				status >>= 1;
				ch++;
	
			} while (status);
	
			// update in sequence No.
			pmb->recv_seqno += msg_num;
	
			// notice : make sure clear IRQ status after data/cmd fetched.
			rk3190_mbox_writel(pmb, MBOX_INTSTAT, clear);
	
			//IPC_DBG(DEBUG_INFO,"%s %d status=%d clear=%d\n",__func__,__LINE__,status,clear);
			if (msg_num > 0) {
				int i;
	
				for (i=0; i<msg_num; i++) {
					// check soft FIFO full
					if (kfifo_avail(&pmb->in_fifo) < sizeof(u32)) {
						WARN(1, "Mailbox Receive FIFO full!\n");
						break;
					}
	
					kfifo_in(&pmb->in_fifo, (unsigned char*)&msg_array[i], sizeof(u32));
				}
			}
		}
	queue_work(pmb->mboxd, &pmb->work);
	spin_unlock_irqrestore(&pmb->lock, flags);
	return IRQ_HANDLED;
}

static void rk3190_mbox_rx_work(struct work_struct *work)
{
	struct rk3190_mbox *pmb = container_of(work, struct rk3190_mbox, work);	
	blocking_notifier_call_chain(&pmb->notifier, RK_IPCMBOX_EVENT_MSG_INCOMING, NULL);
	// update free channels
	if (!mbox_chan_available(pmb) && 
			mbox_chan_update_available(pmb)) {
		blocking_notifier_call_chain(&pmb->notifier, RK_IPCMBOX_EVENT_MSG_WRITEABLE, NULL);
	} 
}


static int rk3190_mbox_startup(struct ipc_mbox *imb)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)imb;
	int i;

	rk3190_mbox_writel(pmb, MBOX_INTEN, 0);			// disable interrupts
	rk3190_mbox_writel(pmb, MBOX_INTSTAT, ~0x0);	// clear interrupts

	pmb->mboxd = create_workqueue("mboxd");
	if (!pmb->mboxd) {
		return -ENOMEM;
	}

	INIT_WORK(&pmb->work, rk3190_mbox_rx_work);

	for (i=0; i<ARRAY_SIZE(pmb->irq); i++) {
		if (pmb->irq[i] >= 0) {
			request_irq(pmb->irq[i], rk3190_mbox_interrupt, 0, "mbox", pmb);
		}
	}
	IPC_DBG(DEBUG_INFO,"=============%s=================",__func__);
	// enable mbox interrupt
	rk3190_mbox_writel(pmb, MBOX_INTEN, MBOX_CHAN_MASKBITS);

	return 0;
}

static int rk3190_mbox_shutdown(struct ipc_mbox *imb)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)imb;
	int i;

	for (i=0; i<ARRAY_SIZE(pmb->irq); i++) {
		if (pmb->irq[i] >= 0) {
			free_irq(pmb->irq[i], pmb);
		}
	}

	destroy_workqueue(pmb->mboxd);

	return 0;
}

static int rk3190_mbox_reset(struct ipc_mbox *imb)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)imb;

	// clear sequence number
	pmb->send_seqno = 0;
	pmb->recv_seqno = 0;

	//TODO other things reset
	return 0;
}

static int rk3190_mbox_msg_put(struct ipc_mbox *imb, u32 msg)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)imb;
	int i;
	if (!mbox_chan_update_available(pmb)){
		return -1;
	}
	for (i=0; i<MBOX_CHAN_NUM; i++) {
		if (mbox_chan_is_free(pmb, i)) {
			mbox_chan_write(pmb, i, msg, pmb->send_seqno++);
			return 0;
		}
	}
	BUG_ON(1);

	return -1;
}

static int rk3190_mbox_msg_get(struct ipc_mbox* imb, u32 *msg)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)imb;
	int len;
	if (kfifo_len(&pmb->in_fifo) >= sizeof(u32)) {
		len = kfifo_out((&pmb->in_fifo), (unsigned char*)msg, sizeof(u32));
		WARN_ON(len != sizeof(u32));

		return 0;
	}
	return -1;
}

static int rk3190_mbox_notifier_register(struct ipc_mbox *imb, struct notifier_block *nb)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)imb;

	if (nb) {
		blocking_notifier_chain_register(&pmb->notifier, nb);
	}

	return 0;
}

static int rk3190_mbox_notifier_unregister(struct ipc_mbox *imb, struct notifier_block *nb)
{
	struct rk3190_mbox *pmb = (struct rk3190_mbox *)imb;

	if (nb) {
		blocking_notifier_chain_unregister(&pmb->notifier, nb);
	}

	return 0;
}

static int __devinit rk3190_mbox_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device *dev;
	void __iomem *base;
	struct rk3190_mbox *pmb;
	int i;

	dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (unlikely(res == NULL)) {
		dev_err(dev, "Invalid IRQ resource!\n");
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(dev, "Invalid MEM resource!\n");
		return -ENODEV;
	}
	
	base = ioremap(res->start, resource_size(res));
	if (base == NULL) {
		dev_err(dev, "Could not ioremap mailbox!\n");
		return -ENOMEM;
	}

	pmb = kzalloc(sizeof(*pmb), GFP_KERNEL);
	if (pmb == NULL) {
		dev_err(dev, "No more memory!\n");
		return -ENOMEM;
	}

	for (i=0; i<ARRAY_SIZE(pmb->irq); i++) {
		pmb->irq[i] = platform_get_irq(pdev, i);
	}
	pmb->base = base;

	pmb->mbox.name = DRV_NAME;
	pmb->mbox.startup = rk3190_mbox_startup;
	pmb->mbox.shutdown = rk3190_mbox_shutdown;
	pmb->mbox.reset = rk3190_mbox_reset;
	pmb->mbox.msg_write = rk3190_mbox_msg_put;
	pmb->mbox.msg_read = rk3190_mbox_msg_get;
	pmb->mbox.notifier_register = rk3190_mbox_notifier_register;
	pmb->mbox.notifier_unregister = rk3190_mbox_notifier_unregister;

	rk_mbox_register(&pmb->mbox);

	BLOCKING_INIT_NOTIFIER_HEAD(&pmb->notifier);

	kfifo_alloc(&pmb->in_fifo, 512, GFP_KERNEL);

	pmb->chan_free_bitmask = 0;	//MBOX_CHAN_MASKBITS;

	spin_lock_init(&pmb->lock);
	
	hw_mbox = pmb;
	pr_info("%s:probe ok\n",__func__);
	return 0;
}


static struct platform_driver rk3190_mbox_driver = {
	.driver = {
		.name = DRV_NAME,
	},
};

static int __init rk3190_mbox_init(void)
{
	return platform_driver_probe(&rk3190_mbox_driver, rk3190_mbox_probe);
}

device_initcall(rk3190_mbox_init);
