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
unsigned int last_msg_datasz;

void msgs_handler(int sig, sMsg *msg /* in the shm */, unsigned int datasz, sGame *g) {
    sGameConf *conf;

    memcpy(&last_msg, msg, datasz); // make a copy because out of this handler, the shm may be modified
    datasz-=sizeof(eMsgsTypes); // datasz will only be the size of the payload
    last_msg_datasz=datasz;

    switch(msg->type) {
    case MSG_JOINGAME:
        conf = game_get_conf(g, NULL);
        strcpy(conf->playername[P_2], msg->data);

        // prepare and send answer to sender
        msg->type=MSG_CONFUPDATE;
        memcpy(msg->data, conf, sizeof(*conf));
        msg_answer(msg, sizeof(*conf));

    case MSG_GAMETURN:
    case MSG_END:   // 1 char payload (0 or 1 to choose if you should delete the histo file)
        sigmsgunlock(); // if we don't return from this isr, we have to unlock the shm
        siglongjmp(jumpenv, LJUMP_ISR);    // let's stop what we are doing
        break;  // never reached
    default:
        fprintf(stderr, "Unhandled message %d\n", msg->type);
        break;
    }
}

int msg_ctl(sGame *g, int cmd, struct sigmsgid_ds *buf) {
    key_t key;

    if((key=ftok(game_get_filepath(g), 0))==(key_t)-1)
        return -1;

    return sigmsgctl(key, cmd, buf);
}

int msg_init(sGame *g, int create) {
    key_t key;

    if((key=ftok(game_get_filepath(g), 0))==(key_t)-1)
        return -1;

    if(sigmsginit(key, 0600|(create?(IPC_CREAT|IPC_EXCL):0))==-1)
        return -1;

    if(sigmsgreg(SIGUSR1, (sigmsghnd)msgs_handler, (void *)g)==-1)
        return -1;

    return 0;
}

int msg_send(sMsg *msg, unsigned int datasz) {
    return sigmsgsend(SIGUSR1, (const void *)msg, datasz+sizeof(eMsgsTypes));
}

int msg_transfer(sMsg *msg, unsigned int *datasz) {
    int ret;

    (*datasz)+=sizeof(eMsgsTypes);

    ret=sigmsgtrans(SIGUSR1, (void *)msg, datasz);

    if(*datasz)
        (*datasz)-=sizeof(eMsgsTypes);

    return ret;
}

int msg_answer(sMsg *msg, unsigned int datasz) {
    return sigmsgans((const void *)msg, datasz+sizeof(eMsgsTypes));
}

int msg_deinit(int destroy) {
    return sigmsgdeinit(destroy);
}

