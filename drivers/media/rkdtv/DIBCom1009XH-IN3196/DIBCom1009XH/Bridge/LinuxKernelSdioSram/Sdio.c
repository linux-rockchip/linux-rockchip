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
* @file "SdioSkeleton.c"
* @brief Target Specific.
*
***************************************************************************************************/

#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridge.h"

#if (DIBINTERF_PHY == DIBINTERF_PHY_SDIO)

#include "Sdio.h"

#if (DIBCOM_TEST_MODE == 1)
#include "Hook.h"
#endif

/*-----------------------------------------------------------
SdioInit()
-------------------------------------------------------------*/
int32_t SdioInit(struct DibBridgeContext *pContext)
{


#if (DIBCOM_TEST_MODE == 1)
   HookInit(pContext,DIBBRIDGE_MODE_SDIO,pContext->BoardHdl);
   HookSdioBusInit(pContext);
#endif

  return DIBSTATUS_SUCCESS;
}

int SdioCmd53(struct DibBridgeContext *pContext, SDIO_CMD *Cmd)
{

#if (DIBCOM_TEST_MODE == 1)
  unsigned char cmd[4];
  cmd[3] = Cmd->size & 0xff;
  cmd[2] = ((Cmd->addr << 1) & 0xFE) | ((Cmd->size >> 8) & 0x01);
  cmd[1] = (Cmd->addr >> 7) & 0xFF;
  cmd[0] = (Cmd->rw << 7) + (Cmd->fct << 4) +(Cmd->block <<3) + ((Cmd->addr >> 15) & 0x3);
  HookSdioBusCmd53(pContext, cmd, Cmd->buffer, Cmd->size);
#endif

  return DIBSTATUS_SUCCESS;
}

int SdioCmd52(struct DibBridgeContext *pContext, SDIO_CMD *Cmd)
{
#if (DIBCOM_TEST_MODE == 1)
  unsigned char cmd[4];
  cmd[3] = Cmd->buffer[0];
  cmd[2] = ((Cmd->addr<<1) & 0xFE);
  cmd[1] = (Cmd->addr>>7) & 0xFF ;
  cmd[0] = (Cmd->rw<<7) + (Cmd->fct<<4) + ((Cmd->addr>>15) & 0x3);
  HookSdioBusCmd52(pContext, cmd, Cmd->size);
#endif
  return DIBSTATUS_SUCCESS;
}

/*-----------------------------------------------------------
-------------------------------------------------------------*/
uint8_t SdioCrc7(uint8_t *buf, int32_t Nb)
{
   uint8_t  crc = 0;
   uint32_t i;
   uint32_t j;

   for(j = 0; j < Nb; j++) 
   {
      for(i = 0; i <= 7; i++) 
      {
         uint32_t crc_in, crc_in2;

         crc_in  = BIT(buf[j], (7 - i)) ^ BIT(crc, 0);
         crc_in2 = crc_in ^ BIT(crc, 4);
         crc     = (crc_in << 6) + (BIT(crc, 6) << 5) + (BIT(crc, 5) << 4) +
                   (crc_in2 << 3) + (BIT(crc, 3) << 2) + (BIT(crc, 2) << 1) + BIT(crc, 1);
      }
   } 
   
   return ((BIT(crc, 0) << 7) + (BIT(crc, 1) << 6) + (BIT(crc, 2) << 5) +
           (BIT(crc, 3) << 4) + (BIT(crc, 4) << 3) + (BIT(crc, 5) << 2) + (BIT(crc, 6) << 1) + 0x1);
}

#endif
