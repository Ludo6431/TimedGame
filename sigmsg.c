#include <sys/types.h>  // key_t, pid_t
#include <sys/ipc.h>
#include <sys/shm.h>    // shm*
#include <unistd.h> // getpid
#include <pthread.h>    // pthread_*
#include <signal.h> // signal, kill
#include <errno.h>  // errno
#include <string.h> // memcpy, memmove, memset

#include "sigmsg.h"

#define BIT(b) (1<<(b))

typedef enum {
    SHM_ANS =BIT(1),    // we have an answer

    SHM_BUSY=BIT(7)     // the shm is busy
} eShm;

typedef struct {
    pthread_mutex_t me;
    pthread_cond_t cv;
    eShm status;
    pid_t dest, pids[2];

    // message content
    char tab[MAX_SIGMSG_SZ];
    unsigned int tablen;
} sShm;

static int _shmid=0;
static sShm *_shm=NULL;
static sigmsghnd _userh=NULL;
static void *_userp=NULL;

// TODO: finish, sanitize, check errors, ...

int sigmsginit(key_t key, int msgflg) {   // create a new message canal
    // TODO: use msgflg

    int rc;

    if((_shmid = shmget(key, sizeof(sShm), msgflg))==-1) {
        _shmid=0;
        return -1;
    }

    if((_shm = shmat(_shmid, NULL, 0))==(void *)-1) {
        _shm=NULL;
        return -1;
    }

    if(msgflg&IPC_CREAT) {
        // the memory is zero-filled on creation

        pthread_mutexattr_t mattr;
        pthread_condattr_t cattr;

        // init mutex
        if((rc=pthread_mutexattr_init(&mattr))) {
            shmdt(_shm);
            errno=rc;
            return -1;
        }

        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

        if((rc=pthread_mutex_init(&_shm->me, &mattr))) {
            pthread_mutexattr_destroy(&mattr);
            shmdt(_shm);
            errno=rc;
            return -1;
        }

        pthread_mutexattr_destroy(&mattr);

        // init cond var
        if((rc=pthread_condattr_init(&cattr))) {
            shmdt(_shm);
            errno=rc;
            return -1;
        }

        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

        if((rc=pthread_cond_init(&_shm->cv, &cattr))) {
            pthread_condattr_destroy(&cattr);
            shmdt(_shm);
            errno=rc;
            return -1;
        }

        pthread_condattr_destroy(&cattr);

        _shm->pids[0]=getpid();
    }
    else {
        pthread_mutex_lock(&_shm->me);

        _shm->pids[1]=getpid();

        pthread_mutex_unlock(&_shm->me);
    }

    return 0;
}

void _dispatch(int sig) {
    pthread_mutex_lock(&_shm->me);

    _shm->status&=~SHM_BUSY;    // we are not busy anymore
    pthread_cond_signal(&_shm->cv);

    _userh(sig, _shm->tab, _shm->tablen, _userp);

    pthread_mutex_unlock(&_shm->me);    /// after that, the sender processus will be liberated
}

int sigmsgreg(int sig, sigmsghnd func, void *userp) {  // register a function which will be called on a received message
    struct sigaction act;
    memset((void *)&act, '\0', sizeof(struct sigaction));

    if(!_shm) {
        errno=ENXIO;
        return -1;
    }
    if(!func) {
        errno=EINVAL;
        return -1;
    }

    _userp=userp;
    _userh=func;

    act.sa_handler=_dispatch;
    act.sa_flags=SA_RESTART;
    sigaction(sig, &act, NULL);

    return 0;
}

inline int _sigmsgsend(int sig, const void *msgp, size_t msgsz) {
    if(_shm->status & (SHM_BUSY|SHM_ANS)) {  // wtf? something is not finished
        errno=EBUSY;
        return -1;
    }

    _shm->status|=SHM_BUSY;
    _shm->dest=(_shm->pids[0]==getpid())?_shm->pids[1]:_shm->pids[0];   // TODO: check this variable
    if(!_shm->dest) {
        errno=ENXIO;
        return -1;
    }

    memcpy(_shm->tab, msgp, msgsz);
    _shm->tablen=msgsz;

    // let's tell the other processus there's a message for it
    kill(_shm->dest, sig);
    while(_shm->status & SHM_BUSY)   // and wait until he acknoledges it
        pthread_cond_wait(&_shm->cv, &_shm->me);

    return 0;
}

int sigmsgsend(int sig, const void *msgp, size_t msgsz) {   // send a message on the canal
    if(!_shm) {
        errno=ENXIO;
        return -1;
    }
    if(msgsz && !msgp) {
        errno=EINVAL;
        return -1;
    }

    pthread_mutex_lock(&_shm->me);

    // send the message
    if(_sigmsgsend(sig, msgp, msgsz)) {
        pthread_mutex_unlock(&_shm->me);
        return -1;
    }

     // we received an answer, we are not supposed to expose it here => let's call the signal handler
    if(_shm->status & SHM_ANS) {
        _shm->status&=~SHM_ANS;  // acknowledge

        // TODO: mask signal sig

        _userh(sig, _shm->tab, _shm->tablen, _userp);

        // TODO: unmask signal sig
    }

    pthread_mutex_unlock(&_shm->me);

    return 0;
}

int sigmsgans(const void *msgp, size_t msgsz) {    // answer to a message on the canal (you need to be in the signal handler to call it)
    if(!_shm) {
        errno=ENXIO;
        return -1;
    }
    if(msgsz && !msgp) {
        errno=EINVAL;
        return -1;
    }

    if(_shm->status & (SHM_BUSY|SHM_ANS)) {  // wtf? something is not finished
        errno=EBUSY;
        return -1;
    }

    if(!msgsz)  // nothing to do
        return 0;

    _shm->status |= SHM_ANS;    // tell the sender we send him back an answer

    // we may use the provided data in the signal handler to store the data, in this case data could overlap => we use memmove
    if(_shm->tab != msgp)
        memmove(_shm->tab, msgp, msgsz);
    _shm->tablen=msgsz;

    return 0;
}

int sigmsgtrans(int sig, void *msgp, size_t *msgsz) {   // send a message on the canal and get the answer immediately sent (if any)
    if(!_shm) {
        errno=ENXIO;
        return -1;
    }
    if(!msgsz || !msgp) {
        errno=EINVAL;
        return -1;
    }

    pthread_mutex_lock(&_shm->me);

    // send the message
    if(_sigmsgsend(sig, (const void *)msgp, *msgsz)) {
        pthread_mutex_unlock(&_shm->me);
        return -1;
    }

    // we received an answer, we are supposed to expose it here
    if(_shm->status & SHM_ANS) {
        _shm->status&=~SHM_ANS;  // acknowledge

        memcpy(msgp, _shm->tab, _shm->tablen);
        *msgsz=_shm->tablen;
    }
    else
        *msgsz=0;

    pthread_mutex_unlock(&_shm->me);

    return 0;
}

inline int sigmsglock() {   // lock the canal   [use at your own risks]
    return pthread_mutex_lock(&_shm->me);
}
inline int sigmsgunlock() { // unlock the canal   [use at your own risks]
    return pthread_mutex_unlock(&_shm->me);
}

int sigmsgdeinit(int destroy) {
    if(destroy)
        shmctl(_shmid, IPC_RMID, NULL);

    shmdt(_shm);

    _shmid=0;
    _shm=NULL;

    return 0;
}

