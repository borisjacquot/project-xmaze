#include "libsockets.h"

#define PORT 1337
#define MAX_TAMPON 50

int connexionServ(server_t Serveur){
	struct addrinfo precisions,*resultat,*origine;
	int statut;
	int s;

	memset(&precisions,0,sizeof precisions);
	precisions.ai_family=AF_UNSPEC;
	precisions.ai_socktype=SOCK_STREAM;
	// ajouter l'adresse, obtenue dans Serveur.addr_Serv->sa_data
	statut=getaddrinfo(inet_ntoa(Serveur.addr_Server),Serveur.portTCP,&precisions,&origine);
	if(statut<0){ perror("connexionServ.getaddrinfo"); exit(EXIT_FAILURE); }
	struct addrinfo *p;
	for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
		if(p->ai_family==AF_INET6){ resultat=p; break; }

	s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
	if(s<0){ perror("connexionServ.socket"); exit(EXIT_FAILURE); }

	if(connect(s,resultat->ai_addr,resultat->ai_addrlen)<0) return -1;

	freeaddrinfo(origine);

	return s;
}

int main(){
	server_t serv;
	//server_t serv = udpEcoute(PORT);
	strcpy(serv.portTCP,"1330");
	inet_aton("172.26.145.69",&serv.addr_Server);
	printf("Le port de la partie choisie est : %s\n",serv.portTCP);
	/* === Connexion TCP client : utilisation du cours === */
	int socket;
	socket=connexionServ(serv);
	if(socket<0){ fprintf(stderr,"Erreur de connexion au serveur\n");
		exit(EXIT_FAILURE);
	}

	struct pollfd descripteurs[2];
	descripteurs[0].fd=socket;
	descripteurs[0].events=POLLIN;
	descripteurs[1].fd=0;
	descripteurs[1].events=POLLIN;
	while(1){
		char tampon[MAX_TAMPON];
		int nb=poll(descripteurs,2,-1);
		if(nb<0){ perror("main.poll"); exit(EXIT_FAILURE); }
		if((descripteurs[0].revents&POLLIN)!=0){
			int taille=read(socket,tampon,MAX_TAMPON);
			if(taille<=0) break;
			write(1,tampon,taille);
		}
		if((descripteurs[1].revents&POLLIN)!=0){
			int taille=read(0,tampon,MAX_TAMPON);
			if(taille<=0) break;
			if(fflush(0)){
				fprintf(stderr,"Impossible de flush le stream\n");
			}
			write(socket,tampon,taille);
		}
	}

	shutdown(socket,SHUT_RDWR);

	return 0;
}
