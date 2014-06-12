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
* @file "DibMsgApi.h"
* @brief Definitions for API messages use
*
***************************************************************************************************/
#ifndef DIB_MSG_API_H
#define DIB_MSG_API_H

/**
 * MSG API types
 */

/* deprecated */
/* #define MSG_API_TYPE_SYS               0
 * #define MSG_API_TYPE_FE                1
 */
#define MSG_API_TYPE_PCSC              2
#define MSG_API_TYPE_STV               3
#define MSG_API_TYPE_HAL               4
#define MSG_API_TYPE_BCAS              5
#define MSG_API_TYPE_MBBMS             6
#define MSG_API_TYPE_NAGRA             7

/**
 * MSG API status
 */
#define MSG_API_SUCCESS                0
#define MSG_API_PENDING                1
#define MSG_API_ERROR                  2
#define MSG_API_INVALID_ID             3
#define MSG_API_INVALID_PARAM          4
#define MSG_API_TIMEOUT                5

#endif /* DIB_MSG_API_H */
