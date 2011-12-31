/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appel�es par choix_menu
*/
#include <stdio.h>  // printf
#include <pthread.h>    // pthread_*
#include <errno.h>  // errno
#include <sys/types.h>
#include <sys/ipc.h>    // IPC_*

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

    /* r�cup�ration des informations sur la partie et initialisation de g */
    printf("Nom de la partie       : ");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu

    printf("Vous �tes le joueur n�1\n");
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

    /* cr�ation canal de communication */
    if(msg_init(game_get_filepath(g), 0600|IPC_CREAT|IPC_EXCL)==-1)
        exitOnErrSyst("msg_init", NULL);

    /* on peut d�marrer le timer d'attente de connexion */
    void _update(int sig, int t, void *data) {
        printf("\x1b[s\x1b[0;0H");

        printf("Il te reste %02d secondes", t);

        printf("\x1b[u");
        fflush(stdout);
    }

    timer_start(30, &jumpenv, LJUMP_TIMER, _update, NULL);

    return 0;
}

int connexion(sGame *g) {
    char tmp[256];
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
    game_set_playername(g, P_2, tmp);

    // ouverture de la m�moire partag�e existante
    if(msg_init(game_get_filepath(g), 0600)==-1)
        exitOnErrSyst("msg_init", NULL);

    return 0;
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

void retour_menu(sGame *g) {
    msg_deinit(!g->me);  // destroy if we are the host of the game
}

