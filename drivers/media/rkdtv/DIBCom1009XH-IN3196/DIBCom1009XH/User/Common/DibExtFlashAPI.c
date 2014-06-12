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
* @file "DibExtAPI.c"
* @brief SDK Internal External Api functions.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (EMBEDDED_FLASH_SUPPORT == 1)
#include "DibBoardFlRdSelection.h"
#include "DibBoardFlWrSelection.h"
#include "DibDriver.h"
#include "DibDriverMessages.h"
#include "DibBoardParameters.h"

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
#include <sys/time.h>
#include <time.h>

FILE    *gLogFile              = NULL;
uint32_t gLogFileStartTimeSec  = 0;
uint32_t gLogFileStartTimeMsec = 0;
#endif

/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibOpenFromFlash(struct DibDriverContext **pContextAddr, enum DibBoardType BoardType, BOARD_HDL BoardHdl)
{
   DIBSTATUS status = DIBSTATUS_ERROR;

   struct DibDriverContext *pContext = (struct DibDriverContext *)DibMemAlloc(sizeof(struct DibDriverContext));

   if(pContext == NULL)
      return DIBSTATUS_ERROR;

   memset(pContext, 0, sizeof(struct DibDriverContext));

   DibAllocateLock(&pContext->UserLock);
   DibInitLock(&pContext->UserLock);
   DibAcquireLock(&pContext->UserLock);

   pContext->Hd.BoardConfig = BoardFlrdList[BoardType];
   if (pContext->Hd.BoardConfig == NULL)
      status = DIBSTATUS_ERROR;
   else
      status = DibDriverInit(pContext, BoardType, BoardHdl);

   if(status == DIBSTATUS_SUCCESS)
   {
      *pContextAddr   = pContext;
      DibReleaseLock(&pContext->UserLock);
   }
   else
   {
   /* Do not set to null even if we should sice client may use twice a dibopen using the same context
      we do not check if the pointer was set to a value as the client may not have initialised it's variable */

   /* *pContextAddr = NULL; */
       DibDeAllocateLock(&pContext->UserLock);
       DibMemFree(pContext, sizeof(struct DibDriverContext));
   }
   return status;
}

#if (WRITE_FLASH_SUPPORT != eWRFL_NONE)

extern struct DibDemodBoardConfig *BoardFlwrList[eMAX_NB_BOARDS] ;
/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibOpenWriteFlash(struct DibDriverContext **pContextAddr, enum DibBoardType BoardType, BOARD_HDL BoardHdl)
{
   DIBSTATUS status = DIBSTATUS_ERROR;

   printf("DibOpenWriteFlash BoardType %d \n",BoardType);

   struct DibDriverContext *pContext = (struct DibDriverContext *)DibMemAlloc(sizeof(struct DibDriverContext));

   if(pContext == NULL)
      return DIBSTATUS_ERROR;

   memset(pContext, 0, sizeof(struct DibDriverContext));

   DibAllocateLock(&pContext->UserLock);
   DibInitLock(&pContext->UserLock);
   DibAcquireLock(&pContext->UserLock);

   pContext->Hd.BoardConfig = BoardFlwrList[BoardType];
   if (pContext->Hd.BoardConfig == NULL)
      status = DIBSTATUS_ERROR;
   else
   {
      status = DibDriverInit(pContext, BoardType, BoardHdl);
   }

   if(status == DIBSTATUS_SUCCESS)
   {
      *pContextAddr   = pContext;
      DibReleaseLock(&pContext->UserLock);
   }
   else
   {
   /* Do not set to null even if we should sice client may use twice a dibopen using the same context
      we do not check if the pointer was set to a value as the client may not have initialised it's variable */

   /* *pContextAddr = NULL; */
       DibDeAllocateLock(&pContext->UserLock);
       DibMemFree(pContext, sizeof(struct DibDriverContext));
   }
   return status;
}

/***************************************************************************
* program flash
****************************************************************************/
DIBSTATUS DibProgramFlash(struct DibDriverContext *pContext, char* filename)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = DibDriverProgramFlash(pContext, filename);
   DibReleaseLock(&pContext->UserLock);

   return Status;
}
/***************************************************************************
* program flash
****************************************************************************/
DIBSTATUS DibProgramFlashStatus(struct DibDriverContext *pContext)
{
   DIBSTATUS Status = DIBSTATUS_ERROR;

   DIB_ASSERT(pContext);

   DibAcquireLock(&pContext->UserLock);
   Status = pContext->MsgAckStatus;
   DibReleaseLock(&pContext->UserLock);

   return Status;
}


#endif /* (WRITE_FLASH_SUPPORT != eWRFL_NONE) */

#endif
