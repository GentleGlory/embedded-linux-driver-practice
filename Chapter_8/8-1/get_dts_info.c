#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <linux/of.h>



struct device_node *led_test_device_node;
struct device_node *led_device_node;
struct property    *led_property;

int size = 0;
unsigned int out_values[18];

static const struct of_device_id of_gpio_led_match[] = {
    {.compatible = "get_dts_info_test"},
    {}
};

static int get_dts_info_probe(struct platform_device *pdev)
{
    int error_status = -1;

    printk(KERN_INFO "%s\n", __func__ );
    
    led_test_device_node = of_find_node_by_path("/get_dts_info_test");
    if(!led_test_device_node){
        printk(KERN_INFO "get led_device_node failed. \n");
        return -EINVAL;
    }

    printk(KERN_INFO "name:%s \n", led_test_device_node->name);
    printk(KERN_INFO "child name:%s \n", led_test_device_node->child->name);

    led_device_node = of_get_next_child(led_test_device_node, NULL);
    if(!led_device_node){
        printk(KERN_INFO "get led_device_node failed. \n");
        return -EINVAL;
    }

    printk(KERN_INFO "name:%s \n", led_device_node->name);
    printk(KERN_INFO "parent name:%s \n", led_device_node->parent->name);

    led_property = of_find_property(led_device_node, "compatible", &size);   
    if(!led_property){
        printk(KERN_INFO "get led_property failed. \n");
        return -EINVAL;
    }

    printk(KERN_INFO "size: %d\n",size);
    printk(KERN_INFO "name: %s\n",led_property->name);
    printk(KERN_INFO "length: %d\n",led_property->length);
    printk(KERN_INFO "value: %s\n", (char*)led_property->value);

    error_status = of_property_read_u32_array(led_device_node, "reg", out_values, 2);
    if(error_status != 0){
        printk(KERN_INFO "get out_values failed !\n");
        return error_status;
    }
    
    printk(KERN_INFO "0x%08X\n", out_values[0]);
    printk(KERN_INFO "0x%08X\n", out_values[1]);

    return 0;
}

static void get_dts_info_remove(struct platform_device * pdev)
{
    printk(KERN_INFO "%s\n", __func__ );
}


static struct platform_driver get_dts_info_driver = {
    .probe = get_dts_info_probe,
    .remove = get_dts_info_remove,
    .driver = {
        .name = "get_dts_info_test",
        .of_match_table = of_gpio_led_match,
    }, 
};

module_platform_driver(get_dts_info_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");