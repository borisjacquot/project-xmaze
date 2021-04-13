#include "server.h"

int gameStarted = 0;
int nbClients = 0;

balise_cotcp listClients[MAX_CONNEXIONS];
playerPosition positions[MAX_CONNEXIONS];

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
    int s, id, key;
    s = udpInit(KEY_PORT);

    struct sockaddr_in addrClient;
    socklen_t size = sizeof addrClient;
    char buffer[MAX_LIGNE];
    while(1) {
      recvfrom(s, buffer, MAX_CMD, 0, (struct sockaddr *)&addrClient, &size);
      id = buffer[0];
      key = buffer[1];
      switch (key) {
        case HAUT:
          positions[id].x += 20*sin(2*M_PI*positions[id].angle/360);
          positions[id].z += 20*cos(2*M_PI*positions[id].angle/360);
          break;
        case BAS:
          positions[id].x -= 20*sin(2*M_PI*positions[id].angle/360);
          positions[id].z -= 20*cos(2*M_PI*positions[id].angle/360);
          break;
        case GAUCHE:
          positions[id].angle -= 5;
          break;
        case DROITE:
          positions[id].angle += 5;
          break;
      }
      printf("id: %d : %d %d %d ; %d\n", id, positions[id].x, positions[id].y, positions[id].z, positions[id].angle);
    }
}


int cmdHandler(char * cmd) {
    if (strcmp(START, cmd) == 0 && gameStarted == 0) {
      gameStarted = 1;
      printf("\033[0;36m(INFO) --- GAME IS STARTING\n\033[0m");
      sleep(1);
      printf("\033[0;36m(INFO) --- 3\n\033[0m");
      sleep(1);
      printf("\033[0;36m(INFO) --- 2\n\033[0m");
      sleep(1);
      printf("\033[0;36m(INFO) --- 1\n\033[0m");
      sleep(1);

      char start[MAX_CMD] = "CMD START\r\n";
      for(int i = 0; i<=nbClients; i++) {
          if (listClients[i].connected) {
            write(listClients[i].s, start, strlen(start));
          }
      }

      launchThread(controlsHandler, NULL, 0); 

      return 1;
    }

    if (strcmp(STOP, cmd) == 0) {

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

#define MAX_NOM   1024

int saveTCP(int s) {
    balise_cotcp b;
    playerPosition p;
    struct sockaddr_storage address;
    socklen_t len;
    if (nbClients < MAX_CONNEXIONS) {
      b.s = s;
      b.i = nbClients;
      b.connected = 0;
      
      len = sizeof(address);
      getpeername(s, (struct sockaddr*)&address, &len);
      char nom[MAX_NOM];
      getnameinfo((struct sockaddr*)&address, len, nom,MAX_NOM,NULL,0,0);
      printf("IP client : %s\n", nom);
      

      launchThread(clientChat, &b, sizeof(b));
      listClients[nbClients] = b;
      p.x = 0;
      p.y = 0;
      p.z = 0;
      p.angle = 0;
      positions[nbClients] = p;

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

    pack.b.port = atoi(SERVER_PORT);
    strcpy(pack.b.name, SERVER_NAME);

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
