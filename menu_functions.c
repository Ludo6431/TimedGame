/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appel�es par choix_menu
*/
#include <stdio.h>  // printf
#include <pthread.h>    // pthread_*
#include <errno.h>  // errno

#include "tools.h"  // readStdin, exitOnErrSyst
#include "game.h"   // game_*
#include "shm.h"    // shm_*
#include "timer.h"  // timer_*

#include "menu_functions.h"

sShm *nouvelle_partie(sGame *g) {
    char tmp[256];
    time_t ttmp;
    sShm *shm;
    pthread_condattr_t condattr;
    int rc;

    /* r�cup�ration des informations sur la partie et initialisation de g */
    printf("Nom de la partie       : ");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu

    printf("Votre pseudo           : ");
    readStdin(tmp, sizeof(tmp));
    game_set_playername(g, P_1, tmp);

    printf("Dur�e de la partie (s) : ");
    readStdin(tmp, sizeof(tmp));
    game_set_totaltime(g, (time_t)atoi(tmp));

    printf("Dur�e par coup (s)     : ");
    readStdin(tmp, sizeof(tmp));
    ttmp = atoi(tmp);
    game_set_turntime(g, ttmp);
    game_set_remainingtime(g, ttmp);

    /* cr�ation fichier .histo vide pour cr�er la m�moire partag�e */
    game_histo_save(g);

    /* cr�ation m�moire partag�e */
    shm = shm_open(game_get_filepath(g), 1 /* cr�ation */);

    // initialisation attributs variables conditionnelles
    if((rc=pthread_condattr_init(&condattr))) {
        errno=rc;
        exitOnErrSyst("pthread_condattr_init", NULL);
    }
    if((rc=pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED))) {
        errno=rc;
        exitOnErrSyst("pthread_condattr_setpthread", NULL);
    }

    shm_lock(shm);  // zone prot�g�e

    shm->stp=g->player;
    shm->stg=g->state;

    if((rc=pthread_cond_init(&shm->catt, &condattr))) {
        errno=rc;
        exitOnErrSyst("pthread_cond_init", NULL);
    }

    shm_unlock(shm);

    // destruction attributs variables conditionnelles
    if((rc=pthread_condattr_destroy(&condattr))) {
        errno=rc;
        exitOnErrSyst("pthread_condattr_destroy", NULL);
    }

    printf("Vous �tes le joueur n�1\n");

    void _update(int sig, int t, void *data) {
        printf("\x1b[s\x1b[0;0H");

        printf("Il te reste %02d secondes", t);

        printf("\x1b[u");
        fflush(stdout);
    }

    timer_start(30, _update, NULL);

    return shm;
}

sShm *connexion(sGame *g) {
    char tmp[256];
    sShm *shm;
    LIST *l, *ltmp;

    printf("Parties ouvertes :\n");
    ltmp=l=game_histo_getlist();
    while(ltmp) {
        printf("\t%s\n", (char *)ltmp->data);
        // TODO v�rifier que les parties sont en cours

        ltmp=ltmp->next;
    }

    printf("Entrez le nom de la partie que vous voulez rejoindre :\n");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu
    // TODO choisir une partie dans la liste avec son num�ro ?

    /*r�cup�ration des informations sur la partie et initialisation de g*/
    printf("Vous �tes le joueur n�2\n");
    game_set_me(g, P_2);

    printf("Entrez votre nom :\n");
    readStdin(tmp, sizeof(tmp));
    game_set_player(g, P_2, tmp);

    // ouverture de la m�moire partag�e existante
    shm = shm_open(game_get_filepath(g), 0 /* attachement */);

    /* �criture dans la memoire partag�e */
    shm_lock(shm);

    shm_unlock(shm);

    return shm;
}

#if 0
void reprise_partie_sauvegarde(void) {


}
/* a mettre dans game.c ..?*/
void alarm_connexion(int numSig) {

    pthread_mutex_lock(adresse->matt);
    pthread_cond_signal(adresse->catt);
    pthread_mutex_lock(adresse->matt);

}
#endif

void retour_menu(sGame *game, sShm *shm) {
    // TODO synchronize with other player

    pthread_cond_destroy(&shm->catt);

    shm_close(shm, 1 /* destroy */);
}

