#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

extern struct bus_type xbus;

char *name = "xdrv";

static ssize_t drvname_show(struct device_driver* drv, char *buf)
{
    return sprintf(buf, "%s\n", name);
}

DRIVER_ATTR_RO(drvname);

static int xdrv_probe(struct device* dev)
{
    printk(KERN_INFO "%s-%s\n", __FILE__, __func__);
    return 0;
}

static int xdrv_remove(struct device* dev)
{
    printk("%s-%s\n",__FILE__, __func__);
    return 0;
}

static struct device_driver xdrv = {
    .name = "xdev",
    .bus = &xbus,
    .probe = xdrv_probe,
    .remove = xdrv_remove,
};

static __init int xdrv_init(void)
{
    int ret;
    printk(KERN_INFO "xdrv init \n");
    
    ret = driver_register(&xdrv);
    if(ret){
        printk(KERN_INFO "xdrv register failed\n");
        goto error;
    }
    
    ret = driver_create_file(&xdrv, &driver_attr_drvname);

error:
    return ret;
}

module_init(xdrv_init);

static __exit void xdrv_exit(void)
{
    printk(KERN_INFO "xdrv exit\n");
    driver_remove_file(&xdrv, &driver_attr_drvname);
    driver_unregister(&xdrv);
}

module_exit(xdrv_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");
