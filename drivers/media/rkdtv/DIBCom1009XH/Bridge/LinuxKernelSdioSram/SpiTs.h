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

#define SPITS_MAX_RD_SIZE 		64	
#define SPITS_MAX_WR_SIZE 	        64	


int SpiTsInit(struct DibBridgeContext *pContext);
int SpiTsDeInit(struct DibBridgeContext *pContext);
int SpiTsSetClock(struct DibBridgeContext *pContext,unsigned int);
int SpiTsWrite(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int);
int SpiTsRead(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int);


