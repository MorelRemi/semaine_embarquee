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

/**
*
* 	Directives de pré-processeur
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

// fonction qui indique à l'utilisateur si des données à récupérer sont disponibles
static unsigned int dev_poll(struct file *file, poll_table *wait);


// création d'une Wait Queue pour la fonction poll 

static DECLARE_WAIT_QUEUE_HEAD(fortune_wait);

// fonction qui va gérer les interruptions déclenchées par les GPIO d'entrée
static irq_handler_t handler_wake_user(unsigned int irq, void* dev_id, struct pt_regs *regs);

// création du fichier virtuel
struct miscdevice digicode;

static struct file_operations fops = {
	.read=dev_read,
	.write=dev_write,
	.open=dev_open,
	.release=dev_rls,
	.poll=dev_poll
};

/* Tableau des gpios utilisées pour le KeyPad
*
* 4 en entrée pour les lignes
* 3 en sortie pour les colonnes
*/
static struct gpio gpTab[] = {
	{1, GPIOF_OUT_INIT_HIGH, "COLONNE 2"},
	{2, GPIOF_IN, "LIGNE 1"},
	{4, GPIOF_OUT_INIT_HIGH, "COLONNE 1"},
	{5, GPIOF_IN, "LIGNE 4"},
	{6, GPIOF_OUT_INIT_HIGH, "COLONNE 3"},
	{7, GPIOF_IN, "LIGNE 3 "},
	{16, GPIOF_IN, "LIGNE 2"}
};

// tableau de char qui modélise le keypad
static char keypad[4][3] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};

// une irq pour chaque GPIO d'entrée
static int irqNumbers[4] = {0,0,0,0};
int ligne, colonne;

/**
*
* @param file : pointeur sur fichier virtuel
* @return : code d'erreur à tester
*
* est appelée par le programme utilisateur
* regarde si une des GPIO d'entrée reçoit un courant (une touche est donc pressée)
*
*/
static unsigned int dev_poll(struct file *file, poll_table *wait){
	printk(KERN_INFO"Entry in Poll function\n");
	poll_wait(file, &fortune_wait, wait);
	if((gpio_get_value(gpTab[1].gpio) == 1) || (gpio_get_value(gpTab[3].gpio) == 1) || (gpio_get_value(gpTab[5].gpio) == 1) || (gpio_get_value(gpTab[6].gpio) == 1))
		return POLLIN;
	return 0;
}


/**
*
* @param irq : l'irq de la GPIO qui va déclencher l'interruption
* @return : code d'erreur à tester
*
* réveille le programme utilisateur endormi par le poll
*
*/
static irq_handler_t handler_wake_user(unsigned int irq, void* dev_id, struct pt_regs *regs){
	wake_up_interruptible(&fortune_wait);

	return (irq_handler_t) IRQ_HANDLED;
}

/**
*	Fonction d'Init du module Noyau
* 
* @return : code d'erreur à tester
*/
static int __init fonctionInit(void){
	int returnCode,j;
	digicode.name = "digicode";
	digicode.minor = MISC_DYNAMIC_MINOR;
	digicode.fops = &fops;
	misc_register(&digicode);

        // réservation des GPIO
        returnCode = gpio_request_array(gpTab,ARRAY_SIZE(gpTab));
  	// test du code de retour
        if(returnCode != 0){
                printk(KERN_INFO"Error allocate GPIO\n");
		goto errorAllocate;
        }

	printk(KERN_INFO "Démarrage de folie \n");

	/* association de chaque GPIO d'entrée avec un irq
	*  cet irq est stocké dans le tableau irqNumber
	*/
	irqNumbers[0] = gpio_to_irq(gpTab[1].gpio);
	returnCode = request_irq(irqNumbers[0],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);
	if(returnCode != 0){
		printk(KERN_INFO"Error request IRQ\n");
                goto errorRequestIrq;
	}


	irqNumbers[1] = gpio_to_irq(gpTab[3].gpio);
        returnCode = request_irq(irqNumbers[1],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);
	if(returnCode != 0){
		printk(KERN_INFO"Error request IRQ\n");
		goto errorRequestIrq;
	}

	irqNumbers[2] = gpio_to_irq(gpTab[5].gpio);
        returnCode = request_irq(irqNumbers[2],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);
	if(returnCode != 0){
		printk(KERN_INFO"Error request IRQ\n");
                goto errorRequestIrq;
	}

	irqNumbers[3] = gpio_to_irq(gpTab[6].gpio);
        returnCode = request_irq(irqNumbers[3],(irq_handler_t)handler_wake_user,IRQF_TRIGGER_RISING,"handler_wake_user",NULL);
	if(returnCode != 0){
		printk(KERN_INFO"Error request IRQ\n");
                goto errorRequestIrq;
	}

	return 0;

	//Gestion des codes d'erreurs
	errorRequestIrq:
		for(j=0;j<4;j++){
			free_irq(irqNumbers[j],NULL);
		}
	errorAllocate:
		gpio_free_array(gpTab,ARRAY_SIZE(gpTab));

	return 1;
}

/**
*	Fonction Exit du module Noyau
* on désalloue les GPIO
**/
static void __exit fonctionExit(void){
	int i;

	gpio_free_array(gpTab,ARRAY_SIZE(gpTab));
	for(i=0;i<4;i++){
		free_irq(irqNumbers[i],NULL);
	}
	misc_deregister(&digicode);
}

/**
*
* @param fil : pointeur sur fichier virtuel
*
*/
static int dev_rls(struct inode *inod, struct file *fil) {
	//printk(KERN_ALERT "Alerte fermeture !! \n");
	return 0;
}

/**
*
* @param fil : pointeur sur fichier virtuel
*
*/
static int dev_open(struct inode *inod, struct file *fil) {
	//printk(KERN_ALERT "Alerte ouverture !! \n");
	return 0;
}

/**
*
* @param fil : pointeur sur fichier virtuel
* @param buff : pointeur sur espace mémoire utilisé par copy_to_user
* @return : taille effectivement lue
*
*
* Quand le programme utilisateur lit dans le fichier virtuel associé à ce module,
* cette fonction va déterminer quelle touche est pressée sur le keypad.
*
* L'algorithme consiste à désactiver le courant dans toutes les colonnes
* (les GPIO de sortie) puis à le réactiver une par une. ce faisant on sera en mesure
* de déterminer la ligne et la colonne de la touche.
*
* On envoit ensuite la touche trouvée sous forme d'un char au programme utilisateur via 
* la fonction copy_to_user.
*
*/
static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off) {
	int dummy,i;
	// on désactive le courant sur les colonnes
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
		dummy = copy_to_user(buff, &(keypad[ligne - 1][colonne - 1]), 1);
        }

	printk(KERN_ALERT "%c", keypad[ligne-1][colonne-1]);

	// on remet le courant dans les colonnes
	gpio_direction_output(gpTab[0].gpio,1);
        gpio_direction_output(gpTab[2].gpio,1);
        gpio_direction_output(gpTab[4].gpio,1);


	return 1;
}

static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off) {
	return 0;
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Maeckereel & Rémi Morel");
MODULE_DESCRIPTION("Test Digicode");
