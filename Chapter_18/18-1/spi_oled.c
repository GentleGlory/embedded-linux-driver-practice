#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/cdev.h>
#include <linux/fs.h>


#include "spi_oled.h"


#define DEV_NAME    "spi_oled"
#define DEV_CNT     1

static dev_t oled_devno;
static struct cdev oled_chr_dev;
static struct class *class_oled;
struct device *device_oled;


struct spi_device *oled_spi_device = NULL;
struct device_node *oled_device_node;
int oled_control_pin_number;

u8 oled_init_data[] = {
    0xae, 0xae, 0x00, 0x10, 0x40,
	0x81, 0xcf, 0xa1, 0xc8, 0xa6,
	0xa8, 0x3f, 0xd3, 0x00, 0xd5,
	0x80, 0xd9, 0xf1, 0xda, 0x12,
	0xdb, 0x40, 0x20, 0x02, 0x8d,
	0x14, 0xa4, 0xa6, 0xaf
};

#define SPI_SEND_COMMAND_CHECK(client, data) \
    do { \
        int __ret = oled_send_command(client, data); \
        if (__ret < 0) { \
            printk(KERN_ERR "oled_send_command failed at %s:%d, ret=%d\n", __func__, __LINE__, __ret); \
            return __ret; \
        } \
    } while (0)

#define SPI_SEND_DATA_CHECK(client, buffer, length) \
    do { \
        int __ret = oled_send_data(client, buffer, length); \
        if (__ret < 0) { \
            printk(KERN_ERR "oled_send_command failed at %s:%d, ret=%d\n", __func__, __LINE__, __ret); \
            return __ret; \
        } \
    } while (0)


static int oled_send(struct spi_device* dev, u8 command, bool data)
{
    int ret = 0;

    struct spi_transfer t = {
        .tx_buf = &command,
        .len = 1,
    };
    struct spi_message m;
    
    gpio_direction_output(oled_control_pin_number, data ? 1 : 0);
    
    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    ret = spi_sync(dev, &m);
    if (ret < 0) {
        dev_err(&dev->dev, "Failed to send command: %d\n", ret);        
    }

    gpio_direction_output(oled_control_pin_number, data ? 0 : 1);
    
    return ret;

}

static int oled_send_command(struct spi_device* dev, u8 command)
{
    return oled_send(dev, command, false);
}

static int oled_send_commands(struct spi_device* dev, u8 *commands, u16 length)
{
    int err = 0;
    int index;

    for(index = 0; index < length; index++) {
        err = oled_send_command(dev, commands[index]);
        if (err < 0) {
            dev_err(&dev->dev, "Failed to send command %d: %d\n", commands[index], err);
            return err;
        }
    }
    
    return 0;
}

static int oled_send_one_data(struct spi_device* dev, u8 data)
{
    return oled_send(dev, data, true);
}

static int oled_send_data(struct spi_device* dev, u8 *data, u16 length)
{
    int err = 0;
    int index;

    for(index = 0; index < length; index++) {
        err = oled_send_one_data(dev, data[index]);
        if (err < 0) {
            dev_err(&dev->dev, "Failed to send command %d: %d\n", data[index], err);
            return err;
        }
    }
    
    return 0;
}

static void oled_fill(unsigned char bmp_dat)
{
    u8 x, y;

    for(y = 0; y < 8; y++){
        oled_send_command(oled_spi_device, 0xb0 + y);
        oled_send_command(oled_spi_device, 0x01);
        oled_send_command(oled_spi_device, 0x10);

        for(x = 0; x < X_WIDTH; x++){
            oled_send_one_data(oled_spi_device,bmp_dat);
        }
    }
}

static int oled_display_buffer(u8 *display_buffer, u8 x, u8 y, u16 length)
{
    u16 index = 0;
    
    do {
        SPI_SEND_COMMAND_CHECK(oled_spi_device, 0xb0 + y);
        SPI_SEND_COMMAND_CHECK(oled_spi_device, ((x & 0xf0) >> 4) | 0x10);
        SPI_SEND_COMMAND_CHECK(oled_spi_device, (x & 0xf0) | 0x01);

        if(length > (X_WIDTH - x)){
            SPI_SEND_DATA_CHECK(oled_spi_device, display_buffer + index, X_WIDTH - x);
            length -= (X_WIDTH - x);
            index += (X_WIDTH - x);
            x = 0;
            y++;
        }else{
            SPI_SEND_DATA_CHECK(oled_spi_device, display_buffer + index, length);
            index += length;
            length = 0;
        }
    }while(length > 0);

    return index;
} 

static void spi_oled_init(void)
{
    oled_send_commands(oled_spi_device, oled_init_data, sizeof(oled_init_data));

    oled_fill(0x00);
}

static int oled_open(struct inode *inode, struct file *filp)
{
    spi_oled_init();
    return 0;
}

static ssize_t oled_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *off)
{
    int copy_number = 0;

    oled_display_struct *write_data;
    write_data = (oled_display_struct*) kzalloc(cnt, GFP_KERNEL);

    copy_number = copy_from_user(write_data, buf, cnt);

    oled_display_buffer(write_data->display_buffer, write_data->x , write_data->y, write_data->length);

    kfree(write_data);

    return 0;
}

static int oled_release(struct inode *inode, struct file *filp)
{
    oled_send_command(oled_spi_device, 0xae);
    return 0;
}

static struct file_operations oled_chr_dev_fops = {
    .owner = THIS_MODULE,
    .open = oled_open,
    .write = oled_write,
    .release = oled_release,
};


static const struct spi_device_id oled_device_id[] = {
    {"rpi3b,spi_oled", 0},
    {}
};

static const struct of_device_id oled_of_match_table[] = {
    {.compatible = "rpi3b,spi_oled"},
    {}
};

static int oled_probe(struct spi_device *spi)
{
    int ret = 0;
    struct device_node *node = spi->dev.of_node;

    printk(KERN_INFO "match success\n");

    oled_control_pin_number = of_get_named_gpio(node, "dc_control_pin", 0);
    printk(KERN_INFO "oled_control_pin_number = %d\n", oled_control_pin_number);

    gpio_request(oled_control_pin_number, "dc_control_pin");
    gpio_direction_output(oled_control_pin_number,1);
    
    oled_spi_device = spi;
    oled_spi_device->mode = SPI_MODE_3;
    oled_spi_device->max_speed_hz = 2000000;
    spi_setup(oled_spi_device);

    printk(KERN_INFO "max_speed_hz = %d\n", oled_spi_device->max_speed_hz);
    printk(KERN_INFO "bits_per_word = %d\n", (int)oled_spi_device->bits_per_word);
    printk(KERN_INFO "mode = %02X\n", oled_spi_device->mode);
	
    ret = alloc_chrdev_region(&oled_devno, 0, DEV_CNT, DEV_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate chrdev region\n");
        goto alloc_err;
    }

    oled_chr_dev.owner = THIS_MODULE;
    cdev_init(&oled_chr_dev, &oled_chr_dev_fops);

    ret = cdev_add(&oled_chr_dev, oled_devno, 1);
    if (ret < 0) {
        printk(KERN_ERR "Failed to add cdev\n");
        goto add_error;
    }

    class_oled = class_create(DEV_NAME);
    if (IS_ERR(class_oled)) {
        printk(KERN_ERR "Failed to create class\n");
        ret = PTR_ERR(class_oled);
        goto class_create_error;
    }

    device_oled = device_create(class_oled, NULL, oled_devno, NULL, DEV_NAME);
    if (IS_ERR(device_oled)) {
        printk(KERN_ERR "Failed to create device\n");
        ret = PTR_ERR(device_oled);
        goto device_create_error;
    }

    return 0;
    
device_create_error:
    class_destroy(class_oled);
class_create_error:
    cdev_del(&oled_chr_dev);
add_error:
    unregister_chrdev_region(oled_devno,DEV_CNT);
alloc_err:
    gpio_free(oled_control_pin_number);
    return ret;
}

static void oled_remove(struct spi_device *spi)
{
    device_destroy(class_oled,oled_devno);
    class_destroy(class_oled);
    cdev_del(&oled_chr_dev);
    unregister_chrdev_region(oled_devno,DEV_CNT);
    gpio_free(oled_control_pin_number);

    printk(KERN_INFO "oled_removed\n"); 
}


struct spi_driver oled_driver = {
    .probe = oled_probe,
    .remove = oled_remove,
    .id_table = oled_device_id,
    .driver = {
        .name = "spi_oled",
        .of_match_table = oled_of_match_table
    }
};

module_spi_driver(oled_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frank Lin");