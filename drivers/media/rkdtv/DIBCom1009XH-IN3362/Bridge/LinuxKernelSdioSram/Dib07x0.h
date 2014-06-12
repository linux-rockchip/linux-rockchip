/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

/**************************************************************************************************
* @file "Dib07x0.h"
* @brief this file represents the interface between kernel and userspace.
*
***************************************************************************************************/
/* the dev-node structure will be:
 *   USB device:
 *       /dev/dib07x0/N
 *                     /control   - for querying ID, doing synchronous transfers via ioctl (to each endpoint)
 *                     /endpointX
 *                             -X = number of endpoint
 *
 *   PCI device: (specific for the dib0710)
 *       /dev/dib07x0/N
 *                     /control  - for querying ID and doing synchronous transfer via ioctl
 *                     /dvb0     - DVB0 (VideoPort1 in DVB Mode)
 *                     /dvb1     - DVB1 (VideoPort2 in DVB Mode)
 *                     /video    - Analog Video (VP1 in Analog Mode)
 *                     /audio    - Analog Audio (VP2 in Analog Mode)
 *                     /vbi      - VBI Data
 *
 *
 */
#ifndef _DIB07X0_H_
#define _DIB07X0_H_

#include <asm/types.h>

#define DIB07X0_CONTROL_NODE  "/dev/dib07x0/%d/control"
#define DIB07X0_ENDPOINT_NODE "/dev/dib07x0/%d/endpoint%d%s"

#define DIB07X0_API_VERSION 6

enum Dib07x0BusTypes 
{
   DIB07X0_USB,
   DIB07X0_PCI,
};

struct Dib07x0DeviceId 
{
   enum Dib07x0BusTypes Type;

   union 
   {
      struct 
      {
         __u8  intf_no;
         __u16 vendor_id;
         __u16 product_id;
      } usb;

      struct 
      {
         __u32 vendor_id;
         __u32 product_id;
         __u32 subsystem_vid;
         __u32 subsystem_pid;
      } pci;

      __u8 pad[500]; /* reserve some space for future binary compatibility */
   } u;
};

/* requests that can be done on a dib07x0/0/control-node which is USB - not to mix up with the RequestType of a control-transfer */
enum Dib07x0UsbRequestType 
{
   DIB07X0_USB_CONTROL_READ,
   DIB07X0_USB_CONTROL_WRITE,
   DIB07X0_USB_BULK_READ,
   DIB07X0_USB_BULK_WRITE,
};

/* this is directly copied from <linux/usb_ch9.h> */
#define DIB07X0_USB_DIR_OUT           0   /* to device */
#define DIB07X0_USB_DIR_IN         0x80   /* to host */

#define DIB07X0_USB_TYPE_MASK          (0x03 << 5)
#define DIB07X0_USB_TYPE_STANDARD      (0x00 << 5)
#define DIB07X0_USB_TYPE_CLASS         (0x01 << 5)
#define DIB07X0_USB_TYPE_VENDOR        (0x02 << 5)

struct Dib07x0UsbRequest 
{
   enum Dib07x0UsbRequestType RequestType;

   __u8   endpoint;
   __u32  timeout;

   int32_t result;

   struct 
   {
      __u8  request;
      __u8  RequestType; /* will be defaulted to VENDOR_REQUEST for now */
      __u16 Value;
      __u16 index;
   } setup;

   __u16 Length;
   __u8 *Buffer;
};

#define DIB07X0_USB_REQUEST_QUEUE_MAX 10
struct Dib07x0UsbRequestQueue 
{
   int32_t count;
   struct Dib07x0UsbRequest request[DIB07X0_USB_REQUEST_QUEUE_MAX];
};

enum Dib07x0UsbReadMode 
{
   /* enable concatenating Data read from multiple URBs */
   DIB07X0_USB_BUFFER_READ_MODE,

   /* force read on exactly one URB */
   DIB07X0_USB_URB_READ_MODE,
};

/* structure to be used to get/set the URB-configuration for asynchronous read/write */
struct Dib07x0UsbUrbConfig 
{
   __u8                    count;
   __u8                    frames; /* how many frames/URB - Isochronous only */
   enum Dib07x0UsbReadMode ReadMode;
   __u32                   buffer_size;
};

/* requests that can be done on a control-node which PCI */
enum Dib07x0PciRequestType
{
   DIB07X0_PCI_IOM_ACCESS,

   DIB07X0_PCI_SET_GPIO,
   DIB07X0_PCI_I2C_ACCESS,
};

struct Dib07x0PciRequest 
{
   enum Dib07x0PciRequestType RequestType;

   union 
   {
      struct 
      {
         __u32 relative_iom_addr;
         __u32 num_dwords;
         __u32 *Buffer;
      } iom_access;

      struct 
      {
         __u8 num;
         __u8 dir; // 1 = OUT, 0 = IN
         __u8 Val;
      } set_gpio;

      struct 
      {
         __u8  Addr;
         __u8 *tx;
         __u16 txlen;
         __u8 *rx;
         __u16 rxlen;
      } i2c_access;

      __u8 pad[500]; /* reserve some space for future binary compatibility */
   } u;
};

#define DIB07X0_IOCG_API_VERSION        _IOR(0xdc, 0, __u32)
#define DIB07X0_IOCG_ID                 _IOR(0xdc, 1, struct Dib07x0DeviceId)
#define DIB07X0_IOC_USB_REQUEST        _IOWR(0xdc, 2, struct Dib07x0UsbRequest)
#define DIB07X0_IOC_PCI_REQUEST        _IOWR(0xdc, 3, struct Dib07x0PciRequest)
#define DIB07X0_IOCG_USB_URB_CFG        _IOR(0xdc, 4, struct Dib07x0UsbUrbConfig)
#define DIB07X0_IOCS_USB_URB_CFG        _IOW(0xdc, 5, struct Dib07x0UsbUrbConfig)
#define DIB07X0_IOC_USB_REQUEST_QUEUE  _IOWR(0xdc, 6, struct Dib07x0UsbRequestQueue)

/* for kernel-to-kernel communication we can also use this header-file */
#ifdef __KERNEL__

#include <linux/pci.h>
#include <linux/usb.h>
#include <linux/cdev.h>
#include <linux/kref.h>
#include <linux/completion.h>

#include "Compat.h"

/* generic device structure, one per connected device */
struct Dib07x0State 
{
   int32_t devnum;                       /* acquired ID */

   struct Dib07x0DeviceId device_id;     /* information about the device */
   struct mutex           ctl_sem;       /* a semaphore to protect sensitive areas of the whole device */
   struct kref            kref;          /* kref for delayed memory free for a whole device */

   /* USB */
   struct usb_device *udev;              /* usb device - in case it is USB */

   u8 Buffer[65535];                     /* Buffer to be used for synchonized calls */

   /* PCI */
   struct pci_dev *pdev;
   void __iomem   *iomem;

   u32 __iomem                *iom_fw;
   struct leon_regs   __iomem *iom_leon_reg;
   struct HookConfig  __iomem *iom_hook_cfg;

   /* IRQ */
   spinlock_t IrqLock;
   u32        irq;

   int32_t last_i2c_status;

   struct completion i2c_done;
   struct mutex      pci_request_sem;


};

struct Dib07x0Dev 
{
   struct Dib07x0State *state;      /* the corresponding device */
   struct class_device *class_dev;  /* keeping the entry in the /sys-filesystem */
   struct cdev          cdev;       /* the char-dev representation */
   struct kref          kref;       /* kref for delayed memory free of a particular device node */
   int32_t minor;                   /* the acquired minor number */

   struct file_operations *fops;    /* real device operations on this node */
   struct file_operations *k_fops;  /* real device operations on this node, from the kernel */

   int32_t present;                 /* indicates if the device has been removed and the kfree is pending */

   uint32_t readers;                /* number of maximum/left readers */
   uint32_t writers;                /* number of maximum/left writers */

/* USB transfer stuff: less copying in kernel - maximum flexibility - for now only good with BULK */
#define DIB07X0_MAX_NO_URBS 150
   struct Dib07x0UsbUrbConfig urb_config; /* information of the current URB-setup for reading */
   struct Dib07x0Urb 
   {
      struct list_head list_head;

      int32_t     id;
      struct urb *urb;
      u8         *urb_buf;           /* buffers allocated for the URBs */
      dma_addr_t  urb_dma_addr;      /* dma_addresses for the buffers */

      struct Dib07x0Dev *dev;
   } urbs[DIB07X0_MAX_NO_URBS];      /* array[max_no_urbs] of allocated URBs */

   u8     *isoc_buffer;              /* Buffer to store the most recent Data sent by the isoc device */
   ssize_t isoc_buffer_write;        /* Position in the isoc Buffer to write new Data */
   ssize_t isoc_buffer_read;         /* Position in the isoc Buffer where Data can be read */

   struct list_head  completed_urbs; /* list of completed URBs */
   ssize_t           pos_cur_buf;    /* Position in the Buffer of the current URB - changed at read */
   wait_queue_head_t usb_wait_queue; /* wait queue for the channel -- maybe can be reused for PCI */
   int32_t           is_data;
   int32_t           usb_streaming_started;        /* once a read/write has started this is set to one */
   int32_t           pipe;                         /* USB pipe of this dev-node */
};

extern struct Dib07x0Dev *Dib07x0GetDevice(int32_t);

#endif

#endif
