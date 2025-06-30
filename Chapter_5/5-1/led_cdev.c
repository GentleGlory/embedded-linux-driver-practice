#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/device/class.h>
#include "led_cdev.h"

#define DEV_NAME    "LedCharDev"
#define DEV_CNT     1

static dev_t devno;
struct class *led_chrdev_class = NULL;

struct led_chrdev{
    struct cdev dev;
    unsigned int __iomem *va_ddr;

    unsigned int led_pin;
};

static int led_chrdev_open(struct inode *inode, struct file *filp);
static int led_chrdev_release(struct inode *inode, struct file *filp);
static ssize_t led_chrdev_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos);
static struct file_operations chr_dev_fops = 
{
    .owner = THIS_MODULE,
    .open = led_chrdev_open,
    .release = led_chrdev_release,
    .write = led_chrdev_write,
};

static int led_chrdev_open(struct inode *inode, struct file *filp)
{
    unsigned int val = 0;
    register unsigned int r;

    struct led_chrdev* led_cdev = (struct led_chrdev*) container_of(inode->i_cdev, struct led_chrdev, dev);
    filp->private_data = led_cdev;

    printk(KERN_INFO "led chardev opened.\n");

    val = ioread32( GPIO_GPFSEL_ADDR(led_cdev->va_ddr, led_cdev->led_pin));
    val = (val & GPIO_FUNCTION_SELECT_MASK(led_cdev->led_pin)) | GPIO_FUNCTION_SELECT_OUTPUT(led_cdev->led_pin);
	iowrite32(val, GPIO_GPFSEL_ADDR(led_cdev->va_ddr, led_cdev->led_pin));
    
    iowrite32(0, GPIO_GPPUD_ADDR(led_cdev->va_ddr));
    r=150; while(r--){asm volatile("nop");}
	val = GPIO_GPPUDCLK_ASSERT_LINE(led_cdev->led_pin);
	iowrite32(val,GPIO_GPPUDCLK_ADDR(led_cdev->va_ddr, led_cdev->led_pin));
	r=150; while(r--){asm volatile("nop");}
	iowrite32(0, GPIO_GPPUD_ADDR(led_cdev->va_ddr));

    return 0;
}

static int led_chrdev_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "led chardev released.\n");
    return 0;
}

static ssize_t led_chrdev_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos)
{
    unsigned int val = 0;
    char ret = 0;
    struct led_chrdev* led_cdev = (struct led_chrdev*) filp->private_data;

    printk(KERN_INFO "led chardev write.\n");
    
    val = GPIO_GP_SET_CLR_ASSERT_LINE(led_cdev->led_pin);
    get_user(ret, buf);
    if(ret == '0'){        
        iowrite32(val, GPIO_GPCLR_ADDR(led_cdev->va_ddr, led_cdev->led_pin));        
    } else {
        iowrite32(val, GPIO_GPSET_ADDR(led_cdev->va_ddr, led_cdev->led_pin));        
    }

    printk(KERN_INFO "led chardev write, val=%u.\n",val);  

    return count;
}

static struct led_chrdev led_cdev[DEV_CNT] = {
    {.led_pin = 17},
};


static int __init led_chrdev_init(void)
{
    int i = 0; 
    dev_t cur_dev;

    printk(KERN_INFO "led_chrdev init\n");

    alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);

    led_chrdev_class = class_create("led_chrdev");
    
    for(; i < DEV_CNT; i++){
        
        cdev_init(&led_cdev[i].dev, &chr_dev_fops);
        led_cdev[i].dev.owner = THIS_MODULE;
        cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);

        led_cdev[i].va_ddr = ioremap(GPIO_BASE_ADDR, GPIO_REG_SIZE);
        
        cdev_add(&led_cdev[i].dev, cur_dev, 1);
        
        device_create(led_chrdev_class, NULL, cur_dev, NULL, DEV_NAME "%d", i);
    }
    return 0;
}

static void __exit led_chrdev_exit(void)
{
    int i = 0; 
    dev_t cur_dev;

    printk(KERN_INFO "led_chrdev exit\n");

    for(; i < DEV_CNT; i++){     
        iounmap(led_cdev[i].va_ddr);        

        cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);

        device_destroy(led_chrdev_class, cur_dev);
        cdev_del(&led_cdev[i].dev);
    }

    unregister_chrdev_region(devno, DEV_CNT);
    class_destroy(led_chrdev_class);
}

module_init(led_chrdev_init);
module_exit(led_chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");
MODULE_DESCRIPTION("led chrdev module");
MODULE_ALIAS("led_chrdev_module");