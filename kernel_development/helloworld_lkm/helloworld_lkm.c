#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int __init helloworld_lkm_init(void)
{
	pr_info("helloworld_lkm: hello, kernel!\n");
	return 0;
}

static void __exit helloworld_lkm_exit(void)
{
	pr_info("helloworld_lkm: goodbye, kernel!\n");
}

module_init(helloworld_lkm_init);
module_exit(helloworld_lkm_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple hello world kernel module");
