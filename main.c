/************************** main.c ************************/
#include <stdio.h>


int main(int argc, char** argv){

    while(1){
        affiche_menu();
        choix=getchar();

                switch(choix){
                     case '1' : nouvelle_partie();
                                break;
                     case '2' : connexion();
                                break;
                     case '3' : reprise_partie_sauvegarde();
                                break;
                     case '4' : interruption();
                                break;
                     case '5' : pause();
                                break;
                     case '6' : reprendre();
                                break;
                     case '7' : afficher_historique();
                                break;
                     case '8' : quitter_avec_sauvegarde();
                                break;
                     case '9' : quitter();
                                break;
                     case '/' : jouer_coup();
                                break;
                     default : printf("\n Vous n'avez pas entré de choix correct\n");
                     }

    }//end while

    return(0);
}
