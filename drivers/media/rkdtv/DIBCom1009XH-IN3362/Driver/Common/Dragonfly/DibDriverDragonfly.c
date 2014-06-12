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
* @file "DibDriverDragonfly.c"
* @brief Dragonfly Driver specific functionality.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)
#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverDowncalls.h"
#include "DibBoardSelection.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"
#include "DibDriverDragonflyIntDbg.h"
#include "DibFirmwareSelection.h"

#define __FIELD_SPANNING__                 1

#define MAX_IV_SIZE   16
#define MAX_KEY_SIZE  32

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

int32_t IvNum[MAX_IV_SIZE/4]={
0x00010203,
0x04050607,
0x08090A0B,
0x0C0D0E0F
};

int32_t CrKey[MAX_KEY_SIZE/4]={
0x508a3d90,
0x92cd3bb7,
0xe14f2b44,
0xff6a2130,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
};

/** BigEndian to Cpu short convertion */
static __inline uint16_t IntReadShortBigEndian(uint16_t BigEndianShort)
{
   return ((BigEndianShort >> 8) & 0xFF) | ((BigEndianShort & 0xFF) << 8);
}

/** BigEndian to Cpu long convertion */
static __inline uint32_t IntReadLongBigEndian(uint32_t BigEndianLong)
{
   return ( ((BigEndianLong >> 24)  & 0xFF)
           |(((BigEndianLong >> 16) & 0xFF) << 8)
           |(((BigEndianLong >> 8)  & 0xFF) << 16)
           |((BigEndianLong & 0xFF) << 24) );
}


/****************************************************************************
 * Reset the Leon, this can be necessary before downloading the firmware.
 ****************************************************************************/
static void DibDriverWriteReg32_(struct DibDriverContext * pContext, uint32_t Addr, uint32_t Value)
{
   DibDriverWriteReg32(pContext, Addr, Value);
   /*DibMSleep(1);*/
}

void IntDriverVoyager1ResetCpu(struct DibDriverContext *pContext)
{
   DIB_DEBUG(SOFT_LOG, (CRB "DibBoard29098Init" CRA));

   DibDriverWriteReg32_(pContext, 0x8000d040, 0xfcf);
   DibDriverWriteReg32_(pContext, 0x8000d044, 0x000);
   DibMSleep(10);
   DibDriverWriteReg32_(pContext, 0x8000d044, 0xc00);
   DibDriverWriteReg32_(pContext, 0x8000d044, 0xfc1);
   DibDriverWriteReg32_(pContext, 0x8000a0ac, 0x3);
   DibDriverWriteReg32_(pContext, 0x800000a0, 0xf36f);
   DibDriverWriteReg32_(pContext, 0x800000a4, 0xf0);
   DibDriverWriteReg32_(pContext, 0x800000a0, 0xf34f);
   DibDriverWriteReg32_(pContext, 0x800000a0, 0xf36f);
   DibDriverWriteReg32_(pContext, 0x800000a0, 0xf36f);
   DibDriverWriteReg32_(pContext, 0x800000a0, 0xf32f);
   DibDriverWriteReg32_(pContext, 0x800000a0, 0xf36f); 

   DibDriverWriteReg32_(pContext, 0x8000d10c, 0); 
   DibDriverWriteReg32_(pContext, 0x8000d110, 1); 
}

void IntDriverUpdateHostBusDrive(struct DibDriverContext *pContext)
{
   /*** Default Value = 0 ***/
   /*** Minimum Value = 0 ***/
   /*** Maximum Value = 7 ***/
   uint8_t  DriveHostBus0 = 7;
   uint8_t  DriveHostBus1 = 7;
   uint8_t  DriveHostBus2 = 7;
   uint8_t  DriveHostBus3 = 7;
   uint8_t  DriveHostBus4 = 7;
   uint8_t  DriveHostBus5 = 7;
   uint8_t  DriveHostBus6 = 7;
   uint8_t  DriveHostBus7 = 7;
   uint8_t  DriveHostBus8 = 7;
   uint8_t  DriveHostBus9 = 7;
   uint8_t  DriveHostBus10 = 7;
   uint8_t  DriveHostBus11 = 7;
   uint8_t  DriveHostBus12 = 7;
   uint8_t  DriveHostBus13 = 7;
   uint8_t  DriveHostBus14 = 7;
   uint8_t  DriveHostBus15 = 7;

   uint32_t Word = 0;

   DibDriverReadReg32(pContext, pContext->DragonflyRegisters.JedecAddr, &Word);
   DibDriverReadReg32(pContext, pContext->DragonflyRegisters.JedecAddr, &Word);

   if (pContext->Hd.BoardConfig->DibChipVersion == VERSION(1,1))
   {

      DibDriverWriteReg32_(pContext, 0x80012214, ((DriveHostBus0 << 12) | 
               (DriveHostBus1 << 18) | 
               (DriveHostBus2 << 24)));
      DibDriverWriteReg32_(pContext, 0x80012218, ((DriveHostBus3 << 0) | 
               (DriveHostBus4 << 6) | 
               (DriveHostBus5 <<12)| 
               (DriveHostBus6 << 18) | 
               (DriveHostBus7 << 24)));
      DibDriverWriteReg32_(pContext, 0x8001221c, ((DriveHostBus8 << 0) | 
               (DriveHostBus9 << 6) | 
               (DriveHostBus10 <<12)| 
               (DriveHostBus11 << 18) | 
               (DriveHostBus12 << 24)));
      DibDriverWriteReg32_(pContext, 0x80012220, ((DriveHostBus13 << 0) | 
               (DriveHostBus14 << 6) | 
               (DriveHostBus15 <<12)));
   }
   else if ((pContext->Hd.BoardConfig->DibChipVersion == VERSION(2,0)) ||
           (pContext->Hd.BoardConfig->DibChipVersion == VERSION(2,1)))
   {
      DibDriverWriteReg32_(pContext, 0x80012210, ((DriveHostBus0 << 13) | 
               (DriveHostBus1 << 20) | 
               (DriveHostBus2 << 27)));
      DibDriverWriteReg32_(pContext, 0x80012214, ((DriveHostBus3 << 2) | 
               (DriveHostBus4 << 9) | 
               (DriveHostBus5 <<16)| 
               (DriveHostBus6 << 23)));  
      DibDriverWriteReg32_(pContext, 0x80012218, ((DriveHostBus7 << 0) | 
               (DriveHostBus8 << 7) | 
               (DriveHostBus9 <<14)| 
               (DriveHostBus10 << 21) | 
               (DriveHostBus11 << 28)));
      DibDriverWriteReg32_(pContext, 0x8001221c, ((DriveHostBus12 << 3) | 
               (DriveHostBus13 << 10) | 
               (DriveHostBus14 << 17) | 
               (DriveHostBus15 << 24)));
   }

}
void IntDriverNautilus1ResetCpu(struct DibDriverContext *pContext)
{
   uint32_t Word = 0;
   uint16_t data16;

   IntDriverUpdateHostBusDrive(pContext);

   /* Async interf */
   DibDriverWriteReg32_(pContext, 0x8000b028, 0x820);

   /* Check Id */
   DibDriverReadReg32(pContext, pContext->DragonflyRegisters.JedecAddr, &Word);
   if(Word != pContext->DragonflyRegisters.JedecValue)
   {
      DibDriverReadReg32(pContext, pContext->DragonflyRegisters.JedecAddr, &Word);
      if(Word != pContext->DragonflyRegisters.JedecValue) {
         DIB_DEBUG(SOFT_ERR, (CRB "Invalid Id(0x%X : 0x%X)" CRA, pContext->DragonflyRegisters.JedecValue, Word));
         return;
      }
   }

   switch(pContext->Hd.BoardConfig->DibChipVersion) {

      case VERSION(1,0):
        break;

      case VERSION(2,0):
      case VERSION(2,1):
        /* open all system ram */
        DibDriverWriteReg32_(pContext,  0x8000d040, 0x44CC0);  
        DibDriverWriteReg32_(pContext,  0x8000d044, 0x44CC0);  
        DibDriverWriteReg32_(pContext, 0x8000b038, 0);
        DibDriverWriteReg32_(pContext, 0x8000b03c, 0xffffffff);
        break;

      case VERSION(1,1):

        DibDriverWriteReg32_(pContext,  0x8000a028, 0x0  );     /* P_demod_pwr_en = 0 */

        /*** PMU Startup **/ 
        DibDriverWriteReg32_(pContext,  0x8000a030, 0x100  );          
        DibDriverWriteReg32_(pContext,  0x8000a034, 0x40000  );       
        DibDriverWriteReg32_(pContext,  0x8000a034, 0x42000  );
        DibDriverWriteReg32_(pContext,  0x8000a030, 0x500  );
        DibDriverWriteReg32_(pContext,  0x8000a030, 0x8500  );
        DibMSleep(1);
        DibDriverWriteReg32_(pContext,  0x8000a030, 0x8D00  );
        DibDriverWriteReg32_(pContext,  0x8000a030, 0xC08D00  );
        DibDriverWriteReg32_(pContext,  0x8000a030, 0x40c08d00  );
        DibDriverWriteReg32_(pContext,  0x8000a034, 0x42008  );

        /** Power On Octopus, and reset ***/
        DibDriverWriteReg32_(pContext,  0x8000a028, 0x1  );

        DibDriverWriteReg32_(pContext,0x8000d040, 0xcc05); /* enable  DCOM, ROM, AHB_RAM, STREAMIF, PROC and URAM clocks */
        DibDriverWriteReg32_(pContext,0x8000d044, 0xcc05); /* release DCOM, ROM, AHB_RAM, STREAMIF, PROC and URAM resets */

        /** Configure URAM **/   
        DibDriverWriteReg32_(pContext,  0x8000d100, 0xc  );         /*  wake-up delay and reset delay */
        DibDriverWriteReg32_(pContext,  0x8000d104, 0x303d  );      /* refresh anticipation and period */
        DibDriverWriteReg32_(pContext,  0x8000d108, 0x0  );         /*                                 */
        DibDriverWriteReg32_(pContext,  0x8000d10c, 0x0  );         /*                                 */
        DibDriverWriteReg32_(pContext,  0x8000d110, 0x0  );         /*                                 */
        DibDriverWriteReg32_(pContext,  0x8000d114, 0x32  );        /* P_bist_Ncycle_work              */
        DibDriverWriteReg32_(pContext,  0x8000d118, 0x1  );         /* P_bist_Ncycle_Wait              */
        DibDriverWriteReg32_(pContext,  0x8000d11c, 0x3  );         /* P_bist_speed                    */
        DibDriverWriteReg32_(pContext,  0x8000d120, 0x1  );         /* P_ram65nm_tmcs2pr               */
        DibDriverWriteReg32_(pContext,  0x8000d124, 0x1  );         /* P_ram65nm_tmwl2sa               */
        DibDriverWriteReg32_(pContext,  0x8000d128, 0x1  );         /* P_ram65nm_tmsa2cs               */
        DibDriverWriteReg32_(pContext,  0x8000d12c, 0x1  );         /* P_ram65nm_tmpscs                */
        DibDriverWriteReg32_(pContext,  0x8000d130, 0x0  );         /* P_ram65nm_tmeq2wl               */

        DibDriverWriteReg32_(pContext,  0x8000d040, 0xc405  );  /* Disable AHB_RAM clock     */
        DibDriverWriteReg32_(pContext,  0x8000d044, 0xc405  );  /* Reset AHB_RAM                        */
        DibDriverWriteReg32_(pContext,  0x8000d040, 0xcc05  );  /* Enable AHB_RAM clock                 */
        DibDriverWriteReg32_(pContext,  0x8000d044, 0xc805  );  /* Release AHB_RAM reset and reset PROC */
        DibDriverWriteReg32_(pContext,  0x8000d044, 0xcc05  );  /* Release PROC reset                   */
        DibDriverWriteReg32_(pContext,  0x8000d040, 0xcc45  );  /* Start VCXO Clock                     */
        DibDriverWriteReg32_(pContext,  0x8000d044, 0xcc45  );  /* Release VCXO reset                   */
        break;
   }
}

/****************************************************************************
 * Init the CPU. This can be necessary before initialisation of the mailbox.
 ****************************************************************************/
void IntDriverDragonflyInitCpu(struct DibDriverContext *pContext)
{
#if (DF_PROFILE == 1)
   FILE *fp;
   /* clean the file */
   fp = fopen("ProfileLog.txt","w");
   if(fp) fclose(fp);
#endif
}

DIBSTATUS IntDriverDragonflyUploadMicrocode(struct DibDriverContext *pContext)
{
   uint32_t  FirmwareIdReg   = pContext->DragonflyRegisters.FirmwareIdReg;
   uint8_t * FileBuf   = (uint8_t*)pContext->Hd.BoardConfig->FeCfg[0].UDemod.Dib9000.FirmwareRiscA;
   uint32_t  tmp, i, Addr, Offset, Size; 

#if (FAST_DOWNLOAD == 1)
   struct DibDriverDmaCtx DmaCtx;
   uint8_t * FwBuf = NULL;
#endif

   DibMSleep(50);

   DIB_DEBUG(SOFT_ERR, (CRB "IntDriverDragonflyUploadMicrocode: Start 1" CRA));

   IntDriverWaitIfDebugMode(pContext);

   DIB_DEBUG(SOFT_ERR, (CRB "IntDriverDragonflyUploadMicrocode: Start 2" CRA));
   

   /*** Write Test pattern in memory ***/
   if(DibDriverWriteReg32(pContext, FirmwareIdReg, 0x11223344) != DIBSTATUS_SUCCESS)
      goto End;

   if(DibDriverWriteReg32(pContext, pContext->DragonflyRegisters.DownloadModeReg, 0) != DIBSTATUS_SUCCESS)
       goto End;

   /* leon irq_force => force irq 6 */
   if(DibDriverWriteReg32(pContext, 0x80000098, 1 << 6) != DIBSTATUS_SUCCESS)
     goto End;

   if(DibDriverWriteReg32(pContext, 0x8000b038, 0) != DIBSTATUS_SUCCESS)
     goto End;
   if(DibDriverWriteReg32(pContext, 0x8000b03c, 0xffffffff) != DIBSTATUS_SUCCESS)
     goto End;

   DIB_DEBUG(SOFT_ERR, (CRB "IntDriverDragonflyUploadMicrocode: Start 3" CRA));

   for(i=0;i<pContext->DragonflyRegisters.FirmwareNbRegion;i++)
   {
      Size   = pContext->DragonflyRegisters.FirmwareRegions[i].Size;
      Offset = pContext->DragonflyRegisters.FirmwareRegions[i].Offset;
      Addr   = pContext->DragonflyRegisters.FirmwareRegions[i].Addr;
      if(Size > 0)
      {
         /* ajust the size to allow burst using words */
         Size += (4 - (Size & 3)) & 3;

         //DIB_DEBUG(SOFT_LOG, (CRB "downloading PROGRAM %d: %d dwords to addr %08x" CRA, i, Size, Addr));
#if (FAST_DOWNLOAD == 1)
         /* allocate a buffer to store the firmware starting at a valid 32 bit host address */
         FwBuf = (uint8_t *)DibMemAlloc(Size);
		 DIB_DEBUG(SOFT_ERR, (CRB "downloading 0 PROGRAM %d: %d dwords to addr %08x" CRA, i, Size, Addr));
		 
         DibMoveMemory(FwBuf, FileBuf + Offset, Size);
         DIB_ASSERT(((long)FwBuf & 3) == 0);
         DIB_ASSERT(((long)Size & 3) == 0);

		 DIB_DEBUG(SOFT_ERR, (CRB "downloading 1 PROGRAM %d: %d dwords to addr %08x" CRA, i, Size, Addr));
         /* transfer the buffer */
         DmaCtx.Dir         = DIBDRIVER_DMA_WRITE;
         DmaCtx.ChipAddr    = Addr;
         DmaCtx.ChipBaseMin = 0;
         DmaCtx.ChipBaseMax = 0;
         DmaCtx.DmaLen      = Size;
         DmaCtx.pHostAddr   = FwBuf;
         DibD2BTransfertBuf(pContext, &DmaCtx);
          /* free the buffer */
         DibMemFree(FwBuf, Size);
		 DIB_DEBUG(SOFT_ERR, (CRB "downloading 2 PROGRAM %d: %d dwords to addr %08x" CRA, i, Size, Addr));
#else
         while(Size--) 
         {
            if(DibDriverWriteReg8(pContext, Addr + Size, FileBuf[Offset + Size]) != DIBSTATUS_SUCCESS)
               return DIBSTATUS_ERROR;
         }
#endif

#if (CHECK_FIRMWARE_DOWNLOAD == 1)
         IntDriverDragonflyDownloadCheck(pContext, Addr, FileBuf + Offset, Size);
#endif
      }
   }

   DIB_DEBUG(SOFT_ERR, (CRB "IntDriverDragonflyUploadMicrocode: Start 4" CRA));

   DibMSleep(1);
   IntDriverWaitIfDebugMode(pContext);

   DIB_DEBUG(SOFT_ERR, (CRB "IntDriverDragonflyUploadMicrocode: Start 5" CRA));

   /* Address containing the RAM jump address */
   if(DibDriverWriteReg32(pContext, pContext->DragonflyRegisters.JumpAddressReg, 
                                    pContext->DragonflyRegisters.EntryPoint) != DIBSTATUS_SUCCESS)
     goto End;

   /* leon irq_force => force irq 6 */
   if(DibDriverWriteReg32(pContext, 0x80000098, 1 << 6) != DIBSTATUS_SUCCESS)
     goto End;

   DibMSleep(10);
   IntDriverWaitIfDebugMode(pContext);

   /* URAM address to see whether the firmware was started */
   if(DibDriverReadReg32(pContext, FirmwareIdReg, &tmp) != DIBSTATUS_SUCCESS)
      goto End;

   DIB_DEBUG(SOFT_ERR, (CRB "IntDriverDragonflyUploadMicrocode: Start 6" CRA));

   DIB_DEBUG(SOFT_LOG, (CRB "reading %08x @ 0x%08x" CRA, tmp, FirmwareIdReg));
   if (tmp == 0xdeadbeef)
   {
      DIB_DEBUG(SOFT_LOG, (CRB "Firmware Download successfull" CRA));
      return DIBSTATUS_SUCCESS;
   }
   else
   {
      DIB_DEBUG(SOFT_ERR, (CRB "Firmware Download failed" CRA));
      return DIBSTATUS_ERROR;
   }

End:
   DIB_DEBUG(SOFT_ERR, (CRB "Firmware Download Error" CRA));
   return DIBSTATUS_ERROR;
}

DIBSTATUS IntDriverDragonflyWaitForFirmware(struct DibDriverContext *pContext)
{
   uint32_t  FirmwareIdReg   = pContext->DragonflyRegisters.FirmwareIdReg;
   uint32_t  tmp = 0, i;

   for (i =0 ; (tmp!=0xbeefdead) && i< 100; i++)
   for (i =0 ; (tmp!=0xbeefdead) && i< 1000; i++)
   {

      DibMSleep(10);
      /* URAM address to see whether the firmware was started */
      if(DibDriverReadReg32(pContext, FirmwareIdReg, &tmp) != DIBSTATUS_SUCCESS)
         return DIBSTATUS_ERROR;
   }
   //printk"final firmware copied after %d0ms\n" ,i);

   if (tmp!=0xbeefdead)
      return DIBSTATUS_ERROR;
#if 0
   FILE* fp= fopen("../../../Firmware/firmware_nautilus_2_0.bin","r");
   if (!fp)
   {
      printf ("error opening  file \n");
      return 1;
   }
   fseek(fp,0x3D2B8,0);
   uint8_t bf,ff;
   for (i= 0;i<82196;i++)
   {
      DibDriverReadReg8(pContext, 0x70000000+i, &ff);
      fread(&bf,1,1,fp);
      if (bf != ff)
         printf("%d : firmware %02x file %02x\n", i, ff, bf);
   }
   fclose(fp);
   printf("firmware tested\n");
   fp= fopen("../../../Firmware/firmware_nautilus_2_0.bin","r");
   if (!fp)
   {
      printf ("error opening  file \n");
      return 1;
   }
   fseek(fp,0x88,0);
   for (i= 0;i<250416;i++)
   {
      DibDriverReadReg8(pContext, 0x70100000+i, &ff);
      fread(&bf,1,1,fp);
      if (bf != ff)
         printf("%d : firmware %02x file %02x\n", i, ff, bf);
   }
   fclose(fp);
   printf("subfirmware tested\n");

#endif
   DibDriverWriteReg32(pContext, FirmwareIdReg, 0x01234567);

   for (i =0 ; (tmp!=0xdeadbeef) && i< 100; i++)
   {

      DibMSleep(10);
      /* URAM address to see whether the firmware was started */
      if(DibDriverReadReg32(pContext, FirmwareIdReg, &tmp) != DIBSTATUS_SUCCESS)
         return DIBSTATUS_ERROR;

   }
   //printk("final firmware boot after %d0ms\n" ,i);
   if (tmp!=0xdeadbeef)
      return DIBSTATUS_ERROR;

   IntDriverDragonflyStartCpu(pContext);
   return DIBSTATUS_SUCCESS;
}


/*
 * Check that a region has been correctly downloaded by reading values back.
 */
void IntDriverDragonflyDownloadCheck(struct DibDriverContext *pContext, uint32_t Addr, uint8_t * HostBuf, uint32_t Size)
{
   uint8_t Data;
   while(Size--) 
   {
      if(DibDriverReadReg8(pContext, Addr + Size, &Data) != DIBSTATUS_SUCCESS)
         return;

      if(HostBuf[Size] != Data)
      {
         DIB_DEBUG(SOFT_ERR, (CRB "-E at %08x: %02x => %02x" CRA,
                  Addr + Size,
                  HostBuf[Size],
                  Data));
      }
   }
}

void IntDriverDragonflyStartCpu(struct DibDriverContext *pContext)
{
  /* configure internal clock to 160 MHz */

  /* IntDriverDragonflySetPll19000(pContext, 160000); */
  DibDriverWriteReg32(pContext, pContext->DragonflyRegisters.FirmwareIdReg, 0xcafebabe);

}

/****************************************************************************
 * init and synchro risc with host
 ****************************************************************************/
DIBSTATUS IntDriverDragonflyInitMailbox(struct DibDriverContext *pContext)
{
  if(DibDriverWriteReg32(pContext, REG_HIF_HOST_IRQ_EN, 1) != DIBSTATUS_SUCCESS)
      goto End;

  /* write the address of the address in ram where we change the WR pointer of the mac mailbox */
  if(DibDriverWriteReg32(pContext, REG_HIF_HOST_ADDR, pContext->DragonflyRegisters.MacMbxWrPtrReg) != DIBSTATUS_SUCCESS)
      goto End;

  return DIBSTATUS_SUCCESS;

End:
  return DIBSTATUS_ERROR;
}

/****************************************************************************
 * Tests the RAM (R&W tests)
 ****************************************************************************/
uint32_t IntDriverDragonflyTestRamInterface(struct DibDriverContext *pContext)
{
  return 0;
}

void IntDriverDragonflyRegisterIfInit(struct DibDriverContext *pContext)
{
}


void IntDriverDragonflyEnableClearIrq(struct DibDriverContext *pContext)
{
  uint32_t Mask;

  /* Enable Messaging interrupts only */
  Mask = MASK(FLD_HIF_MBX_NEW);
  DibDriverWriteReg32(pContext, REG_HIF_INT_EN, Mask);

  /* Clear interrupts stats */
  Mask = MASK(FLD_HIF_IRQ_ALL);
  DibDriverWriteReg32(pContext, REG_HIF_INT_CLR, Mask);

#if  (IRQ_POLARITY == IRQ_ACTIVE_LOW)
  DibDriverWriteReg32(pContext, REG_HIF_HOST_IRQ_MODE, (0<<SHIFT(FLD_HIF_IRQ_OD)) | (1<<SHIFT(FLD_HIF_IRQ_LEVEL)));
#else
  DibDriverWriteReg32(pContext, REG_HIF_HOST_IRQ_MODE, (0<<SHIFT(FLD_HIF_IRQ_OD)) | (0<<SHIFT(FLD_HIF_IRQ_LEVEL)));
#endif


  /* Enable Sdio interrupts */
  DibDriverWriteReg32(pContext, REG_HIF_SDIO_IRQ_EN, (1<<SHIFT(FLD_HIF_SDIO_SWAP)) | (0<<SHIFT(FLD_HIF_SDIO_INV)) | (1<<SHIFT(FLD_HIF_SDIO_EN)));

  /* Enable Host to interrupt the MAC */
  DibDriverWriteReg32(pContext, REG_HIF_HOST_IRQ_EN, 1);
}

/**
 * DVBH_driver_init_pll
 */
static void IntDriverDragonflySetDll19000(struct DibDriverContext *pContext, uint32_t target_clock_mhz, uint32_t input_clock_mhz)
{
   /* uint32_t n_P_dll_delay_forced; */
   uint32_t tmp;
   uint32_t n_P_enable_dll     = 0;
   uint32_t n_P_dll_vco_freq   = 0;
   uint32_t n_P_dll_freq_alpha = 7;
   uint32_t n_P_dll_phase_alpha= 7;

   /*
   n_P_dll_delay_forced = 50;
   dib19000_write32(state, 0x8000a104, (n_P_dll_delay_forced << 6));
   */
   n_P_dll_vco_freq = target_clock_mhz*input_clock_mhz;

   DibDriverWriteReg32(pContext, 0x8000a0fc, (n_P_enable_dll << 30) | ((n_P_dll_vco_freq & 0xFFFFFFFF) << 10) | (n_P_dll_freq_alpha<<5) | n_P_dll_phase_alpha);
   DibDriverReadReg32(pContext, 0x8000a110, &tmp);

   while((tmp & 1)==0)
   {
      DIB_DEBUG(SOFT_LOG, (CRB "Lock + n_M_dll_accu_err %08x %08x" CRA, tmp, n_P_dll_vco_freq ));
      DibDriverReadReg32(pContext, 0x8000a110, &tmp);
   }

   DIB_DEBUG(SOFT_LOG, ("DLL_LOCKED"));
   n_P_enable_dll = 1;

   DibDriverWriteReg32(pContext, 0x8000a0fc, (n_P_enable_dll << 30) | ((n_P_dll_vco_freq & 0xFFFFFFFF) << 10) | (n_P_dll_freq_alpha<<5) | n_P_dll_phase_alpha);
   DIB_DEBUG(SOFT_LOG, ("DLL_ENABLED"));
}

static int32_t IntDriverDragonflySetPll19000(struct DibDriverContext *pContext, uint32_t n_pll_freq)
{
   uint32_t n_pllclk_select = 0;
   uint32_t n_pllclk_div    = 0;
   uint32_t n_PllRange     = 1; /* 0 : 5- - 125, 1 : 125 - 250, 2 : 250 - 500, 3 :500 - 1000 */
   uint32_t n_PllPrediv    = 0;
   uint32_t n_pll_loopdiv   = 0;
   uint32_t n_pll_rst       = 1;
   uint32_t n_pll_test      = 0;
   uint32_t PLL_ref_clk     = 120;

   /* input ref clock must be between 10 - 25 Mhz :  10MHz < fbamse /prediv < 25MHZ */
   n_PllPrediv    = 12;
   n_pll_rst       = 1;

   n_pllclk_select = 1;
   n_pll_loopdiv = (uint32_t)((n_pll_freq/1000)*n_PllPrediv)/(PLL_ref_clk);

   if (n_pll_freq < 125000)
      n_PllRange = 0;
   else if (n_pll_freq < 250000)
      n_PllRange = 1;
   else
      n_PllRange = 2;

   DibDriverWriteReg32(pContext, 0x8000a008, ((n_pllclk_select << 22) | (n_pllclk_div << 18) | (n_PllRange << 16) | (n_PllPrediv << 11) |
                                              (n_PllPrediv << 11) | (n_pll_loopdiv << 2) | (n_pll_rst << 1) | n_pll_test ));

   return DIBSTATUS_SUCCESS;
}

/**
 * DibDriverFrontendInit
 */
DIBSTATUS IntDriverDragonflyFrontendInit(struct DibDriverContext *pContext)
{
  /* compiler happy! */
  (void)IntDriverDragonflySetDll19000;
  (void)IntDriverDragonflySetPll19000;

  return DIBSTATUS_SUCCESS;
}

#endif /* USE_DRAGONFLY */
