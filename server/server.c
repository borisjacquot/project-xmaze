/* A METTRE DANS LA LIB */
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

#define UDP_PORT "1337"

int keepRunning = 1;

struct addrinfo * udpSocket() {
    /* EXEMPLE MAN GETADDRINFO */
    struct addrinfo hints, *result;
    int s;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* IPv4 ou IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Non connecté */
    hints.ai_flags = AI_PASSIVE;    /* Wildcard IP */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, UDP_PORT, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    return result;
}

int bindUdp(struct addrinfo * s) {
    struct addrinfo *rp;
    int sfd;

    /* On boucle jusqu'à temps qu'on réussisse à bind */
    for (rp = s; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    return sfd;
}

struct sockaddr_in setBroadcast(int sfd) {
    int optval=1, optlen=sizeof(int);
    struct sockaddr_in broad;

    if(setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, (char *) &optval, optlen)){
        perror("Error setting socket to BROADCAST mode");
        exit(1);
    }

    broad.sin_family = AF_INET;
    broad.sin_port=htons((unsigned short) atoi(UDP_PORT));
    broad.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    bzero(&(broad.sin_zero), 8);

    return broad;
}

void sendBroadcast(int sfd, struct sockaddr_in broad, char * msg) {
    int numbytes;
    if ((numbytes=sendto(sfd, msg, strlen(msg), 0, (struct sockaddr *)&broad, sizeof(struct sockaddr))) == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes to %s\n",numbytes,inet_ntoa(broad.sin_addr));
}

void hand(int sig) {
    if (sig == SIGINT) {
        keepRunning = 0;
    }
}

int main(void) {

    /* --- BROADCAST UDP --- */
    struct addrinfo *result;
    struct sigaction action = {0};
    int sfd;
    struct sockaddr_in broad;
    char msg[10];
    msg[0] = 0x32;
    msg[1] = 0x05;
    strcpy(msg + 2, "xmazing");


    /* création du socket udp */
    result = udpSocket();
    /* binding */
    sfd = bindUdp(result);
    /* result devient useless*/
    freeaddrinfo(result);
    /* set le broadcast */
    broad = setBroadcast(sfd);

    action.sa_handler = hand;
    sigaction(SIGINT, &action, NULL);

    /* envoi du message */
    while(keepRunning) {
        sendBroadcast(sfd, broad, msg);
        sleep(1);
    }
    close(sfd);
    /* --- FIN BROADCAST UDP --- */
    printf("Belle journée\n");
    return 0;
}
