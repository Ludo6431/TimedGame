/************************** memoirepartagee.c ************************/
/* Ce fichier comporte :
 - creation_memoire_partage
 - attachement_memoire_partage

*/

#include "shm.h"

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

