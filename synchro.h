#ifndef _SYNCHRO_H
#define _SYNCHRO_H

typedef enum {
    SHM_ASKNAME,
    SHM_ASKTTURN,
    SHM_ASKTTOTAL,
    SHM_ASKTREMAINING,
    SHA_ASKREADY,
    SHM_ASKQUIT,

    SHM_GAVENAME,
    SHM_GAVETTURN,
    SHM_GAVETTOTAL,
    SHM_GAVETREMAINING,
    SHA_READY,
    SHM_QUIT,

    SHM_SYNCHRO_OK

} eShmsynchro;


void synchro_handler(int numSig);

#endif
