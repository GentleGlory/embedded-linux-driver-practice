#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/types.h>

#include "i2c_mpu6050.h"


#define DEV_NAME    "i2c1_mpu6050"
#define DEV_CNT     1

static dev_t mpu6050_devno;
static struct cdev mpu6050_chr_dev;
static struct class *mpu6050_class;
struct device *device_mpu6050;
struct device_node *mpu6050_device_node;

struct i2c_client *mpu6050_client = NULL;

#define I2C_WRITE_CHECK(client, addr, data) \
    do { \
        int __ret = i2c_write_mpu6050(client, addr, data); \
        if (__ret < 0) { \
            printk(KERN_ERR "i2c_write_mpu6050 failed at %s:%d, ret=%d\n", __func__, __LINE__, __ret); \
            return __ret; \
        } \
    } while (0)

#define I2C_READ_CHECK(client, addr, pdata, len) \
    do { \
        int __ret = i2c_read_mpu6050(client, addr, pdata, len); \
        if (__ret < 0) { \
            printk(KERN_ERR "i2c_read_mpu6050 failed at %s:%d, ret=%d\n", __func__, __LINE__, __ret); \
            return __ret; \
        } \
    } while (0)

static int i2c_write_mpu6050(struct i2c_client *client, u8 address, u8 data)
{
    int error = 0;
    u8 write_data[2];
    struct i2c_msg send_msg;

    write_data[0] = address;
    write_data[1] = data;

    send_msg.addr = client->addr;
    send_msg.flags = 0; 
    send_msg.buf = write_data;
    send_msg.len = sizeof(write_data);
    
    error = i2c_transfer(client->adapter, &send_msg, 1);
    if (error < 0) {
        printk(KERN_ERR "i2c_transfer failed: %d\n", error);
        return error;   
    }

    return 0;
}

static int i2c_read_mpu6050(struct i2c_client *client, u8 address, u8 *data,u32 length)
{
    int error = 0;
    struct i2c_msg read_msg[2];
    u8 write_data[1];

    write_data[0] = address;

    read_msg[0].addr = client->addr;
    read_msg[0].flags = 0; 
    read_msg[0].buf = write_data;
    read_msg[0].len = sizeof(write_data);

    read_msg[1].addr = client->addr;
    read_msg[1].flags = I2C_M_RD; 
    read_msg[1].buf = data;
    read_msg[1].len = length;

    error = i2c_transfer(client->adapter, read_msg, 2);
    if (error < 0) {
        printk(KERN_ERR "i2c_transfer failed: %d\n", error);
        return error;   
    }

    return 0;
}

static int mpu6050_init(void)
{
    I2C_WRITE_CHECK(mpu6050_client, PWR_MGMT_1, 0x00);
    I2C_WRITE_CHECK(mpu6050_client, SMPLRT_DIV, 0x07);
    I2C_WRITE_CHECK(mpu6050_client, CONFIG, 0x06);
    I2C_WRITE_CHECK(mpu6050_client, ACCEL_CONFIG, 0x01);
    
    return 0;
}

static int mpu6050_open(struct inode *inode, struct file *file)
{
    int ret;
    printk(KERN_INFO "mpu6050 device opened\n");
    ret = mpu6050_init();
    if (ret < 0)
        return ret;
    return 0;
}

static ssize_t mpu6050_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    u8 data_H;
    u8 data_L;
    short mpu6050_result[6] = {};
    char str[128] = {0};
    int len = 0;


    if (*ppos > 0)
        return 0;

    if (sizeof(mpu6050_result) > count)
        return -EINVAL;    

    I2C_READ_CHECK(mpu6050_client, ACCEL_XOUT_H, &data_H, 1);
    I2C_READ_CHECK(mpu6050_client, ACCEL_XOUT_L, &data_L, 1);
    mpu6050_result[0] = (data_H << 8) | data_L;

    I2C_READ_CHECK(mpu6050_client, ACCEL_YOUT_H, &data_H, 1);
    I2C_READ_CHECK(mpu6050_client, ACCEL_YOUT_L, &data_L, 1);
    mpu6050_result[1] = (data_H << 8) | data_L;

    I2C_READ_CHECK(mpu6050_client, ACCEL_ZOUT_H, &data_H, 1);
    I2C_READ_CHECK(mpu6050_client, ACCEL_ZOUT_L, &data_L, 1);
    mpu6050_result[2] = (data_H << 8) | data_L;

    I2C_READ_CHECK(mpu6050_client, GYRO_XOUT_H, &data_H, 1);
    I2C_READ_CHECK(mpu6050_client, GYRO_XOUT_L, &data_L, 1);
    mpu6050_result[3] = (data_H << 8) | data_L;

    I2C_READ_CHECK(mpu6050_client, GYRO_YOUT_H, &data_H, 1);
    I2C_READ_CHECK(mpu6050_client, GYRO_YOUT_L, &data_L, 1);
    mpu6050_result[4] = (data_H << 8) | data_L;    

    I2C_READ_CHECK(mpu6050_client, GYRO_ZOUT_H, &data_H, 1);
    I2C_READ_CHECK(mpu6050_client, GYRO_ZOUT_L, &data_L, 1);
    mpu6050_result[5] = (data_H << 8) | data_L;

    len = snprintf(str, sizeof(str), "X=%d, Y=%d, Z=%d, Gx=%d, Gy=%d, Gz=%d\n",
                   mpu6050_result[0], mpu6050_result[1], mpu6050_result[2],
                   mpu6050_result[3], mpu6050_result[4], mpu6050_result[5]);

    if (copy_to_user(buf, str, len + 1)) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }

    printk(KERN_INFO "mpu6050 read data: X=%d, Y=%d, Z=%d, Gx=%d, Gy=%d, Gz=%d\n",
           mpu6050_result[0], mpu6050_result[1], mpu6050_result[2],
           mpu6050_result[3], mpu6050_result[4], mpu6050_result[5]);

    *ppos += len + 1;
    return len + 1;
}

static int mpu6050_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mpu6050 device released\n");
    return 0;
}

static struct file_operations mpu6050_fops = {
    .owner = THIS_MODULE,
    .open = mpu6050_open,
    .read = mpu6050_read,
    .release = mpu6050_release,
};

static int mpu6050_probe(struct i2c_client *client)
{
    int ret;
    printk(KERN_INFO "match success\n");

    ret = alloc_chrdev_region(&mpu6050_devno, 0, DEV_CNT, DEV_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate chrdev region: %d\n", ret);
        goto alloc_chrdev_region_error;        
    }

    cdev_init(&mpu6050_chr_dev, &mpu6050_fops);
    mpu6050_chr_dev.owner = THIS_MODULE;
    ret = cdev_add(&mpu6050_chr_dev, mpu6050_devno, DEV_CNT);
    if (ret < 0) {
        printk(KERN_ERR "Failed to add cdev: %d\n", ret);
        goto cdev_add_error;        
    }
    
    mpu6050_class = class_create(DEV_NAME);
    if (IS_ERR(mpu6050_class)) {
        ret = PTR_ERR(mpu6050_class);
        printk(KERN_ERR "Failed to create class: %d\n", ret);
        goto class_create_error;
        return ret; 
    }   
    device_mpu6050 = device_create(mpu6050_class, NULL, mpu6050_devno, NULL, DEV_NAME);
    if (IS_ERR(device_mpu6050)) {
        ret = PTR_ERR(device_mpu6050);
        printk(KERN_ERR "Failed to create device: %d\n", ret);
        goto device_create_error;
        return ret;
    }
    mpu6050_client = client;
    i2c_set_clientdata(client, NULL); 

    return 0;

device_create_error:
    class_destroy(mpu6050_class);
class_create_error:    
    cdev_del(&mpu6050_chr_dev);
cdev_add_error:
    unregister_chrdev_region(mpu6050_devno, DEV_CNT);
alloc_chrdev_region_error:
    return ret;
}

static void mpu6050_remove(struct i2c_client *client)
{
    device_destroy(mpu6050_class, mpu6050_devno);
    class_destroy(mpu6050_class);
    cdev_del(&mpu6050_chr_dev);
    unregister_chrdev_region(mpu6050_devno, DEV_CNT);    
}

static const struct i2c_device_id mpu6050_id[] = {
    { "rpi3b,i2c_mpu6056", 0 },
    { }
};

static const struct of_device_id mpu6050_of_match[] = {
    { .compatible = "rpi3b,i2c_mpu6056" },
    { }
};

struct i2c_driver mpu6050_driver = {
    .driver = {
        .name = "rpi3b,i2c_mpu6056",
        .owner = THIS_MODULE,
        .of_match_table = mpu6050_of_match,
    },
    .probe = mpu6050_probe,
    .remove = mpu6050_remove,
    .id_table = mpu6050_id,
};

module_i2c_driver(mpu6050_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");