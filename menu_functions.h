#ifndef _MENU_FUNCTIONS_H
#define _MENU_FUNCTIONS_H

// prototypes of menu_functions.c




sGame *nouvelle_partie(sShm* adresse);

sGame* connexion(sGame *g);

void alarm_connexion(int numSig);

#endif

