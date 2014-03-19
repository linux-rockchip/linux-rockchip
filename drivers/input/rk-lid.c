


#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/adc.h>
#include <linux/wakelock.h>

#include <asm/gpio.h>
#include <mach/board.h>
#include <plat/rk-lid.h>

#include <linux/suspend.h>
#if 0
#define lidprintk(x...) printk(x)
#else
#define lidprintk(x...)
#endif
static irqreturn_t lid_interrupt_handler(int irq, void *dev_id);

const char* lidstates [] = {"unknow","close","open"};
char* lidstate = "unknow";
static struct wake_lock lidaction_wake_lock;

struct rk_lid_drvdata {
	int in_suspend;	/* Flag to indicate if we're suspending/resuming */
	int irq;
	struct input_dev *input;
	struct rk_lid_platform_data *rk_lid_pdata ;
	struct delayed_work  	lid_delay_work;
	struct workqueue_struct *lid_wq;
};

extern void rk29_send_power_key(int state);
extern suspend_state_t get_suspend_state(void);
static void lid_report(struct work_struct *work)
{
	struct delayed_work *delay_work;
	struct rk_lid_drvdata *lid_data;
	struct rk_lid_platform_data *rk_lid_pdata ;
	int value = 0;
	static int old_value = 1;
	int err;
	
	delay_work=container_of(work, struct delayed_work,work);
	lid_data = container_of(delay_work, struct rk_lid_drvdata, lid_delay_work);
	rk_lid_pdata = lid_data->rk_lid_pdata;

	if (lid_data->input == NULL){
	        printk("[%s]LID input device doesn't exist\n",__FUNCTION__);
	        return;
	}

	wake_unlock(&lidaction_wake_lock);
	
	value = gpio_get_value(rk_lid_pdata->lid_gpio);
	if(old_value != value){
		lidprintk("[lid_report][line %d]value = %d,old_value=%d\n", __LINE__,value,old_value);
		if(value == rk_lid_pdata->lid_close_value){
			lidstate = lidstates[1];
			lidprintk("lid: computer close *****************\n");
			
		        //input_report_switch(lid_data->input, KEY_LID, 0);
		        if(get_suspend_state()==0){
				lidprintk("lid: report poweron to sheep\n");
				rk29_send_power_key(1);
				rk29_send_power_key(0);
			}
		} else {
			lidstate = lidstates[2];
			lidprintk("lid: computer opened *****************\n");
		        //input_report_switch(lid_data->input, KEY_LID, 1);
		        if(get_suspend_state()!=0){
				lidprintk("lid: report poweron to wakeup\n");
				rk29_send_power_key(1);
				rk29_send_power_key(0);
			}
		}
		input_sync(lid_data->input);
		
		free_irq(lid_data->irq,lid_data);
		lid_data->irq = gpio_to_irq(rk_lid_pdata->lid_gpio);
		err = request_irq(lid_data->irq, lid_interrupt_handler,value?IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH, "rk_lid", lid_data);
		if(unlikely(err)){
			printk("[LID-ERROR]%s,,request irq error,err=%d\n",__FUNCTION__,err);
			return ;
		}
		old_value = value;
		enable_irq_wake(lid_data->irq);
	}
	else{
		lidprintk("[lid_report][line %d]value = %d,old_value=%d\n", __LINE__,value,old_value);
		old_value = value;
		enable_irq_wake(lid_data->irq);
		enable_irq(lid_data->irq);
	}
}

static ssize_t lid_state_show(struct class *class,
		struct class_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = 0;
	// sprintf(buf, "%#x\n", revision);
	sprintf(buf, "%s.\n", lidstate);
	ret = strlen(buf) + 1;
	return ret;
}
static CLASS_ATTR(lid_state, 0660, lid_state_show, NULL);

struct class *lid_class;

static int lid_sysfs_init(void)
{
	int ret ;

	lid_class = class_create(THIS_MODULE, "lid");
	ret =  class_create_file(lid_class, &class_attr_lid_state);
	if (ret)
	{
		printk("%s:Fail to creat class\n",__func__);
		return ret;
	}
	lidprintk("%s done!\n",__func__);
	return 0 ;
}


static irqreturn_t lid_interrupt_handler(int irq, void *dev_id)
{
	struct rk_lid_drvdata * lid_data = (struct rk_lid_drvdata *)dev_id;
	struct rk_lid_platform_data *rk_lid_pdata  = lid_data->rk_lid_pdata;
	lidprintk("%s\n",__FUNCTION__);
    	disable_irq_wake(irq);
	disable_irq_nosync(irq);

	wake_lock(&lidaction_wake_lock);
	queue_delayed_work(lid_data->lid_wq, &lid_data->lid_delay_work,msecs_to_jiffies(rk_lid_pdata->debounce_interval));

	return IRQ_HANDLED;
}

static int __devinit lid_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rk_lid_platform_data *pdata = dev_get_platdata(dev);
	struct rk_lid_drvdata *ddata;
	
	struct input_dev *lid_input;
	int err = 0;
	int irq;
	int value;
	
	printk("rk-lid,%s start\n",__FUNCTION__);
	
	wake_lock_init(&lidaction_wake_lock, WAKE_LOCK_SUSPEND, "lid-action");
	
	ddata = kzalloc(sizeof(struct rk_lid_drvdata) ,GFP_KERNEL);
	if(!ddata){
		printk("[%s]ddata allocation fails\n",__FUNCTION__);
		err = -ENOMEM;
		goto exit;
	}
	lid_input = input_allocate_device();
	if ( !lid_input) {
	        printk("[%s]lid_input allocation fails\n",__FUNCTION__);
	        err = -ENOMEM;
	        goto exit_ddata_free;
	}
	
	platform_set_drvdata(pdev, ddata);

	lid_input->name = "lid_input";
	lid_input->phys = "/dev/input/lid_input";

	ddata->input = lid_input;
	ddata->rk_lid_pdata = pdata;
	
	
	set_bit(EV_SW, lid_input->evbit);
	//set_bit(KEY_LID, lid_input->swbit);

	ddata->lid_wq= create_singlethread_workqueue("lid_wq");
	if(!ddata->lid_wq){
		printk("[%s] create work queue failed \n",__FUNCTION__);
		err=-ENOMEM;
		goto creat_work_queue_failed;
	}
	INIT_DELAYED_WORK(&ddata->lid_delay_work, lid_report);
//	queue_delayed_work(ddata->lid_wq, &ddata->lid_delay_work,msecs_to_jiffies(pdata->debounce_interval));
	//printk("ddata->lid_wq=%x\n",ddata->lid_wq);
	if(pdata->lid_gpio != INVALID_GPIO){
		err = gpio_request(pdata->lid_gpio,  "lid");
		if (err < 0) {
			printk("[%s]gpio %d request error\n", __FUNCTION__,pdata->lid_gpio);
			goto gpio_request_failed;
		}
		gpio_direction_input(pdata->lid_gpio);
		irq = gpio_to_irq(pdata->lid_gpio);
		ddata->irq = irq;
		err = request_irq(irq, lid_interrupt_handler,IRQF_TRIGGER_LOW, "rk_lid", (void*)ddata);
		if(err)
			goto err_gpio_request_irq_fail;
		value = gpio_get_value(pdata->lid_gpio);
		lidstate = (value == pdata->lid_close_value)?lidstates[1]:lidstates[2];
	}
	else{
		printk("[%s] pdata->lid_gpio == INVALID_GPIO,so exit\n",__FUNCTION__);
		err = -1;
		goto gpio_request_failed;
	}

	err = input_register_device(lid_input);
	if (err) {
	        printk("[%s]lid_input registration fails\n",__FUNCTION__);
	        goto exit_input_free;
	}
	
	err = lid_sysfs_init();
	if (err < 0) {
		lidprintk(KERN_ERR
            "lid_probe: lid sysfs init failed\n");
		goto exit_LID_sysfs_init_failed;
	}
	enable_irq_wake(irq);
	printk("rk-lid,%s end\n",__FUNCTION__);
	return err;
exit_LID_sysfs_init_failed:
exit_input_free:
	input_free_device(lid_input);
	lid_input = NULL;
err_gpio_request_irq_fail:
	gpio_free(pdata->lid_gpio);
gpio_request_failed:
	cancel_delayed_work(&ddata->lid_delay_work);
creat_work_queue_failed:
	destroy_workqueue(ddata->lid_wq);
exit_ddata_free:
	kfree(ddata);
	ddata = NULL;
exit:
	return err;
}




static int __devexit lid_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rk_lid_platform_data *pdata = dev_get_platdata(dev);
	struct rk_lid_drvdata *ddata = dev_get_drvdata(dev);
	struct input_dev *input = ddata->input;

	
	if(pdata->lid_gpio != INVALID_GPIO){
		free_irq(ddata->irq, ddata);
		gpio_free(pdata->lid_gpio);
	}
	input_unregister_device(input);

	return 0;
}

#ifdef CONFIG_PM
static int lid_suspend(struct device *dev)
{
//	struct rk_lid_platform_data *pdata = dev_get_platdata(dev);
//	struct rk_lid_drvdata *ddata = dev_get_drvdata(dev);

	return 0;
}

static int lid_resume(struct device *dev)
{
//	struct rk_lid_platform_data *pdata = dev_get_platdata(dev);
//	struct rk_lid_drvdata *ddata = dev_get_drvdata(dev);


	return 0;
}


static const struct dev_pm_ops lid_pm_ops = {
	.suspend	= lid_suspend,
	.resume		= lid_resume,
};
#endif

static struct platform_driver lid_device_driver = {
	.probe		= lid_probe,
	.remove		= __devexit_p(lid_remove),
	.driver		= {
		.name	= "rk-lid",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &lid_pm_ops,
#endif
	}
};

static int __init lid_init(void)
{
	return platform_driver_register(&lid_device_driver);
}

static void __exit lid_exit(void)
{
	platform_driver_unregister(&lid_device_driver);
}



module_init(lid_init);
module_exit(lid_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zwp <zwp@rock-chips.com>");
MODULE_DESCRIPTION("lid driver for rock-chips platform");
MODULE_ALIAS("platform:rk-lid");

