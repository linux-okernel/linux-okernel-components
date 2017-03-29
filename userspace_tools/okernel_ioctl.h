#include <sys/ioctl.h> 

#define DEVICE_NAME "okernel"
#define DEVICE_PATH "/dev/okernel"

#define MAGIC_NO '4'

/* 
 * Set the message of the device driver 
 */
#define OKERNEL_ON_CMD _IOW(MAGIC_NO, 0, unsigned int)
