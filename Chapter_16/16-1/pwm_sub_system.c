#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/slab.h>

struct pwm_device *pwd_demo;

static int pwm_demo_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device* dev = &pdev->dev;
    struct pwm_state state;

    printk(KERN_INFO "match success\n");

    pwd_demo = devm_pwm_get(dev, "pwd-demo"); 
    if (IS_ERR(pwd_demo)) {
        ret = PTR_ERR(pwd_demo);
        printk(KERN_ERR "Failed to get PWM device: %d\n", ret);
        return ret; 
    }

    pwm_init_state(pwd_demo, &state);
    state.polarity = PWM_POLARITY_INVERSED;
    state.duty_cycle = 100000000; 
    state.period = 500000000;    
    state.enabled = true;
    ret = pwm_apply_might_sleep(pwd_demo, &state);
    if (ret) {
        printk(KERN_ERR "Failed to apply PWM state: %d\n", ret);
        return ret;
    }

    return ret;
}

static void pwm_demo_remove(struct platform_device* pdev)
{
    struct pwm_state state;
    pwm_init_state(pwd_demo, &state);
    state.enabled = false;
    pwm_apply_might_sleep(pwd_demo, &state);
    printk(KERN_INFO "pwm_demo removed\n");
}

static const struct of_device_id pwm_leds_match[] = {
    {.compatible = "pwm-demo"}, // 與 device tree 一致
    {},
};

struct platform_driver pwm_demo_driver = {
    .probe = pwm_demo_probe,
    .remove = pwm_demo_remove,
    .driver = {
        .name = "pwm_demo",
        .owner = THIS_MODULE,
        .of_match_table = pwm_leds_match,
    },
};

module_platform_driver(pwm_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");