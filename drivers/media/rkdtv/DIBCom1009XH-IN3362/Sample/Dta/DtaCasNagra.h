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

/** ***********************************************************************************************
 * @file "DtaCasNagra.h"
 * @brief Nagra message processing.
 *
 ***************************************************************************************************/

#ifndef _DTA_CAS_NAGRA_H_
#define _DTA_CAS_NAGRA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OsTypes.h"
#include "../../Sample/Examples/DibNagraMsg.h"

extern uint8_t MsgBuf[256];

void DibProcessNagraPopup(uint8_t * pMsgBuf, uint32_t MsgLen);
void DibNagraDisplay(void * pCallbackCtx, uint16_t MsgType, uint16_t MsgIndex, uint8_t * pMsgBuf, uint32_t MsgLen);


#endif /* _DTA_CAS_NAGRA_H_ */
