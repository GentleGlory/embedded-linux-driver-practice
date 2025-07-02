#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "led_pdev.h"

static struct resource led_resource[] = {
    [0] = DEFINE_RES_MEM(GPIO_BASE_ADDR,GPIO_REG_SIZE),  
};

static void led_release(struct device* dev)
{


}

unsigned int led_hwinfo[1] = {17};

static struct platform_device led_pdev = {
    .name = "led_pdev",
    .id = 0,
    .num_resources = ARRAY_SIZE(led_resource),
    .resource = led_resource,
    .dev = {
        .release = led_release,
        .platform_data = led_hwinfo,   
    },
};

static __init int led_pdev_init(void)
{
    printk(KERN_INFO "led pdev init\n");
    platform_device_register(&led_pdev);
    return 0; 
}

module_init(led_pdev_init);

static __exit void led_pdev_exit(void)
{
    printk(KERN_INFO "led pdev exit\n");
    platform_device_unregister(&led_pdev);
}

module_exit(led_pdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");


