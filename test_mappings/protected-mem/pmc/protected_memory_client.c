/*
 Copyright [2015-2017] Hewlett Packard Enterprise Development LP

This program is free software; you can redistribute it and/or modify it under the terms 
of version 2 of the GNU General Public License as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; 
if not, write to:
  Free Software Foundation, Inc.
  51 Franklin Street, Fifth Floor
  Boston, MA 02110-1301, USA.
*/


#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include "okernel.h"



// C.I.Dalton Oct 2016
// Quick hack client program to excercise the okernel rudimentary protected memory interface.

void sigint_handler(int signal)
{
	// Need to install this as we may be pid=1 when run in a container.
	// (Default sig handling different if pid=1)
	printf("Caught signal, exiting.\n");
	exit(0);
}

int main(int argc, char **argv)
{
	char buffer[BUFLEN] = "Something Secret";
	unsigned long phys_addr = 0;
	int fd;
	struct protected_data pd;
	pid_t pid;

	signal(SIGINT, sigint_handler);
	
	pid = getpid();
	
	printf("Starting:=(%s) pid:=(%d)\n", argv[0], pid);

	if((fd = open(DEVICE_PATH, O_RDWR)) == -1){
		printf("Failed to open ioctl device (%s) errno (%d)\n", DEVICE_PATH, errno);
		return -1;
	}

	/* 
	  This asks the kernel for a protected memory page that we will try and access 
	  later via a faked-up kernel vulnerability from another process.
	*/
	if((ioctl(fd, OKERNEL_ALLOCATE_PROTECTED_PAGE, &phys_addr)) == -1){
		printf("IOCTL failed - errno (%d)\n", errno);
		return -1;
	}
	
	printf("Phys addr of allocated protected page:=(%#lx)\n", phys_addr);

	// Write something to the allocated page...
	pd.p_data = buffer;
	pd.p_addr = phys_addr;;
	
	printf("Data to write to protected page:=(%s)\n", pd.p_data);
	
	if((ioctl(fd, OKERNEL_PROTECTED_PAGE_WRITE, &pd)) == -1){
		printf("IOCTL failed - errno (%d)\n", errno);
		return -1;
	}

	printf("Written data to protected page ok.\n");
	
	memset(buffer, 0, BUFLEN);

	if((ioctl(fd, OKERNEL_PROTECTED_PAGE_READ, &pd)) == -1){
		printf("IOCTL failed - errno (%d)\n", errno);
		return -1;
	}
	
	printf("Read back protected page data:=(%s)\n", buffer);
	printf("Just wait now until we are quit...\n");
	pause();
	close(fd);
	return 0;
}