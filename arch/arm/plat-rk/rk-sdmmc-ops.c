/***************************************************************************************************
 * arch/arm/palt-rk/rk-sdmmc-ops.c
 *
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * Description: define the gpio for SDMMC module on various platforms
 *
 * Author: Michael Xie
 * E-mail: xbw@rock-chips.com
 *
 * History:
 *     ver1.0 Unified function interface for new imoux-API, created at 2013-01-15
 *     ver1.1 add drive strength control and the setting of IO-voltage, created at 2013-01-29
 *
 **************************************************************************************************/

//use the new iomux-API
#if defined(CONFIG_MACH_RK30_PS2_VEHICLE)||defined(CONFIG_ARCH_RK2928)
    #define SDMMC_USE_NEW_IOMUX_API 0
#else
    #define SDMMC_USE_NEW_IOMUX_API 1
#endif


//define IO volatga
#if defined(CONFIG_ARCH_RK3066B)||defined(CONFIG_ARCH_RK3168)||defined(CONFIG_ARCH_RK3188)// ||defined(CONFIG_ARCH_RK319X)
#define SDMMC_SET_IO_VOLTAGE    1
#else
#define SDMMC_SET_IO_VOLTAGE    0
#endif

#if SDMMC_SET_IO_VOLTAGE
//GRF_IO_CON0                        0x0F4
//eMMC data[3:0],cmd,clk
#define SDMMC2_DRIVER_STRENGTH_2MA            (0x00 << 2)
#define SDMMC2_DRIVER_STRENGTH_4MA            (0x01 << 2)
#define SDMMC2_DRIVER_STRENGTH_8MA            (0x02 << 2)
#define SDMMC2_DRIVER_STRENGTH_12MA           (0x03 << 2)
#define SDMMC2_DRIVER_STRENGTH_MASK           (0x03 << 18)
//eMMC data4--data7
#define SDMMC2_D47_DRIVER_STRENGTH_2MA        (0x00 << 4)
#define SDMMC2_D47_DRIVER_STRENGTH_4MA        (0x01 << 4)
#define SDMMC2_D47_DRIVER_STRENGTH_8MA        (0x02 << 4)
#define SDMMC2_D47_DRIVER_STRENGTH_12MA       (0x03 << 4)
#define SDMMC2_D47_DRIVER_STRENGTH_MASK       (0x03 << 20)
//GRF_IO_CON2                        0x0FC
#define SDMMC0_DRIVER_STRENGTH_2MA            (0x00 << 6)
#define SDMMC0_DRIVER_STRENGTH_4MA            (0x01 << 6)
#define SDMMC0_DRIVER_STRENGTH_8MA            (0x02 << 6)
#define SDMMC0_DRIVER_STRENGTH_12MA           (0x03 << 6)
#define SDMMC0_DRIVER_STRENGTH_MASK           (0x03 << 22)

//GRF_IO_CON3                        0x100
#define SDMMC1_DRIVER_STRENGTH_2MA            (0x00 << 2)
#define SDMMC1_DRIVER_STRENGTH_4MA            (0x01 << 2)
#define SDMMC1_DRIVER_STRENGTH_8MA            (0x02 << 2)
#define SDMMC1_DRIVER_STRENGTH_12MA           (0x03 << 2)
#define SDMMC1_DRIVER_STRENGTH_MASK           (0x03 << 18)

//GRF_IO_CON4       0x104
//vccio0
#define SDMMC0_IO_VOLTAGE_33            (0x00 << 12)
#define SDMMC0_IO_VOLTAGE_18            (0x01 << 12)
#define SDMMC0_IO_VOLTAGE_MASK          (0x01 << 28)
//ap0
#define SDMMC1_IO_VOLTAGE_33            (0x00 << 8)
#define SDMMC1_IO_VOLTAGE_18            (0x01 << 8)
#define SDMMC1_IO_VOLTAGE_MASK          (0x01 << 24)
//flash_vc
#define SDMMC2_IO_VOLTAGE_33            (0x00 << 11)
#define SDMMC2_IO_VOLTAGE_18            (0x01 << 11)
#define SDMMC2_IO_VOLTAGE_MASK          (0x01 << 17)

#define SDMMC_write_grf_reg(addr, val)  __raw_writel(val, addr+RK30_GRF_BASE)
#define SDMMC_mask_grf_reg(addr, msk, val)	write_grf_reg(addr,(val)|((~(msk))&read_grf_reg(addr)))
#else
#define SDMMC_write_grf_reg(addr, val)  
#define SDMMC_mask_grf_reg(addr, msk, val)	
#endif

#define SDMMC_read_grf_reg(addr) __raw_readl(addr+RK30_GRF_BASE)

//emmc flash select used for iomux, IO_FLASH_DATA[7:0],IO_FLASH_WP are selected for emmc insted of flash.
#ifdef CONFIG_ARCH_RK319X
#define RK_EMMC_FLAHS_SEL   (1<<6)
#define RK_EMMC_FLASH_REGISTER (GRF_SOC_CON3)
#else
#define RK_EMMC_FLAHS_SEL	(1<<11)  
#define RK_EMMC_FLASH_REGISTER (GRF_SOC_CON0)
#endif

int rk31sdk_wifi_voltage_select(void)
{
    int voltage;
    int voltage_flag = 0;

    voltage = rk31sdk_get_sdio_wifi_voltage();
   
     if(voltage >= 2700)
        voltage_flag = 0;
     else if(voltage <= 2000)
        voltage_flag = 1;
     else
        voltage_flag = 1;

    return voltage_flag;
}


#if defined(CONFIG_ARCH_RK3066B) || defined(CONFIG_ARCH_RK3188)
/*
* define the gpio for sdmmc0
*/
static struct rksdmmc_gpio_board rksdmmc0_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN3_PA2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_clkout",
            .fgpio      = GPIO3_A2,
            .fmux       = MMC0_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN3_PA3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_cmd",
            .fgpio      = GPIO3_A3,
            .fmux       = MMC0_CMD,
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN3_PA4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d0",
            .fgpio      = GPIO3_A4,
            .fmux       = MMC0_D0,
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN3_PA5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d1",
            .fgpio      = GPIO3_A5,
            .fmux       = MMC0_D1,
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN3_PA6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d2",
            .fgpio      = GPIO3_A6,
            .fmux       = MMC0_D2,
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN3_PA7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d3",
            .fgpio      = GPIO3_A7,
            .fmux       = MMC0_D3,
        },
    }, 

               
    .power_en_gpio      = {   
#if defined(RK29SDK_SD_CARD_PWR_EN) && (INVALID_GPIO != RK29SDK_SD_CARD_PWR_EN)
                    .io             = RK29SDK_SD_CARD_PWR_EN,
                    .enable         = RK29SDK_SD_CARD_PWR_EN_LEVEL,
        #ifdef RK29SDK_SD_CARD_PWR_EN_PIN_NAME
                    .iomux          = {
                        .name       = RK29SDK_SD_CARD_PWR_EN_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO
                        .fgpio      = RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX
                        .fmux       = RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX,
            #endif
                    },
        #endif
#else
                    .io             = INVALID_GPIO,
                    .enable         = GPIO_LOW,
#endif
                }, 
            
        .detect_irq       = {
#if defined(RK29SDK_SD_CARD_DETECT_N) && (INVALID_GPIO != RK29SDK_SD_CARD_DETECT_N)  
                    .io             = RK29SDK_SD_CARD_DETECT_N,
                    .enable         = RK29SDK_SD_CARD_INSERT_LEVEL,
        #ifdef RK29SDK_SD_CARD_DETECT_PIN_NAME
                    .iomux          = {
                        .name       = RK29SDK_SD_CARD_DETECT_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO
                        .fgpio      = RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FMUX
                        .fmux       = RK29SDK_SD_CARD_DETECT_IOMUX_FMUX,
            #endif
                    },
        #endif
#else
                    .io             = INVALID_GPIO,
                    .enable         = GPIO_LOW,
#endif            
    },

};


/*
* define the gpio for sdmmc1
*/
static struct rksdmmc_gpio_board rksdmmc1_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN3_PC5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_clkout",
            .fgpio      = GPIO3_C5,
            .fmux       = MMC1_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN3_PC0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_cmd",
            .fgpio      = GPIO3_C0,
            .fmux       = MMC1_CMD,
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN3_PC1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d0",
            .fgpio      = GPIO3_C1,
            .fmux       = MMC1_D0,
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN3_PC2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d1",
            .fgpio      = GPIO3_C2,
            .fmux       = MMC1_D1,
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN3_PC3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d2",
            .fgpio      = GPIO3_C3,
            .fmux       = MMC1_D2,
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN3_PC4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d3",
            .fgpio      = GPIO3_C4,
            .fmux       = MMC1_D3,
        },
    }, 
};

#if defined(CONFIG_SDMMC2_RK29)
/*
* define the gpio for sdmmc2, mainly used to support eMMC.
*/
static struct rksdmmc_gpio_board rksdmmc2_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN0_PD0,//RK30_PIN2_PA7,
        .enable         = GPIO_HIGH,
     .iomux          = {
            .name       = "mmc2_clkout",
            .fgpio      = GPIO0_D0,//GPIO2_A7,
            .fmux       = EMMC_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN0_PD2,//RK30_PIN1_PC6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_cmd",
            .fgpio      = GPIO0_D2,//GPIO1_C6,
            .fmux       = EMMC_CMD,
        },
    },      
#if 0//def CONFIG_ARCH_RK3026
    //need no define data0-data
#endif

    .rstnout_gpio       = {
        .io             = RK30_PIN0_PD3,//RK30_PIN1_PC7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_rstn",
            .fgpio      = GPIO0_D3,//GPIO1_C7,
            .fmux       = EMMC_RSTNOUT,
        },
    },
   
};
#endif

// ---end -#if defined(CONFIG_ARCH_RK3066B)|| defined(CONFIG_ARCH_RK3188)

#elif defined(CONFIG_ARCH_RK319X)
/*
* define the gpio for sdmmc0
*/
static struct rksdmmc_gpio_board rksdmmc0_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN4_PA6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_clkout",
            .fgpio      = GPIO4_A6,
            .fmux       = MMC0_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN4_PA7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_cmd",
            .fgpio      = GPIO4_A7,
            .fmux       = MMC0_CMD,
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN4_PA0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d0",
            .fgpio      = GPIO4_A0,
            .fmux       = MMC0_D0,
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN4_PA1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d1",
            .fgpio      = GPIO4_A1,
            .fmux       = MMC0_D1,
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN4_PA2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d2",
            .fgpio      = GPIO4_A2,
            .fmux       = MMC0_D2,
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN4_PA3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d3",
            .fgpio      = GPIO4_A3,
            .fmux       = MMC0_D3,
        },
    }, 

               
    .power_en_gpio      = {   
#if defined(RK29SDK_SD_CARD_PWR_EN) && (INVALID_GPIO != RK29SDK_SD_CARD_PWR_EN)
                    .io             = RK29SDK_SD_CARD_PWR_EN,
                    .enable         = RK29SDK_SD_CARD_PWR_EN_LEVEL,
        #ifdef RK29SDK_SD_CARD_PWR_EN_PIN_NAME
                    .iomux          = {
                        .name       = RK29SDK_SD_CARD_PWR_EN_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO
                        .fgpio      = RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX
                        .fmux       = RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX,
            #endif
                    },
        #endif
#else
                    .io             = INVALID_GPIO,
                    .enable         = GPIO_LOW,
#endif
                }, 
            
        .detect_irq       = {
#if defined(RK29SDK_SD_CARD_DETECT_N) && (INVALID_GPIO != RK29SDK_SD_CARD_DETECT_N)  
                    .io             = RK29SDK_SD_CARD_DETECT_N,
                    .enable         = RK29SDK_SD_CARD_INSERT_LEVEL,
        #ifdef RK29SDK_SD_CARD_DETECT_PIN_NAME
                    .iomux          = {
                        .name       = RK29SDK_SD_CARD_DETECT_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO
                        .fgpio      = RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FMUX
                        .fmux       = RK29SDK_SD_CARD_DETECT_IOMUX_FMUX,
            #endif
                    },
        #endif
#else
                    .io             = INVALID_GPIO,
                    .enable         = GPIO_LOW,
#endif            
    },

};


/*
* define the gpio for sdmmc1
*/
static struct rksdmmc_gpio_board rksdmmc1_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN3_PB1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_clkout",
            .fgpio      = GPIO3_B1,
            .fmux       = MMC1_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN3_PB0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_cmd",
            .fgpio      = GPIO3_B0,
            .fmux       = MMC1_CMD,
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN3_PA4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d0",
            .fgpio      = GPIO3_A4,
            .fmux       = MMC1_D0,
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN3_PA5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d1",
            .fgpio      = GPIO3_A5,
            .fmux       = MMC1_D1,
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN3_PA6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d2",
            .fgpio      = GPIO3_A6,
            .fmux       = MMC1_D2,
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN3_PA7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d3",
            .fgpio      = GPIO3_A7,
            .fmux       = MMC1_D3,
        },
    }, 
};

#if defined(CONFIG_SDMMC2_RK29)
/*
* define the gpio for sdmmc2, mainly used to support eMMC.
*/
static struct rksdmmc_gpio_board rksdmmc2_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN2_PD6,
        .enable         = GPIO_HIGH,
     .iomux          = {
            .name       = "mmc2_clkout",
            .fgpio      = GPIO2_D6,
            .fmux       = EMMC_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN2_PD4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_cmd",
            .fgpio      = GPIO2_D4,
            .fmux       = EMMC_CMD,
        },
    },      

   //no iomux data0--data7
	.power_en_gpio      = {
		.io				= RK30_PIN2_PC5,
		.enable         = GPIO_HIGH,
		.iomux          = {
			.name       = "mmc2_pwr",
			.fgpio      = GPIO2_C5,//GPIO1_C7,
			.fmux       = EMMC_PWREN,
		},
	},
    .rstnout_gpio       = {
        .io             = RK30_PIN2_PD5,//RK30_PIN1_PC7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_rstn",
            .fgpio      = GPIO2_D5,//GPIO1_C7,
            .fmux       = EMMC_RSTNOUT,
        },
    },
   
};
#endif

// ---end --#elif defined(CONFIG_ARCH_RK319X)

#elif defined(CONFIG_ARCH_RK2928)
/*
* define the gpio for sdmmc0
*/
static struct rksdmmc_gpio_board rksdmmc0_gpio_init = {

     .clk_gpio       = {
        .io             = RK2928_PIN1_PC0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO1C0_MMC0_CLKOUT_NAME,
            .fgpio      = GPIO1C_GPIO1C0,
            .fmux       = GPIO1C_MMC0_CLKOUT,
	    #endif
        },
    },   

    .cmd_gpio           = {
        .io             = RK2928_PIN1_PC7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO1B7_MMC0_CMD_NAME,
            .fgpio      = GPIO1B_GPIO1B7,
            .fmux       = GPIO1B_MMC0_CMD,
	    #endif
        },
    },      

   .data0_gpio       = {
        .io             = RK2928_PIN1_PC2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO1C2_MMC0_D0_NAME,
            .fgpio      = GPIO1C_GPIO1C2,
            .fmux       = GPIO1C_MMC0_D0,
	    #endif
        },
    },      

    .data1_gpio       = {
        .io             = RK2928_PIN1_PC3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO1C3_MMC0_D1_NAME,
            .fgpio      = GPIO1C_GPIO1C3,
            .fmux       = GPIO1C_MMC0_D1,
	    #endif
        },
    },      

    .data2_gpio       = {
        .io             = RK2928_PIN1_PC4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO1C4_MMC0_D2_NAME,
            .fgpio      = GPIO1C_GPIO1C4,
            .fmux       = GPIO1C_MMC0_D2,
	    #endif
        },
    }, 

    .data3_gpio       = {
        .io             = RK2928_PIN1_PC5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO1C5_MMC0_D3_NAME,
            .fgpio      = GPIO1C_GPIO1C5,
            .fmux       = GPIO1C_MMC0_D3,
	    #endif
        },
    }, 

   
    .power_en_gpio      = {   
#if defined(RK29SDK_SD_CARD_PWR_EN) && (INVALID_GPIO != RK29SDK_SD_CARD_PWR_EN)
        .io             = RK29SDK_SD_CARD_PWR_EN,
        .enable         = RK29SDK_SD_CARD_PWR_EN_LEVEL,
        #ifdef RK29SDK_SD_CARD_PWR_EN_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_SD_CARD_PWR_EN_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO
            .fgpio      = RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX
            .fmux       = RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif
    }, 

    .detect_irq       = {
#if defined(RK29SDK_SD_CARD_DETECT_N) && (INVALID_GPIO != RK29SDK_SD_CARD_DETECT_N)  
        .io             = RK29SDK_SD_CARD_DETECT_N,
        .enable         = RK29SDK_SD_CARD_INSERT_LEVEL,
        #ifdef RK29SDK_SD_CARD_DETECT_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_SD_CARD_DETECT_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO
            .fgpio      = RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FMUX
            .fmux       = RK29SDK_SD_CARD_DETECT_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif            
    }, 
};


/*
* define the gpio for sdmmc1
*/
static struct rksdmmc_gpio_board rksdmmc1_gpio_init = {

     .clk_gpio       = {
        .io             = RK2928_PIN0_PB1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO0B1_MMC1_CLKOUT_NAME,
            .fgpio      = GPIO0B_GPIO0B1,
            .fmux       = GPIO0B_MMC1_CLKOUT,
	    #endif
        },
    },   

    .cmd_gpio           = {
        .io             = RK2928_PIN0_PB0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO0B0_MMC1_CMD_NAME,
            .fgpio      = GPIO0B_GPIO0B0,
            .fmux       = GPIO0B_MMC1_CMD,
	    #endif
        },
    },      

   .data0_gpio       = {
        .io             = RK2928_PIN0_PB3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO0B3_MMC1_D0_NAME,
            .fgpio      = GPIO0B_GPIO0B3,
            .fmux       = GPIO0B_MMC1_D0,
	    #endif
        },
    },      

    .data1_gpio       = {
        .io             = RK2928_PIN0_PB4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO0B4_MMC1_D1_NAME,
            .fgpio      = GPIO0B_GPIO0B4,
            .fmux       = GPIO0B_MMC1_D1,
	    #endif
        },
    },      

    .data2_gpio       = {
        .io             = RK2928_PIN0_PB5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO0B5_MMC1_D2_NAME,
            .fgpio      = GPIO0B_GPIO0B5,
            .fmux       = GPIO0B_MMC1_D2,
	    #endif
        },
    }, 

    .data3_gpio       = {
        .io             = RK2928_PIN0_PB6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO0B6_MMC1_D3_NAME,
            .fgpio      = GPIO0B_GPIO0B6,
            .fmux       = GPIO0B_MMC1_D3,
	    #endif
        },
    }, 
};
    /*
    * define the gpio for sdmmc2
    */
    
    static struct rksdmmc_gpio_board rksdmmc2_gpio_init = {
    
             .clk_gpio       = {
                .io             = RK2928_PIN2_PA7,
                .enable         = GPIO_HIGH,
             .iomux          = {
                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO2A7_NAND_DPS_EMMC_CLKOUT_NAME,
                    .fgpio      = GPIO2A_GPIO2A7,
                    .fmux       = GPIO2A_EMMC_CLKOUT,
                #endif    
                },
            },   
        
            .cmd_gpio           = {
                .io             = RK2928_PIN1_PC6,
                .enable         = GPIO_HIGH,
                .iomux          = {
                    #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1C6_NAND_CS2_EMMC_CMD_NAME,
                    .fgpio      = GPIO1C_GPIO1C6,
                    .fmux       = GPIO1C_EMMC_CMD,
                    #endif
                },
            },      

           .data0_gpio       = {
                .io             = RK2928_PIN1_PD0,
                .enable         = GPIO_HIGH,
                .iomux          = {
                   #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1D0_NAND_D0_EMMC_D0_NAME,
                    .fgpio      = GPIO1D_GPIO1D0,
                    .fmux       = GPIO1D_EMMC_D0,
                   #endif 
                },
            },      
        
            .data1_gpio       = {
                .io             = RK2928_PIN1_PD1,
                .enable         = GPIO_HIGH,
                .iomux          = {
                   #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1D1_NAND_D1_EMMC_D1_NAME,
                    .fgpio      = GPIO1D_GPIO1D1,
                    .fmux       = GPIO1D_EMMC_D1,
                   #endif 
                },
            },      
        
            .data2_gpio       = {
                .io             = RK2928_PIN1_PD2,
                .enable         = GPIO_HIGH,
                .iomux          = {
                   #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1D2_NAND_D2_EMMC_D2_NAME,
                    .fgpio      = GPIO1D_GPIO1D2,
                    .fmux       = GPIO1D_EMMC_D2,
                   #endif 
                },
            }, 
        
            .data3_gpio       = {
                .io             = RK2928_PIN1_PD3,
                .enable         = GPIO_HIGH,
                .iomux          = {
                  #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1D3_NAND_D3_EMMC_D3_NAME,
                    .fgpio      = GPIO1D_GPIO1D3,
                    .fmux       = GPIO1D_EMMC_D3,
                  #endif
                },
            }, 
#ifdef USE_SDMMC_DATA4_DATA7
            .data4_gpio       = {
                .io             = RK2928_PIN1_PD4,
                .enable         = GPIO_HIGH,
                .iomux          = {
                   #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1D4_NAND_D4_EMMC_D4_NAME,
                    .fgpio      = GPIO1D_GPIO1D4,
                    .fmux       = GPIO1D_EMMC_D4,
                   #endif 
                },
            },     
        
            .data5_gpio       = {
                .io             = RK2928_PIN1_PD5,
                .enable         = GPIO_HIGH,
                .iomux          = {
                   #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1D5_NAND_D5_EMMC_D5_NAME,
                    .fgpio      = GPIO1D_GPIO1D5,
                    .fmux       = GPIO1D_EMMC_D5,
                   #endif
                },
            },     
        
            .data6_gpio       = {
                .io             = RK2928_PIN1_PD6,
                .enable         = GPIO_HIGH,
                .iomux          = {
                    .name       = GPIO1D6_NAND_D6_EMMC_D6_NAME,
                    .fgpio      = GPIO1D_GPIO1D6,
                    .fmux       = GPIO1D_EMMC_D6,
                },
            }, 
        
            .data7_gpio       = {
                .io             = RK2928_PIN1_PD7,
                .enable         = GPIO_HIGH,
                .iomux          = {
                   #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1D7_NAND_D7_EMMC_D7_NAME,
                    .fgpio      = GPIO1D_GPIO1D7,
                    .fmux       = GPIO1D_EMMC_D7,
                   #endif 
                },
            }, 
 #endif //---#ifdef USE_SDMMC_DATA4_DATA7
       
	   		.power_en_gpio      = {
			    .io             = RK2928_PIN2_PA5,
			    .enable         = GPIO_HIGH,
			    .iomux          = {
					#if !(!!SDMMC_USE_NEW_IOMUX_API)
				    .name       = GPIO2A5_NAND_WP_EMMC_PWREN_NAME,
				    .fgpio      = GPIO2A_GPIO2A5,//GPIO1_C7,
				    .fmux       = GPIO2A_EMMC_PWREN,
					#endif
				},
			},
	    
            .rstnout_gpio       = {
                .io             = RK2928_PIN1_PC7,
                .enable         = GPIO_HIGH,
                .iomux          = {
                    #if !(!!SDMMC_USE_NEW_IOMUX_API)
                    .name       = GPIO1C7_NAND_CS3_EMMC_RSTNOUT_NAME,
                    .fgpio      = GPIO1C_GPIO1C7,
                    .fmux       = GPIO1C_EMMC_RSTNOUT,
                    #endif
                },
            },
           
};
// ---end -#if defined(CONFIG_ARCH_RK2928)
#elif defined(CONFIG_ARCH_RK3026)
/*
* define the gpio for sdmmc0
*/
static struct rksdmmc_gpio_board rksdmmc0_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN1_PC0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_clk",
            .fgpio      = GPIO1_C0,
            .fmux       = MMC0_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN1_PB7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_cmd",
            .fgpio      = GPIO1_B7,
            .fmux       = MMC0_CMD,
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN1_PC2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d0",
            .fgpio      = GPIO1_C2,
            .fmux       = MMC0_D0,
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN1_PC3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d1",
            .fgpio      = GPIO1_C3,
            .fmux       = MMC0_D1,
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN1_PC4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d2",
            .fgpio      = GPIO1_C4,
            .fmux       = MMC0_D2,
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN1_PC5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc0_d3",
            .fgpio      = GPIO1_C5,
            .fmux       = MMC0_D3,
        },
    }, 

   
    .power_en_gpio      = {   
#if defined(RK29SDK_SD_CARD_PWR_EN) && (INVALID_GPIO != RK29SDK_SD_CARD_PWR_EN)
        .io             = RK29SDK_SD_CARD_PWR_EN,
        .enable         = RK29SDK_SD_CARD_PWR_EN_LEVEL,
        #ifdef RK29SDK_SD_CARD_PWR_EN_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_SD_CARD_PWR_EN_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO
            .fgpio      = RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX
            .fmux       = RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif
    }, 

    .detect_irq       = {
#if defined(RK29SDK_SD_CARD_DETECT_N) && (INVALID_GPIO != RK29SDK_SD_CARD_DETECT_N)  
        .io             = RK29SDK_SD_CARD_DETECT_N,
        .enable         = RK29SDK_SD_CARD_INSERT_LEVEL,
        #ifdef RK29SDK_SD_CARD_DETECT_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_SD_CARD_DETECT_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO
            .fgpio      = RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FMUX
            .fmux       = RK29SDK_SD_CARD_DETECT_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif            
    }, 
};


/*
* define the gpio for sdmmc1
*/
static struct rksdmmc_gpio_board rksdmmc1_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN0_PB1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_clk",
            .fgpio      = GPIO0_B1,
            .fmux       = MMC1_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN0_PB0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_cmd",
            .fgpio      = GPIO0_B0,
            .fmux       = MMC1_CMD,
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN0_PB3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d0",
            .fgpio      = GPIO0_B3,
            .fmux       = MMC1_D0,
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN0_PB4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d1",
            .fgpio      = GPIO0_B4,
            .fmux       = MMC1_D1,
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN0_PB5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d2",
            .fgpio      = GPIO0_B5,
            .fmux       = MMC1_D2,
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN0_PB6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc1_d3",
            .fgpio      = GPIO0_B6,
            .fmux       = MMC1_D3,
        },
    }, 


};

/*
* define the gpio for sdmmc2
*/

static struct rksdmmc_gpio_board rksdmmc2_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN0_PD0,//RK30_PIN2_PA7,
        .enable         = GPIO_HIGH,
     .iomux          = {
            .name       = "mmc2_clkout",
            .fgpio      = GPIO0_D0,//GPIO2_A7,
            .fmux       = EMMC_CLKOUT,
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN0_PD2,//RK30_PIN1_PC6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_cmd",
            .fgpio      = GPIO0_D2,//GPIO1_C6,
            .fmux       = EMMC_CMD,
        },
    },      
#ifdef CONFIG_ARCH_RK3026
   .data0_gpio       = {
        .io             = RK30_PIN1_PD0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d0",
            .fgpio      = GPIO1_D0,
            .fmux       = EMMC_D0,
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN1_PD1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d1",
            .fgpio      = GPIO1_D1,
            .fmux       = EMMC_D1,
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN1_PD2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d2",
            .fgpio      = GPIO1_D2,
            .fmux       = EMMC_D2,
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN1_PD3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d3",
            .fgpio      = GPIO1_D3,
            .fmux       = EMMC_D3,
        },
    }, 
#ifdef USE_SDMMC_DATA4_DATA7
    .data4_gpio       = {
        .io             = RK30_PIN1_PD4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d4",
            .fgpio      = GPIO1_D4,
            .fmux       = EMMC_D4,
        },
    },     

    .data5_gpio       = {
        .io             = RK30_PIN1_PD5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d5",
            .fgpio      = GPIO1_D5,
            .fmux       = EMMC_D5,
        },
    },     

    .data6_gpio       = {
        .io             = RK30_PIN1_PD6,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d6",
            .fgpio      = GPIO1_D6,
            .fmux       = EMMC_D6,
        },
    }, 

    .data7_gpio       = {
        .io             = RK30_PIN1_PD7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_d7",
            .fgpio      = GPIO1_D7,
            .fmux       = EMMC_D7,
        },
    }, 
 #endif //---#ifdef USE_SDMMC_DATA4_DATA7
#endif

    .rstnout_gpio       = {
        .io             = RK30_PIN0_PD3,//RK30_PIN1_PC7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = "mmc2_rstn",
            .fgpio      = GPIO0_D3,//GPIO1_C7,
            .fmux       = EMMC_RSTNOUT,
        },
    },
   
};

// ---end -#if defined(CONFIG_ARCH_RK3026)

#else //default for RK30,RK3066 SDK
/*
* define the gpio for sdmmc0
*/
struct rksdmmc_gpio_board rksdmmc0_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN3_PB0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3B0_SDMMC0CLKOUT_NAME,
            .fgpio      = GPIO3B_GPIO3B0,
            .fmux       = GPIO3B_SDMMC0_CLKOUT,
            #else
    	    .name	= "mmc0_clk",
    	    .fgpio	= GPIO3_B0,
    	    .fmux	= MMC0_CLKOUT,
    	    #endif
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN3_PB1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3B1_SDMMC0CMD_NAME,
            .fgpio      = GPIO3B_GPIO3B1,
            .fmux       = GPIO3B_SDMMC0_CMD,
            #else
    	    .name	= "mmc0_cmd",
    	    .fgpio	= GPIO3_B1,
    	    .fmux	= MMC0_CMD,
    	    #endif
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN3_PB2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3B2_SDMMC0DATA0_NAME,
            .fgpio      = GPIO3B_GPIO3B2,
            .fmux       = GPIO3B_SDMMC0_DATA0,
            #else
    	    .name 	= "mmc0_data0",
    	    .fgpio	= GPIO3_B2,
    	    .fmux	= MMC0_D0,
    	    #endif
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN3_PB3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3B3_SDMMC0DATA1_NAME,
            .fgpio      = GPIO3B_GPIO3B3,
            .fmux       = GPIO3B_SDMMC0_DATA1,
            #else
    	    .name	= "mmc0_data1",
    	    .fgpio	= GPIO3_B3,
    	    .fmux	= MMC0_D1,
    	    #endif
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN3_PB4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3B4_SDMMC0DATA2_NAME,
            .fgpio      = GPIO3B_GPIO3B4,
            .fmux       = GPIO3B_SDMMC0_DATA2,
             #else
    	    .name	= "mmc0_data2",
    	    .fgpio	= GPIO3_B4,
    	    .fmux	= MMC0_D2,
    	    #endif
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN3_PB5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3B5_SDMMC0DATA3_NAME,
            .fgpio      = GPIO3B_GPIO3B5,
            .fmux       = GPIO3B_SDMMC0_DATA3,
             #else
    	    .name	= "mmc0_data3",
    	    .fgpio	= GPIO3_B5,
    	    .fmux	= MMC0_D3,
    	    #endif
        },
    }, 
    
    .power_en_gpio      = {   
#if defined(RK29SDK_SD_CARD_PWR_EN) && (INVALID_GPIO != RK29SDK_SD_CARD_PWR_EN)
        .io             = RK29SDK_SD_CARD_PWR_EN,
        .enable         = RK29SDK_SD_CARD_PWR_EN_LEVEL,
        #ifdef RK29SDK_SD_CARD_PWR_EN_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_SD_CARD_PWR_EN_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO
            .fgpio      = RK29SDK_SD_CARD_PWR_EN_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX
            .fmux       = RK29SDK_SD_CARD_PWR_EN_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif
    }, 

    .detect_irq       = {
#if defined(RK29SDK_SD_CARD_DETECT_N) && (INVALID_GPIO != RK29SDK_SD_CARD_DETECT_N)  
        .io             = RK29SDK_SD_CARD_DETECT_N,
        .enable         = RK29SDK_SD_CARD_INSERT_LEVEL,
        #ifdef RK29SDK_SD_CARD_DETECT_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_SD_CARD_DETECT_PIN_NAME,
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO
            .fgpio      = RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_SD_CARD_DETECT_IOMUX_FMUX
            .fmux       = RK29SDK_SD_CARD_DETECT_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif            
    },
};


/*
* define the gpio for sdmmc1
*/
static struct rksdmmc_gpio_board rksdmmc1_gpio_init = {

     .clk_gpio       = {
        .io             = RK30_PIN3_PC5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3C5_SDMMC1CLKOUT_NAME,
            .fgpio      = GPIO3C_GPIO3C5,
            .fmux       = GPIO3C_SDMMC1_CLKOUT,
            #else
            .name       = "mmc1_clk",
            .fgpio      = GPIO3_C5,
            .fmux       = MMC1_CLKOUT,
	        #endif
        },
    },   

    .cmd_gpio           = {
        .io             = RK30_PIN3_PC0,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3C0_SMMC1CMD_NAME,
            .fgpio      = GPIO3C_GPIO3C0,
            .fmux       = GPIO3C_SMMC1_CMD,
            #else
            .name       = "mmc1_cmd",
            .fgpio      = GPIO3_C0,
            .fmux       = MMC1_CMD,
	        #endif
        },
    },      

   .data0_gpio       = {
        .io             = RK30_PIN3_PC1,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3C1_SDMMC1DATA0_NAME,
            .fgpio      = GPIO3C_GPIO3C1,
            .fmux       = GPIO3C_SDMMC1_DATA0,
            #else
            .name       = "mmc1_data0",
            .fgpio      = GPIO3_C1,
            .fmux       = MMC1_D0,
	        #endif
        },
    },      

    .data1_gpio       = {
        .io             = RK30_PIN3_PC2,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3C2_SDMMC1DATA1_NAME,
            .fgpio      = GPIO3C_GPIO3C2,
            .fmux       = GPIO3C_SDMMC1_DATA1,
            #else
            .name       = "mmc1_data1",
            .fgpio      = GPIO3_C2,
            .fmux       = MMC1_D1,
	        #endif
        },
    },      

    .data2_gpio       = {
        .io             = RK30_PIN3_PC3,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3C3_SDMMC1DATA2_NAME,
            .fgpio      = GPIO3C_GPIO3C3,
            .fmux       = GPIO3C_SDMMC1_DATA2,
            #else
            .name       = "mmc1_data2",
            .fgpio      = GPIO3_C3,
            .fmux       = MMC1_D2,
	        #endif
        },
    }, 

    .data3_gpio       = {
        .io             = RK30_PIN3_PC4,
        .enable         = GPIO_HIGH,
        .iomux          = {
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            .name       = GPIO3C4_SDMMC1DATA3_NAME,
            .fgpio      = GPIO3C_GPIO3C4,
            .fmux       = GPIO3C_SDMMC1_DATA3,
            #else
             .name      = "mmc1_data3",
            .fgpio      = GPIO3_C4,
            .fmux       = MMC1_D3,     
	        #endif
        },
    }, 


    .power_en_gpio      = {   
#if defined(RK29SDK_MMC1_CARD_PWR_EN) && (INVALID_GPIO != RK29SDK_MMC1_CARD_PWR_EN)
        .io             = RK29SDK_MMC1_CARD_PWR_EN,
        .enable         = RK29SDK_MMC1_CARD_PWR_EN_LEVEL,
        #ifdef RK29SDK_MMC1_CARD_PWR_EN_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_MMC1_CARD_PWR_EN_PIN_NAME,
            #ifdef RK29SDK_MMC1_CARD_PWR_EN_IOMUX_FGPIO
            .fgpio      = RK29SDK_MMC1_CARD_PWR_EN_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_MMC1_CARD_PWR_EN_IOMUX_FMUX
            .fmux       = RK29SDK_MMC1_CARD_PWR_EN_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif
    }, 

    .detect_irq       = {
#if defined(RK29SDK_MMC1_CARD_DETECT_N) && (INVALID_GPIO != RK29SDK_MMC1_CARD_DETECT_N)  
        .io             = RK29SDK_MMC1_CARD_DETECT_N,
        .enable         = RK29SDK_MMC1_CARD_INSERT_LEVEL,
        #ifdef RK29SDK_MMC1_CARD_DETECT_PIN_NAME
        .iomux          = {
            .name       = RK29SDK_MMC1_CARD_DETECT_PIN_NAME,
            #ifdef RK29SDK_MMC1_CARD_DETECT_IOMUX_FGPIO
            .fgpio      = RK29SDK_MMC1_CARD_DETECT_IOMUX_FGPIO,
            #endif
            #ifdef RK29SDK_MMC1_CARD_DETECT_IOMUX_FMUX
            .fmux       = RK29SDK_MMC1_CARD_DETECT_IOMUX_FMUX,
            #endif
        },
        #endif
#else
        .io             = INVALID_GPIO,
        .enable         = GPIO_LOW,
#endif            
    },
};

 
#if defined(CONFIG_SDMMC2_RK29)
 /*
 * define the gpio for sdmmc2, mainly used to support eMMC.
 */
 static struct rksdmmc_gpio_board rksdmmc2_gpio_init = {
 
      .clk_gpio       = {
         .io             = RK30_PIN3_PD7,
         .enable         = GPIO_HIGH,
      .iomux          = {
             .name       = "mmc2_clkout",
             .fgpio      = GPIO3_D7,
             .fmux       = EMMC_CLKOUT,
         },
     },   
 
     .cmd_gpio           = {
         .io             = RK30_PIN4_PB1,
         .enable         = GPIO_HIGH,
         .iomux          = {
             .name       = "mmc2_cmd",
             .fgpio      = GPIO4B_GPIO4B1,
             .fmux       = EMMC_CMD,
         },
     },      

    //data0--data7,no iomux
 
     .rstnout_gpio       = {
         .io             = RK30_PIN4_PB2,
         .enable         = GPIO_HIGH,
         .iomux          = {
             .name       = "mmc2_rstn",
             .fgpio      = GPIO4B_GPIO4B2,
             .fmux       = EMMC_RSTNOUT,
         },
     },
    
 };
#endif


 // ---end -defualt rk30sdk,rk3066sdk

#endif



//1.Part 3: The various operations of the SDMMC-SDIO module
/*************************************************************************
* define the varaious operations for SDMMC module
* Generally only the author of SDMMC module will modify this section.
*************************************************************************/
#if !defined(CONFIG_SDMMC_RK29_OLD)	
//static void rk29_sdmmc_gpio_open(int device_id, int on)
void rk29_sdmmc_gpio_open(int device_id, int on)
{
    switch(device_id)
    {
        case 0://mmc0
        {
            #ifdef CONFIG_SDMMC0_RK29
            if(on)
            {
                gpio_direction_output(rksdmmc0_gpio_init.clk_gpio.io, GPIO_HIGH);//set mmc0-clk to high
                gpio_direction_output(rksdmmc0_gpio_init.cmd_gpio.io, GPIO_HIGH);// set mmc0-cmd to high.
                gpio_direction_output(rksdmmc0_gpio_init.data0_gpio.io,GPIO_HIGH);//set mmc0-data0 to high.
                gpio_direction_output(rksdmmc0_gpio_init.data1_gpio.io,GPIO_HIGH);//set mmc0-data1 to high.
                gpio_direction_output(rksdmmc0_gpio_init.data2_gpio.io,GPIO_HIGH);//set mmc0-data2 to high.
                gpio_direction_output(rksdmmc0_gpio_init.data3_gpio.io,GPIO_HIGH);//set mmc0-data3 to high.

                mdelay(30);
            }
            else
            {
                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc0_gpio_init.clk_gpio.iomux.name, rksdmmc0_gpio_init.clk_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc0_gpio_init.clk_gpio.io, "mmc0-clk");
                gpio_direction_output(rksdmmc0_gpio_init.clk_gpio.io,GPIO_LOW);//set mmc0-clk to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc0_gpio_init.cmd_gpio.iomux.name, rksdmmc0_gpio_init.cmd_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc0_gpio_init.cmd_gpio.io, "mmc0-cmd");
                gpio_direction_output(rksdmmc0_gpio_init.cmd_gpio.io,GPIO_LOW);//set mmc0-cmd to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc0_gpio_init.data0_gpio.iomux.name, rksdmmc0_gpio_init.data0_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc0_gpio_init.data0_gpio.io, "mmc0-data0");
                gpio_direction_output(rksdmmc0_gpio_init.data0_gpio.io,GPIO_LOW);//set mmc0-data0 to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc0_gpio_init.data1_gpio.iomux.name, rksdmmc0_gpio_init.data1_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc0_gpio_init.data1_gpio.io, "mmc0-data1");
                gpio_direction_output(rksdmmc0_gpio_init.data1_gpio.io,GPIO_LOW);//set mmc0-data1 to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc0_gpio_init.data2_gpio.iomux.name, rksdmmc0_gpio_init.data2_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc0_gpio_init.data2_gpio.io, "mmc0-data2");
                gpio_direction_output(rksdmmc0_gpio_init.data2_gpio.io,GPIO_LOW);//set mmc0-data2 to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc0_gpio_init.data3_gpio.iomux.name, rksdmmc0_gpio_init.data3_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc0_gpio_init.data3_gpio.io, "mmc0-data3");
                gpio_direction_output(rksdmmc0_gpio_init.data3_gpio.io,GPIO_LOW);//set mmc0-data3 to low.

                mdelay(30);
            }
            #endif
        }
        break;
        
        case 1://mmc1
        {
            #ifdef CONFIG_SDMMC1_RK29
            if(on)
            {
                gpio_direction_output(rksdmmc1_gpio_init.clk_gpio.io,GPIO_HIGH);//set mmc1-clk to high
                gpio_direction_output(rksdmmc1_gpio_init.cmd_gpio.io,GPIO_HIGH);//set mmc1-cmd to high.
                gpio_direction_output(rksdmmc1_gpio_init.data0_gpio.io,GPIO_HIGH);//set mmc1-data0 to high.
                gpio_direction_output(rksdmmc1_gpio_init.data1_gpio.io,GPIO_HIGH);//set mmc1-data1 to high.
                gpio_direction_output(rksdmmc1_gpio_init.data2_gpio.io,GPIO_HIGH);//set mmc1-data2 to high.
                gpio_direction_output(rksdmmc1_gpio_init.data3_gpio.io,GPIO_HIGH);//set mmc1-data3 to high.
                mdelay(100);
            }
            else
            {
                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc1_gpio_init.clk_gpio.iomux.name, rksdmmc1_gpio_init.clk_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc1_gpio_init.clk_gpio.io, "mmc1-clk");
                gpio_direction_output(rksdmmc1_gpio_init.clk_gpio.io,GPIO_LOW);//set mmc1-clk to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc1_gpio_init.cmd_gpio.iomux.name, rksdmmc1_gpio_init.cmd_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc1_gpio_init.cmd_gpio.io, "mmc1-cmd");
                gpio_direction_output(rksdmmc1_gpio_init.cmd_gpio.io,GPIO_LOW);//set mmc1-cmd to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk30_mux_api_set(rksdmmc1_gpio_init.data0_gpio.iomux.name, rksdmmc1_gpio_init.data0_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc1_gpio_init.data0_gpio.io, "mmc1-data0");
                gpio_direction_output(rksdmmc1_gpio_init.data0_gpio.io,GPIO_LOW);//set mmc1-data0 to low.
                
            #if defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC) || defined(CONFIG_MT5931) || defined(CONFIG_MT5931_MT6622)\
                    || defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
                    
                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk29_mux_api_set(rksdmmc1_gpio_init.data1_gpio.iomux.name, rksdmmc1_gpio_init.data1_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc1_gpio_init.data1_gpio.io, "mmc1-data1");
                gpio_direction_output(rksdmmc1_gpio_init.data1_gpio.io,GPIO_LOW);//set mmc1-data1 to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk29_mux_api_set(rksdmmc1_gpio_init.data2_gpio.iomux.name, rksdmmc1_gpio_init.data2_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc1_gpio_init.data2_gpio.io, "mmc1-data2");
                gpio_direction_output(rksdmmc1_gpio_init.data2_gpio.io,GPIO_LOW);//set mmc1-data2 to low.

                #if !(!!SDMMC_USE_NEW_IOMUX_API)
                rk29_mux_api_set(rksdmmc1_gpio_init.data3_gpio.iomux.name, rksdmmc1_gpio_init.data3_gpio.iomux.fgpio);
                #endif
                gpio_request(rksdmmc1_gpio_init.data3_gpio.io, "mmc1-data3");
                gpio_direction_output(rksdmmc1_gpio_init.data3_gpio.io,GPIO_LOW);//set mmc1-data3 to low.
           #endif
                mdelay(100);
            }
            #endif
        }
        break; 
        
        case 2: //mmc2
        break;
        
        default:
        break;
    }
}

static void rk29_sdmmc_set_iomux_mmc0(unsigned int bus_width)
{
    switch (bus_width)
    {
        
    	case 1://SDMMC_CTYPE_4BIT:
    	{
    	    #if SDMMC_USE_NEW_IOMUX_API
            iomux_set(rksdmmc0_gpio_init.data1_gpio.iomux.fmux);
            iomux_set(rksdmmc0_gpio_init.data2_gpio.iomux.fmux);
            iomux_set(rksdmmc0_gpio_init.data3_gpio.iomux.fmux);
    	    #else
        	rk30_mux_api_set(rksdmmc0_gpio_init.data1_gpio.iomux.name, rksdmmc0_gpio_init.data1_gpio.iomux.fmux);
        	rk30_mux_api_set(rksdmmc0_gpio_init.data2_gpio.iomux.name, rksdmmc0_gpio_init.data2_gpio.iomux.fmux);
        	rk30_mux_api_set(rksdmmc0_gpio_init.data3_gpio.iomux.name, rksdmmc0_gpio_init.data3_gpio.iomux.fmux);
        	#endif
    	}
    	break;

    	case 0x10000://SDMMC_CTYPE_8BIT:
    	    break;
    	case 0xFFFF: //gpio_reset
    	{
    	    #if (!!SDMMC_USE_NEW_IOMUX_API) && !defined(CONFIG_SDMMC0_RK29_SDCARD_DET_FROM_GPIO)
    	    iomux_set(MMC0_DETN);
    	    #endif
    	    
    	    #if !(!!SDMMC_USE_NEW_IOMUX_API)
            rk30_mux_api_set(rksdmmc0_gpio_init.power_en_gpio.iomux.name, rksdmmc0_gpio_init.power_en_gpio.iomux.fgpio);
            #endif
            gpio_request(rksdmmc0_gpio_init.power_en_gpio.io,"sdmmc-power");
            gpio_direction_output(rksdmmc0_gpio_init.power_en_gpio.io, !(rksdmmc0_gpio_init.power_en_gpio.enable)); //power-off

        #if 0 //replace the power control into rk29_sdmmc_set_ios(); modifyed by xbw at 2012-08-12
            rk29_sdmmc_gpio_open(0, 0);

            gpio_direction_output(rksdmmc0_gpio_init.power_en_gpio.io, rksdmmc0_gpio_init.power_en_gpio.enable); //power-on

            rk29_sdmmc_gpio_open(0, 1);
          #endif  
    	}
    	break;

    	default: //case 0://SDMMC_CTYPE_1BIT:
        {
            #if SDMMC_USE_NEW_IOMUX_API
        	iomux_set(rksdmmc0_gpio_init.cmd_gpio.iomux.fmux);
        	iomux_set(rksdmmc0_gpio_init.clk_gpio.iomux.fmux);
        	iomux_set(rksdmmc0_gpio_init.data0_gpio.iomux.fmux);
            #else
        	rk30_mux_api_set(rksdmmc0_gpio_init.cmd_gpio.iomux.name, rksdmmc0_gpio_init.cmd_gpio.iomux.fmux);
        	rk30_mux_api_set(rksdmmc0_gpio_init.clk_gpio.iomux.name, rksdmmc0_gpio_init.clk_gpio.iomux.fmux);
        	rk30_mux_api_set(rksdmmc0_gpio_init.data0_gpio.iomux.name, rksdmmc0_gpio_init.data0_gpio.iomux.fmux);
        	#endif

            //IO voltage(vccio);
            #ifdef RK31SDK_SET_SDMMC0_PIN_VOLTAGE
                if(rk31sdk_get_sdmmc0_pin_io_voltage() > 2700)
                    SDMMC_write_grf_reg(GRF_IO_CON4, (SDMMC0_IO_VOLTAGE_MASK |SDMMC0_IO_VOLTAGE_33)); //set SDMMC0 pin to 3.3v
                else
                    SDMMC_write_grf_reg(GRF_IO_CON4, (SDMMC0_IO_VOLTAGE_MASK |SDMMC0_IO_VOLTAGE_18));//set SDMMC0 pin to 1.8v
            #else
            //default set the voltage of SDMMC0 to 3.3V
            SDMMC_write_grf_reg(GRF_IO_CON4, (SDMMC0_IO_VOLTAGE_MASK |SDMMC0_IO_VOLTAGE_33));
            #endif
    
            //sdmmc drive strength control
            SDMMC_write_grf_reg(GRF_IO_CON2, (SDMMC0_DRIVER_STRENGTH_MASK |SDMMC0_DRIVER_STRENGTH_12MA));
            
            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            rk30_mux_api_set(rksdmmc0_gpio_init.data1_gpio.iomux.name, rksdmmc0_gpio_init.data1_gpio.iomux.fgpio);
            #endif
            gpio_request(rksdmmc0_gpio_init.data1_gpio.io, "mmc0-data1");
            gpio_direction_output(rksdmmc0_gpio_init.data1_gpio.io,GPIO_HIGH);//set mmc0-data1 to high.

            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            rk30_mux_api_set(rksdmmc0_gpio_init.data2_gpio.iomux.name, rksdmmc0_gpio_init.data2_gpio.iomux.fgpio);
            #endif
            gpio_request(rksdmmc0_gpio_init.data2_gpio.io, "mmc0-data2");
            gpio_direction_output(rksdmmc0_gpio_init.data2_gpio.io,GPIO_HIGH);//set mmc0-data2 to high.

            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            rk30_mux_api_set(rksdmmc0_gpio_init.data3_gpio.iomux.name, rksdmmc0_gpio_init.data3_gpio.iomux.fgpio);
            #endif
            gpio_request(rksdmmc0_gpio_init.data3_gpio.io, "mmc0-data3");
            gpio_direction_output(rksdmmc0_gpio_init.data3_gpio.io,GPIO_HIGH);//set mmc0-data3 to high.
    	}
    	break;
	}
}

static void rk29_sdmmc_set_iomux_mmc1(unsigned int bus_width)
{
    switch(bus_width)
    {          
        case 0xFFFF:
          #ifdef CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD 
            #if (!!SDMMC_USE_NEW_IOMUX_API) && !defined(CONFIG_SDMMC1_RK29_SDCARD_DET_FROM_GPIO)
    	    iomux_set(MMC1_DETN);
    	    #endif

            #if !(!!SDMMC_USE_NEW_IOMUX_API)
            rk30_mux_api_set(rksdmmc1_gpio_init.power_en_gpio.iomux.name, rksdmmc1_gpio_init.power_en_gpio.iomux.fgpio);
            #endif
            gpio_request(rksdmmc1_gpio_init.power_en_gpio.io,"sdio-power");
            gpio_direction_output(rksdmmc1_gpio_init.power_en_gpio.io, !(rksdmmc1_gpio_init.power_en_gpio.enable)); //power-off
            break;
         #endif 
        default:
            #if SDMMC_USE_NEW_IOMUX_API
            iomux_set(rksdmmc1_gpio_init.cmd_gpio.iomux.fmux);
            iomux_set(rksdmmc1_gpio_init.clk_gpio.iomux.fmux);
            iomux_set(rksdmmc1_gpio_init.data0_gpio.iomux.fmux);
            iomux_set(rksdmmc1_gpio_init.data1_gpio.iomux.fmux);
            iomux_set(rksdmmc1_gpio_init.data2_gpio.iomux.fmux);
            iomux_set(rksdmmc1_gpio_init.data3_gpio.iomux.fmux);
            #else
            rk30_mux_api_set(rksdmmc1_gpio_init.cmd_gpio.iomux.name, rksdmmc1_gpio_init.cmd_gpio.iomux.fmux);
            rk30_mux_api_set(rksdmmc1_gpio_init.clk_gpio.iomux.name, rksdmmc1_gpio_init.clk_gpio.iomux.fmux);
            rk30_mux_api_set(rksdmmc1_gpio_init.data0_gpio.iomux.name, rksdmmc1_gpio_init.data0_gpio.iomux.fmux);
            rk30_mux_api_set(rksdmmc1_gpio_init.data1_gpio.iomux.name, rksdmmc1_gpio_init.data1_gpio.iomux.fmux);
            rk30_mux_api_set(rksdmmc1_gpio_init.data2_gpio.iomux.name, rksdmmc1_gpio_init.data2_gpio.iomux.fmux);
            rk30_mux_api_set(rksdmmc1_gpio_init.data3_gpio.iomux.name, rksdmmc1_gpio_init.data3_gpio.iomux.fmux);
            #endif

            break;
   }
     //IO voltage(vcc-ap0)
    if(rk31sdk_wifi_voltage_select())
        SDMMC_write_grf_reg(GRF_IO_CON4, (SDMMC1_IO_VOLTAGE_MASK|SDMMC1_IO_VOLTAGE_18));       
    else
        SDMMC_write_grf_reg(GRF_IO_CON4, (SDMMC1_IO_VOLTAGE_MASK|SDMMC1_IO_VOLTAGE_33));

    //sdmmc1 drive strength control
    SDMMC_write_grf_reg(GRF_IO_CON3, (SDMMC1_DRIVER_STRENGTH_MASK|SDMMC1_DRIVER_STRENGTH_12MA));
    
}

#ifdef CONFIG_SDMMC2_RK29
static void rk29_sdmmc_set_iomux_mmc2(unsigned int bus_width)
{
#if 0
#if defined(CONFIG_SDMMC2_RK29)
   #if 0
    iomux_set(rksdmmc2_gpio_init.cmd_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.clk_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.rstnout_gpio.iomux.fmux);
  #else
  	iomux_set(EMMC_CLKOUT);
	iomux_set(EMMC_CMD);
	iomux_set(EMMC_RSTNOUT);
  #endif
    printk("%d..%s: =====test =========\n", __LINE__, __FUNCTION__);
    #ifdef CONFIG_ARCH_RK3026
    printk("%d..%s: =====test =========\n", __LINE__, __FUNCTION__);
    iomux_set(rksdmmc2_gpio_init.data0_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.data1_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.data2_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.data3_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.data4_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.data6_gpio.iomux.fmux);
    iomux_set(rksdmmc2_gpio_init.data7_gpio.iomux.fmux);
   #endif
#if 0///
    SDMMC_write_grf_reg(GRF_IO_CON4, (SDMMC2_IO_VOLTAGE_MASK|SDMMC2_IO_VOLTAGE_18));
    SDMMC_write_grf_reg(GRF_IO_CON0, (SDMMC2_DRIVER_STRENGTH_MASK|SDMMC2_DRIVER_STRENGTH_2MA));
    SDMMC_write_grf_reg(GRF_IO_CON0, (SDMMC2_D47_DRIVER_STRENGTH_MASK|SDMMC2_D47_DRIVER_STRENGTH_2MA));
   #endif 
#else
    ;//
#endif    
#endif


#if defined(CONFIG_ARCH_RK2928)
    rk30_mux_api_set(rksdmmc2_gpio_init.cmd_gpio.iomux.name, rksdmmc2_gpio_init.cmd_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.clk_gpio.iomux.name, rksdmmc2_gpio_init.clk_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.rstnout_gpio.iomux.name, rksdmmc2_gpio_init.rstnout_gpio.iomux.fmux);
    
    rk30_mux_api_set(rksdmmc2_gpio_init.data0_gpio.iomux.name, rksdmmc2_gpio_init.data0_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.data1_gpio.iomux.name, rksdmmc2_gpio_init.data1_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.data2_gpio.iomux.name, rksdmmc2_gpio_init.data2_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.data3_gpio.iomux.name, rksdmmc2_gpio_init.data3_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.data4_gpio.iomux.name, rksdmmc2_gpio_init.data4_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.data5_gpio.iomux.name, rksdmmc2_gpio_init.data5_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.data6_gpio.iomux.name, rksdmmc2_gpio_init.data6_gpio.iomux.fmux);
    rk30_mux_api_set(rksdmmc2_gpio_init.data7_gpio.iomux.name, rksdmmc2_gpio_init.data7_gpio.iomux.fmux);
#else
	iomux_set(EMMC_CLKOUT);
	iomux_set(EMMC_CMD);
	iomux_set(EMMC_RSTNOUT);

    #ifdef CONFIG_ARCH_RK3026
	iomux_set(EMMC_PWREN);
	iomux_set(EMMC_D0);
	iomux_set(EMMC_D1);
	iomux_set(EMMC_D2);
	iomux_set(EMMC_D3);
	iomux_set(EMMC_D4);
	iomux_set(EMMC_D5);
	iomux_set(EMMC_D6);
	iomux_set(EMMC_D7);
    #endif
#endif

}
#endif

static void rk29_sdmmc_set_iomux(int device_id, unsigned int bus_width)
{
    switch(device_id)
    {
        case 0:
            #ifdef CONFIG_SDMMC0_RK29
            rk29_sdmmc_set_iomux_mmc0(bus_width);
            #endif
            break;
        case 1:
            #ifdef CONFIG_SDMMC1_RK29
            rk29_sdmmc_set_iomux_mmc1(bus_width);
            #endif
            break;
        case 2:
            #ifdef CONFIG_SDMMC2_RK29
            rk29_sdmmc_set_iomux_mmc2(bus_width);
            #endif
            break;
        default:
            break;
    }    
}

static int sdmmc_is_selected_emmc(int device_id)
{ 
   int ret = 0;
   switch(device_id)
   {
        case 0:
            break;
        case 1:
            break;
        case 2:
        {
  #if defined(CONFIG_SDMMC2_RK29)      
        #ifdef CONFIG_ARCH_RK3026
        	if((iomux_is_set(rksdmmc2_gpio_init.clk_gpio.iomux.fmux) == 1) &&
        	   (iomux_is_set(rksdmmc2_gpio_init.cmd_gpio.iomux.fmux) == 1) &&
        	   (iomux_is_set(rksdmmc2_gpio_init.data0_gpio.iomux.fmux) == 1))
        		ret=1;
        #else
        	if(SDMMC_read_grf_reg(RK_EMMC_FLASH_REGISTER) & RK_EMMC_FLAHS_SEL)
        		ret=1;
        #endif
  #endif      
            break;
        }
        default:
            break;
    }
    if(1==ret)
        printk("%d..%s: RK SDMMC is setted to support eMMC.\n", __LINE__, __FUNCTION__);
    else
        printk("%d..%s: RK SDMMC is not setted to support eMMC.\n", __LINE__, __FUNCTION__);
    return ret;
}
#endif


