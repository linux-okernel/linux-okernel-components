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
#define PG_BIT 20

static size_t len_banner;
static unsigned long mod_start = (unsigned long)PFN_ALIGN(MODULES_VADDR);
static unsigned long mod_end = (unsigned long)PFN_ALIGN(MODULES_END);
static unsigned long targets[10];
static unsigned long kw_text;
static unsigned long kw_etext;
static unsigned long kw__start_rodata;
static unsigned long kw__end_rodata;
static int (*kw_set_memory_rw)(unsigned long addr, int numpages);
static void (*kw_flush_tlb_all)(void);
static void *(*kw_text_poke)(void *addr, const void *opcode, size_t len);
static void (*kw_flush_tlb_kernel_range)(unsigned long start, unsigned long end);
struct mutex *kw_text_mutex;
static char *kw_linux_proc_banner;
static char old_banner[BUFFMAX];

static const char *patched_banner = "Successfully patched linux_proc_banner\n";
static const unsigned long pgmask = ~(((unsigned long)1 << PG_BIT) - 1);
static const unsigned long psize = ((unsigned long)1 << PG_BIT);

static void __init get_targets(void)
{
	printk("kwriter pgmask %#lx\n", pgmask);
	printk("kwriter invoking oktargets %#lx\n", (unsigned long) oktargets);
	oktargets(&targets);
	printk("kwriter returned from oktargets\n");
	kw_text = targets[0];
	kw_etext = targets[1];
	kw__start_rodata = targets[2];
	kw__end_rodata = targets[3];
	kw_set_memory_rw = (int (*)(unsigned long, int))targets[4];
	kw_flush_tlb_all =  (void (*)(void))targets[5];
	kw_linux_proc_banner = (char *)targets[6];
	kw_text_poke = (void *(*)(void *, const void *, size_t))targets[7];
	kw_text_mutex = (struct mutex *)targets[8];
	kw_flush_tlb_kernel_range =
		(void (*)(unsigned long, unsigned long))targets[9];

	printk(KERN_INFO "kwriter _text %#lx\n", kw_text);
	printk(KERN_INFO "kwriter _etext %#lx\n", kw_etext);
	printk(KERN_INFO "kwriter __start_rodata va %#lx\n", kw__start_rodata);
	printk(KERN_INFO "kwriter __end_rodata va %#lx\n", kw__end_rodata);
	printk(KERN_INFO "kwriter set_memory_rw %#lx\n",
	       (unsigned long) kw_set_memory_rw);
	printk(KERN_INFO "kwriter linux_proc_banner %s\n", kw_linux_proc_banner);
	printk(KERN_INFO "kwriter linux_proc_banner va %#lx\n",
	       (unsigned long) kw_linux_proc_banner);
	printk(KERN_INFO "kwriter module space start va %#lx\n", mod_start);
	printk(KERN_INFO "kwriter module space end va %#lx\n", mod_end);
	printk(KERN_INFO "kwriter flush_tlb_kernel_range %#lx\n",
	       (unsigned long) kw_flush_tlb_kernel_range);
}

static void check_va(unsigned long va)
{
	pte_t *kpte;
	unsigned int level;
	pgprot_t prot;

	printk(KERN_INFO "oktest checking guest physical pg perms for va %#lx",
	       va);
	kpte = lookup_address(va, &level);
	if (!kpte){
		printk(KERN_CONT "address not found in guest page tables\n");
		return;
	}
	prot = pte_pgprot(*kpte);
	if (pgprot_val(prot) & pgprot_val(__pgprot(_PAGE_NX)))
		printk(KERN_CONT " NX is set");
	if (pgprot_val(prot) & pgprot_val(__pgprot(_PAGE_RW)))
		printk(KERN_CONT " RW is set");
}

static void set_mem_rw(unsigned long va)
{	pte_t *kpte, old_pte, new_pte;
	unsigned int level;
	pgprot_t new_prot;
	unsigned long pfn;

	kpte = lookup_address(va, &level);
	if (level != PG_LEVEL_4K) {
		printk(KERN_INFO "kwriter set_mem_rw va %#lx not 4k page\n", va);
	}
	if (!kpte){
		printk(KERN_INFO "kwriter set_mem_rw kpte null va %#lx\n", va);
		return;
	}
	old_pte = *kpte;
	if (pte_none(old_pte)) {
		printk(KERN_INFO "kwriter set_mem_rw va %#lx not mapped?\n", va);
		return;
	}
	pfn = pte_pfn(old_pte);
	new_prot = pte_pgprot(old_pte);
	pgprot_val(new_prot) |= pgprot_val(__pgprot(_PAGE_RW));
	new_pte = pfn_pte(pfn, new_prot);
	set_pte_atomic(kpte, new_pte);
	return;
}

 static void old_update_banner(void)
 {
 	size_t n;
	printk(KERN_INFO "kwriter attempting to get text_mutex\n");
	if (!mutex_trylock(kw_text_mutex)) {
		printk(KERN_INFO "kwriter Unable to get text_mutex\n");
 	}
	printk(KERN_INFO "kwriter Got text_mutex\n");
 	if (!(strncpy(old_banner, kw_linux_proc_banner, BUFFMAX))) {
		printk(KERN_INFO "kwriter strncpy failed\n");
		goto end;
 	}
 	printk(KERN_INFO "kwriter attempting to patch linux_proc_banner");
	n = strlen(patched_banner);
//	kw_text_poke((void *)kw_linux_proc_banner, (void *)patched_banner, n);
end:
	mutex_unlock(kw_text_mutex);
 }

static void update_banner(void)
{
	unsigned long va = ((unsigned long)kw_linux_proc_banner) & pgmask;
	printk(KERN_INFO "kwriter attempting to set page rw %#lx\n", va);
	if (kw_set_memory_rw(va, 1)) {
		printk(KERN_INFO "kwriter kw_set_memory_rw failed\n");
		return;
	}
	kw_flush_tlb_all();
	kw_flush_tlb_kernel_range(va, va + psize);
	printk(KERN_INFO "kwriter flushed tlb and page\n");
	if (!(strncpy(old_banner, kw_linux_proc_banner, BUFFMAX))) {
		printk(KERN_INFO "kwriter failed to save banner\n");
		return;
	}
	check_va((unsigned long)kw_linux_proc_banner);
	check_va(va);
	/* Check that we can still read kw_linux_proc_banner get the
	 * protections from the page table and check they are what we
	 * expect them to be. Insert memory barriers?
	 */
	//printk(KERN_INFO "Calling okset_mem_rw");
	//okset_mem_rw(va);
	printk(KERN_INFO "Calling set_mem_rw");
	set_mem_rw(va);

	check_va(va);
	kw_flush_tlb_all();
	kw_flush_tlb_kernel_range(va, va + psize);
	printk(KERN_INFO "kwriter flushed tlb and page\n");
	printk(KERN_INFO "Checking we can still read linux_proc_banner %s\n",
	       kw_linux_proc_banner);
	len_banner = strlen(kw_linux_proc_banner) + 1;
	printk(KERN_INFO "kwriter patching linux_proc_banner len is %ld bytes\n",
		len_banner);
	if (len_banner > BUFFMAX)
		len_banner = BUFFMAX;
	if (!(strncpy(kw_linux_proc_banner, patched_banner, len_banner))) {
 		printk(KERN_INFO "kwriter patching banner failed\n");
		return;
	}
	check_va((unsigned long)kw_linux_proc_banner);
	printk(KERN_INFO "kwriter linux_proc_banner %s\n", kw_linux_proc_banner);
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
	unsigned long va = (unsigned long)kw_linux_proc_banner;

	printk("kwriter: unloading");
	printk(KERN_INFO "Calling set_mem_rw");
	set_mem_rw(va);
	check_va(va);
	if (!(strncpy(kw_linux_proc_banner, old_banner, len_banner))) {
		printk(KERN_INFO "kwriter failed to save banner\n");
		return;
	}
	printk("kwriter: done.\n");
	return;
}

module_init(kwriter_module_init);
module_exit(kwriter_module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nigel Edwards");
MODULE_DESCRIPTION("Module to demonstrate kwriter kernel integrity protection.");
