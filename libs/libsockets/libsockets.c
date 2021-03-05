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
    //TODO voir client.c sur zabeth11 (2 getaddrinfo a faire)
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

/* === POLL SUR ENTREE STANDARD ET LA SOCKET POUR AFFICHAGE ET CHOIX DU SERVEUR === */
server_t pollEcoute(int s){
    struct pollfd descripteurs[2];
    descripteurs[0].fd=s;
    descripteurs[0].events=POLLIN;
    descripteurs[1].fd=0;
    descripteurs[1].events=POLLIN;
    struct sockaddr_in other_socket;
    int choix;
	char buffer[50];
	int keepRunning = 1;
	server_t tab[MAX_SERVER];
	int nbServ=0;
	printf("Entrez le numero du serveur souhaite a n'importe quel moment\n");
	while(keepRunning){
		int nb = poll(descripteurs,2,-1);
		if(nb<0){
			perror("udpEcoute.poll");
			exit(EXIT_FAILURE);
		}
		if((descripteurs[0].revents&POLLIN)!=0){
			memset(buffer,0,sizeof(buffer));
			unsigned sock_len=sizeof(struct sockaddr);
			recvfrom(s, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&other_socket, &sock_len);
			int flag=0;
			if(nbServ!=0){
				for(int i=0; i<=nbServ;i++){
					/* === si le serveur est deja trouve, on ajoute pas le server dans le tableau ===*/
					if(strcmp(buffer,tab[i].nom_brut)==0){
						flag=1;
					}
				}
			}
			if(flag==0){
				nbServ++;
				strcpy(tab[nbServ].nom_brut,buffer);
				int port = buffer[0] + (buffer[1]<<8);
				snprintf(tab[nbServ].portTCP,5,"%d",port);	
				for(long unsigned int i=0;i<sizeof(buffer)-1;i++){
					buffer[i]=buffer[i+2];
				}
				strcpy(tab[nbServ].nom_Server,buffer);
				//TODO Modifier
				//tab[nbServ].addr_Server=(struct sockaddr *)&other_socket;
				printf("%d) La partie de jeu %s est sur le port : %d\n",nbServ,buffer,port);
			}
		}
		if((descripteurs[1].revents&POLLIN)!=0){
			if(nbServ!=0){
				int taille=read(0,buffer,sizeof(buffer));
				if(taille<0) break;
				choix=atoi(buffer);
				while(choix>nbServ || choix<=0){
					printf("Veuillez entrer votre choix de serveur (entre 1 et %d) : ",nbServ);
					scanf("%d",&choix);
				}
				keepRunning=0;
			}
		}
	}
	return tab[choix];
}

//TODO utiliser getaddrinfo ??
/* Fonction d'ecoute du broadcast UDP sur le port 1337 */
server_t udpEcoute(int port){

	struct sockaddr_in mysocket;
	int s;
	s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int broadcast=1;

	setsockopt(s, SOL_SOCKET, SO_BROADCAST,
		    &broadcast, sizeof broadcast);

	memset(&mysocket, 0, sizeof(mysocket));
	mysocket.sin_family = AF_INET;
	mysocket.sin_port = htons(port);
	mysocket.sin_addr.s_addr = INADDR_ANY;
	bzero(mysocket.sin_zero,8);

	bind(s, (struct sockaddr *)&mysocket, sizeof(struct sockaddr));

	printf("Lecture du port %d ...\n",port);
	server_t infoServer=pollEcoute(s);
	close(s);

	return infoServer;
}


