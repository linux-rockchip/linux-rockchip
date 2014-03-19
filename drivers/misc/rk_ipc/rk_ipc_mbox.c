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
#include <linux/notifier.h>
#include <linux/list.h>

#include "rk_ipc_mbox.h"

static LIST_HEAD(mbox_list);



/**
 * @brief get a reference of IPC mailbox, and also register a notify callback.
 *
 * @param[in] name : mailbox name
 * @param[in] nb : notifier block
 * 
 * @return no NULL means success, NULL means fail.
 */
struct ipc_mbox* rk_mbox_get(const char* name, struct notifier_block *nb)
{
	struct ipc_mbox *mbox;

	if (list_empty(&mbox_list)) {
		return NULL;
	}

	if (name == NULL || strcmp(name, "default") == 0) {
		mbox = list_first_entry(&mbox_list, struct ipc_mbox, node);

		goto _found_mbox;
	}

	list_for_each_entry(mbox, &mbox_list, node) {
		if (!strcmp(mbox->name, name)) {
			goto _found_mbox;
		}
	}

	return NULL;

_found_mbox:
	if (nb && mbox->notifier_register) {
		mbox->notifier_register(mbox, nb);
	}

	if (mbox->startup) {
		mbox->startup(mbox);
	}

	return mbox;
}

/**
 * @brief put a reference of IPC mailbox, and also unregister notify callback.
 *
 * @param[in] mbox : IPC mailbox instance
 * @param[in] nb : notifier block
 * 
 * @return void
 */
void rk_mbox_put(struct ipc_mbox* mbox, struct notifier_block *nb)
{
	if (nb && mbox->notifier_unregister) {
		mbox->notifier_unregister(mbox, nb);
	}

	if (mbox->shutdown) {
		mbox->shutdown(mbox);
	}

}

/**
 * @brief register a ipc mailbox
 *
 * @param[in] mbox : IPC mailbox instance
 * 
 * @return if success return 0, otherwise return a negative number.
 */
int rk_mbox_register(struct ipc_mbox *mbox)
{
	if (mbox->msg_write == NULL ||
			mbox->msg_read == NULL ||
			mbox->notifier_register == NULL) {
		return -1;
	}

	list_add_tail(&mbox->node, &mbox_list);

	return 0;
}

EXPORT_SYMBOL_GPL(rk_mbox_get);
EXPORT_SYMBOL_GPL(rk_mbox_put);
EXPORT_SYMBOL_GPL(rk_mbox_register);
