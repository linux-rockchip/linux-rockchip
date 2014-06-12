/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

/******************************************************************************
* @file "DtaPlayout.h"
* @brief DiBcom Test Application - Playout stream directly to chipset memory
* for physical layer agnostic software development.
*******************************************************************************/

#ifndef DTA_PLAYOUT_H
#define DTA_PLAYOUT_H

/* #include <inttypes.h> */

int32_t DtaPlayoutStart  (void);
int32_t DtaPlayoutStop   (void);
int32_t DtaPlayoutOpen   (char *NamePayld, char *NameDescr, uint8_t NbPhyFrames);
int32_t DtaPlayoutClose  (void);
int32_t DtaPlayoutStep   (void);
int32_t DtaPlayoutCorrupt(uint8_t Corruption);
int32_t DtaPlayoutRun    (void);
int32_t DtaPlayoutPause  (void);
int32_t DtaPlayoutRewind (void);
int32_t DtaPlayoutRecord (uint8_t *Descriptor);
int32_t DtaPlayoutErase  (uint8_t Handle);

#endif /* DTA_PLAYOUT_H */
