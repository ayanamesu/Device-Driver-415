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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cesarCipher"
#define BUF_SIZE 100
#define KEY 3

MODULE_DESCRIPTION("A simple cesar cipher driver");
MODULE_LICENSE("GPL");

static int numofOpens = 0;
static char kernel_buffer[BUF_SIZE] = {0};

// Function prototypes
static int devOpen(struct inode *, struct file *);
static int devRelease(struct inode *, struct file *);
static ssize_t devRead(struct file *, char *, size_t, loff_t *);
static ssize_t devWrite(struct file *, const char *, size_t, loff_t *);
static long devIoctl(struct file *fs, unsigned int command, unsigned long data);

// File operations structure
struct file_operations fops = {
    .open = devOpen,
    .release = devRelease,
    .read = devRead,
    .write = devWrite,
    .unlocked_ioctl = devIoctl,
};

static int __init cesarCipher_init(void) {
    int result;
    result = register_chrdev(0, DEVICE_NAME, &fops);
    if (result < 0) {
        printk(KERN_ERR "Failed to register with a device %d\n", result);
        return result;
    }
    return 0;
}

// Opens the function
static int devOpen(struct inode *inode, struct file *fs) {
    numofOpens++;
    printk(KERN_INFO "ccDevice #%d is opening\n", numofOpens);
    return 0;
}

static int devRelease(struct inode *inode, struct file *fs) {
    printk(KERN_INFO "ccDevice closing\n");
    return 0;
}

static ssize_t devRead(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int err;

    if (*offset >= BUF_SIZE) {
        return 0; // End of file
    }

    if (len > BUF_SIZE - *offset) {
        len = BUF_SIZE - *offset; // Read only available bytes
    }

    err = copy_to_user(buffer, kernel_buffer + *offset, len);

    if (err != 0) {
        printk(KERN_INFO "devRead: copy_to_user failed: %d bytes failed to copy\n", err);
        return -1;
    }

    *offset += len;
    return len;
}

static ssize_t devWrite(struct file *file, const char *buf, size_t len, loff_t *off) {
    int err;

    if (len > BUF_SIZE - *off) {
        printk(KERN_INFO "devWrite: input exceeds buffer size\n");
        return -1;
    }

    err = copy_from_user(kernel_buffer + *off, buf, len);

    if (err != 0) {
        printk(KERN_INFO "devWrite: copy_from_user failed: %d bytes failed to copy\n", err);
        return -1;
    }

    // Encrypt the data with Caesar cipher (shift of 3)
    int i;
    for (i = 0; i < len; i++) {
        kernel_buffer[*off + i] = ((kernel_buffer[*off + i] - KEY) % 128 + 128) % 128;
    }

    return len;
}

static long devIoctl(struct file *fs, unsigned int command, unsigned long data) {
    printk(KERN_INFO "devIoctl!");
    int i;

    switch (command) {
        case 0: // ENCRYPT
            // Encrypt the entire buffer with Caesar cipher (shift of 3)
            for (i = 0; i < BUF_SIZE; i++) {
                kernel_buffer[i] = ((kernel_buffer[i] - KEY) % 128 + 128) % 128;
            }
            break;
        case 1: // DECRYPT
            // Decrypt the entire buffer with Caesar cipher (shift of 3)
            for (i = 0; i < BUF_SIZE; i++) {
                kernel_buffer[i] = (kernel_buffer[i] + KEY) % 128;
            }
            break;
        default:
            printk(KERN_ERR "devIoctl invalid command.");
            return -1; // Error code
    }

    return 0;
}

static void __exit cesarCipher_exit(void) {
    unregister_chrdev(0, DEVICE_NAME);
}

module_init(cesarCipher_init);
module_exit(cesarCipher_exit);