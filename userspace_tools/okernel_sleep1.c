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


/* 
 * Simple program to run as a process to run under a outer / inner
 * kernel split.  This version uses an ioctl interface to switch to
 * okernel (NR) mode. This one sleeps to see if we can provoke the scheduler.
 * 
 * C I Dalton 2015
 */

#include <stdio.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

#include "okernel_ioctl.h"

#ifdef DEBUG
#define DPRINTF(fmt, ...) \
	do { printf("%d:%s " fmt, __LINE__, __FILE__, ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) \
    do { } while (0)
#endif

#define LOG(fmt, ...) \
	do { printf("%d:%s " fmt, __LINE__, __FILE__, ## __VA_ARGS__); } while (0)


void usage(void)
{
	printf("okernel_sleep1 interval\n");
	return;
}

int main(int argc, char **argv)
{
	unsigned long interval;
	pid_t pid;
	int fd;
	

	if(argc != 2){
		usage();
		return 0;
	}

	errno = 0;
	
	interval = strtoul(argv[1], NULL, 10);
	
	if(errno != 0){
		printf("Invalid number conversion for arg (%s)\n", argv[1]);
		return 0;
	}
	
	printf("sleep interval:  (%lu)\n", interval);
	
	pid = getpid();

	printf("started (%s) with pid (%ld)\n", argv[0], (long)pid);

#if 1
	if((fd = open(DEVICE_PATH, O_RDWR)) == -1 ){
		LOG("Failed to open (%s) errno (%d)\n", DEVICE_PATH, errno);
		return -1;
	}

	printf("About to call OKERNEL_ON_CMD ioctl...\n");

	ioctl(fd, OKERNEL_ON_CMD, 1);

	asm volatile("xchg %bx, %bx");
	
	printf("Returned sucessefully from OKERNEL_ON_CMD ioctl.\n");

	asm volatile("xchg %bx, %bx");

	close(fd);
#endif
	asm volatile("xchg %bx, %bx");
	
	printf("About to sleep for (%lu)s after next breakpoint...\n", interval);

	asm volatile("xchg %bx, %bx");
	
	sleep(interval);
	
	printf("Now exit...\n");
	
	asm volatile("xchg %bx, %bx");
	
	return 0;
}
