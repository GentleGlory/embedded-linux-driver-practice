#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/of.h>
#include <linux/slab.h>


struct button_data {
    struct gpio_desc *button_input_gpiod;
    struct input_dev *button_input_dev;
    struct platform_device *pdev;
    int irq;
    struct delayed_work button_work;
};

static void button_work_handler(struct work_struct *work)
{
    struct button_data *priv = container_of(work, struct button_data, button_work.work);
    int button_state; 
    
    button_state = (gpiod_get_value(priv->button_input_gpiod) & 1);
    if(button_state){
        printk(KERN_INFO "button_work_handler 1\n");
        input_report_key(priv->button_input_dev, BTN_0, 1);
        input_sync(priv->button_input_dev);
    }else {
        printk(KERN_INFO "button_work_handler 0\n");
        input_report_key(priv->button_input_dev, BTN_0, 0);
        input_sync(priv->button_input_dev);
    }
}

static irqreturn_t button_irq_handler(int irq, void *dev_id)
{
    struct button_data *priv = dev_id;
    
    cancel_delayed_work_sync(&priv->button_work);
    schedule_delayed_work(&priv->button_work,jiffies_to_msecs(5));

    return IRQ_HANDLED;
}

static int button_open(struct input_dev *dev)
{    
    printk(KERN_INFO "input button opned\n");
    return 0;    
}

static void button_close(struct input_dev *dev)
{    
    printk(KERN_INFO "input button closed\n");    
}

static int button_probe(struct platform_device *pdev)
{
    struct button_data *priv;
    struct gpio_desc *gpiod;
    struct input_dev *i_dev;
    int ret;

    printk(KERN_INFO "button probe\n");
    
    priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv) {
        printk(KERN_INFO "Failed to allocate memory for button data\n");
        return -ENOMEM;
    }

    i_dev = input_allocate_device();
    if (!i_dev) {
        printk(KERN_INFO "Failed to allocate input device\n");
        return -ENOMEM;
    }

    i_dev->open = button_open;
    i_dev->close = button_close;
    i_dev->name = "key input";
    i_dev->dev.parent = &pdev->dev;
    priv->button_input_dev = i_dev;
    priv->pdev = pdev;
    INIT_DELAYED_WORK(&priv->button_work, button_work_handler);

    set_bit(EV_KEY, i_dev->evbit);
    set_bit(BTN_0, i_dev->keybit);

    gpiod = gpiod_get(&pdev->dev, "button", GPIOD_IN);
    if (IS_ERR(gpiod)) {
        printk(KERN_INFO "Failed to get button GPIO\n");
        ret = PTR_ERR(gpiod);
        input_free_device(i_dev);
        return ret;
    }
    
    priv->irq = gpiod_to_irq(gpiod);
    priv->button_input_gpiod = gpiod;

    if (priv->irq < 0) {
        printk(KERN_INFO "Failed to get IRQ for button GPIO\n");
        ret = priv->irq;
        goto err_register_device;
    }

    ret = input_register_device(i_dev);
    if (ret) {
        printk(KERN_INFO "Failed to register input device\n");
        goto err_register_device;
    }

    platform_set_drvdata(pdev, priv);

    ret = request_any_context_irq(priv->irq, button_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 
        "input-button", priv);
    if (ret) {
        printk(KERN_INFO "Failed to request IRQ %d\n", priv->irq);  
        goto err_request_irq;
    }

    return 0;

err_request_irq:
    input_unregister_device(i_dev);
err_register_device:    
    gpiod_put(gpiod);
    return ret;
}

static void button_remove(struct platform_device* pdev)
{
    struct button_data *priv = platform_get_drvdata(pdev);
    
    printk(KERN_INFO "button remove\n");

    input_unregister_device(priv->button_input_dev);
    gpiod_put(priv->button_input_gpiod);
    free_irq(priv->irq, priv);
}

static const struct of_device_id button_ids[] = {
    {.compatible = "input_button"},
    {},
};

struct platform_driver button_input = {
    .probe = button_probe,
    .remove = button_remove,
    .driver = {
        .name = "button-platform",
        .owner = THIS_MODULE,
        .of_match_table = button_ids,
    },
};

static int __init button_driver_init(void)
{
    int ret = platform_driver_register(&button_input);
    printk(KERN_INFO "Driver state is %d\n",ret);
    return 0;
}

module_init(button_driver_init);

static void __exit button_driver_exit(void)
{
    platform_driver_unregister(&button_input);
    printk(KERN_INFO "dts test button exit\n");
} 

module_exit(button_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");