# Device-Driver Project
**Background**:

The Linux operating system consists of the Kernel some other minor components and then a large number of Device Drivers.  Device drivers are the key to how various hardware devices interface with the computer.

**Task**:

Develop a device driver that can be loaded and run in Linux.  Then add some functionality to the device driver such as the user/application passing in a string to the device driver and the device driver returns an encrypted version of the string or passes in the excrypted string and returns the original string.  Include a document on how to build, load and interact with the device driver (after the description in the write up template) along with screen shots of output.

**Requirements**:
Written in C.  It must be a valid an loadable device driver with at least some user/application functionality. That includes an open, release, read, write, and at least one ioctl command.  It must also be able to be unloaded, and indicate that it has unloaded from the system.  Make use of the printk and copy_to_user functions.  The read and write functions should follow the concept of linux files and be relevent to reading or writing.

Part of the grading of the driver will be based on the functionality you choose to implement.  It can not be trivial functionality.

This must be run in the linux virtual machine.

You must also write a user application that utilizes your device driver.

There are TWO directories in your GitHub:  The first is `Module` that has your kernel module.  The second is `Test` that will have your test user application.

The writeup should have a detailed description of what your device driver does and clear instructions on how to build your kernel module and your test program and how to install the kernel module and how to use your test program.


	

	
