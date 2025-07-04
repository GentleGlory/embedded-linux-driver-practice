#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h> 

#define DEV_NAME "led_test"
#define DEV_CNT (1)

static dev_t led_devno;
static struct cdev led_chr_dev;
struct class *class_led;
struct device *device;
struct device_node *led_test_device_node;

static struct gpio_desc *led;


static int led_chr_dev_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "led chr dev open\n");
    
    return 0;
}

static ssize_t led_chr_dev_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos )
{
    char ret = 0;
    
    printk(KERN_INFO "led chr dev write.\n");
    
    get_user(ret, buf);
    if(ret == '0'){        
        gpiod_set_value(led, 0);
    } else {
        gpiod_set_value(led, 1);
    }

    return count;
}

static struct file_operations led_chr_dev_fops = {
    .owner = THIS_MODULE,
    .open = led_chr_dev_open,
    .write = led_chr_dev_write
};

static int led_probe(struct platform_device *pdev)
{
    int ret = 0;
    
    led = gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led)) {
        printk(KERN_INFO "Failed to get GPIO\n");
        return PTR_ERR(led);
    }
    
    ret = alloc_chrdev_region(&led_devno, 0, DEV_CNT, DEV_NAME);
    if( ret < 0){
        printk(KERN_INFO "fail to alloc led_devno \n");
        goto alloc_err;
    }

    led_chr_dev.owner = THIS_MODULE;
    cdev_init(&led_chr_dev, &led_chr_dev_fops);

    ret = cdev_add(&led_chr_dev, led_devno, DEV_CNT);
    if(ret < 0){
        printk(KERN_INFO "fail to add  led_chr_dev\n");
        goto add_err;
    }
    
    class_led = class_create(DEV_NAME);

    device = device_create(class_led, NULL, led_devno, NULL, DEV_NAME "%d", 1);
    
    return 0;
add_err:
    unregister_chrdev_region(led_devno,DEV_CNT);
alloc_err:
    return ret;
}

static void led_remove(struct platform_device* pdev)
{
    printk(KERN_INFO "led remove\n");
    
    gpiod_put(led);

    device_destroy(class_led, led_devno);
    class_destroy(class_led);
    cdev_del(&led_chr_dev);
    unregister_chrdev_region(led_devno,1);
    
}


static const struct of_device_id led_ids[] = {
    {.compatible = "fire,led_test"},
    {},
};

struct platform_driver led_platform_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "leds-platform",
        .owner = THIS_MODULE,
        .of_match_table = led_ids,
    },
};

static int __init led_platform_driver_init(void)
{
    int ret = platform_driver_register(&led_platform_driver);
    printk(KERN_INFO "Driver state is %d\n",ret);
    return 0;
}

module_init(led_platform_driver_init);

static void __exit led_platform_driver_exit(void)
{
    platform_driver_unregister(&led_platform_driver);
    printk(KERN_INFO "dts test led exit\n");
} 

module_exit(led_platform_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");