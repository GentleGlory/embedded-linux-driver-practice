#include <linux/init.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

volatile int test_value = 0;

struct kobject *kobj_ref;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    printk(KERN_INFO "read sysfs\n");
    return sprintf(buf, "test_value = %d\n", test_value);
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    printk(KERN_INFO "write sysfs\n");
    sscanf(buf, "%d", &test_value);
    return count;
}

static struct kobj_attribute sysfs_test_attr = __ATTR(test_value, 0664, sysfs_show, sysfs_store);

static int __init sys_test_init(void)
{
    kobj_ref = kobject_create_and_add("sysfs_test", NULL);

    if (!kobj_ref) {
        printk(KERN_ERR "Failed to create kobject\n");
        return -ENOMEM;
    }

    if(sysfs_create_file(kobj_ref, &sysfs_test_attr.attr)) {
        printk(KERN_ERR "Failed to create sysfs file\n");
        kobject_put(kobj_ref);
        return -ENOMEM; 
    }

    printk(KERN_INFO "Init module successfully\n");

    return 0;
} 

module_init(sys_test_init);

static void __exit sys_test_exit(void)
{
    kobject_put(kobj_ref);
    printk(KERN_INFO "Exit module successfully\n");
}

module_exit(sys_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");