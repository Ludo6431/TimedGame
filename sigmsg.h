#ifndef _SIGMSG_H
#define _SIGMSG_H

#define MAX_SIGMSG_SZ 256

typedef void (*sigmsghnd)(int sig, const void *msgp, size_t msgsz, void *userp);

int sigmsginit(key_t key, int msgflg);   // initialize the message canal
int sigmsgsnd(int sig, const void *msgp, size_t msgsz, int msgflg);   // send a message on the canal
int sigmsglock();
int sigmsgunlock();
int sigmsgreg(int sig, sigmsghnd func, void *userp);  // register a function which will be called on a received message
int sigmsgdeinit(int destroy);  // deinit the message canal

#endif

