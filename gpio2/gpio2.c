#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#define GPIO2 2

static int __init fonctionInit(void)
{
	int retour1 = gpio_request(GPIO2,"test Pin 11 GPIO");
	retour1 = gpio_direction_output(GPIO2,1);
	return 0;
}

static void __exit fonctionExit(void)
{
	gpio_set_value(GPIO2,0);
	gpio_free(GPIO2);
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rémi Morel");
MODULE_DESCRIPTION("GPIO Test");

