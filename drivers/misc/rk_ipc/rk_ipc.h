/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __RK_IPC_H
#define __RK_IPC_H

#include "rk_ipc_comn.h"


#define DEBUG_INFO	0x01
#define DEBUG_WARN	0x02
#define DEBUG_ERR	0x04

static int ipc_debug_level = DEBUG_INFO;

#define IPC_DBG(level, fmt,...)		\
	do {							\
		if (level >= ipc_debug_level)			\
			pr_info("ipc dbg: " fmt, ## __VA_ARGS__);	\
	} while (0)


#define IPCMSG_RET_NOHANDLE		__IPCMSG_RET_NOHANDLE

/* 
 * reserved return value for cmd handler function (ipc_msg_handler_t).
 * when cmd handler function need keep the ipc_msg and defer to handle,
 * it may return IPCMSG_RET_PENDING.
 * when ipc_msg has been handled later, this ipc_msg should be given back
 * by call ipc_msg_giveback() manually.
 */
#define IPCMSG_RET_PENDING	(-0x1001)


/*
 * supported command ID : [0, IPCMSG_MAX_CMD).
 */
#define IPCMSG_MAX_CMD			__IPCMSG_CMD_ID_MAX
#define IPCMSG_CMD_PIPE			__IPCMSG_CMD_ID_PIPE
#define	IPCMSG_CMD_HARDWARE		__IPCMSG_CMD_ID_HARDWARE
#define IPCMSG_CMD_FS			__IPCMSG_CMD_ID_FS

struct ipc_msg;

typedef void (*ipc_msg_complete_t)(struct ipc_msg*);
typedef int (*ipc_msg_handler_t)(struct ipc_msg*, void *);


/* IPC msg : the envelop of IPC */
struct ipc_msg __STRUCT_IPC_MSG_AP(struct ipc_msg, unsigned int, ipc_msg_complete_t);

/* IPC msg helper functions */
static inline void IPCMSG_SET_MAGIC(struct ipc_msg *pmsg)
{
	__IPCMSG_SET_MAGIC(pmsg);
}

static inline int IPCMSG_CHECK_MAGIC(struct ipc_msg *pmsg)
{
	return __IPCMSG_CHECK_MAGIC(pmsg);
}

static inline void IPCMSG_SET_CMD(struct ipc_msg *pmsg, u32 cmd, u32 subcmd)
{
	__IPCMSG_SET_CMD(pmsg, cmd, subcmd);
}

static inline void IPCMSG_SET_ARG(struct ipc_msg *pmsg, int index, void *arg)
{
	__IPCMSG_SET_ARG(pmsg, index, arg);
}

static inline void* IPCMSG_GET_ARG(struct ipc_msg *pmsg, int index)
{
	return __IPCMSG_GET_ARG(pmsg, index);
}

static inline void IPCMSG_SET_ARGC(struct ipc_msg *pmsg, int argc)
{
	__IPCMSG_SET_ARGC(pmsg, argc);
}

static inline int IPCMSG_GET_ARGC(struct ipc_msg *pmsg)
{
	return __IPCMSG_GET_ARGC(pmsg);
}

static inline void IPCMSG_PUSH_ARG(struct ipc_msg *pmsg, void *arg)
{
	__IPCMSG_PUSH_ARG(pmsg, arg);
}

static inline void* IPCMSG_POP_ARG(struct ipc_msg *pmsg)
{
	return __IPCMSG_POP_ARG(pmsg);
}

static inline int IPCMSG_GET_RETURN(struct ipc_msg *pmsg)
{
	return __IPCMSG_GET_RETURN(pmsg);
}

static inline void IPCMSG_SET_COMPLETE(struct ipc_msg *pmsg, 
										ipc_msg_complete_t complete_fn,
										void *data)
{
	__IPCMSG_SET_COMPLETE(pmsg, complete_fn, data);
}

static inline void IPCMSG_EMPTY_ARG(struct ipc_msg *pmsg)
{
	__IPCMSG_EMPTY_ARG(pmsg);
}

static inline int IPCMSG_IS_RESPONSE(struct ipc_msg* pmsg)
{
	return __IPCMSG_IS_RESPONSE(pmsg);
}

static inline void IPCMSG_SET_RESPONSE(struct ipc_msg *pmsg)
{
	__IPCMSG_SET_RESPONSE(pmsg);
}

static inline void IPCMSG_COMPLETE(struct ipc_msg *pmsg)
{
	if (pmsg->complete != NULL) {
		pmsg->complete(pmsg);
	}
}
static inline void IPCMSG_SET_PRIVATE(struct ipc_msg *pmsg, void *data)
{
	__IPCMSG_SET_PRIVATE(pmsg, data);
}

static inline void* IPCMSG_GET_PRIVATE(struct ipc_msg *pmsg)
{
	return __IPCMSG_GET_PRIVATE(pmsg);
}

/* IPC interfaces */

/**
 * @brief allocate a IPC msg, and also allocate more share memory optional.
 *
 * @param[in][out] ppmsg : pointer to allocated IPC msg.
 * @param[in] shmem_len : extra share memory length to allocate.
 *
 * @return : 0 means success, -1 means fail.
 */
extern int ipc_msg_new(struct ipc_msg **ppmsg, int shmem_len);

/**
 * @brief free the memory of mailbox request block
 *
 * @param[in] pmsg : the mailbox requet block to be free
 *
 * @return : void
 */
extern int ipc_msg_free(struct ipc_msg *pmsg);

/**
 * @brief re-init a IPC msg for recycle used
 *
 * @param[in] pmsg : IPC msg.
 *
 * @return : 0 means success, -1 means fail.
 */
extern int ipc_msg_reinit(struct ipc_msg *pmsg);

/**
 * @brief submit a IPC msg to send
 *
 * @param[in] pmsg : IPC msg
 *
 * @return : 0 means success, <0 means fail.
 */
extern int ipc_msg_submit(struct ipc_msg *pmsg);

/**
 * @brief submit a IPC msg to send and also wait to complete
 *
 * @param[in] pmsg : IPC msg
 *
 * @return : IPC msg handle result.
 */
extern int ipc_msg_submit_and_wait(struct ipc_msg *pmsg);

/**
 * @brief after handle the IPC msg, call this function to complete
 *
 * @param[in] pmsg : IPC msg
 * @param[in] status : IPC msg handle result to return
 *
 * @return : 0 means success, <0 means fail.
 *
 */
extern int ipc_msg_giveback(struct ipc_msg *pmsg, int status);

/**
 * @brief : register a IPC msg command handler
 *
 * @param[in] cmd: the command ID to register.
 * @param[in] func: registered command handle function.
 * @param[in] data : private data pass to command handle function when execute.
 *
 * @return : 0 means success, < 0 fail.
 */
extern int ipc_msg_register_handler(u32 cmd, ipc_msg_handler_t func, void *data);

/**
 * @brief : reset IPC
 *
 * @return : 0 means success, < 0 fail.
 *
 * @notes : baseband can be shutdown freely, in that case, we need reset ourself.
 */
extern int rk_ipc_reset(void);

/**
 * @brief : convert IPC memory physical address to virtual.
 *
 * @param[in] paddr: physical address of IPC memory.
 *
 * @return : converted virtual address
 */
extern void* rk_ipc_mem_p2v(phys_addr_t paddr);

/**
 * @brief : convert IPC memory virtual address to physical.
 *
 * @param[in] vaddr: virtual address of IPC memory.
 *
 * @return : converted physical address
 */
extern phys_addr_t rk_ipc_mem_v2p(void *vaddr);

/**
 * @brief : IPC system initialization
 *
 * @param[in] ipcmem_base : IPC share memory physical start address.
 * @param[in] ipcmem_sz : IPC share memory size.
 *
 * @return : if success return 0, otherwise return a negative number.
 */
extern int rk_ipc_init(phys_addr_t ipcmem_base, size_t ipcmem_sz);

#endif
