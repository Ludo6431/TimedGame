/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appel�es par choix_menu
*/

#include "tools.h" // readStdin
#include "game.h"
#include "shm.h"
#include "menu_functions.h"

sShm* adresse; 


//sShm *nouvelle_partie(sGame *g) {
sGame *nouvelle_partie(sShm* adresse) {
    char tmp[256];
    time_t ttmp;
    key_t shmkey;
    int shmid;
    sGame g;


    printf("Quel nom voulez-vous donner � votre partie ?\n")
    readStdin(tmp, sizeof(tmp));

    game_new(g, tmp); // game_new intialise le jeu

    /*r�cup�ration des informations sur la partie et initialisation de g*/
    printf("Entrez votre nom :\n")
    readStdin(tmp, sizeof(tmp))
    game_set_playername(g, P_1, tmp);
    
    printf("Entrez la dur�e de la partie (en secondes) : \n")
    readStdin(tmp, sizeof(tmp))
    game_set_totaltime(g, (time_t)atoi(tmp));

    printf("Entrez le temps allou� � un coup : \n")
    readStdin(tmp, sizeof(tmp))
    ttmp = atoi(tmp);
    game_set_turntime(g, ttmp);
    game_set_remainingtime(g, ttmp);


    /* cr�ation de la clef */
    shmkey=ftok(game_get_fi
    printf("Entrez le temps allou� � un coup : \n")lepath(g), 0);
    /* Creation d'un segment de memoire partagee de 80 octets */
    if((shmid = shmget(shmkey, sizeof(sShm), 0600|IPC_CREAT))==-1)
        exitOnErrSyst("shmget", NULL);

    if((int)(adresse = shmat(shmid, NULL, 0)) == -1) {
        shmctl(shmid, IPC_RMID, NULL); /* Suppression du segment */
        exitOnErrSyst("shmat", NULL);
    }

    shm_lock(adresse);

    /*�criture dans la memoire partag�e*/

    adresse.stp=g.player;
    adresse.stg=g.state;

    if(pthread_cond_init(&adresse->catt,&adresse->attratt)!=0){
	exitOnErrSyst("pthread_cond_init", NULL);
    }
    if(pthread_condattr_init(&adresse->attratt)=!0){
	exitOnErrSyst("pthread_condattr_init", NULL);
    }
    if(pthread_condattr_setpthread(&adresse->attratt,PTHREAD_PROCESS_SHARED)=!0){
	exitOnErrSyst("pthread_condattr_setpthread", NULL);
    }

    shm_unlock(adresse);

    printf("Vous �tes le joueur n�1\n");


    /* Installation du handler gestionAlarme pour SIGALRM */
    signal(SIGALRM,alarm_connexion);

    //utiliser var cond pthread_condattr_setpshared, faire toute l'initialisation de la var conditionnelle
    alarm(30);

    pthread_mutex_lock(adresse->matt);
    pthread_cond_wait(adresse->catt);
    pthread_mutex_lock(adresse->matt);

	if(adresse->stp==P_1){
		printf(" Le deuxi�me joueur ne s'est pas connect� � temps");
		//detruire la memoire partag�e ? 
	}

	else{
		printf(" La partie commence");
		//afficher le jeu
	}

    return g;
}




sGame* connexion(sGame *g){

    char tmp[256];
    time_t ttmp;
    key_t shmkey;
    int shmid;
   // sShm* adresse;
    sGame *g;

    printf("Entrez le nom de la partie que vous voulez rejoindre :\n")
    readStdin(tmp, sizeof(tmp));

    game_new(g, tmp); // game_new intialise le jeu

    /*r�cup�ration des informations sur la partie et initialisation de g*/
    printf("Entrez votre nom :\n")
    readStdin(tmp, sizeof(tmp))
    game_set_playername(g, P_2, tmp);
    

    /* cr�ation de la clef */
    shmkey=ftok(game_get_filepath(g), 0);
    /* Ouverture d'un segment de memoire partagee de 80 octets */
    if((shmid = shmget(shmkey, sizeof(sShm), 0600|IPC_CREAT))==-1)
        exitOnErrSyst("shmget", NULL);
    if((int)(adresse = shmat(shmid, NULL, 0)) == -1) {
        shmctl(shmid, IPC_RMID, NULL); /* Suppression du segment */
        exitOnErrSyst("shmat", NULL);
    }

    shm_lock(adresse);

    /*�criture dans la memoire partag�e*/

    adresse.stp=g.player; //permet au P_1 de tester si le P_2 s'est connect�

    shm_unlock(adresse);

    pthread_mutex_lock(adresse->matt);
    pthread_cond_signal(adresse->catt);
    pthread_mutex_lock(adresse->matt);

    printf("Vous �tes le joueur n�2\n");
    //mettre les champs de temps de jeu et de coup de g a jour ( rajouter cela dans la structure sShm et le r�cup�rer
    // afficher jeu

	return g;
}



void reprise_partie_sauvegarde(void){


}


/* a mettre dans game.c ..?*/
void alarm_connexion(int numSig) {

    pthread_mutex_lock(adresse->matt);
    pthread_cond_signal(adresse->catt);
    pthread_mutex_lock(adresse->matt);

}


