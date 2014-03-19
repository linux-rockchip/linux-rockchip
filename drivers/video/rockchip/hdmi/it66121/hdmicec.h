
///*****************************************
//   @file   <hdmitcec.h>
//   @author szy
//   @date   2014/01/16
//******************************************/

#ifndef _HDMICEC_H_
#define _HDMICEC_H_
#include "typedef.h"
#include"it66121.h"

#define _CEC_DEVICE_PLAYBACK_

typedef unsigned char u8_t;
//typedef unsigned char BYTE;
//typedef unsigned char BOOL;


typedef struct
{
	u8_t	SIZE;
	u8_t	HEADER;
	u8_t	OPCODE;
	u8_t	OPERAND1;
	u8_t	OPERAND2;
	u8_t	OPERAND3;
	u8_t	OPERAND4;
	u8_t	OPERAND5;
	u8_t	OPERAND6;
	u8_t	OPERAND7;
	u8_t	OPERAND8;
	u8_t	OPERAND9;
	u8_t	OPERAND10;
	u8_t	OPERAND11;
	u8_t	OPERAND12;
	u8_t	OPERAND13;
	u8_t	OPERAND14;
	u8_t	OPERAND15;
	u8_t	OPERAND16;

}CEC_FRAME, *P_CEC_FRAME;

typedef enum _CEC_TX_StateType
{
	sCECOff=0,
	sCECcmdGet,
	sCECTransfer,
	sCECCheckResult,
	sCECReceiverFail,
	sCECReceiverAck,
	sCECReceiverNack,
	sCECOk,
	sCECNone,
} CEC_TX_StateType;


typedef enum _ARC_StateType
{
	sRequestARCInitiation=0,
	sInitiateARC,
	sReportARCInitiated,

	sRequestARCTermination,
	sTerminateARC,
	sReportARCTermination,

	sARCNone,
} ARC_StateType;

typedef enum _HEC_StateType
{
	sHEC_InquirState			=0x00,
	sHEC_ReportState			=0x01,
	sHEC_SetStateAdjacent		=0x02,
	sHEC_SetState				=0x03,
	sHEC_RequestDeactivation	=0x04,
	sHEC_NotifyAlive			=0x05,
	sHEC_Discover				=0x06,
	sHEC_HPDSetState			=0x10,
	sHEC_HPDReportState		=0x11,
	sHEC_None,
} HEC_StateType;


typedef enum _SystemInfoFeature_StateType
{
	sCECVersioin			=0x9E,
	sGetCECVersion			=0x9F,
	sGivePhysicalAddress	=0x83,	// follower:All
	sGetMenuLanguage		=0x91,	// follower:TV
	sReportPhysicalAddress	=0x84,	// follower:TV
	sSetMenuLanguage		=0x32,	// follower:All,		Initiator:TV
	sSIFnone,
} SystemInfoFeature_StateType;


typedef enum _SystemAudioControl_StateType
{

	sComesOutOfStandby=0,
	sSetSystemAudioModeToTV,
	sWaitingFeatureAbort,
	sSetSystemAudioModeToAll,
	sSACnone,
} SystemAudioControl_StateType;




typedef enum _OneTouchPlay_StateType
{
	sActiveSource,
	sImageViewOn,
	sTextViewOn,
	sOTNone,
} OneTouchPlay_StateType;

typedef enum _LogicAddressing_StateType
{
	sReportPhysicalAddressTransfer,
	sReportPhysicalAddressReceived,
	sPollingMessage,
	sPollingResult,
	sLANone,
} LogicAddressing_StateType;


typedef enum _PollingMessage_StateType
{
	sPollingMessage1,
	sPollingMessage2,
	sPollingMessage3,
	sReportPhyAdr,
	sReportDeviceVendorID,
	sPMnone
} PollingMessage_StateType;

typedef enum _CEC_Logic_DEVICE_ID_t
{
	DEVICE_ID_TV=0,
	DEVICE_ID_RECORDING1,		// 1
	DEVICE_ID_RECORDING2,		// 2
	DEVICE_ID_TUNER1,			// 3
	DEVICE_ID_PLAYBACK1,		// 4
	DEVICE_ID_AUDIO,			// 5
	DEVICE_ID_TUNER2,			// 6
	DEVICE_ID_TUNER3,			// 7
	DEVICE_ID_PLAYBACK2,		// 8
	DEVICE_ID_RECORDING3,		// 9
	DEVICE_ID_TUNER4,			// 10
	DEVICE_ID_PLAYBACK3,		// 11
	DEVICE_ID_RESERVED1,		// 12
	DEVICE_ID_RESERVED2,		// 13
	DEVICE_ID_FREEUSE,			// 14
	DEVICE_ID_BROADCAST,		// 15
	DEVICE_ID_MAX
} CEC_Logic_DEVICE_ID_t;


 typedef struct {
// 	CEC_Logic_DEVICE_ID_t	LogicDevice;
	u8_t						PhyicalAddr1;
	u8_t						PhyicalAddr2;
	u8_t						Active;
 }CECDevice, *pCECDevice;


typedef struct {
//	u8_t 	HDMIInput		: 14;	//bit 0~13	 , HDMI Input 1 as bit13, HDMI Input 2 as bit12
	u8_t 	HECActivation	: 1;		//bit 14	, '0' as HEC Inactive  , '1' as HEC Active
	u8_t		Reserved		: 1;		//bit 15
}HECActivationField, *pHECActivationField;

typedef struct {
//	u8_t 	HDMIInput		: 14;	//bit 0~13	 , HDMI Input 1 as bit13, HDMI Input 2 as bit12
	u8_t 	HECSupport		: 1;		//bit 14	, '0' as HEC Not Support  , '1' as HEC Support
	u8_t		Reserved		: 1;		//bit 15
}HECSupportField, *pHECSupportField;

typedef struct {
	u8_t 	CDCErrorCode	: 2;		//bit 0~1	, HDMI Input 1 as bit13, HDMI Input 2 as bit12
	u8_t 	ENCState		: 2;		//bit 2~3	, '00' Ext Con Not Support  , '01' Ext Con Inactive ,'10' Ext Con Active
	u8_t		HostState		: 2;		//bit 4~5	, '00' Host Not Support  , '01' Host Inactive ,'10' Host Active
	u8_t		HECState		: 2;		//bit 6~7	, '00' HEC Not Support  , '01' HEC Inactive ,'10' HEC Active
}HECStateField, *pHECStateField;

typedef enum {
	eCPEDID_disable=0,
	eCPEDID_ENABLE=1,
	eCPEDID_DISABLE_ENABLE=2,
	eEDID_DISABLE=3,
	eEDID_ENABLE=4,
	eEDID_DISABLE_ENABLE=5,
}HPDStateType, *pHPDStateType;

typedef enum {
	eNoError=0,
	eNoRequestedCapability=1,
	eNoCapableToCarryOut=2,
	eOtherError=3,
	eNoErrorNoVideoStream=4,
}HPDErrorCodeType, *pHPDErrorCodeType;


typedef enum _CEC_COMMAND
{
	eFeatureAbort			=0x00,

// One Touch Play Feature
	eActiveSource			=0x82,	// follower:TV, switch	--> Broadcst , Directly address
	eImageViewOn			=0x04,	// follower:TV, switch	--> Broadcst
	eTextViewOn			=0x0D,	// follower:TV

// Routing Control Feature
	eRoutingChange			=0x80,	// follower:TV, switch	--> Broadcst
	eRoutingInformation		=0x81,	// follower:switch		--> Broadcst
	eRequestActiveSource	=0x85,	// follower:switch		--> Broadcst 	, AVR to request active source when Audio ocntrol feature is active
	eSetStreamPath			=0x86,	// follower:switch		--> Broadcst
	eInactiveSource			=0x9D,	//
	eStandBy				=0x36,	// follower:All			--> Broadcst

// System Information Feature
	eCECVersioin			=0x9E,
	eGetCECVersion			=0x9F,
	eGivePhysicalAddress	=0x83,	// follower:All
	eGetMenuLanguage		=0x91,	// follower:TV
	eReportPhysicalAddress	=0x84,	// follower:TV
	eSetMenuLanguage		=0x32,	// follower:All,		Initiator:TV

// Vendor Specific Commands Feature
	eDeviceVendorID		=0x87,
	eGiveDeviceVendorID	=0x8C,
	eVendorCommand		=0x89,
	eGiveDevicePowerStatus	=0x8F,
	eReportPowerStatus		=0x90,
	eVendorCommandWithID =0xA0,

// other
	ePlay					=0x41,
	eUserPressed			=0x44,
	eUserReleased			=0x45,
	eAudioModeRequest		=0x70,
	eGiveAudioStatus		=0x71,
	eSetSystemAudioMode	=0x72,
	eReportAudioStatus		=0x7A,
	eGiveSystemAudIoModeStatus	=0x7D,
	eSystemAudIoModeStatus		=0x7E,
	eRemoteButtonDown		=0x8A,
	eRemoteButtonUp		=0x8B,
	eReportAudioDescriptor 	=0xA3,
	eRequestAudioDescriptor =0xA4,

// ACR
	eInitiateARC				=0xC0,
	eReportARCInitiated		=0xC1,
	eReportARCTerminated	=0xC2,
	eRequestARCInitiation	=0xC3,
	eRequestARCTermination	=0xC4,
	eTerminateARC			=0xC5,

// HEC
	eCDC					=0xF8,

// Polling Mesage
	ePollingMessage,

	eAbort					=0xff,

	eCECNone,

} eCEC_CMD;

typedef enum _CDC_COMMAND
{
	eHEC_InquirState			=0x00,
	eHEC_ReportState			=0x01,
	eHEC_SetStateAdjacent		=0x02,
	eHEC_SetState				=0x03,
	eHEC_RequestDeactivation	=0x04,
	eHEC_NotifyAlive			=0x05,
	eHEC_Discover				=0x06,
	eHEC_HPDSetState			=0x10,
	eHEC_HPDReportState		=0x11,
}eCDC_CMD;

typedef union {
	struct
	{
		BYTE 	AudioVolumeStatus	:7;	//bit 0~6	Volume status
		BYTE	AudioMuteStatus		:1;	//bit 7 	Mute status
	}Map;

	BYTE AStatus;
}CECAudioStatus;

typedef union {
	struct
	{
		BYTE 	AudioFormatCode	:6;	//bit 0~5	Audio Format Code
		BYTE	AudioFormatID		:2;	//bit 6~7 	Audio Format ID
	}Map;

	BYTE AStatus;
}CECAudioFormatID;

typedef enum {
	eDirectly=0,
	eBroadcast=1,
	eBoth=2,
}HeaderType;


typedef struct {
	eCEC_CMD cmd;
	HeaderType header;
	u8_t	size;
} CECcmdFormat;


//Register Map
#define	REG_VENID_L			0x00
#define	REG_VENID_H			0x01
#define	REG_DEVICEID_L			0x02
#define	REG_DEVICEID_H		0x03
#define	REG_MREVISIONID		0x04
#define	REG_SRVISIONID			0x05
//CEC Initiator Block
#define	REG06					0x06
#define	REG07					0x07
#define	REG08					0x08
#define	B_FIRE_FRAME			0x80
#define	B_DBGCEC_CLR			0x40
#define	B_CEC_OE				0x20
#define	B_CEC_FORCE			0x10
#define	B_CEC_SMT				0x08
#define	B_CEC_RST				0x04
#define	B_REFCLK_RST			0x02
#define	B_INT_CLR				0x01

#define	REG09					0x09
#define	B_DATAIT_SEL			0x80
#define	B_REGION_SEL			0x40
#define	B_RXSELF_SEL			0x20
#define	B_REFIRE				0x10
#define	B_ACKTRIG_SEL			0x08
#define	B_PULSE_SEL			0x04
#define	B_NACK_EN				0x02
#define	B_EN100ms_CNT			0x01

#define	REG0A					0x0A
#define	REG_DATA_MIN			0x0B
#define	REG_TIMER_UNIT		0x0C
#define	REG0D					0x0D
#define	REG_TX_HEADER			0x10
#define	REG_TX_OPCODE			0x11
#define	REG_TX_OPERAND1		0x12
#define	REG_TX_OPERAND2		0x13
#define	REG_TX_OPERAND3		0x14
#define	REG_TX_OPERAND4		0x15
#define	REG_TX_OPERAND5		0x16
#define	REG_TX_OPERAND6		0x17
#define	REG_TX_OPERAND7		0x18
#define	REG_TX_OPERAND8		0x19
#define	REG_TX_OPERAND9		0x1A
#define	REG_TX_OPERAND10		0x1B
#define	REG_TX_OPERAND11		0x1C
#define	REG_TX_OPERAND12		0x1D
#define	REG_TX_OPERAND13		0x1E
#define	REG_TX_OPERAND14		0x1F
#define	REG_TX_OPERAND15		0x20
#define	REG_TX_OPERAND16		0x21
#define	REG_CEC_TARGET_ADDR 	0x22
#define	REG_CEC_OUT_NUM		0x23
//AFE BLOCK
#define	REG24					0x24
#define	REG_HEAC_CTRL			0x25
#define	B_SPDIFO_EN			0x08
#define	B_HEC_EN				0x04
#define	B_ARC_RX_EN			0x02
#define	B_ARC_TX_EN			0x01
#define	REG26					0x26
#define	REG27					0x27
#define	REG28					0x28
#define	REG29					0x29
#define	REG2A					0x2A
#define	REG2B					0x2B
#define	REG2C					0x2C
#define	REG2D					0x2D
//CEC FOLLOWER BLOCK
#define	REG_RX_HEADER			0x30
#define	REG_RX_OPCODE			0x31
#define	REG_RX_OPERAND1		0x32
#define	REG_RX_OPERAND2		0x33
#define	REG_RX_OPERAND3		0x34
#define	REG_RX_OPERAND4		0x35
#define	REG_RX_OPERAND5		0x36
#define	REG_RX_OPERAND6		0x37
#define	REG_RX_OPERAND7		0x38
#define	REG_RX_OPERAND8		0x39
#define	REG_RX_OPERAND9		0x3A
#define	REG_RX_OPERAND10		0x3B
#define	REG_RX_OPERAND11		0x3C
#define	REG_RX_OPERAND12		0x3D
#define	REG_RX_OPERAND13		0x3E
#define	REG_RX_OPERAND14		0x3F
#define	REG_RX_OPERAND15		0x40
#define	REG_RX_OPERAND16		0x41
#define	REG_FOLLOWER_RX_NUM	0x42
#define	REG_INITIATOR_TX_NUM	0x43
#define	REG_SYS_STATUS		0x44
#define	B_READY_FIRE			0x40
#define	B_ERROR_STATUS		0x30
#define	B_OUT_STATUS			0x0C
#define	B_ReceiverACK			0x00
#define	B_ReceiverNACK			0x04
#define	B_ReceiverRetry			0x08
#define	B_ReceiverFail			0x0C
#define	B_BUS_STATUS			0x02

#define	REG_MSCOUNT_L			0x45
#define	REG_MSCOUNT_M		0x46
#define	REG_MSCOUNT_H			0x47

#define	REG_DBG_STATE			0x48
#define	REG_DBG_BLOCK			0x49
#define	REG_DBG_BIT			0x4A
#define	REG_DBG_TIMING		0x4B

#define	REG_INT_STATUS		0x4C
#define	B_CECFAIL_INT			0x20
#define	B_RXDONE_INT			0x10
#define	B_TXDONE_INT			0x08
#define	B_RXFAIL_INT			0x04
#define	B_RX_INT				0x02
#define	B_TX_INT				0x01


void cecclrint( void );
void cecconfig( BYTE LogicAdr, BYTE TimeUnit, BYTE CECRst, BYTE EnNAck );
BYTE cecfirerdy( void );
//BYTE cecfirecmd(BYTE TxCmdNum, BYTE TxCmdBuf[] );
void cecfirecmd( void );
void cecmonitor( void );	//BYTE mode, BYTE LogicAdr )

void HDMITX_CEC_Init(void);
void CEC_InactiveSource(BYTE port);
void CEC_RoutingChange(BYTE port);
void CECDecoder(void);
void Switch_OneTouchPlayState(OneTouchPlay_StateType uState);
void OneTouchPlayHandler(void);
void Switch_LogicalAddressingState(LogicAddressing_StateType uc);
void LogicalAddressingHandler(void);

void Switch_SystemInfoFeature(SystemInfoFeature_StateType State);
void SystemInfomationFeature(void);
void switch_PollingMessage(PollingMessage_StateType state);
void PollingMessage(void);

extern  void CECCmdSet(u8_t Follower,eCEC_CMD TxCommand,u8_t cOperand1,u8_t cOperand2);
void iT6601Transmitter(void);

BOOL CheckUpDownPhyAdr(void);
void switch_ARCState(ARC_StateType uState);
void ARCHandler(void);

void Switch_SystemAudioControl(SystemAudioControl_StateType State);
void SystemAudioControl(void);

void CECChangePort(void);

void SwitchHECState(HEC_StateType	uState);

void CECUICommandCode(BYTE UIKey);

void CECTest(eCEC_CMD cmd,u8_t cData);

void CECManager(void);
extern void DumpiT6601Reg(void);

extern u8_t Myself_LogicAdr;

extern u8_t u8_PowerStatus;	// 0: on , 1: standby, 2: in transition standby to on , 3: in transition on to standby
void Initial_Ext_Int1();


///////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_iT6601
#ifdef	DEBUG_iT6601
#define	IT6601_PRINT(x) printk x
#else
#define	IT6601_PRINT(x)
#endif


extern CEC_TX_StateType  CECTxState;
void SetCecAddress(int cecadd);
extern BYTE CEC_ReadI2C_Byte(BYTE RegAddr);
extern SYS_STATUS CEC_WriteI2C_Byte(BYTE offset,BYTE buffer );
extern SYS_STATUS CEC_SetI2C_Byte(BYTE Reg,BYTE Mask,BYTE Value);
#endif // _HDMICEC_H_
