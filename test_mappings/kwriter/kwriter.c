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


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>
#include <linux/mm.h>
#include <asm-generic/sections.h>


/*
 * Attempt to write kernel memory to demonstrate okernel protection
 */
void oktargets(unsigned long (*address)[]);
#define BUFFMAX 1000

static unsigned long mod_start = (unsigned long)PFN_ALIGN(MODULES_VADDR);
static unsigned long mod_end = (unsigned long)PFN_ALIGN(MODULES_END);
static unsigned long targets[7];
static unsigned long kw_text;
static unsigned long kw_etext;
static unsigned long kw__start_rodata;
static unsigned long kw__end_rodata;
static void *(*kw_text_poke)(void *addr, const void *opcode, size_t len);
struct mutex *text_mutex;
static char *kw_linux_banner;
static char old_banner[BUFFMAX];

static const char *patched_banner = "Succesfully patched linux_banner";

static void __init get_targets(void)
{
	printk("Invoking oktargets %#lx\n", (unsigned long) oktargets);
	oktargets(&targets);
	printk("Returned from oktargets\n");
	kw_text = targets[0];
	kw_etext = targets[1];
	kw__start_rodata = targets[2];
	kw__end_rodata = targets[3];
	kw_text_poke = (void *(*)(void *, const void *, size_t))targets[4];
	text_mutex = (struct mutex *)targets[5];
	kw_linux_banner = (char *)targets[6];

	printk(KERN_INFO "kwriter _text %#lx\n", kw_text);
	printk(KERN_INFO "kwriter _etext %#lx\n", kw_etext);
	printk(KERN_INFO "kwriter __start_rodata va %#lx\n", kw__start_rodata);
	printk(KERN_INFO "kwriter __end_rodata va %#lx\n", kw__end_rodata);
	printk(KERN_INFO "kwriter text_poke %#lx\n",
	       (unsigned long) kw_text_poke);
	printk(KERN_INFO "kwriter text_mutex %#lx\n",
	       (unsigned long )text_mutex);
	printk(KERN_INFO "kwriter linux_banner %s\n", kw_linux_banner);
	printk(KERN_INFO "kwriter module space start va %#lx\n", mod_start);
	printk(KERN_INFO "kwriter module space end va %#lx\n", mod_end);
}

static void update_banner(void)
{
	size_t n;
	printk(KERN_INFO "kwriter attempting to get text_mutex\n");
	if (!mutex_trylock(text_mutex)) {
		printk(KERN_INFO "kwriter Unable to get text_mutex\n");
		return;
	}
	printk(KERN_INFO "kwriter Got text_mutex\n");
	if (!(strncpy(old_banner, kw_linux_banner, BUFFMAX))) {
		printk(KERN_INFO "kwriter strncpy failed\n");
		goto end;
	}
	printk(KERN_INFO "kwriter attempting to patch linux_banner");
	n = strlen(patched_banner);
//	kw_text_poke((void *)kw_linux_banner, (void *)patched_banner, n);
end:
	mutex_unlock(text_mutex);
}

static int __init kwriter_module_init(void)
{
    unsigned long cr4;

    printk ("kwriter: loading module...\n");
    printk("kwriter: trying to disable SMEP\n");
    cr4 = native_read_cr4();
    printk("kwriter: CR4 is currently set to %#lx\n", cr4);
    cr4 = cr4 & ~X86_CR4_SMEP;
    native_write_cr4(cr4);
    printk("kwriter: Finished attempt to disable SMEP by writing %#lx to CR4\n", cr4);
    get_targets();
    update_banner();
    printk(KERN_INFO "kwriter done __init\n");
    return 0;
}

static void __exit kwriter_module_exit(void)
{
    printk("kwriter: unloading");
    printk("kwriter: done.\n");
    return;
}

module_init(kwriter_module_init);
module_exit(kwriter_module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nigel Edwards");
MODULE_DESCRIPTION("Module to demonstrate kwriter kernel integrity protection.");
