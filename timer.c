#include <setjmp.h> // sig*jmp
#include <unistd.h> // alarm
#include <signal.h> // signal

#include "timer.h"

static int _timer;
static timer_handler _uf;
static void *_up;
sigjmp_buf _timer_env;

void _handler(int sig) {
    _timer--;

    if(_uf)
        _uf(sig, _timer, _up);

    if(_timer<=0) {
        _timer=0;
        siglongjmp(_timer_env, 1); // timer expired
    }

    alarm(1);
}

int timer_start(int s, timer_handler f, void *userp) {
    _timer=s;
    _uf = f;
    _up = userp;

    if(_uf)
        _uf(SIGALRM, _timer, _up);

    signal(SIGALRM, _handler);

    alarm(1);

    return s;
}

int timer_pause() {
    alarm(0);
    return _timer;
}

int timer_resume() {
    alarm(1);
    return _timer;
}

int timer_stop() {
    int ret=_timer;

    alarm(0);
    _timer=0;

    return ret;
}

