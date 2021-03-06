#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

/**
*
* Programme noyau linux utilisateur.
* Il permet de communiquer avec l'utilisateur : 
* il affiche des informations quand on le lance, quand on écrit
* dans le fichier virtuel associé, quand on lit dedans,
* et quand on quitte le module.
*
*/

// Prototypes des fonctions opérations sur le fichier virtuel
static int dev_open(struct inode *inod, struct file *fil);
static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off);
static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off);
static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
static int dev_rls(struct inode *inod, struct file *fil);

struct miscdevice com;

int hello_start(void);
void hello_end(void);

// affectation des opérations fichiers aux méthodes associées 
static struct file_operations fops = {
	.read=dev_read,
	.write=dev_write,
	.open=dev_open,
	.unlocked_ioctl=dev_ioctl,
	.release=dev_rls
};

// On affiche "Démarrage de folie " au lancement du module
int hello_start() {
	com.name = "com";
	com.minor = MISC_DYNAMIC_MINOR;
	com.fops = &fops;
	misc_register(&com);
	
	printk(KERN_INFO "Démarrage de folie \n");
	return 0;
}

// On affiche "A la prochaine" quand on quitte le module
void hello_end() {
	printk(KERN_INFO "A la prochaine \n");
	misc_deregister(&com);
}

// On affiche "Alerte fermeture !!" quand on ferme le fichier virtuel
static int dev_rls(struct inode *inod, struct file *fil) {
	printk(KERN_ALERT "Alerte fermeture !! \n");
	return 0;
}

// On affiche "Alerte ouverture !!" quand on ouvre le fichier virtuel
static int dev_open(struct inode *inod, struct file *fil) {
	printk(KERN_ALERT "Alerte ouverture !! \n");
	return 0;
}

/* 
 * On affiche "Alerte lecture !!" quand on lit le fichier virtuel.
 * On envoie à l'utilisateur le contenu du buffer du fichier virtuel.
 */
static ssize_t dev_read(struct file *fil, char *buff, size_t len, loff_t *off) {
	char buf[11] = "Lecture \n";
	int dummy, readPos = strlen(buf);
	printk(KERN_ALERT "Alerte lecture !!");
	dummy = copy_to_user(buff, buf, readPos);
	return readPos;
}

/*
 * On affiche "Alerte écriture !!" quand on écrit dans le fichier virtuel.
 * On récupère le contenu du buffer et on l'affiche dans la console.
 */
static ssize_t dev_write(struct file *fil, const char *buff, size_t len, loff_t *off) {
	int dummy, mylen;
	char buf[15] = "Ecriture";
	printk(KERN_ALERT "Alerte écriture !!");
	if (len > 14) mylen = 14; 
	else mylen = len;
	dummy = copy_from_user(buf, buff, mylen);
	buf[mylen] = 0;
	printk(KERN_ALERT "%s", buf);
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

module_init(hello_start);
module_exit(hello_end);
