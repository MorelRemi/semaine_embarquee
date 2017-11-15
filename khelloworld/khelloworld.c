#include <linux/module.h>
#include <linux/kernel.h>

static int __init fonctionInit(void)
{
	printk(KERN_INFO"KHello World ! \n");
	return 0;
}

static void __exit fonctionExit(void)
{
	printk(KERN_INFO"Goodbye world ! \n");
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rémi Morel");
MODULE_DESCRIPTION("KHello World!");

