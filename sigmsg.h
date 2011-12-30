#ifndef _SIGMSG_H
#define _SIGMSG_H

typedef void (*sigmsghnd)(int sig, const void *msgp, size_t msgsz, void *userp);

int sigmsgget(key_t key, int msgflg);   // create a new message canal
int sigmsgsnd(int sig, const void *msgp, size_t msgsz, int msgflg);   // send a message on the canal
int sigmsgreg(int sig, sigmsghnd func, void *userp);  // register a function which will be called on a received message
//int sigmsgctl(int cmd, struct msqid_ds *buf);    // controls a message canal

#endif

