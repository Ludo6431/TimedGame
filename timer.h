#ifndef _TIMER_H
#define _TIMER_H

#include <setjmp.h>

typedef void (*timer_handler)(int sig, int secs, void *userp);

typedef struct {
    int timer;
    timer_handler uf;
    void *up;
    sigjmp_buf *jmpenv;
    int jmpcode;
} sTimer;

int timer_start (sTimer *t);
int timer_pause (sTimer *t);
int timer_resume(sTimer *t);
int timer_stop  (sTimer *t);

#endif

