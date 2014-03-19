/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef __RK_BASEBAND_H__
#define __RK_BASEBAND_H__

struct rk_baseband_platform_data {
	void (*sim_vcc_sel)(int sim_id, int vccsel);		/* vccsel - 0 : off, 1 : 1.8v, 2 : 3.0v */

	/* BB Firmware running memory region */
	phys_addr_t bb_memphys;
	size_t		bb_memsz;

	/* ipc memory region */
	phys_addr_t	ipc_memphys;
	size_t		ipc_memsz;

	int			nr_serial_ports;	/* how many ipc serial ports to be created */
};

#endif	/* __RK_BASEBAND_H__ */
