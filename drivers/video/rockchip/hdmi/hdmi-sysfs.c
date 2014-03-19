#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/display-sys.h>
#include <linux/interrupt.h>
#include "rk_hdmi.h"

static int hdmi_get_enable(struct rk_display_device *device)
{
	struct hdmi *hdmi = device->priv_data;
	int enable;
	
	enable = hdmi->enable;
	return enable;
}

static int hdmi_set_enable(struct rk_display_device *device, int enable)
{
	struct hdmi *hdmi = device->priv_data;
	
	if(enable == 0) {
		hdmi_submit_work(hdmi, HDMI_DISABLE_CTL, 0, NULL);
	}
	else {
		hdmi_submit_work(hdmi, HDMI_ENABLE_CTL, 0, NULL);
	}
	return 0;
}

static int hdmi_get_status(struct rk_display_device *device)
{
	struct hdmi *hdmi = device->priv_data;
	if(hdmi->hotplug == HDMI_HPD_ACTIVED)
		return 1;
	else
		return 0;
}

static int hdmi_get_modelist(struct rk_display_device *device, struct list_head **modelist)
{
	struct hdmi *hdmi = device->priv_data;

	*modelist = &hdmi->edid.modelist;
	return 0;
}

static int hdmi_set_mode(struct rk_display_device *device, struct fb_videomode *mode)
{
	struct hdmi *hdmi = device->priv_data;
	int vic = hdmi_videomode_to_vic(mode);
	
	hdmi->autoset = 0;
	if(vic && hdmi->vic != vic)
	{
		hdmi->vic = vic;
		if(hdmi->hotplug == HDMI_HPD_ACTIVED)
			hdmi_submit_work(hdmi, HDMI_SET_VIDEO, 0, NULL);
	}
	return 0;
}

static int hdmi_get_mode(struct rk_display_device *device, struct fb_videomode *mode)
{
	struct hdmi *hdmi = device->priv_data;
	struct fb_videomode *vmode;
		
	vmode = (struct fb_videomode*) hdmi_vic_to_videomode(hdmi->vic);
	if(unlikely(vmode == NULL))
		return -1;
	*mode = *vmode;
	return 0;
}

static int hdmi_set_scale(struct rk_display_device *device, int direction, int value)
{
	struct hdmi *hdmi = device->priv_data;
	
	if(!hdmi || value < 0 || value > 100)
		return -1;
			
	if(direction == DISPLAY_SCALE_X)
		hdmi->xscale = value;
	else if(direction == DISPLAY_SCALE_Y)
		hdmi->yscale = value;
	else
		return -1;
//	rk_fb_disp_scale(hdmi->xscale, hdmi->yscale, HDMI_SOURCE_DEFAULT);
	return 0;
}

static int hdmi_get_scale(struct rk_display_device *device, int direction)
{
	struct hdmi *hdmi = device->priv_data;
	
	if(!hdmi)
		return -1;
		
	if(direction == DISPLAY_SCALE_X)
		return hdmi->xscale;
	else if(direction == DISPLAY_SCALE_Y)
		return hdmi->yscale;
	else
		return -1;
}

static int hdmi_set_3dmode(struct rk_display_device *device, int mode)
{
	struct hdmi *hdmi = device->priv_data;
	struct list_head *modelist, *pos;
	struct display_modelist *display_modelist = NULL;
	
	if(!hdmi)
		return -1;
	modelist = &hdmi->edid.modelist;
	list_for_each(pos, modelist) {
		display_modelist = list_entry(pos, struct display_modelist, list);
		if(hdmi->vic == display_modelist->vic)
			break;
		else
			display_modelist = NULL;
	}
	
	if(!display_modelist)
		return -1;
		
	if( (mode != HDMI_3D_NONE) && ((display_modelist->format_3d & ( 1 << mode)) == 0 ))
		return -1;
	
	if(hdmi->mode_3d != mode)
	{
		hdmi->mode_3d = mode;
		if(hdmi->hotplug == HDMI_HPD_ACTIVED) {
			hdmi_submit_work(hdmi, HDMI_SET_3D, 0, NULL);
		}
	}
	return 0;
}

static int hdmi_get_3dmode(struct rk_display_device *device)
{
	struct hdmi *hdmi = device->priv_data;
	if(!hdmi)
		return -1;
	else
		return hdmi->mode_3d;
}

//CEA 861-E: Audio Coding Type
//sync width enum hdmi_audio_type
static const char* const sAudioFormatStr[] = {
	"",
	"LPCM",		//HDMI_AUDIO_LPCM = 1,
	"AC3",		//HDMI_AUDIO_AC3,
	"MPEG1",	//HDMI_AUDIO_MPEG1,
	"MP3",		//HDMI_AUDIO_MP3,
	"MPEG2",	//HDMI_AUDIO_MPEG2,
	"AAC-LC",	//HDMI_AUDIO_AAC_LC,		//AAC
	"DTS",		//HDMI_AUDIO_DTS,
	"ATARC",	//HDMI_AUDIO_ATARC,
	"DSD",		//HDMI_AUDIO_DSD,			//One bit Audio
	"E-AC3",	//HDMI_AUDIO_E_AC3,
	"DTS-HD",	//HDMI_AUDIO_DTS_HD,
	"MLP",		//HDMI_AUDIO_MLP,
	"DST",		//HDMI_AUDIO_DST,
	"WMA-PRO",	//HDMI_AUDIO_WMA_PRO
};

static int hdmi_get_edidaudioinfo(struct rk_display_device *device, char *audioinfo, int len)
{
	struct hdmi *hdmi = device->priv_data;
	int i=0;
	int size=0;
	struct hdmi_audio *audio;
	if(!hdmi)
		return -1;

	memset(audioinfo, 0x00, len);

	//printk("hdmi:edid: audio_num: %d\n", hdmi->edid.audio_num);
	for(i = 0; i < hdmi->edid.audio_num; i++)
	{
		audio = &(hdmi->edid.audio[i]);
		if(audio->type<1 || audio->type>HDMI_AUDIO_WMA_PRO){
			printk("audio type: unsupported.");
			continue;
		}
		size = strlen(sAudioFormatStr[audio->type]);
		//printk("size: %d, type: %s\n", size, sAudioFormatStr[audio->type]);
		memcpy(audioinfo, sAudioFormatStr[audio->type], size);
		audioinfo[size]=',';
		audioinfo += (size+1);
	}

	return 0;
}


static struct rk_display_ops hdmi_display_ops = {
	.setenable = hdmi_set_enable,
	.getenable = hdmi_get_enable,
	.getstatus = hdmi_get_status,
	.getmodelist = hdmi_get_modelist,
	.setmode = hdmi_set_mode,
	.getmode = hdmi_get_mode,
	.setscale = hdmi_set_scale,
	.getscale = hdmi_get_scale,
	.set3dmode = hdmi_set_3dmode,
	.get3dmode = hdmi_get_3dmode,
	.getedidaudioinfo = hdmi_get_edidaudioinfo,
};

static int hdmi_display_probe(struct rk_display_device *device, void *devdata)
{
	struct hdmi *hdmi = devdata;
	
	device->owner = THIS_MODULE;
	strcpy(device->type, "HDMI");
	device->priority = DISPLAY_PRIORITY_HDMI;
	device->name = hdmi->property->name;
	device->property = hdmi->property->display;
	device->priv_data = devdata;
	device->ops = &hdmi_display_ops;
	return 1;
}

static struct rk_display_driver display_hdmi = {
	.probe = hdmi_display_probe,
};

struct rk_display_device* hdmi_register_display_sysfs(struct hdmi *hdmi, struct device *parent)
{
	return rk_display_device_register(&display_hdmi, parent, hdmi);
}

void hdmi_unregister_display_sysfs(struct hdmi *hdmi)
{
	if(hdmi->ddev)
		rk_display_device_unregister(hdmi->ddev);
}
