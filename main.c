/************************** main.c ************************/
#include <stdio.h>
#include <menu.h>

//char *menu_run(eMenuState st, char *buf, unsigned int *size);

int main(int argc, char *argv[]) {
    eMenuState MenuState=M_MAIN;
    char buf[256];
    unsigned int size=sizeof(buf);
    char *choix;

    while(1) {
        if(choix=menu_run(MenuState, buf, &size)==NULL || size<=1) {
            printf("Vous n'avez pas entré de choix correct\n");
            continue;   // loop
        }

        switch(choix[0]) {
        case '1':
            nouvelle_partie();
            break;
        case '2':
            connexion();
            break;
        case '3':
            reprise_partie_sauvegarde();
            break;
        case '4':
            interruption(); //retour menu avec sauvegarde
            break;
        case '5':
            pause();
            break;
        case '6':
            reprendre(); //reprise apres pause
            break;
        case '7':
            afficher_historique();
            break;
        case '8':
            quitter_avec_sauvegarde(); //retour au menu avec sauvegarde
            break;
        case '9':
            if(MenuState==M_MAIN) {
                quitter();
            }
            else {
                retour_menu();
            }
            break;
        case '/':
            jouer_coup();
            break;
        default:
            printf("\n Vous n'avez pas entrez de choix correct\n");
            break;
         }
    }//end while

    return 0;
}

