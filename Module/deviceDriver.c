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


#define DEVICE NAME "cesarCipher"
#define BUF_SIZE 100
#define KEY 3


MODULE_LICENSE("GPL");

static int numofOpens = 0;
//file operations  structure
static struct file_operations fops = {
    .open = devOpen,
    .release = devRelease,
    .read = devRead,
    .write = devWrite,
    .unlocked_ioctl = devIoctl,
};
//functions 
static int devOpen(struct inode *, struct file *);
static int devRelease(struct inode *, struct file *);
static ssize_t devRead(struct file *, char *, size_t, loff_t *);
static ssize_t devWrite(struct file *,const char *, size_t, loff_t *);
static long devIoctl(struct file *fs, unsigned int command, unsigned long data);


static int __init cesarCipher_init(void) {
    int result;
    result = register_chrdev(0, DEVICE_NAME, &fops);
    if (result < 0) {
        printk(KERN_ERR "Failed to register with a device %d\n", result);
        return result;
    }
    return 0;
}
//opens the function 
static int devOpen(struct inode * inode, struct file * fs) {
    numofOpens++;
    printk(KERN_INFO "ccDevice #%d is opening\n",numofOpens);
    return 0;
}

static int devRelease(struct inode * inode, struct file * fs) {
     printk(KERN_INFO "ccDevice closing\n");
     return 0;
}

static long devIoctl(struct file *fs, unsigned int command, unsigned long data) {
    printk(KERN_INFO "devIoctl!");
    switch (command) {
        case 0: // ENCRYPT
            // Encrypt the entire buffer with Caesar cipher (shift of 3)
            for (int i = 0; i < BUF_SIZE; i++) {
                kernel_buffer[i] = ((kernel_buffer[i] - KEY) % 128 + 128) % 128;
            }
            break;
        case 1: // DECRYPT
            // Decrypt the entire buffer with Caesar cipher (shift of 3)
            for (int i = 0; i < BUF_SIZE; i++) {
                kernel_buffer[i] = (kernel_buffer[i] + KEY) % 128;
            }
            break;
        default:
            printk(KERN_ERR "devIoctl invalid command.");
            return -1; // Error code
    }
    return 0;
}