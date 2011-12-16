/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
*/

#include "tools.h" // readStdin
#include "game.h"
#include "shm.h"
#include "menu_functions.h"

sShm *nouvelle_partie(sGame *g) {
    char tmp[256];
    time_t ttmp;
    sShm *shm;
    pthread_condattr_t condattr;
    int rc;

    /* récupération des informations sur la partie et initialisation de g */
    printf("Nom de la partie       : ")
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu

    printf("Votre pseudo           : ")
    readStdin(tmp, sizeof(tmp))
    game_set_playername(g, P_1, tmp);
    
    printf("Durée de la partie (s) : ")
    readStdin(tmp, sizeof(tmp))
    game_set_totaltime(g, (time_t)atoi(tmp));

    printf("Durée par coup (s)     : ")
    readStdin(tmp, sizeof(tmp))
    ttmp = atoi(tmp);
    game_set_turntime(g, ttmp);
    game_set_remainingtime(g, ttmp);

    /* création mémoire partagée */
    shm = shm_open(game_get_filepath(g), 1 /* création */);

    // initialisation attributs variables conditionnelles
    if(rc=pthread_condattr_init(&condattr)) {
        errno=rc;
        exitOnErrSyst("pthread_condattr_init", NULL);
    }
    if(rc=pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED)){
        errno=rc;
        exitOnErrSyst("pthread_condattr_setpthread", NULL);
    }

    shm_lock(shm);  // zone protégée

    shm->stp=g.player;
    shm->stg=g.state;

    if(rc=pthread_cond_init(&shm->catt, &condattr)){
        errno=rc;
        exitOnErrSyst("pthread_cond_init", NULL);
    }

    shm_unlock(shm);

    // destruction attributs variables conditionnelles
    if(rc=pthread_condattr_destroy(&condattr)) {
        errno=rc;
        exitOnErrSyst("pthread_condattr_destroy", NULL);
    }

    printf("Vous êtes le joueur n°1\n");

    /* Installation du handler gestionAlarme pour SIGALRM */
    signal(SIGALRM, alarm_connexion);

    //utiliser var cond pthread_condattr_setpshared, faire toute l'initialisation de la var conditionnelle
    alarm(30);

    pthread_mutex_lock(shm->matt);
    pthread_cond_wait(shm->catt);
    pthread_mutex_lock(shm->matt);

    if(shm->stp==P_1){
        printf(" Le deuxième joueur ne s'est pas connecté à temps");
        //detruire la memoire partagée ? 
    }
    else{
        printf(" La partie commence");
        //afficher le jeu
    }

    return shm;
}

sShm *connexion(sGame *g) {
    char tmp[256];
    time_t ttmp;
    key_t shmkey;
    int shmid;
    sShm *shm;

    // TODO lister parties (cf game_histo_*) et en choisir une

    printf("Entrez le nom de la partie que vous voulez rejoindre :\n")
    readStdin(tmp, sizeof(tmp));

    game_new(g, tmp); // game_new intialise le jeu

    /*récupération des informations sur la partie et initialisation de g*/
    printf("Entrez votre nom :\n")
    readStdin(tmp, sizeof(tmp))
    game_set_playername(g, P_2, tmp);
    
    // ouverture de la mémoire partagée existante
    shm = shm_open(game_get_filepath(g), 0 /* attachement */);

    shm_lock(adresse);

    /*écriture dans la memoire partagée*/

    adresse.stp=g.player; //permet au P_1 de tester si le P_2 s'est connecté

    shm_unlock(adresse);

    pthread_mutex_lock(adresse->matt);
    pthread_cond_signal(adresse->catt);
    pthread_mutex_lock(adresse->matt);

    printf("Vous êtes le joueur n°2\n");
    //mettre les champs de temps de jeu et de coup de g a jour ( rajouter cela dans la structure sShm et le récupérer
    // afficher jeu

    return shm;
}

void reprise_partie_sauvegarde(void){


}

/* a mettre dans game.c ..?*/
void alarm_connexion(int numSig) {

    pthread_mutex_lock(adresse->matt);
    pthread_cond_signal(adresse->catt);
    pthread_mutex_lock(adresse->matt);

}

