#include <linux/ioctl.h>
#include <linux/kdev_t.h> /* for MKDEV */


#define DEVICE_NAME "okernel"
#define DEVICE_PATH "/dev/okernel"
#define MAGIC_NO '4'

/* Rudimentary protected memory user space ioctl interface */
#define OKERNEL_ALLOCATE_PROTECTED_PAGE _IOR(MAGIC_NO, 1, unsigned long)
#define OKERNEL_FREE_PROTECTED_PAGE     _IOW(MAGIC_NO, 2, unsigned long)
#define OKERNEL_PROTECTED_PAGE_READ     _IOW(MAGIC_NO, 3, char *)
#define OKERNEL_PROTECTED_PAGE_WRITE    _IOW(MAGIC_NO, 4, char *)

#define BUFLEN 4096

struct protected_data {
        /* This is a physical address we will ask the kernel vuln module to
           access.
        */
        unsigned long p_addr;
        /*
           This should be a pointer to a PAGESIZE buffer. The kernel vuln module
           will try and copy the data from the given physical address into this
           buffer.
        */
        char *p_data;
};