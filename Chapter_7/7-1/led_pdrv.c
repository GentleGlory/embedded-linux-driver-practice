#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include "led_pdrv.h"

#define DEV_NAME    "led"
#define DEV_CNT     1

static dev_t devno;

static struct class *led_test_class;

struct led_data {
    unsigned int led_pin;

    unsigned int __iomem *va_ddr;
    
    struct cdev led_cdev;
};

static int led_cdev_open(struct inode *inode, struct file *filp)
{
    unsigned int val = 0;
    register unsigned int r;
    struct led_data *cur_led = container_of(inode->i_cdev, struct led_data, led_cdev);

    printk(KERN_INFO "led cdev open\n");
    
    val = ioread32( GPIO_GPFSEL_ADDR(cur_led->va_ddr, cur_led->led_pin));
    val = (val & GPIO_FUNCTION_SELECT_MASK(cur_led->led_pin)) | GPIO_FUNCTION_SELECT_OUTPUT(cur_led->led_pin);
	iowrite32(val, GPIO_GPFSEL_ADDR(cur_led->va_ddr, cur_led->led_pin));
    
    iowrite32(0, GPIO_GPPUD_ADDR(cur_led->va_ddr));
    r=150; while(r--){asm volatile("nop");}
	val = GPIO_GPPUDCLK_ASSERT_LINE(cur_led->led_pin);
	iowrite32(val,GPIO_GPPUDCLK_ADDR(cur_led->va_ddr, cur_led->led_pin));
	r=150; while(r--){asm volatile("nop");}
	iowrite32(0, GPIO_GPPUD_ADDR(cur_led->va_ddr));


    filp->private_data = cur_led;
    return 0;
}

static int led_cdev_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "led cdev released.\n");
    return 0;
}

static ssize_t led_cdev_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos)
{
    unsigned int val = 0;
    char ret = 0;
    struct led_data* cur_led = (struct led_data*) filp->private_data;

    printk(KERN_INFO "led cdev write.\n");
    
    val = GPIO_GP_SET_CLR_ASSERT_LINE(cur_led->led_pin);
    get_user(ret, buf);
    if(ret == '0'){        
        iowrite32(val, GPIO_GPCLR_ADDR(cur_led->va_ddr, cur_led->led_pin));        
    } else {
        iowrite32(val, GPIO_GPSET_ADDR(cur_led->va_ddr, cur_led->led_pin));        
    }

    printk(KERN_INFO "led cdev write, val=%u.\n",val);  

    return count;
}


static struct file_operations led_cdev_fops = {
    .open = led_cdev_open,
    .release = led_cdev_release,
    .write = led_cdev_write
};


static int led_pdrv_probe(struct platform_device *pdev)
{
    struct led_data *cur_led;
    unsigned int *led_hwinfo;

    struct resource *mem_Reg;
    
    int ret = 0;

    printk(KERN_INFO "led platform driver probe\n");

    cur_led = devm_kzalloc(&pdev->dev, sizeof(struct led_data), GFP_KERNEL);
    if(!cur_led)
        return -ENOMEM;

    //led_hwinfo = devm_kzalloc(&pdev->dev, sizeof(unsigned int) * 2, GFP_KERNEL);
    //if(!led_hwinfo)
    //    return -ENOMEM;
        
    led_hwinfo = dev_get_platdata(&pdev->dev);
    
    if(!led_hwinfo)
        return -ENOPARAM;
        
    cur_led->led_pin = led_hwinfo[0];

    mem_Reg = platform_get_resource(pdev, IORESOURCE_MEM, 0);

    cur_led->va_ddr = devm_ioremap(&pdev->dev, mem_Reg->start, resource_size(mem_Reg));

    cdev_init(&cur_led->led_cdev, &led_cdev_fops);
    
    ret = cdev_add(&cur_led->led_cdev,devno, 1);
    if(ret < 0)
    {
        printk(KERN_INFO "Failed to add cdev\n");
        goto add_err;
    }

    device_create(led_test_class, NULL, devno, NULL, DEV_NAME "%d", 1);

    platform_set_drvdata(pdev, cur_led);

add_err:
    return ret;
}

static void led_pdrv_remove(struct platform_device* pdev)
{
    struct led_data *cur_led = platform_get_drvdata(pdev); 
    
    printk(KERN_INFO "led pdrv remove\n");
    
    cdev_del(&cur_led->led_cdev);

    device_destroy(led_test_class, devno);
}


static struct platform_device_id led_pdev_ids[] = {
    {.name = "led_pdev"},
    {}
};

MODULE_DEVICE_TABLE(platform, led_pdev_ids);

static struct platform_driver led_pdrv = {
    .driver.name = "led_pdev",
    .probe = led_pdrv_probe,
    .remove = led_pdrv_remove,
    .id_table = led_pdev_ids, 
};

static __init int led_pdrv_init(void)
{

    printk(KERN_INFO "led platform driver init\n");

    alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);
    
    led_test_class = class_create("test_leds");
    
    platform_driver_register(&led_pdrv);

    return 0;
}

module_init(led_pdrv_init)

static __exit void led_pdrv_exit(void)
{
    printk(KERN_INFO "led platform driver exit\n");
    platform_driver_unregister(&led_pdrv);
    
    unregister_chrdev_region(devno,DEV_CNT);
    class_destroy(led_test_class);
}

module_exit(led_pdrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");
