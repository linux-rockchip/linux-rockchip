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

#define DIBSPI_MAX_RD_SIZE 		64	
#define DIBSPI_MAX_WR_SIZE 	        64	


int DibSpiInit(struct DibBridgeContext *pContext);
int DibSpiDeInit(struct DibBridgeContext *pContext);
int DibSpiSetClock(struct DibBridgeContext *pContext,unsigned int);
int DibSpiWrite(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int);
int DibSpiRead(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int);


