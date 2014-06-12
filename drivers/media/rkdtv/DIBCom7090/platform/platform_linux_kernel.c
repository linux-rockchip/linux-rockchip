#include <adapter/debug.h>

#include <stdarg.h>
#include <linux/time.h>


uint32_t systime()
{
    struct timeval tv;
    do_gettimeofday(&tv);
    /* drop the MSBs of the seconds, we just need a tick-count, rather than a date */
    return (tv.tv_sec * 10000) + (tv.tv_usec / 100);
}

void debug_printf_line(const struct dibDebugObject *dbg, const char *fmt, ...)
{
	va_list ap;

	if (dbg)
		printk("%-12s: ", dbg->prefix);
	else
		printk("%-12s: ", "DBG");

	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);

	printk("\n");
}

void debug_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);
}
