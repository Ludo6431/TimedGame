#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "timer.h"

void sighnd(int sig, int timer, void *data) {
    printf("\x1b[s\x1b[0;0H");

    printf("%02d seconds remaining", timer);

    printf("\x1b[u");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    char buffer[256];
    sigjmp_buf env;
    int var=1;

    printf("\x1b[2J\x1b[0;0Hline1\nline2\n");

    timer_start(5, &env, 1, sighnd, NULL);

    if(!sigsetjmp(env, 1)) {
        var=2;
        fgets(buffer, 256, stdin);
        printf("typed: \x1b[07m%s\x1b[00m\n", buffer);
        printf("you had %d seconds left...\n", timer_stop());
    }
    else
        printf("\x1b[07mTimer expired...\x1b[00m\n");

    printf("var=%d\n", var);

    return 0;
}

