/*
 * Tracks user space memory using XPFO. Make sure there is no memory
 * which is released and mapped back into the kernel which is still
 * marked as executable.
 *
 * nigel.edwards@hpe.com
 */


#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MAGIC_NO '4'
#define DEVICE_NAME "oktest"
#define DEVICE_PATH "/dev/oktest"

/* 
 * Set the message of the device driver 
 */
#define OKTEST_USER_MEM_TRACK _IO(MAGIC_NO, 1)


int main(int argc, char **argv)
{
	int fd;
	
	if((fd = open(DEVICE_PATH, O_RDWR)) == -1 ){
		printf("Failed to open (%s) errno (%d)\n", DEVICE_PATH, errno);
		return -1;
	}
	printf("Calling OKTEST_USER_MEM_TRACK\n");
	ioctl(fd, OKTEST_USER_MEM_TRACK);
	printf("Calling OKTEST_USER_MEM_TRACK test done. Check kernel logs\n");
}
