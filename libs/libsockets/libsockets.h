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

/* DEFINE  */

/* STRUCTURES  */
struct broadReturn {
    int sfd;
    struct sockaddr_storage broad;
};

/* PROTOTYPES  */
struct broadReturn setBroadcast(char *); //config les sockets piur broadcast udp
void sendBroadcast(int, struct sockaddr_storage, char *, int); //envoie message broadcast


#endif // LIBSOCKETS_H
