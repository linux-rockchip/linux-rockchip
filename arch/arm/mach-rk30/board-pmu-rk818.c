#include <linux/regulator/machine.h>
#include <linux/mfd/rk818.h>
#include <mach/sram.h>
#include <linux/platform_device.h>

#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/board.h>

#ifdef CONFIG_MFD_RK818

static int rk818_pre_init(struct rk818 *rk818)
{
	int ret,val;
	 printk("%s,line=%d\n", __func__,__LINE__);

	val = rk818_reg_read(rk818,RK818_THERMAL_REG);
        val |= 0x0c;//set hot temperature
        ret = rk818_reg_write(rk818,RK818_THERMAL_REG,val);
        if (ret < 0) {
                printk(KERN_ERR "Unable to write RK818_THERMAL_REG reg\n");
                return ret;
        }

	val = rk818_reg_read(rk818,0xa1);
	val |= 0xc9;//set usb input limit vol and current
	ret = rk818_reg_write(rk818,0xa1,val);
	if (ret < 0) {
                printk(KERN_ERR "Unable to write 0xa1 reg\n");
                return ret;
        }
	
	 #if 0
	 /***********set ILIM ************/
	val = rk818_reg_read(rk818,RK818_BUCK3_CONFIG_REG);
	val &= (~(0x7 <<0));
	val |= (0x2 <<0);
	ret = rk818_reg_write(rk818,RK818_BUCK3_CONFIG_REG,val);
	if (ret < 0) {
                printk(KERN_ERR "Unable to write RK818_BUCK3_CONFIG_REG reg\n");
                return ret;
        }

	val = rk818_reg_read(rk818,RK818_BUCK4_CONFIG_REG);
	val &= (~(0x7 <<0));
	val |= (0x3 <<0);
	ret = rk818_reg_write(rk818,RK818_BUCK4_CONFIG_REG,val);
	if (ret < 0) {
                printk(KERN_ERR "Unable to write RK818_BUCK4_CONFIG_REG reg\n");
                return ret;
        }
	
	val = rk818_reg_read(rk818,RK818_BOOST_CONFIG_REG);
	val &= (~(0x7 <<0));
	val |= (0x1 <<0);
	ret = rk818_reg_write(rk818,RK818_BOOST_CONFIG_REG,val);
	if (ret < 0) {
                printk(KERN_ERR "Unable to write RK818_BOOST_CONFIG_REG reg\n");
                return ret;
        }
	/*****************************************/
	/***********set buck OTP function************/
	ret = rk818_reg_write(rk818,0x6f,0x5a);
	if (ret < 0) {
                printk(KERN_ERR "Unable to write 0x6f reg\n");
                return ret;
        }
	
	ret = rk818_reg_write(rk818,0x91,0x80);
	if (ret < 0) {
                printk(KERN_ERR "Unable to write 0x91 reg\n");
                return ret;
        }

        ret = rk818_reg_write(rk818,0x92,0x55);
	 if (ret <0) {
                printk(KERN_ERR "Unable to write 0x92 reg\n");
                return ret;
        }
	/*****************************************/
	#endif
	/***********set buck 12.5mv/us ************/
	val = rk818_reg_read(rk818,RK818_BUCK1_CONFIG_REG);
	val &= (~(0x3 <<3));
	val |= (0x3 <<0);
	ret = rk818_reg_write(rk818,RK818_BUCK1_CONFIG_REG,val);
	if (ret < 0) {
                printk(KERN_ERR "Unable to write RK818_BUCK1_CONFIG_REG reg\n");
                return ret;
        }

	val = rk818_reg_read(rk818,RK818_BUCK2_CONFIG_REG);
       val &= (~(0x3 <<3));
	val |= (0x3 <<0);
       ret = rk818_reg_write(rk818,RK818_BUCK2_CONFIG_REG,val);
	 if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_BUCK2_CONFIG_REG reg\n");
                return ret;
        }
	/*****************************************/
	
	/*******set boost voltage***********/
	val = rk818_reg_read(rk818,RK818_BOOST_LDO9_ON_VSEL_REG);
       val &=( ~(0x7 << 5));    //
	val |= (0x3 << 5);    //set boost 5V
       ret = rk818_reg_write(rk818,RK818_BOOST_LDO9_ON_VSEL_REG,val);
         if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_BOOST_LDO9_ON_VSEL_REG reg\n");
                return ret;
	}
	/****************************************/

	/*******enable switch and boost***********/
	val = rk818_reg_read(rk818,RK818_DCDC_EN_REG);
       val |= (0x3 << 5);    //enable switch1/2
	val |= ((0x1 << 4));    //enable boost
       ret = rk818_reg_write(rk818,RK818_DCDC_EN_REG,val);
         if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_DCDC_EN_REG reg\n");
                return ret;
         }
	/****************************************/

	/****************set vbat low **********/
	val = rk818_reg_read(rk818,RK818_VB_MON_REG);
       val &=(~(VBAT_LOW_VOL_MASK | VBAT_LOW_ACT_MASK));
       val |= (RK818_VBAT_LOW_2V9 | EN_VBAT_LOW_IRQ);
       ret = rk818_reg_write(rk818,RK818_VB_MON_REG,val);
         if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_VB_MON_REG reg\n");
                return ret;
        }
	/**************************************/
	/********set dcdc/ldo/switch off when in sleep******/
	 val = rk818_reg_read(rk818,RK818_SLEEP_SET_OFF_REG1);
//     val |= (0x3<<5);   
        ret = rk818_reg_write(rk818,RK818_SLEEP_SET_OFF_REG1,val);
         if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_SLEEP_SET_OFF_REG1 reg\n");
                return ret;
        }
	val = rk818_reg_read(rk818,RK818_SLEEP_SET_OFF_REG2);
//    val |= (0x1<<4);     
        ret = rk818_reg_write(rk818,RK818_SLEEP_SET_OFF_REG2,val);
         if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_SLEEP_SET_OFF_REG2 reg\n");
                return ret;
        }
	/**************************************************/
	/**********mask int****************/
	 val = rk818_reg_read(rk818,RK818_INT_STS_MSK_REG1);
        val |= (0x1<<0); //mask vout_lo_int    
        ret = rk818_reg_write(rk818,RK818_INT_STS_MSK_REG1,val);
         if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_INT_STS_MSK_REG1 reg\n");
                return ret;
        }
	val = rk818_reg_read(rk818,RK818_INT_STS_MSK_REG2);
        val |= 0xfc; //mask vout_lo_int    
        ret = rk818_reg_write(rk818,RK818_INT_STS_MSK_REG2,val);
         if (ret <0) {
                printk(KERN_ERR "Unable to write RK818_INT_STS_MSK_REG1 reg\n");
                return ret;
        }
	/**********************************/
	printk("%s,line=%d\n", __func__,__LINE__);
}
static int rk818_set_init(struct rk818 *rk818)
{
	struct regulator *dcdc;
	struct regulator *ldo;
	int i = 0;
	printk("%s,line=%d\n", __func__,__LINE__);

	#ifndef CONFIG_RK_CONFIG
	g_pmic_type = PMIC_TYPE_RK818;
	#endif
	printk("%s:g_pmic_type=%d\n",__func__,g_pmic_type);
	
	for(i = 0; i < ARRAY_SIZE(rk818_dcdc_info); i++)
	{

                if(rk818_dcdc_info[i].min_uv == 0 && rk818_dcdc_info[i].max_uv == 0)
                        continue;
	        dcdc =regulator_get(NULL, rk818_dcdc_info[i].name);
	        regulator_set_voltage(dcdc, rk818_dcdc_info[i].min_uv, rk818_dcdc_info[i].max_uv);
		 regulator_set_suspend_voltage(dcdc, rk818_dcdc_info[i].suspend_vol);
	        regulator_enable(dcdc);
	        printk("%s  %s =%duV end\n", __func__,rk818_dcdc_info[i].name, regulator_get_voltage(dcdc));
	        regulator_put(dcdc);
	        udelay(100);
	}
	
	for(i = 0; i < ARRAY_SIZE(rk818_ldo_info); i++)
	{
                if(rk818_ldo_info[i].min_uv == 0 && rk818_ldo_info[i].max_uv == 0)
                        continue;
	        ldo =regulator_get(NULL, rk818_ldo_info[i].name);

	        regulator_set_voltage(ldo, rk818_ldo_info[i].min_uv, rk818_ldo_info[i].max_uv);
		 regulator_set_suspend_voltage(ldo, rk818_ldo_info[i].suspend_vol);
	        regulator_enable(ldo);
	        printk("%s  %s =%duV end\n", __func__,rk818_ldo_info[i].name, regulator_get_voltage(ldo));
	        regulator_put(ldo);
	}

	#ifdef CONFIG_RK_CONFIG
	if(sram_gpio_init(get_port_config(pmic_slp).gpio, &pmic_sleep) < 0){
		printk(KERN_ERR "sram_gpio_init failed\n");
		return -EINVAL;
	}
	if(port_output_init(pmic_slp, 0, "pmic_slp") < 0){
		printk(KERN_ERR "port_output_init failed\n");
		return -EINVAL;
	}
	#else
	if(sram_gpio_init(PMU_POWER_SLEEP, &pmic_sleep) < 0){
		printk(KERN_ERR "sram_gpio_init failed\n");
		return -EINVAL;
	}
	gpio_request(PMU_POWER_SLEEP, "NULL");
	gpio_direction_output(PMU_POWER_SLEEP, GPIO_LOW);
		
	#endif
	
	printk("%s,line=%d END\n", __func__,__LINE__);
	
	
	return 0;
}

static struct regulator_consumer_supply rk818_buck1_supply[] = {
	{
		.supply = "rk818_dcdc1",
	},

	 {
                .supply = "vdd_cpu",
        },

};
static struct regulator_consumer_supply rk818_buck2_supply[] = {
	{
		.supply = "rk818_dcdc2",
	},	

	 {
                .supply = "vdd_core",
        },

};
static struct regulator_consumer_supply rk818_buck3_supply[] = {
	{
		.supply = "rk818_dcdc3",
	},
};

static struct regulator_consumer_supply rk818_buck4_supply[] = {
	{
		.supply = "rk818_dcdc4",
	},

};

static struct regulator_consumer_supply rk818_ldo1_supply[] = {
	{
		.supply = "rk818_ldo1",
	},
};
static struct regulator_consumer_supply rk818_ldo2_supply[] = {
	{
		.supply = "rk818_ldo2",
	},
};

static struct regulator_consumer_supply rk818_ldo3_supply[] = {
	{
		.supply = "rk818_ldo3",
	},
};
static struct regulator_consumer_supply rk818_ldo4_supply[] = {
	{
		.supply = "rk818_ldo4",
	},
};
static struct regulator_consumer_supply rk818_ldo5_supply[] = {
	{
		.supply = "rk818_ldo5",
	},
};
static struct regulator_consumer_supply rk818_ldo6_supply[] = {
	{
		.supply = "rk818_ldo6",
	},
};

static struct regulator_consumer_supply rk818_ldo7_supply[] = {
	{
		.supply = "rk818_ldo7",
	},
};
static struct regulator_consumer_supply rk818_ldo8_supply[] = {
	{
		.supply = "rk818_ldo8",
	},
};
static struct regulator_consumer_supply rk818_ldo9_supply[] = {
	{
		.supply = "rk818_ldo9",
	},
};

static struct regulator_init_data rk818_buck1 = {
	.constraints = {
		.name           = "RK818_DCDC1",
		.min_uV			= 700000,
		.max_uV			= 1500000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_buck1_supply),
	.consumer_supplies =  rk818_buck1_supply,
};

/* */
static struct regulator_init_data rk818_buck2 = {
	.constraints = {
		.name           = "RK818_DCDC2",
		.min_uV			= 700000,
		.max_uV			= 1500000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_buck2_supply),
	.consumer_supplies =  rk818_buck2_supply,
};

/* */
static struct regulator_init_data rk818_buck3 = {
	.constraints = {
		.name           = "RK818_DCDC3",
		.min_uV			= 1000000,
		.max_uV			= 1800000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_buck3_supply),
	.consumer_supplies =  rk818_buck3_supply,
};

static struct regulator_init_data rk818_buck4 = {
	.constraints = {
		.name           = "RK818_DCDC4",
		.min_uV			= 1800000,
		.max_uV			= 3300000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_buck4_supply),
	.consumer_supplies =  rk818_buck4_supply,
};

static struct regulator_init_data rk818_ldo1 = {
	.constraints = {
		.name           = "RK818_LDO1",
		.min_uV			= 1800000,
		.max_uV			= 3400000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo1_supply),
	.consumer_supplies =  rk818_ldo1_supply,
};

/* */
static struct regulator_init_data rk818_ldo2 = {
	.constraints = {
		.name           = "RK818_LDO2",
		.min_uV			= 1800000,
		.max_uV			= 3400000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo2_supply),
	.consumer_supplies =  rk818_ldo2_supply,
};

/* */
static struct regulator_init_data rk818_ldo3 = {
	.constraints = {
		.name           = "RK818_LDO3",
		.min_uV			= 800000,
		.max_uV			= 2500000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo3_supply),
	.consumer_supplies =  rk818_ldo3_supply,
};

/* */
static struct regulator_init_data rk818_ldo4 = {
	.constraints = {
		.name           = "RK818_LDO4",
		.min_uV			= 1800000,
		.max_uV			= 3400000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo4_supply),
	.consumer_supplies =  rk818_ldo4_supply,
};

static struct regulator_init_data rk818_ldo5 = {
	.constraints = {
		.name           = "RK818_LDO5",
		.min_uV			= 1800000,
		.max_uV			= 3400000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo5_supply),
	.consumer_supplies =  rk818_ldo5_supply,
};

static struct regulator_init_data rk818_ldo6 = {
	.constraints = {
		.name           = "RK818_LDO6",
		.min_uV			= 800000,
		.max_uV			= 2500000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo6_supply),
	.consumer_supplies =  rk818_ldo6_supply,
};

static struct regulator_init_data rk818_ldo7 = {
	.constraints = {
		.name           = "RK818_LDO7",
		.min_uV			= 800000,
		.max_uV			= 2500000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo7_supply),
	.consumer_supplies =  rk818_ldo7_supply,
};

static struct regulator_init_data rk818_ldo8 = {
	.constraints = {
		.name           = "RK818_LDO8",
		.min_uV			= 1800000,
		.max_uV			= 3400000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo8_supply),
	.consumer_supplies =  rk818_ldo8_supply,
};

static struct regulator_init_data rk818_ldo9 = {
	.constraints = {
		.name           = "RK818_LDO9",
		.min_uV			= 1800000,
		.max_uV			= 3400000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(rk818_ldo9_supply),
	.consumer_supplies =  rk818_ldo9_supply,
};

struct rk818_regulator_subdev rk818_regulator_subdev_id[] = {
	{
		.id=0,
		.initdata=&rk818_buck1,		
	 },

	{
		.id=1,
		.initdata=&rk818_buck2,		
	 },
	{
		.id=2,
		.initdata=&rk818_buck3,		
	 },
        {
		.id=3,
		.initdata=&rk818_buck4,		
	 },

	{
		.id=4,
		.initdata=&rk818_ldo1,		
	 },

	{
		.id=5,
		.initdata=&rk818_ldo2,		
	 },

	{
		.id=6,
		.initdata=&rk818_ldo3,		
	 },

	{
		.id=7,
		.initdata=&rk818_ldo4,		
	 },

	{
		.id=8,
		.initdata=&rk818_ldo5,		
	 },

	{
		.id=9,
		.initdata=&rk818_ldo6,		
	 },

	{
		.id=10,
		.initdata=&rk818_ldo7,		
	 },

	{
		.id=11,
		.initdata=&rk818_ldo8,		
	 },
	 {
		.id=12,
		.initdata=&rk818_ldo9,		
	 },
};
static  uint16_t    ocv_table[] = {
	3300, 3414, 3488, 3510, 3577,
	3624, 3656, 3680, 3699, 3716,
	3732, 3754, 3783, 3831, 3870,
	3906, 3945, 3991, 4040, 4090,
	4147,};
	
struct ocv_config  ocv_cfg = {
	.voltage_diff = 75,
	.current_diff = 30,

	.sleep_enter_current = 100,
	.sleep_enter_samples = 3,
	.sleep_exit_current = 150,
	.sleep_exit_samples=3,

	.relax_period = 300,

	.flat_zone_low = 40,
	.flat_zone_high = 50,
	.max_ocv_discharge = 1300,
};

struct cell_config cell_cfg = {
	.cc_polarity = true,
	.cc_out = true,
	.ocv_below_edv1 = false,

	.cc_voltage = 4200,
	.cc_current = 400,

	.design_capacity = 4131,
	.design_qmax =4140,

	.r_sense = 20,

	.qmax_adjust = 1,
	.fcc_adjust = 2,

	.max_overcharge = 100,
	.electronics_load = 2,

	.max_increment = 150,
	.max_decrement = 150,
	.low_temp = 119,
	.deep_dsg_voltage = 30,
	.max_dsg_estimate = 300,
	.light_load = 100,
	.near_full = 500,
	.cycle_threshold = 3500,
	.recharge = 300,
	.mode_switch_capacity = 5,
	.call_period =2,

	.ocv = &ocv_cfg,
};

static struct battery_platform_data bci_data = {
        /* How often the fuel gauging algorithm runs */
        .monitoring_interval            = 10,
        /* The max current for charger sets CHARGERUSB_VICHRG */
        .max_charger_currentmA          = 1500,
        /* The maximum charge voltage, sets CHARGERUSB_CTRLLIMIT1 */
        .max_charger_voltagemV          = 4260,
        /* The maximum battery voltage, sets CHARGERUSB_VOREG */
        .max_bat_voltagemV              = 4200,
        /* The termination current for charging, ITERM in CHARGERUSB_CTRL2 */
        .termination_currentmA          = 100,
        .battery_ocv   = ocv_table,
	 .ocv_size      = ARRAY_SIZE(ocv_table),
        /* The size of sense resistor for current measurements */
        .sense_resistor_mohm            = 20,
        .cell_cfg = &cell_cfg,
};

 struct rk818_platform_data rk818_data={
	.pre_init=rk818_pre_init,
	.set_init=rk818_set_init,
	.num_regulators=13,
	.regulators=rk818_regulator_subdev_id,
	.irq 	= (unsigned)RK818_HOST_IRQ,		
	.irq_base = IRQ_BOARD_BASE,
	.battery_data = &bci_data ,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
void rk818_early_suspend(struct early_suspend *h)
{
}

void rk818_late_resume(struct early_suspend *h)
{
}
#endif
#ifdef CONFIG_PM

void rk818_device_suspend(void)
{
}
void rk818_device_resume(void)
{
}       
#endif

void __sramfunc board_pmu_rk818_suspend(void)
{	
	//#ifdef CONFIG_CLK_SWITCH_TO_32K
//	 sram_gpio_set_value(pmic_sleep, GPIO_HIGH);  
	//#endif
}
void __sramfunc board_pmu_rk818_resume(void)
{
//	#ifdef CONFIG_CLK_SWITCH_TO_32K
 //	sram_gpio_set_value(pmic_sleep, GPIO_LOW);  
//	sram_32k_udelay(10000);
//	#endif
}

#endif




