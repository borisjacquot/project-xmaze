#include "common.h"
#include "libsockets.h"
#include "libthreads.h"

#define UDP_PORT          "1337"
#define MAX_LIGNE         512
#define MAX_CONNEXIONS    256
#define MAX_ARGS          200
#define MAX_CMD           56

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

void clientChat(void *pack) {
    balise_cotcp *b = pack;
    char cmd[MAX_CMD], args[MAX_ARGS];
    
    /* Obtient une structure de fichier */
    FILE *dialogue=fdopen(b->s,"a+");
    if(dialogue==NULL){ perror("gestionClient.fdopen"); exit(EXIT_FAILURE); }
    
    /* Bonjour */
   // char ligne[MAX_LIGNE];
    fprintf(dialogue, "\033[0;32mBienvenue joueur %d!\033[0m\n", b->i);
    fflush(dialogue); //vider le buffer pour envoyer
    printf("\033[0;32mClient %d connected\033[0m\n", b->i);

    while (fscanf(dialogue, "%s %s", cmd, args) > 0) {
      printf("\033[0;35mCMD: \033[0m%s\n\033[0;35mARGS: \033[0m%s\n", cmd, args);

      if (strcmp(cmd, "CONNEXION") == 0) {
        fprintf(dialogue, "Ton pseudo est %s\n", args);
      }
    }

    /* Fin dialogue */
    // fclose(dialogue);

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
    if (s < 0) {
      perror("Erreur initialisation serveur");
      exit(-1);
    }
    b.s = s;
    b.fonction=saveTCP;


    launchThread(boucleServeur, &b, sizeof(b));
    /* --- FIN CONNEXION TCP  ---  */

    pause();

    printf("\nBelle journée\n");
    return 0;
}
