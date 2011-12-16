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

    if(creat)   // on initialise la mémoire si on vient de créer la zone de mémoire
        memset((void *)shm, '\0', sizeof(sShm));

    shm->shmid=shmid;

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

void shm_close(sShm *shm) {
    shmdt((void *)shm); // on se détache
}

void shm_destroy(sShm *shm) {
    shmctl(shm->shmid, IPC_RMID, NULL); /* marque segment pour suppression */
}

