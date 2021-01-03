#include <time.h>
#include <assert.h>
#include <GL4D/gl4dp.h>
#include "moteur.h"
#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>
extern unsigned int *labyrinth(int w, int h);
static void init(void);
static void draw(void);
static void key(int keycode);
static void sortie(void);
double CubeVx[192]; // vecteur dans lequel on va stocker toute les position x des cubes
double CubeVy[192]; // vecteur dans lequel on va stocker toute les position y des cubes
double Bonusx[249]; // on va stocker la position x des bonus
double Bonusy[249]; // position y des bonus
double a = 0;
double a2 = 0;
double a3 = 0;
int x = 192;
int compteur = 0;
/*!\brief une surface représentant un quadrilatère */
static surface_t *_quad = NULL;
/*!\brief une surface représentant un cube */
static surface_t *_cube = NULL;
static surface_t *_cube2 = NULL;
static surface_t *_cube3 = NULL;
static surface_t *_cube4 = NULL;
static surface_t *_sphere = NULL; //sphere qui fera office de pacman
static surface_t *_sphere2 = NULL;
static surface_t *_sphere3 = NULL;
static surface_t *_sphere4 = NULL;
/*!\brief variable pour changer de vue, 0 -> isométrique ; 1 -> dessus */
static int _vue = 0;
int pacspeed = 1;
int flashmod = 0;
int speed = 1;
int level = 1;
/*!\brief le labyrinthe */
static unsigned int *_laby = NULL;
/*!\brief la largeur du labyrinthe */
static int lW = 21;
/*!\brief la hauteur du labyrinthe */
static int lH = 21;
int col = 0; // variable dans l'aquel on va stocker les states pendant une collision
static vec3 _ball = {-10.2f, 0.0f, 0.0f};
static vec3 _ghost = {6.0f, 0.0f, 0.0f};
static vec3 _ghost2 = {0.0f, 0.0f, 0.0f};
static vec3 _ghost3 = {-4.0f, 0.0f, 0.0f};
int state = 0; // variable state qui va nous permettre de nous deplacer
int statef = 0;
int statef2 = 0;
int statef3 = 0;
int scores = 0;
/*!\brief paramètre l'application et lance la boucle infinie. */

void restart(){
  int i;
  a = 0;
  a2 = 0;
  a3 = 0;
  x = 192;
  col = 0;
  _ball.x = -10.2f;
  _ball.y = 0.0f;
  _ghost.x = 6.0f;
  _ghost.y = 0.0f;
  _ghost2.x = 0.0f;
  _ghost2.y = 0.0f;
  _ghost3.x = -4.0f;
  _ghost3.y = 0.0f;
  state = 0; 
  statef = 0;
  statef2 = 0;
  statef3 = 0;
  flashmod = 0;
  pacspeed = 1;
  if (level == 1){ // si le level est 1 la vitesse des fantome sera de 1
    speed = 1;
    scores = 0;
  }
  else{ // sinon on augmente de 1 la vitesse des fantome
    speed += 1;
  }
  if(speed == 4){
    speed = 4;
  }
  for(i = 0; i < 249 ; i++){
    Bonusx[i] = 0.0;
    Bonusy[i] = 0.0;
  }
}


int main(int argc, char **argv)
{
  /* tentative de création d'une fenêtre pour GL4Dummies */
  if (!gl4duwCreateWindow(argc, argv,       /* args du programme */
                          "Pacman3D",       /* titre */
                          10, 10, 650, 600, /* x, y, largeur, heuteur */
                          GL4DW_SHOWN) /* état visible */)
  {
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
void init(void)
{
  vec4 j = {1.0f, 1.0f, 0.0f, 0.0f}, g = {0.7f, 0.7f, 0.7f, 1}, b = {0.1f, 0.1f, 0.7f, 1}, r = {1.0f, 0.0f, 0.0f, 0.0f}, ro = {0.8f, 0.0f, 0.8f, 0.25f};
  /* on créé nos deux types de surfaces */
  _quad = mkQuad(); /* ça fait 2 triangles        */
  _cube = mkCube(); /* ça fait 2x6 triangles      */
  _sphere = mkSphere(12, 6);
  _cube2 = mkCube();
  _cube3 = mkCube();
  _cube4 = mkCube();
  _sphere2 = mkSphere(8,2);
  _sphere3 = mkSphere(9,4);
  _sphere4 = mkSphere(9,4);
  /* on change les couleurs de surfaces */
  _quad->dcolor = g;
  _cube->dcolor = b;
  _sphere->dcolor = j;
  _cube2->dcolor = g;
  _cube3->dcolor = j;
  _cube4->dcolor = r;
  _sphere2->dcolor = ro;
  _sphere3->dcolor = r;
  _sphere4->dcolor = j;
  /* on leur rajoute la texture au cube */
  setTexId(_quad, getTexFromBMP("images/sol.bmp"));
  setTexId(_cube, getTexFromBMP("images/mur.bmp"));
  setTexId(_sphere, getTexFromBMP("images/pacman.bmp"));
  setTexId(_cube2, getTexFromBMP("images/ghost.bmp"));
  setTexId(_cube3, getTexFromBMP("images/ghost.bmp"));
  setTexId(_cube4, getTexFromBMP("images/ghost.bmp"));
  setTexId(_sphere2, getTexFromBMP("images/bonus.bmp"));
  setTexId(_sphere3, getTexFromBMP("images/bonus.bmp"));
  setTexId(_sphere4, getTexFromBMP("images/mur.bmp"));
  /* on active l'utilisation de la texture pour le cube */
  enableSurfaceOption(_quad, SO_USE_TEXTURE);
  enableSurfaceOption(_cube, SO_USE_TEXTURE);
  enableSurfaceOption(_sphere, SO_USE_TEXTURE);
  enableSurfaceOption(_cube2, SO_USE_TEXTURE);
  enableSurfaceOption(_cube3, SO_USE_TEXTURE);
  enableSurfaceOption(_cube4, SO_USE_TEXTURE);
  enableSurfaceOption(_sphere2, SO_USE_TEXTURE);
  enableSurfaceOption(_sphere3, SO_USE_TEXTURE);
  enableSurfaceOption(_sphere4, SO_USE_TEXTURE);
  /* on active l'ombrage */
  enableSurfaceOption(_quad, SO_USE_LIGHTING);
  enableSurfaceOption(_cube, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere, SO_USE_LIGHTING);
  enableSurfaceOption(_cube2, SO_USE_LIGHTING);
  enableSurfaceOption(_cube3, SO_USE_LIGHTING);
  enableSurfaceOption(_cube4, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere2, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere3, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere4, SO_USE_LIGHTING);
  _laby = labyrinth(lW, lH);
  /* mettre en place la fonction à appeler en cas de sortie */
  atexit(sortie);
}

/*!\brief la fonction appelée à chaque display. */
void draw(void)
{
  int i, j;
  float mvMat[16], projMat[16], nmv[16];
  int z = 0;
  int rst = 0;
  int ct = 0;
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
  if (_vue == 0)
  {
    /* on place la caméra en haut, au sud-ouest du centre de la scène */
    lookAt(mvMat, -20, 50, 20, 0, 0, 0, 0, 1, 0);
  }
  else
  {
    /* on place la caméra en vue du dessus */
    lookAt(mvMat, 10, 50, 20, 0, 0, 0, 0, 1, 0);
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
  //printf("%f",Bonusy[240]);
  /* le cube est relevé (+1 en y) et placé là où il y a des murs */
  for (i = -lH / 2; i <= lH / 2; ++i)
    for (j = -lW / 2; j <= lW / 2; ++j)
    {
      /* vide, pas de bloc mur */
      if (_laby[(i + lH / 2) * lW + j + lW / 2] == 0){ // dans le cas ou il ny'a pas de mur dans le labyrinth
        memcpy(nmv, mvMat, sizeof nmv); 
        a = 2 * j;
        a2 = 2 * i;
        a3 = 0;
        if (_ball.x + 1.0 >= a && _ball.x - 1.0 <= a && _ball.y + 1.0+6.0 >= a2 && _ball.y - 1.0+6.0 <= a2){ // on regarde ou se trouve le pacman 
          if (Bonusx[rst] == 50.0 && Bonusy[rst]== 50.0){
            if (a == 0 || a == 12 || a == - 12 || a2 == 0 || a2 == 12 || a2 == -12){
              if (a == 2 && a2 == 0){
                flashmod = 1; // quand le pacman prend le bonus du flashmod on active en passant flashmod a 1
              }
              scores -= 3; 
            }
            else{
              scores -= 1; // si le pacman a deja pris le bonus dans la case vide on enleve 1 au score
            }
          }
          Bonusx[rst] = 50.0; // a l'endroit ou se trouve le pacman on va utiliser le vecteur qu'on a creer dans motheur.h
          Bonusy[rst] = 50.0; // et on va definir la valeur 50.0 a la position de la case vide ou le pacman se trouve
          if (a == 0 || a == 12 || a == - 12 || a2 == 0 || a2 == 12 || a2 == -12){
            scores += 3;
          }
          else{
            scores += 1; // on rajoute 1 au score
          }
        }
        if (Bonusx[rst] != 50.0 && Bonusy[rst]!= 50.0){ // si le la position de la case vide n'est pas = 50.0
          if (a == 0 || a == 12 || a == - 12 || a2 == 0 || a2 == 12 || a2 == -12){
            if(a==2 && a2 == 0){
              translate(nmv, a, a3, a2); // on va draw le bonus du flashmod
              translate(nmv, 0.0f, 1.0f, 0.0f);
              scale(nmv, 0.8f,1.0f, 0.8f);
              transform_n_raster(_sphere4, nmv, projMat);
            }
            else{
              translate(nmv, a, a3, a2); // on va draw les bonus qui rapporte plus de points
              translate(nmv, 0.0f, 1.0f, 0.0f);
              scale(nmv, 0.55f,1.0f, 0.55f);
              transform_n_raster(_sphere3, nmv, projMat);
            }
          }
          else{
            translate(nmv, a, a3, a2); // on va draw les bonus
            translate(nmv, 0.0f, 1.0f, 0.0f);
            scale(nmv, 0.5f,1.0f, 0.5f);
            transform_n_raster(_sphere2, nmv, projMat);
          }
        }
        rst += 1;
      }
      else {
        memcpy(nmv, mvMat, sizeof nmv);  /* copie mvMat dans nmv */
        translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
        a = 2 * j;                       // a va etre = a la position x et a2 a la position y
        a2 = 2 * i;
        CubeVx[z] = a; // et on va stocker a et a2 dans leur vecteur correspondant
        CubeVy[z] = a2;
        z += 1;
        translate(nmv, 0.0f, 1.0f, 0.0f);
        scale(nmv, 0.75f,1.0f, 0.75f);
        transform_n_raster(_cube, nmv, projMat);
        }
    }
  memcpy(nmv, mvMat, sizeof nmv); //affichage du pacman
  translate(nmv, _ball.x, 1.0f, _ball.y + 6.0f);
  transform_n_raster(_sphere, nmv, projMat);

  memcpy(nmv, mvMat, sizeof nmv);
  translate(nmv, _ghost.x, 1.0f, _ghost.y);
  transform_n_raster(_cube2, nmv, projMat);

  memcpy(nmv, mvMat, sizeof nmv);
  translate(nmv, _ghost2.x, 1.0f, _ghost2.y + 6.0f);
  transform_n_raster(_cube3, nmv, projMat);

  memcpy(nmv, mvMat, sizeof nmv);
  translate(nmv, _ghost3.x, 1.0f, _ghost3.y + 10.0f);
  transform_n_raster(_cube4, nmv, projMat);

  // on attribue les state au ghost
  if (statef == 0)
  {
    _ghost.y -= (0.04f*speed);
  }
  if (statef == 1)
  {
    _ghost.x += (0.04f*speed);
  }
  if (statef == 2)
  {
    _ghost.y += (0.04f*speed);
  }
  if (statef == 3)
  {
    _ghost.x -= (0.04f*speed);
  }
  if (statef == 4)
  {
    _ghost.x += 0.0f;
    _ghost.y += 0.0f;
  }

  if (statef2 == 0)
  {
    _ghost2.y -= (0.04f*speed);
  }
  if (statef2 == 1)
  {
    _ghost2.x += (0.04f*speed);
  }
  if (statef2 == 2)
  {
    _ghost2.y += (0.04f*speed);
  }
  if (statef2 == 3)
  {
    _ghost2.x -= (0.04f*speed);
  }
  if (statef2 == 4)
  {
    _ghost2.x += 0.0f;
    _ghost2.y += 0.0f;
  }

  if (statef3 == 0)
  {
    _ghost3.y -= (0.04f*speed);
  }
  if (statef3 == 1)
  {
    _ghost3.x += (0.04f*speed);
  }
  if (statef3 == 2)
  {
    _ghost3.y += (0.04f*speed);
  }
  if (statef3 == 3)
  {
    _ghost3.x -= (0.04f*speed);
  }
  if (statef3 == 4)
  {
    _ghost3.x += 0.0f;
    _ghost3.y += 0.0f;
  }
  srand(time(NULL));

  for (i = 0; i < x; i++)
  { // on regard si le ghost rentre en collision avec les murs
    if (_ghost.x + 1.9 >= CubeVx[i] && _ghost.x - 1.9 <= CubeVx[i] && _ghost.y + 1.9 >= CubeVy[i] && _ghost.y - 1.9 <= CubeVy[i])
    {
      col = statef;       // on stock le state dans col
      int n = rand() % 4; // on choisi un nombre aleatoire entre 0 et 4 exclus
      if (col == 0)       // pour chaque cas des col
      {
        if (n == col) // on verifie si le nombre aleatoire choisi est = a col
        {
          n = rand() % 4; // si c'est le cas on chosi un autre nombre aléatoire
        }
        _ghost.y += ((0.04f*speed) + 0.01f); // on deplace le ghost a lopposer de la colision en sorte qu'il ne le soit plus
        statef = n;        // et le state du ghost sera = au nombre aleatoire qui a été choisi
      }
      if (col == 1) // la meme pour les autre cas de col
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost.x -= ((0.04f*speed) + 0.01f);
        statef = n;
      }
      if (col == 2)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost.y -= ((0.04f*speed) + 0.01f);
        statef = n;
      }
      if (col == 3)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost.x += ((0.04f*speed) + 0.01f);
        statef = n;
      }
    }
  }
  // la meme pour les autres ghost
  for (i = 0; i < x; i++)
  {
    if (_ghost2.x + 1.9 >= CubeVx[i] && _ghost2.x - 1.9 <= CubeVx[i] && _ghost2.y + 1.9 + 6.0 >= CubeVy[i] && _ghost2.y - 1.9 + 6.0 <= CubeVy[i])
    {
      col = statef2;
      int n = rand() % 4;
      if (col == 0)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.y += ((0.04f*speed) + 0.01f);
        statef2 = n;
      }
      if (col == 1)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.x -= ((0.04f*speed) + 0.01f);
        statef2 = n;
      }
      if (col == 2)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.y -= ((0.04f*speed) + 0.01f);
        statef2 = n;
      }
      if (col == 3)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.x += ((0.04f*speed) + 0.01f);
        statef2 = n;
      }
    }
  }

  for (i = 0; i < x; i++)
  {
    if (_ghost3.x + 1.9 >= CubeVx[i] && _ghost3.x - 1.9 <= CubeVx[i] && _ghost3.y + 1.9 + 10.0 >= CubeVy[i] && _ghost3.y - 1.9 + 10.0 <= CubeVy[i])
    {
      col = statef3;
      int n = rand() % 4;
      if (col == 0)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.y += ((0.04f*speed) + 0.01f);
        statef3 = n;
      }
      if (col == 1)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.x -= ((0.04f*speed) + 0.01f);
        statef3 = n;
      }
      if (col == 2)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.y -= ((0.04f*speed) + 0.01f);
        statef3 = n;
      }
      if (col == 3)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.x += ((0.04f*speed) + 0.01f);
        statef3 = n;
      }
    }
  }

  if(flashmod == 1){ // active le flashmod
    pacspeed = 3; // on augmente la vitesse du pacman
    compteur += 1; // on continue tant que le compteur n'atteint pas 2500
  }

  if(compteur == 2500){ // quand le compteur atteint 2500 on le remet les valeurs initiale
    pacspeed = 1;
    compteur = 0;
    flashmod = 0;
  }

  for (i = 0; i < x; i++)
  {
    if (_ball.x + 1.7 >= CubeVx[i] && _ball.x - 1.7 <= CubeVx[i] && _ball.y + 1.7 + 6.0 >= CubeVy[i] && _ball.y - 1.7 + 6.0 <= CubeVy[i])
    {              // range de la collison
      col = state; // on stock le state dans la variable col
      state = 4;   // on passe au state 4 qui nous fige
      if (col == 0)
      {                   // on regarde le state dans laquel on était durant la collision
        _ball.y += ((0.04f*pacspeed) + 0.01f); // et on deplace la balle dans du cote opposer du state en sorte que la balle ne soit plus en collision
      }
      if (col == 1)
      {
        _ball.x -= ((0.04f*pacspeed) + 0.01f);
      }
      if (col == 2)
      {
        _ball.y -= ((0.04f*pacspeed) + 0.01f);
      }
      if (col == 3)
      {
        _ball.x += ((0.04f*pacspeed) + 0.01f);
      }
    }
  }
  // effet de collision  entre les ghost et le pacman
  if (_ball.x + 2.0 >= _ghost.x && _ball.x - 2.0 <= _ghost.x && _ball.y + 2.0+6.0 >= _ghost.y && _ball.y - 2.0+6.0 <= _ghost.y)
  { 
    printf("Votre scores est de %d et vous avez atteint le niveau %d \n",scores,level);     
    level = 1;    
    restart();
  }

  if (_ball.x + 2.0 >= _ghost2.x && _ball.x - 2.0 <= _ghost2.x && _ball.y + 2.0 >= _ghost2.y && _ball.y - 2.0 <= _ghost2.y)
  {        
    printf("Votre scores est de %d et vous avez atteint le niveau %d \n",scores,level);
    level = 1;  
    restart();
  }

  if (_ball.x + 2.0 >= _ghost3.x && _ball.x - 2.0 <= _ghost3.x && _ball.y + 2.0-4.0 >= _ghost3.y && _ball.y - 2.0-4.0 <= _ghost3.y)
  {      
    printf("Votre scores est de %d et vous avez atteint le niveau %d \n",scores,level);
    level = 1;   
    restart();
  }

  for(i = 0; i < 249 ; i++){ // fonction qui va nous permettre de changer de niveau
    if (Bonusx[i] == 50.0 && Bonusy[i] == 50.0){
      ct += 1; // on regarde si les bonus on été pris
    }
    if (ct == 249){ // si tous les bonus ont été pris (les 249)
      level += 1; // on augmente le niveau de 1 et on restart
      restart();
    }
  }

  if (state == 0)
  { // pour chaque state le pacman va avancer dans une direction
    _ball.y -= (0.04f * pacspeed);
  }
  if (state == 1)
  {
    _ball.x += (0.04f * pacspeed);
  }
  if (state == 2)
  {
    _ball.y += (0.04f * pacspeed);
  }
  if (state == 3)
  {
    _ball.x -= (0.04f * pacspeed);
  }
  if (state == 4)
  {
    _ball.x += 0.0f;
    _ball.y += 0.0f;
  }
  /* déclarer qu'on a changé (en bas niveau) des pixels du screen  */
  gl4dpScreenHasChanged();
  /* fonction permettant de raffraîchir l'ensemble de la fenêtre*/
  gl4dpUpdateScreen(NULL);
}

/*!\brief intercepte l'événement clavier pour modifier les options. */
void key(int keycode)
{
  switch (keycode)
  {
  case GL4DK_RIGHT: // ici on va definir pour la touche droite du clavier un state
    if (state == 4)
    { // on regarde d'abord si nous somme en state 4 c'est a dire en collision
      if (col == 1)
      {        // si c'est le cas on regarde quel etat était le state pendant la colision
        break; // si le state durant la collision et le meme que l'on veut appliquer avec notre touche on break
      }
    }
    state = 1;
    break;
  case GL4DK_LEFT: // pareille pour les autres touches
    if (state == 4)
    {
      if (col == 3)
      {
        break;
      }
    }
    state = 3;
    break;
  case GL4DK_UP:
    if (state == 4)
    {
      if (col == 0)
      {
        break;
      }
    }
    state = 0;
    break;
  case GL4DK_DOWN:
    if (state == 4)
    {
      if (col == 2)
      {
        break;
      }
    }
    state = 2;
    break;
  case GL4DK_v: /* 'v' changer de vue */
    _vue = !_vue;
    break;
  default:
    break;
  }
}

/*!\brief à appeler à la sortie du programme. */
void sortie(void)
{
  /* on libère nos 2 surfaces */
  if (_quad)
  {
    freeSurface(_quad);
    _quad = NULL;
  }
  if (_cube)
  {
    freeSurface(_cube);
    _cube = NULL;
  }
  if (_sphere)
  {
    freeSurface(_sphere);
    _sphere = NULL;
  }
  if (_cube2)
  {
    freeSurface(_cube2);
    _cube2 = NULL;
  }
  /* on libère le labyrinthe */
  free(_laby);
  /* libère tous les objets produits par GL4Dummies, ici
   * principalement les screen */
  gl4duClean(GL4DU_ALL);
}
