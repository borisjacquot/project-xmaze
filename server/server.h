/* --- LIBS  --- */
#include "common.h"
#include "libsockets.h"
#include "libthreads.h"

/* --- INCLUDES ---  */
#include <math.h>

/* --- DEFINES ---  */
#define UDP_PORT          "1337"
#define SERVER_PORT	  "1330"
#define KEY_PORT	  1331
#define SERVER_NAME	  "Xmazing"

// cmd
#define START		  "START"
#define STOP		  "STOP"


#define MAX_LIGNE         1024
#define MAX_CONNEXIONS    256
#define MAX_ARGS          200
#define MAX_CMD           56
#define MAX_LIST          512
#define MAX_PSEUDO	  64

#define HAUT              4
#define BAS               8
#define GAUCHE            1
#define DROITE            2

/* --- STRUCTURES --- */
typedef struct {
  struct broadReturn br;
  balise_t b;
} beaconPack;

typedef struct {
  int x;
  int y;
  int z;
  int angle;
} playerPosition;

typedef struct {
    int s;
    int i;
    char pseudo[MAX_PSEUDO];
    FILE *fd;
    int connected;
    int admin;
} balise_cotcp;
