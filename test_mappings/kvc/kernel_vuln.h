#include <linux/ioctl.h>
#include <linux/kdev_t.h> /* for MKDEV */

/* Keep this PAGESIZE or smaller */

#define BUFLEN 4096

struct vuln_data {
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

/* For the demo hack kernel vuln ioctl device */
#define VULN_DEVICE_NAME "kernel_vuln"
#define VULN_DEVICE_PATH "/dev/kernel_vuln"
static int major_no;
#define VULN_MAGIC_NO '5'

#define VULN_PHYSICAL_ADDR_CMD _IOW(VULN_MAGIC_NO, 1, char *)
#define VULN_WRITE_PHYSICAL_ADDR _IOWR(VULN_MAGIC_NO, 1, void *)
#define VULN_READ_PHYSICAL_ADDR _IOWR(VULN_MAGIC_NO, 1, void *)


