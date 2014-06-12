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
* @file "DibDriverFirefly.c"
* @brief Firefly Driver specific functionality.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverIf.h"
#include "DibDriverDowncalls.h"
#include "DibBoardSelection.h"
#include "DibDriverFirefly.h"
#include "DibDriverFireflyRegisters.h"
#include "DibDriverFireflyMessages.h"
#include "DibDriverFireflyData.h"
#include "DibDriverFireflyStd.h"
#include "DibDriverFireflyErrCodes.h"
#include "DibDriverFireflyIntDbg.h"
#include "DibMsgMac.h"

#define MAC_IRQ      (1 << 1)
#define IRQ_POL_MSK  (1 << 4)

/**
* Name of the message for each ID
*/
#ifdef DIB_MSG_OUT
#undef DIB_MSG_OUT
#endif

#ifdef DIB_MSG_IN
#undef DIB_MSG_IN
#endif

#define DIB_MSG_OUT(a)
#define DIB_MSG_IN(a) #a,

#if (DEBUG_MODE == DEBUG_PRINT)
static char *msg_txt[IN_MSG_LAST + 1] =
{
#include "DibDriverFireflyMsgNames.h"
   ""
};
#endif

/** Chip output Mode. */
enum DibOutputMode
{
/* output mode for frontends depending on the capabilities of the streaming interface */
   eHIGH_Z                       = 0,
   eANALOG_ADC                   = 1 << 0,
   eMPEG2_FIFO                   = 1 << 1,
   eMPEG2_PAR_GATED_CLK          = 1 << 2,
   eMPEG2_PAR_CONT_CLK           = 1 << 3,
   eMPEG2_SERIAL                 = 1 << 4,
   eDIVERSITY                    = 1 << 7,

   eENCAPSULATED                 = 1 << 20 /**< use this one with other modes to setup an encapsulation */
};


/** local definitions */
static DIBSTATUS IntDriverFireflySetPrioritryCh(struct DibDriverContext *pContext, uint16_t TsChannelNumber);
static DIBSTATUS IntDriverFireflyClearMonit(struct DibDriverContext *pContext, ELEM_HDL ItemHdl);
static DIBSTATUS IntDriverFireflyUpdateDiversity(struct DibDriverContext *pContext);

/** old api reference for firefly */
static DIBSTATUS IntDriverAddPid(struct DibDriverContext *pContext, struct DibAddPidInfo *pPidInfo, ELEM_HDL elemHdl, FILTER_HDL filterHdl);
static DIBSTATUS IntDriverDelPid(struct DibDriverContext *pContext, struct DibDelPidInfo *pDelPid,  ELEM_HDL elemHdl, FILTER_HDL filterHdl);

/**
* Tune the channel.
* @param pContext         pointer to the context
* @param ChannelIdx       channel concerned
* @param pDescriptor      pointer to the structure of information
* @return result
*/

/****************************************************************************
* Main Initialization. This is all that needs to be called by the
* system
****************************************************************************/
static void IntDriverFireflyResetCpu(struct DibDriverContext *pContext)
{
   /* Reset the RISCs */
   DibDriverWriteReg16(pContext, REG_MAC_CPUA_CTRL, 2);
   DibDriverWriteReg16(pContext, REG_MAC_CPUB_CTRL, 2);
}

/**
* embedded sofware upload
*/
static DIBSTATUS IntDriverFireflySetUploadMicrocode(struct DibDriverContext *pContext, uint32_t len, uint32_t RiscId, uint16_t key, const uint8_t * buf)
{
#if (SLOW_MICROCODE_UPLOAD == 1)
   uint16_t k;
#endif

   uint16_t MboxOff;

   DIB_ASSERT(buf);

   if(RiscId == ID_RISCB)
      MboxOff = 16;
   else
      MboxOff = 0;

   /* Config crtl reg */
   DibDriverWriteReg16(pContext, REG_MAC_CPUA_CTRL  + MboxOff, 0x0f);
   DibDriverWriteReg16(pContext, REG_MAC_CPUA_CADDR + MboxOff, 0);
   DibDriverWriteReg16(pContext, REG_MAC_CPUA_CKEY  + MboxOff, key);

#if (SLOW_MICROCODE_UPLOAD == 1)
   for(k = 0; k < len; k += 2)
   {
      uint32_t Data;
      uint8_t c0, c1;

      c1   = buf[k];
      c0   = buf[k + 1];
      Data = (c0 | (c1 << 8));

      if(DibDriverWriteReg16(pContext, REG_MAC_CPUA_CDATA + MboxOff, Data) != DIBSTATUS_SUCCESS)
         return DIBSTATUS_ERROR;
   }
#else
   if(DibD2BWriteReg(pContext, REG_MAC_CPUA_CDATA + MboxOff, DIBBRIGDE_BIT_MODE_16, len, (uint8_t *) buf) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_ERROR;
#endif

   DIB_DEBUG(MSG_LOG, (CRB "  Microcode for RISC %c loaded" CRA, 'A' + RiscId));

   return DIBSTATUS_SUCCESS;
}


static DIBSTATUS IntDriverFireflyUploadMicrocode(struct DibDriverContext *pContext)
{
   DIBSTATUS status = DIBSTATUS_SUCCESS;
   if(pContext->Hd.BoardConfig->DibDemod == DIB_DEMOD_9000)
   {
      status = IntDriverFireflySetUploadMicrocode(pContext,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib9000.FwRiscASize,
                                         ID_RISCA,
                                         DIB_MICROCODE_A_KEY,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib9000.FirmwareRiscA);

      if(status != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  Microcode for RISC A could not be loaded" CRA));
         goto End;
      }

      status = IntDriverFireflySetUploadMicrocode(pContext,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib9000.FwRiscBSize,
                                         ID_RISCB,
                                         DIB_MICROCODE_B_KEY,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib9000.FirmwareRiscB);

   }
   else if(pContext->Hd.BoardConfig->DibDemod == DIB_DEMOD_7000 )
   {
      status = IntDriverFireflySetUploadMicrocode(pContext,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib7000.FwRiscASize,
                                         ID_RISCA,
                                         DIB_MICROCODE_A_KEY,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib7000.FirmwareRiscA);

      if(status != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  Microcode for RISC A could not be loaded" CRA));
         goto End;
      }

      status = IntDriverFireflySetUploadMicrocode(pContext,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib7000.FwRiscBSize,
                                         ID_RISCB,
                                         DIB_MICROCODE_B_KEY,
                                         pContext->Hd.BoardConfig->FeCfg[0].UDemod.
                                         Dib7000.FirmwareRiscB);
   }

End:
   return status;
}

static void IntDriverFireflyInitCpu(struct DibDriverContext *pContext)
{
   /* Initialise (and run) the RISCs. Needed before initialisation of the mailbox */
   DibDriverWriteReg16(pContext, REG_MAC_CPUA_CTRL, 0);
   DibDriverWriteReg16(pContext, REG_MAC_CPUB_CTRL, 0);
}

static void IntDriverFireflyStartCpu(struct DibDriverContext *pContext)
{
}

/****************************************************************************
* init and synchro risc with host
****************************************************************************/
static DIBSTATUS IntDriverFireflyInitMsg(struct DibDriverContext *pContext, uint8_t RiscId)
{
   uint16_t MboxOff;
   uint16_t ResetReg;
   uint16_t tries = 0;

   if(RiscId == ID_RISCB)
      MboxOff = 16;
   else
      MboxOff = 0;

   /* Reset mailbox  */
   DibDriverWriteReg16(pContext, REG_MAC_MBXA_OUT + MboxOff, 0x8000);

   /* Read reset status */
   DibDriverReadReg16(pContext, REG_MAC_MBXA_OUT + MboxOff, &ResetReg);

   while((ResetReg & 0x8000) && (tries++ < 10000))
   {
      if(DibDriverReadReg16(pContext, REG_MAC_MBXA_OUT + MboxOff, &ResetReg) != DIBSTATUS_SUCCESS)
         return DIBSTATUS_ERROR;
      DibMSleep(1);
   }

   if(ResetReg & 0x8000)
   {
      DIB_DEBUG(MSG_LOG, (CRB "DIB_message_init ERROR, no response from RISC %c" CRA, 'A' + RiscId));
      return DIBSTATUS_ERROR;
   }
   else
   {
      return DIBSTATUS_SUCCESS;
   }
}

static DIBSTATUS IntDriverFireflyInitMailbox(struct DibDriverContext *pContext)
{
   /*  init mailboxes */
   if(IntDriverFireflyInitMsg(pContext, ID_RISCA) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_ERROR;

   if(IntDriverFireflyInitMsg(pContext, ID_RISCB) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_ERROR;

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Tests the RAM (R&W tests)
****************************************************************************/
static uint32_t IntDriverFireflyTestRamInterface(struct DibDriverContext *pContext)
{
   uint16_t j;
   uint32_t NbByte;
   uint32_t NbErr  = 0;

   /* test internal ram */
   uint32_t AdMin  = 10;
   uint32_t AdMax  = 1000;
   uint32_t AdBase = AdMin;
   uint32_t AdInc  = 1;

   /*  Write in internal ram */
   DibDriverWriteReg16(pContext, REG_RAMIF_MODE, 0 << 15 | AdInc);
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEH, (AdBase >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEL, (AdBase) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MAXH, (AdMax >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MAXL, (AdMax) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MINH, (AdMin >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MINL, (AdMin) & 0xffff);

   for(j = 0; j < 100; j++)
   {
      DibDriverWriteReg16(pContext, REG_RAMIF_DATA, j);
   }

   /*  Read in internal ram */
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEH, (AdBase >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEL, (AdBase) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MODE, (uint32_t) 1 << 15 | AdInc);

   for(j = 0; j < 100; j++)
   {
      uint16_t DatRd;

      DibDriverReadReg16(pContext, REG_RAMIF_DATA, &DatRd);

      if(DatRd != j)
         NbErr++;
   }

   /* test External  ram */
   NbByte = (uint32_t) 1 << 8;
   AdMin  = 10 + ((uint32_t) 1 << 23);
   AdMax  = ((uint32_t) 1 << 19) + ((uint32_t) 1 << 23);
   AdBase = AdMax - 100;
   AdInc  = 1;

   /*  Write in External ram */
   DibDriverWriteReg16(pContext, REG_RAMIF_MODE, 0 << 15 | AdInc);
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEH, (AdBase >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEL, (AdBase) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MAXH, (AdMax >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MAXL, (AdMax) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MINH, (AdMin >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MINL, (AdMin) & 0xffff);

   for(j = 0; j < NbByte; j++)
   {
      DibDriverWriteReg16(pContext, REG_RAMIF_DATA, j % 256);
   }

   /*  Read in External ram */
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEH, (AdBase >> 16) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_BASEL, (AdBase) & 0xffff);
   DibDriverWriteReg16(pContext, REG_RAMIF_MODE, (uint32_t) 1 << 15 | AdInc);

   for(j = 0; j < NbByte; j++)
   {
      uint16_t DatRd;

      DibDriverReadReg16(pContext, REG_RAMIF_DATA, &DatRd);

      if(DatRd != (j % 256))
      {
         NbErr++;
         DIB_DEBUG(MSG_LOG, (CRB "test external ram read=%x , expected=%x , diff=%x " CRA, DatRd, ((j) % 256), DatRd - ((j) % 256)));
      }
   }

   return NbErr;
}

static void IntDriverFireflyRegisterIfInit(struct DibDriverContext *pContext)
{
   /*  Reconfig pool mac ram */
   DibDriverWriteReg16(pContext, REG_MAC_MEMCFG1, 0x02);      /* A: 8k C, 4 k D - B: 32k C 6 k D - IRAM 96k */
   DibDriverWriteReg16(pContext, REG_MAC_MEMCFG2, 0x05);

   /* Toggles IP crypto to Host APB interface. */
   DibDriverWriteReg16(pContext, REG_IPCRP_BSEL, 1);

   /* Set jump and no jump in the dma box */
   DibDriverWriteReg16(pContext, REG_RAMIF_NOJMP, 0);
   DibDriverWriteReg16(pContext, REG_RAMIF_JMP, 0);

   /* Set MAC as APB Master. */
   DibDriverWriteReg16(pContext, REG_MAC_APBSW, 0);
}


static void IntDriverFireflyEnableClearIrq(struct DibDriverContext *pContext)
{
   uint16_t Mask;
   uint16_t MacIrq;

   Mask = MAC_IRQ;

#if (IRQ_POLARITY == IRQ_ACTIVE_LOW)
   Mask &= ~IRQ_POL_MSK;
#else
#if (IRQ_POLARITY == IRQ_ACTIVE_HIGH)
   Mask |= IRQ_POL_MSK;
#else
#error Either IRQ_ACTIVE_LOW or IRQ_ACTIVE_HIGH must be set.
#endif
#endif

#if (DEMOD_TRACE == 1)
   DibDriverWriteReg16(pContext, REG_MAC_IRQ_MASK, 0x000e);   /* MAC: enable RISC A & B and APB DMA IRQs */
#else
   DibDriverWriteReg16(pContext, REG_MAC_IRQ_MASK, 0x0006);   /* MAC: enable RISC A & B IRQs */
#endif
   DibDriverWriteReg16(pContext, MASTER_IRQ_CTRL, Mask);      /* CHIP: enable MAC IRQ */

   /* Flush possibly pending MAC IRQs */
   DibDriverReadReg16(pContext, REG_MAC_IRQ, &MacIrq);

   while(MacIrq)
   {
      DIB_DEBUG(SIRQ_LOG, (CRB "Ignoring MacIrq %x" CRA, MacIrq));
      DibDriverReadReg16(pContext, REG_MAC_IRQ, &MacIrq);
   }
}

/*******************************************************************************
* IntDriverFireflyWriteDemod
*******************************************************************************/
static DIBSTATUS IntDriverFireflyWriteDemod(struct DibDriverContext *pContext, uint8_t DemId, uint32_t SubAdd, uint16_t Value)
{
   uint16_t ad, da, status;
   uint16_t stop = 0;
   uint16_t j;
   uint8_t  txbuf[4];

   /* Demod I2C address. */
   uint16_t address = pContext->Hd.DemodI2CAdd[DemId];
   DIB_ASSERT(DemId < DIB_MAX_NB_DEMODS);

   if((SubAdd >= 112) && (SubAdd <= 332))
      SubAdd++;

   txbuf[0] = (uint8_t) ((SubAdd >> 8) & 0xff);
   txbuf[1] = (uint8_t) (SubAdd & 0xff);
   txbuf[2] = (uint8_t) ((Value >> 8) & 0xff);
   txbuf[3] = (uint8_t) (Value & 0xff);

   /* IF MAIN DEMOD. */
   if(DemId == 0)
   {
      return DibDriverWriteReg16(pContext, SubAdd, Value);
   }

   /* DibDriverWriteReg16( pContext, 772 , 1  ); */
   /*  return ; */

   /* ELSE */
   ad = 2;
   DibDriverReadReg16(pContext, 768 + ad, &status);   /* Reset fifo ptr. */

   /* wr Data */
   ad = 0;

   for(j = 0; j < 4; j += 2)
      DibDriverWriteReg16(pContext, 768 + ad, (txbuf[j] << 8 | txbuf[j + 1]));

   stop = 1;

   /* Command. */
   da = (uint16_t) ((((address / 2) & 0x7f) << 8) |        /* Addr */
                     ((1 & 0x1) << 7) |     /* master */
                     ((1 & 0x1) << 6) |     /* rq     */
                     ((stop & 0x1) << 5) |  /* stop   */
                     ((4 & 0x7) << 2) |     /* Nb     */
                     ((0 & 0x1) << 1) |     /* rw     */
                     ((0 & 0x1) << 0));     /* irqen  */
   ad = 1;

   DibDriverWriteReg16(pContext, 768 + ad, da);

   ad     = 2;
   status = 0;

   while((status & 0x0100) == 0)
   {
      DibDriverReadReg16(pContext, 768 + ad, &status);
   }

   return DIBSTATUS_SUCCESS;
}

/*******************************************************************************
* DibDriverdem_read_word
*******************************************************************************/
static uint16_t IntDriverFireflyReadDemod(struct DibDriverContext *pContext, uint8_t DemId, uint32_t SubAdd)
{
   /* Demod I2C address. */
   uint16_t Value = 0;

   DIB_ASSERT(DemId == 0); /* reading slave demods is not supported */

   if((SubAdd >= 112) && (SubAdd <= 332))
      SubAdd++;

   if(DemId == 0)
   {
      DibDriverReadReg16(pContext, SubAdd, &Value);
      return Value;
   }
   return 0;
}

/**
* DVBH_driver_init_pll
*/
static void IntDriverFireflyInitDib0080(struct DibDriverContext *pContext, uint8_t reg, uint16_t tx)
{
   uint8_t i;

   if(DibDriverReadDemod(pContext, 0, 791) != 0)
      DIB_DEBUG(FRONTEND_LOG, (CRB "-E-  TunerITF: write busy" CRA));

   DibDriverWriteDemod(pContext, 0, 785, tx);
   DibDriverWriteDemod(pContext, 0, 784, reg);
   DibDriverWriteDemod(pContext, 0, 787, 0);       /* 1 Value */
   DibDriverWriteDemod(pContext, 0, 786, 0);       /* start write */

   i = 100;
   while(DibDriverReadDemod(pContext, 0, 791) > 0 && i)
   {
      i--;
      DibMSleep(1);
   }

   if(i == 0)
      DIB_DEBUG(FRONTEND_ERR, (CRB "-E-  TunerITF: write failed" CRA));
}

static void IntDriverFireflyInitPll9000(struct DibDriverContext *pContext)
{
   /*  set power-down level */
   DibDriverWriteDemod(pContext, 0, 904, 0);
   DibDriverWriteDemod(pContext, 0, 905, 0);
   DibDriverWriteDemod(pContext, 0, 906, 0);
   DibDriverWriteDemod(pContext, 0, 907, 0);

   /*  reset all */
   DibDriverWriteDemod(pContext, 0, 898, 0xffff);
   DibDriverWriteDemod(pContext, 0, 899, 0xffff);
   DibDriverWriteDemod(pContext, 0, 900, 0x0001);
   DibDriverWriteDemod(pContext, 0, 901, 0xff19);
   DibDriverWriteDemod(pContext, 0, 902, 0x003c);
   DibDriverWriteDemod(pContext, 0, 898, 0);
   DibDriverWriteDemod(pContext, 0, 899, 0);
   DibDriverWriteDemod(pContext, 0, 900, 0);
   DibDriverWriteDemod(pContext, 0, 901, 0);
   DibDriverWriteDemod(pContext, 0, 902, 0);

   DibDriverWriteDemod(pContext, 0, 904, 0);
   DibDriverWriteDemod(pContext, 0, 905, 0);
   DibDriverWriteDemod(pContext, 0, 906, 0);
   DibDriverWriteDemod(pContext, 0, 907, 0);

   DibDriverWriteDemod(pContext, 0, 911, pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib9000.Drivers);

   /*  Clock configuration (default with BW=8MHz) */
   DibDriverWriteDemod(pContext, 0, 23, (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib9000.Bw->timf >> 16) & 0xffff);
   DibDriverWriteDemod(pContext, 0, 24, (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib9000.Bw->timf) & 0xffff);

   DibDriverWriteDemod(pContext, 0, 773, 0x1e);
   DibDriverWriteDemod(pContext, 0, 774, 0x20);

   if(pContext->Hd.BoardConfig->DibTuner == DIB_TUNER_0080) {
      IntDriverFireflyInitDib0080(pContext, 0x16, 0x0284); /* SLEEP_EN */

      IntDriverFireflyInitDib0080(pContext, 0x20, 0x0080); /* TUNER_EN */

      /* adcClkOutRatio=8->7, release reset */
      IntDriverFireflyInitDib0080(pContext, 0x1b, (7 << 11) | (1 << 9) | (1 << 8) | (7 << 4) | (7 << 0));

      IntDriverFireflyInitDib0080(pContext, 0x1f, (1 << 15) | (1 << 14) | (7 << 10) | (1 << 9) | (0 << 8) | (7 << 5) | (1 << 4) | (0 << 2) | (0 << 0));

      /* enable pll, de-activate reset, ratio: 2/1 = 60MHz */
      IntDriverFireflyInitDib0080(pContext, 0x1c, (0 << 15) | (1 << 13) | (1 << 12) | (8 << 6) | (1 << 0));
   } else {
      IntDriverFireflyInitDib0080(pContext, 0x16, 0x0284); /* SLEEP_EN */

      IntDriverFireflyInitDib0080(pContext, 0x20, 0x0080); /* TUNER_EN */
      IntDriverFireflyInitDib0080(pContext, 32, (7 << 11) | (0 << 10) | (1 << 9) | (1 << 8) | (0 << 4) | 0);
      IntDriverFireflyInitDib0080(pContext, 35, (0 << 15) | (1 << 14) | (1 << 10) | (1 << 9) | (0 << 8) | (7 << 5) | (1 << 4) | (0 << 2) | (0));
      IntDriverFireflyInitDib0080(pContext, 33, (0 << 15) | (1 << 13) | (1 << 12) | (8 << 6) | (1));
   }
   DibMSleep(1);
}

static void IntDriverFireflyInitPll(struct DibDriverContext *pContext)
{
   /*  set power-down level */
   DibDriverWriteDemod(pContext, 0, 903, 0);
   DibDriverWriteDemod(pContext, 0, 904, 0);
   DibDriverWriteDemod(pContext, 0, 905, 0);
   DibDriverWriteDemod(pContext, 0, 906, 0);

   /*  reset all */
   DibDriverWriteDemod(pContext, 0, 898, 0xffff);
   DibDriverWriteDemod(pContext, 0, 899, 0xffff);
   DibDriverWriteDemod(pContext, 0, 900, 0xff0f);
   DibDriverWriteDemod(pContext, 0, 901, 0xfffc);
   DibDriverWriteDemod(pContext, 0, 898, 0);
   DibDriverWriteDemod(pContext, 0, 899, 0);
   DibDriverWriteDemod(pContext, 0, 900, 0);
   DibDriverWriteDemod(pContext, 0, 901, 0);

   DibDriverWriteDemod(pContext, 0, 903, 0);
   DibDriverWriteDemod(pContext, 0, 904, 0);
   DibDriverWriteDemod(pContext, 0, 905, 0);
   DibDriverWriteDemod(pContext, 0, 906, 0);

   /*  Clock configuration (default with BW=8MHz) */
   DibDriverWriteDemod(pContext, 0, 21, 0);
   DibDriverWriteDemod(pContext, 0, 22, 0);
   DibDriverWriteDemod(pContext, 0, 23, (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->timf >> 16) & 0xffff);
   DibDriverWriteDemod(pContext, 0, 24, (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->timf) & 0xffff);

   DibDriverWriteDemod(pContext, 0, 907, (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->PllPrediv << 8) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->PllRatio));
   DibMSleep(1);

   DibDriverWriteDemod(pContext, 0, 908, (0 << 14) | (3 << 12) | (0 << 11) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->IoClkEnCore  << 10) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->ByPclkDiv    << 5) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->EnableRefDiv << 4) |
                                         (1 << 3) | /* force the bypass while  setting PLL parameter */
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->PllRange     << 1) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->PllReset     << 0));

   DibMSleep(1);

   DibDriverWriteDemod(pContext, 0, 908, (0 << 14) | (3 << 12) | (0 << 11) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->IoClkEnCore  << 10) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->ByPclkDiv    << 5) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->EnableRefDiv << 4) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->PllBypass    << 3) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->PllRange     << 1) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->PllReset     << 0));

   DibMSleep(1);

   DibDriverWriteDemod(pContext, 0, 909, (3 << 4) | 1);

   DibMSleep(1);

   DibDriverWriteDemod(pContext, 0, 910, (1 << 12) | (2 << 10) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->modulo   << 8) |
                                         (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->ADClkSrc << 7));

   DibDriverWriteDemod(pContext, 0, 912, pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Drivers);

   DibDriverWriteDemod(pContext, 0, 928, (pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib7000.Bw->SadCfg));
}

/**
* DibDriverFrontendInit
*/
static DIBSTATUS IntDriverFireflyFrontendInit(struct DibDriverContext *pContext)
{
   uint16_t Value;

   /* reset the MAC of the Master */
   DibDriverWriteDemod(pContext, 0, REG_MAC_RESET, 1);     /* Reset Mac */
   DibDriverWriteDemod(pContext, 0, REG_MAC_RESET, 0);
   DibDriverWriteDemod(pContext, 0, 0xffff, 0xffff);       /* reset */

   if(pContext->Hd.BoardConfig->SramDelayAddLatch)
      DibDriverWriteDemod(pContext, 0, 1817, 3);    /* P_host_read_lead_in */

   Value = DibDriverReadDemod(pContext, 0, 896);
   DIB_DEBUG(FRONTEND_LOG, (CRB "Jedec ID: %04X" CRA, Value));

   if(Value != 0x01b3)
   {
      DIB_DEBUG(FRONTEND_ERR, (CRB "Incorrect Jedec ID %04X" CRA, Value));
      return DIBSTATUS_ERROR;
   }

   if(pContext->Hd.BoardConfig->DibDemod == DIB_DEMOD_9000)
   {
      IntDriverFireflyInitPll9000(pContext);
   }
   else if(pContext->Hd.BoardConfig->DibDemod == DIB_DEMOD_7000)
   {
      IntDriverFireflyInitPll(pContext);
   }

   /* initialize the PLL of the first demod */
   DibDriverWriteDemod(pContext, 0, 1805, pContext->Hd.BoardConfig->RegSramCfg1805);

   Value = DibDriverReadDemod(pContext, 0, 897);

   DIB_DEBUG(FRONTEND_LOG, (CRB "Device ID: %04X" CRA, Value));
   if(Value != 0x4001 && Value != 0x4002 && Value != 0x4003 && Value != 0x4004 && Value != 0x4005)
      return DIBSTATUS_ERROR;
   else
      return DIBSTATUS_SUCCESS;
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgEndBridgeApbRw(struct DibDriverContext *pContext, uint16_t * Data)
{
   uint16_t ad  = Data[0];
   uint16_t Val = Data[1];
   DIB_ASSERT(Data);

   pContext->ChipContext.APBBridgeAddressRead = ad;
   pContext->ChipContext.APBBridgeDataRead    = Val;

   DIB_DEBUG(MSG_LOG, (CRB "-IN_MSG_END_BRIDGE_APB_RW: Addr: %d, Val: %d" CRA, ad, Val));
   DibSetEvent(&pContext->BridgeRWEvent);
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgEndBridgeI2CRw(struct DibDriverContext *pContext, uint32_t rx_nb, uint16_t * Data)
{
   uint16_t k;
   DIB_ASSERT(Data);

   if(2 * (rx_nb) > 8)
   {
      DIB_DEBUG(MSG_LOG, (CRB "-IN_MSG_END_BRIDGE_I2C_RW: Message too long " CRA));
   }
   else
   {
      DIB_DEBUG(MSG_LOG, (CRB "-IN_MSG_END_BRIDGE_I2C_RW" CRA));

      for(k = 0; k < rx_nb; k++)
      {
         pContext->ChipContext.I2CBridgeDataRead[2 * k]     = (Data[k] >> 0) & 0xff;
         pContext->ChipContext.I2CBridgeDataRead[2 * k + 1] = (Data[k] >> 8) & 0xff;
         DIB_DEBUG(MSG_LOG, ("0x%04x ", Data[k]));
      }

      DIB_DEBUG(MSG_LOG, (CRB "" CRA));
   }

   DibSetEvent(&pContext->BridgeRWEvent);
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgFwDwld(struct DibDriverContext *pContext, uint16_t *Data)
{
   DIB_ASSERT(Data);

   DIB_DEBUG(MSG_LOG, (CRB "-IN_MSG_FE_FW_DL_DONE : 0x%04x " CRA, *Data));

   pContext->ChipContext.FwDwldData = *Data;

   DibSetEvent(&pContext->FWDwldEvent);
}


/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgEndOfScan(struct DibDriverContext *pContext, uint16_t *Data)
{
   uint8_t        TsChannelNb = Data[0] & 0xf;
   uint8_t        ScanStatus  = (Data[0] >> 8) & 0x0f;
   uint8_t        timeout     = Data[8] & 0x1;
   uint8_t        abort       = (Data[8] & 0x2) >> 1;
   uint8_t        failed      = (Data[8] & 0x4) >> 2;

   uint8_t ChannelIdx  = pContext->FireflyChannel[TsChannelNb];

   DIB_ASSERT(Data);
   DIB_ASSERT(TsChannelNb < DIB_MAX_NB_TS_CHANNELS);
   DIB_ASSERT(ChannelIdx < DIB_MAX_NB_CHANNELS);

   if(abort)
      ScanStatus = DIB_SCAN_ABORTED;
   else if(timeout)
      ScanStatus = DIB_SCAN_TIMEOUT;
   else if(failed)
      ScanStatus = DIB_SCAN_INVALID_PARAM;

   DIB_DEBUG(MSG_LOG, (CRB "  End of scan message from TS channel no. %d: ScanStatus=%d, timeout=%d" CRA,
                      TsChannelNb, ScanStatus, timeout));

   if(TsChannelNb < 3)
   {
      DibDriverFireflyGetChannelInfoDvb(pContext, ChannelIdx, ScanStatus, Data, 0);

      /* Handles synchronicity. */
      DibSetEvent(&pContext->ChannelInfo[ChannelIdx].ScanEvent);
   }
   else if(TsChannelNb == 3)
   {
      DibDriverFireflyGetChannelInfoDvb(pContext, ChannelIdx, ScanStatus, Data, 1);
      /* SCAN is folowed by MONIT_DEMOD for channel 3 */
   }
   else
   {
      DIB_DEBUG(MSG_ERR, (CRB "  DIB_messages_IN_MSG_END_OF_SCAN: ERROR: illegal n_ts_channel %d" CRA, TsChannelNb));
   }
}

/****************************************************************************
*::
****************************************************************************/
static void IntDriverFireflyProcessMsgMonitDemod(struct DibDriverContext *pContext, uint16_t *Data)
{
   uint8_t   TsChannelNumber  = (uint8_t) Data[0] & 0x03;
   uint8_t   DemId            = (((uint8_t) Data[0]) >> 2) & 0x03;
   uint8_t   Timeout          =  (Data[0] >> 4) & 0x01;
   uint16_t  TmpExp;
   uint32_t  TmpFp;
   uint8_t   ChannelIdx      = pContext->FireflyChannel[TsChannelNumber];
   uint8_t  NbDemodMonitMsg;

   struct DibTotalDemodMonit *pTotalMonitInfo;

   DIB_ASSERT(Data);
   DIB_ASSERT(TsChannelNumber < DIB_MAX_NB_TS_CHANNELS);
   DIB_ASSERT(ChannelIdx < DIB_MAX_NB_CHANNELS);
   DIB_ASSERT(DemId < DIB_MAX_NB_DEMODS);
   DIB_DEBUG(MSG_LOG, (CRB "  End of monit message from TS channel no. %d: timeout=%d" CRA, TsChannelNumber, Timeout));

   if(pContext->FireflyDiversityActiv == 0)
      NbDemodMonitMsg = 1;
   else
      NbDemodMonitMsg = pContext->Hd.BoardConfig->NbFrontends;

   /**< nb demod needed by the user at the moment where the GetSignalMonitoring is done. Also set when GetChannel and TuneMonit */
   DIB_ASSERT(pContext->ChannelInfo[ChannelIdx].NbDemodsUser > 0);

   if(TsChannelNumber == 3)
   {
      pTotalMonitInfo = &pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelMonit;
   }
   else
   {
      pTotalMonitInfo = &pContext->ChannelInfo[ChannelIdx].ChannelMonit;
   }

   DibDriverFireflyProcessMsgMonitDemodDvb(pContext, Data, pTotalMonitInfo);

   if(DemId == NbDemodMonitMsg - 1)
   {
      pTotalMonitInfo->Type = eSTANDARD_DVB;

      /** nb demodulation really fill in the demod structure */
      pTotalMonitInfo->NbDemods = NbDemodMonitMsg;

      /** General Monit Status */
      TmpExp=0; /* number of timeout */
      for(TmpFp=0; TmpFp < NbDemodMonitMsg; TmpFp++)
      {
         if(DIB_DRIVER_FIREFLY_GET_TIMEOUT) 
            TmpExp++;
      }
      if((TsChannelNumber == 3) && (pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelDescriptor.ScanStatus != DIB_SCAN_SUCCESS))
      {
         /* Monitoring is failed if scanning was failed (Fixed) */
         pTotalMonitInfo->MonitStatus = pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelDescriptor.ScanStatus; /* Failure, abort, invalid param */
      }
      else if(TmpExp == 0)
      {
         pTotalMonitInfo->MonitStatus = DIB_MONIT_RELIABLE;
      }
      else if(TmpExp == pTotalMonitInfo->NbDemods)
      {
         pTotalMonitInfo->MonitStatus = DIB_MONIT_TIMEOUT;
      }
      else
      {
         pTotalMonitInfo->MonitStatus = DIB_MONIT_PARTIAL;
      }

      if(TsChannelNumber == 3)
      {
         /* free this ts channel descriptor */
         pContext->FireflyChannel[TsChannelNumber] = DIB_UNSET;
         DIB_DEBUG(MSG_LOG, (CRB "  CLEAR CHANNEL %d" CRA, TsChannelNumber));
      }

      /** nb demodulation for this channel in the actual configuration. shld be >= of pTotalMonitInfo->NbDemods */
      pTotalMonitInfo->ChannelIndex = ChannelIdx;
      pTotalMonitInfo->MonitoringVersion++;

      if((TsChannelNumber == 3) && (pContext->ChannelInfo[ChannelIdx].MonitContext.pTuneMonitCb))
      {
         DibTuneMonitCallback CallBack = pContext->ChannelInfo[ChannelIdx].MonitContext.pTuneMonitCb;

         CallBack(&pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit, pContext->ChannelInfo[ChannelIdx].MonitContext.pTuneMonitCtx);

         DIB_ASSERT(pContext->ChannelInfo[ChannelIdx].InUse);

         IntDriverRemoveChannel(pContext, ChannelIdx, eDIB_TRUE);
         IntDriverFireflyUpdateDiversity(pContext);

         /* Free monitoring channel */
         DibReleaseLock(&pContext->MonitLock);
      }
      else if((TsChannelNumber < 3) && (pContext->ChannelInfo[ChannelIdx].pGetSignalCb))
      {
         DibGetSignalCallback CallBack = pContext->ChannelInfo[ChannelIdx].pGetSignalCb;

         CallBack(pTotalMonitInfo, &NbDemodMonitMsg, pContext->ChannelInfo[ChannelIdx].pGetSignalCtx);

         /* Free signal monit lock */
         DibReleaseLock(&pContext->SignalMonitLock);
      }
      else
      {
         /* synchronous - lock release by caller */
         DibSetEvent(&pContext->ChannelInfo[ChannelIdx].MonitContext.MonitEvent);
         DIB_DEBUG(MSG_LOG, (CRB "  RESET EVENT" CRA));
      }
   }
   else
   {
      DIB_DEBUG(MSG_LOG, (CRB "Waiting for another DEMOD_INFO" CRA));
   }
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgError(struct DibDriverContext *pContext, uint16_t *Data)
{
   uint16_t p1, p2;
   uint8_t ErrCode, p3;

   DIB_ASSERT(Data);
   ErrCode  = (Data[0] & 0xFF);
   p3       = (Data[0] >> 8) & 0xFF;
   p1       =  Data[1];
   p2       =  Data[2];

   DIB_DEBUG(MSG_ERR, (CRB "****** FW ERROR (%d) => ", ErrCode));

   switch(ErrCode)
   {
   case FFERR_FRAME:
      DIB_DEBUG(MSG_ERR, ("Wrong frame Size (%d): is %d rows, should be %d" CRA, p3, p2, p1));
      break;
   /*case FFERR_DIV0:
      DIB_DEBUG(MSG_ERR, ("Division by 0 detected on svc %d" CRA, p3));
      break;*/
   case FFERR_A2B_FIFO:
      DIB_DEBUG(MSG_ERR, ("A to B fifo is full : svc = %d (Fill Lvl %d Len %d)" CRA, p3, p1, p2));
      break;
   case FFERR_CRC:
      DIB_DEBUG(MSG_ERR, ("CRC corrupted (0x%02x) on fifo[%d], svc %d" CRA, p1, p2, p3));
      break;
   case FFERR_TS_OVFL:
      DIB_DEBUG(MSG_ERR, ("TS input overflow (lvl %d Wr %d Rd %d)" CRA, p1, p2, p3));
      break;
   default:
      DIB_DEBUG(MSG_ERR, ("P1: %d P2: %d P3: %d" CRA, p1, p2, p3));
      break;
   }
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgEvent(struct DibDriverContext *pContext, uint16_t *Data)
{
   struct DibDriverEventInfoUp DataInfo;
   uint8_t Event = (Data[0] & 0xFF);
   uint8_t Svc   = (Data[0] >> 8) & 0xFF;

   DIB_ASSERT(Svc < DIB_MAX_NB_SERVICES);

   if(Event == 0)
   {
      DIB_DEBUG(MSG_LOG, (CRB "BURST_LOST: service %d (item %d)" CRA, Svc, pContext->FireflyService[Svc].ItemIndex));
      DataInfo.EventType = eBURST_LOST;
   }
   else if(Event == 1)
   {
      DIB_DEBUG(MSG_LOG, (CRB "BURST_FAILED: service %d (item %d)" CRA, Svc, pContext->FireflyService[Svc].ItemIndex));
      DataInfo.EventType = eBURST_FAILED;
   }
   else if(Event == 2)
   {
      DIB_DEBUG(MSG_LOG, (CRB "BURST_DETECTED: service %d (item %d)" CRA, Svc, pContext->FireflyService[Svc].ItemIndex));
      DataInfo.EventType = eDATA_DETECTED;
   }
   else
   {
      DIB_DEBUG(MSG_ERR, (CRB "INVALID Event: Event %d service %d (item %d)" CRA, Event, Svc, pContext->FireflyService[Svc].ItemIndex));
      return;
   }

   DataInfo.EventConfig.BurstLostEvent.ItemHdl = pContext->FireflyService[Svc].ItemIndex;
   DibDriverPostEvent(pContext, &DataInfo);
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgDebugBuf(struct DibDriverContext *pContext, uint32_t rx_nb, uint16_t *Data)
{
   int8_t *tmp;
   uint32_t ts;

   DIB_ASSERT(Data);

   ts  = Data[0] | Data[1] << 16;
   tmp = (int8_t *)(&Data[2]);

   tmp[2 * (rx_nb - 2) - 1] = '\0';      /* Bullet proof the Buffer */

   if(*tmp == '~')
   {
      tmp++;
      DIB_DEBUG(MSG_ERR, ("%s", tmp));
   }
   else
   {
      DIB_DEBUG(MSG_ERR, (" %u.%04u: ", ts / 10000, ts % 10000));

      if(*tmp)
      {
         DIB_DEBUG(MSG_ERR, (CRB "%s" CRA, tmp));
      }
      else
      {
         DIB_DEBUG(MSG_ERR, (CRB "<empty>" CRA));
      }
   }
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyProcessMsgVersion(struct DibDriverContext *pContext, uint16_t * Data)
{
   uint8_t *dataChar = (uint8_t *) Data;
   DIB_ASSERT(Data);

   pContext->ChipContext.EmbVersions[0] = (dataChar[0] << 8) + dataChar[1];
   pContext->ChipContext.EmbVersions[1] = (dataChar[2] << 8) + dataChar[3];
   pContext->ChipContext.SupportedModes = DVB_SUPPORT | BBM_SUPPORT;

   DibSetEvent(&pContext->ChipContext.GlobalMonitEvent);
}

/****************************************************************************
*
****************************************************************************/
static void IntDriverFireflyTestMsgChecker(struct DibDriverContext *pContext, uint16_t * Data)
{
   /* Proprietary message between bridge and driver */
   uint8_t ItemHdl   = (uint8_t) Data[0];
   uint32_t Cc       = ((uint32_t) Data[1]) | (((uint32_t) Data[2]) << 16);
   uint32_t Err      = ((uint32_t) Data[3]) | (((uint32_t) Data[4]) << 16);
   uint32_t Tot      = ((uint32_t) Data[5]) | (((uint32_t) Data[6]) << 16);
   FILTER_HDL        FilterHdl;
   enum DibDataType  FilterType;
   struct DibDriverMpeServiceCtx    *pMpe;
   struct DibDriverFilter           *pFilter;

   DIB_ASSERT(Data);

   /* Item has been removed since last call*/
   if(ItemHdl == DIB_UNSET)
      return;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);

   FilterHdl   = pContext->ItemInfo[ItemHdl].ParentFilter;
   FilterType  = pContext->FilterInfo[FilterHdl].TypeFilter;

   switch (FilterType)
   {
      case eMPEFEC:
         pMpe = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService;
         if(pMpe != NULL)
         {
            DIB_DEBUG(MSG_LOG, (CRB "Received IP checker monitoring for item %d" CRA, ItemHdl));
            /* Total is retreived from IN_MSG_MPE_MONITOR */
            pMpe->DvbHMonitoring.CcFailCnt      = Cc;
            pMpe->DvbHMonitoring.ErrCnt         = Err;
         }
         break;

      case eTS:
         pFilter = &pContext->FilterInfo[FilterHdl];
         if(pFilter != NULL)
         {
            DIB_DEBUG(MSG_LOG, (CRB "Received RAWTS checker monitoring for filter %d item %d" CRA, FilterHdl, ItemHdl));
            /* Total is retreived from IN_MSG_RAWTS_MONITOR */
            pFilter->DvbTMonitoring.CheckCcFailCnt     = Cc;
            pFilter->DvbTMonitoring.CheckErrCnt        = Err;
            pFilter->DvbTMonitoring.CheckTotalPackets  = Tot;
         }
         break;

      default:
         DIB_DEBUG(MSG_LOG, (CRB "Received unknown debug monitoring for item %d" CRA, ItemHdl));
         break;
   };

}


/****************************************************************************
* This function is called when a interrupt shows a message was sent to
* the host by one (or both) RISC(s).
*
* It handles inbound Data messages, debug messages, ...
*
* This function does not contain any locking function, as it is only called
* under an interruption context.
****************************************************************************/
static void IntDriverFireflyProcessMessage(struct DibDriverContext *pContext, uint32_t Size, uint16_t * Data)
{
   uint8_t  msg_id;
   uint8_t  value;
   uint32_t rx_nb;

   DIB_ASSERT(Data);

   /* retrieve information from msg */
   msg_id = Data[0] >> 8;
   rx_nb  = (Size/2) - 1;
   Data++;

   if(msg_id != 0XFF)
   {
      DIB_DEBUG(MSG_LOG, (CRB "Rcvd %s (%d)" CRA, msg_txt[msg_id], msg_id));
   }

   switch (msg_id)
   {
   case IN_MSG_MPE_MONITOR:     /*  mpe fec info timing */
      if(rx_nb < 15)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 15));
      }
      else
      {
         DibDriverFireflyProcessMsgMonitorMpeFec(pContext, Data);
      }
      break;

   case IN_MSG_RAWTS_MONITOR:     /*  DVB-T monitoring info */
      if(rx_nb < 5)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 3));
      }
      else
      {
         DibDriverFireflyProcessMsgMonitorRawTs(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_END_BRIDGE_APB_RW:       /*  end of read or write on bridge */
      if(rx_nb < 2)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 2));
      }
      else
      {
         IntDriverFireflyProcessMsgEndBridgeApbRw(pContext, Data);
      }
      break;

   case IN_MSG_FE_FW_DL_DONE:
      if(rx_nb < 1)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 1));
      }
      else
      {
         IntDriverFireflyProcessMsgFwDwld(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_END_BRIDGE_I2C_RW:       /*  end of read or write on bridge */
      IntDriverFireflyProcessMsgEndBridgeI2CRw(pContext, rx_nb, Data);
      break;

   case IN_MSG_END_OF_SCAN:
      if(rx_nb < 9)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 9));
      }
      else
      {
         IntDriverFireflyProcessMsgEndOfScan(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_MONIT_DEMOD:
      if(rx_nb < 16)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 16));
      }
      else
      {
         IntDriverFireflyProcessMsgMonitDemod(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_ERROR:     /*  Firmware Error */
      if(rx_nb < 3)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 3));
      }
      else
      {
         IntDriverFireflyProcessMsgError(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_EVENT:     /*  Table prog is not OK */
      if(rx_nb < 1)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 1));
      }
      else
      {
         IntDriverFireflyProcessMsgEvent(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_DEBUG_BUF:       /*  debug buf */
      if(rx_nb < 3)
      {
         DIB_DEBUG(MSG_ERR,(CRB "  message too short: %d instead of %d" CRA, rx_nb, 1));
      }
      else
      {
         IntDriverFireflyProcessMsgDebugBuf(pContext, rx_nb, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_VERSION:
      if(rx_nb < 1)
      {
         DIB_DEBUG(MSG_ERR,(CRB "  message too short: %d instead of %d" CRA, rx_nb, 1));
      }
      else
      {
         IntDriverFireflyProcessMsgVersion(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   case IN_MSG_ACK_CHANGE_SVC:
      value = pContext->FireflyService[(Data[0]&ALL_SVC_MSK)].ItemIndex;

      DIB_ASSERT(value < DIB_MAX_NB_ITEMS);
      pContext->ItemInfo[value].FlushAcq |= FF_MASK_RISCB;

      if(pContext->ItemInfo[value].FlushAcq == FF_ALL_RISC)
         DibSetEvent(&pContext->SvcChangeEvent);
      break;

   case IN_MSG_ACK_FREE_ITEM:
      value = pContext->FireflyService[(Data[0]&ALL_SVC_MSK)].ItemIndex;

      DIB_ASSERT(value < DIB_MAX_NB_ITEMS);
      pContext->ItemInfo[value].FlushAcq |= FF_MASK_RISCA;

      if(pContext->ItemInfo[value].FlushAcq == FF_ALL_RISC)
         DibSetEvent(&pContext->SvcChangeEvent);
      break;

   case IN_MSG_HBM_PROF:     /*  HBM profiling info */
      if(rx_nb < 3)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 3));
      }
      else
      {
         DibDriverFireflyProcessMsgHbmProf(pContext, Data);
      }
      break;

   case 0xFF:
      if(rx_nb < 7)
      {
         DIB_DEBUG(MSG_ERR, (CRB "  message too short: %d instead of %d" CRA, rx_nb, 7));
      }
      else
      {
         IntDriverFireflyTestMsgChecker(pContext, Data);
      }
      break;

      /* -------------------------------------------------------------------------- */
   default:
      DIB_DEBUG(MSG_LOG, (CRB "  IntDriverFireflyProcessMessage: ERROR: unknown msg_id %d" CRA, msg_id));
      break;
   }
}

/****************************************************************************
* Set FW debug help
****************************************************************************/
static DIBSTATUS IntDriverFireflySetDebugHelp(struct DibDriverContext *pContext, uint16_t i, uint16_t v, uint16_t ch)
{
   uint16_t buf[2];

   if(i >= 16)
      return DIBSTATUS_INVALID_PARAMETER;

   buf[0] = (OUT_MSG_DEBUG_HELP << 8) + 2;
   buf[1] = (uint8_t)i | (uint8_t)v << 8;

   return DibDriverSendMessage(pContext, (uint32_t*)buf, 4);
}

/****************************************************************************
* Writes in the chip (internal bus)
****************************************************************************/
static DIBSTATUS IntDriverFireflyApbWrite(struct DibDriverContext *pContext, uint32_t adr, uint32_t Data)
{
   uint16_t buf[10];
   uint16_t Nb = 3;

   buf[0] = (OUT_MSG_BRIDGE_APB_W << 8) + Nb;
   buf[1] = (adr & 0xffff);
   buf[2] = (Data & 0xffff);

   if(DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(MSG_ERR, (CRB "IntDriverFireflyApbWrite: could not write Buffer!" CRA));

      return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Read in the chip (internal bus)
****************************************************************************/
static DIBSTATUS IntDriverFireflyApbRead(struct DibDriverContext *pContext, uint32_t adr)
{
   uint16_t buf[10];
   uint16_t Nb = 2;

   buf[0] = (OUT_MSG_BRIDGE_APB_R << 8) + Nb;
   buf[1] = (adr & 0xffff);

   if(DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(MSG_ERR, (CRB "IntDriverFireflyApbRead: could not write Buffer!" CRA));
      return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Writes on the I2C bus control by the chip
****************************************************************************/
static DIBSTATUS IntDriverFireflyI2CWrite(struct DibDriverContext *pContext, uint8_t I2cAdr, uint8_t * txbuf, uint8_t nbtx)
{
   uint16_t buf[32];
   uint16_t Nb = 2;
   uint16_t k;
   uint32_t i;

   DIB_ASSERT(txbuf);

   if(nbtx > 8)
   {
      DIB_DEBUG(MSG_LOG, (CRB "  IntDriverFireflyI2CWrite too long  " CRA));
      return DIBSTATUS_ERROR;
   }

   for(k = 0; k < nbtx; k += 2)
   {
      if(k + 1 < nbtx)
         buf[2 + k / 2] = (txbuf[k] | ((uint16_t) txbuf[k + 1] << 8));
      else
         buf[2 + k / 2] = txbuf[k];
      Nb++;
   }

   buf[0] = (OUT_MSG_BRIDGE_I2C_W << 8) + Nb;
   buf[1] = (I2cAdr) | ((uint16_t) nbtx << 8);

   DIB_DEBUG(MSG_LOG, ("IntDriverFireflyI2CWrite: Buffer nbtx %d: ", nbtx));

   for(i = 0; i < Nb; i++)
      DIB_DEBUG(MSG_LOG, ("0x%x 0x%x ", (buf[i] >> 8) & 0xff, buf[i] & 0xff));

   DIB_DEBUG(MSG_LOG, (CRB "" CRA));

   if(DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(MSG_ERR, (CRB "IntDriverFireflyI2CWrite: cannot write " CRA));
      return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Read  on the I2C bus control by the chip
****************************************************************************/
static DIBSTATUS IntDriverFireflyI2CRead(struct DibDriverContext *pContext, uint8_t I2cAdr,
                                         uint8_t *txbuf, uint8_t nbtx, uint8_t * rxbuf, uint8_t nbrx)
{
   uint16_t buf[4];
   uint16_t Nb = 4;

   DIB_ASSERT(txbuf);
   DIB_ASSERT(rxbuf);

   if(nbtx > 2)
   {
      DIB_DEBUG(MSG_ERR, (CRB "  IntDriverFireflyI2CRead too long" CRA));
      return DIBSTATUS_ERROR;
   }

   if(nbrx > 8)
   {
      DIB_DEBUG(MSG_ERR, (CRB "  IntDriverFireflyI2CRead too long" CRA));
      return DIBSTATUS_ERROR;
   }

   buf[0] = (OUT_MSG_BRIDGE_I2C_R << 8) + Nb;
   buf[1] = (I2cAdr) | ((uint16_t) nbtx << 8);

   if(nbtx)
   {
      buf[2] = txbuf[0];

      if(nbtx == 2)
         buf[2] |= ((uint16_t) txbuf[1] << 8);
   }

   buf[3] = nbrx;

   if(DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(MSG_ERR, (CRB "IntDriverFireflyI2CRead: message sending failed" CRA));
      return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Message format:     bits  @BIT  @WORD
--------------------------------------
* no_of_words          8     0     0
* message_id           8     8     0
* SvcNb                6     0     1
* timesliced           1     6     1
* req Type             1     7     1
* ts channel           4     8     1
* format               4     12    1
* PID                  16    0     2
* fec rows (MSB)       8     0     3
* max burst duration   8     8     3
****************************************************************************/
DIBSTATUS DibDriverFireflySetCfgSvc(struct DibDriverContext *pContext, uint8_t tsch, uint16_t ReqType,  /*  Either add or remove PID. */
                                    uint16_t Pid, uint16_t SvcNb, uint8_t IsWatch,
                                    uint16_t Format, enum DibPidMode Prefetch)
{
#if (DEBUG_MODE == DEBUG_PRINT)
   static uint8_t *req_s[2]    = { (uint8_t *) "Delete", (uint8_t *) "Add" };
   static uint8_t *format_s[14] = { (uint8_t *) "",
                                   (uint8_t *) " SI/PSI",
                                   (uint8_t *) " Raw TS",
                                   (uint8_t *) " MPE",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) " PES VIDEO",
                                   (uint8_t *) " PES AUDIO",
                                   (uint8_t *) " PES OTHER",
                                   (uint8_t *) " PCR"};
#endif

   uint16_t  buf[4];
   ELEM_HDL  ItemHdl = pContext->FireflyService[SvcNb].ItemIndex;
   uint8_t   MaxBurstDuration = 0xff;
   uint8_t   fec_len          = 0;
   DIBSTATUS ret              = DIBSTATUS_ERROR;

   DIB_ASSERT(tsch < DIB_MAX_NB_TS_CHANNELS);
   DIB_ASSERT(ReqType < REQ_PID_MAX);
   DIB_ASSERT(Pid < DIB_MAX_NB_PIDS);
   DIB_ASSERT(SvcNb < DIB_MAX_NB_SERVICES);
   DIB_ASSERT((IsWatch == eDIB_TRUE) || (IsWatch == eDIB_FALSE));
   DIB_ASSERT((Prefetch == eACTIVE) || (Prefetch == ePREFETCH));

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);

   if(Format == FORMAT_MPE)
   {
      fec_len   = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService->RsNumRows >> 8;

      if(pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService->MaxBurstDuration != DIB_MAX_BURST_TIME_UNKNOWN)
         MaxBurstDuration = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService->MaxBurstDuration;

      if(MaxBurstDuration == MAX_BURST_TIME_NO_TIME_SLICE)
         IsWatch = 0;
      else
         IsWatch = 1;
   }

   DIB_DEBUG(MSG_LOG, (CRB "IntDriverFireflySetCfgSvc : %s%s service" CRA, req_s[ReqType], format_s[Format]));
   DIB_DEBUG(MSG_LOG, (CRB " SvcNb              %4d" CRA, SvcNb));
   DIB_DEBUG(MSG_LOG, (CRB " Pid                %4d" CRA, Pid));
   DIB_DEBUG(MSG_LOG, (CRB " TsChannelNb        %4d" CRA, tsch));
   DIB_DEBUG(MSG_LOG, (CRB " IsWatch            %4d" CRA, IsWatch));

   if(ReqType && Format == FORMAT_MPE)
   {
      DIB_DEBUG(MSG_LOG, (CRB " NbRows           %4d" CRA, fec_len << 8));
      DIB_DEBUG(MSG_LOG, (CRB " MaxBurstDuration %4d (10ms)" CRA, MaxBurstDuration));
   }

   if(Prefetch)
      SvcNb |= CONF_SVC_PREFETCH;

   if(IsWatch & 1 && Format != FORMAT_RAWTS)
      SvcNb |= CONF_SVC_TSLICED;

   if(ReqType & 1)
      SvcNb |= CONF_SVC_ADDING;
   else
      Pid = 0;

   if(ReqType == REQ_ADD_PID)
      SvcNb |= CONF_SVC_ADDING;
   else if(ReqType == REQ_MOD_PID)
      Pid    = 0xFFFF;   /* Mask to specify prefecth/activate Mode change */
   else
      Pid    = 0;

   /** In prefetch mode, just the riscB of the firefly cares about the new pid */
   if(ReqType != REQ_MOD_PID)
      pContext->ItemInfo[ItemHdl].FlushAcq = 0;
   else
      pContext->ItemInfo[ItemHdl].FlushAcq = FF_MASK_RISCA;

   DibResetEvent(&pContext->SvcChangeEvent);

   buf[0] = (OUT_MSG_CONF_SVC << 8) | 4;
   buf[1] = SvcNb | tsch << 8 | Format << 12;
   buf[2] = Pid;
   buf[3] = fec_len | MaxBurstDuration << 8;

   ret = DibDriverSendMessage(pContext, (uint32_t*)buf, 8);

   if(ret != DIBSTATUS_SUCCESS)
   {
      goto End;
   }

   ret = DibWaitForEvent(&pContext->SvcChangeEvent, 5000);       /* 1s */

   if(ret == DIBSTATUS_TIMEOUT)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverFireflySetCfgSvc: Timeout on Event" CRA));
   }
   else if(ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverFireflySetCfgSvc: Error waiting Event" CRA));
   }

End:
   return ret;
}

/**
 * Abort any pending TuneMonitRequests
 */
static DIBSTATUS IntDriverFireflyAbortTuneMonitChannel(struct DibDriverContext *pContext)
{
   uint16_t buf[2];

   buf[0] = (OUT_MSG_CTL_MONIT << 8) + 2;
   buf[1] = 0;                            /* Abort */

   return DibDriverSendMessage(pContext, (uint32_t*)buf, (2 << 1));
}

static DIBSTATUS IntDriverFireflyTuneMonitChannelEx(struct DibDriverContext *pContext, struct DibStream * pStream, CHANNEL_HDL ChannelHdl, struct DibTuneMonit * pTuneMonit)
{
   return DIBSTATUS_ERROR;
}

/**
* Volatile Scan of channel parameters and signal. Tune on volatile ts channel 3
*/
static DIBSTATUS IntDriverFireflyTuneMonitChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl,  uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibTuneMonit * pTuneMonit)
{
   enum DibDemodType Type = pTuneMonit->ChannelMonit.Type;
   struct DibTuneChan *pTc = &pTuneMonit->ChannelDescriptor;

	DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);
	DIB_ASSERT((Type == eSTANDARD_DVB) || (Type == (eSTANDARD_DVB | eFAST)));
	DIB_ASSERT(StreamParameters == 0);

   /* use ts_channel 3, associated with the channel info */
   pContext->FireflyChannel[3] = ChannelHdl;

   /* indicate the embedded firmware what is the used channel */
   pTc->ChannelHdl = ChannelHdl;

   /* Store user requested Values */
   pContext->ChannelInfo[ChannelHdl].MinNbDemod = MinNbDemod;
   pContext->ChannelInfo[ChannelHdl].MaxNbDemod = MaxNbDemod;

   if((MaxNbDemod == DIB_DEMOD_AUTO) || (MaxNbDemod ==  pContext->Hd.BoardConfig->NbFrontends))
      pContext->ChannelInfo[ChannelHdl].ChannelMonit.NbDemods = pContext->Hd.BoardConfig->NbFrontends;
   else
      pContext->ChannelInfo[ChannelHdl].ChannelMonit.NbDemods = 1;

   IntDriverFireflyUpdateDiversity(pContext);

   return DibDriverFireflyScanChannelDvb(pContext, 3, pTc, Type);
}

/**
* Tune on an existing channel
*/
static DIBSTATUS IntDriverFireflyTuneChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelIdx, uint8_t TsCh, struct DibTuneChan *pTc, enum DibDemodType Type)
{
   DIBSTATUS              ret    = DIBSTATUS_ERROR;

   DIB_ASSERT(pTc);
   DIB_ASSERT(ChannelIdx < DIB_MAX_NB_CHANNELS);

   DibResetEvent(&pContext->ChannelInfo[ChannelIdx].ScanEvent);

   /* We copy the given information to the context structure. The scan() and tune()
   * functions will need them */
   DibMoveMemory(&pContext->ChannelInfo[ChannelIdx].ChannelDescriptor, pTc, sizeof(struct DibTuneChan));

   /* Phase 1: scan the channel */
   ret = DibDriverFireflyScanChannelDvb(pContext, TsCh, pTc, Type);

   if(ret != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyTuneChannel: scan timeout: no answer from firmware" CRA));
   }
   /* Synchronous call handling */
   else
   {
      /* Asynchronous call handling */
      DibWaitForEvent(&pContext->ChannelInfo[ChannelIdx].ScanEvent, TUNE_TIMEOUT_MS);

      /* No lock needed since when event is signaled data has been copied */
      DibMoveMemory((pTc), &pContext->ChannelInfo[ChannelIdx].ChannelDescriptor, sizeof(struct DibTuneChan));

      if(pTc->ScanStatus != DIB_SCAN_SUCCESS)
      {
         if(pTc->ScanStatus == DIB_SCAN_TIMEOUT)
         {
            DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyTuneChannel: scan timeout: firmware answered it could not process request" CRA));
            return DIBSTATUS_TIMEOUT;
         }

         return DIBSTATUS_ERROR;
      }
   }
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Requests that the demod is initilized by the embedded software
****************************************************************************/
static DIBSTATUS IntDriverFireflyInitDemod(struct DibDriverContext *pContext, uint16_t FeId)
{
   uint16_t buf[31] = { 0 };
   struct DibDemodBoardConfig *pBoard;

   pBoard = pContext->Hd.BoardConfig;

   DIB_ASSERT(FeId < pBoard->NbFrontends);

   buf[0] = (uint16_t) (OUT_MSG_INIT_DEMOD << 8) | (sizeof(buf)/sizeof(uint16_t));
   buf[1] = (FeId << 8) | (pBoard->NbFrontends & 0xff);
   buf[2] = (pBoard->FeCfg[FeId].DemodI2CAdd << 8) | (pBoard->FeCfg[FeId].TunerI2CAdd);

   if(pContext->Hd.BoardConfig->DibDemod == DIB_DEMOD_7000)
   {
      /*tuner dib007X */
      buf[3] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->XtalHz >> 16) & 0xffff;
      buf[4] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->XtalHz)       & 0xffff;
      buf[5] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->vcxo >> 16)   & 0xffff;
      buf[6] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->vcxo)         & 0xffff;
      buf[7] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->timf >> 16)   & 0xffff;
      buf[8] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->timf)         & 0xffff;
      buf[9] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->PllPrediv << 8) | pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->PllRatio;

      buf[10] = (0 << 14) | (3 << 12) | (0 << 11) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->IoClkEnCore << 10) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->ByPclkDiv   << 5) | (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->EnableRefDiv << 4) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->PllBypass   << 3) | (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->PllRange << 1) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->PllReset    << 0);

      buf[11] = (3 << 4) | 1;
      buf[12] = (1 << 12) | (2 << 10) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->modulo   << 8) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->ADClkSrc << 7);
      buf[13] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Bw->SadCfg);

      buf[14] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc1Pt3);
      buf[15] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Setup);
      buf[16] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->InvGain);
      buf[17] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->TimeStabiliz);
      buf[18] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->AlphaLevel << 12) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->ThLock);

      buf[19] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->AlphaMant << 5) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->AlphaExp);

      buf[20] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->BetaMant << 6) | (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->BetaExp);
      buf[21] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->WbdRef);
      buf[22] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc1Max);
      buf[23] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc1Min);
      buf[24] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc2Max);
      buf[25] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc2Min);
      buf[26] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc1Pt1 << 8) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc1Pt2);

      buf[27] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc1Slope1 << 8) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc1Slope2);

      buf[28] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc2Pt1 << 8) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc2Pt2);

      buf[29] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc2Slope1 << 8) |
                (pBoard->FeCfg[FeId].UDemod.Dib7000.Agc->Agc2Slope2);

      buf[30] = (pBoard->FeCfg[FeId].UDemod.Dib7000.Drivers);
   }
   else if(pContext->Hd.BoardConfig->DibDemod == DIB_DEMOD_9000) /* 9080 */
   {
      buf[3]  = (pBoard->FeCfg[FeId].UDemod.Dib9000.Bw->XtalHz >> 16) & 0xffff;
      buf[4]  = (pBoard->FeCfg[FeId].UDemod.Dib9000.Bw->XtalHz)       & 0xffff;
      buf[5]  = (pBoard->FeCfg[FeId].UDemod.Dib9000.Bw->vcxo >> 16)   & 0xffff;
      buf[6]  = (pBoard->FeCfg[FeId].UDemod.Dib9000.Bw->vcxo) & 0xffff;
      buf[7]  = (pBoard->FeCfg[FeId].UDemod.Dib9000.Bw->timf >> 16)   & 0xffff;
      buf[8]  = (pBoard->FeCfg[FeId].UDemod.Dib9000.Bw->timf)         & 0xffff;
      buf[30] = (pBoard->FeCfg[FeId].UDemod.Dib9000.Drivers);
   }

   if(DibDriverSendMessage(pContext, (uint32_t*)buf, sizeof(buf)) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_ERROR;

   if(pBoard->FeCfg[FeId].PMUIsPresent)
   {
      uint8_t size = (sizeof(pBoard->FeCfg[FeId].PMU.Config)/sizeof(uint16_t)) +1;

      buf[0] = (uint16_t) ((OUT_MSG_INIT_PMU << 8) | size);
      DibMoveMemory(&buf[1], pBoard->FeCfg[FeId].PMU.Config, sizeof(pBoard->FeCfg[FeId].PMU.Config));

      if(DibDriverSendMessage(pContext, (uint32_t*)buf, size*2) != DIBSTATUS_SUCCESS)
         return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
*
****************************************************************************/
static DIBSTATUS IntDriverFireflyDwldSlaveFw(struct DibDriverContext *pContext, uint16_t FeId)
{
   DIBSTATUS Status =  DIBSTATUS_ERROR;

   struct DibDriverDmaCtx DmaCtx;
   struct DibDemodBoardConfig *pBoard = NULL;
   uint16_t buf[4] = {0};
   uint8_t *fw  = NULL;
   uint32_t len = 0;

   pBoard = pContext->Hd.BoardConfig;

   DIB_ASSERT(FeId < pBoard->NbFrontends);

   if(pBoard->DibDemod == DIB_DEMOD_7000)
   {
      fw  = (uint8_t *) pBoard->FeCfg[FeId].UDemod.Dib7000.FirmwareRiscB;
      len = pBoard->FeCfg[FeId].UDemod.Dib7000.FwRiscBSize;
   }
   else if(pBoard->DibDemod == DIB_DEMOD_9000)
   {
      fw  = (uint8_t *) pBoard->FeCfg[FeId].UDemod.Dib9000.FirmwareRiscB;
      len = pBoard->FeCfg[FeId].UDemod.Dib9000.FwRiscBSize;
   }
   else
   {
      DIB_DEBUG(MSG_ERR, ("IntDriverFireflyDwldSlaveFw : Invalid Configuration."));
      goto End;
   }

   DIB_DEBUG(MSG_LOG, ("Dwnld Slave firmware %d",FeId+1));

   DmaCtx.Dir         = DIBDRIVER_DMA_WRITE;
   DmaCtx.ChipAddr    = FF_FRONTEND_FIRMWARE_ADDRESS;
   DmaCtx.ChipBaseMin = 0;
   DmaCtx.ChipBaseMax = 0;
   DmaCtx.DmaLen      = len;
   DmaCtx.pHostAddr   = fw;
   DibD2BTransfertBuf(pContext, &DmaCtx);

   DibResetEvent(&pContext->FWDwldEvent);

   buf[0] = (uint16_t) (OUT_MSG_FE_FW_DL << 8) | 4;
   buf[1] = (uint16_t) FF_FRONTEND_FIRMWARE_ADDRESS;
   buf[2] = (uint16_t) len;
   buf[3] = FeId;

   if((Status = DibDriverSendMessage(pContext, (uint32_t*)buf, 8)) != DIBSTATUS_SUCCESS)
      goto End;

   if(DibWaitForEvent(&pContext->FWDwldEvent, 2000) != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(MSG_ERR, ("FWDwld timed out"));
      Status = DIBSTATUS_TIMEOUT;
      goto End;
   }
   if(pContext->ChipContext.FwDwldData != 0) {
      DIB_DEBUG(MSG_ERR, ("FWDwld failed"));
      Status = DIBSTATUS_ERROR;
      goto End;
   }

   Status = DIBSTATUS_SUCCESS;

End:
   return Status;
}

/****************************************************************************
 * Select a specific Driver Configuration
 ****************************************************************************/
static DIBSTATUS IntDriverFireflySetCfg(struct DibDriverContext *pContext, uint32_t Cfg)
{
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
 * Select a specific Driver Configuration
 ****************************************************************************/
static DIBSTATUS IntDriverFireflyDeInitFirmware(struct DibDriverContext *pContext)
{
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Sends a "get monit" command
****************************************************************************/
static DIBSTATUS IntDriverFireflyGetMonit(struct DibDriverContext *pContext, uint8_t ChannelHdl)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;
   uint16_t  buf[2];
   uint8_t   Nb = 2;                 /*  2 uint16_t */
   uint8_t   TSChannelNum;

   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);

   for(TSChannelNum = 0; TSChannelNum < DIB_NB_PERSISTENT_TS_CHANNELS; TSChannelNum++)
   {
      if(pContext->FireflyChannel[TSChannelNum] == ChannelHdl)
         break;
   }

   if(TSChannelNum == DIB_NB_PERSISTENT_TS_CHANNELS)
      return DIBSTATUS_INVALID_PARAMETER;

   buf[0] = Nb + (uint16_t) (OUT_MSG_MONIT_DEMOD << 8);
   buf[1] = TSChannelNum & 0xff;
   ret    = DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1);

   return ret;
}

/****************************************************************************
* Requests the embedded softwares send their versions
****************************************************************************/
static DIBSTATUS IntDriverFireflyGetEmbeddedInfo(struct DibDriverContext * pContext)
{
   uint16_t buf[4];
   uint8_t Nb = 2;

   buf[1] = 0;
   buf[0] = Nb + (uint16_t) (OUT_MSG_REQ_VERSION << 8);
   DibDriverSendMessage(pContext, (uint32_t *)buf, Nb << 1);

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Configures the GPIOs
****************************************************************************/
static DIBSTATUS IntDriverFireflySetCfgGpio(struct DibDriverContext * pContext, struct DibDemodGpioFn *pConfig, uint16_t FeId)
{
   uint16_t b[17] = { 0 };
   uint8_t  Nb = (sizeof(b)/sizeof(uint16_t));
   uint8_t i;
   struct DibDemodGpioFn *f;

   DIB_ASSERT(pConfig);

   /* Do not replace this by a memcopy of pConfig, as there will be
   * alignment issues */
   b[0]  = Nb + (uint16_t) (OUT_MSG_CONF_GPIO << 8);
   for (i = 0; i < GPIO_FUNC_MAX; i++) {
      f = &pConfig[i];
      if (f->Mask) {
         switch (f->Func) {
            case GPIO_FUNC_TUNER_ON:   b[1] = (uint16_t) f->Mask; b[2] = (uint16_t) f->Direction; b[3] = (uint16_t) f->Value; break;
            case GPIO_FUNC_TUNER_OFF:  b[4] = (uint16_t) f->Mask; b[5] = (uint16_t) f->Direction; b[6] = (uint16_t) f->Value; break;
            case GPIO_FUNC_LNA_ON:     b[7] = (uint16_t) f->Mask; b[8] = (uint16_t) f->Direction; b[9] = (uint16_t) f->Value; break;
            case GPIO_FUNC_LNA_OFF:    b[10] = (uint16_t) f->Mask; b[11] = (uint16_t) f->Direction; b[12] = (uint16_t) f->Value; break;
            case GPIO_FUNC_LNA_BYPASS: b[13] = (uint16_t) f->Mask; b[14] = (uint16_t) f->Direction; b[15] = (uint16_t) f->Value; break;
            default: break;
         }
      }
   }
   b[16] = FeId;

   return DibDriverSendMessage(pContext, (uint32_t*)b, Nb << 1);
}

/******************************************************************************
* IntDriverFireflySubbandSelect
******************************************************************************/
static DIBSTATUS IntDriverFireflySubbandSelect(struct DibDriverContext *pContext, struct DibDemodSubbandSelection *pConfig, uint16_t FeId)
{
   uint16_t buf[1 + 1 + (DIB_MAX_NB_SUBBANDS * 4) + 1];
   uint8_t  i, Nb = 0;

   struct DibDemodGpioFn    *p_gpio = NULL;
   struct DibDemodPwmFn     *p_pwm = NULL;
   struct DibDemodBoardConfig *pBoard = pContext->Hd.BoardConfig;

   DIB_ASSERT(pConfig);
   DIB_ASSERT(pConfig->Size < DIB_MAX_NB_SUBBANDS);

   /* If the whole frequency band is one subband then no action taken. */
   if(pConfig->Size == 0)
      return DIBSTATUS_SUCCESS;

   DIB_DEBUG(MSG_ERR, (CRB "  ANTENNA SUB BAND SELECTOR: NB=%d, TYPE=%d, MASK=%04x" CRA, DIB_MAX_NB_SUBBANDS, (int32_t) (pConfig->Type), pConfig->Mask));

   buf[0] = (uint16_t) (OUT_MSG_SUBBAND_SEL << 8);    /* Message header. */
   buf[1] = (uint16_t) pConfig->Size + (((uint16_t) (pConfig->Type)) << 8);

   if(pConfig->Type == 0) { /* GPIO */
      for(i = 0; i < pBoard->FeCfg[FeId].SubbandSel.Size; i++) {
         p_gpio = &(pConfig->Subband[i].u.Gpio);
         buf[2 + i*4] = pConfig->Subband[i].F_mhz;
         buf[3 + i*4] = p_gpio->Mask;
         buf[4 + i*4] = p_gpio->Direction;
         buf[5 + i*4] = p_gpio->Value;
         DIB_DEBUG(MSG_ERR, (CRB "  SUB BAND %d: F=%3d MHz, Msk=%04x, Dir=%04x, Val=%04x" CRA, i, buf[2+i*4], buf[3+i*4], buf[4+i*4], buf[5+i*4]));
      }
      Nb = 3 + i*4;
      buf[0]       |= Nb;
      buf[2 + i*4]  = FeId;
   }
   else if(pConfig->Type == 1) /* PWM */
   {
      for(i = 0; i < pBoard->FeCfg[FeId].SubbandSel.Size; i++)
      {
         p_pwm = &(pConfig->Subband[i].u.Pwm);
         buf[2 + i*4] = pConfig->Subband[i].F_mhz;
         buf[3 + i*4] = (uint16_t) p_pwm->FrequencyDivider + (((uint16_t) (p_pwm->Position)) << 8);
         buf[4 + i*4] = p_pwm->Slope;
         buf[5 + i*4] = p_pwm->Offset;
         DIB_DEBUG(MSG_ERR, (CRB "  SUB BAND %d: F_mhz=%d, Position=%d, FrequencyDivider=%d, Slope=%04x, Offset=%04x" CRA,
                  i, buf[2 + i*4], (buf[3 + i*4]>>8)&0xff, buf[3 + i*4]&0xff,  buf[4 + i*4], buf[5 + i*4]));
      }
      Nb = 3 + i*4;
      buf[0] |= Nb;
      buf[2 + i*4] = FeId;
   }
   else
   {
      DIB_DEBUG(MSG_ERR, (CRB "IntDriverFireflySubbandSelect: *** Error: Illegal subband selector Type (%d)" CRA, (int32_t) (pConfig->Type)));
      return DIBSTATUS_ERROR;
   }

   return DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1);
}

/******************************************************************************
* IntDriverFireflySetPrioritryCh
******************************************************************************/
static DIBSTATUS IntDriverFireflySetPrioritryCh(struct DibDriverContext *pContext, uint16_t TsChannelNumber)
{
   uint16_t buf[2];
   uint8_t  Nb = 2;

   DIB_ASSERT((TsChannelNumber < DIB_MAX_NB_TS_CHANNELS) || (TsChannelNumber == TS_CHANNEL_UNSET) );

   buf[0] = Nb + (uint16_t) (OUT_MSG_SET_PRIORITARY_CHANNEL << 8);
   buf[1] = TsChannelNumber;
   return DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1);
}

/**
 * Send a diversity message to the firefly and retune is needed
 * @param   pContext pointer to the context
 * @param   Enable   1 : diversity on, 0 : off in function of the strategy and the min/max number of demod by channel
 * @param   Tune     1 : indicates that if a tune is necessary (tsoff or rawts) it has to be done now
 * @param   return   DIBSTATUS
 */
static DIBSTATUS IntDriverFireflySetDiversity(struct DibDriverContext * pContext, uint16_t Enable, uint8_t Tune)
{
   DIBSTATUS res = DIBSTATUS_ERROR;
   uint16_t  buf[10];
   uint8_t   Nb = 2;
   uint8_t   tune = DIB_MAX_NB_FILTERS, ii = DIB_UNSET, jj = DIB_UNSET;

   DIB_DEBUG(MSG_LOG, (CRB "  IntDriverFireflySetDiversity: Set Diversity to %d" CRA, Enable));

   DIB_ASSERT(((Enable == 0) || (Enable == 1)));

   buf[0] = (OUT_MSG_ENABLE_DIVERSITY << 8) + Nb;
   buf[1] = Enable;

   /** Send message diversity */
   res = DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1);
   if(res == DIBSTATUS_SUCCESS)
      pContext->FireflyDiversityActiv = (uint8_t) Enable;
   else
      return res;

   /** If time slicing off or rawts, we have to force the tune if enable = 1, that's firefly's specific */
   if(Enable && Tune)
   {
      for(ii = 0; ii < DIB_MAX_NB_FILTERS; ii++)
      {
         if((pContext->FilterInfo[ii].TypeFilter == eTS) || ((pContext->FilterInfo[ii].TypeFilter == eMPEFEC) && (!pContext->FireflyChipTimeSlicing)))
         {
            tune = ii;
            break;
         }
      }
   }

   if(tune != DIB_MAX_NB_FILTERS)
   {
   /** what is the corresponding TsChannel Number of the filter ii */
      for(jj = 0; jj < DIB_MAX_NB_TS_CHANNELS; jj++)
      {
         if(pContext->FireflyChannel[jj] == pContext->FilterInfo[ii].ParentChannel)
            break;
      }
/*      DIB_ASSERT(jj < DIB_MAX_NB_TS_CHANNELS);*/
      DIB_ASSERT(jj == 0);  /**< in fact specific firefly : if ts off, only one ts channel number : 0 */

#if(DIB_DVB_STD == 1)
      pContext->ChannelInfo[pContext->FilterInfo[ii].ParentChannel].ChannelDescriptor.Std.Dvb.Nfft = eDIB_FFT_AUTO; /**< to force the scan */
#endif
/*      IntDriverFireflyScanChannel(pContext, jj, &(pContext->ChannelInfo[pContext->FilterInfo[ii].ParentChannel].ChannelDescriptor.Dvb));*/
      DIB_DEBUG(MSG_LOG, (CRB "IntDriverFireflySetDiversity : Scan channel to do" CRA));
   }
   return res;
}

/****************************************************************************
* Enables/Disables ts output
****************************************************************************/
static DIBSTATUS IntDriverFireflySetOutputMode(struct DibDriverContext *pContext, enum DibOutputMode Mode)
{
   uint16_t buf[10];
   uint8_t Nb = 2;

   buf[0] = (OUT_MSG_SET_OUTPUT_MODE << 8) + Nb;
   switch (Mode) {
   case eHIGH_Z:
       buf[1] = 0;
       break;
   case eMPEG2_PAR_GATED_CLK:
       buf[1] = 1;
       break;
   case eMPEG2_PAR_CONT_CLK:
       buf[1] = 2;
       break;
   case eMPEG2_SERIAL:
       buf[1] = 3;
       break;
   default:
       DIB_ASSERT(Mode == Mode+1);
       return DIBSTATUS_INVALID_PARAMETER;
   }

   DIB_DEBUG(MSG_LOG, (CRB "  IntDriverFireflySetOutputMode: Set Mode to %04x" CRA, Mode));

   return DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1);
}

/****************************************************************************
 * Requests a new stream
 ****************************************************************************/
static DIBSTATUS IntDriverFireflyGetStream(struct DibDriverContext *pContext, struct DibStream * pStream, enum DibDemodType Std, uint32_t Options, enum DibPowerMode EnPowerSaving)
{
   return DIBSTATUS_ERROR;
}

/****************************************************************************
 * Requests stream removal
 ****************************************************************************/
static DIBSTATUS IntDriverFireflyDeleteStream(struct DibDriverContext *pContext, struct DibStream * pStream)
{
   return DIBSTATUS_ERROR;
}

/****************************************************************************
 * Requests a new frontend
 ****************************************************************************/
static DIBSTATUS IntDriverFireflyAddFrontend(struct DibDriverContext *pContext, struct DibStream * pStream, struct DibFrontend * pFrontend, struct DibFrontend * pOutputFrontend)
{
   return DIBSTATUS_ERROR;
}

/****************************************************************************
 * Requests frontend removal
 ****************************************************************************/
static DIBSTATUS IntDriverFireflyRemoveFrontend(struct DibDriverContext *pContext, struct DibFrontend * pFrontend)
{
   return DIBSTATUS_ERROR;
}

/**
* Test if all filter are sipsi on a ts channel or not.
* @param pContext   pointer to the context
* @param tsChannel  numero of the concerned TS channel
* @return DIBSTATUS success (= only sipsi or empty) or error
*/
static enum DibBool IntDriverFireflyIsOnlySipsi(struct DibDriverContext *pContext, uint8_t tsChannel)
{
   uint8_t  fi, ch;

   DIB_ASSERT(tsChannel < DIB_NB_PERSISTENT_TS_CHANNELS);

   ch = pContext->FireflyChannel[tsChannel];
   DIB_ASSERT(ch < DIB_MAX_NB_CHANNELS);
   fi = pContext->ChannelInfo[ch].FirstFilter;

   while(fi != DIB_UNSET)
   {
      if(pContext->FilterInfo[fi].TypeFilter != eSIPSI)
         break;
      fi = pContext->FilterInfo[fi].NextFilter;
   }

   if(fi == DIB_UNSET)
   {
      return eDIB_TRUE;
   }
   else
   {
      return eDIB_FALSE;
   }
}

/**
* Switch bewtween two TS Channels of the firefly. If one of them is empty, the function simply move the pids to the other.
* Every items from the tsNew channel are removed and kept, then tsCurrent is moved to tsNew,
* and kept pids are added to tsCurrent channel.
* @param tsCurrent: a channel with active pids.
* @param tsNew: another channel
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverFireflyMoveTSChannel(struct DibDriverContext *pContext, uint8_t tsCurrent, uint8_t tsNew)
{
   CHANNEL_HDL ChannelHdl;
   FILTER_HDL 	FilterHdl;
   ELEM_HDL 	ElemHdl;
   DIBSTATUS   res = DIBSTATUS_ERROR;
   struct DibDelPidInfo DelPidInfo;

   DIB_ASSERT(tsCurrent < DIB_NB_PERSISTENT_TS_CHANNELS);
   DIB_ASSERT(tsNew < DIB_NB_PERSISTENT_TS_CHANNELS);

   /** 1: Remove all pids from tsCurrent (this will desactivate the frequency on tsCurrent channel) */
   ChannelHdl = pContext->FireflyChannel[tsCurrent];
   FilterHdl  = pContext->ChannelInfo[ChannelHdl].FirstFilter;

   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);

   DelPidInfo.TsChannelNumber = tsCurrent;

   while(FilterHdl != DIB_UNSET)
   {
      DIB_ASSERT((pContext->FilterInfo[FilterHdl].TypeFilter == eTS) || (pContext->FilterInfo[FilterHdl].TypeFilter == eMPEFEC) || (pContext->FilterInfo[FilterHdl].TypeFilter == eSIPSI));

      ElemHdl = pContext->FilterInfo[FilterHdl].FirstItem;

      while(ElemHdl != DIB_UNSET)
      {
         DelPidInfo.Pid = pContext->ItemInfo[ElemHdl].Config.Common.Pid;

         res = IntDriverDelPid(pContext, &DelPidInfo, ElemHdl, FilterHdl);

         DIB_ASSERT(res == DIBSTATUS_SUCCESS);

         ElemHdl = pContext->ItemInfo[ElemHdl].NextItem;
      }
      FilterHdl = pContext->FilterInfo[FilterHdl].NextFilter;
   }
   pContext->FireflyChannel[tsCurrent] = DIB_UNSET;

   /** 2 move tsNew to tsCurrent (this will unset pContext->FireflyChannel[tsNew] and set and pContext->FireflyChannel[tsCurrent]) */
   if(pContext->FireflyChannel[tsNew] != DIB_UNSET)
   {
      res = IntDriverFireflyMoveTSChannel(pContext, tsNew, tsCurrent);

      DIB_ASSERT(res == DIBSTATUS_SUCCESS);
   }

   /** 3: tune tsNew and update FireflyChannel */
   pContext->FireflyChannel[tsNew] = ChannelHdl;

   /* Success : add the new channel in the context */
   DibResetEvent(&pContext->ChannelInfo[ChannelHdl].ScanEvent);

   /* Phase 1: scan the channel */
   res = DibDriverFireflyScanChannelDvb(pContext, tsNew, &(pContext->ChannelInfo[ChannelHdl].ChannelDescriptor), pContext->ChannelInfo[ChannelHdl].Type);

   DIB_ASSERT(res == DIBSTATUS_SUCCESS);

   /* Asynchronous call handling */
   DibWaitForEvent(&pContext->ChannelInfo[ChannelHdl].ScanEvent, TUNE_TIMEOUT_MS);

   if(pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.ScanStatus != DIB_SCAN_SUCCESS)
   {
      if(pContext->ChannelInfo[ChannelHdl].ChannelDescriptor.ScanStatus == DIB_SCAN_TIMEOUT)
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyMoveTSChannel: scan timeout: firmware answered it could not process request" CRA));
         return DIBSTATUS_TIMEOUT;
      }

      return DIBSTATUS_ERROR;
   }

   /** 4: add all pids */
   FilterHdl = pContext->ChannelInfo[ChannelHdl].FirstFilter;

#if(DIB_SIPSI_DATA == 1)
   while(FilterHdl != DIB_UNSET)
   {
	  ElemHdl = pContext->FilterInfo[FilterHdl].FirstItem;

      while(ElemHdl != DIB_UNSET)
      {
         struct DibAddPidInfo PidInfo;

         DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eSIPSI);
         PidInfo.TsChannelNumber = tsNew;

         if(pContext->ItemInfo[ElemHdl].Config.SiPsi.Crc)
            PidInfo.PidTarget = eTARGET_SIPSI;
         else
            PidInfo.PidTarget = eTARGET_SIPSI_NO_CRC;

         PidInfo.Pid = pContext->ItemInfo[ElemHdl].Config.SiPsi.Pid;
         PidInfo.Specific.SiPsiSpecific.pidWatch = pContext->ItemInfo[ElemHdl].Config.SiPsi.PidWatch;

         res = IntDriverAddPid(pContext, &PidInfo, ElemHdl, FilterHdl);

         ElemHdl = pContext->ItemInfo[ElemHdl].NextItem;
         DIB_ASSERT(res == DIBSTATUS_SUCCESS);
      }

      FilterHdl = pContext->FilterInfo[FilterHdl].NextFilter;
   }
#endif

   return res;
}


/**
* Suppression of an item
* For the moment this function called the old api function
* @param pContext pointer to the current context
* @param ElemHdl Handler of the item that the user want to delete
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverFireflyRemoveItem(struct DibDriverContext *pContext, ELEM_HDL ElemHdl)
{
   DIBSTATUS res = DIBSTATUS_ERROR;
   uint8_t tsChNum;
   FILTER_HDL FilterHdl;
   struct DibDelPidInfo DelPidInfo;
   DIB_ASSERT(ElemHdl < DIB_MAX_NB_ITEMS);

   FilterHdl = pContext->ItemInfo[ElemHdl].ParentFilter;

   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   /** be sure this filter exists */
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter != eUNKNOWN_DATA_TYPE);

   /* if it is the last pid : flush the incoming buffers */
   if(pContext->FilterInfo[FilterHdl].NbActivePids == 1)
   {
      pContext->FilterInfo[FilterHdl].FlushActive = eDIB_TRUE;
      DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyRemoveItem Flush activated %d" CRA, FilterHdl));
   }

   for(tsChNum = 0; tsChNum < DIB_MAX_NB_TS_CHANNELS; tsChNum++)
   {
      if(pContext->FireflyChannel[tsChNum] == pContext->FilterInfo[FilterHdl].ParentChannel)
         break;
   }

   DIB_ASSERT(tsChNum < DIB_MAX_NB_TS_CHANNELS);

   DelPidInfo.Pid = pContext->ItemInfo[ElemHdl].Config.Common.Pid;

   DelPidInfo.TsChannelNumber = tsChNum;

   res = IntDriverDelPid(pContext, &DelPidInfo, ElemHdl, FilterHdl);

   /* Do not test the return code any more before calling IntDriverRemoveItem */
   /* This was bringing to possible infinite loop as ElemHdl can never be set to DIB_UNSET */
   IntDriverRemoveItem(pContext, ElemHdl);

   return res;
}

/**
* Removal of an existing filter.
* This removal implies the suppression of all items added before on this filter
* @param pContext pointer to the current context
* @param FilterHdl Handler of the filter that the user want to delete
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverFireflyDeleteFilter(struct DibDriverContext *pContext, FILTER_HDL FilterHdl)
{
   uint8_t     tsChNum, it;
   CHANNEL_HDL ChannelHnd;
   DIBSTATUS   res = DIBSTATUS_SUCCESS;

   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);

   ChannelHnd = pContext->FilterInfo[FilterHdl].ParentChannel;
   DIB_ASSERT(ChannelHnd != DIB_UNSET);
   pContext->FilterInfo[FilterHdl].FlushActive = eDIB_TRUE;

   /** find the ts_ch knowing the channel handler */
   for(tsChNum = 0; tsChNum < DIB_MAX_NB_TS_CHANNELS; tsChNum++)
   {
      if(pContext->FireflyChannel[tsChNum] == pContext->FilterInfo[FilterHdl].ParentChannel)
         break;
   }

   DIB_DEBUG(SOFT_IF_LOG, (CRB "delete filter : %d , tsch %d, chhdl %d" CRA, FilterHdl, tsChNum, pContext->FireflyChannel[tsChNum]));

   DIB_ASSERT(tsChNum < DIB_MAX_NB_TS_CHANNELS);

   /** First remove every items of the filter */
   it = pContext->FilterInfo[FilterHdl].FirstItem;

   while(it != DIB_UNSET)
   {
      res |= DibDriverSpecifRemoveItem(pContext, it);

      it  = pContext->FilterInfo[FilterHdl].FirstItem;
   }

   /** then remove the filter entry */
   IntDriverRemoveFilter(pContext, FilterHdl);

   /** possible change the Data Channel if was the data channel and now is only sipsi*/
   if(pContext->ChannelInfo[ChannelHnd].IsDataChannel && IntDriverFireflyIsOnlySipsi(pContext, tsChNum))
   {
      pContext->ChannelInfo[ChannelHnd].IsDataChannel = eDIB_FALSE;

      DIB_ASSERT(tsChNum < 2);
      if((pContext->FireflyChannel[1-tsChNum] != DIB_UNSET) && !IntDriverFireflyIsOnlySipsi(pContext, 1-tsChNum))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "Update Data channel with ChannelHnd %d (ts %d)" CRA, pContext->FireflyChannel[1-tsChNum], 1-tsChNum));

         res |= IntDriverFireflySetPrioritryCh(pContext, 1-tsChNum);
         DIB_ASSERT(res == DIBSTATUS_SUCCESS);

         pContext->ChannelInfo[pContext->FireflyChannel[1-tsChNum]].IsDataChannel = eDIB_TRUE;
      }
      else
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB "Unset Data channel with ChannelHnd %d (ts %d)" CRA, ChannelHnd, tsChNum));
      }
   }

   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyDeleteFilter:  %d" CRA, FilterHdl));

   return res;
}

/**
* Delete a channel.
* This suppression implies the suppression of all filters added before to this channel
* @param ChannelHdl Handler of the channel that the user want to delete
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverFireflyDeleteChannel(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl)
{
   uint8_t tsChNum, fi;

   if(ChannelHdl >= DIB_MAX_NB_CHANNELS)
      return DIBSTATUS_INVALID_PARAMETER;

   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);
   DIB_ASSERT(pContext->ChannelInfo[ChannelHdl].InUse);

   /** find the corresponding TS channel */
   for(tsChNum = 0; tsChNum < DIB_MAX_NB_TS_CHANNELS; tsChNum++)
   {
      if(pContext->FireflyChannel[tsChNum] == ChannelHdl)
         break;
   }

   DIB_ASSERT(tsChNum < DIB_MAX_NB_TS_CHANNELS);

   /** todo before : delete filter */
   fi = pContext->ChannelInfo[ChannelHdl].FirstFilter;
   while(fi != DIB_UNSET)
   {
      DibDriverSpecifDeleteFilter(pContext, fi);
      fi = pContext->ChannelInfo[ChannelHdl].FirstFilter;
   }

   pContext->FireflyChannel[tsChNum] = DIB_UNSET;

   IntDriverRemoveChannel(pContext, ChannelHdl, eDIB_FALSE);
   IntDriverFireflyUpdateDiversity(pContext);

   DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyDeleteChannel: free tsChannel %d" CRA, ChannelHdl));

   return DIBSTATUS_SUCCESS;
}

/**
* Addition of an item to a filter.
* For the moment this function asks the function of the old api
* @param pContext      pointer to the current context
* @param FilterHdl     associated filter
* @param nbElements    number of items to add
* @param pFilterDesc   pointer to the first item to add structure of information, which is an union depending of the Type of filter
* @param pElemHdl      pointer to the first item's handler, in case of success (chosed by the firmware in case of dragonfly)
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverFireflyAddItem(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, union DibFilters * pFilterDesc, ELEM_HDL * pElemHdl)
{
   struct    DibAddPidInfo PidInfo;
   DIBSTATUS res = DIBSTATUS_ERROR;
   uint8_t   tsChNum, ii;

   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter != eUNKNOWN_DATA_TYPE);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].ParentChannel != DIB_UNSET);
   DIB_ASSERT(pFilterDesc);
   DIB_ASSERT(pElemHdl);

   for(tsChNum = 0; tsChNum < DIB_MAX_NB_TS_CHANNELS; tsChNum++)
   {
      if(pContext->FireflyChannel[tsChNum] == pContext->FilterInfo[FilterHdl].ParentChannel)
         break;
   }

   DIB_ASSERT(tsChNum < DIB_MAX_NB_TS_CHANNELS);

   /** for firefly, we need to check that we have only one TS filter */
   /** for firefly, we need to check that we have only one PID by channel */

   *pElemHdl = DIB_UNSET;

   /* Take the first free item */
   for(ii = 0; ii < DIB_MAX_NB_ITEMS; ii++)
   {
      if(pContext->ItemInfo[ii].ParentFilter == DIB_UNSET)
         break;
   }

   if(ii == DIB_MAX_NB_ITEMS)
   {
      /* no more filter avalaible */
      return DIBSTATUS_RESOURCES;
   }
   else
   {
      /** Success : add the new item in the context */
      PidInfo.TsChannelNumber = tsChNum;

      switch(pContext->FilterInfo[FilterHdl].TypeFilter)
      {
      case eMPEFEC:
         res = DibDriverFireflyAddItemMpeFec(pContext, FilterHdl, pFilterDesc, &PidInfo);
         break;

      case eTS:
         res = DibDriverFireflyAddItemRawTs(pContext, pFilterDesc, &PidInfo);
         break;

      case ePESVIDEO:
         res = DibDriverFireflyAddItemPesVideo(pContext, pFilterDesc, &PidInfo);
         break;

      case ePESAUDIO:
         res = DibDriverFireflyAddItemPesAudio(pContext, pFilterDesc, &PidInfo);
         break;

      case ePESOTHER:
         res = DibDriverFireflyAddItemPesOther(pContext, pFilterDesc, &PidInfo);
         break;

      case ePCR:
         res = DibDriverFireflyAddItemPesPcr(pContext, pFilterDesc, &PidInfo);
         break;

      case eSIPSI:
         res = DibDriverFireflyAddItemSiPsi(pContext, pFilterDesc, &PidInfo);
         break;

      default:
         /** Others Items are not supported by the firefly */
         return DIBSTATUS_INVALID_PARAMETER;
      }

      if(res != DIBSTATUS_SUCCESS)
         return res;

      /** Add the item to the filter before IntDriverAddPid (Fixed) */
      IntDriverAddItem(pContext, ii, FilterHdl, pFilterDesc);
      (*pElemHdl) = ii;

      res = IntDriverAddPid(pContext, &PidInfo, ii, FilterHdl);

      if(res != DIBSTATUS_SUCCESS)
      {
         IntDriverRemoveItem(pContext, ii);
         (*pElemHdl) = DIB_UNSET;
      }else{
         /** There is at least one pid */
         pContext->FilterInfo[FilterHdl].FlushActive = eDIB_FALSE;
      }
   }

   return res;
}

/**
 * Creation of a filter.
 * Despite the name, no message is sent to the chip, in the firefly's case
 * @param pContext   pointer to the current context
 * @param ChannelHdl handler odf the associated channel
 * @param DataType   Type of the filter
 * @param pFilterHdl pointer of the new filter, if command successful, choosed by the firmware in dragonfly case
 * @return DIBSTATUS result
 */
static DIBSTATUS IntDriverFireflyCreateFilter(struct DibDriverContext *pContext, CHANNEL_HDL ChannelHdl, enum DibDataType DataType, struct DibDataBuffer * pDataBuf, FILTER_HDL * pFilterHdl)
{
   DIBSTATUS  res = DIBSTATUS_SUCCESS;
   uint8_t    tsChNum, ii, tsNew;
   uint32_t   TsDataChannel = DIB_UNSET;
   FILTER_HDL FilterHdl;

   if(ChannelHdl >= DIB_MAX_NB_CHANNELS)
      return DIBSTATUS_INVALID_PARAMETER;

   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);
   DIB_ASSERT((pContext->ChannelInfo[ChannelHdl].InUse));
   DIB_ASSERT(pDataBuf);
   DIB_ASSERT(pFilterHdl);

   /** find the ts_ch that corresponds to ChannelHdl */
   for(tsChNum = 0; tsChNum < DIB_NB_PERSISTENT_TS_CHANNELS; tsChNum++)
   {
      if(pContext->FireflyChannel[tsChNum] == ChannelHdl)
         break;
   }

   DIB_ASSERT(tsChNum < DIB_NB_PERSISTENT_TS_CHANNELS);

   /* Parameter checking */
   /** check if added filter is compatible with the other filters of the channel
       no CHECK if data type is eSIPSI, cause it can be added to any channel */
   if(DataType != eSIPSI)
   {
      FilterHdl = pContext->ChannelInfo[ChannelHdl].FirstFilter;
      while(FilterHdl != DIB_UNSET)
      {
         if(DataType == eTS)
         {
            if(pContext->FilterInfo[FilterHdl].TypeFilter != eSIPSI)
            {
               res = DIBSTATUS_RESOURCES;
               DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverFireflyCreateFilter : Unsupported mode - Hw limited resources."));
            }
         }
         else if(DataType == eMPEFEC)
         {
            if(!((pContext->FilterInfo[FilterHdl].TypeFilter == DataType) || (pContext->FilterInfo[FilterHdl].TypeFilter == eSIPSI)))
            {
               res = DIBSTATUS_RESOURCES;
               DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverFireflyCreateFilter : Unsupported mode - Hw limited resources."));
            }
         }
         else
         {
            if(!((pContext->FilterInfo[FilterHdl].TypeFilter != eMPEFEC) && (pContext->FilterInfo[FilterHdl].TypeFilter != eTS) && (pContext->FilterInfo[FilterHdl].TypeFilter != DataType)))
            {
               res = DIBSTATUS_RESOURCES;
               DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverFireflyCreateFilter : Unsupported mode - Hw limited resources."));
            }
         }
         FilterHdl = pContext->FilterInfo[FilterHdl].NextFilter;

         if(res != DIBSTATUS_SUCCESS)
            goto End;
      }
   }

   *pFilterHdl = DIB_UNSET;

   /** Take the first free filter : the filter number is choosed here and not in the firefly, in contrario of the dragonfly */
   for(ii = 0; ii < DIB_MAX_NB_FILTERS; ii++)
   {
      if(pContext->FilterInfo[ii].TypeFilter == eUNKNOWN_DATA_TYPE)
         break;
   }

   if(ii == DIB_MAX_NB_FILTERS)
   {
      /* no more filter avalaible */
      res = DIBSTATUS_RESOURCES;
   }
   else
   {
      /** Success : see if ts_ch need to be changed */
      switch (DataType)
      {
      case eTS:
      case ePESVIDEO:
      case ePESAUDIO:
      case ePESOTHER:
      case ePCR:
      case eMPEFEC:
         /** Find the current data channel, which corresponds to the firefly priority channel */
         if((pContext->FireflyChannel[0] != DIB_UNSET) && (pContext->ChannelInfo[pContext->FireflyChannel[0]].IsDataChannel))
         {
            TsDataChannel = 0;
         }
         if((pContext->FireflyChannel[1] != DIB_UNSET) && (pContext->ChannelInfo[pContext->FireflyChannel[1]].IsDataChannel))
         {
            TsDataChannel = 1;
         }

         /** need to change the TsChannelNumber, if possible */
         if(tsChNum == 2)
         {
            tsNew = DIB_MAX_NB_TS_CHANNELS;
            if((pContext->FireflyChannel[0] == DIB_UNSET) || (pContext->FireflyChannel[1] == DIB_UNSET))
            {
               tsNew = ((pContext->FireflyChannel[0] == DIB_UNSET) ? 0 : 1);

               /** Move 2 ts channel to tsNew, which is supposed free (otherwise the swith function doesn't work) */
               res = IntDriverFireflyMoveTSChannel(pContext, 2, tsNew);
               DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflyMoveTSChannel : %d => %d" CRA, 2, tsNew));
            }
            else
            {
              /** Check if the channel not marked active is only sipsi. In that cause, remove and keep every items and filters from ts_ch 2, move InativeTs */

               if(IntDriverFireflyIsOnlySipsi(pContext, 0))
               {
                  tsNew = 0;
               }
               else if(IntDriverFireflyIsOnlySipsi(pContext, 1))
               {
                  tsNew = 1;
               }

               if(tsNew != DIB_MAX_NB_TS_CHANNELS)
               {
                  res = IntDriverFireflyMoveTSChannel(pContext, 2, tsNew);
               }
               else
               {
                  res = DIBSTATUS_RESOURCES;
               }
            }
         }
         else
         {
            tsNew = tsChNum;
         }
         if((TsDataChannel == DIB_UNSET) && (res == DIBSTATUS_SUCCESS))
         {
            /** First MPE filter : priority channel */
            pContext->ChannelInfo[ChannelHdl].IsDataChannel = eDIB_TRUE;
            res = IntDriverFireflySetPrioritryCh(pContext, tsNew);
         }
         break;
      case eSIPSI:
         break;

      default:
         res = DIBSTATUS_INVALID_PARAMETER;
         break;
      }
   }

   if(res == DIBSTATUS_SUCCESS)
   {
      *pFilterHdl = ii;

      /** Clear monitoring info when RAWTS filter. It is done using a dedicated message as no add filter message exists for Firefly */
      if(DataType == eTS)
         IntDriverFireflyClearMonit(pContext, DIB_UNSET);

      /** Success : add the new filter in the context */
      IntDriverAddFilter(pContext, ii, ChannelHdl, DataType, pDataBuf);
   }

End:
   return res;
}

/**
 * Unpack stream parameters ad check for coherency
 */
static DIBSTATUS IntDriverFireflyCheckStreamParameters(uint32_t StreamParameters, enum DibOutputMode * OutputMode)
{
   uint32_t StreamType = StreamParameters & 0xFFFF;

   switch(StreamType)
   {
      case eSTREAM_TYPE_HOST_DMA:
         *OutputMode = eHIGH_Z;
         return DIBSTATUS_SUCCESS;

      case eSTREAM_TYPE_MPEG_1:
         if((StreamParameters >> 16) & 1)
         {
            /* Parallel mode */
            if((StreamParameters >> 18) & 1)
               *OutputMode = eMPEG2_PAR_CONT_CLK; /* Parallel continuous */
            else
               *OutputMode = eMPEG2_PAR_GATED_CLK; /* Parallel gated */
         }
         else
         {
            *OutputMode = eMPEG2_SERIAL; /* serial mode */
         }
         return DIBSTATUS_SUCCESS;

      default:
         return DIBSTATUS_INVALID_PARAMETER;
   }
}

/** Creation of a channel and send a message to the chip to tune or scan
 * @param pContext current context
 * @param MinNbDemod       min number of demodulation for this channel
 * @param MaxNbDemod       max number of demodulation for this channel
 * @param IsPriorityChannel    indication if this channel is to be used to retreive the Data. Among Channels linked to the same adapter (each channel
 *                         is associated with an adapter), only one is able to give Data. This is the case for DVB-H because two services existing
 *                         on two different frequencies are not synchronized together. Note that an inactive channel channel becomes automatically actives
 *                         if a Data filter is added to him and if no previous active channel exists.
 *                         Only ts 0 and 1 can be active channels for firefly, cause it corresponds to the Priority Channel.
 * @param pDescriptor      pointer to a structure with all the useful information for the configuration of the channel
 * @param pChannelHdl      pointer to the result channel (chosed by the firmware in case of dragonfly), if successful
 * @return DIBSTATUS result
 */
static DIBSTATUS IntDriverFireflyGetChannel(struct DibDriverContext *pContext, uint8_t MinNbDemod, uint8_t MaxNbDemod, uint32_t StreamParameters, struct DibChannel *pDescriptor, CHANNEL_HDL * pChannelHdl)
{
   struct DibDriverChannel *pCh;
   uint8_t   ii    = 0;
   uint8_t   ts_ch = DIB_MAX_NB_TS_CHANNELS;
   DIBSTATUS res   = DIBSTATUS_ERROR;
   enum DibOutputMode OutputMode;

   /* get output mode */
   if(IntDriverFireflyCheckStreamParameters(StreamParameters, &OutputMode) != DIBSTATUS_SUCCESS)
      return DIBSTATUS_INVALID_PARAMETER;

   /** channel handler is invalid until proben. It remains invalid in async mode until successful scan */
   *pChannelHdl = DIB_UNSET;

   /** Only the DVB standard is demodulated in the firefly */
   DIB_ASSERT((pDescriptor->Type == eSTANDARD_DVB) || (pDescriptor->Type == (eSTANDARD_DVB | eFAST)));

   /* RESTRICTION: Div firmware doesn't support SET_DIVERSITY function correctly yet.
    * So only permit to enable or disable diversity using MaxDemod=1 or AUTO or pBoard->NbFrontend */
   if((pContext->Hd.BoardConfig->NbFrontends > 1) && !((MaxNbDemod == 1) || (MaxNbDemod == DIB_DEMOD_AUTO) || (MaxNbDemod == pContext->Hd.BoardConfig->NbFrontends)))
      return DIBSTATUS_INVALID_PARAMETER;

   /** Take the first free channel : the channel number is choosed here and not in the firefly, in contrario of the dragonfly */
   for(ii = 0; ii < DIB_MAX_NB_CHANNELS; ii++)
   {
      if(pContext->ChannelInfo[ii].InUse == 0)
         break;
   }

   if(ii == DIB_MAX_NB_CHANNELS)
   {
      /** no more channel avalaible */
      return DIBSTATUS_RESOURCES;
   }
   DIB_ASSERT(pContext->ChannelInfo[ii].FirstFilter == DIB_UNSET);

   if(pContext->FireflyChipTimeSlicing == eDIB_FALSE)
   {
      if(pContext->FireflyChannel[0] != DIB_UNSET)
      {
         /* cannot support mode than two channel in tsoff mode */
         return DIBSTATUS_RESOURCES;
      }
      ts_ch = 0;
   }
   else
   {

      /** choose the adequate TsChannel of the firefly */

      /**  If ts channel 2 is free, use it cause it is almost ressource free. */
      if(pContext->FireflyChannel[2] == DIB_UNSET)
      {
         ts_ch = 2;
      }
      else
      {
         /** adding and inactive channel on ts 0 or 1 is possible only if a previous one exist on ts 0 or ts 1 (should be active) */
         if(pContext->FireflyChannel[0] == DIB_UNSET)
         {
            ts_ch = 0;
         }
         else if(pContext->FireflyChannel[1] == DIB_UNSET)
         {
            ts_ch = 1;
         }
         else
         {
            /** cause available ts_channel */
            return DIBSTATUS_ERROR;
         }
      }
   }

   DIB_DEBUG(SOFT_IF_LOG, (CRB "" CRA));
   DIB_DEBUG(SOFT_IF_LOG, (CRB "Internal TS channel used: %d" CRA, ts_ch));

   DIB_ASSERT(ts_ch < DIB_NB_PERSISTENT_TS_CHANNELS);

   pContext->FireflyChannel[ts_ch] = ii;

   IntDriverAddChannel(pContext, ii, MinNbDemod, MaxNbDemod, StreamParameters, pDescriptor->Type, &(pDescriptor->ChannelDescriptor));

   /* register the callback if exist in the channel info structure */
   if((MaxNbDemod == DIB_DEMOD_AUTO) || (MaxNbDemod ==  pContext->Hd.BoardConfig->NbFrontends))
      pContext->ChannelInfo[ii].ChannelMonit.NbDemods = pContext->Hd.BoardConfig->NbFrontends;
   else
      pContext->ChannelInfo[ii].ChannelMonit.NbDemods = 1;

   /** In function of the number of demod, is the diversity correctly configure in the firefly ? */
   IntDriverFireflyUpdateDiversity(pContext);

   /** tune! */
   res = IntDriverFireflyTuneChannel(pContext, ii, ts_ch, &(pDescriptor->ChannelDescriptor), pDescriptor->Type);

   /** Success : add the new channel in the context */
   pCh = &(pContext->ChannelInfo[ii]);

   if(res == DIBSTATUS_SUCCESS)
   {
      struct DibStream *pStream = &pContext->Stream[0];
      /* Configure Output mode now */
      IntDriverFireflySetOutputMode(pContext, OutputMode);

      pStream->StreamId = 0;
      pStream->AutoRemove = 0;
      pStream->NbConnFrontends = pContext->Hd.BoardConfig->NbFrontends;
      pStream->Options = 0;
      pStream->Std = 0;
      pStream->ChipTimeSlicing = 0 ;

      pContext->ChannelInfo[ii].pStream = pStream;

      /** in asynchronous mode, the handler is given just before callback */
      *pChannelHdl = (CHANNEL_HDL)ii;
   }
   else
   {
      /** flush! */
      DibZeroMemory(&(pCh->ChannelDescriptor), sizeof(struct DibTuneChan));
      IntDriverRemoveChannel(pContext, ii, eDIB_FALSE);
      pContext->FireflyChannel[ts_ch] = DIB_UNSET;
      IntDriverFireflyUpdateDiversity(pContext);
   }

   return res;
}

/**
* Diversity is only possible if every NbWantedDemod for each channel is different than 1
* @param pContext   pointer to the context of the driver
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverFireflyUpdateDiversity(struct DibDriverContext *pContext)
{
   DIBSTATUS ret         = DIBSTATUS_SUCCESS;
   uint8_t   diversity   = 1;
   uint8_t   ChannelId   = 0;
   uint8_t   InUse       = 0;

   /** Update all the number of demods by channel */
   for(ChannelId = 0; ChannelId < DIB_MAX_NB_CHANNELS; ChannelId++)
   {
      if(pContext->ChannelInfo[ChannelId].InUse)
      {
         InUse = 1;

         /** Firefly : All activated channels, prio or not, are taken into account for the diversity */
         if(pContext->ChannelInfo[ChannelId].ChannelMonit.NbDemods == 1)
         {
            diversity = 0;
            break;
         }
      }
   }

   if((pContext->FireflyDiversityActiv != diversity) && InUse)
   {
      /** the diversity of the firefly has to be set/unset */
      ret = IntDriverFireflySetDiversity(pContext, diversity, 1);
   }
   DIB_DEBUG(MSG_LOG, (CRB "IntDriverFireflyUpdateDiversity: status %d" CRA, ret));

   return ret;
}

static DIBSTATUS IntDriverFireflyGetPowerManagement(struct DibDriverContext * pContext, STREAM_HDL StreamHdl, enum DibBool * Enable)
{
   *Enable = pContext->FireflyChipTimeSlicing;

   return DIBSTATUS_SUCCESS;
}

/**
* Change the strategy of power of the chip
* @param pContext   pointer to the context of the driver
* @param TimeSlicing   Type of power management
* @return DIBSTATUS result
*/
static DIBSTATUS IntDriverFireflySetPowerManagement(struct DibDriverContext * pContext, enum DibBool TimeSlicing, STREAM_HDL StreamHdl)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;
   uint32_t  ii;
   uint16_t  buf[10];
   uint8_t   Nb = 2;
   uint32_t NbChannelUsed = 0;

   DIB_DEBUG(MSG_LOG, (CRB "  IntDriverFireflySetPowerManagement: Set power Mode to %d" CRA, TimeSlicing));

   DIB_ASSERT((TimeSlicing == eDIB_TRUE) || (TimeSlicing == eDIB_FALSE));

   for(ii = 0; ii < DIB_MAX_NB_CHANNELS; ii++)
   {
      if(pContext->ChannelInfo[ii].InUse)
         NbChannelUsed++;
   }

   /* tson/off is not supported when using more than one channel */
   if(NbChannelUsed > 1)
   {
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* if ts channel used is not 0, we need to switch to 0 cause the emb support tson/off mode only
    * for channel 0 */
   if(TimeSlicing == eDIB_FALSE)
   {
      for(ii = 0; ii < DIB_NB_PERSISTENT_TS_CHANNELS; ii++)
      {
         if(pContext->FireflyChannel[ii] != DIB_UNSET)
            break;
      }
      if((ii == 2) || (ii == 1))
      {
         ret = IntDriverFireflyMoveTSChannel(pContext, ii, 0);
         if(ret != DIBSTATUS_SUCCESS)
            return ret;
      }
   }

   buf[0] = (OUT_MSG_ENABLE_TIME_SLICE << 8) + Nb;
   buf[1] = (uint16_t)TimeSlicing;
   ret    = DibDriverSendMessage(pContext, (uint32_t*)buf, Nb << 1);

   if(ret == DIBSTATUS_SUCCESS)
      pContext->FireflyChipTimeSlicing = TimeSlicing;

   return ret;
}

/****************************************************************************/
/*  Cleanup service                                                         */
/****************************************************************************/
static void IntDriverFireflyClearSvc(struct DibDriverContext *pContext, uint8_t SvcNb)
{
   pContext->FireflyService[SvcNb].ValidPid  = PID_FILTER_EMPTY;
   pContext->FireflyService[SvcNb].ItemIndex = DIB_UNSET;
}

/****************************************************************************/
/*  try to find Pid in ts_channel ts_ch. Return the svc                     */
/****************************************************************************/
static DIBSTATUS IntDriverFireflySvcLookup(struct DibDriverContext *pContext, uint16_t Pid, uint8_t ts_ch, uint8_t * SvcNb, enum DibPidTargets target, enum DibDataType FilterType)
{
   uint8_t min, max, i;
   DIB_ASSERT(SvcNb);

   if(Pid == PID_FILTER_EMPTY)
   {
      if(target == eTARGET_MPE)
      {
         min = 0;
         max = DIB_MAX_MPE_SERVICES;
      }
      else if (target == eTARGET_RAW_TS)
      {
         min = 0;
         max = DIB_MAX_NB_SERVICES;
      }
      else
      {
         min = DIB_MAX_MPE_SERVICES;
         max = DIB_MAX_NB_SERVICES;
      }
   }
   else
   {
      min = 0;
      max = DIB_MAX_NB_SERVICES;
   }

   /* look for the service in the filter array */
   for(i = min; i < max; i++)
   {
      if(Pid == pContext->FireflyService[i].ValidPid)
      {
         if(Pid == PID_FILTER_EMPTY)
         {
            *SvcNb = i;
            break;
         }

         if(ts_ch == pContext->FireflyService[i].TsChannelNumber)
         {
            /* Check if the Pid already exists in the filter (a Pid can be requested in different filtertype) */
            FILTER_HDL j = pContext->ItemInfo[pContext->FireflyService[i].ItemIndex].ParentFilter;
            if(pContext->FilterInfo[j].TypeFilter == FilterType)
            {
               *SvcNb = i;
               /* Entry was found, bail out */
               break;
            }
         }
      }
   }

   /* service not found */
   if(i == max)
   {
      DIB_DEBUG(SOFT_IF_LOG, (CRB "IntDriverFireflySvcLookup: Pid %d not found in tschannel %d" CRA, Pid, ts_ch));
      return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
*
****************************************************************************/
DIBSTATUS IntDriverAddPid(struct DibDriverContext *pContext, struct DibAddPidInfo * pPidInfo, ELEM_HDL ItemHdl, FILTER_HDL FilterHdl)
{
   struct    DibDriverItem *pItem;
   uint8_t   SvcIdx  = DIB_MAX_NB_SERVICES;
   uint8_t   format  = 0;
   uint8_t   isWatch = 0;
   enum DibPidMode PidMode = eACTIVE;
   DIBSTATUS rc      = DIBSTATUS_ERROR;
   union DibInformBridge SetBridgeInfo;
   ELEM_HDL  AliasItemHdl = DIB_MAX_NB_ITEMS;

   DIB_ASSERT(pPidInfo);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(FilterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(FilterHdl == pContext->ItemInfo[ItemHdl].ParentFilter);

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      uint8_t *format_s[14] = { (uint8_t *) "",
                                   (uint8_t *) " SI/PSI",
                                   (uint8_t *) " Raw TS",
                                   (uint8_t *) " MPE",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) "",
                                   (uint8_t *) " PES VIDEO",
                                   (uint8_t *) " PES AUDIO",
                                   (uint8_t *) " PES OTHER",
                                   (uint8_t *) " PCR"};
   };
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, "DibDriverAddPid: Pid %d (%s) on tsChannel %d" CRA,
                                   pPidInfo->Pid, format_s[pPidInfo->PidTarget],
                                   pPidInfo->TsChannelNumber);
   }
#endif

   DIB_ASSERT(pPidInfo->TsChannelNumber < DIB_MAX_NB_TS_CHANNELS);

   /* Is it there already? */
   if(IntDriverFireflySvcLookup(pContext, pPidInfo->Pid, pPidInfo->TsChannelNumber, &SvcIdx, pPidInfo->PidTarget, pContext->FilterInfo[FilterHdl].TypeFilter) == DIBSTATUS_SUCCESS)
   {
      AliasItemHdl = pContext->FireflyService[SvcIdx].ItemIndex;
      /* Duplication of PID is possible on same filter type of a same channel if filter handlers are different */
      if((pContext->FilterInfo[FilterHdl].TypeFilter == eSIPSI) 
      && (pContext->ItemInfo[AliasItemHdl].ParentFilter != FilterHdl))
      {
         DIB_DEBUG(SOFT_IF_LOG, (CRB " IntDriverAddPid: PID %d alias of svc %d" CRA, pPidInfo->Pid, SvcIdx));
      }
      else
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB " IntDriverAddPid: PID %d already at svc %d" CRA, pPidInfo->Pid, SvcIdx));
         return DIBSTATUS_ERROR;
      }
   }
   /* Get the service number.. */
   else 
   {
      if(IntDriverFireflySvcLookup(pContext, PID_FILTER_EMPTY, pPidInfo->TsChannelNumber, &SvcIdx, pPidInfo->PidTarget, pContext->FilterInfo[FilterHdl].TypeFilter) != DIBSTATUS_SUCCESS)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB " IntDriverAddPid: Failed to get a service number." CRA));
         return DIBSTATUS_ERROR;
      }

      DIB_ASSERT(SvcIdx < DIB_MAX_NB_SERVICES);

      /* Make sure service is not already enabled */
      if(pContext->FireflyService[SvcIdx].ValidPid != PID_FILTER_EMPTY)
      {
         DIB_DEBUG(SOFT_IF_ERR, (CRB " IntDriverAddPid: PID %d already in service %d" CRA, pPidInfo->Pid, SvcIdx));
         return DIBSTATUS_ERROR;
      }

      /* enable the Pid. No need to protect these structures while the svc message is not sent to the firmware */
      pContext->FireflyService[SvcIdx].TsChannelNumber = pPidInfo->TsChannelNumber;
      pContext->FireflyService[SvcIdx].ValidPid        = pPidInfo->Pid;
      pContext->FireflyService[SvcIdx].ItemIndex       = ItemHdl;
   }

   DIB_DEBUG(SOFT_IF_LOG, (CRB " IntDriverAddPid: allocated service %d on tschannel %d for Pid %d" CRA,
                             SvcIdx, pPidInfo->TsChannelNumber, pPidInfo->Pid));

   pItem = &pContext->ItemInfo[ItemHdl];

   switch (pPidInfo->PidTarget)
   {
   /* ------------------------------------------------------------- */
   case eTARGET_SIPSI:
   case eTARGET_SIPSI_NO_CRC:
      if((rc = DibDriverFireflyAddPidSiPsi(pContext, pItem, pPidInfo, FilterHdl, AliasItemHdl, &format, &isWatch, &PidMode)) != DIBSTATUS_SUCCESS)
         IntDriverFireflyClearSvc(pContext, SvcIdx);
      break;

   /* ------------------------------------------------------------- */
   case eTARGET_RAW_TS:
      rc = DibDriverFireflyAddPidRawTs(pContext, pItem, FilterHdl, &format, &isWatch, &PidMode);
      break;

   /* ------------------------------------------------------------- */
   case eTARGET_PES_VIDEO:
      rc = DibDriverFireflyAddPidPesVideo(pContext, pItem, FilterHdl, &format, &isWatch, &PidMode);
      break;

   /* ------------------------------------------------------------- */
   case eTARGET_PES_AUDIO:
      rc = DibDriverFireflyAddPidPesAudio(pContext, pItem, FilterHdl, &format, &isWatch, &PidMode);
      break;

   /* ------------------------------------------------------------- */
   case eTARGET_PES_OTHER:
      rc = DibDriverFireflyAddPidPesOther(pContext, pItem, FilterHdl, &format, &isWatch, &PidMode);
      break;

   /* ------------------------------------------------------------- */
   case eTARGET_PCR:
      rc = DibDriverFireflyAddPidPesPcr(pContext, pItem, FilterHdl, &format, &isWatch, &PidMode);
      break;

   /* ------------------------------------------------------------- */
   case eTARGET_MPE:
      rc = DibDriverFireflyAddPidMpeFec(pContext, pItem, FilterHdl, &format, SvcIdx, &PidMode, pPidInfo);
      break;

   default:
      break;
   }

   if(rc != DIBSTATUS_SUCCESS)
      return rc;

   DIB_DEBUG(SOFT_IF_LOG, (CRB " IntDriverAddPid: adding PID %d as service %d" CRA, pPidInfo->Pid, SvcIdx));

   /** Do not inform bridge and firmware of an 'sdk alias' */
   if(AliasItemHdl == DIB_MAX_NB_ITEMS)
   {
      SetBridgeInfo.SetService.Svc       = SvcIdx /* firefly case */;
      SetBridgeInfo.SetService.ItemHdl   = ItemHdl;
      SetBridgeInfo.SetService.FilterHdl = FilterHdl;
      SetBridgeInfo.SetService.DataType  = pContext->FilterInfo[FilterHdl].TypeFilter;
      SetBridgeInfo.SetService.DataMode  = pContext->FilterInfo[FilterHdl].DataInfo.DataMode;
      SetBridgeInfo.SetService.Pid       = pPidInfo->Pid;

      /** Success : Give the bridge all the information needed to have the Data path in the way wanted by the user (ip or callback) */
      DibD2BInformBridge(pContext, eSET_SERVICE, &SetBridgeInfo);

      rc = DibDriverFireflySetCfgSvc(pContext,
            pPidInfo->TsChannelNumber,
            REQ_ADD_PID,
            pPidInfo->Pid,
            SvcIdx,
            isWatch,
            format,
            PidMode);
   }

   /* Free FF index as call failed for some reason */
   if(rc != DIBSTATUS_SUCCESS)
   {
      IntDriverFireflyClearSvc(pContext, SvcIdx);
      pContext->FilterInfo[FilterHdl].NbActivePids--;
   }
   else
   {
      /* Store Service Number in Item */
      pItem->ServId = SvcIdx;
   }

   return rc;
}

/****************************************************************************
*
****************************************************************************/
DIBSTATUS IntDriverDelPid(struct DibDriverContext *pContext, struct DibDelPidInfo *pDelPid,  ELEM_HDL elemHdl, FILTER_HDL filterHdl)
{
   struct DibDriverItem   *pItem;
   struct DibDriverFilter *pFilter;
   union DibInformBridge   SetBridgeInfo;
   DIBSTATUS               rc       = DIBSTATUS_ERROR;
   uint8_t                 SvcIdx;

   DIB_ASSERT(pDelPid);
   DIB_ASSERT(elemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(filterHdl < DIB_MAX_NB_FILTERS);
   DIB_ASSERT(filterHdl == pContext->ItemInfo[elemHdl].ParentFilter);

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFileMesg(gLogFile, "DibDriverDelPid: Pid %d on tsChannel %d" CRA, pDelPid->Pid, pDelPid->TsChannelNumber);
   }
#endif

   pItem   = &pContext->ItemInfo[elemHdl];
   pFilter = &pContext->FilterInfo[filterHdl];

   SvcIdx = pItem->ServId;
   DIB_ASSERT(SvcIdx < DIB_MAX_NB_SERVICES);

   /**
    * If an item is aliased to a main item, pContext->FireflyService[SvcIdx].ItemIndex corresponds to the main
    * item and thus this value is different than 'elemHdl'
    * The pid should be physically removed once all aliased items + the main item are removed.
    * If the main item is removed and have some aliased item connected, only the association
    * between Svc and Item should change in driver and in bridge, but the firmware need not be notified.
    */
   if(pContext->FireflyService[SvcIdx].ItemIndex == elemHdl)
   {
      /* pItem == the main item */
      if(pItem->AliasItem == DIB_UNSET)
      {
         /* Removing the main item having no connected alias */
         /*rc = DibDriverFireflySetCfgSvc(pContext, pDelPid->TsChannelNumber, REQ_DEL_PID, pDelPid->Pid, SvcIdx, 0, 0, eACTIVE);*/
         rc = DibDriverFireflySetCfgSvc(pContext, 
                                       pDelPid->TsChannelNumber, 
                                       REQ_DEL_PID, 
                                       pDelPid->Pid, 
                                       SvcIdx, 
                                       0, 
                                       (pFilter->TypeFilter == eMPEFEC ? FORMAT_MPE : 0), /* workaround for multi rawts */
                                       eACTIVE);

         if(rc != DIBSTATUS_SUCCESS)
         {
            DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverDelPid: deleting service %d IntDriverFireflySetCfgSvc failed %d" CRA, SvcIdx, rc));
            return rc;
         }

         /* Disable the Pid */
         IntDriverFireflyClearSvc(pContext, pItem->ServId);
      }
      else
      {
         /* Update Pid <-> Item association by getting the next alias as main item */
         pContext->FireflyService[pItem->ServId].ItemIndex = pItem->AliasItem;
      }

      /* Remove Main item - Main filter information from bridge */
      SetBridgeInfo.SetService.Svc       = SvcIdx;
      SetBridgeInfo.SetService.ItemHdl   = elemHdl;
      SetBridgeInfo.SetService.FilterHdl = filterHdl;
      SetBridgeInfo.SetService.DataType  = pContext->FilterInfo[filterHdl].TypeFilter;
      SetBridgeInfo.SetService.DataMode  = eUNSET;
      SetBridgeInfo.SetService.Pid       = pDelPid->Pid;

      /** Success : Reset the information in the bridge concerning the Data path */
      DibD2BInformBridge(pContext, eSET_SERVICE, &SetBridgeInfo);

      /* Free filter type specific memory */
      if(pFilter->TypeFilter == eSIPSI)
         rc = DibDriverFireflyDelPidSiPsi(pContext, pItem, elemHdl);
      else if(pFilter->TypeFilter == eMPEFEC)
         rc = DibDriverFireflyDelPidMpeFec(pContext, pItem, elemHdl);

      if(pItem->AliasItem != DIB_UNSET)
      {
         /* Inform bridge from new main item - main filter */
         SetBridgeInfo.SetService.ItemHdl   = pItem->AliasItem;
         SetBridgeInfo.SetService.FilterHdl = pContext->ItemInfo[pItem->AliasItem].ParentFilter;
         SetBridgeInfo.SetService.DataMode  = pContext->FilterInfo[filterHdl].DataInfo.DataMode; 

         /** Success : Reset the information in the bridge concerning the Data path */
         DibD2BInformBridge(pContext, eSET_SERVICE, &SetBridgeInfo);
      }
   }
   else
   {
      ELEM_HDL MainItemHdl = pContext->FireflyService[SvcIdx].ItemIndex;
      /* Removing an aliased item: Just update linked list. Bridge and firmware need not to be aware of that. */
      while(MainItemHdl != DIB_UNSET)
      {
         if(pContext->ItemInfo[MainItemHdl].AliasItem == elemHdl)
            break;
         MainItemHdl = pContext->ItemInfo[MainItemHdl].AliasItem;
      }
      DIB_ASSERT(MainItemHdl != DIB_UNSET);
      pContext->ItemInfo[MainItemHdl].AliasItem = pContext->ItemInfo[elemHdl].AliasItem;
   }

   pItem->ServId = DIB_UNSET;                         /* Clear Associated Service Id */
   DIB_ASSERT(pFilter->NbActivePids != 0);
   pFilter->NbActivePids--;
   DIB_DEBUG(SOFT_IF_LOG, (CRB " IntDriverDelPid: No of active pids: %d" CRA, pFilter->NbActivePids));

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Clear Monitoring Info
****************************************************************************/
static DIBSTATUS IntDriverFireflyClearMonit(struct DibDriverContext *pContext, ELEM_HDL ItemHdl)
{
   struct DibDriverItem *pItem;
   uint16_t buf[2];
   uint8_t SvcId;

   DIB_ASSERT(pContext);

   if(ItemHdl == DIB_UNSET)
      SvcId = FF_RAWTS_SVC;
   else
   {
      DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
      pItem = &pContext->ItemInfo[ItemHdl];
      SvcId = pItem->ServId;
   }
   DIB_ASSERT(SvcId < DIB_MAX_NB_SERVICES);

   buf[0] = (OUT_MSG_CTL_MONIT << 8) + 2;
   buf[1] = (SvcId << 8) | 0x1;                        /* Clear Monit Info */

   return DibDriverSendMessage(pContext, (uint32_t*) buf, (2 << 1));
}


/**
* IntDriverFireflySetHbm
*/
DIBSTATUS IntDriverFireflySetHbm(struct DibDriverContext *pContext, uint8_t buffering_mode)
{
   uint16_t buf[2];
   uint8_t blocks, len;

   if(buffering_mode)
   {
	  blocks = FF_HBM_MAX_FRAGMENTS;
	  len    = FF_HBM_SLICE_LINES;
   }
   else
   {
	  blocks = 0;
	  len    = 0;
   }


   DIB_DEBUG(MSG_LOG, (CRB "  IntDriverFireflySetHbm: %d blocks, slice len %d" CRA, blocks, len));
   buf[0] = (OUT_MSG_SET_HBM << 8) | 2;
   buf[1] = blocks | len << 8;

   return DibDriverSendMessage(pContext, (uint32_t*)buf, 4);
}

DIBSTATUS IntDriverFireflyGetChannelEx(struct DibDriverContext *pContext, struct DibStream * pStream, struct  DibChannel *pDescriptor, CHANNEL_HDL  *pChannelHdl)
{
   return DIBSTATUS_ERROR;
}

/******************************************************************************
* dibOutMessageOps::msgFireflyOps
* Specific output message formating for dragonfly
******************************************************************************/
void DibDriverFireflyRegisterChip(struct DibDriverContext *pContext)
{
   /** mac functions */
   pContext->ChipOps.MacResetCpu              = IntDriverFireflyResetCpu;
   pContext->ChipOps.MacInitCpu               = IntDriverFireflyInitCpu;
   pContext->ChipOps.MacStartCpu              = IntDriverFireflyStartCpu;
   pContext->ChipOps.MacUploadMicrocode       = IntDriverFireflyUploadMicrocode;
   pContext->ChipOps.MacInitMailbox           = IntDriverFireflyInitMailbox;
   pContext->ChipOps.MacTestRamInterface      = IntDriverFireflyTestRamInterface;
   pContext->ChipOps.MacEnableClearIrq        = IntDriverFireflyEnableClearIrq;

   /** register_if functions */
   pContext->ChipOps.RegisterIfInit           = IntDriverFireflyRegisterIfInit;
   pContext->ChipOps.WriteDemod               = IntDriverFireflyWriteDemod;
   pContext->ChipOps.ReadDemod                = IntDriverFireflyReadDemod;

   /** packet demux */
   pContext->ChipOps.GetSiPsiInfo             = DibDriverFireflyGetSipsiInfo;

   /** frontend functions */
   pContext->ChipOps.FrontendInit             = IntDriverFireflyFrontendInit;
   pContext->ChipOps.FrontendRfTotalGain      = DibDriverFireflyFrontendRfTotalGain;

   /** handler for input messages */
   pContext->ChipOps.ProcessMessage           = IntDriverFireflyProcessMessage;

   /** output message functions */
   pContext->ChipOps.SetDebugHelp             = IntDriverFireflySetDebugHelp;
   pContext->ChipOps.ApbWrite                 = IntDriverFireflyApbWrite;
   pContext->ChipOps.ApbRead                  = IntDriverFireflyApbRead;
   pContext->ChipOps.I2CWrite                 = IntDriverFireflyI2CWrite;
   pContext->ChipOps.I2CRead                  = IntDriverFireflyI2CRead;
   pContext->ChipOps.AbortTuneMonitChannel    = IntDriverFireflyAbortTuneMonitChannel;
   pContext->ChipOps.TuneMonitChannel         = IntDriverFireflyTuneMonitChannel;
   pContext->ChipOps.TuneMonitChannelEx       = IntDriverFireflyTuneMonitChannelEx;
   pContext->ChipOps.InitDemod                = IntDriverFireflyInitDemod;
   pContext->ChipOps.GetMonit                 = IntDriverFireflyGetMonit;
   pContext->ChipOps.GetEmbeddedInfo          = IntDriverFireflyGetEmbeddedInfo;
   pContext->ChipOps.SetCfgGpio               = IntDriverFireflySetCfgGpio;
   pContext->ChipOps.SubbandSelect            = IntDriverFireflySubbandSelect;
   pContext->ChipOps.SetHbm                   = IntDriverFireflySetHbm;
   pContext->ChipOps.EnableCas                = NULL;
   pContext->ChipOps.GetChannel               = IntDriverFireflyGetChannel;
   pContext->ChipOps.DeleteChannel            = IntDriverFireflyDeleteChannel;
   pContext->ChipOps.CreateFilter             = IntDriverFireflyCreateFilter;
   pContext->ChipOps.DeleteFilter             = IntDriverFireflyDeleteFilter;
   pContext->ChipOps.AddItem                  = IntDriverFireflyAddItem;
   pContext->ChipOps.RemoveItem               = IntDriverFireflyRemoveItem;
   pContext->ChipOps.SetPowerManagement       = IntDriverFireflySetPowerManagement;
   pContext->ChipOps.GetPowerManagement       = IntDriverFireflyGetPowerManagement;
   pContext->ChipOps.SetPidMode               = DibDriverFireflySetPidMode;
   pContext->ChipOps.DwldSlaveFw              = IntDriverFireflyDwldSlaveFw;
   pContext->ChipOps.ClearMonit               = IntDriverFireflyClearMonit;
   pContext->ChipOps.SetCfg                   = IntDriverFireflySetCfg;
   pContext->ChipOps.DeInitFirmware           = IntDriverFireflyDeInitFirmware;
   pContext->ChipOps.GetStreamInfo            = NULL;
   pContext->ChipOps.GetFrontendInfo          = NULL;
   pContext->ChipOps.GetLayoutInfo            = NULL;
   pContext->ChipOps.GetStream                = IntDriverFireflyGetStream;
   pContext->ChipOps.DeleteStream             = IntDriverFireflyDeleteStream;
   pContext->ChipOps.AddFrontend              = IntDriverFireflyAddFrontend;
   pContext->ChipOps.RemoveFrontend           = IntDriverFireflyRemoveFrontend;
   pContext->ChipOps.GetChannelEx             = IntDriverFireflyGetChannelEx;

#if (DIB_INTERNAL_DEBUG == 1)
    pContext->ChipOps.SubmitBlock             = IntDriverFireflySubmitBlock;
#endif
}


#endif /* USE_FIREFLY */
