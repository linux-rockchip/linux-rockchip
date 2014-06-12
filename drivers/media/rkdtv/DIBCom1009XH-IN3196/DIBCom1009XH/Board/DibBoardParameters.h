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
* @file "DibBoardParameters.h"
* @brief Board structure definition.
*
***************************************************************************************************/
#ifndef DIB_BOARD_PARAMETERS_H
#define DIB_BOARD_PARAMETERS_H


#include "DibStatus.h"
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverTargetTypes.h"

#define BAND_LBAND      0x01
#define BAND_UHF        0x02
#define BAND_VHF        0x04
#define BAND_SBAND      0x08
#define BAND_FM         0x10

/* tuner defines */
#define DIB_TUNER_0070     1
#define DIB_TUNER_0080     2
#define DIB_TUNER_007S     3
#define DIB_TUNER_0090     4

/* demod defines */
#define DIB_DEMOD_7000     1
#define DIB_DEMOD_9000     2

/* chip family defines */
#define DIB_FIREFLY        0
/*#define DIB_DRAGONFLY      1*/
#define DIB_VOYAGER        2
#define DIB_NAUTILUS       3

#define MAX_NUMBER_OF_FRONTEND   4

struct DibDriverContext;

struct DibDemodAgcCfg7000
{
   /* defines the capabilities of this AGC-setting - using the BAND_-defines */
   uint8_t  BandCaps;

   /* 7000P: AgcUseSdMod1, AgcUseSdMod2 + AgcFreqPwmDiv, AgcInvPwm1, AgcInvPwm2,
             AgcInhDcRvEst, AgcTimeEst, AgcFreeze, AgcNbEst, AgcWrite */
   uint16_t Setup;

   /* 7000: pAgcInvGain */
   uint16_t InvGain;

   /* 7000: AgcTimeStabiliz */
   uint16_t TimeStabiliz;

   /* 7000: AgcAlphaLevel, AgcThLock */
   uint8_t  AlphaLevel;
   uint16_t ThLock;

   /* 7000: AgcWbdInv, SadWbdOpp */
   uint8_t  WbdInv;

   /* 7000: AgcWbdRef */
   uint16_t WbdRef;

   /* 7000: AgcWbdSel, SadWbdSel */
   uint8_t  WbdSel;

   /* 7000: AgcWbdAlpha */
   uint8_t  WbdAlpha;

   uint16_t Agc1Max;
   uint16_t Agc1Min;
   uint16_t Agc2Max;
   uint16_t Agc2Min;

   uint8_t  Agc1Pt1;
   uint8_t  Agc1Pt2;

   /* 7000: pt3 */
   uint8_t  Agc1Pt3;

   uint8_t  Agc1Slope1;
   uint8_t  Agc1Slope2;

   uint8_t  Agc2Pt1;
   uint8_t  Agc2Pt2;

   uint8_t  Agc2Slope1;
   uint8_t  Agc2Slope2;

   uint8_t  AlphaMant;
   uint8_t  AlphaExp;

   uint8_t  BetaMant;
   uint8_t  BetaExp;

   /**
   * tells the demod to use AGC softsplit and not the wideband-detector, the caller has to fill @see agc_split with useful values then.
   */
   uint8_t  PerformAgcSoftSplit;

   struct
   {
      uint16_t min;      /* AgcSplitMin       */
      uint16_t max;      /* AgcSplitMax       */
      uint16_t MinThres; /* AgcGlobalSplitMin */
      uint16_t MaxThres; /* AgcGlobalSplitMax */
   } split;
};

struct DibDemodPllCfg9000
{
   uint32_t vcxo;                    /* REG_VCXO */
   uint32_t timf;
   uint32_t XtalHz;
};

struct DibDemodPllCfg7000
{
   uint32_t vcxo;                    /* REG_VCXO */
   uint32_t sampling;                /* sampling frequency in kHz */

   /* PllCfg */
   uint8_t  PllPrediv;
   uint8_t  PllRatio;
   uint8_t  PllRange;
   uint8_t  PllReset;
   uint8_t  PllBypass;

   uint8_t  EnableRefDiv;
   uint8_t  ByPclkDiv;
   uint8_t  IoClkEnCore;
   uint8_t  ADClkSrc;
   uint8_t  modulo;

   uint16_t SadCfg;

   uint32_t IfReq;
   uint32_t timf;
   uint32_t XtalHz;
};

struct DibDemodGpioFn
{
   uint32_t                Func;
   uint32_t                Mask;
   uint32_t                Direction;
   uint32_t                Value;
};

struct DibDemodPwmFn
{
   uint8_t  Position;         /* GPIO number                                            */
   uint8_t  FrequencyDivider; /* Minimum pulse width is 2^FrequencyDivider clock cycles */
   uint16_t Slope;
   uint16_t Offset;           /* Value = Slope * F_mhz + Offset                         */
};

#define DIB_MAX_NB_SUBBANDS 8
struct DibDemodSubbandSelection
{
   uint16_t Size;                  /* actual number of subbands   */
   uint8_t  Type;                  /* 0:GPIO , 1:PWM              */
   uint16_t Mask;                  /* redundant in case GPIO      */
   struct
   {
      uint16_t F_mhz;
      struct
      {
         struct DibDemodGpioFn Gpio;
         struct DibDemodPwmFn Pwm;
      } u;
   } Subband[DIB_MAX_NB_SUBBANDS];
};

struct DibDemodCfg7000
{
   uint32_t                   FwRiscASize;
   const uint8_t             *FirmwareRiscA;
   uint32_t                   FwRiscBSize;
   const uint8_t             *FirmwareRiscB;
   uint8_t                    HostBusDiversity;
   uint8_t                    TunerIsBaseband;
   struct DibDemodAgcCfg7000 *Agc;
   struct DibDemodPllCfg7000 *Bw;
   uint16_t                   Drivers;  /* Drivers of the different busses/pads P_io_cfg */
};

/* mapping for each instance of a dragobnfly based firmware */
struct DibDriverDragonflyRegisters
{
   uint32_t DownloadModeReg;               /**< Register that define the download mode                              */
   uint32_t JumpAddressReg;                /**< Register used by the bootload to know where to jump after download  */
   uint32_t HostStatusReg;                 /**< Register that indicated the status of the host (ready or not)       */
   uint32_t FirmwareAddrReg;               /**< Register that indicated where the firmware must be decrypted        */
   uint32_t CryptedDataAddr;               /**< Address where each 1Ko Block of crypted firmware should be sent     */
   uint32_t CryptedKeyAddr;                /**< Address where to write the 32 bytes of crypted key                  */
   uint32_t CryptedIVAddr;                 /**< Address where to write the 16 bytes of Initial Vector               */
   uint32_t JedecAddr;                     /**< Address where to read the Jedec number                              */
   uint32_t JedecValue;                    /**< Value to be read in the Jedec address                               */
   uint32_t EntryPoint;                    /**< Where to jump once the code has been decrypted                      */
   uint32_t FirmwareIdReg;                 /**< Register that permits synchronization for download procedure        */
   uint32_t HostMbxStart;                  /**< The start address of the HOST mailbox                               */
   uint32_t HostMbxEnd;                    /**< The end address + 4 of the HOST mailbox                             */
   uint32_t HostMbxSize;                   /**< The size of the HOST mailbox                                        */
   uint32_t MacMbxStart;                   /**< The start address of the MAC mailbox                                */
   uint32_t MacMbxEnd;                     /**< The end address + 4 of the MAC mailbox                              */
   uint32_t MacMbxSize;                    /**< The size of the MAC mailbox                                         */
   uint32_t HostMbxRdPtrReg;               /**< The Register that own the current Read pointer in HOST mailbox      */
   uint32_t HostMbxWrPtrReg;               /**< The Register that own the current Write pointer in HOST mailbox     */
   uint32_t MacMbxRdPtrReg;                /**< The Register that own the current Read pointer in MAC mailbox       */
   uint32_t MacMbxWrPtrReg;                /**< The Register that own the current Write pointer in MAC mailbox      */
   uint32_t TxRxBufAddr;                   /**< Address used to transfer MSG_API data                               */
   uint32_t TxRxBufSize;                   /**< Size of MSG API buffer                                              */
   uint32_t MainCounterAddr;               /**< Address where the main write the time (indicate cpu is alive)       */

   uint32_t FirmwareNbRegion;              /**< Number of code regions mapped into the firmware.bin file */
   struct
   {
      uint32_t Addr;
      uint32_t Size;
      uint32_t Offset;
   }
   FirmwareRegions[3];
};


struct DibDemodCfg9000
{
   uint32_t                             FwRiscASize;
   const uint8_t                       *FirmwareRiscA;
   uint32_t                             FwRiscBSize;
   const uint8_t                       *FirmwareRiscB;
   struct DibDriverDragonflyRegisters  *Mapping;
   struct DibDemodPllCfg9000           *Bw;
   uint16_t                             Drivers;  /* Drivers of the different busses/pads P_io_cfg */
};

struct DibDemodCfg0070
{
   /*  Offset in kHz */
   int32_t  FreqOffsetKhzUhf;
   int32_t  FreqOffsetKhzVhf;
   uint8_t  OscBufferState;         /* 0= normal, 1= tri-state */
   uint32_t ClockKHz;
   uint8_t  ClockPadDriver;         /* (Drive + 1) * 2mA */
};

struct DibDemodCfg0080
{
   /*  Offset in kHz */
   int32_t  FreqOffsetKhzUhf;
   int32_t  FreqOffsetKhzVhf;
   uint32_t ClockKHz;
};

struct DibPMUConfig
{
   uint16_t Config[13];
};

/*
 * On voyager board based:
 * feId = i+0   => Master of i ème chip
 *        i+1   => Slave of i ème chip
 *        i+2   => SH decoder of i ème chip
 */
struct DibLnaGpioAction
{
    uint32_t FeId;                   /* the gpio bank, identified by any feId of the same chip */
    uint32_t GpioNum;                /* num of the gpio */
    uint32_t GpioDir;                /* direction of the gpio, 1 is an output, 0 is input */
    uint32_t GpioVal;                /* value of the gpio (0 or 1) */
};
 
 
#define OR                      1
#define AND                     0

/* TuneState masks */
#define ORCT_ALL                0xffff
#define ORCT_SHUTDOWN           0x1
#define ORCT_STANDBY            0x2
#define ORCT_TUNSTART           0x4


struct DibLnaTrigger
{
    struct DibLnaGpioAction GpioAction;   /* what todo when fe->tune_state in TuneStateMask
                                                   [OR|AND] fe->freq [INSIDE|OUTSIDE] FreqRangeMhz
                                                       AND  fe->gain [<=|>] ThresholdVal) is TRUE 
                                            for EVERY feId in FeIdMask */
    uint32_t FeIdMask;                    /* mask of all frontend on the the tests apply */
    uint32_t TuneStateMask;               /* The mask of all tune states in which the fe->tune_state should be */
    uint8_t Condition;                    /* OR or AND : test between cond1 and cond2 */
    uint8_t OutsideBand;                  /* apply on cond2. 1 means outside range freq0-freq1, 0 means inside */
    uint16_t FreqRangeMhz[2];             /* range of frequency */
    uint8_t ThresholdDir;                 /* 1 : (gain > threshold) 0: (gain <= threshold) */
    uint32_t ThresholdVal;                /* gain threshold for this 3rd cond */

};

struct DibDemodFeCfg
{
   struct
   {
      struct DibDemodCfg7000 Dib7000;
      struct DibDemodCfg9000 Dib9000;
   } UDemod;

   struct DibDemodGpioFn           GpioFunctions[10];
   struct DibDemodSubbandSelection SubbandSel;

   uint8_t DemodI2CAdd;
   uint8_t TunerI2CAdd;

   struct
   {
      struct DibDemodCfg0070 Dib0070;
      struct DibDemodCfg0080 Dib0080;
   } UTuner;

   uint8_t PMUIsPresent;
   struct DibPMUConfig PMU;
};

struct DibDemodBoardConfig
{
   uint16_t DibDemod;
   uint8_t  DibTuner;
   uint8_t  DibChip;
   uint8_t  DibChipVersion;
   uint8_t  NbFrontends;
   uint8_t  SramDelayAddLatch;
   uint8_t  Hbm;                   /* 0: BBM  1: HBM  2: AUTO */
   uint16_t BoardConfig;           /* Identify the board configuration in emb */
   uint16_t RegSramCfg1805;
   uint32_t DefAgcDynamicsDb;
   uint32_t LnaConfig;             /* 0: No lna, 3: Lna generic */
   uint32_t NbLnaTriggers;
   struct DibLnaTrigger * LnaTriggers;
   uint16_t GpioDefaultDir [2];
   uint16_t GpioDefaultVal [2];
   DIBSTATUS (*LayoutInit)(struct DibDriverContext *pContext);
   struct DibDemodFeCfg FeCfg[MAX_NUMBER_OF_FRONTEND];
};

#define VERSION(Major, Minor) (((Major & 0xF) << 4) | (Minor & 0xF))

#endif
