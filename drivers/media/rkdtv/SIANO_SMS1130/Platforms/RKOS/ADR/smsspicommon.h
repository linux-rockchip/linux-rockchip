#ifndef _SIANOSPICOMMON_H_
#define _SIANOSPICOMMON_H_

#define RX_PACKET_SIZE  		0x1000
#define SPI_PACKET_SIZE_BITS	8
#define SPI_PACKET_SIZE 		(1<<SPI_PACKET_SIZE_BITS)
#define SPI_MAX_CTRL_MSG_SIZE	0x100

#define MSG_HDR_FLAG_SPLIT_MSG_HDR  0x0004
#define MSG_HDR_LEN					8

#define RX_BUF_SIZE	0x1200
#define TX_BUF_SIZE	0x200


typedef enum _spi_rx_state{
	RxsWait_a5=0,
	RxsWait_5a,
	RxsWait_e7,
	RxsWait_7e,
	RxsTypeH,
	RxsTypeL,
	RxsGetSrcId,
	RxsGetDstId,
	RxsGetLenL,
	RxsGetLenH,
	RxsFlagsL,
	RxsFlagsH,
	RxsData
}spi_rx_state;


typedef struct
{
	void* 			ptr;
	unsigned long 	phy_addr;
}rx_buffer_st;
typedef struct _rx_packet_request
{
	rx_buffer_st* 	msg_buf;
	int				msg_offset;
	int				msg_len;
	int				msg_flags;
}rx_packet_request;

typedef struct
{
    void  (*transfer_data_cb)(void* context, unsigned char*, unsigned long, unsigned char*, unsigned long, int);
    void (*msg_found_cb)(void*, void*, int, int);
    rx_buffer_st* (*allocate_rx_buf)(void*, int);
    void (*free_rx_buf)(void*, rx_buffer_st*);
}spi_dev_cb_st;

typedef struct spi_dev{
	void*				context;
	void*				phy_context;
    spi_dev_cb_st       cb;
    char*               rx_buf;
	spi_rx_state  		rxState;
	rx_packet_request	rxPacket;
	char*				internal_tx_buf;
}spi_dev_st;

typedef struct spi_msg{
	char* 			buf;
	unsigned long	buf_phy_addr;
	int 			len;
}spi_msg_st;

void smsspi_common_transfer_msg(struct spi_dev* dev, struct spi_msg* txmsg, int padding_allowed);
int smsspicommon_init(struct spi_dev* dev, void* contex, void* phy_context, spi_dev_cb_st* cb);
#endif /*_SIANOSPICOMMON_H_*/
