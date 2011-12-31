#include <stdio.h>  // fprintf
#include <setjmp.h> // siglongjmp
#include <sys/types.h>  // key_t
#include <sys/ipc.h>    // ftok
#include <signal.h> // SIGUSR1

#include "tools.h"
#include "game.h"   // sGame
#include "sigmsg.h" // sigmsg*
#include "longjump.h"   // long jump stuff

#include "msgs.h"

sMsg last_msg;
unsigned int last_msg_len;

void msgs_handler(int sig, sMsg *msg /* in the shm */, unsigned int msglen, sGame *g) {
    memcpy(&last_msg, msg, msglen); // make a copy because out of this handler, the shm may be modified
    last_msg_len=msglen;
    switch(msg->type) {
    case MSG_ENDGAME:
        sigmsgunlock(); // if we don't return from this isr, we have to unlock the shm
        siglongjmp(jumpenv, LJUMP_ISR);    // let's stop what we are doing
        break;
    default:
        fprintf(stderr, "Unhandled message %d\n", msg->type);
        break;
    }
}

int msg_init(char *path, int msgflg) {
    key_t key;

    if((key=ftok(path, 0))==(key_t)-1)
        exitOnErrSyst("ftok", NULL);

    if(sigmsginit(key, msgflg)==-1)
        exitOnErrSyst("sigmsgget", NULL);

    if(sigmsgreg(SIGUSR1, (sigmsghnd)msgs_handler, NULL)==-1)
        exitOnErrSyst("sigmsgreg", NULL);

    return 0;
}

int msg_send(eMsgsTypes type, char *data, unsigned int datasz) {
    sMsg msg;

    msg.type=type;
    memcpy(msg.data, data, datasz);
    return sigmsgsnd(SIGUSR1, (void *)&msg, datasz+sizeof(eMsgsTypes), 0);
}

int msg_deinit(int destroy) {
    sigmsgdeinit(destroy);

    return 0;
}

