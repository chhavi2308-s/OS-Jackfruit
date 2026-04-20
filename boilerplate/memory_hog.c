#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static int __init monitor_init(void) {
    printk(KERN_INFO "Container Monitor Loaded\n");
    return 0;
}

static void __exit monitor_exit(void) {
    printk(KERN_INFO "Container Monitor Removed\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
