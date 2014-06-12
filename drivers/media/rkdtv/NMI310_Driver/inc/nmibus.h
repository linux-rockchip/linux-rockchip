////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Newport Media Inc.  All rights reserved.
//
// Module Name:  NMIBUS.H
//
// Author : K.Yu
//
// Date : 2th July. 2007
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _NMIBUS_H_
#define _NMIBUS_H_

/********************************************
	Bus Functions
********************************************/
typedef int (*NMI_BUS_INIT) (void *);
typedef void (*NMI_BUS_DEINIT) (void *);
typedef int (*NMI_BUS_READ) (void *, uint32_t, uint8_t*, uint32_t);
typedef int (*NMI_BUS_WRITE) (void *, uint32_t, uint8_t *, uint32_t);

typedef struct {
	int (*nmi_bus_init)(void *);
	void (*nmi_bus_deinit)(void *);
	int (*nmi_bus_read)(void *, uint32_t, uint8_t *, uint32_t);
	int (*nmi_bus_write)(void *, uint32_t, uint8_t *, uint32_t);
} NMIBUSVTBL;

#endif
