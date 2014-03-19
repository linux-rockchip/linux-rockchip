#include "camsys_mipicsi_phy.h"


static int camsys_mipiphy_ops (void *phy, void *phyinfo, unsigned int on)
{
    unsigned char val;
    camsys_mipiphy_t *mipiphy = (camsys_mipiphy_t*)phy;
    camsys_phyinfo_t *camsys_phyinfo=(camsys_phyinfo_t*)phyinfo;
    camsys_meminfo_t *reg = camsys_phyinfo->reg;
    unsigned int base = reg->vir_base;
    
    if (on) { 
        val = mipiphy->data_en_bit;
        val <<= 2;
        val |= 0x41;
        __raw_writel(val, base+CSI_REG000);
        __raw_writel(0x88, base+CSI_REG200);
        __raw_writel(0x88, base+CSI_REG300);
        __raw_writel(0x88, base+CSI_REG400);
        //__raw_writel(0xd0, base+CSI_REG012);
        __raw_writel(0x1e, base+CSI_REG100);
        __raw_writel(0x1f, base+CSI_REG100);        
        __raw_writel(0x3f, base+CSI_REG20A);
        udelay(10);  
        camsys_trace(1,"Mipi csi phy turn on  0x%x!",val);
    } else {
        __raw_writeb(0x01, base+CSI_REG000);
        __raw_writeb(0xe3, base+CSI_REG001);
        udelay(10);
        camsys_trace(1,"Mipi csi phy turn off !");
    }

    return 0;
}

static int camsys_mipiphy_clkin_cb(void *ptr, unsigned int on)
{
    camsys_dev_t *camsys_dev = (camsys_dev_t*)ptr;
    camsys_mipiphy_clk_t *clk = (camsys_mipiphy_clk_t*)camsys_dev->mipiphy.clk;
    
    spin_lock(&clk->lock);
    if (on && !clk->in_on) {        
        clk_enable(clk->pd_mipi_csi);
        clk_enable(clk->pclk_mipiphy_csi);
    	
        clk->in_on = true;
        camsys_trace(1, "%s mipi csi phy clock in turn on",dev_name(camsys_dev->miscdev.this_device));
    } else if (!on && clk->in_on) {
        clk_disable(clk->pd_mipi_csi);
    	clk_disable(clk->pclk_mipiphy_csi);
        clk->in_on = false;
        camsys_trace(1, "%s mipi csi phy clock in turn off",dev_name(camsys_dev->miscdev.this_device));
    }
    spin_unlock(&clk->lock);
    return 0;
}

static int camsys_mipiphy_remove_cb(struct platform_device *pdev)
{
    camsys_dev_t *camsys_dev = platform_get_drvdata(pdev);
    camsys_mipiphy_clk_t *clk = (camsys_mipiphy_clk_t*)camsys_dev->mipiphy.clk;

    if (clk) {
        if (clk->in_on) {
            camsys_mipiphy_clkin_cb(camsys_dev,0);
        }
        if (clk->pd_mipi_csi)
            clk_put(clk->pd_mipi_csi);
        if (clk->pclk_mipiphy_csi)
            clk_put(clk->pclk_mipiphy_csi);        
    }
    return 0;
}
int camsys_mipiphy_probe_cb(struct platform_device *pdev, camsys_dev_t *camsys_dev)
{
    int err = 0;
    camsys_meminfo_t *meminfo;
    struct resource *register_res;
    camsys_mipiphy_clk_t *clk;

    clk = kzalloc(sizeof(camsys_mipiphy_clk_t),GFP_KERNEL);
    if (clk == NULL) {
        err = -ENOMEM;
        goto clk_fail;
    }

    clk->pd_mipi_csi = clk_get(NULL, "pd_mipi_csi");
    clk->pclk_mipiphy_csi = clk_get(NULL, "pclk_mipiphy_csi");
    if (IS_ERR_OR_NULL(clk->pd_mipi_csi) || IS_ERR_OR_NULL(clk->pclk_mipiphy_csi)) {
        err = -ENODEV;
        goto clk_fail;
    }    
    spin_lock_init(&clk->lock);
    clk->in_on = false;
    
    register_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, CAMSYS_REGISTER_MIPIPHY_RES_NAME);
    
    meminfo = kzalloc(sizeof(camsys_meminfo_t),GFP_KERNEL);
    if (meminfo == NULL) {
        err = -ENOMEM;
        goto reg_fail;
    }
    
    if (!request_mem_region(register_res->start, register_res->end - register_res->start + 1,
                        dev_name(&pdev->dev))) {
        err = -ENOMEM;
        camsys_err("request_mem_region failed for mipi csi phy\n");
        goto reg_fail;
    }
    
    meminfo->vir_base = (unsigned int)ioremap(register_res->start, register_res->end - register_res->start + 1);
    if (meminfo->vir_base == 0) {
        camsys_err("%s ioremap %s failed",dev_name(&pdev->dev), CAMSYS_REGISTER_MIPIPHY_RES_NAME);
        err = -ENXIO;
        goto reg_fail;
    }

    strlcpy(meminfo->name, CAMSYS_REGISTER_MIPIPHY_RES_NAME,sizeof(meminfo->name));
    meminfo->phy_base = register_res->start;
    meminfo->size = register_res->end - register_res->start + 1; 

    camsys_dev->mipiphy.clk = (void*)clk;
    camsys_dev->mipiphy.reg = meminfo;
    camsys_dev->mipiphy.clkin_cb = camsys_mipiphy_clkin_cb;
    camsys_dev->mipiphy.ops = camsys_mipiphy_ops;
    camsys_dev->mipiphy.remove = camsys_mipiphy_remove_cb;
    list_add_tail(&meminfo->list, &camsys_dev->devmems.memslist);

    return 0;

reg_fail:
    if (meminfo) {
        if (meminfo->vir_base) {
            iounmap((void __iomem *)meminfo->vir_base);
            release_mem_region(meminfo->phy_base,meminfo->size);
        }
        kfree(meminfo);
        meminfo = NULL;
    }
clk_fail:
    if (clk) {
        kfree(clk);
        clk = NULL;
    }
    return err;

}

