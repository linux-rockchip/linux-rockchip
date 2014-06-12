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
* @file "DibBridgeTestIf.h"
* @brief Test Bridge Interface.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTestIf.h"
#include "DibBridge.h"

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))

uint8_t *rambuf;
uint8_t *ramtest;

static int32_t IntBridgeTestFastRam(struct DibBridgeContext *pContext);
static int32_t IntBridgePerformanceTest(struct DibBridgeContext *pContext);


/*----------------------------------------------------------------------------
TEST Entry Point 
-----------------------------------------------------------------------------*/
int32_t DibBridgeTestIf(struct DibBridgeContext *pContext, uint8_t Mode)
{ 
   int32_t rc1 = 0;
   int32_t rc2 = 0;
   int32_t rc3 = 0;
   int32_t rc4 = 0;
   int32_t rc5 = 0;

   rambuf  = (uint8_t *)DibMemAlloc(RAM_TEST_SIZE);
   ramtest = (uint8_t *)DibMemAlloc(RAM_TEST_SIZE);

   DIB_DEBUG(PORT_LOG,(CRB "-----------------------------" CRA));
   DIB_DEBUG(PORT_LOG,(CRB "--- CHIP INTERFACE TEST -----" CRA));
   DIB_DEBUG(PORT_LOG,(CRB "-----------------------------" CRA));

   /* TODO: Basic test should be adapted for dragonfly based chipset causes JEDEC is not the same */
   if(Mode & BASIC_READ_TEST)
   {
      DIB_DEBUG(PORT_LOG,(CRB "----- Simple Read Test ------" CRA));
      rc1  = IntBridgeTestBasicRead(pContext);
   }

   if(Mode & REGISTER_TEST)
   {
      DIB_DEBUG(PORT_LOG,(CRB "----- Register Read/Write --" CRA));
      rc2  = IntBridgeTestRegister(pContext);
   }

   if(Mode & INT_RAM_TEST)
   {
      DIB_DEBUG(PORT_LOG,(CRB "----- Internal RAM Test ----" CRA));
      rc3  = IntBridgeTestInternalRam(pContext);
   }

   if(Mode & EXT_RAM_TEST)
   {
      DIB_DEBUG(PORT_LOG,(CRB "----- External RAM Test ----" CRA));
      DibMSleep(1000);
      rc4  = IntBridgeTestExternalRam(pContext);
   }

   if(Mode & FAST_RAM_TEST)
   {
      DIB_DEBUG(PORT_LOG,(CRB "----- Fast RAM Test --------" CRA));
      rc5  = IntBridgeTestFastRam(pContext);
   }

   if(Mode & PERF_TEST)
   {
      DIB_DEBUG(PORT_LOG,(CRB "----- Performance Test-----" CRA));
      IntBridgePerformanceTest(pContext);
   }

   DIB_DEBUG(PORT_LOG,(CRB "---------------------------------------" CRA));
   DIB_DEBUG(PORT_LOG,(CRB "------------- TEST RESULT -------------" CRA));
   DIB_DEBUG(PORT_LOG,(CRB "---------------------------------------" CRA));

   if(Mode & BASIC_READ_TEST)
   {
      if(rc1 == 0)
      {
         DIB_DEBUG(PORT_LOG,(CRB "Basic Read Test: OK" CRA));
      }
      else
      {
         DIB_DEBUG(PORT_LOG,(CRB "Basic Read Test: Failed" CRA));
      }
   }

   if(Mode & REGISTER_TEST)
   {
      if(rc2 == 0)
      {
         DIB_DEBUG(PORT_LOG,(CRB "Register Test: OK" CRA));
      }
      else
      {
         DIB_DEBUG(PORT_LOG,(CRB "Register Test: Failed" CRA));
      }
   }

   if(Mode & INT_RAM_TEST)
   {
      if(rc3 == 0)
      {
         DIB_DEBUG(PORT_LOG,(CRB "Internal RAM Test:  OK" CRA));
      }
      else
      {
         DIB_DEBUG(PORT_LOG,(CRB "Internal RAM Test:  Failed " CRA));
      }
   }

   if(Mode & EXT_RAM_TEST)
   {
      if(rc4 == 0)
      {
         DIB_DEBUG(PORT_LOG,(CRB "External RAM Test:  OK" CRA));
      }
      else
      {
         DIB_DEBUG(PORT_LOG,(CRB "External RAM Test:  Failed" CRA));
      }
   }

   if(Mode & FAST_RAM_TEST)
   {
      if(rc5 == 0)
      {
         DIB_DEBUG(PORT_LOG,(CRB "Fast RAM Test:  OK" CRA));
      }
      else
      {
         DIB_DEBUG(PORT_LOG,(CRB "Fast RAM Test:  Failed" CRA));
      }
   }

   DIB_DEBUG(PORT_LOG,(CRB "---------------------------------------" CRA));
   DIB_DEBUG(PORT_LOG,(CRB "---------------------------------------" CRA));
   DibMemFree(rambuf,RAM_TEST_SIZE);
   DibMemFree(ramtest,RAM_TEST_SIZE);

   return (rc1+rc2+rc3+rc4+rc5);
}

/*----------------------------------------------------------------------------
Write to Internal or External Memory 
-----------------------------------------------------------------------------*/
void IntBridgeTestIfWriteFastMemory(struct DibBridgeContext *pContext, uint8_t *Buffer, uint32_t Size, uint32_t Offset)
{ 
   struct DibBridgeDmaCtx DmaCtx;

   memset(&DmaCtx.DmaFlags,0,sizeof(struct DibBridgeDmaFlags));
   DmaCtx.ChipBaseMin = 0+Offset;
   DmaCtx.ChipBaseMax = 0x10000+Offset;
   DmaCtx.ChipAddr = 0+Offset;
   DmaCtx.DmaLen = Size;
   DmaCtx.Dir = DIBBRIDGE_DMA_WRITE;
   DmaCtx.pHostAddr = Buffer;

   DibBridgeSetupDma(pContext, &DmaCtx);

   DibBridgeRequestDma(pContext, &DmaCtx);
}

/*----------------------------------------------------------------------------
Read to Internal or External Memory 
-----------------------------------------------------------------------------*/
void IntBridgeTestIfReadFastMemory(struct DibBridgeContext *pContext,uint8_t *Buffer,uint32_t Size, uint32_t Offset)
{ 
   struct DibBridgeDmaCtx DmaCtx;

   memset(&DmaCtx.DmaFlags,0,sizeof(struct DibBridgeDmaFlags));
   DmaCtx.ChipBaseMin = 0+Offset;
   DmaCtx.ChipBaseMax = 0x10000+Offset;
   DmaCtx.ChipAddr = 0+Offset;
   DmaCtx.DmaLen = Size;
   DmaCtx.Dir = DIBBRIDGE_DMA_READ;
   DmaCtx.pHostAddr = Buffer;

   DibBridgeSetupDma(pContext, &DmaCtx);

   DibBridgeRequestDma(pContext, &DmaCtx);
}

/*----------------------------------------------------------------------------
Fast Memory Test 
-----------------------------------------------------------------------------*/
static int32_t IntBridgeTestFastRam(struct DibBridgeContext *pContext)
{ 
   uint32_t i = 0;
   uint32_t OffIntRam = DibBridgeGetRamAddr(pContext);

#if (RAM_TEST_LOOP >= 0xFFFFFFFF)
#error "RAM_TEST_LOOP is too big !"
#endif

   for(i = 0; i < RAM_TEST_LOOP; i++)
   {
      IntBridgeInitRamBuf((uint8_t)(i%256), RAM_TEST_SIZE);

      IntBridgeTestIfWriteFastMemory(pContext, rambuf, RAM_TEST_SIZE, OffIntRam);
      IntBridgeTestIfReadFastMemory(pContext, ramtest, RAM_TEST_SIZE, OffIntRam);

      if(0 != memcmp(rambuf, ramtest, RAM_TEST_SIZE))
      {
         DIB_DEBUG(PORT_LOG,(CRB "###### Fast Ram Test Error ######" CRA));
         return -1;
      } 

      DIB_DEBUG(PORT_LOG,(CRB "Fast Ram Test %d (%d bytes) OK" CRA,i, RAM_TEST_SIZE));
   }

   return (0);
}

/*----------------------------------------------------------------------------
Read / Write Performance test 
-----------------------------------------------------------------------------*/
static int32_t IntBridgePerformanceTest(struct DibBridgeContext *pContext)
{
   uint32_t Start, End, Duration, i;
   uint32_t Loop = 2000;
   uint32_t Size = RAM_TEST_SIZE * Loop;
   uint32_t OffIntRam = DibBridgeGetRamAddr(pContext);
   uint32_t bitrate;

   Start = IntBridgeGetTimeInMs();
   for(i = 0; i < Loop; i++)
   {
      IntBridgeTestIfWriteFastMemory(pContext,rambuf,RAM_TEST_SIZE,OffIntRam);
   }
   End = IntBridgeGetTimeInMs();

   if(!Start && !End)
   {
      DIB_DEBUG(PORT_LOG,(CRB "ERROR IntBridgeGetTimeInMs may not be defined on this target" CRA));
      return 0;
   }

   Duration = End - Start;
   DIB_DEBUG(PORT_LOG,(CRB "Write %d bytes in %d ms" CRA, Size, Duration));
   if(Duration)
   {
      bitrate = Size / Duration;
      bitrate *= 1000;
      DIB_DEBUG(PORT_LOG,(CRB "Write BW = %d Bytes/Sec" CRA, bitrate));
   }
   else
   {
      DIB_DEBUG(PORT_LOG,(CRB "Write BW calculation is impossible, duration is null" CRA));
   }

   Start = IntBridgeGetTimeInMs();
   for(i = 0; i < Loop; i++)
   {
      IntBridgeTestIfReadFastMemory(pContext,rambuf,RAM_TEST_SIZE,OffIntRam);
   }
   End = IntBridgeGetTimeInMs();
   Duration = End - Start;
   DIB_DEBUG(PORT_LOG,(CRB "Read %d bytes in %d ms" CRA, Size, Duration));
   if(Duration)
   {
      bitrate = Size / Duration;
      bitrate *= 1000;
      DIB_DEBUG(PORT_LOG,(CRB "Read BW = %d Bytes/Sec" CRA, bitrate));
   }
   else
   {
      DIB_DEBUG(PORT_LOG,(CRB "Read BW calculation is impossible, duration is null" CRA));
   }

   return 0;
}

/*----------------------------------------------------------------------------
fill test Buffer with pattern 
-----------------------------------------------------------------------------*/
void IntBridgeInitRamBuf(uint8_t Val, uint32_t Size)
{ 
   uint32_t i;

   for(i = 0; i < Size / 2; i++)
   {
      rambuf[2*i]    = Val; 
      rambuf[2*i+1]  = (uint8_t) i;
      ramtest[2*i]   = 0;
      ramtest[2*i+1] = 0;
   }
}
#endif

#if (TEST_TRANSFERT == 1)
void DibBridgeSwap32(uint8_t * buf, int32_t Size)
{
   uint8_t reg;
   Size -= 4;

   while(Size >= 0)
   {
      reg         = buf[Size];
      buf[Size]   = buf[Size+3];
      buf[Size+3] = reg;
      reg         = buf[Size+1];
      buf[Size+1] = buf[Size+2];
      buf[Size+2] = reg;

      Size -= 4;
   }
}

void DibBridgeTestDmaTransfert(struct DibBridgeContext *pContext, uint32_t add, uint32_t *dst, uint32_t DmaLen)
{
   uint32_t i,diff=0;
   uint32_t org_word, copied_word;
   DIBSTATUS Status = DIBSTATUS_SUCESS;

   for(i = 0; i < DmaLen; i += 4)
   {
      if((Status = DibBridgeReadReg32(pContext, add, &org_word) != DIBSTATUS_SUCESS)
      {
         DibBridgeTargetLogFunction(CRB "CHECK DMA : Read Failed" CRA);
         return;
      }

      IntBridgeSwap32((uint8_t *) &org_word, 4);
      copied_word=*dst;
      if(copied_word != org_word)
      {
         DibBridgeTargetLogFunction(CRB "%6d %08x %08x" CRA,i,copied_word, org_word);
         diff++;
      }
      add+=4;
      dst+=4;
   }

   DibBridgeTargetLogFunction(CRB "CHECK DMA : %d diff" CRA,diff);
}
#endif


