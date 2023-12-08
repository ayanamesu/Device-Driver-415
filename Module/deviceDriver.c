/**************************************************************
* Class:  CSC-415-03 Fall 2023
* Name:Wing Lee
* Student ID:920558688
* GitHub ID:ayanamesu
* Project: Assignment 6 – Device Driver
*
* File:deviceDriver.c
*
* Description: This is an encryption device driver.  It takes
* an user input and encrypt it using a cesar cipher.
*
**************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h> // kmalloc()
#include <linux/uaccess.h>


#define DEVICE_NAME "caeserCipher"
#define BUFFER_SIZE 256
#define KEY 3  // default key

#define ENCRYPT _IO('e', 0)
#define DECRYPT _IO('e', 1)
#define SETKEY _IO('e', 2)

static ssize_t devRead(struct file * fs, char __user * buf, size_t len, loff_t * off);
static ssize_t devWrite(struct file * fs, const char __user * buf, size_t len, loff_t * off);
static int devOpen(struct inode * inode, struct file * fs);
static int devRelease(struct inode * inode, struct file * fs);
static int encrypt(int key);
static int decrypt(int key);
static long devIoCtl (struct file * fs, unsigned int command, unsigned long data);

char *kernel_buffer;

struct cdev my_cdev;
dev_t dev;

MODULE_DESCRIPTION("A simple caesar cipher device driver");
MODULE_LICENSE("GPL");

// file operations structure
struct file_operations fops = {
    .open = devOpen,
    .release = devRelease,
    .write = devWrite,
    .read = devRead,
    .unlocked_ioctl = devIoCtl
};

// structure used for holding the encryption key and a flag 
// that tells us if its encrypted or decrypted.
typedef struct encds {
    int key;
    int flag; 
} encds;

// init to initialize the device driver 
int init_module(void) {
    int ret;
    dev_t dev;
    struct class *dev_class;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if(ret < 0) {
        printk(KERN_ERR "Error in major number");
        return ret;
    }
    printk("Major for dev = %d\n", MAJOR(dev));
    
    dev_class = class_create(THIS_MODULE, "caeserCipher");
    if (dev_class < 0)
    {
        printk("Cannot create the struct class for device\n");
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    cdev_init(&my_cdev, &fops);

    kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (kernel_buffer == NULL) {
        printk(KERN_ERR "Failed to kmalloc kernel_buffer.\n");
        return -1;
    }

    ret = cdev_add(&my_cdev, dev, 1);
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to add character device\n");
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "caeserCipher")))
    {
        pr_err("Cannot create the device...\n");
        class_destroy(dev_class);
        return -1;
    } 
    return ret;
}

// this open function initialized the encds structure and saves it in the
// private data fs.
static int devOpen(struct inode * inode, struct file * fs) {
    struct encds * enc_data;

    enc_data = kmalloc(sizeof(struct encds), GFP_KERNEL);

    if (enc_data == 0) {
        printk(KERN_ERR "Cannot kmalloc, File not opened.\n");
        return -1;
    }

    enc_data->key = KEY;
    enc_data->flag = 0;
    fs->private_data = enc_data;
    return 0;
}

//write function takes in the buffer from user space and brings it into
// kernel space.  then it encrypts it in the kernel buffer.
static ssize_t devWrite (struct file * fs, const char __user * buf, size_t len, loff_t * off) {
    int err;
    struct encds * enc_data;

    printk(KERN_INFO " inside devWrite\n");

    enc_data = (struct encds *) fs->private_data;

    err = copy_from_user(kernel_buffer + *off, buf, len);

    printk(KERN_SOH "WRITE: Offset: %lld", *off);

    if (err != 0) {
        printk(KERN_ERR "encrypt failed: %d \n", err);
        return -1;
    }

    encrypt(enc_data->key);

    enc_data->flag = 1;

    return len;
}

// this read function decrypts the data in the kernel buffer then copies it back
// to the user space.
static ssize_t devRead (struct file * fs, char __user * buf, size_t len, loff_t * off) {
    int err, bufLen;
    struct encds * enc_data;

    // printk(KERN_INFO "" inside devRead");

    enc_data = (struct encds *) fs->private_data;

    bufLen = strlen(kernel_buffer);

    printk(KERN_SOH "READ: Buffer Length: %d    Offset: %lld", bufLen, *off);

    if (bufLen < len) {
        len = bufLen;
    }

    
    err = copy_to_user(buf, kernel_buffer + *off, len);
    
    if (err != 0) {
        printk(KERN_ERR "decrypt failed: %d \n", err);
        return -1;
    }

    return 0;
}

//devRelease function frees the file system data
static int devRelease(struct inode * inode, struct file * fs) {
    struct encds * enc_data;

    enc_data = (struct encds *) fs->private_data;
    vfree(enc_data);
    return 0;
}

// Caesar cipher encryption function
static int encrypt(int key) {
    int i, bufLen;

    bufLen = strlen(kernel_buffer);

    for (i = 0; i < bufLen; i++) {
        kernel_buffer[i] = (kernel_buffer[i] + key - 1) % 128;
    }

    printk(KERN_INFO "Encrypted Text:\n%s\n", kernel_buffer);

    return 0;
}

// Caesar cipher decryption function
static int decrypt(int key) {
    int i, bufLen;

    bufLen = strlen(kernel_buffer);

    for (i = 0; i < bufLen; i++) {
        kernel_buffer[i] = (kernel_buffer[i] - key + 128 ) % 128;
    }

    printk(KERN_INFO "Decrypted Text:\n%s\n", kernel_buffer);

    return 0;
}

// this is a way to deal with device files where the data has already been
// encrypted/decrypted; it also allows for the key to be reset
static long devIoCtl (struct file * fs, unsigned int command, unsigned long data) {
    struct encds * enc_data;

    printk(KERN_INFO "inside myIoCtl");

    enc_data = (struct encds *) fs->private_data;
    switch (command) {
        case ENCRYPT:
            encrypt(enc_data->key);
            enc_data->flag = 1;
            break;
        case DECRYPT:
            decrypt(enc_data->key);
            enc_data->flag = 0;
            break;
        case SETKEY:
            enc_data->key = (int) data;
            break;
        default:
            printk(KERN_ERR "myIoCtl: invalid command entered");
    }
    return 0;
}
// removes and destroys module
void cleanup_module(void) {
    unregister_chrdev_region(dev, 1);
    cdev_del(&my_cdev);
    kfree(kernel_buffer);
    kernel_buffer = NULL;

    printk(KERN_INFO "closing from deviceDriver.\n");
}
