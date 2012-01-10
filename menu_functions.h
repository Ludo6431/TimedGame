#ifndef _MENU_FUNCTIONS_H
#define _MENU_FUNCTIONS_H

#include "game.h"   // sGame

// prototypes of menu_functions.c

int     nouvelle_partie             (sGame *g);
int     connexion                   (sGame *g);
int     reprise_partie_sauvegarde   (sGame *g);
int     sauvegarder                 (sGame *g);
void    afficher_historique         (sGame *g);
void    jouer_coup                  (sGame *g, char *s);
void    retour_menu                 (sGame *g, int del);

#endif

