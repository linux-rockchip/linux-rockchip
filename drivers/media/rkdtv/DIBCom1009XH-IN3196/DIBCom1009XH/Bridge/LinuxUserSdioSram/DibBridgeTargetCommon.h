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
* @file "DibBridgeTargetCommon.h"
* @brief Target Specific.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_TARGET_COMMON_H
#define DIB_BRIDGE_TARGET_COMMON_H

#include "DibDriverTarget.h"
#include <pthread.h>

/**********************************************************/
/****************** PLATFORM SPECIFIC FLAGS  **************/
/**********************************************************/
/** Supported Types of Physical Interface (DIBINTERF_PHY) */
#define DIBINTERF_PHY_SDIO       1
#define DIBINTERF_PHY_SRAM       2
#define DIBINTERF_PHY_SPI        3 
#define DIBINTERF_PHY_I2C        4 
#define DIBINTERF_PHY_SPITS      5 

/* Select Physical Interface */
#define DIBINTERF_PHY               DIBINTERF_PHY_I2C

#define USE_DIBSPI_WORKAROUND	  0

#define DIBCTRL_ADDR DIBCTRL_DEFAULT_ADDR
#define DIBCTRL_DEFAULT_ADDR  0x40
#define DIBCTRL_DIV_ADDR      0x3F

/*** Test mode used internally to verify host interface using Dib0700 USB bridge **/
#define TEST_MODE_NONE		0	
#define TEST_MODE_HOOK		1	
#define TEST_MODE_SPP		  2	

/* use custom physical read/write access */
#define DIBCOM_TEST_MODE 	TEST_MODE_NONE
#define EP2_STREAMING_SRAM_MODE       1
#define EP2_STREAMING_TS_MODE         2
#define EP2_STREAMING_ANALOG_ADC      3



#define INTERRUPT_MODE  USE_POLLING
/*************************************************************/
/*** Platform specific part of the Bridge Context structure.**/
/*************************************************************/

#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
#include "Dib07x0.h"
#include "I2c.h"
#endif
#if (DIBCOM_TEST_MODE == TEST_MODE_SPP)
#include "I2c.h"
#endif
#if (DIBINTERF_PHY == DIBINTERF_PHY_I2C)
#include "I2c.h"
#endif


struct DibBridgeTargetServiceCtx
{
};

/* Specify we have a Target Context */
#define DIB_BRIDGE_TARGET_CTX    1

struct DibBridgeTargetCtx
{
   struct DibDriverContext *pDriverContext;

   /** High Priority thread variables */
   DIB_LOCK                IrqLock;
   pthread_t               IrqThread;
   struct sched_param      IrqSched;
   pthread_attr_t          IrqAttr;

#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
   uint8_t                 Interface;
   uint8_t                 StreamingMode;
   uint8_t                 AcquiMode;
   struct Dib07x0DeviceId  Id;      /* Device id                        */
   int32_t                 Unit;    /* Device number                    */
   int32_t                 CtrlFd;  /* Control file descriptor          */
   int32_t                 Ep2Fd;   /* USB : endpoint2 file descriptor  */
   int32_t                 BufFd;   /* PCIe: buffer file descriptor     */
   volatile uint8_t      * CtrlPtr; /* PCIe: control pointer            */
   volatile uint8_t      * BufPtr;  /* PCIe: buffer pointer             */
#endif
/*#if (DIBCOM_TEST_MODE == TEST_MODE_SPP)*/
#if (DIBINTERF_PHY == DIBINTERF_PHY_I2C)
   unsigned char DataBuffer[5+I2C_MAX_RDWR_SIZE];
#endif

};

#endif
