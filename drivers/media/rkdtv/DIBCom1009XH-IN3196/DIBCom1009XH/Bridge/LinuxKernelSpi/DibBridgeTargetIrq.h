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
* @file "DibBridgeTargetIrq.h"
* @brief Target specific implementation.
*
***************************************************************************************************/
int32_t DibBridgeTargetStartIrqPollThread(struct DibBridgeContext *pContext);
int32_t DibBridgeTargetStopIrqPollThread(struct DibBridgeContext *pContext);
void    DibBridgeTargetIrqWakeupThread(struct DibBridgeContext *pContext);

