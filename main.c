/************************** main.c ************************/
#include <stdio.h>
#include <signal.h>

#include "menu.h"   // menu_run
#include "game.h"
#include "menu_functions.h" // ...
#include "timer.h"  // timer_stop
#include "msgs.h"   // to get the last received message
#include "longjump.h"   // long jump stuff

void _timer_conn(int sig, int t, sGame *g) {
    printf("%c7\x1b[2;0H", '\x1b'); // on sauvegarde la position du curseur actuelle et on se place en haut de l'écran

    printf("Temps restant connexion : %02ds", t);

    printf("%c8", '\x1b');  // on restaure la position sauvegardée du curseur
    fflush(stdout);
}

void _timer_turn(int sig, int t, sGame *g) {
    printf("%c7\x1b[2;0H", '\x1b'); // on sauvegarde la position du curseur actuelle et on se place en haut de l'écran

    printf("Temps restant tour  : %02ds", t);

    printf("%c8", '\x1b');  // on restaure la position sauvegardée du curseur
    fflush(stdout);
}

void _timer_glob(int sig, int t, sGame *g) {
    printf("%c7\x1b[1;0H", '\x1b'); // on sauvegarde la position du curseur actuelle et on se place en haut de l'écran

    game_get_state(g, NULL)->t_remaining=t;   // update remaining time

    printf("Temps total restant : %02ds", t);

    printf("%c8", '\x1b');  // on restaure la position sauvegardée du curseur
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    eMenuState MenuState=M_MAIN;
    sGame game; // partie en cours
    sGameConf *conf = game_get_conf(&game, NULL);
    sGameState *state = game_get_state(&game, NULL);
    sMsg msg;
    char buf[256], ps1[16]="> ";
    char *choix;
    int ljump, ret;
    char msgmenu[64]={0}; // one-line message in the menu

    sTimer timer_glob={ 0, (timer_handler)_timer_glob, (void *)&game, &jumpenv, LJUMP_TIMER_GLOB };
    sTimer timer_conn={ 0, (timer_handler)_timer_conn, (void *)&game, &jumpenv, LJUMP_TIMER_CONN };
    sTimer timer_turn={ 0, (timer_handler)_timer_turn, (void *)&game, &jumpenv, LJUMP_TIMER_TURN };

    while(1) {
        printf("\x1b[2J\x1b[0;0H");  // clear screen & go to the upper left hand corner

        // TODO: flush the input buffer before printing the new menu

        printf("\n\n");

        if(MenuState==M_MAIN)
            strcpy(ps1, "> ");
        else
            sprintf(ps1, "%s> ", conf->playername[game.pme]);    // print our name

        if(!(choix=menu_run(MenuState, msgmenu, ps1, buf, sizeof(buf))) || !strlen(choix)) {
            strcpy(msgmenu, "Entrée invalide");
            continue;   // loop
        }

        switch(choix[0]) {
        case '1':   // M_MAIN, "Nouvelle partie"
            nouvelle_partie(&game);

            timer_start(&timer_conn, 30);   // start the 30s connexion timer

            MenuState=M_WAITCON;
            break;
        case '2':   // M_MAIN, "Connexion à une partie"
            if(!(ret=connexion(&game))) { // ok
                MenuState=(game_isit_myturn(&game)?M_MYTURN:M_HISTURN);

                sprintf(msgmenu, "Vous venez de rejoindre %s, partie en %us (%us par tour)", conf->playername[!game.pme], (unsigned int)conf->t_total, (unsigned int)conf->t_turn);

                timer_start(&timer_glob, state->t_remaining);
                if(MenuState==M_MYTURN)
                    timer_start(&timer_turn, conf->t_turn);
            }
            else if(ret==-2)
                strcpy(msgmenu, "Il n'y a aucune partie en attente de joueurs");
            break;
        case '3':   // M_MAIN, "Charger une partie sauvegardée"
            if(!(ret=reprise_partie_sauvegarde(&game))) { // ok
                timer_start(&timer_conn, 30);

                MenuState=M_WAITCON;
            }
            else if(ret==-2)
                strcpy(msgmenu, "Il n'y a aucune partie sauvegardee");
            break;
        case '4':   // M_MYTURN|M_HISTURN|M_PAUSED, "Stopper en sauvegardant" -> retour au menu principal
            if(MenuState==M_MYTURN)
                timer_stop(&timer_turn);
            timer_stop(&timer_glob);

            sauvegarder(&game);

            msg.type=MSG_END;
            msg.data[0]=0;  // keep histo file
            strcpy(&msg.data[1], "L'autre joueur a quitte en sauvegardant");
            msg_send(&msg, 1+strlen(&msg.data[1])+1);

            retour_menu(&game, 0 /* keep histo file */);

            MenuState=M_MAIN;
            break;
        case '5':   // M_MYTURN, "Mettre en pause"
            timer_pause(&timer_turn);
            timer_pause(&timer_glob);

            msg.type=MSG_PAUSE;
            msg_send(&msg, 0);

            MenuState=M_PAUSED;
            break;
        case '6':   // M_PAUSED, "Reprendre"
            timer_resume(&timer_glob);
            timer_resume(&timer_turn);

            msg.type=MSG_RESUME;
            *(int *)msg.data=timer_get(&timer_glob);
            msg_send(&msg, sizeof(int));

            MenuState=(game_isit_myturn(&game)?M_MYTURN:M_HISTURN);
            break;
        case '7':   // M_MYTURN|M_HISTURN|M_PAUSED, "Visualiser l'historique"
            if(MenuState==M_MYTURN)
                timer_pause(&timer_turn);
            timer_pause(&timer_glob);

            msg.type=MSG_PAUSE;
            msg_send(&msg, 0);

            afficher_historique(&game);

            MenuState=M_PAUSED;
            break;
        case '8':   // M_MYTURN|M_HISTURN|M_PAUSED, "Stopper en visualisant l'historique" -> retour au menu principal (sans sauvegarder)
            if(MenuState==M_MYTURN)
                timer_stop(&timer_turn);
            timer_stop(&timer_glob);

            afficher_historique(&game);

            msg.type=MSG_END;
            msg.data[0]=1;  // delete histo file
            strcpy(&msg.data[1], "L'autre joueur a quitte");
            msg_send(&msg, 1+strlen(&msg.data[1])+1);

            retour_menu(&game, 1 /* delete histo file */);

            MenuState=M_MAIN;
        case '9':   // M_*
            if(MenuState==M_MAIN) { // M_MAIN, "Quitter"
                exit(0);
            }
            else {  // M_WAITCON|M_MYTURN|M_HISTURN|M_PAUSED, "Retour au menu principal" (sans sauvegarder)
                if(MenuState==M_MYTURN)
                    timer_stop(&timer_turn);
                if(MenuState==M_WAITCON)
                    timer_stop(&timer_conn);
                else
                    timer_stop(&timer_glob);

                msg.type=MSG_END;
                msg.data[0]=1;  // delete histo file
                strcpy(&msg.data[1], "L'autre joueur a quitte");
                msg_send(&msg, 1+strlen(&msg.data[1])+1);

                retour_menu(&game, 1 /* delete histo file */);

                MenuState=M_MAIN;
            }
            break;
        case '/':   // M_MYTURN, "Jouer un coup"
            timer_stop(&timer_turn);

            ret=jouer_coup(&game, choix+1);
            if(state->state==GS_WIN) {
                timer_stop(&timer_glob);

                retour_menu(&game, 1 /* delete histo file */);

                strcpy(msgmenu, "Bravo, tu as gagne");

                MenuState=M_MAIN;
            }
            else
                MenuState=(game_isit_myturn(&game)?M_MYTURN:M_HISTURN);

            if(ret==-2)
                strcpy(msgmenu, "Coup invalide");

            if(MenuState==M_MYTURN)
                timer_start(&timer_turn, conf->t_turn);
            break;
        }

        // save the environment here, we will come back here if the timer expires or if we receive some messages from the other process (like end_of_game, ...)
        switch((ljump=sigsetjmp(jumpenv, 1 /* save signals mask */))) {
        case 0: // ok, environment saved
            break;
        case LJUMP_TIMER_CONN:   // M_WAITCON, timer expired
            strcpy(msgmenu, "Temps d'attente de connexion écoulé");

            retour_menu(&game, 1 /* delete histo file */);

            MenuState=M_MAIN;
            break;
        case LJUMP_TIMER_TURN:   // M_MYTURN, timer expired
            timer_stop(&timer_glob);

            strcpy(msgmenu, "Temps écoulé, tu as perdu");

            msg.type=MSG_END;
            msg.data[0]=1;  // delete histo file
            strcpy(&msg.data[1], "Temps écoulé pour l'autre joueur, tu as gagné");
            msg_send(&msg, 1+strlen(&msg.data[1])+1);

            retour_menu(&game, 1 /* delete histo file */);

            MenuState=M_MAIN;
            break;
        case LJUMP_TIMER_GLOB:   // M_MYTURN|M_HISTURN, timer expired
            if(MenuState==M_MYTURN)
                timer_stop(&timer_turn);

            strcpy(msgmenu, "Temps de la partie écoulé, match nul");

            msg.type=MSG_END;
            msg.data[0]=1;  // delete histo file
            strcpy(&msg.data[1], "Temps de la partie écoulé, match nul");
            msg_send(&msg, 1+strlen(&msg.data[1])+1);

            retour_menu(&game, 1 /* delete histo file */);

            MenuState=M_MAIN;
            break;
        case LJUMP_ISR: // the other process ask me to do something
            switch(last_msg.type) {
            case MSG_READY: // M_WAITCON
                timer_stop(&timer_conn);   // ok someone is here, we can stop the connection wait timer

                sprintf(msgmenu, "%s vient de vous rejoindre dans la partie", conf->playername[!game.pme]);

                MenuState=(game_isit_myturn(&game)?M_MYTURN:M_HISTURN);

                timer_start(&timer_glob, state->t_remaining);
                if(MenuState==M_MYTURN)
                    timer_start(&timer_turn, conf->t_turn);
                break;
            case MSG_TURN:  // M_HISTURN|M_PAUSED, the other process send us the turn the player just did
                game_playturn(&game, (sGameTurn *)last_msg.data);   // play this turn locally
                if(state->state==GS_WIN) {
                    timer_stop(&timer_glob);

                    retour_menu(&game, 1 /* delete histo file */);

                    strcpy(msgmenu, "Tu as perdu");

                    MenuState=M_MAIN;
                }
                else {
                    MenuState=(game_isit_myturn(&game)?M_MYTURN:M_HISTURN);

                    // sync the two global timers
                    timer_set(&timer_glob, ((sGameTurn *)last_msg.data)->t_remaining);
                }

                if(MenuState==M_MYTURN)
                    timer_start(&timer_turn, conf->t_turn);
                break;
            case MSG_PAUSE: // 
                if(MenuState==M_MYTURN)
                    timer_pause(&timer_turn);
                timer_pause(&timer_glob);

                MenuState=M_PAUSED;
                break;
            case MSG_RESUME:
                MenuState=(game_isit_myturn(&game)?M_MYTURN:M_HISTURN);

                if(pid_less!=0) {
                    kill(pid_less, SIGTERM);
                    pid_less=0;
                }

                if(MenuState==M_MYTURN)
                    timer_resume(&timer_turn);
                timer_set(&timer_glob, *(int *)last_msg.data);
                timer_resume(&timer_glob);
                break;
            case MSG_END:   // M_MYTURN|M_HISTURN|M_PAUSED, the other process quit
                if(MenuState==M_MYTURN)
                    timer_stop(&timer_turn);
                timer_stop(&timer_glob);

                strcpy(msgmenu, &last_msg.data[1]);

                retour_menu(&game, last_msg.data[0]); // we quit aswell

                MenuState=M_MAIN;
                break;
            default:
                fprintf(stderr, "unhandled received msg(%d)\n", last_msg.type);
                break;
            }
            break;
        default:
            fprintf(stderr, "unhandled long jump(%d)\n", ljump);
            break;
        }
    }   // end while

    return 0;
}

