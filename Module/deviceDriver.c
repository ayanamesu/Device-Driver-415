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

static struct file_operations fops = {
    .open = devOpen,
    .release = devRelease,
    .read = devRead,
    .write = devWrite,
    .unlocked_ioctl = devIoctl,
};

static int devOpen(struct inode *, struct file *);
static int devRelease(struct inode *, struct file *);
static ssize_t devRead(struct file *, char *, size_t, loff_t *);
static ssize_t devWrite(struct file *,const char *, size_t, loff_t *);
static long devIoctl(struct file *fs, unsigned int command, unsigned long data);

static int __init