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
	perror("socket");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, (char *) &optval, optlen)){
        perror("Error setting socket to BROADCAST mode");
        exit(1);
    }
    #ifdef SO_REUSEADDR
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, optlen)){
        perror("Error setting socket to REUSEADDR mode");
        exit(1);
    }
    #endif

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != 0){
	perror("bind");
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

    printf("\033[0;36m(INFO) sent %d bytes in broadcast mode\033[0m\n",numbytes);
}

/* Fonction de recuperation d'une adresse depuis le nom de l'hote */
int nomVersAdresse(char *hote,struct sockaddr_storage *padresse){
	struct addrinfo *resultat,*origine;
	int statut=getaddrinfo(hote,NULL,NULL,&origine);
	if(statut==EAI_NONAME) return -1;
	if(statut<0){ 
		perror("nomVersAdresse.getaddrinfo");
		exit(EXIT_FAILURE);
	}
	struct addrinfo *p;
	for(p=origine,resultat=origine;p!=NULL;p=p->ai_next){
		if(p->ai_family==AF_INET){
			resultat=p;
			break;
		}
	}
	memcpy(padresse,resultat->ai_addr,resultat->ai_addrlen);
	return 0;
}

/* Fonction d'ecoute du broadcast UDP sur le port 1337 */
int udpInit(int port,int hasAddr,char *hostname,int rcvPassant){

	struct sockaddr_in mysocket;
	int s;
	s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(s<0){
		perror("udpEcoute.socket");
		exit(EXIT_FAILURE);
	}
	int broadcast=1;

	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR,&broadcast, sizeof broadcast))!=0){
		perror("udpEcoute.setsockopt");
		exit(EXIT_FAILURE);
	}

	if(rcvPassant){
		struct timeval minuteur;
		minuteur.tv_sec=0;
		minuteur.tv_usec=100;
		if(setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&minuteur,sizeof minuteur)<0){
			perror("udpInit.setsockopt (RCVPASSANT)");
			exit(EXIT_FAILURE);
		}
	}

	memset(&mysocket, 0, sizeof(mysocket));
	mysocket.sin_family = AF_INET;
	mysocket.sin_port = htons(port);
	if(hasAddr){
		if(nomVersAdresse(hostname,(void *)&mysocket)<0){
			fprintf(stderr,"udpInit.nomVersAdresse : Erreur\n");
		}
	}else{
		mysocket.sin_addr.s_addr = INADDR_ANY;
	}
	bzero(mysocket.sin_zero,8);

	if((bind(s, (struct sockaddr *)&mysocket, sizeof(struct sockaddr)))!=0){
		perror("udpInit.bind");
		exit(EXIT_FAILURE);
	}
	return s;
}

/* Reception adresse serveur */
void receptionServer(int socket,char *buffer, char *name,int bufferSize,int nameSize){
	struct sockaddr_storage adresse;
	unsigned len=sizeof(struct sockaddr);
	recvfrom(socket, buffer, bufferSize-1,0,(struct sockaddr *)&adresse,&len);
	getpeername(socket,(struct sockaddr *)&adresse,&len);
	getnameinfo((struct sockaddr *)&adresse, len, name,nameSize,NULL,0,0);
}

void envoieTouche(int socket,int port, char *msg,int sizemsg,char *hostname){
	struct sockaddr_in adresse;
	adresse.sin_family=AF_INET;
	if(nomVersAdresse(hostname,(void *)&adresse)<0){
		fprintf(stderr,"envoieTouche.nomVersAdresse : Erreur\n");
	}
	adresse.sin_port=htons(port);
	if((sendto(socket,msg,sizemsg,0,(struct sockaddr *)&adresse,sizeof adresse))==-1){
		perror("envoieTouche.sendto");
	}
}

/* Reception Objets */
void receptionObjets(int socket,char *objets,int objSize,char *hostname,int port){
	struct sockaddr_in adresse;
	unsigned len=sizeof(struct sockaddr);
	adresse.sin_family=AF_INET;
	if(nomVersAdresse(hostname,(void *)&adresse)<0){
		fprintf(stderr,"envoieTouche.nomVersAdresse : Erreur\n");
	}
	adresse.sin_port=htons(port);
	if(recvfrom(socket,objets,objSize,0,(struct sockaddr *)&adresse,&len)<0){
		perror("receptionObjets.recvfrom");
	}
}


/* Initialisation de la connexion TCP avec le serveur */
int connexionServ(char *hostname,char *portTCP){
	struct addrinfo precisions,*resultat,*origine;
	int statut;
	int s;

	memset(&precisions,0,sizeof precisions);
	precisions.ai_family=AF_UNSPEC;
	precisions.ai_socktype=SOCK_STREAM;

	statut=getaddrinfo(hostname,portTCP,&precisions,&origine);
	if(statut<0){ perror("connexionServ.getaddrinfo"); exit(EXIT_FAILURE); }
	struct addrinfo *p;
	for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
		if(p->ai_family==AF_INET6){ resultat=p; break; }

	s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
	if(s<0){ perror("connexionServ.socket"); exit(EXIT_FAILURE); }

	if(connect(s,resultat->ai_addr,resultat->ai_addrlen)<0) return -1;

	freeaddrinfo(origine);

	return s;
}

int initialisationServeur(char *service,int connexions){
    struct addrinfo precisions,*resultat,*origine;
    int statut;
    int s;

    /* Construction de la structure adresse */
    memset(&precisions,0,sizeof precisions);
    precisions.ai_family=AF_UNSPEC;
    precisions.ai_socktype=SOCK_STREAM;
    precisions.ai_flags=AI_PASSIVE;

    statut=getaddrinfo(NULL,service,&precisions,&origine);
    if(statut<0){
      perror("initialisationServeur.getaddrinfo");
      exit(EXIT_FAILURE);
    }

    struct addrinfo *p;
    for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
      if(p->ai_family==AF_INET6){ resultat=p; break; }

    /* Creation d'une socket */
    s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
    if(s<0){ perror("initialisationServeur.socket"); exit(EXIT_FAILURE); }

    /* Options utiles */
    int vrai=1;
    if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0){
      perror("initialisationServeur.setsockopt (REUSEADDR)");
      exit(EXIT_FAILURE);
    }
    if(setsockopt(s,IPPROTO_TCP,TCP_NODELAY,&vrai,sizeof(vrai))<0){
      perror("initialisationServeur.setsockopt (NODELAY)");
      exit(EXIT_FAILURE);
    }

    /* Specification de l'adresse de la socket */
    statut=bind(s,resultat->ai_addr,resultat->ai_addrlen);
    if(statut<0) return -1; 

    /* Liberation de la structure d'informations */
    freeaddrinfo(origine);

    /* Taille de la queue d'attente */
    statut=listen(s,connexions);
    if(statut<0) return -1;

    return s;
}

void boucleServeur(void *pack) {
    int dialogue;
    balise_trait *b = pack;
    while(1){
      
      /* Attente d'une connexion */
      if((dialogue=accept(b->s,NULL,NULL))<0) exit(-1);

      /* Passage de la socket de dialogue a la fonction de traitement */
      if(b->fonction(dialogue)<0){ shutdown(b->s,SHUT_RDWR); exit(0);}

    }
}

int initSocketUDP (char * service) {
    struct addrinfo precisions, *resultat, *origine;
    int s, statut;

    memset(&precisions,0,sizeof precisions);
    precisions.ai_family=AF_UNSPEC;
    precisions.ai_socktype=SOCK_DGRAM;
    precisions.ai_flags=AI_PASSIVE;

    statut=getaddrinfo(NULL,service,&precisions,&origine);
    if(statut<0){ perror("initialisationSocketUDP.getaddrinfo"); exit(EXIT_FAILURE); }
    
    struct addrinfo *p;

    for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
      if(p->ai_family==AF_INET){ resultat=p; break; }

    /* Creation d'une socket */
    s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
    if(s<0){ perror("initialisationSocketUDP.socket"); exit(EXIT_FAILURE); }

    /* Options utiles */
    int vrai=1;
    if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0){
      perror("initialisationServeurUDPgenerique.setsockopt (REUSEADDR)");
      exit(-1);
    }

    /* Specification de l'adresse de la socket */
    statut=bind(s,resultat->ai_addr,resultat->ai_addrlen);
    if(statut<0) {perror("initialisationServeurUDP.bind"); exit(-1);}

    /* Liberation de la structure d'informations */
    freeaddrinfo(origine);

    return s;

}
