#ifndef __PLAT_PWM_H
#define __PLAT_PWM_H

#include <linux/io.h>



#define PWM_DIV_MASK    (0xf << 9)
#define PWM_CAPTURE     (1 << 8)
#define PWM_RESET       (1 << 7)
#define PWM_INTCLR      (1 << 6)
#define PWM_INTEN       (1 << 5)
#define PWM_SINGLE      (1 << 4)

#define PWM_ENABLE      (1 << 3)
#define PWM_TIMER_EN    (1 << 0)
#define PWM_TimeEN      PWM_TIMER_EN

#define PWM_REG_CNTR    0x00
#define PWM_REG_HRC     0x04
#define PWM_REG_LRC     0x08
#define PWM_REG_CTRL    0x0c

#if defined CONFIG_ARCH_RK319X

#define PWM_REG_CNTR 			0x00
#define PWM_REG_PERIOD_HPR	0x04
#define PWM_REG_DUTY_LRC     	0x08
#define PWM_REG_CTRL 			0x0c


#define RK_PWM_ENABLE   		(1 << 0)
#define RK_PWM_DISABLE  		(0 << 0)

#define RK_PWM_SHOT                  	(0x00 << 1)
#define RK_PWM_CONTINUMOUS 	(0x01 << 1)
#define RK_PWM_CAPTURE		(0x01 << 1)

#define RK_PWM_DUTY_POSTIVE	(0x01 << 3)
#define RK_PWM_DUTY_NEGATIVE	(0x00 << 3)

#define RK_PWM_INACTIVE_POSTIVE	(0x01 << 4)
#define RK_PWM_INACTIVE_NEGATIVE	(0x00 << 4)

#define RK_PWM_OUTPUT_LEFT	(0x00 << 5)
#define RK_PWM_OUTPUT_ENTER	(0x01 << 5)


#define RK_PWM_LP_ENABLE 		(1<<8)
#define RK_PWM_LP_DISABLE 		(0<<8)

#define RK_PWM_CLK_SCALE  		(1 << 9)
#define RK_PWM_CLK_NON_SCALE  		(0 << 9)

enum pwm_div {
	PWM_DIV1		= (0x0 << 12),
	PWM_DIV2		= (0x1 << 12),
	PWM_DIV4		= (0x2 << 12),
	PWM_DIV8		= (0x3 << 12),
	PWM_DIV16		= (0x4 << 12),
	PWM_DIV32		= (0x5 << 12),
	PWM_DIV64		= (0x6 << 12),
	PWM_DIV128	= (0x7 << 12),
};
#else
enum pwm_div {
	PWM_DIV2        = (0x0 << 9),
	PWM_DIV4        = (0x1 << 9),
	PWM_DIV8        = (0x2 << 9),
	PWM_DIV16       = (0x3 << 9),
	PWM_DIV32       = (0x4 << 9),
	PWM_DIV64       = (0x5 << 9),
	PWM_DIV128      = (0x6 << 9),
	PWM_DIV256      = (0x7 << 9),
	PWM_DIV512      = (0x8 << 9),
	PWM_DIV1024     = (0x9 << 9),
	PWM_DIV2048     = (0xa << 9),
	PWM_DIV4096     = (0xb << 9),
	PWM_DIV8192     = (0xc << 9),
	PWM_DIV16384    = (0xd << 9),
	PWM_DIV32768    = (0xe << 9),
	PWM_DIV65536    = (0xf << 9),
};
#endif
#if defined CONFIG_ARCH_RK319X

static inline void __rk_pwm_suspend(const void __iomem *base)
{
	u32 con;

	barrier();
	con = readl_relaxed(base + PWM_REG_CTRL);
	con &= ~(RK_PWM_ENABLE);
//	con |= RK_PWM_DISABLE;
	dsb();
	writel_relaxed(con, base + PWM_REG_CTRL);//disable channel
	dsb();
	con |= RK_PWM_LP_ENABLE;  //lower power mode
	dsb();
	writel_relaxed(con, base + PWM_REG_CTRL);
}

static inline void __rk_pwm_resume(const void __iomem *base)
{

	u32 con;

	barrier();
	con = readl_relaxed(base + PWM_REG_CTRL);
	dsb();
	con &= ~RK_PWM_LP_ENABLE;
	writel_relaxed(con, base + PWM_REG_CTRL); //quit lower power mode
	con |= RK_PWM_ENABLE;
	dsb();
	writel_relaxed(con, base + PWM_REG_CTRL);//enable channel
}
#endif
static inline void __rk_pwm_setup(const void __iomem *base, enum pwm_div div, u32 divh, u32 div_total)
{
	u32 off;
	u32 on;
#if defined CONFIG_ARCH_RK319X
	u32 conf;
	//off = div |RK_PWM_DISABLE |;
	conf = div|RK_PWM_CLK_NON_SCALE|RK_PWM_OUTPUT_LEFT|RK_PWM_LP_DISABLE|
				RK_PWM_CONTINUMOUS|RK_PWM_DUTY_POSTIVE|RK_PWM_INACTIVE_NEGATIVE;
	barrier();

	writel_relaxed(RK_PWM_DISABLE, base + PWM_REG_CTRL);
	dsb();
	writel_relaxed(div_total, base + PWM_REG_PERIOD_HPR);  // lrc witch 
	writel_relaxed(divh, base + PWM_REG_DUTY_LRC);  // different with RW_PWM    
	writel_relaxed(0, base + PWM_REG_CNTR);
	dsb();
	//writel_relaxed(conf, base + PWM_REG_CTRL);
	dsb();
	writel_relaxed(conf|RK_PWM_ENABLE, base + PWM_REG_CTRL);
	dsb();
#else
	off = div | PWM_RESET;
	on = div | PWM_ENABLE | PWM_TIMER_EN;

	barrier();
	writel_relaxed(off, base + PWM_REG_CTRL);
	dsb();
	writel_relaxed(divh, base + PWM_REG_HRC);
	writel_relaxed(div_total, base + PWM_REG_LRC);
	writel_relaxed(0, base + PWM_REG_CNTR);
	dsb();
	writel_relaxed(on, base + PWM_REG_CTRL);
	dsb();
#endif
}

struct clk *rk_pwm_get_clk(unsigned pwm_id);
void __iomem *rk_pwm_get_base(unsigned pwm_id);
void rk_pwm_setup(unsigned pwm_id, enum pwm_div div, u32 hrc, u32 lrc);
#if defined CONFIG_ARCH_RK319X
void rk_pwm_resume(unsigned id);
void rk_pwm_suspend(unsigned id);
#endif
#endif
