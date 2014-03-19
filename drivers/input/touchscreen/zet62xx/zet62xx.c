/**
 * @file drivers/input/touchscreen/zet62xx.c
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ZEITEC Semiconductor Co., Ltd
  * @author JLJuang <JL.Juang@zeitecsemi.com>
 * @note Copyright (c) 2010, Zeitec Semiconductor Ltd., all rights reserved.
 * @version $Revision: 14 $
 * @note
*/


#include <asm/types.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/input-polldev.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <mach/iomux.h>
#include <mach/gpio.h>
#include <mach/board.h>
#include <linux/wakelock.h>

#include <linux/input.h>
#include <linux/slab.h>

#include <linux/init.h>
#include <linux/i2c.h>

#include <linux/pm.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/platform_device.h>
#include <linux/async.h>
#include <linux/hrtimer.h>
#include <linux/ioport.h>
#include <linux/kthread.h>
#include <linux/input/mt.h>
#include <linux/fs.h> 
#include <linux/file.h> 

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/hardware.h>


#include "zet6221_fw.h"
#include "zet6223_fw.h"
#include "zet6231_fw.h"
#include "zet6251_fw.h"


///=============================================================================================///
/// 驅動檢查表(Checklist): 請依客戶需求，來決定下列的宏是否應該開啟或關閉
///=============================================================================================///
///---------------------------------------------------------------------------------///
///  1. FW Upgrade
///---------------------------------------------------------------------------------///
#define FEATURE_FW_UPGRADE			///< 啟動下載器(downloader) 進行固件燒錄
#ifdef FEATURE_FW_UPGRADE
	#define FEATURE_FW_SIGNATURE		///< 固件燒綠後, 燒入簽章
	#define FEATURE_FW_COMPARE		///< 固件燒綠後，進行比對
	#define FEATURE_FW_UPGRADE_RESUME	///< 休眠喚醒後，是否進行固件燒錄
		#define FEATURE_FW_CHECK_SUM       ///< ZET6251 休眠喚醒後，固件燒錄比對,
	#define FEATURE_FW_SKIP_FF		///< 固件燒綠時，放棄全為0xFF的頁面
#endif ///< for FEATURE_FW_UPGRADE

///---------------------------------------------------------------------------------///
///  2. Hardware check only and do no FW upgrade
///---------------------------------------------------------------------------------///
//#define FEATURE_HW_CHECK_ONLY			///< [除錯用(debug)]僅比固件對版，不進行固件燒錄

///---------------------------------------------------------------------------------///
///  3. Read TP information (B2 Command)
///---------------------------------------------------------------------------------///
//#define FEATURE_TPINFO				///< 從IC讀聽手指數，分辨率及按錄啟動(B2指令）

///---------------------------------------------------------------------------------///
///  4. Virtual key
///---------------------------------------------------------------------------------///
//#define FEATURE_VIRTUAL_KEY			///< 驅動虛擬按鍵開啟 （注意：非固件或是硬件按鍵） 

///---------------------------------------------------------------------------------///
///  5. Multi-touch type B
///---------------------------------------------------------------------------------///
#define FEATURE_MT_TYPE_B			///< 安卓multitouch type B protocol，可增進報點效率 (注意： 舊系統不支持）
//#define FEATURE_BTN_TOUCH			///< 舊系統按壓控制(逃離神廟）
#ifdef FEATURE_MT_TYPE_B
	#define FEAURE_LIGHT_LOAD_REPORT_MODE   ///<   減少重複的點往上報，可增進系統效能
	#define PRESSURE_CONST	(1)		
#endif ///< for FEATURE_MT_TYPE_B
///---------------------------------------------------------------------------------///
///  6. Hihg impedance mode (ZET6221)
///---------------------------------------------------------------------------------///
//#define FEATURE_HIGH_IMPEDENCE_MODE  		///< ZET6221 high impedance 模式開啟

///---------------------------------------------------------------------------------///
///  7. Coordinate translation
///---------------------------------------------------------------------------------///
//#define FEATURE_TRANSLATE_ENABLE		///< 驅動原點轉換功能(注意：請使用固件的原點轉換)

///---------------------------------------------------------------------------------///
///  8. Auto Zoom translation
///---------------------------------------------------------------------------------///
//#define FEATURE_AUTO_ZOOM_ENABLE			///< FW to driver XY auto zoom in

///---------------------------------------------------------------------------------///
///  9. Firmware download check the last page
///---------------------------------------------------------------------------------///
//#define FEATURE_CHECK_LAST_PAGE 		///< 增加固件最後一頁比對，來決定是否燒錄 

///---------------------------------------------------------------------------------///
///  10. Dummy report (without pull high resistor)
///---------------------------------------------------------------------------------///
//#define FEATURE_DUMMY_REPORT			///< 重啟後，INT低位時不讀點(無上拉電阻，請開啟）
#ifdef FEATURE_DUMMY_REPORT
	#define SKIP_DUMMY_REPORT_COUNT		(1) ///< skip # times int low, if there is no pull high resistor, used 1
#else ///< for FEATURE_FUMMY_REPORT
	#define SKIP_DUMMY_REPORT_COUNT		(0) ///< skip # times int low, if there is no pull high resistor, used 1
#endif ///< for FEATURE_FUMMY_REPORT

///---------------------------------------------------------------------------------///
///  11. Finger number 
///---------------------------------------------------------------------------------///
#define FINGER_NUMBER 				(5)		///< 設定手指數，若有開TPINFO，則以TPINFO為主

///---------------------------------------------------------------------------------///
///  12. key number 
///---------------------------------------------------------------------------------///
#define KEY_NUMBER 				(0)		///< 設定按錄數，若有開TPINFO，則以TPINFO為主

///---------------------------------------------------------------------------------///
///  13. Finger up debounce count  
///---------------------------------------------------------------------------------///
#define DEBOUNCE_NUMBER				(1)		///< 收到幾次無手指，則斷掉線，預設1次 

///=========================================================================================///
///  14. Device Name 
///=========================================================================================///

#define ZET_TS_ID_NAME 			"zet6221-ts"
#define MJ5_TS_NAME 			  "zet6221_touchscreen" ///< ZET_TS_ID_NAME

///=========================================================================================///
///  15.Charge mode  
///=========================================================================================///
#define FEATURE_CHARGER_MODE		///< 請修改AXP20-sply.c，將充電模式啟動

///---------------------------------------------------------------------------------///
///  16. IOCTRL Debug
///---------------------------------------------------------------------------------///
#define FEATURE_IDEV_OUT_ENABLE
#define FEATURE_MBASE_OUT_ENABLE
#define FEATURE_MDEV_OUT_ENABLE
#define FEATURE_INFO_OUT_EANBLE
#define FEATURE_IBASE_OUT_ENABLE


///=============================================================================================///
/// 驅動檢查表結束
///=============================================================================================///
///=============================================================================================///
/// Reset Timing 
///=============================================================================================///
#define TS_RESET_LOW_PERIOD			(1)		///< 開機重啟： RST 從高位轉低位持續1ms再復高位
#define TS_INITIAL_HIGH_PERIOD			(30)		///< 開啟重啟： 承上，RST 轉高位後，特續30ms
#define TS_WAKEUP_LOW_PERIOD			(10)		///< 喚醒重啟： RST 從高位轉低位持續20ms再復高位 
#define TS_WAKEUP_HIGH_PERIOD			(20)            ///< 喚醒重啟： 承上，RST 轉高位後，特續20m

///=============================================================================================///
/// Device numbers
///=============================================================================================///
#define I2C_MINORS 				(256)		///< 副裝置碼上限
#define I2C_MAJOR 				(126)		///< 主裝置碼

///=============================================================================================///
/// Flash control Definition
///=============================================================================================///
#define CMD_WRITE_PASSWORD			(0x20)
	#define CMD_PASSWORD_HIBYTE			(0xC5)
	#define CMD_PASSWORD_LOBYTE			(0x9D)

	#define CMD_PASSWORD_1K_HIBYTE			(0xB9)
	#define CMD_PASSWORD_1K_LOBYTE			(0xA3)
	
	#define CMD_WRITE_PASSWORD_LEN			(3)
#define CMD_WRITE_CODE_OPTION			(0x21)
#define CMD_WRITE_PROGRAM			(0x22)
#define CMD_PAGE_ERASE				(0x23)
	#define CMD_PAGE_ERASE_LEN         		(2)
#define CMD_MASS_ERASE				(0x24)
#define CMD_PAGE_READ_PROGRAM			(0x25)
	#define CMD_PAGE_READ_PROGRAM_LEN		(2)
#define CMD_MASS_READ_PROGRAM			(0x26)
#define CMD_READ_CODE_OPTION			(0x27)
#define CMD_ERASE_CODE_OPTION			(0x28)
#define CMD_RESET_MCU				(0x29)
#define CMD_OUTPUT_CLOCK			(0x2A)
#define CMD_WRITE_SFR				(0x2B)
#define CMD_READ_SFR				(0x2C)
	#define SFR_UNLOCK_FLASH			(0x3D)
	#define SFR_LOCK_FLASH				(0x7D)	
#define CMD_ERASE_SPP				(0x2D)
#define CMD_WRITE_SPP				(0x2E)
#define CMD_READ_SPP				(0x2F)
#define CMD_PROG_INF				(0x30)
#define CMD_PROG_MAIN				(0x31)
#define CMD_PROG_CHECK_SUM			(0x36)
#define CMD_PROG_GET_CHECK_SUM			(0x37)
#define CMD_OUTPUT_CLOCK1			(0x3B)
#define CMD_FILL_FIFO				(0x60)
#define CMD_READ_FIFO				(0x61)

#define FLASH_PAGE_LEN				(128)


#define  FLASH_SIZE_ZET6221			(0x4000)
#define  FLASH_SIZE_ZET6223			(0x10000)
#define  FLASH_SIZE_ZET6231			(0x8000)

///=============================================================================================///
/// Macro Definition
///=============================================================================================///
#define MAX_FLASH_BUF_SIZE			(0x10000)

/// for debug INT
#define GPIO_BASE                		(0x01c20800)
#define GPIO_RANGE               		(0x400)
#define PH2_CTRL_OFFSET          		(0x104)
#define PH_DATA_OFFSET          		(0x10c)

///=========================================================================================///
///  TP related define : configured for all tp
///=========================================================================================///

/// Boolean definition
#define TRUE 					(1)
#define FALSE 					(0)

/// Origin definition
#define ORIGIN_TOP_RIGHT			(0)
#define ORIGIN_TOP_LEFT  			(1)
#define ORIGIN_BOTTOM_RIGHT			(2)
#define ORIGIN_BOTTOM_LEFT			(3)

#define ORIGIN					(ORIGIN_BOTTOM_RIGHT)

/// Max key number
#define MAX_KEY_NUMBER    			(8)

/// Max finger number
#define MAX_FINGER_NUMBER			(16)

/// X, Y Resolution
#define FW_X_RESOLUTION				(1024)		///< the FW setting X resolution
#define FW_Y_RESOLUTION				(600)		///< the FW setting Y resolution
#define X_MAX	 				(800)		///< the X resolution of TP AA(Action Area)
#define Y_MAX 					(480)		///< the Y resolution of TP AA(Action Area)

///=========================================================================================///
///  Model Type
///=========================================================================================///
#define MODEL_ZET6221				(0)
#define MODEL_ZET6223				(1)
#define MODEL_ZET6231				(2)
#define MODEL_ZET6241				(3)
#define MODEL_ZET6251				(4)

///=========================================================================================///
///  Rom Type
///=========================================================================================///
#define ROM_TYPE_UNKNOWN			(0x00)
#define ROM_TYPE_SRAM				(0x02)
#define ROM_TYPE_OTP				(0x06)
#define ROM_TYPE_FLASH				(0x0F)

///=========================================================================================///
///  Working queue error number
///=========================================================================================///
#define ERR_WORK_QUEUE_INIT_FAIL		(100)
#define ERR_WORK_QUEUE1_INIT_FAIL		(101)

///=========================================================================================///
///  Virtual Key
///=========================================================================================///
#ifdef FEATURE_VIRTUAL_KEY
  #define TP_AA_X_MAX				(480)	///< X resolution of TP VA(View Area)
  #define TP_AA_Y_MAX				(600)   ///< Y resolution of TP VA(View Area)
#endif ///< for FEATURE_VIRTUAL_KEY

///=========================================================================================///
///  Impedance byte
///=========================================================================================///
#define IMPEDENCE_BYTE	 			(0xf1)	///< High Impendence Mode : (8M) 0xf1 (16M) 0xf2 

#define P_MAX					(255)
#define S_POLLING_TIME  			(100)

///=========================================================================================///
///  Signature
///=========================================================================================///
#ifdef FEATURE_FW_SIGNATURE
#define SIG_PAGE_ID             		(255)   ///< 簽章所在的頁
#define SIG_DATA_ADDR           		(128  - SIG_DATA_LEN)   ///< 簽章所在的位址
#define SIG_DATA_LEN            		(4)     ///< 簽章所在的頁
static const u8 sig_data[SIG_DATA_LEN] 		= {'Z', 'e', 'i', 'T'};
#endif ///< for FEATURE_FW_SIGNATURE

///=============================================================================================///
/// IOCTL control Definition
///=============================================================================================///
#define ZET_IOCTL_CMD_FLASH_READ		(20)
#define ZET_IOCTL_CMD_FLASH_WRITE		(21)
#define ZET_IOCTL_CMD_RST      			(22)
#define ZET_IOCTL_CMD_RST_HIGH 		   	(23)
#define ZET_IOCTL_CMD_RST_LOW    		(24)

#define ZET_IOCTL_CMD_DYNAMIC			(25)

#define ZET_IOCTL_CMD_FW_FILE_PATH_GET		(26)
#define ZET_IOCTL_CMD_FW_FILE_PATH_SET   	(27)

#define ZET_IOCTL_CMD_MDEV   			(28)
#define ZET_IOCTL_CMD_MDEV_GET   		(29)

#define ZET_IOCTL_CMD_TRAN_TYPE_PATH_GET	(30)
#define ZET_IOCTL_CMD_TRAN_TYPE_PATH_SET	(31)

#define ZET_IOCTL_CMD_IDEV   			(32)
#define ZET_IOCTL_CMD_IDEV_GET   		(33)

#define ZET_IOCTL_CMD_MBASE   			(34)
#define ZET_IOCTL_CMD_MBASE_GET  		(35)

#define ZET_IOCTL_CMD_INFO_SET			(36)
#define ZET_IOCTL_CMD_INFO_GET			(37)

#define ZET_IOCTL_CMD_TRACE_X_SET		(38)
#define ZET_IOCTL_CMD_TRACE_X_GET		(39)

#define ZET_IOCTL_CMD_TRACE_Y_SET		(40)
#define ZET_IOCTL_CMD_TRACE_Y_GET		(41)

#define ZET_IOCTL_CMD_IBASE   			(42)
#define ZET_IOCTL_CMD_IBASE_GET   		(43)

#define ZET_IOCTL_CMD_DRIVER_VER_GET		(44)
#define ZET_IOCTL_CMD_MBASE_EXTERN_GET  		(45)

#define IOCTL_MAX_BUF_SIZE          		(1024)

///----------------------------------------------------///
/// IOCTL ACTION
///----------------------------------------------------///
#define IOCTL_ACTION_NONE			(0)
#define IOCTL_ACTION_FLASH_DUMP			(1<<0)

static int ioctl_action = IOCTL_ACTION_NONE;

///=============================================================================================///
///  Transfer type
///=============================================================================================///
#define TRAN_TYPE_DYNAMIC		        (0x00)
#define TRAN_TYPE_MUTUAL_SCAN_BASE         	(0x01)
#define TRAN_TYPE_MUTUAL_SCAN_DEV           	(0x02)
#define TRAN_TYPE_INIT_SCAN_BASE 		(0x03)
#define TRAN_TYPE_INIT_SCAN_DEV		      	(0x04)
#define TRAN_TYPE_KEY_MUTUAL_SCAN_BASE		(0x05)
#define TRAN_TYPE_KEY_MUTUAL_SCAN_DEV 		(0x06)
#define TRAN_TYPE_KEY_DATA  			(0x07)
#define TRAN_TYPE_MTK_TYPE  			(0x0A)
#define TRAN_TYPE_FOCAL_TYPE  		        (0x0B)

///=============================================================================================///
///  TP Trace
///=============================================================================================///
#define TP_DEFAULT_ROW (15)
#define TP_DEFAULT_COL (10)


///=============================================================================================///
/// Macro Definition
///=============================================================================================///

struct i2c_dev
{
	struct list_head list;	
	struct i2c_adapter *adap;
	struct device *dev;
};

static struct class *i2c_dev_class;
static LIST_HEAD (i2c_dev_list);
static DEFINE_SPINLOCK(i2c_dev_list_lock);

static union
{
	unsigned short		dirty_addr_buf[2];
	const unsigned short	normal_i2c[2];
}u_i2c_addr = {{0x00},};


///=========================================================================================///
/// TP related parameters 
///=========================================================================================///

/// resolutions setting
static u16 resolution_x		= X_MAX;
static u16 resolution_y		= Y_MAX;

/// Finger and key
static u16 finger_num		= 0;
static u16 key_num		= 0;
static int finger_packet_size	= 0;	///< Finger packet buffer size	

static u8 xy_exchange         	= 0;
static u16 finger_up_cnt	= 0;	///< recieved # finger up count

static u8 pcode[8];  			///< project code[] from b2
static u8 sfr_data[16]		= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static u8 ic_model		= MODEL_ZET6221;	///< MODEL_ZET6221, MODE_ZET6223, MODEL_ZET6231
#define DRIVER_VERSION "$Revision: 14 $"

///=========================================================================================///
///  the light load report mode
///=========================================================================================///
#ifdef FEAURE_LIGHT_LOAD_REPORT_MODE
#define PRE_PRESSED_DEFAULT_VALUE            (-1)
struct light_load_report_mode
{
	u32 pre_x;
	u32 pre_y;
	u32 pre_z;
	int pressed;
};
static struct light_load_report_mode pre_event[MAX_FINGER_NUMBER];
#endif ///< for  FEAURE_LIGHT_LOAD_REPORT_MODE

///----------------------------------------------------///
/// FW variables
///----------------------------------------------------///
static u16 pcode_addr[8]	= {0x3DF1,0x3DF4,0x3DF7,0x3DFA,0x3EF6,0x3EF9,0x3EFC,0x3EFF}; ///< default pcode addr: zet6221
static u16 pcode_addr_6221[8]	= {0x3DF1,0x3DF4,0x3DF7,0x3DFA,0x3EF6,0x3EF9,0x3EFC,0x3EFF}; ///< zet6221 pcode_addr[8]
static u16 pcode_addr_6223[8]	= {0x7BFC,0x7BFD,0x7BFE,0x7BFF,0x7C04,0x7C05,0x7C06,0x7C07}; ///< zet6223 pcode_addr[8]

static int dummy_report_cnt	= 0;
static int charger_status	= 0;	///< 0 : discharge,  1 : charge
static u16 polling_time		= S_POLLING_TIME;
static u8 hover_status		= 0;

static u8 download_ok 		= FALSE;

///-------------------------------------///
/// key variables
///-------------------------------------///
static u8 key_menu_pressed	= 0x00;	///< key#0
static u8 key_back_pressed	= 0x00;	///< key#1
static u8 key_home_pressed	= 0x00;	///< key#2
static u8 key_search_pressed	= 0x00;	///< key#3

static u8 zet_tx_data[131] __initdata;
static u8 zet_rx_data[131] __initdata;

#ifdef FEATURE_FW_UPGRADE
static u8 firmware_upgrade	= TRUE; 
static u8 rom_type		= ROM_TYPE_UNKNOWN; ///< Flash:0xf SRAM:0x2 OTP:0x6
#endif /// for FEATURE_FW_UPGRADE

///=========================================================================================///
/// suspend no read any finger packet
///=========================================================================================///
static u8 suspend_mode 		= FALSE;

///=========================================================================================///
/// resume wait download finish then send charger mode
///=========================================================================================///
#ifdef FEATURE_FW_UPGRADE_RESUME
static u8 resume_download	= FALSE;
#endif ///< for FEATURE_FW_UPGRADE_RESUME
#ifdef FEATURE_VIRTUAL_KEY
static int tpd_keys_dim[4][4] = 
{
///	{X_LEFT_BOUNDARY,X_RIGHT_BOUNDARY,Y_TOP_BOUNDARY,Y_BOTTOM_BOUNDARY}
	{33, 122, 897, 1019},
	{184, 273, 879, 1019},
	{363, 451, 879, 1019},	
	{527, 615, 879, 1019},	
};
#endif ///< for FEATURE_VIRTUAL_KEY

#ifdef FEATURE_HW_CHECK_ONLY
	#ifndef FEATURE_FW_UPGRADE
		#define FEATURE_FW_UPGRADE
  	#endif ///< for FEATURE_FW_UPGRADE
	firmware_upgrade = FALSE;
#endif ///< for FEATURE_HW_CHECK_ONLY

///-------------------------------------///
///  firmware save / load
///-------------------------------------///
#define FW_FILE_NAME 		"/system/vendor/zet62xx.bin"
u32 data_offset 		= 0;
u8 *flash_buffer 		= NULL;
struct inode *inode 		= NULL;
mm_segment_t old_fs;

char driver_version[128];
char fw_file_name[128];
///-------------------------------------///
///  Transmit Type Mode Path parameters 
///-------------------------------------///
///  External SD-Card could be
///      "/mnt/sdcard/"
///      "/mnt/extsd/"
///-------------------------------------///
#define TRAN_MODE_FILE_PATH		"/mnt/sdcard/"
char tran_type_mode_file_name[128];
u8 *tran_data = NULL;

///-------------------------------------///
///  Mutual Dev Mode  parameters 
///-------------------------------------///
///  External SD-Card could be
///      "/mnt/sdcard/zetmdev"
///      "/mnt/extsd/zetmdev"
///-------------------------------------///
#ifdef FEATURE_MDEV_OUT_ENABLE
	#define MDEV_FILE_NAME		"zetmdev"
	#define MDEV_MAX_FILE_ID	(10)
	#define MDEV_MAX_DATA_SIZE	(2048)
///-------------------------------------///
///  mutual dev variables
///-------------------------------------///
	u8 *mdev_data = NULL;
	int mdev_file_id = 0;
#endif ///< FEATURE_MDEV_OUT_ENABLE

///-------------------------------------///
///  Initial Base Mode  parameters 
///-------------------------------------///
///  External SD-Card could be
///      "/mnt/sdcard/zetibase"
///      "/mnt/extsd/zetibase"
///-------------------------------------///
#ifdef FEATURE_IBASE_OUT_ENABLE
	#define IBASE_FILE_NAME		"zetibase"
	#define IBASE_MAX_FILE_ID	(10)
	#define IBASE_MAX_DATA_SIZE	(512)

///-------------------------------------///
///  initial base variables
///-------------------------------------///
	u8 *ibase_data = NULL;
	int ibase_file_id = 0;
#endif ///< FEATURE_IBASE_OUT_ENABLE

///-------------------------------------///
///  Initial Dev Mode  parameters 
///-------------------------------------///
///  External SD-Card could be
///      "/mnt/sdcard/zetidev"
///      "/mnt/extsd/zetidev"
///-------------------------------------///
#ifdef FEATURE_IDEV_OUT_ENABLE
	#define IDEV_FILE_NAME		"zetidev"
	#define IDEV_MAX_FILE_ID	(10)	
	#define IDEV_MAX_DATA_SIZE	(512)

///-------------------------------------///
///  initial dev variables
///-------------------------------------///
	u8 *idev_data = NULL;
	int idev_file_id = 0;
#endif ///< FEATURE_IDEV_OUT_ENABLE

///-------------------------------------///
///  Mutual Base Mode  parameters 
///-------------------------------------///
///  External SD-Card could be
///      "/mnt/sdcard/zetmbase"
///      "/mnt/extsd/zetmbase"
///-------------------------------------///
#ifdef FEATURE_MBASE_OUT_ENABLE
	#define MBASE_FILE_NAME		"zetmbase"
	#define MBASE_MAX_FILE_ID	(10)
	#define MBASE_MAX_DATA_SIZE	(2048)

///-------------------------------------///
///  mutual base variables
///-------------------------------------///
	u8 *mbase_data = NULL;
	int mbase_file_id = 0;
#endif ///< FEATURE_MBASE_OUT_ENABLE

///-------------------------------------///
///  infomation variables
///-------------------------------------///
#ifdef FEATURE_INFO_OUT_EANBLE
	#define INFO_MAX_DATA_SIZE	(64)
	#define INFO_DATA_SIZE		(17)
	#define ZET6221_INFO		(0x00)
	#define ZET6231_INFO		(0x0B)
	#define ZET6223_INFO		(0x0D)
	#define ZET6251_INFO		(0x0C)	
	#define UNKNOW_INFO		(0xFF)
	u8 *info_data = NULL;
#endif ///< FEATURE_INFO_OUT_EANBLE
///-------------------------------------///
///  Default transfer type
///-------------------------------------///
u8 transfer_type = TRAN_TYPE_DYNAMIC;

///-------------------------------------///
///  Default TP TRACE
///-------------------------------------///
int row = TP_DEFAULT_ROW;
int col = TP_DEFAULT_COL;

///=========================================================================================///
///  TP related parameters/structures : configured for all tp
///=========================================================================================///
static struct task_struct *resume_download_task = NULL;
static struct i2c_client *this_client;

struct msm_ts_platform_data
{
	unsigned int x_max;
	unsigned int y_max;
	unsigned int pressure_max;
};

struct zet622x_tsdrv
{
	struct i2c_client *i2c_dev;
	struct work_struct work1; 		///< get point from ZET62xx task queue 
	struct work_struct work2; 		///<  write_cmd
	struct workqueue_struct *ts_workqueue;  ///< get point from ZET62xx task queue 
	struct workqueue_struct *ts_workqueue1; ///< write_cmd
	struct input_dev *input;
	struct timer_list zet622x_ts_timer_task;
	struct early_suspend early_suspend;
	unsigned int rst_gpio; /// reset gpio
	unsigned int irq_gpio; 			///< GPIO used for interrupt of TS1
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	unsigned int pressure_max;
	int (*get_system_charge_status)(void); 
};

/// Touch Screen id tables
static const struct i2c_device_id zet622x_ts_idtable[] =
{
      { ZET_TS_ID_NAME, 0 },
      { }
};



static int __devinit zet622x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devexit zet622x_ts_remove(struct i2c_client *dev);

s32 zet622x_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length);
s32 zet622x_i2c_read_tsdata(struct i2c_client *client, u8 *data, u8 length);

static int  zet_fw_size(void);
static void zet_fw_save(char *file_name);
static void zet_fw_load(char *file_name);
static void zet_fw_init(void);
#ifdef FEATURE_MDEV_OUT_ENABLE
static void zet_mdev_save(char *file_name);
#endif ///< FEATURE_MDEV_OUT_ENABLE
#ifdef FEATURE_IDEV_OUT_ENABLE
static void zet_idev_save(char *file_name);
#endif ///< FEATURE_IDEV_OUT_ENABLE
#ifdef FEATURE_IBASE_OUT_ENABLE
static void zet_ibase_save(char *file_name);
#endif ///< FEATURE_IBASE_OUT_ENABLE
#ifdef FEATURE_MBASE_OUT_ENABLE
static void zet_mbase_save(char *file_name);
#endif ///< FEATURE_MBASE_OUT_ENABLE
static void zet_information_save(char *file_name);

int __init zet622x_downloader( struct i2c_client *client, u8 upgrade, u8 *pRomType, u8 icmodel);

///**********************************************************************
///   [function]:  ctp_set_reset_low
///   [parameters]: void
///   [return]: void
///**********************************************************************
void ctp_set_reset_low(void)
{
	struct zet622x_tsdrv *zet6221_ts = i2c_get_clientdata(this_client);
	gpio_direction_output(zet6221_ts->rst_gpio, GPIO_LOW);
}

///**********************************************************************
///   [function]:  ctp_set_reset_high
///   [parameters]: void
///   [return]: void
///**********************************************************************
void ctp_set_reset_high(void)
{
	struct zet622x_tsdrv *zet6221_ts = i2c_get_clientdata(this_client);
	gpio_direction_output(zet6221_ts->rst_gpio, GPIO_HIGH);
}

///**********************************************************************
///   [function]:  ctp_wakeup
///   [parameters]: void
///   [return]: void
///**********************************************************************
static void ctp_wakeup(void)
{
	printk("[ZET] : %s. \n", __func__);
	ctp_set_reset_low();
	mdelay(TS_WAKEUP_LOW_PERIOD);
	ctp_set_reset_high();
	mdelay(TS_WAKEUP_HIGH_PERIOD);
}

///**********************************************************************
///   [function]:  ctp_wakeup2
///   [parameters]: delay_ms
///   [return]: void
///**********************************************************************
static void ctp_wakeup2(int delay_ms)
{

	printk("[ZET] : %s. \n", __func__);
	ctp_set_reset_low();
	mdelay(TS_WAKEUP_LOW_PERIOD);
	ctp_set_reset_high();
	mdelay(delay_ms);
}


///**********************************************************************
///   [function]:  zet622x_i2c_get_free_dev
///   [parameters]: adap
///   [return]: void
///**********************************************************************
static struct i2c_dev *zet622x_i2c_get_free_dev(struct i2c_adapter *adap) 
{
	struct i2c_dev *i2c_dev;

	if (adap->nr >= I2C_MINORS)
	{
		printk("[ZET] : i2c-dev:out of device minors (%d) \n", adap->nr);
		return ERR_PTR (-ENODEV);
	}

	i2c_dev = kzalloc(sizeof(*i2c_dev), GFP_KERNEL);
	if (!i2c_dev)
	{
		return ERR_PTR(-ENOMEM);
	}
	i2c_dev->adap = adap;

	spin_lock(&i2c_dev_list_lock);
	list_add_tail(&i2c_dev->list, &i2c_dev_list);
	spin_unlock(&i2c_dev_list_lock);
	
	return i2c_dev;
}

///**********************************************************************
///   [function]:  zet622x_i2c_dev_get_by_minor
///   [parameters]: index
///   [return]: i2c_dev
///**********************************************************************
static struct i2c_dev *zet622x_i2c_dev_get_by_minor(unsigned index)
{
	struct i2c_dev *i2c_dev;
	spin_lock(&i2c_dev_list_lock);
	
	list_for_each_entry(i2c_dev, &i2c_dev_list, list)
	{
		printk(" [ZET] : line = %d ,i2c_dev->adapt->nr = %d,index = %d.\n", __LINE__, i2c_dev->adap->nr, index);
		if(i2c_dev->adap->nr == index)
		{
		     goto LABEL_FOUND;
		}
	}
	i2c_dev = NULL;
	
LABEL_FOUND: 
	spin_unlock(&i2c_dev_list_lock);
	
	return i2c_dev ;
}

///**********************************************************************
///   [function]:  zet622x_i2c_read_tsdata
///   [parameters]: client, data, length
///   [return]: s32
///***********************************************************************
s32 zet622x_i2c_read_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr     = client->addr;
	msg.flags    = I2C_M_RD;
	msg.len      = length;
	msg.buf      = data;
	msg.scl_rate = 300*1000;
	return i2c_transfer(client->adapter,&msg, 1);
}

///**********************************************************************
///   [function]:  zet622x_i2c_write_tsdata
///   [parameters]: client, data, length
///   [return]: s32
///***********************************************************************
s32 zet622x_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr     = client->addr;
	msg.flags    = 0;
	msg.len      = length;
	msg.buf      = data;
	msg.scl_rate = 300*1000;
	return i2c_transfer(client->adapter,&msg, 1);
}

///**********************************************************************
///   [function]:  zet622x_cmd_sndpwd
///   [parameters]: client
///   [return]: u8
///**********************************************************************
u8 zet622x_cmd_sndpwd(struct i2c_client *client)
{
	u8 ts_cmd[3] = {CMD_WRITE_PASSWORD, CMD_PASSWORD_HIBYTE, CMD_PASSWORD_LOBYTE};	
	int ret;
	
	ret = zet622x_i2c_write_tsdata(client, ts_cmd, 3);
	return ret;
}

///**********************************************************************
///   [function]:  zet622x_cmd_sndpwd_1k (ZET6223 only)
///   [parameters]: client
///   [return]: u8
///**********************************************************************
u8 zet622x_cmd_sndpwd_1k(struct i2c_client *client)
{
	u8 ts_cmd[3] = {CMD_WRITE_PASSWORD, CMD_PASSWORD_1K_HIBYTE, CMD_PASSWORD_1K_LOBYTE};	
	int ret;
	
	ret = zet622x_i2c_write_tsdata(client, ts_cmd, 3);
	return ret;
}

///**********************************************************************
///   [function]:  zet622x_cmd_codeoption
///   [parameters]: client, romtype
///   [return]: u8
///**********************************************************************
u8 zet622x_cmd_codeoption(struct i2c_client *client, u8 *romtype)
{
	u8 ts_cmd[1] = {CMD_READ_CODE_OPTION};
	u8 code_option[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#ifdef FEATURE_HIGH_IMPEDENCE_MODE
	u8 ts_code_option_erase[1] = {CMD_ERASE_CODE_OPTION};
	u8 tx_buf[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif ///< for FEATURE_HIGH_IMPEDENCE_MODE

	int ret;
	u16 model;
	int i;
	
	printk("\n[ZET] : option write : "); 

	ret = zet622x_i2c_write_tsdata(client, ts_cmd, 1);

	msleep(1);
	
	printk("%02x ",ts_cmd[0]); 
	
	printk("\n[ZET] : read : "); 

	ret = zet622x_i2c_read_tsdata(client, code_option, 16);

	msleep(1);

	for(i = 0; i < 16; i++)
	{
		printk("%02x ",code_option[i]); 
	}
	printk("\n"); 

	model = 0x0;
	model = code_option[7];
	model = (model << 8) | code_option[6];

	/// Set the rom type	
	*romtype = (code_option[2] & 0xf0)>>4;
	
	switch(model)
	{ 
		case 0xFFFF: 
			ret = 1;
			ic_model = MODEL_ZET6221;
			for(i = 0; i < 8; i++)
			{
				pcode_addr[i] = pcode_addr_6221[i];
			}
			
#ifdef FEATURE_HIGH_IMPEDENCE_MODE
			if(code_option[2] != IMPEDENCE_BYTE)
			{
				///------------------------------------------///
				/// unlock the flash 
				///------------------------------------------///				
				if(zet622x_cmd_sfr_read(client) == 0)
				{
					return 0;
				}
				if(zet622x_cmd_sfr_unlock(client) == 0)
				{
					return 0;
				}
				///------------------------------------------///
				/// Erase Code Option
				///------------------------------------------///							
				ret = zet622x_i2c_write_tsdata(client, ts_code_option_erase, 1);
				msleep(50);

				///------------------------------------------///
				/// Write Code Option
				///------------------------------------------///	
				tx_buf[0] = CMD_WRITE_CODE_OPTION;
				tx_buf[1] = 0xc5;
				for(i = 2; i < 18; i++)
				{
					tx_buf[i]=code_option[i-2];
				}				
				tx_buf[4] = IMPEDENCE_BYTE;
			
				ret = zet622x_i2c_write_tsdata(client, tx_buf, 18);
				msleep(50);

				///------------------------------------------///
				/// Read Code Option back check
				///------------------------------------------///					
				ret = zet622x_i2c_write_tsdata(client, ts_cmd, 1);
				msleep(5);	
				printk("%02x ",ts_cmd[0]); 	
				printk("\n[ZET] : (2)read : "); 
				ret = zet622x_i2c_read_tsdata(client, code_option, 16);
				msleep(1);
				for(i = 0; i < 16; i++)
				{
					printk("%02x ",code_option[i]); 
				}
				printk("\n"); 
				
			}
#endif ///< for  FEATURE_HIGH_IMPEDENCE_MODE
      		break; 
		case 0x6231: 
		 	ret = 1;
			ic_model = MODEL_ZET6231;
			for(i = 0; i < 8; i++)
			{
				pcode_addr[i] = pcode_addr_6223[i];
			} 
		break;           
		case 0x6223:
		 	ret = 1;
			ic_model = MODEL_ZET6223;
			for(i = 0; i < 8; i++)
			{
				pcode_addr[i] = pcode_addr_6223[i];
			}
		break; 
    		case 0x6251:
			ic_model = MODEL_ZET6251;
			for(i = 0; i < 8; i++)
			{
				pcode_addr[i] = pcode_addr_6223[i];
			}
			break;
		default:
		 	ret = 1;
			ic_model = MODEL_ZET6223;
			for(i = 0; i < 8; i++)
			{
				pcode_addr[i] = pcode_addr_6223[i];
			}
		break;         
	} 

	return ret;
}

///**********************************************************************
///   [function]:  zet622x_cmd_sfr_read
///   [parameters]: client
///   [return]: u8
///**********************************************************************
u8 zet622x_cmd_sfr_read(struct i2c_client *client)
{

	u8 ts_cmd[1] = {CMD_READ_SFR};
	int ret;
	int i;
	
	printk("\n[ZET] : write : "); 

	ret = zet622x_i2c_write_tsdata(client, ts_cmd, 1);

	msleep(5);
	
	printk("%02x ",ts_cmd[0]); 
	
	printk("\n[ZET] : sfr_read : "); 

	ret = zet622x_i2c_read_tsdata(client, sfr_data, 16);

	msleep(1);

	for(i = 0; i < 16; i++)
	{
		printk("%02x ",sfr_data[i]); 
	}
	printk("\n"); 

	if((sfr_data[14] != SFR_UNLOCK_FLASH) && 
	   (sfr_data[14] != SFR_LOCK_FLASH))
	{
		printk("[ZET] : The SFR[14] shall be 0x3D or 0x7D\n"); 
		return FALSE;
	}
	
	return TRUE;
}

///**********************************************************************
///   [function]:  zet622x_cmd_sfr_unlock
///   [parameters]: client
///   [return]: u8
///**********************************************************************
u8 zet622x_cmd_sfr_unlock(struct i2c_client *client)
{
	u8 tx_buf[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int ret;
	int i;
	
	printk("\nsfr_update : "); 	
	for(i = 0 ; i < 16 ; i++)
	{
		tx_buf[i+1] = sfr_data[i];
		printk("%02x ",sfr_data[i]); 
	}
	printk("\n"); 

	if(sfr_data[14] != SFR_UNLOCK_FLASH)
	{
		tx_buf[0]  = CMD_WRITE_SFR;	
		tx_buf[15] = SFR_UNLOCK_FLASH;
		ret = zet622x_i2c_write_tsdata(client, tx_buf, 17);
	}
	
	return TRUE;
}

///***********************************************************************
///   [function]:  zet622x_cmd_masserase
///   [parameters]: client
///   [return]: u8
///************************************************************************
u8 zet622x_cmd_masserase(struct i2c_client *client)
{
	u8 ts_cmd[1] = {CMD_MASS_ERASE};
	
	int ret;

	ret = zet622x_i2c_write_tsdata(client, ts_cmd, 1);

	return ret;
}

///***********************************************************************
///   [function]:  zet622x_cmd_pageerase
///   [parameters]: client, npage
///   [return]: u8
///************************************************************************
u8 zet622x_cmd_pageerase(struct i2c_client *client, int npage)
{
	u8 ts_cmd[3] = {CMD_PAGE_ERASE, 0x00, 0x00};
	u8 len = 0;
	int ret;

	switch(ic_model)
	{
		case MODEL_ZET6221: ///< 6221
			ts_cmd[1] = npage;
			len = 2;
			break;
		case MODEL_ZET6223: ///< 6223
		case MODEL_ZET6231: ///< 6231
		case MODEL_ZET6251: ///< 6251
		default: 
			ts_cmd[1] = npage & 0xff;
			ts_cmd[2] = npage >> 8;
			len=3;
			break;
	}

	ret = zet622x_i2c_write_tsdata(client, ts_cmd, len);
		printk( " [ZET] : page erase\n");
	return TRUE;
}

///***********************************************************************
///   [function]:  zet622x_cmd_resetmcu
///   [parameters]: client
///   [return]: u8
///************************************************************************
u8 zet622x_cmd_resetmcu(struct i2c_client *client)
{
	u8 ts_cmd[1] = {CMD_RESET_MCU};
	
	int ret;

	ret = zet622x_i2c_write_tsdata(client, ts_cmd, 1);

	return ret;
}

///***********************************************************************
///   [function]:  zet622x_cmd_read_check_sum
///   [parameters]: client, page_id, buf
///   [return]: int
///************************************************************************
int zet622x_cmd_read_check_sum(struct i2c_client *client, int page_id, u8 * buf)
{
	int ret;
	int cmd_len = 3;

	buf[0] = CMD_PROG_CHECK_SUM;
	buf[1] = (u8)(page_id) & 0xff; 
	buf[2] = (u8)(page_id >> 8);   		
	ret = zet622x_i2c_write_tsdata(client, buf, cmd_len);
	if(ret <= 0)
	{
		printk("[ZET]: Read check sum fail");
		return ret;
	}

	buf[0] = CMD_PROG_GET_CHECK_SUM;
	cmd_len = 1;
	ret = zet622x_i2c_write_tsdata(client, buf, cmd_len);
	if(ret <= 0)
	{
		printk("[ZET]: Read check sum fail");
		return ret;
	}
	
	cmd_len = 1;
	ret = zet622x_i2c_read_tsdata(client, buf, cmd_len);
	if(ret <= 0)		
	{
		printk("[ZET]: Read check sum fail");
		return ret;
	}
	return 1;
}

///***********************************************************************
///   [function]:  zet622x_cmd_readpage
///   [parameters]: client, page_id, buf
///   [return]: int
///************************************************************************
int zet622x_cmd_readpage(struct i2c_client *client, int page_id, u8 * buf)
{
	int ret;
	int cmd_len = 3;

	switch(ic_model)
	{
		case MODEL_ZET6221:
			buf[0] = CMD_PAGE_READ_PROGRAM;
			buf[1] = (u8)(page_id); ///< (pcode_addr[0]/128);			
			cmd_len = 2;
			break;
		case MODEL_ZET6223: 
		case MODEL_ZET6231: 
		case MODEL_ZET6251: 
			buf[0] = CMD_PAGE_READ_PROGRAM;
			buf[1] = (u8)(page_id) & 0xff; ///< (pcode_addr[0]/128);
			buf[2] = (u8)(page_id >> 8);   ///< (pcode_addr[0]/128);			
			cmd_len = 3;
			break;
		default: 
			buf[0] = CMD_PAGE_READ_PROGRAM;
			buf[1] = (u8)(page_id) & 0xff; ///< (pcode_addr[0]/128);
			buf[2] = (u8)(page_id >> 8);   ///< (pcode_addr[0]/128);			
			cmd_len = 3;
			break;
	}
	
	ret = zet622x_i2c_write_tsdata(client, buf, cmd_len);
	if(ret <= 0)
	{
		printk("[ZET]: Read page command fail");
		return ret;
	}

	ret = zet622x_i2c_read_tsdata(client, buf, FLASH_PAGE_LEN);
	if(ret <= 0)		
	{
		printk("[ZET]: Read page data fail");
		return ret;
	}
	return 1;
}

///***********************************************************************
///   [function]:  zet622x_cmd_writepage
///   [parameters]: client, page_id, buf
///   [return]: int
///************************************************************************
int zet622x_cmd_writepage(struct i2c_client *client, int page_id, u8 * buf)
{
	int ret;
	int cmd_len = 131;
	int cmd_idx = 3;
	u8 tx_buf[256];
	int i;

	switch(ic_model)
	{
		case MODEL_ZET6221: ///< for 6221
			cmd_len = 130;
			
			tx_buf[0] = CMD_WRITE_PROGRAM;
			tx_buf[1] = page_id;
			cmd_idx = 2;
			break;
		case MODEL_ZET6223: ///< for 6223
		case MODEL_ZET6231: ///< for 6231
		case MODEL_ZET6251: ///< for 6251
		default: 
			cmd_len = 131;
			
			tx_buf[0] = CMD_WRITE_PROGRAM;
			tx_buf[1] = (page_id & 0xff);
			tx_buf[2] = (u8)(page_id >> 8);
			cmd_idx = 3;
			break;
	}

	for(i = 0; i < FLASH_PAGE_LEN; i++)
	{
		tx_buf[i + cmd_idx] = buf[i];
	}
	ret = zet622x_i2c_write_tsdata(client, tx_buf, cmd_len);
	if(ret <= 0)
	{
		printk("[ZET] : write page %d failed!!", page_id);
	}
	return ret;
}

///***********************************************************************
///   [function]:  zet622x_ts_check_version
///   [parameters]: void
///   [return]: void
///************************************************************************
u8 zet622x_ts_check_version(void)
{	
	int i;
		
	printk("[ZET]: pcode_now : ");
	for(i = 0; i < 8; i++)
	{
		printk("%02x ",pcode[i]);
	}
	printk("\n");
	
	printk("[ZET]: pcode_new : ");
	for(i = 0; i < 8; i++)
	{
		printk("%02x ", flash_buffer[pcode_addr[i]]);
	}
	printk("\n");
	
	for(i = 0; i < 8; i++)
	{
		if(pcode[i] != flash_buffer[pcode_addr[i]])
		{
			printk("[ZET]: Version different\n");
			return FALSE;
		}
	}
	printk("[ZET]: Version the same\n");			
	return TRUE;
}

#ifdef FEATURE_FW_SKIP_FF
///***********************************************************************
///   [function]:  zet622x_ts_check_skip_page
///   [parameters]: u8 point
///   [return]: skip download is TRUE/FALSE
///************************************************************************
u8 zet622x_ts_check_skip_page(u8 *data)
{	
	int j;
	
	for(j = 0 ; j < FLASH_PAGE_LEN ; j++)
	{
		if(data[j] != 0xFF)	
		{
			return FALSE;
		}
	}
		
	return TRUE;
}
#endif ///< for FEATURE_FW_SKIP_FF

#ifdef FEATURE_FW_CHECK_SUM
///***********************************************************************
///   [function]:  zet622x_ts_check_skip_page
///   [parameters]: u8 point
///   [return]: check sum is TRUE/FALSE
///************************************************************************
u8 zet622x_ts_sram_check_sum(struct i2c_client *client, int page_id, u8 *data)
{		
	u8 get_check_sum	= 0;
	u8 check_sum 		= 0;
	int i;
	int ret;
	u8 tmp_data[16];

	///---------------------------------///
	///  Get check sum
	///---------------------------------///
	for(i = 0 ; i < FLASH_PAGE_LEN ; i++)
	{
		if(i == 0)
		{
			check_sum = data[i];
		}
		else
		{
			check_sum = check_sum ^ data[i];
		}
	}
	
	///---------------------------------///
	/// Read check sum
	///---------------------------------///
	memset(tmp_data, 0, 16);
	ret = zet622x_cmd_read_check_sum(client, page_id, &tmp_data[0]);	
	if(ret <= 0)
	{
		return FALSE;
	}
	get_check_sum = tmp_data[0];
	//printk("[ZET]: page=%3d  ,Check sum : %x ,get check sum : %x\n", page_id, check_sum, get_check_sum);
	if(check_sum == get_check_sum)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif ///< for FEATURE_FW_CHECK_SUM

///**********************************************************************
///   [function]:  zet622x_ts_hover_status_get
///   [parameters]: void
///   [return]: void
///***********************************************************************
u8 zet622x_ts_hover_status_get(void)
{
	return hover_status;
}

EXPORT_SYMBOL_GPL(zet622x_ts_hover_status_get);

///**********************************************************************
///   [function]:  zet622x_ts_set_transfer_type
///   [parameters]: void
///   [return]: void
///**********************************************************************
int zet622x_ts_set_transfer_type(u8 bTransType)
{
	u8 ts_cmd[10] = {0xC1, 0x02, TRAN_TYPE_DYNAMIC, 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00}; 
	int ret = 0;
	ts_cmd[2] = bTransType;
	ret = zet622x_i2c_write_tsdata(this_client, ts_cmd, 10);
  return ret;
}


///**********************************************************************
///   [function]:  zet622x_ts_set_transfer_type
///   [parameters]: void
///   [return]: void
///**********************************************************************
#ifdef FEATURE_INFO_OUT_EANBLE
int zet622x_ts_set_info_type(void)
{
	int ret = 1;
	char info_file_name_out[128];

	 /// ic type
	 switch(ic_model)
        {
        case MODEL_ZET6221:
                tran_data[0] = ZET6221_INFO;
        	break;
        case MODEL_ZET6223:
                tran_data[0] = ZET6223_INFO;
                break;
        case MODEL_ZET6231:
                tran_data[0] = ZET6231_INFO;
                break;
        case MODEL_ZET6251:
                tran_data[0] = ZET6251_INFO;
                break;
        default:
        	   tran_data[0] = UNKNOW_INFO;
                break;
        }
	 
	 /// resolution
	if(xy_exchange == 1)
	{
		tran_data[16] = 0x8;
		tran_data[9] = ((resolution_y >> 8)&0xFF);
		tran_data[8] = (resolution_y &0xFF);
		tran_data[11] = ((resolution_x >> 8)&0xFF);
		tran_data[10] = (resolution_x &0xFF);
	}
	else
	{
		tran_data[16] = 0x00;
		tran_data[9] = ((resolution_x >> 8)&0xFF);
		tran_data[8] = (resolution_x &0xFF);
		tran_data[11] = ((resolution_y >> 8)&0xFF);
		tran_data[10] = (resolution_y &0xFF);
	}
	
	/// trace X
       tran_data[13] = TP_DEFAULT_COL;  ///< trace x
       /// trace Y
       tran_data[14] = TP_DEFAULT_ROW;  ///< trace y
        
	if(key_num > 0)
	{
		tran_data[15] = (0x80 | finger_num);
	}
	else
	{
		tran_data[15] = finger_num;
	}
	
        sprintf(info_file_name_out, "%sinfo.bin",tran_type_mode_file_name);
        zet_information_save(info_file_name_out);

  	return ret;
}
#endif ///< FEATURE_INFO_OUT_EANBLE

///**********************************************************************
///   [function]:  zet62xx_ts_auto_zoom
///   [parameters]: px, py, p
///   [return]: void
///**********************************************************************
void zet62xx_ts_auto_zoom(u32 *px, u32 *py, u32 p)
{
	int i;
	u32 value_x;
	u32 value_y;
	u8 pressed;
	
	for(i = 0 ; i < MAX_FINGER_NUMBER ; i++)
	{
		pressed = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x01;
		if(pressed)
		{
			value_x = (u32)(((px[i]*X_MAX*10)/FW_X_RESOLUTION + 5)/10);
			value_y = (u32)(((py[i]*Y_MAX*10)/FW_Y_RESOLUTION + 5)/10);
			//printk("[ZET] %d: (%d,%d) = (%d,%d)\n", i, px[i], py[i], value_x, value_y );	
			px[i] = value_x;
			py[i] = value_y;
		}
	}
}

///**********************************************************************
///   [function]:  zet622x_ts_coordinate_translating
///   [parameters]: px, py, p
///   [return]: void
///**********************************************************************
void zet622x_ts_coordinate_translating(u32 *px, u32 *py, u8 p)
{
	int i;
	u8 pressed;

#if ORIGIN == ORIGIN_TOP_RIGHT
	for(i = 0; i < MAX_FINGER_NUMBER; i++)
	{
		pressed = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x01;
		if(pressed == TRUE)
		{
			px[i] = X_MAX - px[i];
		}
	}
#elif ORIGIN == ORIGIN_BOTTOM_RIGHT
	for(i = 0; i < MAX_FINGER_NUMBER; i++)
	{
		pressed = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x01;
		if(pressed == TRUE)
		{
			px[i] = X_MAX - px[i];
			py[i] = Y_MAX - py[i];
		}
	}
#elif ORIGIN == ORIGIN_BOTTOM_LEFT
	for(i = 0; i < MAX_FINGER_NUMBER; i++)
	{
		pressed = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x01;
		if(pressed == TRUE)
		{
			py[i] = Y_MAX - py[i];
		}
	}
#endif ///< for ORIGIN
}

///**********************************************************************
///   [function]:  zet622x_ts_parse_mutual_dev
///   [parameters]: client
///   [return]: u8
///**********************************************************************
#ifdef FEATURE_MDEV_OUT_ENABLE
u8 zet622x_ts_parse_mutual_dev(struct i2c_client *client)
{
	int mdev_packet_size = (row+2) * (col + 2);
	int ret = 0;
	int idx = 0;
	int len =  mdev_packet_size;
	char mdev_file_name_out[128];
	
	int step_size = col + 2;
	
	while(len > 0)
	{
		if(len < step_size)
		{
			step_size = len;
		}

		ret = zet622x_i2c_read_tsdata(client, &tran_data[idx], step_size);
		len -= step_size;
		idx += step_size;
	}
	
	sprintf(mdev_file_name_out, "%s%s%02d.bin", tran_type_mode_file_name, MDEV_FILE_NAME, mdev_file_id);	
	zet_mdev_save(mdev_file_name_out);
	mdev_file_id  =  (mdev_file_id +1)% (MDEV_MAX_FILE_ID);
	return ret;
}
#endif ///< FEATURE_MDEV_OUT_ENABLE

///**********************************************************************
///   [function]:  zet622x_ts_parse_initial_base
///   [parameters]: client
///   [return]: u8
///**********************************************************************
#ifdef FEATURE_IBASE_OUT_ENABLE
u8 zet622x_ts_parse_initial_base(struct i2c_client *client)
{
	int ibase_packet_size = (row + col) * 2;
	int ret = 0;
	int idx = 0;
	int len =  ibase_packet_size;
	char ibase_file_name_out[128];
	
	int step_size = ibase_packet_size;
	
	while(len > 0)
	{
		ret = zet622x_i2c_read_tsdata(client, &tran_data[idx], step_size);
		len -= step_size;
	}
	sprintf(ibase_file_name_out, "%s%s%02d.bin", tran_type_mode_file_name, IBASE_FILE_NAME, ibase_file_id);	
	zet_ibase_save(ibase_file_name_out);
	ibase_file_id  =  (ibase_file_id +1)% (IBASE_MAX_FILE_ID);
	return ret;
}
#endif ///< FEATURE_IBASE_OUT_ENABLE

///**********************************************************************
///   [function]:  zet622x_ts_parse_initial_dev
///   [parameters]: client
///   [return]: u8
///**********************************************************************
#ifdef FEATURE_IDEV_OUT_ENABLE
u8 zet622x_ts_parse_initial_dev(struct i2c_client *client)
{
	int idev_packet_size = (row + col);
	int ret = 0;
	int idx = 0;
	int len =  idev_packet_size;
	char idev_file_name_out[128];
	
	int step_size = idev_packet_size;
	
	while(len > 0)
	{
		ret = zet622x_i2c_read_tsdata(client, &tran_data[idx], step_size);
		len -= step_size;
	}
	sprintf(idev_file_name_out, "%s%s%02d.bin", tran_type_mode_file_name, IDEV_FILE_NAME, idev_file_id);	
	zet_idev_save(idev_file_name_out);
	idev_file_id  =  (idev_file_id +1)% (IDEV_MAX_FILE_ID);
	return ret;
}
#endif ///< FEATURE_IDEV_OUT_ENABLE

///**********************************************************************
///   [function]:  zet622x_ts_parse_mutual_base
///   [parameters]: client
///   [return]: u8
///**********************************************************************
#ifdef FEATURE_MBASE_OUT_ENABLE
u8 zet622x_ts_parse_mutual_base(struct i2c_client *client)
{
	int mbase_packet_size = (row * col * 2);
	int ret = 0;
	int idx = 0;
	int len =  mbase_packet_size;
	char mbase_file_name_out[128];
	
	int step_size = col*2;
	
	while(len > 0)
	{
		if(len < step_size)
		{
			step_size = len;
		}

		ret = zet622x_i2c_read_tsdata(client, &tran_data[idx], step_size);
		len -= step_size;
		idx += step_size;
	}
	sprintf(mbase_file_name_out, "%s%s%02d.bin",tran_type_mode_file_name, MBASE_FILE_NAME, mbase_file_id);	
	zet_mbase_save(mbase_file_name_out);
	mbase_file_id  =  (mbase_file_id +1)% (MBASE_MAX_FILE_ID);
	return ret;
}
#endif ///< FEATURE_MBASE_OUT_ENABLE

///**********************************************************************
///   [function]:  zet622x_ts_get_information
///   [parameters]: client
///			    x[out]:  values of X axis;
///			    y[out]:  values of Y axis;
///			    z[out]:  values of Z axis;
///			    pr[out]:  pressed of released status of fingers;
///			    ky[out]:  pressed of released status of ke
///   [return]: u8
///**********************************************************************
u8 zet622x_ts_parse_finger(struct i2c_client *client, u32 *x, u32 *y, u32 *z, u32 *pr, u32 *ky)
{
	u8  ts_data[70];
	int ret;
	int i;
	
	memset(ts_data,0,70);

	ret = zet622x_i2c_read_tsdata(client, ts_data, finger_packet_size);
	
	*pr = ts_data[1];
	*pr = (*pr << 8) | ts_data[2];
		
	for(i = 0; i < finger_num; i++)
	{
		x[i] = (u8)((ts_data[3+4*i])>>4)*256 + (u8)ts_data[(3+4*i)+1];
		y[i] = (u8)((ts_data[3+4*i]) & 0x0f)*256 + (u8)ts_data[(3+4*i)+2];
		z[i] = (u8)((ts_data[(3+4*i)+3]) & 0xff);
	}
		
	//if key enable
	if(key_num > 0)
	{
		*ky = ts_data[3+4*finger_num];
	}

	return ts_data[0];
}

///**********************************************************************
///   [function]:  zet622x_ts_get_information
///   [parameters]: client
///   [return]: u8
///**********************************************************************
u8 zet622x_ts_get_information(struct i2c_client *client)
{
	u8 ts_report_cmd[1] = {0xB2};
	u8 ts_in_data[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int ret;
	int i;
	u8 key_enable = FALSE;
	
	ret = zet622x_i2c_write_tsdata(client, ts_report_cmd, 1);

	if (ret > 0)
	{
		msleep(10);
		printk ("[ZET] : B2 read\n");
		ret = zet622x_i2c_read_tsdata(client, ts_in_data, 17);
			
		if(ret > 0)
		{
			for(i = 0; i < 8; i++)
			{
				pcode[i] = ts_in_data[i] & 0xff;
			}

			xy_exchange = (ts_in_data[16] & 0x8) >> 3;
			if(xy_exchange == 1)
			{
				resolution_y = ts_in_data[9] & 0xff;
				resolution_y = (resolution_y << 8)|(ts_in_data[8] & 0xff);
				resolution_x = ts_in_data[11] & 0xff;
				resolution_x = (resolution_x << 8) | (ts_in_data[10] & 0xff);
			}
			else
			{
				resolution_x = ts_in_data[9] & 0xff;
				resolution_x = (resolution_x << 8)|(ts_in_data[8] & 0xff);
				resolution_y = ts_in_data[11] & 0xff;
				resolution_y = (resolution_y << 8) | (ts_in_data[10] & 0xff);
			}
					
			finger_num = (ts_in_data[15] & 0x7f);
			
			key_enable = (ts_in_data[15] & 0x80);
			if(key_enable == 0)
			{
				finger_packet_size  = 3 + 4*finger_num;
			}
			else
			{
				finger_packet_size  = 3 + 4*finger_num + 1;
			}
		}
		else
		{
			printk ("[ZET] : B2 fail\n");
			return ret;
		}
	}
	else
	{
		return ret;
	}
	return 1;
}

///**********************************************************************
///   [function]:  zet622x_ts_interrupt
///   [parameters]: irq, dev_id
///   [return]: irqreturn_t
///**********************************************************************
static irqreturn_t zet622x_ts_interrupt(int irq, void *dev_id)
{
	struct zet622x_tsdrv *ts_drv = dev_id;
	if(gpio_get_value(ts_drv->irq_gpio) == GPIO_LOW)
	{
		queue_work(ts_drv->ts_workqueue, &ts_drv->work1);
	}
	return IRQ_HANDLED;
}

///************************************************************************
///   [function]:  zet62xx_ts_init
///   [parameters]: 
///   [return]: void
///************************************************************************
#ifdef FEAURE_LIGHT_LOAD_REPORT_MODE
static void zet62xx_ts_init(void)
{
	u8 i;
	/// initital the pre-finger status 
	for(i = 0 ; i < MAX_FINGER_NUMBER; i++)
	{
		pre_event[i].pressed = PRE_PRESSED_DEFAULT_VALUE;
	}
}
#endif ///< for FEAURE_LIGHT_LOAD_REPORT_MODE

///**********************************************************************
///   [function]:  zet622x_ts_work
///   [parameters]: work
///   [return]: void
///**********************************************************************
static void zet622x_ts_work(struct work_struct *_work)
{
	u32 x[MAX_FINGER_NUMBER];
	u32 y[MAX_FINGER_NUMBER];
	u32 z[MAX_FINGER_NUMBER];
	u32 pr;
	u32 ky;
	u32 points;
	u32 px,py,pz;
	u8 ret;
	u8 pressed;
	int i;

	struct zet622x_tsdrv *ts =
		container_of(_work, struct zet622x_tsdrv, work1);

	struct i2c_client *tsclient1 = ts->i2c_dev;

	///-------------------------------------------///
	/// Read no fingers in suspend mode
	///-------------------------------------------///
	if(suspend_mode == TRUE)
	{
		return;
	}

	if(finger_packet_size == 0)
	{
		return;
	}

#ifdef FEATURE_FW_UPGRADE_RESUME
	if(resume_download == TRUE)
	{
		return;
	}
#endif ///< for FEATURE_FW_UPGRADE_RESUME
	///-------------------------------------------///
	/// Dummy report 
	///-------------------------------------------///
	if(dummy_report_cnt == 1)
	{
		dummy_report_cnt = 0;
		return;
	}

	///-------------------------------------------///
	/// Transfer Type : Mutual Dev Mode 
	///-------------------------------------------///
#ifdef FEATURE_MDEV_OUT_ENABLE
	if(transfer_type == TRAN_TYPE_MUTUAL_SCAN_DEV)
	{
		zet622x_ts_parse_mutual_dev(tsclient1);
		return;	
	}
#endif ///< FEATURE_MDEV_OUT_ENABLE

	///-------------------------------------------///
	/// Transfer Type : Initial Base Mode 
	///-------------------------------------------///
#ifdef FEATURE_IBASE_OUT_ENABLE
	if(transfer_type == TRAN_TYPE_INIT_SCAN_BASE)
	{
		zet622x_ts_parse_initial_base(tsclient1);
		return;	
	}	
#endif ///< FEATURE_IBASE_OUT_ENABLE

	///-------------------------------------------///
	/// Transfer Type : Initial Dev Mode 
	///-------------------------------------------///
#ifdef FEATURE_IDEV_OUT_ENABLE
	if(transfer_type == TRAN_TYPE_INIT_SCAN_DEV)
	{
		zet622x_ts_parse_initial_dev(tsclient1);
		return;	
	}
#endif ///< TRAN_TYPE_INIT_SCAN_DEV

	///-------------------------------------------///
	/// Transfer Type : Mutual Base Mode 
	///-------------------------------------------///
#ifdef FEATURE_MBASE_OUT_ENABLE
	if(transfer_type == TRAN_TYPE_MUTUAL_SCAN_BASE)
	{
		zet622x_ts_parse_mutual_base(tsclient1);
		return;	
	}
#endif ///< FEATURE_MBASE_OUT_ENABLE

	///-------------------------------------------///
	/// Transfer Type : Dynamic Mode 
	///-------------------------------------------///
	ret = zet622x_ts_parse_finger(tsclient1, x, y, z, &pr, &ky);
	
	if(ret == 0x3C)
	{
		points = pr;
		hover_status = (z[0]>>7)&0x1;
		
#ifdef FEATURE_AUTO_ZOOM_ENABLE
		zet62xx_ts_auto_zoom(x, y, points);
#endif ///< for FEATURE_AUTO_ZOOM_ENABLE

#ifdef FEATURE_TRANSLATE_ENABLE
		zet622x_ts_coordinate_translating(x, y, points);
#endif ///< for FEATURE_TRANSLATE_ENABLE
		///-------------------------------------------///
		/// all finger up report 
		///-------------------------------------------///
		if(points == 0)
		{
			finger_up_cnt++;
			if(finger_up_cnt >= DEBOUNCE_NUMBER)
			{
				finger_up_cnt = 0;
	
#ifdef FEATURE_BTN_TOUCH
				input_report_key(ts->input, BTN_TOUCH, 0);
#endif ///< for FEATURE_BTN_TOUCH

#ifdef FEATURE_MT_TYPE_B
				for(i = 0; i < finger_num; i++)
				{
	#ifdef FEAURE_LIGHT_LOAD_REPORT_MODE
					if(pre_event[i].pressed == FALSE)  ///< check the pre-finger status is up
					{
						continue;
					}
					pre_event[i].pressed = FALSE;
	#endif  ///< for FEAURE_LIGHT_LOAD_REPORT_MODE
					input_mt_slot(ts->input, i);
					input_mt_report_slot_state(ts->input, MT_TOOL_FINGER, false);
					input_report_abs(ts->input, ABS_MT_TRACKING_ID, -1);
				}
				input_mt_report_pointer_emulation(ts->input, true);
#else ///< for FEATURE_MT_TYPE_B				
				input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(ts->input);
#endif ///< for FEATURE_MT_TYPE_B
			}
			goto LABEL_NO_FINGER;
		}
#ifdef FEATURE_VIRTUAL_KEY
		key_num = 4;
		if(points == 0x8000) // only finger 1 enable. 
		{
			if(y[0] > TP_AA_Y_MAX)
			{
				if((x[0] >= tpd_keys_dim[0][0]) && 
				   (x[0] <= tpd_keys_dim[0][1]) && 
				   (y[0] >= tpd_keys_dim[0][2]) && 
				   (y[0] <= tpd_keys_dim[0][3]))
				{
					ky=0x1;
				}
				else if((x[0] >= tpd_keys_dim[1][0]) && 
					(x[0] <= tpd_keys_dim[1][1]) && 
					(y[0] >= tpd_keys_dim[1][2]) && 
					(y[0] <= tpd_keys_dim[1][3]) ) 
				{
					ky=0x2;
				}
				else if((x[0] >= tpd_keys_dim[2][0]) && 
					(x[0] <= tpd_keys_dim[2][1]) && 
					(y[0] >= tpd_keys_dim[2][2]) && 
					(y[0] <= tpd_keys_dim[2][3]) ) 
				{
					ky=0x4;
				}
				else if((x[0] >= tpd_keys_dim[3][0]) && 
					(x[0] <= tpd_keys_dim[3][1]) && 
					(y[0] >= tpd_keys_dim[3][2]) && 
					(y[0] <= tpd_keys_dim[3][3]) ) 
				{
					ky=0x8;
				}

				goto LABEL_KEY_FINGER;
			}
		}
#endif ///< for FEATURE_VIRTUAL_KEY
		///-------------------------------------------///
		/// finger down report 
		///-------------------------------------------///
		finger_up_cnt = 0;
		for(i = 0; i < finger_num; i++)
		{
			pressed = (points >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
			if(pressed == TRUE)
			{
#ifdef FEATURE_BTN_TOUCH
				input_report_key(ts->input, BTN_TOUCH, 1);
#endif ///< for FEATURE_BTN_TOUCH 
#ifdef FEAURE_LIGHT_LOAD_REPORT_MODE
        			/// check the pre-finger status is pressed and X,Y is same, than skip report to the host
				if((pre_event[i].pressed == TRUE) &&
				(pre_event[i].pre_x == x[i]) &&
				(pre_event[i].pre_y == y[i]))
				{
				  continue;
				}
				/// Send finger down status to host
				pre_event[i].pressed = TRUE;
				pre_event[i].pre_x = x[i];
				pre_event[i].pre_y = y[i];
				pre_event[i].pre_z = z[i];
#endif ///< for FEAURE_LIGHT_LOAD_REPORT_MODE 
				px = x[i];
				py = y[i];
				pz = z[i];
#ifdef FEATURE_VIRTUAL_KEY
				if(py > TP_AA_Y_MAX)
				{ 
					py = TP_AA_Y_MAX;
				}
#endif ///< for FEATURE_VIRTUAL_KEY


//printk("[ZET] : px========%d,py==========%d\n",px,py);

#ifdef FEATURE_MT_TYPE_B
				input_mt_slot(ts->input, i);
				input_mt_report_slot_state(ts->input, MT_TOOL_FINGER,true);
				input_report_abs(ts->input,ABS_MT_PRESSURE,	PRESSURE_CONST);
#endif ///< for FEATURE_MT_TYPE_B

				input_report_abs(ts->input, ABS_MT_TRACKING_ID, i);
				input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, PRESSURE_CONST);
				input_report_abs(ts->input, ABS_MT_POSITION_X, px);
				input_report_abs(ts->input, ABS_MT_POSITION_Y, py);
#ifndef FEATURE_MT_TYPE_B	    		
				input_mt_sync(ts->input);
#endif ///< for FEATURE_MT_TYPE_B
			
			}
			else
			{
#ifdef FEATURE_MT_TYPE_B

	#ifdef FEAURE_LIGHT_LOAD_REPORT_MODE
        			/// If pre-finger status is not pressed, than skip report to the host
				if(pre_event[i].pressed == FALSE)
				{
				  continue;
				}
				pre_event[i].pressed = FALSE;
	#endif ///< for FEAURE_LIGHT_LOAD_REPORT_MODE
	
				/// Send finger up status to host
				input_mt_slot(ts->input, i);
				input_mt_report_slot_state(ts->input, MT_TOOL_FINGER,false);
				input_report_abs(ts->input, ABS_MT_TRACKING_ID, -1);
	
#endif  ///< for FEATURE_MT_TYPE_B
			}
		}
		
#ifdef FEATURE_MT_TYPE_B
		input_mt_report_pointer_emulation(ts->input, true);
#endif ///< for FEATURE_MT_TYPE_B

#ifdef FEATURE_VIRTUAL_KEY
LABEL_KEY_FINGER:
#endif ///< for FEATURE_VIRTUAL_KEY
LABEL_NO_FINGER:
		if(key_num > 0)
		{
			for(i = 0; i < KEY_NUMBER; i++)
			{			
				pressed = ky & ( 0x01 << i );
				switch(i)
				{
					case 0:
						if(pressed == TRUE)
						{
							if(!key_search_pressed)
							{
								input_report_key(ts->input, KEY_SEARCH, 1);
								key_search_pressed = 0x1;
							}
						}
						else
						{
							if(key_search_pressed)
							{
								input_report_key(ts->input, KEY_SEARCH, 0);
								key_search_pressed = 0x0;
							}
						}
						
						break;
					case 1:
						if(pressed == TRUE)
						{
							if(!key_back_pressed)
							{
								input_report_key(ts->input, KEY_BACK, 1);
 								key_back_pressed = 0x1;
							}
						}
						else
						{
							if(key_back_pressed)
							{
  								input_report_key(ts->input, KEY_BACK, 0);
 								key_back_pressed = 0x0;
							}
						}
						
						break;
					case 2:
						if(pressed == TRUE)
						{
							if(!key_home_pressed)
							{
   								input_report_key(ts->input, KEY_HOME, 1);
								key_home_pressed = 0x1;
							}
						}
						else
						{
							if(key_home_pressed)
							{
 								input_report_key(ts->input, KEY_HOME, 0);
								key_home_pressed = 0x0;
							}
						}
						
						break;
					case 3:
						if(pressed == TRUE)
						{
							if(!key_menu_pressed)
							{
								input_report_key(ts->input, KEY_MENU, 1);
								key_menu_pressed = 0x1;
							}
						}
						else
						{
							if(key_menu_pressed)
							{
								input_report_key(ts->input, KEY_MENU, 0);
								key_menu_pressed = 0x0;
							}
						}
						break;
					case 4:
						break;
					case 5:
						break;
					case 6:
						break;
					case 7:
						break;
				}

			}
		}
		input_sync(ts->input);		
	}
}

///**********************************************************************
///   [function]:  zet622x_ts_timer_task
///   [parameters]: arg
///   [return]: void
///***********************************************************************
static void zet622x_ts_timer_task(unsigned long arg)
{
	struct zet622x_tsdrv *ts_drv = (struct zet622x_tsdrv *)arg;
	queue_work(ts_drv->ts_workqueue1, &ts_drv->work2);
	if(ts_drv->get_system_charge_status)
	  mod_timer(&ts_drv->zet622x_ts_timer_task,jiffies + msecs_to_jiffies(polling_time));	
	else
		mod_timer(&ts_drv->zet622x_ts_timer_task,jiffies + msecs_to_jiffies(polling_time*20));//2s detect read sysfs
}

///**********************************************************************
///   [function]:  zet622x_ts_charge_mode_enable
///   [parameters]: void
///   [return]: void
///**********************************************************************
void zet622x_ts_charge_mode_enable(void)
{
	u8 ts_write_charge_cmd[1] = {0xb5}; 
	int ret = 0;
	
#ifdef FEATURE_FW_UPGRADE_RESUME
	if(resume_download == TRUE)
	{
		return;
	}
#endif ///< for FEATURE_FW_UPGRADE_RESUME
	if(suspend_mode == TRUE)
	{
		return;
	}
	printk("[ZET] : enable charger mode\n");
	ret = zet622x_i2c_write_tsdata(this_client, ts_write_charge_cmd, 1);

}
EXPORT_SYMBOL_GPL(zet622x_ts_charge_mode_enable);

///**********************************************************************
///   [function]:  zet622x_ts_charge_mode_disable
///   [parameters]: client
///   [return]: u8
///**********************************************************************
void zet622x_ts_charge_mode_disable(void)
{
	u8 ts_write_cmd[1] = {0xb6}; 
	int ret = 0;

#ifdef FEATURE_FW_UPGRADE_RESUME
	if(resume_download == TRUE)
	{
		return;
	}
#endif ///< for FEATURE_FW_UPGRADE_RESUME
	if(suspend_mode == TRUE)
	{
		return;
	}
	printk("[ZET] : disable charger mode\n");
	ret = zet622x_i2c_write_tsdata(this_client, ts_write_cmd, 1);

}
EXPORT_SYMBOL_GPL(zet622x_ts_charge_mode_disable);

#define BATTERY_STATUS_PATH	"sys/class/power_supply/battery/status"
static int zet62xx_read_charger_status(char *filename)
{	
    struct file *fd;
    int retLen = -1;
    int offset=0;
    mm_segment_t old_fs_;
    char bat_status_buf[20];
    ssize_t len;
    
    len=sizeof(bat_status_buf);
	  memset(bat_status_buf,0,sizeof(bat_status_buf));
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    fd = filp_open(filename, O_RDONLY, 0);
    
    if(IS_ERR(fd)) {
        printk("zet62xx_read_charger_status : failed to open!!\n");
        return -1;
    }
    do{
        if ((fd->f_op == NULL) || (fd->f_op->read == NULL)){
            printk("zet62xx_read_charger_status: file can not be read!!\n");
            break;
    		} 	
        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        		    if(fd->f_op->llseek(fd, offset, 0) != offset) {
						        printk("zet62xx_read_charger_status: failed to seek!!\n");
					          break;
        		    }
        	  } else {
        		    fd->f_pos = offset;
        	  }
        }    		     
    		retLen = fd->f_op->read(fd,bat_status_buf,len,&fd->f_pos);			
    }while(false);
    filp_close(fd, NULL);
    set_fs(old_fs_);
    //
    if(memcmp(bat_status_buf,"Charging",8) == 0 || memcmp(bat_status_buf,"Full",4) == 0){
		   return 1;
	  }
	  else{
		   return 0;
	  }
}


///**********************************************************************
///   [function]:  zet622x_charger_cmd_work
///   [parameters]: work
///   [return]: void
///***********************************************************************
static void zet622x_charger_cmd_work(struct work_struct *_work)
{
	struct zet622x_tsdrv *zet6221_ts = i2c_get_clientdata(this_client);
	int det_charge=0;
	if(suspend_mode == TRUE)
	{
		return;
	}

#ifdef FEATURE_FW_UPGRADE_RESUME
	if(resume_download == TRUE)
	{
		return;
	}
#endif ///< for FEATURE_FW_UPGRADE_RESUME
  
  if(zet6221_ts->get_system_charge_status){
  	det_charge=zet6221_ts->get_system_charge_status();
  }else{
  	det_charge=zet62xx_read_charger_status(BATTERY_STATUS_PATH);
  }
  
	if(det_charge != charger_status){	
		if(det_charge==1){
			zet622x_ts_charge_mode_enable();
			printk("[ZET]:Charger Mode On\n");		
		}
		else{
			zet622x_ts_charge_mode_disable();
			printk("[ZET]:Charger Mode Off\n");		
		}
		charger_status = det_charge;
	}
	
	///-------------------------------------------------------------------///
	/// IOCTL Action
	///-------------------------------------------------------------------///
	if(ioctl_action  & IOCTL_ACTION_FLASH_DUMP){
		printk("[ZET]: IOCTL_ACTION: Dump flash\n");
		zet_fw_save(fw_file_name);
		ioctl_action &= ~IOCTL_ACTION_FLASH_DUMP;
	}
}

#ifdef FEATURE_FW_SIGNATURE
///************************************************************************
///   [function]:  zet622x_ts_sig_check
///   [parameters]: client
///   [return]: void
///************************************************************************
int zet622x_ts_sig_check(struct i2c_client *client)
{
	int i;
	int ret = TRUE;

	///---------------------------------///
        /// if zet6221, then leaves
        ///---------------------------------///
	if(ic_model == MODEL_ZET6221)
	{
		printk("[ZET]: signature check ignored\n");
		return	TRUE;
	}

	///---------------------------------///
        /// Read sig page
        ///---------------------------------///
	ret = zet622x_cmd_readpage(client, SIG_PAGE_ID, &zet_rx_data[0]);
        if(ret <= 0)
        {
		printk("[ZET]: signature check fail\n");
        	return FALSE;
        }	

	///---------------------------------///
        /// Clear the signature position
        ///---------------------------------///
        for(i = 0 ; i < SIG_DATA_LEN ; i++)
	{
		/// erase the sig page last 4 bytes data
		flash_buffer[SIG_PAGE_ID * FLASH_PAGE_LEN + SIG_DATA_ADDR + i] = 0xFF;
	}

	///---------------------------------///
        /// check signature
        ///---------------------------------///
        printk("[ZET]: sig_curr[] =  ");
        for(i = 0 ; i < SIG_DATA_LEN ; i++)
	{
		printk("%02X ", zet_rx_data[i + SIG_DATA_ADDR]);
        }
	printk("\n");

	printk("[ZET]: sig_data[] =  ");
        for(i = 0 ; i < SIG_DATA_LEN ; i++)
	{
		printk("%02X ", sig_data[i]);
        }
	printk("\n");      

    	printk("[ZET]: sig_data[] =  ");
	for(i = 0 ; i < SIG_DATA_LEN ; i++)
	{
		if(zet_rx_data[i + SIG_DATA_ADDR] != sig_data[i])
		{
			printk("[ZET]: signature check : not signatured!!\n");
			return FALSE;
		}
	}
	printk("[ZET]: signature check : signatured\n");
	return  TRUE;

}

///************************************************************************
///   [function]:  zet622x_ts_sig_write
///   [parameters]: client
///   [return]: void
///************************************************************************
int zet622x_ts_sig_write(struct i2c_client *client)
{
	int i;
	int ret;

	///---------------------------------///
        /// if zet6221, then leaves
        ///---------------------------------///
	if(ic_model == MODEL_ZET6221)
	{
		printk("[ZET]: signature write ignore\n");
		return	TRUE;
	}

	///---------------------------------///
        /// set signature
        ///---------------------------------///
	for(i = 0; i < FLASH_PAGE_LEN; i++)
	{
		zet_tx_data[i] = flash_buffer[SIG_PAGE_ID * FLASH_PAGE_LEN + i];
	}

	printk("[ZET] : old data\n");
        for(i = 0; i < FLASH_PAGE_LEN; i++)
        {
                printk("%02x ", zet_tx_data[i]);
                if((i%0x10) == 0x0F)
                {
                        printk("\n");
                }
                else if((i%0x08) == 0x07)
                {
                        printk(" - ");
                }
        }


	///---------------------------------///
        /// set signature
        ///---------------------------------///
        for(i = 0 ; i < SIG_DATA_LEN ; i++)
        {
                zet_tx_data[ i + SIG_DATA_ADDR] = sig_data[i];
        }

	printk("[ZET] : new data\n");
	for(i = 0; i < FLASH_PAGE_LEN; i++)
	{
		printk("%02x ", zet_tx_data[i]);
		if((i%0x10) == 0x0F)
		{
			printk("\n");
		}
		else if((i%0x08) == 0x07)
		{
			printk(" - ");
		}		
	}

	///---------------------------------///
        /// write sig page
        ///---------------------------------///
	ret = zet622x_cmd_writepage(client, SIG_PAGE_ID, &zet_tx_data[0]);
        if(ret <= 0)
        {
		printk("[ZET]: signature write fail\n");
        	return FALSE;
	}
	msleep(2);
	ret = zet622x_ts_sig_check(client);
	if(ret <= 0)
        {
		printk("[ZET]: signature write fail\n");
        	return FALSE;
	}
	printk("[ZET]: signature write ok\n");	
	return TRUE;
}
#endif ///< for FEATURE_FW_SIGNATURE

///************************************************************************
///   [function]:  zet622x_downloader
///   [parameters]: client, upgrade, romtype, icmodel
///   [return]: int
///************************************************************************
int __init zet622x_downloader( struct i2c_client *client, u8 upgrade, u8 *pRomType, u8 icmodel)
{
	int ret;
	int i;
	
	int flash_total_len 	= 0;
	int flash_rest_len 	= 0;	
	int flash_page_id	= 0;
		
	int now_flash_rest_len	= 0;
	int now_flash_page_id	= 0;

	int retry_count	= 0;
	
	
	u8 uRomType=*pRomType;
#ifdef FEATURE_FW_SKIP_FF
	u8 bSkipWrite = TRUE;
#endif ///< for FEATURE_FW_SKIP_FF

	download_ok = TRUE;

	///----------------------------------------///
	/// 1. set_reset pin low
	///----------------------------------------///
	ctp_set_reset_low();

	msleep(1);
	///----------------------------------------///
	/// 2. send password
	///----------------------------------------///
	ret = zet622x_cmd_sndpwd(client);	
	if(ret <= 0)
	{
		return ret;
	}
	msleep(10);
	
	///----------------------------------------///
	/// Read Code Option
	///----------------------------------------///
	ret = zet622x_cmd_codeoption(client, &uRomType);
	if(ret <= 0)
	{
		return ret;
	}
 	*pRomType = uRomType;	
	msleep(10);
		
	if(upgrade == 0)
	{
		printk("[ZET]: HW_CHECK_ONLY enable! It is zeitec product and not going to upgrade FW. \n");
		return 1;
	}

	/// unlock the write protect of 0xFC00~0xFFFF
	if(ic_model == MODEL_ZET6223)
	{
		ret = zet622x_cmd_sndpwd_1k(client);	
		if(ret <= 0)
		{
			return ret;
		}	
	}
	
	///------------------------------------------------///
	/// init the file
	///------------------------------------------------///
	zet_fw_init();

	///------------------------------------------------///
	/// the SRAM need download code
	///------------------------------------------------///
	if(ic_model == MODEL_ZET6251)
	{
		goto LABEL_START_DOWNLOAD;
	}
	
	///----------------------------------------///
	/// Clear Read-in buffer
	///----------------------------------------///
	memset(zet_rx_data, 0x00, 131);

	///----------------------------------------///
	/// Read Data page for flash version check#1
	///----------------------------------------///
	ret = zet622x_cmd_readpage(client, (pcode_addr[0]>>7), &zet_rx_data[0]);		
	if(ret <= 0)
	{
		return ret;
	}
	printk("[ZET]: page=%3d ",(pcode_addr[0] >> 7)); ///< (pcode_addr[0]/128));
	for(i = 0; i < 4; i++)
	{
		pcode[i] = zet_rx_data[(pcode_addr[i] & 0x7f)]; ///< [(pcode_addr[i]%128)];
		printk("offset[%04x] = %02x ",i,(pcode_addr[i] & 0x7f));    ///< (pcode_addr[i]%128));
	}
	printk("\n");

	///----------------------------------------///
	/// Read Data page for flash version check#2
  	///----------------------------------------///
	ret = zet622x_cmd_readpage(client, (pcode_addr[4]>>7), &zet_rx_data[0]);		
	if(ret <= 0)
	{
		return ret;
	}	

	printk("[ZET]: page=%3d ",(pcode_addr[4] >> 7)); //(pcode_addr[4]/128));
	for(i = 4; i < 8; i++)
	{
		pcode[i] = zet_rx_data[(pcode_addr[i] & 0x7f)]; //[(pcode_addr[i]%128)];
		printk("offset[%04x] = %02x ",i,(pcode_addr[i] & 0x7f));  //(pcode_addr[i]%128));
	}
	printk("\n");
	
	///================================///
	///        Check version         
	///================================///

	#ifdef FEATURE_FW_SIGNATURE
	///----------------------------------------///
        /// Check the data flash version
        ///----------------------------------------///
        if(zet622x_ts_sig_check(client) == TRUE)
        {
		///----------------------------------------///
		/// Check the data flash version
		///----------------------------------------///
		if(zet622x_ts_check_version() == TRUE)
		{
			goto LABEL_EXIT_DOWNLOAD;
		}
        }
	#else ///< for FEATURE_FW_SIGNATURE
	///----------------------------------------///
	/// Check the data flash version
	///----------------------------------------///
	if(zet622x_ts_check_version() != 0)
	{
		goto LABEL_EXIT_DOWNLOAD;
	}
	#endif  ///< for FEATURE_FW_SIGNATURE

	///================================///
	///        Start to download
	///================================///
LABEL_START_DOWNLOAD:
	///----------------------------------------///
	/// Read SFR
	///----------------------------------------///
	ret = zet622x_cmd_sfr_read(client);	
	if(ret <= 0)
	{
		return ret;
	}
	///----------------------------------------///
	/// Update the SFR[14] = 0x3D
	///----------------------------------------///  
	if(zet622x_cmd_sfr_unlock(client) == 0)
	{
		return 0;
	}
	msleep(20);
	

	///------------------------------///
	/// mass erase
	///------------------------------///		
	if(uRomType == ROM_TYPE_FLASH)
	{
		zet622x_cmd_masserase(client);
		msleep(30);
	}

	flash_total_len = zet_fw_size();

	flash_rest_len = flash_total_len;
	
	while(flash_rest_len > 0)
	{
		memset(zet_tx_data, 0x00, 131);

#ifdef FEATURE_FW_COMPARE
LABEL_DOWNLOAD_PAGE:
#endif ///< for FEATURE_FW_COMPARE

		/// Do page erase
		if(retry_count > 0)
  		{
 			///------------------------------///
    			/// Do page erase
    			///------------------------------///    
    			if(uRomType == ROM_TYPE_FLASH)
    			{
      				zet622x_cmd_pageerase(client, flash_page_id);
      				msleep(30);
    			}

 		}

		//printk( " [ZET] : write page%d\n", flash_page_id);
		now_flash_rest_len = flash_rest_len;
		now_flash_page_id  = flash_page_id;
#ifdef FEATURE_FW_SKIP_FF

		bSkipWrite = zet622x_ts_check_skip_page(&flash_buffer[flash_page_id * FLASH_PAGE_LEN]);

		if(bSkipWrite == TRUE)
		{
			//printk( " [ZET] : skip write page%d\n", flash_page_id);
			retry_count = 0;
	       	 	flash_page_id += 1;	
			flash_rest_len -= FLASH_PAGE_LEN;
			continue;
		}	
#endif ///< for FEATURE_SKIP_FF	
		
		///---------------------------------///
		/// Write page
		///---------------------------------///		
		ret = zet622x_cmd_writepage(client, flash_page_id, &flash_buffer[flash_page_id * FLASH_PAGE_LEN]);
		flash_rest_len -= FLASH_PAGE_LEN;

		if(ic_model != MODEL_ZET6251)
		{
			msleep(5);
		}
		
#ifdef FEATURE_FW_COMPARE

		///---------------------------------///
		/// Read page
		///---------------------------------///
		ret = zet622x_cmd_readpage(client, flash_page_id, &zet_rx_data[0]);		
		if(ret <= 0)
		{
			return ret;
		}
		
		for(i = 0; i < FLASH_PAGE_LEN; i++)
		{
			if(i < now_flash_rest_len)
			{
				if(flash_buffer[flash_page_id * FLASH_PAGE_LEN + i] != zet_rx_data[i])
				{
					flash_rest_len = now_flash_rest_len;
					flash_page_id = now_flash_page_id;
				
					if(retry_count < 5)
					{
						retry_count++;
						goto LABEL_DOWNLOAD_PAGE;
					}
					else
					{
						download_ok = FALSE;
						retry_count = 0;						
						ctp_set_reset_high();
						msleep(20);		
						ctp_set_reset_low();
						msleep(20);
						ctp_set_reset_high();
						msleep(20);
						goto LABEL_EXIT_DOWNLOAD;
					}

				}
			}
		}
		
#endif ///< for FEATURE_FW_COMPARE
		retry_count = 0;
		flash_page_id += 1;
	}

	///---------------------------------///
        /// write signature
        ///---------------------------------///

#ifdef FEATURE_FW_SIGNATURE
  if(download_ok == TRUE)
  {	
	if(uRomType == ROM_TYPE_FLASH)
	{
		if(zet622x_ts_sig_write(client) == FALSE)
		{
			download_ok = FALSE;
		}
	}
  }
#endif ///< for FEATURE_FW_SIGNATURE
LABEL_EXIT_DOWNLOAD:
	if(download_ok == FALSE)
	{
		printk("[ZET] : download failed!\n");
	}

	zet622x_cmd_resetmcu(client);
	msleep(10);

	ctp_set_reset_high();
	msleep(20);
	
	return 1;
}

#ifdef FEATURE_FW_UPGRADE_RESUME
///************************************************************************
///   [function]:  zet622x_resume_downloader
///   [parameters]: client, upgrade, romtype, icmodel
///   [return]: int
///************************************************************************
static int zet622x_resume_downloader(struct i2c_client *client, u8 upgrade, u8 *romtype, u8 icmodel)
{
	int ret = 0;
#ifdef FEATURE_FW_SKIP_FF
	u8 bSkipWrite;
#endif ///< for FEATURE_FW_SKIP_FF

#ifdef FEATURE_FW_CHECK_SUM
	int retry_count		= 0;
	u8 check_sum		= 0;
#endif ///< for FEATURE_FW_CHECK_SUM

	int flash_total_len 	= FLASH_SIZE_ZET6231;
	int flash_rest_len 	= 0;
	int flash_page_id 	= 0;
		
	
	///-------------------------------------------------------------///
	/// Set RST=LOW
	///-------------------------------------------------------------///
	ctp_set_reset_low();
	printk("[ZET] : RST = LOW\n");

	///-------------------------------------------------------------///
	/// 2.Send password
	///-------------------------------------------------------------///
	ret = zet622x_cmd_sndpwd(client);
	if(ret<=0)
	{
		return ret;
	}

	switch(ic_model)
	{
		case MODEL_ZET6221:
			flash_total_len = FLASH_SIZE_ZET6221;
			break;
		case MODEL_ZET6223: 
			flash_total_len = FLASH_SIZE_ZET6223;
			break;
		case MODEL_ZET6231: 			
		case MODEL_ZET6251: 
		default: 
			flash_total_len = FLASH_SIZE_ZET6231;
			break;
	}
	
	/// unlock the write protect of 0xFC00~0xFFFF
	if(ic_model == MODEL_ZET6223)
	{
		ret = zet622x_cmd_sndpwd_1k(client);	
		if(ret <= 0)
		{
			return ret;
		}
	}
	
	if(rom_type == ROM_TYPE_FLASH)
	{
  	///----------------------------------------///
  	/// Read SFR
  	///----------------------------------------///
  	ret = zet622x_cmd_sfr_read(client);	
  	if(ret <= 0)
  	{
  		return ret;
  	}
  	///----------------------------------------///
  	/// Update the SFR[14] = 0x3D
  	///----------------------------------------///  
  	if(zet622x_cmd_sfr_unlock(client) == 0)
  	{
  		return 0;
  	}
  	msleep(20);
	
  	///------------------------------///
  	/// mass erase
  	///------------------------------///		
		zet622x_cmd_masserase(client);
		msleep(30);
	}

	flash_rest_len = flash_total_len;

	///-------------------------------------------------------------///
	/// Read Firmware from BIN if any
	///-------------------------------------------------------------///
	zet_fw_load(fw_file_name);
#ifdef FEATURE_FW_CHECK_SUM
	///-------------------------------------------------------------///
	/// add the sram check sum to compare the data
	///-------------------------------------------------------------///
	while(flash_rest_len>0)
	{
		check_sum = zet622x_ts_sram_check_sum(client, flash_page_id, &flash_buffer[flash_page_id * FLASH_PAGE_LEN]);
		if(check_sum == FALSE)
		{
			printk("[ZET] :  check the check sum have differ\n");
			goto LABEL_START_RESUME_DOWNLOAD;
		}
		flash_rest_len -= FLASH_PAGE_LEN;
		flash_page_id++;
	}
	goto LABEL_RESUME_DOWNLOAD_FINISH;

LABEL_START_RESUME_DOWNLOAD:
	//printk("[ZET] :  LABEL_START_RESUME_DOWNLOAD\n");
	flash_rest_len = flash_total_len;
	flash_page_id = 0;
#endif  ///< for FEATURE_FW_CHECK_SUM

	while(flash_rest_len>0)
	{

#ifdef FEATURE_FW_SKIP_FF
		bSkipWrite = zet622x_ts_check_skip_page(&flash_buffer[flash_page_id * FLASH_PAGE_LEN]);
		if(bSkipWrite == TRUE)
		{
		        //printk( " [ZET] : skip write page%d\n", flash_page_id);
		        flash_rest_len-=FLASH_PAGE_LEN;
		        flash_page_id += 1;
		        continue;
		}
#endif ///< for FEATURE_SKIP_FF 
		//---------------------------------///
		/// 5. Write page
		///--------------------------------///		
		
#ifdef FEATURE_FW_CHECK_SUM
LABEL_RETRY_DOWNLOAD_PAGE:
#endif  ///< for FEATURE_FW_CHECK_SUM
		ret = zet622x_cmd_writepage(client, flash_page_id, &flash_buffer[flash_page_id * FLASH_PAGE_LEN]);
		flash_rest_len -= FLASH_PAGE_LEN;
#ifdef FEATURE_FW_CHECK_SUM

		check_sum = zet622x_ts_sram_check_sum(client, flash_page_id, &flash_buffer[flash_page_id * FLASH_PAGE_LEN]);
		
		if(check_sum == FALSE)
		{		
			if(retry_count < 5)
			{
				retry_count++;
				flash_rest_len += FLASH_PAGE_LEN;
				/// zet6251 add reset function
				ctp_set_reset_high();
				msleep(1);		
				ctp_set_reset_low();
				msleep(1);
				zet622x_cmd_sndpwd(client);		
				goto LABEL_RETRY_DOWNLOAD_PAGE;
			}
			else
			{
				retry_count = 0;						
				ctp_set_reset_high();
				msleep(20);		
				ctp_set_reset_low();
				msleep(20);
				ctp_set_reset_high();
				msleep(20);
				printk("[ZET] zet622x_resume_downloader fail\n");
				return ret;
			}
			
		}
		retry_count  = 0;	
#endif  ///< for FEATURE_FW_CHECK_SUM
		flash_page_id++;
				 	
	}

#ifdef FEATURE_FW_CHECK_SUM
LABEL_RESUME_DOWNLOAD_FINISH:
#endif ///< for FEATURE_FW_CHECK_SUM

	printk("[ZET] RST = HIGH\n");

	///-------------------------------------------------------------///
	/// reset_mcu command
	///-------------------------------------------------------------///
    printk("[ZET] zet622x_cmd_resetmcu\n");
	zet622x_cmd_resetmcu(client);
	msleep(10);	

	///-------------------------------------------------------------///
	///   SET RST=HIGH
	///-------------------------------------------------------------///
	ctp_set_reset_high();
	msleep(20);

	///-------------------------------------------------------------///
	/// RST toggle 	
	///-------------------------------------------------------------///
	ctp_set_reset_low();
	msleep(2);
	ctp_set_reset_high();
	msleep(2);

	printk("[ZET]: zet622x_resume_downloader finish\n");
	return ret;
}
#endif ///< for FEATURE_FW_UPGRADE


#ifdef FEATURE_FW_UPGRADE_RESUME
///************************************************************************
///   [function]:  zet622x_resume_download_thread
///   [parameters]: arg
///   [return]: int
///************************************************************************
static int zet622x_resume_download_thread(void *arg)
{
	int ret = 0;

	printk("[ZET] : Thread Enter\n");
	resume_download = TRUE;
	if((rom_type == ROM_TYPE_SRAM) || 
	   (rom_type == ROM_TYPE_OTP)) //SRAM,OTP
  	{
	    	if(ic_model == MODEL_ZET6251)
  		{
			zet622x_resume_downloader(this_client, firmware_upgrade, &rom_type, ic_model);
			printk("zet622x download OK\n");
  		}
	}
	printk("[ZET] : Thread Leave\n");
	resume_download = FALSE;
	return ret;
}
#endif ///< for FEATURE_FW_UPGRADE


///************************************************************************
///   [function]:  zet622x_ts_late_resume
///   [parameters]:
///   [return]:
///************************************************************************
static void zet622x_ts_late_resume(struct early_suspend *handler)
{	
	printk("[ZET] : Resume START\n");

	dummy_report_cnt = SKIP_DUMMY_REPORT_COUNT;
	charger_status = 0;
	//ctp_ops.ts_wakeup();
	ctp_wakeup();

#ifdef FEATURE_FW_UPGRADE_RESUME
	if(rom_type != ROM_TYPE_SRAM)
	{
		goto LABEL_RESUME_END;
	}
 	
	resume_download_task = kthread_create(zet622x_resume_download_thread, NULL, "resume_download");
	if(IS_ERR(resume_download_task))
	{
		printk(KERN_ERR "%s: cread thread failed\n",__FILE__);	
	}
	wake_up_process(resume_download_task); 

LABEL_RESUME_END:	
#endif ///< for TURE_FW_UPGRADE

	///------------------------------------------------///
	/// init the finger pressed data
	///------------------------------------------------///
#ifdef FEAURE_LIGHT_LOAD_REPORT_MODE
	zet62xx_ts_init();
#endif ///< for FEAURE_LIGHT_LOAD_REPORT_MODE
 
	printk("[ZET] : Resume END\n");
	
	/// leave suspend mode
	suspend_mode = FALSE;

  ///--------------------------------------///
  /// Set transfer type to dynamic mode
  ///--------------------------------------///
	transfer_type = TRAN_TYPE_DYNAMIC;

	return;
}

///************************************************************************
///   [function]:  zet622x_ts_early_suspend
///   [parameters]: early_suspend
///   [return]: void
///************************************************************************
static void zet622x_ts_early_suspend(struct early_suspend *handler)
{
	u8 ts_sleep_cmd[1] = {0xb1}; 
	int ret = 0;
	suspend_mode = TRUE;
	ret = zet622x_i2c_write_tsdata(this_client, ts_sleep_cmd, 1);
	return;	        
}

///************************************************************************
///	zet622x_i2c_driver
///************************************************************************
static struct i2c_driver zet622x_i2c_driver = 
{
	.class = I2C_CLASS_HWMON, 
	.driver = 
	{
		.owner	= THIS_MODULE,
		.name	= ZET_TS_ID_NAME,
	},
	.probe	  	= zet622x_ts_probe,
	.remove		= __devexit_p(zet622x_ts_remove),
	.id_table	= zet622x_ts_idtable,
	.address_list	= u_i2c_addr.normal_i2c,
};

///***********************************************************************
///   [function]:  zet_mdev_save
///   [parameters]: char *
///   [return]: void
///************************************************************************
static void zet_mdev_save(char *file_name)
{
        struct file *fp;
        int data_total_len  = (row+2) * (col + 2);

        ///-------------------------------------------------------///        
        /// create the file that stores the mutual dev data
        ///-------------------------------------------------------///        
        fp = filp_open(file_name, O_RDWR | O_CREAT, 0644);
        if(IS_ERR(fp))
        {
                printk("[ZET] : Failed to open %s\n", file_name);
                return;
        }
        old_fs = get_fs();
        set_fs(KERNEL_DS);

        vfs_write(fp, tran_data, data_total_len, &(fp->f_pos));
	memcpy(mdev_data, tran_data, data_total_len);
        set_fs(old_fs);
        filp_close(fp, 0);

        return;
}

///***********************************************************************
///   [function]:  zet_idev_save
///   [parameters]: char *
///   [return]: void
///************************************************************************
#ifdef FEATURE_IDEV_OUT_ENABLE
static void zet_idev_save(char *file_name)
{
        struct file *fp;
        int data_total_len  = (row + col);

        ///-------------------------------------------------------///        
        /// create the file that stores the initial dev data
        ///-------------------------------------------------------///        
        fp = filp_open(file_name, O_RDWR | O_CREAT, 0644);
        if(IS_ERR(fp))
        {
                printk("[ZET] : Failed to open %s\n", file_name);
                return;
        }
        old_fs = get_fs();
        set_fs(KERNEL_DS);

        vfs_write(fp, tran_data, data_total_len, &(fp->f_pos));
	memcpy(idev_data, tran_data, data_total_len);
        set_fs(old_fs);
        filp_close(fp, 0);

        return;
}
#endif ///< FEATURE_IDEV_OUT_ENABLE

///***********************************************************************
///   [function]:  zet_ibase_save
///   [parameters]: char *
///   [return]: void
///************************************************************************
#ifdef FEATURE_IBASE_OUT_ENABLE
static void zet_ibase_save(char *file_name)
{
        struct file *fp;
        int data_total_len  = (row + col) * 2;

        ///-------------------------------------------------------///        
        /// create the file that stores the initial base data
        ///-------------------------------------------------------///        
        fp = filp_open(file_name, O_RDWR | O_CREAT, 0644);
        if(IS_ERR(fp))
        {
                printk("[ZET] : Failed to open %s\n", file_name);
                return;
        }
        old_fs = get_fs();
        set_fs(KERNEL_DS);

        vfs_write(fp, tran_data, data_total_len, &(fp->f_pos));
	memcpy(ibase_data, tran_data, data_total_len);
        set_fs(old_fs);
        filp_close(fp, 0);

        return;
}
#endif ///< FEATURE_IBASE_OUT_ENABLE

///***********************************************************************
///   [function]:  zet_mbase_save
///   [parameters]: char *
///   [return]: void
///************************************************************************
#ifdef FEATURE_MBASE_OUT_ENABLE
static void zet_mbase_save(char *file_name)
{
        struct file *fp;
        int data_total_len  = (row * col * 2);

        ///-------------------------------------------------------///        
        /// create the file that stores the mutual base data
        ///-------------------------------------------------------///        
        fp = filp_open(file_name, O_RDWR | O_CREAT, 0644);
        if(IS_ERR(fp))
        {
                printk("[ZET] : Failed to open %s\n", file_name);
                return;
        }
        old_fs = get_fs();
        set_fs(KERNEL_DS);

        vfs_write(fp, tran_data, data_total_len, &(fp->f_pos));
	memcpy(mbase_data, tran_data, data_total_len);
        set_fs(old_fs);
        filp_close(fp, 0);

        return;
}
#endif ///< FEATURE_MBASE_OUT_ENABLE

///***********************************************************************
///   [function]:  zet_information_save
///   [parameters]: char *
///   [return]: void
///************************************************************************
#ifdef FEATURE_INFO_OUT_EANBLE
static void zet_information_save(char *file_name)
{
        struct file *fp;
        int data_total_len  = INFO_DATA_SIZE;

        ///-------------------------------------------------------///        
        /// create the file that stores the mutual base data
        ///-------------------------------------------------------///        
        fp = filp_open(file_name, O_RDWR | O_CREAT, 0644);
        if(IS_ERR(fp))
        {
                printk("[ZET] : Failed to open %s\n", file_name);
                return;
        }
        old_fs = get_fs();
        set_fs(KERNEL_DS);

        vfs_write(fp, tran_data, data_total_len, &(fp->f_pos));
	 memcpy(info_data, tran_data, data_total_len);
        set_fs(old_fs);
        filp_close(fp, 0);

        return;
}
#endif ///< FEATURE_INFO_OUT_EANBLE

///************************************************************************
///   [function]:  zet_dv_set_file_name
///   [parameters]: void
///   [return]: void
///************************************************************************
static void zet_dv_set_file_name(char *file_name)
{
	strcpy(driver_version, file_name);
}

///************************************************************************
///   [function]:  zet_fw_set_file_name
///   [parameters]: void
///   [return]: void
///************************************************************************
static void zet_fw_set_file_name(char *file_name)
{
	strcpy(fw_file_name, file_name);
}

///************************************************************************
///   [function]:  zet_mdev_set_file_name
///   [parameters]: void
///   [return]: void
///************************************************************************
static void zet_tran_type_set_file_name(char *file_name)
{
	strcpy(tran_type_mode_file_name, file_name);
}


///***********************************************************************
///   [function]:  zet_fw_size
///   [parameters]: void
///   [return]: void
///************************************************************************
static int zet_fw_size(void)
{
	int flash_total_len 	= 0x8000;
	
	switch(ic_model)
	{
		case MODEL_ZET6221:
			flash_total_len = 0x4000;
			break;
		case MODEL_ZET6223: 
			flash_total_len = 0x10000;
			break;
		case MODEL_ZET6231: 			
		case MODEL_ZET6251: 
		default: 
			flash_total_len = 0x8000;
			break;
	}
	
	return flash_total_len;
}


///***********************************************************************
///   [function]:  zet_fw_save
///   [parameters]: file name
///   [return]: void
///************************************************************************
static void zet_fw_save(char *file_name)
{
	struct file *fp;
	int flash_total_len 	= 0;
	
	fp = filp_open(file_name, O_RDWR | O_CREAT, 0644);
	if(IS_ERR(fp))
	{
		printk("[ZET] : Failed to open %s\n", file_name);
		return;
	}
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	flash_total_len = zet_fw_size();
	printk("[ZET] : flash_total_len = 0x%04x\n",flash_total_len );

	vfs_write(fp, flash_buffer, flash_total_len, &(fp->f_pos));
	
	set_fs(old_fs);

	filp_close(fp, 0);	

	
	return;
}

///***********************************************************************
///   [function]:  zet_fw_load
///   [parameters]: file name
///   [return]: void
///************************************************************************
static void zet_fw_load(char *file_name)
{	
#ifdef FEATURE_FW_UPGRADE
	int file_length = 0;
	struct file *fp;
	loff_t *pos;
	
	printk("[ZET]: find %s\n", file_name);
	fp = filp_open(file_name, O_RDONLY, 0644);
	if(IS_ERR(fp))
	{			
		printk("[ZET]: No firmware file detected\n");
		return;
	}

	///----------------------------///
	/// Load from file
	///----------------------------///		
	printk("[ZET]: Load from %s\n", file_name);	

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/// Get file size
	inode = fp->f_dentry->d_inode;
	file_length = (int)inode->i_size;

	pos = &(fp->f_pos); 

	vfs_read(fp, &flash_buffer[0], file_length, pos);

	//file_length
	set_fs(old_fs);
	filp_close(fp, 0);

#endif ///< for FEATURE_FW_UPGRADE		

}

///************************************************************************
///   [function]:  zet_fw_init
///   [parameters]: void
///   [return]: void
///************************************************************************
static void zet_fw_init(void)
{
	int i;
	
	if(flash_buffer == NULL)
	{
  		flash_buffer = kmalloc(MAX_FLASH_BUF_SIZE, GFP_KERNEL);	
	}

        ///---------------------------------------------///
        /// Init the mutual dev buffer
        ///---------------------------------------------///
	if(mdev_data== NULL)
	{
		mdev_data   = kmalloc(MDEV_MAX_DATA_SIZE, GFP_KERNEL);
	}
	if(idev_data== NULL)
	{
		idev_data   = kmalloc(IDEV_MAX_DATA_SIZE, GFP_KERNEL);
	}

	if(mbase_data== NULL)
	{
		mbase_data  = kmalloc(MBASE_MAX_DATA_SIZE, GFP_KERNEL);
	}
	if(ibase_data== NULL)
	{
		ibase_data  = kmalloc(IBASE_MAX_DATA_SIZE, GFP_KERNEL);
	}	
	
        if(tran_data == NULL)
        {
	        tran_data  = kmalloc(MBASE_MAX_DATA_SIZE, GFP_KERNEL);
        }
	
        if(info_data == NULL)
        {
	        info_data  = kmalloc(INFO_MAX_DATA_SIZE, GFP_KERNEL);
        }
        
	printk("[ZET]: Load from header\n");

	if(ic_model == MODEL_ZET6221)
	{
		for(i = 0 ; i < sizeof(zeitec_zet6221_firmware) ; i++)
		{
			flash_buffer[i] = zeitec_zet6221_firmware[i];
		}
	}
	else if(ic_model == MODEL_ZET6223)
	{
		for(i = 0 ; i < sizeof(zeitec_zet6223_firmware) ; i++)
		{
			flash_buffer[i] = zeitec_zet6223_firmware[i];
		}
	}
	else if(ic_model == MODEL_ZET6231)
	{
		for(i = 0 ; i < sizeof(zeitec_zet6231_firmware) ; i++)
		{
			flash_buffer[i] = zeitec_zet6231_firmware[i];
		}
	}
	else if(ic_model == MODEL_ZET6251)
	{
		for(i = 0 ; i < sizeof(zeitec_zet6251_firmware) ; i++)
		{
			flash_buffer[i] = zeitec_zet6251_firmware[i];
		}
	}
	
	/// Load firmware from bin file
	zet_fw_load(fw_file_name);
}

///************************************************************************
///   [function]:  zet_fw_exit
///   [parameters]: void
///   [return]: void
///************************************************************************
static void zet_fw_exit(void)
{
        ///---------------------------------------------///
	/// free mdev_data
        ///---------------------------------------------///
	if(mdev_data!=NULL)
	{
		kfree(mdev_data);
		mdev_data = NULL;
	}

	if(idev_data!=NULL)
	{			
		kfree(idev_data);
		idev_data = NULL;
	}

	if(mbase_data!=NULL)
	{	
		kfree(mbase_data);
		mbase_data = NULL;
	}

	if(ibase_data!=NULL)
	{	
		kfree(ibase_data);
		ibase_data = NULL;
	}
		
	if(tran_data != NULL)	
	{
		kfree(tran_data);
		tran_data = NULL;
	}

	if(info_data != NULL)	
	{
		kfree(info_data);
		info_data = NULL;
	}

	
        ///---------------------------------------------///
	/// free flash buffer
        ///---------------------------------------------///
	if(flash_buffer!=NULL)
	{
	kfree(flash_buffer);
	flash_buffer = NULL;
}

}

///************************************************************************
///   [function]:  zet_fops_open
///   [parameters]: file
///   [return]: int
///************************************************************************
static int zet_fops_open(struct inode *inode, struct file *file)
{
	int subminor;
	int ret = 0;	
	struct i2c_client *client;
	struct i2c_adapter *adapter;	
	struct i2c_dev *i2c_dev;	
	
	subminor = iminor(inode);
	printk("[ZET] : ZET_FOPS_OPEN ,  subminor=%d\n",subminor);
	
	i2c_dev = zet622x_i2c_dev_get_by_minor(subminor);	
	if (!i2c_dev)
	{	
		printk("error i2c_dev\n");		
		return -ENODEV;	
	}
	
	adapter = i2c_get_adapter(i2c_dev->adap->nr);	
	if(!adapter)
	{		
		return -ENODEV;	
	}	
	
	client = kzalloc(sizeof(*client), GFP_KERNEL);	
	
	if(!client)
	{		
		i2c_put_adapter(adapter);		
		ret = -ENOMEM;	
	}	
	snprintf(client->name, I2C_NAME_SIZE, "pctp_i2c_ts%d", adapter->nr);
	client->driver = &zet622x_i2c_driver;
	client->adapter = adapter;		
	file->private_data = client;
		
	return 0;
}


///************************************************************************
///   [function]:  zet_fops_release
///   [parameters]: inode, file
///   [return]: int
///************************************************************************
static int zet_fops_release (struct inode *inode, struct file *file) 
{
	struct i2c_client *client = file->private_data;

	printk("[ZET] : zet_fops_release -> line : %d\n",__LINE__ );
	
	i2c_put_adapter(client->adapter);
	kfree(client);
	file->private_data = NULL;
	return 0;	  
}

///************************************************************************
///   [function]:  zet_fops_read
///   [parameters]: file, buf, count, ppos
///   [return]: size_t
///************************************************************************
static ssize_t zet_fops_read(struct file *file, char __user *buf, size_t count,
			loff_t *ppos)
{
	int i;
	int iCnt = 0;
	char str[256];
	int len = 0;

	printk("[ZET] : zet_fops_read -> line : %d\n",__LINE__ );
	
	///-------------------------------///
	/// Print message
	///-------------------------------///	
	sprintf(str, "Please check \"%s\"\n", fw_file_name);
	len = strlen(str);

	///-------------------------------///
	/// if read out
	///-------------------------------///		
	if(data_offset >= len)
	{
		return 0;
        }		 	
	for(i = 0 ; i < count-1; i++)
	{
		buf[i] = str[data_offset];
		buf[i+1] = 0;
		iCnt++;
		data_offset++;
		if(data_offset >= len)
		{
			break;
		}
	}	
	
	///-------------------------------///
	/// Save file
	///-------------------------------///	
	if(data_offset == len)
	{
		zet_fw_save(fw_file_name);
	}	
	return iCnt;
}

///************************************************************************
///   [function]:  zet_fops_write
///   [parameters]: file, buf, count, ppos
///   [return]: size_t
///************************************************************************
static ssize_t zet_fops_write(struct file *file, const char __user *buf,
                                                size_t count, loff_t *ppos)
{	
	printk("[ZET]: zet_fops_write ->  %s\n", buf);
	data_offset = 0;
	return count;
}

///************************************************************************
///   [function]:  ioctl
///   [parameters]: file , cmd , arg
///   [return]: long
///************************************************************************
static long zet_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg )
{
  u8 __user * user_buf = (u8 __user *) arg;

	u8 buf[IOCTL_MAX_BUF_SIZE];
	int data_size;
	
	if(copy_from_user(buf, user_buf, IOCTL_MAX_BUF_SIZE))
	{
		printk("[ZET]: zet_ioctl: copy_from_user fail\n");
		return 0;
	}

	printk("[ZET]: zet_ioctl ->  cmd = %d, %02x, %02x\n",  cmd, buf[0], buf[1]);

	if(cmd == ZET_IOCTL_CMD_FLASH_READ)
	{
		printk("[ZET]: zet_ioctl -> ZET_IOCTL_CMD_FLASH_DUMP  cmd = %d, file=%s\n",  cmd, (char *)buf);
		ioctl_action |= IOCTL_ACTION_FLASH_DUMP;
	}
	else if(cmd == ZET_IOCTL_CMD_FLASH_WRITE)
	{
		printk("[ZET]: zet_ioctl -> ZET_IOCTL_CMD_FLASH_WRITE  cmd = %d\n",  cmd);		
	        zet622x_resume_downloader(this_client, firmware_upgrade, &rom_type, ic_model);		
	}
	else if(cmd == ZET_IOCTL_CMD_RST)
	{
		printk("[ZET]: zet_ioctl -> ZET_IOCTL_CMD_RST  cmd = %d\n",  cmd);
		//ctp_reset();
		ctp_set_reset_high();

		ctp_set_reset_low();	
		msleep(20);
		ctp_set_reset_high();

		transfer_type = TRAN_TYPE_DYNAMIC;			
	}
	else if(cmd == ZET_IOCTL_CMD_RST_HIGH)
	{
		ctp_set_reset_high();
	}
	else if(cmd == ZET_IOCTL_CMD_RST_LOW)
	{
		ctp_set_reset_low();	
	}
	else if(cmd == ZET_IOCTL_CMD_MDEV)
	{
		///---------------------------------------------------///
		/// set mutual dev mode
		///---------------------------------------------------///
		zet622x_ts_set_transfer_type(TRAN_TYPE_MUTUAL_SCAN_DEV);
		transfer_type = TRAN_TYPE_MUTUAL_SCAN_DEV;			
		
	}
	else if(cmd == ZET_IOCTL_CMD_IBASE)
	{
		///---------------------------------------------------///
		/// set initial base mode
		///---------------------------------------------------///
		zet622x_ts_set_transfer_type(TRAN_TYPE_INIT_SCAN_BASE);
		transfer_type = TRAN_TYPE_INIT_SCAN_BASE;
		
	}	
#ifdef FEATURE_IDEV_OUT_ENABLE 
	else if(cmd == ZET_IOCTL_CMD_IDEV)
	{
		///---------------------------------------------------///
		/// set initial dev mode
		///---------------------------------------------------///
		zet622x_ts_set_transfer_type(TRAN_TYPE_INIT_SCAN_DEV);
		transfer_type = TRAN_TYPE_INIT_SCAN_DEV;
		
	}
#endif ///< 	FEATURE_IDEV_OUT_ENABLE
#ifdef FEATURE_MBASE_OUT_ENABLE
	else if(cmd == ZET_IOCTL_CMD_MBASE)
	{
		///---------------------------------------------------///
		/// set Mutual Base mode
		///---------------------------------------------------///
		zet622x_ts_set_transfer_type(TRAN_TYPE_MUTUAL_SCAN_BASE);
		transfer_type = TRAN_TYPE_MUTUAL_SCAN_BASE;
		
	}
#endif ///< FEATURE_MBASE_OUT_ENABLE
 	else if(cmd == ZET_IOCTL_CMD_DYNAMIC)
  {
		zet622x_ts_set_transfer_type(TRAN_TYPE_DYNAMIC);
		transfer_type = TRAN_TYPE_DYNAMIC;
  }
	else if(cmd == ZET_IOCTL_CMD_FW_FILE_PATH_GET)
	{
		memset(buf, 0x00, 64);
		strcpy(buf, fw_file_name);		
		printk("[ZET]: zet_ioctl: Get FW_FILE_NAME = %s\n", buf);
	}
	else if(cmd == ZET_IOCTL_CMD_FW_FILE_PATH_SET)
	{
		strcpy(fw_file_name, buf);		
		printk("[ZET]: zet_ioctl: set FW_FILE_NAME = %s\n", buf);
	}
	else if(cmd == ZET_IOCTL_CMD_MDEV_GET)
        {
		data_size = (row+2)*(col+2);
		memcpy(buf, mdev_data, data_size);
                printk("[ZET]: zet_ioctl: Get MDEV data size=%d\n", data_size);
        }
	else if(cmd == ZET_IOCTL_CMD_TRAN_TYPE_PATH_SET)
	{
		strcpy(tran_type_mode_file_name, buf);		
		printk("[ZET]: zet_ioctl: Set ZET_IOCTL_CMD_TRAN_TYPE_PATH_ = %s\n", buf);
	}
	else if(cmd == ZET_IOCTL_CMD_TRAN_TYPE_PATH_GET)
	{
		memset(buf, 0x00, 64);
		strcpy(buf, tran_type_mode_file_name);	
		printk("[ZET]: zet_ioctl: Get ZET_IOCTL_CMD_TRAN_TYPE_PATH = %s\n", buf);
	}
	else if(cmd == ZET_IOCTL_CMD_IDEV_GET)
  	{
		data_size = (row + col);
		memcpy(buf, idev_data, data_size);
    		printk("[ZET]: zet_ioctl: Get IDEV data size=%d\n", data_size);
  	}
	else if(cmd == ZET_IOCTL_CMD_IBASE_GET)
  	{
		data_size = (row + col)*2;
		memcpy(buf, ibase_data, data_size);
    		printk("[ZET]: zet_ioctl: Get IBASE data size=%d\n", data_size);
  	}	
	else if(cmd == ZET_IOCTL_CMD_MBASE_GET)
	{
		data_size = (row*col*2);
		if(data_size > IOCTL_MAX_BUF_SIZE)
		{
			data_size = IOCTL_MAX_BUF_SIZE;
		}
		memcpy(buf, mbase_data, data_size);
		printk("[ZET]: zet_ioctl: Get MBASE data size=%d\n", data_size);
	}
	else if(cmd == ZET_IOCTL_CMD_INFO_SET)
  	{
		printk("[ZET]: zet_ioctl: ZET_IOCTL_CMD_INFO_SET\n");
		zet622x_ts_set_info_type();
	}
	else if(cmd == ZET_IOCTL_CMD_INFO_GET)
	{
		data_size = INFO_DATA_SIZE;
		memcpy(buf, info_data, data_size);
    		printk("[ZET]: zet_ioctl: Get INFO data size=%d,IC: %x,X:%d,Y:%d\n", data_size, info_data[0], info_data[13], info_data[14]);
  	}
	else if(cmd == ZET_IOCTL_CMD_TRACE_X_SET)
	{
		printk("[ZET]: zet_ioctl: ZET_IOCTL_CMD_TRACE_X_SET\n");
	}
	else if(cmd == ZET_IOCTL_CMD_TRACE_X_GET)
	{
		printk("[ZET]: zet_ioctl: Get TRACEX data\n");
	}
	else if(cmd == ZET_IOCTL_CMD_TRACE_Y_SET)
	{
		printk("[ZET]: zet_ioctl: ZET_IOCTL_CMD_TRACE_Y_SET\n");
	}
	else if(cmd == ZET_IOCTL_CMD_TRACE_Y_GET)
	{
		printk("[ZET]: zet_ioctl: Get TRACEY data \n");
	}
	else if(cmd == ZET_IOCTL_CMD_DRIVER_VER_GET)
	{
		memset(buf, 0x00, 64);
		strcpy(buf, driver_version);		
		printk("[ZET]: zet_ioctl: Get DRIVER_VERSION = %s\n", buf);
		printk("[ZET]: zet_ioctl: Get SVN = %s\n", DRIVER_VERSION);
	}
	else if(cmd == ZET_IOCTL_CMD_MBASE_EXTERN_GET)
	{
		data_size = (row*col*2) - IOCTL_MAX_BUF_SIZE;
		if(data_size < 1)
		{
			data_size = 1;
		}
		memcpy(buf, (mbase_data+IOCTL_MAX_BUF_SIZE), data_size);
		printk("[ZET]: zet_ioctl: Get MBASE extern data size=%d\n", data_size);
	}
	
	if(copy_to_user(user_buf, buf, IOCTL_MAX_BUF_SIZE))
	{
		printk("[ZET]: zet_ioctl: copy_to_user fail\n");
		return 0;
	}

	return 0;
}

///************************************************************************
///	file_operations
///************************************************************************
static const struct file_operations zet622x_ts_fops =
{	
	.owner		= THIS_MODULE, 	
	.open 		= zet_fops_open, 	
	.read 		= zet_fops_read, 	
	.write		= zet_fops_write, 
	.unlocked_ioctl = zet_fops_ioctl,
	.compat_ioctl	= zet_fops_ioctl,
	.release	= zet_fops_release, 
};

///************************************************************************
///   [function]:  zet622x_ts_remove
///   [parameters]:
///   [return]:
///************************************************************************
static int __devexit zet622x_ts_remove(struct i2c_client *dev)
{
	struct zet622x_tsdrv *zet6221_ts = i2c_get_clientdata(dev);

	printk("==zet622x_ts_remove=\n");
	del_timer_sync(&zet6221_ts->zet622x_ts_timer_task);
	free_irq(zet6221_ts->irq, zet6221_ts);
	///------------------------------------------///
	/// unregister early_suspend
	///------------------------------------------///
	unregister_early_suspend(&zet6221_ts->early_suspend);
	input_unregister_device(zet6221_ts->input);
	input_free_device(zet6221_ts->input);
	destroy_workqueue(zet6221_ts->ts_workqueue); //  workqueue
	destroy_workqueue(zet6221_ts->ts_workqueue1);
	kfree(zet6221_ts);
	i2c_set_clientdata(dev, NULL);
	/// release the buffer
	zet_fw_exit();

	return 0;
}

///************************************************************************
///   [function]:  zet622x_ts_probe
///   [parameters]:  i2c_client, i2c_id
///   [return]: int
///************************************************************************
static int __devinit zet622x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int result;
	int err = 0;
	
	struct input_dev *input_dev;
	struct zet622x_tsdrv *zet6221_ts;
	struct i2c_dev *i2c_dev;
	struct device *dev;
	struct zet62xx_platform_data *pdata;
	
	printk("[ZET]: Probe Zet62xx\n");
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		dev_err(&client->dev, "Must have I2C_FUNC_I2C.\n");
		result = -ENODEV;
		goto LABEL_CHECK_FUNC_FAIL;
	}
	
	pdata=(struct zet62xx_platform_data *)client->dev.platform_data;
	///------------------------------------------------///
	/// Check the rst pin have other driver used
	///------------------------------------------------///
	if(pdata&&pdata->init_platform_hw){
		if(pdata->init_platform_hw()<0)
			goto LABEL_GPIO_REQUEST_FAIL;
	}
	
	///------------------------------------------------///
	/// init the finger pressed data
	///------------------------------------------------///
#ifdef FEAURE_LIGHT_LOAD_REPORT_MODE
	zet62xx_ts_init();
#endif  ///< for FEAURE_LIGHT_LOAD_REPORT_MODE

	///------------------------------------------------///
	/// allocate zet32xx touch screen device driver
	///------------------------------------------------///
	zet6221_ts = kzalloc(sizeof(struct zet622x_tsdrv), GFP_KERNEL);
	if(zet6221_ts == NULL) {
		printk("kzalloc zet6221_ts failed.........");
		goto LABEL_MEM_REQ_FAIL;
	}

	///------------------------------------------------///
	/// hook i2c to this_client
	///------------------------------------------------///
	zet6221_ts->i2c_dev = client;
	if(pdata&&(pdata->irq_gpio!=INVALID_GPIO))
		zet6221_ts->irq_gpio=pdata->irq_gpio;
	else
	  zet6221_ts->irq_gpio = client->irq;

	if(pdata&&(pdata->reset_gpio!=INVALID_GPIO))
		zet6221_ts->rst_gpio=pdata->reset_gpio;
	else
		zet6221_ts->rst_gpio=INVALID_GPIO;
	if(pdata&&pdata->get_system_charge_status)
		zet6221_ts->get_system_charge_status=pdata->get_system_charge_status;
	else
		zet6221_ts->get_system_charge_status=NULL;
	
	this_client = client;	
	i2c_set_clientdata(client, zet6221_ts);

	///------------------------------------------------///
	/// driver
	///------------------------------------------------///
	client->driver = &zet622x_i2c_driver;

	///------------------------------------------------///
	///  init finger report work
	///------------------------------------------------///
	INIT_WORK(&zet6221_ts->work1, zet622x_ts_work);
	zet6221_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!zet6221_ts->ts_workqueue)
	{
		printk("[ZET] : ts_workqueue ts_probe error ==========\n");
		goto LABEL_WORK_QUEUE_INIT_FAIL0;
	}
	
	///-----------------------------------------------///
	///   charger detect : write_cmd
	///-----------------------------------------------///
	INIT_WORK(&zet6221_ts->work2, zet622x_charger_cmd_work);
	zet6221_ts->ts_workqueue1 = create_singlethread_workqueue(dev_name(&client->dev)); //  workqueue
	if (!zet6221_ts->ts_workqueue1)
	{
		printk("ts_workqueue1 ts_probe error ==========\n");
		goto LABEL_WORK_QUEUE_INIT_FAIL1;
	}

	///-----------------------------------------------///
	/// touch input device regist
	///-----------------------------------------------///
	input_dev = input_allocate_device();
	if (!input_dev || !zet6221_ts)
	{
		result = -ENOMEM;
		goto LABEL_DEVICE_ALLOC_FAIL;
	}
	
	input_dev->name = MJ5_TS_NAME;
	input_dev->phys = "zet6221_touch/input0";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor  = 0x0001;
	input_dev->id.product = 0x0002;
	input_dev->id.version = 0x0100;

	ic_model = MODEL_ZET6221; ///< Set the default model name

	///-----------------------------------------------///
	/// Set the default firmware bin file name & mutual dev file name
	///-----------------------------------------------///
	zet_dv_set_file_name(DRIVER_VERSION);
	zet_fw_set_file_name(FW_FILE_NAME);
	zet_tran_type_set_file_name(TRAN_MODE_FILE_PATH);


#ifdef FEATURE_FW_UPGRADE
	///-----------------------------------------------///
	///   Do firmware downloader
	///-----------------------------------------------///
	if(zet622x_downloader(client,firmware_upgrade,&rom_type,ic_model) <= 0)
	{
		goto LABEL_DOWNLOAD_FAIL;	
	}
#endif  ///< for FEATURE_FW_UPGRADE
        ///-----------------------------------------------///
        /// wakeup pin for reset        
        ///-----------------------------------------------///
   ctp_wakeup2(5);

#ifdef FEATURE_TPINFO 
	///-----------------------------------------------///
	/// B2 Command : read tp information
	///-----------------------------------------------///
	if(zet622x_ts_get_information(client) <= 0)
	{
		goto LABEL_GET_INFO_FAIL;
	}
	
#else ///< for FEATURE_TPINFO
	///-----------------------------------------------///	
	/// set the TP information not by B2
	///-----------------------------------------------///
	resolution_x = X_MAX;
	resolution_y = Y_MAX;
	
	finger_num   = FINGER_NUMBER;
	key_num      = KEY_NUMBER;   
	if(key_num == 0)
	{
		finger_packet_size  = 3 + 4*finger_num;
	}
	else
	{
		finger_packet_size  = 3 + 4*finger_num + 1;
	}
#endif ///< for FEATURE_TPINFO
	
	printk( "[ZET] : resolution= (%d x %d ), finger_num=%d, key_num=%d\n",resolution_x,resolution_y,finger_num,key_num);

	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);	

#ifdef FEATURE_MT_TYPE_B
	///-----------------------------------------------///	
	/// set type B finger number
	///-----------------------------------------------///
	input_mt_init_slots(input_dev, finger_num);	
#endif ///< for FEATURE_MT_TYPE_B
	
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit); 
	set_bit(ABS_MT_POSITION_X,  input_dev->absbit); 
	set_bit(ABS_MT_POSITION_Y,  input_dev->absbit); 
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit); 
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, P_MAX, 0, 0);


	///------------------------------------------///
	/// Set virtual key
	///------------------------------------------///
#ifdef FEATURE_VIRTUAL_KEY
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, TP_AA_X_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, TP_AA_Y_MAX, 0, 0);
#else ///< for FEATURE_VIRTUAL_KEY
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, resolution_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, resolution_y, 0, 0);
#endif ///< for FEATURE_VIRTUAL_KEY

	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(KEY_HOME, input_dev->keybit);
	set_bit(KEY_SEARCH, input_dev->keybit);

	input_dev->evbit[0] = BIT(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);

	result = input_register_device(input_dev);
	if(result)
	{
		goto LABEL_DEV_REGISTER_FAIL;
	}

	///------------------------------------------///
	/// Config early_suspend
	///------------------------------------------///
	printk("==register_early_suspend =\n");
	zet6221_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 5;
	zet6221_ts->early_suspend.suspend = zet622x_ts_early_suspend;
	zet6221_ts->early_suspend.resume = zet622x_ts_late_resume;
	register_early_suspend(&zet6221_ts->early_suspend);

	zet6221_ts->input = input_dev;

	input_set_drvdata(zet6221_ts->input, zet6221_ts);

	

	///-----------------------------------------------///
	/// Try to Get GPIO to see whether it is allocated to other drivers
	///-----------------------------------------------///
	printk( "[ZET]: ------request GPIO start------\n");
	result = gpio_request(zet6221_ts->irq_gpio, "GPN"); 
	if (result)
	{
		printk( "[ZET]: ------request GPIO failed------\n");
		goto LABEL_IRQ_GPIO_ALLOC_FAIL;
	}
	///-----------------------------------------------///
	/// Set IRQ corresponding to GPIO
	///-----------------------------------------------///
	zet6221_ts->irq = gpio_to_irq(zet6221_ts->irq_gpio);
	printk( "[ZET]: zet6221_ts_probe.gpid_to_irq [zet6221_ts->irq=%d]\n", zet6221_ts->irq);
	
	///--------------------------------------------///
	/// set the finger report interrupt (INT = low)
	///--------------------------------------------///
	err = request_irq(zet6221_ts->irq, zet622x_ts_interrupt, 
				(IRQF_TRIGGER_FALLING | IRQF_SHARED), ZET_TS_ID_NAME, zet6221_ts);

	if(err < 0)
	{
		printk( "[ZET]:zet622x_ts_probe.request_irq failed. err=%d\n",err);
		goto LABEL_IRQ_REQUEST_FAIL;
	}
  ///------------------------------------------///
	/// Set charger mode timer
	///------------------------------------------///
	setup_timer(&zet6221_ts->zet622x_ts_timer_task, zet622x_ts_timer_task, (unsigned long)zet6221_ts);
	mod_timer(&zet6221_ts->zet622x_ts_timer_task, jiffies + msecs_to_jiffies(800));
	///--------------------------------------------///
	/// Get a free i2c dev
	///--------------------------------------------///
	i2c_dev = zet622x_i2c_get_free_dev(client->adapter);	
	if(IS_ERR(i2c_dev))
	{	
		err = PTR_ERR(i2c_dev);		
		return err;	
	}
	dev = device_create(i2c_dev_class, &client->adapter->dev, 
				MKDEV(I2C_MAJOR,client->adapter->nr), NULL, "zet62xx_ts%d", client->adapter->nr);	
	if(IS_ERR(dev))
	{		
		err = PTR_ERR(dev);		
		return err;	
	}
  printk("zet62xx probe ok........");
	return 0;

LABEL_IRQ_REQUEST_FAIL:
	  free_irq(zet6221_ts->irq,zet6221_ts);
		gpio_free(zet6221_ts->irq_gpio);
LABEL_IRQ_GPIO_ALLOC_FAIL:
LABEL_DEV_REGISTER_FAIL:
	input_unregister_device(input_dev);
LABEL_GET_INFO_FAIL:
LABEL_DOWNLOAD_FAIL:
	input_free_device(input_dev);
LABEL_DEVICE_ALLOC_FAIL:
	destroy_workqueue(zet6221_ts->ts_workqueue1);
LABEL_WORK_QUEUE_INIT_FAIL1:
	destroy_workqueue(zet6221_ts->ts_workqueue);
LABEL_WORK_QUEUE_INIT_FAIL0:
	gpio_free(zet6221_ts->rst_gpio);
	kfree(zet6221_ts);
LABEL_MEM_REQ_FAIL:
LABEL_GPIO_REQUEST_FAIL:
LABEL_CHECK_FUNC_FAIL:
	return result;
}



///************************************************************************
///   [function]:  zet622x_module_init
///   [parameters]:  void
///   [return]: int
///************************************************************************
static int __init zet622x_module_init(void)
{
	int ret = -1;

	///---------------------------------///
	/// Set file operations
	///---------------------------------///	
	ret= register_chrdev(I2C_MAJOR, "zet_i2c_ts", &zet622x_ts_fops );
	if(ret)
	{	
		printk(KERN_ERR "%s:register chrdev failed\n",__FILE__);	
		return ret;
	}

	///---------------------------------///
	/// Create device class
	///---------------------------------///
	i2c_dev_class = class_create(THIS_MODULE,"zet_i2c_dev");
	if(IS_ERR(i2c_dev_class))
	{		
		ret = PTR_ERR(i2c_dev_class);		
		class_destroy(i2c_dev_class);	
	}

	///---------------------------------///
	/// Add the zet622x_ts to i2c drivers
	///---------------------------------///
	i2c_add_driver(&zet622x_i2c_driver);
	
	return ret;
}


///***********************************************************************
///   [function]:  ts exit
///   [parameters]:
///   [return]:
///***********************************************************************
static void __exit zet622x_module_exit(void)
{
	i2c_del_driver(&zet622x_i2c_driver);
	if (resume_download_task != NULL)
	{
		kthread_stop(resume_download_task);
	}
}

module_init(zet622x_module_init);
module_exit(zet622x_module_exit);
MODULE_DESCRIPTION("ZET6221 I2C Touch Screen driver");
MODULE_LICENSE("GPL v2");
