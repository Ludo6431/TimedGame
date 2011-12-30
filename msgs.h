#ifndef _MSGS_H
#define _MSGS_H

#include "game.h"

typedef enum {
    MSG_HELLO,
    MSG_TURN,
    MSG_QUIT
} eMsgs;

void msgs_handler(int sig, void *msg, unsigned int msglen, sGame *g);

#endif

