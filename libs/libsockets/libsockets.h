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

#define MAX_NAME 12
#define MAX_BUFFER  1024

/* STRUCTURES  */
struct broadReturn {
    int sfd;
    struct sockaddr_storage broad;
};

typedef struct {
    int s;
    int (*fonction) (int);
} balise_trait;

/* PROTOTYPES  */
struct broadReturn setBroadcast(char *); //config les sockets piur broadcast udp
void sendBroadcast(int, struct sockaddr_storage, char *, int); //envoie message broadcast
int udpInit(int,int,char *,int); // init socket udp
void receptionServer(int,char *,char *,int,int); // reception des serveurs broadcast
int connexionServ(char *,char *); // initialisation de la connexion TCP avec le serveur
int compareAdresse(char *); // compare adresse avec l'adresse locale
int nomVersAdresse(char *,struct sockaddr_storage *); // transformation nom d'hote vers une adresse
void udpEnvoi(int,int,char *,int,char *);//envoie message udp
int initialisationServeur(char *,int);// initialisation serveur tcp
void boucleServeur(void *);
int checkAddress(int);
int initSocketUDP(char *);
int udpRecep(int, char *, int);// reception UDP
void sendFromSock(int, int, void*, int, int);


#endif // LIBSOCKETS_H
