#ifndef _MENU_H
#define _MENU_H

#include "tools.h"  // BIT

typedef enum {
    M_MAIN      =BIT(0),
    M_WAITCON   =BIT(1),
    M_MYTURN    =BIT(2),
    M_HISTURN   =BIT(3),
    M_PAUSED    =BIT(4)
} eMenuState;

char *menu_run(eMenuState st, char *msg, char *ps1, char *buf, unsigned int size);

#endif

