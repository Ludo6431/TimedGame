/************************** memoirepartagee.c ************************/
/* Ce fichier comporte :
 - creation_memoire_partage
 - attachement_memoire_partage

*/

#include "shm.h"


/*fonction appelé par nouvelle_partie(), connextion(), qui crée/ouvre l'espace de mémoire partagée*/
int ouverture_memoire_partage(int cle){

    /* Creation d'un segment de memoire partagee de 80 octets */
    if ((id = shmget(cle, 80, 0600|IPC_CREAT))==-1) {
        perror("Creation memoire partagee");
        exit(1);
    }

    return id;
}


/* Attachement de la zone de memoire a l'espace memoire du processus, connaissant l'id */
char* attachement_memoire_partage(int id){
    char *adresse; /* adresse du segment de memoire partagee */

    if ((int)(adresse = shmat(id, NULL, 0)) == -1) {
        perror("Attachement memoire partagee");
        shmctl(id, IPC_RMID, NULL); /* Suppression du segment */
        exit(1);
    }
	return adresse;
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

