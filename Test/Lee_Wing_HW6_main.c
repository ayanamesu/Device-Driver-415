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

#define ENCRYPT _IO('e', 0)
#define DECRYPT _IO('e', 1)
#define SETKEY _IO('e', 2)

#define SIZE 256

int main() {
  int key, fd;
  char *text = malloc(SIZE);
  char *res = malloc(SIZE);

  while (1) {
    printf("Opening the deviceDriver\n");

    fd = open("/dev/cesarCipher", O_RDWR);
    if (fd < 0) {
      perror("failed to open device");
      return fd;
    }

    printf("Enter a key for encryption: ");
    scanf("%d", &key);
    getc(stdin);

    // ioctl to set the key for the device driver
    ioctl(fd, SETKEY, key);

    printf("Enter some characters to be encrypted:\n");
    scanf("%[^\n]s", text);
    getc(stdin);

    printf("Length: %ld\n", strlen(text));

    printf("This is the text you are encrypting:\n%s\n", text);

    write(fd, text, strlen(text));
    read(fd, res, strlen(text));

    printf("\nEncrypted text:\n%s\n\n", res);
    printf("Decrypting text\n\n");

    // ioctl to decrypt the data on the driver (the kernel buffer)
    ioctl(fd, DECRYPT);

    read(fd, res, strlen(text));

    printf("Decrypted text: \n%s\n\n", res);


    printf("Closing deviceDriver\n");

    close(fd);

  }
}