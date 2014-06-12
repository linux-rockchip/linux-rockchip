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
* @file "DibBoard9080.h"
* @brief 9080 board parameters.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (DIB_BOARD_SDIO9080 == 1)

#include "DibBoardSelection.h"
#include "../Firmware/microcode_A.h"
#include "../Firmware/microcode_B_9080.h"
#include "DibMsgMac.h"

struct DibDemodBoardConfig Config9080 =
{
   DIB_DEMOD_9000,
   DIB_TUNER_0080,
   DIB_FIREFLY,
   VERSION(1,0),
   1,       /* NbFrontends        */
   0,       /* SramDelayAddLatch  */
   1,       /* Hbm                */
   0,       /* BoardConfig        */
   0x0805,  /* RegSramCfg1805     */
   70,      /* DefAgcDynamicsDb   */
   0,          /* NO LNA             */
   0,          /* 0 Lna triggers     */
   NULL,       /* Lna Trigger table  */
   {0, 0},      /* Gpio default direction */
   {0, 0},      /* Gpio default value */
   NULL,        /* LayoutInit */
   {
      /* FeCfg */
      {
         /* FeCfg[0] */
         {
            /* UDemod */
            {0}, /* Dib7000 */
            {
               /* Dib9000 */
               sizeof(microcode_A_buffer) / sizeof(uint8_t), microcode_A_buffer,
               sizeof(microcode_B_buffer) / sizeof(uint8_t), microcode_B_buffer,
               NULL,
               &DibPllConfig9080,

               /* AGCdrain AGC drv     AGC slew    I2C drv     I2C slew   IOCLK dr   IOCLK sl   HOST drv   HOST slew  SRAM drv   SRAM slew */
               (1 << 15) | (1 << 13) | (0 << 12) | (1 << 10) | (0 << 9) | (1 << 7) | (0 << 6) | (3 << 4) | (0 << 3) | (3 << 1) | (0),
            },
         },
         {
            /* Gpio */
            { GPIO_FUNC_INIT,      0xffff, 0x001e, 0x0021 },
            { GPIO_FUNC_TUNER_ON,  0X0031, 0x0000, 0x0030 },
            { GPIO_FUNC_TUNER_OFF, 0X0031, 0x0000, 0x0021 },
         },
         {   /* SubBand */
            0, 0, 0,
            {
               {0}, /* SubBand[0] */
            }
         },
         18,
         0,  /* TunerI2CAdd */
         {   /* UTuner      */
            {0},           /* Dib0070 */
            {0, 0, 30000}, /* Dib0080 */
         },

         1, /* PMUIsPresent */
         {
            { 0x00cf, 0x1334, 0x01ae, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0406, 0x07ce, 0xfe0f, 0x002f, 0x0002,}
         },
      },
      {
         /* FeCfg[1] */
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
            { GPIO_FUNC_INIT, 0, 0, 0},
         },
         {
            /* SubBand */
            0, 0, 0,
            {
            {0}, /* SubBand[0] */
            }
         },
         0,
         0,
         {
            /* UTuner */
            {0, 0, 0, 0, 0},  /* Dib0070 */
            {0},              /* Dib0080 */
         },
      },
   }
};
#endif
