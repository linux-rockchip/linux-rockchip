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

#ifndef _FIRMRAM_H_
#define _FIRMRAM_H_

static uint8_t FirmRamArray[] =
{ 
	0x0A,
};

#endif

#ifndef _MAPPING_H_
#define _MAPPING_H_

#define CODE_HEAP_SIZE                  0
#define CRYPTED_BLOCK_END_ADDR          0x7000f13c
#define CRYPTED_BLOCK_START_ADDR        0x7000ed00
#define CRYPTED_DATA_ADDR               0x7000ed00
#define CRYPTED_DATA_SIZE               1024
#define CRYPTED_IV_ADDR                 0x7000f120
#define CRYPTED_KEY_ADDR                0x7000f100
#define DOWNLOAD_MODE_REG               0x7000f130
#define ENTRY_POINT_ADDR                0x701d0000
#define ENTRY_STACK_SIZE                7168
#define EXCEPTION_ADDR                  0x7000fc60
#define FIRMRAM_LENGTH                  0
#define FIRMRAM_START_ADDR              0x70000000
#define FIRMWARE_ADDR_REG               0x7000f138
#define FIRMWARE_ID_REG                 0x7000fce8
#define FOO_SYSRAM_ADDR                 0x7000fcdc
#define HOST_MBX_END_ADDR               0x70200000
#define HOST_MBX_RD_PTR_REG             0x7000fcec
#define HOST_MBX_SIZE                   2048
#define HOST_MBX_START_ADDR             0x701ff800
#define HOST_MBX_WR_PTR_REG             0x7000fcf0
#define HOST_STATUS_REG                 0x7000f13c
#define JEDEC_ADDR                      0x8000a08c
#define JEDEC_REG_VALUE                 0x01b33801
#define JUMP_ADDRESS_REG                0x7000f134
#define MAC_MBX_END_ADDR                0x7000fffc
#define MAC_MBX_RD_PTR_REG              0x7000fcf4
#define MAC_MBX_SIZE                    768
#define MAC_MBX_START_ADDR              0x7000fcfc
#define MAC_MBX_WR_PTR_REG              0x7000fcf8
#define MAIN_COUNTER_ADDR               0x7000fce4
#define MAX_IV_SIZE                     16
#define MAX_KEY_SIZE                    32
#define MBX_END_ADDR                    0x70200000
#define MBX_SIZE                        2848
#define MBX_START_ADDR                  0x7000fcdc
#define MSG_API_BUF_ADDR                0x7000f2b8
#define MSG_API_BUF_SIZE                264
#define SRAM_END_ADDR                   0x70010000
#define SRAM_START_ADDR                 0x70000000
#define STACK_END_ADDR                  0x701ff800
#define SUBFIRMRAM_LENGTH               187392
#define SUBFIRMRAM_START_ADDR           0x701d0000
#define TRAP_BASE_ADDR                  0x701d0000
#define URAM_END_ADDR                   0x70200000
#define URAM_START_ADDR                 0x70100000
#define VECTOR_SIZE                     1120

#endif /*_MAPPING_H_*/
