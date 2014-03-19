///*****************************************
//   @file   <hdmitcec.c>
//   @author szy
//   @date   2014/01/16
//******************************************/
#include "hdmicec.h"
static int CECADDR;
#define Q_SIZE 5
//for iT6601Receiver( )
CEC_FRAME     queue[ Q_SIZE ];
P_CEC_FRAME   pCECRxBuf;
u8_t          head;
u8_t          tail;
//for iT6601Transmitter( )
CEC_FRAME     queue_Tx[ Q_SIZE ];
P_CEC_FRAME   pCECBuf_Tx;
u8_t          head_Tx;
u8_t          tail_Tx;
CEC_FRAME     TxCmdBuf;
CECDevice     CECList[15];
CEC_TX_StateType  CECTxState=sCECOff;
eCEC_CMD OldTxCECcmd=eCECNone;
//for CECManager( )
ARC_StateType ARCState=sARCNone;
LogicAddressing_StateType LAState=sLANone;
OneTouchPlay_StateType OTState=sOTNone;
PollingMessage_StateType PMState=sPMnone;
SystemInfoFeature_StateType SIFState=sSIFnone;
SystemAudioControl_StateType SACState=sSACnone;
//=== Audio System Control ===
//for Spec 13.15.2 and CTS 11.2.15
// 0 Terminating , 1 Initating the system audio control
//need to un-mute Amplifer speaker
BOOL bSystemAudioMode=0;
CECAudioStatus uCECAudioStatus;
CECAudioFormatID uCECAudioFormatID;
BYTE UICommandCode;
u8_t StandbyState;
USHORT  CMDCount=0;
u8_t u8_PowerStatus=0;  // 0: on , 1: standby, 2: in transition standby to on , 3: in transition on to standby
u8_t     CECInt;
u8_t  RxSelf_Dis = 1;   // 1 for check transmiter data by self rx buffer
u8_t     Myself_LogicAdr;
u8_t     Myself_PhyAdr1;
u8_t     Myself_PhyAdr2;
//FOR iT6633+iT6623
u8_t  VSDB_Addr;
u8_t  txphyadr[2], txphyA, txphyB, txphyC, txphyD, txphylevel;
u8_t  rxphyadr[4][2];
u8_t  rxphyA, rxphyB, rxphyC, rxphyD, rxcurport;
//for HEC Command
u8_t     FollowerPhyAdr1;
u8_t     FollowerPhyAdr2;
u8_t     FollowerPhyAdr3;
u8_t     FollowerPhyAdr4;
u8_t     FollowerAddr;
u8_t     timeunit;
// for check Invalid CEC Cmd
const  CECcmdFormat  LookUpTable_CEC[]=
{
    {eFeatureAbort,         eDirectly,      4},         // [Header] + [0x00] + [Feature OpCode] + [Abort Reason]
    {eAbort,                    eDirectly,      2},         // [Header] + [0xFF]
    // One Touch Play Feature
    {eActiveSource,         eBroadcast, 4},         // Header + [0x82] + [Physical Address]
    {eImageViewOn,          eDirectly,      2},         // Header + [0x04]
    {eTextViewOn,           eDirectly,      2},         // Header + [0x0D]
    // Routing Control Feature
    {eInactiveSource,           eBroadcast, 2},         // Header + [0x9D]
    {eRequestActiveSource,      eBroadcast, 2},         // Header + [0x85]
    {eRoutingChange,            eBroadcast, 6},         // Header + [0x80] + [Org Address] + [New Address]
    {eRoutingInformation,       eBroadcast, 4},         // Header + [0x80] + [Org Address] + [New Address]
    {eSetStreamPath,            eBroadcast, 4},         // Header + [0x86] + [Physical Address]
    {eStandBy,              eBoth,      2},         // Header + [0x36]
    // System Information Feature
    {eCECVersioin,          eDirectly,      3},         // Header + [0x82] + [CEC Version]
    {eGetCECVersion,            eDirectly,      2},         // Header + [0x9F]
    {eGivePhysicalAddress,      eDirectly,      2},         // Header + [0x83]
    {eGetMenuLanguage,      eDirectly,      2},         // Header + [0x91]
    {eReportPhysicalAddress,    eBroadcast, 5},         // Header + [0x84] + [Physical Address]+ [Device Type]
    {eSetMenuLanguage,      eBroadcast, 3},         // Header + [0x32] + [Language]
    // Vendor Specific Commands Feature
    {eDeviceVendorID,       eBroadcast, 5},         // Header + [0x87] + [Vendor ID]
    {eGiveDeviceVendorID,       eDirectly,      2},         // Header + [0x8C]
    {eVendorCommand,        eDirectly,      3},         // Header + [0x89] + [Vendor Specific Data]
    {eVendorCommandWithID,  eBoth,      6},         // Header + [0xA0] + [Vendor ID]+ [Vendor Specific Data]
    {eRemoteButtonDown,     eBoth,      3},         // Header + [0x8A] + [Vendor Specific RC code]
    {eRemoteButtonUp,       eBoth,      3},         // Header + [0x8B] + [Vendor Specific RC code]
    // Remote Control PassThrough Feature
    {eUserPressed,          eDirectly,      3},         // Header + [0x44] + [UI command]
    {eUserReleased,         eDirectly,      2},         // Header + [0x45]
    // Power Status Feature
    {eGiveDevicePowerStatus,    eDirectly,      2},         // Header + [0x8F]
    {eReportPowerStatus,        eDirectly,      3},         // Header + [0x90] + [Power Status]
    // System Audio Control Feature
    {eGiveAudioStatus,      eDirectly,      2},         // Header + [0x71]
    {eGiveSystemAudIoModeStatus,eDirectly,  2},         // Header + [0x7D]
    {eReportAudioStatus,        eDirectly,      3},         // Header + [0x7A] + [Audio Status]
    {eReportAudioDescriptor,    eDirectly,      4},         // Header + [0xA3] + [Audio Descriptor]
    {eRequestAudioDescriptor,   eDirectly,      3},         // Header + [0xA4] + [Audio Format ID and Code]
    {eSetSystemAudioMode    ,   eBoth,      3},         // Header + [0x72] + [System Audio Status]
    {eAudioModeRequest,     eDirectly,      4},         // Header + [0x70] + [Physical Address]
    {eSystemAudIoModeStatus,    eDirectly,      3},         // Header + [0x7E] + [System Audio Status]
    // ACR
    {eInitiateARC,              eDirectly,      2},         // Header + [0xC0]
    {eReportARCInitiated,       eDirectly,      2},         // Header + [0xC1]
    {eReportARCTerminated,  eDirectly,      2},         // Header + [0xC2]
    {eRequestARCInitiation,     eDirectly,      2},         // Header + [0xC3]
    {eRequestARCTermination,    eDirectly,      2},         // Header + [0xC4]
    {eTerminateARC,         eDirectly,      2},         // Header + [0xC5]
    // HEC
    {eCDC,                  eDirectly,      3},         // Header + [0xF8]
    // other
    {ePlay,                 eDirectly,      3},         // Header + [0x41] + [Play Mode]
    // Polling Mesage
    //ePollingMessage,
    {eCDC+1,                eBoth,      0},         //end of     lookup table !!!
};
#define     SizeOfLookUpTable_CEC    (sizeof(LookUpTable_CEC)/sizeof(CECcmdFormat))
char * ARCStr[] = {
    "sRequestARCInitiation",
    "sInitiateARC",
    "sReportARCInitiated",
    "sRequestARCTermination",
    "sTerminateARC",
    "sReportARCTermination",
    "sARCNone",
} ;
//==================================================================
//==================================================================


void SetCecAddress(int cecadd)
{
	CECADDR=cecadd;
}

void DumpiT6601Reg(void)
{
#ifdef DEBUG_iT6601
    int i,j ;
    BYTE ucData ;
    printk("       ") ;
    for ( j = 0 ; j < 16 ; j++ )
    {
        printk(" %x",(BYTE)j) ;
        if ( (j == 3)||(j==7)||(j==11) )
        {
            printk("  ") ;
        }
    }
    printk("\n        -----------------------------------------------------\n") ;
    for ( i = 0 ; i < 0x4F ; i+=16 )
    {
        printk("[%x]  ",(BYTE)i) ;
        for ( j = 0 ; j < 16 ; j++ )
        {
            ucData = CEC_ReadI2C_Byte((BYTE)((i+j)&0xFF)) ;
            printk(" %x", ucData) ;
            if ( (j == 3)||(j==7)||(j==11) )
            {
                printk(" -") ;
            }
        }
        printk("\n") ;
        if ( (i % 0x40) == 0x30 )
        {
            printk("        -----------------------------------------------------\n") ;
        }
    }
#endif
}
void
initQ_Tx()
{
    head_Tx = 0;
    tail_Tx = 0;
}
void
EnQueue_Tx(void)
{
    UCHAR   i;
    tail_Tx += 1;
    i=tail_Tx % Q_SIZE;
    queue_Tx[i].SIZE=TxCmdBuf.SIZE;
    queue_Tx[i].HEADER=TxCmdBuf.HEADER;
    queue_Tx[i].OPCODE=TxCmdBuf.OPCODE;
    queue_Tx[i].OPERAND1=TxCmdBuf.OPERAND1;
    queue_Tx[i].OPERAND2=TxCmdBuf.OPERAND2;
    queue_Tx[i].OPERAND3=TxCmdBuf.OPERAND3;
    queue_Tx[i].OPERAND4=TxCmdBuf.OPERAND4;
    queue_Tx[i].OPERAND5=TxCmdBuf.OPERAND5;
    queue_Tx[i].OPERAND6=TxCmdBuf.OPERAND6;
    queue_Tx[i].OPERAND7=TxCmdBuf.OPERAND7;
    queue_Tx[i].OPERAND8=TxCmdBuf.OPERAND8;
    queue_Tx[i].OPERAND9=TxCmdBuf.OPERAND9;
    queue_Tx[i].OPERAND10=TxCmdBuf.OPERAND10;
    queue_Tx[i].OPERAND11=TxCmdBuf.OPERAND11;
    queue_Tx[i].OPERAND12=TxCmdBuf.OPERAND12;
    queue_Tx[i].OPERAND13=TxCmdBuf.OPERAND13;
    queue_Tx[i].OPERAND14=TxCmdBuf.OPERAND14;
    queue_Tx[i].OPERAND15=TxCmdBuf.OPERAND15;
    queue_Tx[i].OPERAND16=TxCmdBuf.OPERAND16;
}
P_CEC_FRAME
DeQueue_Tx()
{
    head_Tx += 1;
    return &(queue_Tx[ head_Tx % Q_SIZE ]);
}
UCHAR
isFull_Tx()
{
    if ( ( head_Tx % Q_SIZE ) == ( ( tail_Tx+1 ) % Q_SIZE ) )
    {
        return TRUE;
    }
    return FALSE  ;
}
UCHAR
isEmpty_Tx()
{
    if ( (head_Tx % Q_SIZE) == (tail_Tx % Q_SIZE) )
    {
        return TRUE;
    }
    return FALSE;
}
UCHAR
AddQ_Tx(void)
{
    // IF buffer is full , can't put data
    if ( isFull_Tx() == TRUE )
    {
        return FALSE;
    }
    // put in buffer
    EnQueue_Tx( );
    return TRUE;
}
//==================================================================
void
initQ()
{
    head = 0;
    tail = 0;
}
void
EnQueue(void)
{
    UCHAR   i;
    tail += 1;
    i=tail % Q_SIZE;
    queue[i].SIZE=CEC_ReadI2C_Byte(REG_FOLLOWER_RX_NUM);
    queue[i].HEADER=CEC_ReadI2C_Byte(REG_RX_HEADER);
    queue[i].OPCODE=CEC_ReadI2C_Byte(REG_RX_OPCODE);
    queue[i].OPERAND1=CEC_ReadI2C_Byte(REG_RX_OPERAND1);
    queue[i].OPERAND2=CEC_ReadI2C_Byte(REG_RX_OPERAND2);
    queue[i].OPERAND3=CEC_ReadI2C_Byte(REG_RX_OPERAND3);
    queue[i].OPERAND4=CEC_ReadI2C_Byte(REG_RX_OPERAND4);
    queue[i].OPERAND5=CEC_ReadI2C_Byte(REG_RX_OPERAND5);
    queue[i].OPERAND6=CEC_ReadI2C_Byte(REG_RX_OPERAND6);
    queue[i].OPERAND7=CEC_ReadI2C_Byte(REG_RX_OPERAND7);
    queue[i].OPERAND8=CEC_ReadI2C_Byte(REG_RX_OPERAND8);
    queue[i].OPERAND9=CEC_ReadI2C_Byte(REG_RX_OPERAND9);
    queue[i].OPERAND10=CEC_ReadI2C_Byte(REG_RX_OPERAND10);
    queue[i].OPERAND11=CEC_ReadI2C_Byte(REG_RX_OPERAND11);
    queue[i].OPERAND12=CEC_ReadI2C_Byte(REG_RX_OPERAND12);
    queue[i].OPERAND13=CEC_ReadI2C_Byte(REG_RX_OPERAND13);
    queue[i].OPERAND14=CEC_ReadI2C_Byte(REG_RX_OPERAND14);
    queue[i].OPERAND15=CEC_ReadI2C_Byte(REG_RX_OPERAND15);
    queue[i].OPERAND16=CEC_ReadI2C_Byte(REG_RX_OPERAND16);
//    queue[tail % Q_SIZE]. = dataX;
}
P_CEC_FRAME
DeQueue()
{
    head += 1;
    return &(queue[ head % Q_SIZE ]);
}
UCHAR
isFull()
{
    if ( ( head % Q_SIZE ) == ( ( tail+1 ) % Q_SIZE ) )
    {
        return TRUE;
    }
    return FALSE  ;
}
UCHAR
isEmpty()
{
    if ( (head % Q_SIZE) == (tail % Q_SIZE) )
    {
        return TRUE;
    }
    return FALSE;
}
UCHAR
AddQ(void)
{
    // IF buffer is full , can't put data
    if ( isFull() == TRUE )
    {
        return FALSE;
    }
    // put in buffer
    EnQueue( );
    return TRUE;
}
//==================================================================
void cecclrint( void )
{
    //CEC_WriteI2C_Byte(REG08,  B_INT_CLR|B_DBGCEC_CLR|B_CEC_SMT);     //Reset all interrupt
    //CEC_WriteI2C_Byte(REG08,  B_DBGCEC_CLR|B_CEC_SMT);  //Reset all interrupt
    //CEC_WriteI2C_Byte(REG08, 0x00|B_CEC_SMT);
    CEC_SetI2C_Byte(0x4C, 0x3F, 0x3F);     //Reset all interrupt
}
BYTE cec_cnt100ms(void)
{
    BYTE  T100us=0;
    USHORT tick;
    USHORT ucTickCount=10;
    //verify 100ms for it6601 timeunit
    //for(T100us=0;T100us<10;T100us++)
    {
        //CEC_SetI2C_Byte( REG09, B_EN100ms_CNT, B_EN100ms_CNT );
        tick=ucTickCount;
        CEC_SetI2C_Byte( REG09, B_EN100ms_CNT, 0 );
        tick=ucTickCount-tick;
        tick = 100 - tick;

        CEC_SetI2C_Byte( REG09, B_EN100ms_CNT, B_EN100ms_CNT );
        //CEC_WriteI2C_Byte(REG09,B_EN100ms_CNT);
        delay1ms(tick);
        //CEC_WriteI2C_Byte( REG09,0x00);
        CEC_SetI2C_Byte( REG09, B_EN100ms_CNT, 0 );
        //LED3=~LED3;
        //LED2=~LED2;
        //LED1=~LED1;
    }
    T100us = ( CEC_ReadI2C_Byte(REG_MSCOUNT_H)*0x10000 + CEC_ReadI2C_Byte(REG_MSCOUNT_M)*0x100 + CEC_ReadI2C_Byte(REG_MSCOUNT_L) )/0x3E8;
    printk("\n------------------------------ \n");
    printk("tick = %x\n", (int)tick);
    printk("T100us = %x\n", T100us);
    printk("H=%x \n",CEC_ReadI2C_Byte(REG_MSCOUNT_H));
    printk("M=%x \n",CEC_ReadI2C_Byte(REG_MSCOUNT_M));
    printk("L=%x\n",CEC_ReadI2C_Byte(REG_MSCOUNT_L));
    return T100us;
}

void cecconfig( BYTE LogicAdr, BYTE TimeUnit, BYTE CECRst, BYTE EnNAck )
{
    //TimeUnit=timeunit;
    BYTE uc;
    Myself_LogicAdr=LogicAdr;

    if ( CECRst==1 )
        CEC_WriteI2C_Byte(REG08,B_CEC_RST|B_CEC_SMT);   //REG_CEC_Rst

    uc=CEC_ReadI2C_Byte(REG09);

    if ( RxSelf_Dis==1 )
        //CEC_WriteI2C_Byte(REG09, B_RXSELF_SEL);     //Reg_RxSelf_Dis[1]
        uc |= B_RXSELF_SEL;
    else
        uc &= ~B_RXSELF_SEL;

    if ( EnNAck==1 )
        //CEC_WriteI2C_Byte(REG09, uc|B_NACK_EN);     //REG_NAck_En
        uc |= B_NACK_EN;
    else
        uc &= ~B_NACK_EN;

    CEC_WriteI2C_Byte(REG09, uc);
    CEC_WriteI2C_Byte(REG_DATA_MIN, 0x00);      //REG_Data_Min[7:0]
    CEC_WriteI2C_Byte(REG_TIMER_UNIT, TimeUnit);    //REG_Timer_unit[7:0]
    CEC_WriteI2C_Byte(REG_CEC_TARGET_ADDR, LogicAdr);   //REG_Logical_addr[3:0]



    cecclrint();
#ifdef DEBUG_iT6601
    IT6601_PRINT(("\nSet Configuration : LogicAdr=%x, TimeUnit=%x, CECRst=%x, EnNAck=%x, RxSelf_Dis=%x\n\n", LogicAdr, TimeUnit, CECRst, EnNAck, RxSelf_Dis));
#endif
}

BYTE cecfirerdy( void )
{
    BYTE reg0x44;
    BYTE ReadyFire, BusFree;

    reg0x44 = CEC_ReadI2C_Byte(REG_SYS_STATUS);
    ReadyFire = (reg0x44&B_READY_FIRE)>>6;
    BusFree = (reg0x44&B_BUS_STATUS)>>1;

    if ( ReadyFire==1 && BusFree==1 )  // Ready Fire && Bus Free
        return TRUE;
    else
        return FALSE;
}

void HDMITX_CEC_Init(void)
{
#ifdef  _CEC_DEVICE_AVR_
    BYTE u8_LogicAddress=DEVICE_ID_AUDIO;// yyyyyyyyyyyyyy
#else
    #ifdef  _CEC_DEVICE_PLAYBACK_
        BYTE u8_LogicAddress=DEVICE_ID_PLAYBACK1;
    #else
        BYTE u8_LogicAddress=DEVICE_ID_TV; // rrrrrrrrrrrrrrrrrr
    #endif
#endif
    // initial Queue for save CEC Rx command
    initQ();
    initQ_Tx();

    // get 100ms timer from 8051 to synchronous iT6601 timeunit
    timeunit=(BYTE)cec_cnt100ms();

    // Set Logic Address and timeunit for iT6601 CEC function
    cecconfig(u8_LogicAddress,timeunit,TRUE,FALSE);

    CEC_WriteI2C_Byte( REG06, 0 );
    CEC_WriteI2C_Byte( REG09,CEC_ReadI2C_Byte(REG09)|0x8);

    // Reg07[6]='1' for enable CEC interrupt pin to MCU
    //CEC_WriteI2C_Byte(REG07,CEC_ReadI2C_Byte(REG07)|0x40);
    CEC_WriteI2C_Byte(REG08,CEC_ReadI2C_Byte(REG08)|0x1);

    DumpiT6601Reg();
}

void Initial_Ext_Int1()
{
// disable #ifdef _Enable_CEC_Interrupt_Pin_
// disable
// disable     MFCFGP0_0=0x02;                //HW INT1 P0.0 for connect with it6601 INT pin
// disable     IOCFGP0_0=0xa0;
// disable
// disable     MFCFGP0_1=0x02;                //HW INT1 P0.0 for connect with it6601 INT pin
// disable     IOCFGP0_1=0xa0;
// disable
// disable     PINT1EN = 0x03;
// disable     EX1         = 1;                   //Enable External Interrupt 1    use P0.0
// disable
// disable #endif
}

void iT6601Receiver(void)
{
    // [0]:TxInt, [1]:RxInt, [2]:RxFail [3]:TxDone [4]:RxDone
    CECInt = CEC_ReadI2C_Byte(REG_INT_STATUS);
    if ( CECInt&0x1F )
    {
        if ( CECInt & B_RX_INT )
        {
            if ( CECInt&B_RXDONE_INT )
            {
                AddQ();
                //LED3=~LED3;
            }
        }
        if ( CECInt&B_RXFAIL_INT )
        {
            //printf((" Result => RX Fail \n"));
           // LED2=~LED2;
        }
       //LED1=~LED1;
        cecclrint();    //Reg08[0]='1'->'0' for clear CEC int
    }
}

void cecfirecmd( void )
{
    CEC_SetI2C_Byte(REG08, B_FIRE_FRAME, 0);
    CEC_SetI2C_Byte(REG08, B_FIRE_FRAME, B_FIRE_FRAME);
}

void iT6601Transmitter(void)
{
    switch ( CECTxState )
    {
        case sCECOk:
        case sCECcmdGet:
            // IF buffer is empty , can't get data
            if ( isEmpty_Tx() == TRUE )
            {
                break;
            }
            pCECBuf_Tx = DeQueue_Tx();
            CECTxState = sCECTransfer;
        case sCECTransfer:
            {
                if ( cecfirerdy()==TRUE )
                {
                    CEC_WriteI2C_Byte(REG_CEC_OUT_NUM, (pCECBuf_Tx->SIZE));     //OutNum
                    CEC_WriteI2C_Byte(REG_TX_HEADER, pCECBuf_Tx->HEADER);   //Header
                    CEC_WriteI2C_Byte(REG_TX_OPCODE, pCECBuf_Tx->OPCODE);   //OpCode
                    CEC_WriteI2C_Byte(REG_TX_OPERAND1, pCECBuf_Tx->OPERAND1);   //Para1
                    CEC_WriteI2C_Byte(REG_TX_OPERAND2, pCECBuf_Tx->OPERAND2);   //Para2
                    CEC_WriteI2C_Byte(REG_TX_OPERAND3, pCECBuf_Tx->OPERAND3);   //Para3
                    CEC_WriteI2C_Byte(REG_TX_OPERAND4, pCECBuf_Tx->OPERAND4);   //Para4
                    CEC_WriteI2C_Byte(REG_TX_OPERAND5, pCECBuf_Tx->OPERAND5);   //Para5
                    CEC_WriteI2C_Byte(REG_TX_OPERAND6, pCECBuf_Tx->OPERAND6);   //Para6
                    CEC_WriteI2C_Byte(REG_TX_OPERAND7, pCECBuf_Tx->OPERAND7);   //Para7
                    CEC_WriteI2C_Byte(REG_TX_OPERAND8, pCECBuf_Tx->OPERAND8);   //Para8
                    CEC_WriteI2C_Byte(REG_TX_OPERAND9, pCECBuf_Tx->OPERAND9);   //Para9
                    CEC_WriteI2C_Byte(REG_TX_OPERAND10, pCECBuf_Tx->OPERAND10);     //Para10
                    CEC_WriteI2C_Byte(REG_TX_OPERAND11, pCECBuf_Tx->OPERAND11);     //Para11
                    CEC_WriteI2C_Byte(REG_TX_OPERAND12, pCECBuf_Tx->OPERAND12);     //Para12
                    CEC_WriteI2C_Byte(REG_TX_OPERAND13, pCECBuf_Tx->OPERAND13);     //Para13
                    CEC_WriteI2C_Byte(REG_TX_OPERAND14, pCECBuf_Tx->OPERAND14);     //Para14
                    CEC_WriteI2C_Byte(REG_TX_OPERAND15, pCECBuf_Tx->OPERAND15);     //Para15
                    CEC_WriteI2C_Byte(REG_TX_OPERAND16, pCECBuf_Tx->OPERAND16);     //Para16

                    cecfirecmd();
                    //CEC_WriteI2C_Byte(REG08, B_FIRE_FRAME|B_CEC_SMT);           //FireCmd
                    //OldTxCECcmd=(eCEC_CMD) CEC_ReadI2C_Byte(REG_TX_OPCODE);
                    //IT6601_PRINT(("CEC Tx Handler [ %02x ]\n",OldTxCECcmd));

                    CECTxState=sCECCheckResult;
                }
            }
            break;
        case sCECCheckResult:
            {
                u8_t    uc;
                uc=(CEC_ReadI2C_Byte(REG_SYS_STATUS)&B_OUT_STATUS);
                if ( uc==B_ReceiverFail )
                {
                    IT6601_PRINT((" Result => TX fail\n"));
                    CECTxState=sCECReceiverFail;
                }
                else if ( uc==B_ReceiverNACK )
                {
                    IT6601_PRINT(("Receiver NACK\n"));
                    CECTxState=sCECReceiverNack;
                }
                else if ( uc==B_ReceiverRetry )
                {
                    IT6601_PRINT(("Receiver Retry\n"));
                }
                else
                {
                    IT6601_PRINT(("Receiver ACK\n"));
                    if ( CEC_ReadI2C_Byte(REG_INITIATOR_TX_NUM)==pCECBuf_Tx->SIZE )
                    {
                        IT6601_PRINT((" Result => TX OK\n"));
                        CECTxState=sCECOk;
                    }
                }
            }
            break;
        case sCECReceiverNack:
        case sCECNone:
        case sCECReceiverFail:
        case sCECOff:
            break;
    }
}

HECStateField   HEC_Status;     // 1 byte
HECSupportField HEC_Support;    // 2 byte
HECActivationField  HEC_Activation; // 2 byte


// for iT6633+iT6623void CEC_InactiveSource(BYTE port)
// for iT6633+iT6623{
// for iT6633+iT6623
// for iT6633+iT6623#ifdef     _CEC_DEVICE_AVR_
// for iT6633+iT6623        CECCmdSet(DEVICE_ID_TV,eInactiveSource,txphyadr[0],txphyadr[1]);
// for iT6633+iT6623#else
// for iT6633+iT6623    if(rxcurport==port)
// for iT6633+iT6623        CECCmdSet(DEVICE_ID_TV,eInactiveSource,rxphyadr[rxcurport][0],rxphyadr[rxcurport][1]);
// for iT6633+iT6623#endif
// for iT6633+iT6623}
// for iT6633+iT6623void CEC_RoutingChange(BYTE port)
// for iT6633+iT6623 {
// for iT6633+iT6623    //when Source un-plug(w/o +5v ) then broadcast AVR routing change.
// for iT6633+iT6623
// for iT6633+iT6623    BYTE    adr0,adr1;
// for iT6633+iT6623
// for iT6633+iT6623     if(Myself_LogicAdr==DEVICE_ID_AUDIO)
// for iT6633+iT6623     {
// for iT6633+iT6623
// for iT6633+iT6623        if(port<4)
// for iT6633+iT6623        {
// for iT6633+iT6623             adr0=rxphyadr[rxcurport][0];
// for iT6633+iT6623             adr1=rxphyadr[rxcurport][1];
// for iT6633+iT6623            rxcurport=port;
// for iT6633+iT6623            CECCmdSet(DEVICE_ID_BROADCAST,eRoutingChange,adr0,adr1);
// for iT6633+iT6623        }
// for iT6633+iT6623     }
// for iT6633+iT6623 }

void CECCmdSet(u8_t Follower,eCEC_CMD TxCommand,u8_t cOperand1,u8_t cOperand2)
{
    //FollowerAddr=Follower;
    //TxCmdBuf.HEADER=((Myself_LogicAdr<<4)+FollowerAddr);                // logice address + follower address
    TxCmdBuf.HEADER=((Myself_LogicAdr<<4)+Follower);                // logice address + follower address
    TxCmdBuf.OPCODE=TxCommand;
    TxCmdBuf.OPERAND1=cOperand1;            // Physical Address 1
    TxCmdBuf.OPERAND2=cOperand2;            // Physical Address 2
    TxCmdBuf.SIZE=2;

    switch ( TxCommand )
    {
        case eFeatureAbort:
            TxCmdBuf.SIZE=4;
            break;
        case ePollingMessage:
#ifdef DEBUG_iT6601
            IT6601_PRINT(("PollingMessage Header=%02x \n",TxCmdBuf.HEADER));
#endif
            TxCmdBuf.SIZE=1;
            break;
        case eTextViewOn:           //            =0x0D,    // follower:TV
        case eImageViewOn:          //            =0x04,    // follower:TV, switch    --> Broadcst
            break;
        case eActiveSource:         //            =0x82,    // follower:TV, switch    --> Broadcst , Directly address
            TxCmdBuf.SIZE=4;
            break;
            // Routing Control Feature
        case eRoutingChange:            //            =0x80,    // follower:TV, switch    --> Broadcst
            TxCmdBuf.OPERAND3=rxphyadr[rxcurport][0];           // Physical Address 1
            TxCmdBuf.OPERAND4=rxphyadr[rxcurport][1];           // Physical Address 2
            TxCmdBuf.SIZE=6;
            break;
        case eRoutingInformation:       //            =0x81,    // follower:switch        --> Broadcst
            TxCmdBuf.SIZE=4;
            break;
        case eRequestActiveSource:      //            =0x85,    // follower:switch        --> Broadcst     , AVR to request active source when Audio ocntrol feature is active
            break;
        case eSetStreamPath:            //            =0x86,    // follower:switch        --> Broadcst
            TxCmdBuf.SIZE=4;
            break;
        case eInactiveSource:           //            =0x9D,    //
            //reply TV that it has no video to be presented !!
            TxCmdBuf.SIZE=4;
            break;
        case eStandBy:              //            =0x36,    // follower:All            --> Broadcst
            break;
            // System Information Feature
        case eCECVersioin:          //            =0x9E,
            TxCmdBuf.SIZE=3;
            break;
        case eGetCECVersion:            //            =0x9F,
        case eGivePhysicalAddress:      //            =0x83,    // follower:All
        case eGetMenuLanguage:      //            =0x91,    // follower:TV
        case eReportPhysicalAddress:    //            =0x84,    // follower:TV
            {
                TxCmdBuf.SIZE=5;
                switch ( Myself_LogicAdr )
                {
                    case DEVICE_ID_TV:
                        TxCmdBuf.OPERAND3=0;
                        break;
                    case DEVICE_ID_RECORDING1:
                    case DEVICE_ID_RECORDING2:
                    case DEVICE_ID_RECORDING3:
                        TxCmdBuf.OPERAND3=1;
                        break;
                    case DEVICE_ID_RESERVED1:
                    case DEVICE_ID_RESERVED2:
                    case DEVICE_ID_FREEUSE:
                    case DEVICE_ID_BROADCAST:
                        TxCmdBuf.OPERAND3=2;
                        break;
                    case DEVICE_ID_TUNER1:
                    case DEVICE_ID_TUNER2:
                    case DEVICE_ID_TUNER3:
                    case DEVICE_ID_TUNER4:
                        TxCmdBuf.OPERAND3=3;
                        break;
                    case DEVICE_ID_PLAYBACK1:
                    case DEVICE_ID_PLAYBACK2:
                    case DEVICE_ID_PLAYBACK3:
                        TxCmdBuf.OPERAND3=4;
                        break;
                    case DEVICE_ID_AUDIO:
                        TxCmdBuf.OPERAND3=5;
                        break;
                    default:
                        TxCmdBuf.OPERAND3=6;    //CEC switch
                        TxCmdBuf.OPERAND3=7;    //Video processor
                        break;
                }
            }
            break;
        case eSetMenuLanguage:      //            =0x32,    // follower:All,        Initiator:TV
            // Vendor Specific Commands Feature
        case eDeviceVendorID:       //            =0x87,
            TxCmdBuf.OPERAND1=0;    //CEC switch
            TxCmdBuf.OPERAND2=0;    //CEC switch
            TxCmdBuf.OPERAND3=0;    //CEC switch
            TxCmdBuf.SIZE=5;
            break;
        case eGiveDeviceVendorID:       //            =0x8C,
        case eVendorCommand:        //            =0x89,
        case eVendorCommandWithID:  //             =0xA0,
            break;
        case eReportPowerStatus:
            TxCmdBuf.SIZE=3;
            break;
            // other
        case ePlay:                 //            =0x41,
        case eUserPressed:          //            =0x44,
            TxCmdBuf.SIZE=3;
            break;
        case eUserReleased:         //            =0x45,
        case eAudioModeRequest:     //            =0x70,
            break;
        case eGiveAudioStatus:      //            =0x71,
            break;
        case eSetSystemAudioMode:   // <MS> <0x72 Set Audio Mode> <System Audio On / Off >
            //                             <--  <MS> <0x70> <Phy Adr>
            TxCmdBuf.SIZE=3;
            break;
        case eReportAudioStatus:        // <MS> <0x7A Report Audio Status> <Volume and Mute status>
            //                            <--  <MS> <0x71 Give Audio Status>
        case eSystemAudIoModeStatus:    // <MS> <0x7E System AudIo Mode Status> <Audio On / Off>
            //                            <--  <MS> <0x7D Give System Audio Mode Status>
            TxCmdBuf.SIZE=3;
            break;
        case eRequestAudioDescriptor:{
                TxCmdBuf.OPERAND1=0x01; // PCM
                TxCmdBuf.OPERAND2=0x02; // AC3
                TxCmdBuf.OPERAND3=0x07; // DTS
                TxCmdBuf.SIZE=5;
            }
            break;
        case eReportAudioDescriptor:    //            =0xA3
            //0x23,0x0F, 0x7F, 0x07,        //PCM 8CH, 32~192KHz, 16/20/24Bit
            //0x23,0x15, 0x1F, 0x38,        //AC3 6CH, 32~96KHz, 16/20/24Bit
            //0x23,0x3F, 0x07, 0xC0,        //DTS 8CH, 32~ 48K, 1536Kbps
            TxCmdBuf.OPERAND1=0x0F;
            TxCmdBuf.OPERAND2=0x7F;
            TxCmdBuf.OPERAND3=0x07;
            TxCmdBuf.OPERAND4=0x15;
            TxCmdBuf.OPERAND5=0x1F;
            TxCmdBuf.OPERAND6=0x38;
            TxCmdBuf.OPERAND7=0x3F;
            TxCmdBuf.OPERAND8=0x07;
            TxCmdBuf.OPERAND9=0xC0;
            TxCmdBuf.SIZE=11;
            break;
        case eGiveSystemAudIoModeStatus:    //            =0x7D,
        case eRemoteButtonDown:     //            =0x8A,
        case eRemoteButtonUp:       //            =0x8B,
            break;
            // ACR
        case eInitiateARC:              //            =0xC0,
        case eReportARCInitiated:       //            =0xC1,
        case eReportARCTerminated:  //            =0xC2,
        case eRequestARCInitiation:     //            =0xC3,
        case eRequestARCTermination:    //            =0xC4,
        case eTerminateARC:         //            =0xC5,
            break;
    }
#ifdef DEBUG_iT6601
    printk("CECCmdSet --> Tx Header:%x, opcode:%x , operand1:%x , operand2:%x ,Size:%x-----\n ",
           TxCmdBuf.HEADER,
           TxCmdBuf.OPCODE,
           TxCmdBuf.OPERAND1,
           TxCmdBuf.OPERAND2,
           TxCmdBuf.SIZE
          );
#endif
    AddQ_Tx();
    CECTxState=sCECcmdGet;
}

//11.1.2-1 Routing Control              [AVR: 11.2.2-1 ~ 11.2.2-4]
//11.1.3-1 System Standby               [AVR: 11.2.3-1 ~ 11.2.3-3]
//11.2.9-1 Vendor Specific Commands     [AVR: 11.2.9-1 ~ 11.2.9-3]
//11.2.12-1 Device Menu Control         [AVR: 11.2.12-1 ~ 11.2.12-7]
//11.2.13-1 Remote Control Pass Through [AVR: 11.2.13-1]
//11.3.1-1 CEC Switch                   [AVR: 11.3.1-1 ~ 11.3.2-1]
//12-1 Invalid Massage
void CECDecoder(void)
{
    u8_t uc;
    u8_t index;
    BOOL bInvalidCmd=FALSE;

    // IF buffer is empty , can't get data
    if ( isEmpty() == TRUE )
    {
        return;
    }

    pCECRxBuf = DeQueue();

#ifdef DEBUG_iT6601
    switch ( pCECRxBuf->SIZE )
    {
        default:
        case 18:
            IT6601_PRINT(("OPERAND16=%02x \n",pCECRxBuf->OPERAND16));
        case 17:
            IT6601_PRINT(("OPERAND15=%02x \n",pCECRxBuf->OPERAND15));
        case 16:
            IT6601_PRINT(("OPERAND14=%02x \n",pCECRxBuf->OPERAND14));
        case 15:
            IT6601_PRINT(("OPERAND13=%02x \n",pCECRxBuf->OPERAND13));
        case 14:
            IT6601_PRINT(("OPERAND12=%02x \n",pCECRxBuf->OPERAND12));
        case 13:
            IT6601_PRINT(("OPERAND11=%02x \n",pCECRxBuf->OPERAND11));
        case 12:
            IT6601_PRINT(("OPERAND10=%02x \n",pCECRxBuf->OPERAND10));
        case 11:
            IT6601_PRINT(("OPERAND9=%02x \n",pCECRxBuf->OPERAND9));
        case 10:
            IT6601_PRINT(("OPERAND8=%02x \n",pCECRxBuf->OPERAND8));
        case 9:
            IT6601_PRINT(("OPERAND7=%02x \n",pCECRxBuf->OPERAND7));
        case 8:
            IT6601_PRINT(("OPERAND6=%02x \n",pCECRxBuf->OPERAND6));
        case 7:
            IT6601_PRINT(("OPERAND5=%02x \n",pCECRxBuf->OPERAND5));
        case 6:
            IT6601_PRINT(("OPERAND4=%02x \n",pCECRxBuf->OPERAND4));
        case 5:
            IT6601_PRINT(("OPERAND3=%02x \n",pCECRxBuf->OPERAND3));
        case 4:
            IT6601_PRINT(("OPERAND2=%02x \n",pCECRxBuf->OPERAND2));
        case 3:
            IT6601_PRINT(("OPERAND1=%02x \n",pCECRxBuf->OPERAND1));
        case 2:
            IT6601_PRINT(("OPCODE=%02x \n",pCECRxBuf->OPCODE));
        case 1:
            IT6601_PRINT(("HEADER=%02x \n",pCECRxBuf->HEADER));
        case 0:
            IT6601_PRINT(("SIZE =%02x \n",pCECRxBuf->SIZE));
            break;
    }
#endif

    FollowerAddr=(pCECRxBuf->HEADER & 0x0F);    // Get Follower logic address ;
    IT6601_PRINT(("FollowerAddr=%02x \n", FollowerAddr));
    if ( pCECRxBuf->SIZE==1 )
    {
        if ( Myself_LogicAdr==(pCECRxBuf->HEADER &0x0F) )
            switch_PollingMessage(sReportPhyAdr);
        return;
    }

    // Invalid command Check
    for ( index=0;index<SizeOfLookUpTable_CEC;index++ )
    {
        if ( (pCECRxBuf->OPCODE)==LookUpTable_CEC[index].cmd )
            break;
    }

    IT6601_PRINT(("SizeOfLookUpTable_CEC=%02x \n", index));
    if ( index!=(SizeOfLookUpTable_CEC-1) )
    {
        switch ( LookUpTable_CEC[index].header )
        {
            case eDirectly:
                if ( FollowerAddr==DEVICE_ID_BROADCAST )
                {
                    bInvalidCmd=TRUE;
                    IT6601_PRINT(("!!!Invalid Direct Cmd !!!\n"));
                }
                break;
            case eBroadcast:
                if ( FollowerAddr!=DEVICE_ID_BROADCAST )
                {
                    bInvalidCmd=TRUE;
                    IT6601_PRINT(("!!!Invalid Broadcast Cmd !!!\n"));
                }
                break;
            case eBoth:
                break;
        }

        if ( (LookUpTable_CEC[index].size)!=(pCECRxBuf->SIZE) )
        {
            bInvalidCmd=TRUE;
            IT6601_PRINT(("!!!Invalid Size !!!\n"));
        }
    }
    FollowerAddr=(pCECRxBuf->HEADER & 0xF0)>>4; // Get initator logic address for transfer CEC cmd
    IT6601_PRINT(("initator addr =%02x \n", FollowerAddr));
    // CEC command  decode handler
    switch ( pCECRxBuf->OPCODE )
    {
//=== 12.3 Feature Abort  ====
        case eFeatureAbort :    // <0x00> ,
            IT6601_PRINT(("Receive !!!Feature Abort !!!\n"));
            break;
//=== 12.4  Abort  ====
        case eAbort :       // <MS><0xff> , //example: [0x45 0xff] playback device to AVR
            //for CEC ATC 8-x , 9-x test only, shall respond with a <Feature Abort> message
            {
                IT6601_PRINT(("Receive !!!About !!!\n"));
                uc=(pCECRxBuf->HEADER & 0xf0)>>4;       // get follower logic address
                //if(Myself_LogicAdr==uc)
                {
                    CECCmdSet(uc, eFeatureAbort,eAbort,4);  //<MS> <featuer about> <op code> <abort reason>
                }
            }
            break;
//=== 13.1 One Touch Play ===
        case eImageViewOn : // <MS><0x04> , //example: [0x40 0x04] , Playback device to TV
            IT6601_PRINT(("Receive !!!Image View On !!!\n"));
            break;
        case eTextViewOn :  // <MS><0x0D>, example: [0x40 0x0D] , Playback device to TV
            IT6601_PRINT(("Receive !!!Text View On !!!\n"));
            break;
        case eActiveSource :    //<DB> <0X82> <Physical Address of initator> ,example:[0x4f 0x82 AB CD] playback device[AB CD] broadcast acitve source
            IT6601_PRINT(("Receive !!!Active Source !!!\n"));
            break;
//=== 13.2.1 Routing Control ====
        case eRoutingChange :       //<DB><0x80><Original Adr><New Adr>  , size=6        // send by switch device to manually change port
            //reply new address <routine information> to indicate its current active route.
            IT6601_PRINT(("Receive !!!Routing Change> !!!\n"));
            if ( txphyadr[0]==pCECRxBuf->OPERAND3 && txphyadr[1]==pCECRxBuf->OPERAND4 )
            {
                //CECCmdSet(DEVICE_ID_BROADCAST, eRoutingInformation,rxphyadr[rxcurport][0],rxphyadr[rxcurport][1]);
            }
            break;
        case eRoutingInformation :  //<DB> <0x81> <Physical Address> ,example:[0x5F 0x81 AB CD]  AVR get active route below the switch
            IT6601_PRINT(("Receive !!!Rounting Information !!!\n"));
            if ( txphyadr[0]==pCECRxBuf->OPERAND1 && txphyadr[1]==pCECRxBuf->OPERAND2 )
                CECCmdSet(DEVICE_ID_BROADCAST, eRoutingInformation,rxphyadr[rxcurport][0],rxphyadr[rxcurport][1]);
            else
                IT6601_PRINT(("Invalid -->eRoutingInformation \n"));
            break;
        case eInactiveSource :      //<MS> <0x9D> <Physical Address>        example:<0x40> <0x9D> <AB> <CD>
            // It has no video to be presented to the user, or is goning into standby state.
            IT6601_PRINT(("Receive !!!Inactive Source !!!\n"));
            if ( Myself_LogicAdr==DEVICE_ID_TV )   // for TV only !!!
            {
                //uc=(pCECRxBuf->HEADER & 0x0f);
                //f(Myself_LogicAdr==uc)
                {
                    uc=(((pCECRxBuf->HEADER) & 0xf0)>>4);   //get initator logice address
                    if ( CECList[uc].Active==TRUE )
                    {
                        IT6601_PRINT(("CECList[%02x].Active -> FALSE\n",uc));
                        CECList[uc].Active=FALSE;
                        for ( uc=1;uc<15;uc++ )
                        {
                            if ( CECList[uc].Active==TRUE )
                                CECCmdSet(uc, eSetStreamPath,CECList[uc].PhyicalAddr1,CECList[uc].PhyicalAddr2);    //Auto change to active source !!!
                        }
                    }
                }
            }
            break;
        case eRequestActiveSource : // <DB> <0x85> , need to reply <active source> by CEC active device
            IT6601_PRINT(("Receive !!!Request Active Source !!!\n"));
            {
                //uc=(pCECRxBuf->HEADER & 0xf0)>>4;
                CECCmdSet(DEVICE_ID_BROADCAST, eActiveSource,txphyadr[0],txphyadr[1]);
//                if(Myself_LogicAdr==DEVICE_ID_AUDIO)
//                {
//                    CECCmdSet(DEVICE_ID_BROADCAST, eActiveSource,txphyadr[0],txphyadr[1]);
//                }
//                else if( Myself_LogicAdr==DEVICE_ID_PLAYBACK1 || Myself_LogicAdr==DEVICE_ID_PLAYBACK2
//                            || Myself_LogicAdr==DEVICE_ID_PLAYBACK3)
//                {
//                    //uc=(pCECRxBuf->HEADER & 0xf0)>>4;
//                    CECCmdSet(DEVICE_ID_BROADCAST, eActiveSource,txphyadr[0],txphyadr[1]);
//                }
            }
            break;
        case eSetStreamPath :   // <DB> <0x86> <Physical Address> , example:[0x0F 0x86 AB CD] TV request a streaming path from the specified Physical Address
            {
                IT6601_PRINT(("Receive !!!Set Stream Path !!!\n"));
                if ( Myself_LogicAdr==DEVICE_ID_AUDIO )
                {
#if 0
                    //fail with QD882E test ???
                    //tv send eSetStreamPath then AVR check txphyadr and reply active source by child rxphyadr
                    if ( (txphyadr[0]==pCECRxBuf->OPERAND1) && (txphyadr[1]==pCECRxBuf->OPERAND2) )
                    //    CECCmdSet(DEVICE_ID_BROADCAST, eActiveSource,rxphyadr[rxcurport][0],txphyadr[rxcurport][1]);
                    {
                        if ( rxcurport<4 )
                            //CECCmdSet(DEVICE_ID_BROADCAST, eActiveSource,rxphyadr[rxcurport][0],rxphyadr[rxcurport][1]);
                            CECCmdSet(DEVICE_ID_BROADCAST, eActiveSource,txphyadr[0],txphyadr[1]);
                    }
#else
                    CECChangePort();
#endif
                }
                else if ( Myself_LogicAdr==DEVICE_ID_PLAYBACK1 || Myself_LogicAdr==DEVICE_ID_PLAYBACK2
                          || Myself_LogicAdr==DEVICE_ID_PLAYBACK3 )
                {
                    //if(rxcurport<4)
                    //    CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,txphyadr[0],txphyadr[1]);
                    Switch_OneTouchPlayState(sImageViewOn);
                }
            }
            break;
//=== 13.3 System Stand by ===
        case eStandBy : // <MS> <0X36> or <DB> <0x36> ,
            IT6601_PRINT(("nReceive !!!Standby !!!\n"));
            break;
//=== 13.6 System Information ===
        case eCECVersioin :     //<MS> <0x9E> <CEC version>
            IT6601_PRINT(("Receive !!!CEC Version =%02x!!!\n",pCECRxBuf->OPERAND1));
            break;
        case eGetCECVersion :       //<MS> <0x9F> , example: [0x05 0x9F] TV request AVR to reply CEC version
            IT6601_PRINT(("Receive !!!Report CEC Version !!!\n"));
            uc=(pCECRxBuf->HEADER & 0xf0)>>4;
           CECCmdSet(uc, eCECVersioin,0x04,0); // 5 for Version 1.4a... 4 for Version 1.3A...0~3 for Reserved
            break;
        case eGivePhysicalAddress :    //<MS> <0x83> , example:[0x05 0x83] TV request AVR to return Physical address
            IT6601_PRINT(("Receive !!!Give Physical Address !!!\n"));
#if 0
            if ( Myself_LogicAdr==DEVICE_ID_PLAYBACK1 || Myself_LogicAdr==DEVICE_ID_PLAYBACK2
                 || Myself_LogicAdr==DEVICE_ID_PLAYBACK3 )
            {
                switch_PollingMessage(sPollingMessage1);
            }
            else if ( Myself_LogicAdr==DEVICE_ID_AUDIO )
            {
                CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,txphyadr[0],txphyadr[1]);
            }
#endif
            if ( Myself_LogicAdr==DEVICE_ID_TV )
            {
                //Switch_LogicalAddressingState(sReportPhysicalAddressTransfer);
                CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,0x00,0x00);   //TV always 0,0,0,0
            }
            else if ( Myself_LogicAdr==DEVICE_ID_AUDIO )
            {
                // when HPD On and EDID change  --> then Device need to polling message and report physical address
                // when receiver eGivePhysicalAddress cmd --> then direct to response eReportPhysicalAddress !!!
                CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,txphyadr[0],txphyadr[1]);
            }
            else
            {
                //switch_PollingMessage(sPollingMessage1);        // need to check !!
                CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,txphyadr[0],txphyadr[1]);
            }
            break;

        case eReportPhysicalAddress :   // <DB> <0x84> <Physicall Adress> <Device type>, example:[0x5F 0x84 AB CD 0]
            //if(Myself_LogicAdr==DEVICE_ID_TV){
            //    //need to collect all CEC device Physical address and Device type
            //    Switch_LogicalAddressingState(sReportPhysicalAddressReceived);    //2010/12/21
            //}else
            IT6601_PRINT(("Receive !!!Report Physical Address !!!\n"));
            {
                if ( (pCECRxBuf->HEADER & 0x0f)==0x0f )
                {
                    uc=((pCECRxBuf->HEADER & 0xf0)>>4);
                    CECList[uc].PhyicalAddr1=pCECRxBuf->OPERAND1;
                    CECList[uc].PhyicalAddr2=pCECRxBuf->OPERAND2;
                    CECList[uc].Active=TRUE;
                }
            }
            break;
        case 0x91 : //Get Menu Language
            uc=(pCECRxBuf->HEADER & 0xf0)>>4;       // get follower logic address
            CECCmdSet(uc, eFeatureAbort,0x91,4);  //<MS> <featuer about> <op code> <abort reason>
            IT6601_PRINT(("Receive !!!Get Menu Language !!!\n"));
            break;
        case 0x32 : //Set Menu Language
            uc=(pCECRxBuf->HEADER & 0xf0)>>4;       // get follower logic address
            CECCmdSet(uc, eFeatureAbort,0x32,4);  //<MS> <featuer about> <op code> <abort reason>
            IT6601_PRINT(("Receive !!!Set Menu Language !!!\n"));
            break;
//=== 13.7 Deck Control ===
        case 0x41 : //Play
            uc=(pCECRxBuf->HEADER & 0xf0)>>4;       // get follower logic address
            CECCmdSet(uc, eFeatureAbort,0x41,4);  //<MS> <featuer about> <op code> <abort reason>
            IT6601_PRINT(("Receive !!!Play !!!\n"));
            break;
        case 0x42 : //Deck control
            uc=(pCECRxBuf->HEADER & 0xf0)>>4;       // get follower logic address
            CECCmdSet(uc, eFeatureAbort,0x42,4);  //<MS> <featuer about> <op code> <abort reason>
            IT6601_PRINT(("Receive !!!Deck control !!!\n"));
            break;
        case 0x1A : //Give Deck Status
            uc=(pCECRxBuf->HEADER & 0xf0)>>4;       // get follower logic address
            CECCmdSet(uc, eFeatureAbort,0x1A,4);  //<MS> <featuer about> <op code> <abort reason>
            IT6601_PRINT(("Receive !!!Give Deck Status !!!\n"));
            break;
        case 0x1B : //Deck Status
            uc=(pCECRxBuf->HEADER & 0xf0)>>4;       // get follower logic address
            CECCmdSet(uc, eFeatureAbort,0x1B,4);  //<MS> <featuer about> <op code> <abort reason>
            IT6601_PRINT(("Receive !!!Deck Status !!!\n"));
            break;
//=== 13.9 Vendor Specfic Commands ===
        case 0xA0 : //Vendor Command With ID            // Vendor Specific
            IT6601_PRINT(("Receive !!!Vendor Command With ID !!!\n"));
            break;
        case eDeviceVendorID :      //<DB> <0x87> <Vendor ID>
            IT6601_PRINT(("Receive !!!Device Vendor ID !!!\n"));
            break;
        case eGiveDeviceVendorID :  //<MS> <0x8C>    //need to return <Device Vendor ID>
            IT6601_PRINT(("Receive !!!Give Device Vendor ID !!!\n"));
            //uc=(pCECRxBuf->HEADER & 0xf0)>>4;
            CECCmdSet(DEVICE_ID_BROADCAST, eDeviceVendorID,0,0);
            break;
        case 0x89 :     //Vendor Command                // Vendor Specific
            IT6601_PRINT(("Receive !!!Vendor Command !!!\n"));
            break;
        case 0x8A :     //Vendor Remote Button Down        // Vendor Specific
            IT6601_PRINT(("Receive !!!Vendor Remote Button Down !!!\n"));
            break;
        case 0x8B :     //Vendor Remote Button Up        // Vendor Specific
            IT6601_PRINT(("Receive !!!Vendor Remote Button Up !!!\n"));
            break;
//=== 13.12 Device Menu Control =========
//MenuRequest
//MenuStatus
//=== 13.13 Remote Control Pass Through ===
        case eUserPressed : //<MS> <0x44> <UI Command>
            IT6601_PRINT(("Receive !!!User Control Pressed !!!\n"));
            UICommandCode=pCECRxBuf->OPERAND1;      // SAVE UI command code
            break;
        case eUserReleased :    //<MS> <0X45>
            IT6601_PRINT(("Receive !!!User Control Released !!!\n"));
            CECUICommandCode(UICommandCode);        // executioin UI command code
            break;
//=== 13.14 Device Power Status ===
        case eGiveDevicePowerStatus:{   // <MS> <0x8F Give Device Power Status> --> <MS> <0x90 Report Power Status> < 0~3 >
                IT6601_PRINT(("Receive !!!Give Device Power Status !!!\n"));
                uc=(pCECRxBuf->HEADER & 0xf0)>>4;   //for <MS> Header , get initator logic address then reply cec cmd
                CECCmdSet(uc, eReportPowerStatus,u8_PowerStatus,0); // 0: on , 1: standby, 2: in transition standby to on , 3: in transition on to standby
            }
            break;
        case eReportPowerStatus:
            IT6601_PRINT(("Receive !!!Report Power Status !!!\n"));
            break;
//=== 13.15.1 System Audio Control ====
        case eAudioModeRequest :{           //<MS> <0x70 Audio Mode Request > <Phy Adr> ,example:[0x05 0x70 00 00] TV send Audio mode request ON to AVR
                //<MS> <0x70 Audio Mode Request >         ,example:[0x05 0x70] TV send Audio mode request OFF to AVR
                IT6601_PRINT(("Receive !!!System Audio Mode Request !!!\n"));
                //CTS 11.2.15 Audio system control
                //
                uc=(pCECRxBuf->HEADER & 0xf0)>>4;   // Get initator logic address
                if ( pCECRxBuf->SIZE==4 )
                {
                    bSystemAudioMode=1; // Initating the system audio control
                    if ( uc==DEVICE_ID_TV )
                        CECCmdSet(DEVICE_ID_BROADCAST,eSetSystemAudioMode,bSystemAudioMode,0);  // AVR reply  [0x05 0x72  and Audio On / Off]
                    else
                        Switch_SystemAudioControl(sSetSystemAudioModeToTV);
                    //need to un-mute Amplifer speaker
                }
                else
                {
                    bSystemAudioMode=0; //Terminating the system audio control
                    CECCmdSet(DEVICE_ID_BROADCAST,eSetSystemAudioMode,bSystemAudioMode,0);  // AVR reply  [0x05 0x72  and Audio On / Off]
                    //need to mute Amplifer speaker
                }
            }
            break;
        case eGiveAudioStatus:{         //<MS> <0x71>
                IT6601_PRINT(("Receive !!!Give Audio Status !!!\n"));
                uc=(pCECRxBuf->HEADER & 0xf0)>>4;   // Get initator logic address
                CECCmdSet(uc,eReportAudioStatus,uCECAudioStatus.AStatus,0); //AVR reply  Report Audio Status [0x05 0x7A and Volume and Mute status]
            }
            break;
        case eSetSystemAudioMode :{     //<MS> <0X72> <Audio on/off>
                IT6601_PRINT(("Receive Set System Audio Mode !!!\n"));
                if ( pCECRxBuf->OPERAND1==0 )
                    bSystemAudioMode=1; // AVR audio mode ON
                else
                    bSystemAudioMode=0; // AVR audio mode OFF
            }
            break;
        case eReportAudioStatus:{           //<MS> <0x7A> <Volume and Mute Status>
                IT6601_PRINT(("Receive !!!Report Audio Status !!!\n"));
                uCECAudioStatus.AStatus=pCECRxBuf->OPERAND1;
            }
            break;
        case eGiveSystemAudIoModeStatus :{//<MS> <0x7D >  --> <MS> <0x7E > <Audio On / Off>
                IT6601_PRINT(("Receive !!!Give System Audio Mode Status !!!\n"));
                uc=(pCECRxBuf->HEADER & 0xf0)>>4;   // Get initator logic address
                CECCmdSet(uc,eSystemAudIoModeStatus,bSystemAudioMode,0);    //<Audio On / Off>
            }
            break;
        case eSystemAudIoModeStatus :   { //<MS> <0x7E> <System Audio Status>
                IT6601_PRINT(("Receive !!!System Audio Mode Status !!!\n"));
                if ( pCECRxBuf->OPERAND1==0 )
                    bSystemAudioMode=1; // AVR audio mode ON
                else
                    bSystemAudioMode=0; // AVR audio mode OFF
            }
            break;
        case eRequestAudioDescriptor :{ //<MS> <0xA4> <Audio Format ID and Code>
                IT6601_PRINT(("Receive !!!Request Short Audio Descriptor !!!\n"));
                uc=(pCECRxBuf->HEADER & 0xf0)>>4;   // Get initator logic address
                CECCmdSet(uc,eReportAudioDescriptor,0,0);   // Report EDID Audio Block !!!
            }
            break;
//=== ARC ====
        case eInitiateARC :
            printk("Receive !!!Initiate ARC !!!\n");
//            if(CheckUpDownPhyAdr()==TRUE)
            {
                switch_ARCState(sReportARCInitiated);
            }
            break;
        case eReportARCInitiated :
            printk("Receive !!!Report ARC Initiated !!!\n");
            break;
        case eReportARCTerminated :
            printk("Receive !!!Report ARC Terminated !!!\n");
//            uc=IT6601_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
//
//            //if(Myself_LogicAdr==0)
//            //    uc &= ~(B_ARC_TX_EN);
//            //else
//            //    uc &=~(B_ARC_RX_EN|B_SPDIFO_EN);
//            //uc&=0xFC;    //2010/12/21 for test , disable ARC rx and tx  , not yet !!!
//
//            IT6601_WriteI2C_Byte(REG_HEAC_CTRL,uc);
//            DumpiT6601Reg();
            break;
        case eRequestARCInitiation :
            printk("Receive !!!Request ARC Initiation !!!\n");
//            if(CheckUpDownPhyAdr()==TRUE)
            {
                //FollowerAddr=DEVICE_ID_BROADCAST;
                switch_ARCState(sInitiateARC);
            }
            break;
        case eRequestARCTermination :    //Request ARC Termination
            printk("Receive !!!Request ARC Termination !!!\n");
            if ( CheckUpDownPhyAdr()==TRUE )
            {
                switch_ARCState(sTerminateARC);
            }
            break;
        case eTerminateARC :    //Termiate ARC
            printk("Receive !!!Termiate ARC !!!\n");
            switch_ARCState(sReportARCTermination);
            break;
//=== HEC ===
        case eCDC:
            switch ( pCECRxBuf->OPERAND3 )
            {
                case eHEC_InquirState:
                    IT6601_PRINT(("\nReceive !!!CEC_HEC_InquireState !!!\n"));
//                    SwitchHECState(sHEC_InquirState);
                    break;
                case eHEC_ReportState:
                    IT6601_PRINT(("\nReceive !!!CEC_HEC_Report_State !!!\n"));
                    SwitchHECState(sHEC_ReportState);
                    break;
                case eHEC_SetStateAdjacent:
                    IT6601_PRINT(("\nReceive !!!CEC_HEC_SetStateAdjacent !!!\n"));
                    SwitchHECState(sHEC_SetStateAdjacent);
                    break;
                case eHEC_SetState:
                    IT6601_PRINT(("\nReceive !!!CDC_HEC_SetState !!!\n"));
                    SwitchHECState(sHEC_SetState);
                    break;
                case eHEC_RequestDeactivation:
                    IT6601_PRINT(("\nReceive !!!CEC_HEC_RequestDeactivation !!!\n"));
                    SwitchHECState(sHEC_RequestDeactivation);
                    break;
                case eHEC_NotifyAlive:
                    IT6601_PRINT(("\nReceive !!!CEC_HEC_NotifyAlive !!!\n"));
                    SwitchHECState(sHEC_NotifyAlive);
                    break;
                case eHEC_Discover:
                    IT6601_PRINT(("\nReceive !!!CEC_HEC_Discover !!!\n"));
                    SwitchHECState(sHEC_Discover);
                    break;
                case eHEC_HPDSetState:
                    IT6601_PRINT(("\nReceive !!!CDC_HPD_SetState !!!\n"));
                    SwitchHECState(sHEC_HPDSetState);
                    break;
                case eHEC_HPDReportState:
                    IT6601_PRINT(("\nReceive CEC_HDP_ReportState !!!\n"));
                    SwitchHECState(sHEC_HPDReportState);
                    break;
                default :
                    IT6601_PRINT(("\nReceive CDC UnRecongnized OpCode !!!\n"));
                    break;
            }
            break;
        default :
            IT6601_PRINT(("\nReceive UnRecongnized OpCode !!!\n"));
            break;
    }
    IT6601_PRINT(("!!***************************** %x **************************************************\n \n",CMDCount++));
}

void Switch_OneTouchPlayState(OneTouchPlay_StateType uState)
{
    IT6601_PRINT(("Switch_OneTouchPlayState--> %02x \n",uState));
//    if(Myself_LogicAdr!=DEVICE_ID_AUDIO  && Myself_LogicAdr!=DEVICE_ID_TV)
    if ( Myself_LogicAdr!=DEVICE_ID_TV )
    {
        OTState=uState;
        switch ( uState )
        {

            case sImageViewOn:          //            =0x04,    // follower:TV, switch    --> Broadcst
                CECCmdSet(DEVICE_ID_TV,eImageViewOn,0,0);
                break;
            case sActiveSource:         //            =0x82,    // follower:TV, switch    --> Broadcst , Directly address
                if ( Myself_LogicAdr!=DEVICE_ID_TV ) //&& Myself_LogicAdr!=DEVICE_ID_AUDIO)
                    CECCmdSet(DEVICE_ID_BROADCAST, eActiveSource,txphyadr[0],txphyadr[1]);
                break;
            case sTextViewOn:           //            =0x0D,    // follower:TV
                CECCmdSet(DEVICE_ID_BROADCAST,eTextViewOn,0,0);
                break;
            default:
                break;
        }
    }
}

void OneTouchPlayHandler(void)
{
    switch ( OTState )
    {

        case sImageViewOn:          //            =0x82,    // follower:TV, switch    --> Broadcst , Directly address
            {
                if ( OldTxCECcmd==eImageViewOn )
                {
                    switch ( CECTxState )
                    {

                        case sCECReceiverFail:
                            break;
                        case sCECOk:
                        case sCECNone:
                            Switch_OneTouchPlayState(sActiveSource);
                            //DumpiT6601Reg();
                            break;
                    }
                }
            }
            break;
        case sActiveSource:         //            =0x04,    // follower:TV, switch    --> Broadcst
            {
                if ( OldTxCECcmd==eActiveSource )
                {
                    switch ( CECTxState )
                    {

                        case sCECReceiverFail:
                            break;
                        case sCECOk:
                        case sCECNone:
                            Switch_OneTouchPlayState(sTextViewOn);
                            //DumpiT6601Reg();
                            break;
                    }
                }
            }
            break;
        case sTextViewOn:           //            =0x0D,    // follower:TV
            {
                if ( OldTxCECcmd==sTextViewOn )
                {
                    switch ( CECTxState )
                    {

                        case sCECReceiverFail:
                            break;
                        case sCECOk:
                        case sCECNone:
                            Switch_OneTouchPlayState(sOTNone);
                            DumpiT6601Reg();
                            break;
                    }
                }
            }
            break;
        default:
            break;
    }
}
void switch_PollingMessage(PollingMessage_StateType state)
{
//    if(Myself_LogicAdr==DEVICE_ID_BROADCAST ||Myself_LogicAdr==DEVICE_ID_PLAYBACK1 ||
//        Myself_LogicAdr==DEVICE_ID_PLAYBACK2 || Myself_LogicAdr==DEVICE_ID_PLAYBACK3  ||)
    {
        IT6601_PRINT(("switch_PollingMessage--> %02x \n",state));
        switch ( state )
        {
            case sPollingMessage1:
#ifdef    _CEC_DEVICE_PLAYBACK_
                Myself_LogicAdr=DEVICE_ID_PLAYBACK1;
#endif
#ifdef    _CEC_DEVICE_AVR_
                Myself_LogicAdr=DEVICE_ID_AUDIO;
#endif
                CEC_WriteI2C_Byte(REG_CEC_TARGET_ADDR, Myself_LogicAdr);
                CECCmdSet(Myself_LogicAdr, ePollingMessage,0,0);
                break;
            case sPollingMessage2:
#ifdef    _CEC_DEVICE_PLAYBACK_
                Myself_LogicAdr=DEVICE_ID_PLAYBACK2;
#endif
#ifdef    _CEC_DEVICE_AVR_
                Myself_LogicAdr=DEVICE_ID_AUDIO;
#endif
                CEC_WriteI2C_Byte(REG_CEC_TARGET_ADDR, Myself_LogicAdr);
                CECCmdSet(Myself_LogicAdr, ePollingMessage,0,0);
                break;
            case sPollingMessage3:
#ifdef    _CEC_DEVICE_PLAYBACK_
                Myself_LogicAdr=DEVICE_ID_PLAYBACK3;
#endif
#ifdef    _CEC_DEVICE_AVR_
                Myself_LogicAdr=DEVICE_ID_AUDIO;
#endif
                CEC_WriteI2C_Byte(REG_CEC_TARGET_ADDR, Myself_LogicAdr);
                CECCmdSet(Myself_LogicAdr, ePollingMessage,0,0);
                break;
            case sReportPhyAdr:
                {
//
//                        if(PMState==sPollingPlayer1)
//                            Myself_LogicAdr=DEVICE_ID_PLAYBACK1;
//                        else if(PMState==sPollingPlayer2)
//                            Myself_LogicAdr=DEVICE_ID_PLAYBACK2;
//                        else if(PMState==sPollingPlayer3)
//                            Myself_LogicAdr=DEVICE_ID_PLAYBACK3;
//
//                        IT6601_WriteI2C_Byte(REG_CEC_TARGET_ADDR, Myself_LogicAdr);
                    CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,txphyadr[0],txphyadr[1]);
                }
                break;
            case sReportDeviceVendorID:
                CECCmdSet(DEVICE_ID_BROADCAST, eDeviceVendorID,0,0);
                break;
        }
        PMState=state;
    }
}

void PollingMessage(void)
{
//    BYTE uc;
    switch ( PMState )
    {
        case sPollingMessage1:
            {
                if ( OldTxCECcmd==ePollingMessage )
                {
//
//                        //switch(CECTxState)
//                        uc=(IT6601_ReadI2C_Byte(REG_SYS_STATUS));
//                        IT6601_PRINT(("out status=%02x \n",uc);
//                        uc&=B_OUT_STATUS;
//                        if(uc==B_ReceiverNACK ||uc==B_ReceiverFail )
//                        {
//
//                            //case sCECReceiverFail:
//                                    switch_PollingMessage(sReportPhyAdr);
//                            //        break;
//
//                            //case sCECOk:
//                            //case sCECNone:
//                            //        switch_PollingMessage(sPollingPlayer2);
//                            //        break;
//                        }else
//                            {
//                                switch_PollingMessage(sPollingPlayer2);
//                            }
                    switch ( CECTxState )
                    {
                        case sCECReceiverNack:
                        case sCECReceiverFail:
                            switch_PollingMessage(sReportPhyAdr);
                            break;
                        case sCECOk:
                        case sCECNone:
                            switch_PollingMessage(sPollingMessage2);
                            break;
                    }
                }
            }
            break;
        case sPollingMessage2:
            {
                if ( OldTxCECcmd==ePollingMessage )
                {
//                        uc=(IT6601_ReadI2C_Byte(REG_SYS_STATUS));
//                        IT6601_PRINT(("out status=%02x \n",uc);
//                        uc&=B_OUT_STATUS;
//                        if(uc==B_ReceiverNACK ||uc==B_ReceiverFail )
//                        {
//                                switch_PollingMessage(sReportPhyAdr);
//                        }else
//                            {
//                                switch_PollingMessage(sPollingPlayer3);
//                            }
                    switch ( CECTxState )
                    {

                        case sCECReceiverNack:
                        case sCECReceiverFail:
                            switch_PollingMessage(sReportPhyAdr);
                            break;
                        case sCECOk:
                        case sCECNone:
                            switch_PollingMessage(sPollingMessage3);
                            break;
                    }
                }
            }
            break;
        case sPollingMessage3:
            {
                if ( OldTxCECcmd==ePollingMessage )
                {
//                        uc=(IT6601_ReadI2C_Byte(REG_SYS_STATUS));
//                        IT6601_PRINT(("out status=%02x \n",uc);
//                        uc&=B_OUT_STATUS;
//                        if(uc==B_ReceiverNACK ||uc==B_ReceiverFail )
//                        {
//                                switch_PollingMessage(sReportPhyAdr);
//                        }else
//                            {
//                                switch_PollingMessage(sPMnone);
//                            }
                    switch ( CECTxState )
                    {

                        case sCECReceiverFail:
                            switch_PollingMessage(sReportPhyAdr);
                            break;
                        case sCECOk:
                        case sCECNone:
                            switch_PollingMessage(sPMnone);
                            break;
                    }
                }
            }
            break;
        case sReportPhyAdr:
            if ( OldTxCECcmd==eReportPhysicalAddress )
            {
                switch ( CECTxState )
                {
                    case sCECReceiverNack:
                    case sCECReceiverFail:
//                                    CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,txphyadr[0],txphyadr[1]);
                        break;
                    case sCECOk:
                    case sCECNone:
                        switch_PollingMessage(sReportDeviceVendorID);
                        break;
                }
            }
        case sReportDeviceVendorID:
            if ( OldTxCECcmd==eDeviceVendorID )
            {
                switch ( CECTxState )
                {
                    case sCECReceiverNack:
                    case sCECReceiverFail:
//                                    CECCmdSet(DEVICE_ID_BROADCAST, eDeviceVendorID,0,0);
                        break;
                    case sCECOk:
                    case sCECNone:
                        switch_PollingMessage(sPMnone);
                        break;
                }
            }
        default:
            break;
    }
}

// for iT6633+iT6623 Switch_LogicalAddressingState(LogicAddressing_StateType uState)
// for iT6633+iT6623{
// for iT6633+iT6623    IT6601_PRINT(("Switch_LogicalAddressingState--> %02x \n",uState));
// for iT6633+iT6623    LAState=uState;
// for iT6633+iT6623}
void LogicalAddressingHandler(void)
{
    u8_t        uc;
    static  u8_t        cPollingCount;
    if ( Myself_LogicAdr!=DEVICE_ID_TV )
        return;
    switch ( LAState )
    {

        case sReportPhysicalAddressTransfer:        // only for TV to create CEC device list
            {
                CECCmdSet(DEVICE_ID_BROADCAST, eReportPhysicalAddress,txphyadr[0],txphyadr[1]);
                cPollingCount=0;
                LAState=sPollingMessage;
            }
            break;
        case sPollingMessage:
            {
                cPollingCount++;
                if ( cPollingCount<15 )
                {
                    uc=(cPollingCount |0xF0);
                    CECCmdSet(uc,ePollingMessage,0,0);
                    LAState=sPollingResult;
                }
                else
                {
                    LAState=sLANone;
                    for ( uc=1;uc<15;uc++ )
                    {
                        IT6601_PRINT(("CECList[%02x]= %x , PhysicalAddr=[%x][%x] \n",
                                      uc,
                                      CECList[uc].Active,
                                      CECList[uc].PhyicalAddr1,
                                      CECList[uc].PhyicalAddr2));
                    }
                }
            }
            break;
        case sPollingResult:
            {
                //uc=(IT6601_ReadI2C_Byte(REG_SYS_STATUS)&B_OUT_STATUS);
                //IT6601_PRINT(("out status=%02x \n",uc);
                //if(uc==B_ReceiverFail)        // not yet !!! need to verify out status
                if ( CECTxState==sCECReceiverFail )
                {
                    if ( OldTxCECcmd==ePollingMessage )
                    {
                        //clear value for CEC list
                        CECList[cPollingCount].Active=FALSE;
                        CECList[cPollingCount].PhyicalAddr1=0x00;
                        CECList[cPollingCount].PhyicalAddr2=0x00;
                        LAState=sPollingMessage;
                    }
                }
            }
            break;
        case sReportPhysicalAddressReceived:
            {
                // stat to polling message and check ACK for allocate Logical Address to CEC device
                if ( pCECRxBuf->OPCODE==eReportPhysicalAddress )
                {
                    if ( (pCECRxBuf->HEADER & 0x0f)==0x0f )
                    {
                        uc=((pCECRxBuf->HEADER & 0xf0)>>4);
                        cPollingCount=uc;
                        CECList[cPollingCount].PhyicalAddr1=pCECRxBuf->OPERAND1;
                        CECList[cPollingCount].PhyicalAddr2=pCECRxBuf->OPERAND2;
                        CECList[cPollingCount].Active=TRUE;
                        LAState=sPollingMessage;
                    }
                    IT6601_PRINT(("-----Rx Header=%x, opcode=%x , operand1=%x , operand2=%x ,cPollingCount=%x-----\n ",
                                  pCECRxBuf->HEADER,
                                  pCECRxBuf->OPCODE,
                                  pCECRxBuf->OPERAND1,
                                  pCECRxBuf->OPERAND2,
                                  cPollingCount));
                }
            }
            break;
        case sLANone:
            break;
    }
}

BOOL CheckUpDownPhyAdr(void)
{
    u8_t i;
    BOOL Result=FALSE;
    BYTE pA,pB,pC,pD,MyLevel,TargetLevel;
    for ( i=0;i<4;i++ )
    {
        if ( (rxphyadr[i][0]==CECList[FollowerAddr].PhyicalAddr1) && (rxphyadr[i][1]==CECList[FollowerAddr].PhyicalAddr2) )
        {
            return TRUE;
        }
    }
    pA = ((CECList[FollowerAddr].PhyicalAddr1)&0xF0)>>4;
    pB = ((CECList[FollowerAddr].PhyicalAddr1)&0x0F);
    pC = ((CECList[FollowerAddr].PhyicalAddr2)&0xF0)>>4;
    pD = ((CECList[FollowerAddr].PhyicalAddr2)&0x0F);
    if ( pA==0 && pB==0 && pC==0 && pD==0 )
        TargetLevel = 0;
    else if ( pB==0 && pC==0 && pD==0 )
        TargetLevel = 1;
    else if ( pC==0 && pD==0 )
        TargetLevel = 2;
    else if ( pD==0 )
        TargetLevel = 3;
    pA = ((txphyadr[0])&0xF0)>>4;
    pB = ((txphyadr[0])&0x0F);
    pC = ((txphyadr[1])&0xF0)>>4;
    pD = ((txphyadr[1])&0x0F);
    if ( pA==0 && pB==0 && pC==0 && pD==0 )
        MyLevel = 0;
    else if ( pB==0 && pC==0 && pD==0 )
        MyLevel = 1;
    else if ( pC==0 && pD==0 )
        MyLevel = 2;
    else if ( pD==0 )
        MyLevel = 3;
    if ( MyLevel>TargetLevel )
        Result=TRUE;
    return Result;
}

void switch_ARCState(ARC_StateType uState)
{
    BYTE    uc;
    if ( Myself_LogicAdr==DEVICE_ID_AUDIO||Myself_LogicAdr==DEVICE_ID_TV )
    {
        IT6601_PRINT(("switch ARCState -> %s  !!! \n",ARCStr[uState])) ;
        ARCState=uState;
        switch ( ARCState )
        {

            case sRequestARCInitiation:
                CECCmdSet(FollowerAddr, eRequestARCInitiation,txphyadr[0],txphyadr[1]);
                break;
            case sInitiateARC:
                {
// for iT6633+iT6623                        // Receiver CEC command Queue for I2C slave protocal
// for iT6633+iT6623                        t_IIC_Buffer.I2cRegisterMap.CEC_ARC_Status=1;
// for iT6633+iT6623                        I2C_CECAddQ(0xC0);    //2011/06/09 for ACR cmd
                    CECCmdSet(FollowerAddr, eInitiateARC,txphyadr[0],txphyadr[1]);
                    uc=CEC_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
                    if ( Myself_LogicAdr==0 )
                        uc |= (B_ARC_TX_EN);
                    else
                    {
                        uc |= (B_ARC_RX_EN|B_SPDIFO_EN);
                    }
                    CEC_WriteI2C_Byte(REG_HEAC_CTRL,uc);
                    ARCState=sARCNone;
                    DumpiT6601Reg();
                    printk(" ARC on \n");
                }
                break;
            case sReportARCInitiated:
                {
                    CECCmdSet(FollowerAddr, eReportARCInitiated,txphyadr[0],txphyadr[1]);
                    uc=CEC_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
                    if ( Myself_LogicAdr==0 )
                        uc |= (B_ARC_TX_EN);
                    else
                    {
                        uc |= (B_ARC_RX_EN|B_SPDIFO_EN);
                    }
                    CEC_WriteI2C_Byte(REG_HEAC_CTRL,uc);
                    ARCState=sARCNone;
                    DumpiT6601Reg();
                }
                break;
            case sRequestARCTermination:
                {
                    CECCmdSet(FollowerAddr, eRequestARCTermination,txphyadr[0],txphyadr[1]);
                }
                break;
            case sTerminateARC:
                {
                    CECCmdSet(FollowerAddr, eTerminateARC,txphyadr[0],txphyadr[1]);
                    uc=CEC_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
                    //if(Myself_LogicAdr==0)
                    //    uc &= ~(B_ARC_TX_EN);
                    //else
                    //    uc &=~(B_ARC_RX_EN|B_SPDIFO_EN);
                    //uc&=0xFC;    //2010/12/21 for test , disable ARC rx and tx  , not yet !!!
                    CEC_WriteI2C_Byte(REG_HEAC_CTRL,uc);
                    ARCState=sARCNone;
                    DumpiT6601Reg();
                }
                break;
            case sReportARCTermination:
                {
// for iT6633+iT6623                        // Receiver CEC command Queue for I2C slave protocal
// for iT6633+iT6623                        t_IIC_Buffer.I2cRegisterMap.CEC_ARC_Status=0;
// for iT6633+iT6623                        I2C_CECAddQ(0xC0);    //2011/06/09 for ACR cmd
                    CECCmdSet(FollowerAddr, eReportARCTerminated,txphyadr[0],txphyadr[1]);
                    uc=CEC_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
                    //if(Myself_LogicAdr==0)
                    //    uc &= ~(B_ARC_TX_EN);
                    //else
                    //    uc &=~(B_ARC_RX_EN|B_SPDIFO_EN);
                    //uc&=0xFC;    //2010/12/21 for test , disable ARC rx and tx  , not yet !!!
                    CEC_WriteI2C_Byte(REG_HEAC_CTRL,uc);
                    ARCState=sARCNone;
                    DumpiT6601Reg();
                    //printf(" ARC off \n");
                    printk(" ARC off \n");
                }
                break;
            case sARCNone:
                break;
        }
    }
}

void ARCHandler(void)
{
//    u8_t        uc;
    switch ( ARCState )
    {
        case sRequestARCInitiation:
            {
                //CECCmdSet(DEVICE_ID_BROADCAST, eRequestARCInitiation,txphyadr[0],txphyadr[1]);
                //ARCState=sARCNone;
            }
            break;
        case sInitiateARC:
            {
                //CECCmdSet(DEVICE_ID_BROADCAST, eInitiateARC,txphyadr[0],txphyadr[1]);
                //ARCState=sARCNone;
            }
            break;
        case sReportARCInitiated:
            {
//                    CECCmdSet(FollowerAddr, eReportARCInitiated,txphyadr[0],txphyadr[1]);
//
//                    uc=IT6601_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
//                    if(Myself_LogicAdr==0)
//                        uc |= (B_ARC_TX_EN);
//                    else
//                        {
//                        uc |= (B_ARC_RX_EN|B_SPDIFO_EN);
//                        }
//                    IT6601_WriteI2C_Byte(REG_HEAC_CTRL,uc);
//                    ARCState=sARCNone;
//                    DumpiT6601Reg();
            }
            break;
        case sRequestARCTermination:
            {
                //CECCmdSet(DEVICE_ID_BROADCAST, eRequestARCTermination,txphyadr[0],txphyadr[1]);
                //ARCState=sTerminateARC;
            }
            break;
        case sTerminateARC:
            {
//                    CECCmdSet(DEVICE_ID_BROADCAST, eTerminateARC,txphyadr[0],txphyadr[1]);
//                    ARCState=sARCNone;
            }
            break;
        case sReportARCTermination:
            {
//                    CECCmdSet(FollowerAddr, eReportARCTerminated,txphyadr[0],txphyadr[1]);
//
//                    uc=IT6601_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
//
//                    //if(Myself_LogicAdr==0)
//                    //    uc &= ~(B_ARC_TX_EN);
//                    //else
//                    //    uc &=~(B_ARC_RX_EN|B_SPDIFO_EN);
//                    //uc&=0xFC;    //2010/12/21 for test , disable ARC rx and tx  , not yet !!!
//
//                    IT6601_WriteI2C_Byte(REG_HEAC_CTRL,uc);
//
//                    ARCState=sARCNone;
//                    DumpiT6601Reg();
            }
            break;
        case sARCNone:
        default:
            break;
    }
}
HEC_StateType   HECState=sHEC_None;
void SwitchHECState(HEC_StateType   uState)
{
    HECState=uState;
}
#if 0
void HECHandler(void)
{
//    u8_t        uc;
    switch ( HECState )
    {
        case    sHEC_InquirState:
            {
                //waiting to receive init ARC cmd
                if ( pCECRxBuf->OPERAND3==eHEC_InquirState )
                {
                    //uc=((pCECRxBuf->HEADER & 0xf0)>>4);
                    //FollowerAddr=uc;
                    //pCECRxBuf->OPERAND1
                    IT6601_PRINT(("-----Header=%02x, opcode=%02x , operand1=%02x , operand2=%02x ,FollowerAddr=%02x-----\n ",
                                  pCECRxBuf->HEADER,
                                  pCECRxBuf->OPCODE,
                                  pCECRxBuf->OPERAND1,
                                  pCECRxBuf->OPERAND2,
                                  FollowerAddr
                                 );
                                ARCState=sReportARCInitiated;
                                }
                                }
                                break;case    sHEC_ReportState:{
                                }
                                break;case    sHEC_SetStateAdjacent:{
                                }
                                break;case    sHEC_SetState:{
                                }
                                break;case    sHEC_RequestDeactivation:{
                                }
                                break;case    sHEC_NotifyAlive:{
                                }
                                break;case    sHEC_Discover:{
                                }
                                break;case    sHEC_HPDSetState:{
                                }
                                break;case    sHEC_HPDReportState:{
                                }
                                break;case    sHEC_None:{
                                }
                                break;
                                }
                                }
#endif
//w/o 11.2.6-3~5

void SystemInfomationFeature(void){
#if 0
    switch ( SIFState )
    {
        case sReportPhysicalAddress:
            if ( OldTxCECcmd==eReportPhysicalAddress )
            {
                switch ( CECTxState )
                {
                    case sCECReceiverNack:
                    case sCECReceiverFail:
                        break;
                    case sCECOk:
                    case sCECNone:
                        if ( Myself_LogicAdr!=DEVICE_ID_TV && Myself_LogicAdr!=DEVICE_ID_AUDIO )
                            switch_PollingMessage(sPollingMessage1);
                        SIFState    =sSIFnone;
                        break;
                }
            }
            break;
        case sCECVersioin:
        case sGetCECVersion:
        case sGetMenuLanguage:
        case sGivePhysicalAddress:
        case sSetMenuLanguage:
        case sSIFnone:
            break;
    }
#endif
}

void CECManager(void)
{
    //Receiver of CEC command
    //#ifndef _Enable_CEC_Interrupt_Pin_
    iT6601Receiver();       // Receive CEC command
    //#endif
    //Decoder of CEC command
    CECDecoder();           // De-Queue for receive CEC command
    //Handler of ARC
    //ARCHandler();               //for TV device
    OneTouchPlayHandler();      //for Audio and player device
    //LogicalAddressingHandler(); //for TV device
    PollingMessage();           //for player device
    //SystemInfomationFeature();  //for system information Feature
//    SystemAudioControl();       //for Audio control system CTS 11.2.15
    // Tranmitter of CEC command
//  CECCmdSet(0x0F,eActiveSource,(CECADDR & 0xFF00) >> 8, (CECADDR& 0x00FF));
    iT6601Transmitter();
}
// for iT6633+iT6623
// for iT6633+iT6623//
// for iT6633+iT6623//void switch_Standby(BYTE state)
// for iT6633+iT6623//{
// for iT6633+iT6623//
// for iT6633+iT6623//    StandbyState=state;
// for iT6633+iT6623//
// for iT6633+iT6623//    switch(StandbyState)
// for iT6633+iT6623//    {
// for iT6633+iT6623//        case 0:
// for iT6633+iT6623//            // TODO: nothing!!!
// for iT6633+iT6623//            break;
// for iT6633+iT6623//
// for iT6633+iT6623//        case 1:
// for iT6633+iT6623//            bSystemAudioMode=0;
// for iT6633+iT6623//            CECCmdSet(DEVICE_ID_TV,eSetSystemAudioMode,bSystemAudioMode,0 );
// for iT6633+iT6623//            break;
// for iT6633+iT6623//
// for iT6633+iT6623//        case 1:
// for iT6633+iT6623//            CECCmdSet(DEVICE_ID_BROADCAST,eSetSystemAudioMode,bSystemAudioMode,0 );
// for iT6633+iT6623//            break;
// for iT6633+iT6623//
// for iT6633+iT6623//        case 2:
// for iT6633+iT6623//            CECCmdSet(DEVICE_ID_BROADCAST, eStandBy,0,0);
// for iT6633+iT6623//            break;
// for iT6633+iT6623//    }
// for iT6633+iT6623//}
// for iT6633+iT6623//void StandbyHandler(void)
// for iT6633+iT6623//{
// for iT6633+iT6623//    switch(StandbyState)
// for iT6633+iT6623//    {
// for iT6633+iT6623//        case 0:
// for iT6633+iT6623//            break;
// for iT6633+iT6623//
// for iT6633+iT6623//        case 1:
// for iT6633+iT6623//            break;
// for iT6633+iT6623//    }
// for iT6633+iT6623//
// for iT6633+iT6623//
// for iT6633+iT6623//}
// for iT6633+iT6623
// for iT6633+iT6623void CECTest(eCEC_CMD cmd,u8_t cData)
// for iT6633+iT6623{
// for iT6633+iT6623    u8_t uc;
// for iT6633+iT6623
// for iT6633+iT6623    switch(cmd)
// for iT6633+iT6623    {
// for iT6633+iT6623        case ePollingMessage:
// for iT6633+iT6623
// for iT6633+iT6623        case eActiveSource:            //            =0x82,    // follower:TV, switch    --> Broadcst , Directly address,  Opread: Phyical address
// for iT6633+iT6623//                CECCmdSet(DEVICE_ID_BROADCAST, cmd,txphyadr[0],txphyadr[1]);
// for iT6633+iT6623                Switch_OneTouchPlayState(sImageViewOn);
// for iT6633+iT6623
// for iT6633+iT6623                break;
// for iT6633+iT6623        case eImageViewOn:            //            =0x04,    // follower:TV, switch    --> Broadcst
// for iT6633+iT6623        case eTextViewOn:                //            =0x0D,    // follower:TV
// for iT6633+iT6623                break;
// for iT6633+iT6623        // Routing Control Feature
// for iT6633+iT6623        case eRoutingChange:            //            =0x80,    // follower:TV, switch    --> Broadcst
// for iT6633+iT6623            CEC_RoutingChange(cData);
// for iT6633+iT6623                break;
// for iT6633+iT6623
// for iT6633+iT6623        case eRoutingInformation:        //            =0x81,    // follower:switch        --> Broadcst
// for iT6633+iT6623        case eRequestActiveSource:        //            =0x85,    // follower:switch        --> Broadcst     , AVR to request active source when Audio ocntrol feature is active
// for iT6633+iT6623        case eSetStreamPath:            //            =0x86,    // follower:switch        --> Broadcst
// for iT6633+iT6623
// for iT6633+iT6623        case eInactiveSource:            //            =0x9D,    //
// for iT6633+iT6623            {
// for iT6633+iT6623                CECCmdSet(DEVICE_ID_TV, eInactiveSource,txphyadr[0],txphyadr[1]);
// for iT6633+iT6623            }
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eStandBy:                //            =0x36,    // follower:All            --> Broadcst
// for iT6633+iT6623            CECCmdSet(DEVICE_ID_BROADCAST, eStandBy,0,0);
// for iT6633+iT6623
// for iT6633+iT6623#if 0
// for iT6633+iT6623            bSystemAudioMode=0;
// for iT6633+iT6623
// for iT6633+iT6623            if(cData==0)
// for iT6633+iT6623                CECCmdSet(DEVICE_ID_TV,eSetSystemAudioMode,bSystemAudioMode,0 );
// for iT6633+iT6623            else
// for iT6633+iT6623                CECCmdSet(DEVICE_ID_BROADCAST,eSetSystemAudioMode,bSystemAudioMode,0 );
// for iT6633+iT6623#endif
// for iT6633+iT6623
// for iT6633+iT6623            break;
// for iT6633+iT6623        // System Information Feature
// for iT6633+iT6623        case eCECVersioin:                //            =0x9E,
// for iT6633+iT6623        case eGetCECVersion:            //            =0x9F,
// for iT6633+iT6623        case eGivePhysicalAddress:        //            =0x83,    // follower:All
// for iT6633+iT6623            Switch_LogicalAddressingState(sReportPhysicalAddressTransfer);
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eGetMenuLanguage:        //            =0x91,    // follower:TV
// for iT6633+iT6623        case eReportPhysicalAddress:    //            =0x84,    // follower:TV
// for iT6633+iT6623        case eSetMenuLanguage:        //            =0x32,    // follower:All,        Initiator:TV
// for iT6633+iT6623
// for iT6633+iT6623        // Vendor Specific Commands Feature
// for iT6633+iT6623        case eDeviceVendorID:            //            =0x87,
// for iT6633+iT6623        case eGiveDeviceVendorID:        //            =0x8C,
// for iT6633+iT6623        case eVendorCommand:        //            =0x89,
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eReportPowerStatus:        //            =0x90,
// for iT6633+iT6623                u8_PowerStatus=cData;    // 0: on , 1: standby, 2: in transition standby to on , 3: in transition on to standby
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eVendorCommandWithID:    //             =0xA0,
// for iT6633+iT6623
// for iT6633+iT6623        // other
// for iT6633+iT6623        case ePlay:                    //            =0x41,
// for iT6633+iT6623            break;
// for iT6633+iT6623
// for iT6633+iT6623        case eUserPressed:            //        =0x44,
// for iT6633+iT6623            CECCmdSet(DEVICE_ID_TV, eUserPressed, 0x60,0);
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eUserReleased:            //            =0x45,
// for iT6633+iT6623            CECCmdSet(DEVICE_ID_TV, eUserReleased, 0x60,0);
// for iT6633+iT6623            break;
// for iT6633+iT6623
// for iT6633+iT6623        case eAudioModeRequest:        //            =0x70,
// for iT6633+iT6623        case eGiveAudioStatus:            //            =0x71,
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eSetSystemAudioMode:            //            =0x72,
// for iT6633+iT6623
// for iT6633+iT6623            if(cData==0)
// for iT6633+iT6623                bSystemAudioMode=0;
// for iT6633+iT6623            else
// for iT6633+iT6623                bSystemAudioMode=1;
// for iT6633+iT6623//
// for iT6633+iT6623//            if(cData==0)
// for iT6633+iT6623//                CECCmdSet(DEVICE_ID_TV,eSetSystemAudioMode,bSystemAudioMode,0 );
// for iT6633+iT6623//            else
// for iT6633+iT6623//                CECCmdSet(DEVICE_ID_BROADCAST,eSetSystemAudioMode,bSystemAudioMode,0 );
// for iT6633+iT6623
// for iT6633+iT6623            if(cData==0)
// for iT6633+iT6623                Switch_SystemAudioControl(sSetSystemAudioModeToTV);
// for iT6633+iT6623            else
// for iT6633+iT6623                Switch_SystemAudioControl(sComesOutOfStandby);
// for iT6633+iT6623
// for iT6633+iT6623
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eReportAudioStatus:        //            =0x7A,
// for iT6633+iT6623        case eGiveSystemAudIoModeStatus:        //            =0x7D,
// for iT6633+iT6623        case eSystemAudIoModeStatus:            //            =0x7E,
// for iT6633+iT6623        case eRemoteButtonDown:        //            =0x8A,
// for iT6633+iT6623        case eRemoteButtonUp:            //            =0x8B,
// for iT6633+iT6623                break;
// for iT6633+iT6623        case eRequestAudioDescriptor:    //             =0xA4,
// for iT6633+iT6623            CECCmdSet(FollowerAddr,eRequestAudioDescriptor,0,0);    // Report EDID Audio Block !!!
// for iT6633+iT6623            break;
// for iT6633+iT6623        // ACR
// for iT6633+iT6623        case eInitiateARC:                //            =0xC0,
// for iT6633+iT6623//            switch_ARCState(sInitiateARC);
// for iT6633+iT6623
// for iT6633+iT6623
// for iT6633+iT6623// for iT6633+iT6623                // Receiver CEC command Queue for I2C slave protocal
// for iT6633+iT6623// for iT6633+iT6623                t_IIC_Buffer.I2cRegisterMap.CEC_ARC_Status=1;
// for iT6633+iT6623// for iT6633+iT6623                I2C_CECAddQ(0xC0);    //2011/06/09 for ACR cmd
// for iT6633+iT6623
// for iT6633+iT6623                {
// for iT6633+iT6623                    CECCmdSet(cData, eInitiateARC,txphyadr[0],txphyadr[1]);
// for iT6633+iT6623                    uc=IT6601_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
// for iT6633+iT6623                    if(Myself_LogicAdr==0)
// for iT6633+iT6623                        uc |= (B_ARC_TX_EN);
// for iT6633+iT6623                    else
// for iT6633+iT6623                        {
// for iT6633+iT6623                        uc |= (B_ARC_RX_EN|B_SPDIFO_EN);
// for iT6633+iT6623                        }
// for iT6633+iT6623                    IT6601_WriteI2C_Byte(REG_HEAC_CTRL,uc);
// for iT6633+iT6623                    //ARCState=sARCNone;
// for iT6633+iT6623                    //DumpiT6601Reg();
// for iT6633+iT6623                }
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eReportARCInitiated:        //            =0xC1,
// for iT6633+iT6623//            switch_ARCState(sReportARCInitiated);
// for iT6633+iT6623                    {
// for iT6633+iT6623                        CECCmdSet(cData, eReportARCInitiated,txphyadr[0],txphyadr[1]);
// for iT6633+iT6623
// for iT6633+iT6623                        uc=IT6601_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
// for iT6633+iT6623                        if(Myself_LogicAdr==0)
// for iT6633+iT6623                            uc |= (B_ARC_TX_EN);
// for iT6633+iT6623                        else
// for iT6633+iT6623                            {
// for iT6633+iT6623                            uc |= (B_ARC_RX_EN|B_SPDIFO_EN);
// for iT6633+iT6623                            }
// for iT6633+iT6623                        IT6601_WriteI2C_Byte(REG_HEAC_CTRL,uc);
// for iT6633+iT6623                        //ARCState=sARCNone;
// for iT6633+iT6623                        //DumpiT6601Reg();
// for iT6633+iT6623                    }
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eReportARCTerminated:    //            =0xC2,
// for iT6633+iT6623            //switch_ARCState(sReportARCTerminated);
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eRequestARCInitiation:        //            =0xC3,
// for iT6633+iT6623            FollowerAddr=cData;
// for iT6633+iT6623            switch_ARCState(sRequestARCInitiation);
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eRequestARCTermination:    //            =0xC4,
// for iT6633+iT6623            FollowerAddr=cData;
// for iT6633+iT6623            switch_ARCState(sRequestARCTermination);
// for iT6633+iT6623            break;
// for iT6633+iT6623        case eTerminateARC:            //            =0xC5,
// for iT6633+iT6623//            switch_ARCState(sTerminateARC);
// for iT6633+iT6623
// for iT6633+iT6623
// for iT6633+iT6623// for iT6633+iT6623                // Receiver CEC command Queue for I2C slave protocal
// for iT6633+iT6623// for iT6633+iT6623                t_IIC_Buffer.I2cRegisterMap.CEC_ARC_Status=0;
// for iT6633+iT6623// for iT6633+iT6623                I2C_CECAddQ(0xC0);    //2011/06/09 for ACR cmd
// for iT6633+iT6623                {
// for iT6633+iT6623                    CECCmdSet(cData, eTerminateARC,txphyadr[0],txphyadr[1]);
// for iT6633+iT6623                    uc=IT6601_ReadI2C_Byte(REG_HEAC_CTRL)&(~(B_ARC_TX_EN|B_ARC_RX_EN|B_SPDIFO_EN|B_HEC_EN));
// for iT6633+iT6623
// for iT6633+iT6623                    //if(Myself_LogicAdr==0)
// for iT6633+iT6623                    //    uc &= ~(B_ARC_TX_EN);
// for iT6633+iT6623                    //else
// for iT6633+iT6623                    //    uc &=~(B_ARC_RX_EN|B_SPDIFO_EN);
// for iT6633+iT6623                    //uc&=0xFC;    //2010/12/21 for test , disable ARC rx and tx  , not yet !!!
// for iT6633+iT6623
// for iT6633+iT6623                    IT6601_WriteI2C_Byte(REG_HEAC_CTRL,uc);
// for iT6633+iT6623
// for iT6633+iT6623                    //ARCState=sARCNone;
// for iT6633+iT6623                    //DumpiT6601Reg();
// for iT6633+iT6623                }
// for iT6633+iT6623            break;
// for iT6633+iT6623    }
// for iT6633+iT6623
// for iT6633+iT6623}
void CECChangePort(void)
{
// for iT6633+iT6623
// for iT6633+iT6623    BYTE    i;
// for iT6633+iT6623    BOOL    bResult=FALSE;
// for iT6633+iT6623
// for iT6633+iT6623    for(i=0;i<4;i++)
// for iT6633+iT6623    {
// for iT6633+iT6623        if((rxphyadr[i][0]==pCECRxBuf->OPERAND1) && (rxphyadr[i][1]==pCECRxBuf->OPERAND2) )
// for iT6633+iT6623        {
// for iT6633+iT6623            rxcurport=i;
// for iT6633+iT6623            IT6601_PRINT(("===CEC Active Port = %02x ===\n",i));
// for iT6633+iT6623
// for iT6633+iT6623            if(i<3)
// for iT6633+iT6623            {
// for iT6633+iT6623                if((IT6633_InputSel != i )||(ucCurrentHDMIPort==1))
// for iT6633+iT6623                {
// for iT6633+iT6623                    if(It6633ChangePort(i)==TRUE)
// for iT6633+iT6623                    {
// for iT6633+iT6623                        if(Myself_LogicAdr==DEVICE_ID_AUDIO)
// for iT6633+iT6623                            CECCmdSet(DEVICE_ID_BROADCAST, eRequestActiveSource,0,0);
// for iT6633+iT6623                    }
// for iT6633+iT6623                    CAT6023InputPortSet(0);
// for iT6633+iT6623
// for iT6633+iT6623                    bResult=TRUE;
// for iT6633+iT6623                }
// for iT6633+iT6623            }else if(i==3)
// for iT6633+iT6623            {
// for iT6633+iT6623
// for iT6633+iT6623                if(ucCurrentHDMIPort==0)
// for iT6633+iT6623                {
// for iT6633+iT6623                    if(CAT6023InputPortSet(1)==TRUE)
// for iT6633+iT6623                    {
// for iT6633+iT6623                        if(Myself_LogicAdr==DEVICE_ID_AUDIO)
// for iT6633+iT6623                            CECCmdSet(DEVICE_ID_BROADCAST, eRequestActiveSource,0,0);
// for iT6633+iT6623                    }
// for iT6633+iT6623                    bResult=TRUE;
// for iT6633+iT6623                }
// for iT6633+iT6623            }
// for iT6633+iT6623
// for iT6633+iT6623
// for iT6633+iT6623
// for iT6633+iT6623            //if(bResult==TRUE)
// for iT6633+iT6623            {
// for iT6633+iT6623                if(i==0)
// for iT6633+iT6623                    t_IIC_Buffer.I2cRegisterMap.CEC_Active_Port=2;
// for iT6633+iT6623                else     if(i==2)
// for iT6633+iT6623                    t_IIC_Buffer.I2cRegisterMap.CEC_Active_Port=0;
// for iT6633+iT6623                else
// for iT6633+iT6623                    t_IIC_Buffer.I2cRegisterMap.CEC_Active_Port=i;
// for iT6633+iT6623
// for iT6633+iT6623                I2C_CECAddQ(eActiveSource);    //2011/06/09 for ACR cmd
// for iT6633+iT6623            }
// for iT6633+iT6623
// for iT6633+iT6623        }
// for iT6633+iT6623
// for iT6633+iT6623    }
}

void Switch_SystemAudioControl(SystemAudioControl_StateType State)
{
    IT6601_PRINT(("!!! SwitchSystemInfoFeature -> %02x !!!\n",State));
    SACState=State;
    switch ( SACState )
    {
        case sComesOutOfStandby:
            bSystemAudioMode=1;
            CECCmdSet(DEVICE_ID_BROADCAST,eRequestActiveSource,0,0);
            break;
        case sSetSystemAudioModeToTV:
            CECCmdSet(DEVICE_ID_TV,eSetSystemAudioMode,bSystemAudioMode,0 );
           // CreatTimerTask(eCECTimeOut,200,SysTimerTask_OnceRun);
            break;
        case sWaitingFeatureAbort:
            //CreatTimerTask(eCECTimeOut,1000,SysTimerTask_OnceRun);
            break;
        case sSetSystemAudioModeToAll:
            CECCmdSet(DEVICE_ID_BROADCAST,eSetSystemAudioMode,bSystemAudioMode,0 );
            break;
    }
}

//w/o 11.2.15-10~14 !!!

void SystemAudioControl(void)       //for Audio control system CTS 11.2.15
{
    switch ( SACState )
    {
        case sSACnone:
            // TODO: Nothing.
            break;
        case sComesOutOfStandby:{
                //please refer CEC Figure 27 A in HDMI Specification 1.4
                //Request Active Source then Waitting Active Source command
                if ( pCECRxBuf->OPCODE==eActiveSource )
                {
                    Switch_SystemAudioControl(sSetSystemAudioModeToTV);
                }
            }
            break;
        case sSetSystemAudioModeToTV:{
                if ( OldTxCECcmd==eSetSystemAudioMode )
                {
                    switch ( CECTxState )
                    {
                        case sCECReceiverFail:
                            Switch_SystemAudioControl(sSetSystemAudioModeToAll);
                            break;
                        case sCECReceiverNack:
                            IT6601_PRINT(("!!! Nack !!!\n"));
//                            if ( TimeOutCheck(eCECTimeOut)==TRUE )
//                                Switch_SystemAudioControl(sSetSystemAudioModeToAll);
                            break;
                        case sCECOk:
                        case sCECNone:
                            Switch_SystemAudioControl(sWaitingFeatureAbort);
                            break;
                    }
                }
            }
            break;
        case sWaitingFeatureAbort:{
                if ( pCECRxBuf->OPCODE==eFeatureAbort )
                {
                    Switch_SystemAudioControl(sSACnone);    //finish < Set system Audio Mode >
                }
                else
                {
                   // if ( TimeOutCheck(eCECTimeOut)==TRUE )
                        //Switch_SystemAudioControl(sSetSystemAudioModeToAll);
                }
            }
            break;
    }
}

void CECUICommandCode(BYTE UIKey)
{
    switch ( UIKey )
    {
        case 0x40:  //Power
            if ( u8_PowerStatus==0 || u8_PowerStatus==2 )
                u8_PowerStatus=1;   // standby
            else
                u8_PowerStatus=0;   // on
            break;
        case 0x41:  //Volume Up
            if ( (uCECAudioStatus.Map.AudioVolumeStatus) <0x7F )
                uCECAudioStatus.Map.AudioVolumeStatus++;
            uCECAudioStatus.Map.AudioMuteStatus=FALSE;
// for iT6633+iT6623                    t_IIC_Buffer.I2cRegisterMap.CEC_Audio_Status=(BYTE)uCECAudioStatus.Map.AudioVolumeStatus;
// for iT6633+iT6623                    I2C_CECAddQ(0x41);    //2011/06/09 for ACR cmd
             printk("CEC volume = %x \n",uCECAudioStatus.Map.AudioVolumeStatus);
            break;
        case 0x42:  //Volume Down
            if ( (uCECAudioStatus.Map.AudioVolumeStatus) >0x00 )
            {
                uCECAudioStatus.Map.AudioVolumeStatus--;
                uCECAudioStatus.Map.AudioMuteStatus=FALSE;
// for iT6633+iT6623                        t_IIC_Buffer.I2cRegisterMap.CEC_Audio_Status=(BYTE)uCECAudioStatus.Map.AudioVolumeStatus;
// for iT6633++                        I2C_CECAddQ(0x42);    //2011/06/09 for ACR cmd
// for iT6633+iT6623                        printf("CEC volume = %02x \n",uCECAudioStatus.Map.AudioVolumeStatus);
            }
            else
            {
                if ( (uCECAudioStatus.Map.AudioVolumeStatus) == 0x00 )
                    uCECAudioStatus.Map.AudioMuteStatus=TRUE;
            }
            break;
        case 0x43:  //Mute
            uCECAudioStatus.Map.AudioMuteStatus=~(uCECAudioStatus.Map.AudioMuteStatus);
            break;
        case 0x65:  //Mute
            uCECAudioStatus.Map.AudioMuteStatus=TRUE;
            break;
        case 0x6C:  //Power OFF
            u8_PowerStatus=1;   // standby
            break;
        case 0x6D:  //Power ON
            u8_PowerStatus=0;   // on
            break;
    }
}
















