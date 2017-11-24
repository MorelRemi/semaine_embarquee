#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/init.h>

/*
 *
 * Programme qui va aligner successivement une LED en bleu,
 * en rouge et en vert lorsque l'on appuie sur un bouton.
 *
 * Il va pour cela utiliser les interruptions.
 *
 */

// affectation des led aux différentes GPIO
static struct gpio leds[] = {
	{1,GPIOF_OUT_INIT_LOW,"LED 1B"},
	{2,GPIOF_OUT_INIT_LOW,"LED 2R"},
	{4,GPIOF_OUT_INIT_LOW,"LED 3V"},
	{7,GPIOF_IN,"GPIO Interrupteur In"}
};

// pour sauvegarder le nombre de fois où l'on appuie sur le bouton
static unsigned int irqNumber, nbPress = 0;

// Pour savoir si la LED est allumée
static bool ledOn = 0;

// Prototype de la fonction d'interruption
static irq_handler_t ledchange_irq_handler(unsigned int irq, void* dev_id, struct pt_regs *regs);

static int __init fonctionInit(void)
{
  // allocation mémoire des GPIO
	int ret = gpio_request_array(leds,ARRAY_SIZE(leds));
	if(ret > 0){
		printk(KERN_INFO"Error request gpio array\n");
		gpio_free_array(leds,ARRAY_SIZE(leds));
	}

	// initialisation des LED à 1 : éteintes
	printk(KERN_INFO"Init LEDS\n");
	ret = gpio_direction_output(leds[0].gpio,1);
	ret = gpio_direction_output(leds[1].gpio,1);
	ret = gpio_direction_output(leds[2].gpio,1);
	printk(KERN_INFO"Init button\n");
	ret = gpio_direction_input(leds[3].gpio);
	gpio_set_debounce(leds[3].gpio, 200);
	
	// Affectation d'un irq à la GPIO d'entrée
	irqNumber = gpio_to_irq(leds[3].gpio);
	printk(KERN_INFO"Button mapped to IRQ\n");

	// Affectation de l'interruption à l'irq
	int result = request_irq(irqNumber,(irq_handler_t) ledchange_irq_handler, IRQF_TRIGGER_RISING, "ledchange_irq_handler", NULL);
	
	printk(KERN_INFO"The interrupt request result is : %d\n", result);
	return result;
}

static void __exit fonctionExit(void)
{
  // On éteint toutes les LED
	gpio_direction_output(leds[2].gpio,1);
	gpio_direction_output(leds[0].gpio,1);
	gpio_direction_output(leds[1].gpio,1);
	
	free_irq(irqNumber, NULL);
	gpio_free_array(leds,ARRAY_SIZE(leds));
}

/* fonction d'interruption qui se déclenche quand la GPIO d'entrée
 * reçoit du courant
 */
 

static irq_handler_t ledchange_irq_handler(unsigned int irq, void* dev_id, struct pt_regs *regs){
	printk(KERN_INFO"Entry in IRQ Handler");

	// Si on appuie plus de 2 fois on réinitialise le compteur
	if(nbPress > 2){
		nbPress = 0;
	}

	// Lors de la première pression sur le bouton, on allume la LED bleue
	if(nbPress == 0){
		gpio_direction_output(leds[0].gpio,ledOn);
		gpio_direction_output(leds[1].gpio,!ledOn);
		gpio_direction_output(leds[2].gpio,!ledOn);
	}else{
	  // lors de la 2e pression, on allume la LED rouge
		if(nbPress == 1){
			gpio_direction_output(leds[1].gpio,ledOn);
        	        gpio_direction_output(leds[0].gpio,!ledOn);
	                gpio_direction_output(leds[2].gpio,!ledOn);
		}else{
		  // sinon on allume la LED verte
			gpio_direction_output(leds[2].gpio,ledOn);
	                gpio_direction_output(leds[1].gpio,!ledOn);
        	        gpio_direction_output(leds[0].gpio,!ledOn);
		}
	}
	nbPress++;

	printk(KERN_INFO"After LED traitement\n");

	return (irq_handler_t) IRQ_HANDLED;
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rémi Morel");
MODULE_DESCRIPTION("Test Button Interrupt");

