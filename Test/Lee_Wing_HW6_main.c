/**************************************************************
* Class:  CSC-415-03 Fall 2023
* Name:Wing Lee
* Student ID:920558688
* GitHub ID:ayanamesu
* Project: Assignment 6 – Device Driver
*
* File:Lee_Wing_HW6_main.c
*
* Description: This is an encryption device driver.  It takes
* an user input and encrypt it using a cesar cipher.
*
**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>


#define SIZE 50

static char receive[SIZE];

int main() {
    int fd;
    int ret;
    char stringSize[SIZE];
    fd = open("/dev/cesarCipher", O_RDWR);

    if(fd < 0) {
        printf(" Failed to open device driver.\n");
        perror("Open Error on device");
        return errno;
    } else {
        printf(" Successfully opened the Device.\n");
    }

    printf("\nPlease enter any Input String: \n");
    scanf("%[^\n]%*c", stringSize);

    if(strlen(stringSize) > SIZE) {
        printf("input exceeds length size");
        close(fd);
        return(-1);
    }

    printf("\nMessage writting to device: %s \n", stringSize);
    ret = write(fd, stringSize, strlen(stringSize));

    if(ret < 0) {
        perror("Failed to write to device.");
        return errno;
    }
    int ioctlCommand = 0;
    ret = ioctl(fd, ioctlCommand, 0);


    printf("Reading from device.\n");
    ret = read(fd, receive, SIZE);
    if(ret < 0) {
        perror("failed to read from device.\n");
        return errno;
    }
    printf("The original message is: %s \n", receive);

    printf("End of program.\n");
    return 0;




}