#ifndef _TIMER_H
#define _TIMER_H

#include <setjmp.h>

typedef void (*timer_handler)(int sig, int secs, void *userp);

int timer_start(int s, sigjmp_buf *env, int timer_expired_code, timer_handler f, void *userp);
int timer_pause();
int timer_resume();
int timer_stop();

#endif

