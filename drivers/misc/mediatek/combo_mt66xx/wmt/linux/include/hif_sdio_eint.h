#ifndef __HIF_SDIO_EINT_H__
#define __HIF_SDIO_EINT_H__

struct mtk_sdio_funcs {
	struct sdio_func *func;
	sdio_irq_handler_t *irq_handler;
};

struct _mtk_sdio_eint {
	struct mutex lock;
	struct mtk_sdio_funcs funcs[7];
	wait_queue_head_t thread_wq;
	int func_count;
	struct task_struct *mtk_sdio_eirq_thread;
	atomic_t irq_thread_abort;
	struct mtk_sdio_funcs  *single_func;
	atomic_t intrs;
	int sdio_eirq;
};

enum { /*ext_irq_flag */
	IN_SUSPEND = 0, /* in suspend state or not */
	IRQ_IN_SUSPEND = 1, /* irq triggered during suspend state */
};

int mtk_hif_sdio_irq_flag_set (int flag); 

int mtk_hif_sdio_claim_irq(struct sdio_func *func, sdio_irq_handler_t *handler);

int mtk_hif_sdio_release_irq(struct sdio_func *func);

void mtk_hif_sdio_eint_init(void);

#endif
