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
 * This program incorporates some code by  from Andrey Konovalov
 *  see: https://github.com/xairy/kernel-exploits/tree/master/CVE-2017-7308
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
 * Add Demonstrate linux-okernel SMAP/SMEP protection
 */

#define MN "cr4writer "
void ok_test_dev_init(void);
void ok_test_dev_remove(void);

static int __init cr4w_module_init(void)
{
/*
	unsigned long cr4;
	printk(KERN_INFO MN "updating CR4 to bypass SMAP/SMEP\n");
	cr4 = native_read_cr4();
	printk(MN "CR4 is currently set to %#lx\n", cr4);
	cr4 = cr4 & ~(X86_CR4_SMEP|X86_CR4_SMAP);
	native_write_cr4(cr4);
	printk(MN "SMAP/SMEP disabled CR4 now %#lx\n", cr4);

	printk(KERN_INFO MN "done __init\n");
*/
	ok_test_dev_init();
	return 0;
}

static void __exit cr4w_module_exit(void)
{
	printk(MN "unloading");
	ok_test_dev_remove();
	printk(MN "done.\n");
	return;
}

module_init(cr4w_module_init);
module_exit(cr4w_module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nigel Edwards");
MODULE_DESCRIPTION("Demonstrate linux-okernel protection of CR4");
