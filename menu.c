#include <stdlib.h>
#include <stdio.h>

#include "alloc.h"  // xalloc

/************************** menu.c ************************/
/* Ce fichier comporte :
- la fonction d'affichage du menu qui récupère aussi le choix de l'utilisateur
*/

struct {
    char key;
    char *msg;
    unsigned int flags;
} menu[]={
    {'1', "Nouvelle partie",                            M_MAIN},
    {'2', "Connexion à une partie",                     M_MAIN},
    {'3', "Charger une partie sauvegardée",             M_MAIN},
    {'4', "Stopper en sauvegardant",                    M_INGAME},
    {'5', "Mettre en pause",                            M_INGAME},
    {'6', "Reprendre",                                  M_INGAME},
    {'7', "Visualiser l'historique",                    M_INGAME},
    {'8', "Quitter en visualisant l'historique",        M_INGAME},
    {'9', "Quitter",                                    ~0},
    {'/', "Jouer un coup",                              M_INGAME},
}

// affiche le menu pour un état de menu donné et récupère une ligne tapée au clavier par l'utilisateur
char *menu_run(eMenuState st, char *buf, unsigned int *size) {
    char *ret;

    if(!buf) {
        buf=xmalloc(256);
        *size=256;
    }

    for(i=0; i<sizeof(menu)/sizeof(*menu); i++)
        if(menu[i].flags & st)
            printf("\t%c : %s\n", menu[i].key, menu[i].msg);

    ret=fgets(buf, *size, stdin);

    if(ret) {
        *size=strlen(ret)+1;

        if(ret[*size-2]=='\n') {
            ret[*size-2]='\0';
            (*size)--;
        }
    }
    else
        *size=0;

    return ret;
}

