/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __RK_IPC_MEM_H
#define __RK_IPC_MEM_H

struct ipc_msg_pool;
struct ipc_shmem_pool;

/**
 * @brief create and initialize a IPC_msg memory pool
 *
 * @param[in] base - start address of IPC_msg memory pool
 * @param[in] len - memory pool size
 *
 * @return : not NULL means success, NULL means failure.
 */
extern struct ipc_msg_pool* ipc_msg_pool_create(void *base, int len);

/**
 * @brief allocate a IPC msg from pool
 *
 * @param[in] pool	- IPC msg memory pool
 *
 * @return : not NULL means success, NULL means failure.
 */
extern struct ipc_msg* ipc_msg_pool_alloc(struct ipc_msg_pool *pool);

/**
 * @brief free a IPC msg to pool
 *
 * @param[in] pool	- IPC msg memory pool
 * @param[in] pmsg	- IPC msg
 *
 * @return : void
 */
extern void ipc_msg_pool_free(struct ipc_msg_pool *pool, struct ipc_msg *pmsg);

/**
 * @brief create and initialize a IPC share memory pool
 *
 * @param[in] base - start address of IPC share memory pool
 * @param[in] len - memory pool size
 *
 * @return : not NULL means success, NULL means failure.
 */
extern struct ipc_shmem_pool* ipc_shmem_pool_create(void *base, int len);

/**
 * @brief allocate a IPC share memory from pool
 *
 * @param[in] pool	- IPC share memory pool.
 * @param[in] alloc_len - memory allocate length.
 *
 * @return : not NULL means success, NULL means failure.
 */
extern void* ipc_shmem_pool_alloc(struct ipc_shmem_pool *pool, int alloc_len);

/**
 * @brief free a IPC share memory to pool
 *
 * @param[in] pool	- IPC share memory pool
 * @param[in] p		- memory pointer to free
 * @param[in] size	- memory size to free
 *
 * @return : void
 */
extern void ipc_shmem_pool_free(struct ipc_shmem_pool *pool, void* p, int len);

#endif	/* __RK_IPC_MEM_H */
