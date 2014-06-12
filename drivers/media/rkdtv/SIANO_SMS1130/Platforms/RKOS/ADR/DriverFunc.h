
#ifndef _DRIVER_FUNC_H
#define _DRIVER_FUNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../include_internal/Include/sms_common.h"
#include "smsspicommon.h"
//#include "../../../../../system/gui/guictrl/list.h"


#define SMS_MAX_PAYLOAD_SIZE				240

#define u32 		INT32U
#define u16		INT16U
#define u8 		INT8U

#define min(a, b)  (((a) < (b)) ? (a) : (b))

#define SMS_INIT_MSG_EX(ptr, type, src, dst, len) do { \
	(ptr)->msgType = type; (ptr)->msgSrcId = src; (ptr)->msgDstId = dst; \
	(ptr)->msgLength = len; (ptr)->msgFlags = 0; \
} while (0)
#define SMS_INIT_MSG(ptr, type, len) \
	SMS_INIT_MSG_EX(ptr, type, 0, HIF_TASK, len)

// peter


//#define SPI_PACKET_SIZE	256
//#define RX_PACKET_SIZE	0x1000

#define NUM_OF_RX_PACKES	100//48//24//4
#define SMM100_HDR_SIZE	8
#define SPI_PREAMBLE 0x7EE75AA5//0xA55AE77E//0xA5, 0x5A, 0xE7, 0x7E,//Preamble in little endian
#define SPI_PREAMBLE_REVERSE 0xA55AE77E
#define SPI_PREAMBLE_SIZE 4


#define MAX_REG_PATH_SIZE 100
										//adapter//Buff//Len

typedef struct _SPIDEVICEEXTENSION
{

	struct spi_dev		dev;
	void (*Callback)(char * buf, int len) ;
	BOOL 				PaddingAllowed;
	rx_buffer_st        rxbuf;
//	char                txbuf[TX_BUF_SIZE];
	char                *txbuf;
	unsigned int			WriteOperation;
	UINT32		            pBD_ReadCBFunc;
    //BD_ReadCBFunc       pBD_ReadCBFunc;
	BOOL				novaDevice;
	unsigned int					hBDContext;
}SPIDEVICEEXTENSION, *PSPIDEVICEEXTENSION;


//static char preamble_pattern[] = { 0xa5, 0x5a, 0xe7, 0x7e };
static UINT8 smsspi_startup[] = { 0, 0, 0xde, 0xc1, 0xa5, 0x51, 0xf1, 0xed };
static UINT8 smsspi_preamble[] = { 0xa5, 0x5a, 0xe7, 0x7e };


// peter
typedef struct SmsFirmware_ST {
	UINT32			CheckSum;
	UINT32			Length;
	UINT32			StartAddress;
	UINT8			Payload[1];
} SmsFirmware_ST;


typedef struct SmsDataDownload_ST {
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			MemAddr;
	UINT8			Payload[SMS_MAX_PAYLOAD_SIZE];
}SmsDataDownload_ST;


typedef struct _listitem {
	UINT16 len;
	UINT8 data[4096];
}listitem;


void spi_csn_init(void);
void spi_csn_low(void);
void spi_csn_high(void);
int spi_send_and_wait(SmsMsgHdr_ST *msg);
BOOL download_firmware(unsigned char *buf, int len);
int find_msg();
int spi_send();

void smsspibus_xfer(void* context,unsigned char* txbuf,unsigned long nuused1,unsigned char* rxbuf,unsigned long nuused2,int len);

void MsgFound(void* context, void* buf, int offset, int len);

////////////////////////////////////////////////i2c///////////////////////////////////////////////
int i2c_send(SmsMsgData_ST * p_msg);

#ifdef __cplusplus
}
#endif

#endif // _DRIVER_FUNC_H

