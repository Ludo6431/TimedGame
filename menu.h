#ifndef _MENU_H
#define _MENU_H

#include "tools.h"  // BIT

typedef enum {
    M_MAIN      =BIT(0),
    M_INGAME    =BIT(1),
    NB_MENU
} eMenuState;

char *menu_run(eMenuState st, char *buf, unsigned int *size);

#endif

