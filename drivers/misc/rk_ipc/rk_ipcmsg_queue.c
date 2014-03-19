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
#include <linux/spinlock.h>
#include <linux/slab.h>

#include "rk_ipc.h"
#include "rk_ipcmsg_queue.h"

struct ipcmsg_queue {
	struct ipc_msg *phead;
	struct ipc_msg **pplast;
	int count;

	spinlock_t	lock;
};


/**
 * @brief create a IPC msg queue.
 *
 * @param	none
 *
 * @return : not NULL means success, NULL means fail.
 */
struct ipcmsg_queue* ipcmsg_queue_create(void)
{
	struct ipcmsg_queue *queue;

	queue = (struct ipcmsg_queue*)kmalloc(sizeof(*queue), GFP_KERNEL);
	if (queue) {
		queue->phead = NULL;
		queue->pplast = &queue->phead;
		queue->count = 0;
		spin_lock_init(&queue->lock);
	}

	return queue;
}

/**
 * @brief destroy a IPC msg queue.
 *
 * @param[in] queue : IPC msg queue to destroy.
 *
 * @return void
 */
void ipcmsg_queue_destroy(struct ipcmsg_queue* queue)
{
	kfree(queue);
}

/**
 * @brief add a IPC msg into the tail of queue.
 *
 * @param[in] queue : IPC msg queue.
 * @param[in] pmsg : IPC msg.
 *
 * @return : 0 means success, < 0 means fail.
 */
int ipcmsg_queue_enqueue(struct ipcmsg_queue *queue, struct ipc_msg *pmsg)
{
	unsigned long flags;
	BUG_ON(pmsg==NULL);
	spin_lock_irqsave(&queue->lock, flags);
	*(queue->pplast) = pmsg;
	
	while (pmsg->next != NULL) {
		pmsg = pmsg->next;
		queue->count++;
	}
	
	queue->pplast = &(pmsg->next);
	queue->count++;
	
	spin_unlock_irqrestore(&queue->lock, flags);

	return 0;
}

/**
 * @brief remove a IPC msg from the begining of queue and return.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : not NULL means success, NULL means fail.
 */
struct ipc_msg* ipcmsg_queue_dequeue(struct ipcmsg_queue *queue)
{
	struct ipc_msg *pmsg = NULL;
	unsigned long flags;

	spin_lock_irqsave(&queue->lock, flags);

	if (queue->count > 0) {
		pmsg = queue->phead;
		BUG_ON(pmsg==NULL);
		queue->phead = pmsg->next;
		if (queue->phead == NULL) {
			queue->pplast = &(queue->phead);
		}
		
		pmsg->next = NULL;
		
		queue->count--;
	}

	spin_unlock_irqrestore(&queue->lock, flags);

	return pmsg;
}

/**
 * @brief get a IPC msg from the begining of queue, not remove from queue.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : not NULL means success, NULL means fail.
 */
struct ipc_msg* ipcmsg_queue_peek(struct ipcmsg_queue *queue)
{
	struct ipc_msg *pmsg = NULL;
	unsigned long flags;

	spin_lock_irqsave(&queue->lock, flags);

	if (queue->count > 0) {
		pmsg = queue->phead;
		BUG_ON(pmsg==NULL);
	}

	spin_unlock_irqrestore(&queue->lock, flags);

	return pmsg;
}

/**
 * @brief get a IPC msg from the end of queue, not remove from queue.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : not NULL means success, NULL means fail.
 */
struct ipc_msg* ipcmsg_queue_peek_last(struct ipcmsg_queue *queue)
{
	struct ipc_msg *pmsg = NULL;
	unsigned long flags;

	spin_lock_irqsave(&queue->lock, flags);

	if (queue->count > 0) {
		pmsg = container_of(queue->pplast, struct ipc_msg, next);
		BUG_ON(pmsg==NULL);
	}

	spin_unlock_irqrestore(&queue->lock, flags);

	return pmsg;

}

/**
 * @brief query if queue is empty.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : 1 means queue empty, 0 means queue not empty.
 */
int ipcmsg_queue_isempty(struct ipcmsg_queue *queue)
{
	return ipcmsg_queue_peek(queue) == NULL;
}

/**
 * @brief query queue element counter.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : ipmsg count in queue.
 */
int ipcmsg_queue_count(struct ipcmsg_queue *queue)
{
	return queue->count;
}

/**
 * @brief empty a IPC msg queue.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : 0 means success, < 0 means fail.
 */
int ipcmsg_queue_clear(struct ipcmsg_queue *queue)
{
	unsigned long flags;

	spin_lock_irqsave(&queue->lock, flags);

	queue->phead = NULL;
	queue->pplast = &queue->phead;
	queue->count = 0;

	spin_unlock_irqrestore(&queue->lock, flags);

	return 0;
}

/**
 * @brief for each IPC msg in queue to call @fn function.
 *
 * @param[in] queue : IPC msg queue.
 * @param[in] data : private data pass to @fn.
 * @param[in] fn : callback function
 *
 * @return : 0 means success, < 0 means fail.
 */
int ipcmsg_queue_for_each(struct ipcmsg_queue* queue, void *data, 
							int (*fn)(struct ipc_msg *, void*))
{
	int i;
	struct ipc_msg *pmsg;
	int ret = 0;
	unsigned long flags;


	if (fn == NULL)
		return -1;

	spin_lock_irqsave(&queue->lock, flags);

	pmsg = queue->phead;

	for (i=0; i<queue->count; i++) {
		ret = fn(pmsg, data);
		if (ret)
			break;

		pmsg = pmsg->next;
	}

	spin_unlock_irqrestore(&queue->lock, flags);

	return ret;
}
