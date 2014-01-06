/*
 * CoreChips SR9800 based USB 2.0 Ethernet Devices
 *
 * Copyright (C) 2013 Joker Liu <Joker_liu@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// #define	DEBUG			// error path messages, extra info
// #define	VERBOSE			// more; success messages

#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/workqueue.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/crc32.h>
#include <linux/usb/usbnet.h>
#include <linux/slab.h>

#define	SR9800_AUTOMAC

#define DRIVER_VERSION "11-Jul-2013"
#define DRIVER_NAME "CoreChips"

/* CoreChips SR9800 based USB 2.0 Ethernet Devices */

#define SR_CMD_SET_SW_MII		0x06
#define SR_CMD_READ_MII_REG		0x07
#define SR_CMD_WRITE_MII_REG		0x08
#define SR_CMD_SET_HW_MII		0x0a
#define SR_CMD_READ_EEPROM		0x0b
#define SR_CMD_WRITE_EEPROM		0x0c
#define SR_CMD_WRITE_ENABLE		0x0d
#define SR_CMD_WRITE_DISABLE		0x0e
#define SR_CMD_READ_RX_CTL		0x0f
#define SR_CMD_WRITE_RX_CTL		0x10
#define SR_CMD_READ_IPG012		0x11
#define SR_CMD_WRITE_IPG0		0x12
#define SR_CMD_WRITE_IPG1		0x13
#define SR_CMD_READ_NODE_ID		0x13
#define SR_CMD_WRITE_NODE_ID		0x14
#define SR_CMD_WRITE_IPG2		0x14
#define SR_CMD_WRITE_MULTI_FILTER	0x16
#define SR_CMD_READ_PHY_ID		0x19
#define SR_CMD_READ_MEDIUM_STATUS	0x1a
#define SR_CMD_WRITE_MEDIUM_MODE	0x1b
#define SR_CMD_READ_MONITOR_MODE	0x1c
#define SR_CMD_WRITE_MONITOR_MODE	0x1d
#define SR_CMD_READ_GPIOS		0x1e
#define SR_CMD_WRITE_GPIOS		0x1f
#define SR_CMD_SW_RESET			0x20
#define SR_CMD_SW_PHY_STATUS		0x21
#define SR_CMD_SW_PHY_SELECT		0x22

#define SR_MONITOR_MODE			0x01
#define SR_MONITOR_LINK			0x02
#define SR_MONITOR_MAGIC		0x04
#define SR_MONITOR_HSFS			0x10

#define SR_MCAST_FILTER_SIZE		8
#define SR_MAX_MCAST			64

#define SR_SWRESET_CLEAR		0x00
#define SR_SWRESET_RR			0x01
#define SR_SWRESET_RT			0x02
#define SR_SWRESET_PRTE			0x04
#define SR_SWRESET_PRL			0x08
#define SR_SWRESET_BZ			0x10
#define SR_SWRESET_IPRL			0x20
#define SR_SWRESET_IPPD			0x40

#define SR9800_IPG0_DEFAULT		0x15
#define SR9800_IPG1_DEFAULT		0x0c
#define SR9800_IPG2_DEFAULT		0x12

/* SR9800 Medium Mode Register */
#define SR_MEDIUM_PF		0x0080
#define SR_MEDIUM_JFE		0x0040
#define SR_MEDIUM_TFC		0x0020
#define SR_MEDIUM_RFC		0x0010
#define SR_MEDIUM_ENCK		0x0008
#define SR_MEDIUM_AC		0x0004
#define SR_MEDIUM_FD		0x0002
#define SR_MEDIUM_GM		0x0001
#define SR_MEDIUM_SM		0x1000
#define SR_MEDIUM_SBP		0x0800
#define SR_MEDIUM_PS		0x0200
#define SR_MEDIUM_RE		0x0100

#define SR9800_MEDIUM_DEFAULT	\
	(SR_MEDIUM_FD | SR_MEDIUM_RFC | \
	 SR_MEDIUM_TFC | SR_MEDIUM_PS | \
	 SR_MEDIUM_AC | SR_MEDIUM_RE)

/* SR9800 RX_CTL values */
#define SR_RX_CTL_SO		0x0080
#define SR_RX_CTL_AP		0x0020
#define SR_RX_CTL_AM		0x0010
#define SR_RX_CTL_AB		0x0008
#define SR_RX_CTL_SEP		0x0004
#define SR_RX_CTL_AMALL		0x0002
#define SR_RX_CTL_PRO		0x0001
#define SR_RX_CTL_RH0M		0x0000
#define SR_RX_CTL_RH1M		0x0100
#define SR_RX_CTL_RH2M		0x0200
#define SR_RX_CTL_RH3M		0x0400

#define SR_DEFAULT_RX_CTL	(SR_RX_CTL_SO | SR_RX_CTL_AB | SR_RX_CTL_RH1M)

/* GPIO 0 .. 2 toggles */
#define SR_GPIO_GPO0EN		0x01	/* GPIO0 Output enable */
#define SR_GPIO_GPO_0		0x02	/* GPIO0 Output value */
#define SR_GPIO_GPO1EN		0x04	/* GPIO1 Output enable */
#define SR_GPIO_GPO_1		0x08	/* GPIO1 Output value */
#define SR_GPIO_GPO2EN		0x10	/* GPIO2 Output enable */
#define SR_GPIO_GPO_2		0x20	/* GPIO2 Output value */
#define SR_GPIO_RESERVED	0x40	/* Reserved */
#define SR_GPIO_RSE		0x80	/* Reload serial EEPROM */

#define SR_EEPROM_MAGIC		0xdeadbeef
#define SR9800_EEPROM_LEN	0xff

#define sr9800_MAX_BULKIN_2K		0
#define sr9800_MAX_BULKIN_4K		1
#define sr9800_MAX_BULKIN_6K		2
#define sr9800_MAX_BULKIN_8K		3
#define sr9800_MAX_BULKIN_16K		4
#define sr9800_MAX_BULKIN_20K		5
#define sr9800_MAX_BULKIN_24K		6
#define sr9800_MAX_BULKIN_32K		7
struct {unsigned short size, byte_cnt,threshold;} SR9800_BULKIN_SIZE[] =
{
	/* 2k */
	{2048, 0x8000, 0x8001},
	/* 4k */
	{4096, 0x8100, 0x8147},
	/* 6k */
	{6144, 0x8200, 0x81EB},
	/* 8k */
	{8192, 0x8300, 0x83D7},
	/* 16 */
	{16384, 0x8400, 0x851E},
	/* 20k */
	{20480, 0x8500, 0x8666},
	/* 24k */
	{24576, 0x8600, 0x87AE},
	/* 32k */
	{32768, 0x8700, 0x8A3D},
};

#ifdef	SR9800_AUTOMAC
#define	SR9800_MAC_FILE			"/data/SR9800_MAC_ADDR"
/* Global variables for file-based MAC address Machenism */
int mac_used[129] = {0};
int dev_addr[129] = {0};
DEFINE_SPINLOCK(sr9800_lock);
#endif

/* This structure cannot exceed sizeof(unsigned long [5]) AKA 20 bytes */
struct sr_data {
	u8 multi_filter[SR_MCAST_FILTER_SIZE];
	u8 mac_addr[ETH_ALEN];
	u8 phymode;
	u8 ledmode;
	u8 eeprom_len;
};

struct sr9800_int_data {
	__le16 res1;
	u8 link;
	__le16 res2;
	u8 status;
	__le16 res3;
} __packed;

static int sr_read_cmd(struct usbnet *dev, u8 cmd, u16 value, u16 index,
			    u16 size, void *data)
{
	void *buf;
	int err = -ENOMEM;

	netdev_dbg(dev->net, "sr_read_cmd() cmd=0x%02x value=0x%04x index=0x%04x size=%d\n",
		   cmd, value, index, size);

	buf = kmalloc(size, GFP_KERNEL);
	if (!buf)
		goto out;

	err = usb_control_msg(
		dev->udev,
		usb_rcvctrlpipe(dev->udev, 0),
		cmd,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		buf,
		size,
		USB_CTRL_GET_TIMEOUT);
	if (err == size)
		memcpy(data, buf, size);
	else if (err >= 0)
		err = -EINVAL;
	kfree(buf);

out:
	return err;
}

static int sr_write_cmd(struct usbnet *dev, u8 cmd, u16 value, u16 index,
			     u16 size, void *data)
{
	void *buf = NULL;
	int err = -ENOMEM;

	netdev_dbg(dev->net, "sr_write_cmd() cmd=0x%02x value=0x%04x index=0x%04x size=%d\n",
		   cmd, value, index, size);

	if (data) {
		buf = kmemdup(data, size, GFP_KERNEL);
		if (!buf)
			goto out;
	}

	err = usb_control_msg(
		dev->udev,
		usb_sndctrlpipe(dev->udev, 0),
		cmd,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		buf,
		size,
		USB_CTRL_SET_TIMEOUT);
	kfree(buf);

out:
	return err;
}

static void sr_async_cmd_callback(struct urb *urb)
{
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)urb->context;
	int status = urb->status;

	if (status < 0)
		printk(KERN_DEBUG "sr_async_cmd_callback() failed with %d",
			status);

	kfree(req);
	usb_free_urb(urb);
}

static void
sr_write_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,
				    u16 size, void *data)
{
	struct usb_ctrlrequest *req;
	int status;
	struct urb *urb;

	netdev_dbg(dev->net, "sr_write_cmd_async() cmd=0x%02x value=0x%04x index=0x%04x size=%d\n",
		   cmd, value, index, size);

	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb) {
		netdev_err(dev->net, "Error allocating URB in write_cmd_async!\n");
		return;
	}

	req = kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC);
	if (!req) {
		netdev_err(dev->net, "Failed to allocate memory for control request\n");
		usb_free_urb(urb);
		return;
	}

	req->bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->bRequest = cmd;
	req->wValue = cpu_to_le16(value);
	req->wIndex = cpu_to_le16(index);
	req->wLength = cpu_to_le16(size);

	usb_fill_control_urb(urb, dev->udev,
			     usb_sndctrlpipe(dev->udev, 0),
			     (void *)req, data, size,
			     sr_async_cmd_callback, req);

	status = usb_submit_urb(urb, GFP_ATOMIC);
	if (status < 0) {
		netdev_err(dev->net, "Error submitting the control message: status=%d\n",
			   status);
		kfree(req);
		usb_free_urb(urb);
	}
}

static int sr_rx_fixup(struct usbnet *dev, struct sk_buff *skb)
{
	u8  *head;
	u32  header;
	char *packet;
	struct sk_buff *sr_skb;
	u16 size;

	head = (u8 *) skb->data;
	memcpy(&header, head, sizeof(header));
	le32_to_cpus(&header);
	packet = head + sizeof(header);

	skb_pull(skb, 4);

	while (skb->len > 0) {
		if ((header & 0x07ff) != ((~header >> 16) & 0x07ff))
			netdev_err(dev->net, "sr_rx_fixup() Bad Header Length\n");

		/* get the packet length */
		size = (u16) (header & 0x000007ff);

		if ((skb->len) - ((size + 1) & 0xfffe) == 0) {
			u8 alignment = (unsigned long)skb->data & 0x3;
			if (alignment != 0x2) {
				/*
				 * not 16bit aligned so use the room provided by
				 * the 32 bit header to align the data
				 *
				 * note we want 16bit alignment as MAC header is
				 * 14bytes thus ip header will be aligned on
				 * 32bit boundary so accessing ipheader elements
				 * using a cast to struct ip header wont cause
				 * an unaligned accesses.
				 */
				u8 realignment = (alignment + 2) & 0x3;
				memmove(skb->data - realignment,
					skb->data,
					size);
				skb->data -= realignment;
				skb_set_tail_pointer(skb, size);
			}
			return 2;
		}

		if (size > dev->net->mtu + ETH_HLEN) {
			netdev_err(dev->net, "sr_rx_fixup() Bad RX Length %d\n",
				   size);
			return 0;
		}
		sr_skb = skb_clone(skb, GFP_ATOMIC);
		if (sr_skb) {
			u8 alignment = (unsigned long)packet & 0x3;
			sr_skb->len = size;

			if (alignment != 0x2) {
				/*
				 * not 16bit aligned use the room provided by
				 * the 32 bit header to align the data
				 */
				u8 realignment = (alignment + 2) & 0x3;
				memmove(packet - realignment, packet, size);
				packet -= realignment;
			}
			sr_skb->data = packet;
			skb_set_tail_pointer(sr_skb, size);
			usbnet_skb_return(dev, sr_skb);
		} else {
			return 0;
		}

		skb_pull(skb, (size + 1) & 0xfffe);

		if (skb->len < sizeof(header))
			break;

		head = (u8 *) skb->data;
		memcpy(&header, head, sizeof(header));
		le32_to_cpus(&header);
		packet = head + sizeof(header);
		skb_pull(skb, 4);
	}

	if (skb->len < 0) {
		netdev_err(dev->net, "sr_rx_fixup() Bad SKB Length %d\n",
			   skb->len);
		return 0;
	}
	return 1;
}

static struct sk_buff *sr_tx_fixup(struct usbnet *dev, struct sk_buff *skb,
					gfp_t flags)
{
	int padlen;
	int headroom = skb_headroom(skb);
	int tailroom = skb_tailroom(skb);
	u32 packet_len;
	u32 padbytes = 0xffff0000;

	padlen = ((skb->len + 4) % 512) ? 0 : 4;

	if ((!skb_cloned(skb)) &&
	    ((headroom + tailroom) >= (4 + padlen))) {
		if ((headroom < 4) || (tailroom < padlen)) {
			skb->data = memmove(skb->head + 4, skb->data, skb->len);
			skb_set_tail_pointer(skb, skb->len);
		}
	} else {
		struct sk_buff *skb2;
		skb2 = skb_copy_expand(skb, 4, padlen, flags);
		dev_kfree_skb_any(skb);
		skb = skb2;
		if (!skb)
			return NULL;
	}

	skb_push(skb, 4);
	packet_len = (((skb->len - 4) ^ 0x0000ffff) << 16) + (skb->len - 4);
	cpu_to_le32s(&packet_len);
	skb_copy_to_linear_data(skb, &packet_len, sizeof(packet_len));

	if ((skb->len % 512) == 0) {
		cpu_to_le32s(&padbytes);
		memcpy(skb_tail_pointer(skb), &padbytes, sizeof(padbytes));
		skb_put(skb, sizeof(padbytes));
	}
	return skb;
}

static void sr_status(struct usbnet *dev, struct urb *urb)
{
	struct sr9800_int_data *event;
	int link;

	if (urb->actual_length < 8)
		return;

	event = urb->transfer_buffer;
	link = event->link & 0x01;
	if (netif_carrier_ok(dev->net) != link) {
		if (link) {
			netif_carrier_on(dev->net);
			usbnet_defer_kevent (dev, EVENT_LINK_RESET );
		} else
			netif_carrier_off(dev->net);
		netdev_dbg(dev->net, "Link Status is: %d\n", link);
	}
}

static inline int sr_set_sw_mii(struct usbnet *dev)
{
	int ret;
	ret = sr_write_cmd(dev, SR_CMD_SET_SW_MII, 0x0000, 0, 0, NULL);
	if (ret < 0)
		netdev_err(dev->net, "Failed to enable software MII access\n");
	return ret;
}

static inline int sr_set_hw_mii(struct usbnet *dev)
{
	int ret;
	ret = sr_write_cmd(dev, SR_CMD_SET_HW_MII, 0x0000, 0, 0, NULL);
	if (ret < 0)
		netdev_err(dev->net, "Failed to enable hardware MII access\n");
	return ret;
}

static inline int sr_get_phy_addr(struct usbnet *dev)
{
	u8 buf[2];
	int ret = sr_read_cmd(dev, SR_CMD_READ_PHY_ID, 0, 0, 2, buf);

	netdev_dbg(dev->net, "sr_get_phy_addr()\n");

	if (ret < 0) {
		netdev_err(dev->net, "Error reading PHYID register: %02x\n", ret);
		goto out;
	}
	netdev_dbg(dev->net, "sr_get_phy_addr() returning 0x%04x\n",
		   *((__le16 *)buf));
	ret = buf[1];

out:
	return ret;
}

static int sr_sw_reset(struct usbnet *dev, u8 flags)
{
	int ret;

        ret = sr_write_cmd(dev, SR_CMD_SW_RESET, flags, 0, 0, NULL);
	if (ret < 0)
		netdev_err(dev->net, "Failed to send software reset: %02x\n", ret);

	return ret;
}

static u16 sr_read_rx_ctl(struct usbnet *dev)
{
	__le16 v;
	int ret = sr_read_cmd(dev, SR_CMD_READ_RX_CTL, 0, 0, 2, &v);

	if (ret < 0) {
		netdev_err(dev->net, "Error reading RX_CTL register: %02x\n", ret);
		goto out;
	}
	ret = le16_to_cpu(v);
out:
	return ret;
}

static int sr_write_rx_ctl(struct usbnet *dev, u16 mode)
{
	int ret;

	netdev_dbg(dev->net, "sr_write_rx_ctl() - mode = 0x%04x\n", mode);
	ret = sr_write_cmd(dev, SR_CMD_WRITE_RX_CTL, mode, 0, 0, NULL);
	if (ret < 0)
		netdev_err(dev->net, "Failed to write RX_CTL mode to 0x%04x: %02x\n",
			   mode, ret);

	return ret;
}

static u16 sr_read_medium_status(struct usbnet *dev)
{
	__le16 v;
	int ret = sr_read_cmd(dev, SR_CMD_READ_MEDIUM_STATUS, 0, 0, 2, &v);

	if (ret < 0) {
		netdev_err(dev->net, "Error reading Medium Status register: %02x\n",
			   ret);
		return ret;	/* TODO: callers not checking for error ret */
	}

	return le16_to_cpu(v);

}

static int sr_write_medium_mode(struct usbnet *dev, u16 mode)
{
	int ret;

	netdev_dbg(dev->net, "sr_write_medium_mode() - mode = 0x%04x\n", mode);
	ret = sr_write_cmd(dev, SR_CMD_WRITE_MEDIUM_MODE, mode, 0, 0, NULL);
	if (ret < 0)
		netdev_err(dev->net, "Failed to write Medium Mode mode to 0x%04x: %02x\n",
			   mode, ret);

	return ret;
}

static int sr_write_gpio(struct usbnet *dev, u16 value, int sleep)
{
	int ret;

	netdev_dbg(dev->net, "sr_write_gpio() - value = 0x%04x\n", value);
	ret = sr_write_cmd(dev, SR_CMD_WRITE_GPIOS, value, 0, 0, NULL);
	if (ret < 0)
		netdev_err(dev->net, "Failed to write GPIO value 0x%04x: %02x\n",
			   value, ret);

	if (sleep)
		msleep(sleep);

	return ret;
}

/*
 * SR9800 have a 16-bit RX_CTL value
 */
static void sr_set_multicast(struct net_device *net)
{
	struct usbnet *dev = netdev_priv(net);
	struct sr_data *data = (struct sr_data *)&dev->data;
	u16 rx_ctl = SR_DEFAULT_RX_CTL;

	if (net->flags & IFF_PROMISC) {
		rx_ctl |= SR_RX_CTL_PRO;
	} else if (net->flags & IFF_ALLMULTI ||
		   netdev_mc_count(net) > SR_MAX_MCAST) {
		rx_ctl |= SR_RX_CTL_AMALL;
	} else if (netdev_mc_empty(net)) {
		/* just broadcast and directed */
	} else {
		/* We use the 20 byte dev->data
		 * for our 8 byte filter buffer
		 * to avoid allocating memory that
		 * is tricky to free later */
		struct netdev_hw_addr *ha;
		u32 crc_bits;

		memset(data->multi_filter, 0, SR_MCAST_FILTER_SIZE);

		/* Build the multicast hash filter. */
		netdev_for_each_mc_addr(ha, net) {
			crc_bits = ether_crc(ETH_ALEN, ha->addr) >> 26;
			data->multi_filter[crc_bits >> 3] |=
			    1 << (crc_bits & 7);
		}

		sr_write_cmd_async(dev, SR_CMD_WRITE_MULTI_FILTER, 0, 0,
				   SR_MCAST_FILTER_SIZE, data->multi_filter);

		rx_ctl |= SR_RX_CTL_AM;
	}

	sr_write_cmd_async(dev, SR_CMD_WRITE_RX_CTL, rx_ctl, 0, 0, NULL);
}

static int sr_mdio_read(struct net_device *netdev, int phy_id, int loc)
{
	struct usbnet *dev = netdev_priv(netdev);
	__le16 res;

	mutex_lock(&dev->phy_mutex);
	sr_set_sw_mii(dev);
	sr_read_cmd(dev, SR_CMD_READ_MII_REG, phy_id,
				(__u16)loc, 2, &res);
	sr_set_hw_mii(dev);
	mutex_unlock(&dev->phy_mutex);

	netdev_dbg(dev->net, "sr_mdio_read() phy_id=0x%02x, loc=0x%02x, returns=0x%04x\n",
		   phy_id, loc, le16_to_cpu(res));

	return le16_to_cpu(res);
}

static void
sr_mdio_write(struct net_device *netdev, int phy_id, int loc, int val)
{
	struct usbnet *dev = netdev_priv(netdev);
	__le16 res = cpu_to_le16(val);

	netdev_dbg(dev->net, "sr_mdio_write() phy_id=0x%02x, loc=0x%02x, val=0x%04x\n",
		   phy_id, loc, val);
	mutex_lock(&dev->phy_mutex);
	sr_set_sw_mii(dev);
	sr_write_cmd(dev, SR_CMD_WRITE_MII_REG, phy_id, (__u16)loc, 2, &res);
	sr_set_hw_mii(dev);
	mutex_unlock(&dev->phy_mutex);
}

/* Get the PHY Identifier from the PHYSID1 & PHYSID2 MII registers */
static u32 sr_get_phyid(struct usbnet *dev)
{
	int phy_reg;
	u32 phy_id;
	int i;

	/* Poll for the rare case the FW or phy isn't ready yet.  */
	for (i = 0; i < 100; i++) {
		phy_reg = sr_mdio_read(dev->net, dev->mii.phy_id, MII_PHYSID1);
		if (phy_reg != 0 && phy_reg != 0xFFFF)
			break;
		mdelay(1);
	}

	if (phy_reg <= 0 || phy_reg == 0xFFFF)
		return 0;

	phy_id = (phy_reg & 0xffff) << 16;

	phy_reg = sr_mdio_read(dev->net, dev->mii.phy_id, MII_PHYSID2);
	if (phy_reg < 0)
		return 0;

	phy_id |= (phy_reg & 0xffff);

	return phy_id;
}

static void
sr_get_wol(struct net_device *net, struct ethtool_wolinfo *wolinfo)
{
	struct usbnet *dev = netdev_priv(net);
	u8 opt;

	if (sr_read_cmd(dev, SR_CMD_READ_MONITOR_MODE, 0, 0, 1, &opt) < 0) {
		wolinfo->supported = 0;
		wolinfo->wolopts = 0;
		return;
	}
	wolinfo->supported = WAKE_PHY | WAKE_MAGIC;
	wolinfo->wolopts = 0;
	if (opt & SR_MONITOR_LINK)
		wolinfo->wolopts |= WAKE_PHY;
	if (opt & SR_MONITOR_MAGIC)
		wolinfo->wolopts |= WAKE_MAGIC;
}

static int
sr_set_wol(struct net_device *net, struct ethtool_wolinfo *wolinfo)
{
	struct usbnet *dev = netdev_priv(net);
	u8 opt = 0;

	if (wolinfo->wolopts & WAKE_PHY)
		opt |= SR_MONITOR_LINK;
	if (wolinfo->wolopts & WAKE_MAGIC)
		opt |= SR_MONITOR_MAGIC;

	if (sr_write_cmd(dev, SR_CMD_WRITE_MONITOR_MODE,
			      opt, 0, 0, NULL) < 0)
		return -EINVAL;

	return 0;
}

static int sr_get_eeprom_len(struct net_device *net)
{
	struct usbnet *dev = netdev_priv(net);
	struct sr_data *data = (struct sr_data *)&dev->data;

	return data->eeprom_len;
}

static int sr_get_eeprom(struct net_device *net,
			      struct ethtool_eeprom *eeprom, u8 *data)
{
	struct usbnet *dev = netdev_priv(net);
	__le16 *ebuf = (__le16 *)data;
	int i;

	/* Crude hack to ensure that we don't overwrite memory
	 * if an odd length is supplied
	 */
	if (eeprom->len % 2)
		return -EINVAL;

	eeprom->magic = SR_EEPROM_MAGIC;

	/* sr9800 returns 2 bytes from eeprom on read */
	for (i=0; i < eeprom->len / 2; i++) {
		if (sr_read_cmd(dev, SR_CMD_READ_EEPROM,
			eeprom->offset + i, 0, 2, &ebuf[i]) < 0)
			return -EINVAL;
	}
	return 0;
}

static void sr_get_drvinfo (struct net_device *net,
				 struct ethtool_drvinfo *info)
{
	struct usbnet *dev = netdev_priv(net);
	struct sr_data *data = (struct sr_data *)&dev->data;

	/* Inherit standard device info */
	usbnet_get_drvinfo(net, info);
	strncpy (info->driver, DRIVER_NAME, sizeof info->driver);
	strncpy (info->version, DRIVER_VERSION, sizeof info->version);
	info->eedump_len = data->eeprom_len;
}

static u32 sr_get_link(struct net_device *net)
{
	struct usbnet *dev = netdev_priv(net);

	return mii_link_ok(&dev->mii);
}

static int sr_ioctl (struct net_device *net, struct ifreq *rq, int cmd)
{
	struct usbnet *dev = netdev_priv(net);

	return generic_mii_ioctl(&dev->mii, if_mii(rq), cmd, NULL);
}

static int sr_set_mac_address(struct net_device *net, void *p)
{
	struct usbnet *dev = netdev_priv(net);
	struct sr_data *data = (struct sr_data *)&dev->data;
	struct sockaddr *addr = p;

	if (netif_running(net))
		return -EBUSY;
	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(net->dev_addr, addr->sa_data, ETH_ALEN);

	/* We use the 20 byte dev->data
	 * for our 6 byte mac buffer
	 * to avoid allocating memory that
	 * is tricky to free later */
	memcpy(data->mac_addr, addr->sa_data, ETH_ALEN);
	sr_write_cmd_async(dev, SR_CMD_WRITE_NODE_ID, 0, 0, ETH_ALEN,
							data->mac_addr);

	return 0;
}

static const struct ethtool_ops sr9800_ethtool_ops = {
	.get_drvinfo		= sr_get_drvinfo,
	.get_link		= sr_get_link,
	.get_msglevel		= usbnet_get_msglevel,
	.set_msglevel		= usbnet_set_msglevel,
	.get_wol		= sr_get_wol,
	.set_wol		= sr_set_wol,
	.get_eeprom_len		= sr_get_eeprom_len,
	.get_eeprom		= sr_get_eeprom,
	.get_settings		= usbnet_get_settings,
	.set_settings		= usbnet_set_settings,
	.nway_reset		= usbnet_nway_reset,
};

static int sr9800_link_reset(struct usbnet *dev)
{
	u16 mode;
	struct ethtool_cmd ecmd = { .cmd = ETHTOOL_GSET };

	mii_check_media(&dev->mii, 1, 1);
	mii_ethtool_gset(&dev->mii, &ecmd);
	mode = SR9800_MEDIUM_DEFAULT;

	if (ethtool_cmd_speed(&ecmd) != SPEED_100)
		mode &= ~SR_MEDIUM_PS;

	if (ecmd.duplex != DUPLEX_FULL)
		mode &= ~SR_MEDIUM_FD;

	netdev_dbg(dev->net, "sr9800_link_reset() speed: %u duplex: %d setting mode to 0x%04x\n",
		   ethtool_cmd_speed(&ecmd), ecmd.duplex, mode);

	sr_write_medium_mode(dev, mode);

	return 0;
}

static int sr9800_reset(struct usbnet *dev)
{
	struct sr_data *data = (struct sr_data *)&dev->data;
	int ret, embd_phy;
	u16 rx_ctl;

	ret = sr_write_gpio(dev,
			SR_GPIO_RSE | SR_GPIO_GPO_2 | SR_GPIO_GPO2EN, 5);
	if (ret < 0)
		goto out;

	embd_phy = ((sr_get_phy_addr(dev) & 0x1f) == 0x10 ? 1 : 0);

	ret = sr_write_cmd(dev, SR_CMD_SW_PHY_SELECT, embd_phy, 0, 0, NULL);
	if (ret < 0) {
		dbg("Select PHY #1 failed: %d", ret);
		goto out;
	}

	ret = sr_sw_reset(dev, SR_SWRESET_IPPD | SR_SWRESET_PRL);
	if (ret < 0)
		goto out;

	msleep(150);

	ret = sr_sw_reset(dev, SR_SWRESET_CLEAR);
	if (ret < 0)
		goto out;

	msleep(150);

	if (embd_phy) {
		ret = sr_sw_reset(dev, SR_SWRESET_IPRL);
		if (ret < 0)
			goto out;
	} else {
		ret = sr_sw_reset(dev, SR_SWRESET_PRTE);
		if (ret < 0)
			goto out;
	}

	msleep(150);
	rx_ctl = sr_read_rx_ctl(dev);
	dbg("RX_CTL is 0x%04x after software reset", rx_ctl);
	ret = sr_write_rx_ctl(dev, 0x0000);
	if (ret < 0)
		goto out;

	rx_ctl = sr_read_rx_ctl(dev);
	dbg("RX_CTL is 0x%04x setting to 0x0000", rx_ctl);

	ret = sr_sw_reset(dev, SR_SWRESET_PRL);
	if (ret < 0)
		goto out;

	msleep(150);

	ret = sr_sw_reset(dev, SR_SWRESET_IPRL | SR_SWRESET_PRL);
	if (ret < 0)
		goto out;

	msleep(150);

	sr_mdio_write(dev->net, dev->mii.phy_id, MII_BMCR, BMCR_RESET);
	sr_mdio_write(dev->net, dev->mii.phy_id, MII_ADVERTISE,
			ADVERTISE_ALL | ADVERTISE_CSMA);
	mii_nway_restart(&dev->mii);

	ret = sr_write_medium_mode(dev, SR9800_MEDIUM_DEFAULT);
	if (ret < 0)
		goto out;

	ret = sr_write_cmd(dev, SR_CMD_WRITE_IPG0,
				SR9800_IPG0_DEFAULT | SR9800_IPG1_DEFAULT,
				SR9800_IPG2_DEFAULT, 0, NULL);
	if (ret < 0) {
		dbg("Write IPG,IPG1,IPG2 failed: %d", ret);
		goto out;
	}

	/* Rewrite MAC address */
	memcpy(data->mac_addr, dev->net->dev_addr, ETH_ALEN);
	ret = sr_write_cmd(dev, SR_CMD_WRITE_NODE_ID, 0, 0, ETH_ALEN,
							data->mac_addr);
	if (ret < 0)
		goto out;

	/* Set RX_CTL to default values with 2k buffer, and enable cactus */
	ret = sr_write_rx_ctl(dev, SR_DEFAULT_RX_CTL);
	if (ret < 0)
		goto out;

	rx_ctl = sr_read_rx_ctl(dev);
	dbg("RX_CTL is 0x%04x after all initializations", rx_ctl);

	rx_ctl = sr_read_medium_status(dev);
	dbg("Medium Status is 0x%04x after all initializations", rx_ctl);

	return 0;

out:
	return ret;

}

static const struct net_device_ops sr9800_netdev_ops = {
	.ndo_open		= usbnet_open,
	.ndo_stop		= usbnet_stop,
	.ndo_start_xmit		= usbnet_start_xmit,
	.ndo_tx_timeout		= usbnet_tx_timeout,
	.ndo_change_mtu		= usbnet_change_mtu,
	.ndo_set_mac_address 	= sr_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= sr_ioctl,
	.ndo_set_rx_mode        = sr_set_multicast,
};

static int sr9800_phy_powerup (struct usbnet *dev)
{
	int ret;

	/* set the embedded Ethernet PHY in power-down state */
	if ((ret = sr_sw_reset(dev, SR_SWRESET_IPPD | SR_SWRESET_IPRL)) < 0) {
		netdev_err(dev->net, "Failed to power down PHY : %d", ret);
		return ret;
	}
	msleep(10);
	
	/* set the embedded Ethernet PHY in power-up state */
	if ((ret = sr_sw_reset(dev, SR_SWRESET_IPRL)) < 0) {
		netdev_err(dev->net, "Failed to reset PHY: %d", ret);
		return ret;
	}
	msleep(600);

	/* set the embedded Ethernet PHY in reset state */
	if ((ret = sr_sw_reset(dev, SR_SWRESET_CLEAR)) < 0) {
		netdev_err(dev->net, "Failed to power up PHY: %d", ret);
		return ret;
	}
	msleep(20);

	/* set the embedded Ethernet PHY in power-up state */
	if ((ret = sr_sw_reset(dev, SR_SWRESET_IPRL)) < 0) {
		netdev_err(dev->net, "Failed to reset PHY: %d", ret);
		return ret;
	}

	return 0;
}

static int sr9800_bind(struct usbnet *dev, struct usb_interface *intf)
{
	int ret, embd_phy;
	struct sr_data *data = (struct sr_data *)&dev->data;
	u8 buf[ETH_ALEN];
	u32 phyid;
	u16 rx_ctl;
#ifdef	SR9800_AUTOMAC
	int i;
	u8 defaultAddress[] = {0x00,0x0e,0xc6,0x87,0x72,0x01};
	struct file *fp;
   	mm_segment_t fs; 
   	loff_t pos;
#endif

	data->eeprom_len = SR9800_EEPROM_LEN;

	usbnet_get_endpoints(dev,intf);

	/* set the led as :
	 * MFA0 : Link led
	 * MFA1 : Link & Active led
	 * MFA2 : Link & Active led (ignored, none with sr9800)
	 * MFA3 : Fiber Remote Fault led (ignored, none with sr9800)
	 */
	if ((ret = sr_write_cmd(dev, 0x70, 0x4080, 0x0140, 0, NULL)) < 0) {
			netdev_err(dev->net, "set LINK LED failed : %d", ret);
			goto out;
	}

	/* Get the MAC address */
	ret = sr_read_cmd(dev, SR_CMD_READ_NODE_ID, 0, 0, ETH_ALEN, buf);
	if (ret < 0) {
		dbg("Failed to read MAC address: %d", ret);
		return ret;
	}
	memcpy(dev->net->dev_addr, buf, ETH_ALEN);
    printk("MAC : %2x:%2x:%2x:%2x:%2x:%2x\n",
					buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

#ifdef	SR9800_AUTOMAC
	/* Compare with the file-based MAC address
	 * This is for dongle or product without EEPROM condition
	 */
	if (memcmp( dev->net->dev_addr, defaultAddress, ETH_ALEN ) == 0)
	{
		spin_lock(&sr9800_lock);
		for(i=0; i<129; i++)
		{
			if(mac_used[i] == 0)
			{
				fp = filp_open(SR9800_MAC_FILE, O_RDONLY, 0); 

				if (IS_ERR(fp)) 
				{ 
					netdev_err(dev->net, "open file error");
					fp = filp_open(SR9800_MAC_FILE, O_RDWR | O_CREAT, 0644); 
					if (IS_ERR(fp)) 
					{
						netdev_err(dev->net, "creat file error");        			
						goto out; 
					}
					fs = get_fs(); 
    				set_fs(KERNEL_DS);
					random_ether_addr(dev->net->dev_addr);
					*(u8 *)(dev->net->dev_addr + 0) = 0;
					pos = 0; 
					vfs_write(fp, dev->net->dev_addr, ETH_ALEN, &pos); 	
    				} 
				else
				{
					fs = get_fs(); 
   					set_fs(KERNEL_DS);
				}
		
				pos = 0; 
				vfs_read(fp, dev->net->dev_addr, ETH_ALEN, &pos);

				filp_close(fp, NULL); 
    			set_fs(fs); 
		
				dev_addr[i] = dev->udev->devnum;
				mac_used[i] = 1;
		/*		*(u8*)(dev->net->dev_addr + 0) += i;
				*(u8*)(dev->net->dev_addr + 1) += i;
				*(u8*)(dev->net->dev_addr + 2) += i;
				*(u8*)(dev->net->dev_addr + 3) += i;
				*(u8*)(dev->net->dev_addr + 4) += i;*/
				*(u8*)(dev->net->dev_addr + 5) += i;
				break;
			}	
		}
		spin_unlock(&sr9800_lock);
		/* Set the MAC address */
		if ((ret = sr_write_cmd (dev, SR_CMD_WRITE_NODE_ID,
			0, 0, ETH_ALEN, dev->net->dev_addr)) < 0) {
			netdev_err(dev->net, "set MAC address failed: %d", ret);
			goto out;
		}
	}
	netif_carrier_off(dev->net);
#endif

	/* Initialize MII structure */
	dev->mii.dev = dev->net;
	dev->mii.mdio_read = sr_mdio_read;
	dev->mii.mdio_write = sr_mdio_write;
	dev->mii.phy_id_mask = 0x1f;
	dev->mii.reg_num_mask = 0x1f;
	dev->mii.phy_id = sr_get_phy_addr(dev);

	dev->net->netdev_ops = &sr9800_netdev_ops;
	dev->net->ethtool_ops = &sr9800_ethtool_ops;

	embd_phy = ((dev->mii.phy_id & 0x1f) == 0x10 ? 1 : 0);
	/* Reset the PHY to normal operation mode */
	ret = sr_write_cmd(dev, SR_CMD_SW_PHY_SELECT, embd_phy, 0, 0, NULL);
	if (ret < 0) {
		dbg("Select PHY #1 failed: %d", ret);
		return ret;
	}

	/* Init PHY routine */
	if(( ret = sr9800_phy_powerup(dev) ) < 0)
			goto out;

	rx_ctl = sr_read_rx_ctl(dev);
	dbg("RX_CTL is 0x%04x after software reset", rx_ctl);
	if ((ret = sr_write_rx_ctl(dev, 0x0000)) < 0)
		goto out;

	rx_ctl = sr_read_rx_ctl(dev);
	dbg("RX_CTL is 0x%04x setting to 0x0000", rx_ctl);

	/* Read PHYID register *AFTER* the PHY was reset properly */
	phyid = sr_get_phyid(dev);
	dbg("PHYID=0x%08x", phyid);

	/* medium mode setting */
	sr_mdio_write(dev->net, dev->mii.phy_id, MII_BMCR, BMCR_RESET);
	sr_mdio_write(dev->net, dev->mii.phy_id, MII_ADVERTISE,
			ADVERTISE_ALL | ADVERTISE_CSMA);
	mii_nway_restart(&dev->mii);

	if ((ret = sr_write_medium_mode(dev, SR9800_MEDIUM_DEFAULT)) < 0)
		goto out;

	if ((ret = sr_write_cmd(dev, SR_CMD_WRITE_IPG0,
				SR9800_IPG0_DEFAULT | SR9800_IPG1_DEFAULT,
				SR9800_IPG2_DEFAULT, 0, NULL)) < 0) {
		dbg("Write IPG,IPG1,IPG2 failed: %d", ret);
		goto out;
	}

	/* Set RX_CTL to default values with 2k buffer, and enable cactus */
	if ((ret = sr_write_rx_ctl(dev, SR_DEFAULT_RX_CTL)) < 0)
		goto out;

	rx_ctl = sr_read_rx_ctl(dev);
	dbg("RX_CTL is 0x%04x after all initializations", rx_ctl);

	rx_ctl = sr_read_medium_status(dev);
	dbg("Medium Status is 0x%04x after all initializations", rx_ctl);

#if		0
	/* framing packs multiple eth frames into a 2K usb bulk transfer */
	if (dev->driver_info->flags & FLAG_FRAMING_AX) {
		/* hard_mtu  is still the default - the device does not support
		   jumbo eth frames */
		dev->rx_urb_size = 2048;
	}
#else
	if (dev->udev->speed == USB_SPEED_HIGH) {
		if ((ret = sr_write_cmd (dev, 0x2A,
				SR9800_BULKIN_SIZE[1].byte_cnt,
				SR9800_BULKIN_SIZE[1].threshold,
				0, NULL)) < 0) {
			netdev_err(dev->net, "Reset RX_CTL failed: %d", ret);
			goto out;
		}
		dev->rx_urb_size = SR9800_BULKIN_SIZE[1].size;
	} else {
		if ((ret = sr_write_cmd (dev, 0x2A,
				0x8000, 0x8001, 0, NULL)) < 0) {
			netdev_err(dev->net, "Reset RX_CTL failed: %d", ret);
			goto out;
		}
		dev->rx_urb_size = 2048;
	}
	printk("sr9800_bind : setting rx_urb_size with : %d\n", dev->rx_urb_size);
#endif

	return 0;
out :
	return ret;
}

static void sr9800_unbind(struct usbnet *dev, struct usb_interface *intf)
{
#ifdef	SR9800_AUTOMAC
	int ret = 0;
	int i = 0;
	
    /* stop MAC operation */
	if ((ret = sr_write_rx_ctl(dev, 0x0000)) < 0) {
		netdev_err(dev->net, "stop MAC error in unbind routine.\n");
	}

    /* Power down PHY */
	if ((ret = sr_sw_reset(dev, SR_SWRESET_IPPD)) < 0) {
		netdev_err(dev->net, "Power Down PHY error in unbind routine.\n");
	}

	/* release the file-based mac flag */
    spin_lock(&sr9800_lock);
	for (i=0; i<129; i++) {
		if(dev_addr[i] != 0) {
			if(dev->udev->devnum == dev_addr[i]) {
					mac_used[i] = 0;
					dev_addr[i] = 0;
					break;
			}
		}
	}
    spin_unlock(&sr9800_lock);
#endif
	return;
}


static const struct driver_info sr9800_info = {
	.description = "CoreChips SR9800 USB 2.0 Ethernet",
	.bind = sr9800_bind,
	.unbind = sr9800_unbind,
	.status = sr_status,
	.link_reset = sr9800_link_reset,
	.reset = sr9800_reset,
	.flags = FLAG_ETHER | FLAG_FRAMING_AX | FLAG_LINK_INTR,
	.rx_fixup = sr_rx_fixup,
	.tx_fixup = sr_tx_fixup,
};

static const struct usb_device_id	products [] = {
{
	// SR9800
	USB_DEVICE(0x0fe6, 0x9800),
	.driver_info = (unsigned long) &sr9800_info,
},
	{ },		// END
};
MODULE_DEVICE_TABLE(usb, products);

static struct usb_driver sr_driver = {
	.name =		DRIVER_NAME,
	.id_table =	products,
	.probe =	usbnet_probe,
	.suspend =	usbnet_suspend,
	.resume =	usbnet_resume,
	.disconnect =	usbnet_disconnect,
	.supports_autosuspend = 1,
};

static int __init sr_init(void)
{
 	return usb_register(&sr_driver);
}
module_init(sr_init);

static void __exit sr_exit(void)
{
 	usb_deregister(&sr_driver);
}
module_exit(sr_exit);

MODULE_AUTHOR("Joker Liu");
MODULE_VERSION(DRIVER_VERSION);
MODULE_DESCRIPTION("CoreChips SR9800 based USB 2.0 Ethernet Devices");
MODULE_LICENSE("GPL");

