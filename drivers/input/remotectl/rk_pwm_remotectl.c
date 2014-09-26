#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>
#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/iio/consumer.h>

#include "rk_pwm_remotectl.h"


// sys/module/rk_pwm_remotectl/parameters,modify code_print to change the value
static int rk_remote_print_code = 0;
module_param_named(code_print, rk_remote_print_code, int, 0644);
#define DBG_CODE( args...) \
	do { \
		if (rk_remote_print_code) { \
			pr_info(args); \
		} \
	} while (0)

static int rk_remote_pwm_dbg_level = 0;
module_param_named(dbg_level, rk_remote_pwm_dbg_level, int, 0644);
#define DBG( args...) \
	do { \
		if (rk_remote_pwm_dbg_level) { \
			pr_info(args); \
		} \
	} while (0)


struct rkxx_remote_key_table{
    int scanCode;
	int keyCode;		
};

struct rkxx_remotectl_button {	
    int usercode;
    int nbuttons;
    struct rkxx_remote_key_table *key_table;
};

struct rkxx_remotectl_drvdata {
    void __iomem *base;
    int state;
	int nbuttons;
	int result;
    int scanData;
    int count;
    int keybdNum;
    int keycode;
    int press;
    int pre_press;
    int period;
    int irq;
    int wakeup;
    struct input_dev *input;
    struct timer_list timer;
    struct tasklet_struct remote_tasklet;
    struct wake_lock remotectl_wake_lock;
};

//特殊功能键值定义
    //key_reply      //确定
    //193            //photo
    //194            //video
    //195            //music
    //196            //IE
    //197            //
    //198
    //199
    //200
    
    //183      //rorate_left
    //184      //rorate_right
    //185      //zoom out
    //186      //zoom in
    
static struct rkxx_remote_key_table remote_key_table_meiyu_4040[] = {
    {0xf2, KEY_REPLY},//ok = DPAD CENTER
    {0xba, KEY_BACK}, 
    {0xf4, KEY_UP},
    {0xf1, KEY_DOWN},
    {0xef, KEY_LEFT},
    {0xee, KEY_RIGHT},  ////////
    {0xbd, KEY_HOME},     //home
    {0xea, KEY_VOLUMEUP},
    {0xe3, KEY_VOLUMEDOWN},
    {0xe2, KEY_SEARCH},     //search  //0
    {0xb2, KEY_POWER},     //power off
    {0xbc, KEY_MUTE},       //mute
    {0xec, KEY_MENU},

//media ctrl
    {0xbf,   0x190},      //play pause
    {0xe0,   0x191},      //pre
    {0xe1,   0x192},      //next

//pic
    {0xe9, 183},          //rorate left   //5
    {0xe6, 248},          //rorate right  //8
    {0xe8, 185},          //zoom out      //6
    {0xe7, 186},          //zoom in       //7
//mouse switch
    {0xf0,388},           //1
//display switch
    {0xbe,   0x175},
};


static struct rkxx_remote_key_table remote_key_table_sunchip_ff00[] = {

    {0xf9, KEY_HOME},     // home
    {0xbf, KEY_BACK}, 		// back
    {0xfb, KEY_MENU},			// menu
    {0xaa, KEY_REPLY},		// ok
    {0xb9, KEY_UP},	
    {0xe9, KEY_DOWN},
    {0xb8, KEY_LEFT},
    {0xea, KEY_RIGHT},
    {0xeb, KEY_VOLUMEDOWN},
    {0xef, KEY_VOLUMEUP},
    {0xf7, KEY_MUTE},       //mute
    {0xe7, KEY_POWER},     //power (RED)
    {0xfc, KEY_POWER},     //power (GREEN)
    {0xa9, KEY_VOLUMEDOWN},	// function RED
    {0xa8, KEY_VOLUMEDOWN},	// function GREEN
    {0xe0, KEY_VOLUMEDOWN},	// function YELLOW
    {0xa5, KEY_VOLUMEDOWN},	// function BLUE   
    {0xab, 183},	// Digit 1
    {0xb7, 388},	// Digit 2
    {0xf8, 184},	// Digit 3
    {0xaf, 185},	// Digit 4
    {0xed, KEY_VOLUMEDOWN},	// Digit 5
    {0xee, 186},	// Digit 6
    {0xb3, KEY_VOLUMEDOWN},	// Digit 7
    {0xf1, KEY_VOLUMEDOWN},	// Digit 8
    {0xf2, KEY_VOLUMEDOWN},	// Digit 9
    {0xf3, KEY_SEARCH},	// Digit 0
    {0xb4, KEY_VOLUMEDOWN},	// Delete
    {0xbe, KEY_SEARCH},     // search
};


static struct rkxx_remotectl_button remotectl_button[] = 
{
    {  
       .usercode = 0xff00,
       .nbuttons =  29, 
       .key_table = &remote_key_table_sunchip_ff00[0],
    },
    {  
       .usercode = 0x4040, 
       .nbuttons =  22, 
       .key_table = &remote_key_table_meiyu_4040[0],
    },
};


static int remotectl_keybdNum_lookup(struct rkxx_remotectl_drvdata *ddata)
{	
    int i;	

    for (i = 0; i < sizeof(remotectl_button)/sizeof(struct rkxx_remotectl_button); i++){		
        if (remotectl_button[i].usercode == (ddata->scanData&0xFFFF)){			
            ddata->keybdNum = i;
            return 1;
        }
    }
    return 0;
}


static int remotectl_keycode_lookup(struct rkxx_remotectl_drvdata *ddata)
{	
    int i;	
    unsigned char keyData = ((ddata->scanData >> 8) & 0xff);

    for (i = 0; i < remotectl_button[ddata->keybdNum].nbuttons; i++){
        if (remotectl_button[ddata->keybdNum].key_table[i].scanCode == keyData){			
            ddata->keycode = remotectl_button[ddata->keybdNum].key_table[i].keyCode;
            return 1;
        }
    }
    return 0;
}


static void rk_pwm_remotectl_do_something(unsigned long  data)
{
    struct rkxx_remotectl_drvdata *ddata = (struct rkxx_remotectl_drvdata *)data;

    switch (ddata->state)
    {
        case RMC_IDLE:
        {
            ;
        }
        break;
        
        case RMC_PRELOAD:
        {
            mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(130));
            if ((RK_PWM_TIME_PRE_MIN < ddata->period) && (ddata->period < RK_PWM_TIME_PRE_MAX)){
                ddata->scanData = 0;
                ddata->count = 0;
                ddata->state = RMC_USERCODE;
            }else{
                ddata->state = RMC_PRELOAD;
            }
        }
        break;
        
        case RMC_USERCODE:
        {
            //ddata->scanData <<= 1;
            //ddata->count ++;
            if ((RK_PWM_TIME_BIT1_MIN < ddata->period) && (ddata->period < RK_PWM_TIME_BIT1_MAX)){
                ddata->scanData |= (0x01<<ddata->count);;
            }
            ddata->count ++;
            if (ddata->count == 0x10){//16 bit user code
                DBG_CODE("GET USERCODE=0x%x\n",((ddata->scanData) & 0xffff));
                if (remotectl_keybdNum_lookup(ddata)){
                    ddata->state = RMC_GETDATA;
                    ddata->scanData = 0;
                    ddata->count = 0;
                }else{                //user code error
                    ddata->state = RMC_PRELOAD;
                }
            }
        }
        break;
        
        case RMC_GETDATA:
        {
            //ddata->count ++;
            //ddata->scanData <<= 1;
            
            if ((RK_PWM_TIME_BIT1_MIN < ddata->period) && (ddata->period < RK_PWM_TIME_BIT1_MAX)){
                ddata->scanData |= (0x01<<ddata->count);
            }   
            ddata->count ++;      
            if (ddata->count == 0x10){
                DBG_CODE("RMC_GETDATA=%x\n",(ddata->scanData>>8));
                if ((ddata->scanData&0x0ff) == ((~ddata->scanData >> 8)&0x0ff)){
                    if (remotectl_keycode_lookup(ddata)){
                        ddata->press = 1;
                        input_event(ddata->input, EV_KEY, ddata->keycode, 1);
                        input_sync(ddata->input);
                        ddata->state = RMC_SEQUENCE;
                    }else{
                        ddata->state = RMC_PRELOAD;
                    }
                }else{
                    ddata->state = RMC_PRELOAD;
                }
            }
        }
        break;
             
        case RMC_SEQUENCE:{
            DBG( "S=%d\n",ddata->period);
            if ((RK_PWM_TIME_RPT_MIN < ddata->period) && (ddata->period < RK_PWM_TIME_RPT_MAX)){
                 DBG( "S1\n");
                mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(130));
            }else if ((RK_PWM_TIME_SEQ1_MIN < ddata->period) && (ddata->period < RK_PWM_TIME_SEQ1_MAX)){
                DBG( "S2\n");
                mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(130));
            }else if ((RK_PWM_TIME_SEQ2_MIN < ddata->period) && (ddata->period < RK_PWM_TIME_SEQ2_MAX)){
                DBG( "S3\n");
                mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(130));
            }else{
                DBG( "S4\n");
                input_event(ddata->input, EV_KEY, ddata->keycode, 0);
                input_sync(ddata->input);
                ddata->state = RMC_PRELOAD;
                ddata->press = 0;
            }
        }
        break;
       
        default:
            break;
    } 
	return;
}

static void rk_pwm_remotectl_timer(unsigned long _data)
{
    struct rkxx_remotectl_drvdata *ddata =  (struct rkxx_remotectl_drvdata*)_data;
    //int val;
    
    if(ddata->press != ddata->pre_press) {
        ddata->pre_press = ddata->press = 0;
        
        input_event(ddata->input, EV_KEY, ddata->keycode, 0);
        input_sync(ddata->input);
    }
    ddata->state = RMC_PRELOAD;
}


static irqreturn_t rockchip_pwm_irq(int irq, void *dev_id)
{
    struct rkxx_remotectl_drvdata *ddata =  (struct rkxx_remotectl_drvdata*)dev_id;
    int val;
    
    val = readl_relaxed(ddata->base + PWM_REG_INTSTS);
    if (val&PWM_CH0_INT){
        if ((val&PWM_CH0_POL)==0){
            val = readl_relaxed(ddata->base + PWM_REG_HPR);
            ddata->period = val;
            tasklet_hi_schedule(&ddata->remote_tasklet); 
            DBG("hpr=0x%x\n",val);
        }else{
            val = readl_relaxed(ddata->base + PWM_REG_LPR);
            DBG("lpr=0x%x\n",val);
        }
        writel_relaxed(PWM_CH0_INT, ddata->base + PWM_REG_INTSTS);  
    
        if (ddata->state==RMC_PRELOAD){
            wake_lock_timeout(&ddata->remotectl_wake_lock, HZ);
        }
        return IRQ_HANDLED;
    }
    return IRQ_NONE; 
}

static int rk_pwm_remotectl_hw_init(struct rkxx_remotectl_drvdata *ddata)
{
    //int ret;
	int val;
	
	//printk("rk_pwm_remotectl_hw_init,base=0x%x \n",ddata->base);
	
    val = readl_relaxed(ddata->base + PWM_REG_CTRL);
    val = (val & 0xFFFFFFFE) | PWM_DISABLE;
    writel_relaxed(val, ddata->base + PWM_REG_CTRL);
    
    val = readl_relaxed(ddata->base + PWM_REG_CTRL);
    val = (val & 0xFFFFFFF9) | PWM_MODE_CAPTURE;
    writel_relaxed(val, ddata->base + PWM_REG_CTRL);
    
    val = readl_relaxed(ddata->base + PWM_REG_CTRL);
    val = (val & 0xFF008DFF) | 0x00646200;
    writel_relaxed(val, ddata->base + PWM_REG_CTRL);
    
    val = readl_relaxed(ddata->base + PWM_REG_INT_EN);
    val = (val & 0xFFFFFFFE) | 1;
    writel_relaxed(val, ddata->base + PWM_REG_INT_EN);

    val = readl_relaxed(ddata->base + PWM_REG_CTRL);
    val = (val & 0xFFFFFFFE) | PWM_ENABLE;
    printk("pwm enable val=0x%x \n",val);
    writel_relaxed(val, ddata->base + PWM_REG_CTRL);
    return 0;
}


static int rk_pwm_probe(struct platform_device *pdev)
{
    //struct device_node *np = pdev->dev.of_node;
    //struct device *dev = &pdev->dev;
    struct rkxx_remotectl_drvdata *ddata;
    struct resource *r;
    struct input_dev *input;
    struct clk *clk;
    struct cpumask cpumask;
    int cpu;
    int irq;
    int ret;
    //int val;
    int i,j;
    
    printk(".. rk pwm remotectl v1.0 init\n");
    
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!r) {
        dev_err(&pdev->dev, "no memory resources defined\n");
        return -ENODEV;
    }
    ddata = kzalloc(sizeof(struct rkxx_remotectl_drvdata),GFP_KERNEL);
    if (!ddata) {
        dev_err(&pdev->dev, "failed to allocate memory\n");
        return -ENOMEM;
    }
    memset(ddata,0,sizeof(struct rkxx_remotectl_drvdata));
    ddata->state = RMC_PRELOAD;
    ddata->base = devm_ioremap_resource(&pdev->dev, r);

    if (IS_ERR(ddata->base))
        return PTR_ERR(ddata->base);
    
    //pc->clk = devm_clk_get(&pdev->dev, NULL);
    clk = devm_clk_get(&pdev->dev,"pclk_pwm");
    
    if (IS_ERR(clk))
        return PTR_ERR(clk);
    
    platform_set_drvdata(pdev, ddata);
    
    input = input_allocate_device();
    input->name = pdev->name;
    input->phys = "gpio-keys/input11";
    input->dev.parent = &pdev->dev;
    
    input->id.bustype = BUS_HOST;
    input->id.vendor = 0x0001;
    input->id.product = 0x0001;
    input->id.version = 0x0100;
    ddata->input = input;
    
    ddata->input = input;
    ret = clk_prepare(clk);
    if (ret)
        return ret;
    
    wake_lock_init(&ddata->remotectl_wake_lock, WAKE_LOCK_SUSPEND, "rk29_pwm_remote");
    //	if (of_device_is_compatible(np, "rockchip,pwm")) {
    ret = clk_enable(clk);
    if (ret) {
        clk_unprepare(clk);
        return ret;
    }
    
    irq = ret = platform_get_irq(pdev, 0);
    if (ret < 0) {
        dev_err(&pdev->dev, "cannot find IRQ\n");
        return ret;
    }
    ddata->irq = irq;
    ddata->wakeup = 1;

    ret = devm_request_irq(&pdev->dev, irq, rockchip_pwm_irq, 0, "rk_pwm_irq", ddata);
    if (ret) {
        dev_err(&pdev->dev, "cannot claim IRQ %d\n", irq);
        return ret;
    }
   
    enable_irq_wake(irq); 
    setup_timer(&ddata->timer,rk_pwm_remotectl_timer, (unsigned long)ddata);
    mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(1000));
    tasklet_init(&ddata->remote_tasklet, rk_pwm_remotectl_do_something, (unsigned long)ddata);
    
    for (j=0;j<sizeof(remotectl_button)/sizeof(struct rkxx_remotectl_button);j++){ 
        printk("remotectl probe j=0x%x\n",j);
        for (i = 0; i < remotectl_button[j].nbuttons; i++) {
            unsigned int type = EV_KEY;
    
            input_set_capability(input, type, remotectl_button[j].key_table[i].keyCode);
        }
    }
    ret = input_register_device(input);
    if (ret) {
        pr_err("rk pwm remotectl: Unable to register input device, ret: %d\n", ret);
    }	
    
    	input_set_capability(input, EV_KEY, KEY_WAKEUP);
    
    	device_init_wakeup(&pdev->dev, 1);
    
    	#ifdef RK_PWM_REMOTECTL_PROC	
    	// rk_pwm_remotectl_proc_init();
    	#endif
    	cpu = 2;
    	cpumask_clear(&cpumask);
	cpumask_set_cpu(cpu, &cpumask); 
	irq_set_affinity(irq, &cpumask); 
    	rk_pwm_remotectl_hw_init(ddata);
   	DBG("%s end \n",__FUNCTION__);
    
    	return ret;
}

static int rk_pwm_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM
static int remotectl_suspend(struct device *dev)
{
	int cpu = 0;
	struct cpumask cpumask;
	struct platform_device *pdev = to_platform_device(dev);
  	struct rkxx_remotectl_drvdata *ddata = platform_get_drvdata(pdev);
  	
	cpumask_clear(&cpumask);
	cpumask_set_cpu(cpu, &cpumask); 
	irq_set_affinity(ddata->irq, &cpumask); 
	return 0;
}


static int remotectl_resume(struct device *dev)
{
	int cpu = 2;
	struct cpumask cpumask;
	struct platform_device *pdev = to_platform_device(dev);
  	struct rkxx_remotectl_drvdata *ddata = platform_get_drvdata(pdev);
  	
	cpumask_clear(&cpumask);
	cpumask_set_cpu(cpu, &cpumask); 
	irq_set_affinity(ddata->irq, &cpumask); 
	return 0;
}

static const struct dev_pm_ops remotectl_pm_ops = {
	.suspend = remotectl_suspend,
	.resume	= remotectl_resume,
};
#endif


static const struct of_device_id rk_pwm_of_match[] = {
	{ .compatible =  "rockchip,rk-pwm0"},
	{ }
};

MODULE_DEVICE_TABLE(of, rk_pwm_of_match);

static struct platform_driver rk_pwm_driver = {
	.driver = {
        	.name = "rk-pwm0",
        	.of_match_table = rk_pwm_of_match,
    		#ifdef CONFIG_PM
        	.pm	= &remotectl_pm_ops,
    		#endif	
    	},
	.probe = rk_pwm_probe,
    	.remove = rk_pwm_remove,
};

module_platform_driver(rk_pwm_driver);

MODULE_LICENSE("GPL");
