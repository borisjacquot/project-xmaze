#ifndef LIBSOCKETS_H
#define CLIENT_H

/* INCLUDES */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <netinet/tcp.h>
#include <stdbool.h>

/* DEFINE  */
#define MAX_SERVER 50 //maximum de serveur possible de stocker dans le serveur
#define MAX_TAMPON 200 //taille maximum des messages pour la discussion avec le serveur en TCP

/* STRUCTURES  */
struct broadReturn {
    int sfd;
    struct sockaddr_storage broad;
};

typedef struct server_s{
	char nom_brut[10];
	char nom_Server[10];
	char portTCP[5];
	struct in_addr addr_Server;
}server_t;

/* PROTOTYPES  */
struct broadReturn setBroadcast(char *); //config les sockets piur broadcast udp
void sendBroadcast(int, struct sockaddr_storage, char *, int); //envoie message broadcast
server_t pollEcoute(int); //ecoute le port et l'entree standard pour choisir le serveur de jeu avec poll
server_t udpEcoute(); //configuration de l'ecoute du broadcast udp
int connexionServ(server_t); // initialisation de la connexion TCP avec le serveur
int discussionTCP(int); //envoie des commandes avec le serveur et reception de ses réponses
#endif // LIBSOCKETS_H
