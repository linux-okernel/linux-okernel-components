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
 * Initialize ok_test ioctl device
 */

#define MN "ok_test_init "
void ok_test_dev_init(void);
void ok_test_dev_remove(void);

static int __init oktest_module_init(void)
{
	ok_test_dev_init();
	return 0;
}

static void __exit oktest_module_exit(void)
{
	printk(MN "unloading");
	ok_test_dev_remove();
	printk(MN "done.\n");
	return;
}

module_init(oktest_module_init);
module_exit(oktest__module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nigel Edwards");
MODULE_DESCRIPTION("Initialize ok_test ioctl for testing");
