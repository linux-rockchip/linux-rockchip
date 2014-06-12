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
 * @file "DibBCassMsg.h"
 *
 ***************************************************************************************************/

#ifndef DIB_BCAS_MSG_H
#define DIB_BCAS_MSG_H


#include "DibMsgApi.h"  /* MSG_API type definition */


#define BCAS_CARDTYPE_LEN         1
#define BCAS_CARDID_LEN           11 
#define BCAS_GROUPID_LEN          1


/* Indexes for MSG_API_TYPE_STV */
#define BCAS_CARDTYPE         0
#define BCAS_CARDID           1
#define BCAS_GROUPID          2
#define BCAS_NOTPRESENT       3
#define BCAS_PRESENT          4
#endif /* DIB_BCAS_MSG_H */
