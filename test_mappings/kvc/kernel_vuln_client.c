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
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>
#include "kernel_vuln.h"

// C.I.Dalton Oct 2016
// Quick hack client program to test EPT protection functionality in the okernel.
// Pass a physical address on the command line and try and access it via a kernel
// module (or via an env variable called "KV_PHYSADDR")

void usage(char* pname)
{
	printf("%s 0xPHYSADDR (in hex) [or set KV_PHYSADDR=0xPHYSADDR as env variable]\n", pname);
}

int main(int argc, char **argv)
{
	char *endptr;
		
	char buffer[BUFLEN];
	unsigned long phys_addr = 0;
	int fd;
	struct vuln_data vd;
	char *paddr_str = NULL;
	
	// Physaddr to pass to the module can be set on the cmd line or via an env variable to
	// make docker integration easier.
	if(argc == 1){
		paddr_str = getenv("KV_PHYSADDR");
		if(!paddr_str){
			usage(argv[0]);
			return 0;
		}
	} else if(argc == 2){
		if(!getenv("KV_PHYSADDR")){
			paddr_str = argv[1];		
		} else {
			usage(argv[0]);
			return 0;
		}
		
	} else {
		usage(argv[0]);
		return 0;
	}
	
	errno = 0;
	
	phys_addr =  strtoul(paddr_str, &endptr, 16);
	
	if((errno == ERANGE && (phys_addr == ULONG_MAX)) || (errno != 0 && phys_addr == 0)) {
		printf("Failed to convert:=(%s)\n", paddr_str);
		return 0;
	}
	
	if (endptr == argv[1]) {
		printf("No digits were found in:=(%s)\n", paddr_str);
		return 0;
	}
	
	if(*endptr !='\0'){
		printf("Couldn't convert all of address string:=(%s)\n", paddr_str);
		return 0;
	}
	
	printf("Physical memory address to access via exploit:=(%#lx)\n", phys_addr);
	
	/* Now try and access the given address via the faked-up kernel vulnerability...*/
	if((fd = open(VULN_DEVICE_PATH, O_RDWR)) == -1){
		printf("Failed to open ioctl device (%s) errno (%d)\n", VULN_DEVICE_PATH, errno);
		return -1;
	}

	memset(buffer, 0, BUFLEN);
	vd.p_data = buffer;
	vd.p_addr = phys_addr;;
	
	if((ioctl(fd, VULN_PHYSICAL_ADDR_CMD, &vd)) == -1){
		printf("IOCTL failed - errno (%d)\n", errno);
		return -1;
	}
	close(fd);
	printf("Data read via kernel exploit: (%s) @ (%#lx)\n", buffer?buffer:"NULL", phys_addr);
	return 0;
}
