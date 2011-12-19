#ifndef _TOOLS_H
#define _TOOLS_H


// test the number of given parameters against the needed number of parameters
void ctrlArg(int argc, char **argv, int nb, char **params);

// handle a system error
void exitOnErrSyst(char *func, char *text);

char *readStdin(char *s, int size);


#endif


