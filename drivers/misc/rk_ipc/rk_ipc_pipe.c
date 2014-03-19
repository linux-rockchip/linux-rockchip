/*
 * Rock-chips IPC PIPE
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
#include <linux/idr.h>

#include "rk_ipc.h"
#include "rk_ipcmsg_queue.h"
#include "rk_ipc_pipe.h"

struct ipc_pipe {
	int id;

	struct ipcmsg_queue *freeQ;
	struct ipcmsg_queue *recvQ;
	struct ipcmsg_queue *sendQ;

	int qset;		// current quantum set.
	int quantum;
	int qset_limit;

	wait_queue_head_t	write_wq;
	wait_queue_head_t	read_wq;
	unsigned long flags;
#define IPC_PIPE_WRITE_BLOCK		0x01
#define IPC_PIPE_READ_BLOCK			0x02

	int read_avail;

	struct blocking_notifier_head        notifier;

	u8 state;
#define IPC_PIPE_STATE_CLOSED		0x00
#define IPC_PIPE_STATE_CLOSING		0x01
#define IPC_PIPE_STATE_OPENED		0x02

	u8 sndbuf_no_enough;
	spinlock_t	lock;
};

#define DEFAULT_PIPE_QSET			(8)
#define DEFAULT_PIPE_QUANTUM		(512)

static DEFINE_IDR(ipc_pipe_idr);
static DECLARE_BITMAP(pipes_in_use, __IPC_PIPE_NUM_MAX);

static int save_opened_pipe(struct ipc_pipe *pipe)
{
	int id;
	int status;

__retry:
    if (idr_pre_get(&ipc_pipe_idr, GFP_KERNEL) == 0)
		return -ENOMEM;

	status = idr_get_new_above(&ipc_pipe_idr, pipe, pipe->id, &id);
	if (status == 0 && id != pipe->id) {
		status = -EBUSY;
		idr_remove(&ipc_pipe_idr, id);
	}

    if (status == -EAGAIN)
        goto __retry;

	return status;
}

static inline void switch_pipe_state(struct ipc_pipe *pipe, u8 state)
{
	pipe->state = state;
	IPC_DBG(DEBUG_INFO,"state=%d\n",state);
}

static inline int pipe_in_state(struct ipc_pipe *pipe, u8 state)
{
	return (pipe->state == state);
}

static inline int pipe_not_in_state(struct ipc_pipe *pipe, u8 state)
{
	return (pipe->state != state);
}

static inline int check_pipe_accessable(struct ipc_pipe *pipe)
{
	if (pipe == NULL || 
			pipe_not_in_state(pipe, IPC_PIPE_STATE_OPENED)) {
		return -1;
	}

	return 0;
}

static inline void mark_pipe_used(struct ipc_pipe *pipe)
{
	set_bit(pipe->id, pipes_in_use);
}

static inline void mark_pipe_unused(struct ipc_pipe *pipe)
{
	clear_bit(pipe->id, pipes_in_use);
}

static inline int all_pipes_unused(void)
{
	return (find_first_bit(pipes_in_use, __IPC_PIPE_NUM_MAX) >= __IPC_PIPE_NUM_MAX);
}

static int pipe_ipcmsg_handler(struct ipc_msg* pmsg, void *data)
{
	struct ipc_pipe *pipe;
	int pipe_id;
	unsigned long flags;
	//IPC_DBG(DEBUG_INFO,"%s pipe_id=%d\n",__func__,pmsg->subcmd);
	pipe_id = pmsg->subcmd;

	// pipe invalid
	if (pipe_id < 0 || pipe_id >= __IPC_PIPE_NUM_MAX)
		return -1;

	pipe = idr_find(&ipc_pipe_idr, pipe_id);

	if (check_pipe_accessable(pipe))
		return -1;
	//printk("%s %d\n",__func__,__LINE__);
	ipcmsg_queue_enqueue(pipe->recvQ, pmsg);
	spin_lock_irqsave(&pipe->lock, flags);
	pipe->read_avail += (int)IPCMSG_GET_ARG(pmsg, 1);
	spin_unlock_irqrestore(&pipe->lock, flags);

	if (pipe->flags & IPC_PIPE_READ_BLOCK)
		wake_up_interruptible(&pipe->read_wq);

	blocking_notifier_call_chain(&pipe->notifier, RK_IPCPIPE_EVENT_NEW_DATA, NULL);

	return IPCMSG_RET_PENDING;	// later, we should call ipc_msg_giveback() manually. 
}


/**
 * @brief open a IPC pipe.
 * 
 * @param[in] pipe_id : IPC pipe ID.
 *
 * @return  not NULL means success, NULL means failure.
 */
struct ipc_pipe* ipc_pipe_open(int pipe_id)
{
	struct ipc_pipe *pipe;
	int ret;
	int first;

	if (pipe_id < 0 || pipe_id >= __IPC_PIPE_NUM_MAX)
		return NULL;

	pipe = idr_find(&ipc_pipe_idr, pipe_id);
	if (pipe != NULL) {
		return NULL;
	}

	pipe = (struct ipc_pipe*)kzalloc(sizeof(*pipe), GFP_KERNEL);
	if (pipe == NULL)
		return NULL;

	pipe->id = pipe_id;
	pipe->freeQ = ipcmsg_queue_create();
	pipe->recvQ = ipcmsg_queue_create();
	pipe->sendQ = ipcmsg_queue_create();
	pipe->qset = 0;
	pipe->quantum = DEFAULT_PIPE_QUANTUM;
	pipe->qset_limit = DEFAULT_PIPE_QSET;

	spin_lock_init(&pipe->lock);
	
	BLOCKING_INIT_NOTIFIER_HEAD(&pipe->notifier);
	init_waitqueue_head(&pipe->write_wq);
	init_waitqueue_head(&pipe->read_wq);

	first = all_pipes_unused();

	mark_pipe_used(pipe);

	ret = save_opened_pipe(pipe);
	if (ret)
		goto _error;

	switch_pipe_state(pipe, IPC_PIPE_STATE_OPENED);

	if (first) {
		// we are the first opened pipe.
		ipc_msg_register_handler(IPCMSG_CMD_PIPE, pipe_ipcmsg_handler, NULL);
	}

	return pipe;

_error:
	mark_pipe_unused(pipe);

	kfree(pipe);

	return NULL;
}

static void free_pipe_resources(struct ipc_pipe *pipe)
{
	struct ipc_msg *pmsg;

	if (pipe->qset == 0) {
		// give back all received IPC Msgs
		while (NULL != (pmsg = ipcmsg_queue_dequeue(pipe->recvQ))) {
			ipc_msg_giveback(pmsg, 0);
		}

		switch_pipe_state(pipe, IPC_PIPE_STATE_CLOSED);

		idr_remove(&ipc_pipe_idr, pipe->id);
		kfree(pipe);
	}
}

/**
 * @brief close a IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 *
 * @return  0 means success, < 0 means failure.
 */
int ipc_pipe_close(struct ipc_pipe* pipe)
{
	struct ipc_msg *pmsg;

	if (check_pipe_accessable(pipe))
		return -1;

	switch_pipe_state(pipe, IPC_PIPE_STATE_CLOSING);

	mark_pipe_unused(pipe);

	if (all_pipes_unused()) {
		ipc_msg_register_handler(IPCMSG_CMD_PIPE, NULL, NULL);
	}

	while (NULL != (pmsg = ipcmsg_queue_dequeue(pipe->freeQ))) {
		ipc_msg_free(pmsg);
		pipe->qset--;
	}
	printk("======%s %d\n",__func__,pipe->qset);
	free_pipe_resources(pipe);

	return 0;
}

static int pipe_sndbuf_prealloc(struct ipc_pipe *pipe)
{
	int ret = 0;
	struct ipc_msg *pmsg;

	while (pipe->qset < pipe->qset_limit) {
		ret = ipc_msg_new(&pmsg, pipe->quantum);
		if (ret)
			break;

		ipcmsg_queue_enqueue(pipe->freeQ, pmsg);
		pipe->qset++;
	}

	return ret;
}

/**
 * @brief configure a IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 * @param[in] cfg_cmd : the command of configuration.
 * @param[in] val : configure value.
 *
 * @return  0 means success, < 0 means failure.
 */
int ipc_pipe_config(struct ipc_pipe* pipe, int cfg_cmd, unsigned long val)
{
	switch (cfg_cmd)
	{
	case RK_IPC_PIPE_CFG_SNDBUF_QUANUM_SET:
		pipe->quantum = val;
		break;

	case RK_IPC_PIPE_CFG_SNDBUF_QUANUM_GET:
		if (val != 0) {
			*((u32*)val) = pipe->quantum;
		}
		break;

	case RK_IPC_PIPE_CFG_SNDBUF_QSET_SET:
		pipe->qset_limit = val;
		break;

	case RK_IPC_PIPE_CFG_SNDBUF_QSET_GET:
		if (val != 0) {
			*((u32*)val) = pipe->qset_limit;
		}
		break;

	case RK_IPC_PIPE_CFG_SNDBUF_PREALLOC:
		return pipe_sndbuf_prealloc(pipe);

	case RK_IPC_PIPE_CFG_WRITE_BLOCK_SET:
		if (val)
			pipe->flags |= IPC_PIPE_WRITE_BLOCK;
		else
			pipe->flags &= ~IPC_PIPE_WRITE_BLOCK;
		break;

	case RK_IPC_PIPE_CFG_READ_BLOCK_SET:
		if (val)
			pipe->flags |= IPC_PIPE_READ_BLOCK;
		else
			pipe->flags &= ~IPC_PIPE_READ_BLOCK;
		break;

	default:
		return -1;
	}

	return 0;
}

static struct ipc_msg *pipe_free_ipcmsg_get(struct ipc_pipe *pipe)
{
	struct ipc_msg *pmsg;
	int ret;
	//IPC_DBG(DEBUG_INFO,"%s free count=%d\n",__func__,ipcmsg_queue_count(pipe->freeQ));
	pmsg = ipcmsg_queue_dequeue(pipe->freeQ);
	//if(pmsg==NULL)
	//IPC_DBG(DEBUG_INFO,"pipe_free_ipcmsg_get , pmsg==NULL\n ");
	if (pmsg == NULL && pipe->qset < pipe->qset_limit) {
		ret = ipc_msg_new(&pmsg, pipe->quantum);
		if (ret)
			return NULL;

		pipe->qset++;
	}

	return pmsg;
}

static void pipe_ipcmsg_complete(struct ipc_msg* pmsg)
{
	struct ipc_pipe *pipe;

	pipe = (struct ipc_pipe*)pmsg->context;

	if (pipe_in_state(pipe, IPC_PIPE_STATE_CLOSING)) {
		ipc_msg_free(pmsg);
		pipe->qset--;
		printk("pipe_ipcmsg_complete CLOSING pipe->qset=%d\n",pipe->qset);
		free_pipe_resources(pipe);
		return;
	}

	if (pmsg->shmem_len != pipe->quantum || 
			pipe->qset > pipe->qset_limit) {	// pipe quantum or qset may be changed
		ipc_msg_free(pmsg);
		pipe->qset--;

	} else {		// recycle the msg
		ipc_msg_reinit(pmsg);
		ipcmsg_queue_enqueue(pipe->freeQ, pmsg);
		//IPC_DBG(DEBUG_INFO,"%s free count=%d\n",__func__,ipcmsg_queue_count(pipe->freeQ));
		if (pipe->sndbuf_no_enough) {
			//IPC_DBG(DEBUG_INFO,"pipe->sndbuf_no_enough  %s free count=%d\n",__func__,ipcmsg_queue_count(pipe->freeQ));
			pipe->sndbuf_no_enough = 0;
			blocking_notifier_call_chain(&pipe->notifier, RK_IPCPIPE_EVENT_MORE_SPACE, NULL);
		}
	}

	if (pipe->flags & IPC_PIPE_WRITE_BLOCK)
		wake_up_interruptible(&pipe->write_wq);
}

/**
 * @brief write data to a IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 * @param[in] pbuf : data buffer.
 * @param[in] len : data length.
 *
 * @return  >= 0 means actual sent data length, < 0 means failure.
 *
 * @notes: default this is nonblock function, but you can change to block mode by
 *   setting the pipe use RK_IPC_PIPE_CFG_WRITE_BLOCK_SET configuration.
 */
int ipc_pipe_write(struct ipc_pipe *pipe, const void* pbuf, int len)
{
	DECLARE_WAITQUEUE(wait, current);
	struct ipc_msg *pmsg, *head;
	int actual = 0;

	if (check_pipe_accessable(pipe)){
		printk("%s %d\n",__func__,__LINE__);
		return -1;
	}
_write_process:

	while (len) {
		int count;

		pmsg = pipe_free_ipcmsg_get(pipe);
		if (pmsg == NULL) {
			break;
		}


		count = min_t(int, len, pmsg->shmem_len);

		memcpy(pmsg->shmem, pbuf, count);
			
		IPCMSG_SET_CMD(pmsg, IPCMSG_CMD_PIPE, pipe->id);
		IPCMSG_SET_ARG(pmsg, 0, (void*)pmsg->shmem_phys);
		IPCMSG_SET_ARG(pmsg, 1, (void*)count);
		IPCMSG_SET_ARGC(pmsg, 2);

		IPCMSG_SET_COMPLETE(pmsg, pipe_ipcmsg_complete, (void*)pipe);

		ipcmsg_queue_enqueue(pipe->sendQ, pmsg);

		// update buffer and length
		pbuf += count;
		len -= count;

		actual += count;
	}

	head = ipcmsg_queue_peek(pipe->sendQ);
	if (head != NULL) {
		ipcmsg_queue_clear(pipe->sendQ);
		ipc_msg_submit(head);
	}

	if (len > 0) {
		pipe->sndbuf_no_enough = 1;

		if (pipe->flags & IPC_PIPE_WRITE_BLOCK) {
			prepare_to_wait(&pipe->write_wq, &wait, TASK_INTERRUPTIBLE);
			schedule();
			finish_wait(&pipe->write_wq, &wait);

			if (!signal_pending(current)) {
				goto _write_process;
			}
		}
	}


	return actual;
}

/**
 * @brief read data from a IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 * @param[in] pbuf : data buffer.
 * @param[in] len : data length.
 *
 * @return  >= 0 means actual read data length, < 0 means failure.
 *
 * @notes: default this is nonblock function, but you can change to block mode by
 *   setting the pipe use RK_IPC_PIPE_CFG_READ_BLOCK_SET configuration.
 */
int ipc_pipe_read(struct ipc_pipe *pipe, void* pbuf, int len)
{
	DECLARE_WAITQUEUE(wait, current);
	struct ipc_msg *pmsg;
	unsigned long flags;
	int actual = 0;

	if (check_pipe_accessable(pipe)){
			printk("%s %d\n",__func__,__LINE__);
			return -1;
		}

_read_process:

	while (len) {
		int count;
		void *recv_pdata;
		int recv_len;

		pmsg = ipcmsg_queue_peek(pipe->recvQ);
		if (pmsg == NULL){
			printk("pipe->recvQ is null but msg len is valid,pipe->recvQ=%d\n",ipcmsg_queue_count(pipe->recvQ));
			break;
		}
		recv_pdata = (void*)rk_ipc_mem_p2v(IPCMSG_GET_ARG(pmsg, 0));
		recv_len = (int)IPCMSG_GET_ARG(pmsg, 1);

		count = min_t(int, len, recv_len);

		memcpy(pbuf, recv_pdata, count);
		len -= count;
		pbuf += count;
		actual += count;

		if (count == recv_len) {
			// remove msg from queue
			pmsg = ipcmsg_queue_dequeue(pipe->recvQ);

			// send the IPC msg back manually
			ipc_msg_giveback(pmsg, 0);
		} else {
			IPCMSG_SET_ARG(pmsg, 0, (void*)((char*)recv_pdata + count));
			IPCMSG_SET_ARG(pmsg, 1, (void*)(recv_len - count));
		}
	}

	if (len > 0 && (pipe->flags & IPC_PIPE_READ_BLOCK)) {
		prepare_to_wait(&pipe->read_wq, &wait, TASK_INTERRUPTIBLE);
		schedule();
		finish_wait(&pipe->read_wq, &wait);

		if (!signal_pending(current)) {
			goto _read_process;
		}
	}
	spin_lock_irqsave(&pipe->lock, flags);
	pipe->read_avail -= actual;
	//printk("pipe->read_avail=%d actual=%d\n",pipe->read_avail,actual);
	spin_unlock_irqrestore(&pipe->lock, flags);
	return actual;
}

/**
 * @brief get IPC pipe read available bytes.
 * 
 * @param[in] pipe : IPC pipe.
 *
 * @return  available bytes
 */
int ipc_pipe_read_avail(struct ipc_pipe *pipe)
{
	if (check_pipe_accessable(pipe))
		return 0;

	return pipe->read_avail;
}

/**
 * @brief get IPC pipe write room bytes.
 * 
 * @param[in] pipe : IPC pipe.
 *
 * @return  available write bytes
 */
int ipc_pipe_write_room(struct ipc_pipe *pipe)
{
	if (check_pipe_accessable(pipe))
		return 0;

	pipe_sndbuf_prealloc(pipe);

	return ipcmsg_queue_count(pipe->freeQ) * pipe->quantum;
}

/**
 * @brief register a notifier block to IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 * @param[in] nb : notifier block, when IPC pipe readable, call this notifier.
 *
 * @return  >= 0 means actual read data length, < 0 means failure.
 */
int ipc_pipe_notifier_register(struct ipc_pipe *pipe, struct notifier_block *nb)
{
	if (check_pipe_accessable(pipe))
		return -1;

	if (nb) {
		blocking_notifier_chain_register(&pipe->notifier, nb);

		if (ipcmsg_queue_count(pipe->recvQ)>0){
			printk("have new data to read,recvQ count=%d\n",ipcmsg_queue_count(pipe->recvQ));
			blocking_notifier_call_chain(&pipe->notifier, RK_IPCPIPE_EVENT_NEW_DATA, NULL);
		}
	}

	return 0;
}

EXPORT_SYMBOL_GPL(ipc_pipe_open);
EXPORT_SYMBOL_GPL(ipc_pipe_close);
EXPORT_SYMBOL_GPL(ipc_pipe_write);
EXPORT_SYMBOL_GPL(ipc_pipe_read);
EXPORT_SYMBOL_GPL(ipc_pipe_config);
EXPORT_SYMBOL_GPL(ipc_pipe_read_avail);
EXPORT_SYMBOL_GPL(ipc_pipe_write_room);
EXPORT_SYMBOL_GPL(ipc_pipe_notifier_register);
