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
#include <linux/moduleparam.h>

#define PROC_NAME "syscall_counter"
/* Note: You could use NR_syscalls from <linux/syscalls.h> instead of a fixed MAX_SYSCALLS value */

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
MODULE_DESCRIPTION("Kernel module to track and count system calls with reset functionality");

// Module parameter: reset_counters
static int reset_counters = 0;

// TODO: Define a data structure to store system call counts
// Hint: You'll need an array to count each system call

static unsigned long syscall_counts[NR_syscalls];

// TODO: Add synchronization mechanism to handle concurrent system calls
// Hint: Consider using a spinlock

static spinlock_t syscall_counts_lock;

// Getter for reset_counters (called when reading the parameter via sysfs)
static int reset_counters_get(char *buffer, const struct kernel_param *kp)
{
    // Write the current parameter value into the buffer, including a newline
    return sprintf(buffer, "%d\n", reset_counters);
}

// Setter for reset_counters (called when writing the parameter via sysfs)
static int reset_counters_set(const char *val, const struct kernel_param *kp)
{
    int ret;
    int i;
    unsigned long flags;

    // Parse the string and store the value in reset_counters
    ret = param_set_int(val, kp);
    if (ret != 0)
        return ret;  // parsing error

    // If the user sets the value to 1 -> reset all counters
    if (reset_counters == 1) {
        spin_lock_irqsave(&syscall_counts_lock, flags);
        for (i = 0; i < NR_syscalls; i++)
            syscall_counts[i] = 0;
        spin_unlock_irqrestore(&syscall_counts_lock, flags);

        reset_counters = 0;  // auto reset
        printk(KERN_INFO "syscall_counter: Counters reset via module parameter\n");
    }

    return 0; 
}

// Operations for reset_counters parameter
static const struct kernel_param_ops reset_counters_ops = {
    .set = reset_counters_set,
    .get = reset_counters_get,
};

// Register module parameter with callbacks
module_param_cb(reset_counters, &reset_counters_ops, &reset_counters, 0644);
MODULE_PARM_DESC(reset_counters, "Set to 1 to reset all syscall counters");

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
#elif defined(CONFIG_ARM64)
    "invoke_syscall",
    "compat_arm_syscall",
#endif
};

static const int num_possible_syscall_entries =
    sizeof(possible_syscall_entries) / sizeof(possible_syscall_entries[0]);

// Kprobe pre-handler
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    long nr = -1;
    unsigned long flags;

#if defined(CONFIG_X86_64) || defined(CONFIG_X86)
    nr = regs->orig_ax;
#elif defined(CONFIG_ARM64)
    nr = regs->syscallno;      // IMPORTANT: on ARM64 the syscall number is taken from syscallno
#endif

    if (nr >= 0 && nr < NR_syscalls) {
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

    seq_printf(m, "Syscall statistics since module load:\n");
    seq_printf(m, "%-5s %-30s %s\n", "NR", "NAME", "COUNT");

    spin_lock(&syscall_counts_lock); 

    for (i = 0; i < NR_syscalls; i++) {
        if (syscall_counts[i] == 0)
            continue;

    #ifdef WANT_SYSCALL_NAMES
        const char *name = NULL;
        if (i >= 0 && i < NR_syscalls)  // NR_syscalls from <linux/syscalls.h>
            name = syscall_names[i];   // from syscall_names.h

        seq_printf(m, "%-5d %-30s %lu\n", i, name, syscall_counts[i]);
    #else
        seq_printf(m, "%-5d %-30s %lu\n", i, "N/A", syscall_counts[i]);
    #endif
    }

    spin_unlock(&syscall_counts_lock);
    
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
};

// Module initialization
static int __init mod_syscall_counter_init(void)
{
    printk(KERN_INFO "syscall_counter: mod_syscall_counter_init enter\n");
    int ret = 0;
    int i;
    unsigned long flags;

    for (i = 0; i < NR_syscalls; i++)
        syscall_counts[i] = 0;

    spin_lock_init(&syscall_counts_lock);

    // Handle load-time reset (insmod ... reset_counters=1)
    if (reset_counters == 1) {
        spin_lock_irqsave(&syscall_counts_lock, flags);
        for (i = 0; i < NR_syscalls; i++)
            syscall_counts[i] = 0;
        spin_unlock_irqrestore(&syscall_counts_lock, flags);

        reset_counters = 0;
        printk(KERN_INFO "syscall_counter: Counters reset during module init\n");
    }

    proc_entry = proc_create(PROC_NAME, 0444, NULL, &syscall_counter_fops);
    if (!proc_entry) {
        printk(KERN_ALERT "syscall_counter: Could not create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
    
    // TODO: Initialize your data structures

    memset(&kp, 0, sizeof(kp));
    kp.pre_handler = handler_pre;

    ret = -ENOENT;
    bool has_any_successful_register=false;

    printk(KERN_INFO "syscall_counter: num_possible_syscall_entries=%d\n", num_possible_syscall_entries);

    for (i = 0; i < num_possible_syscall_entries; i++) {
        kp.symbol_name = possible_syscall_entries[i];

    // TODO: Try registering kprobes for different entry points
    // Hint: Loop through possible_syscall_entries[] array and try to register
    // a kprobe for each entry point

        ret = register_kprobe(&kp);
        if (ret == 0) {
            printk(KERN_INFO "syscall_counter: kprobe registered at %s\n",
                   possible_syscall_entries[i]);
            has_any_successful_register=true;
            break;
        } else {
            printk(KERN_INFO "syscall_counter: failed to register kprobe at %s (err=%d)\n",
                   possible_syscall_entries[i], ret);
        }
    }

    // TODO: Check if any probe was registered
    // and remove the proc entry if no probe was registered

    if (!has_any_successful_register) {
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