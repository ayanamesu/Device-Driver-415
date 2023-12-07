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
#include <linux/kmalloc.h>
#include <linux/uaccess.h>


#define DEVICE_NAME "cesarCipher"
#define BUFFER_SIZE 512
#define KEY 5  // default key

#define ENCRYPT _IO('e', 0)
#define DECRYPT _IO('e', 1)
#define SETKEY _IO('e', 2)

static ssize_t devRead(struct file * fs, char __user * buf, size_t hsize, loff_t * off);
static ssize_t devWrite(struct file * fs, const char __user * buf, size_t hsize, loff_t * off);
static int devOpen(struct inode * inode, struct file * fs);
static int devRelease(struct inode * inode, struct file * fs);
static int encrypt(int key);
static int decrypt(int key);
static long devIoCtl (struct file * fs, unsigned int command, unsigned long data);

char *kernel_buffer;

struct cdev my_cdev;
dev_t dev;

MODULE_DESCRIPTION("A simple encryption/decryption program");
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
    int flag; // 0 = unencrypted, 1 = encrypted
} encds;

// creates a device node in /dev, returns error if not made
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
    
    dev_class = class_create(THIS_MODULE, "CaeserCipher");
    if (dev_class < 0)
    {
        printk("Cannot create the struct class for device\n");
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    cdev_init(&my_cdev, &fops);

    kernel_buffer = kmalloc(BUFFER_SIZE);
    if (kernel_buffer == NULL) {
        printk(KERN_ERR "Failed to kmalloc kernel_buffer.\n");
        return -1;
    }

    ret = cdev_add(&my_cdev, dev, 1);

    return ret;
}

// this open function initialized the encds structure and saves it in the
// logs fs.
static int devOpen(struct inode * inode, struct file * fs) {
    struct encds * ds;

    ds = kmalloc(sizeof(struct encds));

    if (ds == 0) {
        printk(KERN_ERR "Cannot kmalloc, File not opened.\n");
        return -1;
    }

    ds->key = KEY;
    ds->flag = 0;
    fs->logs = ds;
    return 0;
}

//write function takes in the buffer from user space and brings it into
// kernel space.  then it encrypts it in the kernel buffer.
static ssize_t devWrite (struct file * fs, const char __user * buf, size_t hsize, loff_t * off) {
    int err;
    struct encds * ds;

    printk(KERN_INFO " inside devWrite\n");

    ds = (struct encds *) fs->logs;

    err = copy_from_user(kernel_buffer + *off, buf, hsize);

    printk(KERN_SOH "WRITE: Offset: %lld", *off);

    if (err != 0) {
        printk(KERN_ERR "encrypt failed: %d \n", err);
        return -1;
    }

    encrypt(ds->key);

    ds->flag = 1;

    return hsize;
}

// this read function decrypts the data in the kernel buffer then copies it back
// to the user space.
static ssize_t devRead (struct file * fs, char __user * buf, size_t hsize, loff_t * off) {
    int err, bufLen;
    struct encds * ds;

    // printk(KERN_INFO "" inside devRead");

    ds = (struct encds *) fs->logs;

    bufLen = strlen(kernel_buffer);

    printk(KERN_SOH "READ: Buffer Length: %d    Offset: %lld", bufLen, *off);

    if (bufLen < hsize) {
        hsize = bufLen;
    }

    
    err = copy_to_user(buf, kernel_buffer + *off, hsize);
    
    if (err != 0) {
        printk(KERN_ERR "decrypt failed: %d \n", err);
        return -1;
    }

    return 0;
}

//devRelease function frees the file system logs
static int devRelease(struct inode * inode, struct file * fs) {
    struct encds * ds;

    ds = (struct encds *) fs->logs;
    vfree(ds);
    return 0;
}

// caesar cypher encryption function
static int encrypt(int key) {
    int i, bufLen;

    bufLen = strlen(kernel_buffer);

    for (i = 0; i < bufLen - 1; i++) {
        kernel_buffer[i] = ((kernel_buffer[i] - key) % 128) + 1;
    }

    printk(KERN_INFO "Encrypted Text:\n%s\n", kernel_buffer);

    return 0;
}

// decrypt function
static int decrypt(int key) {
    int i, bufLen;

    bufLen = strlen(kernel_buffer);

    for (i = 0; i < bufLen - 1; i++) {
        kernel_buffer[i] = (kernel_buffer[i] + key - 1) % 128;
    }

    printk(KERN_INFO "Decrypted Text:\n%s\n", kernel_buffer);

    return 0;
}

// this is a way to deal with device files where the data has already been
// encrypted/decrypted; it also allows for the key to be reset
static long devIoCtl (struct file * fs, unsigned int command, unsigned long data) {
    struct encds * ds;

    printk(KERN_INFO "inside myIoCtl");

    ds = (struct encds *) fs->logs;
    switch (command) {
        case ENCRYPT:
            encrypt(ds->key);
            ds->flag = 1;
            break;
        case DECRYPT:
            decrypt(ds->key);
            ds->flag = 0;
            break;
        case SETKEY:
            ds->key = (int) data;
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

    vfree(kernel_buffer);
    kernel_buffer = NULL;

    printk(KERN_INFO "closing from deviceDriver.\n");
}