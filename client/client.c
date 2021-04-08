#include "libsockets.h"
#include "libthreads.h"
#include "libgraph.h"
#include "common.h"

#define PORT 1337
#define MAX_LIGNE 1024

struct sigaction action;
int statut=0;

typedef struct envTouche_s{
	uint8_t id;
	uint8_t touche;
}envTouche_t;

void hand(int sig){
	if(sig==SIGINT){
		statut=1;
	}
}

/* === POLL SUR ENTREE STANDARD ET LA SOCKET POUR AFFICHAGE ET CHOIX DU SERVEUR === */
server_t pollEcoute(int s){
    	struct pollfd descripteurs[2];
    	descripteurs[0].fd=s;
    	descripteurs[0].events=POLLIN;
    	descripteurs[1].fd=0;
    	descripteurs[1].events=POLLIN;
    	//struct sockaddr_in other_socket;
    	int choix;
    	char buffer[50];
	char servAddr[50];
    	int keepRunning = 1;
    	server_t tab[MAX_SERVER];
    	int nbServ=0;
	printf("Entrez le numero du serveur souhaite a n'importe quel moment\n");
	while(keepRunning){
		int nb = poll(descripteurs,2,-1);
		if(nb<0){
			perror("udpEcoute.poll");
			exit(EXIT_FAILURE);
		}
		if((descripteurs[0].revents&POLLIN)!=0){
			memset(buffer,0,sizeof(buffer));
			memset(servAddr,0,sizeof(servAddr));
			//unsigned sock_len=sizeof(struct sockaddr);
			// TODO : Modif
			receptionServer(s,buffer,servAddr,sizeof(buffer),sizeof(servAddr));
			//recvfrom(s, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&other_socket, &sock_len);
			char name[50];
			strcpy(name,buffer);
			for(long unsigned int i=0;i<sizeof(name)-1;i++){
				name[i]=name[i+2];
			}
			int flag=0;
			if(nbServ!=0){
				for(int i=0; i<=nbServ;i++){
					/* === si le serveur est deja trouve, on ajoute pas le server dans le tableau ===*/
					if(strcmp(name,tab[i].nom_Server)==0){
						flag=1;
					}
				}
			}
			if(flag==0){
				nbServ++;
				strcpy(tab[nbServ].nom_brut,buffer);
				int port = buffer[0] + (buffer[1]<<8);
				snprintf(tab[nbServ].portTCP,5,"%d",port);
				/*	
				for(long unsigned int i=0;i<sizeof(buffer)-1;i++){
					buffer[i]=buffer[i+2];
				}
				*/
				strcpy(tab[nbServ].nom_Server,name);
				strcpy(tab[nbServ].addr_Server,servAddr);
				//tab[nbServ].addr_Server=other_socket.sin_addr;
				printf("%d) La partie de jeu %s est sur le port : %d\n",nbServ,name,port);
			}
		}
		if((descripteurs[1].revents&POLLIN)!=0){
			if(nbServ!=0){
				int taille=read(0,buffer,sizeof(buffer));
				if(taille<0) break;
				choix=atoi(buffer);
				while(choix>nbServ || choix<=0){
					printf("Veuillez entrer votre choix de serveur (entre 1 et %d) : ",nbServ);
					scanf("%d",&choix);
				}
				keepRunning=0;
			}
		}
	}
	return tab[choix];
}
/*
void gestionJeu(void *pack){
	unsigned char *fenetre=pack;
}
*/
void envoieTouches(void *pack){
	server_t *server=pack;
	struct sockaddr_in Addr;
	int socket=udpEcoute(atoi(server->portTCP)+1);
	//Sockaddr_in sur l'adresse du server
	Addr=createAddr(atoi(server->portTCP)+1,server->addr_Server);
	unsigned char resultat,fenetre,quitter;
	int touche;
	int recu;
	envTouche_t envoi;
	envoi.id=server->id;
	socklen_t size = sizeof(struct sockaddr_in);
	//TODO : Modifier avec var globales comme pour labyrinthe.c
	resultat=creerFenetre(640,480,"Test labyrinthe");
	if(!resultat){ fprintf(stderr,"Probleme graphique\n"); exit(-1); }
	while(statut!=1){
		recu=0;
		int evenement=attendreEvenement(&touche,&fenetre,&quitter);
		if(!evenement){ usleep(10000); continue; };
		if(touche){
			if(touche==TOUCHE_GAUCHE) { envoi.touche=0b00000001; recu=1; }
			if(touche==TOUCHE_DROITE) { envoi.touche=0b00000010; recu=1; }
			if(touche==TOUCHE_HAUT) { envoi.touche=0b00000100; recu=1; }
			if(touche==TOUCHE_BAS) { envoi.touche=0b00001000; recu=1; }
			if(touche==TOUCHE_ESPACE) { envoi.touche=0b00010000; recu=1; }
		}
		if(quitter==1){ envoi.touche=0b00100000; printf("Quitter\n"); break; }
		if(recu){
			if((sendto(socket,(void *)&envoi,sizeof(envTouche_t),0,(struct sockaddr *)&Addr,size))==-1){
				perror("envoieTouches.sendto");
			}
		}
	}
	fermerFenetre();
	close(socket);
	printf("Fin de thread envoie des touches\n");
}

void communicationServeur(void *pack){
	server_t *server=pack;
	server_t serv=*server;

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
			char pseudo[MAXNAME];
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
				nb=sscanf(tampon,"%s %[^\r]",cmd,args);
				if(nb>0){
					//Traitement des commandes d'entrée
					if(strcmp(cmd,"CONNECTE")==0){
						serv.id=atoi(args);
						fprintf(stdout,"%s",tampon);
						fflush(stdout);
					}
					if(strcmp(cmd,"JOUEURS")==0){
						fprintf(stdout,"%s",tampon);
						fflush(stdout);
					}
					if(strcmp(cmd,"MSGFROM")==0){
						strcpy(pseudo,args);
						fflush(stdout);
					}
					if(strcmp(cmd,"MSG")==0){
						fprintf(stdout,"<%s> : %s",pseudo,args);
						fflush(stdout);
					}
					if(strcmp(cmd,"CMD")==0){
						if(strcmp(args,"START")==0){
							launchThread(envoieTouches,&serv,sizeof(serv));
						}
					}
					reste-=strlen(tampon);
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
	int socket = udpEcoute(PORT);
	serv=pollEcoute(socket);
	/* === Informations sur le serveur choisi === */
	printf("Le port de la partie choisie est : %s\n",serv.portTCP);
	printf("Adresse de l'hote : %s\n",serv.addr_Server);
	printf("Port UDP de l'envoi des touches : %d\n",atoi(serv.portTCP)+1);

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
