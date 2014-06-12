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
* @file "DibBoardNim30092MD4.h"
* @brief 30092 board parameters.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (DIB_BOARD_NIM3009xHx) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30092MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1) || (DIB_BOARD_NIM3009xM2D2P2 == 1)

#include "DibDriverMessages.h"
#include "DibDriverDragonfly.h"
#include "DibBoardSelection.h"

#if (EMBEDDED_FLASH_SUPPORT == 0)
#ifdef _NSCD_
#include "../Firmware/firmware_nautilus_2_0_nscd.h"
#else
#ifdef _BCAS_
#include "../Firmware/firmware_nautilus_2_0_bcas.h"
#else
#include "../Firmware/firmware_nautilus_2_0.h"
#endif
#endif
#endif
#include "DibMsgMac.h"
#include "DibMsgLayout.h"

#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
static DIBSTATUS Board3009xM2D2P2SetRfSwitch(struct DibDriverContext * pContext, signed char chipId);
#endif

#if 0
static DIBSTATUS Board3009xxxSetRfSwitch(struct DibDriverContext * pContext);
static DIBSTATUS Board3009xxxSetGpio(struct DibDriverContext * pContext);
#endif

#if (EMBEDDED_FLASH_SUPPORT == 1)
/***********************************************************/
/***                  Flash Location                     ***/
/***********************************************************/
#define FLASH_CHIP_NUMBER 0       /*indicates the chip number (0 to 3) on which the flash is connected*/
#endif

/***********************************************************/
/***                  GPIO Configuration                 ***/
/***********************************************************/
#define NIM3009xxx_NB_GPIO_MAX  10

/**** GPIO configuration for Nim30092H2, NIM30092H1 and NIM30092H2 ***/
#if (EMBEDDED_FLASH_SUPPORT == 1)
static const enum GpioDefinition GpioDef_Nim3009xHx[NIM3009xxx_NB_GPIO_MAX] =
{
      eGPIO_FLASH_CLK,            /* Flash clock */
      eGPIO_FLASH_TX,             /* Flash Transmission */
      eGPIO_FLASH_RX,             /* Flash Reception */
      eGPIO_FLASH_CS,             /* Flash Chip Select */
      eGPIO_TUNER_RESET,          /* Tuner reset: don't change */
      eGPIO_CTRL_SW_1,            /* Not present on single but on diversity board ,
                                            defined to use a diversity as a single board*/
      eGPIO_SMARTCARD_SHUTDOWN,   /* Optional: SmartCard Shutdown */
      eGPIO_SMARTCARD_VDD,        /* Optional: SmartCard Power */
      eGPIO_CTRL_SW_2,            /* Not present on single but on diversity board ,
                                            defined to use a diversity as a single board*/
      eGPIO_NOT_CONNECTED         /* Free */
};

/**** GPIO configuration for Nim30096MD2,MD3 and MD4 ***/
static const enum GpioDefinition GpioDef_Master[NIM3009xxx_NB_GPIO_MAX] =
{
      eGPIO_FLASH_CLK,            /* Flash clock */
      eGPIO_FLASH_TX,             /* Flash Transmission */
      eGPIO_FLASH_RX,             /* Flash Reception */
      eGPIO_FLASH_CS,             /* Flash Chip Select */
      eGPIO_TUNER_RESET,          /* Tuner reset: don't change */
      eGPIO_CTRL_SW_1,            /* Optional: 1-> 4 RF Switch */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_CTRL_SW_2,            /* Optional: 1-> 4 RF Switch */
      eGPIO_NOT_CONNECTED         /* Free */
};
#else
static const enum GpioDefinition GpioDef_Nim3009xHx[NIM3009xxx_NB_GPIO_MAX] =
{
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
      eGPIO_SMARTCARD_DETECTN,        /* Optional: SmartCard Off */
      eGPIO_TUNER_RESET,          /* Tuner reset: don't change */
      eGPIO_CTRL_SW_1,            /* Not present on single but on diversity board ,
                                            defined to use a diversity as a single board*/
      eGPIO_SMARTCARD_SHUTDOWN,   /* Optional: SmartCard Shutdown */
      eGPIO_SMARTCARD_VDD,        /* Optional: SmartCard Power */
      eGPIO_CTRL_SW_2,            /* Not present on single but on diversity board ,
                                            defined to use a diversity as a single board*/
      eGPIO_NOT_CONNECTED         /* Free */
};

/**** GPIO configuration for Nim30096MD2,MD3 and MD4 ***/
static const enum GpioDefinition GpioDef_Master[NIM3009xxx_NB_GPIO_MAX] =
{
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
      eGPIO_SMARTCARD_DETECTN,        /* Optional: SmartCard Off */
      eGPIO_TUNER_RESET,          /* Tuner reset: don't change */
      eGPIO_CTRL_SW_1,            /* Optional: 1-> 4 RF Switch */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_CTRL_SW_2,            /* Optional: 1-> 4 RF Switch */
      eGPIO_NOT_CONNECTED         /* Free */
};
#endif /* (EMBEDDED_FLASH_SUPPORT == 1) */
/**** GPIO configuration for Slaves CHIPS on Nim30096MD2,MD3 and MD4 ***/
static const enum GpioDefinition GpioDef_Others[NIM3009xxx_NB_GPIO_MAX] =
{
      /*eGPIO_USER1,*/            /* GPIO 0 : set eGPIO_USER1 => example for Board3009xxxSetGpio() */
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
#ifndef _NSCD_
      eGPIO_SMARTCARD_DETECTN,        /* Optional: SmartCard Off */
#else
      eGPIO_NOT_CONNECTED,
#endif
      eGPIO_TUNER_RESET,          /* GPIO 4 : Tuner reset: don't change */
      eGPIO_CTRL_SW_1,            /* GPIO 5 : Optional: 1-> 4 RF Switch */
      eGPIO_DIBCTRL_CLK,          /* GPIO 6 : DibCtrl InterChip Bus: Not used on last chip */
      eGPIO_DIBCTRL_DATA,         /* GPIO 7 : DibCtrl InterChip Bus: Not used on last chip */
      eGPIO_CTRL_SW_2,            /* GPIO 8 : Optional: 1-> 4 RF Switch */
      eGPIO_NOT_CONNECTED
};


static const enum GpioDefinition * GpioTab[4] =
{
      GpioDef_Master,
      GpioDef_Others,
      GpioDef_Others,
      GpioDef_Others
};

#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
/**** GPIO configuration for Slave CHIP on Nim3009xM2D2P2 ***/
static const enum GpioDefinition GpioDef_Slave_NIM3009xM2D2P2[NIM3009xxx_NB_GPIO_MAX] =
{
      /*eGPIO_USER1,*/            /* GPIO 0 : set eGPIO_USER1 => example for Board1009xxxSetGpio() */
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
#ifndef _NSCD_
      eGPIO_SMARTCARD_DETECTN,        /* Optional: SmartCard Off */
#else
      eGPIO_NOT_CONNECTED,
#endif
      eGPIO_TUNER_RESET,          /* GPIO 4 : Tuner reset: don't change */
      eGPIO_CTRL_SW_1,            /* GPIO 5 : Optional: 1-> 4 RF Switch */
      eGPIO_SMARTCARD_SHUTDOWN,   /* Optional: SmartCard Shutdown */
      eGPIO_SMARTCARD_VDD,        /* Optional: SmartCard Power */
      eGPIO_CTRL_SW_2,            /* GPIO 8 : Optional: 1-> 4 RF Switch */
      eGPIO_NOT_CONNECTED
};

static const enum GpioDefinition * GpioTab_NIM3009xM2D2P2[2] =
{
      GpioDef_Master,
      GpioDef_Slave_NIM3009xM2D2P2
};
#endif /* (DIB_BOARD_NIM3009xM2D2P2 == 1) */

static DIBSTATUS Board3009xxxSetGpioConfig(struct DibDriverContext * pContext, uint8_t chipId)
{
   int i = 0;
   struct MsgGpioDefList gpioMsg;
   DIBSTATUS Ret;

   DibZeroMemory(&gpioMsg, sizeof(struct MsgGpioDefList));

   gpioMsg.Head.ChipId     = chipId;
   gpioMsg.Head.MsgId      = OUT_MSG_GPIO_DEF;
   gpioMsg.Head.MsgSize    = GetWords(MsgGpioDefListBits, 32);
   gpioMsg.Head.Sender     = HOST_IDENT;
   gpioMsg.Head.Type       = MSG_TYPE_LAYOUT;
   gpioMsg.GpioSize      = NIM3009xxx_NB_GPIO_MAX;

   for(i = 0; i < NIM3009xxx_NB_GPIO_MAX; i++)
   {
      gpioMsg.GpioDefList[i].GpioPin = i;

      if (pContext->BoardType == eBOARD_NIM_3009xHx)
         gpioMsg.GpioDefList[i].GpioDef = GpioDef_Nim3009xHx[i];
      else
#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
         if(pContext->BoardType == eBOARD_NIM_3009xM2D2P2)
            gpioMsg.GpioDefList[i].GpioDef = GpioTab_NIM3009xM2D2P2[chipId][i];
         else
#endif
         gpioMsg.GpioDefList[i].GpioDef = GpioTab[chipId][i];
   }

   MsgGpioDefListPackInit(&gpioMsg, &pContext->TxSerialBuf);

   DibResetEvent(&pContext->MsgAckEvent);
   DibDriverSendMessage(pContext, pContext->TxBuffer, gpioMsg.Head.MsgSize * 4);
   Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);
   if(Ret == DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Gpio configured on Chip %d" CRA, chipId));
   }
   else
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Error Gpio Config on Chip %d" CRA, chipId));
   }
   return Ret;
}

/***********************************************************/
/***                  Clock Configuration                 ***/
/***********************************************************/
static DIBSTATUS Board3009xxxSetClockConfig(struct DibDriverContext * pContext, uint8_t chipId, uint8_t forwardClk)
{
   DIBSTATUS Ret;

   /** clock configuration */
   struct MsgLayoutDiBxxx9xPllConfig clkMsg = {
      {
         GetWords(MsgLayoutDiBxxx9xPllConfigBits, 32),
         chipId,
         OUT_MSG_LAYOUT_CLOCK,
         HOST_IDENT,
         MSG_TYPE_LAYOUT
      },
      30000,      /*Crystal Clock*/
      0,          /*pll_bypass*/
      1,          /*pll_range*/
      1,          /*pll_prediv*/
      8,          /*pll_loopdiv*/
      8,          /*adc_clock_ratio*/
      0,          /*pll_int_loop_filt*/
      forwardClk, /*clkouttobamse*/
   };

   MsgLayoutDiBxxx9xPllConfigPackInit(&clkMsg, &pContext->TxSerialBuf);

   DibResetEvent(&pContext->MsgAckEvent);
   DibDriverSendMessage(pContext, pContext->TxBuffer, clkMsg.Head.MsgSize * 4);

   Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);
   if(Ret == DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Clock configured on Chip %d" CRA, chipId));
   }
   else
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Error Clock Config on Chip %d" CRA, chipId));
   }
   return Ret;
}

static DIBSTATUS Board3009xxxGlobalInit(struct DibDriverContext * pContext, uint8_t ChipId, uint8_t nbSlaveChips)
{
   DIBSTATUS Ret;

   /*** Setup GPIO **/
   Ret = Board3009xxxSetGpioConfig(pContext, ChipId);
   if (Ret != DIBSTATUS_SUCCESS)
      return Ret;

   /*** Setup PLL **/
   Ret = Board3009xxxSetClockConfig(pContext, ChipId, ChipId != nbSlaveChips);
   if (Ret != DIBSTATUS_SUCCESS)
      return Ret;

   return Ret;
}

DIBSTATUS Board3009xxxLayoutInit(struct DibDriverContext *pContext)
{
   DIBSTATUS Ret;
   uint8_t slave_index, slave_addr;
   int32_t detection_status;
   uint8_t nbslavechips;

   /*******************************************************/
   /****  Master Initialization ***************************/
   /*******************************************************/
   struct MsgSetBasicLayoutInfo basic = {
      {
         GetWords(MsgSetBasicLayoutInfoBits, 32),
         0,
         OUT_MSG_SET_BASIC_LAYOUT_INFO,
         HOST_IDENT,
         MSG_TYPE_MAC
      },
      {
         0,
         CHIP_MODEL_SPITS,
         HOST_IF_SPI,
         1,
         DIB_3009X,
#if (EMBEDDED_FLASH_SUPPORT == 1)
         FLASH_CHIP_NUMBER ,
#else
         -1,
#endif
#if ((defined _NSCD_)||(defined _BCAS_))
         /* SMARTCARD_CHIP_NUMBER: the smartcard is connected on chip 0 if no flash onboard, on chip 1 if flash on board.*/
         (pContext->BoardType == eBOARD_NIM_3009xM2D2P2) ? 1 : 0
#else
         -1
#endif
      }
   };
   DibResetEvent(&pContext->MsgAckEvent);
   MsgSetBasicLayoutInfoPackInit(&basic, &pContext->TxSerialBuf);
   Ret = DibDriverSendMessage(pContext, pContext->TxBuffer, basic.Head.MsgSize * 4);
   DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Master Set chip id, status = %d" CRA, Ret));
   Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);

   if (Ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Master Init Failed" CRA));
      return Ret;
   }

   if (pContext->BoardType == eBOARD_NIM_3009xHx)
     nbslavechips = 0;
   if ((pContext->BoardType == eBOARD_NIM_30092MD2) || (pContext->BoardType == eBOARD_NIM_3009xM2D2P2))
     nbslavechips = 1;
   else if (pContext->BoardType == eBOARD_NIM_30092MD3)
     nbslavechips = 2;
   else if (pContext->BoardType == eBOARD_NIM_30092MD4)
     nbslavechips = 3;


   /*******************************************************/
   /****  Master Configuration  ***************************/
   /*******************************************************/
   Ret = Board3009xxxGlobalInit(pContext, 0, nbslavechips);

   if (Ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Master Configuration Failed" CRA));
      return Ret;
   }

#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
   /* configure the RF switch of master */
   if (pContext->BoardType == eBOARD_NIM_3009xM2D2P2)
   {
      Ret = Board3009xM2D2P2SetRfSwitch(pContext, 0);
      if (Ret != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Setting master RF switch failed" CRA));
         return Ret;
      }
   }
#endif /* (DIB_BOARD_NIM3009xM2D2P2 == 1) */

   detection_status = 1;

   /*******************************************************/
   /****  Slave Chips Initialization **********************/
   /*******************************************************/
   for (slave_index = 0; slave_index < nbslavechips && detection_status > 0; slave_index++) {

      struct MsgAddSlaveDevice add_slave = {
         {
            GetWords(MsgAddSlaveDeviceBits, 32),
            MASTER_IDENT,
            OUT_MSG_ADD_SLAVE_DEVICE,
            HOST_IDENT,
            MSG_TYPE_MAC
         },
         {
            slave_index+1,
            ((slave_index == 1) & (pContext->BoardType == eBOARD_NIM_30092MD3)) ? CHIP_MODEL_MPEG1_TX_DIV : CHIP_MODEL_TX_DIV,
            HOST_IF_DIBCTRL,
            1,
            DIB_3009X,
         },
         0,
         5,
      };


	   detection_status = 1; /* detection_status: -1 => no more slave to be detected; 1 => pending; 2 => a slave has been detected */

	   for (slave_addr = 0x3f; slave_addr <= 0x40 && detection_status == 1; slave_addr++) {

		   DIB_DEBUG(SOFT_IF_LOG, (CRB "Trying to detect the slave#%i with addr=%x" CRA, slave_index, slave_addr));

		   add_slave.deviceAddress = slave_addr;

		   MsgAddSlaveDevicePackInit(&add_slave, &pContext->TxSerialBuf);

           DibResetEvent(&pContext->MsgAckEvent);
		   DibDriverSendMessage(pContext, pContext->TxBuffer, add_slave.Head.MsgSize * 4);
		   Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);

		   DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: CHIP %d: add slave, status = %d Ack: %d" CRA, slave_index+1, Ret, pContext->MsgAckStatus));

		   if (Ret == DIBSTATUS_SUCCESS && pContext->MsgAckStatus == DIBSTATUS_SUCCESS) {
			   /* a slave has been detected */
			   detection_status = 2;
			   DIB_DEBUG(SOFT_IF_LOG, (CRB "Detected and added the slave#%i with the address 0x%x" CRA, slave_index, slave_addr));

            /*******************************************************/
            /****  Slave Chips Configuration  **********************/
            /*******************************************************/
			   Ret = Board3009xxxGlobalInit(pContext, slave_index + 1, nbslavechips);

#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
               /* configure the RF switches of slave */
               if (pContext->BoardType == eBOARD_NIM_3009xM2D2P2)
               {
                  Ret = Board3009xM2D2P2SetRfSwitch(pContext, slave_index+1);
                  if (Ret != DIBSTATUS_SUCCESS)
                  {
                     DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Setting slave RF switch failed" CRA));
                     return Ret;
                  }
               }
#endif /* (DIB_BOARD_NIM3009xM2D2P2 == 1) */

               /* if needed you can configure the RF switch here */
               /*Board3009xxxSetRfSwitch(pContext);*/

            if (Ret != DIBSTATUS_SUCCESS)
            {
               DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Slave %d Configuration Failed" CRA,slave_index));
               return Ret;
            }
		   }
      }

	   if ((slave_addr > 0x40) && (detection_status == 1)) {
		   /* end of the chain, no more slave can be detected */
		   DIB_DEBUG(SOFT_IF_LOG, (CRB "no more slaves to be found" CRA));
		   detection_status = -1;
	   }
   }

   return Ret;
}

#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
static DIBSTATUS Board3009xM2D2P2SetRfSwitch(struct DibDriverContext * pContext, signed char chipId)
{
    DIBSTATUS Ret = DIBSTATUS_SUCCESS;

    /** set Gpio 5 to be 1 if freqTuned < 470 MHz and 1 if freqTuned >= 470 MHz */
    struct MsgRfSwitchControl activMsg =
    {
        {
            GetWords(MsgRfSwitchControlBits, 32),
            chipId,
            OUT_MSG_RF_SWITCH_CONTROL,
            HOST_IDENT,
            MSG_TYPE_LAYOUT
        },{
            {
                380, /* for 0 MHz < freq < 380 Mhz */
                1,   /* eGPIO_CTRL_SW_1 (GPIO 5 by default) will be 1 */
                0,   /* eGPIO_CTRL_SW_2 (GPIO 8 by default) not connected */
            },
            {
                900, /* for 380 MHz < freq < 900 Mhz */
                0,   /* eGPIO_CTRL_SW_1 will be 0 */
                0,   /* eGPIO_CTRL_SW_1 not connected */
            },
            {0,0,0},
            {0,0,0},
            {0,0,0},
            {0,0,0}}
    };
    DibResetEvent(&pContext->MsgAckEvent);
    MsgRfSwitchControlPackInit(&activMsg, &pContext->TxSerialBuf);

    DibDriverSendMessage(pContext, pContext->TxBuffer, activMsg.Head.MsgSize * 4);
    Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);
    return Ret;
}
#endif /* (DIB_BOARD_NIM3009xM2D2P2 == 1) */

#if 0
static DIBSTATUS Board3009xxxSetGpio(struct DibDriverContext * pContext)
{
    DIBSTATUS Ret = DIBSTATUS_SUCCESS;

    /** set gpio0 of chip 4 */
    struct MsgGpioActivation activMsg =
    {
        {
            GetWords(MsgGpioActivationBits, 32),
            /* Chip Id = 3 for chip 4*/
            3,
            OUT_MSG_GPIO_ACTIVATION,
            HOST_IDENT,
            MSG_TYPE_LAYOUT
        },
        /* eGPIO_USER1 has to be set at the right place in enum
         * GpioDefinition */
        eGPIO_USER1,
        /* Set this Gpio to 1 */
        1,
    };
    MsgGpioActivationPackInit(&activMsg, &pContext->TxSerialBuf);

    DibResetEvent(&pContext->MsgAckEvent);
    DibDriverSendMessage(pContext, pContext->TxBuffer, activMsg.Head.MsgSize * 4);
    Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);
    if(Ret == DIBSTATUS_SUCCESS)
    {
        DIB_DEBUG(SOFT_IF_ERR, (CRB "Gpio0 chip 3 configured" CRA));
    }
    return Ret;
}

static DIBSTATUS Board3009xxxSetRfSwitch(struct DibDriverContext * pContext)
{
    DIBSTATUS Ret = DIBSTATUS_SUCCESS;

    /** set Gpio 5 & 8 of chip 4 to be 0 if freqTuned < 490 MHz and 1 if freqTuned > 490 MHz */
    struct MsgRfSwitchControl activMsg =
    {
        {
            GetWords(MsgRfSwitchControlBits, 32),
            /* ChipID = 3 for chip 4 */
            3,
            OUT_MSG_RF_SWITCH_CONTROL,
            HOST_IDENT,
            MSG_TYPE_LAYOUT
        },{
            {
                490, /* for 0 MHz < freq < 490 Mhz */
                0,   /* eGPIO_CTRL_SW_1 (GPIO 5 by default) will be 0 */
                0,   /* eGPIO_CTRL_SW_2 (GPIO 8 by default) will be 0 */
            },
            {
                850, /* for 491 MHz < freq < 850 Mhz */
                1,   /* eGPIO_CTRL_SW_1 will be 1 */
                1,   /* eGPIO_CTRL_SW_1 will be 1 */
            },
            {0,0,0},
            {0,0,0},
            {0,0,0},
            {0,0,0}}
    };
    MsgRfSwitchControlPackInit(&activMsg, &pContext->TxSerialBuf);

    DibResetEvent(&pContext->MsgAckEvent);
    DibDriverSendMessage(pContext, pContext->TxBuffer, /*activMsg.Head.MsgSize * 4*/sizeof(activMsg));
    Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);
    if(Ret == DIBSTATUS_SUCCESS)
    {
        DIB_DEBUG(SOFT_IF_ERR, (CRB "Ctrl switch chip 3 configured" CRA));
    }
    return Ret;
}
#endif
#if (EMBEDDED_FLASH_SUPPORT == 0)
struct DibDemodBoardConfig ConfigNim3009xxx =
{
   DIB_DEMOD_9000,
   DIB_TUNER_0090,
   DIB_NAUTILUS,	/* DIB_NAUTILUS           */
   VERSION(2,0),	/* NAUTILUS version       */
   4,             /* NbFrontends            */
   0,             /* SramDelayAddLatch      */
   2,             /* Hbm = eAUTO            */
   /* 8, */       /* BoardConfig */ /* for 26MHz crystal */
   100,           /* BoardConfig            */ /* for 30MHz crystal */
   0x0805,        /* RegSramCfg1805         */
   70,            /* DefAgcDynamicsDb       */
   0,             /* NO LNA                 */
   0,             /* 0 Lna triggers         */
   NULL,          /* Lna Trigger table      */
   {0, 0},        /* Gpio default direction */
   {0, 0},        /* Gpio default value     */
   Board3009xxxLayoutInit,
   {
      /* FeCfg */
      {
         /* FeCfg[0] */
         {
           /* UDemod */
           { 0 }, /* Dib7000 */
           /* Dib9000 */
           {
           sizeof(FirmwareArray), (const uint8_t*)FirmwareArray,
           0, NULL,
           NULL,
           &DibPllConfig9080,
           /* AGCdrain AGC drv     AGC slew    I2C drv     I2C slew   IOCLK dr   IOCLK sl   HOST drv   HOST slew  SRAM drv   SRAM slew */
           (1 << 15) | (1 << 13) | (0 << 12) | (1 << 10) | (0 << 9) | (1 << 7) | (0 << 6) | (3 << 4) | (0 << 3) | (0 << 1) | (0),
           },
        },
        {   /* Gpio */
           { GPIO_FUNC_INIT,      0xffff, 0x001e, 0x0021 },
           { GPIO_FUNC_TUNER_ON,  0X0031, 0x0000, 0x0030 },
           { GPIO_FUNC_TUNER_OFF, 0X0031, 0x0000, 0x0021 },
        },
        {   /* SubBand */
           0, 0, 0,
           {
           { 0 }, /* subband[0] */
        }
      },
      18,
      0,  /* TunerI2CAdd */
      {   /* UTuner */
         {0},           /* Dib0070 */
         {0, 0, 30000}, /* Dib0080 */
      },

      0, /* No PMUIsPresent */
      {
         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
      },
    },
    {    /* FeCfg[1] */
      {
         /* UDemod */
         {
            /* Dib7000 */
            0, NULL, /* No microcode on the slave */
            0, NULL, /* No microcode on the slave */
            0,       /* HostBusDiversity          */
            0,       /* TunerIsBaseband           */
            0,
            0,
            0,
         },
         {0}, /* Dib9000 */
         },
         {   /* Gpio */
            { GPIO_FUNC_INIT, 0, 0, 0},
            { GPIO_FUNC_INIT, 0, 0, 0},
            { GPIO_FUNC_INIT, 0, 0, 0},
            { GPIO_FUNC_INIT, 0, 0, 0}
         },
         {   /* SubBand */
            0, 0, 0,
            {
            {0}, /* SubBand[0] */
            }
         },
         0,
         0,
         {   /* UTuner */
         {0, 0, 0, 0, 0},  /* Dib0070 */
         {0},              /* Dib0080 */
         },
      },
   }
};

#endif


#endif
