#ifndef SMS_FIRMWARE_API_H_
#define SMS_FIRMWARE_API_H_

#define SMS_TYPES_H		// For firmware compilation, to avoid duplication with sms_types_rom.h


#define SMS_MIN(a,b)    (((a)<(b))?(a):(b))

#define SMS_MAX(a,b)    (((a)>(b))?(a):(b))

enum Bool_E
{
	SMS_FALSE = 0,
	SMS_TRUE  = 1
};
typedef enum Bool_E Bool_ET;


typedef struct SmsMsgHdr_S
{
	UINT16 	msgType;
	UINT8	msgSrcId;
	UINT8	msgDstId;
	UINT16	msgLength;	// Length is of the entire message, including header
	UINT16	msgFlags;
} SmsMsgHdr_ST;


typedef struct SmsMessage_S
{
	SmsMsgHdr_ST	xMsgHeader;
	void*	pMsg;
} SmsMessage_ST;

typedef struct SmsMsgData_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			msgData[1];
} SmsMsgData_ST;

typedef struct SmsMsgData2Args_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			msgData[2];
} SmsMsgData2Args_ST;


typedef struct SmsMsgData3Args_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			msgData[3];
} SmsMsgData3Args_ST;

typedef struct
{
	SmsMsgHdr_ST xMsgHeader;
	//Statistics parameters
	UINT32 DemodLocked;
	UINT32 InBandPwr;
	UINT32 BER;
	UINT32 SNR;
	UINT32 TotalTsPackets;
	UINT32 ErrorTSPackets;
} DummyStatMsg_ST;


// Definitions for the flags field in the message header
// bits 13,14,15 of msgFlags allocated to SmsCommIntf
// The definition is on a per-bit basis
#define MSG_HDR_DEFAULT_DYNAMIC_MSG	0x0000	// Message is dynamic 
#define MSG_HDR_FLAG_STATIC_MSG		0x0001	// Message is dynamic when this bit is '0'
#define MSG_HDR_FLAG_RX_DIR			0x0002	// Direction is RX when this bit is '1'
#define MSG_HDR_FLAG_SPLIT_MSG_HDR  0x0004  // Message format is SmsMessage_ST, with pMsg pointing to remainder of message
#define MSG_HDR_FLAG_TS_PAYLOAD		0x0008	// Message payload is in the form of TS packets
#define MSG_HDR_FLAG_ALIGN_MASK     0x0300	// Two bits denoting number of padding bytes inserted at the 
// start of the data in split messages. Used for alignment
#define MSG_HDR_FLAG_EXT_LEN_HDR	0xF000  // Extended msg len (MS nibble).


#define	SMS_CMMB_NETWORK_LEVEL_USE_CURRENT					(0xCBCBCBCB)


/*------------------------------**
** Base Error and success codes **
**------------------------------*/

// The standard return value that each function must return
// See sm_results.h for standard return values
typedef UINT32 SmsResult;


/* general success */
#define SMS_S_OK 	((SmsResult)0x00000000)

/* general error */
#define SMS_E_FAIL	((SmsResult)0x80000000)

/*-------------------------------------------------**
** General Macros to test for fail/success result. **
**-------------------------------------------------*/
#define SMS_FAILED(x) 	    ((x) & SMS_E_FAIL)
#define SMS_SUCCEEDED(x) 	(!((x) & SMS_E_FAIL))

#define MALLOC_ERR				(SMS_E_FAIL | 0x80000)
#define FREE_ERR				(SMS_E_FAIL | 0x90000)


/*---------------------**
** general Success codes **
**---------------------*/

#define SMS_S_PENDING              (SMS_S_OK | 1) /* successful but uncompleted asynchronous call.*/
#define SMS_S_EMPTY                (SMS_S_OK | 2) 

/*---------------------**
** general Error codes **
**---------------------*/

#define SMS_E_FATAL              (SMS_E_FAIL | 1)
#define SMS_E_OUT_OF_MEM         (SMS_E_FAIL | 2)
#define SMS_E_BAD_PARAMS         (SMS_E_FAIL | 3)
#define SMS_E_NOT_FOUND          (SMS_E_FAIL | 4)
#define SMS_E_MAX_EXCEEDED       (SMS_E_FAIL | 5)
#define SMS_E_OUT_OF_RANGE       (SMS_E_FAIL | 6)
#define SMS_E_NOT_INIT           (SMS_E_FAIL | 7)
#define SMS_E_FULL               (SMS_E_FAIL | 8)
#define SMS_E_EMPTY              (SMS_E_FAIL | 9)
#define SMS_E_INVALID_SIZE       (SMS_E_FAIL | 10)
#define SMS_E_INVALID_OPERATION  (SMS_E_FAIL | 11)
#define SMS_E_TIMEOUT            (SMS_E_FAIL | 12)
#define SMS_E_NULL_PTR           (SMS_E_FAIL | 13)
#define SMS_E_BUSY               (SMS_E_FAIL | 14)
#define SMS_E_ALREADY_INIT       (SMS_E_FAIL | 15)
#define SMS_E_NACK			     (SMS_E_FAIL | 16)
#define SMS_E_ALREADY_EXISTING   (SMS_E_FAIL | 17)

#define SMS_MAX_SERVICE_HANDLE	(16)

/********************************************************
SMS Host Library IDs
*********************************************************/
#define SMS_HOST_ID_BASE	100
#define SMS_HOST_LIB				(SMS_HOST_ID_BASE + 50)
#define SMS_HOST_LIB_INTERNAL		(SMS_HOST_ID_BASE + 51)
#define SMS_HOST_LIB_INTERNAL2		(SMS_HOST_ID_BASE + 52)

#define HIF_TASK				11		// Firmware messages processor task IS
#define HIF_TASK_SLAVE			22


typedef enum
{
	FW_LOG_SEVERITY_NONE,
	FW_LOG_SEVERITY_ERROR,
	FW_LOG_SEVERITY_INFO,
	FW_LOG_SEVERITY_DEBUG,
} LogSeverity_ET;

typedef enum LogMode_E
{
	LOG_TYPE_DISABLED,
	LOG_TYPE_ENABLED, //old log mode
	LOG_TYPE_EXTENDED,  //new log mode (many logs in a single message)

}LogMode_ET;



// Definitions of the message types.
// For each type, the format used (excluding the header) is specified
// The message direction is also specified
typedef enum MsgTypes_E
{
	MSG_TYPE_BASE_VAL = 500,

	MSG_SMS_SET_RF_CTRL_REQ = 501,	// Set the RF Control
	// Format: 32-bit - PGA , Coarse 1 (dB)
	//		   32-bit -		  Coarse 2 (dB)
	//		   32-bit -		  Fine (dB)
	//		   32-bit - Mixer
	//		   32-bit - Mixer Offset
	//		   32-bit - LNA
	//		   32-bit - IQICGain
	//		   32-bit - IQICPhase
	// Direction: Host->SMS

	MSG_SMS_SET_RF_CTRL_RES = 502,	// The response to MSG_SMS_SET_RF_CTRL_REQ
	// Format:	32-bit - status
	// Direction: SMS->Host

	MSG_SMS_GET_VERSION_REQ = 503,	// Get version
	// Format: None
	// Direction: Host->SMS

	MSG_SMS_GET_VERSION_RES = 504,	// The response to MSG_SMS_GET_VERSION_REQ
	// Format:	8-bit - Version string
	// Direction: SMS->Host

	MSG_SMS_START_MFER_REQ = 505,	// Start MFER & FER accumulation
	// Format: 32-bit - PID
	//		   32-bit -	Num of rows in MPE Table
	//		   32-bit -	Max Good Tables

	MSG_SMS_START_MFER_RES = 506,	// The response to MSG_SMS_START_MFER_REQ
	// Format: 32-bit - FER
	//		   32-bit -	MFER
	//		   32-bit -	Max Good Tables
	// Direction: SMS->Host

	MSG_SMS_GPIO_CONFIG_REQ = 507,

	MSG_SMS_GPIO_CONFIG_RES = 508,

	MSG_SMS_GPIO_SET_LEVEL_REQ = 509,

	MSG_SMS_GPIO_SET_LEVEL_RES = 510,

	MSG_SMS_GPIO_GET_LEVEL_REQ = 511,

	MSG_SMS_GPIO_GET_LEVEL_RES = 512,

	MSG_SMS_COMM_STATE_CHANGED_IND = 513,	// Notification to host that the current communication state has changed
	// Format: 32-bit - Msg Status
	//		   32-bit -	comm state
	// Direction: HL driver->Host

	MSG_SMS_LOG_ENABLE_CHANGE_REQ = 514,// Change the state of (enable/disable) log messages flow from SMS to Host (MSG_SMS_LOG_ITEM)
	// Format: 32-bit address value for g_log_enable
	// Direction: Host->SMS

	MSG_SMS_LOG_ENABLE_CHANGE_RES = 515,// A reply to MSG_SMS_LOG_ENABLE_CHANGE_REQ
	// Format: 32-bit address value for g_log_enable
	// Direction: SMS->Host

	MSG_SMS_SET_MAX_TX_MSG_LEN_REQ = 516,	// Set the maximum length of a receiver message
	// Format: 32-bit value of length in bytes, must be modulo of 4
	MSG_SMS_SET_MAX_TX_MSG_LEN_RES = 517,	// ACK/ERR for MSG_SMS_SET_MAX_TX_MSG_LEN_REQ

	MSG_DEBUG = 527,			// Obsolete - replaced by MSG_SMS_LOG_ITEM
	// Carries general debug information
	// Format: The data is carried immediately after the message header
	// Direction: N/A (from host to SMS and vice versa)
	MSG_DEBUG_RSSI_IND = 528,	// Indication of an RSSI measurement by the modem
	// Format: The RSSI measurement is immediately following the header
	// Direction: SMS->Host
	MSG_WR_REG_DEMOD_REQ =529,		// Request to write a value to a given modem register
	// Format: Register address is 32-bit, following header
	//		   Register value is 32-bit, following address
	// Direction: Host->SMS
	MSG_WR_REG_DEMOD_RES =530,	// A reply to a MSG_WR_REG_DEMOD_REQ message
	// Format: Result of write operation status
	// Direction: SMS->Host
	MSG_RD_REG_DEMOD_REQ =531,	// Request to read the value of a given register
	// Format: 32-bit address of the register, following header
	// Direction: Host->SMS
	MSG_RD_REG_DEMOD_RES =532,	// Response to MSG_RD_REG_DEMOD_REQ message
	// Format: 32-bit value of the register, following header
	// Direction: SMS->Host
	MSG_WR_REG_RFT_REQ   =533,	// Write value to a given RFT register
	// Format: 32-bit address of register, following header
	//		   32-bit of value, following address
	// Direction: Host->SMS
	MSG_WR_REG_RFT_RES   =534,	// Response to MSG_WR_REG_RFT_REQ message
	// Format: Status of write operation, following header
	// Direction: SMS->Host
	MSG_RD_REG_RFT_REQ   =535,	// Read the value of a given RFT register
	// Format: 32-bit address of the register, following header
	// Direction: Host->SMS
	MSG_RD_REG_RFT_RES   =536,	// Response to MSG_RD_REG_RFT_RES message
	// Format: 32-bit value of register, following header
	// Direction: SMS->Host
	MSG_RD_REG_ALL_RFT_REQ=537,	// Read all 16 RFT registers
	// Format: N/A (nothing after the header)
	// Direction: Host->SMS
	MSG_RD_REG_ALL_RFT_RES=538,	// Response to MSG_RD_REG_ALL_RFT_REQ message
	// Format: For each register, 32-bit address followed by 32-bit value (following header)
	// Direction: SMS->Host
	MSG_HELP_INT          =539,	// Internal (SmsMonitor) message
	// Format: N/A (nothing after header)
	// Direction: Host->Host
	MSG_RUN_SCRIPT_INT   =540,	// Internal (SmsMonitor) message
	// Format: Name of script(file) to run, immediately following header
	// direction: N/A
	MSG_SMS_SW_DL_MODE_REQ=541,	// Instruct the SMS to enter/leave SW download mode
	// Format: Same format as MSG_SMS_SW_DL_REQ with no data
	// Direction: Host->SMS
	MSG_SMS_SW_DL_MODE_RES=542, // Response to MSG_SMS_SW_DL_MODE_REQ message
	// Format: 32-bit status of MSG_SMS_SW_DL_MODE_REQ
	// Direction: SMS->Host
	MSG_SMS_SW_DL_REQ     =543,	// SW download of data to the SMS
	// Format: 32-bit - Counter
	//		   32-bit - Crc
	//		   32-bit - Total image size
	//		   32-bit - Packet len
	//		   32-bit - Dest addr
	//		   List of 8-bit - Image data
	// Direction: Host->SMS
	MSG_SMS_SW_DL_RES     =544,	// Response to MSG_SMS_SW_DL_REQ message
	// Format: Status of write operation (following header)
	// Direction: SMS->Host
	MSG_SMS_SW_DL_WITH_ADDR_REQ=545,// Obsolete !
	// SW download of data to the SMS with a specified address
	// Format: 32-bit address (following header) followed by the data itself
	// Direction: Host->SMS
	MSG_SMS_SW_DL_WITH_ADDR_RES=546,// Obsolete !
	// Response to MSG_SMS_SW_DL_WITH_ADDR_REQ message
	// Format: Status of write operation (following header)
	// Direction: SMS->Host
	MSG_SMS_STAT_MODE_REQ =547,	// Sets the status of the statistical monitoring task
	// Format: 32-bit boolean (1-start or 0-stop) following header
	//		   32-bit of reporting interval (in ms)
	// Direction: Host->SMS
	MSG_SMS_STAT_MODE_RES =548,	// Obsolete
	//
	//
	MSG_SMS_STAT_MEAS_REQ =549,	// Obsolete
	//
	//
	MSG_SMS_STAT_MEAS_RES =550,	// Obsolete
	//
	//
	MSG_SMS_STAT_MEAS_IND =551,	// Obsolete

	MSG_SMS_RD_MEM_REQ    =552, // A request to read address in memory
	// Format: 32-bit of address, followed by 32-bit of range (following header)
	// Direction: Host->SMS
	MSG_SMS_RD_MEM_RES    =553,	// The response to MSG_SMS_RD_MEM_REQ
	// Format: 32-bit of data X range, following header
	// Direction: SMS->Host
	MSG_SMS_WR_MEM_REQ    =554,	// A request to write data to memory
	// Format:	32-bit of address
	//			32-bit of range (in bytes)
	//			32-bit of value
	// Direction: Host->SMS
	MSG_SMS_WR_MEM_RES    =555,	// Response to MSG_SMS_WR_MEM_REQ
	// Format: 32-bit of result
	// Direction: SMS->Host
	MSG_SMS_RD_DEMOD_PARAMS_REQ=556,// A request to read demodulator and DLU parameters
	// Format: Nothing other than message header
	// Direction: Host->SMS
	MSG_SMS_RD_DEMOD_PARAMS_RES=557,// The response to MSG_SMS_RD_DEMOD_PARAMS_REQ
	// Format:	32-bit - Transmission mode
	//			32-bit - Guard Interval
	//			32-bit - Code Rate
	//			32-bit - Bandwidth
	//			32-bit - In-depth interleaver
	//			32-bit - Hierarchy
	//			32-bit - Modem State
	//			32-bit - Burst Size (DLU)
	//			32-bit - Number of rows (DLU)
	//			32-bit - Number of padding columns
	//			32-bit - Number of puncturing columns
	//			32-bit - Burst duration
	//			32-bit - Burst cycle time
	//			32-bit - Burst bandwidth
	//			32-bit - Average bit rate
	// Direction: SMS->Host
	MSG_SMS_RD_DEMOD_PARAMS_IND=558,// Indication from SMS for RFT related data
	// Format: Same as MSG_SMS_RD_DEMOD_PARAMS_RES
	MSG_SMS_WR_DEMOD_PARAMS_REQ=559,// A request to force some of the demodulator parameters
	// Format:	32-bit - Transmission mode (2K, 4K or 8K)
	//			32-bit - Guard Interval (1/n, where n is the integer 4, 8, 16, 32)
	//			32-bit - Code Rate (1/n, where n is the integer 2, ...)
	//			32-bit - Bandwidth (n MHz)
	// For each 32-bit, the MSB indicates whether this value is valid (1) or not (0)
	// Direction: Host->SMS
	MSG_SMS_WR_DEMOD_PARAMS_RES=560,// A response to MSG_SMS_WR_DEMOD_PARAMS_REQ
	// Format: 32-bit value of the response (following header)
	// Direction: SMS->Host
	MSG_SMS_RF_TUNE_REQ=561,	// A request to tune to a new frequency
	// Format:	32-bit - Frequency
	//			32-bit - Bandwidth
	//			32-bit - Crystal
	// Direction: Host->SMS
	MSG_SMS_RF_TUNE_RES=562,    // A response to MSG_SMS_RF_TUNE_REQ
	// Format: 32-bit response, following header
	// Direction: SMS->Host
	MSG_SMS_RF_SET_TUNE_DIV_REQ=563,// A request to set the crystal and dividers
	// Format:	32-bit - M (for tuning)
	//			32-bit - SD (sigma-delta for tuning)
	//			32-bit - Crystal value, in Hz
	//			32-bit - CLKDIV value
	//			32-bit - REFDIV value
	//			32-bit - Band
	//			32-bit - BW
	// Direction: Host->SMS
	MSG_SMS_RF_SET_TUNE_DIV_RES=564,// The response to MSG_SMS_RF_SET_CLK_DIV_REQ
	// Format: 32-bit of response (1 - success indicates lock, 0-failure)
	// Direction: SMS->Host
	MSG_SMS_RF_SET_ALL_REGS_FUNC_REQ=565,	// A request to set all (see below) RF functionality in a single message
	// Format:	32-bit - Tune frequency
	//			32-bit - LNA gain
	//			32-bit - Mixer coarse
	//			32-bit - Mixer fine
	//			32-bit - PGA gain
	//			32-bit - Clk Division
	//			32-bit - Ref Division
	//			32-bit - Band
	//			32-bit - LPF BW
	//			32-bit - RSSI Threshold
	//			32-bit - Power mode
	//			32-bit - Crystal
	//			32-bit - M value (for frequency tune)
	//			32-bit - SD value (for frequency tune)
	// Direction: Host->SMS
	MSG_SMS_RF_SET_ALL_REGS_FUNC_RES=566,// A response to MSG_SMS_RF_SET_ALL_REGS_FUNC_REQ
	// Format:	32-bit status
	// Direction: SMS->Host
	MSG_SMS_RF_RD_ALL_REGS_FUNC_REQ=567,	//
	MSG_SMS_RF_RD_ALL_REGS_FUNC_RES=568,	// An unsolicited message in the format of MSG_SMS_RF_SET_ALL_REGS_FUNC_REQ
	// Format: See MSG_SMS_RF_SET_ALL_REGS_FUNC_REQ
	// Direction: SMS->Host
	MSG_SMS_SET_BYPASS_MODE_REQ=569,	// A request to set a bypass on an internal procedure
	// Format: For each mode, 32-bit of bypass status (1-bypass, 0-don't bypass), using the MSB validation bit
	//			32-bit - PGA bypass
	//			32-bit - RFT startup bypass
	// Direction: Host->SMS
	MSG_SMS_SET_BYPASS_MODE_RES=570,	// The response to MSG_SMS_SET_BYPASS_MODE_REQ
	// Format: The status of the request operation
	// Direction: SMS->Host
	MSG_SMS_EEPROM_WRITE_REQ=571,		// A request to program the EEPROM
	// Format:	32-bit - Section status indication (0-first,running index,0xFFFFFFFF -last)
	//			32-bit - (optional) Image CRC or checksum
	//			32-bit - Total image length, in bytes, immediately following this DWORD
	//			32-bit - Actual section length, in bytes, immediately following this DWORD
	// Direction: Host->SMS
	MSG_SMS_EEPROM_WRITE_RES=572,		// The status response to MSG_SMS_EEPROM_WRITE_REQ
	// Format:	32-bit of the response
	// Direction: SMS->Host
	MSG_SMS_FLASH_DL_ABORT_REQ=573, 	// Indicates that a FLASH programming was aborted
	// Format:	No body
	// Direction: Host->SMS
	MSG_SMS_FLASH_DL_ABORT_RES=574,		// The response to MSG_SMS_FLASH_DL_ABORT_REQ
	// Format:	32-bit of response
	// Direction: SMS->Host
	MSG_SMS_FLASH_ERASE_REQ=575,		// A request to erase the FLASH
	// Format: No body
	// Direction: Host->SMS
	MSG_SMS_FLASH_ERASE_RES=576,		// The response to MSG_SMS_FLASH_ERASE_REQ
	MSG_SMS_HIF_ONLINE_IND=577,			// An indication from HIF that it's "online"
	// Message should be sent once per TBD seconds

	MSG_SMS_INIT_DEVICE_REQ=578,		// A request to init device
	// Format: 32-bit - device mode (DVBT,DVBH,TDMB,DAB)
	//		   32-bit - Crystal
	//		   32-bit - Clk Division
	//		   32-bit - Ref Division
	// Direction: Host->SMS
	MSG_SMS_INIT_DEVICE_RES=579,		// The response to MSG_SMS_INIT_DEVICE_REQ
	// Format:	32-bit - status
	// Direction: SMS->Host

	MSG_SMS_SET_PID_FILTER_REQ=580, 	// Sets a new PID list
	// Format: List of 16-bit PIDs
	// Direction: Host->SMS
	MSG_SMS_SET_PID_FILTER_RES=581,		// The response to MSG_SMS_SET_PID_FILTER_REQ
	// Format:	32-bit - Status
	// Direction: SMS->Host

	MSG_SMS_SET_IP_FILTER_REQ=582,		// Sets a new IPs list
	// Format: concatenated of null terminated strings
	// Direction: Host->SMS
	MSG_SMS_SET_IP_FILTER_RES=583,		// The response to MSG_SMS_IP_FILTER_REQ
	// Format:	32-bit - Status
	// Direction: SMS->Host

	MSG_SMS_SET_STREAM_OUTPUT_STATUS_REQ=584, // Requests to start outputting the TS
	// Format: 32-bit - New status (start TRUE,stop FALSE)
	// Direction: Host->SMS
	MSG_SMS_SET_STREAM_OUTPUT_STATUS_RES=585, // The response to MSG_SMS_SET_STREAM_OUTPUT_STATUS_REQ
	// Format:	32-bit - Status
	// Direction: SMS->Host

	MSG_SMS_SET_MODE_REQ=586,			// Set new device mode (DVBT,DVBH,TDMB,DAB)
	// Format: 32-bit - device mode
	// Direction: Host->SMS
	MSG_SMS_SET_MODE_RES=587,			// The response to MSG_SMS_SET_MODE_REQ
	// Format:	32-bit - status
	// Direction: SMS->Host

	MSG_SMS_WAITING_FOR_SW = 588,		// Response on startup to msgs that not part of dl sw
	// Format:	32-bit - status
	// Direction: SMS->Host

	MSG_SMS_SUB_CHANNEL_START_REQ=589,	//
	//

	MSG_SMS_SUB_CHANNEL_START_RES=590,	//
	//

	MSG_SMS_SUB_CHANNEL_STOP_REQ=591,	//
	//

	MSG_SMS_SUB_CHANNEL_STOP_RES=592,	//
	//

	MSG_SMS_SNR_BER_REQ=593,			// Obsolete
	MSG_SMS_SNR_BER_RES=594,			// Obsolete

	MSG_SMS_SYNCING_REQ=595,			// Obsolete
	MSG_SMS_SYNCING_RES=596,			// Obsolete

	MSG_SMS_DAB_STATISTICS_REQ=597,		// Obsolete
	MSG_SMS_DAB_STATISTICS_RES=598,		// Obsolete
	//

	MSG_SMS_WAIT_CMD=599,				// Internal SmsMonitor wait command

	MSG_SMS_NOKIA_MSG = 600,

	MSG_SMS_ADD_PID_FILTER_REQ=601,		// Add PID to filter list
	// Format: 32-bit PID
	// Direction: Host->SMS
	MSG_SMS_ADD_PID_FILTER_RES=602,			// The response to MSG_SMS_ADD_PID_FILTER_REQ
	// Format:	32-bit - Status
	// Direction: SMS->Host

	MSG_SMS_REMOVE_PID_FILTER_REQ=603,		// Remove PID from filter list
	// Format: 32-bit PID
	// Direction: Host->SMS
	MSG_SMS_REMOVE_PID_FILTER_RES=604,		// The response to MSG_SMS_REMOVE_PID_FILTER_REQ
	// Format:	32-bit - Status
	// Direction: SMS->Host

	MSG_SMS_FAST_INFORMATION_CHANNEL_REQ=605,// A request for a of a Fast Information Channel (FIC)
	MSG_SMS_FAST_INFORMATION_CHANNEL_RES=606,// Forwarding of a Fast Information Channel (FIC)
	// Format:	Sequence counter and FIC bytes with Fast Information Blocks { FIBs  as described in "ETSI EN 300 401 V1.3.3 (2001-05)":5.2.1 Fast Information Block (FIB))
	// Direction: SMS->Host

	MSG_SMS_DAB_CHANNEL=607,			// Forwarding of a played channel
	// Format:	H.264
	// Direction: SMS->Host
	MSG_SMS_GET_PID_FILTER_LIST_REQ=608,// Request to get current PID filter list
	// Format: None
	// Direction: Host->SMS
	MSG_SMS_GET_PID_FILTER_LIST_RES=609,// The response to MSG_SMS_REMOVE_PID_FILTER_LIST_REQ
	// Format:	array of 32-bit of PIDs
	// Direction: SMS->Host

	MSG_SMS_TABLE_DOWNLOAD_REQ=610,	    // Initialization of SMS memory section (table) with const values.
	// Each message holds a single A single segment out of a table.
	// Direction: Host->SMS
	MSG_SMS_TABLE_DOWNLOAD_RES=611,    	// A response for a single table segment transfer
	// Direction: SMS->Host

	MSG_SMS_PRINTF_DISABLE=612, 		// stop sending sms_printf msg's

	MSG_SMS_SET_MPE_TBL_PARAMS_REQ=613,	// Force MPE PID
	// Format:	32-bit - MPE PID
	//			32-bit - Num. of rows in table
	// Direction: Host->SMS
	MSG_SMS_SET_MPE_TBL_PARAMS_RES=614,	// The response to MSG_SMS_SET_MPE_TBL_PARAMS_REQ
	// Format:	32-bit - Status
	// Direction: SMS->Host

	MSG_SMS_GET_STATISTICS_REQ=615,		// Request statistics information
	MSG_SMS_GET_STATISTICS_RES=616,		// The response to MSG_SMS_GET_STATISTICS_REQ
	// Format:	SmsMsgStatisticsInfo_ST
	// Direction: SMS->Host

	MSG_SMS_SEND_DUMP=617,

	MSG_SMS_SCAN_START_REQ=618,			// Start Scan
	// Format:
	//			32-bit - Bandwidth
	//			32-bit - Scan Flags
	//			32-bit - Param Type
	// If Param Type is SCAN_PARAM_TABLE:
	//			32-bit - Number of frequencies N
	//			N*32-bits - List of frequencies
	// If Param Type is SCAN_PARAM_RANGE:
	//			32-bit - Start Frequency
	//			32-bit - Gap between frequencies
	//			32-bit - End Frequency
	// Direction: Host->SMS
	MSG_SMS_SCAN_START_RES=619,			// Scan Start Reply
	// Format:	32-bit - ACK/NACK
	// Direction: SMS->Host
	MSG_SMS_SCAN_STOP_REQ=620,			// Stop Scan
	// Direction: Host->SMS
	MSG_SMS_SCAN_STOP_RES=621,			// Scan Stop Reply
	// Format:	32-bit - ACK/NACK
	// Direction: SMS->Host
	//	MSG_SMS_SCAN_RESULTS_REQ=622,		// Obsolete
	//	MSG_SMS_SCAN_RESULTS_RES=623,		// Obsolete
	MSG_SMS_SCAN_PROGRESS_IND=622,		// Scan progress indications
	// Format:
	//		32-bit Frequency
	//		32-bit boolean - IsFrequencyGood
	//
	MSG_SMS_SCAN_COMPLETE_IND=623,		// Scan completed
	// Format: 32-bit Status

	MSG_SMS_LOG_ITEM        =624,       // Format:	SMSHOSTLIB_LOG_ITEM_ST.
	// Actual size depend on the number of parameters
	// Direction: Host->SMS
	MSG_SMS_ADD_IP_FILTER_REQ = 625,	// Request to add a new IP address for service mapping
	// Format:	32-bit - IP type (IPv4 or IPv6)
	//			32-bit - IP address
	// Direction: Host->SMS
	MSG_SMS_ADD_IP_FILTER_RES = 626,	// Response to Add IP request
	// Format:	32-bit - ACK/NACK

	MSG_SMS_UART_FOR_INFRARED	=627,

	MSG_SMS_DAB_SUBCHANNEL_RECONFIG_REQ = 628,

	MSG_SMS_DAB_SUBCHANNEL_RECONFIG_RES = 629,

	// Handover - start (630)
	MSG_SMS_HO_FIRST_MESSAGE		= 630,
	MSG_SMS_HO_PER_SLICES_IND		= 630,
	MSG_SMS_HO_INBAND_POWER_IND		= 631,
	MSG_SMS_MANUAL_DEMOD_REQ		= 632,
	//MSG_SMS_HO_RESERVED1_RES		= 633,
	//MSG_SMS_HO_RESERVED2_RES		= 634,
	//MSG_SMS_HO_RESERVED3_RES		= 635,
	MSG_SMS_HO_TUNE_ON_REQ			= 636,
	MSG_SMS_HO_TUNE_ON_RES			= 637,
	MSG_SMS_HO_TUNE_OFF_REQ			= 638,
	MSG_SMS_HO_TUNE_OFF_RES			= 639,
	MSG_SMS_HO_PEEK_FREQ_REQ		= 640,
	MSG_SMS_HO_PEEK_FREQ_RES		= 641,
	MSG_SMS_HO_PEEK_FREQ_IND		= 642,
	MSG_SMS_HO_LAST_MESSAGE			= 650,
	// Handover - end (650)

	MSG_SMS_SET_ANTENNA_CONFIG_REQ = 651,				// Request to config antenna gpios
	// Format: SMS_ANTENNA_CONFIG_ST
	// Direction Host->SMS

	MSG_SMS_SET_ANTENNA_CONFIG_RES = 652,				// Reply to MSG_SMS_SET_ANTENNA_CONFIG_REQ
	// Format: 32 bit value: 1- success, 0 - failure
	// Direction SMS->Host

	MSG_SMS_GET_STATISTICS_EX_REQ   = 653,				//
	MSG_SMS_GET_STATISTICS_EX_RES   = 654,				// Format:
	// 32 bit ErrCode
	// The rest: A mode-specific statistics struct starting
	// with a 32 bits type field.
	MSG_SMS_SLEEP_RESUME_COMP_IND	= 655,				// Indicates that a resume from sleep has been completed

	MSG_SMS_DLU_RESUME_CMD_REQ		= 656,				// Indication to the device to resume DLU operation (after suspend)
	MSG_SMS_DLU_RESUME_CMD_RES		= 657,				// Response to resume DLU operation

	MSG_SMS_DATA_DOWNLOAD_REQ		= 660,				// Not supported in Stellar
	MSG_SMS_DATA_DOWNLOAD_RES		= 661,				// Not supported in Stellar

	MSG_SMS_DATA_VALIDITY_REQ		= 662,				// Not supported in Stellar
	MSG_SMS_DATA_VALIDITY_RES		= 663,				// Not supported in Stellar

	MSG_SMS_SWDOWNLOAD_TRIGGER_REQ	= 664,				// Not supported in Stellar
	MSG_SMS_SWDOWNLOAD_TRIGGER_RES	= 665,				// Not supported in Stellar

	MSG_SMS_SWDOWNLOAD_BACKDOOR_REQ	= 666,				// Not supported in Stellar
	MSG_SMS_SWDOWNLOAD_BACKDOOR_RES	= 667,				// Not supported in Stellar

	MSG_SMS_GET_VERSION_EX_REQ		= 668,				// Supported in Nova & Stellar(partially)
	MSG_SMS_GET_VERSION_EX_RES		= 669,				// Supported in Nova & Stellar(partially)

	MSG_SMS_CLOCK_OUTPUT_CONFIG_REQ = 670,				// Request to clock signal output from stellar
	// Format: 32-bit - Enable/Disable clock signal
	//         32-bit - Requested clock frequency
	MSG_SMS_CLOCK_OUTPUT_CONFIG_RES = 671,				// Response to clock signal output config request
	// Format: 32-bit - Status

	MSG_SMS_I2C_SET_FREQ_REQ		= 685,
	MSG_SMS_I2C_SET_FREQ_RES		= 686,

	MSG_SMS_GENERIC_I2C_REQ			= 687,
	MSG_SMS_GENERIC_I2C_RES			= 688,

	MSG_POWER_DOWN_REQ				= 689,
	MSG_POWER_DOWN_RES				= 690,

	MSG_SMS_DMB_POWER_CYCLING_DISABLE_SET_REQ = 691,	// Request to disable/enable Power Cycling in DAB/TDMB
	// Format: 32 bit value: 1 - disable, 0 - enable
	// Direction Host->SMS

	MSG_SMS_DMB_POWER_CYCLING_DISABLE_SET_RES = 692,	// Reply to MSG_SMS_DMB_POWER_CYCLING_DISABLE_SET_REQ
	// Format: 32 bit value: 1- success, 0 - failure
	// Direction SMS->Host
	MSG_SMS_DVBT_BDA_DATA			= 693,

	MSG_SMS_DEMOD_IS_LOCKED_IND		= 694,

	MSG_SMS_KEEP_ALIVE_REQ			= 695,
	MSG_SMS_KEEP_ALIVE_RES			= 696,

	MSG_SW_RELOAD_REQ				= 697,
	MSG_SW_RELOAD_RES				= 698,

	MSG_SMS_DATA_MSG				= 699,

	///  NOTE: Opcodes targeted for Stellar cannot exceed 700
	MSG_TABLE_UPLOAD_REQ			= 700,
	MSG_TABLE_UPLOAD_RES			= 701,

	// reload without reseting the interface
	MSG_SW_RELOAD_START_REQ			= 702,
	MSG_SW_RELOAD_START_RES			= 703,
	MSG_SW_RELOAD_EXEC_REQ			= 704,
	MSG_SW_RELOAD_EXEC_RES			= 705,

	MSG_SMS_SPI_INT_LINE_SET_REQ	= 710,
	MSG_SMS_SPI_INT_LINE_SET_RES	= 711,

	MSG_SMS_GPIO_CONFIG_EX_REQ		= 712,
	MSG_SMS_GPIO_CONFIG_EX_RES		= 713,

	MSG_SMS_CLOCKS_CFG_REQ			= 714,
	MSG_SMS_CLOCKS_CFG_RES			= 715,

	MSG_SMS_WATCHDOG_ACT_REQ		= 716,
	MSG_SMS_WATCHDOG_ACT_RES		= 717,

	MSG_SMS_LOOPBACK_REQ			= 718,
	MSG_SMS_LOOPBACK_RES			= 719,

	MSG_SMS_RAW_CAPTURE_START_REQ	= 720,
	MSG_SMS_RAW_CAPTURE_START_RES	= 721,

	MSG_SMS_RAW_CAPTURE_ABORT_REQ	= 722,
	MSG_SMS_RAW_CAPTURE_ABORT_RES	= 723,

	MSG_SMS_RAW_COUNTER_GET_REQ		= 724,
	MSG_SMS_RAW_COUNTER_GET_RES		= 725,

	MSG_SMS_RAW_AGC_GET_REQ			= 726,
	MSG_SMS_RAW_AGC_GET_RES			= 727,

	MSG_SMS_RAW_CAPTURE_COMPLETE_IND = 728,

	MSG_SMS_DATA_PUMP_IND			= 729,
	MSG_SMS_DATA_PUMP_REQ			= 730,
	MSG_SMS_DATA_PUMP_RES			= 731,
	MSG_SMS_FLASH_DL_REQ			= 732,			// A request to program the FLASH
	// Format:	32-bit - Section status indication (0-first,running index,0xFFFFFFFF -last)
	//			32-bit - (optional) Image CRC or checksum
	//			32-bit - Total image length, in bytes, immediately following this DWORD
	//			32-bit - Actual section length, in bytes, immediately following this DWORD
	// Direction: Host->SMS
	MSG_SMS_FLASH_DL_RES			= 733,			// The status response to MSG_SMS_FLASH_DL_REQ
	// Format:	32-bit of the response
	// Direction: SMS->Host

	MSG_SMS_EXEC_TEST_1_REQ			= 734,
	MSG_SMS_EXEC_TEST_1_RES			= 735,

	MSG_SMS_ENBALE_TS_INTERFACE_REQ	= 736,			// A request set TS interface as the DATA(!) output interface
	// Format:	32-bit - Requested Clock speed in Hz(0-disable)
	//			32-bit - transmission mode (Serial or Paallel)
	// Direction: Host->SMS
	MSG_SMS_ENBALE_TS_INTERFACE_RES	= 737,

	MSG_SMS_SPI_SET_BUS_WIDTH_REQ	= 738,
	MSG_SMS_SPI_SET_BUS_WIDTH_RES	= 739,

	MSG_SMS_ISDBT_SET_LAYER_REQ		= 740,
	MSG_SMS_ISDBT_SET_LAYER_RES		= 741,

	MSG_SMS_EXT_ANTENNA_REQ			= 746,      //Activate external antenna search algorithm 
	MSG_SMS_EXT_ANTENNA_RES			= 747, 
	MSG_SMS_CMMB_GET_NET_OF_FREQ_REQ		= 748,	// Get the network level/num of a given frequency 
	//	Data[0]: Frequency 
	//	Data[1]: Reserved - must be 0
	MSG_SMS_CMMB_GET_NET_OF_FREQ_RES		= 749,	//	Data[0]: RetCode
	//	Data[1]: Network ID - 
	//				[15:12] Network Level
	//				[11:00] Network Number
	MSG_SMS_DLU_CPU_REQ						= 750,
	MSG_SMS_DLU_CPU_RES						= 751,

	MSG_SMS_CMMB_INJECT_TABLE_REQ   		= 752,  // Data: control table
	MSG_SMS_CMMB_INJECT_TABLE_RES   		= 753,  // Data[0]: return code
	MSG_SMS_HOST_NOTIFICATION_IND 			= 755,	// F/W notification to host
	// Data[0]:	SMSHOSTLIB_CMMB_HOST_NOTIFICATION_TYPE_ET
	MSG_SMS_CMMB_GET_CONTROL_TABLE_REQ		= 756,	// Data[0]: UINT32 Requested Control Table ID 
	MSG_SMS_CMMB_GET_CONTROL_TABLE_RES		= 757,	// Data: The requested table.
	//	The first byte should contain the table ID
	// (according to spec)

	MSG_SMS_CMMB_SET_SERVICE_OUTPUT_FORMAT_REQ		= 758,	// OBSOLETE 
	MSG_SMS_CMMB_SET_SERVICE_OUTPUT_FORMAT_RES		= 759,	// OBSOLETE 
	MSG_SMS_CMMB_GET_NETWORKS_REQ			= 760,	// Data[0]: Reserved - has to be 0
	MSG_SMS_CMMB_GET_NETWORKS_RES			= 761,	// Data[0]: RetCode
	// Data[1]: Number of networks (N)
	// Followed by N * SmsCmmbNetworkInfo_ST

	MSG_SMS_CMMB_START_SERVICE_REQ			= 762,	// Data[0]: UINT32 NetworkLevel
	// Data[1]: UINT32 NetworkNumber
	// Data[2]: UINT32 ServiceId
	MSG_SMS_CMMB_START_SERVICE_RES			= 763,	// Data[0]: UINT32 RetCode
	// Data[1]: UINT32 ServiceHandle

	MSG_SMS_CMMB_STOP_SERVICE_REQ			= 764,	// Data[0]: UINT32 ServiceHandle
	MSG_SMS_CMMB_STOP_SERVICE_RES			= 765,	// Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_GET_SERVICES_REQ			= 766,	// Data[0]: Reserved - has to be 0
	MSG_SMS_CMMB_GET_SERVICES_RES			= 767,	// Data[0]: RetCode
	// Data[1]: Number of services (N)
	//   Followed by N * SmsCmmbServiceInfo_ST

	MSG_SMS_CMMB_ADD_CHANNEL_FILTER_REQ		= 768,	// Data[0]: UINT32 Channel ID
	MSG_SMS_CMMB_ADD_CHANNEL_FILTER_RES		= 769,	// Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_REQ	= 770,	// Data[0]: UINT32 Channel ID
	MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_RES	= 771,	// Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_START_CONTROL_INFO_REQ		= 772,	// Data[0]: UINT32 NetworkLevel
	// Data[1]: UINT32 NetworkNumber
	MSG_SMS_CMMB_START_CONTROL_INFO_RES		= 773,	// Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ		= 774,	// Data[0]: UINT32 Reserved - has to be 0
	MSG_SMS_CMMB_STOP_CONTROL_INFO_RES		= 775,	// Data[0]: UINT32 RetCode

	MSG_SMS_ISDBT_TUNE_REQ					= 776,	// A request to tune to a new frequency
	// Data[0]:	UINT32 Frequency
	// Data[1]:	UINT32 Bandwidth
	// Data[2]:	UINT32 Crystal
	// Data[3]:	UINT32 Segment number
	//	Direction: Host->SMS
	MSG_SMS_ISDBT_TUNE_RES					= 777,	// Data[0]:	UINT32 RetCode
	// Direction: SMS->Host

	MSG_SMS_SLEEP_REQ		                = 778,	// No Parameters
	MSG_SMS_SLEEP_RES               		= 779,	// Data[0]: UINT32 RetCode

	MSG_SMS_SET_DC_CANCELLER_REQ            = 780,	// Change the state (enable/disable) of DC canceller opration in FW
	// Format: 32-bit new status (TRUE - operate, FALSE - stop)
	// Direction: Host->SMS

	MSG_SMS_SET_DC_CANCELLER_RES         	= 781,	// A reply to MSG_SMS_SET_DC_CANCELLER_REQ
	// Format: 32-bit status
	// Direction: SMS->Host

	MSG_SMS_TRANSMISSION_IND				= 782,  // Use in HostLib internally
	MSG_SMS_PID_STATISTICS_IND				= 783,	// Use in HostLib internally

	MSG_SMS_POWER_DOWN_IND					= 784,	// FW-->Host indicates start of the power down to sleep mode procedure
	//           data[0] - requestId, data[1] - message quarantine time
	MSG_SMS_POWER_DOWN_CONF					= 785,	// Host-->FW confirms the power down procedure, data[0] - requestId, data[1] - quarantine time
	MSG_SMS_POWER_UP_IND					= 786,	// FW-->Host indicates end of sleep mode,       data[0] - requestId
	MSG_SMS_POWER_UP_CONF					= 787,	// Host-->FW confirms the end of sleep mode,    data[0] - requestId

	MSG_SMS_CMMB_GET_CHANNELS_INFO_REQ		= 788,	// Request to get the current channels info. data[0] = TMP
	MSG_SMS_CMMB_GET_CHANNELS_INFO_RES		= 789,	// Data[0]: UINT32 RetCode
	// Data[1]: Number of channels (N)
	//	Followed by N * CmmbDbMpxFrame_ST

	MSG_SMS_POWER_MODE_SET_REQ				= 790,	// Host-->FW set the interslice power down (sleep) mode
	MSG_SMS_POWER_MODE_SET_RES				= 791,	// FW-->Host response to the previous request

	MSG_SMS_DEBUG_HOST_EVENT_REQ			= 792,	// An opaque event host-> FW for debugging purposes
	// Format:	data[0] = Event type (enum)
	//			data[1] = Param
	MSG_SMS_DEBUG_HOST_EVENT_RES			= 793,	// Response. Format:  data[0] = RetCode, data[1] = RetParam


	MSG_SMS_NEW_CRYSTAL_REQ					= 794,	// report crystal input to FW
	MSG_SMS_NEW_CRYSTAL_RES					= 795,

	MSG_SMS_CONFIG_SPI_REQ					= 796,	// Configure SPI interface (also activates I2C slave interface)
	// Format:	data[0] = SPI Controller (UINT32)
	//			data[1] = SPI Mode - Master/Slave (UINT32)
	//			data[2] = SPI Type - Mot/TI (UINT32)
	//			data[3] = SPI Width - 8bit/32bit (UINT32)
	//			data[4] = SPI Clock - in Hz (UINT32)
	MSG_SMS_CONFIG_SPI_RES					= 797,

	MSG_SMS_DUMMY_STAT_RES					= 798,

	MSG_SMS_START_IR_REQ					= 800,
	MSG_SMS_START_IR_RES					= 801,
	MSG_SMS_IR_SAMPLES_IND					= 802,
	//MSG_SMS_RESERVED1						= 803,

	MSG_SMS_SLAVE_DEVICE_DETECTED			= 804,

	MSG_SMS_INTERFACE_LOCK_IND				= 805,	// FW->Host, firmware requests that the host does not transmit anything on the interface
	MSG_SMS_INTERFACE_UNLOCK_IND			= 806,	// FW->Host, firmware signals that the host may resume transmission

	MSG_SMS_IF_DIAG_SEND_PKTS_REQ			= 807,	// Interface diagnostics request for FW to send to the Host test packets
	MSG_SMS_IF_DIAG_FW_TO_HOST_PKT			= 808,	// Test packets in response to the request above
	MSG_SMS_IF_DIAG_LISTEN_REQ				= 809,	// Interface diagnostics request for FW to listen to packets from the Host and gather statistics
	MSG_SMS_IF_DIAG_HOST_TO_FW_PKT			= 810,	// Test packets sent by the host after the request above
	MSG_SMS_IF_DIAG_GET_STATISTICS_REQ		= 811,	// Request to the FW to send the statistics gathered
	MSG_SMS_IF_DIAG_GET_STATISTICS_RES		= 812,	// FW response with the requested statistics
	MSG_SMS_IF_DIAG_GET_PARAMS_REQ			= 813,	// Request to the FW to send interface communication parameters
	MSG_SMS_IF_DIAG_GET_PARAMS_RES			= 814,	// Response to the above

	MSG_SMS_SET_AES128_KEY_REQ				= 815,
	MSG_SMS_SET_AES128_KEY_RES				= 816,

	MSG_SMS_DUMMY							= 819,	// Dummy messge, to be discarded

	MSG_SMS_IQ_STREAM_START_REQ				= 820,
	MSG_SMS_IQ_STREAM_START_RES				= 821,
	MSG_SMS_IQ_STREAM_STOP_REQ				= 822,
	MSG_SMS_IQ_STREAM_STOP_RES				= 823,
	MSG_SMS_IQ_STREAM_DATA_BLOCK			= 824,

	MSG_SMS_GET_EEPROM_VERSION_REQ          = 825,	// Request to get EEPROM version string

	MSG_SMS_GET_EEPROM_VERSION_RES          = 826,	// Response to get EEPROMK version string request
	// Format: 32-bit - Status
	//         32-bit - Length of string
	//         N*bytes - EEPROM version string

	MSG_SMS_SIGNAL_DETECTED_IND				= 827,
	MSG_SMS_NO_SIGNAL_IND					= 828,

	MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ			= 830,	// Power down MRC slave to save power
	MSG_SMS_MRC_SHUTDOWN_SLAVE_RES			= 831,
	MSG_SMS_MRC_BRINGUP_SLAVE_REQ			= 832,	// Return back the MRC slave to operation
	MSG_SMS_MRC_BRINGUP_SLAVE_RES			= 833,

	LOCAL_TUNE								= 850,	// Internal message sent by the demod after tune/resync
	MSG_INTERFERENCE_DECISION				= 851,	// Send the interference decision to the MRC slave (Orion MRC app)
	MSG_RESYNC_REQ							= 852,	// Internal resync request used by the MRC master

	MSG_SMS_CMMB_GET_MRC_STATISTICS_REQ		= 853,	// MRC statistics request (external, not exposed to users)			
	MSG_SMS_CMMB_GET_MRC_STATISTICS_RES		= 854,	
	// followed by N  SMSHOSTLIB_LOG_ITEM_ST  

	MSG_SMS_LOG_EX_ITEM						= 855,  // Format: 32-bit - number of log messages
	MSG_SMS_DEVICE_DATA_LOSS_IND			= 856,  // Indication on data loss on the device level
	MSG_SMS_HOST_DRIVER_DATA_LOSS_IND		= 857,  // Format: 32-bit - number of log messages

	MSG_LAST_MSG_TYPE						= 900	// Note: Stellar ROM limits this number to 700, other chipsets to 900


}MsgTypes_ET;


/************************************************************************/
/* Definitions for useful macros                                        */
/************************************************************************/

#define STELLAR_CHIP_MODEL		(0x1002)
#define NOVA_CHIP_MODEL			(0x1102)

#define SMS_MAX_SERVICE_HANDLE	(16)
#define	MAX_PROBE_CHANNELS		24		// maximum nr of channels to probe

typedef struct SmsMsgRFTRdAllRes_S
{
	UINT32 regData[14];		// Registers 0x0-0xB, 0xD, 0xE
	UINT32 regDataC[4];		// Register 0xC, Test RAM Data
	UINT32 regDataF[2];		// Register 0xF, Debug-2
} SmsMsgRFTRdAllRes_ST;

typedef struct SmsMsgOrionRdAllRes_S
{
	UINT32 regData[14];		// Registers 0x0-0xD				- 8 to 32 bits
	UINT32 regDataE[2];		// Register 0xE - "testmode reg2"	- 64 bits
	UINT32 regDataF[2];		// Register 0xF - "Ram test Data"	- 40 bits
} SmsMsgOrionRdAllRes_ST;


///////////////////////////////////////////////////////////////////////////////
//		Source / Target IP
///////////////////////////////////////////////////////////////////////////////


/// Statistics information returned by MSG_SMS_GET_STATISTICS_RES from sms1000 to smsCntrlLib
// and
// Statistics information returned by SMSHOSTLIB_MSG_GET_STATISTICS_RES from smsCntrlLib to caller

typedef struct
{
	UINT32 RequestResult;

	SMSHOSTLIB_STATISTICS_ST Stat;

	// Split the calc of the SNR in DAB
	UINT32 Signal;				//!< dB
	UINT32 Noise;				//!< dB

} SmsMsgStatisticsInfo_ST;


typedef struct SmsVersionRes_S
{
	SmsMsgHdr_ST					xMsgHeader;
	SMSHOSTLIB_VERSION_ST			xVersion;
} SmsVersionRes_ST;

// SmsSetAntennaConfiguration
typedef struct SMS_ANTENNA_FREQ_DOMAINS_S
{
	UINT32		buttom_freq;
	UINT32		top_freq;

} SMS_ANTENNA_FREQ_DOMAINS_ST;

typedef struct SMS_ANTENNA_CONFIG_S
{
	UINT32						PinNum[2];

	SMS_ANTENNA_FREQ_DOMAINS_ST	FreqDomains[4];

} SMS_ANTENNA_CONFIG_ST;


/* ------------------------------------------- */
/* Message subtypes for device interface tests */
/* ------------------------------------------- */

// message type
#define		DEV_INT_TEST_MSG_TYPE_CTRL			0
#define		DEV_INT_TEST_MSG_TYPE_DATA			1
#define		DEV_INT_TEST_NO_MSG					2				// To stop sending messages

// data type
#define		DEV_INT_TEST_DATA_GENERATE			0
#define		DEV_INT_TEST_DATA_COPY				1

// with MSG_SMS_IF_DIAG_SEND_PKTS_REQ
typedef struct DevIntTest_FwToHostSendPktsReq_S					// DevIntTest_SendPktsReq_ST
	{
	UINT32									rqstId;
	UINT32									rqstMsgType;		// DEV_INT_TEST_MSG_TYPE_CTRL, DEV_INT_TEST_MSG_TYPE_DATA, DEV_INT_TEST_NO_MSG
	UINT32									rqstDataType;		// DEV_INT_TEST_DATA_GENERATE, DEV_INT_TEST_DATA_COPY
	UINT32									rqstCount;
	UINT32									rqstSize;
	UINT32									rqstBurstSize;		// length of burst to be used
	UINT32									rqstPktRate;		// rate of packets to be sent, 0 - no restrictions
	} DevIntTest_SendPktsReq_ST;

// with MSG_SMS_IF_DIAG_FW_TO_HOST_PKT
typedef struct DevIntTest_FwToHostPkt_S
	{
	UINT32									rqstId;
	UINT32									seqNr;				// sequence number
	UINT32									dataType;
	UINT32									durationUsec;		// time in microseconds from the first sent packet
	} DevIntTest_FwToHostPkt_ST;

// with MSG_SMS_IF_DIAG_LISTEN_REQ
typedef struct DevIntTest_ListenReq_S					
	{
	UINT32									rqstId;				// id carried by the host to fw packets
	} DevIntTest_ListenReq_ST;

// with MSG_SMS_IF_DIAG_HOST_TO_FW_PKT
typedef struct DevIntTest_HostToFwPkt_S
	{
	UINT32									rqstId;
	UINT32									seqNr;
	UINT32									dataLen;
	} DevIntTest_HostToFwPkt_ST;

// with MSG_SMS_IF_DIAG_GET_STATISTICS_REQ
typedef struct DevIntTest_GetStatisticsReq_S
	{
	UINT32									rqstId;				// id carried by the host to fw packets
	} DevIntTest_GetStatisticsReq_ST;

// with MSG_SMS_IF_DIAG_GET_STATISTICS_RES
typedef struct DevIntTest_GetStatisticsRes_S
	{
	UINT32									rqstId;				// id carried by the host to fw packets
	UINT32									validPktsNr;		// how many valid packets for this request ID have been received
	UINT32									durationUsec;		// interval between the listen request and the last valid packet
	} DevIntTest_GetStatisticsRes_ST;

// with MSG_SMS_IF_DIAG_GET_PARAMS_RES
typedef struct DevIntTest_GetParamsRes_S
	{
	UINT32									maxFwToHostCtrlPktSize;	// max fw to host ctrl packet size as a function of the current interface
	UINT32									maxFwToHostDataPktSize;	// max fw to host data packet size as a function of the current interface
	UINT32									maxHostToFwPktSize;		// max host to fw packet size as a function of the current interface
	UINT32									maxHostToFwBurstLength;	// max host to fw packet burst length
	UINT8									ifName [32];			// interface name
	} DevIntTest_GetParamsRes_ST;



#endif





