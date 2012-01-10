#ifndef _LONGJUMP_H
#define _LONGJUMP_H

#include <setjmp.h>

typedef enum {
    LJUMP_TIMER_CONN=1,
    LJUMP_TIMER_TURN,
    LJUMP_TIMER_GLOB,
    LJUMP_ISR
} eLJumps;

extern sigjmp_buf jumpenv;

#endif

