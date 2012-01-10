#ifndef _MSGS_H
#define _MSGS_H

#include "game.h"   // sGame
#include "sigmsg.h" // MAX_SIGMSG_SZ

typedef enum {
    // init handshake
    MSG_JOIN,
    MSG_INITST8,
    MSG_READY,
    MSG_START,

    // in-game messages
    MSG_TURN,
    MSG_PAUSE,
    MSG_RESUME,

    // ending message
    MSG_END
} eMsgsTypes;

typedef struct {
    eMsgsTypes type;

    char data[MAX_SIGMSG_SZ-sizeof(eMsgsTypes)];    // payload
} sMsg;

extern sMsg last_msg;
extern unsigned int last_msg_len;

int msg_ctl         (sGame *g, int cmd, struct sigmsgid_ds *buf);
int msg_init        (sGame *g, int create);

int msg_send        (sMsg *msg, unsigned int datasz);
int msg_transfer    (sMsg *msg, unsigned int *datasz);
int msg_answer      (sMsg *msg, unsigned int datasz);

int msg_deinit      (int destroy);

#endif

