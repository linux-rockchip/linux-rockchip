//
// Module Name:  NMITYPES.H
//
// Author : K.Yu
//
// Date : 6th June. 2006
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _NMIBUS_TYPES_
#define _NMIBUS_TYPES_

/********************************************
	Bus Type Defines
********************************************/
typedef enum {
	_I2C_ = 1,
	_SPI1_,
	_SPI_,
	_USB_,
	_SDIO_,
	_EBI_,
} NMIBUSTYPE;

#endif
