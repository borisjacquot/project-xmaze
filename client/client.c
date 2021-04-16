#include "client.h"

struct sigaction action;
int statut=0;

void hand(int sig){
	if(sig==SIGINT){
		statut=1;
	}
}

/* Intersection d'un segment */

unsigned char inter_seg_v(point2D a,point2D b,int x,int *y){
if(sign(a.x-x)==sign(b.x-x)) return 0;
*y=a.y+(b.y-a.y)*(x-a.x)/(b.x-a.x);
return 1;
}

unsigned char inter_seg_h(point2D a,point2D b,int *x,int y){
if(sign(a.y-y)==sign(b.y-y)) return 0;
*x=a.x+(b.x-a.x)*(y-a.y)/(b.y-a.y);
return 1;
}

/* Intersection d'un polygone avec un rectangle */

void inter_poly_rect(point2D *orig,int no,point2D *result,int *nr){
point2D avant[POINTS_MAX];
point2D apres[POINTS_MAX];
int i,j;
for(i=0;i<no;i++) avant[i]=orig[i];
#ifdef DEBUG
printf("??");
for(i=0;i<no;i++) printf("(%d,%d) ",avant[i].x,avant[i].y);
printf("\n");
#endif
int nv=no;
for(i=0;i<4;i++){
  int np=0;
  for(j=0;j<nv;j++){
    int p=(j+nv-1)%nv;
    point2D a=avant[p];
    point2D b=avant[j];
    int x,y;
    unsigned char inta,intb;
    switch(i){
      case 0:
        x=0;
        inter_seg_v(a,b,x,&y);
        if(a.x>=0) inta=1; else inta=0;
        if(b.x>=0) intb=1; else intb=0;
        break;
      case 1:
        y=HAUTEUR;
        inter_seg_h(a,b,&x,y);
        if(a.y<=HAUTEUR) inta=1; else inta=0;
        if(b.y<=HAUTEUR) intb=1; else intb=0;
        break;
      case 2:
        x=LARGEUR;
        inter_seg_v(a,b,x,&y);
        if(a.x<=LARGEUR) inta=1; else inta=0;
        if(b.x<=LARGEUR) intb=1; else intb=0;
        break;
      case 3:
        y=0;
        inter_seg_h(a,b,&x,y);
        if(a.y>=0) inta=1; else inta=0;
        if(b.y>=0) intb=1; else intb=0;
        break;
      }
    if(intb){
      if(!inta){ apres[np].x=x; apres[np].y=y; np++; }
      apres[np++]=b;
      }
    else{
      if(inta){ apres[np].x=x; apres[np].y=y; np++; }
      }
    }
  for(j=0;j<np;j++) avant[j]=apres[j];
  nv=np;
  }
for(i=0;i<nv;i++) result[i]=avant[i];
#ifdef DEBUG
printf("!!");
for(i=0;i<nv;i++) printf("(%d,%d) ",result[i].x,result[i].y);
printf("\n");
#endif
*nr=nv;
}

/* Dessin d'un labyrinthe */

void dessine_2D(objet2D *objet,int no){
int i,j;
short int x[POINTS_MAX];
short int y[POINTS_MAX];
for(i=0;i<no;i++){
  if(objet[i].type==TYPE_MUR){
    point2D poly[POINTS_MAX];
    int np;
    inter_poly_rect(objet[i].def.p,4,poly,&np);
    for(j=0;j<np;j++){
      x[j]=poly[j].x;
      y[j]=HAUTEUR-poly[j].y;
    }
    polygonePlein(x,y,np,COULEUR_ROUGE,COULEUR_ROSE);
    }
  }
}

/* === POLL SUR ENTREE STANDARD ET LA SOCKET POUR AFFICHAGE ET CHOIX DU SERVEUR === */
server_t pollEcoute(int s){
    	int choix;
    	char buffer[MAX_TAMPON];
	char hostname[MAX_HOSTNAME];
	char name[MAX_TAMPON];
    	server_t tab[MAX_SERVER];
    	int keepRunning = 1;
    	int nbServ=0;
	int flag;
	int port;

    	struct pollfd descripteurs[2];
    	descripteurs[0].fd=s;
    	descripteurs[0].events=POLLIN;
    	descripteurs[1].fd=0;
    	descripteurs[1].events=POLLIN;
	printf("Entrez le numero du serveur souhaite a n'importe quel moment\n");
	while(keepRunning){
		int nb = poll(descripteurs,2,-1);
		if(nb<0){
			perror("udpEcoute.poll");
			exit(EXIT_FAILURE);
		}
		if((descripteurs[0].revents&POLLIN)!=0){
			memset(buffer,0,sizeof(buffer));
			memset(hostname,0,sizeof(hostname));
			receptionServer(s,buffer,hostname,sizeof(buffer),sizeof(hostname));
			strcpy(name,buffer);
			for(long unsigned int i=0;i<sizeof(name)-1;i++){
				name[i]=name[i+2];
			}
			flag=0;
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
				port = buffer[0] + (buffer[1]<<8);
				snprintf(tab[nbServ].portTCP,5,"%d",port);
				strcpy(tab[nbServ].nom_Server,name);
				strcpy(tab[nbServ].hostname,hostname);
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

/* Envoie des touches au serveur + gestion affichage */
void envoieTouches(void *pack){
	server_t *server=pack;
	unsigned char resultat,fenetre,quitter;
	char envoi[TAILLE_TOUCHES];
	int touche;
	int recu;
	int portUDP=atoi(server->portTCP)+1;
	int socket;
	int newsock;
	if(compareAdresse(server->hostname)){
		socket=udpInit(portUDP,1,server->hostname,0);
		newsock=udpInit(atoi(server->portTCP),1,server->hostname,1);
	}else{
		socket=udpInit(portUDP,0,server->hostname,0);
		newsock=udpInit(atoi(server->portTCP),0,server->hostname,1);
	}

	resultat=creerFenetre(LARGEUR,HAUTEUR,TITRE);
	if(!resultat){ fprintf(stderr,"Probleme graphique\n"); exit(-1); }

	while(statut!=1){
		/* Envoie touche */
		recu=0;
		int evenement=attendreEvenement(&touche,&fenetre,&quitter);
		if(!evenement){ usleep(10000); continue; };
		if(touche){
			if(touche==TOUCHE_GAUCHE) { envoi[1]=0b00000001; recu=1; }
			if(touche==TOUCHE_DROITE) { envoi[1]=0b00000010; recu=1; }
			if(touche==TOUCHE_HAUT) { envoi[1]=0b00000100; recu=1; }
			if(touche==TOUCHE_BAS) { envoi[1]=0b00001000; recu=1; }
			if(touche==TOUCHE_ESPACE) { envoi[1]=0b00010000; recu=1; }
		}
		if(quitter==1){ envoi[1]=0b00100000; printf("Quitter\n"); break; }
		if(recu){
			envoieTouche(socket,portUDP,envoi,TAILLE_TOUCHES,server->hostname);
		}
		/* Reception et affichage jeu */
		objet2D *objets=malloc(sizeof(objet2D));
		receptionObjets(newsock,(void *)&objets,sizeof(objet2D),server->hostname,atoi(server->portTCP));
		effacerFenetre();
		dessine_2D(objets,sizeof(objets));
		free(objets);
		synchroniserFenetre();
	}
	fermerFenetre();
	close(socket);
	close(newsock);
	printf("Fin de thread envoie des touches et gestion du jeu\n");
}

void traitementCMD(char *tampon,char *cmd,char *args,server_t *server,int isReception){
	char msg[MAX_LIGNE];
	char pseudo[MAXNAME];
	memset(msg,0,MAX_LIGNE);
	if(isReception){
		if(strcmp(cmd,CONNECTE)==0){
			server->id=atoi(args);
			fprintf(stdout,"%s",tampon);
			fflush(stdout);
		}
		if(strcmp(cmd,JOUEURS)==0){
			fprintf(stdout,"%s",tampon);
			fflush(stdout);
		}
		if(strcmp(cmd,MSGFROM)==0){
			strcpy(pseudo,args);
			fflush(stdout);
		}
		if(strcmp(cmd,MSG)==0){
			fprintf(stdout,"<%s> : %s",pseudo,args);
			fflush(stdout);
		}
		if(strcmp(cmd,CMD)==0){
			if(strcmp(args,"START")==0){
				launchThread(envoieTouches,(void *)server,sizeof(server_t));
			}
		}
	}else{
		int ecrit=0;
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

void communicationServeur(void *pack){
	server_t *server=pack;
	server_t serv=*server;
	char pseudo[MAX_PSEUDO];
	char tampon[MAX_TAMPON];
	char str[MAX_TAMPON];

	/* Envoi du pseudo */
	printf("Quel est votre pseudo ?\n");
	fgets(pseudo,MAX_TAMPON,stdin);
	fflush(stdin);
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
				statut=1; 
			}
			while(reste>0){
				if(fgets(tampon,MAX_LIGNE,server->fileSock)==NULL){
					statut=1;
				}
				if(fflush(server->fileSock)){
					perror("fflush");
					exit(EXIT_FAILURE);
				}
				nb=sscanf(tampon,"%s %[^\r]",cmd,args);
				if(nb>0){
					//Traitement des commandes d'entrée
					traitementCMD(tampon,cmd,args,&serv,1);
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
			//int ecrit=0;
			int nb=sscanf(tampon,"%s %[^\n]",cmd,args);
			if(nb>0){
				traitementCMD(tampon,cmd,args,&serv,0);
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
	int socket = udpInit(PORT,0,NULL,0);
	serv=pollEcoute(socket);
	/* === Informations sur le serveur choisi === */
	printf("Le port de la partie choisie est : %s\n",serv.portTCP);
	printf("Nom de l'hote : %s\n",serv.hostname);
	printf("Port UDP de l'envoi des touches : %d\n",atoi(serv.portTCP)+1);

	/* === Connexion TCP client : utilisation du cours === */
	serv.socketTCP=connexionServ(serv.hostname,serv.portTCP);
	serv.fileSock=fdopen(serv.socketTCP,"a+");
	launchThread(communicationServeur,&serv,sizeof(serv));
	/* Fin de la connexion */

	pause();

	fclose(serv.fileSock);
	close(serv.socketTCP);
	printf("\nFin du client ! \n");
	return 0;
}
