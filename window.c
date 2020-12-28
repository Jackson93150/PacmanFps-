/*!\file window.c
 * \brief Utilisation du raster "maison" pour visualisation en vue
 * isométrique ou par dessus d'un labyrinthe (celui-ci est donné juste
 * à titre d'exemple).
 * \author Farès BELHADJ, amsi@up8.edu
 * \date December 4, 2020.
 *
 * A titre indicatif, temps de programmation de cet exemple : 35
 * minutes, en partant de sc_00_07_moteur-0.3.tgz, commentaires
 * compris.
 */
#include <assert.h>
/* inclusion des entêtes de fonctions de gestion de primitives simples
 * de dessin. La lettre p signifie aussi bien primitive que
 * pédagogique. */
#include <GL4D/gl4dp.h>
/* inclure notre bibliothèque "maison" de rendu */
#include "moteur.h"

/* inclusion des entêtes de fonctions de création et de gestion de
 * fenêtres système ouvrant un contexte favorable à GL4dummies. Cette
 * partie est dépendante de la bibliothèque SDL2 */
#include <GL4D/gl4duw_SDL2.h>
/* dans makeLabyrinthe.c, vous avez le droit de changer de générateur ... */
extern unsigned int * labyrinth(int w, int h);
/* protos de fonctions locales (static) */
static void init(void);
static void draw(void);
static void key(int keycode);
static void sortie(void);

/*!\brief une surface représentant un quadrilatère */
static surface_t * _quad = NULL;
/*!\brief une surface représentant un cube */
static surface_t * _cube = NULL;
static surface_t * _sphere = NULL; //sphere qui fera office de pacman
/*!\brief variable pour changer de vue, 0 -> isométrique ; 1 -> dessus */
static int _vue = 0;
/*!\brief le labyrinthe */
static unsigned int * _laby = NULL;
/*!\brief la largeur du labyrinthe */
static int lW = 21;
/*!\brief la hauteur du labyrinthe */
static int lH = 21;
static vec3 _ball = {0, 0, 0.84f};
int state = 0; // variable state qui va nous permettre de nous deplacer
/*!\brief paramètre l'application et lance la boucle infinie. */
int main(int argc, char ** argv) {
  /* tentative de création d'une fenêtre pour GL4Dummies */
  if(!gl4duwCreateWindow(argc, argv, /* args du programme */
			 "Mon moteur de rendu <<Maison>>", /* titre */
			 10, 10, 500, 500, /* x, y, largeur, heuteur */
			 GL4DW_SHOWN) /* état visible */) {
    /* ici si échec de la création souvent lié à un problème d'absence
     * de contexte graphique ou d'impossibilité d'ouverture d'un
     * contexte OpenGL (au moins 3.2) */
    return 1;
  }
  /* Pour forcer la désactivation de la synchronisation verticale */
  SDL_GL_SetSwapInterval(0);
  init();
  /* création d'un screen GL4Dummies (texture dans laquelle nous
   * pouvons dessiner) aux dimensions de la fenêtre */
  gl4dpInitScreen();
  /* mettre en place la fonction d'interception clavier */
  gl4duwKeyDownFunc(key);
  /* mettre en place la fonction de display */
  gl4duwDisplayFunc(draw);
  /* boucle infinie pour éviter que le programme ne s'arrête et ferme
   * la fenêtre immédiatement */
  gl4duwMainLoop();
  return 0;
}

/*!\brief init de nos données, spécialement les trois surfaces
 * utilisées dans ce code */
void init(void) {
  vec4 j = {1.0f, 1.0f, 0.0f, 0.0f}, g = {0.7f, 0.7f, 0.7f, 1}, b = {0.1f, 0.1f, 0.7f, 1} ;
  /* on créé nos deux types de surfaces */
  _quad   =   mkQuad();       /* ça fait 2 triangles        */
  _cube   =   mkCube();       /* ça fait 2x6 triangles      */
  _sphere = mkSphere(12,4);
  /* on change les couleurs de surfaces */
  _quad->dcolor = g; _cube->dcolor = b;_sphere->dcolor = j;
  /* on leur rajoute la texture au cube */
  setTexId(  _quad, getTexFromBMP("images/tex.bmp"));// 
  setTexId(  _cube, getTexFromBMP("images/tex2.bmp"));
  /* on active l'utilisation de la texture pour le cube */
  enableSurfaceOption(  _quad, SO_USE_TEXTURE);
  enableSurfaceOption(  _cube, SO_USE_TEXTURE);
  /* on active l'ombrage */
  enableSurfaceOption(  _quad, SO_USE_LIGHTING);
  enableSurfaceOption(  _cube, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere, SO_USE_LIGHTING);
  _laby = labyrinth(lW, lH);
  /* mettre en place la fonction à appeler en cas de sortie */
  atexit(sortie);
}

/*!\brief la fonction appelée à chaque display. */
void draw(void) {
  int i, j;
  float mvMat[16], projMat[16], nmv[16];
  //static float angle = 0.0f;
  /* effacer l'écran et le buffer de profondeur */
  gl4dpClearScreen();
  clearDepth();
  /* des macros facilitant le travail avec des matrices et des
   * vecteurs se trouvent dans la bibliothèque GL4Dummies, dans le
   * fichier gl4dm.h */
  /* charger un frustum dans projMat */
  MFRUSTUM(projMat, -0.05f, 0.05f, -0.05f, 0.05f, 0.1f, 1000.0f);
  /* charger la matrice identité dans model-view */
  MIDENTITY(mvMat);
  if(_vue == 0) {
    /* on place la caméra en haut, au sud-ouest du centre de la scène */
    lookAt(mvMat, -20, 50, 20, 0, 0, 0, 0, 1, 0);
  } else {
    /* on place la caméra en vue du dessus */
    lookAt(mvMat, 0, 50, -1, 0, 0, 0, 0, 1, 0);
  }
  /* je fais tourner l'ensemble pour que vous puissiez constater que
   * c'est bien en 3D, commentez ces 3 lignes pour que ça s'arrête */
  //rotate(mvMat, 5 * cos(angle), 1, 0, 0);
  //rotate(mvMat, 5 * sin(angle), 0, 2, 0);
  //angle += 0.01f;
  /* le quadrilatère est agrandi et couché */
  memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
  rotate(nmv, -90.0f, 1.0f, 0.0f, 0.0f);
  scale(nmv, lW - 1, lH - 1, 1.0f);
  transform_n_raster(_quad, nmv, projMat);
  /* le cube est relevé (+1 en y) et placé là où il y a des murs */
  for(i = -lH / 2; i <= lH / 2; ++i)
    for(j = -lW / 2; j <= lW / 2; ++j) {
       /* vide, pas de bloc mur */
      if(_laby[(i + lH / 2) * lW + j + lW / 2] == 0)continue;
      memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
      translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
      translate(nmv, 0.0f, 1.0f, 0.0f);
      transform_n_raster(_cube, nmv, projMat);
    }
  memcpy(nmv, mvMat, sizeof nmv); //affichage du pacman
  translate(nmv, _ball.x, 0, _ball.y);
  translate(nmv, -10.0f, 1.0f, 6.0f);
  transform_n_raster(_sphere, nmv, projMat);
  if(state == 0){ // pour chaque state le pacman va avancer dans une direction
    _ball.y -= 0.04f;
  }
  if(state == 1){
    _ball.x += 0.04f;
  }
  if(state == 2){
    _ball.y += 0.04f;
  }
  if(state == 3){
    _ball.x -= 0.04f;
  }
  /* déclarer qu'on a changé (en bas niveau) des pixels du screen  */
  gl4dpScreenHasChanged();
  /* fonction permettant de raffraîchir l'ensemble de la fenêtre*/
  gl4dpUpdateScreen(NULL);
}

/*!\brief intercepte l'événement clavier pour modifier les options. */
void key(int keycode) {
  switch(keycode) {
  case GL4DK_RIGHT: // ici on va definir pour la touche droite du clavier un state 
    if (state == 1){ // et si le pacman est daja dans un autre state autre que 0 on va lui assigner un state specifique
      state = 2;
      break;
    }
    if (state == 2){
      state = 3;
      break;
    }
    if (state == 3){
      state = 0;
      break;
    }
    state = 1;
    break;
  case GL4DK_LEFT: // pareille pour la touche gauche
    if (state == 1){
      state = 0;
      break;
    }
    if (state == 2){
      state = 1;
      break;
    }
    if (state == 3){
      state = 2;
      break;
    }
    state = 3;
    break;
  case GL4DK_v: /* 'v' changer de vue */
    _vue = !_vue;
    break;
  default: break;
  }
}

/*!\brief à appeler à la sortie du programme. */
void sortie(void) {
  /* on libère nos 2 surfaces */
  if(_quad) {
    freeSurface(_quad);
    _quad = NULL;
  }
  if(_cube) {
    freeSurface(_cube);
    _cube = NULL;
  }
  if(_sphere) {
    freeSurface(_sphere);
    _sphere = NULL;
  }
  /* on libère le labyrinthe */
  free(_laby);
  /* libère tous les objets produits par GL4Dummies, ici
   * principalement les screen */
  gl4duClean(GL4DU_ALL);
}
