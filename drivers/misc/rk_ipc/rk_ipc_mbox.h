/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __RK_IPC_MBOX_H
#define __RK_IPC_MBOX_H

struct ipc_mbox {
	const char* name;

	/* startup mailbox */
	int (*startup)(struct ipc_mbox *);

	/* shutdown mailbox */
	int (*shutdown)(struct ipc_mbox *);

	/* reset mailbox state */
	int (*reset)(struct ipc_mbox *);

	/* write a msg to mailbox */
	int (*msg_write)(struct ipc_mbox *, u32);

	/* read a msg from mailbox */
	int (*msg_read)(struct ipc_mbox *, u32 *);

	/* register a notifier to mailbox */
	int (*notifier_register)(struct ipc_mbox *, struct notifier_block *nb);

	/* unregister a notifier from mailbox */
	int (*notifier_unregister)(struct ipc_mbox *, struct notifier_block *nb);

	struct list_head node;
};

/* IPC Mailbox Notify Event */
#define RK_IPCMBOX_EVENT_MSG_INCOMING		0x01
#define RK_IPCMBOX_EVENT_MSG_WRITEABLE		0x02

/**
 * @brief get a reference of IPC mailbox, and also register a notify callback.
 *
 * @param[in] name : mailbox name
 * @param[in] nb : notifier block
 * 
 * @return no NULL means success, NULL means fail.
 */
extern struct ipc_mbox* rk_mbox_get(const char* name, struct notifier_block *nb);

/**
 * @brief put a reference of IPC mailbox, and also unregister notify callback.
 *
 * @param[in] mbox : IPC mailbox instance
 * @param[in] nb : notifier block
 * 
 * @return void
 */
extern void rk_mbox_put(struct ipc_mbox* mbox, struct notifier_block *nb);

/**
 * @brief register a ipc mailbox
 *
 * @param[in] mbox : IPC mailbox instance
 * 
 * @return if success return 0, otherwise return a negative number.
 */
extern int rk_mbox_register(struct ipc_mbox *mbox);

#endif	/* __RK_IPC_MBOX_H */
