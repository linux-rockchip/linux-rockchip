/*************************************************************************/
/*                                                                       */
/* Copyright (C) 2005-2007 Siano Mobile Silicon Ltd. All rights reserved */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                                             */
/*                                                                       */
/*      AppDriverLinux.c                                                 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Implementation Of Application driver interface For Linux		 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*************************************************************************/

#include <linux/delay.h>
//#include <linux/i2c.h>

#include "../../../include_internal/SmsLiteAppDriver.h"
#include "DriverFunc.h"

#include "../SmsLitePlatDefs.h"
#include "../../../include/SmsHostLibTypes.h"
#include "../../../include_internal/SmsFirmwareApi.h"
//#include"DVB_global.h"
#include"../../../include/SmsHostLibLiteMs.h"
#include "../../../include_internal/Include/sms_common.h"

#define ADR_NUM_OF_HANDLES	(SMS_MAX_SERVICE_HANDLE+1)
#define SMS_ADR_LITE_MAX_DEVICE_NAME_LEN (256)
#define ZERO_MEM_OBJ(pObj) memset((pObj), 0, sizeof(*(pObj)))

/*not used*/
#define DVB_SINAO_TUNER_FW_DOWN_CLK 0
#define DVB_DEMODE_DIBCOM_I2C_RATE_K 0
//////////////////////////////////////////////////////////////////////////////

#if 1
#define DBG(x...)	printk(KERN_INFO x) //printk(KERN_ERR x)
#else
#define DBG(x...)
#endif

// Struct that describes an SMS device in Win 32
typedef struct WIN32_ADR_DeviceData_S
{
	char				DeviceName[SMS_ADR_LITE_MAX_DEVICE_NAME_LEN];
	BOOL				IsDeviceFound;
//	WIN32_ADR_HANDLE_ST	AdrHandleList[ADR_NUM_OF_HANDLES];
	UINT32				OpenHandles;
	SMSHOSTLIB_DEVICE_MODES_E DeviceMode;
	SmsLiteAdr_pfnFuncCb pfnCtrlCallback[ADR_NUM_OF_HANDLES];
	SmsLiteAdr_pfnFuncCb pfnDataCallback[ADR_NUM_OF_HANDLES];
} WIN32_ADR_DeviceData_ST;

static BOOL						g_AdrInit = FALSE;
static WIN32_ADR_DeviceData_ST	g_DevData = {0};
//extern BOOL g_bHaveSignalIndicator;
//extern BOOL g_bDummyHaveSignal;
//extern int32 __devinit _I2CInit(eI2C_ch_t i2cNumb, uint16 speed,uint16 I2CSlaveAddr,eI2C_Address_Reg_t addressBit, eI2C_mode_t mode);

#define SPI_INTERFACE  0
#define I2C_TS_INTERFACE 1
//#define SMS_FILE_SYSTEM 0

#define FW_FILE_DVB_T		"C:\\dvbb0.inp"
#define FW_FILE_ISDB_T		"C:\\isdbtb0.inp"
//#define FW_FILE_ISDB_T		"C:\\dvbb0.inp"

#define I2C_SEC_CTR			1
#define NUM_RX_BUFFERS 	    5

#define DEFAULT_I2C_DEVICE_HANDLE		1
#define I2C_SIANO_DEVICE_ADDR			0x68

#define SIANO_FW_NAME "C:\\SYSTEM\\sianoFW.bin"

#define 	MSG_SMS_NO_IP_DATA_CHANNEL  	607 		// cmmb, t-dmb, isdbt-t, dvb-t MSG_SMS_DAB_CHANNEL=607,
#define   MSG_SMS_IP_DATA_CHANNEL  		699		//dvbh 	MSG_SMS_DATA_MSG				= 699,
//ALIGN(4)  char                g_RX_BUFF[RX_BUF_SIZE];

//#define SIANO_SMS1140_TUNER
//#define TUNER_ISDB_T

__aligned(4) const char Siano1130FW[]=
{
#if(defined(SIANO_SMS1130_TUNER))
    #ifdef TUNER_ISDB_T
        //#include "isdbt1130b0.inc"    
	#include "isdbt_1140_11_02.inc"//SIANO 巴西实测
    #else
        //#include "dvb1130b0.inc"
        #include "dvbt1140_1013.inc"
	//#include "dvbt1140_1113.inc"
	//#include "dvbt1140_2_0_33.inc"
	//#include "dvbt1140_2_0_34.inc"
	//#include "dvb_1140_rk_12Mhz_b0_2_0_37rk.inc"
	// for test
				 // #include "dvbt1140_2_0_27.inc"
    #endif    
#elif(defined(SIANO_SMS1140_TUNER))  
    #ifdef TUNER_ISDB_T
        //#include "isdb1140_0916.inc"    
        #include "isdbt_1140_11_02.inc"//SIANO 巴西实测
    #else
//        #include "dvbt1140_0916.inc"
// for test
//				  #include "dvbt1140_2_0_27.inc"
// for test 1012 morning
 //    #include "dvbt1140_10_12.inc"
 
  #include "dvbt1140_1013.inc"
  //#include "dvbt1140_1113.inc"
  //#include "dvbt1140_2_0_33.inc"
  //#include "dvbt1140_2_0_34.inc"
  //#include "dvb_1140_rk_12Mhz_b0_2_0_37rk.inc"
    #endif     
#endif
    //0x00, 0x00, 0x00, 0x00
};


typedef enum SmsTsiMode_E
{
	TSI_SERIAL_ON_SDIO,
	TSI_SERIAL_ON_HIF,
	TSI_PARALLEL_ON_HIF,
	TSI_MAX_MODE
}SmsTsiMode_ET;

typedef enum SmsTsiSigActive_E
{
	TSI_SIGNALS_ACTIVE_LOW,
	TSI_SIGNALS_ACTIVE_HIGH,
	TSI_MAX_SIG_ACTIVE
}SmsTsiSigActive_ET;

typedef enum SmsTsiSensPolar_E
{
	TSI_SIG_OUT_FALL_EDGE,
	TSI_SIG_OUT_RISE_EDGE,
	TSI_MAX_CLK_POLAR
}SmsTsiSensPolar_ET;

typedef enum SmsTsiBitOrder_E
{
	TSI_BIT7_IS_MSB,
	TSI_BIT0_IS_MSB,
	TSI_MAX_BIT_ORDER
}SmsTsiBitOrder_ET;

typedef struct SmsIntLine_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			Controler;
	UINT32			GpioNum;
	UINT32			PulseWidth;
} SmsIntLine_ST;

/*
typedef struct SmsTsEnable_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			TsClock;                // TS Clock Speed in Hz
	UINT32			eTsiMode;               // TS Mode of operation Serial (on SDIO or HIF Pins), or Parallel
	UINT32		    eTsiSignals;            // Level of Valid, Sync and Error signals when active
	UINT32			nTsiPcktDelay;          // number of delay bytes between TS packets (for 204bytes mode set to 16)
	UINT32		    eTsClockPolarity;		// Clock edge to sample data
	UINT32		    TsBitOrder;				// Bit order in TS output
    UINT32          EnableControlOverTs;    // Enable Control messages over TS interface
} SmsTsEnable_ST;
*/

typedef enum SmsTsiFormat_E
{
	TSI_TRANSPARENT,
	TSI_ENCAPSULATED,
	TSI_MAX_FORMAT
}SmsTsiFormat_ET;

typedef enum SmsTsiErrActive_E
{
	TSI_ERR_NOT_ACTIVE,
	TSI_ERR_ACTIVE,
	TSI_MAX_ERR_ACTIVE
}SmsTsiErrActive_ET;


typedef enum SmsTsiClockKeepGo_E
{
	TSI_CLK_STAY_LOW_GO_NO_PKT,
	TSI_CLK_KEEP_GO_NO_PKT,
	TSI_MAX_CLK_ON
}SmsTsiClockKeepGo_ET;



//////////////////////////////////////////////////////////////////////////
//
// TS enable message format
//
typedef struct SmsTsEnable_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			TsClock;                // TS Clock Speed in Hz
	UINT32			eTsiMode;               // TS Mode of operation Serial (on SDIO or HIF Pins), or Parallel
	UINT32		    eTsiSignals;            // Level of Valid, Sync and Error signals when active
	UINT32			nTsiPcktDelay;          // number of delay bytes between TS packets (for 204bytes mode set to 16)
	UINT32		    eTsClockPolarity;		// Clock edge to sample data
	UINT32		    TsBitOrder;				// Bit order in TS output
	UINT32          EnableControlOverTs;    // Enable Control messages over TS interface
	UINT32          TsiEncapsulationFormat; // TS encapsulation method
	UINT32			TsiPaddingPackets;		// Number of TS padding packets appended to control messages
} SmsTsEnable_ST;

typedef struct SmsAdrI2cDevice_S
{
	UINT32						deviceInit;
	SMSHOSTLIB_DEVICE_MODES_E	deviceMode;
	ADR_pfnFuncCb				pfnFuncCb;
	void*						clientPtr;
	char				DeviceName[SMS_ADR_LITE_MAX_DEVICE_NAME_LEN];
	BOOL				IsDeviceFound;
	
} SmsAdrI2cDevice_ST;

/*i2c interface*/
extern struct i2c_client *gTSTVModuleClient;
extern int  tstv_control_set_reg(struct i2c_client *client, UINT8 reg, UINT8 const buf[], int len);
extern int  tstv_control_read_regs(struct i2c_client *client, UINT8 reg, UINT8 buf[], unsigned len);


__aligned(4) static  UINT8  TX_CMD_BUFF[256]={0};
__aligned(4) SmsIntLine_ST	g_GpioMsg		= {{MSG_SMS_SPI_INT_LINE_SET_REQ, 0, 11, 20, MSG_HDR_FLAG_STATIC_MSG}, I2C_SEC_CTR, 32, 50};
__aligned(4) SmsTsEnable_ST g_TsEnableMsg    = {{MSG_SMS_ENBALE_TS_INTERFACE_REQ, 0, 11, sizeof(SmsTsEnable_ST), MSG_HDR_FLAG_STATIC_MSG}, 
//								    4000000, 
#ifdef TUNER_ISDB_T
								    1600000,  // For ISDB-T OK
#else
								    4000000,  //  For DVB-T
#endif
								    TSI_PARALLEL_ON_HIF,
								    TSI_SIGNALS_ACTIVE_LOW,
								    0,
								    TSI_SIG_OUT_FALL_EDGE,
								    TSI_BIT7_IS_MSB,
//                                    1,  // control msg output from ts interface
                                    0, // control msg output from i2c
                                    TSI_TRANSPARENT,		// Enable encapsulated data
 //                                   								10};
								22};


__aligned(4) SmsMsgData_ST g_GetVerSmsMsg = {{MSG_SMS_GET_VERSION_EX_REQ, 0, 11, sizeof(SmsMsgData_ST), MSG_HDR_FLAG_STATIC_MSG},0};
	


OSW_TaskId				gp_I2CRxTask;

Event					g_I2cRxEvent;

__aligned(4) static  UINT8 RX_RAM_BUFF[0x5000];
UINT8*			g_rx_buffers[NUM_RX_BUFFERS];
static int g_iReadRetOK =0;
static int g_iWriteRetOK =0;


//extern void SmsHostADRLog(UINT32 mask,char* format, ...);
void handle_completed_msg(SmsMsgData_ST* pMsg);

void handle_completed_msg(SmsMsgData_ST* pMsg)
{

	  //send message to host library
	if ((g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE] != NULL) && (pMsg->xMsgHeader.msgLength < 0x1000) && (pMsg->xMsgHeader.msgDstId != 0))
	{
		g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE](ADR_CTRL_HANDLE, (UINT8* )pMsg, pMsg->xMsgHeader.msgLength);
	}



}

		
//#define IO_DBG_SerialPrintf_(mask,format)	SmsHostADRLog(mask,format)

// write data throught I2c bus
void SIANO_Demode_dib_tcc_I2C_Write_manual( UINT32 I2c_addr, UINT8*pBuffer, UINT32 uLength,UINT32 I2C_SPEED)
{
	static int iRet;
	//_I2CInit(I2c_addr,I2C_SPEED);
	//iRet =I2C_Write(I2c_addr,pBuffer,uLength,DirectMode);
        DBG("TSTV: pBuffer %p, uLength %d\t%s[%d]\n", pBuffer, uLength, __FUNCTION__, __LINE__);
        iRet = tstv_control_set_reg(gTSTVModuleClient, 0, pBuffer, uLength);
	if(iRet != 0)
	{
                DBG("TSTV: set i2c register failed\t%s[%d]\n", __FUNCTION__, __LINE__);
		iRet++;
	}
	else
        {
		g_iWriteRetOK ++;
                DBG("TSTV: set i2c register success\t%s[%d]\n", __FUNCTION__, __LINE__);
        }
	//I2C_Deinit();	
}

// read data throught I2c bus
void SIANO_Demode_dib_tcc_I2C_Read_manual( UINT32 I2c_addr, UINT8 p1, UINT8 p2, UINT8*pBuffer, UINT32 uLength)
{
	static int iRet;
	
	//_I2CInit(I2c_addr,DVB_DEMODE_DIBCOM_I2C_RATE_K);
	//iRet =_I2CRead(I2c_addr,pBuffer,uLength,DirectMode);
        DBG("TSTV: pBuffer %p, uLength %d\t%s[%d]\n", pBuffer, uLength, __FUNCTION__, __LINE__);
        iRet = tstv_control_read_regs(gTSTVModuleClient, 0, pBuffer, uLength);
	if(iRet != 0)
	{
                DBG("TSTV: read i2c register failed\t%s[%d]\n", __FUNCTION__, __LINE__);
		iRet++;
	}
	else
        {
		g_iReadRetOK ++;
                DBG("TSTV: read i2c register success\t%s[%d]\n", __FUNCTION__, __LINE__);
        }
	//_I2CDeinit();
}



int i2c_send(SmsMsgData_ST* p_msg)
{
#if 0
	UINT8 *buf = malloc(256);
	if(p_msg)
	{
//		memcpy(buf, smsspi_preamble, sizeof(smsspi_preamble));
		memcpy(buf, p_msg, p_msg->xMsgHeader.msgLength);
		RockOSSendMsg(CMMB_DEMUX_TASK, AS_CMMB_RECEIVE, buf);
		return 1;
	}
	return 0;
#else
	int result = -1, len;
	len =p_msg->xMsgHeader.msgLength;
	//I2C_Init((unsigned char)0xD0,DVB_DEMODE_DIBCOM_I2C_RATE_K);
	//result =I2C_Write((unsigned char)0xD0,(UINT8 *)p_msg,len,DirectMode);
        result = tstv_control_set_reg(gTSTVModuleClient, 0, (UINT8 *)p_msg, len);
	//I2C_Deinit();
#endif
}



int i2c_read_response( UINT32  request)
{
    // do nothing
}

#if 1
int i2c_read(UINT32  response )  //ran shalgi
{

 BOOLEAN ret = FALSE;
 SmsMsgData_ST* p_msg = NULL;
 UINT8*   i2c_rx_buf;
 UINT32   bytes_to_read;
 UINT32   cur_rx_buf=0, j;
 UINT32       ret_code=0;
 UINT32   i = 0;

 DummyStatMsg_ST* pDummyMsg=NULL;

 i2c_rx_buf = (UINT8*)(g_rx_buffers[0]);
 memset (i2c_rx_buf, 0xaa, 0x1000);
 // read message header
 SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0, i2c_rx_buf, sizeof(DummyStatMsg_ST));   
           
 p_msg = (SmsMsgData_ST*)i2c_rx_buf;
 /*(SmsMsgData_ST*)*/pDummyMsg = (SmsMsgData_ST*)i2c_rx_buf;

 //////////////// 11/18/2009 Ran Shalgi - added section
 if ((p_msg->xMsgHeader.msgType == MSG_SMS_DUMMY_STAT_RES) && (response == MSG_SMS_DUMMY_STAT_RES))
 {
       // Correct message received, we're done
       //send message to host library
       if ((g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE] != NULL) && (p_msg->xMsgHeader.msgLength < 0x1000) && (p_msg->xMsgHeader.msgDstId != 0))
       {
             g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE](ADR_CTRL_HANDLE, (UINT8* )p_msg, p_msg->xMsgHeader.msgLength);
       }

      return 0xffff;
 }
 ///////////////////////////////////////



 while ( (p_msg->xMsgHeader.msgType == MSG_SMS_DUMMY_STAT_RES) && (response != MSG_SMS_DUMMY_STAT_RES) && (i < 10))
 {
  i++;
  SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0, i2c_rx_buf, sizeof(SmsMsgHdr_ST));   
  //OSW_TaskSleep(10);
  msleep(10); //jan
  //mdelay(10); //jan
 
 } 
 // IO_DBG_SerialPrintf_("ADR_I2cRxTask: Read header. Type=%d, Length=%d\n", 
            //                   pMsg->xMsgHeader.msgType, pMsg->xMsgHeader.msgLength);

          // check if there is a payload to the message
 if ((p_msg->xMsgHeader.msgLength > 8) && (p_msg->xMsgHeader.msgType != MSG_SMS_DUMMY_STAT_RES))
// if ((p_msg->xMsgHeader.msgLength > 8))
 {
   bytes_to_read = p_msg->xMsgHeader.msgLength > 0x1000 ? 0x1000 : p_msg->xMsgHeader.msgLength;
     
   // since host is much faster then device, need to wait for device to prepare next buffer for read
   //OSW_TaskSleep(20);
   msleep(20); //jan
   //mdelay(20); //jan
   //lock bus
   //OSSchedLock(); //jan      

   SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0,(UINT8*)&i2c_rx_buf[sizeof(SmsMsgHdr_ST)], bytes_to_read - 8);
       //unlock bus
       //OSSchedUnlock();// ca test //jan            
 }

 //////////////// 11/18/2009 Ran Shalgi - this section needs to be removed.  

//    if ((p_msg->xMsgHeader.msgLength > 8)&&(p_msg->xMsgHeader.msgType == MSG_SMS_DUMMY_STAT_RES)){
//
//   bytes_to_read = p_msg->xMsgHeader.msgLength > 0x1000 ? 0x1000 : p_msg->xMsgHeader.msgLength;
//   // since host is much faster then device, need to wait for device to prepare next buffer for read
//   OSW_TaskSleep(20);
//     //lock bus
//       OSSchedLock();        
//
//   SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0,(UINT8*)&i2c_rx_buf[0], bytes_to_read);
//       //unlock bus
//      OSSchedUnlock();// ca test             
//     
////   if (pDummyMsg->DemodLocked == TRUE ){
////       g_bDummyHaveSignal = TRUE;
////   }
//  }
  
/////////////////////////////////////////////////
 

   //send message to host library
 if ((g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE] != NULL) && (p_msg->xMsgHeader.msgLength < 0x1000) && (p_msg->xMsgHeader.msgDstId != 0))
 {
  g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE](ADR_CTRL_HANDLE, (UINT8* )p_msg, p_msg->xMsgHeader.msgLength);
 }

  //    if ((p_msg->xMsgHeader.msgType == MSG_SMS_NO_SIGNAL_IND) || (p_msg->xMsgHeader.msgType ==MSG_SMS_SIGNAL_DETECTED_IND))
//    g_bHaveSignalIndicator = TRUE;
    
      if (p_msg->xMsgHeader.msgType == response)
               return 0xffff;

 return p_msg->xMsgHeader.msgLength;
}

#else
int i2c_read(UINT32  response )
{

	BOOLEAN ret = FALSE;
	SmsMsgData_ST*	p_msg = NULL;
	UINT8*			i2c_rx_buf;
	UINT32			bytes_to_read;
	UINT32			cur_rx_buf=0, j;
	UINT32       ret_code=0;
	UINT32			i = 0;

	DummyStatMsg_ST* pDummyMsg=NULL;

	i2c_rx_buf = (UINT8*)(g_rx_buffers[0]);
	memset (i2c_rx_buf, 0xaa, 0x1000);
	// read message header
	SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0, i2c_rx_buf, sizeof(DummyStatMsg_ST));			
        			
	p_msg = (SmsMsgData_ST*)i2c_rx_buf;
	(SmsMsgData_ST*)pDummyMsg = (SmsMsgData_ST*)i2c_rx_buf;

	while ( (p_msg->xMsgHeader.msgType == MSG_SMS_DUMMY_STAT_RES) && (response != MSG_SMS_DUMMY_STAT_RES) && (i < 10))
	{
		i++;
		SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0, i2c_rx_buf, sizeof(SmsMsgHdr_ST));			
		OSW_TaskSleep(10);

	
	}	
	//	IO_DBG_SerialPrintf_("ADR_I2cRxTask: Read header. Type=%d, Length=%d\n", 
		          //                   pMsg->xMsgHeader.msgType, pMsg->xMsgHeader.msgLength);

		        // check if there is a payload to the message
	if ((p_msg->xMsgHeader.msgLength > 8) && (p_msg->xMsgHeader.msgType != MSG_SMS_DUMMY_STAT_RES))
//	if ((p_msg->xMsgHeader.msgLength > 8))
	{
			bytes_to_read = p_msg->xMsgHeader.msgLength > 0x1000 ? 0x1000 : p_msg->xMsgHeader.msgLength;
					
			// since host is much faster then device, need to wait for device to prepare next buffer for read
			OSW_TaskSleep(20);
			//lock bus
    			OSSchedLock();        

			SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0,(UINT8*)&i2c_rx_buf[sizeof(SmsMsgHdr_ST)], bytes_to_read - 8);
    			//unlock bus
    			OSSchedUnlock();// ca test             
	}


   	if ((p_msg->xMsgHeader.msgLength > 8)&&(p_msg->xMsgHeader.msgType == MSG_SMS_DUMMY_STAT_RES)){

		bytes_to_read = p_msg->xMsgHeader.msgLength > 0x1000 ? 0x1000 : p_msg->xMsgHeader.msgLength;
		// since host is much faster then device, need to wait for device to prepare next buffer for read
		OSW_TaskSleep(20);
		//lock bus
    	 	OSSchedLock();        

		SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0,(UINT8*)&i2c_rx_buf[0], bytes_to_read);
		//unlock bus
  		OSSchedUnlock();// ca test             
    	
//			if (pDummyMsg->DemodLocked == TRUE ){
//			   	g_bDummyHaveSignal = TRUE;
//			}
		}
		
	

	  //send message to host library
	if ((g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE] != NULL) && (p_msg->xMsgHeader.msgLength < 0x1000) && (p_msg->xMsgHeader.msgDstId != 0))
	{
		g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE](ADR_CTRL_HANDLE, (UINT8* )p_msg, p_msg->xMsgHeader.msgLength);
	} else if ((g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE] != NULL) && (p_msg->xMsgHeader.msgLength < 0x1000) && (p_msg->xMsgHeader.msgType==MSG_SMS_GET_VERSION_EX_RES)){
			g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE](ADR_CTRL_HANDLE, (UINT8* )p_msg, p_msg->xMsgHeader.msgLength);
	}

  //    if ((p_msg->xMsgHeader.msgType == MSG_SMS_NO_SIGNAL_IND) || (p_msg->xMsgHeader.msgType ==MSG_SMS_SIGNAL_DETECTED_IND))
//	  	g_bHaveSignalIndicator = TRUE;
	  	
      if (p_msg->xMsgHeader.msgType == response)
               return 0xffff;

	return p_msg->xMsgHeader.msgLength;
}

#endif



//  RX task
/*
BOOLEAN SmsDataRcv(ROCK_MSG_ID id, void *msg)
{
	BOOLEAN ret = FALSE;
	PSPIDEVICEEXTENSION pSpiDevExt = &gstSpiDeviceExt;
	switch(id)
	{
	case AS_CMMB_RECEIVE:
		{
			if (msg)
			{
	            struct spi_msg txmsg = {0};

                g_writespimsg_count ++;
				txmsg.len = 256;
				txmsg.buf = msg;
                   //lock spi
        OSSchedLock();
        IOMUX_SetSDPort(SD_SPI_TYPE);
        delay_nops(500); 
        GPIO_SetPinLevel(GPIOPortC_Pin5,GPIO_HIGH);// cs spi switch to spi mode
				smsspi_common_transfer_msg(&pSpiDevExt->dev, &txmsg, 1);
       //unlock spi    
        OSSchedUnlock();// ca test                    
				pSpiDevExt->WriteOperation = 1;
				free(msg);
			}
			else
			{
                g_datarcvspiint_count ++;
                   //lock spi
        OSSchedLock();        
            IOMUX_SetSDPort(SD_SPI_TYPE);
            delay_nops(500); 
            GPIO_SetPinLevel(GPIOPortC_Pin5,GPIO_HIGH);// cs spi switch to spi mode
				smsspi_common_transfer_msg(&pSpiDevExt->dev, NULL, pSpiDevExt->PaddingAllowed);
       //unlock spi    
        OSSchedUnlock();// ca test                 
			}
		}
		break;
	default:
		CMMB_CMD_Iterface(id,msg);
		break;
	}
}

*/

#if 0
BOOLEAN SmsDataRcv(ROCK_MSG_ID id, void *msg)
{
	BOOLEAN ret = FALSE;
	SmsMsgData_ST*	p_msg = NULL;
	UINT8*			i2c_rx_buf;
	UINT32			bytes_to_read;
	UINT32			cur_rx_buf=0, i,j;
//	PSPIDEVICEEXTENSION pSpiDevExt = &gstSpiDeviceExt;
	switch(id)
	{
	case AS_CMMB_RECEIVE:
		{
			if (msg)  // TX, need to write to bus
			{
		          p_msg = msg;

             		g_writespimsg_count ++;
				//lock  bus
      				  OSSchedLock();
      
				// send Message
				SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)p_msg, p_msg->xMsgHeader.msgLength);
				//	i2c_write(I2C_SIANO_DEVICE_ADDR, (UINT8*)p_msg, p_msg->xMsgHeader.msgLength);
			       //unlock bus
			        OSSchedUnlock();// ca test                    
			
				free(msg);
			}
			else  // RX, need to read from bus
			{
 		                g_datarcvspiint_count ++;

				// wait for I2C device to be ready
				OSW_MutexGet(&g_ReadWriteMutex);

//		IO_DBG_SerialPrintf_("ADR_I2cRxTask: After Sem Get\n");
				i2c_rx_buf = (UINT8*)(g_rx_buffers[cur_rx_buf]);
				if (!i2c_rx_buf)
				{
	//		IO_DBG_SerialPrintf_("ADR_I2cRxTask: BUFFER ERROR - Exiting\n");
				return;
				}
				//lock bus
      			  	OSSchedLock();        
				// read message header
				SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0, i2c_rx_buf, sizeof(SmsMsgHdr_ST));			
       			//unlock bus
        			OSSchedUnlock();// ca test             
        			
				p_msg = (SmsMsgData_ST*)i2c_rx_buf;

			//	IO_DBG_SerialPrintf_("ADR_I2cRxTask: Read header. Type=%d, Length=%d\n", 
		          //                   pMsg->xMsgHeader.msgType, pMsg->xMsgHeader.msgLength);

		        // check if there is a payload to the message
				if (p_msg->xMsgHeader.msgLength > 8)
				{
					bytes_to_read = p_msg->xMsgHeader.msgLength > 0x1000 ? 0x1000 : p_msg->xMsgHeader.msgLength;
					
					// since host is much faster then device, need to wait for device to prepare next buffer for read
					OSW_TaskSleep(10);
					//lock bus
	      			  	OSSchedLock();        

					SIANO_Demode_dib_tcc_I2C_Read_manual(I2C_SIANO_DEVICE_ADDR<<1,NULL,0,(UINT8*)&i2c_rx_buf[sizeof(SmsMsgHdr_ST)], bytes_to_read - 8);
	       			//unlock bus
      		  			OSSchedUnlock();// ca test             
				}

				// Release device
				OSW_MutexPut(&g_ReadWriteMutex);

				cur_rx_buf++;
				if (cur_rx_buf >= NUM_RX_BUFFERS)
					cur_rx_buf = 0;

		        //send message to host library
				if ((g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE] != NULL) && (p_msg->xMsgHeader.msgLength < 0x1000) && (p_msg->xMsgHeader.msgDstId != 0))
				{
					g_DevData.pfnCtrlCallback[ADR_CTRL_HANDLE](ADR_CTRL_HANDLE, (UINT8* )p_msg, p_msg->xMsgHeader.msgLength);
				}
		            
                
			}
		}
		break;
	default:
		CMMB_CMD_Iterface(id,msg);
		break;
	}
}
#endif

void SmsReadIntr()
{
//    zDebugTimePrintf("Interrupt\n"); //调试用
//  receive  an interrupt  to trigger SPI read thread, or I2C read thread
//	RockOSSendMsg(CMMB_DEMUX_TASK, AS_CMMB_RECEIVE, NULL);

}

#if SPI_INTERFACE

///////////////////////////////////////////////////////////////////////////////
/*!
	Stream Driver Initialization.

	\param[in]	e_comm_type	Communication type
	\param[in]	p_params	Driver dependent extra parameters

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
/*
SMSHOSTLIB_ERR_CODES_E ADR_Init(SMSHOSTLIB_COMM_TYPES_E e_comm_type)
{
	MY_FILE *fp;
	size_t len,len2;
	unsigned char *buf;
	SMSHOSTLIB_ERR_CODES_E ret = SMSHOSTLIB_ERR_COMM_ERR;
// peter add
// physical  bus initialzie 
//	SIANO_COM_SPI_Init();

	fp = FSFileOpen(SIANO_FW_NAME, "rb");
	if (!fp)		goto exit;

	len = FSFileGetSize(fp);
	if(len>100*1024)		goto exit;

	buf = (void*)malloc(len);
	if(!buf) 		goto exit;

	len2 = FSFileRead((void*)buf, len, fp);
	if(len!=len2)		goto exit;


	GPIO_DisableIntr(GPIOPortF_Pin2);

	if(download_firmware(buf, len))
		ret = SMSHOSTLIB_ERR_OK;
exit:
	if(fp)	FSFileClose(fp);
	if(buf)	free(buf);
	return ret;


}
*/
#endif

#if I2C_TS_INTERFACE
///////////////////////////////////////////////////////////////////////////////
/*!
	Stream Driver Initialization.

	\param[in]	e_comm_type	Communication type
	\param[in]	p_params	Driver dependent extra parameters

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code.
*/
SMSHOSTLIB_ERR_CODES_E ADR_Init(SMSHOSTLIB_COMM_TYPES_E e_comm_type)
{
	UINT32 rc;

	if (g_AdrInit == SMS_TRUE)
	{
		return SMSHOSTLIB_ERR_UNDEFINED_ERR;
	}

	/* check args */
	if (e_comm_type != SMSHOSTLIB_COMM_I2C)
	{
		return SMSHOSTLIB_ERR_INVALID_ARG;
	}

// create RX task
	memset ((UINT8*)RX_RAM_BUFF, 0, 0x500);
	g_rx_buffers[0] =(UINT8*) &RX_RAM_BUFF[0x0000];
	g_rx_buffers[1] =(UINT8*) &RX_RAM_BUFF[0x1000];
	g_rx_buffers[2] =(UINT8*) &RX_RAM_BUFF[0x2000];
	g_rx_buffers[3] =(UINT8*) &RX_RAM_BUFF[0x3000];
	g_rx_buffers[4] =(UINT8*) &RX_RAM_BUFF[0x4000];
	

	
//	IO_DBG_SerialPrintf_("Before gp_I2CRxTask = OSW_TaskCreate()\n");
/*
	gp_I2CRxTask = OSW_TaskCreate(	"SmsI2CRxTask",	// Task name
									6,				// Task priority
									0x200,			// Task stack
									(TaskFunc) ADR_I2cRxTask,	// Task function
									0);				// Task creation parameter
	if (gp_I2CRxTask == NULL)
	{
		OSW_MutexDelete (&g_ReadWriteMutex);
//		IO_DBG_SerialPrintf_("SMSHOSTLIB_ERR_TASK_CREATION_FAILED\n");
		return SMSHOSTLIB_ERR_TASK_CREATION_FAILED;
	}
*/
	// Register ISR - Need tp be updated according to specific platform
	// register_isr(ADR_I2cRxIsr, [isr_pin], EDGE_DETECT)

	// enable the data interrupt
//	GPIO_SetIntrType(GPIOPortF_Pin2, IntrTypeRisingEdge);
//	GPIO_RegIntrCallback(SmsReadIntr, GPIOPortF_Pin2);
//	GPIO_EnableIntr(GPIOPortF_Pin2);
	



	g_AdrInit		= SMS_TRUE;

//	IO_DBG_SerialPrintf_("ADR_Init- EXIT ()\n");

	return SMSHOSTLIB_ERR_OK;
}


#endif


#if I2C_TS_INTERFACE
//////////////////////////////////////////////////////////////////////////////////////////////////
// Device Init
SMSHOSTLIB_ERR_CODES_E ADR_DeviceInit(SMSHOSTLIB_DEVICE_MODES_E	DeviceMode )
{
	//MY_FILE*	pFwFile;
        struct file* pFwFile; //jan

	UINT8*	pFwImage;
	UINT32	fwFileSize;
	UINT32	RetCode;
	UINT32  i ; 
	static BOOL bInsert;
	
	bInsert =FALSE;
	//printk("[sms] ADR_DeviceInit download fw start =%d",OSTimeGet());//jan
//	IO_DBG_SerialPrintf_("ADR_DeviceInit()\n");

//	IO_DBG_SerialPrintf_("ADR_DeviceInit PHASE 2()\n");

	///////////////////////////////
	// first thing we need to download the firmware
	//////////////////////////////
#ifdef SMS_FILE_SYSTEM //Nucleus doesn't have file system for TCC
	switch (DeviceMode)
	{
	case SMSHOSTLIB_DEVMD_DVBT:
		pFwFile = FSFileOpen(FW_FILE_DVB_T, "rb");
		break;

	case SMSHOSTLIB_DEVMD_ISDBT:
		pFwFile = FSFileOpen(FW_FILE_ISDB_T, "rb");
		break;
	default:
		return SMSHOSTLIB_ERR_INVALID_ARG;
	}

	if (!pFwFile)
		return SMSHOSTLIB_ERR_FS_ERROR;

	// allocate buffer for FW image;
	pFwImage = OSW_MemAlloc(104*1024);
	fwFileSize = FSFileRead((void *)pFwImage, 104*1024, pFwFile);
	FSFileClose(pFwFile);
#else
	pFwImage = (UINT8 *)Siano1130FW;//fw_image;
	fwFileSize=sizeof(Siano1130FW);
#endif	

/// test before downloading

	 	// second thing we need to do is set the interrupt gpio
//	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)&g_GetVerSmsMsg, sizeof(SmsMsgData_ST));

    // OSW_TaskSleep(20);


	//i2c_read();
	
	//OSW_TaskSleep(20);
        msleep(20); //jan
        //mdelay(20); //jan
//////////////////////////////////////////////////////	

	// wait for I2C device to be ready

	// IO_DBG_SerialPrintf_("Before write of FW header. Size = %d, Ptr = %x \n", fwFileSize, pFwImage);
	//write FW to device


#if 0 // need to activate only for SMS1130 B0

//	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)pFwImage, 12,DVB_DEMODE_DIBCOM_I2C_RATE_K*3);
	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)pFwImage, 12,DVB_DEMODE_DIBCOM_I2C_RATE_K*3);
	OSW_TaskSleep(10);
    //Pull reset pin down
//    Siano_ResetControl(FALSE);
	DVB_PowerDeinit();
    //Pull reset pin back up
//    Siano_ResetControl(TRUE);
	DVB_PowerOnInit();
    OSW_TaskSleep(10);

	//c_read();
    SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)pFwImage, 11,DVB_DEMODE_DIBCOM_I2C_RATE_K*3);
    OSW_TaskSleep(10);
#else
        //while(1) //test
        //{
        DBG("TSTV: download siano1130 firmware start\t%s[%d]\n", __FUNCTION__, __LINE__);
	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)pFwImage, 11,DVB_SINAO_TUNER_FW_DOWN_CLK/*zzf DVB_DEMODE_DIBCOM_I2C_RATE_K*3*/);
	//OSW_TaskSleep(10);
        msleep(10); //jan
        //} 
        //mdelay(10); //jan
#endif

//    IO_DBG_SerialPrintf_("Before write of FW payload. Size = %d, Ptr = %x \n", fwFileSize - 12, pFwImage+12);
      SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)(pFwImage+12), (fwFileSize - 12),DVB_SINAO_TUNER_FW_DOWN_CLK/*DVB_DEMODE_DIBCOM_I2C_RATE_K*3*/);
      DBG("TSTV: download siano1130 firmware end, (firmware size - 12) = %d\t%s[%d]\n", __FUNCTION__, __LINE__, fwFileSize - 12);

#ifdef SMS_FILE_SYSTEM //Nucleus doesn't have file system for TCC
	OSW_MemFree(pFwImage);
#endif

//	IO_DBG_SerialPrintf_("Done with FW...\n");

	//OSW_TaskSleep(100);
        msleep(100); //jan
        //mdelay(100); //jan

	memset (TX_CMD_BUFF, 0xff, 256);
	memcpy (TX_CMD_BUFF, (UINT8*)&g_GpioMsg,sizeof(SmsIntLine_ST) );
	// second thing we need to do is set the interrupt gpio
	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)&g_GpioMsg, sizeof(SmsIntLine_ST),DVB_DEMODE_DIBCOM_I2C_RATE_K);
	//SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)TX_CMD_BUFF, 256);        
        for ( i =0 ; i < 5 ; i ++)
     	{
//     		OSW_TaskSleep(50);
     		//OSW_TaskSleep(10);//zzf
                //while(1) //test
                //{
                //    DBG("###################################test##################################\t%s[%d]\n", __FUNCTION__, __LINE__);
                //    msleep(5);
                //    DBG("TSTV: i2c_read return 0x%4x\n", i2c_read(MSG_SMS_SPI_INT_LINE_SET_RES));
                //}
                DBG("###################################try##################################\t%s[%d]\n", __FUNCTION__, __LINE__);
                msleep(10); //jan
                //mdelay(10); //jan
		if (i2c_read(MSG_SMS_SPI_INT_LINE_SET_RES) == 0xffff)
			break;
     	}
        if(i == 5)
        {
            bInsert = TRUE;
            DBG("TSTV: i2c read timeout\t%s[%d]\n", __FUNCTION__, __LINE__);
            return (SMSHOSTLIB_ERR_OK + 1);
        }

	DBG("TSTV: i2c_read to check\t%s[%d]\n", __FUNCTION__, __LINE__);
#if 0	
	{
		int i=0;


		OSW_TaskSleep(20);

		memset (TX_CMD_BUFF, 0xff, 256);
		memcpy (TX_CMD_BUFF, (UINT8*)&g_TsEnableMsg,sizeof(SmsTsEnable_ST) );
		// third and last thing - send the TS interface enable command.
		SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)&g_TsEnableMsg, sizeof(SmsTsEnable_ST),DVB_DEMODE_DIBCOM_I2C_RATE_K);
		//SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)TX_CMD_BUFF, 256);
		OSW_TaskSleep(200);	


		for(i=0;i<20000;i++)
		{
			OSW_TaskSleep(220);
			ISDB_Siano_Get_Very_TS(5000);

			OSW_TaskSleep(220);

			
			// second thing we need to do is set the interrupt gpio
			SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)&g_GetVerSmsMsg, sizeof(SmsMsgData_ST),DVB_DEMODE_DIBCOM_I2C_RATE_K);
			//		SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)TX_CMD_BUFF, 256);

			 OSW_TaskSleep(20);


			i2c_read();

			OSW_TaskSleep(20);
			
		}
	}
#endif
	// second thing we need to do is set the interrupt gpio
	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)&g_GetVerSmsMsg, sizeof(SmsMsgData_ST),DVB_DEMODE_DIBCOM_I2C_RATE_K);
	//	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)TX_CMD_BUFF, 256);

        for ( i =0 ; i < 5 ; i ++)
     	{
     		//OSW_TaskSleep(50);
                DBG("###################################try##################################\t%s[%d]\n", __FUNCTION__, __LINE__);
                msleep(50); //jan
                //mdelay(50); //jan

		if (i2c_read(MSG_SMS_GET_VERSION_EX_RES) == 0xffff)
		{
			bInsert =TRUE;
			break;
		}
     	}
        if(i == 5)
        {
            bInsert = TRUE;
            DBG("TSTV: i2c read timeout\t%s[%d]\n", __FUNCTION__, __LINE__);
            return (SMSHOSTLIB_ERR_OK + 1);
        }

	memset (TX_CMD_BUFF, 0xff, 256);
	memcpy (TX_CMD_BUFF, (UINT8*)&g_TsEnableMsg,sizeof(SmsTsEnable_ST) );
	// third and last thing - send the TS interface enable command.
	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)&g_TsEnableMsg, sizeof(SmsTsEnable_ST),DVB_DEMODE_DIBCOM_I2C_RATE_K);
//SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)TX_CMD_BUFF, 256);
	//OSW_TaskSleep(200);
        msleep(200); //jan
        //mdelay(200); //jan
        for ( i =0 ; i < 5 ; i ++)
     	{
//     		OSW_TaskSleep(50);
     		//OSW_TaskSleep(10); // zzf
                DBG("###################################try##################################\t%s[%d]\n", __FUNCTION__, __LINE__);
                msleep(10); //jan
                //mdelay(10); //jan

		if (i2c_read(MSG_SMS_ENBALE_TS_INTERFACE_RES) == 0xffff)
			break;
     	}
        if(i == 5)
        {
            bInsert = TRUE;
            DBG("TSTV: i2c read timeout\t%s[%d]\n", __FUNCTION__, __LINE__);
            return (SMSHOSTLIB_ERR_OK + 1);
        }

#if 0 // disable by zzf
	// second thing we need to do is set the interrupt gpio
	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)&g_GetVerSmsMsg, sizeof(SmsMsgData_ST),DVB_DEMODE_DIBCOM_I2C_RATE_K);
	//	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)TX_CMD_BUFF, 256);

     for ( i =0 ; i < 5 ; i ++)
     	{
     		OSW_TaskSleep(50);


		if (i2c_read(MSG_SMS_GET_VERSION_EX_RES) == 0xffff)
			break;
     	}
#endif
	// Release device
	//OSW_TaskSleep(100);

//	IO_DBG_SerialPrintf_("ADR_DeviceInit -EXIT()\n");


//	SmsLiteIsdbtTune_Req(473143000,BW_6_MHZ,1);
    //printk("[sms] ADR_DeviceInit download fw end =%d",OSTimeGet());
    return SMSHOSTLIB_ERR_OK;
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////

static SMSHOSTLIB_ERR_CODES_E ADR_OpenHandle( UINT32 handle_num ,   SmsLiteAdr_pfnFuncCb pfnControlCb, 
									  SmsLiteAdr_pfnFuncCb pfnDataCb)
{
	long InitialId = handle_num ? handle_num : SMS_HOST_LIB;
	UINT32 buffSize;
	UINT32 buffNum;

	if (g_AdrInit == FALSE)
	{
		return SMSHOSTLIB_ERR_NOT_INITIALIZED;
	}

	if (handle_num >= ADR_NUM_OF_HANDLES)
	{
		return SMSHOSTLIB_ERR_INVALID_ARG;
	}


	g_DevData.pfnCtrlCallback[handle_num] = pfnControlCb;
	g_DevData.pfnDataCallback[handle_num] = pfnDataCb;
	

	g_DevData.OpenHandles++;

	return SMSHOSTLIB_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E ADR_CloseHandle(	UINT32 handle_num )
{
	if (g_AdrInit == FALSE)
	{
		return SMSHOSTLIB_ERR_NOT_INITIALIZED;
	}

	if (handle_num >= ADR_NUM_OF_HANDLES)
	{
		return SMSHOSTLIB_ERR_INVALID_ARG;
	}

	if (g_DevData.OpenHandles == 0)
	{
		return SMSHOSTLIB_ERR_OK;
	}

	g_DevData.pfnCtrlCallback[handle_num] = NULL;
	g_DevData.pfnDataCallback[handle_num] = NULL;


	g_DevData.OpenHandles--;

	return SMSHOSTLIB_ERR_OK;
}


/////////////////////////////////////////


SMSHOSTLIB_ERR_CODES_E ADR_DeviceTerminate()
{
	UINT32 i;
	SMSHOSTLIB_ERR_CODES_E RetCode;
	SMSHOSTLIB_ERR_CODES_E FinalRetCode = SMSHOSTLIB_ERR_OK;
	if ( !g_DevData.IsDeviceFound )
	{
		return SMSHOSTLIB_ERR_DEVICE_DOES_NOT_EXIST;
	}

	for ( i = 0 ; i < ADR_NUM_OF_HANDLES ; i++ )
	{

		{
			RetCode = ADR_CloseHandle( i );
			if ( RetCode != SMSHOSTLIB_ERR_OK )
			{
				FinalRetCode = RetCode;
			}
		}
	}

      g_AdrInit = FALSE ;
	return SMSHOSTLIB_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
/*!
	Stream Driver Close.

	\param[in]	DeviceHandle	Handle to the ADR device.
	\param[in]	handle_num		Handle number generated by from control library. 0 for control library.

	\return		#SMSHOSTLIB_ERR_CODES_E	Return code
*/

/*
#if I2C_TS_INTERFACE

///////////////////////////////////////////////////////////////////////////////////////////////////
// Send message to device
SMSHOSTLIB_ERR_CODES_E ADR_WriteMsg(void*			DeviceHandle,
									SmsMsgData_ST*	p_msg )
{
    OSW_TaskSleep(10);

	// wait for I2C device to be ready
	OSW_MutexGet(&g_ReadWriteMutex);

	// send Message
	SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)p_msg, p_msg->xMsgHeader.msgLength);
//	i2c_write(I2C_SIANO_DEVICE_ADDR, (UINT8*)p_msg, p_msg->xMsgHeader.msgLength);

	// Release device
	OSW_MutexPut(&g_ReadWriteMutex);

    return SMSHOSTLIB_ERR_OK;
}

#endif
*/

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E SmsLiteAdrInit( SMSHOSTLIB_DEVICE_MODES_E DeviceMode, 
									  SmsLiteAdr_pfnFuncCb pfnControlCb, 
									  SmsLiteAdr_pfnFuncCb pfnDataCb )
{
	SMSHOSTLIB_ERR_CODES_E Ret = SMSHOSTLIB_ERR_OK;
	UINT32 NumHandlesToOpen = ADR_NUM_OF_HANDLES;

	UINT32 i;
	
	if (g_AdrInit == TRUE)
	{
		return SMSHOSTLIB_ERR_ALREADY_INITIALIZED;
	}

	ZERO_MEM_OBJ( &g_DevData );
	g_DevData.DeviceMode = DeviceMode;

//	g_AdrInit=TRUE; 

// create  I2c Rx task
	ADR_Init(SMSHOSTLIB_COMM_I2C);
	//if (!smshwInitialize(GetModuleHandle(NULL), SmsHwPnpCallback))
	{
//		Ret = SMSHOSTLIB_ERR_UNDEFINED_ERR;
	}

	g_DevData.DeviceMode = DeviceMode;
	
	for ( i = 0 ; i < NumHandlesToOpen ; i++ )
	{
		Ret = ADR_OpenHandle( i, pfnControlCb, pfnDataCb);
		if ( Ret != SMSHOSTLIB_ERR_OK )
		{
			while( i > 0 )
			{
				ADR_CloseHandle(i);
				i--;
			}
			return Ret;
		}
	}
	
	g_DevData.IsDeviceFound = TRUE;

// 	///////////////////////////////
// first thing we need to download the firmware, init device	
	 Ret = ADR_DeviceInit( g_DevData.DeviceMode );
		
	if (Ret == SMSHOSTLIB_ERR_OK)
	{
		g_AdrInit = TRUE;
	}

	return Ret;
}

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E SmsLiteAdrTerminate( void )
{
	if (g_AdrInit == FALSE)
	{
		return SMSHOSTLIB_ERR_NOT_INITIALIZED;
	}

//	smshwTerminate();

	if ( g_DevData.IsDeviceFound )
	{
		ADR_DeviceTerminate();
	}

	g_AdrInit = FALSE;

	ZERO_MEM_OBJ( &g_DevData );

	return SMSHOSTLIB_ERR_OK;
}
///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E SmsLiteAdrWriteMsg( SmsMsgData_ST*	p_msg )
{
	UINT32 i =0;
	UINT8    tx_msg[256] ={0};
	if (g_AdrInit == FALSE)
	{
		return SMSHOSTLIB_ERR_NOT_INITIALIZED;
	}

	if ( !g_DevData.IsDeviceFound )
	{
		return SMSHOSTLIB_ERR_DEVICE_DOES_NOT_EXIST;
	}

//	if (!smshwSendMessageRequest(g_DevData.AdrHandleList[0].hDevice, p_msg, p_msg->xMsgHeader.msgLength))

     memcpy (tx_msg, (UINT8*)p_msg, p_msg->xMsgHeader.msgLength);

    if (p_msg)
	
	{
//		i2c_send(p_msg);
//		SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)p_msg, sizeof(SmsMsgData_ST),DVB_DEMODE_DIBCOM_I2C_RATE_K);
//		SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,(UINT8*)p_msg, p_msg->xMsgHeader.msgLength,DVB_DEMODE_DIBCOM_I2C_RATE_K);
//		p_msg = (SmsMsgData_ST*)tx_msg;
		SIANO_Demode_dib_tcc_I2C_Write_manual(I2C_SIANO_DEVICE_ADDR<<1,tx_msg, /*p_msg->xMsgHeader.msgLength*/256,DVB_DEMODE_DIBCOM_I2C_RATE_K);

		//DelayMs_nops(50);
                mdelay(50); //jan
		
//		return SMSHOSTLIB_ERR_COMM_ERR;
	}
/*
	for ( i =0 ; i < 3 ; i ++)
	{
		if (i2c_read())
			return SMSHOSTLIB_ERR_OK;
		DelayMs_nops(50);
	}

*/	
	return SMSHOSTLIB_ERR_OK;
}



