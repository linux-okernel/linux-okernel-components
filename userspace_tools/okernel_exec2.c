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
 * Exec a process to run under a outer / inner kernel split.
 * This version uses an ioctl interface to switch to okernel (NR) mode.
 * C I Dalton 2015
 */

#include <stdio.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
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

int our_init_data = 30;
int our_noinit_data;

void usage(void)
{
	printf("okernel_exec2 'executable filename'\n");
	return;
}

void our_prints(void)
{
         int our_local_data = 1;
         printf("\nPid of the process is = %d", getpid());
         printf("\nAddresses which fall into:");
         printf("\n 1) Data  segment = %p",
                 &our_init_data);
         printf("\n 2) BSS   segment = %p",
                 &our_noinit_data);
         printf("\n 3) Code  segment = %p",
                 &our_prints);
         printf("\n 4) Stack segment = %p\n",
                 &our_local_data);

 }

int main(int argc, char **argv)
{
	pid_t pid;
	int fd;

	if(argc < 2){
		usage();
		return 0;
	}
	DPRINTF("arg (%s)\n", argv[1]);
	
	pid = getpid();

	DPRINTF("started (%s) with pid (%ld)\n", argv[0], (long)pid);

	if((fd = open(DEVICE_PATH, O_RDWR)) == -1 ){
		LOG("Failed to open (%s) errno (%d)\n", DEVICE_PATH, errno);
		return -1;
	}

	DPRINTF("About to call OKERNEL_ON_CMD ioctl...\n");

	ioctl(fd, OKERNEL_ON_CMD, 1);

	asm volatile("xchg %bx, %bx");
	
	printf("Returned sucessefully from OKERNEL_ON_CMD ioctl.\n");

	asm volatile("xchg %bx, %bx");

	close(fd);

	asm volatile("xchg %bx, %bx");


	printf("Current memory segments:\n");
	our_prints();
	printf("Current memory segments done.\n");
	
	printf("About to exec (%s)\n", argv[1]);

	asm volatile("xchg %bx, %bx");
	
	/* Exec program specified in the cmd line */

	if(execvp(argv[1], &argv[1]) < 0){
		LOG("execvp failed (%s) errno (%d)\n", argv[1], errno);
		return -1;
	}
	
	/* Shouldn't get here! */
	printf("Shouldn't get here!\n");
	return 0;
}

