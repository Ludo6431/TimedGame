#ifndef _SHM_H
#define _SHM_H

// this is where we handle the protected access to the shared memory

#include <pthread.h>

typedef struct {
    // synchro accés shm
    pthread_mutex_t m;

    // synchro attente
    pthread_mutex_t matt
    pthread_cond_t catt;

    // état en cours
    eStates stm; // ??
    eGameState stg;
    ePlayer stp;

    // passage infos
    char nom[128]; 
} sShm;

void    shm_lock                    (sShm *Shm);
void    shm_unlock                  (sShm *Shm);

#endif

