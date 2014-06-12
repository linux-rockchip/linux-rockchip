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
* @file "DtaTime.h"
* @brief DiBcom Test Application - Timing function prototypes.
*
***************************************************************************************************/
#ifndef _DTA_TIME_H_
#define _DTA_TIME_H_

#ifndef timercpy
#define timercpy(tvd, tvs)             \
  do                                   \
   {                                   \
      (tvd)->tv_sec = (tvs)->tv_sec;   \
      (tvd)->tv_usec = (tvs)->tv_usec; \
   } while(0)
#endif /* timercpy */

#ifndef timermsec
#define timermsec(tv)   ((tv)->tv_sec*1000+(tv)->tv_usec/1000)
#endif /* timermsec */

#ifndef timerusec
#define timerusec(tv)   ((tv)->tv_sec*1000000+(tv)->tv_usec)
#endif /* timerusec */

#ifndef timerisset
#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#endif /* timerisset */

#ifndef timercmp
#define timercmp(tvp, uvp, cmp) ((tvp)->tv_sec cmp (uvp)->tv_sec ||  \
                                 (tvp)->tv_sec == (uvp)->tv_sec  &&  \
                                 (tvp)->tv_usec cmp (uvp)->tv_usec)
#endif /* timercmp */

#ifndef timerclear
#define timerclear(tvp)   ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#endif /* timerclear */

#ifndef timersub
#define timersub(a, b, result)                        \
   do                                                 \
   {                                                  \
      (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;   \
      (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;\
      if ((result)->tv_usec < 0)                      \
      {                                               \
         --(result)->tv_sec;                          \
         (result)->tv_usec += 1000000;                \
      }                                               \
   } while (0)
#endif /* timersub */

void    DtaTimeLogInit(void);
void    DtaRealTimeLogInit(void);
void    DtaRealTimeLogExit(void);
void    DtaRealTimeSet(void);
void    DtaRealTimeClear(void);
int32_t DtaRealTimeDiffMs(void);
void    DtaRealTimeLogDiff(const char * string);
void    DtaTimeLogDate(FILE * fdlog, const char * string);

#endif
