#ifndef _ALLOC_H
#define _ALLOC_H

#include <stdlib.h> // calloc, malloc, free, realloc
#include <string.h> // strdup

#include "tools.h"  // exitOnErrSyst

// fonctions qui permettent de gérer les erreurs pour les fonctions d'allocations dynamique

static inline void *xcalloc(size_t nmemb, size_t size) {
    void *ret=calloc(nmemb, size);

    if(nmemb && size && !ret)
        exitOnErrSyst("calloc", NULL);

    return ret;
}

static inline void *xmalloc(size_t size) {
    void *ret=malloc(size);

    if(size && !ret)
        exitOnErrSyst("malloc", NULL);

    return ret;
}

static inline void xfree(void *ptr) {
    if(!ptr)
        exitOnErrSyst("free", NULL);

    free(ptr);
}

static inline void *xrealloc(void *ptr, size_t size) {
    void *ret=realloc(ptr, size);

    if(size && !ret)
        exitOnErrSyst("realloc", NULL);

    return ret;
}

static inline char *xstrdup(const char *s) {
    char *ret=strdup(s);

    if(s && !ret)
        exitOnErrSyst("strdup", (char *)s);

    return ret;
}

#endif

