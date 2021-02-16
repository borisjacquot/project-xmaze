/* A METTRE DANS LA LIB */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define UDP_PORT "1337"
#define BUF_SIZE 500

/* EXEMPLE MAN GETADDRINFO */

int main(int argc, char *argv[]) {

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    char buf[BUF_SIZE];
    struct sockaddr_in broad;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, UDP_PORT, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* On boucle jusqu'à temps qu'on réussisse à bind */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
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

    freeaddrinfo(result);           /* No longer needed */

    int optval=1, optlen=sizeof(int);
    if(setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, (char *) &optval, optlen)){
        perror("Error setting socket to BROADCAST mode");
        exit(1);
 	  }

    broad.sin_family = AF_INET;
    broad.sin_port=htons((unsigned short) atoi(UDP_PORT));
    broad.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    bzero(&(broad.sin_zero), 8);

    int numbytes;

    if ((numbytes=sendto(sfd, "je suis le plus heureux des hommes", strlen("je suis le plus heureux des hommes"), 0, \
            (struct sockaddr *)&broad, sizeof(struct sockaddr))) == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes to %s\n",numbytes,inet_ntoa(broad.sin_addr));

    close(sfd);

    return 0;
}
