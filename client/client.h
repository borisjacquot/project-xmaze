#ifndef CLIENT_H
#define CLIENT_H

/** INCLUDES **/

#include "libsockets.h"
#include "libthreads.h"
#include "libgraph.h"
#include "common.h"

/** DEFINES **/

#define PORT 1337

#define MAX_LIGNE 1024
#define MAX_PSEUDO 64
#define MAX_SERVER 50
#define MAX_TAMPON 512
#define MAX_NAME_SERVER 12
#define MAX_HOSTNAME 64
#define TAILLE_TOUCHES 2
#define TAILLE_PORT 5

#define CONNECTE "CONNECTE"
#define JOUEURS "JOUEURS"
#define MSGFROM "MSGFROM"
#define MSG "MSG"
#define CMD "CMD"
#define START "/start"
#define STOP "/stop"

#define TITRE	"Xmazing"
#define LARGEUR 640
#define HAUTEUR 480

#define POINTS_MAX 32

#define TYPE_MUR 0

#define sign(a) (((a)==0)?0:(((a)>0)?1:-1))

/** STRUCTURES **/

typedef struct{
	int x,y;
}point2D;

typedef struct{
	int type;
	union{
		point2D p[4];
	}def;
}objet2D;

typedef struct server_s{
	char nom_brut[MAX_NAME_SERVER];
	char nom_Server[MAX_NAME_SERVER];
	char portTCP[TAILLE_PORT];
	int socketTCP;
	char id;
	FILE *fileSock;
	char hostname[MAX_HOSTNAME];
}server_t;

#endif //CLIENT_H
