#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include "led_test.h"

#define DEV_NAME "led_test"
#define DEV_CNT (1)

struct led_resource {

    struct device_node *device_node;

    void __iomem *va_addr;

    unsigned int pin_num;
};

static dev_t led_devno;
static struct cdev led_chr_dev;
struct class *class_led;
struct device *device;
struct device_node *led_test_device_node;

struct led_resource led_res;

static int led_chr_dev_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "led chr dev open\n");
    
    return 0;
}

static ssize_t led_chr_dev_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos )
{
    unsigned int val = 0;
    char ret = 0;
    
    printk(KERN_INFO "led chr dev write.\n");
    
    val = GPIO_GP_SET_CLR_ASSERT_LINE(led_res.pin_num);
    get_user(ret, buf);
    if(ret == '0'){        
        iowrite32(val, GPIO_GPCLR_ADDR(led_res.va_addr, led_res.pin_num));        
    } else {
        iowrite32(val, GPIO_GPSET_ADDR(led_res.va_addr, led_res.pin_num));        
    }

    printk(KERN_INFO "led chr dev write, val=%u.\n",val);  

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
    unsigned int val = 0;
    register unsigned int r;

    printk(KERN_INFO "match successed. \n");

    led_test_device_node = of_find_node_by_path("/led_test");
    if(!led_test_device_node){
        printk(KERN_INFO"Get led_test_device_node failed.\n");
        return -EINVAL; 
    }

    led_res.device_node = of_find_node_by_name(led_test_device_node, "led");
    if(!led_res.device_node){
        printk(KERN_INFO"Get led_device_node failed.\n");
        return -EINVAL; 
    }

    led_res.va_addr = of_iomap(led_res.device_node, 0);
    if(!led_res.device_node){
        printk(KERN_INFO"of_iomap failed.\n");
        return -EINVAL; 
    }

    if(of_property_read_u32(led_res.device_node,"pin_num",&led_res.pin_num) != 0) {
        printk(KERN_INFO"get pin_num failed.\n");
        return -EINVAL; 
    }
    
    printk(KERN_INFO "match successed. 2, %d\n",led_res.pin_num);

    val = ioread32( GPIO_GPFSEL_ADDR(led_res.va_addr, led_res.pin_num));
    val = (val & GPIO_FUNCTION_SELECT_MASK(led_res.pin_num)) | GPIO_FUNCTION_SELECT_OUTPUT(led_res.pin_num);
	iowrite32(val, GPIO_GPFSEL_ADDR(led_res.va_addr, led_res.pin_num));
    
    iowrite32(0, GPIO_GPPUD_ADDR(led_res.va_addr));
    r=150; while(r--){asm volatile("nop");}
	val = GPIO_GPPUDCLK_ASSERT_LINE(led_res.pin_num);
	iowrite32(val,GPIO_GPPUDCLK_ADDR(led_res.va_addr, led_res.pin_num));
	r=150; while(r--){asm volatile("nop");}
	iowrite32(0, GPIO_GPPUD_ADDR(led_res.va_addr));

    printk(KERN_INFO "match successed. 3\n");
    //Register char dev
    ret = alloc_chrdev_region(&led_devno, 0, DEV_CNT, DEV_NAME);
    if( ret < 0){
        printk(KERN_INFO "fail to alloc led_devno \n");
        goto alloc_err;
    }

    led_chr_dev.owner = THIS_MODULE;
    cdev_init(&led_chr_dev, &led_chr_dev_fops);

    printk(KERN_INFO "match successed. 4\n");

    ret = cdev_add(&led_chr_dev, led_devno, DEV_CNT);
    if(ret < 0){
        printk(KERN_INFO "fail to add  led_chr_dev\n");
        goto add_err;
    }
    
    class_led = class_create(DEV_NAME);

    device = device_create(class_led, NULL, led_devno, NULL, DEV_NAME "%d", 1);
    printk(KERN_INFO "match successed. 5\n");
    return 0;
add_err:
    unregister_chrdev_region(led_devno,DEV_CNT);
alloc_err:
    iounmap(led_res.va_addr);

    return ret;
}

static void led_remove(struct platform_device* pdev)
{
    printk(KERN_INFO "led remove\n");
    
    device_destroy(class_led, led_devno);
    class_destroy(class_led);
    cdev_del(&led_chr_dev);
    unregister_chrdev_region(led_devno,1);
    
    iounmap(led_res.va_addr);    
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