/*
 * Regulator driver for rk818 PMIC chip for rk31xx
 *
 * Based on rk818.c that is work by zhangqing<zhangqing@rock-chips.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/bug.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/regulator/driver.h>
#include <linux/mfd/rk818.h>
#include <linux/mfd/core.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include <mach/iomux.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>


#if 0
#define DBG(x...)	printk(KERN_INFO x)
#else
#define DBG(x...)
#endif
#if 1
#define DBG_INFO(x...)	printk(KERN_INFO x)
#else
#define DBG_INFO(x...)
#endif
#define PM_CONTROL

struct rk818 *g_rk818;
#define DCDC_RAISE_VOL_BYSTEP 0
#define DCDC_VOL_STEP 25000  //25mv

static struct mfd_cell rk818s[] = {
	{
		.name = "rk818-rtc",
	},
	{
		.name = "rk818-battery",
	},
	{
		.name = "rk818-power",
	},	
};

#define BUCK_VOL_MASK 0x3f
#define LDO_VOL_MASK 0x3f
#define LDO9_VOL_MASK 0x1f
#define BOOST_VOL_MASK 0xe0

#define VOL_MIN_IDX 0x00
#define VOL_MAX_IDX 0x3f
#define RK818_I2C_ADDR_RATE  200*1000

const static int buck_set_vol_base_addr[] = {
	RK818_BUCK1_ON_REG,
	RK818_BUCK2_ON_REG,
	RK818_BUCK3_CONFIG_REG,
	RK818_BUCK4_ON_REG,
};
const static int buck_set_slp_vol_base_addr[] = {
	RK818_BUCK1_SLP_REG,
	RK818_BUCK2_SLP_REG,
	RK818_BUCK3_CONFIG_REG,
	RK818_BUCK4_SLP_VSEL_REG,
};
const static int buck_contr_base_addr[] = {
	RK818_BUCK1_CONFIG_REG,
 	RK818_BUCK2_CONFIG_REG,
 	RK818_BUCK3_CONFIG_REG,
 	RK818_BUCK4_CONFIG_REG,
};
#define rk818_BUCK_SET_VOL_REG(x) (buck_set_vol_base_addr[x])
#define rk818_BUCK_CONTR_REG(x) (buck_contr_base_addr[x])
#define rk818_BUCK_SET_SLP_VOL_REG(x) (buck_set_slp_vol_base_addr[x])


const static int ldo_set_vol_base_addr[] = {
	RK818_LDO1_ON_VSEL_REG,
	RK818_LDO2_ON_VSEL_REG,
	RK818_LDO3_ON_VSEL_REG,
	RK818_LDO4_ON_VSEL_REG, 
	RK818_LDO5_ON_VSEL_REG, 
	RK818_LDO6_ON_VSEL_REG, 
	RK818_LDO7_ON_VSEL_REG, 
	RK818_LDO8_ON_VSEL_REG, 
	RK818_BOOST_LDO9_ON_VSEL_REG, 
};
const static int ldo_set_slp_vol_base_addr[] = {
	RK818_LDO1_SLP_VSEL_REG,
	RK818_LDO2_SLP_VSEL_REG,
	RK818_LDO3_SLP_VSEL_REG,
	RK818_LDO4_SLP_VSEL_REG, 
	RK818_LDO5_SLP_VSEL_REG, 
	RK818_LDO6_SLP_VSEL_REG, 
	RK818_LDO7_SLP_VSEL_REG, 
	RK818_LDO8_SLP_VSEL_REG, 
	RK818_BOOST_LDO9_SLP_VSEL_REG, 
};
/*
const static int ldo_contr_base_addr[] = {
	rk818_LDO1_CONTR_BASE,
	rk818_LDO2_CONTR_BASE,
	rk818_LDO3_CONTR_BASE,
	rk818_LDO4_CONTR_BASE,
	rk818_LDO5_CONTR_BASE,
	rk818_LDO6_CONTR_BASE,
	rk818_LDO7_CONTR_BASE,
	rk818_LDO8_CONTR_BASE,
//	rk818_LDO9_CONTR_BASE,
};
*/
#define rk818_LDO_SET_VOL_REG(x) (ldo_set_vol_base_addr[x])
#define rk818_LDO_SET_SLP_VOL_REG(x) (ldo_set_slp_vol_base_addr[x])

const static int buck_voltage_map[] = {
	  700,  712,  725,  737,  750, 762,  775,  787,  800, 
	  812,  825,  837,  850,862,  875,  887,  900,  912,
	  925,  937,  950, 962,  975,  987, 1000, 1012, 1025, 
	  1037, 1050,1062, 1075, 1087, 1100, 1112, 1125, 1137, 
	  1150,1162, 1175, 1187, 1200, 1212, 1225, 1237, 1250,
	  1262, 1275, 1287, 1300, 1312, 1325, 1337, 1350,1362, 
	  1375, 1387, 1400, 1412, 1425, 1437, 1450,1462, 1475, 
	  1487, 1500,
};

const static int buck4_voltage_map[] = {
           1800, 1900, 2000, 2100, 2200,  2300,  2400, 2500, 2600,
          2700, 2800, 2900, 3000, 3100, 3200,3300, 3400,3500,3600,
};

const static int ldo_voltage_map[] = {
	  1800, 1900, 2000, 2100, 2200,  2300,  2400, 2500, 2600, 
	  2700, 2800, 2900, 3000, 3100, 3200,3300, 3400, 
};
const static int ldo3_voltage_map[] = {
	 800, 900, 1000, 1100, 1200,  1300, 1400, 1500, 1600, 
	 1700, 1800, 1900,  2000,2100,  2200,  2500,
};
const static int ldo6_voltage_map[] = {
	 800, 900, 1000, 1100, 1200,  1300, 1400, 1500, 1600, 
	 1700, 1800, 1900,  2000,2100,  2200,  2300,2400,2500,
};
const static int boost_voltage_map[] = {
	 4700,4800,4900,5000,5100,5200,5300,5400,
};

static int rk818_ldo_list_voltage(struct regulator_dev *dev, unsigned index)
{
	int ldo= rdev_get_id(dev) - RK818_LDO1;
	if (ldo == 2){
	if (index >= ARRAY_SIZE(ldo3_voltage_map))
		return -EINVAL;
	return 1000 * ldo3_voltage_map[index];
	}
	else if (ldo == 5 || ldo ==6){
	if (index >= ARRAY_SIZE(ldo6_voltage_map))
		return -EINVAL;
	return 1000 * ldo6_voltage_map[index];
	}
	else{
	if (index >= ARRAY_SIZE(ldo_voltage_map))
		return -EINVAL;
	return 1000 * ldo_voltage_map[index];
	}
}
static int rk818_ldo_is_enabled(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int ldo= rdev_get_id(dev) - RK818_LDO1;	
	u16 val;

	if (ldo == 8){
		val = rk818_reg_read(rk818, RK818_DCDC_EN_REG);  //ldo9
		if (val < 0)
			return val;
		if (val & (1 << 5))
			return 1;
		else
			return 0; 
	}
	val = rk818_reg_read(rk818, RK818_LDO_EN_REG);
	if (val < 0)
		return val;
	if (val & (1 << ldo))
		return 1;
	else
		return 0; 		
}
static int rk818_ldo_enable(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int ldo= rdev_get_id(dev) - RK818_LDO1;

	if (ldo == 8){
		 rk818_set_bits(rk818, RK818_DCDC_EN_REG, 1 << 5, 1 << 5); //ldo9
	}
	else
	 	rk818_set_bits(rk818, RK818_LDO_EN_REG, 1 << ldo, 1 << ldo);

	 return 0;	
}
static int rk818_ldo_disable(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int ldo= rdev_get_id(dev) - RK818_LDO1;

	if (ldo == 8){
		 rk818_set_bits(rk818, RK818_DCDC_EN_REG, 1 << 5, 1 << 0); //ldo9
	}
	else
	 	rk818_set_bits(rk818, RK818_LDO_EN_REG, 1 << ldo, 0);

	 return 0;
}
static int rk818_ldo_get_voltage(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int ldo= rdev_get_id(dev) - RK818_LDO1;
	u16 reg = 0;
	int val;
	reg = rk818_reg_read(rk818,rk818_LDO_SET_VOL_REG(ldo));
	if (ldo == 8){
		reg &= LDO9_VOL_MASK;
	}
	else
		reg &= LDO_VOL_MASK;
	
	if (ldo ==2){
	val = 1000 * ldo3_voltage_map[reg];	
	}
	else if (ldo == 5 || ldo ==6){
	val = 1000 * ldo6_voltage_map[reg];	
	}
	else{
	val = 1000 * ldo_voltage_map[reg];	
	}
	return val;
}
static int rk818_ldo_set_sleep_voltage(struct regulator_dev *dev,
					    int uV)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int ldo= rdev_get_id(dev) - RK818_LDO1;
	const int *vol_map = ldo_voltage_map;
	int min_vol = uV / 1000;
	u16 val;
	int ret = 0,num =0;
	
	if (ldo ==2){
	vol_map = ldo3_voltage_map;	
	num = 15;
	}
	else if (ldo == 5 || ldo ==6){
	vol_map = ldo6_voltage_map;		
	num = 17;
	}
	else {
	num = 16;
	}
	
	if (min_vol < vol_map[0] ||
	    min_vol > vol_map[num])
		return -EINVAL;

	for (val = 0; val <= num; val++){
		if (vol_map[val] >= min_vol)
			break;	
        }

	if (ldo == 8){
		ret = rk818_set_bits(rk818, rk818_LDO_SET_SLP_VOL_REG(ldo),LDO9_VOL_MASK, val);
	}
	else
		ret = rk818_set_bits(rk818, rk818_LDO_SET_SLP_VOL_REG(ldo),LDO_VOL_MASK, val);
	
	return ret;
}

static int rk818_ldo_set_voltage(struct regulator_dev *dev,
				  int min_uV, int max_uV,unsigned *selector)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int ldo= rdev_get_id(dev) - RK818_LDO1;
	const int *vol_map;
	int min_vol = min_uV / 1000;
	u16 val;
	int ret = 0,num =0;
	
	if (ldo ==2){
	vol_map = ldo3_voltage_map;	
	num = 15;
	}
	else if (ldo == 5 || ldo ==6){
	vol_map = ldo6_voltage_map;		
	num = 17;
	}
	else {
	vol_map = ldo_voltage_map;
	num = 16;
	}
	
	if (min_vol < vol_map[0] ||
	    min_vol > vol_map[num])
		return -EINVAL;

	for (val = 0; val <= num; val++){
		if (vol_map[val] >= min_vol)
			break;	
        }

	if (ldo == 8){
		ret = rk818_set_bits(rk818, rk818_LDO_SET_VOL_REG(ldo),LDO9_VOL_MASK, val);
	}
	else
		ret = rk818_set_bits(rk818, rk818_LDO_SET_VOL_REG(ldo),LDO_VOL_MASK, val);
	
	return ret;

}

static struct regulator_ops rk818_ldo_ops = {
	.set_voltage = rk818_ldo_set_voltage,
	.get_voltage = rk818_ldo_get_voltage,
	.list_voltage = rk818_ldo_list_voltage,
	.is_enabled = rk818_ldo_is_enabled,
	.enable = rk818_ldo_enable,
	.disable = rk818_ldo_disable,
	.set_suspend_voltage = rk818_ldo_set_sleep_voltage,	
};

static int rk818_dcdc_list_voltage(struct regulator_dev *dev, unsigned selector)
{
	int volt;
	int buck = rdev_get_id(dev) - RK818_DCDC1;

	if (selector < 0x0 ||selector > BUCK_VOL_MASK )
		return -EINVAL;

	switch (buck) {
	case 0:
	case 1:
		volt = 700000 + selector * 12500;
		break;
	case 3:
		volt = 1800000 + selector * 100000;
		break;
	case 2:
		volt = 1200000;
		break;
	default:
		BUG();
		return -EINVAL;
	}

	return  volt ;
}
static int rk818_dcdc_is_enabled(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;
	u16 val;
	
	val = rk818_reg_read(rk818, RK818_DCDC_EN_REG);
	if (val < 0)
		return val;
	if (val & (1 << buck))
		return 1;
	else
		return 0; 	
}
static int rk818_dcdc_enable(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;

	return rk818_set_bits(rk818, RK818_DCDC_EN_REG, 1 << buck, 1 << buck);

}
static int rk818_dcdc_disable(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;
	
	 return rk818_set_bits(rk818, RK818_DCDC_EN_REG, 1 << buck , 0);
}
static int rk818_dcdc_get_voltage(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;
	u16 reg = 0;
	int val;

	reg = rk818_reg_read(rk818,rk818_BUCK_SET_VOL_REG(buck));
	
	reg &= BUCK_VOL_MASK;
    	val = rk818_dcdc_list_voltage(dev,reg);
	return val;
}
static int rk818_dcdc_select_min_voltage(struct regulator_dev *dev,
					   int min_uV, int max_uV ,int buck)
{
	u16 vsel =0;
	
	if (buck == 0 || buck ==  1){
		if (min_uV < 700000)
		vsel = 0;
		else if (min_uV <= 1500000)
		vsel = ((min_uV - 700000) / 12500) ;
		else
		return -EINVAL;
	}
	else if (buck ==3){
		if (min_uV < 1800000)
		vsel = 0;
		else if (min_uV <= 3300000)
		vsel = ((min_uV - 1800000) / 100000) ;
		else
		return -EINVAL;
	}
	if (rk818_dcdc_list_voltage(dev, vsel) > max_uV)
		return -EINVAL;
	return vsel;
}

static int rk818_dcdc_set_voltage(struct regulator_dev *dev,
				  int min_uV, int max_uV,unsigned *selector)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;
	u16 val;
	int ret = 0,old_voltage =0,vol_temp =0;

	if (buck ==2){
		return 0;
	}else if (buck==3){
		val = rk818_dcdc_select_min_voltage(dev,min_uV,max_uV,buck);	
		ret = rk818_set_bits(rk818, rk818_BUCK_SET_VOL_REG(buck), BUCK_VOL_MASK, val);
	}
	else {
#if 0
//#if defined(DCDC_RAISE_VOL_BYSTEP)
		old_voltage = rk818_dcdc_get_voltage(dev);
			if (max_uV >old_voltage){
				vol_temp = old_voltage;
			       do{
					vol_temp +=   DCDC_VOL_STEP;
					val = rk818_dcdc_select_min_voltage(dev,vol_temp,vol_temp,buck);
				//	printk("rk818_dcdc_set_voltage buck = %d vol_temp= %d old_voltage= %d min_uV =%d \n",buck,vol_temp,old_voltage,min_uV);
					ret = rk818_set_bits(rk818, rk818_BUCK_SET_VOL_REG(buck), BUCK_VOL_MASK, val);	
				}while(vol_temp != max_uV);
			}
			else{
				val = rk818_dcdc_select_min_voltage(dev,min_uV,max_uV,buck);
				ret = rk818_set_bits(rk818, rk818_BUCK_SET_VOL_REG(buck), BUCK_VOL_MASK, val);
			}
#else

		val = rk818_dcdc_select_min_voltage(dev,min_uV,max_uV,buck);
		ret = rk818_set_bits(rk818, rk818_BUCK_SET_VOL_REG(buck), BUCK_VOL_MASK, val);
#endif
	}
	return ret;
}
static int rk818_dcdc_set_sleep_voltage(struct regulator_dev *dev,
					    int uV)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;
	u16 val;
	int ret = 0;

	if (buck ==2){
	return 0;
	}else{
	val = rk818_dcdc_select_min_voltage(dev,uV,uV,buck);
	ret = rk818_set_bits(rk818, rk818_BUCK_SET_SLP_VOL_REG(buck) , BUCK_VOL_MASK, val);
	}
	return ret;
}
static unsigned int rk818_dcdc_get_mode(struct regulator_dev *dev)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;
	u16 mask = 0x80;
	u16 val;
	val = rk818_reg_read(rk818, rk818_BUCK_SET_VOL_REG(buck));
        if (val < 0) {
                return val;
        }
	val=val & mask;
	if (val== mask)
		return REGULATOR_MODE_FAST;
	else
		return REGULATOR_MODE_NORMAL;

}
static int rk818_dcdc_set_mode(struct regulator_dev *dev, unsigned int mode)
{
	struct rk818 *rk818 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RK818_DCDC1;
	u16 mask = 0x80;
	switch(mode)
	{
	case REGULATOR_MODE_FAST:
		return rk818_set_bits(rk818, rk818_BUCK_SET_VOL_REG(buck), mask, mask);
	case REGULATOR_MODE_NORMAL:
		return rk818_set_bits(rk818, rk818_BUCK_SET_VOL_REG(buck), mask, 0);
	default:
		printk("error:pmu_rk818 only powersave and pwm mode\n");
		return -EINVAL;
	}

}
static int rk818_dcdc_set_voltage_time_sel(struct regulator_dev *dev,   unsigned int old_selector,
				     unsigned int new_selector)
{
	int old_volt, new_volt;
	
	old_volt = rk818_dcdc_list_voltage(dev, old_selector);
	if (old_volt < 0)
		return old_volt;
	
	new_volt = rk818_dcdc_list_voltage(dev, new_selector);
	if (new_volt < 0)
		return new_volt;

	return DIV_ROUND_UP(abs(old_volt - new_volt)*2, 2500);
}

static struct regulator_ops rk818_dcdc_ops = { 
	.set_voltage = rk818_dcdc_set_voltage,
	.get_voltage = rk818_dcdc_get_voltage,
	.list_voltage= rk818_dcdc_list_voltage,
	.is_enabled = rk818_dcdc_is_enabled,
	.enable = rk818_dcdc_enable,
	.disable = rk818_dcdc_disable,
	.get_mode = rk818_dcdc_get_mode,
	.set_mode = rk818_dcdc_set_mode,
	.set_suspend_voltage = rk818_dcdc_set_sleep_voltage,
	.set_voltage_time_sel = rk818_dcdc_set_voltage_time_sel,
};
static struct regulator_desc regulators[] = {

        {
		.name = "RK818_DCDC1",
		.id = 0,
		.ops = &rk818_dcdc_ops,
		.n_voltages = ARRAY_SIZE(buck_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_DCDC2",
		.id = 1,
		.ops = &rk818_dcdc_ops,
		.n_voltages = ARRAY_SIZE(buck_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_DCDC3",
		.id = 2,
		.ops = &rk818_dcdc_ops,
		.n_voltages = ARRAY_SIZE(buck4_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_DCDC4",
		.id = 3,
		.ops = &rk818_dcdc_ops,
		.n_voltages = ARRAY_SIZE(buck4_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	{
		.name = "RK818_LDO1",
		.id =4,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_LDO2",
		.id = 5,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_LDO3",
		.id = 6,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo3_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_LDO4",
		.id = 7,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	{
		.name = "RK818_LDO5",
		.id =8,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_LDO6",
		.id = 9,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo6_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_LDO7",
		.id = 10,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo6_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_LDO8",
		.id = 11,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RK818_LDO9",
		.id = 12,
		.ops = &rk818_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	
};

/*
 *
 */
 int rk818_i2c_read(struct rk818 *rk818, char reg, int count,u8 *dest)
{
	struct i2c_client *i2c = rk818->i2c;

      int ret;
    struct i2c_adapter *adap;
    struct i2c_msg msgs[2];

    if(!i2c)
		return ret;

	if (count != 1)
		return -EIO;  
  
    adap = i2c->adapter;		
    
    msgs[0].addr = i2c->addr;
    msgs[0].buf = &reg;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].scl_rate = 200*1000;
    
    msgs[1].buf = dest;
    msgs[1].addr = i2c->addr;
    msgs[1].flags =  I2C_M_RD;
    msgs[1].len = count;
    msgs[1].scl_rate = RK818_I2C_ADDR_RATE;

    ret = i2c_transfer(adap, msgs, 2);

	DBG("***run in %s %x  % x\n",__FUNCTION__,i2c->addr,msgs[0].buf);
    return 0;
}

int rk818_i2c_write(struct rk818 *rk818, char reg, int count,  const u8 src)
{
	int ret=-1;
	struct i2c_client *i2c = rk818->i2c;
	struct i2c_adapter *adap;
	struct i2c_msg msg;
	char tx_buf[2];

	if(!i2c)
		return ret;
	if (count != 1)
		return -EIO;
    
	adap = i2c->adapter;		
	tx_buf[0] = reg;
	tx_buf[1] = src;
	
	msg.addr = i2c->addr;
	msg.buf = &tx_buf[0];
	msg.len = 1 +1;
	msg.flags = i2c->flags;   
	msg.scl_rate = RK818_I2C_ADDR_RATE;	

	ret = i2c_transfer(adap, &msg, 1);
	return ret;	
}

u8 rk818_reg_read(struct rk818 *rk818, u8 reg)
{
	u8 val = 0;

	mutex_lock(&rk818->io_lock);

	rk818_i2c_read(rk818, reg, 1, &val);

	DBG("reg read 0x%02x -> 0x%02x\n", (int)reg, (unsigned)val&0xff);

	mutex_unlock(&rk818->io_lock);

	return val & 0xff;	
}
EXPORT_SYMBOL_GPL(rk818_reg_read);

int rk818_reg_write(struct rk818 *rk818, u8 reg, u8 val)
{
	int err =0;

	mutex_lock(&rk818->io_lock);

	err = rk818_i2c_write(rk818, reg, 1,val);
	if (err < 0)
		dev_err(rk818->dev, "Write for reg 0x%x failed\n", reg);

	mutex_unlock(&rk818->io_lock);
	return err;
}
EXPORT_SYMBOL_GPL(rk818_reg_write);

 int rk818_set_bits(struct rk818 *rk818, u8 reg, u8 mask, u8 val)
{
	u8 tmp;
	int ret;

	mutex_lock(&rk818->io_lock);

	ret = rk818_i2c_read(rk818, reg, 1, &tmp);
	DBG("1 reg read 0x%02x -> 0x%02x\n", (int)reg, (unsigned)tmp&0xff);
	tmp = (tmp & ~mask) | val;
	if (ret == 0) {
		ret = rk818_i2c_write(rk818, reg, 1, tmp);
		DBG("reg write 0x%02x -> 0x%02x\n", (int)reg, (unsigned)val&0xff);
	}
	rk818_i2c_read(rk818, reg, 1, &tmp);
	DBG("2 reg read 0x%02x -> 0x%02x\n", (int)reg, (unsigned)tmp&0xff);
	mutex_unlock(&rk818->io_lock);

	return 0;//ret;	
}
EXPORT_SYMBOL_GPL(rk818_set_bits);

int rk818_clear_bits(struct rk818 *rk818, u8 reg, u8 mask)
{
	u8 data;
	int err;

	mutex_lock(&rk818->io_lock);
	err = rk818_i2c_read(rk818, reg, 1, &data);
	if (err <0) {
		dev_err(rk818->dev, "read from reg %x failed\n", reg);
		goto out;
	}

	data &= ~mask;
	err = rk818_i2c_write(rk818, reg, 1, data);
	if (err <0)
		dev_err(rk818->dev, "write to reg %x failed\n", reg);

out:
	mutex_unlock(&rk818->io_lock);
	return err;
}
EXPORT_SYMBOL_GPL(rk818_clear_bits);

int rk818_bulk_read(struct rk818 *rk818, u8 reg,
		     int count, u8 *buf)
{
	int ret;
                    
#if defined(CONFIG_MFD_RK610)    
	int i;             //Solve communication conflict when rk610 and rk818 on the same i2c 

	mutex_lock(&rk818->io_lock);
	for(i=0; i<count; i++){
		ret = rk818_reg_read(rk818, reg+i);
		if(ret < 0){
			printk("%s: failed read reg 0x%0x, ret = %d\n", __FUNCTION__, reg+i, ret);
			mutex_unlock(&rk818->io_lock);
			return ret;
		}else{
			buf[i] = ret & 0x000000FF;
		}
	}
	mutex_unlock(&rk818->io_lock);
#else
	mutex_lock(&rk818->io_lock);
	
	ret = rk818->read(rk818, reg, count, buf);

	mutex_unlock(&rk818->io_lock);
#endif
	return 0;

}
EXPORT_SYMBOL_GPL(rk818_bulk_read);

int rk818_bulk_write(struct rk818 *rk818, u8 reg,
		     int count, u8 *buf)
{
	int ret;
	
#if defined(CONFIG_MFD_RK610)    
	int i;       // //Solve communication conflict when rk610 and 808 on the same i2c 

	mutex_lock(&rk818->io_lock);
	for(i=0; i<count; i++){
		ret = rk818_reg_write(rk818, reg+i, buf[i]);
		if(ret < 0){
			printk("%s: failed write reg=0x%0x, val=0x%0x, ret = %d\n", __FUNCTION__, reg+i, buf[i], ret);
			mutex_unlock(&rk818->io_lock);
			return ret;
		}
	}
	mutex_unlock(&rk818->io_lock);
#else
	mutex_lock(&rk818->io_lock);
	
	ret = rk818->write(rk818, reg, count, buf);

	mutex_unlock(&rk818->io_lock);
#endif
	return 0;

}
EXPORT_SYMBOL_GPL(rk818_bulk_write);


#if 1
static ssize_t rk818_test_store(struct kobject *kobj, struct kobj_attribute *attr,
                                const char *buf, size_t n)
{
    u32 getdata[8];
    u16 regAddr;
    u8 data;
    char cmd;
    const char *buftmp = buf;
    struct rk818 *rk818 = g_rk818;
    /**
     * W Addr(8Bit) regAddr(8Bit) data0(8Bit) data1(8Bit) data2(8Bit) data3(8Bit)
     * 		:data can be less than 4 byte
     * R regAddr(8Bit)
     * C gpio_name(poweron/powerhold/sleep/boot0/boot1) value(H/L)
     */
        regAddr = (u16)(getdata[0] & 0xff);
	 if (strncmp(buf, "start", 5) == 0) {
        

    } else if (strncmp(buf, "stop", 4== 0) ){
    
    } else{
        sscanf(buftmp, "%c ", &cmd);
        printk("------zhangqing: get cmd = %c\n", cmd);
        switch(cmd) {

        case 'w':
		sscanf(buftmp, "%c %x %x ", &cmd, &getdata[0],&getdata[1]);
		 regAddr = (u16)(getdata[0] & 0xff);
                data = (u8)(getdata[1] & 0xff);
                printk("get value = %x\n", data);

             rk818_i2c_write(rk818, regAddr, 1, data);
	     rk818_i2c_read(rk818, regAddr, 1, &data);
	     printk("%x   %x\n", getdata[1],data);

            break;

        case 'r':
            sscanf(buftmp, "%c %x ", &cmd, &getdata[0]);
            printk("CMD : %c %x\n", cmd, getdata[0]);

            regAddr = (u16)(getdata[0] & 0xff);
            rk818_i2c_read(rk818, regAddr, 1, &data);
		printk("%x %x\n", getdata[0],data);

            break;

        default:
            printk("Unknown command\n");
            break;
        }
}
    return n;

}
static ssize_t rk818_test_show(struct kobject *kobj, struct kobj_attribute *attr,
                               char *buf)
{
   char *s = buf;
    buf = "hello";
    return sprintf(s, "%s\n", buf);

}

static struct kobject *rk818_kobj;
struct rk818_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf);
	ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t n);
};

static struct rk818_attribute rk818_attrs[] = {
	/*     node_name	permision		show_func	store_func */
	__ATTR(rk818_test,	S_IRUGO | S_IWUSR,	rk818_test_show,	rk818_test_store),
};
#endif

static int __devinit setup_regulators(struct rk818 *rk818, struct rk818_platform_data *pdata)
{	
	int i, err;

	rk818->num_regulators = pdata->num_regulators;
	rk818->rdev = kcalloc(pdata->num_regulators,
			       sizeof(struct regulator_dev *), GFP_KERNEL);
	if (!rk818->rdev) {
		return -ENOMEM;
	}
	/* Instantiate the regulators */
	for (i = 0; i < pdata->num_regulators; i++) {
		int id = pdata->regulators[i].id;
		rk818->rdev[i] = regulator_register(&regulators[id],
			rk818->dev, pdata->regulators[i].initdata, rk818);
/*
		if (IS_ERR(rk818->rdev[i])) {
			err = PTR_ERR(rk818->rdev[i]);
			dev_err(rk818->dev, "regulator init failed: %d\n",
				err);
			goto error;
		}*/
	}

	return 0;
error:
	while (--i >= 0)
		regulator_unregister(rk818->rdev[i]);
	kfree(rk818->rdev);
	rk818->rdev = NULL;
	return err;
}

extern void rk28_send_wakeup_key(void);
static irqreturn_t rk818_vbat_lo_irq(int irq, void *data)
{
        printk("rk818 vbat low %s:irq=%d\n",__func__,irq);
	rk818_set_bits(g_rk818,0x4c,(0x1 << 1),(0x1 <<1));
	rk28_send_wakeup_key();
        return IRQ_HANDLED;
}

int rk818_device_shutdown(void)
{
	int ret;
	int err = -1;
	struct rk818 *rk818 = g_rk818;
	
	printk("%s\n",__func__);

	ret = rk818_reg_read(rk818,RK818_DEVCTRL_REG);
	ret = rk818_set_bits(rk818, RK818_DEVCTRL_REG,(0x1<<0),(0x1<<0));
//	ret = rk818_set_bits(rk818, RK818_DEVCTRL_REG,(0x1<<4),(0x1<<4));
	if (ret < 0) {
		printk("rk818 power off error!\n");
		return err;
	}
	return 0;	
}
EXPORT_SYMBOL_GPL(rk818_device_shutdown);

__weak void  rk818_device_suspend(void) {}
__weak void  rk818_device_resume(void) {}
#ifdef CONFIG_PM
static int rk818_suspend(struct i2c_client *i2c, pm_message_t mesg)
{		
	rk818_device_suspend();
	return 0;
}

static int rk818_resume(struct i2c_client *i2c)
{
	rk818_device_resume();
	return 0;
}
#else
static int rk818_suspend(struct i2c_client *i2c, pm_message_t mesg)
{		
	return 0;
}

static int rk818_resume(struct i2c_client *i2c)
{
	return 0;
}
#endif


#ifdef CONFIG_HAS_EARLYSUSPEND
__weak void rk818_early_suspend(struct early_suspend *h) {}
__weak void rk818_late_resume(struct early_suspend *h) {}
#endif

 
static int __devinit rk818_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	struct rk818 *rk818;	
	struct rk818_platform_data *pdata = i2c->dev.platform_data;
	int ret;
	
	printk("%s,line=%d\n", __func__,__LINE__);

	rk818 = kzalloc(sizeof(struct rk818), GFP_KERNEL);
	if (rk818 == NULL) {
		ret = -ENOMEM;		
		goto err;
	}
	rk818->i2c = i2c;
	rk818->dev = &i2c->dev;
	i2c_set_clientdata(i2c, rk818);
	rk818->read = rk818_i2c_read;
	rk818->write = rk818_i2c_write;
	mutex_init(&rk818->io_lock);	

	ret = rk818_reg_read(rk818,0x2f);
	if ((ret < 0) || (ret == 0xff)){
		printk("The device is not rk818\n");
		return 0;
	}

	ret = mfd_add_devices(rk818->dev, -1,
			      rk818s, ARRAY_SIZE(rk818s),
			      NULL, 0);
	if (ret < 0)
		goto err;

	if (pdata) {
		ret = setup_regulators(rk818, pdata);
		if (ret < 0)		
			goto err;
	} else
		dev_warn(rk818->dev, "No platform init data supplied\n");

	pdata->pre_init(rk818);

	ret = rk818_irq_init(rk818, pdata->irq, pdata);
	if (ret < 0)
		goto err;
	/********************vbat low int**************/
	 ret = request_threaded_irq(rk818->irq_base + RK818_IRQ_VB_LO, NULL, rk818_vbat_lo_irq,
                                   IRQF_TRIGGER_RISING, "rk818_vbatlow",
                                   rk818);
        if (ret != 0) {
                dev_err(rk818->dev, "Failed to request periodic IRQ %d: %d\n",
                        rk818->irq_base + RK818_IRQ_VB_LO, ret);

        }

	/*********************************************/
	g_rk818 = rk818;
	pdata->set_init(rk818);

	#ifdef CONFIG_HAS_EARLYSUSPEND
	rk818->rk818_suspend.suspend = rk818_early_suspend,
	rk818->rk818_suspend.resume = rk818_late_resume,
	rk818->rk818_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1,
	register_early_suspend(&rk818->rk818_suspend);
	#endif

	#if 1
	int i =0;
	rk818_kobj = kobject_create_and_add("rk818", NULL);
	if (!rk818_kobj)
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(rk818_attrs); i++) {
		ret = sysfs_create_file(rk818_kobj, &rk818_attrs[i].attr);
		if (ret != 0) {
			printk("create index %d error\n", i);
			return ret;
		}
	}
	#endif
	
	return 0;

err:
	return ret;	

}

static int __devexit rk818_i2c_remove(struct i2c_client *i2c)
{
	struct rk818 *rk818 = i2c_get_clientdata(i2c);
	int i;

	for (i = 0; i < rk818->num_regulators; i++)
		if (rk818->rdev[i])
			regulator_unregister(rk818->rdev[i]);
	kfree(rk818->rdev);
	i2c_set_clientdata(i2c, NULL);
	kfree(rk818);

	return 0;
}

static const struct i2c_device_id rk818_i2c_id[] = {
       { "rk818", 0 },
       { }
};

MODULE_DEVICE_TABLE(i2c, rk818_i2c_id);

static struct i2c_driver rk818_i2c_driver = {
	.driver = {
		.name = "rk818",
		.owner = THIS_MODULE,
	},
	.probe    = rk818_i2c_probe,
	.remove   = __devexit_p(rk818_i2c_remove),
	.id_table = rk818_i2c_id,
	#ifdef CONFIG_PM
	.suspend	= rk818_suspend,
	.resume		= rk818_resume,
	#endif
};

static int __init rk818_module_init(void)
{
	int ret;
	ret = i2c_add_driver(&rk818_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register I2C driver: %d\n", ret);
	return ret;
}
//module_init(rk818_module_init);
//subsys_initcall(rk818_module_init);
//rootfs_initcall(rk818_module_init);
subsys_initcall_sync(rk818_module_init);

static void __exit rk818_module_exit(void)
{
	i2c_del_driver(&rk818_i2c_driver);
}
module_exit(rk818_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangqing <zhangqing@rock-chips.com>");
MODULE_DESCRIPTION("rk818 PMIC driver");

