/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __RK_IPC_PIPE_H
#define __RK_IPC_PIPE_H


struct ipc_pipe;


/* notifier block event */
#define RK_IPCPIPE_EVENT_NEW_DATA	0x00
#define RK_IPCPIPE_EVENT_MORE_SPACE	0x01

/* used for ipc_pipe_config cmd */
enum {
	RK_IPC_PIPE_CFG_SNDBUF_QUANUM_SET = 0,		// Send Buffer Block Size in bytes
	RK_IPC_PIPE_CFG_SNDBUF_QUANUM_GET,
	RK_IPC_PIPE_CFG_SNDBUF_QSET_SET,			// Send Buffer Block Count
	RK_IPC_PIPE_CFG_SNDBUF_QSET_GET,
	RK_IPC_PIPE_CFG_SNDBUF_PREALLOC,
	RK_IPC_PIPE_CFG_WRITE_BLOCK_SET,			// set pipe write block mode or unblock mode
	RK_IPC_PIPE_CFG_READ_BLOCK_SET,				// set pipe read block mode or unblock mode

	RK_IPC_PIPE_CFG_MAX
};

/**
 * @brief open a IPC pipe.
 * 
 * @param[in] pipe_id : IPC pipe ID.
 *
 * @return  not NULL means success, NULL means failure.
 */
extern struct ipc_pipe* ipc_pipe_open(int pipe_id);

/**
 * @brief close a IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 *
 * @return  0 means success, < 0 means failure.
 */
extern int ipc_pipe_close(struct ipc_pipe* pipe);

/**
 * @brief configure a IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 * @param[in] cfg_cmd : the command of configuration.
 * @param[in] val : configure value.
 *
 * @return  0 means success, < 0 means failure.
 */
extern int ipc_pipe_config(struct ipc_pipe* pipe, int cfg_cmd, unsigned long val);

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
extern int ipc_pipe_write(struct ipc_pipe *pipe, const void* pbuf, int len);

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
extern int ipc_pipe_read(struct ipc_pipe *pipe, void* pbuf, int len);

/**
 * @brief get IPC pipe read available bytes.
 * 
 * @param[in] pipe : IPC pipe.
 *
 * @return  available bytes
 */
extern int ipc_pipe_read_avail(struct ipc_pipe *pipe);

/**
 * @brief get IPC pipe write room bytes.
 * 
 * @param[in] pipe : IPC pipe.
 *
 * @return  available write bytes
 */
extern int ipc_pipe_write_room(struct ipc_pipe *pipe);

/**
 * @brief register a notifier block to IPC pipe.
 * 
 * @param[in] pipe : IPC pipe.
 * @param[in] nb : notifier block, when IPC pipe readable, call this notifier.
 *
 * @return  >= 0 means actual read data length, < 0 means failure.
 */
extern int ipc_pipe_notifier_register(struct ipc_pipe *pipe, struct notifier_block *nb);

#endif	/* __RK_IPC_PIPE_H */
