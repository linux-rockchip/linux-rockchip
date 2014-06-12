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
* @file "DibBridgeTargetNet.h"
* @brief Target specific implementation.
*
***************************************************************************************************/
void DibBridgeTargetNetSubmitPacket(int32_t len, uint8_t *buf);

void DibBridgeTargetNetSubmitRawTsPacket(uint8_t *buf, int32_t len);

void DibBridgeTargetNetCleanup(void);

int32_t DibBridgeTargetNetInit(void);

int32_t DibBridgeTargetNetRegisteredInit(struct net_device *dev);

//int32_t DibBridgeTargetNetPoll(struct net_device *dev, int32_t *budget);
