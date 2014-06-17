#rtl8192c packed Makefile
# (gwl)

rk_cfg := wifi_power.o wifi_power_usb.o wifi_power_ops.o

rkcfg-y += $(rk_cfg)

obj-$(CONFIG_RTL8188EU) += rkcfg.o rtl8188eu/
obj-$(CONFIG_RTL8192CU) += rkcfg.o rtl8192cu/
obj-$(CONFIG_RTL8723AU) += rkcfg.o rtl8723au/
obj-$(CONFIG_RTL8723BU) += rkcfg.o rtl8723bu/
obj-$(CONFIG_RTL8192DU) += rkcfg.o rtl8192du/
obj-$(CONFIG_RT5370) += rkcfg.o
obj-$(CONFIG_MT7601) += rkcfg.o
.PHONY: clean

clean:
	find . -name '*.o*' -exec rm -f {} \; 
