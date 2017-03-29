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
 * Run simple thread test  under a outer / inner kernel split.
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
#include <pthread.h>
#include <stdlib.h>
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
	printf("okernel_thread1\n");
	return;
}

pthread_t tid[2];

void* doSomeThing(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0]))
    {
        printf("\n First thread processing\n");
    }
    else
    {
        printf("\n Second thread processing\n");
    }

    for(i=0; i<(0xFFFFFFFF);i++);

    return NULL;
}

int main(int argc, char **argv)
{
    int i = 0;
    int err;
    int fd;
    pid_t pid;

    if(argc != 1){
	    usage();
	    return 0;
    }

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
    printf("about to create threads...\n");
	
    asm volatile("xchg %bx, %bx");

    while(i < 2)
    {
        err = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");

        i++;
    }

    sleep(5);
    return 0;
}
