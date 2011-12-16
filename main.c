/************************** main.c ************************/
#include <stdio.h>
#include <menu.h>

//char *menu_run(eMenuState st, char *buf, unsigned int *size);

int main(int argc, char *argv[]) {
    eMenuState MenuState=M_MAIN;
    sGame game; // partie en cours
    sShm *shm=NULL; // mémoire partagée associée à la partie en cours
    char buf[256];
    unsigned int size=sizeof(buf);
    char *choix;

    while(1) {
        if((choix=menu_run(MenuState, buf, &size))==NULL || size<=1) {
            printf("Vous n'avez pas entré de choix correct\n");
            continue;   // loop
        }

        switch(MenuState) {
        case M_MAIN:
            switch(choix[0]) {
            case '1':   // "Nouvelle partie"
                shm=nouvelle_partie(&game);
                break;
            case '2':   // "Connexion à une partie"
                shm=connexion(&game);
                break;
            case '3':   // "Charger une partie sauvegardée"
                shm=reprise_partie_sauvegarde(&game);
                break;
            case '9':   // "Quitter"
                exit(0);
                break;
            default:
                printf("\n Vous n'avez pas entré de choix correct\n");
                break;
            }
            if(shm)
                MenuState=M_INGAME;

            break;
        case M_INGAME:
            switch(choix[0]) {
            case '4':   // "Stopper en sauvegardant" -> retour au menu principal
                sauvegarder(&game, shm);

                retour_menu(&game, shm);
                shm=NULL;
                break;
            case '5':   // "Mettre en pause"
                pause(&game, shm);    // on reste en état INGAME
                break;
            case '6':   // "Reprendre"
                reprendre(&game, shm);
                break;
            case '7':   // "Visualiser l'historique"
                afficher_historique(&game, shm);
                break;
            case '8':   // "Stopper en visualisant l'historique" -> retour au menu principal (sans sauvegarder)
                afficher_historique(&game, shm);
            case '9':   // "Retour au menu" -> retour au menu principal (sans sauvegarder)
                retour_menu(&game, shm);
                shm=NULL;
                break;
            case '/':   // "Jouer un coup"
                jouer_coup(&game, shm);
                break;
            default:
                printf("\n Vous n'avez pas entré de choix correct\n");
                break;
            }
            if(!shm)
                MenuState=M_MAIN;

            break;
        }
    }   // end while

    return 0;
}

