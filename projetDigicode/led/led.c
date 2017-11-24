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

/**
*
*       Directives de pré-processeur
*
* 
* Prototypes des fonctions opérations du module noyau : open, read, write, ioctl, rls et poll
*  
*/

// fonction qui se déclenche à l'ouverture du fichier virtuel correspondant au module
static int dev_open(struct inode *inod, struct file *fil);

// fonction qui se déclenche à la lecture du fichier virtuel correspondant au module
static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off);

// fonction qui se déclenche à l'écriture du fichier virtuel correspondant au module
static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off);

// fonction qui se déclenche à la fermeture du fichier virtuel correspondant au module

static int dev_rls(struct inode *inod, struct file *fil);





// création du fichier virtuel

struct miscdevice led;

static struct file_operations fops = {
	.read=dev_read,
	.write=dev_write,
	.open=dev_open,
	.release=dev_rls
};

// tableau des gpios utilisées pour le KeyPad
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

	// test du code de retour
	if(returnCode != 0){
		printk(KERN_INFO"Error allocate GPIO\n");
		gpio_free_array(ledTab,ARRAY_SIZE(ledTab));
	}
	
	return 0;
}

/**
*	Fonction Exit du module Noyau
*
* on remet du courant dans les GPIO : LED éteintes
* on désalloue les GPIO 
*
*/
static void __exit fonctionExit(void){
	misc_deregister(&led);
	gpio_direction_output(ledTab[0].gpio,1);
	gpio_direction_output(ledTab[1].gpio,1);	
	gpio_free_array(ledTab,ARRAY_SIZE(ledTab));
}

/**
*
* @param fil : pointeur sur fichier virtuel
*
*/

static int dev_rls(struct inode *inod, struct file *fil) {
	printk(KERN_ALERT "Alerte fermeture !! \n");
	return 0;
}

/**
*
* @param fil : pointeur sur fichier virtuel
*
*/

static int dev_open(struct inode *inod, struct file *fil) {
	printk(KERN_ALERT "Alerte ouverture !! \n");
	return 0;
}

/**
*
* @param fil : pointeur sur fichier virtuel
* @return : taille du buffer effectivement lue
*
*/
static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off) {
	printk(KERN_INFO "Alerte lecture !! \n");
	return 0;
}

/**
*
* @param fil : pointeur sur fichier virtuel
* @param buff : pointeur sur espace mémoire utilisé par copy_from_user
* @return : taille effectivement lue
*
*
* Quand le programme utilisateur écrit dans le fichier virtuel associé à ce module,
* cette fonction va allumer la LED correspondante en fonction de la valeur écrite.
*
*/
static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off) {
	printk(KERN_ALERT "Alerte écriture !! \n");
	int dummy, mylen;
	char buf[2] = "?";
	if (len > 2) mylen = 2; 
	else mylen = len;
	
	// on récupère la valeur envoyée par le programme utilisateur
	dummy = copy_from_user(buf, buff, mylen);
	buf[mylen] = 0;
	printk(KERN_ALERT "buffer : %s test\n", buf);

	// si la valeur récupérée est '1' on  allume la LED verte
	if(buf[0] == '1') {
		gpio_direction_output(ledTab[0].gpio,1);
		gpio_direction_output(ledTab[1].gpio,0);
	}
	// si la valeur récupérée est '2' on allume la LED rouge
	else if (buf[0] == '2') {
        	gpio_direction_output(ledTab[0].gpio,0);
		gpio_direction_output(ledTab[1].gpio,1);		
	}
	// sinon on éteint les 2 LED
	else {
                gpio_direction_output(ledTab[1].gpio,1);
		gpio_direction_output(ledTab[0].gpio,1);
        }

	return mylen;
}


module_init(fonctionInit);
module_exit(fonctionExit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Maeckereel & Rémi Morel");
MODULE_DESCRIPTION("Test Digicode");
