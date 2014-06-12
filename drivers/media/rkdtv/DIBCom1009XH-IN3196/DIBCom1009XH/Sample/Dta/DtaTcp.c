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

#ifdef DTA_TCP
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "DibConfig.h"
#include "OsTypes.h"
#include "OsDebug.h"

#include "DibExtDefines.h"
#include "DibExtAPI.h"

#include "DtaTcp.h"
#include "DtaTcpCmds.h"

#include "DtaCmds.h"
#include "DtaScan.h"
//#include "../../Driver/Common/Debug/DibDriverDebugPlatform.h"
#define dbgp(args...) fprintf(stderr, args)
#define max(a,b) ((a)>(b)?(a):(b))

void DibDtaWriteRaw(struct DibDtaDebugPlatformInstance *instance, char *data, int size);

static const struct {
  const char *cmd;
  int (*handle)(struct DibDtaDebugPlatformInstance *,const char **,int );
} AllowedCmd[] = {
 
  { "IADC",       adc }, //sprintf(d,"%f",m.I_adc_power);
  { "QADC",       adc }, //sprintf(d,"%f",m.Q_adc_power);
  { "BER",        berPer }, //sprintf(d,"%f",m.ber);
  { "SNR",        snr }, //sprintf(d,"%f",m.CoN);
  { "MPEG",       mpeg }, //sprintf(d,"%d",m.locks.fec_mpeg);
  { "QUAD",       NULL }, //sprintf(d,"%f",m.iq_phase_mismatch);
  { "UQMIS",      NULL }, // sprintf(d,"%f",m.iq_gain_mismatch);
  { "PACKET",     NULL }, // if (m.CoN>0) sprintf(d,"%f", (double) m.PacketErrorCount); else sprintf(d,"%f", 1000.); //Force the packet error to be high
  { "CARR",       NULL }, // sprintf(d,"%f", (double) m.locks.fec_frm);
  { "CHAN",       NULL }, //Tune other Channel {   cd = m_Fe->current_adapter->current_channel; cd->RF_kHz=atoi(Par1); tuner_tune(m_Fe->demod,cd); sprintf(d,"OK");
  { "SCAN",       getChannel },
  { "CHIP",       chipId },
  { "STREAM",     streamId },
  { "FRONTEND",   getFrontend},
  { "TUNE",       tuneFrontend},
};

void DibDtaWriteRaw(struct DibDtaDebugPlatformInstance *instance, char *data, int size) {
  write(instance->sock, data, size);
}
/* this is the 'SocketHandler' */
int DibDtaParser(struct DibDtaDebugPlatformInstance *instance, char *b, uint32_t size)
{
  const char *Par0, *Par1, *Par2, *Par3;
  char *pos;
  int i,nPar;
  
  const char *Par[10];
  memset(Par,'\0',10*sizeof(*Par));
  Par[0]=strtok(b," ");
  for (nPar =1;nPar<10;nPar++) {
    Par[nPar]=strtok(NULL," ");
    if (Par[nPar]=='\0')
      break;
  }
  nPar--;
  for (i = 0; i < sizeof(AllowedCmd)/sizeof(AllowedCmd[0]); i++) {
      if (strcasecmp(AllowedCmd[i].cmd, Par[0]) == 0) {
          if (AllowedCmd[i].handle)
            AllowedCmd[i].handle(instance,Par,nPar);
          else 
            dbgp("unhandled command received: '%s'\n", Par[0]);

          break;
        }
      }

  if (i == sizeof(AllowedCmd)/sizeof(AllowedCmd[0])) {
    dbgp("unknown command received: '%s' \n", Par[0]);
    DibDtaWriteRaw(instance, "NOK", 3);
  }
  
  return 1;
}

static void * DtaTcpConnection(void *arg)
{
  struct DibDtaDebugPlatformInstance *instance = arg;
  struct DibDriverContext *t=instance->parentCtx->DibDrvCtx;
  char b[1024];
  
  while (!instance->Stop) {
    fd_set rfds;
    int retval;
 
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;
    
    FD_ZERO(&rfds);
    FD_SET(instance->sock, &rfds);
    
    retval = select(instance->sock+1, &rfds, NULL, NULL, &tv);
    switch (retval) {
      case -1:
        perror("select failed");
        instance->Stop = 1;
        break;
      case 0: /* timeout */
        break;
      default: { /* data to be read */
        
        int len;
        len = read(instance->sock, b, 1023); /* read the header */
        if (len <= 0) { /* connection closed */
          dbgp("len is not valid (%d) - connection closed\n", len);
          goto bailout;
        }
        b[len] = '\0';
        DibDtaParser(instance,b,len);
        
        break;
bailout:
        instance->Stop = 1;
        
        break;
        
      }
    }
  }
  instance->IsClosing = 1;
  
  shutdown(instance->sock, SHUT_RDWR);
  instance->Stop = 0;
  
  dbgp("%s: DebugPlatformConnectionThread stopped (%d).\n", __func__,0);
  
//  DibDriverDebugReleaseInstance(instance);
  
  return NULL;
}

static void DtaSocketAccept(struct DibDtaDebugPlatformContext *DtaTcpCtx, int fd,
                                         void * (*handler)(void*)
                                         )
{
  int ConnectFD = accept(fd, NULL, NULL);
  struct DibDtaDebugPlatformInstance *instance;
  if(ConnectFD < 0) {
    perror("error accept failed");
    return;
  }
  instance = &DtaTcpCtx->DtaTcpInstance;
  instance->parentCtx = DtaTcpCtx; //Fill the Parent
  if (instance == NULL) {
    shutdown(ConnectFD, SHUT_RDWR);
    dbgp("%s: ERROR: could not allocate instance for connection\n", __func__);
  } else {
    dbgp("%s: starting Connection for DebugInstance\n", __func__);
    
    instance->sock = ConnectFD;
    instance->IsClosing = 0;
    //instance->msgHandler = msg;
    
     pthread_mutex_init(&instance->msgBufferLock, NULL);
     pthread_cond_init(&instance->msgResponseCondition, NULL);
     pthread_create(&instance->Thread, NULL, handler, instance);
  }
}

/* Creates the server and listens for incoming connections */
static void * TcpServer1(void *arg)
{
  int i;
  struct DibDtaDebugPlatformContext *DtaTcpCtx=arg;

  dbgp("%s: DtaPlatformThread running.\n", __func__);
  
  /* wait for Board to ready */

  while (!DtaTcpCtx->Stop) {
    fd_set rfds;
    int retval;
    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;
    FD_ZERO(&rfds);
    FD_SET(DtaTcpCtx->sock, &rfds);
    retval = select(max(DtaTcpCtx->sock,DtaTcpCtx->sock)+1, &rfds, NULL, NULL, &tv);
    switch (retval) {
      case -1:
	perror("select failed");
        DtaTcpCtx->Stop = 1;
	break;
      case 0: /* timeout */
	break;
      default: 
        if (FD_ISSET(DtaTcpCtx->sock, &rfds)) 
          DtaSocketAccept(DtaTcpCtx, DtaTcpCtx->sock,DtaTcpConnection);
    }
  DtaTcpCtx->Stop = 0;  
  }
  return NULL;
}

void DtaTcpInit(struct DibDtaDebugPlatformContext *DtaTcpCtx,struct DibDriverContext *ctx)
{   struct sockaddr_in stSockAddr;
    int reuse_addr = 1;
    
    DtaTcpCtx->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    setsockopt(DtaTcpCtx->sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    memset(&stSockAddr, 0, sizeof stSockAddr);
    
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(8001);  /* PORT to be determined by running instance */
    stSockAddr.sin_addr.s_addr = INADDR_ANY; 
    
    if(-1 == bind(DtaTcpCtx->sock, (struct sockaddr *)&stSockAddr, sizeof stSockAddr)) {
      perror("error bind failed DtaTcp");
      goto bind_error;
    }
    
    if(-1 == listen(DtaTcpCtx->sock, 10)) {
      perror("error listen failed");
      goto listen_error;
    }
    pthread_create(&DtaTcpCtx->Thread, NULL, TcpServer1, DtaTcpCtx);
    DtaTcpCtx->DibDrvCtx=ctx;
    goto out;
    
listen_error:
bind_error:
    close(DtaTcpCtx->sock);
out:
     return;
  
}
#endif
