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
 * @file "DtaCasBcas.h"
 * @brief Bcas message processing.
 *
 ***************************************************************************************************/

#ifndef _DTA_CAS_BCAS_H_
#define _DTA_CAS_BCAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../cas/bcas/DibBCasMsg.h"

static uint8_t MsgBuf[256];

static void DibBCasDisplay(void * pCallbackCtx, uint16_t MsgType, uint16_t MsgIndex, uint8_t * pMsgBuf, uint32_t MsgLen)
{
   switch(MsgIndex)
   {
      case BCAS_CARDTYPE:       
        printf( "[BCAS] Card Type 1: %02x\n" , pMsgBuf[0]); 
        break;
      case BCAS_CARDID:  
      {
        unsigned char i;
        unsigned char M;

        printf( "[BCAS] Card Type 2: %c%d\n",pMsgBuf[1],pMsgBuf[2] );

        printf( "[BCAS] Card Id: " );
        for (i=1;i<11;i++)
          printf( "%02x " , pMsgBuf[i]);
        printf( "\n");

        M = pMsgBuf[0];
        if (M == 1)
          printf( "[BCAS] Group Id: Not Set\n" );
        else
          printf( "[BCAS] Group Id: %d\n",M );
      }
        break;
      case BCAS_GROUPID:     printf( "[BCAS] Group ID: %d\n"  , pMsgBuf[0]); 
        break;
      case BCAS_PRESENT:
        printf("[BCAS] BCAS card present\n");
        break;
      case BCAS_NOTPRESENT:
        printf("[BCAS] BCAS card not present\n");
        break;
      default:               printf( "[BCAS] RECEIVED UNKNOWN MSG %d\n" , MsgIndex); 
        break;
   };
}

#endif /* _DTA_CAS_BCAS_H_ */
