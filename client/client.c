#include "libsockets.h"
#include "libthreads.h"

#define PORT 1337

struct sigaction action;
static int arret=0;

void hand(int sig){
	if(sig==SIGINT){
		arret=1;
	}
}

void communicationServeur(void *pack){
	server_t *server=pack;
	int socket=connexionServ(*server);
	while(!arret){
		arret=discussionTCP(socket);
	}
	printf("Fin de la connexion TCP, appuyez sur CTRL+C pour arreter le client\n");
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
	launchThread(communicationServeur,&serv,sizeof(serv));
	/* Fin de la connexion */

	pause();
	printf("\nFin du client ! \n");
	return 0;
}
