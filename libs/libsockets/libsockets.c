#include "libsockets.h"

struct broadReturn setBroadcast(char * service) {
    /* EXEMPLE MAN GETADDRINFO */
    struct addrinfo hints, *result, *rp;
    struct sockaddr_storage broad;
    struct broadReturn r;
    int s, sfd;
    int optval=1, optlen=sizeof(int);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* IPv4 ou IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Non connecté */
    hints.ai_flags = AI_PASSIVE;    /* Wildcard IP */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, service, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* On boucle jusqu'à temps qu'on réussisse à bind */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
      if (rp->ai_family==AF_INET) break;
    }

    if (rp == NULL) rp = result;

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not find socket address\n");
        exit(EXIT_FAILURE);
    }

    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd < 0) { 
        fprintf(stderr, "Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != 0){
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    switch (rp->ai_family) {
        case AF_INET: {
            struct sockaddr_in *addr = (struct sockaddr_in *) rp->ai_addr;
            addr->sin_addr.s_addr = htonl(INADDR_BROADCAST);
            break;
                      }

        case AF_INET6: {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *) rp->ai_addr;
            inet_pton(AF_INET6, "ff02::1", &addr->sin6_addr);
            break;
                     }
    }

    broad=*(struct sockaddr_storage *)rp->ai_addr;

    freeaddrinfo(result); // result devient useless

    if(setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, (char *) &optval, optlen)){
        perror("Error setting socket to BROADCAST mode");
        exit(1);
    }


    r.sfd = sfd;
    r.broad = broad;

    return r;

}

void sendBroadcast(int sfd, struct sockaddr_storage broad, char * msg, int size) {
    int numbytes;
    if ((numbytes=sendto(sfd, msg, size, 0, (struct sockaddr *)&broad, sizeof(broad))) == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes\n",numbytes);
}
