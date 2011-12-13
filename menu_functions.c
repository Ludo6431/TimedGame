/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
*/

#include "tools.h"  // readStdin
#include "game.h"

#include "menu_functions.h"

sShm *nouvelle_partie(sGame *g) {
    char tmp[256];
    time_t ttmp;
    key_t shmkey;
    int shmid;
    sShm* adresse;

    printf("Quel nom voulez-vous donner à votre partie ?\n")
    readStdin(tmp, sizeof(tmp));

    game_new(g, tmp);    // game_new intialise le jeu

    /*récupération des informations sur la partie et initialisation de g*/
    printf("Entrez votre nom :\n")
    readStdin(tmp, sizeof(tmp))
    game_set_playername(g, P_1, tmp);
    
    printf("Entrez la durée de la partie (en secondes) : \n")
    readStdin(tmp, sizeof(tmp))
    game_set_totaltime(g, (time_t)atoi(tmp));

    printf("Entrez le temps alloué à un coup : \n")
    readStdin(tmp, sizeof(tmp))
    ttmp = atoi(tmp);
    game_set_turntime(g, ttmp);
    game_set_remainingtime(g, ttmp);


    /* création de la clef */
    shmkey=ftok(game_get_filepath(g), 0);
    /* Creation d'un segment de memoire partagee de 80 octets */
    if((shmid = shmget(shmkey, sizeof(sShm), 0600|IPC_CREAT))==-1)
        exitOnErrSyst("shmget", NULL);

    if((int)(adresse = shmat(shmid, NULL, 0)) == -1) {
        shmctl(shmid, IPC_RMID, NULL); /* Suppression du segment */
        exitOnErrSyst("shmat", NULL);
    }

    shm_lock(adresse);

    /*écriture dans la memoire partagée*/

    adresse.stp=g.player;
    adresse.stg=g.state;

    shm_unlock(adresse);

    printf("Vous êtes le joueur n°1\n");

    return g;
}

#endif







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

