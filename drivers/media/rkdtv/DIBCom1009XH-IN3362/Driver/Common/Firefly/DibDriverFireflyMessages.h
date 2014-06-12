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
* @file "DibDriverFireflyMessages.h"
* @brief Firefly specific messages.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_FIREFLY_MESSAGES_H
#define DIB_DRIVER_FIREFLY_MESSAGES_H

/*
 * Messages between RISC and HOST.
 */

#ifdef DIB_MSG_OUT
#undef DIB_MSG_OUT
#endif

#ifdef DIB_MSG_IN
#undef DIB_MSG_IN
#endif

#define DIB_MSG_OUT(a) a,
#define DIB_MSG_IN(a)
enum _msgs_out_a 
{
#include "DibDriverFireflyMsgNames.h" 
};

#undef DIB_MSG_OUT
#undef DIB_MSG_IN
#define DIB_MSG_OUT(a)
#define DIB_MSG_IN(a) a,
enum _msgs_in 
{
#include "DibDriverFireflyMsgNames.h"
  IN_MSG_LAST
};

#endif
