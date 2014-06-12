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
* @file "DibMessages.c"
* @brief Generic Message Handling.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverConstants.h"
#include "DibDriverTargetTypes.h"
#include "DibDriverCommon.h"
#include "DibDriverTargetDebug.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverDowncalls.h"
#include "DibDriverIf.h"
#include "DibDriver.h"
#include "DibMsgGenericIf.h"

/**
 * Sends a message to one of the riscs from application context (not irq context)
 * @param pContext pointer to the context
 * @param Data     pointer to the data to send
 * @param Nb       size of the message in bytes
 */
DIBSTATUS DibDriverSendMessage(struct DibDriverContext *pContext, uint32_t * Data, uint32_t Nb)
{
   int8_t Sender;

   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(Data);

   if (pContext->DibChip != DIB_FIREFLY) {

      /* TODO fix all .Senders */
      Sender = GET_MSG_SENDER(*Data);
      if (Sender > 7)
         Sender -= 16;

      if (Sender > -1) {
         DIB_DEBUG(SOFT_ERR, (CRB "Messages send to Chip without Sender ID properly set - forcing it to -1" CRA));
         *Data |= (0xf) << 24;
      }
   }

   Status = (DIBSTATUS)DibD2BSendMsg(pContext, Nb, Data);

   return Status;
}



