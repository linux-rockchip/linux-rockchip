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
* @file "Hook.c"
* @brief .
*
***************************************************************************************************/
#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#if (DIBCOM_TEST_MODE == TEST_MODE_HOOK)
#include "Hook.h"
#include "Dib07x0.h"

#define MAX_INTEL_HEX_RECORD_LENGTH 16
typedef struct 
{
   uint8_t  Length;
   uint16_t Address;
   uint8_t  Type;
   uint8_t  Data[MAX_INTEL_HEX_RECORD_LENGTH];
   uint8_t  CheckSum;
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;

#include "../../Firmware/usb_dvbh.h"

/* copied from latest DVB-T HOOK1_1_ROM1/rom/include/define.h */
typedef enum
{
   REQUEST_UNUSED_0 = 0,
   REQUEST_DOWNLOAD,
   REQUEST_I2C_READ,
   REQUEST_I2C_WRITE,
   REQUEST_POLL_RC,
   REQUEST_DISCONNECT,
   REQUEST_GET_GPIO,
   REQUEST_SET_IOCTL,
   REQUEST_JUMPRAM,
   REQUEST_AHB_READ,
   REQUEST_AHB_WRITE,
   REQUEST_SET_CLOCK,
   REQUEST_SET_GPIO,
   REQUEST_SET_PWM,
   REQUEST_SET_ANA_MODE,
   REQUEST_ENABLE_VIDEO,
   REQUEST_SET_I2C_PARAM,
   REQUEST_SET_RC,
   REQUEST_NEW_I2C_READ,
   REQUEST_NEW_I2C_WRITE,
   REQUEST_SET_SUSP_PARAM,
   REQUEST_GET_VERSION,
   REQUEST_SRAM_READ_SMALL,
   REQUEST_SRAM_READ_BIG,
   REQUEST_SRAM_WRITE,
   REQUEST_SRAMBUS_INIT,
   REQUEST_SRAMBUS_WRITE,
   REQUEST_SRAMBUS_READ,
   REQUEST_SPIBUS_INIT,
   REQUEST_SPIBUS_WRITE,
   REQUEST_SPIBUS_READ,
   REQUEST_SDIOBUS_INIT,
   REQUEST_SDIOBUS_CMD53WRITE,
   REQUEST_SDIOBUS_CMD53READ,
   REQUEST_SDIOBUS_CMD52WRITE,
   REQUEST_SDIOBUS_CMD52READ,
   REQUEST_MAX
} TYPE_USB_REQUEST;

#define DIB07X0_MAX_MINORS 255

/* max timeout for a control message */
#define USB_CTRL_TMOUT 1000

/* local functions */
#define STK7078_ID 0x1ba6
#define ITF7000_9000_ID		0x0066
#define STK29098_ID        0x71AA

static int32_t HookStartBoard(struct DibBridgeContext *pContext);
static struct Dib07x0Dev *UsbEpOpen(struct DibBridgeContext *pContext, int32_t ep_nr);
static int32_t EP2Read(struct DibBridgeContext *pContext, uint8_t *buf, uint32_t Nb);
static int32_t UsbEnableVideo(struct DibBridgeContext *pContext, uint8_t video_en, uint8_t video_mode, uint8_t video_sub_mode, uint8_t channel1_en, uint8_t channel2_en);
static int32_t UsbGetVersion(struct DibBridgeContext *pContext, uint32_t *HardwareVersion, uint32_t *RomVersion, uint32_t *RamVersion, uint32_t *FirmwareType);
static int32_t UsbDownloadFirmware(struct DibBridgeContext *pContext, PINTEL_HEX_RECORD ptr);
static int32_t TranslateGpioNum(uint8_t GpioHookNum);
static int32_t UsbSetGpio(struct DibBridgeContext *pContext, uint8_t gpio_num, unsigned char gpio_dir, unsigned char gpio_value);


int32_t HookInit(struct DibBridgeContext *pContext, uint32_t SubMode, BOARD_HDL BoardHdl)
{
   int32_t  status          = DIBSTATUS_SUCCESS;
   uint32_t minor           = 0;
   struct   Dib07x0Dev *dev = NULL;
   int32_t  Err             = -1;
   uint32_t HardwareVersion, RomVersion, RamVersion, FirmwareType;
   struct file file;
   int32_t  Id = 0;

   if(BoardHdl)
      Id = *((int32_t *)BoardHdl);

   /* init bridge params */
   pContext->BridgeTargetCtx.CtrlDev = NULL;
   pContext->BridgeTargetCtx.state   = NULL;
   pContext->BridgeTargetCtx.Ep2Dev  = NULL;
   pContext->BridgeTargetCtx.Id      = -1;

   while(minor < DIB07X0_MAX_MINORS) 
   {
      /* opening driver */
      dev = Dib07x0GetDevice(minor);

      if(dev != NULL) 
      {
         /* check that we have a real control node */
         if(((dev->state->udev != 0) && (dev->pipe == 0)) && ((dev->state->devnum == Id) || (Id != -1))) 
         {
            DIB_DEBUG(PORT_LOG, (CRB "-I- DevNum %d Found Control minor %d" CRA, dev->state->devnum, minor));
            pContext->BridgeTargetCtx.Id = dev->state->devnum;
            break;
         }
         dev = NULL;
      }
      minor++;
   }

   if((dev == NULL) || (pContext->BridgeTargetCtx.Id == -1))
   {
      DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot open DevNum %d for control" CRA, Id));
      return FILE_DESCRIPTOR_OPEN_ERROR;
   }

   status                            = DIBSTATUS_SUCCESS;
   pContext->BridgeTargetCtx.CtrlDev = dev;         /* init ctrl_fd for following USB_get_version() */
   pContext->BridgeTargetCtx.state   = dev->state;  /* remember state for the "base" USB device */
   file.private_data                 = dev;
   file.f_flags                      = O_RDWR;

   if(dev->k_fops->open) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-I- before ctrl k_fops->open" CRA));
      Err = dev->k_fops->open(NULL, &file);
   }

   if(Err < 0)
      return FILE_DESCRIPTOR_OPEN_ERROR;

   /* state->devnum is the "hook number" */
   status = UsbGetVersion(pContext, &HardwareVersion, &RomVersion, &RamVersion, &FirmwareType);

   /* XXX should assert FirmwareType == 1 */

   if (status < 0) {
      DIB_DEBUG(PORT_LOG, (CRB "-I- No firmware" CRA));
      status = UsbDownloadFirmware(pContext, usb_dvbh);
      DibMSleep(200); /* wait for the Hook to start */
   }

   /* also open EP2 */
   dev = UsbEpOpen(pContext, 2);
   if(dev == NULL) 
   {
      /* reset of ctrl_fd */
      HookClose(pContext);
      return FILE_DESCRIPTOR_OPEN_ERROR;
   }

   pContext->BridgeTargetCtx.Ep2Dev = dev;

   /* dib07x0 module Config */
   /* kernel_driver_config(pContext, 2, 64*1024);*/

   pContext->BridgeTargetCtx.SubMode = SubMode;

   if((status = HookStartBoard(pContext)) != DIBSTATUS_SUCCESS)
      goto End;

   /* select and enable the correct Hook SubMode */
   UsbEnableVideo(pContext, 0, 3, 0, 0, 0);
   status = UsbEnableVideo(pContext, 1, 3, SubMode, 1, 1);

End:
   if(status != DIBSTATUS_SUCCESS) 
      HookClose(pContext);

   return status;
}

int32_t HookClose(struct DibBridgeContext *pContext)
{
   int32_t     status = DIBSTATUS_SUCCESS;
   struct file file;

   /* close ep2 node */
   if(pContext->BridgeTargetCtx.Ep2Dev != NULL) 
   {
      /* stop DVB-H Mode */
      UsbEnableVideo(pContext, 0, 3, 0, 0, 0);

      file.private_data = pContext->BridgeTargetCtx.Ep2Dev;
      file.f_flags      = O_RDONLY;

      if(pContext->BridgeTargetCtx.Ep2Dev->k_fops->release) 
      {
         DIB_DEBUG(PORT_LOG, (CRB "-I- before Ep2Dev k_fops->release" CRA));
         pContext->BridgeTargetCtx.Ep2Dev->k_fops->release(NULL, &file);
      }

      pContext->BridgeTargetCtx.Ep2Dev = NULL;
   }

   /* close ctrl node */
   if(pContext->BridgeTargetCtx.CtrlDev != NULL) 
   {
      file.private_data = pContext->BridgeTargetCtx.CtrlDev;
      file.f_flags      = O_RDWR;

      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->release) 
      {
         DIB_DEBUG(PORT_LOG, (CRB "-I- before ctrl k_fops->release" CRA));
         pContext->BridgeTargetCtx.CtrlDev->k_fops->release(NULL, &file);
      }

      pContext->BridgeTargetCtx.CtrlDev = NULL;
   }

   pContext->BridgeTargetCtx.Id = -1;

   return status;
}

static int32_t HookStartBoard(struct DibBridgeContext *pContext)
{
   struct Dib07x0DeviceId id;
   int32_t                status = DIBSTATUS_ERROR;
   struct file            file;

   file.private_data = pContext->BridgeTargetCtx.CtrlDev;

   if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
   {
      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOCG_ID, (LINUX_PTR_CAST)  &id) != 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "-E- error while reading id" CRA));
      } 
      else 
      {
         switch (id.Type) 
         {
         case DIB07X0_USB:
            if((id.u.usb.product_id == STK7078_ID) || (id.u.usb.product_id == ITF7000_9000_ID) || (id.u.usb.product_id == STK29098_ID))
            {
            /* FE reset */
            if((status = UsbSetGpio(pContext, 10, 1, 0)) != DIBSTATUS_SUCCESS)
               break;

            /* LED */
            if((status = UsbSetGpio(pContext,  0, 1, 0)) != DIBSTATUS_SUCCESS)
               break;

            /* power supply */
            if((status = UsbSetGpio(pContext,  6, 1, 0)) != DIBSTATUS_SUCCESS)
               break;
            DibMSleep(100); /* 100 ms */

            /* power supply */
            if((status = UsbSetGpio(pContext,  6, 1, 1)) != DIBSTATUS_SUCCESS)
               break;
            DibMSleep(10);  /* 10 ms */

            /* FE reset */
            if((status = UsbSetGpio(pContext, 10, 1, 0)) != DIBSTATUS_SUCCESS)
               break;
            DibMSleep(10);  /* 10 ms */

            if((status = UsbSetGpio(pContext, 10, 1, 1)) != DIBSTATUS_SUCCESS)
               break;
            DibMSleep(10);  /* 10 ms */

            /* LED */
            status = UsbSetGpio(pContext,  0, 1, 1);

            }
            status = DIBSTATUS_SUCCESS;
            break;

         default:
            DIB_DEBUG(PORT_ERR, (CRB "-E- unhandled Type" CRA));
            break;
         }
      }
   }

   return status;
}

/**
 * UsbEpOpen : open a sub-device == endpoint on a Hook
 * @param unit : index of the Hook
 * @param ep_nr : index of the endpoint on the Hook
 * @return file descriptor to the endpoint
 */
static struct Dib07x0Dev *UsbEpOpen(struct DibBridgeContext *pContext, int32_t ep_nr)
{
   struct Dib07x0Dev   *dev   = NULL;
   struct file          file;
   uint32_t             minor = 0;
   int32_t              Err   = -1;

   if(pContext->BridgeTargetCtx.Id == -1)
   {
      DIB_DEBUG(PORT_ERR, (CRB "-E-  No Id specified" CRA));
      goto End;
   }

   while(minor < DIB07X0_MAX_MINORS) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-I- Trying to open minor %d" CRA, minor));

      /* opening driver */
      dev = Dib07x0GetDevice(minor);

      if(dev != NULL) 
      {
         /* check that we have the right bulk node */
         if((usb_pipebulk(dev->pipe)) && (usb_pipeendpoint(dev->pipe) == ep_nr) && 
            (dev->state->devnum == pContext->BridgeTargetCtx.Id)) 
         {
            DIB_DEBUG(PORT_LOG, (CRB "-I- DevNum %d UsbEpOpen EP%d found minor %d" CRA, pContext->BridgeTargetCtx.Id, ep_nr, minor));

            file.private_data = dev;
            file.f_flags      = O_RDONLY;

            if(dev->k_fops->open) 
            {
               DIB_DEBUG(PORT_LOG, (CRB "-I- before EP k_fops->open" CRA));
               Err = dev->k_fops->open(NULL, &file);
            }
            break;
         }
         
      }
      minor++;
   }

   if(Err < 0)
   {
      DIB_DEBUG(PORT_ERR, (CRB "-E-  Could not open End points for DevNum %d " CRA, pContext->BridgeTargetCtx.Id));
      dev = NULL;
   }

End:
   return dev;
}

int32_t HookSpiBusInit(struct DibBridgeContext *pContext)
{
   uint8_t  *b;
   uint32_t status; 
   struct file file;

   b = kmalloc(MAX_WRITE_BURST_LEN, GFP_KERNEL);
   if (b == NULL) {
       DIB_DEBUG(PORT_ERR, (CRB "%s: not enough memory\n" CRA, __func__));
       return DIBSTATUS_ERROR;
   }

   struct Dib07x0UsbRequest req;
   req.RequestType         = DIB07X0_USB_CONTROL_WRITE;
   req.endpoint            = 0x00;
   req.timeout             = USB_CTRL_TMOUT;
   req.Length              = 1;
   req.Buffer              = b;
   req.setup.request       = REQUEST_SPIBUS_INIT;
   req.setup.RequestType   = DIB07X0_USB_DIR_OUT | DIB07X0_USB_TYPE_VENDOR;
   req.setup.Value         = 0;
   req.setup.index         = 0;

      file.private_data = pContext->BridgeTargetCtx.CtrlDev;
      status            = DIBSTATUS_ERROR;

      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
      {
         if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot write" CRA));
         } 
         else if(req.result == req.Length)
         {
            status = DIBSTATUS_SUCCESS;
         }
      }

   return status;
}
int32_t HookSpiBusWrite(struct DibBridgeContext *pContext,uint8_t *Buffer, unsigned int Size)
{
   /************************************************************************
   *****   TDADA  Structure  *****
   -- 0      : REQUEST_SRAM_WRITE
   -- 1      : Reg Addr 
   -- 2      : Data 
   ************************************************************************/
   uint32_t status; 
   uint8_t  *b;

   struct file file;

   b = kmalloc(MAX_WRITE_BURST_LEN, GFP_KERNEL);
   if (b == NULL) {
       DIB_DEBUG(PORT_ERR, (CRB "%s: not enough memory\n" CRA, __func__));
       return DIBSTATUS_ERROR;
   }
   memcpy(b, Buffer, Size);

   struct Dib07x0UsbRequest req;
   req.RequestType         = DIB07X0_USB_CONTROL_WRITE;
   req.endpoint            = 0x00;
   req.timeout             = USB_CTRL_TMOUT;
   req.Length              = Size;
   req.Buffer              = b;
   req.setup.request       = REQUEST_SPIBUS_WRITE;
   req.setup.RequestType   = DIB07X0_USB_DIR_OUT | DIB07X0_USB_TYPE_VENDOR;
   req.setup.Value         = Size;
   req.setup.index         = 0;

      file.private_data = pContext->BridgeTargetCtx.CtrlDev;
      status            = DIBSTATUS_ERROR;

      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
      {
         if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot write" CRA));
         } 
         else if(req.result == req.Length)
         {
            status = DIBSTATUS_SUCCESS;
         }
      }

   return status;
}

int32_t HookSpiBusRead(struct DibBridgeContext *pContext,  uint8_t *Buffer, unsigned int Size)
{
   int32_t status = DIBSTATUS_ERROR;
   struct Dib07x0UsbRequest req;
   struct file file;

   req.RequestType         = DIB07X0_USB_CONTROL_READ;
   req.endpoint            = 0x00;
   req.timeout             = USB_CTRL_TMOUT;
   req.Length              = Size & 0x3FFF; 
   req.Buffer              = Buffer;
   req.setup.request       = REQUEST_SPIBUS_READ;
   req.setup.RequestType   = DIB07X0_USB_DIR_IN | DIB07X0_USB_TYPE_VENDOR;
   req.setup.Value         = Size; 
   req.setup.index         = 0;

      file.private_data = pContext->BridgeTargetCtx.CtrlDev;
      status            = DIBSTATUS_ERROR;

      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
      {
         if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot write" CRA));
         } 
         else if(req.result == req.Length)
         {
            status = DIBSTATUS_SUCCESS;
         }
      }
   return status;
}

int32_t HookSramWrite(struct DibBridgeContext *pContext, uint32_t address, uint8_t byteMode, uint32_t Nb, uint8_t *buf)
{
   /************************************************************************
   *****   TDADA  Structure  *****
   -- 0      : REQUEST_SRAM_WRITE
   -- 1      : ByteMode
   -- 2      : Address MSB
   -- 3      : Address mSB
   -- 4      : Address lSB
   -- 5      : Address LSB
   ************************************************************************/
   uint8_t  *b;
   uint32_t status, header_len = WRITE_PACKET_HEADER_LEN;
   struct file file;

   b = kmalloc(MAX_WRITE_BURST_LEN, GFP_KERNEL);
   if (b == NULL) {
       DIB_DEBUG(PORT_ERR, (CRB "%s: not enough memory\n" CRA, __func__));
       return DIBSTATUS_ERROR;
   }

   b[0] = REQUEST_SRAM_WRITE;
   b[1] = byteMode;
   /* address is already set up in big endian, as needed by the Leon */
   b[2] = (address >> 24) & 0xff;
   b[3] = (address >> 16) & 0xff;
   b[4] = (address >> 8) & 0xff;
   b[5] = address & 0xff;

   memcpy(b+header_len, buf, Nb);

   {
      struct Dib07x0UsbRequest req = 
      {
         .RequestType = DIB07X0_USB_CONTROL_WRITE,
         .endpoint    = 0x00,
         .timeout     = USB_CTRL_TMOUT,
         .Length      = Nb + header_len,
         .Buffer      = b,
         .setup       = {.request     = REQUEST_SRAM_WRITE,
                         .RequestType = DIB07X0_USB_DIR_OUT | DIB07X0_USB_TYPE_VENDOR,
                         .Value       = 0,
                         .index       = 0,
                        },
      };

      file.private_data = pContext->BridgeTargetCtx.CtrlDev;
      status            = DIBSTATUS_ERROR;

      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
      {
         if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot write" CRA));
         } 
         else if(req.result == req.Length)
         {
            status = DIBSTATUS_SUCCESS;
         }
      }
   }

   kfree(b);

   return status;
}

int32_t HookSramReadSmall(struct DibBridgeContext *pContext, uint32_t address, uint8_t byteMode, uint32_t Nb, uint8_t *buf)
{
   /* 1024 is the maximum usb length when bytemode is 6 */
   uint8_t *tmpbuf;
   int32_t i, status = DIBSTATUS_ERROR;

   tmpbuf = kmalloc(MAX_WRITE_BURST_LEN, GFP_KERNEL);
   if (tmpbuf == NULL) {
       DIB_DEBUG(PORT_ERR, (CRB "%s: not enough memory\n" CRA, __func__));
       return DIBSTATUS_ERROR;
   }

   if(Nb <= DIB_IF_MAX_READ_SMALL) 
   {
      struct file file;
      struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_CONTROL_READ,
                                      .endpoint     = 0x00,
                                      .timeout      = USB_CTRL_TMOUT,
                                      .Length       = (Nb&0x7f)|(byteMode<<7),
                                      .Buffer       = tmpbuf,
                                      .setup        = {.request     = REQUEST_SRAM_READ_SMALL,
                                                       .RequestType = DIB07X0_USB_DIR_IN | DIB07X0_USB_TYPE_VENDOR,
                                                       .Value       = (address >> 16) & 0xffff, /* little-endian like native USB */
                                                       .index       =  address & 0xffff,        /* little-endian like native USB */
                                                      },
      };

      file.private_data = pContext->BridgeTargetCtx.CtrlDev;
      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
      {
         if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot read small" CRA));
         } 
         else
         {
            if(req.result < 0)
            {
               status = req.result;
            }
            else 
            {
               for(i=0 ; i<Nb ; i++)
               {
                  buf[i] = tmpbuf[i];
               }

               status = DIBSTATUS_SUCCESS;

            }
         }
      }
   }

   kfree(tmpbuf);

   return status;
}

static int32_t EP2Read(struct DibBridgeContext *pContext, uint8_t *buf, uint32_t Nb)
{
#define WITH_DVBH_EP_IOCTL 0

#if (WITH_DVBH_EP_IOCTL == 1)
   int32_t read_len = 0;
   int32_t status   = DIBSTATUS_SUCCESS;

   do {
      struct file file;
      struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_BULK_READ,
                                      .endpoint     = 0x02,
                                      .timeout      = USB_CTRL_TMOUT,
                                      .Length       = Nb  - read_len,
                                      .Buffer       = buf + read_len,
                                     };

      file.private_data = CtrlDev;
      if(req.Length != 0) 
      {
         if(CtrlDev->k_fops->ioctl) 
         {
            if(CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
            {
               DIB_DEBUG(PORT_ERR, (CRB "-E- USB_EP2_read ioctl failed" CRA));
            }

            if (req.result >= 0) 
            {
/*                if (req.result != req.Length)
                     DIB_DEBUG(PORT_LOG, (CRB "-I- EP2 partial read len %d/%d/%d" CRA, req.result, req.Length, Nb)); */
               read_len += req.result;
            } 
            else 
            {
               DIB_DEBUG(PORT_ERR, (CRB "-E- USB_EP2_read bad result %d/%d/%d" CRA, req.result, req.Length, Nb));
               status = DIBSTATUS_ERROR;
            }
         }
      }
   }
   while((read_len < Nb) && (status != DIBSTATUS_ERROR));

   return read_len;
#else
   struct file file;
   int32_t rcv_bytes = -EINVAL;

   file.private_data = pContext->BridgeTargetCtx.Ep2Dev;

   if(pContext->BridgeTargetCtx.Ep2Dev->k_fops->read) 
   {
/*       DIB_DEBUG(PORT_LOG, (CRB "-I- before EP2 fops->read Nb=%d" CRA, Nb)); */
      rcv_bytes = pContext->BridgeTargetCtx.Ep2Dev->k_fops->read(&file, buf, Nb, (loff_t *) /* XXX wrong */ buf);
   }

/*    DIB_DEBUG(PORT_LOG, (CRB "-I- after EP2 no k_fops->read Nb=%d rcv_bytes=%d" CRA, Nb, rcv_bytes)); */

   return rcv_bytes;
#endif
}

int32_t HookSramReadBig(struct DibBridgeContext *pContext, uint32_t address, uint8_t byteMode, uint32_t Nb, uint8_t *buf)
{
   /************************************************************************
   *****   TDADA  Structure  *****
   -- 0      : REQUEST_SRAM_READ
   -- 1      : ByteMode
   -- 2      : Number of bytes to read MSB
   -- 3      : Number of bytes to read LSB
   -- 4      : Address MSB
   -- 5      : Address mSB
   -- 6      : Address lSB
   -- 7      : Address LSB
   ************************************************************************/
   uint8_t bo[8];
   int32_t received_bytes, header_len = 8;
   int32_t status                     = DIBSTATUS_ERROR;

   if((Nb>=DIB_IF_MAX_READ_SMALL) && (Nb<=DIB_IF_MAX_READ_BIG)) 
   {
      struct file file;
      struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_CONTROL_WRITE,
                                      .endpoint     = 0x00,
                                      .timeout      = USB_CTRL_TMOUT,
                                      .Length       = header_len,
                                      .Buffer       = bo,
                                      .setup        = {.request      = REQUEST_SRAM_READ_BIG,
                                                       .RequestType = DIB07X0_USB_DIR_OUT | DIB07X0_USB_TYPE_VENDOR,
                                                       .Value        = 0,
                                                       .index        = 0,
                                                      },
                                    };

      bo[0] = REQUEST_SRAM_READ_BIG;
      bo[1] = byteMode;
      /* address and Nb are already set up in big endian, as needed by the Leon */
      bo[2] = (Nb >> 8) & 0xff;
      bo[3] = Nb & 0xff;
      bo[4] = (address >> 24) & 0xff;
      bo[5] = (address >> 16) & 0xff;
      bo[6] = (address >> 8) & 0xff;
      bo[7] = address & 0xff;

      file.private_data = pContext->BridgeTargetCtx.CtrlDev;
      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
      {
         if (pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
         {
            DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot read big" CRA));
         } 
         else if(req.result == req.Length) 
         {
            if((received_bytes = EP2Read(pContext, buf, Nb)) == Nb)
            {
               status = DIBSTATUS_SUCCESS;
            }
            else  
            {
               DIB_DEBUG(PORT_ERR, (CRB "-E-  bad EP2 read Nb=%d rcv=%d" CRA, Nb, received_bytes));
            }
         }
      }
   }

   return status;
}

int32_t HookSetClock(struct DibBridgeContext *pContext, 
                            uint16_t          en_pll, 
                            uint16_t          pll_src, 
                            uint16_t          pll_range,
                            uint16_t          pll_prediv,
                            uint16_t          pll_loopdiv, 
                            uint16_t          free_div,
                            uint16_t          dsuScaler /* dsuScaler = ((ahbclk Frequency) / 921600) - 0.5 */)
{
   int32_t status = DIBSTATUS_ERROR;
   uint8_t buf[MAX_WRITE_DATA_PACKET_LEN];

   struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_CONTROL_WRITE,
                                   .endpoint     = 0x00,
                                   .timeout      = USB_CTRL_TMOUT,
                                   .Length       = 10,
                                   .Buffer       = buf,
                                   .setup        = {.request     = REQUEST_SET_CLOCK,
                                                    .RequestType = DIB07X0_USB_DIR_OUT | DIB07X0_USB_TYPE_VENDOR,
                                                    .Value       = 0,
                                                    .index       = 0,
                                                   },
                                  };
   struct file file;

   buf[0] = REQUEST_SET_CLOCK;
   buf[1] = (en_pll<<7) | (pll_src<<6) | (pll_range<<5)/* | (clock_output_gp3<<4)*/;
   buf[2] = (pll_prediv>>8) & 0xff;    /* MSB */
   buf[3] = pll_prediv & 0xff;         /* LSB */
   buf[4] = (pll_loopdiv>>8) & 0xff;   /* MSB */
   buf[5] = pll_loopdiv & 0xff;        /* LSB */
   buf[6] = (free_div>>8) & 0xff;      /* MSB */
   buf[7] = free_div & 0xff;           /* LSB */
   buf[8] = (dsuScaler>>8) & 0xff;     /* MSB */
   buf[9] = dsuScaler & 0xff;          /* LSB */

   file.private_data = pContext->BridgeTargetCtx.CtrlDev;
   if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
   {
      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot set clock" CRA));
      } 
      else if(req.result == req.Length)
      {
         status = DIBSTATUS_SUCCESS;
      }
   }

   return status;
}

/**
 * UsbEnableVideo : start/stop streaming
 * @param fd  : fd of the control device node
 * @param video_en : flag to enable the video (1 to start, 0 to stop)
 * @param video_mode : 0 for MPEG2TS, 1 for ITU656 and 2 for TRANSPARENT
 * @param video_sub_mode : 0: 525 lines(ITU)/Slave(MPEG) and 1: 625 lines(ITU)/Master(MPEG)
 * @param channel1_en : enable streaming on video port 1
 * @param channel2_en : enable streaming on video port 2
 * @return  status == 0 for success < 0 for failure
 */
static int32_t UsbEnableVideo(struct DibBridgeContext *pContext, uint8_t video_en, uint8_t video_mode, uint8_t video_sub_mode, uint8_t channel1_en, uint8_t channel2_en)
{
   // 23                                      7                          0
   //---------------------------------------------------------------------
   //     control     |  Video    |  Video    |  MPEG2/ITU |  Channel    |
   //      byte       |  Enable   |   Mode    |   SubMode  |   Number    |
   //    (8 bits)     |  (4 bits) | (4 bits)  |  (4 bits)  |  (4 bits)   |
   //---------------------------------------------------------------------

   int32_t status = DIBSTATUS_ERROR;
   uint8_t buf[MAX_WRITE_DATA_PACKET_LEN];

   struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_CONTROL_WRITE,
                                   .endpoint    = 0x00,
                                   .timeout     = USB_CTRL_TMOUT,
                                   .Length      = 3,
                                   .Buffer      = buf,
                                   .setup       = {.request      = REQUEST_ENABLE_VIDEO,
                                                   .RequestType = DIB07X0_USB_DIR_OUT | DIB07X0_USB_TYPE_VENDOR,
                                                   .Value        = 0,
                                                   .index        = 0,
                                                  },
                                  };
   struct file file;

   buf[0] = REQUEST_ENABLE_VIDEO;
   buf[1] = ((video_en<<4)&0xf0) | (video_mode&0x0f);
   buf[2] = ((video_sub_mode<<4)&0xf0) | ((channel2_en<<1)&0x2) | (channel1_en&0x1);

   DIB_DEBUG(PORT_LOG, (CRB "-I- UsbEnableVideo channel1_en %d channel2_en %d %x %x" CRA, channel1_en, channel2_en, buf[1], buf[2]));

   file.private_data = pContext->BridgeTargetCtx.CtrlDev;
   if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
   {
      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "-E-  Cannot enable video" CRA));
      } 
      else if(req.result == req.Length)
      {
         status = DIBSTATUS_SUCCESS;
      }
   }

   return status;
}

static int32_t UsbGetVersion(struct DibBridgeContext *pContext, uint32_t *HardwareVersion, uint32_t *RomVersion, uint32_t *RamVersion, uint32_t *FirmwareType)
{
   uint8_t b[16];
   int32_t status = DIBSTATUS_ERROR;

   struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_CONTROL_READ,
                                   .endpoint    = 0x00,
                                   .timeout     = USB_CTRL_TMOUT,
                                   .Length      = 16,
                                   .Buffer      = b,
                                   .setup       = {.request     = REQUEST_GET_VERSION,
                                                   .RequestType = DIB07X0_USB_DIR_IN | DIB07X0_USB_TYPE_VENDOR,
                                                  },
                                  };
   struct file file;

   *HardwareVersion = *RomVersion = *RamVersion = *FirmwareType = 0;

   file.private_data = pContext->BridgeTargetCtx.CtrlDev;
   if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
   {
      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "-E-  USB error during get version!" CRA));
      } 
      else if(req.result < 0)
      {
         status = req.result;
      }
      else 
      {
         status = DIBSTATUS_SUCCESS;

         *HardwareVersion = (b[0] << 24)  | (b[1] << 16)  | (b[2] << 8)  | b[3];
         *RomVersion      = (b[4] << 24)  | (b[5] << 16)  | (b[6] << 8)  | b[7];
         *RamVersion      = (b[8] << 24)  | (b[9] << 16)  | (b[10] << 8) | b[11];
         *FirmwareType    = (b[12] << 24) | (b[13] << 16) | (b[14] << 8) | b[15];
      }
   }

   return status;
}

/* function to download a firmware (in Intel HEX format) to the hook) */
static int32_t UsbDownloadFirmware(struct DibBridgeContext *pContext, PINTEL_HEX_RECORD ptr)
{
   uint8_t usbBuffer[MAX_EP1_DATA_PACKET_LEN];
   int32_t status = DIBSTATUS_ERROR;

   struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_BULK_WRITE,
                                   .endpoint     = 0x01,
                                   .timeout      = USB_CTRL_TMOUT,
                                   .Buffer       = usbBuffer,
                                  };
   struct file file;

   file.private_data = pContext->BridgeTargetCtx.CtrlDev;

   if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-I- downloading firmware..." CRA));

      while (1) 
      {
         req.Length   = ptr->Length+5;
         usbBuffer[0] = ptr->Length;
         usbBuffer[1] = (uint8_t)((ptr->Address & 0xFF00) >> 8);
         usbBuffer[2] = (uint8_t)(ptr->Address & 0x00FF);
         usbBuffer[3] = ptr->Type;

         memcpy(&usbBuffer[4], ptr->Data, ptr->Length);

         usbBuffer[4 + ptr->Length] = ptr->CheckSum;

         if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
         {
            DIB_DEBUG(PORT_ERR, (CRB " -E- USB error during firmware download!" CRA));
            return DIBSTATUS_ERROR;
         } 
         else if(req.result != req.Length)
         {
            return DIBSTATUS_ERROR;
         }

         if(ptr->Type == 1)
            break;

         ptr++;
      }

      /* Just need to do one more bulk */
      req.Length   = 8,
      usbBuffer[0] = REQUEST_JUMPRAM;
      usbBuffer[1] = 0;
      usbBuffer[2] = 0;
      usbBuffer[3] = 0;
      usbBuffer[4] = 0x70; /* inBulk.address_1; */
      usbBuffer[5] = 0x00; /* inBulk.address_2; */
      usbBuffer[6] = 0x00; /* inBulk.address_3; */
      usbBuffer[7] = 0x00; /* inBulk.address_4 + jmpOffset; */

      status = DIBSTATUS_ERROR;

      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB " -E- USB error during Jump RAM!" CRA));
      } 
      else
      {
         status = DIBSTATUS_SUCCESS;
      }

   }

   return status;
}

static int32_t TranslateGpioNum(uint8_t GpioHookNum)
{
  uint8_t GpioNum;

  /* Mapping for the MOD3700 : ( from mod3700p2.c::MOD3700P2_power_control() )
   * GPIO0 LED1
   *   GpioNum = 0;
   * modulation polarity control
   *   GpioNum = 2;
   * GPIO3 LED2 on mod3700p2 (or 12MHz output on some designs)
   *   GpioNum = 3;
   * GPIO I2C
   *   GPIONum = 4;
   * IR detector
   *   GpioNum = 5;
   * 1.8V and 3.3V front-end power supplies control
   *   GpioNum = 6;
   * GPIO I2C
   *   GpioNum = 7;
   * SAW Sound IF control
   *   GpioNum = 8;
   * 5V front-end power supply control
   *   GpioNum = 9;
   * front-end reset control
   *   GpioNum = 10; 
   */

  if (GpioHookNum == 0)
    GpioNum = GpioHookNum;
  else if (GpioHookNum <= 5)
    GpioNum = GpioHookNum+1;
  else if (GpioHookNum == 6)
    GpioNum = GpioHookNum+2;
  else if (GpioHookNum <= 8)
    GpioNum = GpioHookNum+3;
  else if (GpioHookNum <= 10)
    GpioNum = GpioHookNum+5;
  else
    return DIBSTATUS_ERROR;

  return GpioNum;
}

/**
 * UsbSetGpio : configure one GPIO bit
 * @param fd  : fd of the control device node
 * @param gpio_num : index of GPIO bit
 * @param gpio_dir : Direction : IN or OUT
 * @param gpio_value : 0 or 1 if Direction is OUT
 * @return  status == 0 for success < 0 for failure
 */
static int32_t UsbSetGpio(struct DibBridgeContext *pContext, uint8_t hook_gpio_num, unsigned char gpio_dir, unsigned char gpio_value)
{
   int32_t status = DIBSTATUS_ERROR;
   uint8_t buf[MAX_WRITE_DATA_PACKET_LEN];

   struct Dib07x0UsbRequest req = {.RequestType = DIB07X0_USB_CONTROL_WRITE,
                                   .endpoint    = 0x00,
                                   .timeout     = USB_CTRL_TMOUT,
                                   .Length      = 3,
                                   .Buffer      = buf,
                                   .setup       = {.request     = REQUEST_SET_GPIO,
                                                   .RequestType = DIB07X0_USB_DIR_OUT | DIB07X0_USB_TYPE_VENDOR,
                                                   .Value       = 0,
                                                   .index       = 0,
                                                  },
                                  };
   struct file file;

   buf[0] = REQUEST_SET_GPIO;
   buf[1] = TranslateGpioNum(hook_gpio_num);
   buf[2] = (gpio_dir<<7) | (gpio_value<<6);

   file.private_data = pContext->BridgeTargetCtx.CtrlDev;
   if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl) 
   {
      if(pContext->BridgeTargetCtx.CtrlDev->k_fops->ioctl(NULL, &file, DIB07X0_IOC_USB_REQUEST, (LINUX_PTR_CAST) &req) != 0) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "-E- UsbSetGpio ioctl failed" CRA));
      } 
      else if(req.result == req.Length)
      {
         status = DIBSTATUS_SUCCESS;
      }
   }

   return status;
}
#endif
