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

#ifndef _DTA_DEBUG_H_
#define _DTA_DEBUG_H_

#if DIB_INTERNAL_MONITORING == 1

struct DbgOctoClockParameters
{
   uint32_t OctoClk;
};
struct DbgReflexCLockParameters
{
   uint32_t FemtoClk;
   uint32_t CceClk;
};
union ClockFrequency
{
   struct DbgOctoClockParameters Octopus;
   struct DbgReflexCLockParameters Reflex;
};

/// Init and close
int  DtaDebugInit(int32_t BoardId);
void DtaDebugClose(void);

/// Tests
void dbg_acg_freeze(int8_t ChipId, uint8_t Enable, uint16_t AntennaId);
void dbg_acqui(int8_t ChipId, uint8_t Enable, uint8_t Type);
void dbg_set_clock(int8_t ChipId, uint8_t Type, union ClockFrequency *Param);

#endif

#endif
