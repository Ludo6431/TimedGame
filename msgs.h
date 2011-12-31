#ifndef _MSGS_H
#define _MSGS_H

#include "game.h"
#include "sigmsg.h"

typedef enum {
    MSG_JOINGAME,
    MSG_GAMETURN,
    MSG_ENDGAME
} eMsgsTypes;

typedef struct {
    eMsgsTypes type;

    char data[MAX_SIGMSG_SZ-sizeof(eMsgsTypes)];
} sMsg;

extern sMsg last_msg;
extern unsigned int last_msg_len;

int msg_init(char *path, int msgflg);
int msg_send(eMsgsTypes type, char *data, unsigned int datasz);
int msg_deinit(int destroy);

#endif

