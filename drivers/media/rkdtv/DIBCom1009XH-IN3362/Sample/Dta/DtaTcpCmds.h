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

 #ifndef __DTATCPCMDS__
 #define __DTATCPCMDS__

 int snr(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int getChannel(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int adc(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int berPer(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int mpeg(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int chipId(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int streamId(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int getFrontend(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 int tuneFrontend(struct DibDtaDebugPlatformInstance *i, const char **Par,int nbPar);
 #endif
