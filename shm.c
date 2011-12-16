/************************** memoirepartagee.c ************************/
/* Ce fichier comporte :
 - creation_memoire_partage
 - attachement_memoire_partage

*/

#include <string.h> // memset
#include <sys/types.>   // key_t
#include <sys/ipc.h>    // ftok
#include <sys/shm.h>    // shm*

#include "tools."   // exitOnErrSyst

#include "shm.h"    // sShm

sShm *shm_open(char *path, int creat) {
    key_t shmkey;
    int shmid;
    sShm *shm;

    /* création de la clef */
    if((shmkey=ftok(path, 0))==(key_t)-1)
        exitOnErrSyst("ftok", game_get_filepath(g));

    /* création d'un segment de memoire partagee */
    if((shmid = shmget(shmkey, sizeof(sShm), 0600|(creat?IPC_CREAT|IPC_EXCL:0)))==-1)
        exitOnErrSyst("shmget", NULL);

    /* attachement à ce segment */
    if((shm = (sShm *)shmat(shmid, NULL, 0)) == (sShm *)-1) {
        shmctl(shmid, IPC_RMID, NULL); /* Suppression du segment */
        exitOnErrSyst("shmat", NULL);
    }

    if(creat) { // on initialise la mémoire si on vient de créer la zone de mémoire
        pthread_mutexattr_t mattr;
        int rc;

        memset((void *)shm, '\0', sizeof(sShm));

        if(rc=pthread_mutexattr_init(&mattr)) {
            errno=rc;
            exitOnErrSyst("pthread_mutexattr_init", NULL);
        }
        if(rc=pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED)) {
            errno=rc;
            exitOnErrSyst("pthread_mutexattr_setpshared", NULL);
        }

        if(rc=pthread_mutex_init(&shm->m, &mattr)) {
            errno=rc;
            exitOnErrSyst("pthread_mutex_init", NULL);
        }

        if(rc=pthread_mutexattr_destroy(&mattr)) {
            errno=rc;
            exitOnErrSyst("pthread_mutexattr_destroy", NULL);
        }

        shm->shmid=shmid;
    }

    return shm;
}

void shm_lock(sShm *Shm) {
    int rc;

    if(rc=pthread_mutex_lock(&Shm->m)) {
        errno=rc;
        exitOnErrSyst("pthread_mutex_lock", "shm");
    }
}

void shm_unlock(sShm *Shm) {
    int rc;

    if(rc=pthread_mutex_unlock(&Shm->m))
        errno=rc;
        exitOnErrSyst("pthread_mutex_unlock", "shm");
    }
}

void shm_close(sShm *shm, int destroy) {
    shmdt((void *)shm); // on se détache

    if(destroy) {
        int rc;

        if(rc=pthread_mutex_destroy(&shm->m)) {
            errno=rc;
            exitOnErrSyst("pthread_mutex_destroy", NULL);
        }

        shmctl(shm->shmid, IPC_RMID, NULL); /* marque segment pour suppression */
    }
}

