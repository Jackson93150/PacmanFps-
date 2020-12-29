#include <time.h>
#include <assert.h>
#include <GL4D/gl4dp.h>
#include "moteur.h"
#include <GL4D/gl4duw_SDL2.h>
extern unsigned int *labyrinth(int w, int h);
static void init(void);
static void draw(void);
static void key(int keycode);
static void sortie(void);
double CubeVx[192]; // vecteur dans lequel on va stocker toute les position x des cubes
double CubeVy[192]; // vecteur dans lequel on va stocker toute les position y des cubes
double a = 0;
double a2 = 0;
int x = 192;
/*!\brief une surface représentant un quadrilatère */
static surface_t *_quad = NULL;
/*!\brief une surface représentant un cube */
static surface_t *_cube = NULL;
static surface_t *_cube2 = NULL;
static surface_t *_cube3 = NULL;
static surface_t *_cube4 = NULL;
static surface_t *_sphere = NULL; //sphere qui fera office de pacman
/*!\brief variable pour changer de vue, 0 -> isométrique ; 1 -> dessus */
static int _vue = 0;
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
/*!\brief paramètre l'application et lance la boucle infinie. */
int main(int argc, char **argv)
{
  /* tentative de création d'une fenêtre pour GL4Dummies */
  if (!gl4duwCreateWindow(argc, argv,       /* args du programme */
                          "Pacman3D",       /* titre */
                          10, 10, 500, 500, /* x, y, largeur, heuteur */
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
  vec4 j = {1.0f, 1.0f, 0.0f, 0.0f}, g = {0.7f, 0.7f, 0.7f, 1}, b = {0.1f, 0.1f, 0.7f, 1}, r = {1.0f, 0.0f, 0.0f, 0.0f};
  /* on créé nos deux types de surfaces */
  _quad = mkQuad(); /* ça fait 2 triangles        */
  _cube = mkCube(); /* ça fait 2x6 triangles      */
  _sphere = mkSphere(12, 2);
  _cube2 = mkCube();
  _cube3 = mkCube();
  _cube4 = mkCube();
  /* on change les couleurs de surfaces */
  _quad->dcolor = g;
  _cube->dcolor = b;
  _sphere->dcolor = j;
  _cube2->dcolor = g;
  _cube3->dcolor = j;
  _cube4->dcolor = r;
  /* on leur rajoute la texture au cube */
  setTexId(_quad, getTexFromBMP("images/sol.bmp"));
  setTexId(_cube, getTexFromBMP("images/mur.bmp"));
  setTexId(_sphere, getTexFromBMP("images/pacman.bmp"));
  setTexId(_cube2, getTexFromBMP("images/ghost.bmp"));
  setTexId(_cube3, getTexFromBMP("images/ghost.bmp"));
  setTexId(_cube4, getTexFromBMP("images/ghost.bmp"));
  /* on active l'utilisation de la texture pour le cube */
  enableSurfaceOption(_quad, SO_USE_TEXTURE);
  enableSurfaceOption(_cube, SO_USE_TEXTURE);
  enableSurfaceOption(_sphere, SO_USE_TEXTURE);
  enableSurfaceOption(_cube2, SO_USE_TEXTURE);
  enableSurfaceOption(_cube3, SO_USE_TEXTURE);
  enableSurfaceOption(_cube4, SO_USE_TEXTURE);
  /* on active l'ombrage */
  enableSurfaceOption(_quad, SO_USE_LIGHTING);
  enableSurfaceOption(_cube, SO_USE_LIGHTING);
  enableSurfaceOption(_sphere, SO_USE_LIGHTING);
  enableSurfaceOption(_cube2, SO_USE_LIGHTING);
  enableSurfaceOption(_cube3, SO_USE_LIGHTING);
  enableSurfaceOption(_cube4, SO_USE_LIGHTING);
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
  for (i = -lH / 2; i <= lH / 2; ++i)
    for (j = -lW / 2; j <= lW / 2; ++j)
    {
      /* vide, pas de bloc mur */
      if (_laby[(i + lH / 2) * lW + j + lW / 2] == 0)
        continue;
      memcpy(nmv, mvMat, sizeof nmv);  /* copie mvMat dans nmv */
      translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
      a = 2 * j;                       // a va etre = a la position x et a2 a la position y
      a2 = 2 * i;
      CubeVx[z] = a; // et on va stocker a et a2 dans leur vecteur correspondant
      CubeVy[z] = a2;
      z += 1;
      translate(nmv, 0.0f, 1.0f, 0.0f);
      transform_n_raster(_cube, nmv, projMat);
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

  if (statef == 0)
  {
    _ghost.y -= 0.04f;
  }
  if (statef == 1)
  {
    _ghost.x += 0.04f;
  }
  if (statef == 2)
  {
    _ghost.y += 0.04f;
  }
  if (statef == 3)
  {
    _ghost.x -= 0.04f;
  }
  if (statef == 4)
  {
    _ghost.x += 0.0f;
    _ghost.y += 0.0f;
  }
  
  if (statef2 == 0)
  {
    _ghost2.y -= 0.04f;
  }
  if (statef2 == 1)
  {
    _ghost2.x += 0.04f;
  }
  if (statef2 == 2)
  {
    _ghost2.y += 0.04f;
  }
  if (statef2 == 3)
  {
    _ghost2.x -= 0.04f;
  }
  if (statef2 == 4)
  {
    _ghost2.x += 0.0f;
    _ghost2.y += 0.0f;
  }

  if (statef3 == 0)
  {
    _ghost3.y -= 0.04f;
  }
  if (statef3 == 1)
  {
    _ghost3.x += 0.04f;
  }
  if (statef3 == 2)
  {
    _ghost3.y += 0.04f;
  }
  if (statef3 == 3)
  {
    _ghost3.x -= 0.04f;
  }
  if (statef3 == 4)
  {
    _ghost3.x += 0.0f;
    _ghost3.y += 0.0f;
  }
  srand(time(NULL));

  for (i = 0; i < x; i++)
  {
    if (_ghost.x + 1.9 >= CubeVx[i] && _ghost.x - 1.9 <= CubeVx[i] && _ghost.y + 1.9 >= CubeVy[i] && _ghost.y - 1.9 <= CubeVy[i])
    {
      col = statef;
      int n = rand() % 4;
      if (col == 0)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost.y += 0.05f;
        statef = n;
      }
      if (col == 1)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost.x -= 0.05f;
        statef = n;
      }
      if (col == 2)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost.y -= 0.05f;
        statef = n;
      }
      if (col == 3)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost.x += 0.05f;
        statef = n;
      }
    }
  }

  for (i = 0; i < x; i++)
  {
    if (_ghost2.x + 1.9 >= CubeVx[i] && _ghost2.x - 1.9 <= CubeVx[i] && _ghost2.y + 1.9 + 6.0 >= CubeVy[i] && _ghost2.y - 1.9 + 6.0<= CubeVy[i])
    {
      col = statef2;
      int n = rand() % 4;
      if (col == 0)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.y += 0.05f;
        statef2 = n;
      }
      if (col == 1)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.x -= 0.05f;
        statef2 = n;
      }
      if (col == 2)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.y -= 0.05f;
        statef2 = n;
      }
      if (col == 3)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost2.x += 0.05f;
        statef2 = n;
      }
    }
  }

  for (i = 0; i < x; i++)
  {
    if (_ghost3.x + 1.9 >= CubeVx[i] && _ghost3.x - 1.9 <= CubeVx[i] && _ghost3.y + 1.9 + 10.0 >= CubeVy[i] && _ghost3.y - 1.9 + 10.0<= CubeVy[i])
    {
      col = statef3;
      int n = rand() % 4;
      if (col == 0)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.y += 0.05f;
        statef3 = n;
      }
      if (col == 1)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.x -= 0.05f;
        statef3 = n;
      }
      if (col == 2)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.y -= 0.05f;
        statef3 = n;
      }
      if (col == 3)
      {
        if (n == col)
        {
          n = rand() % 4;
        }
        _ghost3.x += 0.05f;
        statef3 = n;
      }
    }
  }

  for (i = 0; i < x; i++)
  {
    if (_ball.x + 1.7 >= CubeVx[i] && _ball.x - 1.7 <= CubeVx[i] && _ball.y + 1.7 + 6.0 >= CubeVy[i] && _ball.y - 1.7 + 6.0 <= CubeVy[i])
    {              // range de la collison
      col = state; // on stock le state dans la variable col
      state = 4;   // on passe au state 4 qui nous fige
      if (col == 0)
      {                   // on regarde le state dans laquel on était durant la collision
        _ball.y += 0.05f; // et on deplace la balle dans du cote opposer du state en sorte que la balle ne soit plus en collision
      }
      if (col == 1)
      {
        _ball.x -= 0.05f;
      }
      if (col == 2)
      {
        _ball.y -= 0.05f;
      }
      if (col == 3)
      {
        _ball.x += 0.05f;
      }
    }
  }
  if (state == 0)
  { // pour chaque state le pacman va avancer dans une direction
    _ball.y -= 0.04f;
  }
  if (state == 1)
  {
    _ball.x += 0.04f;
  }
  if (state == 2)
  {
    _ball.y += 0.04f;
  }
  if (state == 3)
  {
    _ball.x -= 0.04f;
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
