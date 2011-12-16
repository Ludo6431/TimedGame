#ifndef _MENU_FUNCTIONS_H
#define _MENU_FUNCTIONS_H

// prototypes of menu_functions.c

sShm *  nouvelle_partie (sGame *g);
sShm *  connexion       (sGame *g);

void alarm_connexion(int numSig);

#endif

