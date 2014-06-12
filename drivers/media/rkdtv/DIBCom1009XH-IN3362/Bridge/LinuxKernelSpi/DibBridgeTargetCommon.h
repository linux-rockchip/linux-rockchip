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

//#include "DibBridgeTargetModule.h"
#include "linux/list.h"
#include <linux/version.h>

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
#define DIBINTERF_PHY               DIBINTERF_PHY_SPITS

#define DIBCTRL_DEFAULT_ADDR  0x40
#define DIBCTRL_DIV_ADDR      0x3F
#define DIBCTRL_ADDR DIBCTRL_DEFAULT_ADDR

/*** Test mode used internally to verify host interface using Dib0700 USB bridge **/
#define TEST_MODE_NONE		0	
#define TEST_MODE_HOOK		1	
#define TEST_MODE_SPP		  2	
#define DIBCOM_TEST_MODE 	TEST_MODE_NONE

#define USE_SDIOSPI_MODE	      0
#define USE_SDIOSPI_WORKAROUND	1
#define USE_DIBSPI_WORKAROUND	  0

/* option to read big Data blocks over EP2 */
#define WITH_EP2_STREAMING    1

/***** IRQ MODE *******************************************/
#define INTERRUPT_MODE  USE_POLLING

/*************************************************************/
/*** Platform specific part of the DIB_DEVICE structure. ****/
/*************************************************************/
#define DIB_MAX_QUEUED  (1000)

typedef struct kthread_struct 
{
  struct task_struct *thread;
  struct work_struct  work;
  struct semaphore    StartStopSem;
  wait_queue_head_t   queue;
  int32_t             terminate;
  void               *arg;
} kthread_t;

struct DibBridgeTargetServiceCtx
{
};

/* Specify we have a Target Context */
#define DIB_BRIDGE_TARGET_CTX    1
struct DibBridgeTargetCtx
{
   //struct net_device       *pDibBridgeTargetNet;
   struct DibDriverContext *pDriverContext;
   #if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
   struct delayed_work      DibWork;
   #else
   struct work_struct       DibWork;
   #endif
   struct workqueue_struct  *DibWorkQueue;

   DIB_LOCK          IoctlLock;
   DIB_LOCK          IrqLock;
   kthread_t         IrqPollThread;
   struct IoctlHead *IoctlReq;
   int32_t           SubMode; 
   wait_queue_head_t wait_queue;               /**< queue where to put the thread when sleeping */

   /* local state : fd of the current Hook control node */
   struct Dib07x0Dev   *CtrlDev;
   struct Dib07x0State *state;
   struct Dib07x0Dev   *Ep2Dev;
   int32_t              Id;
};

struct my_sk_buff 
{
  struct sk_buff  *skb;
  struct list_head list;
};

#endif
