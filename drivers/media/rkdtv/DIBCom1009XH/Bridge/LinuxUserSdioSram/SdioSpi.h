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
int DibSdioSpiRead(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int size);
int DibSdioSpiWrite(struct DibBridgeContext *pContext,unsigned int addr,unsigned char* buf, unsigned int size);
int DibSdioSpiInitMode(struct DibBridgeContext *pContext);

void DibSdioSpiCmd52Write(struct DibBridgeContext *pContext,unsigned int Addr, unsigned char fct, unsigned char data);
unsigned char DibSdioSpiCmd53Read(struct DibBridgeContext *pContext,unsigned int Addr,unsigned char fct);
unsigned char DibSdioSpiCmd52Read(struct DibBridgeContext *pContext,unsigned int Addr,unsigned char fct);

#define SDIO_CMD_WRITE 		1
#define SDIO_CMD_READ 		0

#define SDIO_CMD_53 	 	53
#define SDIO_CMD_52 	 	52

#define SDIO_FCT_1 		1
#define SDIO_FCT_2 		2
#define SDIO_FCT_3 		3
#define SDIO_FCT_4 		4
#define SDIO_FCT_5 		5
#define SDIO_FCT_6 		6
#define SDIO_FCT_7 		7

