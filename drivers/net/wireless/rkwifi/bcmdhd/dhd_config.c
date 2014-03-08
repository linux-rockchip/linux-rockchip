
#include <typedefs.h>
#include <osl.h>

#include <bcmutils.h>
#if defined(HW_OOB)
#include <bcmdefs.h>
#include <bcmsdh.h>
#include <hndsoc.h>
#include <sdio.h>
#include <sbchipc.h>
#endif

#include <dhd_config.h>
#include <dhd_dbg.h>

/* message levels */
#define CONFIG_ERROR_LEVEL	0x0001
#define CONFIG_TRACE_LEVEL	0x0002

uint config_msg_level = CONFIG_ERROR_LEVEL;

#define CONFIG_ERROR(x) \
	do { \
		if (config_msg_level & CONFIG_ERROR_LEVEL) { \
			printk(KERN_ERR "CONFIG-ERROR) ");	\
			printk x; \
		} \
	} while (0)
#define CONFIG_TRACE(x) \
	do { \
		if (config_msg_level & CONFIG_TRACE_LEVEL) { \
			printk(KERN_ERR "CONFIG-TRACE) ");	\
			printk x; \
		} \
	} while (0)

#define MAXSZ_BUF		512
#define	MAXSZ_CONFIG	4096

#define BCM43362A0_CHIP_REV     0
#define BCM43362A2_CHIP_REV     1
#define BCM4330B2_CHIP_REV      4
#define BCM43341B0_CHIP_REV     2
#define BCM43241B4_CHIP_REV     5
#define BCM4335A0_CHIP_REV      2
#define BCM4339A0_CHIP_REV      1

#define FW_TYPE_STA     0
#define FW_TYPE_APSTA   1
#define FW_TYPE_P2P     2
#define FW_TYPE_MFG     3
#define FW_TYPE_G       0
#define FW_TYPE_AG      1

const static char *bcm4330b2_fw_name[] = {
	"fw_RK903b2.bin",
	"fw_RK903b2_apsta.bin",
	"fw_RK903b2_p2p.bin",
	"fw_RK903b2_mfg.bin"
};

const static char *bcm4330b2ag_fw_name[] = {
	"fw_RK903_ag.bin",
	"fw_RK903_ag_apsta.bin",
	"fw_RK903_ag_p2p.bin",
	"fw_RK903_ag_mfg.bin"
};

const static char *bcm43362a0_fw_name[] = {
	"fw_RK901a0.bin",
	"fw_RK901a0_apsta.bin",
	"fw_RK901a0_p2p.bin",
	"fw_RK901a0_mfg.bin"
};

const static char *bcm43362a2_fw_name[] = {
	"fw_RK901a2.bin",
	"fw_RK901a2_apsta.bin",
	"fw_RK901a2_p2p.bin",
	"fw_RK901a2_mfg.bin"
};

const static char *bcm43341b0ag_fw_name[] = {
	"fw_bcm43341b0_ag.bin",
	"fw_bcm43341b0_ag_apsta.bin",
	"fw_bcm43341b0_ag_p2p.bin",
	"fw_bcm43341b0_ag_mfg.bin"
};

const static char *bcm43241b4ag_fw_name[] = {
	"fw_bcm43241b4_ag.bin",
	"fw_bcm43241b4_ag_apsta.bin",
	"fw_bcm43241b4_ag_p2p.bin",
	"fw_bcm43241b4_ag_mfg.bin"
};

const static char *bcm4339a0ag_fw_name[] = {
	"fw_bcm4339a0_ag.bin",
	"fw_bcm4339a0_ag_apsta.bin",
	"fw_bcm4339a0_ag_p2p.bin",
	"fw_bcm4339a0_ag_mfg.bin"
};

void
dhd_conf_set_fw_name_by_chip(dhd_pub_t *dhd, char *dst, char *src)
{
    int fw_type, ag_type;
    static uint chip, chiprev, first=1;
    int i;

    //if(src[0] == '\0')
    // rule out bcm4330 (gwl)
    if (strstr(dst, "bcm4330"))
        return;

    //strcpy(dst, src);

    /* find out the last '/' */
    i = strlen(dst);
    while (i>0){
        if (dst[i] == '/') break;
        i--;
    }

    ag_type = strstr(&dst[i], "_ag") ? FW_TYPE_AG : FW_TYPE_G;
    fw_type = (strstr(&src[i], "_mfg") ?
        FW_TYPE_MFG : (strstr(&src[i], "_apsta") ?
        FW_TYPE_APSTA : (strstr(&src[i], "_p2p") ?
        FW_TYPE_P2P : FW_TYPE_STA)));

    if (first) {
        chip = dhd_bus_chip_id(dhd);
        chiprev = dhd_bus_chiprev_id(dhd);
        first = 0;
    }

    if (ag_type == FW_TYPE_G) {
        switch (chip) {
            case BCM4330_CHIP_ID:
                    strcpy(&dst[i+1], bcm4330b2_fw_name[fw_type]);
                break;
            case BCM43362_CHIP_ID:
                if (chiprev==BCM43362A0_CHIP_REV)
                    strcpy(&dst[i+1], bcm43362a0_fw_name[fw_type]);
                else
                    strcpy(&dst[i+1], bcm43362a2_fw_name[fw_type]);
                break;
            case BCM43341_CHIP_ID:
                if (chiprev == BCM43341B0_CHIP_REV)
                    strcpy(&dst[i+1], bcm43341b0ag_fw_name[fw_type]);
                break;
            case BCM4324_CHIP_ID:
                if (chiprev == BCM43241B4_CHIP_REV)
                    strcpy(&dst[i+1], bcm43241b4ag_fw_name[fw_type]);
                break;
            case BCM4335_CHIP_ID:
                if (chiprev == BCM4335A0_CHIP_REV)
                    strcpy(&dst[i+1], bcm4339a0ag_fw_name[fw_type]);
                break;
            case BCM4339_CHIP_ID:
                if (chiprev == BCM4339A0_CHIP_REV)
                    strcpy(&dst[i+1], bcm4339a0ag_fw_name[fw_type]);
                break;
        }
    } else {
        switch (chip) {
            case BCM4330_CHIP_ID:
                    strcpy(&dst[i+1], bcm4330b2ag_fw_name[fw_type]);
                break;
        }
    }

    printk("%s: firmware_path=%s\n", __FUNCTION__, dst);
}

#if defined(HW_OOB)
void
dhd_conf_set_hw_oob_intr(bcmsdh_info_t *sdh, uint chip)
{
	uint32 gpiocontrol, addr;

	if (CHIPID(chip) == BCM43362_CHIP_ID) {
		printf("%s: Enable HW OOB for 43362\n", __FUNCTION__);
		addr = SI_ENUM_BASE + OFFSETOF(chipcregs_t, gpiocontrol);
		gpiocontrol = bcmsdh_reg_read(sdh, addr, 4);
		gpiocontrol |= 0x2;
		bcmsdh_reg_write(sdh, addr, 4, gpiocontrol);
		bcmsdh_cfg_write(sdh, SDIO_FUNC_1, 0x10005, 0xf, NULL);
		bcmsdh_cfg_write(sdh, SDIO_FUNC_1, 0x10006, 0x0, NULL);
		bcmsdh_cfg_write(sdh, SDIO_FUNC_1, 0x10007, 0x2, NULL);
	}
}
#endif

void
dhd_conf_set_fw_path(dhd_pub_t *dhd, char *fw_path)
{
	if (dhd->conf->fw_path[0]) {
		strcpy(fw_path, dhd->conf->fw_path);
		printf("%s: fw_path is changed to %s\n", __FUNCTION__, fw_path);
	}
}

void
dhd_conf_set_nv_path(dhd_pub_t *dhd, char *nv_path)
{
	if (dhd->conf->nv_path[0]) {
		strcpy(nv_path, dhd->conf->nv_path);
		printf("%s: nv_path is changed to %s\n", __FUNCTION__, nv_path);
	}
}

int
dhd_conf_set_band(dhd_pub_t *dhd)
{
	int bcmerror = -1;

	if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_BAND, &dhd->conf->band,
		sizeof(dhd->conf->band), TRUE, 0)) < 0)
		CONFIG_ERROR(("%s: band set failed %d\n", __FUNCTION__, bcmerror));

	return bcmerror;
}

uint
dhd_conf_get_band(dhd_pub_t *dhd)
{
	return dhd->conf->band;
}

int
dhd_conf_set_country(dhd_pub_t *dhd)
{
	int bcmerror = -1;
	char iovbuf[WL_EVENTING_MASK_LEN + 12];	/*  Room for "event_msgs" + '\0' + bitvec  */
	
	memset(&dhd->dhd_cspec, 0, sizeof(wl_country_t));
	printf("%s: Set country %s, revision %d\n", __FUNCTION__,
		dhd->conf->cspec.ccode, dhd->conf->cspec.rev);
	bcm_mkiovar("country", (char *)&dhd->conf->cspec,
		sizeof(wl_country_t), iovbuf, sizeof(iovbuf));
	if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0)) < 0)
		printf("%s: country code setting failed %d\n", __FUNCTION__, bcmerror);

	return bcmerror;
}

int
dhd_conf_get_country(dhd_pub_t *dhd)
{
	int bcmerror = -1;
	wl_country_t cspec;

	memset(&cspec, 0, sizeof(wl_country_t));
	bcm_mkiovar("country", NULL, 0, (char*)&cspec, sizeof(wl_country_t));
	if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_GET_VAR, &cspec, sizeof(wl_country_t), FALSE, 0)) < 0)
		printf("%s: country code getting failed %d\n", __FUNCTION__, bcmerror);
	else
		printf("Country code: %s (%s/%d)\n", cspec.country_abbrev, cspec.ccode, cspec.rev);

	return bcmerror;
}

bool
dhd_conf_match_channel(dhd_pub_t *dhd, uint32 channel)
{
	int i;

	if (dhd->conf->channels.count== 0)
		return true;
	for (i=0; i<dhd->conf->channels.count; i++) {
		if (channel == dhd->conf->channels.channel[i]) {
			return true;
		}
	}

	return false;
}

int
dhd_conf_set_roam(dhd_pub_t *dhd)
{
	int bcmerror = -1;
	char iovbuf[WL_EVENTING_MASK_LEN + 12];	/*  Room for "event_msgs" + '\0' + bitvec  */

	printf("%s: Set roam_off %d\n", __FUNCTION__, dhd->conf->roam_off);
	dhd_roam_disable = dhd->conf->roam_off;
	bcm_mkiovar("roam_off", (char *)&dhd->conf->roam_off, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);

	if (!dhd->conf->roam_off || !dhd->conf->roam_off_suspend) {
		printf("%s: Set roam_trigger %d\n", __FUNCTION__, dhd->conf->roam_trigger[0]);
		if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_TRIGGER, dhd->conf->roam_trigger,
				sizeof(dhd->conf->roam_trigger), TRUE, 0)) < 0)
			CONFIG_ERROR(("%s: roam trigger set failed %d\n", __FUNCTION__, bcmerror));

		printf("%s: Set roam_scan_period %d\n", __FUNCTION__, dhd->conf->roam_scan_period[0]);
		if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_SCAN_PERIOD, dhd->conf->roam_scan_period,
				sizeof(dhd->conf->roam_scan_period), TRUE, 0)) < 0)
			CONFIG_ERROR(("%s: roam scan period set failed %d\n", __FUNCTION__, bcmerror));

		printf("%s: Set roam_delta %d\n", __FUNCTION__, dhd->conf->roam_delta[0]);
		if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_DELTA, dhd->conf->roam_delta,
				sizeof(dhd->conf->roam_delta), TRUE, 0)) < 0)
			CONFIG_ERROR(("%s: roam delta set failed %d\n", __FUNCTION__, bcmerror));

		printf("%s: Set fullroamperiod %d\n", __FUNCTION__, dhd->conf->fullroamperiod);
		bcm_mkiovar("fullroamperiod", (char *)&dhd->conf->fullroamperiod, 4, iovbuf, sizeof(iovbuf));
		if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0)) < 0)
			CONFIG_ERROR(("%s: roam fullscan period set failed %d\n", __FUNCTION__, bcmerror));
	}

	return bcmerror;
}

void
dhd_conf_set_bw(dhd_pub_t *dhd)
{
	int bcmerror = -1;
	char iovbuf[WL_EVENTING_MASK_LEN + 12];	/*  Room for "event_msgs" + '\0' + bitvec  */
	uint32 mimo_bw_cap = 1; /* Turn HT40 on in 2.4 GHz */

	if (dhd_bus_chip_id(dhd) == BCM43341_CHIP_ID ||
			dhd_bus_chip_id(dhd) == BCM4324_CHIP_ID ||
			dhd_bus_chip_id(dhd) == BCM4335_CHIP_ID) {
		/* Enable HT40 in 2.4 GHz */
		printf("%s: Enable HT40 in 2.4 GHz\n", __FUNCTION__);
		bcm_mkiovar("mimo_bw_cap", (char *)&mimo_bw_cap, 4, iovbuf, sizeof(iovbuf));
		if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0)) < 0)
			CONFIG_ERROR(("%s: mimo_bw_cap set failed %d\n", __FUNCTION__, bcmerror));
	}
}


void
dhd_conf_set_srl(dhd_pub_t *dhd)
{
	int bcmerror = -1;
	uint srl = 0;

	if (dhd->conf->srl >= 0) {
		srl = (uint)dhd->conf->srl;
		printf("%s: set srl %d\n", __FUNCTION__, srl);
		if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_SRL, &srl , sizeof(srl), true, 0)) < 0)
			CONFIG_ERROR(("%s: WLC_SET_SRL setting failed %d\n", __FUNCTION__, bcmerror));
	}
}

void
dhd_conf_set_lrl(dhd_pub_t *dhd)
{
	int bcmerror = -1;
	uint lrl = 0;

	if (dhd->conf->lrl >= 0) {
		lrl = (uint)dhd->conf->lrl;
		printf("%s: set lrl %d\n", __FUNCTION__, lrl);
		if ((bcmerror = dhd_wl_ioctl_cmd(dhd, WLC_SET_LRL, &lrl , sizeof(lrl), true, 0)) < 0)
			CONFIG_ERROR(("%s: WLC_SET_LRL setting failed %d\n", __FUNCTION__, bcmerror));
	}
}


unsigned int
process_config_vars(char *varbuf, unsigned int len, char *pickbuf, char *param)
{
	bool findNewline, pick=FALSE;
	int column;
	unsigned int n, pick_column=0;

	findNewline = FALSE;
	column = 0;

	for (n = 0; n < len; n++) {
		if (varbuf[n] == '\r')
			continue;
		if (findNewline && varbuf[n] != '\n')
			continue;
		findNewline = FALSE;
		if (varbuf[n] == '#') {
			findNewline = TRUE;
			continue;
		}
		if (varbuf[n] == '\n') {
			if (column == 0)
				continue;
			column = 0;
			continue;
		}
		if (!memcmp(&varbuf[n], param, strlen(param)) && column==0) {
			pick = TRUE;
			column = strlen(param);
			n += column;
			pick_column = 0;
		} else {
			if (pick && column==0)
				pick = FALSE;
			else
				column++;
		}
		if (pick) {
			pickbuf[pick_column] = varbuf[n];
			pick_column++;
		}
	}

	return pick_column;
}

int
dhd_conf_download_config(dhd_pub_t *dhd)
{
	int bcmerror = -1, i;
	uint len, len_val;
	void * image = NULL;
	char * memblock = NULL;
	char *bufp, pick[MAXSZ_BUF], *pch, *pick_tmp;
	char *pconf_path;
	bool conf_file_exists;

	pconf_path = dhd->conf_path;

	conf_file_exists = ((pconf_path != NULL) && (pconf_path[0] != '\0'));
	if (!conf_file_exists)
		return (0);

	if (conf_file_exists) {
		image = dhd_os_open_image(pconf_path);
		if (image == NULL) {
			printk("%s: Ignore config file %s\n", __FUNCTION__, pconf_path);
			goto err;
		}
	}

	memblock = MALLOC(dhd->osh, MAXSZ_CONFIG);
	if (memblock == NULL) {
		CONFIG_ERROR(("%s: Failed to allocate memory %d bytes\n",
		           __FUNCTION__, MAXSZ_CONFIG));
		goto err;
	}

	/* Download variables */
	if (conf_file_exists) {
		len = dhd_os_get_image_block(memblock, MAXSZ_CONFIG, image);
	}
	if (len > 0 && len < MAXSZ_CONFIG) {
		bufp = (char *)memblock;
		bufp[len] = 0;

		/* Process firmware path */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "fw_path=");
		if (len_val) {
			memcpy(dhd->conf->fw_path, pick, len_val);
			printf("%s: fw_path = %s\n", __FUNCTION__, dhd->conf->fw_path);
		}

		/* Process nvram path */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "nv_path=");
		if (len_val) {
			memcpy(dhd->conf->nv_path, pick, len_val);
			printf("%s: nv_path = %s\n", __FUNCTION__, dhd->conf->nv_path);
		}

		/* Process band */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "band=");
		if (len_val) {
			if (!strncmp(pick, "b", len_val))
				dhd->conf->band = WLC_BAND_2G;
			printf("%s: band = %d\n", __FUNCTION__, dhd->conf->band);
		}

		/* Process country code */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "ccode=");
		if (len_val) {
			memset(&dhd->conf->cspec, 0, sizeof(wl_country_t));
			memcpy(dhd->conf->cspec.country_abbrev, pick, len_val);
			memcpy(dhd->conf->cspec.ccode, pick, len_val);
			memset(pick, 0, MAXSZ_BUF);
			len_val = process_config_vars(bufp, len, pick, "regrev=");
			if (len_val)
				dhd->conf->cspec.rev = (int32)simple_strtol(pick, NULL, 10);
		}

		/* Process channels */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "channels=");
		pick_tmp = pick;
		if (len_val) {
			pch = bcmstrtok(&pick_tmp, " ,.-", 0);
			i=0;
			while (pch != NULL && i<WL_NUMCHANNELS) {
				dhd->conf->channels.channel[i] = (uint32)simple_strtol(pch, NULL, 10);
				pch = bcmstrtok(&pick_tmp, " ,.-", 0);
				i++;
			}
			dhd->conf->channels.count = i;
			printf("%s: channels = ", __FUNCTION__);
			for (i=0; i<dhd->conf->channels.count; i++)
				printf("%d ", dhd->conf->channels.channel[i]);
			printf("\n");
		}

		/* Process roam */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "roam_off=");
		if (len_val) {
			if (!strncmp(pick, "0", len_val))
				dhd->conf->roam_off = 0;
			else
				dhd->conf->roam_off = 1;
			printf("%s: roam_off = %d\n", __FUNCTION__, dhd->conf->roam_off);
		}

		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "roam_off_suspend=");
		if (len_val) {
			if (!strncmp(pick, "0", len_val))
				dhd->conf->roam_off_suspend = 0;
			else
				dhd->conf->roam_off_suspend = 1;
			printf("%s: roam_off_suspend = %d\n", __FUNCTION__,
				dhd->conf->roam_off_suspend);
		}

		if (!dhd->conf->roam_off || !dhd->conf->roam_off_suspend) {
			memset(pick, 0, MAXSZ_BUF);
			len_val = process_config_vars(bufp, len, pick, "roam_trigger=");
			if (len_val)
				dhd->conf->roam_trigger[0] = (int)simple_strtol(pick, NULL, 10);
			printf("%s: roam_trigger = %d\n", __FUNCTION__,
				dhd->conf->roam_trigger[0]);

			memset(pick, 0, MAXSZ_BUF);
			len_val = process_config_vars(bufp, len, pick, "roam_scan_period=");
			if (len_val)
				dhd->conf->roam_scan_period[0] = (int)simple_strtol(pick, NULL, 10);
			printf("%s: roam_scan_period = %d\n", __FUNCTION__,
				dhd->conf->roam_scan_period[0]);

			memset(pick, 0, MAXSZ_BUF);
			len_val = process_config_vars(bufp, len, pick, "roam_delta=");
			if (len_val)
				dhd->conf->roam_delta[0] = (int)simple_strtol(pick, NULL, 10);
			printf("%s: roam_delta = %d\n", __FUNCTION__, dhd->conf->roam_delta[0]);

			memset(pick, 0, MAXSZ_BUF);
			len_val = process_config_vars(bufp, len, pick, "fullroamperiod=");
			if (len_val)
				dhd->conf->fullroamperiod = (int)simple_strtol(pick, NULL, 10);
			printf("%s: fullroamperiod = %d\n", __FUNCTION__,
				dhd->conf->fullroamperiod);
		}

		/* Process filter out all packets */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "filter_out_all_packets=");
		if (len_val) {
			if (!strncmp(pick, "0", len_val))
				dhd->conf->filter_out_all_packets = 0;
			else
				dhd->conf->filter_out_all_packets = 1;
			printf("%s: filter_out_all_packets = %d\n", __FUNCTION__,
				dhd->conf->filter_out_all_packets);
		}

		/* Process keep alive period */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "keep_alive_period=");
		if (len_val) {
			dhd->conf->keep_alive_period = (int)simple_strtol(pick, NULL, 10);
			printf("%s: keep_alive_period = %d\n", __FUNCTION__,
				dhd->conf->keep_alive_period);
		}

		/* Process dhd_doflow parameters */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "dhd_doflow=");
		if (len_val) {
			if (!strncmp(pick, "0", len_val))
				dhd_doflow = FALSE;
			else
				dhd_doflow = TRUE;
			printf("%s: dhd_doflow = %d\n", __FUNCTION__, dhd_doflow);
		}


		/* Process srl parameters */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "srl=");
		if (len_val) {
			dhd->conf->srl = (int)simple_strtol(pick, NULL, 10);
			printf("%s: srl = %d\n", __FUNCTION__, dhd->conf->srl);
		}

		/* Process lrl parameters */
		memset(pick, 0, MAXSZ_BUF);
		len_val = process_config_vars(bufp, len, pick, "lrl=");
		if (len_val) {
			dhd->conf->lrl = (int)simple_strtol(pick, NULL, 10);
			printf("%s: lrl = %d\n", __FUNCTION__, dhd->conf->lrl);
		}
 

		bcmerror = 0;
	} else {
		CONFIG_ERROR(("%s: error reading config file: %d\n", __FUNCTION__, len));
		bcmerror = BCME_SDIO_ERROR;
	}

err:
	if (memblock)
		MFREE(dhd->osh, memblock, MAXSZ_CONFIG);

	if (image)
		dhd_os_close_image(image);

	return bcmerror;
}

int
dhd_conf_preinit(dhd_pub_t *dhd)
{
	memset(dhd->conf, 0, sizeof(dhd_conf_t));

	dhd->conf->band = WLC_BAND_AUTO;
	strcpy(dhd->conf->cspec.country_abbrev, "ALL");
	dhd->conf->cspec.rev = 0;
	strcpy(dhd->conf->cspec.ccode, "ALL");
	memset(&dhd->conf->channels, 0, sizeof(wl_channel_list_t));
	dhd->conf->roam_off = 1;
	dhd->conf->roam_off_suspend = 1;
#ifdef CUSTOM_ROAM_TRIGGER_SETTING
	dhd->conf->roam_trigger[0] = CUSTOM_ROAM_TRIGGER_SETTING;
#else
	dhd->conf->roam_trigger[0] = -65;
#endif
	dhd->conf->roam_trigger[1] = WLC_BAND_ALL;
	dhd->conf->roam_scan_period[0] = 10;
	dhd->conf->roam_scan_period[1] = WLC_BAND_ALL;
#ifdef CUSTOM_ROAM_DELTA_SETTING
	dhd->conf->roam_delta[0] = CUSTOM_ROAM_DELTA_SETTING;
#else
	dhd->conf->roam_delta[0] = 15;
#endif
	dhd->conf->roam_delta[1] = WLC_BAND_ALL;
#ifdef FULL_ROAMING_SCAN_PERIOD_60_SEC
	dhd->conf->fullroamperiod = 60;
#else /* FULL_ROAMING_SCAN_PERIOD_60_SEC */
	dhd->conf->fullroamperiod = 120;
#endif /* FULL_ROAMING_SCAN_PERIOD_60_SEC */
	dhd->conf->filter_out_all_packets = 0;
#ifdef CUSTOM_KEEP_ALIVE_SETTING
	dhd->conf->keep_alive_period = CUSTOM_KEEP_ALIVE_SETTING;
#else
	dhd->conf->keep_alive_period = 28000;
#endif
	dhd->conf->srl = -1;
	dhd->conf->lrl = -1;

	return 0;
}

int
dhd_conf_attach(dhd_pub_t *dhd)
{
	dhd_conf_t *conf;

	dhd->conf = NULL;
	/* Allocate private bus interface state */
	if (!(conf = MALLOC(dhd->osh, sizeof(dhd_conf_t)))) {
		CONFIG_ERROR(("%s: MALLOC failed\n", __FUNCTION__));
		goto fail;
	}
	memset(conf, 0, sizeof(dhd_conf_t));

	dhd->conf = conf;

	return 0;

fail:
	if (conf != NULL)
		MFREE(dhd->osh, conf, sizeof(dhd_conf_t));
	return BCME_NOMEM;
}

void
dhd_conf_detach(dhd_pub_t *dhd)
{
	if (dhd->conf)
		MFREE(dhd->osh, dhd->conf, sizeof(dhd_conf_t));
	dhd->conf = NULL;
}

#ifdef POWER_OFF_IN_SUSPEND
struct net_device *g_netdev;
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
struct sdio_early_suspend_info {
	struct sdio_func *func;
	struct early_suspend sdio_early_suspend;
	struct work_struct	tqueue;
	int do_late_resume;
};
struct sdio_early_suspend_info sdioinfo[4];

void
dhd_conf_wifi_stop(struct net_device *dev)
{
	if (!dev) {
		CONFIG_ERROR(("%s: dev is null\n", __FUNCTION__));
		return;
	}

	printk("%s in 1\n", __FUNCTION__);
	dhd_net_if_lock(dev);
	printk("%s in 2: g_wifi_on=%d, name=%s\n", __FUNCTION__, g_wifi_on, dev->name);
	if (g_wifi_on) {
		wl_cfg80211_user_sync(true);
		wl_cfg80211_stop();
		dhd_bus_devreset(bcmsdh_get_drvdata(), true);
		sdioh_stop(NULL);
		dhd_customer_gpio_wlan_ctrl(WLAN_RESET_OFF);
		g_wifi_on = FALSE;
		wl_cfg80211_user_sync(false);
	}
	printk("%s out\n", __FUNCTION__);
	dhd_net_if_unlock(dev);

}

void
dhd_conf_wifi_power(bool on)
{
	printk("%s: Enter %d\n", __FUNCTION__, on);
	if (on) {
		wl_cfg80211_user_sync(true);
		wl_android_wifi_on(g_netdev);
		wl_cfg80211_send_disconnect();
		wl_cfg80211_user_sync(false);
	} else {
		dhd_conf_wifi_stop(g_netdev);
	}
}

void
dhd_conf_probe_workqueue(struct work_struct *work)
{
    dhd_conf_wifi_power(true);
}

void
dhd_conf_early_suspend(struct early_suspend *h)
{
	struct sdio_early_suspend_info *sdioinfo = container_of(h, struct sdio_early_suspend_info, sdio_early_suspend);

	printk("%s: Enter\n", __FUNCTION__);
	if(sdioinfo->func->num == 2)
		sdioinfo->do_late_resume = 0;
}

void
dhd_conf_late_resume(struct early_suspend *h)
{
	struct sdio_early_suspend_info *sdioinfo = container_of(h, struct sdio_early_suspend_info, sdio_early_suspend);

	printk("%s: Enter\n", __FUNCTION__);
	if(sdioinfo->func->num == 2 && sdioinfo->do_late_resume ){
		sdioinfo->do_late_resume = 0;
		schedule_work(&sdioinfo->tqueue);
	}
}
#endif /* defined(CONFIG_HAS_EARLYSUSPEND) */

void
dhd_conf_wifi_suspend(struct sdio_func *func)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (!sdioinfo[func->num].do_late_resume) {
		dhd_conf_wifi_power(false);
		sdioinfo[func->num].do_late_resume = 1;
	}
#endif
}

void
dhd_conf_register_wifi_suspend(struct sdio_func *func)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (func->num == 2) {
		sdioinfo[func->num].func = func;
		sdioinfo[func->num].do_late_resume = 0;
		sdioinfo[func->num].sdio_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 30;
		sdioinfo[func->num].sdio_early_suspend.suspend = dhd_conf_early_suspend;
		sdioinfo[func->num].sdio_early_suspend.resume = dhd_conf_late_resume;
		register_early_suspend(&sdioinfo[func->num].sdio_early_suspend);
		INIT_WORK(&sdioinfo[func->num].tqueue, dhd_conf_probe_workqueue);
	}
#endif
}

void
dhd_conf_unregister_wifi_suspend(struct sdio_func *func)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (func->num == 2) {
		if (sdioinfo[func->num].sdio_early_suspend.suspend) {
			unregister_early_suspend(&sdioinfo[func->num].sdio_early_suspend);
			sdioinfo[func->num].sdio_early_suspend.suspend = NULL;
		}
	}
#endif
}
#endif

