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
#define TAILLE_PORT 5

/** STRUCTURES **/

typedef struct envTouche_s{
	uint8_t id;
	uint8_t touche;
}envTouche_t;

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
