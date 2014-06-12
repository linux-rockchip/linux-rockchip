#include <adapter/debug.h>
#include <sys/time.h>
#include <adapter/i2c.h>

#include <adapter/common.h>

#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef CONFIG_DEBUG
void debug_printf_line(const struct dibDebugObject *dbg, const char *fmt, ...)
{
	va_list ap;

	if (dbg)
		fprintf(stderr, "%-12s: ", dbg->prefix);
	else
		fprintf(stderr, "%-12s: ", "DBG");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
}

void debug_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
#endif

#ifdef DIBCOM_TESTING

void debug_dump_i2c(struct dibI2CAccess *msg)
{
	int i;
	printf("%c %d ", msg->rx == NULL ? 'w' : 'r', msg->addr);
	for (i = 0; i < msg->txlen; i++)
		printf("0x%02x ", msg->tx[i]);
	printf("\n");
}

#endif

/* FOR VOYAGER SIMU
struct dibDataBusClient *platform_client;
uint32_t systime()
{
    return (data_bus_client_read16(platform_client, 0x90000000)<<16) | data_bus_client_read16(platform_client, 0x90000002);
}
*/

/* get a rough time approximation (precision required is 1ms) */
/* function required for the asynchronous tune */
uint32_t systime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    /* drop the MSBs of the seconds, we just need a tick-count, rather than a date */
    return (tv.tv_sec * 10000) + (tv.tv_usec / 100);
}

void busy_wait(uint32_t usec)
{
    uint32_t stop = systime() + usec/100 + 1;
    while (systime() < stop);
}

static double since;

void ptime(const char *s)
{
	struct timeval t;
	double now;
	gettimeofday(&t, NULL);

	now =  t.tv_sec + t.tv_usec/1e6;
	printf("%3.6f: %s\n", now - since, s);
	since = now;
}

int platform_request_firmware(const char *name,
                              int options,
                              int size,
                              int * off,
                              int (*platform_firmware_indication)(void * ctx, uint8_t * buf, uint32_t len, uint32_t off, uint32_t done),
                              void *ctx)
{
    dbgp("'%s' firmware requested\n", name);
    char filename[1024];

    int fd;

    uint8_t * buffer;
    int32_t len;
    uint32_t async = options & (1 << 29);
    uint32_t uncompress = options & (1 << 31);
    uint32_t done = 0;

    /* if max length is forwarded keeping the options, clean it */
    size &= ~REQUEST_FIRMWARE_OPTION_MASK;

    snprintf(filename, 1024, "./%s", name);

    if(platform_firmware_indication == 0)
    {
      return DIB_RETURN_ERROR;
    }

    if(uncompress)
    {
        dbgp("uncompress not supported on HOST\n");
        return DIB_RETURN_ERROR;
    }

    fd = open(filename, O_RDONLY);

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
