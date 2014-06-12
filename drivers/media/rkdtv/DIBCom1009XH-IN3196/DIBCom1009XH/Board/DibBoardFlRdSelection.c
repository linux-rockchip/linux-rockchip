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
* @file "DibBoardSelection.c"
* @brief Supported reference Design.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (EMBEDDED_FLASH_SUPPORT == 1)
#include "DibBoardFlRdSelection.h"
#include "DibBoardParameters.h"
#include "DibStruct.h"

extern struct DibDemodPllCfg9000 DibPllConfig9080;

#if (DIB_BOARD_NIM3009xHx == 1) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30092MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1) || (DIB_BOARD_NIM3006X == 1)  || (DIB_BOARD_NIM3009xM2D2P2 == 1)
#include "../Firmware/flrd_nautilus_2_0.h"
#endif

#if (DIB_BOARD_NIM3009xHx == 1) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30092MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1) || (DIB_BOARD_NIM3009xM2D2P2 == 1)

extern DIBSTATUS Board3009xxxLayoutInit(struct DibDriverContext *pContext);

struct DibDemodBoardConfig ConfigFlRdNim3009xxx =
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
           sizeof(FlrdArray_nautilus2), (const uint8_t*)FlrdArray_nautilus2,
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
#endif /*(DIB_BOARD_NIM_3009xHx) || (DIB_BOARD_NIM30092MD2 == 1) || (DIB_BOARD_NIM30092MD3 == 1) || (DIB_BOARD_NIM30092MD4 == 1) */

#if  (DIB_BOARD_NIM1009xHx == 1) ||  (DIB_BOARD_NIM10096MD2 == 1) || (DIB_BOARD_NIM10096MD3 == 1) || (DIB_BOARD_NIM10096MD4 == 1) || (DIB_BOARD_NIM1009xM2D2P2 == 1)

extern DIBSTATUS Board1009xxxLayoutInit(struct DibDriverContext *pContext);
#include "../Firmware/flrd_nautilus_1_1.h"

struct DibDemodBoardConfig ConfigFlRdNim1009xxx =
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
   Board1009xxxLayoutInit,
   {
      /* FeCfg */
      {
         /* FeCfg[0] */
         {
           /* UDemod */
           { 0 }, /* Dib7000 */
           /* Dib9000 */
           {
           sizeof(FlrdArray_nautilus11), (const uint8_t*)FlrdArray_nautilus11,
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

#endif /* (DIB_BOARD_NIM10096MD2 == 1) || (DIB_BOARD_NIM10096MD3 == 1) || (DIB_BOARD_NIM10096MD4 == 1)*/

#if (DIB_BOARD_NIM3006X)

extern DIBSTATUS Board3006XLayoutInit(struct DibDriverContext *pContext);

struct DibDemodBoardConfig ConfigFlRdNim3006X=
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
           sizeof(FlrdArray_nautilus2), (const uint8_t*)FlrdArray_nautilus2,
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




struct DibDemodBoardConfig *BoardFlrdList[eMAX_NB_BOARDS] =
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
   &ConfigFlRdNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD2 == 1)
   &ConfigFlRdNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD3 == 1)
   &ConfigFlRdNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM10096MD4 == 1)
   &ConfigFlRdNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3009xHx == 1)
   &ConfigFlRdNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD2 == 1)
   &ConfigFlRdNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD3 == 1)
   &ConfigFlRdNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM30092MD4 == 1)
   &ConfigFlRdNim3009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3006X == 1)
   &ConfigFlRdNim3006X,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM1009xM2D2P2 == 1)
   &ConfigFlRdNim1009xxx,
#else
   NULL,
#endif

#if (DIB_BOARD_NIM3009xM2D2P2 == 1)
   &ConfigFlRdNim3009xxx,
#else
   NULL,
#endif
};

#endif
