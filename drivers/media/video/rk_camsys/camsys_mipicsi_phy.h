#ifndef __CAMSYS_MIPICSI_PHY_H__
#define __CAMSYS_MIPICSI_PHY_H__

#include <plat/camsys_internal.h>

#define CSI_REGBASE                      0x00
#define CSI_REG000                       (CSI_REGBASE+0x00)
#define CSI_REG001                       (CSI_REGBASE+0x04)
#define CSI_REG003                       (CSI_REGBASE+0x0c)
#define CSI_REG004                       (CSI_REGBASE+0x10)

#define CSI_REG012                       (CSI_REGBASE+0x12*4)

#define CSI_REG100                       (CSI_REGBASE+0x20*4)
//Clock lane
#define CSI_REG200                       (CSI_REGBASE+0x40*4)
#define CSI_REG205                       (CSI_REGBASE+0x45*4)
#define CSI_REG206                       (CSI_REGBASE+0x46*4)
#define CSI_REG207                       (CSI_REGBASE+0x47*4)
#define CSI_REG208                       (CSI_REGBASE+0x48*4)
#define CSI_REG209                       (CSI_REGBASE+0x49*4)
#define CSI_REG20A                       (CSI_REGBASE+0x4A*4)
#define CSI_REG20B                       (CSI_REGBASE+0x4B*4)
#define CSI_REG20C                       (CSI_REGBASE+0x4C*4)
#define CSI_REG20D                       (CSI_REGBASE+0x4D*4)
#define CSI_REG20E                       (CSI_REGBASE+0x4E*4)
#define CSI_REG210                       (CSI_REGBASE+0x50*4)
#define CSI_REG211                       (CSI_REGBASE+0x51*4)
#define CSI_REG212                       (CSI_REGBASE+0x52*4)
//Data0 lane
#define CSI_REG300                       (CSI_REGBASE+0x60*4)
#define CSI_REG305                       (CSI_REGBASE+0x65*4)
#define CSI_REG306                       (CSI_REGBASE+0x66*4)
#define CSI_REG307                       (CSI_REGBASE+0x67*4)
#define CSI_REG308                       (CSI_REGBASE+0x68*4)
#define CSI_REG309                       (CSI_REGBASE+0x69*4)
#define CSI_REG30A                       (CSI_REGBASE+0x6A*4)
#define CSI_REG30B                       (CSI_REGBASE+0x6B*4)
#define CSI_REG30C                       (CSI_REGBASE+0x6C*4)
#define CSI_REG30D                       (CSI_REGBASE+0x6D*4)
#define CSI_REG30E                       (CSI_REGBASE+0x6E*4)
#define CSI_REG310                       (CSI_REGBASE+0x70*4)
#define CSI_REG311                       (CSI_REGBASE+0x71*4)
#define CSI_REG312                       (CSI_REGBASE+0x72*4)
//Data1 lane
#define CSI_REG400                       (CSI_REGBASE+0x80*4)
#define CSI_REG405                       (CSI_REGBASE+0x85*4)
#define CSI_REG406                       (CSI_REGBASE+0x86*4)
#define CSI_REG407                       (CSI_REGBASE+0x87*4)
#define CSI_REG408                       (CSI_REGBASE+0x88*4)
#define CSI_REG409                       (CSI_REGBASE+0x89*4)
#define CSI_REG40A                       (CSI_REGBASE+0x8A*4)
#define CSI_REG40B                       (CSI_REGBASE+0x8B*4)
#define CSI_REG40C                       (CSI_REGBASE+0x8C*4)
#define CSI_REG40D                       (CSI_REGBASE+0x8D*4)
#define CSI_REG40E                       (CSI_REGBASE+0x8E*4)
#define CSI_REG410                       (CSI_REGBASE+0x90*4)
#define CSI_REG411                       (CSI_REGBASE+0x91*4)
#define CSI_REG412                       (CSI_REGBASE+0x92*4)
//Data2 lane
#define CSI_REG500                       (CSI_REGBASE+0xa0*4)
#define CSI_REG505                       (CSI_REGBASE+0xa5*4)
#define CSI_REG506                       (CSI_REGBASE+0xa6*4)
#define CSI_REG507                       (CSI_REGBASE+0xa7*4)
#define CSI_REG508                       (CSI_REGBASE+0xa8*4)
#define CSI_REG509                       (CSI_REGBASE+0xa9*4)
#define CSI_REG50A                       (CSI_REGBASE+0xaA*4)
#define CSI_REG50B                       (CSI_REGBASE+0xaB*4)
#define CSI_REG50C                       (CSI_REGBASE+0xaC*4)
#define CSI_REG50D                       (CSI_REGBASE+0xaD*4)
#define CSI_REG50E                       (CSI_REGBASE+0xaE*4)
#define CSI_REG510                       (CSI_REGBASE+0xb0*4)
#define CSI_REG511                       (CSI_REGBASE+0xb1*4)
#define CSI_REG512                       (CSI_REGBASE+0xb2*4)
//Data3 lane
#define CSI_REG600                       (CSI_REGBASE+0xc0*4)
#define CSI_REG605                       (CSI_REGBASE+0xc5*4)
#define CSI_REG606                       (CSI_REGBASE+0xc6*4)
#define CSI_REG607                       (CSI_REGBASE+0xc7*4)
#define CSI_REG608                       (CSI_REGBASE+0xc8*4)
#define CSI_REG609                       (CSI_REGBASE+0xc9*4)
#define CSI_REG60A                       (CSI_REGBASE+0xcA*4)
#define CSI_REG60B                       (CSI_REGBASE+0xcB*4)
#define CSI_REG60C                       (CSI_REGBASE+0xcC*4)
#define CSI_REG60D                       (CSI_REGBASE+0xcD*4)
#define CSI_REG60E                       (CSI_REGBASE+0xcE*4)
#define CSI_REG610                       (CSI_REGBASE+0xd0*4)
#define CSI_REG611                       (CSI_REGBASE+0xd1*4)
#define CSI_REG612                       (CSI_REGBASE+0xd2*4)


typedef struct camsys_mipiphy_clk_s {
    struct clk       *pd_mipi_csi;
    struct clk       *pclk_mipiphy_csi;
    bool             in_on;
    spinlock_t       lock;
} camsys_mipiphy_clk_t;


int camsys_mipiphy_probe_cb(struct platform_device *pdev, camsys_dev_t *camsys_dev);

#endif
