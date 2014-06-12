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
* @file "DibBridgeDragonflyTest.c"
* @brief Dragonfly specific bridge tests.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibBridgeDragonflyTest.h"

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))

/****************************************************************************
* Checks message coming from the RISC and acts appropriately
****************************************************************************/

uint8_t * TestBuf = 0;

void IntBridgeDragonflyFillPattern(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size) 
{
   uint32_t i;

   for(i = 0; i < Size; i++) 
   {
      pBuf[i] = (i+10)%WRAP;
   }
}

uint32_t IntBridgeDragonflyCheckPattern(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size) 
{
   uint32_t i, Err = 0;

   for(i = 0; i < Size; i++) 
   {
      if(pBuf[i] != (i+10)%WRAP) 
         Err++;
   }
   return Err;
}

#if (PRINT_BUFFER == 1)
void IntBridgeDragonflyPrintBuffer(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size) 
{
   uint32_t i,j;

   for(i = 0; i < Size/4; i++) 
   {
      for(j = 0; j < 4; j++)
      {
         DibBridgeTargetLogFunction("%d%s\t", pBuf[i*4+j], pBuf[i*4+j] == ((i*4+j+10)%WRAP)?"":"#");
      }

      if((i & 3) == 3) 
         DibBridgeTargetLogFunction(CRB "" CRA);
   }
}
#else
#define IntBridgeDragonflyPrintBuffer(pContext,pBuf,Size)
#endif

#if 0
static int32_t IntBridgeNautilus1TestBist(struct DibBridgeContext *pContext)
{
   uint32_t word, RamNumber;

   /* activate URAM first */
   DibBridgeReadReg32(pContext, 0x8000D040, &word);
   DibBridgeWriteReg32(pContext, 0x8000D040, word | 1);
   DibBridgeWriteReg32(pContext, 0x8000D044, word | 1);

   /* BIST test */
   for (RamNumber = 0; RamNumber <= 19; RamNumber++) {
       DibBridgeWriteReg32(pContext, 0x8000D040, 0x7FFFF);
       DibBridgeWriteReg32(pContext, 0x8000D044, 0x7FFFF);
       DibBridgeWriteReg32(pContext, 0x80012220, 0xF);
       DibBridgeWriteReg32(pContext, 0x8000D124, 0x1);
       DibBridgeWriteReg32(pContext, 0x8000D128, 0x1);
       DibBridgeWriteReg32(pContext, 0x8000D12C, 0x1);
       DibBridgeWriteReg32(pContext, 0x8000D120, 0x1);
       DibBridgeWriteReg32(pContext, 0x8000D130, 0x0);
       DibBridgeWriteReg32(pContext, 0x80100400, 0xFFF);
       DibBridgeWriteReg32(pContext, 0x80100402, 0x0);
       DibBridgeWriteReg32(pContext, 0x80100500, 0xFFF);
       DibBridgeWriteReg32(pContext, 0x80100502, 0x0);

       DibBridgeWriteReg32(pContext, 0x8000A044, 0x1);
       DibBridgeWriteReg32(pContext, 0x8000A000, 0x101);
       DibBridgeWriteReg32(pContext, 0x8000A000, 0x0);

       DibBridgeWriteReg32(pContext, 0x8000A010, (RamNumber << 9) | 0x10);
       DibBridgeWriteReg32(pContext, 0x8000A00C, 0x806);

       do {
           DibMSleep(10);
           DibBridgeReadReg32(pContext, 0x8000A008, &word);
       } while ((word & 0x1) != 1);
       DIB_DEBUG(PORT_LOG, (CRB "BIST: RAM number=%i result=%i" CRA, RamNumber, word));
   }

   return DIBSTATUS_SUCCESS;
}
#endif

/* read the memory using safe read32 */
void IntBridgeDragonflySafeReadMem(struct DibBridgeContext *pContext, uint32_t XferAdd, uint8_t *pBuf, uint32_t Size)
{
   uint32_t i;

   for(i = 0; i < Size; i++)
   {
      /* read big endian word */
      DibBridgeReadReg8(pContext, XferAdd+i, pBuf+i);
   }
}

/* write to the memory using safe write32 */
void IntBridgeDragonflySafeWriteMem(struct DibBridgeContext *pContext, uint32_t XferAdd, uint8_t *pBuf, uint32_t Size)
{
   uint32_t i;

   for(i = 0; i < Size; i++)
   {
      DibBridgeWriteReg8(pContext, XferAdd+i, pBuf[i]);
   }
}

/* write a pattern to the memory */
uint32_t IntBridgeDragonflyTestDmaWrite(struct DibBridgeContext *pContext, uint32_t XferAdd, uint32_t XferSize)
{
   struct DibBridgeDmaCtx DmaCtx;
   struct DibBridgeDmaFlags flags;
   uint32_t Err;

   memset(&flags,0,sizeof(struct DibBridgeDmaFlags));

   /* fill TestBuf with a pattern 10, 11, 12, 13,...,255, 0, 1, */
   IntBridgeDragonflyFillPattern(pContext, TestBuf, XferSize);

   DmaCtx.Dir         = DIBBRIDGE_DMA_WRITE;
   DmaCtx.ChipAddr    = XferAdd;
   DmaCtx.ChipBaseMin = 0;
   DmaCtx.ChipBaseMax = 0;
   DmaCtx.DmaLen      = XferSize;
   DmaCtx.pHostAddr   = TestBuf;
   DmaCtx.DmaFlags    = flags;

   /* start a (virtual) dma. */
   DibBridgeSetupDma(pContext, &DmaCtx);
   DibBridgeRequestDma(pContext, &DmaCtx);

   /* dump on chip memory */
   IntBridgeDragonflySafeReadMem(pContext, XferAdd, TestBuf, XferSize);

   Err = IntBridgeDragonflyCheckPattern(pContext, TestBuf, XferSize);
   if(Err) IntBridgeDragonflyPrintBuffer(pContext, TestBuf, XferSize);
   return Err;
}

/* read the memory, find also the pattern */
uint32_t IntBridgeDragonflyTestDmaRead(struct DibBridgeContext *pContext, uint32_t XferAdd, uint32_t XferSize)
{
   struct DibBridgeDmaCtx DmaCtx;
   struct DibBridgeDmaFlags flags;
   uint32_t Err;

   memset(&flags,0,sizeof(struct DibBridgeDmaFlags));

   /* fill TestBuf with a pattern 10, 11, 12, 13,...,255, 0, 1, */
   IntBridgeDragonflyFillPattern(pContext, TestBuf, XferSize);

   /* safely write to memory */
   IntBridgeDragonflySafeWriteMem(pContext, XferAdd, TestBuf, XferSize);

   /* clean the Buffer */
   memset(TestBuf, 0, XferSize);

   DmaCtx.Dir         = DIBBRIDGE_DMA_READ;
   DmaCtx.ChipAddr    = XferAdd;
   DmaCtx.ChipBaseMin = 0;
   DmaCtx.ChipBaseMax = 0;
   DmaCtx.DmaLen      = XferSize;
   DmaCtx.pHostAddr   = TestBuf;
   DmaCtx.DmaFlags    = flags;

   /* start a (virtual) dma transfert from on chip memory to host memory */
   DibBridgeSetupDma(pContext, &DmaCtx);
   DibBridgeRequestDma(pContext, &DmaCtx);

   Err = IntBridgeDragonflyCheckPattern(pContext, TestBuf, XferSize);
   if(Err) IntBridgeDragonflyPrintBuffer(pContext, TestBuf, XferSize);
   return Err;
}

/* write a pattern to the memory */
uint32_t IntBridgeDragonflyTestSafeDma(struct DibBridgeContext *pContext, uint32_t XferAdd, uint32_t XferSize)
{
   uint32_t Err;

   DibBridgeTargetLogFunction(CRB "" CRA);
   DibBridgeTargetLogFunction(CRB "START: IntBridgeDragonflyTestSafeDma" CRA);

   /* fill TestBuf with a pattern 10, 11, 12, 13,...,255, 0, 1, */
   IntBridgeDragonflyFillPattern(pContext, TestBuf, XferSize);

   IntBridgeDragonflySafeWriteMem(pContext, XferAdd, TestBuf, XferSize);

   /* clean the Buffer */
   memset(TestBuf,0,XferSize);

   /* dump on chip memory */
   IntBridgeDragonflySafeReadMem(pContext, XferAdd, TestBuf, XferSize);

   Err = IntBridgeDragonflyCheckPattern(pContext, TestBuf, XferSize);
   if(Err) IntBridgeDragonflyPrintBuffer(pContext, TestBuf, XferSize);

   DibBridgeTargetLogFunction(CRB "results: %d errors over %d bytes copied" CRA,Err, XferSize);
   DibBridgeTargetLogFunction(CRB "FINISHED: IntBridgeDragonflyTestSafeDma" CRA);
   return Err;
}

/* start dma test */
void IntBridgeDragonflyTest32bits(struct DibBridgeContext *pContext, uint32_t StartAdd, uint32_t EndAdd)
{
   uint32_t XferAdd;
   uint32_t temp, Value, toggle=0;
   uint32_t Err=0;

   DibBridgeTargetLogFunction(CRB "" CRA);
   DibBridgeTargetLogFunction(CRB "START: IntBridgeDragonflyTest32bits:!" CRA);

   for(XferAdd = StartAdd; XferAdd < EndAdd; XferAdd += 4)
   {
      /* for each 32 bit address, we test every bit of Data */
      if(toggle)
      {
         temp  = 0xdeadbeef;
         toggle= 0;
      } 
      else 
      {
         temp  = 0x12345678;
         toggle= 1;
      }

      /* first test: target_dma_write */
      DibBridgeWriteReg32(pContext, XferAdd, temp);

      /* second test: target_dma_read */
      DibBridgeReadReg32(pContext, XferAdd, &Value);

      if(XferAdd == 0x70120000 || 
         XferAdd == 0x70140000 || 
         XferAdd == 0x70160000 || 
         XferAdd == 0x70180000 || 
         XferAdd == 0x701a0000 || 
         XferAdd == 0x701c0000 || 
         XferAdd == 0x701e0000)
         DibBridgeTargetLogFunction(CRB "reached %08x" CRA,XferAdd);

      if(temp != Value)
      {
         Err++;
         DibBridgeTargetLogFunction(CRB "error 32bits: Addr=%08x wdata=%08x rdata=%08x" CRA,XferAdd, temp, Value);
      }
   }
   DibBridgeTargetLogFunction(CRB "FINISHED: IntBridgeDragonflyTest32bits:!" CRA);
}

/* start dma test */
void IntBridgeDragonflyTest16bits(struct DibBridgeContext *pContext, uint32_t StartAdd, uint32_t EndAdd)
{
   uint32_t XferAdd;
   uint16_t temp, Value, toggle = 0;
   uint32_t Err=0;

   DibBridgeTargetLogFunction(CRB "" CRA);
   DibBridgeTargetLogFunction(CRB "START: IntBridgeDragonflyTest16bits!" CRA);
   
   for(XferAdd = StartAdd; XferAdd < EndAdd; XferAdd += 2)
   {
      /* for each 32 bit address, we test every bit of Data */
      if(toggle)
      {
         temp  = 0xdead;
         toggle= 0;
      } 
      else 
      {
         temp  = 0xbeef;
         toggle= 1;
      }

      /* first test: target_dma_write */
      DibBridgeWriteReg16(pContext, XferAdd, temp);

      /* second test: target_dma_read */
      DibBridgeReadReg16(pContext, XferAdd, &Value);

      if(XferAdd == 0x70120000 || 
         XferAdd == 0x70140000 || 
         XferAdd == 0x70160000 || 
         XferAdd == 0x70180000 || 
         XferAdd == 0x701a0000 || 
         XferAdd == 0x701c0000 || 
         XferAdd == 0x701e0000)
         DibBridgeTargetLogFunction(CRB "reached %08x" CRA,XferAdd);

      if(temp != Value)
      {
         Err++;
         DibBridgeTargetLogFunction(CRB "error 16bits: Addr=%08x wdata=%08x rdata=%08x" CRA,XferAdd, temp, Value);
      }
   }
   DibBridgeTargetLogFunction(CRB "FINISHED: IntBridgeDragonflyTest16bits:!" CRA);
}

/* start dma test */
void IntBridgeDragonflyTest8bits(struct DibBridgeContext *pContext, uint32_t StartAdd, uint32_t EndAdd)
{
   uint32_t XferAdd;
   uint8_t temp, Value, toggle = 0;
   uint32_t Err=0;

   DibBridgeTargetLogFunction(CRB "" CRA);
   DibBridgeTargetLogFunction(CRB "START: IntBridgeDragonflyTest8bits:!" CRA);
   
   for(XferAdd = StartAdd; XferAdd < EndAdd; XferAdd += 1)
   {
      /* for each 32 bit address, we test every bit of Data */
      if(toggle)
      {
         temp  = 0xb6;
         toggle= 0;
      } 
      else 
      {
         temp  = 0x88;
         toggle= 1;
      }

      /* first test: target_dma_write */
      DibBridgeWriteReg8(pContext, XferAdd, temp);

      /* second test: target_dma_read */
      DibBridgeReadReg8(pContext, XferAdd, &Value);

      if(XferAdd == 0x70120000 || 
         XferAdd == 0x70140000 || 
         XferAdd == 0x70160000 || 
         XferAdd == 0x70180000 || 
         XferAdd == 0x701a0000 || 
         XferAdd == 0x701c0000 || 
         XferAdd == 0x701e0000)
         DibBridgeTargetLogFunction(CRB "reached %08x" CRA,XferAdd);

      if(temp != Value)
      {
         Err++;
         DibBridgeTargetLogFunction(CRB "error 8bits: Addr=%08x wdata=%08x rdata=%08x" CRA, XferAdd, temp, Value);
      }
   }
   DibBridgeTargetLogFunction(CRB "FINISHED: IntBridgeDragonflyTest8bits:!" CRA);
}

/* start dma test */
/* TODO add return status */
uint32_t IntBridgeDragonflyTestDma(struct DibBridgeContext *pContext, uint8_t Mode, uint32_t SizeMin, uint32_t SizeMax, uint32_t AddrMin, uint32_t AddrMax, int32_t SizeStep)
{
   uint32_t XferSize = SizeMin;
   uint32_t XferAdd;
   uint32_t TotalError;
   uint32_t Error;
   uint32_t Err = 0;

   if((SizeStep < 0) && (SizeMin == 0))
      XferSize = 1;

   DibBridgeTargetLogFunction(CRB "" CRA);
   DibBridgeTargetLogFunction(CRB "START IntBridgeDragonflyTestDma %s" CRA, (Mode==TEST_MAC2HOST)?"M2H":"H2M");
   DibBridgeTargetLogFunction(CRB "=>address from %08x to %08x" CRA, AddrMin, AddrMax);
   DibBridgeTargetLogFunction(CRB "=>Size    from %8d to %8d" CRA, SizeMin, SizeMax);
   DibBridgeTargetLogFunction(CRB "" CRA);

   TotalError = 0;

   while(XferSize <= SizeMax)
   {
      XferAdd=AddrMin;
      if(XferSize > 0)
      {
         Error=0;
         while(XferAdd <= AddrMax)
         {
            if(Mode == TEST_HOST2MAC) 
            {
               /* first test: target_dma_write */
               Err=IntBridgeDragonflyTestDmaWrite(pContext, XferAdd, XferSize);
            }
            else if(Mode == TEST_MAC2HOST) 
            {
               /* second test: target_dma_read */
               Err=IntBridgeDragonflyTestDmaRead(pContext, XferAdd, XferSize);
            }

            if(Err>0)
               DibBridgeTargetLogFunction(CRB "transfert error: Addr=%08x Size=%6d Err=%6d" CRA,XferAdd, XferSize, Err);

            Error   += Err;
            XferAdd += XferSize;
         }
         TotalError += Error;
         DibBridgeTargetLogFunction(CRB "test dma %s: Size=%6d Error=%6d" CRA, (Mode==TEST_MAC2HOST)?"M2H":"H2M", XferSize, Error);
      }
      if(SizeStep == 0)
         break;
      else if(SizeStep < 0)
         XferSize *= -SizeStep;
      else
         XferSize   += SizeStep;
   } 
   DibBridgeTargetLogFunction(CRB "" CRA);
   DibBridgeTargetLogFunction("=>IntBridgeDragonflyTestDma %s: ",(Mode==TEST_MAC2HOST)?"M2H":"H2M");
   DibBridgeTargetLogFunction("%s (%d errors)",TotalError?"FAILED":"PASSED", TotalError);
   DibBridgeTargetLogFunction(CRB CRB "" CRA CRA);
   return TotalError;
}

/*----------------------------------------------------------------------------
JEDEC tests
-----------------------------------------------------------------------------*/
int32_t IntBridgeDragonflyTestBasicRead(struct DibBridgeContext *pContext)
{
   int32_t  status = 0;

   uint32_t data32;
   uint16_t data16;
   uint8_t  data8;

   /* 32 bit read access */
   DibBridgeReadReg32(pContext, pContext->DragonflyRegisters.JedecAddr, &data32);
   DibBridgeReadReg32(pContext, pContext->DragonflyRegisters.JedecAddr, &data32); // by infospace, dckim
   DIB_DEBUG(PORT_LOG,(CRB "=>Read JEDEC[0..31]=%08x...%s" CRA,data32,(data32 == pContext->DragonflyRegisters.JedecValue)? "PASSED":"FAILED"));
   if(data32 != pContext->DragonflyRegisters.JedecValue)
   {
      status = -1;
   }

   /** With Nautilus, 32 bits register are only read/written in 32 bits mode, so read in 16 bit mode does not work. **/
   if( pContext->DibChip != DIB_NAUTILUS )
   {
     /* 16 bit read access */
     DibBridgeReadReg16(pContext, pContext->DragonflyRegisters.JedecAddr, &data16);
     DIB_DEBUG(PORT_LOG,(CRB "=>Read JEDEC[0..15]=%04x...%s" CRA, data16,(data16 == ((pContext->DragonflyRegisters.JedecValue >> 16) & 0xFFFF))?"PASSED":"FAILED"));
     if(data16 != ((pContext->DragonflyRegisters.JedecValue >> 16) & 0xFFFF))
     {
        status = -1;
     }
   }
  

   DibBridgeReadReg16(pContext, pContext->DragonflyRegisters.JedecAddr + 2, &data16);
   DIB_DEBUG(PORT_LOG,(CRB "=>Read JEDEC[16..31]=%04x...%s" CRA,data16,(data16 == (pContext->DragonflyRegisters.JedecValue & 0xFFFF))?"PASSED":"FAILED"));
   if(data16 != (pContext->DragonflyRegisters.JedecValue & 0xFFFF))
   {
      status = -1;
   }
  
   /* 8 bit access */
   DibBridgeReadReg8(pContext, pContext->DragonflyRegisters.JedecAddr, &data8);
   DIB_DEBUG(PORT_LOG,(CRB "=>Read JEDEC[0..7]=%02x...%s" CRA,data8,(data8 == ((pContext->DragonflyRegisters.JedecValue >> 24) & 0xFF))?"PASSED":"FAILED"));
   if(data8 != ((pContext->DragonflyRegisters.JedecValue >> 24) & 0xFF))
   {
      status = -1;
   }
  
   DibBridgeReadReg8(pContext, pContext->DragonflyRegisters.JedecAddr + 1, &data8);
   DIB_DEBUG(PORT_LOG,(CRB "=>Read JEDEC[8..15]=%02x...%s" CRA,data8,(data8 == ((pContext->DragonflyRegisters.JedecValue >> 16) & 0xFF))?"PASSED":"FAILED"));
   if(data8 != ((pContext->DragonflyRegisters.JedecValue >> 16) & 0xFF))
   {
      status = -1;
   }
  
   DibBridgeReadReg8(pContext, pContext->DragonflyRegisters.JedecAddr + 2, &data8);
   DIB_DEBUG(PORT_LOG,(CRB "=>Read JEDEC[16..23]=%02x...%s" CRA,data8,(data8 == ((pContext->DragonflyRegisters.JedecValue >> 8) & 0xFF))?"PASSED":"FAILED"));
   if(data8 != ((pContext->DragonflyRegisters.JedecValue >> 8) & 0xFF))
   {
      status = -1;
   }
  
   DibBridgeReadReg8(pContext, pContext->DragonflyRegisters.JedecAddr + 3, &data8);
   DIB_DEBUG(PORT_LOG,(CRB "=>Read JEDEC[24..31]=%02x...%s" CRA,data8,(data8 == (pContext->DragonflyRegisters.JedecValue & 0xFF))?"PASSED":"FAILED"));
   if(data8 != (pContext->DragonflyRegisters.JedecValue & 0xFF))
   {
      status = -1;
   }

   return (status);
}

/*----------------------------------------------------------------------------
Register Write / Read Test 
-----------------------------------------------------------------------------*/
struct DemodTest
{
   uint32_t Address;
   uint32_t Value;
   uint32_t Write;
   uint32_t Check;
};

static int32_t IntBridgeDragonflyTestDemodAccess(struct DibBridgeContext *pContext, struct DemodTest TableAddress[])
{
   uint16_t Value16;
   uint32_t Address, Value32, Error = 0, i=0;

   DIB_DEBUG(PORT_LOG,(CRB "+Test_Demod_Access" CRA));
   while(TableAddress[i].Address != 0)
   {
      Address=TableAddress[i].Address;
      if(TableAddress[i].Write == 0)
      {
         if(IS_DEMOD_ADDRESS(Address))
         {
            DibBridgeReadReg16(pContext, Address, &Value16);
            DIB_DEBUG(PORT_LOG,(CRB "(*%08x)=>%04x",Address,Value16));
            Value32 = (uint32_t)Value16;
         }
         else
         {
            DibBridgeReadReg32(pContext, Address, &Value32);
            DIB_DEBUG(PORT_LOG,(CRB "(*%08x)=>%08x ",Address,Value32));
         }
         if(Value32 != TableAddress[i].Check)
         {
            Error ++;
            DIB_DEBUG(PORT_LOG,(CRB "...FAILED" CRA));
         }
         else
         {
            DIB_DEBUG(PORT_LOG,(CRB "...PASSED" CRA));
         }
      }
      else
      {
         if(IS_DEMOD_ADDRESS(Address))
         {
            Value16=TableAddress[i].Value;
            DibBridgeWriteReg16(pContext, Address, Value16);
            DIB_DEBUG(PORT_LOG,(CRB "(*%08x)<=%04x" CRA,Address,Value16));
         }
         else
         {
            Value32=TableAddress[i].Value;
            DibBridgeWriteReg32(pContext, Address, Value32);
            DIB_DEBUG(PORT_LOG,(CRB "(*%08x)<=%08x" CRA,Address,Value32));
         }
      }
      i++;
   }
   DIB_DEBUG(PORT_LOG,(CRB "-Test_Demod_Access" CRA));
   return Error;
}

int32_t IntBridgeVoyager1TestRegister(struct DibBridgeContext *pContext)
{
   struct DemodTest TableAddress[] = {

      { 0x8000d040,   0xfcf,      1,            0 }, 
      { 0x8000d044,   0xc00,      1,            0 }, 
      { 0x8000d044,   0xfc1,      1,            0 }, 
      { 0x8000A0AC,   3,          1,            0 }, 
      { 0x8000fc00,   0,          1,            0 }, 
      { 0x8000fc02,   0,          1,            0 }, 
      { 0x8000fc04,   0,          1,            0 }, 
      { 0x8000fc06,   0,          1,            0 }, 
      { 0x8000fc08,   0,          1,            0 }, 
      { 0x8000E000,   0,          0,            3 },
      { 0x8000A0AC,   0,          0,            3 },
      { 0x8000A08C,   0,          0,   0x01b33801 },
      { 0x8000E002,   0xFFFF,     1,            0 },
      { 0x8000E002,   0,          0,       0x1FFF },
      { 0x7000FCE8,   0x12345678, 1,            0 },
      { 0x7000FCE8,   0,          0,   0x12345678 },
      { 0x8000E000,   0,          0,            3 },
      { 0x8000A08C,   0,          0,   0x01b33801 },
      { 0,            0,          0,            0 }
   };

   return IntBridgeDragonflyTestDemodAccess(pContext, TableAddress);
}

int32_t IntBridgeNautilus1TestRegister(struct DibBridgeContext *pContext)
{
   struct DemodTest TableAddress[] = {
      { 0x8000a020,   0,          0,   0x01b31068 },
      { 0x70000000,   0x900DBEEF, 1,            0 },
      { 0x70000000,   0,          0,   0x900DBEEF },
      { 0x70000004,   0xB0B0CACA, 1,            0 },
      { 0x70000004,   0,          0,   0xB0B0CACA },
      { 0x70000008,   0xCAFEBABE, 1,            0 },
      { 0x70000008,   0,          0,   0xCAFEBABE },
      { 0x7000000C,   0xDEADBEEF, 1,            0 },
      { 0x7000000C,   0,          0,   0xDEADBEEF },
      { 0x70000010,   0x55555555, 1,            0 },
      { 0x70000010,   0,          0,   0x55555555 },
      { 0x70000014,   0xBBBBBBBB, 1,            0 },
      { 0x70000014,   0,          0,   0xBBBBBBBB },
      { 0,            0,          0,            0 }
   };
   
   TableAddress[0].Check = pContext->DragonflyRegisters.JedecValue;

   return IntBridgeDragonflyTestDemodAccess(pContext, TableAddress);
}

int32_t IntBridgeNautilus1MPTestRegister(struct DibBridgeContext *pContext)
{
   struct DemodTest TableAddress[] = {
      { 0x8000a020,   0,          0,   0x01b31069 },
      { 0x70000000,   0x900DBEEF, 1,            0 },
      { 0x70000000,   0,          0,   0x900DBEEF },
      { 0x70000004,   0xB0B0CACA, 1,            0 },
      { 0x70000004,   0,          0,   0xB0B0CACA },
      { 0x70000008,   0xCAFEBABE, 1,            0 },
      { 0x70000008,   0,          0,   0xCAFEBABE },
      { 0x7000000C,   0xDEADBEEF, 1,            0 },
      { 0x7000000C,   0,          0,   0xDEADBEEF },
      { 0x70000010,   0x55555555, 1,            0 },
      { 0x70000010,   0,          0,   0x55555555 },
      { 0x70000014,   0xBBBBBBBB, 1,            0 },
      { 0x70000014,   0,          0,   0xBBBBBBBB },
      { 0,            0,          0,            0 }
   };

   return IntBridgeDragonflyTestDemodAccess(pContext, TableAddress);
}

/*----------------------------------------------------------------------------
SRAM Test
-----------------------------------------------------------------------------*/
int32_t IntBridgeDragonflyTestInternalRam(struct DibBridgeContext *pContext)
{
   uint32_t status = 0;

   TestBuf = (uint8_t *)DibMemAlloc(SYSRAM_SIZE);

   /*status += IntBridgeDragonflyTestSafeDma(pContext, SYSRAM_START, SYSRAM_SIZE);*/
   status += IntBridgeDragonflyTestDma(pContext, TEST_MAC2HOST, 1, SYSRAM_SIZE, SYSRAM_START, SYSRAM_START, SYSRAM_STEP);
   status += IntBridgeDragonflyTestDma(pContext, TEST_HOST2MAC, 1, SYSRAM_SIZE, SYSRAM_START, SYSRAM_START, SYSRAM_STEP);

   DibMemFree(TestBuf, SYSRAM_SIZE);
   TestBuf = 0;
   return status;
}

/*----------------------------------------------------------------------------
URAM Test 
-----------------------------------------------------------------------------*/

int32_t IntBridgeDragonflyTestExternalRam(struct DibBridgeContext *pContext)
{
   uint32_t status = 0;
   uint32_t Bank;

   TestBuf = (uint8_t *)DibMemAlloc(URAM_BANK_SIZE);

   /*status += IntBridgeDragonflyTestSafeDma(pContext, URAM_START, TRANSFERT_SIZE_MAX);*/
   for(Bank = 0; Bank < URAM_NB_BANK; Bank++)
   {
      status += IntBridgeDragonflyTestDma(pContext, TEST_MAC2HOST, 1, URAM_SIZE, URAM_BANK(Bank), URAM_BANK(Bank), URAM_STEP);
      status += IntBridgeDragonflyTestDma(pContext, TEST_HOST2MAC, 1, URAM_SIZE, URAM_BANK(Bank), URAM_BANK(Bank), URAM_STEP);
   }
   DibMemFree(TestBuf, URAM_BANK_SIZE);
   TestBuf = 0;
   return status;
}

int32_t IntBridgeVoyager1TestExternalRam(struct DibBridgeContext *pContext)
{
   uint32_t word;

   /* activate URAM first */
   DibBridgeReadReg32(pContext, 0x8000D040, &word);
   DibBridgeWriteReg32(pContext, 0x8000D040, word | 1);
   DibBridgeWriteReg32(pContext, 0x8000D044, word | 1);

   return IntBridgeDragonflyTestExternalRam(pContext);
}

int32_t IntBridgeNautilus1TestExternalRam(struct DibBridgeContext *pContext)
{
   uint32_t word;

   /* activate URAM first */
   DibBridgeReadReg32(pContext, 0x8000D040, &word);
   DibBridgeWriteReg32(pContext, 0x8000D040, word | 1);
   DibBridgeWriteReg32(pContext, 0x8000D044, word | 1);

   return IntBridgeDragonflyTestExternalRam(pContext);
}

/******************************************************************************
 * return RAM address
 ******************************************************************************/
uint32_t DibBridgeDragonflyGetRamAddr(struct DibBridgeContext *pContext)
{
   return SYSRAM_START;
}

#endif

#endif /* USE_DRAGONFLY */
