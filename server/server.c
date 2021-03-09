#include <stdint.h>
#include "common.h"
#include "libsockets.h"
#include "libthreads.h"

#define UDP_PORT "1337"
#define MAX_LIGNE 512
#define MAX_CONNEXIONS 256

typedef struct {
  struct broadReturn br;
  balise_t b;
} beaconPack;

int gameStarted = 0;

/*
void hand(int sig) {
    if (sig == SIGINT) {
        keepRunning = 0;
    }
}
*/

void beacon(void *pack){
    beaconPack *p=pack;
    struct broadReturn *br=&p->br;
    balise_t *b=&p->b;
    while(!gameStarted) {
        sendBroadcast(br->sfd, br->broad, (void *)b, sizeof(balise_t));
        sleep(5);
    }
    close(br->sfd);
}

bool startsWith(const char * src, const char * base) {
    return strncmp(src, base, strlen(base)) == 0;
}

void clientChat(void *pack) {
    balise_cotcp *b = pack;
    printf("Client %d connecté\n", b->i);
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

void boucleServeur(void * arg) {
    int dialogue, i=0;
    int s = *(int*) arg;
    balise_cotcp b;
    while(1){
      
      /* Attente d'une connexion */
      
      if (i < MAX_CONNEXIONS) {
        if((dialogue=accept(s,NULL,NULL))<0) exit(-1);
        b.s = dialogue;
        b.i = i;
        launchThread(clientChat, &b, sizeof(b));
        i++;
      }

    }
}

int gestionClient(int s){

    /* Obtient une structure de fichier */
    FILE *dialogue=fdopen(s,"a+");
    if(dialogue==NULL){ perror("gestionClient.fdopen"); exit(EXIT_FAILURE); }

    /* Echo */
    char ligne[MAX_LIGNE];
    while(startsWith(fgets(ligne,MAX_LIGNE,dialogue), "CONNEXION") == 0) {
      printf("\033[0;31merror : %s\033[0m\n", ligne);  
      fprintf(dialogue,"> ERREUR\n");
    }

    char *ptok = strtok(ligne, " ");
    ptok = strtok(NULL, " ");
    fprintf(dialogue, "> Bienvenue %s\n", ptok);

    /* Termine la connexion */
    fclose(dialogue);
    return 0;
}

int main(void) {
    /* --- BROADCAST UDP --- */
//    struct sigaction action = {0};
    
    /*
    action.sa_handler = hand;
    sigaction(SIGINT, &action, NULL);
    */

    beaconPack pack;

    pack.b.port = atoi("1330");
    strcpy(pack.b.name, "Xmazing");

    pack.br = setBroadcast(UDP_PORT);
    launchThread(beacon,&pack,sizeof(pack));
    /* --- FIN BROADCAST UDP --- */


    /* --- CONNEXION TCP  --- */
    int s;

    /* Initialisation du serveur */
    s=initialisationServeur("1330",MAX_CONNEXIONS);

    launchThread(boucleServeur, &s, sizeof s);
    /* --- FIN CONNEXION TCP  ---  */

    pause();

    printf("\nBelle journée\n");
    return 0;
}
