#if 0
#include "osw.h"

UINT32 OSW_MutexCreate
	(
	Mutex*					pMutex
	)
	{
#if 0 // peter	
	SMS_ASSERT (pMutex);

	*pMutex = CreateMutex
		(
		NULL,			// Security attributes
		FALSE,			// Not initial owner
		(LPCTSTR)NULL	// Mutex name
		);

	return (*pMutex == NULL) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif
	} /* OSW_MutexCreate */


UINT32 OSW_MutexDelete
	(
	Mutex*					pMutex
	)
	{
#if 0 // peter	
	BOOL	rc;
	
	SMS_ASSERT (pMutex);

	rc = CloseHandle (*pMutex);
	return (rc == 0) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif
	} /* OSW_MutexDelete */


UINT32 OSW_MutexGet
	(
	Mutex*					pMutex
	)
	{
#if 0 // peter	
	DWORD		rc;
	
	SMS_ASSERT (pMutex);

	rc = WaitForSingleObject (*pMutex, INFINITE);
	return (rc == WAIT_FAILED) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif
	} /* OSW_MutexGet */


UINT32 OSW_MutexPut
	(
	Mutex*					pMutex
	)
	{
#if 0 // peter	
	BOOL	rc;

	SMS_ASSERT (pMutex);

	rc = ReleaseMutex (*pMutex);
	return (rc == 0) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif	
	} /* OSW_MutexPut */

#endif //jan
