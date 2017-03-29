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
	printf("okernel_test2 src_filename dst_filename\n");
	return;
}

int main(int argc, char **argv)
{
	int a = 0;
	pid_t pid;
	pid_t ppid;
	int fd;
	DIR *dp;
	struct dirent *dirp;
	char* filename;
	FILE *fp;
	char ch;
	FILE *src;
	FILE *dst;
	char buf[255];
	int n;
	char *src_name;
	char *dst_name;
	
	if(argc != 3){
		usage();
		return 0;
	}

	src_name = argv[1];
	dst_name = argv[2];
	
	DPRINTF("arg src (%s) dst (%s)\n", src_name, dst_name);

	filename = (char*)malloc(PATH_MAX+1);
	
		
	pid = getpid();

	DPRINTF("started (%s) with pid (%ld)\n", argv[0], (long)pid);

#if 1
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
#endif
	asm volatile("xchg %bx, %bx");
	
	a = a + 1;
	
	printf("Just print out value of a (%i)\n", a);
	
	asm volatile("xchg %bx, %bx");

	printf("Try another syscall...\n");

	asm volatile("xchg %bx, %bx");

	ppid = getppid();

	asm volatile("xchg %bx, %bx");
	
	printf("Parent pid (%ld) from getppid()\n", (long)ppid);

	asm volatile("xchg %bx, %bx");

#if 0
	printf("About to sleep for 5s after next breakpoint...\n");

	asm volatile("xchg %bx, %bx");
	
	sleep(5);
#endif	
	// Now list the directory contents
	printf("Opening directory '.'\n");
	
	asm volatile("xchg %bx, %bx");
	
	dp = opendir(".");
	
	asm volatile("xchg %bx, %bx");
	
	printf("Directory Opened.\n");
	
	asm volatile("xchg %bx, %bx");
	
	if(dp == NULL){
		printf("failed to open '.' directory.\n");
		return 0;
	}
		
	//asm volatile("xchg %bx, %bx");
	
	while((dirp = readdir(dp)) != NULL){
		memset(filename, 0, PATH_MAX+1);
		strcpy(filename, dirp->d_name);
		printf("File: %s\n", filename);
		//asm volatile("xchg %bx, %bx");
	}
	
	printf("Done directory listing.\n");
	asm volatile("xchg %bx, %bx");
	
	printf("Done directory listing.\n");
	
	fp = fopen(argv[1], "r");
	
	if(!fp){
		printf("Failed to open (%s) for reading.\n", argv[1]);
		return 0;
	}
	
	printf("Opened %s, read next...\n", argv[1]);
	
	asm volatile("xchg %bx, %bx");
	
	if(fp == NULL)
	{
		printf("Given NULL filename to open.\n");
		return 0;
	}
	
	
	printf("Reading file %s...\n", argv[1]);
	asm volatile("xchg %bx, %bx");
	
	while ((ch = fgetc(fp))!=EOF){
		printf("%c",ch);
	}
	printf("Done reading %s.\n", argv[1]);
	
	fclose(fp);

	// Ok this time copy src into dst file
	src = fopen(src_name, "rb");
	
	if(!src){
		printf("Failed to open (%s) for reading.\n", src_name);
		return 0;
	}
	
	dst = fopen(dst_name, "wb");
	
	if(!dst){
		printf("Failed to open (%s) for writing.\n", dst_name);
		return 0;
	}
	
	while((n=fread(buf, 1, 255, src)))
	{
		fwrite(buf, 1, n, dst);
	}
	
	fclose(src);
	fclose(dst);
	
	printf("Now exit...\n");
	
	asm volatile("xchg %bx, %bx");
	
	return 0;
}
