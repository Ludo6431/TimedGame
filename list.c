#include <stdlib.h> // malloc, free

#include "alloc.h"  // xmalloc, xfree

#include "list.h"   // LIST

// gestion de listes simplement chainées

LIST *list_append(LIST *list, const void *data) {
    LIST *start=list;

    if(list) {
        while(list->next)
            list = list->next;

        list->next=(LIST*)xmalloc(sizeof(LIST));
        list->next->next=NULL;

        list=list->next;
    }
    else {
        start=list=(LIST*)xmalloc(sizeof(LIST));
        list->next=NULL;
    }

    list->data=(void *)data;

    return start;
}

LIST *list_destroy(LIST *list) {
    LIST *next;

    while(list) {
        next=list->next;

        free(list);

        list=next;
    }

    return NULL;
}

LIST *list_destroy_full(LIST *list, free_handler handler) {
    LIST *next;

    if(!handler)
        return list_destroy(list);

    while(list) {
        next=list->next;

        handler(list->data);
        free(list);

        list=next;
    }

    return NULL;
}

