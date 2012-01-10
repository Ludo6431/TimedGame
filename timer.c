#include <setjmp.h> // sig*jmp
#include <unistd.h> // alarm
#include <signal.h> // signal
#include <string.h> // memset
#include <stdlib.h> // free
#include <errno.h>  // errno

#include <stdio.h>

#include "list.h"   // singly-linked list management

#include "timer.h"

static LIST *_timers_alive, *_timers_paused, *_timers_expired;

void _handler(int sig) {
    LIST *prev=NULL, *tmp, *next;
    sTimer *env;

    // first, decrement each timer and remove timedout timers from the alive list
    tmp=_timers_alive;
    while(tmp) {
        next=tmp->next;

        env=(sTimer *)tmp->data;

        // decrement 
        env->timer--;

        if(env->uf)
            env->uf(sig, env->timer, env->up);

        // end of time
        if(env->timer<=0) {
            env->timer=0;

            // extract tmp from the _timers_alive list
            if(prev)
                prev->next=tmp->next;
            else
                _timers_alive=tmp->next;

            // append tmp to the _timers_expired list
            tmp->next=NULL;
            if(_timers_expired) {
                LIST *el=_timers_expired;
                while(el->next)
                    el=el->next;
                el->next=tmp;
            }
            else
                _timers_expired=tmp;
        }
        else
            prev=tmp;

        tmp=next;
    }

    // second, if there's still some timers running, reschedule a SIGALRM signal
    if(_timers_alive || (_timers_expired && _timers_expired->next))
        alarm(1);

    // third, go to first timedout timer's saved environment
    // (we can't handle all in one run => we may have other timers timedout in the done list (they will be handled next time this handler will be called))
    if(_timers_expired) {
        tmp=_timers_expired;
        _timers_expired=_timers_expired->next;

        env=(sTimer *)tmp->data;

        free(tmp);

        siglongjmp(*env->jmpenv, env->jmpcode); // timer expired

        // never reached
    }
}

int timer_start(sTimer *t, int v) {
// when the timer expires, there will be a long jump to env with this timer_expired_code
    struct sigaction act;
    memset((void *)&act, '\0', sizeof(struct sigaction));

    // TODO: add SIGALRM to sigmask

    if(v)
        t->timer=v;
    _timers_alive=list_append(_timers_alive, (void *)t);

    if(t->uf)
        t->uf(SIGALRM, t->timer, t->up);

    act.sa_handler=_handler;
    act.sa_flags=SA_RESTART;
    sigaction(SIGALRM, &act, NULL);

    // TODO: remove SIGALRM from sigmask

    alarm(1);

    return 0;
}

int timer_set(sTimer *t, int v) {
    // TODO: add SIGALRM to sigmask

    t->timer=v;

    // TODO: remove SIGALRM from sigmask

    return 0;
}

int timer_get(sTimer *t) {
    int v;

    // TODO: add SIGALRM to sigmask

    v=t->timer;

    // TODO: remove SIGALRM from sigmask

    return v;
}

int timer_pause(sTimer *t) {
    LIST *prev=NULL, *tmp=_timers_alive;

    // TODO: add SIGALRM to sigmask

    while(tmp) {
        if(tmp->data==t) {
            // extract tmp from the _timers_alive list
            if(prev)
                prev->next=tmp->next;
            else
                _timers_alive=tmp->next;

            // prepend tmp to the _timers_paused list
            tmp->next=_timers_paused;
            _timers_paused=tmp;

            break;
        }

        prev=tmp;
        tmp=tmp->next;
    }

    // TODO: remove SIGALRM from sigmask

    if(!_timers_alive && !_timers_expired)
        alarm(0);

    if(!tmp) {
        errno=EINVAL;
        return -1;
    }

    return ((sTimer *)tmp->data)->timer;
}

int timer_resume(sTimer *t) {
    LIST *prev=NULL, *tmp=_timers_paused;

    // TODO: add SIGALRM to sigmask

    while(tmp) {
        if(tmp->data==t) {
            // extract tmp from the _timers_paused list
            if(prev)
                prev->next=tmp->next;
            else
                _timers_paused=tmp->next;

            // prepend tmp to the _timers_alive list
            tmp->next=_timers_alive;
            _timers_alive=tmp;

            break;
        }

        prev=tmp;
        tmp=tmp->next;
    }

    // TODO: remove SIGALRM from sigmask

    if(_timers_alive || _timers_expired)
        alarm(1);

    if(!tmp) {
        errno=EINVAL;
        return -1;
    }

    return ((sTimer *)tmp->data)->timer;
}

int timer_stop(sTimer *t) {
    LIST *prev, *tmp;
    int ret;

    // TODO: add SIGALRM to sigmask

    // first, search in alive timers
    prev=NULL;
    tmp=_timers_alive;
    while(tmp) {
        if(tmp->data==t) {
            // extract tmp from the _timers_alive list
            if(prev)
                prev->next=tmp->next;
            else
                _timers_alive=tmp->next;
            break;
        }

        prev=tmp;
        tmp=tmp->next;
    }

    // second, search in paused timers
    if(!tmp) {
        prev=NULL;
        tmp=_timers_paused;
        while(tmp) {
            if(tmp->data==t) {
                // extract tmp from the _timers_alive list
                if(prev)
                    prev->next=tmp->next;
                else
                    _timers_alive=tmp->next;
                break;
            }

            prev=tmp;
            tmp=tmp->next;
        }
    }

    // TODO: remove SIGALRM from sigmask

    if(!_timers_alive && !_timers_expired)
        alarm(0);

    if(!tmp) {
        errno=EINVAL;
        return -1;
    }

    ret=((sTimer *)tmp->data)->timer;

    free(tmp);

    return ret;
}

