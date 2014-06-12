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
* @file "DibBoardNim3006xxx.h"
* @brief 3006x board parameters.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (DIB_BOARD_NIM3006X)

#include "DibDriverMessages.h"
#include "DibDriverDragonfly.h"
#include "DibBoardSelection.h"
#include "../Firmware/firmware_nautilus_2_0.h"
#include "DibMsgMac.h"
#include "DibMsgLayout.h"

#if (EMBEDDED_FLASH_SUPPORT == 1)
/***********************************************************/
/***                  Flash Location                     ***/
/***********************************************************/
#define FLASH_CHIP_NUMBER 0       /*indicates the chip number (0 to 3) on which the flash is connected*/
#endif

/***********************************************************/
/***               SmartCard Location                    ***/
/***********************************************************/

#if ((defined _NSCD_)||(defined _BCAS_))
#define SMARTCARD_CHIP_NUMBER  1 /* the smartcard is connected on chip 0 if no flash onboard, on chip 1 if flash on board.*/
#endif

/***********************************************************/
/***                  GPIO Configuration                 ***/
/***********************************************************/
#define NIM3006X_NB_GPIO_MAX  10

/**** GPIO configuration ***/
#if (EMBEDDED_FLASH_SUPPORT == 1)
static const enum GpioDefinition GpioDef_Nim3006X[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_FLASH_CLK,            /* Flash clock */
      eGPIO_FLASH_TX,             /* Flash Transmission */
      eGPIO_FLASH_RX,             /* Flash Reception */
      eGPIO_FLASH_CS,             /* Flash Chip Select */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};
#else
static const enum GpioDefinition GpioDef_Nim3006X[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_TUNER_RESET,          /* Tuner reset: don't change */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED         /* Free */
};
#endif
static DIBSTATUS Board3006XSetGpioConfig(struct DibDriverContext * pContext, uint8_t chipId)
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
   gpioMsg.GpioSize        = NIM3006X_NB_GPIO_MAX;

   for(i = 0; i < NIM3006X_NB_GPIO_MAX; i++)
   {
      gpioMsg.GpioDefList[i].GpioPin = i;
      gpioMsg.GpioDefList[i].GpioDef =  GpioDef_Nim3006X[i];
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
static DIBSTATUS Board3006XSetClockConfig(struct DibDriverContext * pContext, uint8_t chipId, uint8_t forwardClk)
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

static DIBSTATUS Board3006XGlobalInit(struct DibDriverContext * pContext, uint8_t ChipId, uint8_t nbSlaveChips)
{
   DIBSTATUS Ret;

   /*** Setup GPIO **/
   Ret = Board3006XSetGpioConfig(pContext, ChipId);
   if (Ret != DIBSTATUS_SUCCESS)
      return Ret;

   /*** Setup PLL **/
   Ret = Board3006XSetClockConfig(pContext, ChipId, ChipId != nbSlaveChips);
   if (Ret != DIBSTATUS_SUCCESS)
      return Ret;

   return Ret;
}

DIBSTATUS Board3006XLayoutInit(struct DibDriverContext *pContext)
{
   DIBSTATUS Ret;
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
         DIB_3006X,
#if (EMBEDDED_FLASH_SUPPORT == 1)
         FLASH_CHIP_NUMBER ,
#else
         -1,
#endif
#if ((defined _NSCD_)||(defined _BCAS_))
         SMARTCARD_CHIP_NUMBER
#else
         -1
#endif
      }
   };

   MsgSetBasicLayoutInfoPackInit(&basic, &pContext->TxSerialBuf);
   DibResetEvent(&pContext->MsgAckEvent);
   Ret = DibDriverSendMessage(pContext, pContext->TxBuffer, basic.Head.MsgSize * 4);
   DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Master Set chip id, status = %d" CRA, Ret));
   Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);

   if (Ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Master Init Failed" CRA));
      return Ret;
   }

   if (pContext->BoardType == eBOARD_NIM_3006X)
      nbslavechips = 0;
   else
      return DIBSTATUS_ERROR;

   /*******************************************************/
   /****  Master Configuration  ***************************/
   /*******************************************************/
   Ret = Board3006XGlobalInit(pContext, 0, nbslavechips);

   if (Ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Master Configuration Failed" CRA));
      return Ret;
   }

   detection_status = 1;

   return Ret;
}

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

struct DibDemodBoardConfig ConfigNim3006X=
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
   Board3006XLayoutInit,
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
