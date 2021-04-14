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

//laby
#define TYPE_MUR	  0
#define TYPE_SPHERE	  1

#define LABY_X		  8
#define LABY_Y		  8
#define MUR_TAILLE	  200
#define MUR_HAUTEUR	  200

#define FOCALE		  (3*MUR_TAILLE)

/* --- MACROS --- */
#define max(a,b)	  (((a)<b))?(a):(b)
#define min(a,b)	  (((a)>b))?(a):(b)
#define sign(a)		  (((a)==0)?0:(((a)>0)?1:-1))

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

typedef struct {
    int x,y,z;
} point;

typedef struct {
    int x,y;
} point2D;

typedef struct {
    point p[4];
} mur;

typedef struct {
    point o;
    int r;
} sphere;

typedef struct {
    int type;
    union {
	point2D p[4];
	point2D o;
	int r;
    } def;
} objet2D;

