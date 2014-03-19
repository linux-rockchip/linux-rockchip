/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __RK_IPCMSG_QUEUE_H
#define __RK_IPCMSG_QUEUE_H

struct ipcmsg_queue;

/**
 * @brief create a IPC msg queue.
 *
 * @param	none
 *
 * @return : not NULL means success, NULL means fail.
 */
extern struct ipcmsg_queue* ipcmsg_queue_create(void);

/**
 * @brief destroy a IPC msg queue.
 *
 * @param[in] queue : IPC msg queue to destroy.
 *
 * @return void
 */
extern void ipcmsg_queue_destroy(struct ipcmsg_queue* queue);

/**
 * @brief add a IPC msg into the tail of queue.
 *
 * @param[in] queue : IPC msg queue.
 * @param[in] pmsg : IPC msg.
 *
 * @return : 0 means success, < 0 means fail.
 */
extern int ipcmsg_queue_enqueue(struct ipcmsg_queue *queue, struct ipc_msg *pmsg);

/**
 * @brief remove a IPC msg from the begining of queue and return.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : not NULL means success, NULL means fail.
 */
extern struct ipc_msg* ipcmsg_queue_dequeue(struct ipcmsg_queue *queue);

/**
 * @brief get a IPC msg from the begining of queue, not remove from queue.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : not NULL means success, NULL means fail.
 */
extern struct ipc_msg* ipcmsg_queue_peek(struct ipcmsg_queue *queue);

/**
 * @brief get a IPC msg from the end of queue, not remove from queue.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : not NULL means success, NULL means fail.
 */
extern struct ipc_msg* ipcmsg_queue_peek_last(struct ipcmsg_queue *queue);

/**
 * @brief empty a IPC msg queue.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : 0 means success, < 0 means fail.
 */
extern int ipcmsg_queue_clear(struct ipcmsg_queue *queue);

/**
 * @brief query if queue is empty.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : 1 means queue empty, 0 means queue not empty.
 */
extern int ipcmsg_queue_isempty(struct ipcmsg_queue *queue);

/**
 * @brief query queue element counter.
 *
 * @param[in] queue : IPC msg queue.
 *
 * @return : ipmsg count in queue.
 */
extern int ipcmsg_queue_count(struct ipcmsg_queue *queue);

/**
 * @brief for each IPC msg in queue to call @fn function.
 *
 * @param[in] queue : IPC msg queue.
 * @param[in] data : private data pass to @fn.
 * @param[in] fn : callback function
 *
 * @return : 0 means success, < 0 means fail.
 */
extern int ipcmsg_queue_for_each(struct ipcmsg_queue* queue, void *data, 
							int (*fn)(struct ipc_msg *, void*));

#endif	/* __RK_IPCMSG_QUEUE_H */
