/**** Bibliotheque graphique ****/

/** Fichiers d'inclusion **/

#include "SDL2/SDL.h"
#include "SDL2/SDL2_gfxPrimitives.h"

#include "libgraph.h"

/** Types **/

typedef struct { int r,v,b; } couleur;

/** Constantes **/

#define BITS_PAR_PIXEL	32

/** Macros **/

#define GFX_COULEUR(r,v,b)	(((((r<<8)|v)<<8)|b)<<8|0xff)

/** Variables globales **/

static const couleur couleurs[]={
  {255,255,255},
  {0,0,0},
  {255,0,0},
  {0,255,0},
  {0,0,255},
  {255,105,180},
  {-1,-1,-1}
  };

static SDL_Window *fenetre;
static SDL_Renderer *support;

/** Fonctions **/

/* Initialisation de la fenetre */

static int fenetre_x=-1,fenetre_y=-1;
unsigned char creerFenetre(int largeur,int hauteur,char *titre){
if(SDL_Init(SDL_INIT_VIDEO)!=0) return 0;
fenetre=SDL_CreateWindow(
     titre,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
     largeur,hauteur,0
     );
if(fenetre==NULL) return 0;
support=SDL_CreateRenderer(fenetre,-1,SDL_RENDERER_ACCELERATED);
if(support==NULL) return 0;
fenetre_x=largeur;
fenetre_y=hauteur;
return 1;
}

/* Fermeture de la fenetre */

void fermerFenetre(void){
if(support!=NULL) SDL_DestroyRenderer(support);
if(fenetre!=NULL) SDL_DestroyWindow(fenetre);
SDL_Quit();
}

/* Effacer la fenêtre */

void effacerFenetre(void){
SDL_SetRenderDrawColor(support,0,0,0,255);
SDL_RenderClear(support);
}

/* Affiche les dessins effectues */

void synchroniserFenetre(void){
SDL_RenderPresent(support);
}

/* Dessin d'un polygone plein */

void polygonePlein(short int *x,short int *y,int n,int c_int,int c_ext){
couleur rgb=couleurs[c_int];
filledPolygonColor(support,x,y,n,GFX_COULEUR(rgb.r,rgb.v,rgb.b));
rgb=couleurs[c_ext];
polygonColor(support,x,y,n,GFX_COULEUR(rgb.r,rgb.v,rgb.b));
}

/* Dessin d'un rectangle plein */

void rectanglePlein(int x,int y,int l,int h,int c_int,int c_ext){
short int xp[4]={x,x+l,x+l,x};
short int yp[4]={y,y,y+h,y+h};

polygonePlein(xp,yp,4,c_int,c_ext);
}

/* Dessin d'un disque */

void disque(int x,int y,int r,int c_int,int c_ext){
couleur rgb=couleurs[c_int];
filledCircleColor(support,x,y,r,GFX_COULEUR(rgb.r,rgb.v,rgb.b));
rgb=couleurs[c_ext];
circleColor(support,x,y,r,GFX_COULEUR(rgb.r,rgb.v,rgb.b));
}

/* Manipulation de copie de surface en BMP */

unsigned char sauverSurface(char *fichier){
int statut=-1;
SDL_Surface* surface=SDL_CreateRGBSurface(0,fenetre_x,fenetre_y,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
if(surface){
  SDL_RenderReadPixels(support,NULL,SDL_PIXELFORMAT_ARGB8888,surface->pixels,surface->pitch);
  statut=SDL_SaveBMP(surface,fichier);
  SDL_FreeSurface(surface);
  }
return (statut<0)?0:1;
}

unsigned char chargerSurface(char *fichier){
SDL_Surface *image=SDL_LoadBMP(fichier);
if(image!=NULL){
  SDL_Texture *texture=SDL_CreateTextureFromSurface(support,image);
  SDL_RenderCopy(support,texture,NULL,NULL);
  SDL_RenderPresent(support);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(image);
  }
return (image!=NULL);
}

/* Attendre l'appui d'une touche */

unsigned char attendreEvenement(int *touche,unsigned char *fenetre,unsigned char *quitter){
SDL_Event event;
*fenetre=0;
*quitter=0;
if(SDL_PollEvent(&event)==0) return 0;
switch(event.type){
  case SDL_KEYDOWN:
    switch(event.key.keysym.sym){
      case SDLK_LEFT: *touche=TOUCHE_GAUCHE; break;
      case SDLK_RIGHT: *touche=TOUCHE_DROITE; break;
      case SDLK_UP: *touche=TOUCHE_HAUT; break;
      case SDLK_DOWN: *touche=TOUCHE_BAS; break;
      case SDLK_SPACE: *touche=TOUCHE_ESPACE; break;
      default: *touche=TOUCHE_AUTRE; break;
      }
    break;
  case SDL_KEYUP:
    *touche=0;
    break;
  case SDL_WINDOWEVENT:
    *fenetre=1;
    break;
  case SDL_QUIT:
    *quitter=1;
    break;
  }
return 1;
}
