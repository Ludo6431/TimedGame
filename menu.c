/************************** menu.c ************************/
/* Ce fichier comporte :
- la fonction d'affichage du menu qui récupère aussi le choix de l'utilisateur
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h> // strlen

#include "menu.h"

struct {
    unsigned int flag;
    char *msg;
} states[]={
    {M_MAIN,    "Menu principal"},
    {M_WAITCON, "Attente d'un joueur..."},
    {M_MYTURN,  "Partie en cours, à votre tour de jouer"},
    {M_HISTURN, "Partie en cours, attente que l'autre joueur ait joué son tour"},
    {M_PAUSED,  "Partie en pause"},
};

struct {
    char key;
    char *msg;
    unsigned int flags;
} menu[]={
    {'1', "Nouvelle partie",                        M_MAIN                                           },
    {'2', "Connexion à une partie",                 M_MAIN                                           },
    {'3', "Charger une partie sauvegardée",         M_MAIN                                           },
    {'4', "Stopper en sauvegardant",                                   M_MYTURN| M_HISTURN| M_PAUSED },
    {'5', "Mettre en pause",                                           M_MYTURN                      },
    {'6', "Reprendre",                                                                      M_PAUSED },
    {'7', "Visualiser l'historique",                                   M_MYTURN| M_HISTURN| M_PAUSED },
    {'8', "Stopper en visualisant l'historique",                       M_MYTURN| M_HISTURN| M_PAUSED },
    {'9', "Quitter",                                M_MAIN                                           },
    {'9', "Annuler",                                        M_WAITCON                                },
    {'9', "Retour au menu principal",                                  M_MYTURN| M_HISTURN| M_PAUSED },
    {'/', "Jouer un coup",                                             M_MYTURN                      }
};

int _get_max_items() {
    int i, j, k, nb=0;

    for(i=0; i<sizeof(states)/sizeof(*states); i++) {   // pour chaque état de menu ...
        k=0;

        for(j=0; j<sizeof(menu)/sizeof(*menu); j++) // ... on regarde le nombre max d'éléments qu'on peut avoir
            if(menu[j].flags & states[i].flag)
                k++;

        nb=MAX(nb, k);
    }

    return nb;
}

// affiche le menu pour un état de menu donné, récupère une ligne tapée au clavier par l'utilisateur et vérifie que le choix est possible
char *menu_run(eMenuState st, char *msg, char *buf, unsigned int size) {
    char *ret;
    int i, nb=0;
    static int nbitemsmax = 0;

    if(!nbitemsmax)
        nbitemsmax=_get_max_items();

    // print menu title
    for(i=0; i<sizeof(states)/sizeof(*states); i++)
        if(states[i].flag & st) {
            printf("\x1b[K%s\n", states[i].msg);    // clear the line and write the new content
            nb++;
        }

    // print menu items
    for(i=0; i<sizeof(menu)/sizeof(*menu); i++)
        if(menu[i].flags & st) {
            printf("\x1b[K\t%c : %s\n", menu[i].key, menu[i].msg);  // clear the line and write the new content
            nb++;
        }

    for(i=nb; i<nbitemsmax+1; i++)
        printf("\x1b[K\n"); // clear the line and write the new content

    if(msg && *msg)
        printf("\x1b[05m%s\x1b[00m", msg);
    msg[0]='\x0';

    printf("\n>");    // ceci sera toujours affiché au même endroit peut-importe le nombre d'éléments dans le menu (cf boucle ci-dessus) 
    fflush(stdout);

    // get a line from the standard input
    ret=fgets(buf, size, stdin);
    if(!ret)
        return NULL;

    size=strlen(ret);

    // remove the trailing line feeding
    if(ret[size-1]=='\n')
        ret[size-1]='\0';

    // check user input
    for(i=0; i<sizeof(menu)/sizeof(*menu); i++)
        if(menu[i].flags & st && ret[0]==menu[i].key)
            return ret;

    return NULL;
}

