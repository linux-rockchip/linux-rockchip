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


/**************************************************************************************************
* @file "DibDriverDragonflyTest.h"
* @brief Dragonfly specific debugging functions.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_DRAGONFLY_TEST_H_
#define DIB_DRIVER_DRAGONFLY_TEST_H_

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverTargetTypes.h"
#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverDowncalls.h"
#include "DibBoardSelection.h"

#include "DibDriverCtx.h"

#if (mSDK==0)

#define TEST_MDMA   1
#define TEST_API    2
#define NO_TEST     3
#define TEST_ACCESS     4
    
#define CURRENT_TEST         NO_TEST   

/* BELOW : only for New Api Test */

struct Ch
{
    uint32_t freq;
    uint8_t  prio;
    uint8_t  MaxDemod;
    uint8_t  MinDemod;
    uint8_t  Type;
    uint8_t  bdwth;
    uint8_t  adapter;
    uint8_t  idx;
};

struct Filt
{
    uint8_t ParentCh;
    uint8_t FiltType;
    uint8_t idx;
};

struct It
{
    uint8_t          ParentFilt;
    uint8_t          idx;
    union DibFilters ConfigInfo;
};


uint8_t DibDriverDragonflyIntTest(struct DibDriverContext *pContext, uint32_t TestStep);


#if CURRENT_TEST == TEST_API


#define NB_CH_MAX           4
#define NB_FILT_MAX         4
#define NB_ITEM_MAX         4

extern uint8_t gCurr;

extern struct Ch   ch[NB_CH_MAX];
extern struct Filt f[NB_FILT_MAX];
extern struct It   item[NB_ITEM_MAX];

void InitTest(void);

#endif

void DibDriverDragonflyMDMA(struct DibDriverContext *pContext, uint32_t src, uint32_t dst, uint32_t xsize, uint32_t fillsize, uint32_t fillvalue, uint32_t test_step);

void DibDriverDragonflyTestAccess(struct DibDriverContext *pContext);

#endif /*DIB_DRIVER_DRAGONFLY_TEST_H_*/
#endif
