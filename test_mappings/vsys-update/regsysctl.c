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


#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/sysctl.h>
#include <linux/uaccess.h>
#include <asm/current.h>
#include <asm/pgtable.h>
#include <asm-generic/sections.h>


/*
 * Add Demonstrate linux-okernel vsys protection
 */

#define VSYSCALL 0xffffffffff600000
#define MN "reg_sys_ctl "

void oktargets(unsigned long (*address)[]);

static unsigned long targets[20];
static struct ctl_table_header *(*do_reg)(struct ctl_table *);

static void __init get_targets(void)
{
	printk(MN "invoking oktargets %#lx\n", (unsigned long) oktargets);
	oktargets(&targets);
	printk(MN "returned from oktargets\n");
	do_reg = (struct ctl_table_header *(*)(struct ctl_table *))targets[10];
}


static __init void regsysctl(void)
{
	printk(KERN_INFO MN "Attempting to read VSYSCALL page\n");
	printk(KERN_INFO MN "Found at (VSYSCALL+0xf00): %s\n", (char *)(VSYSCALL+0xf00));
	/* Unfortunately the following causes an unhandled page fault on >=4.13*/
	/*
	printk(KERN_CRIT MN "calling register_sysctl_table..\n");
	do_reg((struct ctl_table *)VSYSCALL+0x850);
	*/
}

static int __init sysctl_module_init(void)
{

	get_targets();
	regsysctl();
	
	printk(KERN_INFO MN "done __init\n");
	return 0;
}

static void __exit sysctl_module_exit(void)
{
	printk(MN "unloading");

	printk(MN "done.\n");
	return;
}

module_init(sysctl_module_init);
module_exit(sysctl_module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nigel Edwards");
MODULE_DESCRIPTION("Demonstrate linux-okernel protection of vsys / systcl.");
