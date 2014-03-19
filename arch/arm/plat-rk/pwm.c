#define pr_fmt(fmt) "pwm: " fmt
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <plat/pwm.h>
#include <mach/io.h>
#include <linux/io.h>
#if defined CONFIG_ARCH_RK319X
#include <mach/grf.h>
#endif
static spinlock_t pwm_lock[4] = {
	__SPIN_LOCK_UNLOCKED(pwm_lock0),
	__SPIN_LOCK_UNLOCKED(pwm_lock1),
	__SPIN_LOCK_UNLOCKED(pwm_lock2),
	__SPIN_LOCK_UNLOCKED(pwm_lock3),
};

struct clk *rk_pwm_get_clk(unsigned id)
{
#if defined(CONFIG_ARCH_RK29)
	if (id < 4)
		return clk_get(NULL, "pwm");
#elif defined(CONFIG_ARCH_RK30) || defined(CONFIG_ARCH_RK3188)
	if (id == 0 || id == 1)
		return clk_get(NULL, "pwm01");
	else if (id== 2 || id == 3)
		return clk_get(NULL, "pwm23");
#elif defined(CONFIG_ARCH_RK2928) || defined(CONFIG_ARCH_RK3026)
	if (id < 3)
		return clk_get(NULL, "pwm01");
#endif
#if defined CONFIG_ARCH_RK319X
		return clk_get(NULL, "pclk_pwm");;
#endif

	pr_err("invalid pwm id %d\n", id);
	BUG();
	return ERR_PTR(-ENOENT);
}
EXPORT_SYMBOL(rk_pwm_get_clk);

void __iomem *rk_pwm_get_base(unsigned id)
{
#if defined(CONFIG_ARCH_RK29)
	if (id < 4)
		return RK29_PWM_BASE + id * 0x10;
#elif defined(CONFIG_ARCH_RK30) || defined(CONFIG_ARCH_RK3188)
	if (id == 0 || id == 1)
		return RK30_PWM01_BASE + id * 0x10;
	else if (id== 2 || id == 3)
		return RK30_PWM23_BASE + id * 0x10;
#elif defined(CONFIG_ARCH_RK2928) || defined(CONFIG_ARCH_RK3026)
	if (id < 3)
		return RK2928_PWM_BASE + id * 0x10;
#endif
#if defined CONFIG_ARCH_RK319X
		return RK319X_PWM_BASE +(id&0x03) * 0x10;
#endif


	pr_err("invalid pwm id %d\n", id);
	BUG();
	return 0;
}
EXPORT_SYMBOL(rk_pwm_get_base);

#if defined CONFIG_ARCH_RK319X
void rk_pwm_suspend(unsigned id)
{
	const void __iomem *base = rk_pwm_get_base(id);
	__rk_pwm_suspend(base);
}
EXPORT_SYMBOL(rk_pwm_suspend);

void rk_pwm_resume(unsigned id)
{
	const void __iomem *base = rk_pwm_get_base(id);
	__rk_pwm_resume(base);
	
}
EXPORT_SYMBOL(rk_pwm_resume);

#endif



void rk_pwm_setup(unsigned id, enum pwm_div div, u32 divh, u32 div_total)
{
	unsigned long flags;
	spinlock_t *lock;
#if defined CONFIG_ARCH_RK319X
	u32 grf_soc_con3;

	grf_soc_con3 = readl_relaxed(RK319X_GRF_BASE + 0x6c);
	if(!(grf_soc_con3 &(1 << 7)))
		writel_relaxed(((1<<23)|(1<<7)),RK319X_GRF_BASE + 0x6c);  //RK_PWM
#endif

	const void __iomem *base = rk_pwm_get_base(id);
	if (divh >div_total) {
		pr_err("invalid divh %d div_total %d\n", divh, div_total);
		return;
	}

	lock = &pwm_lock[id];
	spin_lock_irqsave(lock, flags);
	__rk_pwm_setup(base, div, divh, div_total);
	spin_unlock_irqrestore(lock, flags);
}
EXPORT_SYMBOL(rk_pwm_setup);
