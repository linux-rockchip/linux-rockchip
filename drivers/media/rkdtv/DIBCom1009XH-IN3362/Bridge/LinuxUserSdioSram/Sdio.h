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

#define SDIO_BLOCK_SIZE_2K		2048

#define SDIO_MAX_RD_SIZE 	        64	
#define SDIO_MAX_WR_SIZE 		64	

#define SDIO_SPI_COMMAND_SIZE 	6

#define BIT(a,b) (((a)>>(b))&1)

typedef struct {
  unsigned char 	cmd;
  unsigned long		addr;
  unsigned char 	rw;
  unsigned char 	*buffer;
  unsigned long 	size;
  unsigned char 	block;
  unsigned long 	block_size;
  unsigned char 	fct;
} SDIO_CMD;


int SdioInit(struct DibBridgeContext *pContext);
int SdioCmd53(struct DibBridgeContext *pContext,SDIO_CMD *cmd);
int SdioCmd52(struct DibBridgeContext *pContext,SDIO_CMD *cmd);

