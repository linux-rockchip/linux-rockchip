#include <linux/fb.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/board.h>
#include <linux/rk_screen.h>

#define SCREEN_TYPE		SCREEN_RGB
#define LVDS_FORMAT		LVDS_8BIT_1
#define OUT_FACE		OUT_P888
#define LCDC_ACLK		312000000

#define DCLK	    	27000000
#define H_PW			62
#define H_BP			60
#define H_VD			720
#define H_FP			16
#define V_PW			6
#define V_BP			30
#define V_VD			480
#define V_FP			9

#define LCD_WIDTH       0//720*10
#define LCD_HEIGHT      0//480*10
/* Other */
#define DCLK_POL		1
#define DEN_POL			0
#define VSYNC_POL		1
#define HSYNC_POL		1

#define SWAP_RB			0
#define SWAP_RG			0
#define SWAP_GB			0 



