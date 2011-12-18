#ifndef _LIST_H
#define _LIST_H

// gestion de listes simplement chainées

typedef struct LIST LIST;
struct LIST {
    struct LIST *next;

    void *data;
};

typedef void (*free_handler)(void *);

LIST *  list_append         (LIST *list, const void *data);
LIST *  list_destroy        (LIST *list);
LIST *  list_destroy_full   (LIST *list, free_handler handler);

#endif

