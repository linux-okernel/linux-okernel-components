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
#include "kernel_vuln.h"


/* 
   Quick and dirty demo hack for the okernel kernel modifications  - cid Oct 2016 
   Try and access a passed-in physical address from kernel mode.
*/

static int device_open(struct inode *inode, struct file *file)
{
    printk("KV: Opening device <%s>\n", VULN_DEVICE_NAME);
    return 0;
}


long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    unsigned long  p_addr = 0;
    unsigned long*  v_addr = 0;
    struct vuln_data vd;
    char* p;
    
    switch(cmd)
    {
    case VULN_PHYSICAL_ADDR_CMD:
	    copy_from_user(&vd, (void*)arg, sizeof(struct vuln_data));
	    p_addr = vd.p_addr;
	    printk("KV: Passed physical Address <%#lx>\n", p_addr);
	    v_addr = phys_to_virt(p_addr);
	    printk("KV: Kernel virtual address <%p>\n", v_addr);
	    p = (char*)v_addr;
	    printk("KV: Read from p_addr/vaddr (%#lx/%#lx) in kernel: %s\n",
		   p_addr, (unsigned long)v_addr, p);
	    copy_to_user(vd.p_data, p, BUFLEN);
	    break;
    default:
	    printk("KV: unsupported cmd.\n");
	    return -EINVAL;
    }
    return 0;
}


static int device_release(struct inode *inode, struct file *file)
{
    printk("KV: Releasing device <%s>\n", VULN_DEVICE_NAME);
    return 0;
}

static struct class *my_class;

static struct file_operations fops={
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
};


static int __init kernel_vuln_module_init(void)
{
    printk ("KV: Loading kernel vuln demo module...\n");
    major_no = register_chrdev(0, VULN_DEVICE_NAME, &fops);
    printk("KV: Creating Device Major_no : %d\n", major_no);
    my_class = class_create(THIS_MODULE, VULN_DEVICE_NAME);
    device_create(my_class, NULL, MKDEV(major_no,0), NULL, VULN_DEVICE_NAME);
    printk("KV: Device <%s> Initialized in kernel.\n", VULN_DEVICE_NAME);
    return 0;
}

static void __exit kernel_vuln_module_exit(void)
{
    printk ("KV: Unloading kernel vuln demo module and removing device...");
    device_destroy(my_class,MKDEV(major_no,0));
    class_destroy(my_class);
    unregister_chrdev(major_no, VULN_DEVICE_NAME);
    printk("KV: done.\n");
    return;
}

module_init(kernel_vuln_module_init);
module_exit(kernel_vuln_module_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("C Dalton"); 
MODULE_DESCRIPTION("Module to provide demo hack kernel vulnerability.");
