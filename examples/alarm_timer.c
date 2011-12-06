#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

unsigned int timer=10;

void sighnd(int i) {
    printf("\x1b[s\x1b[0;0H");

    printf("Il te reste %02d secondes !!", --timer);

    if(timer<=3) printf("\x07");
    if(timer<=0) exit(0);

    printf("\x1b[u");
    fflush(stdout);

    alarm(1);
}

int main(int argc, char *argv[]) {
    signal(SIGALRM, sighnd);

    printf("\x1b[2J\x1b[0;0Hline1\nline2\n");

    alarm(1);

    char buffer[256];
    fgets(buffer, 256, stdin);

    printf("\x1b[07m%s", buffer);

    exit(0);
}

