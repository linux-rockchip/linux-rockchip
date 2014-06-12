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
 * @file "DtaCasNagra.c"
 * @brief Nagra message processing.
 *
 ***************************************************************************************************/

#ifdef USE_NAGRA
#include "DtaCasNagra.h"

/* Message interface buffer */
uint8_t MsgBuf[256];

void DibProcessNagraPopup(uint8_t * pMsgBuf, uint32_t MsgLen)
{
   uint8_t isPersistent, nbContent, *curLangMsg = NULL, *engMsg = NULL, *otherMsg = NULL, *rdPtr;
   char msgLang[4] = {'N', 'U', 'L', '\0'}, *curLang = "fra";
   uint32_t contentLen, tt;
   
   /* encoding of the sendBuffer:
    * sendBuf[0] = persistance (true if BDC_POP_UP_PERSISTENCE_ALWAYS)
    * sendBuf[1] = Number of content buffers (languages)
    * --- first content buffer begin ---
    * sendBuf[2..4] = language
    * sendBuf[5..8] = contentLen
    * sendBuf[9..9+contentLen-1] = content
    * --- first content buffer end ---
    * sendBuf[9+contentLen..9+contentLen+2] = language of second content buffer
    * ...
    */
   isPersistent = pMsgBuf[0];
   nbContent = pMsgBuf[1];
   rdPtr = pMsgBuf + 2;

   for (tt = 0; tt < nbContent; tt++) 
   {
      /* read language and length */ 
      if (rdPtr - pMsgBuf + 7 > MsgLen)
      {
         printf("++++++BDC popup: ERROR: popup content truncated!\n");
         break;
      }

      memcpy(msgLang, rdPtr, 3);
      rdPtr += 3;

      /* fix endianness */
      contentLen = 0;
      contentLen |= (*(rdPtr++) << 24);
      contentLen |= (*(rdPtr++) << 16);
      contentLen |= (*(rdPtr++) << 8 );
      contentLen |= (*(rdPtr++));
       
      if (rdPtr - pMsgBuf + contentLen > MsgLen)
      {
         printf("++++++BDC popup: ERROR: popup content truncated!\n");
         break;
      }

      /* read languages. We need current, english or any language */
      if (strcmp(msgLang, curLang) == 0) 
      {
         curLangMsg = malloc(contentLen+1);
         memcpy(curLangMsg, rdPtr, contentLen);
         curLangMsg[contentLen] = 0;
      }
      if (strcmp(msgLang, "eng") == 0) 
      {
         engMsg = malloc(contentLen+1);
         memcpy(engMsg, rdPtr, contentLen);
         engMsg[contentLen] = 0;
      }
      if (otherMsg == NULL) 
      {
         otherMsg = malloc(contentLen+1);
         memcpy(otherMsg, rdPtr, contentLen);
         otherMsg[contentLen] = 0;
      }
      rdPtr += contentLen;
   }
   
   if (!curLangMsg)
   {
      curLangMsg = engMsg;
      engMsg = NULL;
   }
   if (!curLangMsg)
   {
      curLangMsg = otherMsg;
      otherMsg = NULL;
   }

   printf("=======BDC pops up ");
   if (!isPersistent)
      printf("non-");
   printf("persistent message:\n");
   if (curLangMsg == NULL)
   {
      printf ("<no Msg>\n");
   }
   else if (curLangMsg[0] >= 20)
   {
      printf("%s", curLangMsg);
   }
   else
   {
      switch (curLangMsg[0])
      {
      case 0x10: printf ("<ISO 8859 Msg>\n"); break;
      case 0x11: printf ("<ISO/IEC 6937 Msg>\n"); break;
      case 0x1: 
      case 0x2: 
      case 0x3: 
      case 0x4: 
      case 0x5: printf ("<ISO/IEC 8859 5-9 Msg>\n"); break;
      default: printf ("<strange Msg 0x%x>\n", curLangMsg[0]); break;
      }
   }
   printf("\n=========\n");
   free(curLangMsg);
   free(engMsg);
   free(otherMsg);
}

void DibNagraDisplay(void * pCallbackCtx, uint16_t MsgType, uint16_t MsgIndex, uint8_t * pMsgBuf, uint32_t MsgLen)
{
   switch(MsgIndex)
   {
      case NAGRA_VERSION:       printf( "[NAGRA] CasVersion: %s\n" , pMsgBuf); break;
      case NAGRA_VENDOR:        printf( "[NAGRA] CasVendor: %s\n"  , pMsgBuf); break;
      case NAGRA_CA_SYSTEM_ID:  printf( "[NAGRA] CaSystemId: %d\n" , (pMsgBuf[0] << 8) + pMsgBuf[1]); break;
      case NAGRA_CA_CARD_ID:    printf( "[NAGRA] CardId: 0x"); 
      {
         uint8_t ii;
         for (ii=0; ii<MsgLen; ii++)
            printf("%02x", pMsgBuf[ii]);
         printf("\n");
      }
      break;
      case NAGRA_PRINT_OUTPUT : printf( "%s" , pMsgBuf); fflush(stdout); break;
      case NAGRA_BDC_POPUP    : DibProcessNagraPopup(pMsgBuf, MsgLen); break;
      case NAGRA_BDC_POPUP_REMOVE: printf( "++++++++ nagra removes popup\n"); break;
      case NAGRA_ACCESS_STATUS: 
         if(*pMsgBuf == eACCESS_GRANTED)
            printf( "Access GRANTED\n");
         else
            printf( "Access DENIED\n");
         break;
      case NAGRA_SUBSCRIBER_STATUS: 
         switch (*pMsgBuf)
         {
            case eSUBSCRIBER_UNKNOWN        : printf("Subscriber is unknown\n"); break;
            case eSUBSCRIBER_REGISTERED     : printf("Subscriber is registered\n"); break;
            case eSUBSCRIBER_SUSPENDED      : printf("Subscriber is suspended \n"); break;
            case eSUBSCRIBER_NOT_REGISTERED : printf("Subscriber not registered\n"); break;
            case eSUBSCRIBER_ERROR          : printf("Subscriber status error \n"); break;
            default: printf( "[NAGRA] invalid subscriber status message\n");
         }
         break;

      case NAGRA_ERROR: 
         switch (*pMsgBuf)
         {
            case eNAGRA_ERROR           : printf("Nagra CAS error\n"); break;
            case eNAGRA_ERROR_SMARTCARD : printf("Nagra Smartcard error\n"); break;
            default: printf( "[NAGRA] invalid Nagra error message %d\n", *pMsgBuf);
         }
         break;
      default:               printf( "[NAGRA] RECEIVED UNKNOWN MSG %d\n" , MsgIndex); break;
   };
}

#endif /* USE_NAGRA */
