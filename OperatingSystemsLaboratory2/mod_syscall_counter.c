/**
 * mod_syscall_counter_template.c - Linux kernel module to count system calls
 *
 * This module creates a /proc entry that displays the count of different
 * system calls made since the module was loaded. It uses kprobes to hook
 * into the system call entry points.
 * 
 * STUDENT EXERCISE: Complete the TODOs in this file
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/syscalls.h>

#define PROC_NAME "syscall_counter"
/* Note: You could use NR_syscalls from <linux/syscalls.h> instead of a fixed MAX_SYSCALLS value */
#define MAX_SYSCALLS 450  // Maximum number of system calls to track

#define WANT_SYSCALL_NAMES

/**
 * Provides you with an array of syscall names
 * which you can use to print the syscall name instead of the syscall number
 * syscall_names[i] will give you the name of the i-th syscall.
 */
#ifdef WANT_SYSCALL_NAMES
#  include "syscall_names.h"
#endif


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student Names: Aleksandra Baibik & Samira Alika"); // TODO: Add your name
MODULE_DESCRIPTION("A module that counts system calls");

// TODO: Define a data structure to store system call counts
// Hint: You'll need an array to count each system call

static unsigned long syscall_counts[MAX_SYSCALLS];

// TODO: Add synchronization mechanism to handle concurrent system calls
// Hint: Consider using a spinlock

static spinlock_t syscall_counts_lock;

// Proc directory entry
static struct proc_dir_entry *proc_entry;

// Kprobe structure for system call entry
static struct kprobe kp;

// TODO: Define possible system call entry points for different architectures
// Hint: Different architectures use different function names for system call entry

static const char *possible_syscall_entries[] = {
#if defined(CONFIG_X86_64)
    "do_syscall_64",
    "entry_SYSCALL_64",
#elif defined(CONFIG_X86)
    "sysenter_do_call",
#endif
};

static const int num_possible_syscall_entries =
    sizeof(possible_syscall_entries) / sizeof(possible_syscall_entries[0]);

// Kprobe pre-handler
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    // TODO: Extract the system call number from registers
    // Hint: Different architectures store system call numbers in different registers

    long nr = -1;
    unsigned long flags;
    
    // TODO: Increment the counter for this system call
    // Hint: Make sure to protect this with a lock for concurrency

    #if defined(CONFIG_X86_64)
        nr = regs->orig_ax;
    #elif defined (CONFIG_X86)
        nr = regs->orig_ax;
    #endif

    if(nr >= 0 && nr < MAX_SYSCALLS) {
        spin_lock_irqsave(&syscall_counts_lock, flags);
        syscall_counts[nr]++;
        spin_unlock_irqrestore(&syscall_counts_lock, flags);
    }
    
    return 0;
}

// Function to display the proc file output
static int syscall_counter_show(struct seq_file *m, void *v)
{
    // TODO: Display the system call counts in the proc file
    // Hint: Use seq_printf() to format the output

    int i;
    unsigned long flags;

    seq_printf(m, "Syscall statistics since module load:\n");
    seq_printf(m, "%-5s %-30s %s\n", "NR", "NAME", "COUNT");

    spin_lock_irqsave(&syscall_counts_lock, flags);

    for (i = 0; i < MAX_SYSCALLS; i++) {
        if (syscall_counts[i] == 0)
            continue;

    #ifdef WANT_SYSCALL_NAMES
        const char *name = NULL;
        if (i >= 0 && i < NR_syscalls)  // NR_syscalls из <linux/syscalls.h>
            name = syscall_names[i];   // из syscall_names.h
        if (!name)
            name = "unknown";
        seq_printf(m, "%-5d %-30s %lu\n", i, name, syscall_counts[i]);
    #else
        seq_printf(m, "%-5d %-30s %lu\n", i, "unknown", syscall_counts[i]);
    #endif
    }

    spin_unlock_irqrestore(&syscall_counts_lock, flags);
    
    return 0;
}

// Open operation for the proc file
static int syscall_counter_open(struct inode *inode, struct file *file)
{
    return single_open(file, syscall_counter_show, NULL);
}

// File operations for the proc file
static const struct proc_ops syscall_counter_fops = {
    .proc_open    = syscall_counter_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_open_release,
};

// Module initialization
static int __init mod_syscall_counter_init(void)
{
    int ret = 0; 
    int i;

    for (i = 0; i < MAX_SYSCALLS; i++)
        syscall_counts[i] = 0;

    spin_lock_init(&syscall_counts_lock);

    proc_entry = proc_create(PROC_NAME, 0444, NULL, &syscall_counter_fops);
    if (!proc_entry) {
        printk(KERN_ALERT "syscall_counter: Could not create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
    
    // TODO: Initialize your data structures

    memset(&kp, 0, sizeof(kp));
    kp.pre_handler = handler_pre;

    ret = -ENOENT;

    for (i = 0; i < num_possible_syscall_entries; i++) {
        kp.symbol_name = possible_syscall_entries[i];

    // TODO: Try registering kprobes for different entry points
    // Hint: Loop through possible_syscall_entries[] array and try to register
    // a kprobe for each entry point

        ret = register_kprobe(&kp);
        if (ret == 0) {
            printk(KERN_INFO "syscall_counter: kprobe registered at %s\n",
                   possible_syscall_entries[i]);
            break;
        } else {
            printk(KERN_INFO "syscall_counter: failed to register kprobe at %s (err=%d)\n",
                   possible_syscall_entries[i], ret);
        }
    }

    // TODO: Check if any probe was registered
    // and remove the proc entry if no probe was registered

    if (ret != 0) {
        remove_proc_entry(PROC_NAME, NULL);
        printk(KERN_ALERT "syscall_counter: no valid syscall entry point found\n");
        return ret;
    }

    printk(KERN_INFO "syscall_counter: Module loaded successfully\n");

    return 0;
}

// Module cleanup
static void __exit mod_syscall_counter_exit(void)
{
    // TODO: Clean up your resources

    // TODO: Unregister the kprobe

    unregister_kprobe(&kp);
    
    // Remove the proc entry

     if (proc_entry)
        remove_proc_entry(PROC_NAME, NULL);
    
    printk(KERN_INFO "syscall_counter: Module unloaded successfully\n");
}

// Module initialization and cleanup
module_init(mod_syscall_counter_init);
module_exit(mod_syscall_counter_exit); 