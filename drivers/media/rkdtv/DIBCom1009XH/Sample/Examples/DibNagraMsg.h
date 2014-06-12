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
 * @file "DibNagra.h"
 * @brief Dibcom Nagra API messages.
 *
 ***************************************************************************************************/

#ifndef DIB_NAGRA_MSG_H
#define DIB_NAGRA_MSG_H


#include "DibMsgApi.h"  /* MSG_API type definition */


/* Indexes for MSG_API_TYPE_NAGRA */
#define NAGRA_VERSION           0
#define NAGRA_VENDOR            1
#define NAGRA_CA_SYSTEM_ID      2
#define NAGRA_CA_CARD_ID        3
#define NAGRA_CHAR_INPUT        4 /* testsuite only */
#define NAGRA_PRINT_OUTPUT      5 /* dbg cas library only */
#define NAGRA_BDC_POPUP         6 /* Nagra classic only (not for NSCD) */
#define NAGRA_BDC_POPUP_REMOVE  7  /* Nagra classic only (not for NSCD) */
#define NAGRA_ACCESS_STATUS     8  /* granted / denied */
#define NAGRA_SUBSCRIBER_STATUS 9
#define NAGRA_ERROR             10

/* Nagra CAS Access Status */
enum DibAccessStatus
{
   eACCESS_GRANTED  =  0,
   eACCESS_DENIED   =  1
};

enum DibSubscriberStatus
{
   eSUBSCRIBER_UNKNOWN        = 0,   /* Subscriber is unknown */
   eSUBSCRIBER_REGISTERED     = 1,   /* Subscriber is registered */
   eSUBSCRIBER_SUSPENDED      = 2,   /* Subscriber is suspended */
   eSUBSCRIBER_NOT_REGISTERED = 3,   /* Subscriber is not registered */
   eSUBSCRIBER_ERROR          = 0xFF /* Subscriber status error */
};

enum DibNagraError
{
   eNAGRA_ERROR               = 0,
   eNAGRA_ERROR_SMARTCARD     = 1,   /* Smartcard communication error */
};

#endif /* DIB_NAGRA_MSG_H */
