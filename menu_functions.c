/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
*/

#include "game.h"


void nouvelle_partie(void){

	char nom_partie[50];
	sGame donnees_partie;

	printf("Quel nom voulez vous donner à votre partie?\n")
	fgets(nom_partie,50,stdin);
	
	game_new(&donnees_partie,nom_partie);//game_new intialise le jeu
	
    /* Installation du handler gestionAlarme pour SIGALRM */
    signal(SIGALRM,gestionAlarme);
	
	//utiliser var cond pthread_condattr_setpshared, faire toute l'initialisation de la var conditionnelle
    alarm(30);
	
	pthread_mutex_lock(&m1);
	pthread_cond_wait(&connexion);
	pthread_mutex_lock(&m1);
	
   /* while(temps_ecoule!=0){
        if(donnees_partie.connexion==1){		
            printf("Le deuxième joueur s'est connecter, la partie commence.\n");	
			temps_ecoule=0;
        }
		else{
			sleep(1);
			temps_ecoule=alarm(0);
		
		}
    }//end while*/
   /* if(donnees_partie.connexion==0){
        printf("Le deuxième joueur ne s'sest pas connecté a temps\n");
    }*/
	
	
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
    scle=ftok(nom_partie, 0));

    id=ouverture_memoire_partage(cle);

    attachement_memoire_partage(id);

    //donnees_partie.connexion=1; //previent qu'il s'est connecté
	pthread_mutex_lock(&m1);
	pthread_cond_signal(&connexion);
	pthread_mutex_lock(&m1);
	
	afficher_jeu(); //affichage du plateau, des "labels"...

}


void reprise_partie_sauvegarde(void){


}


/* a mettre dans gestiontemps.c*/

//sigjmp_buf contexteAlarme;
void gestionAlarme(int numSig) {

	pthread_mutex_lock(&m1);
	pthread_cond_signal(&connexion);
	printf("Le deuxième joueur ne s'sest pas connecté a temps\n");
	pthread_mutex_lock(&m1);

}

