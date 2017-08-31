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
 * Make vsyscall page writabe to demonstrate linux-okernel vsyscall protection
 */

#define MN "vsysrw "
#define VSYSCALL 0xffffffffff600000
#define OFF (1UL << 20)
#define K4  (4UL << 10)

void oktargets(unsigned long (*address)[]);

static unsigned long targets[20];
static int (*set_memory_rw)(unsigned long addr, int numpages);

static void __init check_va(unsigned long va)
{
	pte_t *kpte;
	unsigned int level;
	pgprot_t prot;

	printk(KERN_INFO MN "checking guest physical pg perms for va %#lx",
	       va);
	kpte = lookup_address(va, &level);
	if (!kpte){
		printk(KERN_CONT "address not found in guest page tables\n");
		return;
	}
	if (!(pte_val(*kpte) & _PAGE_PRESENT))
		printk(KERN_CONT " page NOT present");
	else
		printk(KERN_CONT " page present");
	prot = pte_pgprot(*kpte);
	if (pgprot_val(prot) & pgprot_val(__pgprot(_PAGE_NX)))
		printk(KERN_CONT " NX is set");
	if (pgprot_val(prot) & pgprot_val(__pgprot(_PAGE_RW)))
		printk(KERN_CONT " RW is set");
	printk("\n");
}


static void __init get_targets(void)
{
	printk(MN "invoking oktargets %#lx\n", (unsigned long) oktargets);
	oktargets(&targets);
	printk(MN "returned from oktargets\n");
	set_memory_rw = (int (*)(unsigned long, int))targets[4];

}

static __init void mk_vsys_rw(void)
{
	unsigned long i;

	for(i=0; i < 10; i++) {
		check_va((unsigned long) VSYSCALL + (i* K4));
	}
	check_va((unsigned long)VSYSCALL);
	check_va((unsigned long) OFF + VSYSCALL);
	printk(MN "making vsyscall page writable..\n");
	set_memory_rw((unsigned long)VSYSCALL, 1);
	check_va((unsigned long)VSYSCALL);
	check_va((unsigned long) OFF + VSYSCALL);
}

static int __init sysctl_module_init(void)
{

	get_targets();
	mk_vsys_rw();
	
	printk(KERN_INFO MN "mk_vsys_rw done __init\n");
	return 0;
}

static void __exit sysctl_module_exit(void)
{
	printk(MN " unloading");

	printk(MN "done.\n");
	return;
}

module_init(sysctl_module_init);
module_exit(sysctl_module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nigel Edwards");
MODULE_DESCRIPTION("Demonstrate linux-okernel protection of vsyscall page");
