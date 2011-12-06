#ifndef _TOOLS_H
#define _TOOLS_H

// test the number of given parameters against the needed number of parameters
void ctrlArg(int argc, char **argv, int nb, char **params);
    // argc  : actual # of parameters
    // argv  : actual list of parameters
    // nb    : needed # of parameters
    // params: description of the parameters we need

// handle a system error
void exitOnErrSyst(char *func, char *text);
    // func: name of the function which failed
    // text: user message

#define EXIT_IF_M1(f, msg) if((f)==-1) exitOnErrSyst(#f, (msg))
#define EXIT_IF_0 (f, msg) if((f)==0 ) exitOnErrSyst(#f, (msg))
#define EXIT_IF_N0(f, msg) if((f)!=0 ) exitOnErrSyst(#f, (msg))

#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)<(b)?(b):(a))

#define BIT(b) (1<<(b))

#endif

