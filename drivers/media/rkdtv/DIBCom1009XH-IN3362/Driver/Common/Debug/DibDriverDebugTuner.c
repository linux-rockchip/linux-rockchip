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

#include "DibDriverCommon.h"
#include "DibDriverMessages.h"

#include "DibMsgDebug.h"
/*
#include "../../../Sample/Dta/DtaCmds.h"
#include "../../../Sample/Dta/DtaScan.h"

struct DibDriverContext * gUserContext;

#define CONTEXT (gUserContext)

#define CHIP_ID 0
*/
#define dbgp(args...) fprintf(stderr, args)


static void waitForMessage(struct DibDriverDebugInstance *i, int id)
{
   i->Platform.expectedId = id;
   pthread_cond_wait(&i->Platform.msgResponseCondition, &i->Platform.msgBufferLock);
}

static int regAccess(struct DibDriverDebugInstance *i, uint32_t address, uint32_t value, uint32_t bits, uint8_t rw)
{
   char result[25];
   struct MsgRegisterAccess access = {
      .Head = {
         .MsgSize = GetWords(MsgRegisterAccessBits, 32),
         .ChipId  = i->Platform.chipId,
         .MsgId   = OUT_MSG_REGISTER_ACCESS,
         .Sender  = 0,
         .Type    = MSG_TYPE_DEBUG,
      },

      .ReadWrite  = rw,
      .Address    = address,
      .Offset     = 0,
      .Bits       = bits,
      .AccessType = bits,
      .Msb        = 0,
      .Lsb        = value,
   };

   pthread_mutex_lock(&i->Platform.msgBufferLock);
   MsgRegisterAccessPackInit(&access, &i->Platform.SerialBuf);
   DibDriverDebugOutMessageCollector(i, i->Platform.Buffer, &access.Head);

   waitForMessage(i, IN_MSG_REGISTER_ACCESS_ACK);

   struct MsgRegisterAccessAck ack;
   MsgRegisterAccessAckUnpackInit(&i->Platform.SerialBuf, &ack);
   if (rw)
      snprintf(result, sizeof(result), "OK");
   else
      snprintf(result, sizeof(result), "%u", ack.Lsb);


   pthread_mutex_unlock(&i->Platform.msgBufferLock);

   DibDriverDebugPlatformInstanceWriteRaw(i, result, strlen(result));
   return 1;
}

static int forceWbd(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{    
    int bus     = 2;
    int address = 0;
    int value   = atoi(p1);
    
    dbgp("The Wbd will be forced to %d\n",value);
    return regAccess(i, 0x10000000 + (bus * 500) + address, value, 32, 1);
}
static int forceSplit(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{    
    int bus     = 2;
    int address = 1;
    int value   = atoi(p1);
    
    dbgp("The RFsplit will be forced at %d\n",value);
    return regAccess(i, 0x10000000 + (bus * 500) + address, value, 32, 1);
}

static int freeze(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{
  char result[25];
  
  uint8_t enable=atoi(p1);
  uint8_t antennaId=atoi(p2);
  dbgp("Freeze %s\n", (enable==1)?"On":"Off");
  struct MsgTunerAgcFreeze access = {
    .Head = {
      .MsgSize = GetWords(MsgTunerAgcFreezeBits, 32),
      .ChipId  = i->Platform.chipId,
      .MsgId   = OUT_MSG_TUNER_AGC_FREEZE,
      .Sender  = 0,
      .Type    = MSG_TYPE_DEBUG,
    },
    .Enable    = enable,
    .AntennaId = antennaId,
  };
  
  pthread_mutex_lock(&i->Platform.msgBufferLock);
  MsgTunerAgcFreezePackInit(&access, &i->Platform.SerialBuf);
  DibDriverDebugOutMessageCollector(i, i->Platform.Buffer, &access.Head);

  waitForMessage(i, IN_MSG_DEBUG_ACK_INDICATION);

  struct MsgRegisterAccessAck ack;
  MsgRegisterAccessAckUnpackInit(&i->Platform.SerialBuf, &ack);
  
  snprintf(result, sizeof(result), "OK");

  
  
  pthread_mutex_unlock(&i->Platform.msgBufferLock);
  
  DibDriverDebugPlatformInstanceWriteRaw(i, result, strlen(result));
  return 1;
}

static int readI2C(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{
   int address = strtol(p1, NULL, 10);
   return regAccess(i, 0x10000000 + address, 0, 16, 0);
}

static int writeI2C(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{
   int address = strtol(p1, NULL, 10);
   int value   = strtol(p2, NULL, 10);
   return regAccess(i, 0x10000000 + address, value, 16, 1);
}
static int forceWbd16(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{
    int value   = strtol(p1, NULL, 10);
    return regAccess(i, 0x10000000 + 266, value, 16, 1);
}

static int read65Nm(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{
   int bus   = strtol(p1, NULL, 10);
   int address = strtol(p2, NULL, 10);
   return regAccess(i, 0x10000000 + (bus * 500) + address , 0, 32, 0);
}

static int write65Nm(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{
   int bus     = strtol(p1, NULL, 10);
   int address = strtol(p2, NULL, 10);
   int value   = strtol(p3, NULL, 10);
   return regAccess(i, 0x10000000 + (bus * 500) + address, value, 32, 1);
}

static int chipId(struct DibDriverDebugInstance *i, const char *p1, const char *p2, const char *p3)
{
    i->Platform.chipId = atoi(p1);
  dbgp("chip monitored is %d\n", i->Platform.chipId);
   return 1;
}


static const struct {
   const char *cmd;
   int (*handle)(struct DibDriverDebugInstance *, const char *, const char *, const char *);
} command[] = {
   { "READI2C",    readI2C },
   { "WRITEI2C",   writeI2C },
   { "READ_65NM",  read65Nm },
   { "WRITE_65NM", write65Nm },
   { "WRITE0WIR",  NULL },
   { "CHIP",       chipId},
   { "FREEZE",     freeze},
    { "WBDCTRL", forceWbd},
    { "WBDCTRL16", forceWbd16},
    { "RFSPLIT", forceSplit},
};

/* this is the 'SocketHandler' */
int DibDriverDebugTunerEmulatorAccess(struct DibDriverDebugInstance *instance, char *b, uint32_t size)
{
   const char *Par0, *Par1, *Par2, *Par3;
   char *pos;
   int i;

   dbgp("received '%s' in %d chars: ", b, size);
	Par0=strtok_r(b," ", &pos);
	Par1=strtok_r(NULL," ", &pos);
	Par2=strtok_r(NULL," ", &pos);
	Par3=strtok_r(NULL," ", &pos);

   dbgp("after parsing: '%s' '%s' '%s' '%s'\n", Par0, Par1, Par2, Par3);
   for (i = 0; i < sizeof(command)/sizeof(command[0]); i++) {
      if (strcasecmp(command[i].cmd, Par0) == 0) {
         if (command[i].handle)
            command[i].handle(instance, Par1, Par2, Par3);
         else
            dbgp("unhandled command received: '%s' (with '%s' and '%s' as parameter)\n", Par0, Par1, Par2);

         break;
      }
   }
   if (i == sizeof(command)/sizeof(command[0])) {
      dbgp("unknown command received: '%s' (with '%s' and '%s' as parameter)\n", Par0, Par1, Par2);
      DibDriverDebugPlatformInstanceWriteRaw(instance, "NOK", 3);
   }

   return 1;
}

void DibDriverDebugTunerEmulatorAccessMsgResponse(struct DibDriverDebugInstance *i, const uint32_t *data, uint32_t size)
{
   struct MsgHeader head;

   pthread_mutex_lock(&i->Platform.msgBufferLock);

   memcpy(i->Platform.Buffer, data, size * 4);
   MsgHeaderUnpackInit(&i->Platform.SerialBuf, &head);

   if (head.MsgId == i->Platform.expectedId)
      pthread_cond_signal(&i->Platform.msgResponseCondition);
   else
      dbgp("unexpected message has arrived (ID: %d) -> dropped\n", head.MsgId);

   pthread_mutex_unlock(&i->Platform.msgBufferLock);
}
