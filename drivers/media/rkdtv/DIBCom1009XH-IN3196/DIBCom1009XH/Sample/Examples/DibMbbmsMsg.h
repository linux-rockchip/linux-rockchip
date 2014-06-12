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
 * @file "DibMbbmsMsg.h"
 * @brief Dibcom MBBMS API messages.
 *
 ***************************************************************************************************/

#ifndef DIB_MBBMS_MSG_H
#define DIB_MBBMS_MSG_H


#include "DibMsgApi.h"  /* MSG_API type definition */

/* this shall be replaced */
#ifndef MSG_API_TYPE_MBBMS
#define MSG_API_TYPE_MBBMS             6
#endif

#define MBBMS_SCTRANSMIT         0
#define MBBMS_CASYSTEMID_SET     1

#define MBBMS_PROTOVER_IND       2
#define MBBMS_CARDINFO_IND       3
#define MBBMS_CMMBSN_IND         4
#define MBBMS_SRVLIST_IND        5

#endif
