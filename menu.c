/************************** menu.c ************************/
/* Ce fichier comporte :
- la fonction d'affichage du menu
- la fonction qui récupère le choix de l'utilisateur et le traite
*/


/* fonction affichant les options offertes par le menu à l'utilisateur */
void affiche_menu(void){

    printf("\t\t\t\t MENU PRINCIPAL\n\n\n\n");
    printf("\t\t 1  :  Nouvelle partie \n");
    printf("\t\t 2  :  Connexion à une partie \n");
    printf("\t\t 3  :  Reprendre une partie sauvegardée \n");
    printf("\t\t 4  :  Interruption et sauvegarde\n");
    printf("\t\t 5  :  Pause \n\n");
	printf("\t\t 6  :  Reprise de la partie \n\n");
    printf("\t\t 7  :  Historique de la partie \n\n");
    printf("\t\t 8  :  Visualiser l'historique et Quitter \n\n");
    printf("\t\t 9  :  Quitter \n\n");
    printf("\t\t /  :  Jouer un coup \n\n");

}

