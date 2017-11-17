#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

static char code[4] = {'1','5','2','8'};

/**

**/
int main(int argc, char** argv){
	int fourTouch, echec,i;
	short revents;
	struct pollfd pfd;
	char tentative[4];
	int nbTouch, j, returnRead;
	FILE *f,*fbis;
	char carRec;
	char color;

	printf("Debug 1\n");

	fourTouch = 0;
	nbTouch = 0;

	if((f = fopen("/dev/digicode","r")) > 0){
		fprintf(stderr,"%s: unable to open %s: %s\n","Digigame","/dev/digicode",errno);
        }
	pfd.fd = fileno(f);
	pfd.events = POLLIN;
	//Récupération de 4 touches tapées
	while(fourTouch == 0){
		printf("Entrée boucle before poll\n");
		//Demande récupération caractère
		i = poll(&pfd, 1, -1);
		if(i == -1){
			perror("poll");
			exit(EXIT_FAILURE);
		}
		revents = pfd.revents;
		if(revents & POLLIN){
			returnRead = fread(&carRec,1,1,f);
			printf("Touche frappée : %c\n",carRec);
			tentative[nbTouch] = carRec;
			nbTouch++;
			sleep(1);
		}
		//sinon ajout dans tentative[nbTouch] puis nbTouch++
		if(nbTouch >= 4){
			fourTouch = 1;
		}
	}
	fclose(f);

	printf("Debug 2\n");

	echec = 0;

	//Vérification du code tapé
	for(j=0;j<4;j++){
		printf("tentative %d : %c \n", j,  tentative[j]); 
		if(code[j] != tentative[j]){
			echec = 1;
			break;
		}
	}

	printf("Debug 3\n");

	//Détermination de la couleur de sortie
	if(echec == 1){
		//allumer ROUGE
		color = '2';
		printf("Code incorrect\n");
	}else{
		//allumer VERT
		color = '1';
		printf("Code correct\n");
	}

	printf("Debug 4\n");

	//Ecriture dans fichier virtuel de la led
	if((fbis = fopen("/dev/led","w")) == NULL){
                fprintf(stderr,"%s: unable to open %s: %s\n", "Digigame", "/dev/led", strerror(errno));
        }
	fputc(color,fbis);
	fclose(fbis);	
	sleep(3);

	 if((fbis = fopen("/dev/led","w")) > 0){
                fprintf(stderr,"%s: unable to open %s: %s\n", "Digigame", "/dev/led", strerror(errno));
        }
	color = '0';
	fputc(color,fbis);
	fclose(fbis);

	return 0;
}
