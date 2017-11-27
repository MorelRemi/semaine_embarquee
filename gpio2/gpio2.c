#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#define GPIO2 2

/**
 *
 * Programme noyau Linux qui va tester une GPIO en sortie.
 * Au démarrage du module avec modprobe GPIO2, on envoie un courant dans 
 * la GPIO 2 (Pin 11).
 *
 * Lors de l'arret du module avec rmmod GPIO2, on désactive
 * le courant dans cette GPIO
 *
 */

static int __init fonctionInit(void)
{
	int retour1 = gpio_request(GPIO2,"test Pin 11 GPIO");
	if(retour1 > 0){
		printk(KERN_INFO"Error request GPIO 11\n");
		gpio_free(GPIO2);
		return 1;
	}
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

