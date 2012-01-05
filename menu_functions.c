/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
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
    sGameConf *conf;
    char tmp[256];
    time_t ttmp;

    /* récupération des informations sur la partie et initialisation de g */
    printf("Nom de la partie       : ");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu
    conf=game_get_conf(g, NULL);

    printf("Vous êtes le joueur n°1\n");
    printf("Votre pseudo           : ");
    readStdin(tmp, sizeof(tmp));
    strcpy(conf->playername[P_1], tmp);

    // TODO: demander qui commence la partie et modifier conf->firstplayer

    printf("Durée de la partie (s) : ");
    readStdin(tmp, sizeof(tmp));
    ttmp = (time_t)atoi(tmp);
    conf->t_total=ttmp;
    game_set_remainingtime(g, ttmp);

    printf("Durée par coup (s)     : ");
    readStdin(tmp, sizeof(tmp));
    conf->t_turn=(time_t)atoi(tmp);

    /* création fichier .histo vide pour créer la mémoire partagée */
    game_histo_save(g);

    /* création canal de communication */
    if(msg_init(game_get_filepath(g), 0600|IPC_CREAT|IPC_EXCL, (void *)g)==-1)
        exitOnErrSyst("msg_init", NULL);

    /* on peut démarrer le timer d'attente de connexion */
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
    unsigned int utmp;
    LIST *l, *ltmp;
    sMsg msg;

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
    // TODO: choisir une partie dans la liste avec son numéro ?

    /* récupération des informations sur la partie et initialisation de g */
    printf("Vous êtes le joueur n°2\n");
    game_set_me(g, P_2);

    printf("Entrez votre nom :\n");
    readStdin(tmp, sizeof(tmp));
    utmp=strlen(tmp)+1;
//    game_set_playername(g, P_2, tmp);

    // ouverture de la mémoire partagée existante
    if(msg_init(game_get_filepath(g), 0600, g)==-1)
        exitOnErrSyst("msg_init", NULL);

    // on se signale à l'autre processus (en indiquant notre nom)
    msg.type=MSG_JOINGAME;
    strcpy(msg.data, tmp);
    if(msg_transfer(&msg, &utmp)==-1)
        exitOnErrSyst("msg_transfer", tmp);

    // l'autre processus devrait nous renvoyer la conf complète du jeu
    if(msg.type==MSG_CONFUPDATE) {
        sGameConf *conf = (sGameConf *)msg.data;

        game_set_conf(g, conf);    // configuration synchronized
        game_set_player(g, conf->firstplayer);

        // FIXME: attention, ceci est recouvert par le menu
        printf("Configuration du jeu :\n");
        printf("Joueur 1 : %s\n", conf->playername[P_1]);
        printf("Joueur 2 : %s\n", conf->playername[P_2]);
        printf("Le joueur %d commence\n", conf->firstplayer+1);
        printf("Durée de la partie : %us\n", (unsigned int)conf->t_total);
        printf("Durée par coup     : %us\n", (unsigned int)conf->t_turn);
    }
    else {
        // TODO: return to menu nicely or ask the config
        fprintf(stderr, "we didn't get the config\n");
    }

    return 0;
}

void reprise_partie_sauvegarde(sGame *g) {
    // TODO
}

void sauvegarder(sGame *g) {
    // TODO
}

void pause(sGame *g) {
    // TODO
}

void reprendre(sGame *g) {
    // TODO
}

void afficher_historique(sGame *g) {
    // TODO
}

void jouer_coup(sGame *g, char *s) {
    sGameTurn turn;
    sMsg msg;

    turn.player=game_get_player(g);
    turn.t_remaining=game_get_remainingtime(g);

    if(!strcasecmp(s, "normal") || !strcasecmp(s, ":)"))
        turn.type=T_OK;
    else if(!strcasecmp(s, "gagne") || !strcasecmp(s, ":D"))
        turn.type=T_WIN;
    else
        turn.type=T_INVALID;

    // FIXME: attention, ceci est recouvert par le menu
    printf("%s : ", game_get_conf(g, NULL)->playername[turn.player]);

    switch(turn.type) {
    case T_OK:
        printf("coup normal :)\n");
        break;
    case T_INVALID:
        printf("coup invalide :x\n");
        break;
    case T_WIN:
        printf("coup gagnant :D\n");
        break;
    }

    game_playturn(g, &turn);

    msg.type=MSG_GAMETURN;
    memcpy(msg.data, &turn, sizeof(turn));
    msg_send(&msg, sizeof(turn));
}

void retour_menu(sGame *g) {
    msg_deinit(!g->me);  // destroy if we are the host of the game
}

