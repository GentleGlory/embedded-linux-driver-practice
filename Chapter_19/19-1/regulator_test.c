#include <linux/init.h>
#include <linux/module.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/platform_device.h>

static struct regulator_dev *my_requlator_test_rdev;
static struct regulator_consumer_supply relate = {
    .dev_name = "my_consumer",
    .supply = "VCC"
};

static struct regulator_init_data my_regulator_initdata = {
    .constraints = {
        .name = "my_regulator",
        .always_on = 0,
        .min_uV = 500000,
        .max_uV = 1350000,
        .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
        .valid_modes_mask = REGULATOR_MODE_NORMAL
    },
    .num_consumer_supplies = 1,
    .consumer_supplies = &relate
};

static struct regulator_ops my_regulator_ops;

static struct regulator_desc my_regulator_desc = {
    .name = "my_regulator_test",
    .id = -1,
    .n_voltages = 16,
    .type = REGULATOR_VOLTAGE,
    .owner = THIS_MODULE,
    .ops = &my_regulator_ops
};

static int regulator_driver_probe(struct platform_device *pdev)
{
    struct regulator_config config = {};
    int ret;
    config.dev = &pdev->dev;
    config.init_data = &my_regulator_initdata;
    
    my_requlator_test_rdev = regulator_register(&pdev->dev,&my_regulator_desc, &config);
    if (IS_ERR(my_requlator_test_rdev)) {
        ret = PTR_ERR(my_requlator_test_rdev);
        printk(KERN_ERR "Failed to register regulator: %d\n", ret);
        return ret;
    }
    
    return 0;
}

static void regulator_driver_remove(struct platform_device *pdev)
{
    regulator_unregister(my_requlator_test_rdev);
    printk(KERN_INFO "Regulator unregistered\n");
}

static struct platform_driver my_regulator_driver = {
    .probe = regulator_driver_probe,
    .remove = regulator_driver_remove,
    .driver = {
        .name = "my_regulator",
        .owner = THIS_MODULE
    },
};

static struct platform_device *regulator_pdev;

static int my_regulator_test_init(void)
{
    int ret;

    regulator_pdev = platform_device_alloc("my_regulator", -1);
    if (!regulator_pdev) {
        printk(KERN_ERR "Failed to allocate platform device for regulator\n");
        return -ENOMEM;
    }

    ret = platform_device_add(regulator_pdev);
    if (ret) {
        printk(KERN_ERR "Failed to add platform device for regulator\n");
        platform_device_put(regulator_pdev);
        return ret;
    }
    
    ret = platform_driver_register(&my_regulator_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register regulator driver\n");
        platform_device_unregister(regulator_pdev);
        platform_device_put(regulator_pdev);
        return ret;
    }

    return 0;
} 

module_init(my_regulator_test_init);

static void my_regulator_test_exit(void)
{
    platform_device_unregister(regulator_pdev);
    platform_device_put(regulator_pdev);
    platform_driver_unregister(&my_regulator_driver);
    printk(KERN_INFO "Regulator test module exited\n");
}

module_exit(my_regulator_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");