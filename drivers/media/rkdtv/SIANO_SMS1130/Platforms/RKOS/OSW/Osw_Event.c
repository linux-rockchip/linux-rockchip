#include "../../../include_internal/Include/sms_common.h"
#if 0
UINT32 OSW_EventCreate
	(
	Event*		pEvent			// where to store the event 
    )
    {

#if 0 // Peter
	SMS_ASSERT (pEvent);

	*pEvent = CreateEvent
		(
		NULL,			// Security settings
		0,				// ManualReset - false - system resets the state after a single waiter was signaled
		0,				// InitialState (signaled or not)
		(LPCTSTR)NULL	// Event name
		);

	return (*pEvent == NULL) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif
    } /* OSW_EventCreate */


UINT32 OSW_EventDelete
	(
	Event*		pEvent
	)
	{
#if 0 // peter	
	BOOL	rc;

	SMS_ASSERT (pEvent);

	rc = CloseHandle (*pEvent);
	return (rc == 0) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif

	} /* OSW_EventDelete */
	

UINT32 OSW_EventSet
	(
	Event*		pEvent
	)
	{
#if 0 // Peter	
	BOOL	rc;

	SMS_ASSERT (pEvent);

	rc = SetEvent (*pEvent);
	return (rc == 0) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif
	} /* OSW_EventSet */


UINT32 OSW_EventClear
	(
	Event*		pEvent
	)
	{
#if 0 // peter	
	BOOL	rc;

	SMS_ASSERT (pEvent);

	rc = ResetEvent (*pEvent);
	return (rc == 0) ? OSW_ERROR : OSW_OK;
#else
	return OSW_OK;
#endif
	} /* OSW_EventClear */


UINT32 OSW_EventWait
	(
	Event*		pEvent,
	UINT32		timeout
	)
	{
#if 0 // peter	
	DWORD		rc;

	SMS_ASSERT (pEvent);

	timeout = (timeout == OSW_WAIT_FOREVER) ? INFINITE : timeout;

	rc = WaitForSingleObject (*pEvent, timeout);

	if (rc == WAIT_FAILED)
		{
		return OSW_ERROR;
		}

	else if (rc == WAIT_TIMEOUT)
		{
		return OSW_TIMEOUT;
		}
	
	return OSW_OK;
#else
	return OSW_OK;
#endif
	} /* OSW_EventWait */
#endif //jan
