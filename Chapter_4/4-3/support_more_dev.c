#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define DEV_NAME    "EmbedCharDev"
#define DEV_CNT     (2)
#define BUFF_SIZE   128 

static dev_t devno;

struct chr_dev{
    struct cdev dev;
    char vbuf[BUFF_SIZE];
};

static struct chr_dev vcdev1;
static struct chr_dev vcdev2;

static int chr_dev_open(struct inode *inode, struct file *filp);
static int chr_dev_release(struct inode *inode, struct file *filp);
static ssize_t chr_dev_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos);
static ssize_t chr_dev_read(struct file *filp, char __user *buf, 
    size_t count, loff_t *ppos);
static struct file_operations chr_dev_fops = 
{
    .owner = THIS_MODULE,
    .open = chr_dev_open,
    .release = chr_dev_release,
    .write = chr_dev_write,
    .read = chr_dev_read
};

static int chr_dev_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "chardev opened.\n");

    filp->private_data = container_of(inode->i_cdev, struct chr_dev, dev);

    return 0;
}

static int chr_dev_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "chardev released.\n");
    return 0;
}

static ssize_t chr_dev_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret;
    int tmp = count;
    struct chr_dev *dev = filp->private_data;
    char* vbuf = dev->vbuf;

    if( p > BUFF_SIZE )
        return 0;
    
    if( tmp > BUFF_SIZE - p )
        tmp = BUFF_SIZE - p;
    
    ret = copy_from_user(vbuf + p, buf, tmp);
    if(ret < 0)
        return -EFAULT;
    
    *ppos += tmp;

    return tmp;
}

static ssize_t chr_dev_read(struct file *filp, char __user *buf, 
    size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret;
    int tmp = count;
    struct chr_dev *dev = filp->private_data;
    char* vbuf = dev->vbuf;
    
    if( p >= BUFF_SIZE )
        return 0;

    if( tmp > BUFF_SIZE - p)
        tmp = BUFF_SIZE - p;
    
    ret = copy_to_user(buf, vbuf + p, tmp);
    if(ret < 0)
        return -EFAULT;

    *ppos += tmp;
    return tmp;
}

static int __init chrdev_init(void)
{
    int ret = 0;
    printk(KERN_INFO "chardev init\n");

    ret = alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);
    if( ret < 0){
        printk(KERN_ERR "failed to alloc devno.\n");
        goto alloc_err;
    }
    
    cdev_init(&vcdev1.dev, &chr_dev_fops);
    ret = cdev_add(&vcdev1.dev, devno + 0, 1);
    if(ret < 0){
        printk(KERN_ERR "failed to add cdev.\n");
        goto add_err1;
    }
    
    cdev_init(&vcdev2.dev, &chr_dev_fops);
    ret = cdev_add(&vcdev2.dev, devno + 1, 1);
    if(ret < 0){
        printk(KERN_ERR "failed to add cdev.\n");
        goto add_err2;
    }


    return 0;

add_err2:
    cdev_del(&vcdev1.dev);

add_err1:
    unregister_chrdev_region(devno, DEV_CNT);    

alloc_err:
    return ret;
}

static void __exit chrdev_exit(void)
{
    printk(KERN_INFO "chrdev exit\n");
    
    cdev_del(&vcdev1.dev);
    cdev_del(&vcdev2.dev);
    unregister_chrdev_region(devno, DEV_CNT);    
}

module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL2");
MODULE_AUTHOR("Frank Lin");
MODULE_DESCRIPTION("cdev module");
MODULE_ALIAS("cdev_module");