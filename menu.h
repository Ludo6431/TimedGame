#ifndef _MENU_H
#define _MENU_H

#include "tools.h"  // BIT

typedef enum {
    M_MAIN      =BIT(0),
    M_WAIT      =BIT(1),
    M_INGAME    =BIT(2),
    M_PAUSED    =BIT(3),
} eMenuState;

char *menu_run(eMenuState st, char *buf, unsigned int size);

#endif

