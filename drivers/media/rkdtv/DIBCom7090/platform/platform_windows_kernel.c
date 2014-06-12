#include "platform/platform_windows_kernel.h"
#include "adapter/common.h"
#include <adapter/debug.h>


uint32_t systime(void)
{
	LARGE_INTEGER time;
	uint32_t timeIncrement100ns;
	uint32_t time0_1ms;
	uint32_t result;

	timeIncrement100ns = KeQueryTimeIncrement ();
	time0_1ms = timeIncrement100ns / 1000;
	KeQueryTickCount(&time);
	result = time.LowPart * time0_1ms;

	return (result); // the unit must be 0.1ms
}

uint8_t DibWaitForEvent(DIB_EVENT *event, int32_t timeout_milli)
{
	LARGE_INTEGER	timeout;
	PLARGE_INTEGER timeout_pointer;
	NTSTATUS		   status;

	if( timeout_milli == -1 )
		timeout_pointer = NULL;
	else {
		timeout.QuadPart = -1000 * 10 * timeout_milli;
		timeout_pointer = &timeout;
	}

	status = KeWaitForSingleObject(  event,
										Executive,
										KernelMode ,
										FALSE ,
										timeout_pointer );
	if( status == STATUS_SUCCESS )
		return DIB_RETURN_SUCCESS;

	if( status == STATUS_TIMEOUT )
		return DIB_RETURN_TIMEOUT;

	/*  otherwise */
	return DIB_RETURN_ERROR;
}

void DibMSleep(uint32_t delay_ms)
{
   NTSTATUS        ntStatus;
   LARGE_INTEGER	Delay;

   delay_ms = (delay_ms >= 10 ? delay_ms : 10);

   Delay.QuadPart = (INT)delay_ms * -10000;
   ntStatus = KeDelayExecutionThread(KernelMode, FALSE, &Delay);
}

int platform_request_firmware(const char *name,
                              int options,
                              int size,
                              int * off,
                              int (*platform_firmware_indication)(void * ctx, uint8_t * buf, uint32_t len, uint32_t off, uint32_t done),
                              void *ctx)
{
	int							fd=0;
	NTSTATUS						ntStatus=STATUS_SUCCESS;
	HANDLE						fileHandle;
	OBJECT_ATTRIBUTES			objectAttributes;
	UNICODE_STRING          fileName;
	IO_STATUS_BLOCK			ioStatus;

	return DIB_RETURN_ERROR;

#if 0
	dbgp("'%s' firmware requested\n", name);

	RtlInitUnicodeString ( &fileName, (PCWSTR) CONFIG_FIRMWARE_PATH );
	RtlAppendUnicodeToString( &fileName, (PCWSTR)name);

	InitializeObjectAttributes(	&objectAttributes,
											&fileName,
											OBJ_KERNEL_HANDLE,
											(HANDLE) NULL,
											(PSECURITY_DESCRIPTOR) NULL );

	ntStatus = ZwOpenFile(	&fileHandle,
									STANDARD_RIGHTS_ALL,
									&objectAttributes,
									&ioStatus,
									0,
									FILE_NON_DIRECTORY_FILE );

	fw->buffer = NULL;
	fw->length = 0;

	if (ntStatus != STATUS_SUCCESS) {
	  dbgp("could not open '%s'\n", fileName);
	  return DIB_RETURN_ERROR;
	}

	fw->buffer = ExAllocatePoolWithTag( NonPagedPool, 300000, 'cBiD');

	ntStatus = ZwReadFile(	fileHandle,
									NULL, NULL, NULL,
									&ioStatus,
									fw->buffer,
									300000,
									NULL, NULL);

	fw->length = ioStatus.Information;

	ZwClose( fileHandle);

	return DIB_RETURN_SUCCESS;
#endif
}



#if DBG
void debug_printf(const char *fmt, ...)
{

   char buffer[256];
   va_list ap;

   va_start(ap, fmt);
   _vsnprintf(buffer,sizeof(buffer), fmt, ap);
   
   KdPrint((buffer));
   va_end(ap);
}
#endif

#if DBG
void debug_printf_line(const struct dibDebugObject *dbg, const char *fmt, ...)
{

   va_list ap;
   char buffer[256];

   if (dbg)
      _snprintf(buffer,sizeof(buffer), "%-12s: ", dbg->prefix);
   else
      _snprintf(buffer,sizeof(buffer), "%-12s: ", "DBG");

   va_start(ap, fmt);
   _vsnprintf(buffer+14, sizeof(buffer)-14, fmt, ap);
   va_end(ap);

   strncat(buffer, "\n", 2*sizeof(UCHAR));

   KdPrint((buffer));
 
}
#endif  
