#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/poll.h>

static int dev_open(struct inode *inod, struct file *fil);
static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off);
static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off);
static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
static int dev_rls(struct inode *inod, struct file *fil);

static unsigned int dev_poll(struct file *file, poll_table *wait);
static DECLARE_WAIT_QUEUE_HEAD(fortune_wait);

static irq_handler_t handler_wake_user(unsigned int irq, void* dev_id, struct pt_regs *regs);
static int irqNumbers[4];


struct miscdevice digicode;

static struct file_operations fops = {
	.read=dev_read,
	.write=dev_write,
	.open=dev_open,
	.unlocked_ioctl=dev_ioctl,
	.release=dev_rls,
	.poll=dev_poll
};

//Tableau des gpios utilisées pour le KeyPad
static struct gpio gpTab[] = {
	{1, GPIOF_OUT_INIT_HIGH, "COLONNE 2"},
	{2, GPIOF_IN, "LIGNE 1"},
	{4, GPIOF_OUT_INIT_HIGH, "COLONNE 1"},
	{5, GPIOF_IN, "LIGNE 4"},
	{6, GPIOF_OUT_INIT_HIGH, "COLONNE 3"},
	{7, GPIOF_IN, "LIGNE 3 "},
	{16, GPIOF_IN, "LIGNE 2"}
};

static char keypad[4][3] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};

int ligne, colonne;


static unsigned int dev_poll(struct file *file, poll_table *wait){
	printk(KERN_INFO"Entry in Poll function\n");
	poll_wait(file, &fortune_wait, wait);
	if((gpio_get_value(gpTab[1].gpio) == 1) || (gpio_get_value(gpTab[3].gpio) == 1) || (gpio_get_value(gpTab[5].gpio) == 1) || (gpio_get_value(gpTab[6].gpio) == 1))
		return POLLIN;
	return 0;
}

static irq_handler_t handler_wake_user(unsigned int irq, void* dev_id, struct pt_regs *regs){
	wake_up_interruptible(&fortune_wait);

	return (irq_handler_t) IRQ_HANDLED;
}

/**
	Fonction d'Init du module Noyau
**/
static int __init fonctionInit(void){
	int returnCode;
	digicode.name = "digicode";
	digicode.minor = MISC_DYNAMIC_MINOR;
	digicode.fops = &fops;
	misc_register(&digicode);

        //réservation des GPIO
        returnCode = gpio_request_array(gpTab,ARRAY_SIZE(gpTab));
        if(returnCode != 0){
                printk(KERN_INFO"Error allocate GPIO\n");
                gpio_free_array(gpTab,ARRAY_SIZE(gpTab));
        }

	printk(KERN_INFO "Démarrage de folie \n");

	irqNumbers[0] = gpio_to_irq(gpTab[1].gpio);
	returnCode = request_irq(irqNumbers[0],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);

	irqNumbers[1] = gpio_to_irq(gpTab[3].gpio);
        returnCode = request_irq(irqNumbers[1],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);

	irqNumbers[2] = gpio_to_irq(gpTab[5].gpio);
        returnCode = request_irq(irqNumbers[2],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);

	irqNumbers[3] = gpio_to_irq(gpTab[6].gpio);
        returnCode = request_irq(irqNumbers[3],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);

	return 0;
}

/**
	Fonction Exit du module Noyau
**/
static void __exit fonctionExit(void){
	gpio_free_array(gpTab,ARRAY_SIZE(gpTab));
	misc_deregister(&digicode);
}

static int dev_rls(struct inode *inod, struct file *fil) {
	//printk(KERN_ALERT "Alerte fermeture !! \n");
	return 0;
}

static int dev_open(struct inode *inod, struct file *fil) {
	//printk(KERN_ALERT "Alerte ouverture !! \n");
	return 0;
}

static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off) {
	int returnCode,i;
	gpio_direction_output(gpTab[0].gpio,0);
	gpio_direction_output(gpTab[2].gpio,0);
	gpio_direction_output(gpTab[4].gpio,0);
         for(i=0;i<5;i=i+2){
                colonne = -1;
                switch(i){
                        case 0:
                                colonne = 2;
                                break;
                        case 2:
                                colonne = 1;
                                break;
                        case 4:
                                colonne = 3;
                                break;
                }

		gpio_direction_output(gpTab[i].gpio,1);
                if(gpio_get_value(gpTab[1].gpio) == 1){
                        ligne = 1;
                        break;
                }else{
                        if(gpio_get_value(gpTab[3].gpio) == 1){
                                ligne = 4;
                                break;
                        }else{
                                if(gpio_get_value(gpTab[5].gpio) == 1){
                                        ligne = 3;
                                        break;
				}else{
                                        if(gpio_get_value(gpTab[6].gpio) == 1){
                                                ligne = 2;
                                                break;
                                        }else{
                                                ligne = -1;
                                                gpio_direction_output(gpTab[i].gpio,0);
                                        }
                               }
                        }
                }
        }

	gpio_direction_output(gpTab[i].gpio,0);
        if((ligne == -1) || (colonne == -1)){
                printk(KERN_INFO"Aucune touche appuyée\n");
        }else{
                printk(KERN_INFO"Touche sélectionnée : %c\n",keypad[ligne-1][colonne-1]);
        }

	int dummy, mylen;
	dummy = copy_to_user(buff, &(keypad[ligne - 1][colonne - 1]), 1);
	printk(KERN_ALERT "%c", keypad[ligne-1][colonne-1]);

	gpio_direction_output(gpTab[0].gpio,1);
        gpio_direction_output(gpTab[2].gpio,1);
        gpio_direction_output(gpTab[4].gpio,1);


	return 1;
}

static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off) {
	return 0;
}

static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
	static char var[10];
	int dummy;
	//printk(KERN_ALERT "Alerte %d", cmd);
	switch (cmd) {
		case 0 : dummy = copy_from_user(var, (char*)arg, 5);
			//printk(KERN_ALERT "Alerte 0 %s \n", (char*) var);
			break;
		case 1 : //printk(KERN_ALERT "Alerte 1 %s \n", (char*) var);
			dummy = copy_to_user((char*) arg, var, 5);
			break;
		default : //printk(KERN_ALERT "unknown");
			break;
	}
	return 0;
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Maeckereel & Rémi Morel");
MODULE_DESCRIPTION("Test Digicode");
