#include "common.h"
#include "libsockets.h"
#include "libthreads.h"

#define UDP_PORT          "1337"
#define MAX_LIGNE         1024
#define MAX_CONNEXIONS    256
#define MAX_ARGS          200
#define MAX_CMD           56
#define MAX_LIST          512

typedef struct {
  struct broadReturn br;
  balise_t b;
} beaconPack;

typedef struct {
  int id;
  int x;
  int z;
  int angle;
} playerPosition;

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

void controlsHandler() {
    /* écoute udp  */
    int s;
    s = udpEcoute(1331);

    struct sockaddr_in addrClient;
    socklen_t size = sizeof addrClient;
    char buffer[MAX_LIGNE];
    int nbBytes;
    while(1) {
      nbBytes = recvfrom(s, buffer, MAX_LIGNE-1, 0, (struct sockaddr *)&addrClient, &size);

      buffer[nbBytes] = '\0';
      printf("%s\n", buffer);
    }
}


int cmdHandler(char * cmd) {
    if (strcmp("START", cmd) == 0 && gameStarted == 0) {
      gameStarted = 1;
      printf("\033[0;36m(INFO) --- GAME IS STARTING\n\033[0m");
      sleep(1);
      printf("\033[0;36m(INFO) --- 3\n\033[0m");
      sleep(1);
      printf("\033[0;36m(INFO) --- 2\n\033[0m");
      sleep(1);
      printf("\033[0;36m(INFO) --- 1\n\033[0m");
      sleep(1);
      
      launchThread(controlsHandler, NULL, 0); 

      return 1;
    }

    if (strcmp("STOP", cmd) == 0) {

      // TODO
      return 1;
    }

    return 0;

}

void clientChat(void *pack) {
    balise_cotcp *b = pack;
    char args[MAX_ARGS];
    char ligne[MAX_LIGNE];
    char list[MAX_LIST];
    char buffer[MAX_LIGNE];
    int statut, cpt=0;


    
    /* Obtient une structure de fichier */
    FILE *dialogue=fdopen(b->s,"a+");
    if(dialogue==NULL){ perror("gestionClient.fdopen"); exit(EXIT_FAILURE); }
    
    /* Bonjour */
    printf("\033[0;32m(CONNECTION) client %d connected\033[0m\n", b->i);

    /* Traitement des commandes */
    while (fgets(ligne, MAX_LIGNE, dialogue) != NULL) {

      statut = sscanf(ligne, "CONNEXION %s", args);
      if (statut == 1 && listClients[b->i].connected == 0) {
        strcpy(listClients[b->i].pseudo, args);
        listClients[b->i].connected = 1;
        fprintf(dialogue, "CONNECTE %d\r\n", b->i);
        fflush(dialogue);
        for (int i = 0; i<=nbClients; i++) {
          if (listClients[i].connected) {
            strcat(list, listClients[i].pseudo);
            strcat(list, " ");
            cpt++;
          }
        }

        printf("\033[0;36m(INFO) players : (%d) %s\033[0m\n", cpt, list);
        
        sprintf(buffer, "JOUEURS (%d) %s\r\n", cpt, list);
        for(int i = 0; i<=nbClients; i++) {
          if (listClients[i].connected) {
            write(listClients[i].s, buffer, strlen(buffer));
          }
        }

        memset(list, 0, sizeof(list));
        cpt = 0;

      }

      statut = sscanf(ligne, "MSG %[^\n]", args);
      if (statut == 1) {
        printf("\033[0;35m(CHAT) %s: %s\033[0m\n", listClients[b->i].pseudo, args);
        sprintf(buffer, "MSGFROM %s\r\nMSG %s\n", listClients[b->i].pseudo, args);
        for(int i = 0; i<=nbClients; i++) {
          if (listClients[i].connected) {
            write(listClients[i].s, buffer, strlen(buffer));
          }
        }
      }
      
      statut = sscanf(ligne, "CMD %s", args);
      if (statut == 1) {
        if(!cmdHandler(args)) {
          printf("\033[0;31m(ERROR) client %d: unknown cmd: %s\033[0m\n", b->i, args);
        } 
      }

    }

    /* Fin dialogue */
    listClients[b->i].connected = 0;
    fclose(dialogue);
    printf("\033[0;31m(DISCONNECTION) client %d disconnected\033[0m\n", b->i);

}


int saveTCP(int s) {
    balise_cotcp b;
    struct sockaddr_in address;
    socklen_t len;
    if (nbClients < MAX_CONNEXIONS) {
      b.s = s;
      b.i = nbClients;
      b.connected = 0;
      
      len = sizeof(address);
      getpeername(s, (struct sockaddr*)&address, &len);
      printf("IP : %s\n", inet_ntoa(address.sin_addr));

      launchThread(clientChat, &b, sizeof(b));
      listClients[nbClients] = b;
      nbClients++;
      return 0;
    }
    printf("\033[0;31m(ERROR) Maximum number of connections has been reached\033[0m\n");
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
