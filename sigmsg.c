#include <sys/types.h>  // key_t, pid_t
#include <sys/ipc.h>
#include <sys/shm.h>    // shm*
#include <unistd.h> // getpid
#include <pthread.h>    // pthread_*
#include <signal.h> // signal, kill
#include <errno.h>  // errno
#include <string.h> // memcpy

#include "sigmsg.h"

typedef enum {
    SHM_INIT=0,
    SHM_IDLE,
    SHM_BUSY
} eShm;

#define MAX_SIGMSG_SZ 256
typedef struct {
    pthread_mutex_t me;
    pthread_cond_t cv;
    eShm status;
    pid_t dest, pids[2];

    // message content
    char tab[MAX_SIGMSG_SZ];
    unsigned int tablen;
} sShm;

static int shmid=0;
static sShm *shm=NULL;
static sigmsghnd _userh=NULL;
static void *_userp=NULL;

// TODO: finish, sanitize, check errors, ...

int sigmsgget(key_t key, int msgflg) {   // create a new message canal
    // TODO: use msgflg

    int rc;

    if((shmid = shmget(key, sizeof(sShm), msgflg))==-1) {
        shmid=0;
        return -1;
    }

    if((shm = shmat(shmid, NULL, 0))==(void *)-1) {
        shm=NULL;
        return -1;
    }

    if(msgflg&IPC_CREAT) {
        pthread_mutexattr_t mattr;
        pthread_condattr_t cattr;

        // init memory
        memset((void *)shm, '\0', sizeof(sShm));

        // init mutex
        if((rc=pthread_mutexattr_init(&mattr))) {
            shmdt(shm);
            errno=rc;
            return -1;
        }

        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

        if((rc=pthread_mutex_init(&shm->me, &mattr))) {
            pthread_mutexattr_destroy(&mattr);
            shmdt(shm);
            errno=rc;
            return -1;
        }

        pthread_mutexattr_destroy(&mattr);

        // init cond var
        if((rc=pthread_condattr_init(&cattr))) {
            shmdt(shm);
            errno=rc;
            return -1;
        }

        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

        if((rc=pthread_cond_init(&shm->cv, &cattr))) {
            pthread_condattr_destroy(&cattr);
            shmdt(shm);
            errno=rc;
            return -1;
        }

        pthread_condattr_destroy(&cattr);

        shm->pids[0]=getpid();
        shm->status=SHM_IDLE;
    }
    else {
        pthread_mutex_lock(&shm->me);

        shm->pids[1]=getpid();

        pthread_mutex_unlock(&shm->me);
    }

    return 0;
}

int sigmsgsnd(int sig, const void *msgp, size_t msgsz, int msgflg) {   // send a message on the canal
    if(!shm) {
        errno=ENXIO;
        return -1;
    }
    if(msgsz && !msgp) {
        errno=EINVAL;
        return -1;
    }

    pthread_mutex_lock(&shm->me);

    if(shm->status!=SHM_IDLE) {
        pthread_mutex_unlock(&shm->me);

        errno=EBUSY;

        return -1;
    }
    shm->status=SHM_BUSY;
    shm->dest=(shm->pids[0]==getpid())?shm->pids[1]:shm->pids[0];

    memcpy(shm->tab, msgp, msgsz);
    shm->tablen=msgsz;
    
    kill(shm->dest, sig);
    while(shm->status!=SHM_IDLE)
        pthread_cond_wait(&shm->cv, &shm->me);

    pthread_mutex_unlock(&shm->me);

    return 0;
}

void _dispatch(int sig) {
    pthread_mutex_lock(&shm->me);

    _userh(sig, shm->tab, shm->tablen, _userp);

    pthread_mutex_unlock(&shm->me);
}

int sigmsgreg(int sig, sigmsghnd func, void *userp) {  // register a function which will be called on a received message
    if(!shm) {
        errno=ENXIO;
        return -1;
    }
    if(!func) {
        errno=EINVAL;
        return -1;
    }

    pthread_mutex_lock(&shm->me);

    _userp=userp;
    _userh=func;

    signal(sig, _dispatch);

    pthread_mutex_unlock(&shm->me);

    return 0;
}
/*
int sigmsgctl(int cmd, struct msqid_ds *buf) {    // controls a message canal
    // TODO

    // shmctl(shmid, IPC_RMID, NULL); ...
    // shmdt(shm); ...
}*/

