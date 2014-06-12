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
* @file "DibBridgePayloadCheckers.c"
* @brief data check.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridge.h"

#if (SUPPORT_CAV == 1)
#include "DibBridgeCav.h"
#endif


#if (DIB_CHECK_RTP_DATA == 1)
static uint16_t VideoSeqNumber[20];
static uint16_t AudioSeqNumber[20];
static uint8_t  VideoFirstPacket;
static uint8_t  AudioFirstPacket;
#endif

#if (DIB_CHECK_RAWTS_DATA == 1)
const  uint8_t  BitMask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
#endif


#if (DIB_CHECK_RTP_DATA == 1)
/*-----------------------------------------------------------------------
|  DibIntBridgeCheckersPrintBuffer
-----------------------------------------------------------------------*/
static void DibIntBridgeCheckersPrintBuffer(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Cnt)
{
   int32_t i;
   int32_t j   = 0;
   int32_t line;

   while(Cnt > 0) 
   {
      if(Cnt > 16)
         line = 16;
      else
         line = Cnt;

      for(i = 0; i < line; i++) 
      {
         DIB_DEBUG(CHECKER_ERR, ("%02x ", pBuf[i + 16 * j]));
      }

      DIB_DEBUG(CHECKER_ERR, (CRB "" CRA));
      Cnt -= line;
      j++;
   }
}
#endif

/* Debug output */
#if (DIB_CHECK_DATA_IN_FILE == 1)
FILE     *pFile;
uint32_t  NbFrames = 0;
uint32_t  File = 0;
#define NB_FRAMES_MAX    10   /* 25 */
#endif

#if (DIB_CHECK_RAWTS_DATA == 1)
/**
* DibBridgeCheckRawTsInit
*/
uint8_t DibBridgeCheckRawTsInit(void)
{
#if (DIB_CHECK_DATA_IN_FILE == 1)
   pFile = fopen("Tdmb.file", "wb+");
   File = 1;
#endif
   
#if (SUPPORT_CAV == 1)
   DIB_CAV_RAWTS_INIT
#endif
   return DIBSTATUS_SUCCESS;
}

/**
* DibBridgeCheckRawTsData
*/
void DibBridgeCheckRawTsData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, uint8_t Item)
{
   static uint32_t init = 1;
   uint16_t        Pid;
   uint8_t         cc;
   uint8_t        *pBuffer = pBuf;
   uint32_t        Size_   = Size;
   uint8_t         FilterId = pContext->ItSvc[Item].FilterParent;
   struct CheckRawTs * pCheckRawTs = & pContext->FilterInfo[FilterId].CheckRawTs;

   DIB_ASSERT(pBuf);

   if(init)
   {
      DibBridgeCheckRawTsInit();
      init=0;
   }

#if (SUPPORT_CAV == 1)
   DIB_CAV_RAWTS
#endif

   if(Size % TP_SIZE != 0) 
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "TS Size not multiple of 188: is %d, %d of extra" CRA, Size, (Size % TP_SIZE)));
      return;
   }

   while(Size_ != 0) 
   {
      if(pBuffer[0] != 0x47) 
      {
         pCheckRawTs->CorruptedPacketsCount++;
      }
#if (DIB_CHECK_DATA_IN_FILE == 1)
      for(i = 0; i < TP_SIZE; i++)
      {
         fprintf(pFile, "%c", pBuffer[i]);
      }
#endif     
      
      if((pBuffer[1]&0x80) != 0)
      {
         pCheckRawTs->ErrorIndicFlag++;
      }

      Pid  = (pBuffer[1] & 0x1F) << 8;
      Pid |= pBuffer[2];

      if(Pid != 8191) 
      {
         pCheckRawTs->TotalNbPackets++;

         cc = pBuffer[3] & 0x0f;

         if((pCheckRawTs->PidsOk[Pid / 8] & BitMask[Pid & 7]) == 0) 
         {
            pCheckRawTs->PidsOk[Pid / 8] |= BitMask[Pid & 7];
            pCheckRawTs->PidsCc[Pid] = (cc + 1) & 0x0f;
            pCheckRawTs->TotalNbPid++;
            DIB_DEBUG(CHECKER_ERR, (CRB "New pid : %d" CRA, Pid));
         }
         else 
         {
            if(((pCheckRawTs->PidsCc[Pid]) != (cc))) 
            {
               /* DIB_DEBUG(CHECKER_ERR, (CRB "CC failed Pid %d received %d instead of %d packet %d" CRA, Pid, pCheckRawTs->PidsCc[Pid]&0xFE, cc&0xFE, (Size-Size_)/188)); */
               pCheckRawTs->DiscontinuitiesCount++;
            }
            pCheckRawTs->PidsCc[Pid] = (cc + 1) & 0x0f;
         }
      }else{
         pCheckRawTs->NbNullPackets++;
      }

      pBuffer += TP_SIZE;
      Size_   -= TP_SIZE;
   }

   DIB_DEBUG(CHECKER_LOG, (CRB "\E[H\E[2J" CRA)); /* clear the screen */
   DIB_DEBUG(CHECKER_LOG, (CRB "CorruptedPacketsCount\t%u" CRA, pCheckRawTs->CorruptedPacketsCount));
   DIB_DEBUG(CHECKER_LOG, (CRB "ErrorIndicFlag\t\t%u" CRA, pCheckRawTs->ErrorIndicFlag));
   DIB_DEBUG(CHECKER_LOG, (CRB "DiscontinuitiesCount\t%u" CRA, pCheckRawTs->DiscontinuitiesCount));
   DIB_DEBUG(CHECKER_LOG, (CRB "TotalNbPackets\t\t%u" CRA, pCheckRawTs->TotalNbPackets));
   DIB_DEBUG(CHECKER_LOG, (CRB "NbNullPackets\t\t%u" CRA, pCheckRawTs->NbNullPackets));
   DIB_DEBUG(CHECKER_LOG, (CRB "TotalNbPid\t\t%u" CRA, pCheckRawTs->TotalNbPid));
   DIB_DEBUG(CHECKER_LOG, (CRB "" CRA));

#if (DIB_CHECK_MONITORING == 1)
   /* To test RAWTS checker monitoring */
   pCheckRawTs->DiscontinuitiesCount++;
   pCheckRawTs->CorruptedPacketsCount++;
#endif

#if (DIB_CHECK_DATA_IN_FILE == 1)
   NbFrames++;
   if(NbFrames == NB_FRAMES_MAX)
   {
      fflush(pFile);
      fclose(pFile);
      File = 0;
      DIB_DEBUG(CHECKER_ERR, (CRB "END OF FILE" CRA));
      DIB_DEBUG(CHECKER_ERR, (CRB CRB "" CRA CRA));
   }
#endif

}
#endif /* DIB_CHECK_RAWTS_DATA */


#if (DIB_CHECK_FIG_DATA == 1)
#define NB_FIB_BYTES    30

uint8_t DibBridgeCheckFigInit(uint8_t *pBuf, uint32_t Size)
{
#if (DIB_CHECK_DATA_IN_FILE == 1)
   pFile = fopen("ext2.fic", "w+");
   File = 1;
#endif
   return 0;
}

void DibBridgeCheckFigData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size)
{
   uint8_t  Length    = 0;
   uint8_t  FIGType   = 0;
   uint8_t  FIGLength = 0;
   uint8_t  Size_     = Size;
   uint8_t *pBuffer   = pBuf;
   uint8_t  i         = 0;

   if(Size % NB_FIB_BYTES != 0) 
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "Fib Size not multiple of 30: is %d, %d of extra" CRA, Size, (Size % NB_FIB_BYTES)));
      return;
   }

   while(Size_ > 0)
   {

      Length = 0;
      DIB_DEBUG(CHECKER_ERR, (CRB "" CRA));

      while(Length < NB_FIB_BYTES)
      {
         FIGType   = ((pBuffer[Length] & 0xE0) >> 5);
         FIGLength = (pBuffer[Length] & 0x1F);
         DIB_DEBUG(CHECKER_ERR, (CRB "FIG : T %d L %d" CRA, FIGType, FIGLength));

         if((FIGType == 7) && (FIGLength == 0x1F))
         {
            DIB_DEBUG(CHECKER_ERR, (CRB "End Marker" CRA));
            break;
         }  
         else
         {
            if(FIGType == 0)
            {
               DIB_DEBUG(CHECKER_ERR, (CRB "FIGType 0 : (ext %d)" CRA, pBuffer[Length + 1]&0x1F));

#if (DIB_CHECK_DATA_IN_FILE == 1)
               if(File)
               {
                  if((pBuffer[Length + 1]&0x1F) == 2)
                  {
                     for(i = 0; i < (FIGLength + 1); i++)
                     {
                        fprintf(pFile, "%02x ", pBuffer[Length + i]);
                     }
                     fprintf(pFile, CRB CRB "" CRA CRA);                    
                  }
               }
#endif

               for(i = 0; i < FIGLength; i++)
                  DIB_DEBUG(CHECKER_ERR, ("%#x ", pBuffer[Length + i + 1]));

               DIB_DEBUG(CHECKER_ERR, (CRB "" CRA));
            }

         }
         Length += (FIGLength + 1); 
      }
      pBuffer += (NB_FIB_BYTES);
      Size_   -= NB_FIB_BYTES;

   }  

#if (DIB_CHECK_DATA_IN_FILE == 1)
   NbFrames++;
   if(NbFrames == NB_FRAMES_MAX)
   {
      fflush(pFile);
      fclose(pFile);
      File = 0;
      DIB_DEBUG(CHECKER_ERR, (CRB "File closed" CRA));      
   }
#endif
}
#endif

#if (DIB_CHECK_MSC_DATA == 1) 

#if (DIB_CHECK_DATA_IN_FILE == 1)
FILE     *pFileMsc;
uint32_t  NbFramesMsc = 0;
uint32_t FileMsc = 0;
#define NB_FRAMES_MAX_MSC    100   /* 25 */
#endif

uint8_t DibBridgeCheckMscInit(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item)
{
#if (DIB_CHECK_DATA_IN_FILE == 1)
   pFileMsc = fopen("msc.file", "wb+");
   FileMsc = 1;
#else
   uint16_t FrameSize = 0;
   FrameSize = ((pBuf[0] << 8) | pBuf[1]); 
   if(FrameSize > Size)
   {
      return 0;
   }
   if(FrameSize != 0)
   {
      pContext->ItSvc[Item].DataLenRx = FrameSize;
      pContext->ItSvc[Item].CurCc = pBuf[2];
      return 1;
   }
#endif
   return 0;
}

/**
 * DIB_CHECK_DATA_IN_FILE : 
 * SET TO 1 : All data are stored derectly and without processing in a file that ca be played
 * SET TO 0 : For non regression test. The msc data in eti file have been replaced by : 
 * byte 0, 1 : the size of the frame
 * byte 2 : a frame counter
 * byte 3 to the end of the frame : a counter from 0.
 * @param pcontext   
 * @param pBuf       data
 * @param Size       length of the data buffer
 * @param Item       item concerned
 * @return 1 if the check has been done, 0 if the init has to be redone to fix the frame length
 */
uint8_t DibBridgeCheckMscData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item)
{
#if (DIB_CHECK_DATA_IN_FILE == 1)
   uint32_t i = 0;
   if(NbFramesMsc == NB_FRAMES_MAX_MSC)
      return;
   for(i = 0; i < Size; i++)
   {
      fprintf(pFileMsc, "%c", pBuf[i]);
   }
   
   NbFramesMsc++;
   if(NbFramesMsc == NB_FRAMES_MAX_MSC)
   {
      fflush(pFileMsc);
      fclose(pFileMsc);
      FileMsc = 0;
      DIB_DEBUG(CHECKER_ERR, (CRB "File closed" CRA));      
   }
   return 0;
#else
   uint8_t LastGlobalInc = pContext->ItSvc[Item].CurCc;
   uint32_t FrameSize = pContext->ItSvc[Item].DataLenRx;
   uint32_t Count = 0;
   uint32_t LocalNbMscPacket = 0;
   uint8_t SizeErrCount = 0;  
   uint32_t * pCcFailCnt, * pErrCnt;
   uint32_t * pNbMscPacket;
   pCcFailCnt = &(pContext->ItSvc[Item].CcFailCnt);
   pErrCnt  = &(pContext->ItSvc[Item].ErrCnt);
   pNbMscPacket = &(pContext->ItSvc[Item].NbMaxFrames);
   
   if(Size < FrameSize)
   {
      /* Error or maybe we do not have to trust FrameSize value */
      return 0;
   }
   else
   {
      while(Count < Size)
      {
         uint16_t LocalFrameSize = (pBuf[Count] << 8) | pBuf[Count + 1];
         if(LocalFrameSize != FrameSize)
         {
            SizeErrCount++;
            /* 10 continuous errors ... maybe we do not have to trust FrameSize value */
            if(SizeErrCount > (LocalNbMscPacket + 1))
/*            if(SizeErrCount == 10)*/
            {
               DIB_DEBUG(CHECKER_ERR, ("MSC : Reset the check algo size %u is not correct\n", FrameSize));
               return 0;
            }
            (*pErrCnt)++;
         }
         else
         {
            uint32_t ii;
            uint8_t Val = 0;
            SizeErrCount = 0; /* Reinit the value */
            if(pBuf[Count + 2] != LastGlobalInc)
            {
               /* nb frames are bad... */
               (*pCcFailCnt)++;
            }

            for(ii = Count + 3; ii < Count + FrameSize; ii++)
            {
               if(pBuf[ii] != Val)
               {
                  (*pErrCnt)++;
                  goto Error;
               }
               Val++;
            }
            (*pNbMscPacket)++;
            LocalNbMscPacket++;
            LastGlobalInc = pBuf[Count + 2] + 1;
         }
Error:
         Count += FrameSize;
      }
   }
   pContext->ItSvc[Item].CurCc = LastGlobalInc;
/*   DIB_DEBUG(CHECKER_ERR, ("MSC Check item %d : T/CC/KO : %u/%u/%u\n", Item, *pNbMscPacket, *pCcFailCnt, *pErrCnt)); */
   return 1;
#endif
}

uint8_t DibBridgeCheckMscDataGroup(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item)
{
   static uint8_t temp = 0xFF;
   static uint32_t CcFailCnt = 0;
   static uint32_t NbMaxFrames = 0;
   uint8_t ii = 0;
   uint8_t ExtField, CrcFlag, SegFlag, UserAcc, CC, RepIdx;

   /* dg header */
/*   ExtField = ((pBuf[ii]&0x80) >> 7) << 1;  nb bytes of extension field
   CrcFlag = (pBuf[ii]&0x40) >> 6;
   SegFlag = (pBuf[ii]&0x20) >> 5;
   UserAcc = (pBuf[ii]&0x10) >> 4;*/
   RepIdx = pBuf[ii+1]&0xF;

   CC = (pBuf[ii+1]&0xF0) >> 4;

   if((temp != CC) && (temp != 0xFF))
   {
      CcFailCnt++;
      DIB_DEBUG(CHECKER_ERR, (CRB "KO Cur %d CC %d RepIdx %d" CRA, temp, CC, RepIdx));
   }
   else
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "OK Cur %d CC %d RepIdx %d"CRA, temp, CC, RepIdx));
   }
   temp = (CC + 1)%16;
   NbMaxFrames++;

   pContext->ItSvc[Item].NbMaxFrames++;
   pContext->ItSvc[Item].CurCc = (CC + 1)%16;

   DIB_DEBUG(CHECKER_ERR, (CRB "DiscontinuitiesCount\t%u" CRA, CcFailCnt));
   DIB_DEBUG(CHECKER_ERR, (CRB "TotalNbPackets\t\t%u" CRA, NbMaxFrames));
   DIB_DEBUG(CHECKER_ERR, (CRB "" CRA));

   return 1;
}
#endif

#if (DIB_CHECK_IP_DATA == 1)
/*-----------------------------------------------------------------------
|  DibBridgeCheckIpData
-------------------------------------------------------------------------
|  For IP checker v1
|
-----------------------------------------------------------------------*/
uint8_t DibBridgeCheckIpData(struct DibBridgeContext *pContext,  uint8_t *Data, uint32_t Cnt, uint8_t item)
{
   uint32_t *CcFailCnt, *IpErrCnt;
   uint16_t *PacketCc;
   uint8_t  *pCurrent;
   uint32_t  NbIpPacket;
   uint16_t  i, Size;
   uint8_t   Err;

   pCurrent  = Data;
   CcFailCnt = &(pContext->ItSvc[item].CcFailCnt);
   IpErrCnt  = &(pContext->ItSvc[item].ErrCnt);
   PacketCc  = &(pContext->ItSvc[item].CurCc);

   NbIpPacket = 0;
   Err        = 0;

   DIB_DEBUG(CHECKER_ERR, (CRB "-- IP CHECK svc %2d, %d bytes: ", item, Cnt));

   /* checking IP counter */
   while(pCurrent < (Data + Cnt) && *pCurrent != 0 && Err < 3) 
   {
      Size = 0;

      switch (*pCurrent & 0xf0) 
      {
      case 0x40:
         /* IPV4 support */
         Size = (pCurrent[2] << 8) + (pCurrent[3]);

         if(Size > 1600)
         {
            Err = 3;
            DIB_DEBUG(CHECKER_ERR, (CRB "ERROR: IP size oversize at Offset %d: 0x%02x - 0x%02x 0x%02x" CRA,
                                   (uint32_t) (pCurrent - Data), pCurrent[0], pCurrent[1], pCurrent[2]));
            break;
         }
         if(Size > 30) 
         {
            if(*PacketCc != 0xFFFF) 
            {
               if((*PacketCc + 1) % 256 != pCurrent[28]) 
               {
                  DIB_DEBUG(CHECKER_ERR, ("CC fail: 0x%02x should be 0x%02x, ", pCurrent[28], *PacketCc));
                  Err = 1;
               }
            }
            *PacketCc = pCurrent[28];

            for(i = 29; i < Size; i++) 
            {
               if(pCurrent[i] != ((i - 29) % 256)) 
               {
                  Err = 3;
                  DIB_DEBUG(CHECKER_ERR, (CRB "ERROR (IPv4): 0x%02x instead of 0x%02x at Offset %d (%d)" CRA,
                                         pCurrent[i], ((i - 29) % 256),
                                         (uint32_t) (pCurrent + i - Data), i));

                  DIB_DEBUG(CHECKER_LOG, (CRB "Values around are: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x - 0x%02x - 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" CRA,
                                          pCurrent[i - 5], pCurrent[i - 4], pCurrent[i - 3],
                                          pCurrent[i - 2], pCurrent[i - 1], pCurrent[i],
                                          pCurrent[i + 1], pCurrent[i + 2], pCurrent[i + 3],
                                          pCurrent[i + 4], pCurrent[i + 5]));
                  break;
               }
            }
            NbIpPacket++;
         }
         else
         {
            if(((*pCurrent & 0xf) << 2) == Size)
            {
               NbIpPacket++;
            }
         }
         break;

      case 0x60:
         /* IPV6 support */
         Size = 40 + (pCurrent[4] << 8) + (pCurrent[5]);

         if(Size > 60) 
         {
            uint16_t i;

            if(*PacketCc != 0xFFFF) 
            {
               if((*PacketCc + 1) % 256 != pCurrent[48]) 
               {
                  DIB_DEBUG(CHECKER_ERR, ("CC fail: 0x%02x should be 0x%02x, ", pCurrent[48], *PacketCc));
                  Err = 1;
               }
            }
            *PacketCc = pCurrent[48];

            for(i = 49; i < Size; i++) 
            {
               if(pCurrent[i] != ((i - 49) % 256)) 
               {
                  Err = 3;

                  DIB_DEBUG(CHECKER_ERR, (CRB "ERROR (IPv6): 0x%02x instead of 0x%02x at Offset %d (%d)" CRA,
                                         pCurrent[i], ((i - 49) % 256),
                                         (uint32_t) (pCurrent + i - Data), i));

                  DIB_DEBUG(CHECKER_LOG, ("Values around are: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x - 0x%02x - 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
                                          pCurrent[i - 5], pCurrent[i - 4], pCurrent[i - 3],
                                          pCurrent[i - 2], pCurrent[i - 1], pCurrent[i],
                                          pCurrent[i + 1], pCurrent[i + 2], pCurrent[i + 3],
                                          pCurrent[i + 4], pCurrent[i + 5]));

                  break;
               }
            }
            NbIpPacket++;
         }
         break;

      default:
         Err = 4;

         if(pCurrent > Data + 5) 
         {
            DIB_DEBUG(CHECKER_ERR, (CRB "ERROR: byte not IP at Offset %d: 0x%02x 0x%02x - 0x%02x - 0x%02x 0x%02x" CRA,
                                   (uint32_t) (pCurrent - Data), pCurrent[-2], pCurrent[-1],
                                   pCurrent[0], pCurrent[1], pCurrent[2]));
         }
         else 
         {
            DIB_DEBUG(CHECKER_ERR, (CRB "ERROR: byte not IP at Offset %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" CRA,
                                   (uint32_t) (pCurrent - Data), pCurrent[0], pCurrent[1],
                                   pCurrent[2], pCurrent[3], pCurrent[4]));
         }
         break;
      }

      if(Size == 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "IP check: null Size found!" CRA));
         Err = 3;
         break;
      }
      pCurrent += Size;
   }

   if(NbIpPacket == 0)
   {
      DIB_DEBUG(CHECKER_ERR, ("ERROR, "));
      Err = 2;
   }

   if(Err == 1)
      (*CcFailCnt)++;

   if(Err > 1)
      (*IpErrCnt)++;

   if(!Err)
      DIB_DEBUG(CHECKER_ERR, ("OK, "));

   if(Err < 3)
      DIB_DEBUG(CHECKER_ERR, ("%d IP packets found" CRA, NbIpPacket));


#if (DIB_CHECK_MONITORING == 1)
/* To test IP checker monitoring */
      (*CcFailCnt)++;
      (*IpErrCnt)++;
#endif

   return 0;
}
#endif /* DIB_CHECK_IP_DATA */

#if (DIB_CHECK_RTP_DATA == 1)
/*-----------------------------------------------------------------------
|  IntBridgeCheckRtpPackets
-------------------------------------------------------------------------
|
|
-----------------------------------------------------------------------*/
static uint8_t IntBridgeCheckRtpPackets(struct DibBridgeContext *pContext, uint8_t *pData, uint32_t Size, uint8_t MpeBurstStart, uint8_t SvcNb)
{
   uint8_t  rc       = 0;
   uint8_t  DtOff;
   uint16_t SeqNumber;

#if (SUPPORT_CAV == 1)
   DIB_CAV_IP
#endif

   if(MpeBurstStart) 
   {
      VideoFirstPacket = 1;
      AudioFirstPacket = 1;
   }

   if((*pData & 0xf0) == 0x40)
   {
      DtOff = 29;       /* IPv4 */
   }
   else if((*pData & 0xf0) == 0x60)
   {
      DtOff = 49;       /* IPv6 */
   }
   else 
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "Service %d, Not IP Data" CRA, SvcNb));
      return 1;
   }

   if((pData[DtOff] & 0x7f) == 96) 
   {
      SeqNumber = (pData[DtOff + 1] * 256 + pData[DtOff + 2]);

      if((VideoSeqNumber[SvcNb]) != SeqNumber) 
      {
         if(VideoFirstPacket) 
         {
            DIB_DEBUG(CHECKER_ERR, (CRB "## starting with video seq# %d" CRA, SeqNumber));
         }
         else 
         {
            rc = 3;
            DIB_DEBUG(CHECKER_ERR, (CRB "** missing %d video RTP packets: seq# %d, expected %d (miss %d)"
                                    CRA, SeqNumber - VideoSeqNumber[SvcNb], SeqNumber,
                                    VideoSeqNumber[SvcNb],
                                    SeqNumber - VideoSeqNumber[SvcNb]));
         }
         VideoSeqNumber[SvcNb] = SeqNumber + 1;
      }
      else
      {
         VideoSeqNumber[SvcNb]++;
      }

      VideoFirstPacket = 0;
   }
   else if((pData[DtOff] & 0x7f) == 97)
   {
      SeqNumber = (pData[DtOff + 1] * 256 + pData[DtOff + 2]);

      if((AudioSeqNumber[SvcNb]) != SeqNumber) 
      {
         if(AudioFirstPacket) 
         {
            DIB_DEBUG(CHECKER_ERR, (CRB "## starting with audio seq# %d" CRA, SeqNumber));
         }
         else 
         {
            rc = 3;
            DIB_DEBUG(CHECKER_ERR, (CRB "** missing %d audio RTP packets: seq# %d, expected %d (miss %d)"
                                    CRA, SeqNumber - AudioSeqNumber[SvcNb], SeqNumber,
                                    AudioSeqNumber[SvcNb],
                                    SeqNumber - AudioSeqNumber[SvcNb]));
         }
         AudioSeqNumber[SvcNb] = SeqNumber + 1;
      }
      else
      {
         AudioSeqNumber[SvcNb]++;
      }

      AudioFirstPacket = 0;
   }
   else if ((pData[DtOff] & 0xf8) == 0xc8)   /* RTCP packet */
   {
   }
   else                                      /* Unknown type */
   { 
      rc = 2;
   }

   return rc;
}

/*-----------------------------------------------------------------------
|  DibBridgeCheckRtpData
-------------------------------------------------------------------------
|  Parse MPE frame and RTP continuity counter
|
-----------------------------------------------------------------------*/
void DibBridgeCheckRtpData(struct DibBridgeContext *pContext, uint8_t *pData, uint32_t MpeSize, uint8_t SvcNb)
{
   uint8_t *CurBuf;
   uint32_t packets, error, Size, Ret;
   uint16_t min, max;
   uint8_t  SyncLost, BurstStart;

   min        = 0xffff;
   max        = 0;
   packets    = 0;
   error      = 0;
   Size       = 0;
   SyncLost   = 1;
   BurstStart = 1;
   Ret        = 0;

   DIB_DEBUG(CHECKER_ERR, (CRB "-- RTP CHECK svc %d, %d bytes: ", SvcNb, MpeSize));

   CurBuf = pData;

   while(CurBuf < pData + MpeSize) 
   {
      /*** End of MPE frame is reached, exit ****/
      if(*CurBuf == 0x00) 
      {
         if(SyncLost == 1) 
         {
            /* DibErrPrint(CRB "Loosing Sync, Offset=%d   Frame Size=%d" CRA, CurBuf - pData,MpeSize); */
            SyncLost = 0;
         }
         CurBuf++;
         continue;
      }

      /**** IPV6 ****/
      if((*CurBuf & 0xf0) == 0x60) 
      {
         /*** Read IP packet Size ***/
         Size = 40 + (CurBuf[4] << 8) + CurBuf[5];
      }
      /**** IPV4 ****/
      else if((*CurBuf & 0xf0) == 0x40) 
      {
         /*** Read IP packet Size ***/
         Size = (CurBuf[2] << 8) + CurBuf[3];
      }
      else 
      {
         /*** Unknown Format: exit ****/
         DIB_DEBUG(CHECKER_ERR, ("#### MPE frame: wrong header byte 0x%2x, Offset: %d",
                                 *CurBuf, (uint32_t) (CurBuf - pData)));

         DIB_DEBUG(CHECKER_ERR, (", last pkt Size %d" CRA, Size));

         if(Size > 0x40)
            DIB_DEBUG(CHECKER_ERR, (CRB "Truncating...." CRA));

         Size = 0x40;

         DibIntBridgeCheckersPrintBuffer(pContext, &pData[CurBuf - pData - (Size / 2)], Size);
         error = 1;
         break;
      }

      if(Size == 0) 
      {
         DIB_DEBUG(CHECKER_ERR, (CRB "error: null Size found!" CRA));
         error = 1;
         break;
      }

      if(min > Size)
         min = Size;

      if(max < Size)
         max = Size;

      /**** Submit IP Packet ***/
      if(CurBuf + Size <= pData + MpeSize) 
      {
         Ret = IntBridgeCheckRtpPackets(pContext, CurBuf, Size, BurstStart, SvcNb);
         if(Ret != 0) 
         {
            error = Ret;
            break;
         }
         BurstStart = 0;
      }

      CurBuf += Size;
      packets++;
   }

   DIB_DEBUG(CHECKER_ERR, ("(min/max IP Size %d/%d) ", min, max));

   if(!error) 
   {
      DIB_DEBUG(CHECKER_ERR, ("OK:"));
   }
   else 
   {
      DIB_DEBUG(CHECKER_ERR, ("## Failed at %s level after", (error == 1) ? "IP" : "RTP"));
      if(error <= 2)
      {
         pContext->ItSvc[SvcNb].ErrCnt ++;
      }
      else
      {
         pContext->ItSvc[SvcNb].CcFailCnt ++;
      }
   }

   DIB_DEBUG(CHECKER_ERR, (" %d packets" CRA, packets));

#if (DIB_CHECK_MONITORING == 1)
   /* To test RTP checker monitoring */
   pContext->ItSvc[SvcNb].CcFailCnt ++;
   pContext->ItSvc[SvcNb].ErrCnt ++;
#endif
}
#endif

#if (DIB_CHECK_PES_DATA == 1)
/*-----------------------------------------------------------------------
|  DibBridgeCheckPesData
-------------------------------------------------------------------------*/
void DibBridgeCheckPesData(struct DibBridgeContext *pContext, uint8_t *pBuf, struct DibBridgeDmaCtx * pDmaCtx)
{
   uint8_t *PesStartAddr = pBuf;
   uint8_t StreamId = 0;
   uint16_t PesPacketLen;
   uint32_t Offset = 0, temp = 0;
   uint8_t Format = pDmaCtx->DmaFlags.Type;
   uint32_t Size = pDmaCtx->DmaLen;
   static int32_t OffsetAudio = 0, NbSyncAudio = 0, NbLostSyncAudio = 0;
   static int32_t OffsetOther = 0, NbSyncOther = 0, NbLostSyncOther = 0;
   int32_t *pPreviousOffset, *pNbSync, *pNbLostSync;

   if(Format == FORMAT_PES_VIDEO)
   {
/*
Video PES cannot really be checked because:
   - their section length is equal to 0
   - the buffer may not contain section start
*/
#if (SUPPORT_CAV == 1)
      DIB_CAV_PES_VIDEO
#endif
      return;
   }
   else if(Format == FORMAT_PES_OTHER)
   {
#if (SUPPORT_CAV == 1)
      DIB_CAV_PES_OTHER
#endif
      pPreviousOffset = &OffsetOther;
      pNbSync = &NbSyncOther;
      pNbLostSync = &NbLostSyncOther;
   }
   else if(Format == FORMAT_PES_AUDIO)
   {
#if (SUPPORT_CAV == 1)
      DIB_CAV_PES_AUDIO
#endif
      pPreviousOffset = &OffsetAudio;
      pNbSync = &NbSyncAudio;
      pNbLostSync = &NbLostSyncAudio;
   }
   else
   {
      return;
   }
/*
For Audio and Other PES, the checker cannot only be based on the current buffer
as PES start code prefix can also appears in the PES payload.
At the first lost synchronization, a fail may happen and should be ignored.
*/
   Offset = *pPreviousOffset;
   while(Offset <= (Size - 3))
   {
      if(PesStartAddr[Offset + 0] != 0 || PesStartAddr[Offset + 1] != 0 || PesStartAddr[Offset + 2] != 1)
      {
         if(Offset != *pPreviousOffset)
         {
            DIB_DEBUG(CHECKER_ERR, (CRB "PES checker FAILED at offset %d format %d: %02x %02x %02x  ", Offset, Format, PesStartAddr[Offset + 0],PesStartAddr[Offset + 1],PesStartAddr[Offset + 2]));
            (*pNbLostSync)++;
            temp = Offset;
         }
         do
         {
            Offset ++;
         } while((Offset <= (Size - 3)) && (PesStartAddr[Offset + 0] != 0 || PesStartAddr[Offset + 1] != 0 || PesStartAddr[Offset + 2] != 1));
         if(temp)
            DIB_DEBUG(CHECKER_ERR, ("syncback %d bytes after"CRA, Offset - temp));
         if(Offset > (Size - 3))
         {
            *pPreviousOffset = 0;
         }
      }
      else
      {
         (*pNbSync)++;
         StreamId = PesStartAddr[Offset + 3];
         PesPacketLen = (PesStartAddr[Offset + 4] << 8) | (PesStartAddr[Offset + 5]);
         Offset += (6 + PesPacketLen);
         *pPreviousOffset = Offset - Size;
      }
   }
   if(*pNbLostSync || (*pNbSync > 100))
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "-- PES checker for %s type 0x%02x   success:%d fail:%d " CRA,((StreamId & 0xf0) == 0xc0) ? "AUDIO" : "OTHER", StreamId, *pNbSync, *pNbLostSync));
      *pNbSync = 0;
      *pNbLostSync = 0;
   }
}
#endif

#if (DIB_CHECK_PCR_DATA == 1)
/*-----------------------------------------------------------------------
|  DibBridgeCheckPcrData: only displays received PCR field
-------------------------------------------------------------------------*/
void DibBridgeCheckPcrData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size)
{
   if(Size != 7)
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "Received incorrect PCR data length %d instead of 7" CRA, Size));
   }
   else
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "PCR: %02x %02x%02x %02x%02x %02x%02x" CRA, pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5], pBuf[6]));
   }
}
#endif

#if (DIB_CHECK_CMMB_DATA == 1)
static uint32_t IntBridgeCrc32Compute(uint8_t *Buf, uint32_t Length)
{
   uint32_t result;
   uint32_t i, j;
   uint8_t byte;

   result = -1;
   for(i = 0; i < Length; i++)
   {
      byte = *(Buf++);
      for(j = 0; j < 8; j++)
      {
         if((byte >> 7) ^ (result >> 31))
         {
            result = (result << 1) ^ ((uint32_t)0x04C11DB7);
         }
         else
         {
            result = (result << 1);
         }
         byte <<= 1;
      }
   }

   return result;
}

typedef struct MultiplexFrameHeader
{
   uint32_t InitialCode;
   uint8_t  Headerlen;
   uint16_t ProtocolNumber;
   uint16_t LowerProtocolNumber;
   uint16_t MultiplexFrameId;
   uint8_t  HaveUrgentBroadcast;
   uint8_t  HaveNextFrame;
   uint8_t  ChangeNotifFrameNbr;
   uint8_t  NitUpdateNumber;
   uint8_t  CmctUpdateNumber;
   uint8_t  CsctUpdateNumber;
   uint8_t  SmctUpdateNumber;
   uint8_t  SsctUpdateNumber;
   uint8_t  EsgUpdateNumber;
   uint8_t  QuantityOfMsf;
   uint32_t MsfLength[15];
   uint8_t  ParameterOfNextFrame[5];
   uint32_t Crc32;
} tMultiplexFrameHeader;

/*
static void IntBridgePrintMultiplexFrameHeader(struct DibBridgeContext *pContext, tMultiplexFrameHeader *Header)
{
   int i;
   DIB_DEBUG(CHECKER_LOG, (CRB "--------------------------------" CRA));
   DIB_DEBUG(CHECKER_LOG, (CRB "MULTIPLEX FRAME HEADER" CRA));
   DIB_DEBUG(CHECKER_LOG, (CRB "--------------------------------" CRA));
   DIB_DEBUG(CHECKER_LOG, (CRB "InitialCode         = %d" CRA, Header->InitialCode));
   DIB_DEBUG(CHECKER_LOG, (CRB "Headerlen           = %d" CRA, Header->Headerlen));
   DIB_DEBUG(CHECKER_LOG, (CRB "ProtocolNumber      = %d" CRA, Header->ProtocolNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "LowerProtocolNumber = %d" CRA, Header->LowerProtocolNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "MultiplexFrameId    = %d" CRA, Header->MultiplexFrameId));
   DIB_DEBUG(CHECKER_LOG, (CRB "HaveUrgentBroadcast = %d" CRA, Header->HaveUrgentBroadcast));
   DIB_DEBUG(CHECKER_LOG, (CRB "HaveNextFrame       = %d" CRA, Header->HaveNextFrame));
   DIB_DEBUG(CHECKER_LOG, (CRB "ChangeNotifFrameNbr = %d" CRA, Header->ChangeNotifFrameNbr));
   DIB_DEBUG(CHECKER_LOG, (CRB "NitUpdateNumber     = %d" CRA, Header->NitUpdateNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "CmctUpdateNumber    = %d" CRA, Header->CmctUpdateNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "CsctUpdateNumber    = %d" CRA, Header->CsctUpdateNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "SmctUpdateNumber    = %d" CRA, Header->SmctUpdateNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "SsctUpdateNumber    = %d" CRA, Header->SsctUpdateNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "EsgUpdateNumber     = %d" CRA, Header->EsgUpdateNumber));
   DIB_DEBUG(CHECKER_LOG, (CRB "QuantityOfMsf       = %d" CRA, Header->QuantityOfMsf));
   for(i = 0; i < Header->QuantityOfMsf; i++)
      DIB_DEBUG(CHECKER_LOG, (CRB "  MsfLength[%02d] = %d" CRA, i, Header->MsfLength[i]));
   if(Header->HaveNextFrame)
      for(i = 0; i < 5; i++)
         DIB_DEBUG(CHECKER_LOG, (CRB "  ParameterOfNextFrame[%02d] = %d" CRA, i, Header->ParameterOfNextFrame[i]));
   DIB_DEBUG(CHECKER_LOG, (CRB "Crc32               = 0x%08x" CRA, Header->Crc32));
}
*/
static DIBSTATUS IntBridgeReadMultiplexFrameHeader(struct DibBridgeContext *pContext, tMultiplexFrameHeader *Header, uint8_t *Mfh)
{
   uint8_t Msf;
   /* Extract from buffer. */
   uint32_t ofs = 0;

   /* Sanity checks. */
   if((!Header) || (!Mfh))
      return DIBSTATUS_ERROR;

   /* [0...3] */
   Header->InitialCode          = (((uint32_t)Mfh[ofs])   << 24)&0xff000000;
   Header->InitialCode         |= (((uint32_t)Mfh[ofs+1]) << 16)&0x00ff0000;
   Header->InitialCode         |= (((uint32_t)Mfh[ofs+2]) <<  8)&0x0000ff00;
   Header->InitialCode         |= (((uint32_t)Mfh[ofs+3]) <<  0)&0x000000ff;
   ofs += 4;

   /* [4] */
   Header->Headerlen            = Mfh[ofs];
   ofs += 1;

   /* [5] */
   Header->ProtocolNumber       = (Mfh[ofs] >> 3)&0x1f;
   Header->LowerProtocolNumber  = (Mfh[ofs] << 2)&0x1c;
   ofs += 1;

   /* [6] */
   Header->LowerProtocolNumber |= (Mfh[ofs] >> 6)&0x03;
   Header->MultiplexFrameId     = (Mfh[ofs])&0x3f;
   ofs += 1;

   /* [7] */
   Header->HaveUrgentBroadcast  = (Mfh[ofs] >> 6)&0x03;
   Header->HaveNextFrame        = (Mfh[ofs] >> 5)&0x01;
   Header->ChangeNotifFrameNbr  = (Mfh[ofs])&0x03;
   ofs += 1;

   /* [8] */
   Header->NitUpdateNumber     = (Mfh[ofs] >> 4)&0x0f;
   Header->CmctUpdateNumber    = (Mfh[ofs])&0x0f;
   ofs += 1;

   /* [9] */
   Header->CsctUpdateNumber    = (Mfh[ofs] >> 4)&0x0f;
   Header->SmctUpdateNumber    = (Mfh[ofs])&0x0f;
   ofs += 1;

   /* [10] */
   Header->SsctUpdateNumber    = (Mfh[ofs] >> 4)&0x0f;
   Header->EsgUpdateNumber     = (Mfh[ofs])&0x0f;
   ofs += 1;

   /* [11] */
   Header->QuantityOfMsf       = (Mfh[ofs])&0x0f;
   ofs += 1;

   /* Loop over multiplex subframes. */
   for(Msf = 0; Msf < Header->QuantityOfMsf; Msf++)
   {
      Header->MsfLength[Msf]  = (((uint32_t)Mfh[ofs])   << 16)&0x00ff0000;
      Header->MsfLength[Msf] |= (((uint32_t)Mfh[ofs+1]) <<  8)&0x0000ff00;
      Header->MsfLength[Msf] |= (((uint32_t)Mfh[ofs+2]) <<  0)&0x000000ff;
      ofs += 3;
   }

   /* Next frame parameters. */
   if(Header->HaveNextFrame)
   {
      Header->ParameterOfNextFrame[0] = Mfh[ofs];
      Header->ParameterOfNextFrame[1] = Mfh[ofs+1];
      Header->ParameterOfNextFrame[2] = Mfh[ofs+2];
      Header->ParameterOfNextFrame[3] = Mfh[ofs+3];
      Header->ParameterOfNextFrame[4] = Mfh[ofs+4];
      ofs += 5;
   }

   /* Crc. */
   Header->Crc32  = (((uint32_t)Mfh[ofs])   << 24)&0xff000000;
   Header->Crc32 |= (((uint32_t)Mfh[ofs+1]) << 16)&0x00ff0000;
   Header->Crc32 |= (((uint32_t)Mfh[ofs+2]) <<  8)&0x0000ff00;
   Header->Crc32 |= (((uint32_t)Mfh[ofs+3]) <<  0)&0x000000ff;
   ofs += 4;

   /* Check crc. */
   if(IntBridgeCrc32Compute(Mfh, ofs) != 0)
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "*** Error: Wrong MF header CRC32" CRA));
      return DIBSTATUS_ERROR;
   }
   return DIBSTATUS_SUCCESS;
}

void DibBridgeCheckCmmbMultiplexFrame(struct DibBridgeContext *pContext, uint8_t *Mf, uint32_t Size, uint32_t ItemId)
{
   tMultiplexFrameHeader mfh;
   /* Read offset. */
   uint32_t RdOffset = 0;
   uint32_t Loop;
   uint32_t *CcFailCnt, *ErrCnt;
   uint16_t *PacketCc, Cc;
   uint16_t ExpectedCc;


   /* Sanity check. */
   if(Mf == 0)
      return;

   CcFailCnt = &(pContext->ItSvc[ItemId].CcFailCnt);
   ErrCnt    = &(pContext->ItSvc[ItemId].ErrCnt);
   PacketCc  = &(pContext->ItSvc[ItemId].CurCc);

   DIB_DEBUG(CHECKER_ERR, (CRB "-- MF CHECK svc %2d, %d bytes: " CRA, ItemId, Size));

   /* Check multiplex frame initial code and header length (5 bytes). */
   if((Mf[0] != 0x00) || (Mf[1] != 0x00) ||
      (Mf[2] != 0x00) || (Mf[3] != 0x01))
   {
      DIB_DEBUG(CHECKER_ERR, (CRB "-- ERROR: Wrong MF initial code (%02x%02x%02x%02x)" CRA, Mf[0], Mf[1], Mf[2], Mf[3]));
      (*ErrCnt)++;
      return;
   }

   /* Read multiplex frame header. */
   if(IntBridgeReadMultiplexFrameHeader(pContext, &mfh, Mf) == DIBSTATUS_SUCCESS)
   {
      /* IntBridgePrintMultiplexFrameHeader(pContext, &mfh); */
      RdOffset += mfh.Headerlen + 4;

      /* Display for debug. */
      DIB_DEBUG(CHECKER_LOG, (CRB "%02x %02x %02x %02x %02x %02x %02x %02x ... %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x" CRA,
               *(Mf + RdOffset + 0),
               *(Mf + RdOffset + 1),
               *(Mf + RdOffset + 2),
               *(Mf + RdOffset + 3),
               *(Mf + RdOffset + 4),
               *(Mf + RdOffset + 5),
               *(Mf + RdOffset + 6),
               *(Mf + RdOffset + 7),
               *(Mf + RdOffset + mfh.MsfLength[0] - 12),
               *(Mf + RdOffset + mfh.MsfLength[0] - 11),
               *(Mf + RdOffset + mfh.MsfLength[0] - 10),
               *(Mf + RdOffset + mfh.MsfLength[0] - 9),
               *(Mf + RdOffset + mfh.MsfLength[0] - 8),
               *(Mf + RdOffset + mfh.MsfLength[0] - 7),
               *(Mf + RdOffset + mfh.MsfLength[0] - 6),
               *(Mf + RdOffset + mfh.MsfLength[0] - 5),
               *(Mf + RdOffset + mfh.MsfLength[0] - 4),
               *(Mf + RdOffset + mfh.MsfLength[0] - 3),
               *(Mf + RdOffset + mfh.MsfLength[0] - 2),
               *(Mf + RdOffset + mfh.MsfLength[0] - 1)));

      /* Payload counters added for MF of mfid > 0 */
      if(mfh.MultiplexFrameId > 0)
      {
         if(mfh.MsfLength[0] >= 2)
         {
            Cc = *(Mf + RdOffset) + ((*(Mf + RdOffset + 1)) << 8);
            ExpectedCc = ((*PacketCc) + 1) % 60;
            if((*PacketCc != 0xFFFF) && (Cc != ExpectedCc))
            {
               DIB_DEBUG(CHECKER_ERR, (CRB  "-- ERROR: Mf dicountinuity %d, expected %d" CRA, Cc, *PacketCc+1));
               (*CcFailCnt)++;
               Loop = mfh.MsfLength[0] + 1;
            }
            *PacketCc = Cc;
         }

         /* Payload checker. */
         for(Loop = 2; Loop < mfh.MsfLength[0]; Loop++)
         {
            /* Counter payload. */
            if((*(Mf + RdOffset + Loop)) != ((uint8_t)((Loop - 2) & 0xff)))
            {
               DIB_DEBUG(CHECKER_ERR, (CRB  "-- ERROR: Payload counter mismatch at offset 0x%08x" CRA, Loop));
               DIB_DEBUG(CHECKER_ERR, (CRB  "... %02x %02x %02x %02x [%02x] %02x %02x %02x %02x ..." CRA,
                        *(Mf + RdOffset + Loop - 4),
                        *(Mf + RdOffset + Loop - 3),
                        *(Mf + RdOffset + Loop - 2),
                        *(Mf + RdOffset + Loop - 1),
                        *(Mf + RdOffset + Loop),
                        *(Mf + RdOffset + Loop + 1),
                        *(Mf + RdOffset + Loop + 2),
                        *(Mf + RdOffset + Loop + 3),
                        *(Mf + RdOffset + Loop + 4)));
               (*ErrCnt)++;
               break;
            }
         }
      }
   }
   else
   {
      (*ErrCnt)++;
   }
}
#endif /* DIB_CHECK_CMMB_DATA */
