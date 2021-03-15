#include "common.h"
#include "libsockets.h"
#include "libthreads.h"

#define UDP_PORT          "1337"
#define MAX_LIGNE         512
#define MAX_CONNEXIONS    256

typedef struct {
  struct broadReturn br;
  balise_t b;
} beaconPack;

int gameStarted = 0;
int nbClients = 0;

balise_cotcp listClients[MAX_CONNEXIONS];

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

int saveTCP(int s) {
    balise_cotcp b;
    if (nbClients < MAX_CONNEXIONS) {
      b.s = s;
      b.i = nbClients;
      launchThread(clientChat, &b, sizeof(b));
      listClients[nbClients] = b;
      nbClients++;
      return 0;
    }
    printf("\033[0;31mError: Maximum number of connections has been reached\033[0m\n");
    return -1;
}

void clientChat(void *pack) {
    balise_cotcp *b = pack;
    printf("Client %d connecté\n", b->i);
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
    balise_trait b;

    /* Initialisation du serveur */
    s=initialisationServeur("1330",MAX_CONNEXIONS);
    b.s = s;
    b.fonction=saveTCP;


    launchThread(boucleServeur, &b, sizeof(b));
    /* --- FIN CONNEXION TCP  ---  */

    pause();

    printf("\nBelle journée\n");
    return 0;
}
