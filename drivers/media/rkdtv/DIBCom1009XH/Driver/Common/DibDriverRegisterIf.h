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
* @file "DibDriverRegisterIf.h"
* @brief Internal Access functions prototypes.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_REGISTER_IF_H
#define DIB_DRIVER_REGISTER_IF_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/* Register access */
DIBSTATUS DibDriverWriteReg8(struct DibDriverContext *pContext, uint32_t Addr, uint8_t Value);
DIBSTATUS DibDriverReadReg8(struct DibDriverContext *pContext, uint32_t Addr, uint8_t * Value);
DIBSTATUS DibDriverReadReg16(struct DibDriverContext *pContext, uint32_t Addr, uint16_t * Value);
DIBSTATUS DibDriverWriteReg16(struct DibDriverContext *pContext, uint32_t Addr, uint16_t Value);
DIBSTATUS DibDriverReadReg32(struct DibDriverContext *pContext, uint32_t Addr, uint32_t * Value);
DIBSTATUS DibDriverWriteReg32(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Value);

/* Bus accesss */
DIBSTATUS DibDriverReadI2C(struct DibDriverContext *pContext, uint16_t Addr, uint8_t nbtx, uint32_t Nb, uint8_t *pBuf);
DIBSTATUS DibDriverWriteI2C(struct DibDriverContext *pContext, uint16_t Addr, uint32_t Nb, uint8_t *pBuf);
DIBSTATUS DibDriverReadInternalBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t *pData);
DIBSTATUS DibDriverWriteInternalBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Data);
DIBSTATUS DibDriverReadDirectBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t *pData);
DIBSTATUS DibDriverWriteDirectBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Data);

/* Internal Debug */
DIBSTATUS DibDriverInternalTest(struct DibDriverContext *pContext, uint32_t test);

/*********************** chip specific functions ***************************/

/*----------------------------- Register interface --------------------------------------------*/
#define DibDriverRegisterIfInit(pContext)                     pContext->ChipOps.RegisterIfInit(pContext)
#define DibDriverWriteDemod(pContext, DemId, SubAdd, Value)   pContext->ChipOps.WriteDemod(pContext, DemId,  SubAdd, Value)
#define DibDriverReadDemod(pContext, DemId, SubAdd)           pContext->ChipOps.ReadDemod(pContext, DemId, SubAdd)

#endif /*  DIB_DRIVER_REGISTER_IF_H */
