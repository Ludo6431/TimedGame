#ifndef _MENU_FUNCTIONS_H
#define _MENU_FUNCTIONS_H

#include "shm.h"    // sShm

// prototypes of menu_functions.c

sShm *  nouvelle_partie (sGame *g);
sShm *  connexion       (sGame *g);
void    retour_menu     (sGame *game, sShm *shm);

void alarm_connexion(int numSig);

#endif

