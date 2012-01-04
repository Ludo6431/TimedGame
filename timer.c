#include <setjmp.h> // sig*jmp
#include <unistd.h> // alarm
#include <signal.h> // signal
#include <string.h> // memset

#include "timer.h"

static int _timer;
static timer_handler _uf;
static void *_up;
sigjmp_buf *_timer_env=NULL;
static int _timer_expired_code=0;

void _handler(int sig) {
    _timer--;

    if(_uf)
        _uf(sig, _timer, _up);

    if(_timer<=0) {
        _timer=0;
        siglongjmp(*_timer_env, _timer_expired_code); // timer expired
    }

    alarm(1);
}

int timer_start(int s, sigjmp_buf *env, int timer_expired_code, timer_handler f, void *userp) {
// when the timer expires, there will be a long jump to env with this timer_expired_code
    struct sigaction act;
    memset((void *)&act, '\0', sizeof(struct sigaction));

    _timer=s;
    _uf = f;
    _up = userp;
    _timer_env=env;
    _timer_expired_code=timer_expired_code;

    if(_uf)
        _uf(SIGALRM, _timer, _up);

    act.sa_handler=_handler;
    act.sa_flags=SA_RESTART;
    sigaction(SIGALRM, &act, NULL);

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

