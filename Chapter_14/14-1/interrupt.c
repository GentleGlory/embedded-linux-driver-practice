#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>

#define DEV_NAME "button"
#define DEV_CNT (1)

static dev_t button_devno;
static struct cdev button_chr_dev;
struct class *class_button;
struct device *device;
struct device_node *button_device_node;

int button_gpio_num = 0;
u32 interrupt_num = 0;
atomic_t button_status = ATOMIC_INIT(0); 

static irqreturn_t button_irq_handler(int irq, void *dev_id)
{
    atomic_inc(&button_status);
    printk(KERN_INFO "In button_irq_handler \n");
    return IRQ_HANDLED;
}

static int button_chr_dev_open(struct inode *inode, struct file *filp)
{
    
    printk(KERN_INFO "button chr dev opned\n");

    return 0;    
}

static ssize_t button_chr_dev_read(struct file *filp, char __user *buf, 
    size_t count, loff_t *ppos )
{
    int button_val = 0;
    char val[32] = {0};
    int len;
    int error;

    printk(KERN_INFO "button chr dev read.\n");

    if (*ppos > 0)
        return 0;
    
    button_val = atomic_xchg(&button_status, 0);
    len = snprintf(val, sizeof(val), "%d\n", button_val);

    if (len > count)
        return -EINVAL;

    error = copy_to_user(buf, val, len);
    if (error)
        return -EFAULT;

    *ppos += len;

    return len;
}

static struct file_operations button_chr_dev_fops = {
    .owner = THIS_MODULE,
    .open = button_chr_dev_open,
    .read = button_chr_dev_read
};

static int button_probe(struct platform_device *pdev)
{
    int error = -1;

    printk(KERN_INFO "button probe\n");

    button_device_node = of_find_node_by_path("/button_interrupt");
    if(!button_device_node){
        printk(KERN_INFO "Fail to find button_interrupt node\n");
        return -1;
    }

    button_gpio_num = of_get_named_gpio(button_device_node, "button-gpios", 0);
    if(button_gpio_num < 0){
        printk(KERN_INFO "Fail to get button-gpios %d\r\n",button_gpio_num);
        return button_gpio_num;
    }
    printk(KERN_INFO "button_gpio_num = %d\n",button_gpio_num);

    error = gpio_request(button_gpio_num, "button_gpios");
    if(error < 0){
        printk(KERN_INFO "gpio_request error :%d\n",error);
        return error;
    }

    error = gpio_direction_input(button_gpio_num);
    if(error < 0){
        printk(KERN_INFO "gpio_direction_input error :%d\n",error);

        goto set_input_error;
    }

    interrupt_num = irq_of_parse_and_map(button_device_node, 0);
    if(!interrupt_num){
        printk(KERN_INFO "Fail to irq_of_parse_and_map\n");
        error = -EINVAL;
        goto set_input_error;
    }    

    error = request_irq(interrupt_num,button_irq_handler,
        IRQF_TRIGGER_FALLING, "button_interrupt", NULL);
    if(error < 0){
        printk(KERN_INFO "Fail to request_irq %d\n",error);
        goto set_input_error;
    }
    
    error = alloc_chrdev_region(&button_devno, 0, DEV_CNT, DEV_NAME);
    if(error < 0){
        printk(KERN_INFO "Fail to alloc_chrdev_region %d\n",error);
        goto alloc_chrdev_region_error;
    }

    button_chr_dev.owner = THIS_MODULE;
    cdev_init(&button_chr_dev,&button_chr_dev_fops);

    error = cdev_add(&button_chr_dev, button_devno, DEV_CNT);
    if(error < 0) {
        printk(KERN_INFO "Fail to add cdev %d\n",error);
        goto cdev_add_error;
    }

    class_button = class_create(DEV_NAME);
    device = device_create(class_button, NULL, 
        button_devno, NULL, DEV_NAME "%d",1);

    return 0;
    
cdev_add_error:
    unregister_chrdev_region(button_devno,DEV_CNT);
alloc_chrdev_region_error:
    free_irq(interrupt_num,NULL);
set_input_error:
    gpio_free(button_gpio_num);

    return error;
}

static void button_remove(struct platform_device* pdev)
{
    printk(KERN_INFO "button remove\n");
    
    device_destroy(class_button,button_devno);
    class_destroy(class_button);

    cdev_del(&button_chr_dev);
    unregister_chrdev_region(button_devno,DEV_CNT);

    free_irq(interrupt_num,NULL);
    gpio_free(button_gpio_num);
}


static const struct of_device_id button_ids[] = {
    {.compatible = "button_interrupt"},
    {},
};

struct platform_driver button_platform_driver = {
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
    int ret = platform_driver_register(&button_platform_driver);
    printk(KERN_INFO "Driver state is %d\n",ret);
    return 0;
}

module_init(button_driver_init);

static void __exit button_driver_exit(void)
{
    platform_driver_unregister(&button_platform_driver);
    printk(KERN_INFO "dts test button exit\n");
} 

module_exit(button_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");