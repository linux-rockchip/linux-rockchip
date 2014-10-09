 /****************************************************************************************
 * File:			drivers/misc/hsic_hub_reset.c
 * Copyright:		Copyright (C) 2014-2015 RK Corporation.
 * Author:			LiBing <libing@rock-chips.com>
 * Date:			2014.10.09
 * Description:		This driver use for rk32 chip usb hub. Use i2c IF ,the chip is 
 * 				    hisc usb4604.
 *****************************************************************************************/

#include <dt-bindings/gpio/gpio.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/slab.h>
#include <linux/delay.h>

static struct of_device_id hub_reset_of_match[] = {
	{ .compatible = "hub_reset" },
	{ }
};

MODULE_DEVICE_TABLE(of, hub_reset_of_match);


static int hub_reset_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	enum of_gpio_flags flags;
	int gpio, ret,en_value;

	if (!node)
		return -ENODEV;

	gpio = of_get_named_gpio_flags(node, "reset,pin", 0, &flags);
	en_value = (flags == GPIO_ACTIVE_HIGH)? 1:0;
	//gpio =  of_get_named_gpio(node, "gpios", 0);
	if(!gpio_is_valid(gpio)){
		dev_err(&pdev->dev, "invalid hub rst gpio%d\n", gpio);
	}

	ret = devm_gpio_request(&pdev->dev, gpio, "hub_rst_gpio");
	if (ret) {
		dev_err(&pdev->dev, "failed to request GPIO%d for ohub_rst_drv\n", gpio);
		return -EINVAL;
	}
	gpio_direction_output(gpio, 1);
	msleep(10);
	gpio_direction_output(gpio, 0);
	msleep(10);
	gpio_direction_output(gpio, 1);
	msleep(1);
	return 0;
}

static int hub_reset_remove(struct platform_device *pdev)
{
	//printk("func: %s\n", __func__); 
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int hub_reset_suspend(struct device *dev)
{
	//printk("func: %s\n", __func__); 
	return 0;
}

static int hub_reset_resume(struct device *dev)
{
	//printk("func: %s\n", __func__); 
	return 0;
}
#endif

static const struct dev_pm_ops hub_reset_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend	= hub_reset_suspend,
	.resume		= hub_reset_resume,
	.poweroff	= hub_reset_suspend,
	.restore	= hub_reset_resume,
#endif
};

static struct platform_driver hub_reset_driver = {
	.driver		= {
		.name		= "_hsic_hub_reset",
		.owner		= THIS_MODULE,
		.pm			= &hub_reset_pm_ops,
		.of_match_table	= of_match_ptr(hub_reset_of_match),
	},
	.probe		= hub_reset_probe,
	.remove		= hub_reset_remove,
};

module_platform_driver(hub_reset_driver);

MODULE_DESCRIPTION("hsic usb hub reset");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LingBing<libing@rock-chips.com>");
MODULE_ALIAS("platform: hsic usb hub");
