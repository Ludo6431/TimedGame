/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
*/


void nouvelle_partie(void){

    char nom_joueur[50];
	char nom_partie[50];
	char concatenation[100]
	int id,cle;
	int attente_joueur=1;

	memoirepartagee donnees_partie;

	printf("Entrez votre nom :\n")
	fgets(nom_joueur,50,stdin);
	printf("Quel nom voulez vous donner à votre partie?\n")
	fgets(nom_partie,50,stdin);

    /*creation de la clee*/
    sprintf(concatenation, "%s%s",nom_joueur,nom_partie);
    cle=strtol(concatenation,NULL,10);
    //cle2=ftok("gestion_temps.c", 'cle'));

    id=ouverture_memoire_partage(cle);

    attachement_memoire_partage(id);

    printf("Entrez la durée de la partie (en secondes) : \n")
	scanf("%d", donnees_partie->dureepartie);
    printf("Entrez le temps alloué à un coup : \n")
	scanf("%d", donnees_partie->dureecoup);

    donnees_partie.connexion=0; // pour attendre la connexion
/****************************afinir*/
    /*attente connextion d'un joueur
      si pas de connextion ...
      else..
      */

    /* Installation du handler gestionAlarme pour SIGALRM */
    signal(SIGALRM,gestionAlarme);

    temps_ecoule=gestionAlarme(); //(renvoie 1 si elle reçoit sigalrm)
    alarm(15);
    while(temps_ecoule==0){
        if(donnees_partie.connexion==1){
            alarm(0);
            printf("Le deuxième joueur s'est connecter, la partie commence.\n");
            //attente_joueur=0;
            initialiser_jeu();
            afficher_jeu(); //affichage du plateau, des "labels"...
        }
    }//end while
    if(donnees_partie.connexion==0){
        printf("Le deuxième joueur ne s'sest pas connecté a temps\n");

    }
}


void connexion(void){

    char nom_joueur[50];
	char nom_partie[50];
	char concatenation[100]
	int id,cle;

	printf("Entrez le nom du joueur avec qui vous voulez jouer:\n")
	fgets(nom_joueur,50,stdin);
	printf("Entrez le nom de la partie :\n")
	fgets(nom_partie,50,stdin);

    /*creation de la clee*/
    sprintf(concatenation, "%s%s",nom_joueur,nom_partie);
    cle=strtol(concatenation,NULL,10);
    //cle2=ftok("gestion_temps.c", 'cle'));

    id=ouverture_memoire_partage(cle);

    attachement_memoire_partage(id);

    donnees_partie.connexion=1; //previent qu'il s'est connecté

	afficher_jeu(); //affichage du plateau, des "labels"...

}


void reprise_partie_sauvegarde(void){


}


/* a mettre dans gestiontemps.c*/

//sigjmp_buf contexteAlarme;
int gestionAlarme(int numSig) {
//siglongjmp( contexteAlarme, 1);
 /* branchement a sigsetjmp() en
retournant 1 */

return 1;
}

