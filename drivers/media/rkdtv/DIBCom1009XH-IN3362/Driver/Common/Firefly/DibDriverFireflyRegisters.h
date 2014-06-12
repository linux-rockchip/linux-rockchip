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
* @file "DibDriverFireflyRegisters.h"
* @brief Firefly Register mapping.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_FIREFLY_REGISTERS_H
#define DIB_DRIVER_FIREFLY_REGISTERS_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#define FF_FRONTEND_FIRMWARE_ADDRESS 100     /**< Address in firefly memory where to load the slave firmware */

#define ID_RISCA                   0
#define ID_RISCB                   1

#define REG_MAC_CPUA_CTRL       1024
#define REG_MAC_CPUA_CADDR      1025
#define REG_MAC_CPUA_CDATA      1026
#define REG_MAC_MBXA_OUT        1027
#define REG_MAC_MBXA_IN         1028
#define REG_MAC_MBXA_DATA       1029

#define REG_MAC_CPUA_CKEY       1031

#define REG_MAC_CPUB_CTRL       1040
#define REG_MAC_CPUB_CADDR      1041
#define REG_MAC_CPUB_CDATA      1042
#define REG_MAC_MBXB_OUT        1043
#define REG_MAC_MBXB_IN         1044
#define REG_MAC_MBXB_DATA       1045

#define REG_MAC_CPUB_CKEY       1047

#define REG_RAMIF_MODE          1056
#define REG_RAMIF_BASEH         1057
#define REG_RAMIF_BASEL         1058
#define REG_RAMIF_MAXH          1059
#define REG_RAMIF_MAXL          1060
#define REG_RAMIF_MINH          1061
#define REG_RAMIF_MINL          1062
#define REG_RAMIF_DATA          1063

#define REG_RAMIF_NOJMP         1074
#define REG_RAMIF_JMP           1075
#define REG_RAMIF_IRAMCFG       1076

#define REG_MAC_IRQ_MASK        1224
#define REG_MAC_MEMCFG1         1225
#define REG_MAC_MEMCFG2         1226
#define REG_MAC_RESET           1227

#define REG_MAC_IRQ             1229

#define REG_MAC_APBSW           1237

#define REG_IPCRP_BSEL          1542

#define MASTER_IRQ_CTRL         1792

#define MBX_MAX_WORDS           (256 - 200 - 2)

#define TS_CHANNEL_UNSET        0xFF
#endif
