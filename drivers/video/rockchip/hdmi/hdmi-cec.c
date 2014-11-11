#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include "hdmi-cec.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/firmware.h>


static struct cec_device *cec_dev;
struct input_dev *devinput;
static struct miscdevice mdev;

int key_table[] = {
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_REPLY,
	KEY_BACK,
	KEY_POWER,
};

static int CecReadFrame(struct cec_framedata *Frame)
{
	if (Frame == NULL || !cec_dev || cec_dev->readframe == NULL)
		return -1;
	else
		return cec_dev->readframe(cec_dev->hdmi, Frame);
}

static int CecSendFrame(struct cec_framedata *Frame)
{
	if (Frame == NULL || !cec_dev || cec_dev->readframe == NULL)
		return -1;
	else
		return cec_dev->sendframe(cec_dev->hdmi, Frame);
}

static int CecSendPing(char LogicAddress)
{
	struct cec_framedata cecFrame;

	memset(&cecFrame, 0, sizeof(struct cec_framedata));
	cecFrame.srcDestAddr = LogicAddress << 4 | LogicAddress;
	return cec_dev->sendframe(cec_dev->hdmi, &cecFrame);
}

/*static int CecSendMessage (char opCode, char dest)
{
	struct cec_framedata cecFrame;

	cecFrame.opcode        = opCode;
	cecFrame.srcDestAddr   = MAKE_SRCDEST(cec_dev->address_logic, dest);
	cecFrame.argCount      = 0;

	return CecSendFrame(&cecFrame);
}*/


/*static void CecSendFeatureAbort (struct cec_framedata *pCpi, char reason)
{
	struct cec_framedata cecFrame;

	if ((pCpi->srcDestAddr & 0x0F) != CEC_LOGADDR_UNREGORBC) {
		cecFrame.opcode        = CECOP_FEATURE_ABORT;
		cecFrame.srcDestAddr   = MAKE_SRCDEST( cec_dev->address_logic,
					( pCpi->srcDestAddr & 0xF0) >> 4 );
		cecFrame.args[0]       = pCpi->opcode;
		cecFrame.args[1]       = reason;
		cecFrame.argCount      = 2;
		CecSendFrame(&cecFrame);
	}
}*/

static void CecHandleInactiveSource(struct cec_framedata *pCpi)
{

}

static void CecHandleFeatureAbort(struct cec_framedata *pCpi)
{

}

static bool ValidateCecMessage(struct cec_framedata *pCpi)
{
	char parameterCount = 0;
	bool    countOK = true;

	/* Determine required parameter count   */

	switch (pCpi->opcode) {
	case CECOP_IMAGE_VIEW_ON:
	case CECOP_TEXT_VIEW_ON:
	case CECOP_STANDBY:
	case CECOP_GIVE_PHYSICAL_ADDRESS:
	case CECOP_GIVE_DEVICE_POWER_STATUS:
	case CECOP_GET_MENU_LANGUAGE:
	case CECOP_GET_CEC_VERSION:
		parameterCount = 0;
		break;
	case CECOP_REPORT_POWER_STATUS:         /* power status*/
	case CECOP_CEC_VERSION:                 /* cec version*/
		parameterCount = 1;
		break;
	case CECOP_INACTIVE_SOURCE:             /* physical address*/
	case CECOP_FEATURE_ABORT:
	case CECOP_ACTIVE_SOURCE:               /* physical address*/
		parameterCount = 2;
		break;
	case CECOP_REPORT_PHYSICAL_ADDRESS:
	case CECOP_DEVICE_VENDOR_ID:            /* vendor id*/
		parameterCount = 3;
		break;
	case CECOP_SET_OSD_NAME:                /* osd name (1-14 bytes)*/
	case CECOP_SET_OSD_STRING:
		parameterCount = 1;    /* must have a minimum of 1 operands*/
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

	if (pCpi->argCount < parameterCount)
		countOK = false;

	return countOK;
}

static bool CecRxMsgHandlerLast(struct cec_framedata *pCpi)
{
	bool			isDirectAddressed;
	struct cec_framedata	cecFrame;

	isDirectAddressed = !((pCpi->srcDestAddr & 0x0F) ==
					CEC_LOGADDR_UNREGORBC);
	pr_info("isDirectAddressed %d\n", (int)isDirectAddressed);
	if (ValidateCecMessage(pCpi)) {
		/* If invalid message, ignore it, but treat it as handled */
	if (isDirectAddressed) {
		switch (pCpi->opcode) {
		case CECOP_USER_CONTROL_PRESSED:
			CECMenuControl(pCpi->args[0]);
			break;

		case CECOP_VENDOR_REMOTE_BUTTON_DOWN:
			CECMenuControl(pCpi->args[0]);
			break;
		case CECOP_FEATURE_ABORT:
			CecHandleFeatureAbort(pCpi);
			break;

		case CECOP_GIVE_OSD_NAME:
			cecFrame.opcode        = CECOP_SET_OSD_NAME;
			cecFrame.srcDestAddr =
				MAKE_SRCDEST(cec_dev->address_logic,
							CEC_LOGADDR_TV);
			cecFrame.args[0]  = 'R';
			cecFrame.args[1]  = 'K';
			cecFrame.args[2]  = '-';
			cecFrame.args[3]  = 'B';
			cecFrame.args[4]  = 'O';
			cecFrame.args[5]  = 'X';
			cecFrame.argCount      = 6;
			CecSendFrame(&cecFrame);
			break;

		case CECOP_VENDOR_COMMAND_WITH_ID:

		if (pCpi->args[2] == 00) {
			cecFrame.opcode        = CECOP_SET_OSD_NAME;
			cecFrame.srcDestAddr =
				MAKE_SRCDEST(cec_dev->address_logic,
							CEC_LOGADDR_TV);
			cecFrame.args[0]  = '1';
			cecFrame.args[1]  = '1';
			cecFrame.args[2]  = '1';
			cecFrame.args[3]  = '1';
			cecFrame.args[4]  = '1';
			cecFrame.args[5]  = '1';
			cecFrame.argCount      = 6;
			CecSendFrame(&cecFrame);
			}
			break;
		case CECOP_IMAGE_VIEW_ON:
		case CECOP_TEXT_VIEW_ON:
		/* In our case, respond the same to both these messages*/
		    break;

		case CECOP_GIVE_DEVICE_VENDOR_ID:
			cecFrame.opcode        = CECOP_DEVICE_VENDOR_ID;
			cecFrame.srcDestAddr   =
				MAKE_SRCDEST(cec_dev->address_logic,
						CEC_LOGADDR_UNREGORBC);
			cecFrame.args[0]       = 0x1;
			cecFrame.args[1]       = 0x2;
			cecFrame.args[2]       = 0x3;
			cecFrame.argCount      = 3;
			CecSendFrame(&cecFrame);
			break;

		case CECOP_STANDBY:             /* Direct and Broadcast*/
		/* Setting this here will let the main task know    */
		/* (via SI_CecGetPowerState) and at the same time   */
		/* prevent us from broadcasting a STANDBY message   */
		/* of our own when the main task responds by        */
		/* calling SI_CecSetPowerState( STANDBY );          */
			cec_dev->powerstatus = CEC_POWERSTATUS_STANDBY;
			break;

		case CECOP_INACTIVE_SOURCE:
			CecHandleInactiveSource(pCpi);
			break;

		case CECOP_GIVE_PHYSICAL_ADDRESS:

			cecFrame.opcode        = CECOP_REPORT_PHYSICAL_ADDRESS;
			cecFrame.srcDestAddr   =
				MAKE_SRCDEST(cec_dev->address_logic,
						CEC_LOGADDR_UNREGORBC);
			cecFrame.args[0]   = (cec_dev->address_phy&0xFF00)>>8;
			cecFrame.args[1]       = (cec_dev->address_phy&0x00FF);
			cecFrame.args[2]       = cec_dev->address_logic;
			cecFrame.argCount      = 3;
			CecSendFrame(&cecFrame);
			break;

		case CECOP_GIVE_DEVICE_POWER_STATUS:
		/* TV responds with power status.   */

			cecFrame.opcode        = CECOP_REPORT_POWER_STATUS;
			cecFrame.srcDestAddr   =
				MAKE_SRCDEST(cec_dev->address_logic,
					(pCpi->srcDestAddr & 0xF0) >> 4);
			cec_dev->powerstatus =  0x00;
			cecFrame.args[0]       = cec_dev->powerstatus;
			cecFrame.argCount      = 1;
			CecSendFrame(&cecFrame);
			break;

		case CECOP_GET_MENU_LANGUAGE:
		/* TV Responds with a Set Menu language command.    */

			cecFrame.opcode         = CECOP_SET_MENU_LANGUAGE;
			cecFrame.srcDestAddr    =
				MAKE_SRCDEST(cec_dev->address_logic,
						CEC_LOGADDR_UNREGORBC);
			cecFrame.args[0]        = 'e';
			cecFrame.args[1]        = 'n';
			cecFrame.args[2]        = 'g';
			cecFrame.argCount       = 3;
			CecSendFrame(&cecFrame);
			break;

		case CECOP_GET_CEC_VERSION:
		/* TV responds to this request with it's CEC version support.*/

			cecFrame.srcDestAddr   =
				MAKE_SRCDEST(cec_dev->address_logic,
						CEC_LOGADDR_TV);
			cecFrame.opcode        = CECOP_CEC_VERSION;
			cecFrame.args[0]       = 0x05;       /* Report CEC1.4b*/
			cecFrame.argCount      = 1;
			CecSendFrame(&cecFrame);
			break;

		case CECOP_REPORT_POWER_STATUS:
		/*Someone sent us their power state.

			l_sourcePowerStatus = pCpi->args[0];

			let NEW SOURCE task know about it.

			if ( l_cecTaskState.task == SI_CECTASK_NEWSOURCE )
			{
			l_cecTaskState.cpiState = CPI_RESPONSE;
			}*/
			 break;

		/* Do not reply to directly addressed 'Broadcast' msgs.  */
		case CECOP_REQUEST_ACTIVE_SOURCE:
			CecSendActiveSource();
			break;

		case CECOP_ACTIVE_SOURCE:
		case CECOP_REPORT_PHYSICAL_ADDRESS:
		case CECOP_ROUTING_CHANGE:
		case CECOP_ROUTING_INFORMATION:
		case CECOP_SET_STREAM_PATH:
		case CECOP_SET_MENU_LANGUAGE:
		case CECOP_DEVICE_VENDOR_ID:
			break;

		case CECOP_ABORT:
			break;
		default:
		/*CecSendFeatureAbort(pCpi, CECAR_UNRECOG_OPCODE);*/
			break;
			}
		} else {
			/* Respond to broadcast messages.   */
			switch (pCpi->opcode) {
			case CECOP_STANDBY:
			/* Setting this here will let the main task know    */
			/* (via SI_CecGetPowerState) and at the same time   */
			/* prevent us from broadcasting a STANDBY message   */
			/* of our own when the main task responds by        */
			/* calling SI_CecSetPowerState( STANDBY );          */
				cec_dev->powerstatus = CEC_POWERSTATUS_STANDBY;
				input_event(devinput, EV_KEY, KEY_POWER, 1);
				input_sync(devinput);
				input_event(devinput, EV_KEY, KEY_POWER, 0);
				input_sync(devinput);
				break;

			case CECOP_ACTIVE_SOURCE:
				/*CecHandleActiveSource( pCpi );*/
				break;

			case CECOP_REPORT_PHYSICAL_ADDRESS:
				/*CecHandleReportPhysicalAddress( pCpi );*/
				cecFrame.srcDestAddr   =
					MAKE_SRCDEST(cec_dev->address_logic,
							CEC_LOGADDR_UNREGORBC);
				cecFrame.opcode        = CECOP_CEC_VERSION;
				cecFrame.args[0]       = 0x05; /* CEC1.4b*/
				cecFrame.argCount      = 1;
				CecSendFrame(&cecFrame);
				break;

		/* Do not reply to 'Broadcast' msgs that we don't need.*/
			case CECOP_REQUEST_ACTIVE_SOURCE:
				CecSendActiveSource();
				break;
			case CECOP_ROUTING_CHANGE:
			case CECOP_ROUTING_INFORMATION:
			case CECOP_SET_STREAM_PATH:
			case CECOP_SET_MENU_LANGUAGE:
				break;
			}
		}
	}

	return 0;
}


void CecSendImageView(void)
{
	 struct cec_framedata cecFrame;

	 cecFrame.opcode		= CECOP_IMAGE_VIEW_ON;
	 cecFrame.srcDestAddr	= MAKE_SRCDEST(cec_dev->address_logic,
					CEC_LOGADDR_UNREGORBC);
	 cecFrame.argCount		= 0;
	 CecSendFrame(&cecFrame);
}



void CecSendActiveSource(void)
{
	struct cec_framedata cecFrame;

	cecFrame.opcode        = CECOP_ACTIVE_SOURCE;
	cecFrame.srcDestAddr   = MAKE_SRCDEST(cec_dev->address_logic,
						CEC_LOGADDR_UNREGORBC);
	cecFrame.args[0]       = (cec_dev->address_phy & 0xFF00) >> 8;
	cecFrame.args[1]       = (cec_dev->address_phy & 0x00FF);
	cecFrame.argCount      = 2;
	CecSendFrame(&cecFrame);
}
static void CecEnumeration(void)
{
	char LogicAddress[3] = {CEC_LOGADDR_PLAYBACK1,
				CEC_LOGADDR_PLAYBACK2,
				CEC_LOGADDR_PLAYBACK3};
	int i;

	if (!cec_dev)
		return;
	for (i = 0; i < 3; i++) {
		if (CecSendPing(LogicAddress[i])) {
			cec_dev->address_logic = LogicAddress[i];
			CECDBG("Logic Address is 0x%x\n",
					cec_dev->address_logic);
			break;
		}
	}
	if (i == 3)
		cec_dev->address_logic = CEC_LOGADDR_UNREGORBC;
	cec_dev->setceclogicaddr(cec_dev->hdmi, cec_dev->address_logic);
	CecSendImageView();
	CecSendActiveSource();
}

static void CecWorkFunc(struct work_struct *work)
{
	struct cec_delayed_work *cec_w =
		container_of(work, struct cec_delayed_work, work.work);
	struct cec_framedata cecFrame;

	switch (cec_w->event) {
	case EVENT_ENUMERATE:
		CecEnumeration();
		break;
	case EVENT_RX_FRAME:
		memset(&cecFrame, 0, sizeof(struct cec_framedata));
		CecReadFrame(&cecFrame);
		CecRxMsgHandlerLast(&cecFrame);
		break;
	default:
		break;
	}

	if (cec_w->data)
		kfree(cec_w->data);
	kfree(cec_w);
}

void hdmi_cec_submit_work(int event, int delay, void *data)
{
	struct cec_delayed_work *work;

	CECDBG("%s event %04x delay %d", __func__, event, delay);

	work = kmalloc(sizeof(struct cec_delayed_work), GFP_ATOMIC);

	if (work) {
		INIT_DELAYED_WORK(&work->work, CecWorkFunc);
		work->event = event;
		work->data = data;
		queue_delayed_work(cec_dev->workqueue,
				   &work->work,
				   msecs_to_jiffies(delay));
	} else {
		CECDBG(KERN_WARNING "CEC: Cannot allocate memory\n");
	}
}

void hdmi_cec_set_physical_address(int devPa)
{
	if (cec_dev)
		cec_dev->address_phy = devPa;
	CecEnumeration();
}

static int hdmi_cec_input_device_init(void)
{
	int err, i;

	devinput = input_allocate_device();
	 if (!devinput)
		return -ENOMEM;
	devinput->name = "hdmi_cec_key";
	/*devinput->dev.parent = &client->dev;*/
	devinput->phys = "hdmi_cec_key/input0";
	devinput->id.bustype = BUS_HOST;
	devinput->id.vendor = 0x0001;
	devinput->id.product = 0x0001;
	devinput->id.version = 0x0100;
	err = input_register_device(devinput);
	if (err < 0) {
		input_free_device(devinput);
		CECDBG("%s input device error", __func__);
		return err;
	}
	for (i = 0; i < (sizeof(key_table)/sizeof(int)); i++)
		input_set_capability(devinput, EV_KEY, key_table[i]);
	return 0;
}

void CECMenuControl(int uitemp)
{
	switch (uitemp) {
	case sCECmakesure:  /*make sure*/
		CECDBG("CEC UIcommand  makesure\n");
		input_event(devinput, EV_KEY, KEY_REPLY, 1);
		input_sync(devinput);
		input_event(devinput, EV_KEY, KEY_REPLY, 0);
		input_sync(devinput);
		break;
	case sCECup:  /*up*/
		CECDBG("CEC UIcommand  up\n");
		input_event(devinput, EV_KEY, KEY_UP, 1);
		input_sync(devinput);
		input_event(devinput, EV_KEY, KEY_UP, 0);
		input_sync(devinput);
		break;
	case sCECdown:  /*down*/
		CECDBG("CEC UIcommand  down\n");
		input_event(devinput, EV_KEY, KEY_DOWN, 1);
		input_sync(devinput);
		input_event(devinput, EV_KEY, KEY_DOWN, 0);
		input_sync(devinput);
		break;
	case sCECleft:  /*left*/
		CECDBG("CEC UIcommand  left\n");
		input_event(devinput, EV_KEY, KEY_LEFT , 1);
		input_sync(devinput);
		input_event(devinput, EV_KEY, KEY_LEFT , 0);
		input_sync(devinput);
		break;
	case sCECright:  /*right*/
		CECDBG("CEC UIcommand  right\n");
		input_event(devinput, EV_KEY, KEY_RIGHT, 1);
		input_sync(devinput);
		input_event(devinput, EV_KEY, KEY_RIGHT, 0);
		input_sync(devinput);
		break;
	case sCECback:  /*back*/
		CECDBG("CEC UIcommand  back\n");
		input_event(devinput, EV_KEY, KEY_BACK, 1);
		input_sync(devinput);
		input_event(devinput, EV_KEY, KEY_BACK, 0);
		input_sync(devinput);
		break;
	case sCECVendorback:
		CECDBG("CEC UIcommand  vendor back\n");
		input_event(devinput, EV_KEY, KEY_BACK, 1);
		input_sync(devinput);
		input_event(devinput, EV_KEY, KEY_BACK, 0);
		input_sync(devinput);
		break;
	}
}


static ssize_t  cec_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", cec_dev->cecval);
}

static ssize_t cec_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int ret;

	ret = sscanf(buf, "%s", cec_dev->cecval);
	return strnlen(buf, PAGE_SIZE);
}

static struct device_attribute cec_control_attr = {
	.attr = {.name = "cec", .mode = 0666},
	.show = cec_show,
	.store = cec_store,
};

int hdmi_cec_init(struct hdmi *hdmi,
		  int (*sendframe)(struct hdmi *, struct cec_framedata *),
		  int (*readframe)(struct hdmi *, struct cec_framedata *),
		  void (*setceclogicaddr)(struct hdmi *, int))
{
	int ret;

	mdev.minor = MISC_DYNAMIC_MINOR;
	mdev.name = "cec";
	mdev.mode = 0666;
	cec_dev = kmalloc(sizeof(struct cec_device), GFP_KERNEL);
	if (!cec_dev) {
		pr_err("HDMI CEC: kmalloc fail!");
		return -ENOMEM;
	}
	memset(cec_dev, 0, sizeof(struct cec_device));
	cec_dev->hdmi = hdmi;
	cec_dev->cecval[0] = '1';
	cec_dev->cecval[1] = '\0';
	cec_dev->sendframe = sendframe;
	cec_dev->readframe = readframe;
	cec_dev->setceclogicaddr = setceclogicaddr;
	cec_dev->workqueue = create_singlethread_workqueue("hdmi-cec");
	if (cec_dev->workqueue == NULL) {
		pr_err("HDMI CEC: create workqueue failed.\n");
		return -1;
	}
		hdmi_cec_input_device_init();
	if (misc_register(&mdev)) {
		pr_err("CEC: Could not add cec misc driver\n");
		goto error;
	}

	ret = device_create_file(mdev.this_device, &cec_control_attr);
	if (ret) {
		pr_err("CEC: Could not add sys file enable\n");
	goto error1;
	}
	return 0;

error1:
		misc_deregister(&mdev);
error:
		ret = -EINVAL;
	return ret;
}
