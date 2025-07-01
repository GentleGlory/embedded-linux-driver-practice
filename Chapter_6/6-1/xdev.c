#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

extern struct bus_type xbus;

static void xdev_release(struct device* dev)
{
    printk(KERN_INFO "%s-%s\n", __FILE__, __func__);
}

unsigned long id = 0;

static ssize_t xdev_id_show(struct device *dev, struct device_attribute* attr, char *buf)
{
    return sprintf(buf, "%ld\n", id);
}
DEVICE_ATTR_RO(xdev_id);

static struct device xdev = {
    .init_name = "xdev",
    .bus = &xbus,
    .release = xdev_release,
};

static __init int xdev_init(void)
{
    int ret;

    printk(KERN_INFO "xdev init. \n");
    ret = device_register(&xdev);
    if(ret){
        printk(KERN_INFO "unable to register xdev.\n");
        goto error;
    }

    ret = device_create_file(&xdev,&dev_attr_xdev_id);
error:
    return ret;    
}

module_init(xdev_init);

static __exit void xdev_exit(void)
{
    printk(KERN_INFO "xdev exit. \n");
    device_remove_file(&xdev,&dev_attr_xdev_id);
    device_unregister(&xdev);
}

module_exit(xdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");
