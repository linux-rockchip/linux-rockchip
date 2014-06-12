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
* @file "DibBridgeFireflyTest.c"
* @brief Firefly specific bridge tests.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibBridgeFireflyTest.h"
#include "DibBridgeTestIf.h"

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))

/****************************************************************************/
/***** Local Functions ******************************************************/
/****************************************************************************/
static uint16_t DibBridgeFireflyTestIfReadReg(struct DibBridgeContext *pContext, uint16_t Addr);
static void DibBridgeFireflyTestIfWriteReg(struct DibBridgeContext *pContext, uint16_t Addr, uint16_t Data);
static void DibBridgeFireflyTestIfWriteMemory(struct DibBridgeContext *pContext, uint8_t *Buffer, uint32_t Size,  uint32_t Offset);
static void DibBridgeFireflyTestIfReadMemory(struct DibBridgeContext *pContext, uint8_t *Buffer, uint32_t Size,  uint32_t Offset);

extern uint8_t *rambuf;
extern uint8_t *ramtest;


/*----------------------------------------------------------------------------
Read single 16 bits register
-----------------------------------------------------------------------------*/
static uint16_t DibBridgeFireflyTestIfReadReg(struct DibBridgeContext *pContext, uint16_t Addr)
{
   uint8_t Buffer[2] = { 0, 0 };

   DibBridgeTargetRead(pContext, Addr, REGISTER_MODE, 2, Buffer);

   return (uint16_t)((((uint16_t)Buffer[0])<<8) | Buffer[1]);
}

/*----------------------------------------------------------------------------
Write single 16 bits register
-----------------------------------------------------------------------------*/
static void DibBridgeFireflyTestIfWriteReg(struct DibBridgeContext *pContext, uint16_t Addr, uint16_t Data)
{
   uint8_t Buffer[2];

   Buffer[0] = (Data >> 8) & 0xFF;
   Buffer[1] = (Data) & 0xFF;

   DibBridgeTargetWrite(pContext,Addr, REGISTER_MODE, 2, Buffer);
}

/*----------------------------------------------------------------------------
Write to Internal or External Memory 
-----------------------------------------------------------------------------*/
static void DibBridgeFireflyTestIfWriteMemory(struct DibBridgeContext *pContext, uint8_t *Buffer, uint32_t Size, uint32_t Offset)
{ 
   uint32_t IntRamAddress = 1024 + 32 +7; 
   uint16_t intram        = 1024 + 32; 
   uint32_t ad_min        = 0 + Offset;
   uint32_t ad_max        = 0x10000 + Offset;
   uint32_t ad_base       = 0 + Offset;

   /*** Configure Memory Controller ***/
   DibBridgeFireflyTestIfWriteReg(pContext, intram+0, 1);                      
   DibBridgeFireflyTestIfWriteReg(pContext, intram+1, (uint16_t) (ad_base >> 16)&0xffff); 
   DibBridgeFireflyTestIfWriteReg(pContext, intram+2, (uint16_t) (ad_base)&0xffff);       
   DibBridgeFireflyTestIfWriteReg(pContext, intram+3, (uint16_t) (ad_max >> 16)&0xffff);  
   DibBridgeFireflyTestIfWriteReg(pContext, intram+4, (uint16_t) (ad_max)&0xffff);        
   DibBridgeFireflyTestIfWriteReg(pContext, intram+5, (uint16_t) (ad_min >> 16)&0xffff);  
   DibBridgeFireflyTestIfWriteReg(pContext, intram+6, (uint16_t) (ad_min)&0xffff);        

   /*** Write into memory ***/
   /*DibBridgeTargetWrite(pContext, IntRamAddress, DATA_BYTE_MODE, Size, (uint8_t*)Buffer);*/
   DibBridgeWrite(pContext, IntRamAddress, DATA_BYTE_MODE, Buffer, Size);
}

/*----------------------------------------------------------------------------
Read Internal or External Memory 
-----------------------------------------------------------------------------*/
static void DibBridgeFireflyTestIfReadMemory(struct DibBridgeContext *pContext, uint8_t *Buffer, uint32_t Size, uint32_t Offset)
{ 
   uint32_t IntRamAddress = 1024 + 32 +7; 
   uint16_t intram        = 1024 + 32; 
   uint32_t ad_min        = 0 + Offset;
   uint32_t ad_max        = 0x10000 + Offset;
   uint32_t ad_base       = 0 + Offset;

   /*** Configure Memory Controller ***/
   DibBridgeFireflyTestIfWriteReg(pContext, intram+0, (0<<15)|1);
   DibBridgeFireflyTestIfWriteReg(pContext, intram+1, (uint16_t) (ad_base >> 16)&0xffff);
   DibBridgeFireflyTestIfWriteReg(pContext, intram+2, (uint16_t) (ad_base)&0xffff);
   DibBridgeFireflyTestIfWriteReg(pContext, intram+3, (uint16_t) (ad_max >> 16)&0xffff);
   DibBridgeFireflyTestIfWriteReg(pContext, intram+4, (uint16_t) (ad_max)&0xffff);
   DibBridgeFireflyTestIfWriteReg(pContext, intram+5, (uint16_t) (ad_min >> 16)&0xffff);
   DibBridgeFireflyTestIfWriteReg(pContext, intram+6, (uint16_t) (ad_min)&0xffff);
   DibBridgeFireflyTestIfWriteReg(pContext, intram+0, (1<<15)|1);

   /*** Read from memory ***/
/*   DibBridgeTargetRead(pContext, IntRamAddress, DATA_BYTE_MODE, Size, (uint8_t*)Buffer);*/
   DibBridgeRead(pContext, IntRamAddress, DATA_BYTE_MODE, Buffer, Size);
}

/*----------------------------------------------------------------------------
Register Write / Read Test 
-----------------------------------------------------------------------------*/
int32_t IntBridgeFireflyTestBasicRead(struct DibBridgeContext *pContext)
{
   uint16_t reg;
   int32_t  status = 0;

   reg = DibBridgeFireflyTestIfReadReg(pContext, 896);

   if(reg == 0x01B3)
   {
      DIB_DEBUG(PORT_LOG,(CRB "Vendor ID ok: %04x" CRA,reg));
   }
   else
   {
      DIB_DEBUG(PORT_LOG,(CRB "Vendor ID failed: %04x" CRA,reg));
      status = -1;
   }

   reg = DibBridgeFireflyTestIfReadReg(pContext,897);

   if((reg == 0x4001) || (reg == 0x4002) || (reg == 0x4003) || (reg == 0x4004) ||(reg == 0x4005))
   {
      DIB_DEBUG(PORT_LOG,(CRB "Chip ID ok: %04x" CRA,reg));
   }
   else
   {
      DIB_DEBUG(PORT_LOG,(CRB "Chip ID failed: %04x" CRA,reg));
      status = -1;
   }

   return (status);
}


/*----------------------------------------------------------------------------
Register Write / Read Test 
-----------------------------------------------------------------------------*/
int32_t IntBridgeFireflyTestRegister(struct DibBridgeContext *pContext)
{ 
   uint16_t  i;
   uint16_t data_wr, data_rd;
   uint16_t address;
   int32_t  status = 0;

   data_wr = 0x1234;
   address = 1819;

   for(i = 0 ; i < WORD_TEST_LOOP; i++) 
   {
      data_wr += i;
      data_rd  = 0;

      DibBridgeFireflyTestIfWriteReg(pContext,address, data_wr);

      data_rd = DibBridgeFireflyTestIfReadReg(pContext,address);

      DIB_DEBUG(PORT_LOG,(CRB "%d: Writes 0x%04x    Reads back 0x%04x" CRA,i, data_wr, data_rd));

      if(data_wr != data_rd)
      {
         DIB_DEBUG(PORT_LOG,(CRB "Writes 0x%04x    Reads back 0x%04x" CRA,data_wr, data_rd));
         status = -1;
         break;
      }
   }

   return (status);
}


/*----------------------------------------------------------------------------
Internal Memory Test 
-----------------------------------------------------------------------------*/
int32_t IntBridgeFireflyTestInternalRam(struct DibBridgeContext *pContext)
{ 
   uint32_t i;

   /*** Write and Read bytes from 1 to 128 bytes ***/
   IntBridgeInitRamBuf(1, RAM_TEST_SIZE);

   for(i = 2; i <= 128; i += 2)
   {
      DibBridgeFireflyTestIfWriteMemory(pContext, rambuf, i, OFFSET_RAM_INT);
      DibBridgeFireflyTestIfReadMemory(pContext, ramtest, i, OFFSET_RAM_INT);

      if(0 != memcmp(rambuf, ramtest, i))
      {
         DIB_DEBUG(PORT_LOG,(CRB "###### Internal Ram Test Error: %d Bytes ######" CRA, i));
         return -1;
      } 

      DIB_DEBUG(PORT_LOG,(CRB "Internal Ram Test (%d bytes) OK" CRA,i));
   }

   /*** Write and Read multiple 2K block ***/
   for(i = 0; i < RAM_TEST_LOOP; i++)
   {
      IntBridgeInitRamBuf((uint8_t)(i%256), RAM_TEST_SIZE);

      DibBridgeFireflyTestIfWriteMemory(pContext, rambuf, RAM_TEST_SIZE, OFFSET_RAM_INT);
      DibBridgeFireflyTestIfReadMemory(pContext, ramtest, RAM_TEST_SIZE, OFFSET_RAM_INT);

      if(0 != memcmp(rambuf, ramtest, RAM_TEST_SIZE))
      {
         DIB_DEBUG(PORT_LOG,(CRB "###### Internal Ram Test Error ######" CRA));
         return -1;
      } 

      DIB_DEBUG(PORT_LOG,(CRB "Internal Ram Test %d (%d bytes) OK" CRA,i, RAM_TEST_SIZE));
   }

   return (0);
}


/*----------------------------------------------------------------------------
External Memory Test 
-----------------------------------------------------------------------------*/
int32_t IntBridgeFireflyTestExternalRam(struct DibBridgeContext *pContext)
{ 
   uint32_t i;

   /*** Write and Read bytes from 1 to 128 bytes ***/
   IntBridgeInitRamBuf(1, RAM_TEST_SIZE);

   for(i = 2; i <= 128; i+=2)
   {
      DibBridgeFireflyTestIfWriteMemory(pContext, rambuf, i, OFFSET_RAM_EXT);
      DibBridgeFireflyTestIfReadMemory(pContext, ramtest, i, OFFSET_RAM_EXT);

      if(0 != memcmp(rambuf, ramtest, i))
      {
         DIB_DEBUG(PORT_LOG,(CRB "###### External Ram Test Error: %d Bytes ######" CRA, i));
         return -1;
      } 

      DIB_DEBUG(PORT_LOG,(CRB "External Ram Test (%d bytes) OK" CRA,i));
   }

   /*** Write and Read multiple 2K block ***/
   for(i = 0; i < RAM_TEST_LOOP; i++)
   {
      IntBridgeInitRamBuf((uint8_t) (i%256), RAM_TEST_SIZE);

      DibBridgeFireflyTestIfWriteMemory(pContext, rambuf, RAM_TEST_SIZE, OFFSET_RAM_EXT);
      DibBridgeFireflyTestIfReadMemory(pContext, ramtest, RAM_TEST_SIZE, OFFSET_RAM_EXT);

      if(0 != memcmp(rambuf,ramtest,RAM_TEST_SIZE))
      {
         DIB_DEBUG(PORT_LOG,(CRB "###### External Ram Test Error ######" CRA));
         return -1;
      } 

      DIB_DEBUG(PORT_LOG,(CRB "External Ram Test %d (%d bytes) OK" CRA,i, RAM_TEST_SIZE));
   }

   return (0);
}


/******************************************************************************
 * return RAM address
 ******************************************************************************/
uint32_t DibBridgeFireflyGetRamAddr(struct DibBridgeContext *pContext)
{
   return OFFSET_RAM_INT;
}
#endif


#endif /* USE_FIREFLY */
