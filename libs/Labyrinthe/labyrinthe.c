/**** Programme de creation de labyrinthe ****/

/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "libgraph.h"

/** Constantes **/

#define TITRE		"Test labyrinthe"
#define LARGEUR		640
#define HAUTEUR		480

#define LABY_X		8
#define LABY_Y		8
#define	MUR_TAILLE	200
#define	MUR_HAUTEUR	200

#define FOCALE		(3*MUR_TAILLE)

#define ATTENTE		10000

#define POINTS_MAX	32

#define	TYPE_MUR	0

/** Macros **/

#define max(a,b)	(((a)<(b))?(a):(b))
#define min(a,b)	(((a)>(b))?(b):(a))
#define sign(a)		(((a)==0)?0:(((a)>0)?1:-1))

/** Structures **/

typedef struct{
  int x,y,z;
  } point;

typedef struct{
  int x,y;
  } point2D;

typedef struct{
  point p[4];
  } mur;

typedef struct{
  int type;
  union {
    point2D p[4];
    } def;
  } objet2D;

mur murs[(LABY_X+1)*LABY_Y+(LABY_Y+1)*LABY_X];

/** Variables globales **/

char *laby[2*LABY_Y+1]={
  " - - - - - - - - ",
  "|. . . . . . . .|",
  "                 ",
  "|. .|. . . . . .|",
  "       - -       ",
  "|. .|.|. .|. . .|",
  "                 ",
  "|. .|.|. .|. . .|",
  "                 ",
  "|. .|.|. .|. . .|",
  "       - -       ",
  "|. .|.|. . . . .|",
  "                 ",
  "|. .|.|. . . . .|",
  "                 ",
  "|. . . . . . . .|",
  " - - - - - - - - "
  };

/** Fonctions **/

/* Soustraction de deux points */

point soustraire_points(point p1,point p2){
point p;
p.x=p1.x-p2.x;
p.y=p1.y-p2.y;
p.z=p1.z-p2.z;
return p;
}

/* Copie un labyrinthe */

mur *duplique_murs(mur *murs,int nb){
mur *copie=malloc(nb*sizeof(mur));
memcpy(copie,murs,nb*sizeof(mur));
return copie;
}

/* Compare la distance entre deux murs */

int compare_murs(const void *arg1,const void *arg2){
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

/* Tri des murs */

void tri_murs(mur *murs,int n){
qsort(murs,n,sizeof(mur),compare_murs);
}

/* Calcul des murs en fonction du dessin */

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

/* Décalage d'un labyrinthe */

void decale_murs(mur *murs,int nb,point position){
int i,j;
for(i=0;i<nb;i++) 
  for(j=0;j<4;j++) 
    murs[i].p[j]=soustraire_points(murs[i].p[j],position);
}

/* Rotation d'un labyrinthe */

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

/* Projection en 2D */

void projete_murs(mur *murs,int nb,objet2D *objets,int *no){
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
#ifdef DEBUG
    printf("[%d,%d,%d](%d,%d) ",x,y,z,px,py);
#endif
    }
  (*no)++;
#ifdef DEBUG
  printf("\n");
#endif
  }
}

/* Intersection d'un segment */

unsigned char inter_seg_v(point2D a,point2D b,int x,int *y){
if(sign(a.x-x)==sign(b.x-x)) return 0;
*y=a.y+(b.y-a.y)*(x-a.x)/(b.x-a.x);
return 1;
}

unsigned char inter_seg_h(point2D a,point2D b,int *x,int y){
if(sign(a.y-y)==sign(b.y-y)) return 0;
*x=a.x+(b.x-a.x)*(y-a.y)/(b.y-a.y);
return 1;
}

/* Intersection d'un polygone avec un rectangle */

void inter_poly_rect(point2D *orig,int no,point2D *result,int *nr){
point2D avant[POINTS_MAX];
point2D apres[POINTS_MAX];
int i,j;
for(i=0;i<no;i++) avant[i]=orig[i];
#ifdef DEBUG
printf("??");
for(i=0;i<no;i++) printf("(%d,%d) ",avant[i].x,avant[i].y);
printf("\n");
#endif
int nv=no;
for(i=0;i<4;i++){
  int np=0;
  for(j=0;j<nv;j++){
    int p=(j+nv-1)%nv;
    point2D a=avant[p];
    point2D b=avant[j];
    int x,y;
    unsigned char inta,intb;
    switch(i){
      case 0:
        x=0;
        inter_seg_v(a,b,x,&y);
        if(a.x>=0) inta=1; else inta=0;
        if(b.x>=0) intb=1; else intb=0;
        break;
      case 1:
        y=HAUTEUR;
        inter_seg_h(a,b,&x,y);
        if(a.y<=HAUTEUR) inta=1; else inta=0;
        if(b.y<=HAUTEUR) intb=1; else intb=0;
        break;
      case 2:
        x=LARGEUR;
        inter_seg_v(a,b,x,&y);
        if(a.x<=LARGEUR) inta=1; else inta=0;
        if(b.x<=LARGEUR) intb=1; else intb=0;
        break;
      case 3:
        y=0;
        inter_seg_h(a,b,&x,y);
        if(a.y>=0) inta=1; else inta=0;
        if(b.y>=0) intb=1; else intb=0;
        break;
      }
    if(intb){
      if(!inta){ apres[np].x=x; apres[np].y=y; np++; }
      apres[np++]=b;
      }
    else{
      if(inta){ apres[np].x=x; apres[np].y=y; np++; }
      }
    }
  for(j=0;j<np;j++) avant[j]=apres[j];
  nv=np;
  }
for(i=0;i<nv;i++) result[i]=avant[i];
#ifdef DEBUG
printf("!!");
for(i=0;i<nv;i++) printf("(%d,%d) ",result[i].x,result[i].y);
printf("\n");
#endif
*nr=nv;
}

/* Dessin d'un labyrinthe */

void dessine_2D(objet2D *objet,int no){
int i,j;
short int x[POINTS_MAX];
short int y[POINTS_MAX];
for(i=0;i<no;i++){
  if(objet[i].type==TYPE_MUR){
    point2D poly[POINTS_MAX];
    int np;
    inter_poly_rect(objet[i].def.p,4,poly,&np);
    for(j=0;j<np;j++){
      x[j]=poly[j].x;
      y[j]=HAUTEUR-poly[j].y;
    }
    polygonePlein(x,y,np,COULEUR_ROUGE,COULEUR_ROSE);
    }
  }
}

/* Fonction principale */

int main(void){
unsigned char resultat=creerFenetre(LARGEUR,HAUTEUR,TITRE);
if(!resultat){ fprintf(stderr,"Problème graphique !\n"); exit(-1); }
int nb=dessin_vers_murs(laby,murs);
point p={LABY_X/2*MUR_TAILLE,0,MUR_TAILLE};
int angle=0;
int touche;
unsigned char fenetre,quitter;
while(1){
  int evenement=attendreEvenement(&touche,&fenetre,&quitter);
  if(!evenement){ usleep(ATTENTE); continue; }
  if(touche){
    if(touche==TOUCHE_DROITE) angle += 5;
    if(touche==TOUCHE_GAUCHE) angle -= 5;
    if(angle<0 || angle>360) angle=angle%360;
    if(touche==TOUCHE_HAUT){
      p.x += MUR_TAILLE/10*sin(2*M_PI*angle/360);
      p.z += MUR_TAILLE/10*cos(2*M_PI*angle/360);
      }
    if(touche==TOUCHE_BAS){
      p.x -= MUR_TAILLE/10*sin(2*M_PI*angle/360);
      p.z -= MUR_TAILLE/10*cos(2*M_PI*angle/360);
      }
    }
  if(quitter==1) break;
  if(touche || fenetre){
    mur *m2=duplique_murs(murs,nb);
    decale_murs(m2,nb,p);
    rotation_murs(m2,nb,angle);
    tri_murs(m2,nb);
    objet2D *objets=malloc(nb*sizeof(objet2D));
    int no;
    projete_murs(m2,nb,objets,&no); free(m2);
    effacerFenetre();
    dessine_2D(objets,no); free(objets);
    synchroniserFenetre();
    }
  }
fermerFenetre();
return 0;
}
