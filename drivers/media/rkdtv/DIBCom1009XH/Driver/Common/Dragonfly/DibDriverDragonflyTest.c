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
* @file "DibDriverDragonflyTest.c"
* @brief Dragonfly specific debugging functions.
*
***************************************************************************************************/
#include "DibDriverDragonflyTest.h"




#include "DibDriverDragonflyMessages.h"

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (mSDK==0)

#if (USE_DRAGONFLY == 1)

/*******************************************
 *                TEST MDMA
 ******************************************/
#if CURRENT_TEST == TEST_MDMA

/* si fill_size vaut 0, droite, sinon fill_value   */
uint8_t  fill_value  = 0xBD;
uint32_t Size        = 0x2000;     /* 0x4080;      */
uint32_t fill_size   = 0x2000;     /* 0x20;        */
uint32_t fill_size_0 = 0;

/**
 * Step 0 : write something in the memory to have the poss. to move it after 
 */ 
static uint8_t DibDriverDragonflyWriteMemory(struct DibDriverContext *pContext)
{
   DibDriverDragonflyMDMA(pContext, 0, 0x70100000+fill_size, Size-fill_size, fill_size_0, fill_value, 0);

   return DIBSTATUS_SUCCESS;
}
 

/* soit verification que c'est une pente, soit que c'est fillvalue dans la memoire
   test 10 a 19 */
static uint8_t DibDriverDragonflyVerifyTest(struct DibDriverContext *pContext, uint32_t TestStep)
{
   DibDriverDragonflyMDMA(pContext, 0, 0x70100000 + ((TestStep-10)*Size), Size, fill_size, fill_value, 2);

   return DIBSTATUS_SUCCESS; 
}

#endif


/************************************************
 *                TEST NEW API
 ************************************************/
#if CURRENT_TEST == TEST_API

uint8_t gCurr = 0xFF;

struct Ch   ch[NB_CH_MAX];
struct Filt f[NB_FILT_MAX];
struct It   item[NB_ITEM_MAX];

void InitTest(void)
{
   /* Channel 0   */
   ch[0].freq     = 618000;
   ch[0].MinDemod = 1;
   ch[0].MaxDemod = 2;
   ch[0].Type     = eSTANDARD_DVB;
   ch[0].prio     = 0;
   ch[0].adapter  = 0xFF;
   ch[0].bdwth    = 0;
   ch[0].idx      = 0;

   /* Filt 0      */
   f[0].ParentCh  = 0;     
   f[0].FiltType  = eTS;          /* SIPSI;  */

   /* Filt 1      */
   f[1].ParentCh  = 0;     
   f[1].FiltType  = eTS;          /* MPEFEC; */
   
   /* Item 0      */
   item[0].ParentFilt        = 0;       
   item[0].ConfigInfo.Ts.Pid = 0x1FFF;     /* 0x1FFF; */  /* 0x45; */

   /*    item[0].ConfigInfo.SiPsi.Pid      = 0x12;        */
   /*    item[0].ConfigInfo.SiPsi.PidWatch = 0x34;        */
   /*    item[0].ConfigInfo.SiPSi.Crc      = 0xdeadbeef;  */

   /* Item 1      */
   item[1].ParentFilt = 0;       

   /*    i[tem1].config_info.TS.Pid = 0x2; */

   item[1].ConfigInfo.Ts.Pid = 110;

   /*    item[1].ConfigInfo.Mpe.NbRows           = 1024;   */
   /*    item[1].ConfigInfo.Mpe.MaxBurstDuration = 0x2;    */

   /* Item 2   */
   item[2].ParentFilt        = 0;       
   item[2].ConfigInfo.Ts.Pid = 120;     /* 0x1FFF; */       /* 0x45; */

   /* Item 3   */
   item[3].ParentFilt        = 0;       
   item[3].ConfigInfo.Ts.Pid = 130;     /* 0x1FFF; */       /* 0x45; */
}

#endif


uint8_t DibDriverDragonflyIntTest(struct DibDriverContext *pContext, uint32_t TestStep)
{
#if CURRENT_TEST == TEST_MDMA

   //printf(CRB CRB "TEST MDMA" CRA CRA);

   if(TestStep > 19)
      return DIBSTATUS_ERROR;

   if(TestStep == 0)
   {
      DibDriverDragonflyWriteMemory(pContext);     
   }
   else
   {
      if(TestStep > 9)
      {
         DibDriverDragonflyVerifyTest(pContext, TestStep);
      }else{

         DibDriverDragonflyMDMA(pContext, 0x70100000 + ((TestStep-1)*Size)+fill_size, 0x70100000 + (TestStep*Size), Size, fill_size, fill_value, 1);
      } 
   }


#endif

#if CURRENT_TEST == TEST_API
    
    //printf(CRB CRB "TEST NEW API" CRA CRA);

    switch(TestStep)
    {

        case 9:
        {
            uint32_t TestSubFirmware[2] = {0xCAFEBABE, 0x01234567};

            IntDriverDragonflySendMsgUdib(pContext, &TestSubFirmware, 2); 
        }
        break;
    }
#endif

#if CURRENT_TEST == NO_TEST
    //printf(CRB CRB "PAS DE TEST" CRA CRA);
#endif
    return DIBSTATUS_SUCCESS;
    
}

#if CURRENT_TEST == TEST_ACCESS
void DibDriverDragonflyTestAccess(struct DibDriverContext *pContext)
{
  uint32_t data32;
  uint16_t data16;
  uint8_t  data8;
  int32_t  len = 100, i;
  uint8_t  passed;

  /*------- JEDEC tests -------*/

  /* 32 bit read access */
  if(DibDriverReadReg32(pContext, 0x8000A0F8, &data32) != DIBSTATUS_SUCCESS)
  {
     //printf(CRB "=>Read Access FAILED" CRA);
     return;
  }
  //printf(CRB "=>Read JEDEC[0..31]=%08x...%s" CRA,data32,(data32 == 0x01B31901)?"PASSED":"FAILED");

  /* 16 bit read access */
  DibDriverReadReg16(pContext, 0x8000A0F8, &data16);
  //printf(CRB "=>Read JEDEC[0..15]=%04x...%s" CRA,data16,(data16 == 0x01B3)?"PASSED":"FAILED");
  
  DibDriverReadReg16(pContext, 0x8000A0FA, &data16);
  //printf(CRB "=>Read JEDEC[16..31]=%04x...%s" CRA,data16,(data16 == 0x1901)?"PASSED":"FAILED");
  
  /* 8 bit access */
  DibDriverReadReg8(pContext, 0x8000A0F8, &data8);
  //printf(CRB "=>Read JEDEC[0..7]=%02x...%s" CRA,data8,(data8 == 0x01)?"PASSED":"FAILED");
  
  DibDriverReadReg8(pContext, 0x8000A0F9, &data8);
  //printf(CRB "=>Read JEDEC[8..15]=%02x...%s" CRA,data8,(data8 == 0xB3)?"PASSED":"FAILED");
  
  DibDriverReadReg8(pContext, 0x8000A0FA, &data8);
  //printf(CRB "=>Read JEDEC[16..23]=%02x...%s" CRA,data8,(data8 == 0x19)?"PASSED":"FAILED");
  
  DibDriverReadReg8(pContext, 0x8000A0FB, &data8);
  //printf(CRB "=>Read JEDEC[24..31]=%02x...%s" CRA,data8,(data8 == 0x01)?"PASSED":"FAILED");

  /*------- SRAM tests -------*/

  /* 8 bit tests */
  passed = 1;

  for(i = 0; i < len; i++)
  {
     if(DibDriverWriteReg8(pContext, 0x70000000 + i, i & 0xFF) != DIBSTATUS_SUCCESS)
     {
        //printf(CRB "=>Write Access 8 FAILED" CRA);
        return;
     }
  }

  for(i=0; i<len; i++)
  {
      if(DibDriverReadReg8(pContext, 0x70000000 + i, &data8) != DIBSTATUS_SUCCESS)
      {
        // printf(CRB "=>Read Access 8 FAILED" CRA);
         return;
      }

      if(data8 != (i & 0xFF)) 
         passed = 0;
  }

  //printf(CRB "Test for SRAM 8 bit access (%d bytes)...%s" CRA, len, passed? "PASSED":"FAILED");

  /* 16 bit tests */
  passed = 1;

  for(i = 0; i < len; i += 2)
  {
     if(DibDriverWriteReg16(pContext, 0x70000000 + i, i & 0xFFFF) != DIBSTATUS_SUCCESS)
     {
        //printf(CRB "=>Write Access 16 FAILED" CRA);
        return;
     }
  }

  for(i = 0; i < len; i += 2)
  {
      if(DibDriverReadReg16(pContext, 0x70000000 + i, &data16) != DIBSTATUS_SUCCESS)
      {
         //printf(CRB "=>Read Access 16 FAILED" CRA);
         return;
      }

      if(data16 != (i & 0xFFFF)) 
         passed = 0;
  }
  //printf(CRB "Test for SRAM 16 bit access (%d bytes)...%s" CRA, len, passed? "PASSED":"FAILED");

  /* 32 bit tests */
  passed = 1;

  for(i = 0; i < len; i += 4)
  {
     if(DibDriverWriteReg32(pContext, 0x70000000 + i, i & 0xFFFFFFFF) != DIBSTATUS_SUCCESS)
     {
        //printf(CRB "=>Write Access 32 FAILED" CRA);
        return;
     }
  }

  for(i = 0; i < len; i += 4)
  {
      if(DibDriverReadReg32(pContext, 0x70000000 + i, &data32) != DIBSTATUS_SUCCESS)
      {
         //printf(CRB "=>Read Access 32 FAILED" CRA);
         return;
      }
      
      if(data32 != (i & 0xFFFFFFFF)) 
         passed = 0;
  }
  //printf(CRB "Test for SRAM 32 bit access (%d bytes)...%s" CRA, len, passed? "PASSED":"FAILED");
}
#endif /* TEST == TEST_ACCESS */

#endif /* USE_DRAGONFLY */
#endif
