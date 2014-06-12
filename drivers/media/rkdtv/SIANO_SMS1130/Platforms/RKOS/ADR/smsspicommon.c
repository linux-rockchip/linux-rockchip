//#include "IO_TCC7XX.h"
#include <linux/string.h>
#include "smsspicommon.h"
//#include "../../../common/include/smsDbgLogs.h"
//#include "SianoSPI.h"

#define SMS_MIN(a,b)    (((a)<(b))?(a):(b))

static int smsspi_common_find_msg(struct spi_dev* dev, rx_buffer_st * buf, int offset, int len, int *unused_bytes)
{
	int	i, missing_bytes;
    int recieved_bytes, padded_msg_len;
	int align_fix;
    char* ptr = (char*)buf->ptr + offset;

	//PRN_DBG((TXT( "entering with %d bytes.\n"), len));
	missing_bytes = 0;
	for(i=0;i<len;i++,ptr++)
	{
		switch(dev->rxState)
		{
			case RxsWait_a5:
                dev->rxState = ((*ptr & 0xff) == 0xa5) ? RxsWait_5a : RxsWait_a5;
				dev->rxPacket.msg_offset = (unsigned long)ptr - (unsigned long)buf->ptr + 4;
    			break;
			case RxsWait_5a:
                dev->rxState = ((*ptr & 0xff) == 0x5a) ? RxsWait_e7 : RxsWait_a5;
				//PRN_DBG((TXT( "state %d.\n"), dev->rxState));
                break;
			case RxsWait_e7:
                dev->rxState = ((*ptr & 0xff) == 0xe7) ? RxsWait_7e : RxsWait_a5;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
                break;
			case RxsWait_7e:
                dev->rxState = ((*ptr & 0xff) == 0x7e) ? RxsTypeH : RxsWait_a5;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
                break;
			case RxsTypeH:
                dev->rxPacket.msg_buf = buf;
                dev->rxPacket.msg_offset = (unsigned long)ptr - (unsigned long)buf->ptr;
				dev->rxState = RxsTypeL;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
			break;
			case RxsTypeL:
				dev->rxState = RxsGetSrcId;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
			break;
			case RxsGetSrcId:
				dev->rxState = RxsGetDstId;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
			break;
			case RxsGetDstId:
				dev->rxState = RxsGetLenL;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
			break;
			case RxsGetLenL:
				dev->rxState = RxsGetLenH;
				dev->rxPacket.msg_len = (*ptr & 0xff);
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
			break;
			case RxsGetLenH:
				dev->rxState = RxsFlagsL;
				dev->rxPacket.msg_len += (*ptr & 0xff) <<8;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
				break;
			case RxsFlagsL:
				dev->rxState = RxsFlagsH;
				dev->rxPacket.msg_flags = (*ptr & 0xff);
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
				break;
			case RxsFlagsH:
				dev->rxState = RxsData;
				dev->rxPacket.msg_flags += (*ptr & 0xff) <<8;
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
				break;
			case RxsData:
                recieved_bytes = len + offset - dev->rxPacket.msg_offset;
				padded_msg_len = ((dev->rxPacket.msg_len + 4 + SPI_PACKET_SIZE - 1) >> SPI_PACKET_SIZE_BITS) << SPI_PACKET_SIZE_BITS;
                if (recieved_bytes < padded_msg_len)
                {
                    *unused_bytes = 0;
                    return (padded_msg_len - recieved_bytes);
                }
				dev->rxState = RxsWait_a5;
// Peter，spi state machine处理完当前的256*n bytes buffer里的data message之后，然后需要继续处理剩余的数据，需要把状态机重设成RxsWait_a5
				if (dev->cb.msg_found_cb)
                {
					if (dev->rxPacket.msg_flags & MSG_HDR_FLAG_SPLIT_MSG_HDR)
					{
						align_fix = (dev->rxPacket.msg_flags >>8) & 0x3;
						if (align_fix)
						{ //The FW alligned the message data therefore - alligment bytes should be thrown away.
							// throw the aligment bytes. by moving the header ahead over the alligment bytes.
							ptr = (char*)dev->rxPacket.msg_buf->ptr + dev->rxPacket.msg_offset;
							for (i = MSG_HDR_LEN - 1; i >= 0; i--)
							{
								ptr[i+align_fix] = ptr[i];
							}
							dev->rxPacket.msg_offset += align_fix;
						}
					}
					//PRN_DBG((TXT( "Msg found and sent to callback function.\n")));
					dev->cb.msg_found_cb(dev->context, dev->rxPacket.msg_buf, dev->rxPacket.msg_offset, dev->rxPacket.msg_len);
                    *unused_bytes = len + offset - dev->rxPacket.msg_offset - dev->rxPacket.msg_len;
                    if (*unused_bytes == 0)
                        *unused_bytes = -1;
                    return 0;
                }
                else
                {
                    //PRN_DBG((TXT( "Msg found but no callback. therefore - thrown away.\n")));
                }
				//PRN_DBG((TXT ( "state %d.\n"), dev->rxState));
			break;
		}
// Peter，spi state machine处理完当前的256*n bytes buffer里的data message之后，然后需要继续处理剩余的数据，而不是直接读取下一个256 bytes.
	}
// Peter, please pay attention to the following fixs
	if (dev->rxState == RxsWait_a5)
	{
		*unused_bytes = 0;
		return 0;
	}
	else
	{
		// Workaround to corner case: if the last byte of the buffer is "a5" (first byte of the preamble),
		// the host thinks it should send another 256 bytes.  In case the a5 is the firmware underflow byte,
		// this will cause an infinite loop, so we check for this case explicity.
		// peter ,在某种情况下，siano 1180 没有数据送出，这时host 发命令给siano 1180 ,siano SPI会发送重复fifo里的随机数，比如a5, a5, a5
		//这样会导致spi state machine误入不正常的状态，误以为还需要接受 siano 1180的spi message, 这种极端case可能会在在fw download的时候出现
		if ((dev->rxState == RxsWait_5a) && (*(ptr-2)==0xa5))
		{
			dev->rxState = RxsWait_a5;
			*unused_bytes = 0;
			return 0;
		}


		if (dev->rxPacket.msg_offset >= (SPI_PACKET_SIZE + 4)) // adding 4 for the preamble.
		{	/*The packet will be copied to a new buffer and rescaned by the state machine*/
//peter 这个if出现的情况是，当收完一个长的data message的后，spi state machine 继续处理4k中的剩余数据时，可能会发现下一个spi message的preamble,或者部分message header.
//spi state machine需要，针对这个情况进行处理；下面的做法是需要把这部分数据重新align并重新进状态机，*unused_bytes表示回退需要rescan的数据长度。
// 这个方法考虑到zero cpy的好处。
			*unused_bytes = dev->rxState - RxsWait_a5;
			dev->rxState = RxsWait_a5;
		    dev->cb.free_rx_buf(dev->context, buf);
			return 0;
		}
		else
//  peter 这个else，出现的情况是 第一次spi state machine里所读到的256 bytes里只有 部分 spi message的preamble； 0xa55ae77e 被截断在两个256 bytes的buffer里面，
//这种情况是完全有可能发生的，请注意这个case
// 解决方法是 这个做法是，直接再多读一次256 bytes,把preamble收全
		{  /*report missing bytes and continue with message scan.*/

			*unused_bytes = 0;
			return SPI_PACKET_SIZE;
		}
	}
}

void smsspi_common_transfer_msg(struct spi_dev* dev, struct spi_msg* txmsg, int padding_allowed)
{
	int len, bytes_to_transfer;
	unsigned long tx_phy_addr;
	int missing_bytes, tx_bytes;
	int offset, unused_bytes;
	int align_block;
	char *txbuf;
    rx_buffer_st *buf, *tmp_buf;

	len = 0;
	if (!dev->cb.transfer_data_cb)
	{
		//PRN_ERR((TXT("function called while module is not initialized.\n")));
		return;
	}
	if (txmsg == 0)
	{
		bytes_to_transfer = SPI_PACKET_SIZE;
		txbuf = 0;
		tx_phy_addr = 0;
		tx_bytes = 0;
	}
	else
	{
		tx_bytes = txmsg->len;
		if (padding_allowed)
			bytes_to_transfer = (((tx_bytes + SPI_PACKET_SIZE - 1) >> SPI_PACKET_SIZE_BITS) << SPI_PACKET_SIZE_BITS);
		else
			bytes_to_transfer = (((tx_bytes + 3) >>2) <<2);
		txbuf = txmsg->buf;
		tx_phy_addr = txmsg->buf_phy_addr;
	}
    offset = 0;
	unused_bytes = 0;
    buf = dev->cb.allocate_rx_buf(dev->context, RX_PACKET_SIZE + SPI_PACKET_SIZE);
    if (!buf)
    {
        //PRN_ERR((TXT( "Failed to allocate RX buffer.\n")));
        return;
    }
	while (bytes_to_transfer || unused_bytes)
	{
		if ((unused_bytes <= 0) && (bytes_to_transfer > 0))
		{
			len = SMS_MIN (bytes_to_transfer, RX_PACKET_SIZE);
			//PRN_DBG((TXT( "transfering block of %d bytes\n"), len));
			dev->cb.transfer_data_cb(dev->phy_context, (unsigned char*)txbuf, tx_phy_addr,
			        (unsigned char*)((char*)buf->ptr + offset), buf->phy_addr + offset, len);
		}
		missing_bytes = smsspi_common_find_msg(dev, buf, offset, len, &unused_bytes);
		if (bytes_to_transfer)
		{
			bytes_to_transfer -= len;
		}
		if (tx_bytes)
		{
			tx_bytes -= len;
		}
		if (missing_bytes)
		{
			offset += len;
		}
        if (unused_bytes)
        {
            tmp_buf = dev->cb.allocate_rx_buf(dev->context, RX_PACKET_SIZE);
            if (!tmp_buf)
            {
                //PRN_ERR((TXT( "Failed to allocate RX buffer.\n")));
                return;
            }
			if (unused_bytes > 0)
			{ // Copy the remaining bytes to the end of alingment block (256 bytes) so next read will be alligned.
				align_block = (((unused_bytes + SPI_PACKET_SIZE - 1) >>  SPI_PACKET_SIZE_BITS) << SPI_PACKET_SIZE_BITS);
				memset(tmp_buf->ptr, 0, align_block - unused_bytes);
				memcpy((char*)tmp_buf->ptr + (align_block - unused_bytes), (char*)buf->ptr + offset + len - unused_bytes, unused_bytes);
				len = align_block;
			}
            offset = 0;
            buf = tmp_buf;
        }
		if (tx_bytes <= 0)
		{
			txbuf = 0;
			tx_bytes = 0;
		}
		if (bytes_to_transfer < missing_bytes)
		{
			bytes_to_transfer = (((missing_bytes + SPI_PACKET_SIZE - 1) >> SPI_PACKET_SIZE_BITS) << SPI_PACKET_SIZE_BITS);
			//PRN_DBG((TXT("a message was found, adding bytes to transfer, txmsg %d, total %d\n"), tx_bytes, bytes_to_transfer));
		}
	}
    dev->cb.free_rx_buf(dev->context, buf);
}



int smsspicommon_init(struct spi_dev* dev, void* context, void* phy_context, spi_dev_cb_st* cb)
{
	//PRN_DBG((TXT("entering.\n" )));
	if (cb->transfer_data_cb == 0 ||
		cb->msg_found_cb == 0 ||
		cb->allocate_rx_buf == 0 ||
		cb->free_rx_buf == 0)
	{
		//PRN_ERR((TXT("Invalid input parameters of init routine.\n")));
		return -1;
	}
	dev->context = context;
	dev->phy_context = phy_context;
    memcpy (&dev->cb, cb, sizeof(spi_dev_cb_st));
	dev->rxState = RxsWait_a5;
	//PRN_DBG((TXT("exiting.\n")));
	return 0;
}
