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
* @file "DibDriverConfig.h"
* @brief Gpio related structures.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_CONFIG_H
#define DIB_DRIVER_CONFIG_H

#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

/************************************************************/
/* Possible Configuration Values                            */
/************************************************************/
/** Specific Debug Print Levels (DEBUG_MODE) */
#define NO_DEBUG                    0
#define DEBUG_PRINT                 1

/** Supported Irq Modes         (IRQ_POLARITY)  */
#define IRQ_ACTIVE_LOW              0
#define IRQ_ACTIVE_HIGH             1

/*************************************************************/
/*************************************************************/

#define ENG3_COMPATIBILITY  1


/*************************************************************/
/*** Build Configuration                                  ****/
/*************************************************************/
/* Activates parameter checking to ensure application is passing
   appropriate parameters, once validated, can be disabled for
   faster and more compact code.                            */
#define PARAMETER_CHECKING           1
/* afford multi channels on a stream 1 = afford 0 = forbid */
#define DIB_MULTI_CHANNEL_PER_STREAM 0

/* Activates debug logs, DIB_ASSERT and dumps. Once application is
   validated, can be disabled for faster and more compact
   code.                                                    */
#define DEBUG_MODE                  NO_DEBUG

/*************************************************************/
/*** Debug Flags                                          ****/
/*************************************************************/
/** Check Si Psi Section CRC */
#define CHECK_SIPSI_CRC                         0

/* Log Application calls into a file s*/
#define LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE 0

#endif
