#include <stdlib.h> // exit
#include <stdio.h>  // fprintf, perror
#include <errno.h>  // errno

#include "tools.h"

// test the number of given parameters against the needed number of parameters
void ctrlArg(int argc, char **argv, int nb, char **params) {
    // argc  : actual # of parameters
    // argv  : actual list of parameters
    // nb    : needed # of parameters
    // params: description of the parameters we need

    int i;

    if(argc-1 != nb) {
        // ok, the user failed, we just tell him...
        fprintf(stderr, "%s parameters (%d needed)!\n", argc-1>nb?"Too many":"Not enough", nb);

        // we print the usage line
        fprintf(stderr, "\tUsage: %s", argv[0]);
        for(i=0; i<nb && *params; i++, params++)
            fprintf(stderr, " <%s>", *params);
        fprintf(stderr, "\n");

        // we stop the execution of the program telling that the program failed
        exit(EXIT_FAILURE);
    }
}

// handle a system error
void exitOnErrSyst(char *func, char *text) {
    // func: name of the function which failed
    // text: user message

    // store the error number
    int lerrno = errno;

    // tell the user there is an error and the function which caused it
    fprintf(stderr, "ERROR function %s%s", func, (text && *text)?" - ":"");

    // print the user msg and the system msg
    errno = lerrno; // restore the error
    perror(text?:"");

    // we stop the execution of the program telling which error caused the stop
    exit(lerrno);
}

