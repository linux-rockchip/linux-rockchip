#include "DriverFunc.h"
//#include "CMMB_Global.h"

spi_dev_st dev = {0};
// ?global parameter vs local parameter
__aligned(4) UINT8 g_rdbuf[256] = {0};
__aligned(4) UINT8 g_txbuf[256] = {0};
__aligned(4) UINT8 g_rdmsg_buf[4096] = {0};


#define SIANO_SPI_CS	GPIOPortB_Pin7	//GPIOPortF_Pin2
#define SMS_INTR_PIN	19				//16

extern void DataHdl(UINT8 *msg);


#if 0
void spi_csn_init(void)
{
	IOMUX_SetI2CType(I2C_IOPORT);
	GPIO_SetPinLevel(SIANO_SPI_CS, GPIO_HIGH);
	GPIO_SetPinDirection(SIANO_SPI_CS, GPIO_OUT);
}


void spi_csn_low(void)
{
	RockSemObtain(&gHwI2CSem);
    
    IOMUX_SetSDPort(SD_SPI_ALL_TYPE);
    delay_nops(500); 
    GPIO_SetPinLevel(GPIOPortC_Pin5,GPIO_HIGH);// cs spi
    
	IOMUX_SetI2CType(I2C_IOPORT);
	GPIO_SetPinLevel(SIANO_SPI_CS, GPIO_LOW);
	GPIO_SetPinDirection(SIANO_SPI_CS, GPIO_OUT);
}

void spi_csn_high(void)
{
	IOMUX_SetI2CType(I2C_IOPORT);
	//DelayMs_nops(1);//需要延时，保证数据传输完成了
	GPIO_SetPinLevel(SIANO_SPI_CS, GPIO_HIGH);
	GPIO_SetPinDirection(SIANO_SPI_CS, GPIO_OUT);
	RockSemRelease(&gHwI2CSem);
}


int spi_send_and_wait(SmsMsgHdr_ST *msg)
{
	UINT16 rc;

	memcpy(g_txbuf, smsspi_preamble, sizeof(smsspi_preamble));

	spi_csn_low();
	rc = SPI_Write_Read(g_txbuf, 256, g_rdbuf, 256);
	spi_csn_high();

	DelayMs_nops(3);

	spi_csn_low();
	rc = SPI_Write_Read(0, 0, g_rdbuf, 256);
	spi_csn_high();

	return (rc==0) ? -1 : rc;
}



int spi_send(SmsMsgHdr_ST *msg)
{
	UINT8 *buf = malloc(256);
	if(msg)
	{
		memcpy(buf, smsspi_preamble, sizeof(smsspi_preamble));
		memcpy(buf+sizeof(smsspi_preamble), msg, msg->msgLength);
// peter , close it temporarily for compiling	
//		RockOSSendMsg(CMMB_DEMUX_TASK, AS_CMMB_RECEIVE, buf);
		return 1;
	}
	return 0;
}


extern void SmsReadIntr();

UINT32 StatisticCounter = 0;
BOOL download_firmware(unsigned char *buf, int len)
{
	SmsFirmware_ST *firmware = (SmsFirmware_ST*) buf;
	SmsMsgHdr_ST *msg;

	u32 mem_address = firmware->StartAddress;
	u8 *payload = firmware->Payload;
	int rc = 0;

	msg = (SmsMsgHdr_ST*)(g_txbuf+sizeof(smsspi_preamble));
	OSSchedLock( );
	Scu_ClockEnable(SPI_CLOCK);
	IOMUX_SetSDPort(SD_SPI_ALL_TYPE);
	spi_csn_init();
	SPI_PowerOnInit();


	DelayMs_nops(10);

	spi_csn_low();
	SPI_Write_Read(smsspi_startup, sizeof(smsspi_startup), g_rdbuf, sizeof(smsspi_startup));
	spi_csn_high();
	DelayMs_nops(10);

	// get version
	SMS_INIT_MSG(msg, MSG_SMS_GET_VERSION_EX_REQ, (u16)(sizeof(SmsMsgHdr_ST)));
	rc = spi_send_and_wait(msg);
	DelayMs_nops(10);

	// down firmware
	len -= 12;
	while (len && rc >= 0) {
		SmsDataDownload_ST *DataMsg = (SmsDataDownload_ST*) msg;
		int payload_size = min((int) len, SMS_MAX_PAYLOAD_SIZE);

		SMS_INIT_MSG(msg, MSG_SMS_DATA_DOWNLOAD_REQ,
			     (u16)(sizeof(SmsMsgHdr_ST) + sizeof(u32) + payload_size));

		DataMsg->MemAddr = mem_address;
		memcpy(DataMsg->Payload, payload, payload_size);

		rc = spi_send_and_wait(msg);
		payload += payload_size;
		len -= payload_size;
		mem_address += payload_size;
	}

	// trigger the new firmware
	if (rc >= 0) {
		SmsMsgData_ST *TriggerMsg = (SmsMsgData_ST *) msg;

		SMS_INIT_MSG(msg, MSG_SMS_SWDOWNLOAD_TRIGGER_REQ,
			     (u16)(sizeof(SmsMsgHdr_ST) + (sizeof(u32)*5)));

		TriggerMsg->msgData[0] = firmware->StartAddress;	// Entry point
		TriggerMsg->msgData[1] = 5; 		// Priority
		TriggerMsg->msgData[2] = 0x200; 	// Stack size
		TriggerMsg->msgData[3] = 0; 		// Parameter
		TriggerMsg->msgData[4] = 4; 		// Task ID

		rc = spi_send_and_wait(msg);
	}
	DelayMs_nops(100);

	// change the spi interrupt line
	{
		SmsMsgData_ST *IntrMsg = (SmsMsgData_ST *)msg;
		SMS_INIT_MSG(msg, MSG_SMS_SPI_INT_LINE_SET_REQ, sizeof(SmsMsgHdr_ST)+12 );
		IntrMsg->msgData[0] = 0;
		IntrMsg->msgData[1] = SMS_INTR_PIN;
		IntrMsg->msgData[2] = 100;
		rc = spi_send_and_wait(msg);
		DelayMs_nops(100);
	}


	// get version again
	SMS_INIT_MSG(msg, MSG_SMS_GET_VERSION_EX_REQ, sizeof(SmsMsgHdr_ST) );
	rc = spi_send_and_wait(msg);
	DelayMs_nops(100);

	// init the device
	{
		SmsMsgData_ST *InitMsg = (SmsMsgData_ST *) msg;
		SMS_INIT_MSG(msg, MSG_SMS_INIT_DEVICE_REQ, sizeof(SmsMsgHdr_ST) + (sizeof(UINT32) * 1));
		InitMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB_INTERNAL;
		InitMsg->xMsgHeader.msgDstId = HIF_TASK;
		InitMsg->xMsgHeader.msgFlags = 0;
		InitMsg->xMsgHeader.msgType  = MSG_SMS_INIT_DEVICE_REQ;
		InitMsg->xMsgHeader.msgLength = (UINT16)sizeof(SmsMsgHdr_ST) + (sizeof(UINT32) * 1);
		InitMsg->msgData[0] = SMSHOSTLIB_DEVMD_CMMB;
		rc = spi_send_and_wait(msg);
	}

	// enable the data interrupt
	GPIO_SetIntrType(GPIOPortF_Pin2, IntrTypeRisingEdge);
	GPIO_RegIntrCallback(SmsReadIntr, GPIOPortF_Pin2);
	GPIO_EnableIntr(GPIOPortF_Pin2);

	OSSchedUnlock( );
	return rc;


}

// full -duplex working.
void smsspibus_xfer(void* context,unsigned char* txbuf,unsigned long nuused1,unsigned char* rxbuf,unsigned long nuused2,int len)
{

	OSSchedLock( );
	spi_csn_low();
	SPI_Write_Read(txbuf, (txbuf ? len : 0), rxbuf, (rxbuf ? len : 0));
	spi_csn_high();
	OSSchedUnlock( );

	return ;
}



void MsgFound(void* context, void* buf, int offset, int len)
{
/*
PSPIDEVICEEXTENSION pSpiDevExt = context;
DBGMSG(ZONE_INFO, (TEXT("SmsSpi: MsgFound().\r\n")));
if (pSpiDevExt->pBD_ReadCBFunc)
{
    DBGMSG(ZONE_INFO, (TEXT("SmsSpi: Calling back with message at offset %d: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, \r\n"),
                                                                                            offset,
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[0],
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[1],
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[2],
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[3],
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[4],
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[5],
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[6],
                                                                                (char*)((char*)(((rx_buffer_st*)buf)->ptr) + offset)[7]));
    pSpiDevExt->pBD_ReadCBFunc(pSpiDevExt->hBDContext, (char*)(((rx_buffer_st*)buf)->ptr) + offset, (DWORD)len);

*/
	PSPIDEVICEEXTENSION pSpiDevExt = context;
	UINT32 Msg_len = (UINT32) (((rx_buffer_st*)buf)->ptr) + offset;
	if (Msg_len%2)
	{
        memcpy((UINT8*)g_rdmsg_buf, (UINT8*)(((rx_buffer_st*)buf)->ptr) + offset, len);
	    DataHdl((UINT8*)g_rdmsg_buf);
	}
	else
    	DataHdl((UINT8*)(((rx_buffer_st*)buf)->ptr) + offset);
}

#endif

///////////////////////////////////////////////i2c//////////////////////////////////////////////////
/*
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
	//I2C_Init((unsigned char)0xD0,I2C_Rate_NPM_K); //jan
	//result =I2C_Write((unsigned char)0xc0,(UINT8 *)p_msg,len,DirectMode);//jan
	//I2C_Deinit();//jan	
#endif
}
*/
































