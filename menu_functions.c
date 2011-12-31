/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
*/
#include <stdio.h>  // printf
#include <pthread.h>    // pthread_*
#include <errno.h>  // errno
#include <sys/types.h>  // key_t
#include <sys/ipc.h>    // ftok
#include <signal.h> // SIGUSR1

#include "tools.h"  // readStdin, exitOnErrSyst
#include "game.h"   // game_*
#include "timer.h"  // timer_*
#include "longjump.h"   // long jump stuff
#include "sigmsg.h" // sigmsg*
#include "msgs.h"   // msgs_handler

#include "menu_functions.h"

int nouvelle_partie(sGame *g) {
    char tmp[256];
    time_t ttmp;
    key_t key;

    /* récupération des informations sur la partie et initialisation de g */
    printf("Nom de la partie       : ");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu

    printf("Votre pseudo           : ");
    readStdin(tmp, sizeof(tmp));
    game_set_playername(g, P_1, tmp);

    printf("Durée de la partie (s) : ");
    readStdin(tmp, sizeof(tmp));
    game_set_totaltime(g, (time_t)atoi(tmp));

    printf("Durée par coup (s)     : ");
    readStdin(tmp, sizeof(tmp));
    ttmp = atoi(tmp);
    game_set_turntime(g, ttmp);
    game_set_remainingtime(g, ttmp);

    /* création fichier .histo vide pour créer la mémoire partagée */
    game_histo_save(g);

    /* création canal de communication */
    if((key=ftok(game_get_filepath(g), 0))==(key_t)-1)
        exitOnErrSyst("ftok", NULL);

    if(sigmsgget(key, 0600|IPC_CREAT|IPC_EXCL)==-1)
        exitOnErrSyst("sigmsgget", NULL);

    if(sigmsgreg(SIGUSR1, (sigmsghnd)msgs_handler, (void *)g)==-1)
        exitOnErrSyst("sigmsgreg", NULL);

    /* ok, let's go ! */
    printf("Vous êtes le joueur n°1\n");    // FIXME: l'utilisateur n'aura pas le temps de le voir parce que le menu va l'effacer

    void _update(int sig, int t, void *data) {
        printf("\x1b[s\x1b[0;0H");

        printf("Il te reste %02d secondes", t);

        printf("\x1b[u");
        fflush(stdout);
    }

    timer_start(30, &jumpenv, LJUMP_TIMER, _update, NULL);

    return 0;
}

#if 0
sShm *connexion(sGame *g) {
    char tmp[256];
    sShm *shm;
    LIST *l, *ltmp;

    printf("Parties ouvertes :\n");
    ltmp=l=game_histo_getlist();
    while(ltmp) {
        printf("\t%s\n", (char *)ltmp->data);
        // TODO vérifier que les parties sont en cours

        ltmp=ltmp->next;
    }

    printf("Entrez le nom de la partie que vous voulez rejoindre :\n");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu
    // TODO choisir une partie dans la liste avec son numéro ?

    /*récupération des informations sur la partie et initialisation de g*/
    printf("Vous êtes le joueur n°2\n");
    game_set_me(g, P_2);

    printf("Entrez votre nom :\n");
    readStdin(tmp, sizeof(tmp));
    game_set_player(g, P_2, tmp);

    // ouverture de la mémoire partagée existante
    shm = shm_open(game_get_filepath(g), 0 /* attachement */);

    /* écriture dans la memoire partagée */
    shm_lock(shm);

    shm_unlock(shm);

    return shm;
}

void reprise_partie_sauvegarde(void) {


}
/* a mettre dans game.c ..?*/
void alarm_connexion(int numSig) {

    pthread_mutex_lock(adresse->matt);
    pthread_cond_signal(adresse->catt);
    pthread_mutex_lock(adresse->matt);

}

void retour_menu(sGame *game, sShm *shm) {
    // TODO synchronize with other player

    pthread_cond_destroy(&shm->catt);

    shm_close(shm, 1 /* destroy */);
}
#endif

