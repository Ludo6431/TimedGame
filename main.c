/************************** main.c ************************/
#include <stdio.h>

#include "menu.h"   // menu_run
#include "menu_functions.h" // ...
#include "timer.h"  // timer_expired

int main(int argc, char *argv[]) {
    eMenuState MenuState=M_MAIN;
    sGame game; // partie en cours
    sShm *shm=NULL; // mémoire partagée associée à la partie en cours
    char buf[256];
    char *choix;

    while(1) {
        printf("\x1b[2J\x1b[0;0H");

        if(!(choix=menu_run(MenuState, buf, sizeof(buf))) || !strlen(choix))
            continue;   // loop

        timer_stop();

        switch(choix[0]) {
        case '1':   // M_MAIN, "Nouvelle partie"
            shm=nouvelle_partie(&game);
            if(shm)
                MenuState=M_WAIT;
            break;
        case '2':   // M_MAIN, "Connexion à une partie"
//            shm=connexion(&game);
            if(shm)
                MenuState=M_INGAME;
            break;
        case '3':   // M_MAIN, "Charger une partie sauvegardée"
//            shm=reprise_partie_sauvegarde(&game);
            if(shm)
                MenuState=M_WAIT;
            break;
        case '4':   // M_INGAME|M_PAUSED, "Stopper en sauvegardant" -> retour au menu principal
//            sauvegarder(&game, shm);

            retour_menu(&game, shm);
            shm=NULL;
            MenuState=M_MAIN;
            break;
        case '5':   // M_INGAME, "Mettre en pause"
//            pause(&game, shm);
            MenuState=M_PAUSED;
            break;
        case '6':   // M_PAUSED, "Reprendre"
//            reprendre(&game, shm);
            MenuState=M_INGAME;
            break;
        case '7':   // M_INGAME|M_PAUSED, "Visualiser l'historique"
//            afficher_historique(&game, shm);
            // on ne change pas d'état
            break;
        case '8':   // M_INGAME|M_PAUSED, "Stopper en visualisant l'historique" -> retour au menu principal (sans sauvegarder)
//            afficher_historique(&game, shm);

            retour_menu(&game, shm);
            shm=NULL;
            MenuState=M_MAIN;
        case '9':   // M_*
            if(MenuState==M_MAIN) { // M_MAIN, "Quitter"
                exit(0);
            }
            else {  // M_WAIT|M_INGAME|M_PAUSED, "Retour au menu principal" (sans sauvegarder)
                retour_menu(&game, shm);
                shm=NULL;
                MenuState=M_MAIN;
            }
            break;
        case '/':   // M_INGAME, "Jouer un coup"
//            jouer_coup(&game, shm);
            break;
        }

        if(timer_expired()) {
            // TODO afficher message erreur

            retour_menu(&game, shm);
            shm=NULL;
            MenuState=M_MAIN;
        }
    }   // end while

    return 0;
}

