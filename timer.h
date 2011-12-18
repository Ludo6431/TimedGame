#ifndef _TIMER_H
#define _TIMER_H

#include <setjmp.h>

// DRAFT
typedef void (*timer_handler)(int sig, int secs, void *userp);
extern sigjmp_buf _timer_env;

int timer_start(int s, timer_handler f, void *userp);
int timer_pause();
int timer_resume();
int timer_stop();
#define timer_expired() sigsetjmp(_timer_env, 1)
// in `man sigsetjmp`: "The stack context will be invalidated if the function which called setjmp() returns."
// timer_expired can't be a function

#endif

