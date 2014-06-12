#include "platform/platform_windows_user.h"
#include "adapter/common.h"
#include <adapter/debug.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>

uint32_t systime()
{
	return GetTickCount() * 10;
/*	FILETIME t;
	GetSystemTimeAsFileTime(&t);
	debug_printf("%u, %u %u\n", t.dwHighDateTime, t.dwLowDateTime, GetTickCount());
	return 0;*/
}

uint8_t DibWaitForEvent(DIB_EVENT *event, int32_t timeout_milli)
{
	/*LARGE_INTEGER	timeout;
	PLARGE_INTEGER timeout_pointer;
	NTSTATUS		   status;

	if( timeout_milli == -1 )
		timeout_pointer = NULL;
	else {
		timeout.QuadPart = -1000 * 10 * timeout_milli;
		timeout_pointer = &timeout;
	}

	/*status = KeWaitForSingleObject(  event,
										Executive,
										KernelMode ,
										FALSE ,
										timeout_pointer );
	*//*if( status == STATUS_SUCCESS )
		return DIB_RETURN_SUCCESS;

	if( status == STATUS_TIMEOUT )
		return DIB_RETURN_TIMEOUT;

	/*  otherwise */
	return DIB_RETURN_ERROR;
}


void DibMSleep(uint32_t delay_ms)
{
	Sleep(delay_ms + 15); //add 15 ms to the sleep time to be sure that we wait the right time under windows XP( see msdn doc at : http://msdn.microsoft.com/en-us/library/ms686298(VS.85).aspx )
}

void DibUSleep(uint32_t delay_usec)
{
	DibMSleep(delay_usec / 1000);
}

#ifdef CONFIG_DEBUG
void debug_printf(const char *fmt, ...)
{
    char buffer[256];
	va_list ap;

	va_start(ap, fmt);
    _vsnprintf(buffer,sizeof(buffer), fmt, ap);
    OutputDebugString((buffer));
	va_end(ap);
}


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
    strcat(buffer, "\n");
    OutputDebugString((buffer));
}
#else
void debug_printf(const char *fmt, ...) {}
void debug_printf_line(const struct dibDebugObject *dbg, const char *fmt, ...) {}
#endif

int platform_request_firmware(const char *name,
                              int options,
                              int size,
                              int * off,
                              int (*platform_firmware_indication)(void * ctx, uint8_t * buf, uint32_t len, uint32_t off, uint32_t done),
                              void *ctx)
{
    char filename[1024];

    int fd;

    uint8_t * buffer;
    uint32_t len;
    uint32_t async = options & (1 << 29);
    uint32_t uncompress = options & (1 << 31);
    uint32_t done = 0;

    dbgp("'%s' firmware requested\n", name);
    _snprintf(filename, 1024, "%s/%s", CONFIG_FIRMWARE_PATH, name);

    if(platform_firmware_indication == 0)
    {
      return DIB_RETURN_ERROR;
    }

    if(uncompress)
    {
        dbgp("uncompress not supported on HOST\n");
        return DIB_RETURN_ERROR;
    }

    fd = _open(filename, O_RDONLY);

    if (fd < 0) {
        dbgp("could not open '%s'\n", filename);
        return DIB_RETURN_ERROR;
    }

    lseek(fd, *off, SEEK_SET);

    buffer = malloc(size);
    if(buffer == 0)
    {
        close(fd);
        return DIB_RETURN_ERROR;
    }

    while(!done)
    {
        len = read(fd, buffer, size);
        if(len < 0) {
           return DIB_RETURN_ERROR;
        }
        else {
           done = (len < size) ? 1 : 0 ;
           platform_firmware_indication(ctx, buffer, len, *off, done);
           *off += len;
           if(async)
              break;
        }
    }

    close(fd);
    free(buffer);

    return DIB_RETURN_SUCCESS;
}

