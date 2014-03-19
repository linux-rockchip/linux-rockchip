#include "rk3028a_hdmi.h"
#include "rk3028a_hdmi_hw.h"
#include "rk3028a_hdmi_cec.h"

static Cec_t Cec;
static struct rk3028a_hdmi *rk3028a_hdmi = NULL;
static char LA_Player[3] = { CEC_LOGADDR_PLAYBACK1, CEC_LOGADDR_PLAYBACK2, CEC_LOGADDR_PLAYBACK3 };

static int CecReadFrame(CEC_FrameData_t *Frame)
{
	int i, length;
	char *data = Frame;
	if(Frame == NULL)
		return -1;
	length = HDMIRdReg(CEC_RX_LENGTH);
	HDMIWrReg(CEC_RX_OFFSET, 0);
	printk("%s length is %d\n", __FUNCTION__, length);
	for(i = 0; i < length; i++) {
		data[i] =  HDMIRdReg(CEC_DATA);
		printk("%02x\n", data[i]);
	}
	return 0;
}

static int CecSendFrame(CEC_FrameData_t *Frame)
{
	int i;
	
	CECDBG("TX srcDestAddr %02x opcode %02x ",
		 Frame->srcDestAddr, Frame->opcode);
	if(Frame->argCount) {
		CECDBG("args:");
		for(i = 0; i < Frame->argCount; i++) {
			CECDBG("%02x ", Frame->args[i]);
		}
	}
	CECDBG("\n");
	
	HDMIWrReg(CEC_TX_OFFSET, 0);
	HDMIWrReg(CEC_DATA, Frame->srcDestAddr);
	HDMIWrReg(CEC_DATA, Frame->opcode);
	for(i = 0; i < Frame->argCount; i++)
		HDMIWrReg(CEC_DATA, Frame->args[i]);
	HDMIWrReg(CEC_TX_LENGTH, Frame->argCount + 2);
	
	//Wait for bus free
	Cec.busfree = 1;
	HDMIWrReg(CEC_CTRL, m_BUSFREETIME_ENABLE);
	if(wait_event_interruptible_timeout(Cec.wait, Cec.busfree == 0, msecs_to_jiffies(17))) {
		return -1;
	}
	//Start TX
	Cec.tx_done = 0;
	HDMIWrReg(CEC_CTRL, m_BUSFREETIME_ENABLE|m_START_TX);
	if(wait_event_interruptible_timeout(Cec.wait, Cec.tx_done != 0, msecs_to_jiffies(100)))
	HDMIWrReg(CEC_CTRL, 0);
	if(Cec.tx_done == 1) {
		return 0;
	}
	else
		return -1;
}

static int CecSendMessage ( char opCode, char dest )
{
	CEC_FrameData_t cecFrame;

    cecFrame.opcode        = opCode;
    cecFrame.srcDestAddr   = MAKE_SRCDEST( Cec.address_logic, dest );
    cecFrame.argCount      = 0;
    return CecSendFrame( &cecFrame );
}

static void CecSendFeatureAbort ( CEC_FrameData_t *pCpi, char reason )
{
    CEC_FrameData_t cecFrame;

    if (( pCpi->srcDestAddr & 0x0F) != CEC_LOGADDR_UNREGORBC )
    {
        cecFrame.opcode        = CECOP_FEATURE_ABORT;
        cecFrame.srcDestAddr   = MAKE_SRCDEST( Cec.address_logic, (pCpi->srcDestAddr & 0xF0) >> 4 );
        cecFrame.args[0]       = pCpi->opcode;
        cecFrame.args[1]       = reason;
        cecFrame.argCount      = 2;
        CecSendFrame( &cecFrame );
    }
}

static void CecSendActiveSource(void)
{
	CEC_FrameData_t    cecFrame;

	cecFrame.opcode        = CECOP_ACTIVE_SOURCE;
	cecFrame.srcDestAddr   = MAKE_SRCDEST( Cec.address_logic, CEC_LOGADDR_UNREGORBC);
	cecFrame.args[0]       = (Cec.address_phy & 0xFF00) >> 8;        // [Physical Address]
	cecFrame.args[1]       = (Cec.address_phy & 0x00FF);             // [Physical Address]
	cecFrame.argCount      = 2;
	CecSendFrame( &cecFrame );
}

static void StartActiveSource(void)
{
	int i;
	
	// GPIO simulate CEC timing may be not correct, so we try more times.
	//send image view on first
	for(i = 0; i < 1; i++) {
		if(CecSendMessage(CECOP_IMAGE_VIEW_ON,CEC_LOGADDR_TV) == 0) {
			CecSendActiveSource();
		}
	}
}

static void CecHandleInactiveSource ( CEC_FrameData_t *pCpi )
{
	
}

static void CecHandleFeatureAbort( CEC_FrameData_t *pCpi )
{
   
}

static bool ValidateCecMessage ( CEC_FrameData_t *pCpi )
{
    char parameterCount = 0;
    bool    countOK = true;

    /* Determine required parameter count   */

    switch ( pCpi->opcode )
    {
        case CECOP_IMAGE_VIEW_ON:
        case CECOP_TEXT_VIEW_ON:
        case CECOP_STANDBY:
        case CECOP_GIVE_PHYSICAL_ADDRESS:
        case CECOP_GIVE_DEVICE_POWER_STATUS:
        case CECOP_GET_MENU_LANGUAGE:
        case CECOP_GET_CEC_VERSION:
            parameterCount = 0;
            break;
        case CECOP_REPORT_POWER_STATUS:         // power status
        case CECOP_CEC_VERSION:                 // cec version
            parameterCount = 1;
            break;
        case CECOP_INACTIVE_SOURCE:             // physical address
        case CECOP_FEATURE_ABORT:               // feature opcode / abort reason
        case CECOP_ACTIVE_SOURCE:               // physical address
            parameterCount = 2;
            break;
        case CECOP_REPORT_PHYSICAL_ADDRESS:     // physical address / device type
        case CECOP_DEVICE_VENDOR_ID:            // vendor id
            parameterCount = 3;
            break;
        case CECOP_SET_OSD_NAME:                // osd name (1-14 bytes)
        case CECOP_SET_OSD_STRING:              // 1 + x   display control / osd string (1-13 bytes)
            parameterCount = 1;                 // must have a minimum of 1 operands
            break;
        case CECOP_ABORT:
            break;

        case CECOP_ARC_INITIATE:
            break;
        case CECOP_ARC_REPORT_INITIATED:
            break;
        case CECOP_ARC_REPORT_TERMINATED:
            break;

        case CECOP_ARC_REQUEST_INITIATION:
            break;
        case CECOP_ARC_REQUEST_TERMINATION:
            break;
        case CECOP_ARC_TERMINATE:
            break;
        default:
            break;
    }

    /* Test for correct parameter count.    */

    if ( pCpi->argCount < parameterCount )
    {
        countOK = false;
    }

    return( countOK );
}

static bool CecRxMsgHandlerLast ( CEC_FrameData_t *pCpi )
{
    bool				isDirectAddressed;
    CEC_FrameData_t		cecFrame;

    isDirectAddressed = !((pCpi->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );

    if ( ValidateCecMessage( pCpi ))            // If invalid message, ignore it, but treat it as handled
    {
        if ( isDirectAddressed )
        {
            switch ( pCpi->opcode )
            {
                case CECOP_FEATURE_ABORT:
                    CecHandleFeatureAbort( pCpi );
                    break;

                case CECOP_IMAGE_VIEW_ON:       // In our case, respond the same to both these messages
                case CECOP_TEXT_VIEW_ON:
                    break;

                case CECOP_STANDBY:             // Direct and Broadcast

                        /* Setting this here will let the main task know    */
                        /* (via SI_CecGetPowerState) and at the same time   */
                        /* prevent us from broadcasting a STANDBY message   */
                        /* of our own when the main task responds by        */
                        /* calling SI_CecSetPowerState( STANDBY );          */

                    Cec.powerstatus = CEC_POWERSTATUS_STANDBY;
                    break;

                case CECOP_INACTIVE_SOURCE:
                    CecHandleInactiveSource( pCpi );
                    break;

                case CECOP_GIVE_PHYSICAL_ADDRESS:

                    /* TV responds by broadcasting its Physical Address: 0.0.0.0   */

                    cecFrame.opcode        = CECOP_REPORT_PHYSICAL_ADDRESS;
                    cecFrame.srcDestAddr   = MAKE_SRCDEST( Cec.address_logic, CEC_LOGADDR_UNREGORBC );
                    cecFrame.args[0]       = (Cec.address_phy&0xFF00)>>8;             // [Physical Address]
                    cecFrame.args[1]       = (Cec.address_phy&0x00FF);             // [Physical Address]
                    cecFrame.args[2]       = Cec.address_logic;//CEC_LOGADDR_PLAYBACK1;//2011.08.03 CEC_LOGADDR_TV;   // [Device Type] = 0 = TV
                    cecFrame.argCount      = 3;
                    CecSendFrame( &cecFrame );
                    break;

                case CECOP_GIVE_DEVICE_POWER_STATUS:

                    /* TV responds with power status.   */

                    cecFrame.opcode        = CECOP_REPORT_POWER_STATUS;
                    cecFrame.srcDestAddr   = MAKE_SRCDEST( Cec.address_logic, (pCpi->srcDestAddr & 0xF0) >> 4 );
                    cecFrame.args[0]       = Cec.powerstatus;
                    cecFrame.argCount      = 1;
                    CecSendFrame( &cecFrame );
                    break;

                case CECOP_GET_MENU_LANGUAGE:

                    /* TV Responds with a Set Menu language command.    */

                    cecFrame.opcode         = CECOP_SET_MENU_LANGUAGE;
                    cecFrame.srcDestAddr    = CEC_LOGADDR_UNREGORBC;
                    cecFrame.args[0]        = 'e';     // [language code see iso/fdis 639-2]
                    cecFrame.args[1]        = 'n';     // [language code see iso/fdis 639-2]
                    cecFrame.args[2]        = 'g';     // [language code see iso/fdis 639-2]
                    cecFrame.argCount       = 3;
                    CecSendFrame( &cecFrame );
                    break;

                case CECOP_GET_CEC_VERSION:

                    /* TV responds to this request with it's CEC version support.   */

                    cecFrame.srcDestAddr   = MAKE_SRCDEST( Cec.address_logic, (pCpi->srcDestAddr & 0xF0) >> 4 );
                    cecFrame.opcode        = CECOP_CEC_VERSION;
                    cecFrame.args[0]       = 0x04;       // Report CEC1.3a
                    cecFrame.argCount      = 1;
                    CecSendFrame( &cecFrame );
                    break;

                case CECOP_REPORT_POWER_STATUS:         // Someone sent us their power state.

//                    l_sourcePowerStatus = pCpi->args[0];
//
//                        /* Let NEW SOURCE task know about it.   */
//
//                    if ( l_cecTaskState.task == SI_CECTASK_NEWSOURCE )
//                    {
//                        l_cecTaskState.cpiState = CPI_RESPONSE;
//                    }
                    break;

                /* Do not reply to directly addressed 'Broadcast' msgs.  */

                case CECOP_ACTIVE_SOURCE:
                case CECOP_REPORT_PHYSICAL_ADDRESS:     // A physical address was broadcast -- ignore it.
                case CECOP_REQUEST_ACTIVE_SOURCE:       // We are not a source, so ignore this one.
                case CECOP_ROUTING_CHANGE:              // We are not a downstream switch, so ignore this one.
                case CECOP_ROUTING_INFORMATION:         // We are not a downstream switch, so ignore this one.
                case CECOP_SET_STREAM_PATH:             // We are not a source, so ignore this one.
                case CECOP_SET_MENU_LANGUAGE:           // As a TV, we can ignore this message
                case CECOP_DEVICE_VENDOR_ID:
                    break;

                case CECOP_ABORT:       // Send Feature Abort for all unsupported features.
                default:

                    CecSendFeatureAbort( pCpi, CECAR_UNRECOG_OPCODE );
                    break;
            }
        }

        /* Respond to broadcast messages.   */

        else
        {
            switch ( pCpi->opcode )
            {
                case CECOP_STANDBY:

                        /* Setting this here will let the main task know    */
                        /* (via SI_CecGetPowerState) and at the same time   */
                        /* prevent us from broadcasting a STANDBY message   */
                        /* of our own when the main task responds by        */
                        /* calling SI_CecSetPowerState( STANDBY );          */

                    Cec.powerstatus = CEC_POWERSTATUS_STANDBY;
                    break;

                case CECOP_ACTIVE_SOURCE:
//                    CecHandleActiveSource( pCpi );
                    break;

                case CECOP_REPORT_PHYSICAL_ADDRESS:
//                    CecHandleReportPhysicalAddress( pCpi );
                    break;

                /* Do not reply to 'Broadcast' msgs that we don't need.  */

                case CECOP_REQUEST_ACTIVE_SOURCE:       // We are not a source, so ignore this one.
//                	SI_StartActiveSource(0,0);//2011.08.03
					break;
                case CECOP_ROUTING_CHANGE:              // We are not a downstream switch, so ignore this one.
                case CECOP_ROUTING_INFORMATION:         // We are not a downstream switch, so ignore this one.
                case CECOP_SET_STREAM_PATH:             // We are not a source, so ignore this one.
                case CECOP_SET_MENU_LANGUAGE:           // As a TV, we can ignore this message
                    break;
            }
        }
    }

    return 0;
}
static void CecWorkFunc(struct work_struct *work)
{
	struct Cec_delayed_work *cec_w =
		container_of(work, struct Cec_delayed_work, work.work);
	CEC_FrameData_t cecFrame;
	int i;
	
	switch(cec_w->event)
	{
		case EVENT_ENUMERATE:
			break;
		case EVENT_RX_FRAME:
			memset(&cecFrame, 0, sizeof(CEC_FrameData_t));
			CecReadFrame(&cecFrame);
			CecRxMsgHandlerLast(&cecFrame);
			break;
		default:
			break;
	}
	
	if(cec_w->data)
		kfree(cec_w->data);
	kfree(cec_w);
}

static void CecSubmitWork(int event, int delay, void *data)
{
	struct Cec_delayed_work *work;

//	DBG("%s event %04x delay %d", __FUNCTION__, event, delay);
	
	work = kmalloc(sizeof(struct Cec_delayed_work), GFP_ATOMIC);

	if (work) {
		INIT_DELAYED_WORK(&work->work, CecWorkFunc);
		work->event = event;
		work->data = data;
		queue_delayed_work(Cec.workqueue,
				   &work->work,
				   msecs_to_jiffies(delay));
	} else {
		CECDBG(KERN_WARNING "GPIO CEC: Cannot allocate memory to "
				    "create work\n");;
	}
}

int CecEnumerate(void)
{
	int i;
	
//	for(i = 0; i < 3; i++) {
//		if(Cec_Ping(LA_Player[i]) == 1) {
//			Cec.address_logic = LA_Player[i];
//			break;
//		}
//	}
//	if(i == 3)
//		return -1;
	// Broadcast our physical address.
//	GPIO_CecSendMessage(CECOP_GET_MENU_LANGUAGE,CEC_LOGADDR_TV);
//	msleep(100);
	Cec.address_logic = LA_Player[0];
	HDMIWrReg(CEC_LOGICADDR, Cec.address_logic);
	StartActiveSource();
	return 0;
}

void CecSetDevicePA(int devPa)
{
	CECDBG("Physical Address is %02x", devPa);
	Cec.address_phy = devPa;
}

int CecInit(struct hdmi *hdmi)
{	
    if(hdmi == NULL)
    	return -1;
    	
	rk3028a_hdmi = hdmi->property->priv;
    
    memset(&Cec, 0, sizeof(Cec_t));
    Cec.workqueue = create_singlethread_workqueue("cec");
	if (Cec.workqueue == NULL) {
		CECDBG(KERN_ERR "GPIO CEC: create workqueue failed.\n");
		return -1;
	}
	init_waitqueue_head(&Cec.wait);
	
	//Fref = Fsys / ((register 0xd4 + 1)*(register 0xd5 + 1))
	//Fref = 0.5M, Fsys = 48M
	HDMIWrReg(CEC_CLK_H, 7);
	HDMIWrReg(CEC_CLK_L, 11);
	
	//Set bus free time to 16.8ms
	HDMIWrReg(CEC_BUSFREETIME_L, 0xd0);
	HDMIWrReg(CEC_BUSFREETIME_H, 0x20);
	
	//Enable TX/RX INT
	HDMIWrReg(CEC_TX_INT, 0xFF);
	HDMIWrReg(CEC_RX_INT, 0xFF);
	
    return 0;
}

void CecIsr(void)
{
	int tx_isr = 0, rx_isr = 0;
	
	tx_isr = HDMIRdReg(CEC_TX_INT);
	rx_isr = HDMIRdReg(CEC_RX_INT);
	
	CECDBG("tx_isr %02x  rx_isr %02x\n\n", tx_isr, rx_isr);
	
	HDMIWrReg(CEC_TX_INT, tx_isr);
	HDMIWrReg(CEC_RX_INT, rx_isr);
	
	if(tx_isr & m_TX_BUSNOTFREE) {
		Cec.busfree = 0;
		wake_up_interruptible_all(&Cec.wait);
	}
	else if(tx_isr & m_TX_DONE) {
		Cec.tx_done = 1;
		wake_up_interruptible_all(&Cec.wait);
	}
	else {
		Cec.tx_done = -1;
		wake_up_interruptible_all(&Cec.wait);
	}	
	if(rx_isr & m_RX_DONE)
		CecSubmitWork(EVENT_RX_FRAME, 0, NULL);
}