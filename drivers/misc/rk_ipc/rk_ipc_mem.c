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
#include <linux/genalloc.h>

#include "rk_ipc.h"

/* align x on a size boundary - adjust x up/down if needed */
#define _ALIGN_UP(x, size)    (((x)+((size)-1)) & (~((size)-1)))
#define _ALIGN_DOWN(x, size)  ((x) & (~((size)-1)))

#define WORD_SIZE				sizeof(unsigned long)
#define WORD_ALIGN_UP(x)		_ALIGN_UP(x, WORD_SIZE)
#define WORD_ALIGN_DOWN(x)		_ALIGN_DOWN(x, WORD_SIZE)
#define DWORD_SIZE				(WORD_SIZE << 1)


/*------- IPC MSG POOL (used to alloc ipc_msg structure ) -----------------*/

struct ipcmsg_memnode {
	struct ipcmsg_memnode *next;
};

struct ipc_msg_pool {
	struct ipcmsg_memnode *free;
	spinlock_t	lock;
};


/**
 * @brief create and initialize a IPC_msg memory pool
 *
 * @param[in] base - start address of IPC_msg memory pool
 * @param[in] len - memory pool size
 *
 * @return : not NULL means success, NULL means failure.
 */
struct ipc_msg_pool* ipc_msg_pool_create(void *base, int len)
{
	struct ipc_msg_pool *pool;
	u8 *memnode_memory;
	u16 memnode_num;
	struct ipcmsg_memnode *m, *chunk;
	u16 size;
	unsigned long flags;
	int i;

	pool = (struct ipc_msg_pool *)kmalloc(sizeof(*pool), GFP_KERNEL);
	if (pool == NULL) {
		return NULL;
	}

	printk("ipcmsg pool range : [%08lx, %08lx)\n", (unsigned long)base, (unsigned long)base + len);

	spin_lock_init(&pool->lock);

	size = WORD_ALIGN_UP(sizeof(struct ipc_msg) + sizeof(struct ipcmsg_memnode));
	memnode_memory = (unsigned char *)base;
	memnode_num = len / size;

	printk("memnode_memory = %p, memnode_num = %d\n",
			memnode_memory, memnode_num);

	spin_lock_irqsave(&pool->lock, flags);

	chunk = (struct ipcmsg_memnode *)memnode_memory;
	pool->free = chunk;
	m = chunk;

	for (i = 0; i < memnode_num; i++) {
		m->next = (struct ipcmsg_memnode *)((unsigned char *)m + size);
		chunk = m;
		m = m->next;
	}
	chunk->next = NULL;

	spin_unlock_irqrestore(&pool->lock, flags);

	return pool;

}

/**
 * @brief allocate a IPC msg from pool
 *
 * @param[in] pool	- IPC msg memory pool
 *
 * @return : not NULL means success, NULL means failure.
 */
struct ipc_msg* ipc_msg_pool_alloc(struct ipc_msg_pool *pool)
{
	struct ipcmsg_memnode *m;
	void *mem = NULL;

	m = pool->free;
	if (m != NULL) {
		pool->free = m->next;
		m->next = NULL;

		mem = (unsigned char *)m + sizeof(struct ipcmsg_memnode);
	}

	return (struct ipc_msg *)mem;
}

/**
 * @brief free a IPC msg to pool
 *
 * @param[in] pool	- IPC msg memory pool
 * @param[in] pmsg	- IPC msg
 *
 * @return : void
 */
void ipc_msg_pool_free(struct ipc_msg_pool *pool, struct ipc_msg *pmsg)
{
	struct ipcmsg_memnode *m;

	m = (struct ipcmsg_memnode*)((unsigned char *)pmsg - sizeof(struct ipcmsg_memnode));

	m->next = pool->free;
	pool->free = m;
}


/*---------IPC SHARE MEMORY POOL (used to alloc IPC msg shmem) ---------------*/

struct ipc_shmem_pool {
	struct gen_pool *gen_pool;
	void *base;
	size_t	size;
	wait_queue_head_t wq;
};

/**
 * @brief create and initialize a IPC share memory pool
 *
 * @param[in] base - start address of IPC share memory pool
 * @param[in] len - memory pool size
 *
 * @return : not NULL means success, NULL means failure.
 */
struct ipc_shmem_pool* ipc_shmem_pool_create(void *base, int len)
{
	struct ipc_shmem_pool *pool;
	int ret;

	pool = (struct ipc_shmem_pool *)kmalloc(sizeof(*pool), GFP_KERNEL);
	if (pool == NULL)
		return NULL;

	printk("IPC Shmem buffer range : [%08lx, %08lx)\n", (unsigned long)base, (unsigned long)base + len);

	pool->gen_pool = gen_pool_create(2, -1);
	if (!pool->gen_pool) {
		kfree(pool);
		return NULL;
	}

	pool->base = base;
	pool->size = len;

	ret = gen_pool_add(pool->gen_pool, (unsigned long)pool->base, pool->size, 0);
	if (ret) {
		gen_pool_destroy(pool->gen_pool);
		kfree(pool);
		pool = NULL;
	}

	return pool;
}

/**
 * @brief allocate a IPC share memory from pool
 *
 * @param[in] pool	- IPC share memory pool.
 * @param[in] alloc_len - memory allocate length.
 *
 * @return : not NULL means success, NULL means failure.
 */
void* ipc_shmem_pool_alloc(struct ipc_shmem_pool *pool, int alloc_len)
{
	unsigned long offset;

	offset = gen_pool_alloc(pool->gen_pool, alloc_len);

	return (void*)offset;
}

/**
 * @brief free a IPC share memory to pool
 *
 * @param[in] pool	- IPC share memory pool
 * @param[in] p		- memory pointer to free
 * @param[in] size	- memory size to free
 *
 * @return : void
 */
void ipc_shmem_pool_free(struct ipc_shmem_pool *pool, void* p, int size)
{
	gen_pool_free(pool->gen_pool, (unsigned long)p, size);
}

