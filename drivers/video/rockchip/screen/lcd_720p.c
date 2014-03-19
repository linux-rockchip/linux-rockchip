#include <linux/fb.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/board.h>
#include <linux/rk_fb.h>
#include <linux/rk_screen.h>

#define SCREEN_TYPE		SCREEN_RGB
#define LVDS_FORMAT		LVDS_8BIT_1
#define OUT_FACE		OUT_P888
#define LCDC_ACLK		312000000

#define DCLK	    	74250000
#define H_PW			40
#define H_BP			220
#define H_VD			1280
#define H_FP			110
#define V_PW			5
#define V_BP			20
#define V_VD			720
#define V_FP			5

#define LCD_WIDTH       0//1280
#define LCD_HEIGHT      0//720
/* Other */
#define DCLK_POL		1
#define DEN_POL			0
#define VSYNC_POL		1
#define HSYNC_POL		1

#define SWAP_RB			0
#define SWAP_RG			0
#define SWAP_GB			0 


