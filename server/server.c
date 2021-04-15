#include "server.h"

/* --- GLOBAL VARIABLES --- */
mur murs[(LABY_X+1)*LABY_Y+(LABY_Y+1)*LABY_X];
sphere spheres[MAX_SPHERES];
objet2D objets[(LABY_X+1)*LABY_Y+(LABY_Y+1)*LABY_X+MAX_SPHERES];

int nb;

char *laby[2*LABY_Y+1]={
    " - - - - - - - - ",
    "|. . . . . . . .|",
    "                 ",
    "|. . . . . . . .|",
    "                 ",
    "|. . . . . . . .|",
    "                 ",
    "|. . . . . . . .|",
    "                 ",
    "|. . . . . . . .|",
    "                 ",
    "|. . . . . . . .|",
    "                 ",
    "|. . . . . . . .|",
    "                 ",
    "|. . . . . . . .|",
    " - - - - - - - - "
};

int gameStarted = 0;
int nbClients = 0;

balise_cotcp listClients[MAX_CONNEXIONS];
playerPosition positions[MAX_CONNEXIONS];

/* --- FUNCTIONS --- */

/*
void hand(int sig) {
    if (sig == SIGINT) {
        keepRunning = 0;
    }
}
*/

point soustraire_points(point p1, point p2) {
    point p;
    p.x=p1.x-p2.x;
    p.y=p1.y-p2.y;
    p.z=p1.z-p2.z;
    return p;
}

mur *duplique_murs(mur *murs, int nb) {
    mur *copie=malloc(nb*sizeof(mur));
    memcpy(copie, murs, nb*sizeof(mur));
    return copie;
}

sphere *duplique_spheres (sphere *spheres, int nb) {
    sphere *copie=malloc(nb*sizeof(sphere));
    memcpy(copie, spheres, nb*sizeof(sphere));
    return copie;
}

int compare_murs(const void *arg1, const void *arg2) {
    const mur *mur1=arg1;
    const mur *mur2=arg2;
    int cx1=(mur1->p[0].x+mur1->p[3].x)/2;
    int cz1=(mur1->p[0].z+mur1->p[3].z)/2;
    int cx2=(mur2->p[0].x+mur2->p[3].x)/2;
    int cz2=(mur2->p[0].z+mur2->p[3].z)/2;
    int d1=abs(cx1)+abs(cz1);
    int d2=abs(cx2)+abs(cz2);
    return d2-d1;
}


int compare_spheres(const void *arg1, const void *arg2) {
    const sphere *sph1=arg1;
    const sphere *sph2=arg2;
    int d1=abs(sph1->o.x)+abs(sph1->o.z);
    int d2=abs(sph2->o.x)+abs(sph2->o.z);
    return d2-d1;
}

void tri_murs(mur *murs, int n) {
    qsort(murs, n, sizeof(mur), compare_murs);
}

void tri_spheres(sphere *spheres, int n) {
    qsort(spheres, n, sizeof(sphere), compare_spheres);
}


int dessin_vers_murs(char *laby[],mur *murs){
    int nb=0;
    int i;
    for(i=0;i<9;i++){
      int j;
      for(j=0;j<9;j++){
        if(laby[2*i][2*j+1]=='-'){
          murs[nb].p[0].x=j*MUR_TAILLE;
          murs[nb].p[0].z=i*MUR_TAILLE;
          murs[nb].p[0].y=0;
          murs[nb].p[1].x=j*MUR_TAILLE;
          murs[nb].p[1].z=i*MUR_TAILLE;
          murs[nb].p[1].y=MUR_HAUTEUR;
          murs[nb].p[2].x=(j+1)*MUR_TAILLE;
          murs[nb].p[2].z=i*MUR_TAILLE;
          murs[nb].p[2].y=MUR_HAUTEUR;
          murs[nb].p[3].x=(j+1)*MUR_TAILLE;
          murs[nb].p[3].z=i*MUR_TAILLE;
          murs[nb++].p[3].y=0;
        }

        if(i<8 && laby[2*i+1][2*j]=='|'){
          murs[nb].p[0].x=j*MUR_TAILLE;
          murs[nb].p[0].z=i*MUR_TAILLE;
          murs[nb].p[0].y=0;
          murs[nb].p[1].x=j*MUR_TAILLE;
          murs[nb].p[1].z=i*MUR_TAILLE;
          murs[nb].p[1].y=MUR_HAUTEUR;
          murs[nb].p[2].x=j*MUR_TAILLE;
          murs[nb].p[2].z=(i+1)*MUR_TAILLE;
          murs[nb].p[2].y=MUR_HAUTEUR;
          murs[nb].p[3].x=j*MUR_TAILLE;
          murs[nb].p[3].z=(i+1)*MUR_TAILLE;
          murs[nb++].p[3].y=0;
        }
      }
    }
    return nb;
}

void decale_murs(mur *murs, int nb, point position) {
    int i, j;
    for (i=0; i<nb; i++)
	    for (j=0; j<4; j++)
	      murs[i].p[j]=soustraire_points(murs[i].p[j], position);
}

void decale_spheres(sphere *spheres, int nb, point position) {
    int i;
    for (i=0; i<nb; i++)
	    spheres[i].o=soustraire_points(spheres[i].o, position);
}

void rotation_murs(mur *murs,int nb,int angle){
    int i,j;
    float radians=-2*M_PI*angle/360;
    for(i=0;i<nb;i++){
      for(j=0;j<4;j++){
        int x=murs[i].p[j].x*cos(radians)+murs[i].p[j].z*sin(radians);
        int y=murs[i].p[j].y;
        int z=-murs[i].p[j].x*sin(radians)+murs[i].p[j].z*cos(radians);
        murs[i].p[j].x=x;
        murs[i].p[j].y=y;
        murs[i].p[j].z=z;
      }
    }
}

void rotation_sphere(sphere *spheres,int nb,int angle){
    int i;
    float radians=-2*M_PI*angle/360;
    for(i=0;i<nb;i++){
        int x=spheres[i].o.x*cos(radians)+spheres[i].o.z*sin(radians);
        int y=spheres[i].o.y;
        int z=-spheres[i].o.x*sin(radians)+spheres[i].o.z*cos(radians);
        spheres[i].o.x=x;
        spheres[i].o.y=y;
        spheres[i].o.z=z;
    }
}

int positionOK(int x, int z, mur *murs, int nb) {
    int max = LABY_X*MUR_TAILLE;
    int min = 0;
    int xab, xac, zab, zac;
    int result=1;
    if ((x < max) && (x > min) && (z < max) && (z > min)) {
      for (int i=0; i<nb; i++) {
        xab=murs[i].p[3].x-murs[i].p[0].x;
        zab=murs[i].p[3].z-murs[i].p[0].z;
        xac=x-murs[i].p[0].x;
        zac=z-murs[i].p[0].z;
        // printf(":%d\n", xab*zac-xac*zab);
        if (xab*zac-xac*zab == 0) result=0; // pas colinéaires donc pas de colision
      }
    } else {
      result = 0;
    }
    return result;
}

void projete(mur *murs,int nb,objet2D *objets,int *no){
  int i,j;
  *no=0;
  for(i=0;i<nb;i++){
    int x1=murs[i].p[0].x; int z1=murs[i].p[0].z;
    int x2=murs[i].p[3].x; int z2=murs[i].p[3].z;
    if(z1<=0 && z2<=0) continue;
    if(z1<=0){
      murs[i].p[0].x=x2+(x1-x2)*(1-z2)/(z1-z2); murs[i].p[0].z=1;
      murs[i].p[1].x=murs[i].p[0].x; murs[i].p[1].z=murs[i].p[0].z;
    }
    if(z2<=0){
      murs[i].p[3].x=x1+(x2-x1)*(1-z1)/(z2-z1); murs[i].p[3].z=1;
      murs[i].p[2].x=murs[i].p[3].x; murs[i].p[2].z=murs[i].p[3].z;
    }
    objets[*no].type=TYPE_MUR;
    for(j=0;j<4;j++){
      int z=murs[i].p[j].z;
      int x=murs[i].p[j].x;
      int y=murs[i].p[j].y;
      int px,py;
      px=LARGEUR/2+x*FOCALE/z;
      py=HAUTEUR/4+(y-HAUTEUR/4)*FOCALE/z;
      objets[*no].def.p[j].x=px;
      objets[*no].def.p[j].y=py;
    }
    (*no)++;
  }
}


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
    s = udpInit(KEY_PORT,0,NULL);
    point p;
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
      //printf("id: %d : %d %d %d ; %d\n", id, positions[id].x, positions[id].y, positions[id].z, positions[id].angle);
      
      for (int i=0; i<nbClients; i++) {
        p.x = positions[i].x;
        p.y = positions[i].y;
        p.z = positions[i].z;
        mur *m2=duplique_murs(murs, nb);
        decale_murs(m2, nb, p);
        rotation_murs(m2, nb, positions[i].angle);
        tri_murs(m2, nb);
        objet2D *objets=malloc(nb*sizeof(objet2D));
        int no;
        projete(m2, nb, objets, &no);
        //printf("%d %d\n", objets[0].def.p[0].x, objets[0].def.p[0].y);
        free(objets);
      }

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


//TODO
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

      // init position aléatoire
      p.x=rand()%2001;
      p.z=rand()%2001;
      while (!positionOK(p.x, p.z, murs, nb)) {
        p.x=rand()%2001;
        p.z=rand()%2001;
      }
      p.y = 0;
      p.angle = 0;
      positions[nbClients] = p;

      nbClients++;
      return 0;
    }
    printf("\033[0;31m(ERROR) Maximum number of connections has been reached\033[0m\n");
    return -1;
}

int main(void) {

    srand(time(NULL)); // init random

    /* --- INIT LABY --- */
    nb = dessin_vers_murs(laby, murs);


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
