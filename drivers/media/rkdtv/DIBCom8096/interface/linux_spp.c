#include <adapter/common.h>
#include <adapter/busdescriptor.h>
#include <adapter/busadapter.h>

#include <interface/linux_spp.h>

#include <unistd.h>
#include <sys/io.h>

struct linux_spp_state {
	struct dibSPPDescriptor desc;
};

static uint8_t linux_spp_inb(const struct dibSPPDescriptor * desc, uint16_t port)
{
	return inb(port);
}

static void linux_spp_outb(const struct dibSPPDescriptor * desc, uint16_t addr, uint8_t val)
{
	outb(val, addr);
}

static int linux_spp_check_permission(const struct dibSPPDescriptor *desc, uint16_t port)
{
	return ioperm(port, 3, 1) == 0 ? DIB_RETURN_SUCCESS : DIB_RETURN_ERROR;
}

struct dibSPPDescriptor * linux_spp_open()
{
	struct linux_spp_state *st = MemAlloc(sizeof(struct linux_spp_state));

	st->desc.inb              = linux_spp_inb;
	st->desc.outb             = linux_spp_outb;
	st->desc.check_permission = linux_spp_check_permission;
	st->desc.priv             = st;

	return &st->desc;
}

void linux_spp_close(struct dibSPPDescriptor *desc)
{
	struct linux_spp_state *st = desc->priv;
	DibDbgPrint("-I-  closing SPP\n");
	MemFree(st,sizeof(struct linux_spp_state));
}
