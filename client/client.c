#include "libsockets.h"
#include "libthreads.h"

#define PORT 1337

void communicationServeur(void *pack){
	server_t *server=pack;
	int socket=connexionServ(*server);
	while(1){
		discussionTCP(socket);
	}
}

int main(){
	server_t serv;
	serv = udpEcoute(PORT);
	/* Test avec un serveur uniquement TCP */
	//strcpy(serv.portTCP,"1330"); 
	//inet_aton("172.26.145.69",&serv.addr_Server); 
	/* Fin du test */
	printf("Le port de la partie choisie est : %s\n",serv.portTCP);
	printf("Adresse de l'hote : %s\n",inet_ntoa(serv.addr_Server));

	/* === Connexion TCP client : utilisation du cours === */
	launchThread(communicationServeur,&serv,sizeof(serv));
	/* Fin de la connexion */

	pause();

	return 0;
}
