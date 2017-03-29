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
 * Toggle /proc/<pid>/okernel state to 1 (OKERNEL_ON_EXEC), then
 * exec the specified executable file. The kernel will switch on
 * the value of the okernel state flag.
 * C I Dalton 2015
 */

#include <stdio.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "okernel.h"

#define NUMBUF 13

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
	printf("okernel_exec 'executable filename'\n");
	return;
}

int main(int argc, char **argv)
{
	pid_t pid;
	char  fname[PATH_MAX] = {0};
	FILE* proc_file = NULL;
	char  buf[NUMBUF] = {0};
	int n;
	unsigned long p_state = 0;
	
	if(argc < 2){
		usage();
		return 0;
	}
	DPRINTF("arg (%s)\n", argv[1]);

	pid = getpid();

	DPRINTF("pid (%ld)\n", (long)pid);
	
	if((n = snprintf(fname, sizeof(fname), "/proc/%ld/okernel", (long)pid)) >= PATH_MAX){
		LOG("Path too long for buffer.\n");
		return -1;
	}

	DPRINTF("proc file name (%s)\n", fname);

	if((proc_file = fopen(fname, "r")) == NULL){
		LOG("Failed to open proc file (%d)\n", errno);
		return -1;
	}

	/* test sequence: read -> write ->read */

	/* read */
	if((fgets(buf, sizeof(buf), proc_file)) == 0){
		LOG("Failed to read from proc file (%d)\n", errno);
		fclose(proc_file);
		return -1;
	}
	
	p_state = strtoul(buf, NULL, 10);
	DPRINTF("initial okernel state (%lu)\n", p_state);
	
	if(p_state != OKERNEL_OFF){
		LOG("error: okernel state already set to non-zero value.\n");
		return -1;
	}
	p_state = OKERNEL_ON_EXEC;
	memset(buf, 0, NUMBUF);
	snprintf(buf, sizeof(buf), "%lu", p_state);
	DPRINTF("okernel writing state (%lu) buf (%s)\n", p_state, buf);
	fclose(proc_file);
	
	/* write */
	if((proc_file = fopen(fname, "w")) == NULL){
		LOG("Failed to open proc file (%d)\n", errno);
		return -1;
	}

	if((fputs(buf, proc_file)) == EOF){
		LOG("Failed to write to proc file (%d)\n", errno);
		fclose(proc_file);
		return -1;
	}
	fclose(proc_file);

#if 1	
	/* read back to check for consistency */
	memset(buf, 0, sizeof(buf));

	if((proc_file = fopen(fname, "r")) == NULL){
		LOG("Failed to open proc file (%d)\n", errno);
		return -1;
	}
	
	if((fgets(buf, sizeof(buf), proc_file)) == 0){
		LOG("Failed to read from proc file (%d)\n", errno);
		fclose(proc_file);
		return -1;
	}

	p_state = strtoul(buf, NULL, 10);
	DPRINTF("final okernel state (%lu)\n", p_state);

	//if((p_state != OKERNEL_ON_EXEC) && (p_state != OKERNEL_ON)){
	if(p_state != OKERNEL_ON_EXEC){
		LOG("process okernel state not set correctly.\n");
		return -1;
	}
#endif
	/* Exec program specified in the cmd line */

	if(execvp(argv[1], &argv[1]) < 0){
		LOG("execvp failed (%s) errno (%d)\n", argv[1], errno);
		return -1;
	}
	
	/* Shouldn't get here! */
	
	return 0;
}
