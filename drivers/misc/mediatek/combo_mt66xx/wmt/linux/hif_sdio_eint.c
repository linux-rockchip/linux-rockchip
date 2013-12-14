
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/core.h>
#include <linux/combo_mt66xx.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include "hif_sdio.h"
#include "hif_sdio_eint.h"

static int mtk_hif_mmc_io_rw_direct_host(struct mmc_host *host, int write, unsigned fn, unsigned addr, u8 in, u8 *out);
static int mtk_hif_mmc_io_rw_direct(struct mmc_card *card, int write, unsigned fn, unsigned addr, u8 in, u8 *out);
static int mtk_hif_process_sdio_pending_irqs(struct mmc_card *card);
static void mtk_hif_enable_irq(int enable);
static int mtk_hif_sdio_irq_thread(void *_host);
static int mtk_hif_request_irq(void);
static void mtk_hif_free_irq(void);
static irqreturn_t mtk_hif_eint_isr(int irq, void *p);

struct _mtk_sdio_eint mtk_sdio_eint = {
	.func_count = 0,
	.mtk_sdio_eirq_thread = NULL,
	.single_func = NULL,
	.sdio_eirq = -EINVAL,
};

unsigned long ext_irq_flag; 

static irqreturn_t mtk_hif_eint_isr(int irq, void *p)
{
	mtk_hif_enable_irq(0);

	atomic_set(&(mtk_sdio_eint.intrs), 1);

	if (unlikely(test_bit(IN_SUSPEND, &ext_irq_flag))) {
 		/* record the INT event during suspend state */
		set_bit(IRQ_IN_SUSPEND, &ext_irq_flag);
		smp_wmb();
	} else {
		wake_up_interruptible(&(mtk_sdio_eint.thread_wq));
	}

	return IRQ_HANDLED;
} 
static int mtk_hif_request_irq(void)
{
	int ret = 0;

	ret = request_irq(mtk_sdio_eint.sdio_eirq, mtk_hif_eint_isr, IRQF_TRIGGER_LOW | IRQF_DISABLED, "MTK_SDIO_EINT", NULL);
	if (ret) {
		HIF_SDIO_INFO_FUNC("SDIO EIRQ request_irq fail, ret = %d\n", ret);
	} else {
    	enable_irq_wake(mtk_sdio_eint.sdio_eirq);
		disable_irq(mtk_sdio_eint.sdio_eirq);
	}
	return ret;
}
static void mtk_hif_free_irq(void)
{
   	disable_irq_wake(mtk_sdio_eint.sdio_eirq);
	free_irq(mtk_sdio_eint.sdio_eirq, NULL);
}

static void mtk_hif_enable_irq(int enable)
{
	if (enable) {
		enable_irq(mtk_sdio_eint.sdio_eirq);
	} else {
		disable_irq_nosync(mtk_sdio_eint.sdio_eirq);
	}
	return;
}

static int mtk_hif_mmc_io_rw_direct_host(struct mmc_host *host, int write, unsigned fn,
	unsigned addr, u8 in, u8 *out)
{
	struct mmc_command cmd = {0};
	int err;

	BUG_ON(!host);
	BUG_ON(fn > 7);

	/* sanity check */
	if (addr & ~0x1FFFF)
		return -EINVAL;

	cmd.opcode = SD_IO_RW_DIRECT;
	cmd.arg = write ? 0x80000000 : 0x00000000;
	cmd.arg |= fn << 28;
	cmd.arg |= (write && out) ? 0x08000000 : 0x00000000;
	cmd.arg |= addr << 9;
	cmd.arg |= in;
	cmd.flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_AC;

	err = mmc_wait_for_cmd(host, &cmd, 0);
	if (err)
		return err;

	if (mmc_host_is_spi(host)) {
		/* host driver already reported errors */
	} else {
		if (cmd.resp[0] & R5_ERROR)
			return -EIO;
		if (cmd.resp[0] & R5_FUNCTION_NUMBER)
			return -EINVAL;
		if (cmd.resp[0] & R5_OUT_OF_RANGE)
			return -ERANGE;
	}

	if (out) {
		if (mmc_host_is_spi(host))
			*out = (cmd.resp[0] >> 8) & 0xFF;
		else
			*out = cmd.resp[0] & 0xFF;
	}

	return 0;
}

static int mtk_hif_mmc_io_rw_direct(struct mmc_card *card, int write, unsigned fn,
	unsigned addr, u8 in, u8 *out)
{
	BUG_ON(!card);
	return mtk_hif_mmc_io_rw_direct_host(card->host, write, fn, addr, in, out);
}

static int mtk_hif_process_sdio_pending_irqs(struct mmc_card *card)
{
	int i, ret, count;
	unsigned char pending;
	struct mtk_sdio_funcs *l_func;
	struct sdio_func *func;

	l_func = mtk_sdio_eint.single_func;
	if (l_func) {
		l_func->irq_handler(l_func->func);
		return 1;	
	}

	ret = mtk_hif_mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_INTx, 0, &pending);
	if (ret) {
		HIF_SDIO_INFO_FUNC("%s: error %d reading SDIO_CCCR_INTx\n",
		       mmc_card_id(card), ret);
		return ret;
	}

	if (!pending)
		HIF_SDIO_WARN_FUNC("SDIO_CCCR_INTx = 0, maybe did not enable func intr\n");

	count = 0;
	for (i = 1; i <= 7; i++) {
		if (pending & (1 << i)) {
			func = card->sdio_func[i - 1];
			if (func && mtk_sdio_eint.funcs[func->num].irq_handler) {
				(mtk_sdio_eint.funcs[func->num].irq_handler)(func);
				count++;
			} else {
				HIF_SDIO_WARN_FUNC("irq_handler = %x, func = %x\n", mtk_sdio_eint.funcs[func->num].irq_handler, func);
				ret = -EINVAL;
			}
		}
	}

	if (count)
		return count;

	return ret;
}



static int mtk_hif_sdio_irq_thread(void *_host)
{
	struct mmc_host *host = _host;
	int ret = -1;

	HIF_SDIO_INFO_FUNC("mtk_hif_sdio_irq_thread started.\n");

	for(;;) {
		wait_event_interruptible(mtk_sdio_eint.thread_wq, kthread_should_stop() || atomic_read(&(mtk_sdio_eint.intrs)));

		//mutex_lock(&(mtk_sdio_eint.lock)); avoid dead lock
		if(kthread_should_stop()) {
			HIF_SDIO_INFO_FUNC("kthread_should_stop.\n");
			break;
		}
                mutex_lock(&(mtk_sdio_eint.lock));
		atomic_set(&(mtk_sdio_eint.intrs), 0);

		ret = __mmc_claim_host(host, &(mtk_sdio_eint.irq_thread_abort));
		if (ret)
		{
			mutex_unlock(&(mtk_sdio_eint.lock));
			break;
		}
		//do { //TODO make sure we will not lost irq.
			ret = mtk_hif_process_sdio_pending_irqs(host->card);
		//while(ret > 0);
		mmc_release_host(host);

		mtk_hif_enable_irq(1);
		mutex_unlock(&(mtk_sdio_eint.lock));
	}
        mutex_lock(&(mtk_sdio_eint.lock));
	mtk_hif_enable_irq(0);
	mutex_unlock(&(mtk_sdio_eint.lock));

	HIF_SDIO_INFO_FUNC("%s: IRQ thread exiting with code %d\n", mmc_hostname(host), ret);
	return ret;
}

int mtk_hif_sdio_claim_irq(struct sdio_func *func, sdio_irq_handler_t *handler)
{
	int ret;
	unsigned char reg;
	struct mmc_host *host = NULL; 

	BUG_ON(!func);
	BUG_ON(!func->card);
	BUG_ON(!func->card->host);
	HIF_SDIO_INFO_FUNC("SDIO: Enabling IRQ for func %d.\n", func->num);

	mutex_lock(&mtk_sdio_eint.lock);
	if (mtk_sdio_eint.funcs[func->num].func != NULL) {
		HIF_SDIO_INFO_FUNC("func num %d already on.\n",func->num);
		mutex_unlock(&mtk_sdio_eint.lock);
		return -EBUSY;
	}
	host = func->card->host;

	mtk_sdio_eint.funcs[func->num].func = func;
	mtk_sdio_eint.funcs[func->num].irq_handler = handler;

	ret = mtk_hif_mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IENx, 0, &reg);
	if (ret) {
		HIF_SDIO_INFO_FUNC("mmc io read fail, ret = %d\n", ret);
		mutex_unlock(&mtk_sdio_eint.lock);
		return ret;
	}

	reg |= 1 << func->num;

	reg |= 1; /* Master interrupt enable */

	ret = mtk_hif_mmc_io_rw_direct(func->card, 1, 0, SDIO_CCCR_IENx, reg, NULL);
	if (ret) {
		HIF_SDIO_INFO_FUNC("mmc io write fail, ret = %d\n", ret);
		mutex_unlock(&mtk_sdio_eint.lock);
		return ret;
	}

	if (!mtk_sdio_eint.func_count++) {
		atomic_set(&(mtk_sdio_eint.irq_thread_abort), 0);
		mtk_sdio_eint.mtk_sdio_eirq_thread = kthread_run(mtk_hif_sdio_irq_thread, host, "mtkksdioirqd/%s", mmc_hostname(host));
		if (IS_ERR(mtk_sdio_eint.mtk_sdio_eirq_thread)) {
			int err = PTR_ERR(mtk_sdio_eint.mtk_sdio_eirq_thread);
			atomic_set(&(mtk_sdio_eint.irq_thread_abort), 1);
			mtk_sdio_eint.mtk_sdio_eirq_thread = NULL;
			mtk_sdio_eint.func_count--;
			mtk_sdio_eint.funcs[func->num].func = NULL;
			mtk_sdio_eint.funcs[func->num].irq_handler = NULL;
			HIF_SDIO_INFO_FUNC("SDIO:start thread fail.\n");
			mutex_unlock(&mtk_sdio_eint.lock);
			return err;
		} else {
			mtk_hif_request_irq();
			mtk_hif_enable_irq(1);
			add_quirk(func->card, MMC_QUIRK_LENIENT_FN0);
		}
	}

	if (mtk_sdio_eint.func_count == 1) {
		mtk_sdio_eint.single_func = &mtk_sdio_eint.funcs[func->num];
	} else {
		mtk_sdio_eint.single_func = NULL;
	}

	HIF_SDIO_INFO_FUNC("SDIO: Enabling IRQ for func %d done. func_count = %d\n", func->num, mtk_sdio_eint.func_count);
	{
		int i;
		for(i=1; i<=7; i++){
			HIF_SDIO_INFO_FUNC("func %d, func: %p, handler: %x", i, mtk_sdio_eint.funcs[i].func, mtk_sdio_eint.funcs[i].irq_handler);
		}
	}
	mutex_unlock(&mtk_sdio_eint.lock);
	return ret;
}

int mtk_hif_sdio_release_irq(struct sdio_func *func)
{
	int ret, i;
	unsigned char reg;

	BUG_ON(!func);
	BUG_ON(!func->card);

	HIF_SDIO_INFO_FUNC("SDIO: Disabling IRQ for %s...\n", sdio_func_id(func));

	mutex_lock(&mtk_sdio_eint.lock);

	mtk_sdio_eint.funcs[func->num].func = NULL;
	mtk_sdio_eint.funcs[func->num].irq_handler = NULL;

	if (!--mtk_sdio_eint.func_count) {
		mtk_hif_enable_irq(0);
		mtk_hif_free_irq();
		atomic_set(&(mtk_sdio_eint.irq_thread_abort), 1);
		mutex_unlock(&mtk_sdio_eint.lock);
		kthread_stop(mtk_sdio_eint.mtk_sdio_eirq_thread);
		mutex_lock(&mtk_sdio_eint.lock);
		mtk_sdio_eint.mtk_sdio_eirq_thread = NULL;
	}

	ret = mtk_hif_mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IENx, 0, &reg);
	if (ret) {
		mutex_unlock(&mtk_sdio_eint.lock);
		return ret;
	}

	reg &= ~(1 << func->num);

	/* Disable master interrupt with the last function interrupt */
	if (!(reg & 0xFE))
		reg = 0;

	ret = mtk_hif_mmc_io_rw_direct(func->card, 1, 0, SDIO_CCCR_IENx, reg, NULL);
	if (ret) {
		mutex_unlock(&mtk_sdio_eint.lock);
		return ret;
	}

	if (mtk_sdio_eint.func_count == 1) {
		for (i = 1; i <= 7; i++) {
			if(mtk_sdio_eint.funcs[i].func != NULL) {
				mtk_sdio_eint.single_func = &mtk_sdio_eint.funcs[i];
				break;
			}
		}
	}
	// for Debug
	{
		int i;
		for(i=1; i<=7; i++){
			HIF_SDIO_INFO_FUNC("func %d, func: %p, handler: %x", i, mtk_sdio_eint.funcs[i].func, mtk_sdio_eint.funcs[i].irq_handler);
		}
	}

	HIF_SDIO_INFO_FUNC("SDIO: Disabling IRQ for %s... done\n", sdio_func_id(func));
	mutex_unlock(&mtk_sdio_eint.lock);
	return 0;
}

static int hif_sdio_plt_probe(struct platform_device *pdev)
{
	struct mtk_sdio_eint_platform_data *p = pdev->dev.platform_data;
	int ret = -EINVAL;

	if (gpio_is_valid(p->sdio_eint)) {
		ret = gpio_request(p->sdio_eint, "MT66XX SDIO EINT");
		if (ret) {
			HIF_SDIO_INFO_FUNC("SDIO EINT gpio_request fail, ret = %d\n", ret);
			mtk_sdio_eint.sdio_eirq = -EINVAL;
		} else {
			mtk_sdio_eint.sdio_eirq = gpio_to_irq(p->sdio_eint);
			HIF_SDIO_INFO_FUNC("get SDIO EIRQ success: %d\n", mtk_sdio_eint.sdio_eirq);
		}
	} else {
		HIF_SDIO_INFO_FUNC("invalid SDIO EINT gpio:%d\n", p->sdio_eint);
	}

	return ret;
}

static int hif_sdio_plt_remove(struct platform_device *pdev)
{
	struct mtk_sdio_eint_platform_data *p = pdev->dev.platform_data;

	if (gpio_is_valid(p->sdio_eint)) {
		mtk_sdio_eint.sdio_eirq = -EINVAL;
		gpio_free(p->sdio_eint);
	}
	return 0;
}

static struct platform_driver hif_sdio_plt_driver = {
	.probe = hif_sdio_plt_probe,
	.remove = hif_sdio_plt_remove,
	.driver = {
		.name = "mtk_sdio_eint",
		.owner = THIS_MODULE,
	},
};

void mtk_hif_sdio_eint_init(void)
{
	int ret;
	ret = platform_driver_register(&hif_sdio_plt_driver);
	if (ret) {
		HIF_SDIO_INFO_FUNC("register platform driver failed.\n");
	}
	mutex_init(&(mtk_sdio_eint.lock));
	memset(&(mtk_sdio_eint.funcs[0]), 0, sizeof(mtk_sdio_eint.funcs));
	init_waitqueue_head(&(mtk_sdio_eint.thread_wq));
	mtk_sdio_eint.func_count = 0;
	mtk_sdio_eint.mtk_sdio_eirq_thread = NULL;
	atomic_set(&(mtk_sdio_eint.irq_thread_abort), 1);
	mtk_sdio_eint.single_func = NULL;
	ext_irq_flag = 0;
}

void mtk_hif_sdio_eint_deinit(void)
{
	platform_driver_unregister(&hif_sdio_plt_driver);
}

