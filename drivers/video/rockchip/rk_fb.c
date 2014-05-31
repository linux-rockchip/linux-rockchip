/*
 * drivers/video/rockchip/rk_fb.c
 *
 * Copyright (C) 2012 ROCKCHIP, Inc.
 *Author:yzq<yzq@rock-chips.com>
 	yxj<yxj@rock-chips.com>
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <asm/div64.h>
#include <asm/uaccess.h>
#include <linux/rk_fb.h>
#include <plat/ipp.h>
#include "hdmi/rk_hdmi.h"
#include "rga/rga.h"
#include <linux/linux_logo.h>

#include <mach/clock.h>
#include <linux/clk.h>

void rk29_backlight_set(bool on);
bool rk29_get_backlight_status(void);

static int hdmi_switch_complete = 0;
static int fence_wait_begin = 0;
static int hdmi_xsize = 0;
static int hdmi_ysize = 0;
struct list_head saved_list;

#if defined(CONFIG_FB_MIRRORING)

int (*video_data_to_mirroring)(struct fb_info *info,u32 yuv_phy[2]) = NULL;
EXPORT_SYMBOL(video_data_to_mirroring);

#endif
static struct platform_device *g_fb_pdev;

static struct rk_fb_rgb def_rgb_16 = {
     red:    { offset: 11, length: 5, },
     green:  { offset: 5,  length: 6, },
     blue:   { offset: 0,  length: 5, },
     transp: { offset: 0,  length: 0, },
};


char * get_format_string(enum data_format format,char *fmt)
{
	if(!fmt)
		return NULL;
	switch(format)
	{
	case ARGB888:
		strcpy(fmt,"ARGB888");
		break;
	case RGB888:
		strcpy(fmt,"RGB888");
		break;
	case RGB565:
		strcpy(fmt,"RGB565");
		break;
	case YUV420:
		strcpy(fmt,"YUV420");
		break;
	case YUV422:
		strcpy(fmt,"YUV422");
		break;
	case YUV444:
		strcpy(fmt,"YUV444");
		break;
	case XRGB888:
		strcpy(fmt,"XRGB888");
		break;
	case XBGR888:
		strcpy(fmt,"XBGR888");
		break;
	case ABGR888:
		strcpy(fmt,"XBGR888");
		break;
	default:
		strcpy(fmt,"invalid");
		break;
	}

	return fmt;
	
}

static int get_rga_format(int fmt)
{
	int rga_fmt = RK_FORMAT_RGBA_8888;
        switch (fmt)
        {
        case HAL_PIXEL_FORMAT_RGBX_8888:
		rga_fmt = RK_FORMAT_RGBX_8888;
		break;
        case HAL_PIXEL_FORMAT_RGBA_8888:
		rga_fmt = RK_FORMAT_RGBA_8888;
		break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
		rga_fmt = RK_FORMAT_BGRA_8888;
		break;
        case HAL_PIXEL_FORMAT_RGB_888 :
                rga_fmt = RK_FORMAT_RGB_888;
                break;
        case HAL_PIXEL_FORMAT_RGB_565:          //RGB565
                rga_fmt = RK_FORMAT_RGB_565;
                break;
        //case HAL_PIXEL_FORMAT_YCbCr_422_SP :    // yuv422
        //        rga_fmt = RK_FORMAT_YCbCr_422_SP;
        //        break;
        //case HAL_PIXEL_FORMAT_YCrCb_NV12:       // YUV420---uvuvuv
        //        rga_fmt = RK_FORMAT_YCbCr_420_SP;
        //        break;
        default:
                rga_fmt = RK_FORMAT_RGBA_8888;
                break;
        }

        return rga_fmt;

}

/***********************************************************************************/
//This function is for copying fb by RGA
//RGA support copy RGB to RGB
//but unsupport copy YUV to YUV if rotate
/***********************************************************************************/
static void fb_copy_by_rga(struct fb_info *dst_info, struct fb_info *src_info, int offset)
{
	int ret = 0;
	struct rga_req  Rga_Request;

	memset(&Rga_Request,0x0,sizeof(Rga_Request));

#if defined(CONFIG_FB_ROTATE)
 	int orientation = orientation = 270 - CONFIG_ROTATE_ORIENTATION;
        switch(orientation)
        {
                case 90:
			Rga_Request.rotate_mode = 1;
			Rga_Request.sina = 65536;
			Rga_Request.cosa = 0;
			
			Rga_Request.dst.act_w = dst_info->var.yres;
			Rga_Request.dst.act_h = dst_info->var.xres;
			Rga_Request.dst.x_offset = dst_info->var.xres - 1;
 			Rga_Request.dst.y_offset = 0;
			break;
                case 180:
			Rga_Request.rotate_mode = 1;
                     	Rga_Request.sina = 0;
                        Rga_Request.cosa = -65536;
			Rga_Request.dst.act_w = dst_info->var.xres;
 			Rga_Request.dst.act_h = dst_info->var.yres;
			Rga_Request.dst.x_offset = dst_info->var.xres - 1;
 			Rga_Request.dst.y_offset = dst_info->var.yres - 1;
                        break;
                case 270:
			Rga_Request.rotate_mode = 1;
                        Rga_Request.sina = -65536;
                        Rga_Request.cosa = 0;

			Rga_Request.dst.act_w = dst_info->var.yres;
 			Rga_Request.dst.act_h = dst_info->var.xres;
			Rga_Request.dst.x_offset = dst_info->var.xres - 1;
 			Rga_Request.dst.y_offset = dst_info->var.yres - 1;
                        break;
                default:
			Rga_Request.rotate_mode = 0;
			Rga_Request.dst.act_w = dst_info->var.xres;
 			Rga_Request.dst.act_h = dst_info->var.yres;
			Rga_Request.dst.x_offset = dst_info->var.xres - 1;
 			Rga_Request.dst.y_offset = dst_info->var.yres - 1;
                        break;

        }
#endif	
	Rga_Request.src.yrgb_addr =  src_info->fix.smem_start + offset;
	Rga_Request.src.uv_addr   =  0; 
	Rga_Request.src.v_addr    =  0;
	Rga_Request.src.vir_w = src_info->var.xres_virtual;  
	Rga_Request.src.vir_h =	src_info->var.yres_virtual; 
	Rga_Request.src.format = get_rga_format((src_info->var.nonstd)&0xff);
	Rga_Request.src.act_w = src_info->var.xres;  
	Rga_Request.src.act_h = src_info->var.yres; 
	Rga_Request.src.x_offset = 0;
	Rga_Request.src.y_offset = 0;//par->y_offset;

	Rga_Request.dst.yrgb_addr = dst_info->fix.smem_start + offset;
	Rga_Request.dst.uv_addr  = 0;
	Rga_Request.dst.v_addr   = 0;
	Rga_Request.dst.vir_w = dst_info->var.xres_virtual; 
	Rga_Request.dst.vir_h = dst_info->var.yres_virtual; 
	Rga_Request.dst.format = get_rga_format((dst_info->var.nonstd)&0xff);  
	Rga_Request.clip.xmin = 0;
	Rga_Request.clip.xmax = dst_info->var.xres - 1;
	Rga_Request.clip.ymin = 0;
	Rga_Request.clip.ymax = dst_info->var.yres - 1;

	Rga_Request.scale_mode = 0;

	//Rga_Request.mmu_info.mmu_en    = 1;
	//Rga_Request.mmu_info.mmu_flag  = ((2 & 0x3) << 4) | 1;

	ret = rga_ioctl_kernel(&Rga_Request);	
}


/**********************************************************************
this is for hdmi
name: lcdc device name ,lcdc0 , lcdc1
***********************************************************************/
struct rk_lcdc_device_driver * rk_get_lcdc_drv(char *name)
{
	struct rk_fb_inf *inf =  platform_get_drvdata(g_fb_pdev);
	int i = 0;
	for( i = 0; i < inf->num_lcdc; i++)
	{
		if(!strcmp(inf->lcdc_dev_drv[i]->name,name))
			break;
	}
	return inf->lcdc_dev_drv[i];
	
}

static struct rk_lcdc_device_driver * rk_get_prmry_lcdc_drv(void)
{
	struct rk_fb_inf *inf = NULL; 
	struct rk_lcdc_device_driver *dev_drv = NULL;
	int i = 0;

	if(likely(g_fb_pdev))
		inf = platform_get_drvdata(g_fb_pdev);
	else
		return NULL;
	
	for(i = 0; i < inf->num_lcdc;i++)
	{
		if(inf->lcdc_dev_drv[i]->screen_ctr_info->prop ==  PRMRY)
		{
			dev_drv = inf->lcdc_dev_drv[i];
			break;
		}
	}

	return dev_drv;
}

//get one frame time
int rk_fb_get_prmry_screen_ft(void)
{
	uint32_t pix_count,ft_us,dclk_mhz;
	struct rk_lcdc_device_driver *dev_drv = rk_get_prmry_lcdc_drv();
        if (!dev_drv)
		return 0;
		
        if (0 == dev_drv->id)
                dclk_mhz = clk_get_rate(clk_get(NULL, "dclk_lcdc0"))/(1000*1000);
        else 
                dclk_mhz = clk_get_rate(clk_get(NULL, "dclk_lcdc1"))/(1000*1000);

        pix_count = (dev_drv->cur_screen->upper_margin + dev_drv->cur_screen->lower_margin + dev_drv->cur_screen->y_res +dev_drv->cur_screen->vsync_len)*
        (dev_drv->cur_screen->left_margin + dev_drv->cur_screen->right_margin + dev_drv->cur_screen->x_res + dev_drv->cur_screen->hsync_len);       // one frame time ,(pico seconds)
        
        ft_us = pix_count / dclk_mhz;

        
        return ft_us;

}

static struct rk_lcdc_device_driver * rk_get_extend_lcdc_drv(void)
{
	struct rk_fb_inf *inf = NULL; 
	struct rk_lcdc_device_driver *dev_drv = NULL;
	int i = 0;
	
	if(likely(g_fb_pdev))
		inf = platform_get_drvdata(g_fb_pdev);
	else
		return NULL;
	
	for(i = 0; i < inf->num_lcdc; i++)
	{
		if(inf->lcdc_dev_drv[i]->screen_ctr_info->prop == EXTEND)
		{
			dev_drv = inf->lcdc_dev_drv[i];
			break;
		}
	}

	return dev_drv;
}

rk_screen *rk_fb_get_prmry_screen(void)
{
	struct rk_lcdc_device_driver *dev_drv = rk_get_prmry_lcdc_drv();
	return dev_drv->screen0;
	
}

u32 rk_fb_get_prmry_screen_pixclock(void)
{
	struct rk_lcdc_device_driver *dev_drv = rk_get_prmry_lcdc_drv();
	return dev_drv->pixclock;
}

int rk_fb_poll_prmry_screen_vblank(void)
{
	struct rk_lcdc_device_driver *dev_drv = rk_get_prmry_lcdc_drv();
	if(likely(dev_drv))
	{
		if(dev_drv->poll_vblank)
			return dev_drv->poll_vblank(dev_drv);
		else
			return RK_LF_STATUS_NC;	
	}
	else
		return RK_LF_STATUS_NC;
}

bool rk_fb_poll_wait_frame_complete(void)
{
	uint32_t timeout = RK_LF_MAX_TIMEOUT;
	struct rk_lcdc_device_driver *dev_drv = rk_get_prmry_lcdc_drv();
	
	if (likely(dev_drv)) {
		if (dev_drv->set_irq_to_cpu)
			dev_drv->set_irq_to_cpu(dev_drv,0);
	}

	if (rk_fb_poll_prmry_screen_vblank() == RK_LF_STATUS_NC) {
		if(dev_drv->set_irq_to_cpu)
                        dev_drv->set_irq_to_cpu(dev_drv,1);
		return false;
	}

	while( !(rk_fb_poll_prmry_screen_vblank() == RK_LF_STATUS_FR)  &&  --timeout);
	while( !(rk_fb_poll_prmry_screen_vblank() == RK_LF_STATUS_FC)  &&  --timeout);

	if (likely(dev_drv)) {
                if (dev_drv->set_irq_to_cpu)
                        dev_drv->set_irq_to_cpu(dev_drv,1);
        }

	return true;
}
static int rk_fb_open(struct fb_info *info,int user)
{
    struct rk_lcdc_device_driver * dev_drv = (struct rk_lcdc_device_driver * )info->par;
    int layer_id;
  
    layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
    if(dev_drv->layer_par[layer_id]->state)
    {
    	return 0;    // if this layer aready opened ,no need to reopen
    }
    else
    {
    	dev_drv->open(dev_drv,layer_id,1);
    }
    
    return 0;
}

static int  get_extend_fb_id(char *id )
{
	int fb_id = 0;
	if(!strcmp(id,"fb0"))
	{
		fb_id = 0;
	}
	else if(!strcmp(id,"fb1"))
	{
		fb_id = 1;
	}
#if defined(CONFIG_LCDC_RK30)	//only rk30 lcdc has three windows layer now
	else if(!strcmp(id,"fb2"))
        {
               fb_id = 2;
        }
#endif
	return fb_id;
}

static int rk_fb_close(struct fb_info *info,int user)
{
	struct rk_lcdc_device_driver * dev_drv = (struct rk_lcdc_device_driver * )info->par;
	struct layer_par *par = NULL;
	int layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
	if(layer_id >= 0)
	{
		par = dev_drv->layer_par[layer_id];
		info->fix.smem_start = par->reserved;

		info->var.xres = dev_drv->screen0->x_res;
		info->var.yres = dev_drv->screen0->y_res;
		info->var.grayscale |= (info->var.xres<<8) + (info->var.yres<<20);
#ifdef  CONFIG_LOGO_LINUX_BMP
		info->var.bits_per_pixel = 32;
#else
		info->var.bits_per_pixel = 16;
#endif
		info->fix.line_length  = (info->var.xres)*(info->var.bits_per_pixel>>3);
		info->var.xres_virtual = info->var.xres;
		info->var.yres_virtual = info->var.yres;
		info->var.width =  dev_drv->screen0->width;
		info->var.height = dev_drv->screen0->height;
		info->var.pixclock = dev_drv->pixclock;
		info->var.left_margin = dev_drv->screen0->left_margin;
		info->var.right_margin = dev_drv->screen0->right_margin;
		info->var.upper_margin = dev_drv->screen0->upper_margin;
		info->var.lower_margin = dev_drv->screen0->lower_margin;
		info->var.vsync_len = dev_drv->screen0->vsync_len;
		info->var.hsync_len = dev_drv->screen0->hsync_len;
    }
	
    	return 0;
}


static int get_ipp_format(int fmt)
{
	int ipp_fmt = IPP_XRGB_8888;
	switch (fmt)
	{
	case HAL_PIXEL_FORMAT_RGBX_8888: 
	case HAL_PIXEL_FORMAT_RGBA_8888 :     
	case HAL_PIXEL_FORMAT_BGRA_8888 :     
	case HAL_PIXEL_FORMAT_RGB_888 :
		ipp_fmt = IPP_XRGB_8888;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:  	//RGB565
		ipp_fmt = IPP_RGB_565;
	    	break;
	case HAL_PIXEL_FORMAT_YCbCr_422_SP : 	// yuv422
		ipp_fmt = IPP_Y_CBCR_H2V1;
	    	break;
	case HAL_PIXEL_FORMAT_YCrCb_NV12: 	// YUV420---uvuvuv
		ipp_fmt = IPP_Y_CBCR_H2V2;
	    	break;
	case HAL_PIXEL_FORMAT_YCrCb_444 : // yuv444
		ipp_fmt = IPP_Y_CBCR_H1V1;
		break;
	default:
		ipp_fmt = IPP_IMGTYPE_LIMIT;
		break;
	}

	return ipp_fmt;
}

static void ipp_par_check(int* dst_w, int* dst_h, int* dst_vir_w, 
				int rotation, int fmt)
{
	int align16 = 2;
	int align64 = 8;
	
	

	if(fmt == IPP_XRGB_8888) //XRGB,32bits per pixel
	{
		align16 = 1;
		align64 = 2;
	}

	if(fmt == IPP_RGB_565) //RGB565 ,16bits per pixel
	{
		align16 = 1;
		align64 = 4;
	}
	else  //yuv
	{
		align16 = 2;
		align64 = 8;
	}
	align16 -= 1;  //for YUV, 1
	align64 -= 1;  //for YUV, 7
	
	if(rotation == IPP_ROT_0)//no rotation
	{
		if(fmt > IPP_RGB_565)//only YUV need to align
		{
			if((*dst_w & 1) != 0)
			{
				*dst_w = *dst_w+1;
			}
			if((*dst_h & 1) != 0)
			{
				*dst_h = *dst_h+1;
			}
			if(*dst_vir_w < *dst_w)
			{
				*dst_vir_w = *dst_w;
			}
		}
	}
	else//rotaion
	{

		if((*dst_w & align64) != 0)
		{
			*dst_w = (*dst_w+align64)&(~align64);
		}
		if((fmt > IPP_RGB_565) && ((*dst_h & 1) == 1)) //for yuv ,must 2 pix align
		{
			*dst_h = *dst_h+1;
		}
		if(*dst_vir_w < *dst_w)
		{
			*dst_vir_w = *dst_w;
		}
	}

}

static void fb_copy_by_ipp(struct fb_info *dst_info, struct fb_info *src_info, int y_offset, int c_offset)
{
	struct rk29_ipp_req ipp_req;
 	uint32_t  rotation = 0;
	int dst_w,dst_h,dst_vir_w,dst_vir_h;
	int ipp_fmt;
	u8 data_format = (dst_info->var.nonstd)&0xff;
	
	memset(&ipp_req, 0, sizeof(struct rk29_ipp_req));
#if defined(CONFIG_FB_ROTATE)
	int orientation = 270 - CONFIG_ROTATE_ORIENTATION;
	switch(orientation)
	{
		case 0:
			rotation = IPP_ROT_0;
			break;
		case 90:
			rotation = IPP_ROT_90;
			break;
		case 180:
			rotation = IPP_ROT_180;
			break;
		case 270:
			rotation = IPP_ROT_270;
			break;
		default:
			rotation = IPP_ROT_270;
			break;
			
	}
#endif

	dst_w = dst_info->var.xres;
	dst_h = dst_info->var.yres;
	dst_vir_w = dst_info->var.xres_virtual;
	dst_vir_h = dst_info->var.yres_virtual;
	dst_info->fix.mmio_start = dst_info->fix.smem_start + dst_vir_w*dst_vir_h;
	ipp_fmt = get_ipp_format(data_format);
	ipp_par_check(&dst_w,&dst_h,&dst_vir_w,rotation,ipp_fmt);
	ipp_req.src0.YrgbMst = src_info->fix.smem_start + y_offset;
	ipp_req.src0.CbrMst = src_info->fix.mmio_start + c_offset;
	ipp_req.src0.w = src_info->var.xres;
	ipp_req.src0.h = src_info->var.yres;
	ipp_req.src_vir_w = src_info->var.xres_virtual;
	ipp_req.src0.fmt = ipp_fmt;
	
	ipp_req.dst0.YrgbMst = dst_info->fix.smem_start + y_offset;
	ipp_req.dst0.CbrMst = dst_info->fix.mmio_start + c_offset;
	ipp_req.dst0.w = dst_w;
	ipp_req.dst0.h = dst_h;
	ipp_req.dst_vir_w = dst_vir_w;
	ipp_req.dst0.fmt = ipp_fmt;
	
	ipp_req.timeout = 100;
	ipp_req.flag = rotation;
	ipp_blit_sync(&ipp_req);
	
}

#if 0

static void hdmi_post_work(struct work_struct *work)
{	
	struct rk_fb_inf *inf = container_of(to_delayed_work(work), struct rk_fb_inf, delay_work);
	struct fb_info * info2 = inf->fb[2];    
	struct fb_info * info = inf->fb[0];     
	struct rk_lcdc_device_driver * dev_drv1  = (struct rk_lcdc_device_driver * )info2->par;
	struct rk_lcdc_device_driver * dev_drv  = (struct rk_lcdc_device_driver * )info->par;
	struct layer_par *par = dev_drv->layer_par[1];
	struct layer_par *par2 = dev_drv1->layer_par[1];  	
	struct fb_var_screeninfo *var = &info->var;   
	u32 xvir = var->xres_virtual;	
	dev_drv1->xoffset = var->xoffset;             // offset from virtual to visible 
	dev_drv1->yoffset += var->yres; 
	if(dev_drv1->yoffset >= 3*var->yres)
		dev_drv1->yoffset = 0;++	
		rk_bufferoffset_tran(dev_drv1->xoffset, dev_drv1->yoffset, xvir , par2);
	fb_copy_by_ipp(info2,info,par->y_offset,par2->c_offset);
	dev_drv1->pan_display(dev_drv1,1);
	complete(&(dev_drv1->ipp_done));
}
#endif

void rk_fd_fence_wait(struct rk_lcdc_device_driver *dev_drv, struct sync_fence *fence)
{
	int err = sync_fence_wait(fence, 1000);
	
	if (err >= 0)
		return;

	if (err == -ETIME)
		err = sync_fence_wait(fence, 10 * MSEC_PER_SEC);
	if (err < 0)
		printk("error waiting on fence\n");
}

void rk_fb_free_dma_buf(struct rk_fb_dma_buf_data *dma_buf_data)
{
	if(dma_buf_data->acq_fence){
		sync_fence_put(dma_buf_data->acq_fence);
	}
	memset(dma_buf_data, 0, sizeof(struct rk_fb_dma_buf_data));
}
static void rk_fb_update_reg(struct rk_lcdc_device_driver * dev_drv,struct rk_reg_data *regs)
{
	int i,ret=0;
	ktime_t timestamp = dev_drv->vsync_info.timestamp;

	struct rk_fb_dma_buf_data old_dma_bufs[RK30_MAX_LAYER_SUPPORT];

	if(dev_drv->lcdc_reg_update)
		dev_drv->lcdc_reg_update(dev_drv);

	if(dev_drv->wait_fs == 0){
		ret = wait_event_interruptible_timeout(dev_drv->vsync_info.wait,
			!ktime_equal(timestamp, dev_drv->vsync_info.timestamp),msecs_to_jiffies(dev_drv->cur_screen->ft+5));
	}else{
		kfree(regs);
	}
	sw_sync_timeline_inc(dev_drv->timeline, 1);

	if(dev_drv->win_data.acq_fence_fd[0] >= 0)
	{
		for(i=0;i<RK30_MAX_LAYER_SUPPORT;i++){
			if(dev_drv->win_data.acq_fence_fd[i] > 0){
				put_unused_fd(dev_drv->win_data.acq_fence_fd[i]);
				printk("acq_fd=%d\n",dev_drv->win_data.acq_fence_fd[i]);
			}	
			rk_fb_free_dma_buf(&regs->dma_buf_data[i]);
		}
	}
}

static void rk_fb_update_regs_handler(struct kthread_work *work)
{
	struct rk_lcdc_device_driver * dev_drv =
			container_of(work, struct rk_lcdc_device_driver, update_regs_work);
	struct rk_reg_data *data, *next;
	//struct list_head saved_list;

	mutex_lock(&dev_drv->update_regs_list_lock);
	saved_list = dev_drv->update_regs_list;
	list_replace_init(&dev_drv->update_regs_list, &saved_list);
	mutex_unlock(&dev_drv->update_regs_list_lock);

	list_for_each_entry_safe(data, next, &saved_list, list) {
		rk_fb_update_reg(dev_drv,data);
		list_del(&data->list);
		kfree(data);
	}
}

static int rk_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct rk_lcdc_device_driver * dev_drv = (struct rk_lcdc_device_driver * )info->par;
    	struct layer_par *par = NULL;
#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
	struct rk_fb_inf *inf = dev_get_drvdata(info->device);
	struct layer_par *par2 = NULL;
	struct fb_info * info2 = NULL; 
	int layer_id2 = 0;
	int extend_fb_id = 0;
	struct rk_lcdc_device_driver * dev_drv1  = NULL;
#endif
    	int layer_id = 0;
	u32 xoffset = var->xoffset;		// offset from virtual to visible 
	u32 yoffset = var->yoffset;				
	u32 xvir = var->xres_virtual;
	u8 data_format = var->nonstd&0xff;
	
	layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
	if(layer_id < 0)
	{
		return  -ENODEV;
	}
	else
	{
		 par = dev_drv->layer_par[layer_id];
	}
	switch (par->format)
    	{
    		case XBGR888:
		case ARGB888:
		case ABGR888:
			par->y_offset = (yoffset*xvir + xoffset)*4;
			break;
		case  RGB888:
			par->y_offset = (yoffset*xvir + xoffset)*3;
			break;
		case RGB565:
			par->y_offset = (yoffset*xvir + xoffset)*2;
	            	break;
		case  YUV422:
			par->y_offset = yoffset*xvir + xoffset;
			par->c_offset = par->y_offset;
	            	break;
		case  YUV420:
			par->y_offset = yoffset*xvir + xoffset;
			par->c_offset = (yoffset>>1)*xvir + xoffset;
	            	break;
		case  YUV444 : // yuv444
			par->y_offset = yoffset*xvir + xoffset;
			par->c_offset = yoffset*2*xvir +(xoffset<<1);
			break;
		default:
			printk("un supported format:0x%x\n",data_format);
            		return -EINVAL;
    	}

	dev_drv->pan_display(dev_drv,layer_id);
	#if defined(CONFIG_RK_HDMI)
		#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
			if((hdmi_get_hotplug() == HDMI_HPD_ACTIVED) && (hdmi_switch_complete))
			{
				if(inf->num_fb >= 2)
				{
					extend_fb_id = get_extend_fb_id(info->fix.id);
					info2 = inf->fb[(inf->num_fb>>1) + extend_fb_id];
					dev_drv1 = (struct rk_lcdc_device_driver * )info2->par;
					layer_id2 = dev_drv1->fb_get_layer(dev_drv1,info2->fix.id);
					par2 = dev_drv1->layer_par[layer_id2];
					par2->y_offset = par->y_offset;
					par2->c_offset = par->c_offset;
					//memcpy(info2->screen_base+par2->y_offset,info->screen_base+par->y_offset,
					//	var->xres*var->yres*var->bits_per_pixel>>3);
					#if defined(CONFIG_FB_ROTATE) || !defined(CONFIG_THREE_FB_BUFFER)
						#ifdef CONFIG_ARCH_RK3026
						//RGA support copying RGB to RGB,but not support YUV to YUV if rotate
						fb_copy_by_rga(info2,info,par->y_offset);
						#else
						fb_copy_by_ipp(info2,info,par->y_offset,par->c_offset);
						par2->cbr_start = info2->fix.mmio_start;
						#endif
					#endif
					dev_drv1->pan_display(dev_drv1,layer_id2);
					//queue_delayed_work(inf->workqueue, &inf->delay_work,0);
				}
			}
		#endif
	#endif

	#ifdef	CONFIG_FB_MIRRORING
	if(video_data_to_mirroring!=NULL)
		video_data_to_mirroring(info,NULL);
 	#endif
	return 0;
}

static int rk_fb_get_list_stat(struct rk_lcdc_device_driver *dev_drv)
{
	int i,j;
	i = list_empty(&dev_drv->update_regs_list);
	j = list_empty(&saved_list);
	if((i == 1)&&(j == 1))
		return 1;
	else
		return 0;
}

static int rk_fb_ioctl(struct fb_info *info, unsigned int cmd,unsigned long arg)
{
	struct fb_fix_screeninfo *fix = &info->fix;
	struct rk_lcdc_device_driver *dev_drv = (struct rk_lcdc_device_driver * )info->par;
	struct rk_fb_inf *inf = dev_get_drvdata(info->device);
#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
	struct fb_info * info2 = NULL;
	int extend_fb_id = 0;
	struct rk_lcdc_device_driver * dev_drv1  = NULL;
#endif
	u32 yuv_phy[2];
	int  layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
	int enable; // enable fb:1 enable;0 disable 
	int ovl;	//overlay:0 win1 on the top of win0;1,win0 on the top of win1
	int num_buf; //buffer_number
	int ret,i;
	
	struct rk_reg_data *regs;
	struct sync_fence *release_fence;
	struct sync_fence *retire_fence;
	struct sync_pt *release_sync_pt;
	struct sync_pt *retire_sync_pt;

	struct sync_fence *layer2_fence;
	struct sync_pt *layer2_pt;

	int fence_fd;
	void __user *argp = (void __user *)arg;

	unsigned int dsp_addr[2];
	int list_stat;
		
	switch(cmd)
	{
 		case FBIOPUT_FBPHYADD:
			return fix->smem_start;
			break;
		case RK_FBIOSET_YUV_ADDR:   //when in video mode, buff alloc by android
			{
				if (copy_from_user(yuv_phy, argp, 8))
					return -EFAULT;
				fix->smem_start = yuv_phy[0];  //four y
				fix->mmio_start = yuv_phy[1];  //four uv
			}
			break;
		case RK_FBIOSET_ENABLE:
			if (copy_from_user(&enable, argp, sizeof(enable)))
				return -EFAULT;
			dev_drv->open(dev_drv,layer_id,enable);
			break;
		case RK_FBIOGET_ENABLE:
			enable = dev_drv->get_layer_state(dev_drv,layer_id);
			if(copy_to_user(argp,&enable,sizeof(enable)))
				return -EFAULT;
			break;
		case RK_FBIOSET_OVERLAY_STATE:
			if (copy_from_user(&ovl, argp, sizeof(ovl)))
				return -EFAULT;
			dev_drv->ovl_mgr(dev_drv,ovl,1);
			break;
		case RK_FBIOGET_OVERLAY_STATE:
			ovl = dev_drv->ovl_mgr(dev_drv,0,0);
			if (copy_to_user(argp, &ovl, sizeof(ovl)))
				return -EFAULT;
			break;
		case RK_FBIOPUT_NUM_BUFFERS:
			if (copy_from_user(&num_buf, argp, sizeof(num_buf)))
				return -EFAULT;
			dev_drv->num_buf = num_buf;
			printk("rk fb use %d buffers\n",num_buf);
			break;
		case RK_FBIOSET_VSYNC_ENABLE:
			if (copy_from_user(&enable, argp, sizeof(enable)))
				return -EFAULT;
			dev_drv->vsync_info.active = enable;
			break;
		case RK_FBIOGET_DSP_ADDR:
			dev_drv->get_dsp_addr(dev_drv,dsp_addr);
			if (copy_to_user(argp, &dsp_addr, sizeof(dsp_addr)))
				return -EFAULT;
			break;
		case RK_FBIOGET_LIST_STAT:
			list_stat = rk_fb_get_list_stat(dev_drv);
			if (copy_to_user(argp, &list_stat, sizeof(list_stat)))
				return -EFAULT;

			break;			
		case RK_FBIOSET_CONFIG_DONE:
			regs = kzalloc(sizeof(struct rk_reg_data), GFP_KERNEL);
			ret = copy_from_user(&(dev_drv->win_data),(struct rk_fb_win_config_data __user *)argp,sizeof(dev_drv->win_data));
			dev_drv->wait_fs = dev_drv->win_data.wait_fs;
			fence_wait_begin = dev_drv->win_data.fence_begin;
			if((fence_wait_begin == 1)&&(dev_drv->suspend_flag == 0)){	
				dev_drv->win_data.rel_fence_fd[0] =  get_unused_fd();
				if (dev_drv->win_data.rel_fence_fd[0] < 0){
					printk("rel_fence_fd=%d\n",dev_drv->win_data.rel_fence_fd[0]);
					return -EFAULT;
				}

				dev_drv->win_data.rel_fence_fd[1] =  get_unused_fd();
				if (dev_drv->win_data.rel_fence_fd[1] < 0){
					printk("rel_fence_fd=%d\n",dev_drv->win_data.rel_fence_fd[1]);
					return -EFAULT;
				}

				dev_drv->win_data.ret_fence_fd =  get_unused_fd();
				if (dev_drv->win_data.ret_fence_fd < 0){
					printk("%s,ret_fence_fd=%d\n",dev_drv->win_data.ret_fence_fd);
					return -EFAULT;
				}
				mutex_lock(&dev_drv->update_regs_list_lock);
				dev_drv->timeline_max++;
				release_sync_pt = sw_sync_pt_create(dev_drv->timeline, dev_drv->timeline_max);
				release_fence = sync_fence_create("rel_fence", release_sync_pt);
				sync_fence_install(release_fence, dev_drv->win_data.rel_fence_fd[0]);

				layer2_pt = sw_sync_pt_create(dev_drv->timeline, dev_drv->timeline_max);
				layer2_fence= sync_fence_create("rel2_fence", layer2_pt);
				sync_fence_install(layer2_fence, dev_drv->win_data.rel_fence_fd[1]);

				retire_sync_pt = sw_sync_pt_create(dev_drv->timeline, dev_drv->timeline_max);
				retire_fence = sync_fence_create("ret_fence", retire_sync_pt);
				sync_fence_install(retire_fence, dev_drv->win_data.ret_fence_fd);
				if(dev_drv->wait_fs == 1){
					rk_fb_update_reg(dev_drv,regs);
					mutex_unlock(&dev_drv->update_regs_list_lock);
				}else{
					list_add_tail(&regs->list,&dev_drv->update_regs_list);
					mutex_unlock(&dev_drv->update_regs_list_lock);

					queue_kthread_work(&dev_drv->update_regs_worker,
						&dev_drv->update_regs_work);
				}		
			}else{
				if(dev_drv->lcdc_reg_update)
					dev_drv->lcdc_reg_update(dev_drv);	
			}
			if (copy_to_user((struct rk_fb_win_config_data __user *)arg,
				 &dev_drv->win_data,
				 sizeof(dev_drv->win_data))) {
			ret = -EFAULT;
			break;
		}	
	#if defined(CONFIG_RK_HDMI)
		#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
			if((hdmi_get_hotplug() == HDMI_HPD_ACTIVED) && (hdmi_switch_complete))
			{
				if(inf->num_fb >= 2)
				{
					extend_fb_id = get_extend_fb_id(info->fix.id);
					info2 = inf->fb[(inf->num_fb>>1) + extend_fb_id];
					dev_drv1 = (struct rk_lcdc_device_driver * )info2->par;
					if(dev_drv1->lcdc_reg_update)
						dev_drv1->lcdc_reg_update(dev_drv1);
				}
			}
		#endif 
	#endif
			break;
        	default:
			dev_drv->ioctl(dev_drv,cmd,arg,layer_id);
            		break;
    }
    return 0;
err:
    return -1;
	
}

static int rk_fb_blank(int blank_mode, struct fb_info *info)
{
    	struct rk_lcdc_device_driver *dev_drv = (struct rk_lcdc_device_driver * )info->par;
	struct fb_fix_screeninfo *fix = &info->fix;
	int layer_id;
	
	layer_id = dev_drv->fb_get_layer(dev_drv,fix->id);
	if(layer_id < 0)
	{
		return  -ENODEV;
	}
#if defined(CONFIG_RK_HDMI)
#if defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)
	if(hdmi_get_hotplug() == HDMI_HPD_ACTIVED){
		printk("hdmi is connect , not blank lcdc\n");
	}else
#endif
#endif
	{
		dev_drv->blank(dev_drv,layer_id,blank_mode);
	}
	return 0;
}

static int rk_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	
	if( 0==var->xres_virtual || 0==var->yres_virtual ||
		 0==var->xres || 0==var->yres || var->xres<16 ||
		 ((16!=var->bits_per_pixel)&&(32!=var->bits_per_pixel)) )
	 {
		 printk("%s check var fail 1!!! \n",info->fix.id);
		 printk("xres_vir:%d>>yres_vir:%d\n", var->xres_virtual,var->yres_virtual);
		 printk("xres:%d>>yres:%d\n", var->xres,var->yres);
		 printk("bits_per_pixel:%d \n", var->bits_per_pixel);
		 return -EINVAL;
	 }
 
	 if( ((var->xoffset+var->xres) > var->xres_virtual) ||
	     ((var->yoffset+var->yres) > (var->yres_virtual)) )
	 {
		 printk("%s check_var fail 2!!! \n",info->fix.id);
		 printk("xoffset:%d>>xres:%d>>xres_vir:%d\n",var->xoffset,var->xres,var->xres_virtual);
		 printk("yoffset:%d>>yres:%d>>yres_vir:%d\n",var->yoffset,var->yres,var->yres_virtual);
		 return -EINVAL;
	 }

    return 0;
}

static ssize_t rk_fb_read(struct fb_info *info, char __user *buf,
			   size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;
	u8 *buffer, *dst;
	u8 __iomem *src;
	int c, cnt = 0, err = 0;
	unsigned long total_size;
	struct rk_lcdc_device_driver * dev_drv = (struct rk_lcdc_device_driver * )info->par;
	struct layer_par *par = NULL;
	int layer_id = 0;

	layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
	if(layer_id < 0)
	{
		return  -ENODEV;
	}
	else
	{
		par = dev_drv->layer_par[layer_id];
	}

	if(par->format == RGB565)
	{
		total_size = par->xact*par->yact<<1; //only read the current frame buffer
	}
	else
		total_size = par->xact*par->yact<<2;
	
	
	if (p >= total_size)
		return 0;
	
	if (count >= total_size)
		count = total_size;

	if (count + p > total_size)
		count = total_size - p;

	buffer = kmalloc((count > PAGE_SIZE) ? PAGE_SIZE : count,
			 GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	
	src = (u8 __iomem *) (info->screen_base + p + par->y_offset);

	while (count) {
		c  = (count > PAGE_SIZE) ? PAGE_SIZE : count;
		dst = buffer;
		fb_memcpy_fromfb(dst, src, c);
		dst += c;
		src += c;

		if (copy_to_user(buf, buffer, c)) {
			err = -EFAULT;
			break;
		}
		*ppos += c;
		buf += c;
		cnt += c;
		count -= c;
	}

	kfree(buffer);

	return (err) ? err : cnt;
}

static ssize_t rk_fb_write(struct fb_info *info, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;
	u8 *buffer, *src;
	u8 __iomem *dst;
	int c, cnt = 0, err = 0;
	unsigned long total_size;
	struct rk_lcdc_device_driver * dev_drv = (struct rk_lcdc_device_driver * )info->par;
	struct layer_par *par = NULL;
	int layer_id = 0;

	layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
	if(layer_id < 0)
	{
		return  -ENODEV;
	}
	else
	{
		par = dev_drv->layer_par[layer_id];
	}

	if(par->format == RGB565)
	{
		total_size = par->xact*par->yact<<1; //write the current frame buffer
	}
	else
		total_size = par->xact*par->yact<<2;
	
	if (p > total_size)
		return -EFBIG;

	if (count > total_size) {
		err = -EFBIG;
		count = total_size;
	}

	if (count + p > total_size) {
		if (!err)
			err = -ENOSPC;

		count = total_size - p;
	}

	buffer = kmalloc((count > PAGE_SIZE) ? PAGE_SIZE : count,
			 GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	dst = (u8 __iomem *) (info->screen_base + p + par->y_offset);

	while (count) {
		c = (count > PAGE_SIZE) ? PAGE_SIZE : count;
		src = buffer;

		if (copy_from_user(src, buf, c)) {
			err = -EFAULT;
			break;
		}

		fb_memcpy_tofb(dst, src, c);
		dst += c;
		src += c;
		*ppos += c;
		buf += c;
		cnt += c;
		count -= c;
	}

	kfree(buffer);

	return (cnt) ? cnt : err;
	
}

static int rk_fb_set_par(struct fb_info *info)
{
    	struct fb_var_screeninfo *var = &info->var;
    	struct fb_fix_screeninfo *fix = &info->fix;
    	struct rk_lcdc_device_driver * dev_drv = (struct rk_lcdc_device_driver * )info->par;
    	struct layer_par *par = NULL;
   	rk_screen *screen =dev_drv->cur_screen;
    	int layer_id = 0;	
    	u32 cblen = 0,crlen = 0;
    	u16 xsize =0,ysize = 0;              //winx display window height/width --->LCDC_WINx_DSP_INFO
    	u32 xoffset = var->xoffset;		// offset from virtual to visible 
	u32 yoffset = var->yoffset;		//resolution			
	u16 xpos = (var->nonstd>>8) & 0xfff; //visiable pos in panel
	u16 ypos = (var->nonstd>>20) & 0xfff;
	u32 xvir = var->xres_virtual;
	u32 yvir = var->yres_virtual;
	u8 data_format = var->nonstd&0xff;
	var->pixclock = dev_drv->pixclock;
 	
	layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
	if(layer_id < 0)
	{
		return  -ENODEV;
	}
	else
	{
		par = dev_drv->layer_par[layer_id];
	}
	
	if(var->grayscale>>8)  //if the application has specific the horizontal and vertical display size
	{
		xsize = (var->grayscale>>8) & 0xfff;  //visiable size in panel ,for vide0
		ysize = (var->grayscale>>20) & 0xfff;
	}
	else  //ohterwise  full  screen display
	{
		xsize = screen->x_res;
		ysize = screen->y_res;
	}

#if defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF) || defined(CONFIG_NO_DUAL_DISP)
	if(screen->screen_id == 0) //this is for device like rk2928 ,whic have one lcdc but two display outputs
	{			   //save parameter set by android
		dev_drv->screen0->xsize = xsize;
		dev_drv->screen0->ysize = ysize;
		dev_drv->screen0->xpos  = xpos;
		dev_drv->screen0->ypos = ypos;
	}
	else
	{
		xsize = dev_drv->screen1->xsize; 
		ysize = dev_drv->screen1->ysize;
		xpos = dev_drv->screen1->xpos;
		ypos = dev_drv->screen1->ypos;
	}
#endif
	/* calculate y_offset,c_offset,line_length,cblen and crlen  */
#if 1
	switch (data_format)
	{
		case HAL_PIXEL_FORMAT_RGBX_8888: 
			par->format = XBGR888;
			fix->line_length = 4 * xvir;
			par->y_offset = (yoffset*xvir + xoffset)*4;
			break;
		case HAL_PIXEL_FORMAT_RGBA_8888 :      // rgb
			par->format = ABGR888;
			fix->line_length = 4 * xvir;
			par->y_offset = (yoffset*xvir + xoffset)*4;
			break;
		case HAL_PIXEL_FORMAT_BGRA_8888 :      // rgb
			par->format = ARGB888;
			fix->line_length = 4 * xvir;
			par->y_offset = (yoffset*xvir + xoffset)*4;
			break;
		case HAL_PIXEL_FORMAT_RGB_888 :
			par->format = RGB888;
			fix->line_length = 3 * xvir;
			par->y_offset = (yoffset*xvir + xoffset)*3;
			break;
		case HAL_PIXEL_FORMAT_RGB_565:  //RGB565
			par->format = RGB565;
			fix->line_length = 2 * xvir;
			par->y_offset = (yoffset*xvir + xoffset)*2;
		    	break;
		case HAL_PIXEL_FORMAT_YCbCr_422_SP : // yuv422
			par->format = YUV422;
			fix->line_length = xvir;
			cblen = crlen = (xvir*yvir)>>1;
			par->y_offset = yoffset*xvir + xoffset;
			par->c_offset = par->y_offset;
		    	break;
		case HAL_PIXEL_FORMAT_YCrCb_NV12   : // YUV420---uvuvuv
			par->format = YUV420;
			fix->line_length = xvir;
			cblen = crlen = (xvir*yvir)>>2;
			par->y_offset = yoffset*xvir + xoffset;
			par->c_offset = (yoffset>>1)*xvir + xoffset;
		    	break;
		case HAL_PIXEL_FORMAT_YCrCb_444 : // yuv444
			par->format = 5;
			fix->line_length = xvir<<2;
			par->y_offset = yoffset*xvir + xoffset;
			par->c_offset = yoffset*2*xvir +(xoffset<<1);
			cblen = crlen = (xvir*yvir);
			break;
		default:
			printk("%s:un supported format:0x%x\n",__func__,data_format);
		    return -EINVAL;
	}
#else
	switch(var->bits_per_pixel)
	{
		case 32:
			par->format = ARGB888;
			fix->line_length = 4 * xvir;
			par->y_offset = (yoffset*xvir + xoffset)*4;
			break;
		case 16:
			par->format = RGB565;
			fix->line_length = 2 * xvir;
			par->y_offset = (yoffset*xvir + xoffset)*2;
	    		break;
			
	}
#endif

	par->xpos = xpos;
	par->ypos = ypos;
	par->xsize = xsize;
	par->ysize = ysize;

	par->smem_start =fix->smem_start;
	par->cbr_start = fix->mmio_start;
	par->xact = var->xres;              //winx active window height,is a part of vir
	par->yact = var->yres;
	par->xvir =  var->xres_virtual;		// virtual resolution	 stride --->LCDC_WINx_VIR
	par->yvir =  var->yres_virtual;
	#if defined(CONFIG_RK_HDMI)
		#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
			if((hdmi_get_hotplug() == HDMI_HPD_ACTIVED) && (hdmi_switch_complete))
			{
				struct rk_fb_inf *inf = dev_get_drvdata(info->device);
				int extend_fb_id = get_extend_fb_id(info->fix.id);
				struct fb_info * info2 = inf->fb[(inf->num_fb>>1) + extend_fb_id];
				struct rk_lcdc_device_driver * dev_drv1  = (struct rk_lcdc_device_driver * )info2->par;
				int layer_id2 = dev_drv->fb_get_layer(dev_drv1,info2->fix.id);
				struct layer_par *par2 = dev_drv1->layer_par[layer_id2];
				if(info != info2)
				{
					if(par->xact < par->yact)
					{
						par2->xact = par->yact;
						par2->yact = par->xact;
						par2->xvir = par->yact;
						info2->var.xres = var->yres;
						info2->var.yres = var->xres;
						info2->var.xres_virtual = var->yres;
						info2->var.yres_virtual = var->xres;
					}
					else
					{
						par2->xact = par->xact;
						par2->yact = par->yact;
						par2->xvir = par->xvir;
						info2->var.xres = var->xres;
						info2->var.yres = var->yres;
						info2->var.xres_virtual = var->xres_virtual;
						info2->var.yres_virtual = var->yres_virtual;
					}
				#if !defined(CONFIG_FB_ROTATE) && defined(CONFIG_THREE_FB_BUFFER) 
					par2->smem_start = par->smem_start;
					par2->cbr_start = par->cbr_start;
				#endif
					//the display image of the primary screen is no full screen size when play video that is YUV type
					if(par->xpos != 0 || par->ypos != 0) {
						par2->xsize = hdmi_xsize*par->xsize/screen->x_res;
						par2->ysize = hdmi_ysize*par->ysize/screen->y_res;
						par2->xpos = ((dev_drv1->cur_screen->x_res - hdmi_xsize)>>1) + hdmi_xsize*par->xpos/screen->x_res;
						par2->ypos = ((dev_drv1->cur_screen->y_res - hdmi_ysize)>>1) + hdmi_ysize*par->ypos/screen->y_res;
					}
					else {	//the display image of the primary screen is full screen size
						par2->xpos = (dev_drv1->cur_screen->x_res - hdmi_xsize)>>1;
						par2->ypos = (dev_drv1->cur_screen->y_res - hdmi_ysize)>>1;
						par2->xsize = hdmi_xsize;
						par2->ysize = hdmi_ysize;
					}

					par2->format = par->format;
					info2->var.nonstd &= 0xffffff00;
					info2->var.nonstd |= data_format;
					dev_drv1->set_par(dev_drv1,layer_id2);
				}
			}
		#endif
	#endif
	dev_drv->set_par(dev_drv,layer_id);

	return 0;
}

static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseudo-palette */
		if (regno < 16) {
			u32 *pal = info->pseudo_palette;
			val  = chan_to_field(red,   &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue,  &info->var.blue);
			pal[regno] = val;
		}
		break;
	default:
		return -1;	/* unknown type */
	}

	return 0;
}

static struct fb_ops fb_ops = {
    .owner          = THIS_MODULE,
    .fb_open        = rk_fb_open,
    .fb_release     = rk_fb_close,
    .fb_check_var   = rk_fb_check_var,
    .fb_set_par     = rk_fb_set_par,
    .fb_blank       = rk_fb_blank,
    .fb_ioctl       = rk_fb_ioctl,
    .fb_pan_display = rk_pan_display,
    .fb_read	    = rk_fb_read,
    .fb_write	    = rk_fb_write,
    .fb_setcolreg   = fb_setcolreg,
    .fb_fillrect    = cfb_fillrect,
    .fb_copyarea    = cfb_copyarea,
    .fb_imageblit   = cfb_imageblit,
};



static struct fb_var_screeninfo def_var = {
#ifdef  CONFIG_LOGO_LINUX_BMP
	.red    	= {16,8,0},
	.green  	= {8,8,0},
	.blue   	= {0,8,0},
	.transp 	= {0,0,0},
	.nonstd 	= HAL_PIXEL_FORMAT_BGRA_8888,
#else
	.red		= {11,5,0},
	.green  	= {5,6,0},
	.blue   	= {0,5,0},
	.transp 	= {0,0,0},
	.nonstd 	= HAL_PIXEL_FORMAT_RGB_565,   //(ypos<<20+xpos<<8+format) format
#endif
	.grayscale	= 0,  //(ysize<<20+xsize<<8)
	.activate    	= FB_ACTIVATE_NOW,
	.accel_flags 	= 0,
	.vmode       	= FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo def_fix = {
	.type		 = FB_TYPE_PACKED_PIXELS,
	.type_aux	 = 0,
	.xpanstep	 = 1,
	.ypanstep	 = 1,
	.ywrapstep	 = 0,
	.accel		 = FB_ACCEL_NONE,
	.visual 	 = FB_VISUAL_TRUECOLOR,
		
};


static int rk_fb_wait_for_vsync_thread(void *data)
{
	struct rk_lcdc_device_driver  *dev_drv = data;
	struct rk_fb_inf *inf =  platform_get_drvdata(g_fb_pdev);
	struct fb_info *fbi = inf->fb[0];

	while (!kthread_should_stop()) {
		ktime_t timestamp = dev_drv->vsync_info.timestamp;
		int ret = wait_event_interruptible(dev_drv->vsync_info.wait,
			!ktime_equal(timestamp, dev_drv->vsync_info.timestamp)
			 || dev_drv->vsync_info.irq_stop);

		if (!ret) {
			sysfs_notify(&fbi->dev->kobj, NULL, "vsync");
		}
	}

	return 0;
}

static ssize_t rk_fb_vsync_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = dev_get_drvdata(dev);
	struct rk_lcdc_device_driver * dev_drv = 
		(struct rk_lcdc_device_driver * )fbi->par;
	return scnprintf(buf, PAGE_SIZE, "%llu\n",
			ktime_to_ns(dev_drv->vsync_info.timestamp));
}

static DEVICE_ATTR(vsync, S_IRUGO, rk_fb_vsync_show, NULL);


/*****************************************************************
this two function is for other module that in the kernel which
need show image directly through fb
fb_id:we have 4 fb here,default we use fb0 for ui display
*******************************************************************/
struct fb_info * rk_get_fb(int fb_id)
{
    struct rk_fb_inf *inf =  platform_get_drvdata(g_fb_pdev);
    struct fb_info *fb = inf->fb[fb_id];
    return fb;
}
EXPORT_SYMBOL(rk_get_fb);

void rk_direct_fb_show(struct fb_info * fbi)
{
    rk_fb_set_par(fbi);
    rk_pan_display(&fbi->var, fbi);
}
EXPORT_SYMBOL(rk_direct_fb_show);


static int set_xact_yact_for_hdmi(struct fb_var_screeninfo *pmy_var,
					struct fb_var_screeninfo *hdmi_var)
{
	if(pmy_var->xres < pmy_var->yres)  //vertical  lcd screen
	{
		hdmi_var->xres = pmy_var->yres;
		hdmi_var->yres = pmy_var->xres;
		hdmi_var->xres_virtual = pmy_var->yres;
	}
	else
	{
		hdmi_var->xres = pmy_var->xres;
		hdmi_var->yres = pmy_var->yres;
		hdmi_var->xres_virtual = pmy_var->xres_virtual;
	}

	return 0;
		
}
int rk_fb_dpi_open(bool open)
{
	struct rk_lcdc_device_driver * dev_drv = NULL;
	dev_drv = rk_get_prmry_lcdc_drv();
	dev_drv->dpi_open(dev_drv,open);

	return 0;
}
int rk_fb_dpi_layer_sel(int layer_id)
{
	struct rk_lcdc_device_driver * dev_drv = NULL;
	dev_drv = rk_get_prmry_lcdc_drv();
	dev_drv->dpi_layer_sel(dev_drv,layer_id);

	return 0;
}
int rk_fb_dpi_status(void)
{
	int ret;
	struct rk_lcdc_device_driver * dev_drv = NULL;
	dev_drv = rk_get_prmry_lcdc_drv();
	ret = dev_drv->dpi_status(dev_drv);

	return ret;
}

/******************************************
function:this function will be called by hdmi,when 
              hdmi plug in/out
screen: the screen attached to hdmi
enable: 1,hdmi plug in,0,hdmi plug out
lcdc_id: the lcdc id the hdmi attached ,0 or 1
******************************************/
int rk_fb_switch_screen(rk_screen *screen ,int enable ,int lcdc_id)
{
	struct rk_fb_inf *inf =  platform_get_drvdata(g_fb_pdev);
	struct fb_info *info = NULL;
	struct rk_lcdc_device_driver * dev_drv = NULL;
	struct fb_var_screeninfo *hdmi_var    = NULL;
#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
	struct fb_var_screeninfo *pmy_var = NULL;      //var for primary screen
	struct fb_info *pmy_info = NULL;
	struct fb_fix_screeninfo *pmy_fix = NULL;
#endif
	int i;
	struct fb_fix_screeninfo *hdmi_fix    = NULL;
	char name[6];
	int ret;
	int layer_id;

#if defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF) || defined(CONFIG_NO_DUAL_DISP)
	rk29_backlight_set(0);
#endif
	
	sprintf(name, "lcdc%d",lcdc_id);

#if defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)
	dev_drv = inf->lcdc_dev_drv[0];
#else
	for(i = 0; i < inf->num_lcdc; i++)  //find the driver for the extend display device
	{
		if(inf->lcdc_dev_drv[i]->screen_ctr_info->prop == EXTEND)
		{
			dev_drv = inf->lcdc_dev_drv[i];
			break;
		}
	}
	
	if(i == inf->num_lcdc)
	{
		printk(KERN_ERR "%s driver not found!",name);
		return -ENODEV;
		
	}
#endif
	printk("hdmi %s lcdc%d\n",enable?"connect to":"remove from",dev_drv->id);
	
	if(inf->num_lcdc == 1)
	{
		info = inf->fb[0];
	}
	else if(inf->num_lcdc == 2)
	{
		info = inf->fb[dev_drv->num_layer]; //the main fb of lcdc1
	}

	if(dev_drv->screen1) //device like rk2928 ,have only one lcdc but two outputs
	{
		if(enable)
		{
			memcpy(dev_drv->screen1,screen,sizeof(rk_screen ));
			dev_drv->screen1->lcdc_id = 0; //connect screen1 to output interface 0
			dev_drv->screen1->screen_id = 1;
			dev_drv->screen0->lcdc_id = 1; //connect screen0 to output interface 1
			dev_drv->cur_screen = dev_drv->screen1;
			dev_drv->screen0->ext_screen = dev_drv->screen1;
			if(dev_drv->screen0->sscreen_get)
			{
				dev_drv->screen0->sscreen_get(dev_drv->screen0,
					dev_drv->cur_screen->hdmi_resolution);
			}
			
			
		}
		else
		{
			dev_drv->screen1->lcdc_id = 1; //connect screen1 to output interface 1
			dev_drv->screen0->lcdc_id = 0; //connect screen0 to output interface 0
			dev_drv->cur_screen = dev_drv->screen0;
			dev_drv->screen_ctr_info->set_screen_info(dev_drv->cur_screen,
			dev_drv->screen_ctr_info->lcd_info);
			
			
		}
	}
	else
	{
		if(enable)
		{
			memcpy(dev_drv->cur_screen,screen,sizeof(rk_screen ));
		}
	}

	
	layer_id = dev_drv->fb_get_layer(dev_drv,info->fix.id);
	
	if(!enable && !dev_drv->screen1) //only double lcdc device need to close
	{
		if(dev_drv->layer_par[layer_id]->state) 
		{
			dev_drv->open(dev_drv,layer_id,enable); //disable the layer which attached to this fb
		}
		hdmi_switch_complete = 0;
	
		return 0;
	}
	
	hdmi_var = &info->var;
	hdmi_fix = &info->fix;
	#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
		if(likely(inf->num_lcdc == 2))
		{
			pmy_var = &inf->fb[0]->var;
			pmy_fix = &inf->fb[0]->fix;
			set_xact_yact_for_hdmi(pmy_var,hdmi_var);
			hdmi_var->nonstd &= 0xffffff00;
			hdmi_var->nonstd |= (pmy_var->nonstd & 0xff); //use the same format as primary screen
		}
		else
		{
			printk(KERN_WARNING "%s>>only one lcdc,dual display no supported!",__func__);
		}
	#endif
	hdmi_var->grayscale &= 0xff;
	hdmi_var->grayscale |= (dev_drv->cur_screen->x_res<<8) + (dev_drv->cur_screen->y_res<<20);
	if(dev_drv->screen1)  //device like rk2928,whic have one lcdc but two outputs
	{
	//	info->var.nonstd &= 0xff;
	//	info->var.nonstd |= (dev_drv->cur_screen->xpos<<8) + (dev_drv->cur_screen->ypos<<20);
	//	info->var.grayscale &= 0xff;
	//	info->var.grayscale |= (dev_drv->cur_screen->x_res<<8) + (dev_drv->cur_screen->y_res<<20);
		dev_drv->screen1->xsize = dev_drv->cur_screen->x_res;
		dev_drv->screen1->ysize = dev_drv->cur_screen->y_res;
		dev_drv->screen1->xpos = 0;
		dev_drv->screen1->ypos = 0;
	}
	ret = info->fbops->fb_open(info,1);
	dev_drv->load_screen(dev_drv,1);
	ret = info->fbops->fb_set_par(info);
	if(dev_drv->lcdc_hdmi_process)
		dev_drv->lcdc_hdmi_process(dev_drv,enable);

	hdmi_switch_complete = enable;
	#if defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
		if(likely(inf->num_lcdc == 2))
		{
			pmy_info = inf->fb[0];
			pmy_info->fbops->fb_pan_display(pmy_var,pmy_info);
		}
		else
		{
			printk(KERN_WARNING "%s>>only one lcdc,dual display no supported!",__func__);
		}
	#elif defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)
		info->fbops->fb_pan_display(hdmi_var,info);
	#endif 
	info->fbops->fb_ioctl(info,RK_FBIOSET_CONFIG_DONE,0);
	if(dev_drv->screen1) //for one lcdc use  scaler for dual dispaly
	{
		if(dev_drv->screen0->sscreen_set)
		{
			dev_drv->blank(dev_drv,0,FB_BLANK_NORMAL);
			msleep(100);
			dev_drv->screen0->sscreen_set(dev_drv->screen0,enable);
			dev_drv->blank(dev_drv,0,FB_BLANK_UNBLANK);
		}
	}
#if defined(CONFIG_NO_DUAL_DISP)  //close backlight for device whic do not support dual display
	if(!enable)
		rk29_backlight_set(1);
#elif defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)  //close backlight for device whic do not support dual display
	rk29_backlight_set(1);
#endif
	return 0;

}




/******************************************
function:this function current only called by hdmi for 
	scale the display
scale_x: scale rate of x resolution
scale_y: scale rate of y resolution
lcdc_id: the lcdc id the hdmi attached ,0 or 1
******************************************/

int rk_fb_disp_scale(u8 scale_x, u8 scale_y,u8 lcdc_id)
{
	struct rk_fb_inf *inf =  platform_get_drvdata(g_fb_pdev);
	struct fb_info *info = NULL;
	struct fb_var_screeninfo *var = NULL;
	struct rk_lcdc_device_driver * dev_drv = NULL;
	u16 screen_x,screen_y;
	u16 xpos,ypos;
	u16 xsize,ysize;
	char name[6];
	int i = 0;
	sprintf(name, "lcdc%d",lcdc_id);
	
#if defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)
		dev_drv = inf->lcdc_dev_drv[0];
#else
	for(i = 0; i < inf->num_lcdc; i++)
	{
		if(inf->lcdc_dev_drv[i]->screen_ctr_info->prop == EXTEND)
		{
			dev_drv = inf->lcdc_dev_drv[i];
			break;
		}
	}

	if(i == inf->num_lcdc)
	{
		printk(KERN_ERR "%s driver not found!",name);
		return -ENODEV;
		
	}
#endif
	if(inf->num_lcdc == 1)
	{
		info = inf->fb[0];
	}
	else if(inf->num_lcdc == 2)
	{
		info = inf->fb[dev_drv->num_layer];
	}

	var = &info->var;
	screen_x = dev_drv->cur_screen->x_res;
	screen_y = dev_drv->cur_screen->y_res;
	
#if defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)||defined(CONFIG_NO_DUAL_DISP)
	if(dev_drv->cur_screen->screen_id == 1){
		dev_drv->cur_screen->xpos = (screen_x-screen_x*scale_x/100)>>1;
		dev_drv->cur_screen->ypos = (screen_y-screen_y*scale_y/100)>>1;
		dev_drv->cur_screen->xsize = screen_x*scale_x/100;
		dev_drv->cur_screen->ysize = screen_y*scale_y/100;
	}else
#endif
	{
		xpos = (screen_x-screen_x*scale_x/100)>>1;
		ypos = (screen_y-screen_y*scale_y/100)>>1;
		xsize = screen_x*scale_x/100;
		ysize = screen_y*scale_y/100;
		var->nonstd &= 0xff;
		var->nonstd |= (xpos<<8) + (ypos<<20);
		var->grayscale &= 0xff;
		var->grayscale |= (xsize<<8) + (ysize<<20);	
	}
	hdmi_xsize = xsize;
	hdmi_ysize = ysize;

	info->fbops->fb_set_par(info);
	info->fbops->fb_ioctl(info,RK_FBIOSET_CONFIG_DONE,0);
	return 0;
	
	
}

static int rk_request_fb_buffer(struct fb_info *fbi,int fb_id)
{
	struct rk_lcdc_device_driver * dev_drv = (struct rk_lcdc_device_driver * )fbi->par;
    	struct layer_par *par = NULL;
	int layer_id;
	struct resource *res;
	struct resource *mem;
	int ret = 0;
	struct rk_fb_inf *fb_inf = platform_get_drvdata(g_fb_pdev);
	if (!strcmp(fbi->fix.id,"fb0"))
	{
		res = platform_get_resource_byname(g_fb_pdev, IORESOURCE_MEM, "fb0 buf");
		if (res == NULL)
		{
			dev_err(&g_fb_pdev->dev, "failed to get memory for fb0 \n");
			ret = -ENOENT;
		}
		fbi->fix.smem_start = res->start;
		fbi->fix.smem_len = res->end - res->start + 1;
		mem = request_mem_region(res->start, resource_size(res), g_fb_pdev->name);
		fbi->screen_base = ioremap(res->start, fbi->fix.smem_len);
		memset(fbi->screen_base, 0, fbi->fix.smem_len);
		printk("fb%d:phy:%lx>>vir:%p>>len:0x%x\n",fb_id,
		fbi->fix.smem_start,fbi->screen_base,fbi->fix.smem_len);
	}
	else
	{	
#if defined(CONFIG_FB_ROTATE) || !defined(CONFIG_THREE_FB_BUFFER)
		res = platform_get_resource_byname(g_fb_pdev, IORESOURCE_MEM, "fb2 buf");
		if (res == NULL)
		{
			dev_err(&g_fb_pdev->dev, "failed to get win0 memory \n");
			ret = -ENOENT;
		}
		fbi->fix.smem_start = res->start;
		fbi->fix.smem_len = res->end - res->start + 1;
		mem = request_mem_region(res->start, resource_size(res), g_fb_pdev->name);
		fbi->screen_base = ioremap(res->start, fbi->fix.smem_len);
		memset(fbi->screen_base, 0, fbi->fix.smem_len);
#else    //three buffer no need to copy
		fbi->fix.smem_start = fb_inf->fb[0]->fix.smem_start;
		fbi->fix.smem_len   = fb_inf->fb[0]->fix.smem_len;
		fbi->screen_base    = fb_inf->fb[0]->screen_base;
#endif
		printk("fb%d:phy:%lx>>vir:%p>>len:0x%x\n",fb_id,
			fbi->fix.smem_start,fbi->screen_base,fbi->fix.smem_len);	
	}

	fbi->screen_size = fbi->fix.smem_len;
	layer_id = dev_drv->fb_get_layer(dev_drv,fbi->fix.id);
	if(layer_id >= 0)
	{
		par = dev_drv->layer_par[layer_id];
		par->reserved = fbi->fix.smem_start;
	}

    return ret;
}

static int rk_release_fb_buffer(struct fb_info *fbi)
{
	if(!fbi)
	{
		printk("no need release null fb buffer!\n");
		return -EINVAL;
	}
	if(!strcmp(fbi->fix.id,"fb1")||!strcmp(fbi->fix.id,"fb3"))  //buffer for fb1 and fb3 are alloc by android
		return 0;
	iounmap(fbi->screen_base);
	release_mem_region(fbi->fix.smem_start,fbi->fix.smem_len);
	return 0;
	
}
static int init_layer_par(struct rk_lcdc_device_driver *dev_drv)
{
       int i;
       struct layer_par * def_par = NULL;
       int num_par = dev_drv->num_layer;
       for(i = 0; i < num_par; i++)
       {
               struct layer_par *par = NULL;
               par =  kzalloc(sizeof(struct layer_par), GFP_KERNEL);
               if(!par)
               {
                       printk(KERN_ERR "kzmalloc for layer_par fail!");
                       return   -ENOMEM;
                       
               }
	       def_par = &dev_drv->def_layer_par[i];
               strcpy(par->name,def_par->name);
               par->id = def_par->id;
               par->support_3d = def_par->support_3d;
               dev_drv->layer_par[i] = par;
       }
               
       return 0;
       
       
}


static int init_lcdc_device_driver(struct rk_lcdc_device_driver *dev_drv,
	struct rk_lcdc_device_driver *def_drv,int id)
{
	if(!def_drv)
	{
		printk(KERN_ERR "default lcdc device driver is null!\n");
		return -EINVAL;
	}
	if(!dev_drv)
	{
		printk(KERN_ERR "lcdc device driver is null!\n");
		return -EINVAL;	
	}
	sprintf(dev_drv->name, "lcdc%d",id);
	dev_drv->id		= id;
	dev_drv->open      	= def_drv->open;
	dev_drv->init_lcdc 	= def_drv->init_lcdc;
	dev_drv->ioctl 		= def_drv->ioctl;
	dev_drv->blank 		= def_drv->blank;
	dev_drv->set_par 	= def_drv->set_par;
	dev_drv->pan_display 	= def_drv->pan_display;
	dev_drv->suspend 	= def_drv->suspend;
	dev_drv->resume 	= def_drv->resume;
	dev_drv->load_screen 	= def_drv->load_screen;
	dev_drv->def_layer_par 	= def_drv->def_layer_par;
	dev_drv->num_layer	= def_drv->num_layer;
	dev_drv->get_layer_state= def_drv->get_layer_state;
	dev_drv->get_disp_info  = def_drv->get_disp_info;
	dev_drv->ovl_mgr	= def_drv->ovl_mgr;
	dev_drv->fps_mgr	= def_drv->fps_mgr;
	if(def_drv->fb_get_layer)
		dev_drv->fb_get_layer   = def_drv->fb_get_layer;
	if(def_drv->fb_layer_remap)
		dev_drv->fb_layer_remap = def_drv->fb_layer_remap;
	if(def_drv->set_dsp_lut)
		dev_drv->set_dsp_lut    = def_drv->set_dsp_lut;
	if(def_drv->read_dsp_lut)
		dev_drv->read_dsp_lut   = def_drv->read_dsp_lut;
	if(def_drv->lcdc_hdmi_process)
		dev_drv->lcdc_hdmi_process = def_drv->lcdc_hdmi_process;
	if(def_drv->lcdc_reg_update)
		dev_drv->lcdc_reg_update = def_drv->lcdc_reg_update;
	if(def_drv->set_irq_to_cpu)
		 dev_drv->set_irq_to_cpu = def_drv->set_irq_to_cpu;
	if(def_drv->poll_vblank)
		dev_drv->poll_vblank = def_drv->poll_vblank;
	if(def_drv->dpi_open)
		dev_drv->dpi_open = def_drv->dpi_open;
	if(def_drv->dpi_layer_sel)
		dev_drv->dpi_layer_sel = def_drv->dpi_layer_sel;
	if(def_drv->dpi_status)
		dev_drv->dpi_status = def_drv->dpi_status;
	if(def_drv->get_dsp_addr)
		dev_drv->get_dsp_addr   = def_drv->get_dsp_addr;
	init_layer_par(dev_drv);
	init_completion(&dev_drv->frame_done);
	spin_lock_init(&dev_drv->cpl_lock);
	mutex_init(&dev_drv->fb_win_id_mutex);
	dev_drv->fb_layer_remap(dev_drv,FB_DEFAULT_ORDER); //102
	dev_drv->first_frame = 1;
	
	return 0;
}
 
#ifdef CONFIG_LOGO_LINUX_BMP
static struct linux_logo *bmp_logo;
static int fb_prepare_bmp_logo(struct fb_info *info, int rotate)
{
	bmp_logo = fb_find_logo(24);
	if (bmp_logo == NULL) {
		printk("%s error\n", __func__);
		return 0;
	}
	return 1;
}

static void fb_show_bmp_logo(struct fb_info *info, int rotate)
{
	unsigned char *src=bmp_logo->data;
	unsigned char *dst=info->screen_base;
	int i;
	unsigned int Needwidth=(*(src-24)<<8)|(*(src-23));
	unsigned int Needheight=(*(src-22)<<8)|(*(src-21));
		
	for(i=0;i<Needheight;i++)
		memcpy(dst+info->var.xres*i*4, src+bmp_logo->width*i*4, Needwidth*4);
	
}
#endif

int rk_fb_register(struct rk_lcdc_device_driver *dev_drv,
	struct rk_lcdc_device_driver *def_drv,int id)
{
	struct rk_fb_inf *fb_inf = platform_get_drvdata(g_fb_pdev);
	struct fb_info *fbi;
	int i=0,ret = 0;
	int lcdc_id = 0;
	if(NULL == dev_drv)
	{
        	printk("null lcdc device driver?");
        	return -ENOENT;
    	}
    	for(i=0;i<RK30_MAX_LCDC_SUPPORT;i++)
	{
        	if(NULL==fb_inf->lcdc_dev_drv[i])
		{
            		fb_inf->lcdc_dev_drv[i] = dev_drv;
            		fb_inf->lcdc_dev_drv[i]->id = id;
            		fb_inf->num_lcdc++;
            		break;
        	}
    	}
    	if(i==RK30_MAX_LCDC_SUPPORT)
	{
        	printk("rk_fb_register lcdc out of support %d",i);
        	return -ENOENT;
    	}
    	lcdc_id = i;
	init_lcdc_device_driver(dev_drv, def_drv,id);
	
	dev_drv->init_lcdc(dev_drv);
	/************fb set,one layer one fb ***********/
	dev_drv->fb_index_base = fb_inf->num_fb;
	for(i=0;i<dev_drv->num_layer;i++)
	{
		fbi= framebuffer_alloc(0, &g_fb_pdev->dev);
		if(!fbi)
		{
		    dev_err(&g_fb_pdev->dev,">> fb framebuffer_alloc fail!");
		    fbi = NULL;
		    ret = -ENOMEM;
		}
		fbi->par = dev_drv;
		fbi->var = def_var;
		fbi->fix = def_fix;
		sprintf(fbi->fix.id,"fb%d",fb_inf->num_fb);
		fbi->var.xres = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->x_res;
		fbi->var.yres = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->y_res;
		fbi->var.grayscale |= (fbi->var.xres<<8) + (fbi->var.yres<<20);
#ifdef  CONFIG_LOGO_LINUX_BMP
		fbi->var.bits_per_pixel = 32; 
#else
		fbi->var.bits_per_pixel = 16; 
#endif
		fbi->fix.line_length  = (fbi->var.xres)*(fbi->var.bits_per_pixel>>3);
		fbi->var.xres_virtual = fbi->var.xres;
		fbi->var.yres_virtual = fbi->var.yres;
		fbi->var.width =  fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->width;
		fbi->var.height = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->height;
		fbi->var.pixclock = fb_inf->lcdc_dev_drv[lcdc_id]->pixclock;
		fbi->var.left_margin = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->left_margin;
		fbi->var.right_margin = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->right_margin;
		fbi->var.upper_margin = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->upper_margin;
		fbi->var.lower_margin = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->lower_margin;
		fbi->var.vsync_len = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->vsync_len;
		fbi->var.hsync_len = fb_inf->lcdc_dev_drv[lcdc_id]->cur_screen->hsync_len;
		fbi->fbops			 = &fb_ops;
		fbi->flags			 = FBINFO_FLAG_DEFAULT;
		fbi->pseudo_palette  = fb_inf->lcdc_dev_drv[lcdc_id]->layer_par[i]->pseudo_pal;
		if (i == 0) //only alloc memory for main fb
		{
			rk_request_fb_buffer(fbi,fb_inf->num_fb);
		}
		ret = register_framebuffer(fbi);
		if(ret<0)
		{
		    printk("%s>>fb%d register_framebuffer fail!\n",__func__,fb_inf->num_fb);
		    ret = -EINVAL;
		}
		rkfb_create_sysfs(fbi);
		fb_inf->fb[fb_inf->num_fb] = fbi;
	        printk("%s>>>>>%s\n",__func__,fb_inf->fb[fb_inf->num_fb]->fix.id);
	        fb_inf->num_fb++;
		
		if(i == 0)
		{
			init_waitqueue_head(&dev_drv->vsync_info.wait);
			ret = device_create_file(fbi->dev,&dev_attr_vsync);
			if (ret) 
			{
				dev_err(fbi->dev, "failed to create vsync file\n");
			}
			dev_drv->vsync_info.thread = kthread_run(rk_fb_wait_for_vsync_thread,
				dev_drv, "fb-vsync");

			
			if (dev_drv->vsync_info.thread == ERR_PTR(-ENOMEM)) 
			{
				dev_err(fbi->dev, "failed to run vsync thread\n");
				dev_drv->vsync_info.thread = NULL;
			}
			dev_drv->vsync_info.active = 1;

			INIT_LIST_HEAD(&dev_drv->update_regs_list);
			mutex_init(&dev_drv->update_regs_list_lock);
			init_kthread_worker(&dev_drv->update_regs_worker);

			dev_drv->update_regs_thread = kthread_run(kthread_worker_fn,
					&dev_drv->update_regs_worker, "rk-fb");
			if (IS_ERR(dev_drv->update_regs_thread)) {
				int err = PTR_ERR(dev_drv->update_regs_thread);
				dev_drv->update_regs_thread = NULL;

				printk("failed to run update_regs thread\n");
				return err;
			}
			init_kthread_work(&dev_drv->update_regs_work, rk_fb_update_regs_handler);

			dev_drv->timeline = sw_sync_timeline_create("rk-fb");
			dev_drv->timeline_max = 1;

			
		}
			
	}
#if !defined(CONFIG_FRAMEBUFFER_CONSOLE) && defined(CONFIG_LOGO)
    if(dev_drv->screen_ctr_info->prop == PRMRY) //show logo for primary display device
    {
	    fb_inf->fb[0]->fbops->fb_open(fb_inf->fb[0],1);
	    fb_inf->fb[0]->fbops->fb_set_par(fb_inf->fb[0]);

#if  defined(CONFIG_LOGO_LINUX_BMP)
		if(fb_prepare_bmp_logo(fb_inf->fb[0], FB_ROTATE_UR)) {
			/* Start display and show logo on boot */
			fb_set_cmap(&fb_inf->fb[0]->cmap, fb_inf->fb[0]);
			fb_show_bmp_logo(fb_inf->fb[0], FB_ROTATE_UR);
			fb_inf->fb[0]->fbops->fb_pan_display(&(fb_inf->fb[0]->var), fb_inf->fb[0]);
		}
#else
		if(fb_prepare_logo(fb_inf->fb[0], FB_ROTATE_UR)) {
			/* Start display and show logo on boot */
			fb_set_cmap(&fb_inf->fb[0]->cmap, fb_inf->fb[0]);
			fb_show_logo(fb_inf->fb[0], FB_ROTATE_UR);
			fb_inf->fb[0]->fbops->fb_pan_display(&(fb_inf->fb[0]->var), fb_inf->fb[0]);
		}
#endif
	fb_inf->fb[0]->fbops->fb_ioctl(fb_inf->fb[0],RK_FBIOSET_CONFIG_DONE,0);
		
    }
#endif
	return 0;
	
	
}
int rk_fb_unregister(struct rk_lcdc_device_driver *dev_drv)
{

	struct rk_fb_inf *fb_inf = platform_get_drvdata(g_fb_pdev);
	struct fb_info *fbi;
	int fb_index_base = dev_drv->fb_index_base;
	int fb_num = dev_drv->num_layer;
	int i=0;
	if(NULL == dev_drv)
	{
		printk(" no need to unregister null lcdc device driver!\n");
		return -ENOENT;
	}

	if(fb_inf->lcdc_dev_drv[i]->vsync_info.thread){
		fb_inf->lcdc_dev_drv[i]->vsync_info.irq_stop = 1;
		kthread_stop(fb_inf->lcdc_dev_drv[i]->vsync_info.thread);
	}

	for(i = 0; i < fb_num; i++)
	{
		kfree(dev_drv->layer_par[i]);
	}

	for(i=fb_index_base;i<(fb_index_base+fb_num);i++)
	{
		fbi = fb_inf->fb[i];
		unregister_framebuffer(fbi);
		//rk_release_fb_buffer(fbi);
		framebuffer_release(fbi);	
	}
	fb_inf->lcdc_dev_drv[dev_drv->id]= NULL;
	fb_inf->num_lcdc--;

	return 0;
}



#ifdef CONFIG_HAS_EARLYSUSPEND
struct suspend_info {
	struct early_suspend early_suspend;
	struct rk_fb_inf *inf;
};

static void rkfb_early_suspend(struct early_suspend *h)
{
	struct suspend_info *info = container_of(h, struct suspend_info,
						early_suspend);
	struct rk_fb_inf *inf = info->inf;
	int i;
	for(i = 0; i < inf->num_lcdc; i++)
	{
		if (!inf->lcdc_dev_drv[i])
			continue;
			
		inf->lcdc_dev_drv[i]->suspend(inf->lcdc_dev_drv[i]);
	}
}
static void rkfb_early_resume(struct early_suspend *h)
{
	struct suspend_info *info = container_of(h, struct suspend_info,
						early_suspend);
	struct rk_fb_inf *inf = info->inf;
	int i;
	for(i = 0; i < inf->num_lcdc; i++)
	{
		if (!inf->lcdc_dev_drv[i])
			continue;
		
		inf->lcdc_dev_drv[i]->resume(inf->lcdc_dev_drv[i]);	       // data out
	}

}



static struct suspend_info suspend_info = {
	.early_suspend.suspend = rkfb_early_suspend,
	.early_suspend.resume = rkfb_early_resume,
	.early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
};
#endif

static int __devinit rk_fb_probe (struct platform_device *pdev)
{
	struct rk_fb_inf *fb_inf = NULL;
	int ret = 0;
	g_fb_pdev=pdev;
    	/* Malloc rk_fb_inf and set it to pdev for drvdata */
	fb_inf = kzalloc(sizeof(struct rk_fb_inf), GFP_KERNEL);
	if(!fb_inf)
	{
        	dev_err(&pdev->dev, ">>fb inf kmalloc fail!");
        	ret = -ENOMEM;
    	}
	platform_set_drvdata(pdev,fb_inf);

#ifdef CONFIG_HAS_EARLYSUSPEND
	suspend_info.inf = fb_inf;
	register_early_suspend(&suspend_info.early_suspend);
#endif
	printk("rk fb probe ok!\n");
    return 0;
}

static int __devexit rk_fb_remove(struct platform_device *pdev)
{
	struct rk_fb_inf *fb_inf = platform_get_drvdata(pdev);
	kfree(fb_inf);
    	platform_set_drvdata(pdev, NULL);
    	return 0;
}

static void rk_fb_shutdown(struct platform_device *pdev)
{
	struct rk_fb_inf *inf = platform_get_drvdata(pdev);
	int i;
	for(i = 0; i < inf->num_lcdc; i++)
	{
		if (!inf->lcdc_dev_drv[i])
			continue;

	}
//	kfree(fb_inf);
//	platform_set_drvdata(pdev, NULL);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&suspend_info.early_suspend);
#endif
}

static struct platform_driver rk_fb_driver = {
	.probe		= rk_fb_probe,
	.remove		= __devexit_p(rk_fb_remove),
	.driver		= {
		.name	= "rk-fb",
		.owner	= THIS_MODULE,
	},
	.shutdown   = rk_fb_shutdown,
};

static int __init rk_fb_init(void)
{
    return platform_driver_register(&rk_fb_driver);
}

static void __exit rk_fb_exit(void)
{
    platform_driver_unregister(&rk_fb_driver);
}

subsys_initcall_sync(rk_fb_init);
module_exit(rk_fb_exit);

