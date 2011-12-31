#ifndef _LONGJUMP_H
#define _LONGJUMP_H

#include <setjmp.h>

typedef enum {
    LJUMP_TIMER=1,
    LJUMP_ISR
} eLJumps;

extern sigjmp_buf jumpenv;

#endif

