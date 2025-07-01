#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

static int xbus_match(struct device *dev, const struct device_driver* drv)
{
    printk(KERN_INFO "%s-%s\n", __FILE__, __func__ );

    if(!strncmp( dev_name(dev), drv->name, strlen(drv->name))){
        printk(KERN_INFO "dev & drv match\n");
        return 1;
    }

    return 0;
}

static char *bus_name = "xbus";

static ssize_t xbus_test_show(const struct bus_type *bus, char *buf)
{
    return sprintf(buf, "%s\n", bus_name);
}
BUS_ATTR_RO(xbus_test);

struct bus_type xbus = {
    .name = "xbus",
    .match = xbus_match,
};

EXPORT_SYMBOL(xbus);

static __init int xbus_init(void)
{
    int ret;
    printk(KERN_INFO "xbus init\n");
   
    ret = bus_register(&xbus);
    if(ret){
        printk(KERN_INFO "xbus register failed.\n");
        goto error;
    }

    ret = bus_create_file(&xbus, &bus_attr_xbus_test);    
error:    
    return ret;
}

module_init(xbus_init);

static __exit void xbus_exit(void)
{
    printk(KERN_INFO "xbus exit\n");

    bus_remove_file(&xbus, &bus_attr_xbus_test);
    bus_unregister(&xbus);
}

module_exit(xbus_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");
