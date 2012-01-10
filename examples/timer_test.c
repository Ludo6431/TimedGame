#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "timer.h"

// gcc -o timer -I.. timer.c timer_test.c ../list.c ../tools.c

void sighnd(int sig, int timer, void *data) {
    printf("%c7\x1b[%d;4H", '\x1b', (int)data);

    printf("%d seconds remaining\x1b[K", timer);

    printf("%c8", '\x1b');
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    sigjmp_buf env;
    int var=1, i, s;

    sTimer timers[]={
        {20, sighnd, (void *)1, &env, 20},
        {15, sighnd, (void *)2, &env, 15},
        {13, sighnd, (void *)3, &env, 13},
        {10, sighnd, (void *)4, &env, 10},
        { 9, sighnd, (void *)5, &env,  9},
        { 5, sighnd, (void *)6, &env,  5},
        { 1, sighnd, (void *)7, &env,  1}
    };

    printf("\x1b[2J\x1b[0;0H");

    for(i=0; i<sizeof(timers)/sizeof(*timers); i++) {
        if(i!=4)
            timer_start(&timers[i], 0);
        printf("%2d:\n", timers[i].jmpcode);
    }
    printf("\n");

    s=sigsetjmp(env, 1);

    if(s==0)
        var=2;
    else
        printf("Timer %d expired\n", s);

    if(s==5) {
        timer_pause(&timers[1]);
        printf("Paused timer 15\n");

        timer_start(&timers[4], 0);
        printf("Started timer 9\n");

        timer_stop(&timers[2]);
        printf("Stopped timer 13\n");
    }

    if(s==9) {
        timer_start(&timers[2], 13);
        printf("Started timer 13\n");
    }

    if(s==10) {
        timer_resume(&timers[1]);
        printf("Resumed timer 15\n");
    }

    getchar();

    printf("var=%d\n", var);

    return 0;
}

