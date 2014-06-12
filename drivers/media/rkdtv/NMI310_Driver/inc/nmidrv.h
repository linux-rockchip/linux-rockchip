#ifndef _NMIDRV_H_
#define _NMIDRV_H_

/********************************************
	Driver 
********************************************/
typedef struct {
	NMITV				dtvtype;
	NMIBUSTYPE 	bustype;
	uint32_t			dbgflag;
	int					crystalindex;
	NMIHLPVTBL	tbl;
	int					chipver;
} NMICMN;

#endif
