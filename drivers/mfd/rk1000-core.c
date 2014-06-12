#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/mfd/core.h>
#include <linux/mfd/rk1000.h>
#include <linux/of.h>
#include <linux/of_gpio.h>

#if 1
#define	DBG(x...)	printk(KERN_INFO x)
#else
#define	DBG(x...)
#endif

#define CTRL_ADC				0x00
#define CTRL_CODEC				0x01
#define CTRL_I2C				0x02
#define CTRL_TVE				0x03
#define RGB2CCIR_RESET			0x04
#define ADC_START				0x05

struct rk1000 {
	struct i2c_client *client;
    struct device *dev;
    struct dentry *debugfs_dir;
	
	struct ioctrl io_power;
	struct ioctrl io_reset;
};

static struct rk1000 *rk1000;

void rk1000_power_ctrl(int enable) {
	if(rk1000 && gpio_is_valid(rk1000->io_power.gpio)) {
		if(enable) {
			gpio_set_value(rk1000->io_power.gpio, rk1000->io_power.active);
			msleep(10);
			// after power up, rk1000 need to reset
			if (gpio_is_valid(rk1000->io_power.gpio)) {
				gpio_set_value(rk1000->io_reset.gpio, rk1000->io_reset.active);
				msleep(100);
				gpio_set_value(rk1000->io_reset.gpio, !(rk1000->io_reset.active));
			}
		}
		else
			gpio_set_value(rk1000->io_power.gpio, !(rk1000->io_power.active));
	}
}

int rk1000_i2c_send(const u8 addr, const u8 reg, const u8 value)
{
	struct i2c_adapter *adap;
	struct i2c_msg msg;
	int ret;
	char buf[2];
	
	if(rk1000 == NULL || rk1000->client == NULL){
		printk("rk1000 not init!\n");
		return -1;
	}
	
	adap = rk1000->client->adapter;
	
	buf[0] = reg;
	buf[1] = value;
	
	msg.addr = addr;
	msg.flags = rk1000->client->flags;
	msg.len = 2;
	msg.buf = buf;
	msg.scl_rate = RK1000_I2C_RATE;

	ret = i2c_transfer(adap, &msg, 1);
	if (ret != 1) {
            printk("rk1000 control i2c write err,ret =%d\n",ret);
            return -1;
    }
    else
    	return 0;
}

int rk1000_i2c_recv(const u8 addr, const u8 reg, const char *buf)
{
	struct i2c_adapter *adap;
	struct i2c_msg msgs[2];
	int ret;
	
	if(rk1000 == NULL || rk1000->client == NULL){
		printk("rk1000 not init!\n");
		return -1;
	}
	adap = rk1000->client->adapter;
	msgs[0].addr = addr;
	msgs[0].flags = rk1000->client->flags;
	msgs[0].len = 1;
	msgs[0].buf = (char*)(&reg);
	msgs[0].scl_rate = RK1000_I2C_RATE;

	msgs[1].addr = addr;
	msgs[1].flags = rk1000->client->flags | I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = (char*)buf;
	msgs[1].scl_rate = RK1000_I2C_RATE;

	ret = i2c_transfer(adap, msgs, 2);

	return (ret == 2)? 0 : -1;
}

static int rk1000_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
    struct device_node *rk1000_np;
    enum of_gpio_flags flags;
    struct clk *i2s_clk,*i2s_mclk;
    int ret;
    
    DBG("[%s] start\n", __FUNCTION__);
    rk1000 = kmalloc(sizeof(struct rk1000), GFP_KERNEL);
    if(!rk1000) {
        dev_err(&client->dev, ">> rk1000 core inf kmalloc fail!");
        return -ENOMEM;
    }
    memset(rk1000, 0, sizeof(struct rk1000));
    
    rk1000->client = client;
    rk1000->dev = &client->dev;
    
    rk1000_np = rk1000->dev->of_node;
    
    /********Get reset pin***********/
	rk1000->io_reset.gpio = of_get_named_gpio_flags(rk1000_np,"gpio-reset", 0, &flags);
    if (!gpio_is_valid(rk1000->io_reset.gpio)) {
        printk("invalid rk1000->io_reset.gpio: %d\n", rk1000->io_reset.gpio);
        ret = -1;
        goto err;
    }
    ret = gpio_request(rk1000->io_reset.gpio, "rk1000-reset-io");
    if( ret != 0){
        printk("gpio_request rk1000->io_reset.gpio invalid: %d\n",rk1000->io_reset.gpio);
        goto err;
    }
    rk1000->io_reset.active = !(flags & OF_GPIO_ACTIVE_LOW);
    gpio_direction_output(rk1000->io_reset.gpio, !(rk1000->io_reset.active));
    msleep(10);
    
    /********Get power pin***********/
    rk1000->io_power.gpio = of_get_named_gpio_flags(rk1000_np,"gpio-power", 0, &flags);
    if (gpio_is_valid(rk1000->io_power.gpio)) {
        ret = gpio_request(rk1000->io_power.gpio, "rk1000-power-io");
        if( ret != 0){
            printk("gpio_request rk1000->io_power.gpio invalid: %d\n",rk1000->io_power.gpio);
            goto err;
        }
        rk1000->io_power.active = !(flags & OF_GPIO_ACTIVE_LOW);
        gpio_direction_output(rk1000->io_power.gpio, rk1000->io_power.active);
    }
    
    /********rk1000 reset***********/
    msleep(100);
    gpio_set_value(rk1000->io_reset.gpio, rk1000->io_reset.active);
    msleep(100);
    gpio_set_value(rk1000->io_reset.gpio, !(rk1000->io_reset.active));
    
    // rk1000 is drived by i2s_mclk, we enable i2s_clk first.
    i2s_clk= clk_get(&client->dev, "i2s_clk");
    if (IS_ERR(i2s_clk)) {
        dev_err(&client->dev, "Can't retrieve i2s clock\n");
        ret = PTR_ERR(i2s_clk);
        goto err;
    }else{
        printk("rk1000 get i2s clk success!\n");   
    }
    
    clk_set_rate(i2s_clk, 12288000);	
    clk_set_rate(i2s_clk, 11289600);
    clk_prepare_enable(i2s_clk);
    printk("rk1000 enable i2s clk\n");
    
    i2s_mclk= clk_get(&client->dev, "i2s_mclk");
    if (IS_ERR(i2s_mclk)) {
        dev_err(&client->dev, "Can't retrieve i2s mclock\n");
    }else{
        clk_set_rate(i2s_mclk, 12288000);
        clk_set_rate(i2s_mclk, 11289600);
        clk_prepare_enable(i2s_mclk);
        printk("rk1000 enable i2s mclk\n");
    }

    rk1000_i2c_send(I2C_ADDR_CTRL, CTRL_ADC, 0x88); //ADC power off
    #ifdef CONFIG_SND_SOC_RK1000
    rk1000_i2c_send(I2C_ADDR_CTRL, CTRL_CODEC, 0x00);
    #else
    rk1000_i2c_send(I2C_ADDR_CTRL, CTRL_CODEC, 0x0d);
    #endif
    rk1000_i2c_send(I2C_ADDR_CTRL, CTRL_I2C, 0x22);
    rk1000_i2c_send(I2C_ADDR_CTRL, CTRL_TVE, 0x00);
    
    printk("rk1000 probe ok\n");
    
    return 0;
    
err:
    if(rk1000) {
        kfree(rk1000);
        rk1000 = NULL;
    }
    return ret;
}

static int rk1000_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id rk1000_id[] = {
	{ "rk1000_control", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rk1000_id);

static struct i2c_driver rk1000_driver = {
	.driver = {
		.name = "rk1000_control",
	},
	.probe = rk1000_probe,
	.remove = rk1000_remove,
	.id_table = rk1000_id,
};

static int __init rk1000_init(void)
{
	return i2c_add_driver(&rk1000_driver);
}

static void __exit rk1000_exit(void)
{
	i2c_del_driver(&rk1000_driver);
}

subsys_initcall_sync(rk1000_init);
module_exit(rk1000_exit);


MODULE_DESCRIPTION("RK1000 control driver");
MODULE_AUTHOR("Rock-chips, <www.rock-chips.com>");
MODULE_LICENSE("GPL");

