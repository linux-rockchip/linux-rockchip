
#include "sms_common.h"


////////////////////////////////////////////////////////////////////////////////////////
//
// TS Processor function
//
// This function is called from the TS H/W driver when a block of TS packets is received
// src_len must be a multiple of 188 bytes
//

#define TS_SMS_MAX_MSG_LEN		4096
#define TS_NUM_OF_BUFFERS		4

#define MSG_HDR_FLAG_SPLIT_MSG_HDR  0x0004  
#define MSG_HDR_FLAG_TS_PAYLOAD		0x0008	


UINT32 g_ts_remainder_bytes = 0;
UINT32 g_ts_payload_is_ts = 0;
UINT8  g_ts_reassembly_buf[TS_NUM_OF_BUFFERS][TS_SMS_MAX_MSG_LEN];
UINT32 g_ts_current_reassembly_buf;
UINT32 g_ts_remainder_bytes;
UINT32 g_ts_bytes_in_msg;

// Upper-layer function prototype
void handle_completed_msg(SmsMsgData_ST* pMsg);


void ts_data_processor(UINT8* src_buf, UINT32 src_len)
{
	UINT8   siano_preamble[8] = {0x53, 0x69, 0x61, 0x6E, 0x6F, 0x2D, 0x4D, 0x53}; 
	UINT8*  current_ptr;
	UINT8*	payload_ptr;
	UINT32  bytes_to_copy;
	UINT16  ts_pid;
	SmsMsgData_ST* pMsg;
	UINT32  isSplit;

	current_ptr = src_buf;

	while (current_ptr < src_buf + src_len)
	{
		if (g_ts_remainder_bytes > 0)
		{
			if (g_ts_payload_is_ts)
			{
				// TS payload, keep header
				bytes_to_copy = SMS_MIN(188, g_ts_remainder_bytes);
				payload_ptr = current_ptr;
			}
			else
			{
				// Non-TS payload, strip header
				bytes_to_copy = SMS_MIN(184, g_ts_remainder_bytes);
				payload_ptr = current_ptr+4;
			}
			memcpy(&g_ts_reassembly_buf[g_ts_current_reassembly_buf][g_ts_bytes_in_msg], payload_ptr, bytes_to_copy);
			g_ts_bytes_in_msg += bytes_to_copy;
			g_ts_remainder_bytes -= bytes_to_copy;
		}
		else
		{
			//check for beginning of SMS message
			ts_pid = (current_ptr[1] << 8) + current_ptr[2];
			g_ts_bytes_in_msg = 0;
			if ((ts_pid == 0x1FFF) && (!memcmp(&current_ptr[4], siano_preamble, 8)))
			{
				pMsg = (SmsMsgData_ST*)(void*)&current_ptr[12];
				g_ts_remainder_bytes = pMsg->xMsgHeader.msgLength;

				isSplit				= (pMsg->xMsgHeader.msgFlags & MSG_HDR_FLAG_SPLIT_MSG_HDR);
				g_ts_payload_is_ts  = (pMsg->xMsgHeader.msgFlags & MSG_HDR_FLAG_TS_PAYLOAD);

				if (isSplit)
				{
					bytes_to_copy = sizeof(SmsMsgHdr_ST);
				}
				else
				{
					bytes_to_copy = SMS_MIN(176, g_ts_remainder_bytes);
				}

				memcpy(&g_ts_reassembly_buf[g_ts_current_reassembly_buf][0], pMsg, bytes_to_copy);
				g_ts_remainder_bytes -= bytes_to_copy;
				g_ts_bytes_in_msg = bytes_to_copy;
			}
		}

		if ((g_ts_bytes_in_msg > 0) && (g_ts_remainder_bytes == 0))
		{
			pMsg = (SmsMsgData_ST*)(void*)g_ts_reassembly_buf[g_ts_current_reassembly_buf];
			handle_completed_msg(pMsg);
			g_ts_bytes_in_msg = 0;
			g_ts_current_reassembly_buf++;
			if (g_ts_current_reassembly_buf >= TS_NUM_OF_BUFFERS) 
			{
				g_ts_current_reassembly_buf = 0;
			}
		}

		current_ptr += 188;		// Advance to next PID packet
	}
}



//////////////////////////////////////////////////////////////////////////
//
// TS enable message format
//
//typedef struct SmsTsEnable_S
//{
//	SmsMsgHdr_ST	xMsgHeader;
//	UINT32			TsClock;                // TS Clock Speed in Hz
//	UINT32			eTsiMode;               // TS Mode of operation Serial (on SDIO or HIF Pins), or Parallel
//	UINT32		    eTsiSignals;            // Level of Valid, Sync and Error signals when active
//	UINT32			nTsiPcktDelay;          // number of delay bytes between TS packets (for 204bytes mode set to 16)
//	UINT32		    eTsClockPolarity;		// Clock edge to sample data
//	UINT32		    TsBitOrder;				// Bit order in TS output
//	UINT32          EnableControlOverTs;    // Enable Control messages over TS interface
//	UINT32          TsiEncapsulationFormat; // TS encapsulation method
//	UINT32			TsiPaddingPackets;		// Number of TS padding packets appended to control messages
//} SmsTsEnable_ST;
//
//SmsTsEnable_ST g_TsEnableMsg = {{MSG_SMS_ENBALE_TS_INTERFACE_REQ, 0, 11, sizeof(SmsTsEnable_ST), MSG_HDR_FLAG_STATIC_MSG}, 
//								TSI_CLOCK_RATE_FAST, 
//								TSI_SERIAL_ON_SDIO,
//								TSI_SIGNALS_ACTIVE_HIGH,
//								0,
//								TSI_SIG_OUT_FALL_EDGE,
//								TSI_BIT0_IS_MSB,
//								1,						// Enable control over TS
//								TSI_ENCAPSULATED,		// Enable encapsulated data
//								21};					// Pad each control message to 21*188 bytes
//
//
