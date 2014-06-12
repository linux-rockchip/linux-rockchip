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

#if DIB_INTERNAL_MONITORING == 1


#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "DibMsgDebug.h"

#include "DtaDebug.h"


static int              dbgsockfd;
static struct SerialBuf dbgserialbuf;
static uint32_t         dbgbuf[128];

/// Messages packing / unpacking

static uint32_t freezePack(struct SerialBuf *dbgserialbuf,
                           int8_t            ChipId,
                           uint8_t           Enable,
                           uint16_t          AntennaId)
{
   struct MsgTunerAgcFreeze msg = {
      .Head = {
         .MsgSize = GetWords(MsgTunerAgcFreezeBits, 32),
         .ChipId  = ChipId,
         .MsgId   = OUT_MSG_TUNER_AGC_FREEZE,
         .Sender  = 0,
         .Type    = MSG_TYPE_DEBUG,
      },
      .Enable    = Enable,
      .AntennaId = AntennaId,
   };

   MsgTunerAgcFreezePackInit(&msg, dbgserialbuf);

   return msg.Head.MsgSize*4;
}

static uint32_t acquiPack(struct SerialBuf     *dbgserialbuf,
                          int8_t                ChipId,
                          uint8_t               Enable,
                          enum AcquisitionType  Type,
                          uint8_t               Passthru)
{
   struct MsgAcquisitionModeCtrl msg = {
      .Head = {
         .MsgSize = GetWords(MsgAcquisitionModeCtrlBits, 32),
         .ChipId  = ChipId,
         .MsgId   = OUT_MSG_ACQUISITION_MODE_CTRL,
         .Sender  = 0,
         .Type    = MSG_TYPE_DEBUG,
      },
      .Enable    = Enable,
      .Type      = Type,
      .Passthru  = Passthru,
   };

   MsgAcquisitionModeCtrlPackInit(&msg, dbgserialbuf);

   return msg.Head.MsgSize*4;
}


static uint32_t setClockPack(struct SerialBuf               *dbgserialbuf,
                             int8_t                          ChipId,
                             enum ClockFrequencyType         Type,
                             union ClockFrequencyParameters *Param)
{
   struct MsgSetClockFrequency msg = {
      .Head = {
         .MsgSize = GetWords(MsgSetClockFrequencyBits, 32),
         .ChipId  = ChipId,
         .MsgId   = OUT_MSG_SET_CLOCK_FREQUENCY,
         .Sender  = 0,
         .Type    = MSG_TYPE_DEBUG,
      },
      .Type      = Type,
   };

   memcpy(&msg.Param, Param, sizeof (Param));

   MsgSetClockFrequencyPackInit(&msg, dbgserialbuf);

   return msg.Head.MsgSize*4;
}

static uint8_t AckUnpack(struct SerialBuf *dbgserialbuf)
{
   struct MsgDebugAcknowledge ack;

   MsgDebugAcknowledgeUnpackInit(dbgserialbuf, &ack);

   return ack.idMsgOrigin;
}

/// Buffer and socket management

static void initTxBuffer(struct SerialBuf *dbgserialbuf, uint32_t *dbgbuf)
{
   SerialBufInit(dbgserialbuf, dbgbuf, 32);
   memset(dbgbuf, 0, sizeof(dbgbuf));
}

static int initTxSocket(int32_t BoardId)
{
   int                 dbgsockfd;
   int                 portno;
   struct sockaddr_in  serv_addr;
   struct hostent     *server;

   portno = 25000; // TCP debug port

   dbgsockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (dbgsockfd < 0)
      perror("ERROR opening socket");

   server = gethostbyname("localhost");
   if (server == NULL)
   {
      fprintf(stderr,"ERROR, no such host");
      return 0;
   }

   memset((char *) &serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
   serv_addr.sin_port = htons(portno + 10 * BoardId);

   if (connect(dbgsockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      perror("ERROR connecting socket");

   return dbgsockfd;
}

static int writeTxSocket(uint32_t size)
{
   int n;

   n = write(dbgsockfd, dbgserialbuf.buf, size);
   if (n < 0)
      perror("ERROR writing to socket");

   return n;
}

static uint8_t readTxSocketAck()
{
   int      n;
   uint32_t size;
   uint8_t  msgId = MAX_MSG_DEBUG_OUT;

   size = GetWords(MsgDebugAcknowledgeBits, 32)*4;

   n = read(dbgsockfd, dbgserialbuf.buf, size);
   if (n < 0)
   {
      perror("ERROR reading to socket");
      return -1;
   }
   else
   {
      msgId = AckUnpack(&dbgserialbuf);
      printf("Ack received for msg %u\n", msgId);
   }

   return msgId;
}

/// Init and cLose : socket and dbgbuffer

int DtaDebugInit(int32_t BoardId)
{
   initTxBuffer(&dbgserialbuf, dbgbuf);
   dbgsockfd = initTxSocket(BoardId);

   return 0;
}

void DtaDebugClose(void)
{
   close(dbgsockfd);
}

/// Tests

void dbg_acg_freeze(int8_t ChipId, uint8_t Enable, uint16_t AntennaId)
{
   uint32_t size;

   size = freezePack(&dbgserialbuf, ChipId, Enable, AntennaId);

   if (writeTxSocket(size) > 0)
      readTxSocketAck();
}

void dbg_acqui(int8_t ChipId, uint8_t Enable, uint8_t Type)
{
   uint32_t size;
   uint8_t  Passthru;
   uint8_t  index;

   for (index = 0; index <= ChipId; index ++)
   {
      Passthru = (index == ChipId) ? 0 : 1;
      size = acquiPack(&dbgserialbuf, index, Enable, Type, Passthru);

      if (writeTxSocket(size) > 0)
         readTxSocketAck();
   }
}

void dbg_set_clock(int8_t ChipId, uint8_t Type, union ClockFrequency *Param)
{
   uint32_t size;
   union ClockFrequencyParameters MsgParam;

   switch(Type)
   {
   case CLOCK_FREQUENCY_OCTOPUS:
      MsgParam.Octopus.OctoClk = Param->Octopus.OctoClk;
      break;

   case CLOCK_FREQUENCY_REFLEX:
      MsgParam.Reflex.FemtoClk = Param->Octopus.OctoClk;
      MsgParam.Reflex.CceClk = Param->Octopus.OctoClk;
      break;
   }

   size = setClockPack(&dbgserialbuf, ChipId, Type, &MsgParam);

   if (writeTxSocket(size) > 0)
      readTxSocketAck();
}

#endif
