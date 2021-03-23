#include "libsockets.h"
#include "libthreads.h"

#define PORT 1337
#define MAX_LIGNE 1024

struct sigaction action;
int statut=0;

void hand(int sig){
	if(sig==SIGINT){
		statut=1;
	}
}

void communicationServeur(void *pack){
	server_t *server=pack;

	//FILE *fileSock=fdopen(server->socketTCP,"a+");
	char tampon[MAX_TAMPON];

	/* Envoi du pseudo */
	printf("Quel est votre pseudo ?\n");
	char pseudo[20]; // Ajout d'une constante pour le max dans pseudo
	fgets(pseudo,MAX_TAMPON,stdin);
	fflush(stdin);
	char str[MAX_TAMPON];
	sprintf(str,"CONNEXION %s",pseudo);
	fprintf(server->fileSock,"%s",str);
	fflush(server->fileSock);

	/* Poll entree standard et socket TCP*/
	struct pollfd descripteurs[2];
	descripteurs[0].fd=server->socketTCP;
	descripteurs[0].events=POLLIN;
	descripteurs[1].fd=0;
	descripteurs[1].events=POLLIN;

	while(statut!=1){
		memset(tampon,0,MAX_TAMPON);
		int nb=poll(descripteurs,2,-1);
		if(nb<0){ perror("main.poll"); exit(EXIT_FAILURE); }
		if((descripteurs[0].revents&POLLIN)!=0){
			char cmd[MAX_TAMPON];
			char args[MAX_TAMPON];
			int reste;
			if(ioctl(server->socketTCP,FIONREAD,&reste)<0 ||reste==0){ 
				//fclose(fileSock);
				statut=1; 
			}
			char msg[MAX_LIGNE];
			memset(msg,0,MAX_LIGNE);
			while(reste>0){
				if(fgets(tampon,MAX_LIGNE,server->fileSock)==NULL){
					statut=1;
					//fclose(fileSock);
				}
				if(fflush(server->fileSock)){
					perror("fflush");
					exit(EXIT_FAILURE);
				}
				nb=sscanf(tampon,"%s %[^\n]",cmd,args);
				if(nb>0){
					//Traitement des commandes d'entrée
					if(strcmp(cmd,"CONNECTE")==0){
						fprintf(stdout,"%s",tampon);
						fflush(stdout);
					}
					if(strcmp(cmd,"JOUEURS")==0){
						fprintf(stdout,"%s",tampon);
						fflush(stdout);
					}
					//TODO : Plus belle messagerie
					if(strcmp(cmd,"MSGFROM")==0){
						sprintf(msg,"<%s>",args);
						fprintf(stdout,"%s",tampon);
						fflush(stdout);
					}
					if(strcmp(cmd,"MSG")==0){
						fprintf(stdout,"%s : %s\n",msg,args);
						fflush(stdout);
					}
					reste-=strlen(tampon);
					printf("reste = %d\n",reste);
				}
			}
		}	
		if((descripteurs[1].revents&POLLIN)!=0){
			fgets(tampon,MAX_LIGNE,stdin);
			fflush(stdin);
			char cmd[MAX_TAMPON];
			char args[MAX_LIGNE];
			char msg[MAX_LIGNE];
			memset(msg,0,MAX_LIGNE);
			int ecrit=0;
			int nb=sscanf(tampon,"%s %[^\n]",cmd,args);
			if(nb>0){
				if(strcmp(cmd,"/start")==0){
					fprintf(server->fileSock,"CMD START\n");
					fflush(server->fileSock);
					ecrit=1;
				}
				if(strcmp(cmd,"/stop")==0){
					fprintf(server->fileSock,"CMD STOP\n");
					fflush(server->fileSock);
					ecrit=1;
				}
				if(!ecrit){
					sprintf(msg,"MSG %s",tampon);
					fprintf(server->fileSock,"%s",msg);
					fflush(server->fileSock);
				}
			}
		}
	}
	printf("Deconnexion avec le serveur TCP, appuyez sur CTRL+C pour arreter le client\n");
}

int main(){
	/* === Init du signal d'arret de la connexion TCP avec le serveur === */
	action.sa_handler=hand;
	sigaction(SIGINT,&action,NULL);
	
	/* recuperation du broadcast UDP des serveurs et choix d'un serveur */
	server_t serv;
	serv = udpEcoute(PORT);
	/* === Informations sur le serveur choisi === */
	printf("Le port de la partie choisie est : %s\n",serv.portTCP);
	printf("Adresse de l'hote : %s\n",inet_ntoa(serv.addr_Server));

	/* === Connexion TCP client : utilisation du cours === */
	serv.socketTCP=connexionServ(serv);
	serv.fileSock=fdopen(serv.socketTCP,"a+");
	launchThread(communicationServeur,&serv,sizeof(serv));
	/* Fin de la connexion */

	pause();

	fclose(serv.fileSock);
	close(serv.socketTCP);
	printf("\nFin du client ! \n");
	return 0;
}
