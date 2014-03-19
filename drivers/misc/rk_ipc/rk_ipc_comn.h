/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * Rock-chips Inter-Processor Communication common defintion
 *
 * Modify Date : 2013/12/27
 *
 */
#ifndef __RK_IPC_COMN_H__
#define __RK_IPC_COMN_H__

/* IPC Msg magic value, used to identfy a valid  IPC Msg. */
#define __IPCMSG_MAGIC				(0x43504952)	/* ASCII 'R', 'I', 'P' 'C' */

/* IPC Msg response flag - set in IPC Msg.cmd */
#define __IPCMSG_RESPONSE_FLAG		(0x80000000)

/* the max argument count that IPC Msg can carry on. */
#define __IPCMSG_MAX_ARG_NUM	(8)

/*
 * reserved return value for IPC Msg.result, which
 * indicates that IPC Msg has no handle by remote agent.
 */
#define __IPCMSG_RET_NOHANDLE	(-0x1000)

/* 
 * supported IPC Msg Command ID
 */
enum
{
	__IPCMSG_CMD_ID_PIPE = 0,
	__IPCMSG_CMD_ID_HARDWARE,
	__IPCMSG_CMD_ID_FS,

	__IPCMSG_CMD_ID_MAX
};

/*
 * Sub-Command of PIPE.
 *
 * note : Sub-Command id of cmd pipe equal to IPC Pipe ID.
 */
#define __IPC_PIPE_NUM_MAX		(16)
#define __IPC_PIPE_ID(id)		(id)

enum
{
	/* pipe usage allocation */
	__IPC_PIPE_NV = __IPC_PIPE_ID(0),
	__IPC_PIPE_LOG,
	__IPC_PIPE_AT0,
	__IPC_PIPE_GPRS0,
	__IPC_PIPE_VOC0,
	__IPC_PIPE_AT1,
	__IPC_PIPE_GPRS1,
	__IPC_PIPE_VOC1
};


/*
 * Sub-Command of HARDWARE
 */
#define __IPCMSG_SUBCMD_ID_HW_GPIO_SET_OUTPUT			(0x00)
#define __IPCMSG_SUBCMD_ID_HW_GPIO_SET_INPUT			(0x01)
#define __IPCMSG_SUBCMD_ID_HW_GPIO_SET_VALUE			(0x02)
#define __IPCMSG_SUBCMD_ID_HW_GPIO_GET_VALUE			(0x03)
#define __IPCMSG_SUBCMD_ID_HW_GPIO_SET_DEBOUNCE			(0x04)

#define __IPCMSG_SUBCMD_ID_HW_SIM_VCC_SELECT			(0x10)

#define __IPCMSG_SUBCMD_ID_HW_BB_SYSTEM_HALT			(0x20)
#define __IPCMSG_SUBCMD_ID_HW_BB_SYSTEM_REBOOT			(0x21)


/*
 * Sub-Command of FS
 */
#define __IPCMSG_SUBCMD_ID_FS_READFILE				(0x00)
#define __IPCMSG_SUBCMD_ID_FS_WRITEFILE				(0x01)


/*
 * IPC Msg Structure
 *
 * Notes:
 *
 * for AP define IPC Msg structure use __STRUCT_IPC_MSG_AP, e.g.
 *	struct ipc_msg __STRUCT_IPC_MSG_AP(struct ipc_msg, u32, ipc_msg_complete_t);
 *
 * for BB define IPC Msg structure use __STRUCT_IPC_MSG_BB, e.g.
 *	struct IPCMsg __STRUCT_IPC_MSG_BB(struct IPCMsg, uint32, IPCMsgCompleteFunc_t, IPCMsgWaitFunc_t);
 *
 */
#define __IPC_MSG_ELEMS_COMN1(u32type)								\
	u32type	magic;		/* should be __IPCMSG_MAGIC */				\
	u32type	cmd;		/* [0, __IPCMSG_CMD_ID_MAX), the bit31 is response bit */				\
	u32type	subcmd;		/* deside by specific cmd */				\
																	\
	void *	argv[__IPCMSG_MAX_ARG_NUM];								\
	int		argc;													\
	int		result		/* remote execute result */

#define __IPC_MSG_ELEMS_COMN2(msg_comp_type)						\
	void *	priv;		/* private data for upper user */			\
	msg_comp_type	complete;										\
	void *	context

#define __STRUCT_IPC_MSG_BB(selftype, u32type, msg_comp_type, msg_wait_type)		\
{	\
	__IPC_MSG_ELEMS_COMN1(u32type);					\
	u32type		next_noused;							\
	selftype *	next;									\
	u32type		shmem_noused;							\
	void *		shmem;									\
	int			shmem_len;								\
	__IPC_MSG_ELEMS_COMN2(msg_comp_type);				\
	msg_wait_type	wait;								\
	void *		wait_obj;								\
}

#define __STRUCT_IPC_MSG_AP(selftype, u32type, msg_comp_type)		\
{	\
	__IPC_MSG_ELEMS_COMN1(u32type);		\
	selftype *	next;									\
	u32type		next_phys;								\
	void *		shmem;									\
	u32type		shmem_phys;								\
	int			shmem_len;								\
	__IPC_MSG_ELEMS_COMN2(msg_comp_type);				\
}


/* 
 * IPC Msg helper 
 */
#define __IPCMSG_SET_MAGIC(pmsg)	\
	do { pmsg->magic = __IPCMSG_MAGIC; } while(0)

#define __IPCMSG_CHECK_MAGIC(pmsg)	(__IPCMSG_MAGIC == pmsg->magic)

#define __IPCMSG_SET_CMD(pmsg, cmd, subcmd)		\
	do { pmsg->cmd = (cmd); pmsg->subcmd = (subcmd); } while(0)

#define __IPCMSG_SET_ARG(pmsg, index, parg)		\
	do { pmsg->argv[(index)] = parg; } while(0)

#define __IPCMSG_GET_ARG(pmsg, index)		(((pmsg)->argc > (index)) ? (pmsg)->argv[(index)] : NULL)

#define __IPCMSG_SET_ARGC(pmsg, argc)		\
	do { (pmsg)->argc = (argc); } while(0)

#define __IPCMSG_GET_ARGC(pmsg)		((pmsg)->argc)

#define __IPCMSG_PUSH_ARG(pmsg, parg)		\
	do { (pmsg)->argv[(pmsg)->argc++] = parg; } while(0)

#define __IPCMSG_POP_ARG(pmsg)			(((pmsg)->argc > 0) ? (pmsg)->argv[--(pmsg)->argc] : NULL)

#define __IPCMSG_GET_RETURN(pmsg)		((pmsg)->result)

#define __IPCMSG_SET_COMPLETE(pmsg, comp_fn, comp_data)	\
	do { (pmsg)->complete = comp_fn; (pmsg)->context = (comp_data); } while(0)

#define __IPCMSG_EMPTY_ARG(pmsg)		\
	do { (pmsg)->argc = 0; } while(0)

#define __IPCMSG_IS_RESPONSE(pmsg)	(((pmsg)->cmd & __IPCMSG_RESPONSE_FLAG) != 0);

#define __IPCMSG_SET_RESPONSE(pmsg)		\
	do { (pmsg)->cmd |= __IPCMSG_RESPONSE_FLAG; } while(0)

#define __IPCMSG_SET_PRIVATE(pmsg, pdata)		\
	do { (pmsg)->priv = pdata; } while(0)

#define __IPCMSG_GET_PRIVATE(pmsg)	((pmsg)->priv)

#endif	/* __RK_IPC_COMN_H__ */
