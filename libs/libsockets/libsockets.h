#ifndef LIBSOCKETS_H
#define CLIENT_H

/* INCLUDES */
#include <stdint.h>
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
#include <sys/ioctl.h>

/* DEFINE  */
#define MAX_SERVER      50 // maximum de serveur possible de stocker dans le serveur
#define MAX_TAMPON      800 //taille maximum des messages pour la discussion avec le serveur en TCP
/* STRUCTURES  */
struct broadReturn {
    int sfd;
    struct sockaddr_storage broad;
};

typedef struct {
    int s;
    int (*fonction) (int);
} balise_trait;

// TODO à virer
typedef struct server_s{
	char nom_brut[10];
	char nom_Server[10];
	char portTCP[5];
	int socketTCP;
	char id;
	FILE *fileSock;
	char addr_Server[50];
}server_t;

/* PROTOTYPES  */
struct broadReturn setBroadcast(char *); //config les sockets piur broadcast udp
void sendBroadcast(int, struct sockaddr_storage, char *, int); //envoie message broadcast
int udpEcoute(int); //configuration de l'ecoute du broadcast udp
struct sockaddr_in createAddr(int,char *);
void receptionServer(int,char *,char *,int,int);
int connexionServ(server_t); // initialisation de la connexion TCP avec le serveur
int initialisationServeur(char *,int);
void boucleServeur(void *);

#endif // LIBSOCKETS_H
