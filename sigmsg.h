#ifndef _SIGMSG_H
#define _SIGMSG_H

#define MAX_SIGMSG_SZ 256

typedef void (*sigmsghnd)(int sig, const void *msgp, size_t msgsz, void *userp);

int     sigmsginit      (key_t key, int msgflg);                    // initialize the message canal
int     sigmsgreg       (int sig, sigmsghnd func, void *userp);     // register a function which will be called on a received message

int     sigmsgsend      (int sig, const void *msgp, size_t msgsz);  // send a message on the canal
int     sigmsgans       (const void *msgp, size_t msgsz);           // answer to a message on the canal (you need to be in the signal handler to call it)
int     sigmsgtrans     (int sig, void *msgp, size_t *msgsz);       // send a message on the canal and get the answer immediately sent (if any)

inline
int     sigmsglock      ();                                         // lock the canal   [use at your own risks]
inline
int     sigmsgunlock    ();                                         // unlock the canal   [use at your own risks]

int     sigmsgdeinit    (int destroy);                              // deinit the message canal

#endif

