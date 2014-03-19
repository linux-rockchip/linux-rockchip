#ifndef _LCD_EE070A__
#define _LCD_EE070A__

/* Base */
#define SCREEN_TYPE             SCREEN_LVDS
#define LVDS_FORMAT             LVDS_8BIT_1
#define OUT_FACE                OUT_P888
#define DCLK                    51000000
#define LCDC_ACLK               150000000     //29 lcdc axi DMA ÆµÂÊ

/* Timing */
#define H_PW                    100
#define H_BP                    100
#define H_VD                    1024
#define H_FP                    120

#define V_PW                    3
#define V_BP                    23
#define V_VD                    600
#define V_FP                    9

/* Other */
#define DCLK_POL                1
#define DEN_POL                 0
#define VSYNC_POL               0
#define HSYNC_POL               0

#define SWAP_RB                 0
#define SWAP_RG                 0
#define SWAP_GB                 0 



#define LCD_WIDTH               160    //need modify
#define LCD_HEIGHT              101

#endif
