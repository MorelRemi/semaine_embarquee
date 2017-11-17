#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/sys.h>

static int dev_open(struct inode *inod, struct file *fil);
static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off);
static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off);
static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
static int dev_rls(struct inode *inod, struct file *fil);

struct miscdevice led;

static struct file_operations fops = {
	.read=dev_read,
	.write=dev_write,
	.open=dev_open,
	.unlocked_ioctl=dev_ioctl,
	.release=dev_rls
};

//Tableau des gpios utilisées pour le KeyPad
static struct gpio ledTab[] = {
	{51, GPIOF_OUT_INIT_HIGH, "LED rouge"},
	{52, GPIOF_OUT_INIT_HIGH, "LED verte"}
};

/**
	Fonction d'Init du module Noyau
**/
static int __init fonctionInit(void){

	led.name = "led";
	led.minor = MISC_DYNAMIC_MINOR;
	led.fops = &fops;
	misc_register(&led);
	
	printk(KERN_INFO "Démarrage de folie \n");

	int returnCode;
	//réservation des GPIO
	returnCode = gpio_request_array(ledTab,ARRAY_SIZE(ledTab));
	if(returnCode != 0){
		printk(KERN_INFO"Error allocate GPIO\n");
		gpio_free_array(ledTab,ARRAY_SIZE(ledTab));
	}
	
	return 0;
}

/**
	Fonction Exit du module Noyau
**/
static void __exit fonctionExit(void){
	misc_deregister(&led);
	gpio_direction_output(ledTab[0].gpio,1);
	gpio_direction_output(ledTab[1].gpio,1);	
	gpio_free_array(ledTab,ARRAY_SIZE(ledTab));
}

static int dev_rls(struct inode *inod, struct file *fil) {
	printk(KERN_ALERT "Alerte fermeture !! \n");
	return 0;
}

static int dev_open(struct inode *inod, struct file *fil) {
	printk(KERN_ALERT "Alerte ouverture !! \n");
	return 0;
}

static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off) {
	printk(KERN_INFO "Alerte lecture !! \n");
	return 0;
}

static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off) {
	printk(KERN_ALERT "Alerte écriture !! \n");
	int dummy, mylen;
	char buf[2] = "?";
	if (len > 2) mylen = 2; 
	else mylen = len;

	dummy = copy_from_user(buf, buff, mylen);
	buf[mylen] = 0;
	printk(KERN_ALERT "buffer : %s test\n", buf);

	if(buf[0] == '1') {
		gpio_direction_output(ledTab[0].gpio,1);
		gpio_direction_output(ledTab[1].gpio,0);
	}
	else if (buf[0] == '2') {
        	gpio_direction_output(ledTab[0].gpio,0);
		gpio_direction_output(ledTab[1].gpio,1);		
	}
	else {
                gpio_direction_output(ledTab[1].gpio,1);
		gpio_direction_output(ledTab[0].gpio,1);
        }

	return mylen;
}

static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
	static char var[10];
	int dummy;
	printk(KERN_ALERT "Alerte %d", cmd);
	switch (cmd) {
		case 0 : dummy = copy_from_user(var, (char*)arg, 5);
			printk(KERN_ALERT "Alerte 0 %s \n", (char*) var);
			break;
		case 1 : printk(KERN_ALERT "Alerte 1 %s \n", (char*) var);
			dummy = copy_to_user((char*) arg, var, 5);
			break;
		default : printk(KERN_ALERT "unknown");
			break;
	}
	return 0;
}

module_init(fonctionInit);
module_exit(fonctionExit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Maeckereel & Rémi Morel");
MODULE_DESCRIPTION("Test Digicode");
