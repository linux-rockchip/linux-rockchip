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

#if ((DIB_BOARD_NIM3006xM == 1) || (DIB_BOARD_NIM3006xMD2 == 1) || (DIB_BOARD_NIM3006xMD3 == 1) ||(DIB_BOARD_NIM3006xMD4 == 1) ||(DIB_BOARD_NIM3006xH == 1) || (DIB_BOARD_NIM3006xAPM == 1) || (DIB_BOARD_NIM3006xAPMD2 == 1) || (DIB_BOARD_NIM3006xAPMD3 == 1) || (DIB_BOARD_NIM3006xAPMD4 == 1))

#include "DibDriverMessages.h"
#include "DibDriverDragonfly.h"
#include "DibBoardSelection.h"

#if (EMBEDDED_FLASH_SUPPORT == 0)
#ifdef _NSCD_
#include "../Firmware/firmware_nautilus_2_0-3006x_nscd.h"
#else
#ifdef _BCAS_
#include "../Firmware/firmware_nautilus_2_0-3006x_bcas.h"
#else
#include "../Firmware/firmware_nautilus_2_0-3006x.h"
#endif
#endif
#endif

#if 1 // test by dckim: default = 0
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

#include "DibMsgMac.h"
#include "DibMsgLayout.h"

#if ( (DIB_BOARD_NIM3006xH == 1) || (DIB_BOARD_NIM3006xAPM == 1) || (DIB_BOARD_NIM3006xAPMD2 == 1) || (DIB_BOARD_NIM3006xAPMD3 == 1) || (DIB_BOARD_NIM3006xAPMD4 == 1) )
static DIBSTATUS Board3006xAPSetRfSwitch(struct DibDriverContext * pContext, signed char chipId);
#endif

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
#define SMARTCARD_CHIP_NUMBER  0 /* the smartcard is connected on chip 0 if no flash onboard, on chip 1 if flash on board.*/
#endif

/***********************************************************/
/***                  GPIO Configuration                 ***/
/***********************************************************/
#define NIM3006X_NB_GPIO_MAX  10

/**** Default GPIO configuration ***/
#if (EMBEDDED_FLASH_SUPPORT == 1)

/*** Single Board ***/
static const enum GpioDefinition GpioDef_Nim3006xH[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_FLASH_CLK,            /* Flash clock */
      eGPIO_FLASH_TX,             /* Flash Transmission */
      eGPIO_FLASH_RX,             /* Flash Reception */
      eGPIO_FLASH_CS,             /* Flash Chip Select */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_CTRL_SW_2,            /* Rf Switch control 2 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};

/*** Diversity 4 Board Master Chip ***/
static const enum GpioDefinition GpioDef_Nim3006x_Master[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_FLASH_CLK,            /* Flash clock */
      eGPIO_FLASH_TX,             /* Flash Transmission */
      eGPIO_FLASH_RX,             /* Flash Reception */
      eGPIO_FLASH_CS,             /* Flash Chip Select */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_CTRL_SW_2,            /* Rf Switch control 2 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};

/*** Diversity 4 Board Slaves Chip ***/
static const enum GpioDefinition GpioDef_Nim3006x_Slaves[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_CTRL_SW_2,            /* Rf Switch control 2 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};

/*** Application Diversity 4 Board Master Chip ***/
static const enum GpioDefinition GpioDef_Nim3006xAP_Master[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_FLASH_CLK,            /* Flash clock */
      eGPIO_FLASH_TX,             /* Flash Transmission */
      eGPIO_FLASH_RX,             /* Flash Reception */
      eGPIO_FLASH_CS,             /* Flash Chip Select */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};

#else

/*** Single Board ***/
static const enum GpioDefinition GpioDef_Nim3006xH[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
#ifdef _BCAS_
      eGPIO_SMARTCARD_DETECTN,	/* GPIO 3 : Optional: SmartCard Shutdown */
#else
      eGPIO_NOT_CONNECTED,		/* GPIO 3 : Free */
#endif
      eGPIO_NOT_CONNECTED,		/* GPIO 4 : Free */
      eGPIO_CTRL_SW_1,			/* GPIO 5 : Rf Switch control 1 */
      eGPIO_NOT_CONNECTED,		/* GPIO 6 : Optional: SmartCard DetectN */
      eGPIO_NOT_CONNECTED,		/* GPIO 7 : Optional: SmartCard Sel VCC */
      eGPIO_NOT_CONNECTED,		/* GPIO 8 : Free */
      eGPIO_NOT_CONNECTED		/* GPIO 9 : Free */
};
/*** Diversity 4 Board Master Chip ***/
static const enum GpioDefinition GpioDef_Nim3006x_Master[NIM3006X_NB_GPIO_MAX] =
{
#if 1 // by infospace, dckim
		  eGPIO_SMARTCARD_I_O,		  /* Optional: SmartCard I/O */
		  eGPIO_SMARTCARD_RESET,	  /* Optional: SmartCard Reset */
		  eGPIO_SMARTCARD_CLK,		  /* Optional: SmartCard Clock */
#ifdef _BCAS_
		  eGPIO_SMARTCARD_DETECTN,		  /* Optional: SmartCard Off */
#else
		  eGPIO_NOT_CONNECTED,
#endif
      eGPIO_NOT_CONNECTED,		/* GPIO 4 : Free */
      eGPIO_CTRL_SW_1,		/* GPIO 5 : Free */
      eGPIO_DIBCTRL_CLK,			/* GPIO 6 : DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,			/* GPIO 7 : DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,				/* GPIO 8 : Free */
      eGPIO_UNDEFINED				/* GPIO 9 : Free */
#else
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
      eGPIO_SMARTCARD_SHUTDOWN,   /* Optional: SmartCard Shutdown */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_CTRL_SW_2,            /* Rf Switch control 2 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
#endif
};
/*** Diversity 4 Board Slaves Chip ***/
static const enum GpioDefinition GpioDef_Nim3006x_Slaves[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_CTRL_SW_2,            /* Rf Switch control 2 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};

/*** Application Diversity 4 Board Master Chip ***/
static const enum GpioDefinition GpioDef_Nim3006xAP_Master[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_NOT_CONNECTED,        /* Free */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};
#endif

/*** Application Diversity 4 Board Slave0 Chip ***/
static const enum GpioDefinition GpioDef_Nim3006xAP_Slaves[NIM3006X_NB_GPIO_MAX] =
{
      eGPIO_SMARTCARD_I_O,        /* Optional: SmartCard I/O */
      eGPIO_SMARTCARD_RESET,      /* Optional: SmartCard Reset */
      eGPIO_SMARTCARD_CLK,        /* Optional: SmartCard Clock */
      eGPIO_SMARTCARD_SHUTDOWN,   /* Optional: SmartCard Shutdown */
      eGPIO_CTRL_SW_1,            /* Rf Switch control 1 */
      eGPIO_DIBCTRL_CLK,          /* DibCtrl InterChip Bus: don't change */
      eGPIO_DIBCTRL_DATA,         /* DibCtrl InterChip Bus: don't change */
      eGPIO_UNDEFINED,            /*  */
      eGPIO_UNDEFINED             /*  */
};

static const enum GpioDefinition * GpioTabAP[4] =
{
      GpioDef_Nim3006xAP_Master,
      GpioDef_Nim3006xAP_Slaves,
      GpioDef_Nim3006xAP_Slaves,
      GpioDef_Nim3006xAP_Slaves
};

static const enum GpioDefinition * GpioTab[4] =
{
      GpioDef_Nim3006x_Master,
      GpioDef_Nim3006x_Slaves,
      GpioDef_Nim3006x_Slaves,
      GpioDef_Nim3006x_Slaves
};

static DIBSTATUS Board3006XSetGpioConfig(struct DibDriverContext * pContext, uint8_t chipId)
{
   int i = 0;
   struct MsgGpioDefList gpioMsg;
   DIBSTATUS Ret;
   uint32_t GpioData;

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

      if (pContext->BoardType == eBOARD_NIM_3006xH)
      {
          /*** Board NIM3006xH Single ***/
          gpioMsg.GpioDefList[i].GpioDef =  GpioDef_Nim3006xH[i];
      }
      else if (    (pContext->BoardType == eBOARD_NIM_3006xM)
                || (pContext->BoardType == eBOARD_NIM_3006xMD2)
                || (pContext->BoardType == eBOARD_NIM_3006xMD3)
                || (pContext->BoardType == eBOARD_NIM_3006xMD4) )
      {
          /*** Board NIM3006xMdx Single, div 2, div 3, div 4 ***/
        if (chipId == 0 && i == 8) /* GPIO 8 of the master device is used to deactivate the POWER_DOWN of the slave-devices */
          gpioMsg.GpioDefList[i].GpioDef =  eGPIO_SLAVE_DEVICE_POWER_DOWN;
        else
          gpioMsg.GpioDefList[i].GpioDef = GpioTab[chipId][i];
      }
      else if (    (pContext->BoardType == eBOARD_NIM_3006xAPM)
                || (pContext->BoardType == eBOARD_NIM_3006xAPMD2)
                || (pContext->BoardType == eBOARD_NIM_3006xAPMD3)
                || (pContext->BoardType == eBOARD_NIM_3006xAPMD4) )
      {
          /*** Application Board NIM3006xMdx Single, div 2, div 3, div 4 ***/
        if (chipId == 0 && i == 8) /* GPIO 8 of the master device is used to deactivate the POWER_DOWN of the slave-devices */
          gpioMsg.GpioDefList[i].GpioDef =  eGPIO_SLAVE_DEVICE_POWER_DOWN;
        else
          gpioMsg.GpioDefList[i].GpioDef = GpioTabAP[chipId][i];
      }
   }

   MsgGpioDefListPackInit(&gpioMsg, &pContext->TxSerialBuf);

   DibResetEvent(&pContext->MsgAckEvent);
   DibDriverSendMessage(pContext, pContext->TxBuffer, gpioMsg.Head.MsgSize * 4);
   Ret = IntDriverDragonflyWaitForMsgAck(pContext, 2000);
   if(Ret == DIBSTATUS_SUCCESS)
   {
      //DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Gpio configured on Chip %d" CRA, chipId));
	  
	  DBG("Board Layout: Gpio configured on Chip %d: 0x%08X\n", chipId, chipId); // by infospace, dckim
   }
   else
   {
      //DIB_DEBUG(SOFT_IF_LOG, (CRB "Board Layout: Error Gpio Config on Chip %d" CRA, chipId));
	  
	   DBG("Board Layout: Error Gpio configured on Chip %d: 0x%08X\n", chipId, chipId); // by infospace, dckim
   }
#if 1 //def _BCAS_
	Ret = DibDriverReadReg32(pContext, 0x800000A4, &GpioData);

	DBG("Enable CAS 0: 0x%08X\n", GpioData); // by infospace, dckim

	GpioData |= (1<<9);												/* GPIO_9 => Output Mode(CAS Enable)  */
	GpioData &= ~(1<<3);											/* GPIO_3 => Input Mode(Card Detection) */

	//DBG("Enable CAS 1: 0x%08X\n", GpioData); // by infospace, dckim

	Ret = DibDriverWriteReg32(pContext, 0x800000A4, GpioData);

	//Ret = DibDriverReadReg32(pContext, 0x800000A0, &GpioData);

	//DBG("Enable CAS 2: 0x%08X\n", GpioData); // by infospace, dckim

	///GpioData |= (1<<9);												/* GPIO9 -> High(CAS Enable)					*/

	//Ret = DibDriverWriteReg32(pContext, 0x800000A0, GpioData);
#endif

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

   switch (pContext->BoardType)
   {
   case eBOARD_NIM_3006xM:
   case eBOARD_NIM_3006xH:
   case eBOARD_NIM_3006xAPM:
      nbslavechips = 0;
       break;
   case eBOARD_NIM_3006xMD2:
   case eBOARD_NIM_3006xAPMD2:
       nbslavechips = 1;
       break;
   case eBOARD_NIM_3006xMD3:
   case eBOARD_NIM_3006xAPMD3:
       nbslavechips = 2;
       break;
   case eBOARD_NIM_3006xMD4:
   case eBOARD_NIM_3006xAPMD4:
       nbslavechips = 3;
       break;
   default:
      return DIBSTATUS_ERROR;
   }

   /*******************************************************/
   /****  Master Configuration  ***************************/
   /*******************************************************/
   Ret = Board3006XGlobalInit(pContext, 0, nbslavechips);

   if (Ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Master Configuration Failed" CRA));
      return Ret;
   }

#if ( (DIB_BOARD_NIM3006xH == 1) ||(DIB_BOARD_NIM3006xAPM == 1) ||(DIB_BOARD_NIM3006xAPMD2 == 1) ||(DIB_BOARD_NIM3006xAPMD3 == 1) ||(DIB_BOARD_NIM3006xAPMD4 == 1) )
   if ( (pContext->BoardType == eBOARD_NIM_3006xH)
          || (pContext->BoardType == eBOARD_NIM_3006xAPM)
          || (pContext->BoardType == eBOARD_NIM_3006xAPMD2)
          || (pContext->BoardType == eBOARD_NIM_3006xAPMD3)
          || (pContext->BoardType == eBOARD_NIM_3006xAPMD4) )
   {
     Ret = Board3006xAPSetRfSwitch(pContext, 0);
   }
   if (Ret != DIBSTATUS_SUCCESS)
   {
     DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Setting master RF switch failed" CRA));
   return Ret;
}
#endif

   detection_status = 1; /* detection_status: -1 => no more slave to be detected; 1 => pending; 2 => a slave has been detected */

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
            CHIP_MODEL_TX_DIV,
            HOST_IF_DIBCTRL,
            1,
            DIB_3006X,
            -1, // by infospace, dckim
            -1  // by infospace, dckim
         },
         0,
         5,
    };

       /* Probing the devices doesn't work with the 3006x, so we
        * hard-code the expected address: for now we assume that
        * divstr is always 1 except for the 3 chip */
       if (slave_index == 2)
           add_slave.deviceAddress = 0x40;
       else
           add_slave.deviceAddress = 0x3f;

       DIB_DEBUG(SOFT_IF_LOG, (CRB "Trying to detect the slave#%i with addr=%x" CRA, slave_index, slave_addr));

       MsgAddSlaveDevicePackInit(&add_slave, &pContext->TxSerialBuf);

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
           Ret = Board3006XGlobalInit(pContext, slave_index + 1, nbslavechips);

           if (Ret != DIBSTATUS_SUCCESS)
           {
               DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Slave %d Configuration Failed" CRA,slave_index));
               return Ret;
           }

#if ( (DIB_BOARD_NIM3006xH == 1) ||(DIB_BOARD_NIM3006xAPM == 1) ||(DIB_BOARD_NIM3006xAPMD2 == 1) ||(DIB_BOARD_NIM3006xAPMD3 == 1) ||(DIB_BOARD_NIM3006xAPMD4 == 1) )
           if ( (pContext->BoardType == eBOARD_NIM_3006xH)
               || (pContext->BoardType == eBOARD_NIM_3006xAPM)
               || (pContext->BoardType == eBOARD_NIM_3006xAPMD2)
               || (pContext->BoardType == eBOARD_NIM_3006xAPMD3)
               || (pContext->BoardType == eBOARD_NIM_3006xAPMD4) )
           {
             Ret = Board3006xAPSetRfSwitch(pContext, slave_index + 1 );
           }
           if (Ret != DIBSTATUS_SUCCESS)
    {
             DIB_DEBUG(SOFT_IF_ERR, (CRB "Board Layout: Setting master RF switch failed" CRA));
             return Ret;
           }
#endif
       } else
           detection_status = -1;
    }
    return Ret;
}

#if ( (DIB_BOARD_NIM3006xH == 1) ||(DIB_BOARD_NIM3006xAPM == 1) ||(DIB_BOARD_NIM3006xAPMD2 == 1) ||(DIB_BOARD_NIM3006xAPMD3 == 1) ||(DIB_BOARD_NIM3006xAPMD4 == 1) )
static DIBSTATUS Board3006xAPSetRfSwitch(struct DibDriverContext * pContext, signed char chipId)
{
    DIBSTATUS Ret = DIBSTATUS_SUCCESS;

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
#endif

#if (EMBEDDED_FLASH_SUPPORT == 0)
struct DibDemodBoardConfig ConfigNim3006xxx=
{
   DIB_DEMOD_9000,
   DIB_TUNER_0090,
   DIB_NAUTILUS,	/* DIB_NAUTILUS           */
   VERSION(2,1),	/* NAUTILUS version       */
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
#endif
