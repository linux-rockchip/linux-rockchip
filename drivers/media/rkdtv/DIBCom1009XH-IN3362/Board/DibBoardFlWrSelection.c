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
* @file "DibBoardFlWrSelection.c"
* @brief Supported reference Design.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if ((EMBEDDED_FLASH_SUPPORT == 1) && (WRITE_FLASH_SUPPORT != eWRFL_NONE))
#include "DibBoardFlWrSelection.h"
#include "DibBoardParameters.h"
#include "DibStruct.h"

extern struct DibDemodPllCfg9000 DibPllConfig9080;

#if (DIB_BOARD_NIM3009xHx == 1) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30092MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1) || (DIB_BOARD_NIM3009xAPMD2 == 1) || (DIB_BOARD_NIM3006X == 1) || (DIB_BOARD_NIM3006xM == 1) || (DIB_BOARD_NIM3006xMD2 == 1) || (DIB_BOARD_NIM3006xMD3 == 1) || (DIB_BOARD_NIM3006xMD4 == 1) || (DIB_BOARD_NIM3006xH == 1) || (DIB_BOARD_NIM3006xAPM == 1) || (DIB_BOARD_NIM3006xAPMD2 == 1) || (DIB_BOARD_NIM3006xAPMD3 == 1) || (DIB_BOARD_NIM3006xAPMD4 == 1)
#include "../Firmware/flwr_nautilus_2_0.h"
#endif

#if (DIB_BOARD_NIM3009xHx == 1) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30092MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1) || (DIB_BOARD_NIM3009xAPMD2 == 1) || (DIB_BOARD_NIM3006X == 1) || (DIB_BOARD_NIM3006xM == 1) || (DIB_BOARD_NIM3006xMD2 == 1) || (DIB_BOARD_NIM3006xMD3 == 1) || (DIB_BOARD_NIM3006xMD4 == 1) || (DIB_BOARD_NIM3006xH == 1) || (DIB_BOARD_NIM3006xAPM == 1) || (DIB_BOARD_NIM3006xAPMD2 == 1) || (DIB_BOARD_NIM3006xAPMD3 == 1) || (DIB_BOARD_NIM3006xAPMD4 == 1)
struct DibDemodBoardConfig ConfigFlWrNim3009xxx =
{
   DIB_DEMOD_9000,
   DIB_TUNER_0090,
   DIB_NAUTILUS,  /* DIB_NAUTILUS           */
   VERSION(2,0),  /* NAUTILUS version       */
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
   NULL,
   {
      /* FeCfg */
      {
         /* FeCfg[0] */
         {
           /* UDemod */
           { 0 }, /* Dib7000 */
           /* Dib9000 */
           {
           sizeof(FlwrArray_nautilus2), (const uint8_t*)FlwrArray_nautilus2,
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
#endif /* (DIB_BOARD_NIM_3009cHx) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30092MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1) */

#if (DIB_BOARD_NIM1009xHx == 1) || (DIB_BOARD_NIM10096MD2 == 1) || (DIB_BOARD_NIM10096MD3 == 1) || (DIB_BOARD_NIM10096MD4 == 1) || (DIB_BOARD_NIM1009xAPMD2 == 1)

#include "../Firmware/flwr_nautilus_1_1.h"

struct DibDemodBoardConfig ConfigFlWrNim1009xxx =
{
   DIB_DEMOD_9000,
   DIB_TUNER_0090,
   DIB_NAUTILUS,  /* DIB_NAUTILUS           */
   VERSION(1,1),  /* NAUTILUS version       */
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
   NULL,
   {
      /* FeCfg */
      {
         /* FeCfg[0] */
         {
           /* UDemod */
           { 0 }, /* Dib7000 */
           /* Dib9000 */
           {
           sizeof(FlwrArray_nautilus11), (const uint8_t*)FlwrArray_nautilus11,
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

#endif /* (DIB_BOARD_NIM10096MD2 == 1) || (DIB_BOARD_NIM10096MD3 == 1) || (DIB_BOARD_NIM10096MD4 == 1) */

#if (DIB_BOARD_NIM3006X) || (DIB_BOARD_NIM3006xM == 1) || (DIB_BOARD_NIM3006xMD2 == 1) || (DIB_BOARD_NIM3006xMD3 == 1) || (DIB_BOARD_NIM3006xMD4 == 1) || (DIB_BOARD_NIM3006xH == 1) || (DIB_BOARD_NIM3006xAPM == 1) || (DIB_BOARD_NIM3006xAPMD2 == 1) || (DIB_BOARD_NIM3006xAPMD3 == 1) || (DIB_BOARD_NIM3006xAPMD4 == 1)

struct DibDemodBoardConfig ConfigFlWrNim3006X=
{
   DIB_DEMOD_9000,
   DIB_TUNER_0090,
   DIB_NAUTILUS,  /* DIB_NAUTILUS           */
   VERSION(2,1),  /* NAUTILUS version       */
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
   NULL,
   {
      /* FeCfg */
      {
         /* FeCfg[0] */
         {
           /* UDemod */
           { 0 }, /* Dib7000 */
           /* Dib9000 */
           {
           sizeof(FlwrArray_nautilus2), (const uint8_t*)FlwrArray_nautilus2,
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




struct DibDemodBoardConfig *BoardFlwrList[eMAX_NB_BOARDS] =
{
   NULL, /* DIB_BOARD_SDIO7078 */
   NULL, /* DIB_BOARD_SDIO7070 */
   NULL, /* DIB_BOARD_SDIO9080 */
   NULL, /* DIB_BOARD_STK7078 */
   NULL, /* DIB_BOARD_NIM9080MD */
   NULL, /* DIB_BOARD_STK7078MD4 */
   NULL, /* DIB_BOARD_SDIO9090 */
   NULL, /* DIB_BOARD_NIM9090MD*/
   NULL, /* DIB_BOARD_SDIO19088 */
   NULL, /* DIB_BOARD_NIM29098 */
   NULL, /* DIB_BOARD_STK29098MD4*/
   NULL, /* DIB_BOARD_MOD29098SHA*/
   NULL,
   NULL, /* DIB_BOARD_NIM9090H */

#if (DIB_BOARD_NIM1009xHx == 1)
   &ConfigFlWrNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD2 == 1)
   &ConfigFlWrNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD3 == 1)
   &ConfigFlWrNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD4 == 1)
   &ConfigFlWrNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3009xHx == 1)
   &ConfigFlWrNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD2 == 1)
   &ConfigFlWrNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD3 == 1)
   &ConfigFlWrNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD4 == 1)
   &ConfigFlWrNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xM == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xMD2 == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xMD3 == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xMD4 == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xH == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM1009xAPMD2 == 1)
   &ConfigFlWrNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3009xAPMD2 == 1)
   &ConfigFlWrNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xAPM == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xAPMD2 == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xAPMD3 == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006xAPMD4 == 1)
   &ConfigFlWrNim3006X,
#else
   NULL,
#endif
};

#endif
