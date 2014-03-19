/*
 * Inter-Processor Communication Protocol
 *
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "rk_ipc.h"
#include "rk_ipc_mem.h"
#include "rk_ipc_mbox.h"
#include "rk_ipcmsg_queue.h"


struct rk_ipc {
	struct ipc_mbox *mbox;
	
	phys_addr_t ipc_memphys;
	size_t	ipc_memsize;
	void __iomem* ipc_membase;

	struct ipc_msg_pool *ipcmsgp;
	struct ipc_shmem_pool *shmemp;
	
	struct ipcmsg_queue *txpendingQ;

	struct workqueue_struct *wq;
	struct work_struct work;

	spinlock_t		send_lock;
};

struct cmd_handler_entry {
	ipc_msg_handler_t func;
	void *data;
};

static struct rk_ipc *the_ipc;

static struct cmd_handler_entry handler_info_registered[IPCMSG_MAX_CMD];

static void ipc_msg_flush(void);

/*
	IPC MEMORY LAYOUT:

     |<------------------- ipc_memsize --------------->|

              AP                      BB
-----|------------------------|------------------------|-----
...  |     |                  |      |                 |...
-----|------------------------|------------------------|-----
        ^            ^           ^             ^
       ipcmsg pool  shmem pool  ipcmsg pool  shmem pool
     ^
     ipc_memphys

*/

#define CONFIG_DEBUG_FS 1
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>

static struct dentry *debugfs_root;

static struct runtime_statistics {
	unsigned long alloc_count;
	unsigned long free_count;
	unsigned long submit_count;
	unsigned long complete_count;
	unsigned long receive_count;
	unsigned long giveback_count;
} ipc_debug_statis;

static inline void inc_ipcmsg_alloc_count(void) { ipc_debug_statis.alloc_count++; }
static inline void inc_ipcmsg_free_count(void) { ipc_debug_statis.free_count++; }
static inline void inc_ipcmsg_submit_count(void) { ipc_debug_statis.submit_count++; }
static inline void inc_ipcmsg_complete_count(void) { ipc_debug_statis.complete_count++; }
static inline void inc_ipcmsg_receive_count(void) { ipc_debug_statis.receive_count++; }
static inline void inc_ipcmsg_giveback_count(void) { ipc_debug_statis.giveback_count++; }

static int ipc_statistics_show(struct seq_file *s, void *unused)
{
		seq_printf(s, "ipc statistics:\n\n");

		seq_printf(s, "ipc_msg alloc count :\t%lu\n", ipc_debug_statis.alloc_count);
		seq_printf(s, "ipc_msg free count :\t%lu\n\n", ipc_debug_statis.free_count);

		seq_printf(s, "ipc_msg submit count :\t%lu\n", ipc_debug_statis.submit_count);
		seq_printf(s, "ipc_msg complete count :\t%lu\n\n", ipc_debug_statis.complete_count);

		seq_printf(s, "ipc_msg receive count :\t%lu\n", ipc_debug_statis.receive_count);
		seq_printf(s, "ipc_msg give back count :\t%lu\n\n", ipc_debug_statis.giveback_count);
		return 0;
}

static int ipc_statistics_open(struct inode *inode, struct file *file)
{
		return single_open(file, ipc_statistics_show, inode->i_private);
}

static const struct file_operations statistics_fops = {
	.open           = ipc_statistics_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int __init ipc_debugfs_init(void)
{
	struct dentry       *root;
	struct dentry       *file;
	int ret;

	root = debugfs_create_dir("ipc", NULL);
	if (IS_ERR(root)) {
		ret = PTR_ERR(root);
		goto err0;
	}


	file = debugfs_create_file("statistics", S_IRUSR, root, NULL, &statistics_fops);
	if (IS_ERR(file)) {
		ret = PTR_ERR(file);
		goto err1;
	}

	debugfs_root = root;
	return 0;

err1:
	debugfs_remove_recursive(root);
err0:
	return ret;
}
#else
static inline int __init ipc_debugfs_init(void) {return 0;}

static inline void inc_ipcmsg_alloc_count(void){}
static inline void inc_ipcmsg_free_count(void){}
static inline void inc_ipcmsg_submit_count(void){}
static inline void inc_ipcmsg_complete_count(void){}
static inline void inc_ipcmsg_receive_count(void){}
static inline void inc_ipcmsg_giveback_count(void){}
#endif        /* CONFIG_DEBUG_FS */

/* 
 * change IPC memory address from physical to virtual
 */
static inline void* ipc_mem_phys_to_virt(phys_addr_t paddr)
{
	BUG_ON(the_ipc == NULL ||
		paddr < the_ipc->ipc_memphys || 
		paddr >= (the_ipc->ipc_memphys + the_ipc->ipc_memsize));

	return (void*)((unsigned long)paddr - (unsigned long)the_ipc->ipc_memphys + (unsigned long)the_ipc->ipc_membase);
}

/*
 * change IPC memory address from virtual to physical
 */
static inline phys_addr_t ipc_mem_virt_to_phys(void * vaddr)
{
	BUG_ON(the_ipc == NULL ||
		vaddr < the_ipc->ipc_membase || 
		vaddr >= (the_ipc->ipc_membase + the_ipc->ipc_memsize));

	return (phys_addr_t)((unsigned long)vaddr - (unsigned long)the_ipc->ipc_membase + the_ipc->ipc_memphys);
}

/**
 * @brief : convert IPC memory physical address to virtual.
 *
 * @param[in] paddr: physical address of IPC memory.
 *
 * @return : converted virtual address
 */
void* rk_ipc_mem_p2v(phys_addr_t paddr)
{
	return ipc_mem_phys_to_virt(paddr);
}

/**
 * @brief : convert IPC memory virtual address to physical.
 *
 * @param[in] vaddr: virtual address of IPC memory.
 *
 * @return : converted physical address
 */
phys_addr_t rk_ipc_mem_v2p(void *vaddr)
{
	return ipc_mem_virt_to_phys(vaddr);
}


static int update_ipcmsg_next_phys(struct ipc_msg *pmsg)
{
	if (pmsg->next != NULL) {
		pmsg->next_phys = ipc_mem_virt_to_phys(pmsg->next);

	} else {
		pmsg->next_phys = 0;
	}

	return 0;
}

static int update_ipcmsg_next_virt(struct ipc_msg *pmsg)
{
	if (pmsg->next_phys != 0) {
		pmsg->next = (struct ipc_msg*)ipc_mem_phys_to_virt(pmsg->next_phys);

	} else {
		pmsg->next = NULL;
	}

	return 0;
}

static int iterator_ipcmsg_link(struct ipc_msg *pfirst, int (*fn)(struct ipc_msg *))
{
	struct ipc_msg *pcur, *pprev = NULL;
	int num = 0;
	int ret;

	pcur = pfirst;

	while (pcur) {
		ret = fn(pcur);

		if (unlikely(ret != 0)) {
			WARN_ON(1);
			if (pprev) {
				pprev->next = NULL;
			}

			break;
		}

		num++;
		pprev = pcur;
		pcur = pprev->next;
	}

	return num;
}

static inline void* alloc_shmem(int sz)
{
	struct ipc_shmem_pool *pool;

	pool = the_ipc->shmemp;

	return ipc_shmem_pool_alloc(pool, sz);
}

static inline void free_shmem(void *p, int sz)
{
	struct ipc_shmem_pool *pool;

	pool = the_ipc->shmemp;

	return ipc_shmem_pool_free(pool, p, sz);
}

static inline struct ipc_msg* alloc_ipcmsg(void)
{
	struct ipc_msg_pool *pool;

	pool = the_ipc->ipcmsgp;

	return ipc_msg_pool_alloc(pool);
}

static inline void free_ipcmsg(struct ipc_msg *pmsg)
{
	struct ipc_msg_pool *pool;

	pool = the_ipc->ipcmsgp;

	return ipc_msg_pool_free(pool, pmsg);
}

static inline int tx_queue_empty(void)
{
	return ipcmsg_queue_isempty(the_ipc->txpendingQ);
}


static inline struct ipc_msg* tx_queue_get_first(void)
{
	return ipcmsg_queue_peek(the_ipc->txpendingQ);
}

static inline void tx_queue_clear(void)
{
	ipcmsg_queue_clear(the_ipc->txpendingQ);
}

static inline void tx_queue_for_each(void* data, int (*fn)(struct ipc_msg *, void*))
{
	ipcmsg_queue_for_each(the_ipc->txpendingQ, data, fn);	
}

static inline int mbox_send_msg(struct ipc_msg *pmsg)
{
	u32 pmsg_phys = ipc_mem_virt_to_phys(pmsg);

	return the_ipc->mbox->msg_write(the_ipc->mbox, pmsg_phys);
}

static inline struct ipc_msg* mbox_get_msg(void)
{
	u32 msg;
	int ret;

	// Note : IPC msg receive from mailbox is a physical address.
	ret = the_ipc->mbox->msg_read(the_ipc->mbox, &msg);

	return (ret == 0) ? ipc_mem_phys_to_virt((phys_addr_t)msg) : NULL;
}

/**
 * @brief : register a IPC msg command handler
 *
 * @param[in] cmd: the command ID to register.
 * @param[in] func: registered command handle function.
 * @param[in] data : private data pass to command handle function when execute.
 *
 * @return : 0 means success, < 0 fail.
 */
int ipc_msg_register_handler(u32 cmd, ipc_msg_handler_t func, void *data)
{
	if (cmd < IPCMSG_MAX_CMD) {
		handler_info_registered[cmd].func = func;
		handler_info_registered[cmd].data = data;
		return 0;
	}

	return -EINVAL;
}


/**
 * @brief allocate a IPC msg, and also allocate more share memory optional.
 *
 * @param[in][out] ppmsg : pointer to allocated IPC msg.
 * @param[in] shmem_len : extra share memory length to allocate.
 *
 * @return : 0 means success, -1 means fail.
 */
int ipc_msg_new(struct ipc_msg **ppmsg, int shmem_len)
{
	struct ipc_msg *pmsg;
	void *pbuf = NULL;

	if (shmem_len > 0) {
		pbuf = alloc_shmem(shmem_len);
		if (pbuf == NULL)
			return -1;
	}

	pmsg = alloc_ipcmsg();
	if (pmsg == NULL) {
		free_shmem(pbuf, shmem_len);
		return -1;
	}

	memset((void *)pmsg, 0, sizeof(*pmsg));

	IPCMSG_SET_MAGIC(pmsg);
	pmsg->shmem = pbuf;
	pmsg->shmem_len = shmem_len;

	if (pmsg->shmem != NULL) {
		pmsg->shmem_phys = ipc_mem_virt_to_phys(pmsg->shmem);
	}

	inc_ipcmsg_alloc_count();

	*ppmsg = pmsg;

	return 0;
}

/**
 * @brief re-init a IPC msg for recycle used
 *
 * @param[in] pmsg : IPC msg.
 *
 * @return : 0 means success, -1 means fail.
 */
int ipc_msg_reinit(struct ipc_msg *pmsg)
{
	void *shmem = pmsg->shmem;
	phys_addr_t shmem_phys = pmsg->shmem_phys;
	int shmem_size = pmsg->shmem_len;

	memset((void *)pmsg, 0, sizeof(*pmsg));

	pmsg->shmem = shmem;
	pmsg->shmem_len = shmem_size;
	pmsg->shmem_phys = shmem_phys;

	IPCMSG_SET_MAGIC(pmsg);

	return 0;
}

/**
 * @brief free the memory of mailbox request block
 *
 * @param[in] pmsg : the mailbox requet block to be free
 *
 * @return : void
 */
int ipc_msg_free(struct ipc_msg *pmsg)
{
	if (pmsg->shmem != NULL) {
		free_shmem(pmsg->shmem, pmsg->shmem_len);
	}

	free_ipcmsg(pmsg);

	inc_ipcmsg_free_count();

	return 0;
}

static int pre_ipc_msg_submit(struct ipc_msg * pmsg)
{
	if (!IPCMSG_CHECK_MAGIC(pmsg)) {
		WARN(1, "submit ipcmsg %p invalid\n", pmsg);
		return -EINVAL;
	}

	if (!IPCMSG_IS_RESPONSE(pmsg)) {
		inc_ipcmsg_submit_count();
	}

	update_ipcmsg_next_phys(pmsg);

	return 0;
}

/**
 * @brief submit a IPC msg to send
 *
 * @param[in] pmsg : IPC msg or IPC msg link (when msg->next not NULL).
 *
 * @return : 0 means success, <0 means fail.
 */
int ipc_msg_submit(struct ipc_msg *pmsg)
{
	int ret = -1;
	unsigned long flags;

	if (pmsg == NULL) {
		return -EINVAL;
	}

	if (the_ipc == NULL) {
		return -ENODEV;
	}

	iterator_ipcmsg_link(pmsg, pre_ipc_msg_submit);

	spin_lock_irqsave(&the_ipc->send_lock, flags);

	if (tx_queue_empty()) {
		ret = mbox_send_msg(pmsg);
	}
	
	if (ret < 0) {
		ipcmsg_queue_enqueue(the_ipc->txpendingQ, pmsg);
	}

	spin_unlock_irqrestore(&the_ipc->send_lock, flags);

	if (ret < 0) {
		ipc_msg_flush();
	}

	return 0;
}


struct __sync_ipcmsg_context {
	struct completion done;
	int status;
};

static void __ipcmsg_blocking_completion(struct ipc_msg *pmsg)
{
	struct __sync_ipcmsg_context *ctx = pmsg->context;

	ctx->status = IPCMSG_GET_RETURN(pmsg);
	complete(&ctx->done);
}

/**
 * @brief submit a IPC msg to send and also wait to complete
 *
 * @param[in] pmsg : IPC msg
 *
 * @return : IPC msg handle result.
 */
int ipc_msg_submit_and_wait(struct ipc_msg *pmsg)
{
	struct __sync_ipcmsg_context ctx;
	int ret;

	init_completion(&ctx.done);

	pmsg->complete = __ipcmsg_blocking_completion;
	pmsg->context = &ctx;

	ret = ipc_msg_submit(pmsg);

	wait_for_completion(&ctx.done);

	return ctx.status;
}


/**
 * @brief after handle the IPC msg, call this function to complete
 *
 * @param[in] pmsg : IPC msg
 * @param[in] status : IPC msg handle result to return
 *
 * @return : 0 means success, <0 means fail.
 *
 */
int ipc_msg_giveback(struct ipc_msg *pmsg, int status)
{
	inc_ipcmsg_giveback_count();

	pmsg->result = status;
	IPCMSG_SET_RESPONSE(pmsg);

	return ipc_msg_submit(pmsg);
}

static void ipc_msg_dispatch(struct ipc_msg *pmsg)
{
	struct ipc_msg *pnext;

	while (pmsg != NULL) {
		// modify current ipcmsg's next pointer to virtual address
		update_ipcmsg_next_virt(pmsg);

		pnext = pmsg->next;

		pmsg->next = NULL;
		pmsg->next_phys = 0;

		// sanity check
		if (!IPCMSG_CHECK_MAGIC(pmsg)) {
			WARN(1, "recved ipcmsg %p invalid\n", pmsg);
			break;
		}

		if (IPCMSG_IS_RESPONSE(pmsg)) {
			inc_ipcmsg_complete_count();

			IPCMSG_COMPLETE(pmsg);

		} else {
			struct cmd_handler_entry *pcmdent;
			int ret = IPCMSG_RET_NOHANDLE;

			inc_ipcmsg_receive_count();

			if (pmsg->cmd < IPCMSG_MAX_CMD) {
				pcmdent = &handler_info_registered[pmsg->cmd];

				if (pcmdent->func) {
					// execute IPC msg handler function
					ret = pcmdent->func(pmsg, pcmdent->data);
				}
			}

			if (ret != IPCMSG_RET_PENDING) {
				ipc_msg_giveback(pmsg, ret);
			}
		}

		// set next msg as current
		pmsg = pnext;
	}

}


static int pre_ipc_msg_flush(struct ipc_msg * pmsg)
{
	update_ipcmsg_next_phys(pmsg);

	return 0;
}

static void ipc_msg_flush(void)
{
	struct ipc_msg *pmsg;
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&the_ipc->send_lock, flags);

	pmsg = tx_queue_get_first();

	if (pmsg != NULL) {
		iterator_ipcmsg_link(pmsg, pre_ipc_msg_flush);

		ret = mbox_send_msg(pmsg);
		if (likely(ret == 0)) {
			tx_queue_clear();
		}
	}

	spin_unlock_irqrestore(&the_ipc->send_lock, flags);
}

static void ipc_msg_handle_work(struct work_struct *work)
{
	struct ipc_msg *pmsg;

	while ((pmsg = mbox_get_msg()) != NULL) {

		ipc_msg_dispatch(pmsg);
	}
}

static int mbox_notifier(struct notifier_block *nb, unsigned long action, void *data)
{
	switch (action)
	{
	case RK_IPCMBOX_EVENT_MSG_INCOMING:
		// start a kthread to handle the incoming msg request
		queue_work(the_ipc->wq, &the_ipc->work);
		break;

	case RK_IPCMBOX_EVENT_MSG_WRITEABLE:
		// continue write IPC msg
		ipc_msg_flush();
		break;

	default:
		break;
	}

	return 0;
}


static struct notifier_block mbox_nb = {
	.notifier_call	= mbox_notifier,
};

static int ipc_mem_init(struct rk_ipc *ipc)
{
	void *ipcmsg_pool_base, *shmem_pool_base;
	size_t ipcmsg_pool_size, shmem_pool_size;
	struct ipc_msg_pool *ipcmsg_pool;
	struct ipc_shmem_pool *shmem_pool;

	printk(KERN_INFO "IPC memory :  [%x, %x)\n", \
			(u32)ipc->ipc_memphys, (u32)ipc->ipc_memphys + ipc->ipc_memsize);

	ipc->ipc_membase = ioremap(ipc->ipc_memphys, ipc->ipc_memsize);
	if (ipc->ipc_membase == NULL) {
		return -ENOMEM;
	}

	// the bottom half memory for our used
	// ipcmsg pool occupy 1/4
	ipcmsg_pool_base = ipc->ipc_membase;
	ipcmsg_pool_size = ((ipc->ipc_memsize >> 1) >> 2);

	// shmem pool occupy 3/4
	shmem_pool_base = (void*)((unsigned long)ipcmsg_pool_base + ipcmsg_pool_size);
	shmem_pool_size = (ipc->ipc_memsize >> 1) - ipcmsg_pool_size;

	// create IPC msg pool
	ipcmsg_pool = ipc_msg_pool_create(ipcmsg_pool_base, ipcmsg_pool_size);

	// create IPC shmem pool
	shmem_pool = ipc_shmem_pool_create(shmem_pool_base, shmem_pool_size);
	
	ipc->ipcmsgp = ipcmsg_pool;
	ipc->shmemp = shmem_pool;

	return 0;
}

/**
 * @brief : reset IPC
 *
 * @return : 0 means success, < 0 fail.
 *
 * @notes : baseband can be shutdown freely, in that case, we need reset ourself.
 */
int rk_ipc_reset(void)
{
	if (the_ipc && 
		the_ipc->mbox && 
		the_ipc->mbox->reset) {	
		the_ipc->mbox->reset(the_ipc->mbox);
	}

#ifdef CONFIG_DEBUG_FS
	memset((void*)&ipc_debug_statis, 0, sizeof(ipc_debug_statis));
#endif
	return 0;
}

/**
 * @brief : IPC system initialization
 *
 * @param[in] ipcmem_base : IPC share memory physical start address.
 * @param[in] ipcmem_sz : IPC share memory size.
 *
 * @return : if success return 0, otherwise return a negative number.
 */
int rk_ipc_init(phys_addr_t ipcmem_base, size_t ipcmem_sz)
{
	int ret;
	struct rk_ipc *ipc;
	struct ipc_mbox *mbox;

	if (ipcmem_sz <= 0) {
		return -EINVAL;
	}

	ipc = (struct rk_ipc*)kzalloc(sizeof(*ipc), GFP_KERNEL);
	if (ipc == NULL) {
		return -ENOMEM;
	}

	spin_lock_init(&ipc->send_lock);

	ipc->ipc_memphys = ipcmem_base;
	ipc->ipc_memsize = ipcmem_sz;

	mbox = rk_mbox_get("default", &mbox_nb); 
	if (mbox == NULL) {
		ret = -ENODEV;
		goto mbox_get_error;
	}

	ret = ipc_mem_init(ipc);
	if (ret) {
		ret = -ENOMEM;
		goto mem_init_error;
	}

	ipc_debugfs_init();

	ipc->wq = create_workqueue("rk_ipc");
	INIT_WORK(&ipc->work, ipc_msg_handle_work);

	ipc->mbox = mbox;
	ipc->txpendingQ = ipcmsg_queue_create();

	the_ipc = ipc;

	return 0;

mem_init_error:
	rk_mbox_put(mbox, &mbox_nb);

mbox_get_error:
	kfree(ipc);

	return ret;
}

EXPORT_SYMBOL_GPL(rk_ipc_init);

EXPORT_SYMBOL_GPL(ipc_msg_new);
EXPORT_SYMBOL_GPL(ipc_msg_free);
EXPORT_SYMBOL_GPL(ipc_msg_reinit);
EXPORT_SYMBOL_GPL(ipc_msg_submit);
EXPORT_SYMBOL_GPL(ipc_msg_submit_and_wait);
EXPORT_SYMBOL_GPL(ipc_msg_giveback);
EXPORT_SYMBOL_GPL(ipc_msg_register_handler);
EXPORT_SYMBOL_GPL(rk_ipc_reset);
EXPORT_SYMBOL_GPL(rk_ipc_mem_p2v);
EXPORT_SYMBOL_GPL(rk_ipc_mem_v2p);

