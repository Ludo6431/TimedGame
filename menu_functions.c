/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
*/
#include <stdio.h>  // printf
#include <pthread.h>    // pthread_*
#include <errno.h>  // errno
#include <sys/types.h>
#include <sys/ipc.h>    // IPC_*
#include <unistd.h>
#include <sys/wait.h>

#include "tools.h"  // readStdin, exitOnErrSyst
#include "game.h"   // game_*
#include "timer.h"  // timer_*
#include "msgs.h"   // msgs_handler

#include "menu_functions.h"

typedef enum {
    GT_NOT_RUNNING,
    GT_RUNNING,
    GT_WAITING
} eGameTypes;

pid_t pid_less=0;

LIST *get_games(eGameTypes type) {
    LIST *l, *ret=NULL;
    sGame g;
    struct sigmsgid_ds buf;

    for(l=game_histo_getlist(); l; l=l->next) {
        game_new(&g, (char *)l->data);

        if(msg_ctl(&g, IPC_STAT, &buf)==-1) {   // can't open it => not running
            if(type==GT_NOT_RUNNING)
                ret=list_append(ret, l->data);

            continue;
        }

        if(buf.sigmsg_nattch==1)   // this game is lacking a player
            if(type==GT_WAITING)
                ret=list_append(ret, l->data);

        if(buf.sigmsg_nattch==2)   // this game is running
            if(type==GT_RUNNING)
                ret=list_append(ret, l->data);
    }

    return ret;
}

int nouvelle_partie(sGame *g) {
    sGameConf *conf=game_get_conf(g, NULL);
    sGameState *state=game_get_state(g, NULL);
    char tmp[256];
    time_t ttmp;

    /* récupération des informations sur la partie et initialisation de g */
    printf("Nom de la partie       : ");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu

    conf->phost=P_1;
    conf->pjoin=P_2;
    g->pme=P_1;

    printf("Votre pseudo           : ");
    readStdin(tmp, sizeof(tmp));
    strcpy(conf->playername[g->pme], tmp);

    printf("Durée de la partie (s) : ");
    readStdin(tmp, sizeof(tmp));
    ttmp = (time_t)atoi(tmp);
    conf->t_total=ttmp;
    state->t_remaining=ttmp;

    printf("Durée par coup (s)     : ");
    readStdin(tmp, sizeof(tmp));
    conf->t_turn=(time_t)atoi(tmp);

    while(1) {
        int i;

        printf("Qui commence ?\n\t1: vous (%s)\n\t2: votre adversaire\n", conf->playername[g->pme]);
        readStdin(tmp, sizeof(tmp));

        if(sscanf(tmp, "%d", &i)!=1)
            continue;

        if(i==1 || i==2) {
            state->pcurr=((i==1)?g->pme:!g->pme);
            break;
        }
    }

    /* création fichier .histo vide pour créer la mémoire partagée */
    game_histo_save(g);

    /* création canal de communication */
    if(msg_init(g, 1 /* create */)==-1)
        exitOnErrSyst("msg_init", NULL);

    return 0;
}

int connexion(sGame *g) {
    sGameConf *conf = game_get_conf(g, NULL);
    sGameState *state = game_get_state(g, NULL);
    char tmp[256];
    unsigned int utmp;
    LIST *ltmp;
    sMsg msg;

    while(1) {
        LIST *l;
        unsigned int i=0, j;

        ltmp=l=get_games(GT_WAITING);
        if(!ltmp)
            return -2;   // aucune partie en attente de joueur

        printf("Parties ouvertes :\n");
        while(ltmp) {
            printf("\t%d: %s\n", ++i, (char *)ltmp->data);

            ltmp=ltmp->next;
        }

        printf("Entrez le numéro de la partie que vous voulez rejoindre :\n");
        readStdin(tmp, sizeof(tmp));

        if(sscanf(tmp, "%d", &j)!=1)
            continue;

        if(j>0 && j<=i) {
            ltmp=l;
            i=0;
            while(++i!=j)
                ltmp=ltmp->next;

            break;  // ltmp is the selected game
        }
    }

    if(game_histo_load(g, (char *)ltmp->data))
        exitOnErrSyst("game_histo_load", NULL);

    // ouverture de la mémoire partagée existante
    if(msg_init(g, 0 /* connect to existant */)==-1)
        exitOnErrSyst("msg_init", NULL);

    // on se signale à l'autre processus (en indiquant notre nom)
    msg.type=MSG_JOIN;
    utmp=0; // no payload
    if(msg_transfer(&msg, &utmp)==-1)
        exitOnErrSyst("msg_transfer", NULL);

    // on n'a pas reçu l'état initial que doit nous envoyer l'hôte
    if(msg.type!=MSG_INITST8) {
        msg_deinit(0 /* don't destroy */);
        return -1;
    }

    /* récupération des informations sur la partie et initialisation de g */
    game_set_conf(g, &((sGameInit *)msg.data)->conf);    // configuration synchronized
    game_set_state(g, &((sGameInit *)msg.data)->st);

    g->pme=conf->pjoin;
    if(!strlen(conf->playername[g->pme])) { // we don't have a pseudo, let's ask one
        printf("Votre pseudo :\n");
        readStdin(tmp, sizeof(tmp));
        strcpy(conf->playername[g->pme], tmp);
    }

    // FIXME: attention, ceci est recouvert par le menu
    printf("Configuration du jeu :\n");
    printf("Vous       : %s\n", conf->playername[g->pme]);
    printf("Adversaire : %s\n", conf->playername[!g->pme]);
    printf("%s commence\n", conf->playername[state->pcurr]);
    printf("Durée de la partie : %us\n", (unsigned int)conf->t_total);
    printf("Durée par coup     : %us\n", (unsigned int)conf->t_turn);

    // let's say we are ready and give the full configuration to the hist (with our name)
    msg.type=MSG_READY;
    memcpy(msg.data, conf, sizeof(*conf));
    utmp=sizeof(*conf);
    if(msg_transfer(&msg, &utmp)==-1)
        exitOnErrSyst("msg_transfer", NULL);

    if(msg.type!=MSG_START) {
        msg_deinit(0 /* don't destroy */);
        return -1;
    }

    return 0;   // green light, let's go !!!
}

int reprise_partie_sauvegarde(sGame *g) {
    sGameConf *conf = game_get_conf(g, NULL);
    char tmp[256];
    LIST *ltmp;

    while(1) {
        LIST *l;
        unsigned int i=0, j;

        ltmp=l=get_games(GT_NOT_RUNNING);
        if(!ltmp)
            return -2;   // aucune partie sauvegardée

        printf("Parties sauvegardées :\n");
        while(ltmp) {
            printf("\t%d: %s\n", ++i, (char *)ltmp->data);

            ltmp=ltmp->next;
        }

        printf("Entrez le numéro de la partie que vous voulez ouvrir :\n");
        readStdin(tmp, sizeof(tmp));

        if(sscanf(tmp, "%d", &j)!=1)
            continue;

        if(j>0 && j<=i) {
            ltmp=l;
            i=0;
            while(++i!=j)
                ltmp=ltmp->next;

            break;  // ltmp is the selected game
        }
    }

    if(game_histo_load(g, (char *)ltmp->data))
        exitOnErrSyst("game_histo_load", NULL);

    while(1) {
        int i;

        printf("Qui êtes-vous ?\n\t1: %s\n\t2: %s\n", conf->playername[P_1], conf->playername[P_2]);
        readStdin(tmp, sizeof(tmp));

        if(sscanf(tmp, "%d", &i)!=1)
            continue;

        if(i==1 || i==2) {
            g->pme=((i==1)?P_1:P_2);
            conf->phost=g->pme;
            conf->pjoin=!g->pme;

            break;
        }
    }

    /* création canal de communication */
    if(msg_init(g, 1 /* create */)==-1)
        exitOnErrSyst("msg_init", NULL);

    return 0;
}

int sauvegarder(sGame *g) {
    if(game_histo_save(g))
        exitOnErrSyst("game_histo_save", NULL);

    return 0;
}

void afficher_historique(sGame *g) {
    sGameConf *conf = game_get_conf(g, NULL);
    int pid=0, tube[2];
    int status; /* pour code retour wait() */
    char buf[150];
    LIST *turns=g->turns;

    /* Creation du tube */
    if(pipe(tube)==-1)
        exitOnErrSyst("pipe","creation du pipe impossible");

    if((pid=fork())==-1)
        exitOnErrSyst("fork","creation fils");

    if(pid==0) {
        /* Code du fils */

        /* fermeture du descripteur en ecriture sur le tube (on ne l'utilise pas ici) */
        close(tube[1]);
        close(0);//pour pouvoir recopier tube[0]
        dup(tube[0]);
        close(tube[0]);

        printf("\x1b[2J\n");

        execlp("less", "less", (void *)NULL);
        // never reached
    }

    pid_less=pid;

    /* ecriture dans le tube des données de la partie */
    sprintf(buf, "Nom du Joueur 1 : %s \nNom du Joueur 2 : %s\n Temps total : %04u \n Temps par tour : %04u\r\n", conf->playername[P_1], conf->playername[P_2], (unsigned int)conf->t_total, (unsigned int)conf->t_turn);

    if((write(tube[1],buf,strlen(buf)))==-1)
        exitOnErrSyst("write","ecriture dans le tube");

    while(turns) {
        sGameTurn *turn=(sGameTurn *)turns->data;

        sprintf(buf, "%04u%s %u\r\n", (unsigned int)turn->t_remaining, g->conf.playername[((sGameTurn *)turns->data)->player], ((sGameTurn *)turns->data)->type);
         if((write(tube[1],buf,strlen(buf)))==-1)
                exitOnErrSyst("write","ecriture dans le tube");

        turns=turns->next;
    }

    /* fermeture des descripteur */
    close(tube[0]);
    close(tube[1]);

    wait(&status);

    pid_less=0;
}


void jouer_coup(sGame *g, char *s) {
    sGameConf *conf=game_get_conf(g, NULL);
    sGameState *state=game_get_state(g, NULL);
    sMsg msg;
    sGameTurn *turn = (sGameTurn *)msg.data;

    turn->player=state->pcurr;
    turn->t_remaining=state->t_remaining;

    if(!strcasecmp(s, "normal") || !strcasecmp(s, ":)"))
        turn->type=T_OK;
    else if(!strcasecmp(s, "gagne") || !strcasecmp(s, ":D"))
        turn->type=T_WIN;
    else
        turn->type=T_INVALID;

    // FIXME: attention, ceci est recouvert par le menu
    printf("%s : ", conf->playername[turn->player]);
    switch(turn->type) {
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

    game_playturn(g, turn);

    msg.type=MSG_TURN;
    msg_send(&msg, sizeof(*turn));
}

void retour_menu(sGame *g, int del) {
    if(game_ami_host(g)) {
        if(del)
            unlink(game_get_filepath(g));
        msg_deinit(1);  // destroy if we are the host of the game
    }
    else
        msg_deinit(0);
}

